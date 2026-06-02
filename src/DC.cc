//-------- GEANT4
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4PVReplica.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4SDParticleFilter.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPVParameterisation.hh"
#include "G4SDManager.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "CLHEP/Units/SystemOfUnits.h"
#include "G4TessellatedSolid.hh"
#include "G4TriangularFacet.hh"
#include "G4Isotope.hh"
#include "G4Element.hh"
#include "G4UnitsTable.hh"

#include "DC.hh"

DC::DC(double density, double lunghezza_collimatore)
 :  experimentalHall_log(0),GAS_log(0),PPAC_log(0),SiPM_log(0),collf_log(0),coll_log(0),
    cathode_log(0),anode_log(0),cathodeAl_log(0),anodeAl_log(0),
    experimentalHall_phys(0),GAS_phys(0),PPAC_phys(0),SiPM_phys(0),collf_phys(0),coll_phys(0),
    cathode_phys(0),anode_phys(0),cathodeAl_phys(0),anodeAl_phys(0),
    reflectMPT(nullptr), absorbMPT(nullptr), cf4SiSurface(nullptr)
{
  dens = density;
  collimatore = lunghezza_collimatore;
}

DC::~DC()
{
    if (reflectMPT)   delete reflectMPT;
    if (absorbMPT)    delete absorbMPT;
    if (cf4SiSurface) delete cf4SiSurface;
}

G4VPhysicalVolume* DC::Construct()
{
  DefineMaterials();
  ConstructLaboratory();
  return experimentalHall_phys;
}

//===========================================================================
// DefineMaterials
//===========================================================================
void DC::DefineMaterials()
{
  G4NistManager* pNistManager = G4NistManager::Instance();
  G4double a, z, density;
  G4int ncomponents, natoms;

  // Elements
  G4Element *C  = new G4Element("Carbon",   "C",  6.,  12.011*g/mole);
  G4Element *N  = new G4Element("Nitrogen", "N",  7.,  14.007*g/mole);
  G4Element *O  = new G4Element("Oxygen",   "O",  8.,  15.999*g/mole);
  G4Element *F  = new G4Element("Fluorine", "F",  9.,  18.998*g/mole);
  G4Element *Au = new G4Element("Gold",     "Au", 79., 196.9665*g/mole);

  //===========================================================================
  // Natural boron (for borated polyethylene)
  //===========================================================================
  G4Isotope* B10_nat = new G4Isotope("B10_nat", 5, 10);
  G4Isotope* B11_nat = new G4Isotope("B11_nat", 5, 11);
  G4Element* B_nat   = new G4Element("Boron_nat", "Bnat", ncomponents=2);
  B_nat->AddIsotope(B10_nat, 19.9*perCent);
  B_nat->AddIsotope(B11_nat, 80.1*perCent);

  //===========================================================================
  // Enriched boron — 96% B-10  ← THE CONVERTER
  //
  // B-10(n,α)Li-7 cross-section at 0.025 eV = 3840 barns
  // Alpha energy:  1.47 MeV  (ground state branch, 94%)
  // Li-7 energy:   0.84 MeV
  // Both escape into CF4 and produce scintillation light.
  // Optimal layer thickness: 1 µm
  //   - too thin  → few captures
  //   - too thick → alphas stop inside boron before reaching CF4
  //===========================================================================
  G4Isotope* B10_enr = new G4Isotope("B10_enr", 5, 10);
  G4Isotope* B11_enr = new G4Isotope("B11_enr", 5, 11);
  G4Element* B_enr   = new G4Element("Boron_enr", "Benr", ncomponents=2);
  B_enr->AddIsotope(B10_enr, 96.0*perCent);
  B_enr->AddIsotope(B11_enr,  4.0*perCent);

  boron_converter = new G4Material("BoronConverter", 2.46*g/cm3,
                                    ncomponents=1, kStateSolid, 293*kelvin, 1*atmosphere);
  boron_converter->AddElement(B_enr, natoms=1);

  // Vacuum
  G4Material *Vacuum = new G4Material("Vacuum", 1.e-20*g/cm3, 2, kStateGas);
  Vacuum->AddElement(N, 0.755);
  Vacuum->AddElement(O, 0.245);

  // Lead
  leadMaterial = new G4Material("Lead", 82, 207.2*g/mole, 11.35*g/cm3);

  // Aluminium
  Aluminium = new G4Material("Al", z=13., a=26.98*g/mole, density=2.7*g/cm3);

  // Polyethylene (H element with thermal scattering name for HP physics)
  G4Element* Hpe = new G4Element("TS_H_of_Polyethylene", "H", 1, 1.0079*g/mole);
  G4Element* Cpe = new G4Element("Carbon", "C", 6, 12.01*g/mole);
  polyethylene = new G4Material("polyethylene", 0.93*g/cm3,
                                 ncomponents=2, kStateSolid, 293*kelvin, 1*atmosphere);
  polyethylene->AddElement(Hpe, natoms=4);
  polyethylene->AddElement(Cpe, natoms=2);

  // Borated polyethylene
  b_polyethylene = new G4Material("b_polyethylene", 0.94*g/cm3,
                                   ncomponents=4, kStateSolid, 293*kelvin, 1*atmosphere);
  b_polyethylene->AddElement(Hpe,   11.6*perCent);
  b_polyethylene->AddElement(Cpe,   61.2*perCent);
  b_polyethylene->AddElement(B_nat,  5.0*perCent);
  b_polyethylene->AddElement(O,     22.2*perCent);

  // Silicon
  silicon = pNistManager->FindOrBuildMaterial("G4_Si");

  // CF4 gas
  G4Material* CF4 = new G4Material("CF4", dens*mg/cm3, 2, kStateGas);
  CF4->AddElement(C, 1);
  CF4->AddElement(F, 4);

  // CF4 scintillation spectrum (300 entries)
  const G4int iNbEntries = 300;
  std::vector<G4double> CF4PhotonMomentum = {
    6.2*eV,6.138613861*eV,6.078431373*eV,6.019417476*eV,5.961538462*eV,5.904761905*eV,5.849056604*eV,5.794392523*eV,5.740740741*eV,5.688073394*eV,5.636363636*eV,5.585585586*eV,
    5.535714286*eV,5.486725664*eV,5.438596491*eV,5.391304348*eV,5.344827586*eV,5.299145299*eV,5.254237288*eV,5.210084034*eV,5.166666667*eV,5.123966942*eV,5.081967213*eV,5.040650407*eV,
    5*eV,4.96*eV,4.920634921*eV,4.881889764*eV,4.84375*eV,4.80620155*eV,4.769230769*eV,4.732824427*eV,4.696969697*eV,4.661654135*eV,4.626865672*eV,4.592592593*eV,4.558823529*eV,
    4.525547445*eV,4.492753623*eV,4.460431655*eV,4.428571429*eV,4.397163121*eV,4.366197183*eV,4.335664336*eV,4.305555556*eV,4.275862069*eV,4.246575342*eV,4.217687075*eV,4.189189189*eV,
    4.161073826*eV,4.133333333*eV,4.105960265*eV,4.078947368*eV,4.052287582*eV,4.025974026*eV,4*eV,3.974358974*eV,3.949044586*eV,3.924050633*eV,3.899371069*eV,3.875*eV,3.850931677*eV,
    3.827160494*eV,3.803680982*eV,3.780487805*eV,3.757575758*eV,3.734939759*eV,3.71257485*eV,3.69047619*eV,3.668639053*eV,3.647058824*eV,3.625730994*eV,3.604651163*eV,3.583815029*eV,
    3.563218391*eV,3.542857143*eV,3.522727273*eV,3.502824859*eV,3.483146067*eV,3.463687151*eV,3.444444444*eV,3.425414365*eV,3.406593407*eV,3.387978142*eV,3.369565217*eV,3.351351351*eV,
    3.333333333*eV,3.315508021*eV,3.29787234*eV,3.28042328*eV,3.263157895*eV,3.246073298*eV,3.229166667*eV,3.212435233*eV,3.195876289*eV,3.179487179*eV,3.163265306*eV,3.147208122*eV,
    3.131313131*eV,3.115577889*eV,3.1*eV,3.084577114*eV,3.069306931*eV,3.054187192*eV,3.039215686*eV,3.024390244*eV,3.009708738*eV,2.995169082*eV,2.980769231*eV,2.966507177*eV,
    2.952380952*eV,2.938388626*eV,2.924528302*eV,2.910798122*eV,2.897196262*eV,2.88372093*eV,2.87037037*eV,2.857142857*eV,2.844036697*eV,2.831050228*eV,2.818181818*eV,2.805429864*eV,
    2.792792793*eV,2.780269058*eV,2.767857143*eV,2.755555556*eV,2.743362832*eV,2.731277533*eV,2.719298246*eV,2.707423581*eV,2.695652174*eV,2.683982684*eV,2.672413793*eV,2.660944206*eV,
    2.64957265*eV,2.638297872*eV,2.627118644*eV,2.616033755*eV,2.605042017*eV,2.594142259*eV,2.583333333*eV,2.572614108*eV,2.561983471*eV,2.551440329*eV,2.540983607*eV,2.530612245*eV,
    2.520325203*eV,2.510121457*eV,2.5*eV,2.489959839*eV,2.48*eV,2.470119522*eV,2.46031746*eV,2.450592885*eV,2.440944882*eV,2.431372549*eV,2.421875*eV,2.412451362*eV,2.403100775*eV,
    2.393822394*eV,2.384615385*eV,2.375478927*eV,2.366412214*eV,2.357414449*eV,2.348484848*eV,2.339622642*eV,2.330827068*eV,2.322097378*eV,2.313432836*eV,2.304832714*eV,2.296296296*eV,
    2.287822878*eV,2.279411765*eV,2.271062271*eV,2.262773723*eV,2.254545455*eV,2.246376812*eV,2.238267148*eV,2.230215827*eV,2.222222222*eV,2.214285714*eV,2.206405694*eV,2.19858156*eV,
    2.190812721*eV,2.183098592*eV,2.175438596*eV,2.167832168*eV,2.160278746*eV,2.152777778*eV,2.14532872*eV,2.137931034*eV,2.130584192*eV,2.123287671*eV,2.116040956*eV,2.108843537*eV,
    2.101694915*eV,2.094594595*eV,2.087542088*eV,2.080536913*eV,2.073578595*eV,2.066666667*eV,2.059800664*eV,2.052980132*eV,2.04620462*eV,2.039473684*eV,2.032786885*eV,2.026143791*eV,
    2.019543974*eV,2.012987013*eV,2.006472492*eV,2*eV,1.993569132*eV,1.987179487*eV,1.980830671*eV,1.974522293*eV,1.968253968*eV,1.962025316*eV,1.955835962*eV,1.949685535*eV,
    1.943573668*eV,1.9375*eV,1.931464174*eV,1.925465839*eV,1.919504644*eV,1.913580247*eV,1.907692308*eV,1.901840491*eV,1.896024465*eV,1.890243902*eV,1.88449848*eV,1.878787879*eV,
    1.873111782*eV,1.86746988*eV,1.861861862*eV,1.856287425*eV,1.850746269*eV,1.845238095*eV,1.839762611*eV,1.834319527*eV,1.828908555*eV,1.823529412*eV,1.818181818*eV,1.812865497*eV,
    1.807580175*eV,1.802325581*eV,1.797101449*eV,1.791907514*eV,1.786743516*eV,1.781609195*eV,1.776504298*eV,1.771428571*eV,1.766381766*eV,1.761363636*eV,1.756373938*eV,1.751412429*eV,
    1.746478873*eV,1.741573034*eV,1.736694678*eV,1.731843575*eV,1.727019499*eV,1.722222222*eV,1.717451524*eV,1.712707182*eV,1.707988981*eV,1.703296703*eV,1.698630137*eV,1.693989071*eV,
    1.689373297*eV,1.684782609*eV,1.680216802*eV,1.675675676*eV,1.67115903*eV,1.666666667*eV,1.662198391*eV,1.657754011*eV,1.653333333*eV,1.64893617*eV,1.644562334*eV,1.64021164*eV,
    1.635883905*eV,1.631578947*eV,1.627296588*eV,1.623036649*eV,1.618798956*eV,1.614583333*eV,1.61038961*eV,1.606217617*eV,1.602067183*eV,1.597938144*eV,1.593830334*eV,1.58974359*eV,
    1.585677749*eV,1.581632653*eV,1.577608142*eV,1.573604061*eV,1.569620253*eV,1.565656566*eV,1.561712846*eV,1.557788945*eV,1.553884712*eV};
  std::sort(CF4PhotonMomentum.begin(), CF4PhotonMomentum.end());

  std::vector<G4double> CF4Scintillation_Fast = {
    0.0029,0.0029,0.0017,0.0024,0.0018,0.0011,0.0027,0.0009,0.0003,0.0019,0.0030,0.0024,0.0023,0.0036,0.0039,0.0056,
    0.0049,0.0061,0.0053,0.0052,0.0056,0.0064,0.0072,0.0064,0.0080,0.0071,0.0056,0.0069,0.0053,0.0070,0.0060,0.0057,0.0071,0.0066,0.0066,
    0.0055,0.0082,0.0076,0.0093,0.0089,0.0106,0.0109,0.0105,0.0102,0.0120,0.0121,0.0102,0.0097,0.0120,0.0126,0.0097,0.0103,0.0097,0.0084,
    0.0119,0.0112,0.0096,0.0171,0.0235,0.0078,0.0089,0.0071,0.0065,0.0074,0.0073,0.0074,0.0074,0.0080,0.0143,0.0522,0.0069,0.0076,0.0042,
    0.0059,0.0039,0.0053,0.0054,0.0185,0.0077,0.0599,0.0048,0.0034,0.0041,0.0041,0.0047,0.0059,0.0046,0.0065,0.0128,0.0037,0.0167,0.0053,
    0.0038,0.0042,0.0046,0.0032,0.0037,0.0073,0.0049,0.0067,0.0116,0.0054,0.0077,0.0111,0.0042,0.0043,0.0037,0.0046,0.0041,0.0028,0.0055,
    0.0031,0.0048,0.0057,0.0056,0.0035,0.0039,0.0068,0.0051,0.0037,0.0054,0.0048,0.0061,0.0033,0.0050,0.0052,0.0047,0.0014,0.0043,0.0041,
    0.0023,0.0062,0.0036,0.0038,0.0039,0.0043,0.0049,0.0049,0.0036,0.0048,0.0039,0.0023,0.0035,0.0025,0.0036,0.0010,0.0044,0.0013,0.0041,
    0.0021,0.0016,0.0046,0.0040,0.0034,0.0027,0.0026,0.0034,0.0004,0.0037,0.0004,0.0036,0.0029,0.0029,0.0036,0.0055,0.0034,0.0034,0.0025,
    0.0028,0.0055,0.0064,0.0037,0.0029,0.0047,0.0058,0.0040,0.0062,0.0055,0.0029,0.0067,0.0070,0.0080,0.0060,0.0094,0.0082,0.0072,0.0089,
    0.0117,0.0102,0.0134,0.0131,0.0131,0.0120,0.0135,0.0096,0.0107,0.0179,0.0210,0.0172,0.0165,0.0167,0.0176,0.0137,0.0196,0.0217,0.0175,
    0.0223,0.0192,0.0222,0.0188,0.0184,0.0183,0.0156,0.0098,0.0198,0.0268,0.0188,0.0236,0.0208,0.0171,0.0229,0.0228,0.0227,0.0204,0.0184,
    0.0190,0.0185,0.0145,0.0138,0.0122,0.0180,0.0132,0.0146,0.0087,0.0039,0.0147,0.0000,0.0000,0.0137,0.0084,0.0094,0.0114,0.0078,0.0100,
    0.0069,0.0055,0.0164,0.0113,0.0148,0.0053,0.0054,0.0065,0.0092,0.0000,0.0047,0.0000,0.0071,0.0000,0.0057,0.0063,0.0064,0.0050,0.0077,
    0.0034,0.0025,0.0000,0.0041,0.0025,0.0019,0.0042,0.0030,0.0000,0.0030,0.0000,0.0000,0.0000,0.0027,0.0000,0.0000,0.0000,0.0000,0.0006,
    0.0051,0.0083,0.0000,0.0000,0.0064,0.0003,0.0002,0.0074,0.0038,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000};
  std::vector<G4double> CF4Scintillation_Slow = CF4Scintillation_Fast; // same spectrum

  const G4int iNbEntries_1 = 3;
  G4double CF4PhotonMomentum_1[iNbEntries_1] = {200*eV, 500*eV, 798*eV};
  G4double CF4RefractiveIndex[iNbEntries_1]  = {1.004,  1.004,  1.004};
  G4double CF4AbsorbtionLength[iNbEntries_1] = {100.*cm,100.*cm,100.*cm};
  G4double CF4ScatteringLength[iNbEntries_1] = {30.*cm, 30.*cm, 30.*cm};

  G4MaterialPropertiesTable* CF4PropertiesTable = new G4MaterialPropertiesTable();
  CF4PropertiesTable->AddProperty("SCINTILLATIONCOMPONENT1", CF4PhotonMomentum, CF4Scintillation_Fast, iNbEntries);
  CF4PropertiesTable->AddProperty("SCINTILLATIONCOMPONENT2", CF4PhotonMomentum, CF4Scintillation_Slow, iNbEntries);
  CF4PropertiesTable->AddProperty("RINDEX",    CF4PhotonMomentum_1, CF4RefractiveIndex,  iNbEntries_1);
  CF4PropertiesTable->AddProperty("ABSLENGTH", CF4PhotonMomentum_1, CF4AbsorbtionLength, iNbEntries_1);
  CF4PropertiesTable->AddProperty("RAYLEIGH",  CF4PhotonMomentum_1, CF4ScatteringLength, iNbEntries_1);
  CF4PropertiesTable->AddConstProperty("SCINTILLATIONYIELD",         2500./keV, true);
  CF4PropertiesTable->AddConstProperty("RESOLUTIONSCALE",            1.0);
  CF4PropertiesTable->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 3.*ns,  true);
  CF4PropertiesTable->AddConstProperty("SCINTILLATIONTIMECONSTANT2", 10.*ns, true);
  CF4PropertiesTable->AddConstProperty("YIELDRATIO",                 1.0,    true);
  CF4->SetMaterialPropertiesTable(CF4PropertiesTable);

  // Teflon
  const G4int TNbEntries = 4;
  G4Material* Teflon = new G4Material("Teflon", 2.2*g/cm3, 2, kStateSolid);
  Teflon->AddElement(C, 0.240183);
  Teflon->AddElement(F, 0.759817);
  G4double pdTeflonPhotonMomentum[TNbEntries]   = {1.0*eV, 6.91*eV, 6.98*eV, 7.05*eV};
  G4double pdTeflonRefractiveIndex[TNbEntries]  = {1.34,   1.34,    1.34,    1.34};
  G4double pdTeflonReflectivity[TNbEntries]     = {0.9,    0.9,     0.9,     0.9};
  G4double pdTeflonAbsLength[TNbEntries]        = {0.2*mm, 0.2*mm,  0.2*mm,  0.2*mm};
  G4double pdTeflonScatteringLength[TNbEntries] = {30*cm,  30*cm,   30*cm,   30*cm};
  G4MaterialPropertiesTable* pTeflonPropertiesTable = new G4MaterialPropertiesTable();
  pTeflonPropertiesTable->AddProperty("RINDEX",       pdTeflonPhotonMomentum, pdTeflonRefractiveIndex,  TNbEntries);
  pTeflonPropertiesTable->AddProperty("ABSLENGTH",    pdTeflonPhotonMomentum, pdTeflonAbsLength,        TNbEntries);
  pTeflonPropertiesTable->AddProperty("RAYLEIGH",     pdTeflonPhotonMomentum, pdTeflonScatteringLength, TNbEntries);
  pTeflonPropertiesTable->AddProperty("REFLECTIVITY", pdTeflonPhotonMomentum, pdTeflonReflectivity,     TNbEntries);
  Teflon->SetMaterialPropertiesTable(pTeflonPropertiesTable);

  // Polypropylene
  pNistManager->FindOrBuildMaterial("G4_POLYPROPYLENE");

  // Gold
  const G4int iNbEntries_Au = 4;
  G4Material* Gold = new G4Material("Gold", 19.30*g/cm3, 1, kStateSolid);
  Gold->AddElement(Au, 1.0);
  G4double AuPM[iNbEntries_Au]              = {1*eV,    2.5001*eV, 2.5001*eV, 7*eV};
  G4double AuReflectivity[iNbEntries_Au]    = {0.3,     0.3,       1,         1};
  G4double AuRefractiveIndex[iNbEntries_Au] = {1.25,    0.86,      0.75,      3.56};
  G4double AuAbsLength[iNbEntries_Au]       = {1e-5*mm, 1e-5*mm,   1e-5*mm,   1e-5*mm};
  G4double AuScatteringLength[iNbEntries_Au]= {30*cm,   30*cm,     30*cm,     30*cm};
  G4MaterialPropertiesTable* AuPropertiesTable = new G4MaterialPropertiesTable();
  AuPropertiesTable->AddProperty("RINDEX",       AuPM, AuRefractiveIndex,    iNbEntries_Au);
  AuPropertiesTable->AddProperty("ABSLENGTH",    AuPM, AuAbsLength,          iNbEntries_Au);
  AuPropertiesTable->AddProperty("RAYLEIGH",     AuPM, AuScatteringLength,   iNbEntries_Au);
  AuPropertiesTable->AddProperty("REFLECTIVITY", AuPM, AuReflectivity,       iNbEntries_Au);

  // Aluminium surface (reflecting)
  reflectMPT = new G4MaterialPropertiesTable();
  const G4int numEntriesAl = 11;
  G4double alPhotonEnergy[numEntriesAl] = {1.13*eV,1.24*eV,1.38*eV,1.55*eV,1.77*eV,2.07*eV,2.48*eV,3.10*eV,4.13*eV,6.20*eV,6.53*eV};
  G4double alReflectivity[numEntriesAl] = {0.92,0.92,0.91,0.90,0.89,0.87,0.85,0.82,0.78,0.70,0.65};
  G4double alEfficiency[numEntriesAl]   = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  reflectMPT->AddProperty("REFLECTIVITY", alPhotonEnergy, alReflectivity, numEntriesAl);
  reflectMPT->AddProperty("EFFICIENCY",   alPhotonEnergy, alEfficiency,   numEntriesAl);

  // Teflon surface (absorbing)
  absorbMPT = new G4MaterialPropertiesTable();
  G4double teflonReflectivity[numEntriesAl] = {0.05,0.05,0.05,0.05,0.05,0.05,0.05,0.05,0.05,0.05,0.05};
  G4double teflonEfficiency[numEntriesAl]   = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  absorbMPT->AddProperty("REFLECTIVITY", alPhotonEnergy, teflonReflectivity, numEntriesAl);
  absorbMPT->AddProperty("EFFICIENCY",   alPhotonEnergy, teflonEfficiency,   numEntriesAl);

  // Silicon optical properties
  G4MaterialPropertiesTable* siMPT = new G4MaterialPropertiesTable();
  const G4int iNbEntries_Si = 11;
  G4double SiPM[iNbEntries_Si]              = {1.13*eV,1.24*eV,1.38*eV,1.55*eV,1.77*eV,2.07*eV,2.48*eV,3.10*eV,4.13*eV,6.20*eV,6.53*eV};
  G4double SiRefractiveIndex[iNbEntries_Si] = {3.60,3.64,3.68,3.73,3.81,3.94,4.30,5.57,4.63,1.78,1.65};
  G4double SiAbsLength[iNbEntries_Si]       = {10.0*mm,5.0*mm,2.0*mm,0.5*mm,0.1*mm,0.03*mm,0.01*mm,0.001*mm,0.0001*mm,0.00001*mm,0.00001*mm};
  G4double SiScatteringLength[iNbEntries_Si]= {10000.*m,10000.*m,10000.*m,10000.*m,10000.*m,1000.*m,100.*m,10.*m,1.*m,0.1*m,0.1*m};
  siMPT->AddProperty("RINDEX",    SiPM, SiRefractiveIndex,    iNbEntries_Si);
  siMPT->AddProperty("ABSLENGTH", SiPM, SiAbsLength,          iNbEntries_Si);
  siMPT->AddProperty("RAYLEIGH",  SiPM, SiScatteringLength,   iNbEntries_Si);
  siMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
  silicon->SetMaterialPropertiesTable(siMPT);

  // CF4-Silicon optical boundary
  cf4SiSurface = new G4OpticalSurface("CF4_Si_Surface");
  cf4SiSurface->SetType(dielectric_dielectric);
  cf4SiSurface->SetFinish(polished);
  cf4SiSurface->SetModel(glisur);
  G4double cf4Rindex[iNbEntries_Si]  = {1.0005,1.0005,1.0005,1.0005,1.0005,1.0005,1.0005,1.0005,1.0005,1.0005,1.0005};
  G4double reflectivity_si[iNbEntries_Si];
  for (int i = 0; i < iNbEntries_Si; i++) {
      G4double n1 = cf4Rindex[i], n2 = SiRefractiveIndex[i];
      reflectivity_si[i] = ((n2-n1)/(n2+n1))*((n2-n1)/(n2+n1));
  }
  G4double efficiency_si[iNbEntries_Si] = {0.01,0.02,0.02,0.03,0.05,0.06,0.08,0.18,0.22,0.28,0.30};
  G4MaterialPropertiesTable* cf4SiSurfMPT = new G4MaterialPropertiesTable();
  cf4SiSurfMPT->AddProperty("REFLECTIVITY", SiPM, reflectivity_si, iNbEntries_Si);
  cf4SiSurfMPT->AddProperty("EFFICIENCY",   SiPM, efficiency_si,   iNbEntries_Si);
  cf4SiSurface->SetMaterialPropertiesTable(cf4SiSurfMPT);
}

//===========================================================================
// Sphereball
//===========================================================================
void DC::Sphereball(G4double position) {
    G4Sphere* sphereball = new G4Sphere("sphereball", 0, 1.5*cm, 0*deg,360*deg, 0*deg,180*deg);
    G4LogicalVolume* sphereVolume = new G4LogicalVolume(sphereball, b_polyethylene, "Sphere");
    new G4PVPlacement(0, G4ThreeVector(0,0,position), sphereVolume, "Sphere", experimentalHall_log, false, 0, true);
    G4VisAttributes* blue = new G4VisAttributes(G4Colour::Blue());
    blue->SetVisibility(true); blue->SetForceAuxEdgeVisible(true);
    sphereVolume->SetVisAttributes(blue);
}

//===========================================================================
// Box phantom
//===========================================================================
void DC::ConstructBoxPhantom(G4double position) {
    G4double box_x = 15.0*mm;  // half-width  in x
    G4double box_y = 15.0*mm;  // half-height in y
    G4double box_z = 15.0*mm;  // half-depth  in z (along beam)

    G4Box* solidBox = new G4Box("PhantomBox", box_x, box_y, box_z);
    G4LogicalVolume* boxLog = new G4LogicalVolume(solidBox, b_polyethylene, "PhantomBox_log");
    new G4PVPlacement(0, G4ThreeVector(0, 0, position), boxLog,
                      "PhantomBox_Vol", experimentalHall_log, false, 0, true);

    G4VisAttributes* blue = new G4VisAttributes(G4Colour::Blue());
    blue->SetVisibility(true);
    blue->SetForceAuxEdgeVisible(true);
    boxLog->SetVisAttributes(blue);
}

//===========================================================================
// ConstructResolutionPhantom
//===========================================================================
void DC::ConstructResolutionPhantom()
{
    G4double plateThickness = 2*cm, plateSize = 100*mm;
    G4Box* solidPlate = new G4Box("SolidPlate", plateSize/2, plateSize/2, plateThickness/2);
    std::vector<G4double> holeSizes  = {2.0*mm, 3.0*mm, 5.0*mm, 8.0*mm, 10.0*mm};
    std::vector<G4double> xPositions = {-20*mm,-10*mm,  0*mm,  10*mm,  20*mm};
    G4VSolid* plateWithHoles = solidPlate;
    for (size_t i = 0; i < holeSizes.size(); i++) {
        G4Tubs* solidHole = new G4Tubs("HoleSolid_"+std::to_string(i), 0, holeSizes[i]/2,
                                        plateThickness/2+0.1*mm, 0, 360*deg);
        plateWithHoles = new G4SubtractionSolid("PlateWithHole_"+std::to_string(i),
                                                 plateWithHoles, solidHole, nullptr,
                                                 G4ThreeVector(xPositions[i],0,0));
    }
    G4LogicalVolume* logicPlate = new G4LogicalVolume(plateWithHoles, b_polyethylene, "ResolutionPhantom");
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,100*mm), logicPlate, "ResolutionPhantom",
                      experimentalHall_log, false, 0, true);
    G4VisAttributes* phantomVis = new G4VisAttributes(G4Colour(0.5,0.5,0.0));
    phantomVis->SetVisibility(true); phantomVis->SetForceSolid(true);
    logicPlate->SetVisAttributes(phantomVis);
}

//===========================================================================
// ConstructValve
//===========================================================================
void DC::ConstructValve(G4double position) {
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* steel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    G4Material* water = nist->FindOrBuildMaterial("G4_WATER");
    G4double valveLength=100.0*mm, valveRadius=25.0*mm, cavityRadius=14.9*mm, stemRadius=6.0*mm;
    G4Tubs* solidValveOuter = new G4Tubs("ValveOuter",0,valveRadius,valveLength/2,0,360*deg);
    G4Tubs* solidValveHole  = new G4Tubs("ValveHole", 0,cavityRadius,valveLength/2+0.1*mm,0,360*deg);
    G4SubtractionSolid* solidValveBody = new G4SubtractionSolid("ValveBody",solidValveOuter,solidValveHole);
    G4LogicalVolume* logicValveBody = new G4LogicalVolume(solidValveBody,steel,"ValveBody");
    new G4PVPlacement(nullptr,G4ThreeVector(0,0,position),logicValveBody,"ValveBody",experimentalHall_log,false,0,true);
    G4Tubs* solidCavity = new G4Tubs("Cavity",0,cavityRadius,valveLength/2,0,360*deg);
    G4LogicalVolume* logicCavity = new G4LogicalVolume(solidCavity,water,"Cavity");
    new G4PVPlacement(nullptr,G4ThreeVector(0,0,position),logicCavity,"Cavity",experimentalHall_log,false,0,true);
    G4Tubs* solidStem = new G4Tubs("ValveStem",0,stemRadius,cavityRadius-0.1*mm,0,360*deg);
    G4LogicalVolume* logicStem = new G4LogicalVolume(solidStem,Aluminium,"ValveStem");
    G4RotationMatrix* stemRot = new G4RotationMatrix(); stemRot->rotateX(90*deg);
    new G4PVPlacement(stemRot,G4ThreeVector(0,0,0),logicStem,"ValveStem",logicCavity,false,0,true);
    G4VisAttributes* sv=new G4VisAttributes(G4Colour(0.6,0.6,0.6)); sv->SetVisibility(true); sv->SetForceSolid(true); logicValveBody->SetVisAttributes(sv);
    G4VisAttributes* wv=new G4VisAttributes(G4Colour(0,0,1)); wv->SetVisibility(true); wv->SetForceSolid(true); logicCavity->SetVisAttributes(wv);
    G4VisAttributes* stv=new G4VisAttributes(G4Colour(1,0,0)); stv->SetVisibility(true); stv->SetForceSolid(true); logicStem->SetVisAttributes(stv);
}

//===========================================================================
// ConstructLaboratory
//===========================================================================
void DC::ConstructLaboratory()
{
  G4Material* Vacuum = G4Material::GetMaterial("Vacuum");
  G4Material* CF4    = G4Material::GetMaterial("CF4");
  G4Material* Teflon = G4Material::GetMaterial("Teflon");
  G4Material* PP     = G4Material::GetMaterial("G4_POLYPROPYLENE");

  //--- World ---
  G4Box* experimentalHall_box = new G4Box("expHall_box", 5.0*m, 5.0*m, 5.0*m);
  experimentalHall_log  = new G4LogicalVolume(experimentalHall_box, Vacuum, "expHall_log");
  experimentalHall_phys = new G4PVPlacement(0, G4ThreeVector(), experimentalHall_log, "expHall", 0, false, 0);
  experimentalHall_log->SetVisAttributes(G4VisAttributes::GetInvisible());

  // Imaging phantom (comment/uncomment as needed)
  // Sphereball(200*mm);
   //ConstructValve(200*mm);
 // ConstructResolutionPhantom();
  ConstructBoxPhantom(200*mm);
  //--- PE shield (thin, for any fast neutron n-p conversion) ---
  G4double converter_x = 50.0*mm+collimatore, converter_y = 50.0*mm+collimatore, converter_z = 0.75*mm;
  auto lShield = new G4LogicalVolume(new G4Box("shield",converter_x,converter_y,converter_z), polyethylene, "Shield");
  new G4PVPlacement(0, G4ThreeVector(0.0,0.0,-1.5*mm-converter_z-1.6*um+70.0*cm),
                    lShield, "Shield", experimentalHall_log, false, 0, true);
  G4VisAttributes* yellow = new G4VisAttributes(G4Colour::Yellow());
  yellow->SetVisibility(true);
  lShield->SetVisAttributes(yellow);

  //--- PPAC gas volume ---
  G4double PPAC_x = 50.0*mm+collimatore, PPAC_y = 50.0*mm+collimatore, PPAC_z = 1.5*mm;
  G4Box* PPAC_box = new G4Box("Drift Volume", PPAC_x, PPAC_y, PPAC_z);
  PPAC_log  = new G4LogicalVolume(PPAC_box, CF4, "PPAC_log", 0, 0, 0);
  PPAC_phys = new G4PVPlacement(0, G4ThreeVector(0.0,0.0,70.0*cm),
                                 PPAC_log, "PPAC_Vol", experimentalHall_log, false, 0, true);
  fScoringVolume_1 = PPAC_log;

  //--- Collimator vertical bars ---
  G4double collf_x=collimatore/2, collf_y=0.5*mm, collf_z=1.5*mm;
  G4Box* collf_box = new G4Box("collimator", collf_x, collf_y, collf_z);
  collf_log = new G4LogicalVolume(collf_box, Teflon, "collf_log", 0, 0, 0);
  for (int a=0; a<34; a++) {
    collf_phys = new G4PVPlacement(0,G4ThreeVector(-50.0*mm-collf_x,(-49.5+a*3)*mm,0.0),collf_log,"collf_Vol",PPAC_log,false,a,true);
    collf_phys = new G4PVPlacement(0,G4ThreeVector( 50.0*mm+collf_x,(-49.5+a*3)*mm,0.0),collf_log,"collf_Vol",PPAC_log,false,a+33,true);
  }

  //--- Collimator horizontal bars ---
  G4double coll_x=0.5*mm, coll_y=collimatore/2, coll_z=1.5*mm;
  G4Box* coll_box = new G4Box("collimator", coll_x, coll_y, coll_z);
  coll_log = new G4LogicalVolume(coll_box, Teflon, "collf_log", 0, 0, 0);
  for (int b=0; b<34; b++) {
    coll_phys = new G4PVPlacement(0,G4ThreeVector((-49.5+b*3)*mm,-50.0*mm-coll_y,0.0),coll_log,"coll_Vol",PPAC_log,false,b,true);
    coll_phys = new G4PVPlacement(0,G4ThreeVector((-49.5+b*3)*mm, 50.0*mm+coll_y,0.0),coll_log,"coll_Vol",PPAC_log,false,b+33,true);
  }

  //--- SiPM sensors ---
  G4VisAttributes* red = new G4VisAttributes(G4Colour::Red());
  red->SetVisibility(true); red->SetForceAuxEdgeVisible(true);

  G4double sx=0.5*mm, sy=1.0*mm, sz=0.5*mm;
  G4Box* sensor_box = new G4Box("sensor", sx, sy, sz);
  sensor_log1 = new G4LogicalVolume(sensor_box, silicon, "sensor_log1", 0,0,0); sensor_log1->SetVisAttributes(red);
  sensor_log2 = new G4LogicalVolume(sensor_box, silicon, "sensor_log2", 0,0,0); sensor_log2->SetVisAttributes(red);
  for (int c=0; c<33; c++) {
    sensor_phys = new G4PVPlacement(0,G4ThreeVector((-50.0*mm-collimatore)+sx,(-49.5+c*3+1.5)*mm,0.0),sensor_log1,"sensor_Vol1",PPAC_log,false,c,true);
    sensor_phys = new G4PVPlacement(0,G4ThreeVector(( 50.0*mm+collimatore)-sx,(-49.5+c*3+1.5)*mm,0.0),sensor_log2,"sensor_Vol2",PPAC_log,false,c,true);
  }

  G4double sx2=1.0*mm, sy2=0.5*mm;
  G4Box* sensor_box2 = new G4Box("sensor2", sx2, sy2, sz);
  sensor_log3 = new G4LogicalVolume(sensor_box2, silicon, "sensor_log3", 0,0,0); sensor_log3->SetVisAttributes(red);
  sensor_log4 = new G4LogicalVolume(sensor_box2, silicon, "sensor_log4", 0,0,0); sensor_log4->SetVisAttributes(red);
  for (int d=0; d<33; d++) {
    G4double xpos = (-49.5+d*3+1.5)*mm;
    sensor_phys2 = new G4PVPlacement(0,G4ThreeVector(xpos,(-50.0*mm-collimatore)+sy2,0.0),sensor_log3,"sensor_Vol3",PPAC_log,false,d,true);
    sensor_phys2 = new G4PVPlacement(0,G4ThreeVector(xpos,( 50.0*mm+collimatore)-sy2,0.0),sensor_log4,"sensor_Vol4",PPAC_log,false,d,true);
  }

  //===========================================================================
  // B-10 CONVERTER — standalone volume, placed just upstream of the cathode
  //
  // Stack (beam travels in +z):
  //   z = 70cm + 1.5mm + 0.8µm        → cathode outer face
  //   z = 70cm + 1.5mm + 0.8µm + 1µm  → converter outer face  (beam enters here)
  //   z = 70cm + 1.5mm + 0.8µm + 2µm  → converter inner face  (alphas exit here)
  //
  // Neutron enters converter → B-10(n,α)Li-7 → alpha + Li-7 travel back
  // into the CF4 gas through the thin cathode and produce scintillation.
  //
  // Thickness 1 µm: optimal — alpha range in B ~3-4 µm so most escape.
  // 96% B-10 enrichment: cross-section 3840 barns at 0.025 eV.
  //===========================================================================
  G4double conv_z   = 1.0*um;
  G4double cathode_x = 50.0*mm+collimatore;
  G4double cathode_y = 50.0*mm+collimatore;
  G4double cathode_z = 0.8*um;

  // Converter z-centre = cathode outer face + conv_z
  G4double conv_zpos = 70.0*cm + 1.5*mm + cathode_z + cathode_z + conv_z;
  //                             ↑ PPAC half    ↑ cathode centre offset  ↑ past cathode

  // Simpler: place converter immediately after cathode outer face
  // cathode centre = 70cm + 1.5mm + cathode_z  (from original code)
  // cathode outer face (beam side) = cathode_centre + cathode_z
  //                                = 70cm + 1.5mm + 2*cathode_z
  // converter centre = cathode_outer_face + conv_z
  //                  = 70cm + 1.5mm + 2*cathode_z + conv_z
  G4double cathode_centre_z = 70.0*cm + 1.5*mm + cathode_z;
  G4double cathode_outer_z  = cathode_centre_z + cathode_z;
  G4double conv_centre_z    = cathode_outer_z  + conv_z;

  G4Box* conv_box = new G4Box("B10Converter", cathode_x, cathode_y, conv_z);
  G4LogicalVolume* conv_log = new G4LogicalVolume(conv_box, boron_converter, "B10Conv_log");
  new G4PVPlacement(0,
      G4ThreeVector(0.0, 0.0, conv_centre_z),
      conv_log, "B10Converter_Vol",
      experimentalHall_log,   
      false, 0, true);

  G4VisAttributes* green = new G4VisAttributes(G4Colour::Green());
  green->SetVisibility(true); green->SetForceSolid(true);
  conv_log->SetVisAttributes(green);

  //===========================================================================
  // CATHODE — just downstream of converter
  //===========================================================================
  G4Box* cathode_box = new G4Box("cathode Volume", cathode_x, cathode_y, cathode_z);
  cathode_log  = new G4LogicalVolume(cathode_box, PP, "cathode_log", 0,0,0);
  cathode_phys = new G4PVPlacement(0,
      G4ThreeVector(0.0, 0.0, cathode_centre_z),
      cathode_log, "cathode_Vol", experimentalHall_log, false, 0, true);

  //--- Anode ---
  G4double anode_x = 50.0*mm+collimatore, anode_y = 50.0*mm+collimatore, anode_z = 0.8*um;
  G4Box* anode_box = new G4Box("anode Volume", anode_x, anode_y, anode_z);
  anode_log  = new G4LogicalVolume(anode_box, PP, "anode_log", 0,0,0);
  anode_phys = new G4PVPlacement(0,
      G4ThreeVector(0.0,0.0,-1.5*mm-anode_z+70.0*cm),
      anode_log, "anode_Vol", experimentalHall_log, false, 0, true);

  //--- Cathode Al coating (inside cathode_log, on gas-facing side) ---
  G4double cathodeAl_z = 0.05*um;
  G4Box* cathodeAl_box = new G4Box("cathodeAl Volume", cathode_x, cathode_y, cathodeAl_z);
  cathodeAl_log  = new G4LogicalVolume(cathodeAl_box, Aluminium, "cathodeAu_log");
  cathodeAl_phys = new G4PVPlacement(0,
      G4ThreeVector(0.0,0.0,-cathode_z+cathodeAl_z),
      cathodeAl_log, "cathodeAl_Vol", cathode_log, false, 0, true);

  //--- Anode Al coating ---
  G4double anodeAl_z = 0.05*um;
  G4Box* anodeAl_box = new G4Box("anode Volume", anode_x, anode_y, anodeAl_z);
  anodeAl_log  = new G4LogicalVolume(anodeAl_box, Aluminium, "anodeAu_log");
  anodeAl_phys = new G4PVPlacement(0,
      G4ThreeVector(0.0,0.0,+anode_z-anodeAl_z),
      anodeAl_log, "anodeAu_Vol", anode_log, false, 0, true);

  //--- Optical surfaces ---
  G4OpticalSurface* oppac_Al_gas = new G4OpticalSurface("Reflecting");
  oppac_Al_gas->SetModel(unified); oppac_Al_gas->SetType(dielectric_metal);
  oppac_Al_gas->SetFinish(polished); oppac_Al_gas->SetMaterialPropertiesTable(reflectMPT);
  new G4LogicalBorderSurface("oppac_1", PPAC_phys, cathodeAl_phys, oppac_Al_gas);
  new G4LogicalBorderSurface("oppac_2", PPAC_phys, anodeAl_phys,   oppac_Al_gas);
  new G4LogicalBorderSurface("oppac_3", coll_phys, cathodeAl_phys, oppac_Al_gas);
  new G4LogicalBorderSurface("oppac_4", coll_phys, anodeAl_phys,   oppac_Al_gas);

  G4OpticalSurface* oppac_ab = new G4OpticalSurface("Absorbing");
  oppac_ab->SetModel(unified); oppac_ab->SetType(dielectric_dielectric);
  oppac_ab->SetFinish(ground); oppac_ab->SetMaterialPropertiesTable(absorbMPT);
  new G4LogicalSkinSurface("caccola_1", collf_log, oppac_ab);
  new G4LogicalSkinSurface("caccola_2", coll_log,  oppac_ab);

  new G4LogicalSkinSurface("cf4si_1", sensor_log1, cf4SiSurface);
  new G4LogicalSkinSurface("cf4si_2", sensor_log2, cf4SiSurface);
  new G4LogicalSkinSurface("cf4si_3", sensor_log3, cf4SiSurface);
  new G4LogicalSkinSurface("cf4si_4", sensor_log4, cf4SiSurface);
}

//===========================================================================
// ConstructSDandField
//===========================================================================
void DC::ConstructSDandField()
{
    MySensitiveDetector* sensDet = new MySensitiveDetector("SensitiveDetector");
    G4SDManager::GetSDMpointer()->AddNewDetector(sensDet);
    sensor_log1->SetSensitiveDetector(sensDet);
    sensor_log2->SetSensitiveDetector(sensDet);
    sensor_log3->SetSensitiveDetector(sensDet);
    sensor_log4->SetSensitiveDetector(sensDet);
}
