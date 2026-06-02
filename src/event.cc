#include "event.hh"

MyEventAction::MyEventAction(RunAction*)
{
    fEdep = 0.;
   

}

MyEventAction::~MyEventAction()
{}

void MyEventAction::BeginOfEventAction(const G4Event*)
{
    fEdep = 0.;
    

}

void MyEventAction::EndOfEventAction(const G4Event* evt)
{
     
     
}
