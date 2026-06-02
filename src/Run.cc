#include "Run.hh"
#include <vector>

#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

//
//  Constructor. 
//   (The vector of MultiFunctionalDetector name has to given.)
/*
Run::Run(const std::vector<G4String> mfdName): G4Run()
{
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  //=================================================
  //  Initalize RunMaps for accumulation.
  //  Get CollectionIDs for HitCollections.
  //=================================================
  G4int Nmfd = mfdName.size();
  for ( G4int idet = 0; idet < Nmfd ; idet++)
  {  
    // Loop for all MFD.
    G4String detName = mfdName[idet];
    //G4cout << "** collection found" << detName << G4endl;
    //--- Seek and Obtain MFD objects from SDmanager.
    G4MultiFunctionalDetector* mfd = (G4MultiFunctionalDetector*)(SDman->FindSensitiveDetector(detName));
    //
    if ( mfd )
    {
      totale = 0;
      conta = 0;
      //--- Loop over the registered primitive scorers.
      for (G4int icol = 0; icol < mfd->GetNumberOfPrimitives(); icol++)
      {
        // Get Primitive Scorer object.
        G4VPrimitiveScorer* scorer = mfd->GetPrimitive(icol);
        // collection name and collectionID for HitsCollection,
        // where type of HitsCollection is G4THitsMap in case of primitive scorer.
        // The collection name is given by <MFD name>/<Primitive Scorer name>.
        G4String collectionName = scorer->GetName();
        G4String fullCollectionName = detName+ "/" +collectionName;
        G4int collectionID = SDman->GetCollectionID(fullCollectionName);
        //
        if ( collectionID >= 0 )
        {
          // Store obtained HitsCollection information into data members.
          // And, creates new G4THitsMap for accumulating quantities during RUN.
          theCollName.push_back(fullCollectionName);
          theCollID.push_back(collectionID);
          theRunMap.push_back(new G4THitsMap<G4double>(detName,collectionName));
          //G4cout << "** collection found " << G4endl;
        }
        else
        {
          G4cout << "** collection " << fullCollectionName << " not found. "<< G4endl;
        } 
      }
    }
  }
}

Run::~Run()
{
  //--- Clear HitsMap for RUN
  G4int Nmap = theRunMap.size();
  for ( G4int i = 0; i < Nmap; i++)
  {
    if(theRunMap[i] ) theRunMap[i]->clear();
  }
  theCollName.clear();
  theCollID.clear();
  theRunMap.clear();
  vec.clear();
}

void Run::RecordEvent(const G4Event* aEvent)
{
  // G4cout << " Numero Evento" << numberOfEvent << G4endl;
  numberOfEvent++;  // This is an original line.
  //=============================
  // HitsCollection of This Event
  //============================
  G4HCofThisEvent* HCE = aEvent->GetHCofThisEvent();
  if (!HCE) return;
  //=======================================================
  // Sum up HitsMap of this Event  into HitsMap of this RUN
  //=======================================================
  G4int Ncol = theCollID.size();
  for ( G4int i = 0; i < Ncol ; i++ )
  {  // Loop over HitsCollection
    G4THitsMap<G4double>* EvtMap=0;
    if ( theCollID[i] >= 0 )
    {           // Collection is attached to HCE
      EvtMap = (G4THitsMap<G4double>*)(HCE->GetHC(theCollID[i])); 
    }
    else
    {
      G4cout <<" Error EvtMap Not Found "<< i << G4endl;
    }
    if ( EvtMap )  
    {
      std::map<G4int,G4double*>::iterator itr = EvtMap->GetMap()->begin();
      //=== Sum up HitsMap of this event to HitsMap of RUN.===
      *theRunMap[i] += *EvtMap;
      totale = totale + EvtMap->entries();
      if ( EvtMap->entries() == 0)
      {
        //    G4cout << std::setprecision(3) 
        //     << " Efficiency " << totale/numberOfEvent << " Energy 0.000 " << G4endl;
      }
      else if ( EvtMap->entries() >= 1)
      {
        for(; itr != EvtMap->GetMap()->end(); itr++) 
        {
          //     G4cout << std::setprecision(3)
          //      << " Efficiency " << totale/numberOfEvent << " Energy " << *(itr->second)/keV << G4endl;
          G4double putin = *(itr->second)/keV;
//          G4double putin = *(itr->second);
          //G4cout << " Energy deposited (KeV) " << putin << " Det: " << itr->first << G4endl;
          G4cout << "Deposited Energy = " << putin << " keV" << G4endl;
          ///G4cout << "" << putin << "" << G4endl;
          vec.push_back(putin);
          conta++;
        }
      }
    }
  }
}

//-----
// Access HitsMap.
//  By full description of collection name, that is
//    <MultiFunctional Detector Name>/<Primitive Scorer Name>
G4THitsMap<G4double>* Run::GetHitsMap(const G4String& fullName)
{
  G4int Ncol = theCollName.size();
  for ( G4int i = 0; i < Ncol; i++)
  {
    if ( theCollName[i] == fullName )
    {
      return theRunMap[i];
    }
  }
  return NULL;
}

//-----
// - Dump All HitsMap of this RUN. (for debuging and monitoring of quantity).
//   This method calls G4THisMap::PrintAll() for individual HitsMap.
void Run::DumpAllScorer()
{
  std::vector<G4double>::iterator it;  
  if (vec.size() > 0) 
  {
    for(it = vec.begin(); it != vec.end(); it++)
    {
      G4cout << " Energy deposited in each event " << (*it/keV) << " KeV " << " " << G4endl;
    }
  }
}

*/
