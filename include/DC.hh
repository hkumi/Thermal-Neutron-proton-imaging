#ifndef __DC_H__
#define __DC_H__

#include "G4Material.hh"
#include "G4Box.hh"
#include "G4VSolid.hh"
#include "G4SubtractionSolid.hh"
#include "detector.hh"
#include "G4Sphere.hh"
#include "G4OpticalSurface.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;

#include "G4VUserDetectorConstruction.hh"

class DC : public G4VUserDetectorConstruction
{
  public:
    DC(double density, double lunghezza_collimatore);
    virtual ~DC();

  public:
    virtual G4VPhysicalVolume* Construct();
    G4LogicalVolume *GetScoringVolume() const {return fScoringVolume_1;}
    void Sphereball( G4double position);
    void ConstructValve(G4double position);
    void ConstructResolutionPhantom();
    void ConstructBoxPhantom(G4double position);

  private:
    // Logical volumes
    G4LogicalVolume* experimentalHall_log;
    G4LogicalVolume* GAS_log;
    G4LogicalVolume* PPAC_log;
    G4LogicalVolume* SiPM_log;
    G4LogicalVolume* collf_log;
    G4LogicalVolume* coll_log;
    G4LogicalVolume* cathode_log;
    G4LogicalVolume* anode_log;
    G4LogicalVolume* cathodeAl_log;
    G4LogicalVolume* anodeAl_log;
    // Physical volumes
    G4VPhysicalVolume* experimentalHall_phys;
    G4VPhysicalVolume* GAS_phys;
    G4VPhysicalVolume* PPAC_phys;
    G4VPhysicalVolume* SiPM_phys;
    G4VPhysicalVolume* collf_phys;
    G4VPhysicalVolume* coll_phys;
    G4VPhysicalVolume* cathode_phys;
    G4VPhysicalVolume* anode_phys;
    G4VPhysicalVolume* cathodeAl_phys;
    G4VPhysicalVolume* anodeAl_phys;

   //Scorer.
    G4LogicalVolume   *fScoringVolume_1;

 private:
    void DefineMaterials();
    void ConstructLaboratory();
    double dens;
    double collimatore;
    

 private:
   G4Box  *HDPE_Box1,*Lead_Box,*HDPE_Box2,*HDPE_Box3,*HDPE_Box4,*HDPE_Box5,*HDPE_Box6,*HDPE_Box7,*HDPE_Box8,*HDPE_Box9,*HDPE_Box10,*Lead_Box4,*Lead_Box5,*Graphite_Box;
     G4Box  *HDPE_Box11,*HDPE_Box12,*HDPE_Box13,*HDPE_Box14,*HDPE_Box15,*HDPE_Box16,*HDPE_Box17,*Borated_Box1,*solidScintillator,*Lead_Box1,*Lead_Box2,*Lead_Box3,*Graphite_Box2,*Graphite_Box3;
     G4Box  *HDPE_Box18,*HDPE_Box19,*HDPE_Box20,*HDPE_Box21,*HDPE_Box22,*HDPE_Box23,*HDPE_Box24,*HDPE_Box25;
     G4VPhysicalVolume *Lead_PV,*HDPE_PV1,*HDPE_PV2, *HDPE_PV3,*HDPE_PV4,*HDPE_PV5,*HDPE_PV6,*HDPE_PV7,*HDPE_PV8,*HDPE_PV9,*HDPE_PV10,*Lead_PV4,*Lead_PV5,*Graphite_PV,*Graphite_PV2,*Graphite_PV3;
     G4VPhysicalVolume *HDPE_PV11,*HDPE_PV12,*HDPE_PV13,*HDPE_PV14,*HDPE_PV15,*HDPE_PV16,*HDPE_PV17,*Borated_PV1,*physScintillator,*Lead_PV1,*Lead_PV2,*Lead_PV3,*Hole_PV3,*sensor_phys,*sensor_phys2;
     G4VPhysicalVolume *HDPE_PV18,*HDPE_PV19,*HDPE_PV20,*HDPE_PV21,*HDPE_PV22,*HDPE_PV23,*HDPE_PV24,*HDPE_PV25;
     G4LogicalVolume   *Lead_LV,*HDPE_LV1,*HDPE_LV2,*HDPE_LV3,*HDPE_LV4,*HDPE_LV5,*HDPE_LV6,*HDPE_LV7,*HDPE_LV8,*HDPE_LV9,*HDPE_LV10,*Lead_LV4,*Lead_LV5,*Graphite_LV,*Graphite_LV2,*Graphite_LV3;
     G4LogicalVolume   *HDPE_LV11,*HDPE_LV12,*HDPE_LV13,*HDPE_LV14,*HDPE_LV15,*HDPE_LV16,*HDPE_LV17,*Borated_LV1,*logicScintillator,*Lead_LV1,*Lead_LV2,*Lead_LV3;
     G4LogicalVolume   *HDPE_LV18,*HDPE_LV19,*HDPE_LV20,*HDPE_LV21,*HDPE_LV22,*HDPE_LV23,*HDPE_LV24,*HDPE_LV25;
     G4LogicalVolume *Hole_LV,*Hole_LV3,*sensor_log1,*sensor_log2,*sensor_log3,*sensor_log4;
     G4SubtractionSolid    *collimator;
     G4VSolid *Hole, *Hole3;
     G4Material* boron_converter;
     G4VPhysicalVolume *Hole_PV,*Hole_PV2;

     G4double           fblockSize,fLeadSize,BoratedSize,Borated_thickness,LeadSize,fGraphiteSize;

    G4double LayerThickness;
    G4int NbOfLayers;
    G4Material  *b_polyethylene,  *polyethylene, *NaI,*mat_graphite;
    G4Material  *leadMaterial,*Aluminium,*PP,*silicon;
    G4Element  *Na, *I, *C,*Al;
    G4MaterialPropertiesTable* reflectMPT;
    G4MaterialPropertiesTable* absorbMPT; 
    G4OpticalSurface* cf4SiSurface;    
    virtual void ConstructSDandField();  

};

#endif
