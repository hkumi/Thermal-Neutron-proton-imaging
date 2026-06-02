#include "ActionInitialization.hh"
#include "RunAction.hh"
#include "StepAction.hh"
#include "PG.hh"

ActionInitialization::ActionInitialization(const G4String& dataFile, G4double collimatore,
                                           G4double posX, G4double posY)
    : G4VUserActionInitialization(), fDataFile(dataFile),
      fCollimatore(collimatore), fPosX(posX), fPosY(posY)
{}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const
{
    SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
    
    RunAction* runAction = new RunAction();
    SetUserAction(runAction);

    SetUserAction(new PG(fPosX, fPosY));

    MyEventAction* eventAction = new MyEventAction(runAction);
    SetUserAction(eventAction);

    SetUserAction(new StepAction(eventAction));
}
