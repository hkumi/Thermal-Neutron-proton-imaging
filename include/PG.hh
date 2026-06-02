#ifndef PG_h
#define PG_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4GeneralParticleSource.hh"

class G4ParticleGun;
class G4Event;

class PG : public G4VUserPrimaryGeneratorAction
{
  public:
    PG(double pos_x,double pos_y);
    ~PG();

  public:
    void GeneratePrimaries(G4Event* anEvent);

  private:
    G4GeneralParticleSource* particleGun;

  private:
    double posi_x;
    double posi_y;

};

#endif


