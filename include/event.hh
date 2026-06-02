#ifndef EVENT_HH
#define EVENT_HH

#include "G4UserEventAction.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"
#include "RunAction.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"                                                                                            
#include "detector.hh"
#include "G4SDManager.hh"

class MyEventAction : public G4UserEventAction
{
public:
     MyEventAction(RunAction*);    

    ~MyEventAction();
    
    
    virtual void BeginOfEventAction(const G4Event*);
    virtual void EndOfEventAction(const G4Event*);
    
    void AddEdep(G4double edep) { fEdep += edep; } 
    void AddEnergy(G4double energy) { fEnergy += energy; };
    void SetPosition(G4ThreeVector pos) { fPosition = pos; };
        
private:
    G4double fEdep;
    G4double fEnergy;
    G4ThreeVector fPosition;
};

#endif
