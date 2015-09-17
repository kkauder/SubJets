/** @file PythiaInAuAuSubjetWrapper.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Subjet analysis on pythia jets embedded in Au+Au
    @date Mar 9, 2015
*/

#include "SubjetParameters.hh"
#include "SubjetAnalysis.hh"

#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <TChain.h>
#include <TFile.h>
#include <TBranch.h>
#include <TMath.h>
#include <TRandom.h>
#include "TString.h"
#include "TObjString.h"

using namespace std;
using namespace fastjet;

/**
    - Set up vector of pythia jets
    - Set up Au+Au input tree
    - Set up output histos and tree
    - First cycle over Au+Au - Determine acceptable events
    - To each pythia event, assign at random a fixed amount of Au+Au events
    that it wishes to be mixed with
    - Second cycle over Au+Au - for each event, perform Subjet analysis
    with every pythia event that asks for it.
    The idea behind this two step approach is that we avoid jumping at random through the AuAu chain,
    the tree gets read sequentially, and exactly twice.
*/

int main(){
  // How many times to use every pythia jet
  // ----------------------------------
  Int_t nMix=3; // Reuse pythia jets to keep size manageable

  // read in input particles
  //----------------------------------------------------------
  TString PythiaName = "pytest.root";
  TChain* PythiaJets = new TChain("tree");
  PythiaJets->Add(PythiaName);
  assert ( PythiaJets->GetEntries()>0 && "Something went wrong loading the Pythia jets.");

  TClonesArray* pFullEvent = new TClonesArray("TLorentzVector");
  PythiaJets->GetBranch("Particles")->SetAutoDelete(kFALSE);
  PythiaJets->SetBranchAddress("Particles", &pFullEvent);

  TClonesArray* pHardPartons= new TClonesArray("TLorentzVector");
  PythiaJets->GetBranch("HardPartons")->SetAutoDelete(kFALSE);
  PythiaJets->SetBranchAddress("HardPartons", &pHardPartons);

  TClonesArray* pHardPartonNames= new TClonesArray("TObjString");
  PythiaJets->GetBranch("HardPartonNames")->SetAutoDelete(kFALSE);
  PythiaJets->SetBranchAddress("HardPartonNames", &pHardPartonNames);

  vector< vector<PseudoJet> > FullPythiaEvent;
  vector<PseudoJet> CurrentPythiaEvent;
  TLorentzVector* lv;

  vector< vector<PseudoJet> > TriggerPartons;
  vector<PseudoJet> CurrentTriggerPartons;

  // Int_t NPythiaEvents=PythiaJets->GetEntries();
  Int_t NPythiaEvents=99999 / nMix;
  cout << "Filling pythia vector" << endl;
  for ( Long64_t pythiaEv = 0; pythiaEv< NPythiaEvents ; ++ pythiaEv ){
    if ( !(pythiaEv%1000) ) cout << "Working on " << pythiaEv << " / " << NPythiaEvents << endl;
    PythiaJets->GetEntry(pythiaEv);
    CurrentPythiaEvent.clear();
    for ( int i=0 ; i<pFullEvent->GetEntries() ; ++i ){
      lv = (TLorentzVector*) pFullEvent->At(i);
      // Ensure kinematic similarity
      if ( lv->Pt()<SubjetParameters::PtConsMin ) continue;
      if ( fabs( lv->Eta()>1) ) continue;
      CurrentPythiaEvent.push_back( PseudoJet (*lv ) );
    }
    FullPythiaEvent.push_back ( CurrentPythiaEvent );

    // Save original hard partons
    // --------------------------
    CurrentTriggerPartons.clear();
    for ( int i=0 ; i<pHardPartons->GetEntries() ; ++i ){
      lv = (TLorentzVector*) pHardPartons->At(i);
      PseudoJet pj = PseudoJet (*lv );

      // flavor info
      TString& s = ((TObjString*)(pHardPartonNames->At(i)))->String();
      int qcharge=-999;
      if ( s=="g" ) qcharge = 0;

      if ( s(0)=='u' || s(0)=='c' || s(0)=='t' ) qcharge  = 2;
      if ( s(0)=='d' || s(0)=='s' || s(0)=='b' ) qcharge = -1;
      if ( s.Contains("bar") ) qcharge*=-1;

      if ( abs ( qcharge ) >3 ) cout<< s << endl;

      pj.set_user_info ( new JetAnalysisUserInfo( qcharge ) );
      CurrentTriggerPartons.push_back( pj );
    }
    TriggerPartons.push_back( CurrentTriggerPartons );
  }

  // Load and set up AuAu tree
  // -------------------------
  TString ChainName  = "JetTree";
  TString TriggerName = "MB";
  TString InFileName = "Data/AuAuMB_0_20/*.root";

  TChain* chain = new TChain( ChainName );
  chain->Add( InFileName );

  TStarJetVectorContainer<TStarJetVector>* container;
  TStarJetPicoReader reader = SetupReader( chain, TriggerName );
  // TStarJetPicoDefinitions::SetDebugLevel(10);

  // Files and histograms
  // --------------------
  TString OutFileName = "HighCutPythiaEmbSubjetResult.root";
  TFile* fout = new TFile( OutFileName, "RECREATE");

  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);

  TH2D*  NsubPt      = new TH2D( "NsubPt"   ,"# of subjets;p_{T}^{lead} [GeV/c];#", 100, 10 , 60, 10, -0.5, 9.5 );
  TH2D*  SubPtFrac   = new TH2D( "SubPtFrac  ","pT fraction carried by leading subjet;p_{T}^{lead} [GeV/c];", 100, 10 , 60, 50, 0, 1 );
  TH2D*  OtherPtFrac = new TH2D( "OtherPtFrac","pT fraction carried by non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 10 , 60, 50, 0, 1 );

  TH2D* gNsubPt      = new TH2D( "gNsubPt"   ,"gluon-led, # of subjets;p_{T}^{lead} [GeV/c];#", 100, 10 , 60, 10, -0.5, 9.5 );
  TH2D* gSubPtFrac   = new TH2D( "gSubPtFrac  ","gluon-led, pT fraction carried by leading subjet;p_{T}^{lead} [GeV/c];", 100, 10 , 60, 50, 0, 1 );
  TH2D* gOtherPtFrac = new TH2D( "gOtherPtFrac","gluon-led, pT fraction carried by non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 10 , 60, 50, 0, 1 );

  TH2D* qNsubPt      = new TH2D( "qNsubPt"   ,"quark-led, # of subjets;p_{T}^{lead} [GeV/c];#", 100, 10 , 60, 10, -0.5, 9.5 );
  TH2D* qSubPtFrac   = new TH2D( "qSubPtFrac  ","quark-led, pT fraction carried by leading subjet;p_{T}^{lead} [GeV/c];", 100, 10 , 60, 50, 0, 1 );
  TH2D* qOtherPtFrac = new TH2D( "qOtherPtFrac","quark-led, pT fraction carried by non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 10 , 60, 50, 0, 1 );

  // For pythia jet selection
  // ------------------------
  gRandom->SetSeed(1);

  // Long64_t nEvents=100; // -1 for all
  Long64_t nEvents=-1;
  reader.Init(nEvents);

  // First round - initialize AuAu data
  // ----------------------------------
  cout << "First round - find acceptable AuAu events" << endl;
  // NOTE: This could be a problem if we ever have more than INTMAX events :-/
  // Could solve using http://en.cppreference.com/w/cpp/numeric/random
  vector <Int_t> AcceptedEvents;
  while ( reader.NextEvent() ) {
    reader.PrintStatus(12);
    AcceptedEvents.push_back( reader.GetNOfCurrentEvent() );
  }

  // Now every pythia jet gets assigned nMix many events to be embedded in
  // This way we can cycle through the potentially large AuAu chain sequentially
  // ---------------------------------------------------------------------------
  cout << "Initializing mixing pools." << endl;
  Int_t random;
  int emergencystop=nMix*100;
  vector< set<Int_t> > MixingPool;
  for ( int i =0; i< FullPythiaEvent.size() ; ++i ){
    set<Int_t> EventNos;
    int tries=0;
    while ( EventNos.size() < nMix ){
      if ( tries++ > emergencystop ) { cerr << "Stuck in endless loop - aborting." << endl; return -1;}
      random = gRandom->Integer (AcceptedEvents.size() );
      EventNos.insert(AcceptedEvents.at(random) );
    }
    MixingPool.push_back( EventNos );
  }

  // Second round - actual analysis
  // ------------------------------
  cout << "Second round - embed" << endl;

  // Initialize analysis class
  // -------------------------
  SubjetAnalysis SubjetA( SubjetParameters::R, SubjetParameters::PtJetMin, SubjetParameters::PtJetMax,
			  SubjetParameters::EtaConsCut, SubjetParameters::PtConsMin, SubjetParameters::PtConsMax,
			  NsubPt, SubPtFrac, OtherPtFrac,
			  gNsubPt, gSubPtFrac, gOtherPtFrac,
			  qNsubPt, qSubPtFrac, qOtherPtFrac
			  );

  // Reset the reader
  // ----------------
  reader.ResetEventCounters();
  reader.Init(nEvents);

  // Go through events
  // -----------------
  cout << "Running analysis" << endl;
  Long64_t nJetsUsed=0;

  vector<PseudoJet> AuAuparticles;
  while ( reader.NextEvent() ) {
    reader.PrintStatus(12);

    // Load event
    // ----------
    container = reader.GetOutputContainer();

    // Make AuAu vector
    // ----------------
    AuAuparticles.clear();
    for (int ip = 0; ip<container->GetEntries() ; ++ip ){
      lv = container->Get(ip);
      AuAuparticles.push_back ( MakePseudoJet( lv ) );
    }

    // Find jets that want this event
    // ------------------------------
    vector <int> JetIndices;
    for ( int i =0; i< MixingPool.size() ; ++i ){
      set<Int_t>& EventNos = MixingPool.at(i);
      if ( EventNos.find( reader.GetNOfCurrentEvent() ) != EventNos.end() ) {
	JetIndices.push_back( i );
      }
    }

    // And run analysis individually for each of these pythia events
    // -------------------------------------------------------------
    for ( vector<int>::iterator jit=JetIndices.begin(); jit!=JetIndices.end(); ++ jit ){
      // Add pythia jets
      // -----------
      vector<PseudoJet> particles = AuAuparticles;
      for ( int i=0; i < FullPythiaEvent.at(*jit).size() ; ++i ){
	particles.push_back( FullPythiaEvent.at(*jit).at(i) );
      }

      // Match to original hard parton
      // -----------------------------
      vector<PseudoJet>& partons = TriggerPartons.at(*jit);

      // Run analysis
      // ------------
      int ret =SubjetA.AnalyzeAndFill( particles, &partons );

      switch ( ret ){
      case 0 :
	// cout << "Ok." << endl;
	nJetsUsed++;
	break;
      case -1 :
	// cerr << "No suitable jets found." << endl;
	// Most likely cause is pT<20
	break;
      default :
	throw( string("Unrecognized return value!" ) );
	return -1;
	break;      // Superfluous, but CINT can be brittle (assuming this is ever run in ROOT)
      } //  switch (ret)
    } // jit
  } // reader.NextEvent()
  cout << "##################################################################" << endl;

  // // Scale per used pythia event
  // // ------------------------
  // UsedEventsHiPhiEtaPt->Scale( 1./nJetsUsed );

  cout << "Wrote to " << fout->GetName() << endl;
  cout << "Bye." << endl;

  fout->Write();
  return 0;
}
