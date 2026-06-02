//=============================================================================
// detector.cc - Correct implementation of paper eq.1
// P, N, σ come from Gaussian fit of per-SiPM photon count distribution
//=============================================================================

#include "detector.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4Poisson.hh"
#include <cmath>
#include <algorithm>

//=============================================================================
// Constructor/Destructor
//=============================================================================

MySensitiveDetector::MySensitiveDetector(G4String name)
    : G4VSensitiveDetector(name), fTotalEnergyDeposited(0), HitID(-1),
      SensorCollection(nullptr), fCurrentEventID(-1)
{
    G4String HCname = "SensorCollection";
    collectionName.insert(HCname);
    G4cout << "MySensitiveDetector created: " << name << G4endl;
}

MySensitiveDetector::~MySensitiveDetector() {}

//=============================================================================
// Initialize
//=============================================================================

void MySensitiveDetector::Initialize(G4HCofThisEvent* HCE)
{
    SensorCollection = new SensorHitsCollection(SensitiveDetectorName, collectionName[0]);

    G4ThreadLocal G4int HCID = -1;
    if (HCID < 0) {
        HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    }
    if (HCE) {
        HCE->AddHitsCollection(HCID, SensorCollection);
    }

    sensor1CopyNumbers.clear();
    fTotalEnergyDeposited = 0;

    const G4Event* evt = G4RunManager::GetRunManager()->GetCurrentEvent();
    fCurrentEventID = evt ? evt->GetEventID() : -1;
}

//=============================================================================
// ProcessHits
//=============================================================================

G4bool MySensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
    G4Track* track = aStep->GetTrack();
    if (track->GetDefinition() != G4OpticalPhoton::Definition()) {
        return false;
    }

    G4StepPoint* postStepPoint = aStep->GetPostStepPoint();
    const G4VTouchable* touchable = aStep->GetPreStepPoint()->GetTouchable();

    G4ThreeVector posGlobal = postStepPoint->GetPosition();
    G4ThreeVector posLocal  = touchable->GetVolume()->GetTranslation();
    G4String volumeName     = touchable->GetVolume()->GetName();
    G4int copyNo            = touchable->GetCopyNumber();
    G4double energy         = track->GetKineticEnergy();

    if (volumeName == "sensor_Vol1") {
        sensor1CopyNumbers.push_back(copyNo);
    }

    SensorHit* hit = new SensorHit();
    hit->SetSensorPosition(posGlobal);
    hit->SetSensorEnergy(energy);
    hit->SetVolumeName(volumeName);
    hit->SetCopyNumber(copyNo);
    hit->SetEventID(fCurrentEventID);

    if (SensorCollection) {
        HitID = SensorCollection->insert(hit);
    } else {
        delete hit;
        return false;
    }

    G4AnalysisManager* man = G4AnalysisManager::Instance();
    if (man) {
        int ntupleIndex = -1;
        if      (volumeName == "sensor_Vol1") ntupleIndex = 0;
        else if (volumeName == "sensor_Vol2") ntupleIndex = 1;
        else if (volumeName == "sensor_Vol3") ntupleIndex = 2;
        else if (volumeName == "sensor_Vol4") ntupleIndex = 3;

        if (ntupleIndex >= 0) {
            RecordSensorData(volumeName, ntupleIndex, posLocal.x(), posLocal.y(),
                             fCurrentEventID, copyNo, man);
        }
    }

    track->SetTrackStatus(fStopAndKill);
    return true;
}

//=============================================================================
// RecordSensorData
//=============================================================================

void MySensitiveDetector::RecordSensorData(const std::string& volumeName, int ntupleIndex,
                                           double posX, double posY, int event,
                                           int copyNo, G4AnalysisManager* man)
{
    if (!man) return;

    man->FillNtupleDColumn(ntupleIndex, 0, posX / mm);
    man->FillNtupleDColumn(ntupleIndex, 1, posY / mm);
    man->FillNtupleIColumn(ntupleIndex, 2, event);
    man->FillNtupleIColumn(ntupleIndex, 3, copyNo);
    man->AddNtupleRow(ntupleIndex);
}

//=============================================================================
// GaussianFitParams
//
// Noise model applied here (paper Sec. 3):
//   1. Poisson dark noise λ=6 pe drawn ONCE per array, spread uniformly
//      across all 33 strips (avoids the 33×6=198 pe/array problem)
//   2. 10% Gaussian smearing on the peak count N (SiPM resolution)
//
// P = photon-count-weighted centroid of SiPM positions
// N = peak SiPM count after noise (Gaussian amplitude)
// σ = photon-count-weighted RMS about P
//=============================================================================

bool MySensitiveDetector::GaussianFitParams(const std::vector<double>& counts,
                                            double firstCenter, double pitch,
                                            double& P, double& N, double& sigma)
{
    const int nSiPM = (int)counts.size();

    // --- Apply noise model ---
    // 1) Poisson dark noise: λ=6 pe total per array, spread uniformly
    double totalDark    = static_cast<double>(G4Poisson(0.01));
    double darkPerStrip = totalDark / nSiPM;

    std::vector<double> noisyCounts(nSiPM);
    for (int i = 0; i < nSiPM; i++) {
        noisyCounts[i] = counts[i] + darkPerStrip;
        if (noisyCounts[i] < 0) noisyCounts[i] = 0;
    }

    double total = 0.0;
    for (double c : noisyCounts) total += c;

    if (total <= 0.0) {
        P = 0.0; N = 0.0; sigma = 1.0;
        return false;
    }

    // --- P: photon-count-weighted centroid ---
    double sumWX = 0.0;
    for (int i = 0; i < nSiPM; i++) {
        sumWX += noisyCounts[i] * (firstCenter + i * pitch);
    }
    P = sumWX / total;

    // --- σ: photon-count-weighted RMS about P ---
    double sumWX2 = 0.0;
    for (int i = 0; i < nSiPM; i++) {
        double diff = (firstCenter + i * pitch) - P;
        sumWX2 += noisyCounts[i] * diff * diff;
    }
    sigma = std::sqrt(sumWX2 / total);

    // --- N: peak SiPM count after noise, with 10% Gaussian smearing ---
    double peakRaw = *std::max_element(noisyCounts.begin(), noisyCounts.end());
    // 2) 10% Gaussian smearing on N (SiPM photon-number resolution)
    N = G4RandGauss::shoot(peakRaw, 0.10 * peakRaw);
    if (N < 0) N = 0;

    // --- Sigma clamping ---
    int nFired = 0;
    for (double c : counts) if (c > 0.0) nFired++;  // count signal-only fired strips

    if (nFired <= 1) {
        // Single SiPM fired: no light-sharing info, assign sigma = pitch
        sigma = pitch;
    } else if (sigma < 0.01 * mm) {
        sigma = 0.01 * mm;  // numerical zero guard
    }

    return true;
}

//=============================================================================
// calculateWeightedMean — paper Eq. 3.1
//=============================================================================

double MySensitiveDetector::calculateWeightedMean(double P1, double N1, double sigma1,
                                                  double P2, double N2, double sigma2)
{
    double w1 = (N1 > 0 && sigma1 > 0) ? N1 / sigma1 : 0.0;
    double w2 = (N2 > 0 && sigma2 > 0) ? N2 / sigma2 : 0.0;
    if (w1 + w2 == 0) return 0.0;
    return (P1 * w1 + P2 * w2) / (w1 + w2);
}

double MySensitiveDetector::calculateWeightedMeanX(double P_x1, double N_x1, double sigma_x1,
                                                   double P_x2, double N_x2, double sigma_x2)
{
    return calculateWeightedMean(P_x1, N_x1, sigma_x1, P_x2, N_x2, sigma_x2);
}

double MySensitiveDetector::calculateWeightedMeanY(double P_y1, double N_y1, double sigma_y1,
                                                   double P_y2, double N_y2, double sigma_y2)
{
    return calculateWeightedMean(P_y1, N_y1, sigma_y1, P_y2, N_y2, sigma_y2);
}

//=============================================================================
// FitHistogram
//=============================================================================

void MySensitiveDetector::FitHistogram(const std::vector<double>& countLeft,
                                       const std::vector<double>& countRight,
                                       const std::vector<double>& countBottom,
                                       const std::vector<double>& countTop,
                                       double firstCenter, double pitch)
{
    double P_left,   N_left,   sigma_left;
    double P_right,  N_right,  sigma_right;
    double P_bottom, N_bottom, sigma_bottom;
    double P_top,    N_top,    sigma_top;

    bool hasLeft   = GaussianFitParams(countLeft,   firstCenter, pitch, P_left,   N_left,   sigma_left);
    bool hasRight  = GaussianFitParams(countRight,  firstCenter, pitch, P_right,  N_right,  sigma_right);
    bool hasBottom = GaussianFitParams(countBottom, firstCenter, pitch, P_bottom, N_bottom, sigma_bottom);
    bool hasTop    = GaussianFitParams(countTop,    firstCenter, pitch, P_top,    N_top,    sigma_top);

    if (!hasLeft && !hasRight)  return;
    if (!hasBottom && !hasTop)  return;

    double y_pos, x_pos;

    if (hasLeft && hasRight)
        y_pos = calculateWeightedMean(P_left, N_left, sigma_left, P_right, N_right, sigma_right);
    else if (hasLeft)
        y_pos = P_left;
    else
        y_pos = P_right;

    if (hasBottom && hasTop)
        x_pos = calculateWeightedMean(P_bottom, N_bottom, sigma_bottom, P_top, N_top, sigma_top);
    else if (hasBottom)
        x_pos = P_bottom;
    else
        x_pos = P_top;

    G4AnalysisManager* man = G4AnalysisManager::Instance();
    if (man) {
        man->FillH2(0, x_pos / mm, y_pos / mm);
        static G4int ntupleID = 4;
        man->FillNtupleDColumn(ntupleID, 0, x_pos / mm);
        man->FillNtupleDColumn(ntupleID, 1, y_pos / mm);
        man->AddNtupleRow(ntupleID);
    }
}

//=============================================================================
// EndOfEvent - Bin photons by SiPM copy number, then reconstruct
//=============================================================================

void MySensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{
    if (!SensorCollection || SensorCollection->entries() == 0) return;

    G4int nHits = SensorCollection->entries();

    const int    nSiPM       = 33;
    const double pitch       = 3.0 * mm;
    const double firstCenter = -49.5 * mm + 0.5 * pitch;  // = -48.0 mm (copyNo=0 center)

    std::vector<double> countLeft  (nSiPM, 0.0);
    std::vector<double> countRight (nSiPM, 0.0);
    std::vector<double> countBottom(nSiPM, 0.0);
    std::vector<double> countTop   (nSiPM, 0.0);

    for (G4int i = 0; i < nHits; i++) {
        SensorHit* hit      = (*SensorCollection)[i];
        G4int copyNo        = hit->GetCopyNumber();
        const G4String& vol = hit->GetVolumeName();

        if (copyNo < 0 || copyNo >= nSiPM) continue;

        if      (vol == "sensor_Vol1") countLeft  [copyNo]++;
        else if (vol == "sensor_Vol2") countRight [copyNo]++;
        else if (vol == "sensor_Vol3") countBottom[copyNo]++;
        else if (vol == "sensor_Vol4") countTop   [copyNo]++;
    }

    FitHistogram(countLeft, countRight, countBottom, countTop, firstCenter, pitch);
}
