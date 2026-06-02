#ifndef DETECTOR_HH
#define DETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4Step.hh"
#include "SensorHit.hh"
#include "G4VHitsCollection.hh"

#include <vector>
#include <string>

class G4HCofThisEvent;

class MySensitiveDetector : public G4VSensitiveDetector
{
public:
    MySensitiveDetector(G4String);
    ~MySensitiveDetector();

    // N/σ weighted mean (paper Eq. 3.1)
    double calculateWeightedMean (double P1, double N1, double sigma1,
                                  double P2, double N2, double sigma2);
    double calculateWeightedMeanX(double P_x1, double N_x1, double sigma_x1,
                                  double P_x2, double N_x2, double sigma_x2);
    double calculateWeightedMeanY(double P_y1, double N_y1, double sigma_y1,
                                  double P_y2, double N_y2, double sigma_y2);

    // Extract P, N, σ from per-SiPM count distribution (with noise model)
    bool GaussianFitParams(const std::vector<double>& counts,
                           double firstCenter, double pitch,
                           double& P, double& N, double& sigma);

    void RecordSensorData(const std::string& volumeName, int ntupleIndex,
                          double posX, double posY, int event,
                          int copyNo, G4AnalysisManager* man);

    void FitHistogram(const std::vector<double>& cntLeft,
                      const std::vector<double>& cntRight,
                      const std::vector<double>& cntBottom,
                      const std::vector<double>& cntTop,
                      double firstCenter, double pitch);

    std::vector<int>& GetSensor1CopyNumbers() { return sensor1CopyNumbers; }

    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
    void   Initialize(G4HCofThisEvent*)             override;
    void   EndOfEvent(G4HCofThisEvent*)             override;

private:
    std::vector<int>      sensor1CopyNumbers;
    G4double              fTotalEnergyDeposited;
    G4int                 HitID;
    G4int                 fCurrentEventID;
    SensorHitsCollection* SensorCollection;
};

#endif
