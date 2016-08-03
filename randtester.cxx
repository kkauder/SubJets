//#include "src/DevSubjetAnalysis.hh"
#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <TChain.h>

// Not needed for analysis per se
#include "TStarJetPicoReader.h"
#include "TStarJetPicoEvent.h"
#include "TStarJetPicoEventHeader.h"
#include "TStarJetPicoEventCuts.h"

#include "TStarJetPicoPrimaryTrack.h"
#include "TStarJetPicoTrackCuts.h"
#include "TStarJetPicoTowerCuts.h"

#include "TStarJetVectorContainer.h"
#include "TStarJetVector.h"
#include "TStarJetPicoTriggerInfo.h"
#include "TStarJetPicoUtils.h"
#include "TStarJetVectorJet.h"


#include <map>
#include <iostream>
#include <fstream>
using namespace std;

void randtester(){
  map<UInt_t, UInt_t> UsedSeeds;

  TChain * T = new TChain ( "JetTree" );
  T->Add("Data/ppHT/*root");

  int eventid;
  int runid;

  Long64_t NEvents=T->GetEntries();
  
  TStarJetPicoReader* pReader=new TStarJetPicoReader;
  pReader->SetInputChain (T);
  pReader->SetProcessV0s(false);
  
  TStarJetPicoEventCuts* evCuts = pReader->GetEventCuts();
  evCuts->SetTriggerSelection( "ppHT" ); //All, MB, HT, pp, ppHT, ppJP
  
  TStarJetPicoReader& reader = *pReader;
  reader.SetApplyFractionHadronicCorrection(kTRUE);
  reader.SetFractionHadronicCorrection(0.9999);
  reader.SetRejectTowerElectrons( kFALSE );
  reader.Init(NEvents);

  TStarJetPicoTowerCuts* towerCuts = reader.GetTowerCuts();  
  towerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/OrigY7MBBadTowers.txt");

  TStarJetPicoDefinitions::SetDebugLevel(0); // 10 for more output

  TStarJetPicoEventHeader* header=0;
  while ( true ){
    if ( !pReader->NextEvent() ) {
      // cout << "Can't find a next event" << endl;
      // done=true;
      break;
    }
    pReader->PrintStatus(10);

    header = pReader->GetEvent()->GetHeader();
    eventid = header->GetEventId();
    runid   = header->GetRunId();

    UInt_t seed = eventid*runid % UINT_MAX;
    if ( UsedSeeds[seed]>1 ) {
      cerr << "found one" << endl;
      UsedSeeds[seed]=UsedSeeds[seed]+1;
    } else {
      UsedSeeds[seed]=1;
    }
  }

  map<UInt_t, UInt_t>::iterator it;
  ofstream of("randout.txt");
  for ( it= UsedSeeds.begin(); it!= UsedSeeds.end(); ++it ){
    of << it->first << "  --> " << it->second << endl;
  }

}
