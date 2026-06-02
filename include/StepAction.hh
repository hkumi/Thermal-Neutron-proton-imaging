#ifndef StepAction_h
#define StepAction_h 1
#include "G4UserSteppingAction.hh"
#include "globals.hh"
#include <vector>
#include "DC.hh"
#include "G4RunManager.hh"
#include "G4AnalysisManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4Proton.hh"
#include "event.hh"

class StepAction : public G4UserSteppingAction
{
public:
  StepAction(MyEventAction* eventAction);
  ~StepAction();
  virtual void UserSteppingAction(const G4Step*);
  double hysto(double valore);
private:
  G4String datai_file;
  int s_x1[33];
  int s_x2[33];
  int s_y1[33];
  int s_y2[33];
  double collimatore;
  MyEventAction* fEventAction;
};
#endif
