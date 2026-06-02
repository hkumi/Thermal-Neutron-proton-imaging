#include "RunAction.hh"
#include "Run.hh"
#include "G4Run.hh"
#include "G4SDManager.hh"
#include "G4UnitsTable.hh"
#include <assert.h>

RunAction::RunAction()
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->SetNtupleMerging(true);
    man->SetVerboseLevel(1);

    //===========================================================================
    // NTUPLES for individual sensor hits (IDs 0-3)
    //===========================================================================
    man->CreateNtuple("LeftData1", "LeftData1");
    man->CreateNtupleDColumn("x");
    man->CreateNtupleDColumn("y");
    man->CreateNtupleIColumn("event");
    man->CreateNtupleIColumn("copyNo");
    man->FinishNtuple(0);

    man->CreateNtuple("RightData2", "RightData2");
    man->CreateNtupleDColumn("x");
    man->CreateNtupleDColumn("y");
    man->CreateNtupleIColumn("event");
    man->CreateNtupleIColumn("copyNo");
    man->FinishNtuple(1);

    man->CreateNtuple("BottomData3", "BottomData3");
    man->CreateNtupleDColumn("x");
    man->CreateNtupleDColumn("y");
    man->CreateNtupleIColumn("event");
    man->CreateNtupleIColumn("copyNo");
    man->FinishNtuple(2);

    man->CreateNtuple("TopData4", "TopData4");
    man->CreateNtupleDColumn("x");
    man->CreateNtupleDColumn("y");
    man->CreateNtupleIColumn("event");
    man->CreateNtupleIColumn("copyNo");
    man->FinishNtuple(3);

    //===========================================================================
    // NTUPLE for reconstruction results (ID 4)
    //===========================================================================
    man->CreateNtuple("Reconstruction", "Reconstruction");
    man->CreateNtupleDColumn("x_rec");
    man->CreateNtupleDColumn("y_rec");
    man->FinishNtuple(4);

    //===========================================================================
    // NTUPLE for protons at anode (ID 5)
    // Columns: x, y, z position [mm], kinetic energy [MeV], eventID
    //===========================================================================
    man->CreateNtuple("ProtonsAnode", "Protons reaching anode");
    man->CreateNtupleDColumn("ekin");
    man->FinishNtuple(5);

    //===========================================================================
    // NTUPLE for protons at cathode (ID 6)
    //===========================================================================
    man->CreateNtuple("ProtonsCathode", "Protons reaching cathode");
    man->CreateNtupleDColumn("ekin");
    man->FinishNtuple(6);

    //===========================================================================
    // NTUPLE for protons at converter/shield (ID 7)
    //===========================================================================
    man->CreateNtuple("ProtonsConverter", "Protons reaching converter");
    man->CreateNtupleDColumn("ekin");
    man->FinishNtuple(7);

    //===========================================================================
    // NTUPLE for protons entering gas/PPAC (ID 8)
    //===========================================================================
    man->CreateNtuple("ProtonsGas", "Protons entering gas");
    man->CreateNtupleDColumn("ekin");
    man->FinishNtuple(8);

    //===========================================================================
    // HISTOGRAMS
    //===========================================================================
    // H2 ID 0: 2D reconstructed positions
    man->CreateH2("h0_xy_rec", "Reconstructed Positions",
                  100, -30, 30, 100, -30, 30);

    // H1 ID 0: photon energy spectrum in SiPMs [eV]
    man->CreateH1("h1_photon_energy", "Photon energy spectrum in SiPMs",
                  200, 1.0, 7.0, "eV");

    G4cout << "All histograms and ntuples created successfully!" << G4endl;
}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run* aRun)
{
    G4cout << "*** Run " << aRun->GetRunID() << " start." << G4endl;
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    G4int runID = aRun->GetRunID();
    std::stringstream strRunID;
    strRunID << runID;
    G4String filename = "output" + strRunID.str() + ".root";
    man->OpenFile(filename);
    G4cout << "Output file opened: " << filename << G4endl;
}

void RunAction::EndOfRunAction(const G4Run* aRun)
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();
    G4cout << "Output file closed. Run " << aRun->GetRunID() << " finished." << G4endl;
}
