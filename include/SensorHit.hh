#ifndef SENSORHIT_HH
#define SENSORHIT_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4Threading.hh"
#include "G4UnitsTable.hh"
#include <iomanip>

class SensorHit : public G4VHit
{
public:
    SensorHit();
    SensorHit(const SensorHit&);
    virtual ~SensorHit();

    // Operators
    const SensorHit& operator=(const SensorHit&);
    G4bool operator==(const SensorHit&) const;

    inline void* operator new(size_t);
    inline void  operator delete(void*);

    // Set methods
    void SetSensorPosition(G4ThreeVector pos) { positionSensor = pos; }
    void SetSensorEnergy(G4double energy) { fEnergyDeposited = energy; }
    void SetVolumeName(G4String name) { fVolumeName = name; }
    void SetCopyNumber(G4int copy) { fCopyNumber = copy; }
    void SetEventID(G4int id) { fEventID = id; }

    // Get methods
    G4ThreeVector GetSensorPosition() const { return positionSensor; }
    G4double GetSensorEnergy() const { return fEnergyDeposited; }
    G4String GetVolumeName() const { return fVolumeName; }
    G4int GetCopyNumber() const { return fCopyNumber; }
    G4int GetEventID() const { return fEventID; }

    // Print method
    virtual void Print();

private:
    G4ThreeVector positionSensor;
    G4double fEnergyDeposited;
    G4String fVolumeName;
    G4int fCopyNumber;
    G4int fEventID;
};

typedef G4THitsCollection<SensorHit> SensorHitsCollection;

extern G4ThreadLocal G4Allocator<SensorHit>* SensorHitAllocator;

inline void* SensorHit::operator new(size_t)
{
    if (!SensorHitAllocator) {
        SensorHitAllocator = new G4Allocator<SensorHit>;
    }
    return (void*)SensorHitAllocator->MallocSingle();
}

inline void SensorHit::operator delete(void* aHit)
{
    if (SensorHitAllocator) {
        SensorHitAllocator->FreeSingle((SensorHit*)aHit);
    }
}

#endif
