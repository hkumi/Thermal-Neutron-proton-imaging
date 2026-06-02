//-------- GEANT4
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "globals.hh"
#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4IonTable.hh"
//-------- User
#include "PG.hh"
//
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

PG::PG(double pos_x,double pos_y)
{
  //G4int n_particle = 1;
  particleGun = new G4GeneralParticleSource();

  //particleGun = new G4ParticleGun(n_particle);
  posi_x = pos_x;
  posi_y = pos_y;
}

PG::~PG()
{
  delete particleGun;
}

void PG::GeneratePrimaries(G4Event* anEvent)
{
  // Get the particle from the particle table (in this case, an alpha particle)
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* particle = particleTable->FindParticle("neutron");

    // Set the particle type and other properties
    particleGun->GetCurrentSource()->SetParticleDefinition(particle);
    particleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(0.025*eV);  // Set energy to 2.5 MeV

    // Set position and direction
    particleGun->GetCurrentSource()->GetPosDist()->SetCentreCoords(G4ThreeVector(posi_x * mm, posi_y * mm, -19 * mm));
    particleGun->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));  // Set direction along z-axis

    // Generate the primary vertex in the event
    particleGun->GeneratePrimaryVertex(anEvent);
}


