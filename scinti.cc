// --------------------------------------------------------------
//      GEANT 4 - expe.cc
// --------------------------------------------------------------

//*************************************************************
//-------- GEANT4
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "FTFP_BERT_HP.hh"
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "G4OpticalPhysics.hh"
#include "G4VisManager.hh"

//*************************************************************

#include "G4RunManagerFactory.hh"
#include "G4String.hh"
#include "G4Types.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

//-------- User  <-- your classes
#include "DC.hh" // <----- Geometry
#include "PG.hh"
#include "RunAction.hh"     // <----- Run Action
#include "StepAction.hh"     // <----- Step Action
#include "ActionInitialization.hh"


int main(int argc, char** argv)
{
    

    // Detect interactive mode (if no arguments or only a macro file)
    G4UIExecutive* ui = nullptr;
    int number_of_events = 1;
    G4double collimatore = 20.0, density = 0.171316, pos_x = 0.0, pos_y = 0.0;
    G4String data_file;

    if (argc == 1)  
        ui = new G4UIExecutive(argc, argv);
    

    // Set up the run manager
    auto runManager = G4RunManagerFactory::CreateRunManager();
    runManager->SetVerboseLevel(0);

    // Set up user-defined classes
    auto detector = new DC(density, collimatore);
    runManager->SetUserInitialization(detector);

    G4VModularPhysicsList* physicsList = new FTFP_BERT_HP;
G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
physicsList->RegisterPhysics(opticalPhysics);
runManager->SetUserInitialization(physicsList);

    runManager->SetUserInitialization(new ActionInitialization(data_file, collimatore, pos_x, pos_y));

    // Visualization manager
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // Get the User Interface manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    if (ui) {
        // Interactive mode
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete ui;
    } 
    else {
        // Batch mode
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }

    // Cleanup
    delete visManager;
    delete runManager;

    return 0;
}


