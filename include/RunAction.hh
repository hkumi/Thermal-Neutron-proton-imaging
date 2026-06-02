#ifndef __RunAction_H__
#define __RunAction_H__

#include "G4UserRunAction.hh"
#include "globals.hh"
#include <vector>
#include "G4AnalysisManager.hh"

class G4Run;

class RunAction : public G4UserRunAction
{
public:
  // constructor and destructor
 RunAction();
  virtual ~RunAction();

public:
  // virtual method from G4UserRunAction.
//  virtual G4Run* GenerateRun();
  virtual void BeginOfRunAction(const G4Run*);
  virtual void EndOfRunAction(const G4Run*);

private:
  // Data member 
  // - vector of MultiFunctionalDetecor names.
  std::vector<G4String> theSDName;  

};

#endif
