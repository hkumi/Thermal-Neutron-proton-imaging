#include "G4Step.hh"
#include "G4Track.hh"
#include "G4RunManager.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4OpticalPhoton.hh"
#include "G4Proton.hh"
#include "StepAction.hh" 

StepAction::StepAction(MyEventAction* eventAction) 
: G4UserSteppingAction(), fEventAction(eventAction) 
{}

StepAction::~StepAction() 
{}

void StepAction::UserSteppingAction(const G4Step* step)
{
    G4Track* track       = step->GetTrack();
    G4AnalysisManager* man = G4AnalysisManager::Instance();

    const G4Event* evt   = G4RunManager::GetRunManager()->GetCurrentEvent();
    G4int eventID        = evt ? evt->GetEventID() : -1;

    G4StepPoint* prePoint  = step->GetPreStepPoint();
    G4StepPoint* postPoint = step->GetPostStepPoint();

    if (!prePoint->GetPhysicalVolume() || !postPoint->GetPhysicalVolume()) return;

    G4String preVolName  = prePoint->GetPhysicalVolume()->GetName();
    G4String postVolName = postPoint->GetPhysicalVolume()->GetName();

    //===========================================================================
    // PROTONS
    //===========================================================================
    if (track->GetDefinition() == G4Proton::Definition())
    {
        G4ThreeVector pos  = prePoint->GetPosition();
        G4double ekin      = prePoint->GetKineticEnergy();

        // Proton entering the gas (PPAC) — first step inside PPAC_Vol
        if (postVolName == "PPAC_Vol" && preVolName != "PPAC_Vol")
        {
            
            man->FillNtupleDColumn(8, 0, ekin   / MeV);
            
            man->AddNtupleRow(8);
        }

        // Proton entering the converter/shield
        if (postVolName == "Shield" && preVolName != "Shield")
        {
           
            man->FillNtupleDColumn(7, 0, ekin   / MeV);
           
            man->AddNtupleRow(7);
        }

        // Proton entering the cathode
        if (postVolName == "cathode_Vol" && preVolName != "cathode_Vol")
        {
           
            man->FillNtupleDColumn(6, 0, ekin   / MeV);
         
            man->AddNtupleRow(6);
        }

        // Proton entering the anode
        if (postVolName == "anode_Vol" && preVolName != "anode_Vol")
        {
            
            man->FillNtupleDColumn(5, 0, ekin   / MeV);
           
            man->AddNtupleRow(5);
        }
    }

    //===========================================================================
    // OPTICAL PHOTONS in SiPMs — energy spectrum
    //===========================================================================
    if (track->GetDefinition() == G4OpticalPhoton::Definition())
    {
        G4String volName = prePoint->GetPhysicalVolume()->GetName();

        if (volName == "sensor_Vol1" || volName == "sensor_Vol2" ||
            volName == "sensor_Vol3" || volName == "sensor_Vol4")
        {
            G4double photonE = track->GetKineticEnergy();
            man->FillH1(0, photonE / eV);
        }
    }
}

double StepAction::hysto(double valore)
{
    
    return valore;
}
