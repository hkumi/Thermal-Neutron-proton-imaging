#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "RunAction.hh"
#include "StepAction.hh"
#include "PG.hh"
#include "event.hh"

class ActionInitialization : public G4VUserActionInitialization
{
public:
    ActionInitialization(const G4String& dataFile, G4double collimatore, G4double posX, G4double posY);
    virtual ~ActionInitialization();

    virtual void BuildForMaster() const override;
    virtual void Build() const override;

private:
    G4String fDataFile;
    G4double fCollimatore;
    G4double fPosX;
    G4double fPosY;
};

#endif
