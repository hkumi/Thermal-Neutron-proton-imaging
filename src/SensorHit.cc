#include "SensorHit.hh"

G4ThreadLocal G4Allocator<SensorHit>* SensorHitAllocator = 0;

SensorHit::SensorHit()
    : G4VHit(),
      positionSensor(G4ThreeVector(0., 0., 0.)),
      fEnergyDeposited(0.),
      fVolumeName(""),
      fCopyNumber(-1),
      fEventID(-1)
{ }

SensorHit::~SensorHit() { }

SensorHit::SensorHit(const SensorHit& right)
    : G4VHit(),
      positionSensor(right.positionSensor),
      fEnergyDeposited(right.fEnergyDeposited),
      fVolumeName(right.fVolumeName),
      fCopyNumber(right.fCopyNumber),
      fEventID(right.fEventID)
{ }

const SensorHit& SensorHit::operator=(const SensorHit& right)
{
    positionSensor = right.positionSensor;
    fEnergyDeposited = right.fEnergyDeposited;
    fVolumeName = right.fVolumeName;
    fCopyNumber = right.fCopyNumber;
    fEventID = right.fEventID;
    return *this;
}

G4bool SensorHit::operator==(const SensorHit& right) const
{
    return (positionSensor == right.positionSensor &&
            fEnergyDeposited == right.fEnergyDeposited &&
            fVolumeName == right.fVolumeName &&
            fCopyNumber == right.fCopyNumber &&
            fEventID == right.fEventID);
}

void SensorHit::Print()
{
    G4cout << "Sensor hit at position: " 
           << G4BestUnit(positionSensor, "Length")
           << " Energy: " << G4BestUnit(fEnergyDeposited, "Energy")
           << " Volume: " << fVolumeName
           << " Copy: " << fCopyNumber
           << " Event: " << fEventID
           << G4endl;
}
