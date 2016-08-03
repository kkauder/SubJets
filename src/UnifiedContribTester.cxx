/** @file UnifiedSubjetWrapper.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Subjet Analysis for pythia and/or real events.
    @details Perform Subjet analysis on a chain of LorentzVectors or TStarJetPicoTrees.
    @date Sep 17, 2015
*/

#include "DevSubjetAnalysis.hh"

#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <TChain.h>
#include <TFile.h>
#include <TBranch.h>
#include <TMath.h>
#include <TRandom.h>
#include "TString.h"
#include "TObjString.h"


#include <cmath>
#include <climits>

using namespace std;
using namespace fastjet;

#include "fastjet/contrib/Recluster.hh"

ostream & operator<<(ostream &, const PseudoJet &);

/** 
    - Parse parameters
    - Set up input tree
    - Set up output histos and tree
    - Initialize SubjetAnalysis object
    - If needed, combine input from two sources
    - Loop through events
    \arg argv: flags.
    Display options with
    <BR><tt>% UnifiedSubjetWrapper -h </tt> 
    <BR>Note that wildcarded file patterns should be in single quotes.
*/


int main( int argc, const char** argv ){
  
  enum INTYPE{ INTREE, INPICO };

  // Defaults
  // --------
  // Pythia
  // ------
  TString InputName = "Data/PythiaAndMc_0.root";
  INTYPE intype = INTREE;
  TString ChainName = "tree";

  // // pico
  // // ------
  // TString InputName = "Data/SmallAuAu/Small_Clean809.root";
  // INTYPE intype = INPICO;
  // TString ChainName = "JetTree";

  // Allow Embedding
  // ---------------
  TString EmbInputName = "Data/PythiaAndMc_0.root";
  INTYPE Embintype = INTREE;
  TString EmbChainName = "tree";

  // ----- Common ----
  TString OutFileName = "Results/TmpSubjetResult.root";

  float R    = SubjetParameters::R;
  float SubR = SubjetParameters::SubR;
  JetAlgorithm LargeJetAlgorithm = SubjetParameters::LargeJetAlgorithm;
  JetAlgorithm SubJetAlgorithm   = SubjetParameters::SubJetAlgorithm;
  
  float PtJetMin      = SubjetParameters::PtJetMin;
  float PtJetMax      = SubjetParameters::PtJetMax;
  float EtaConsCut    = SubjetParameters::EtaConsCut;
  float PtConsMin     = SubjetParameters::PtConsMin;
  float PtConsMax     = SubjetParameters::PtConsMax;

  TString TriggerName = "HT"; // FIXME
    
  // parse arguments
  // ---------------
  vector<string> arguments(argv + 1, argv + argc);
  bool argsokay=true;
  Long64_t NEvents=-1;
  for ( vector<string>::iterator parg = arguments.begin() ; parg!=arguments.end() ; ++parg){
    string arg=*parg;
    if ( arg == "-R" ){      
      if (++parg==arguments.end() ){ argsokay=false; break; }
      R = atof((parg)->data());
    } else if ( arg == "-S" ){
      if (++parg==arguments.end() ){ argsokay=false; break; }
      SubR = atof((parg)->data());
    } else if ( arg == "-lja" ){     
      if (++parg==arguments.end() ){ argsokay=false; break; }
      LargeJetAlgorithm = AlgoFromString ( *parg);
    } else if ( arg == "-sja" ){
      if (++parg==arguments.end() ){ argsokay=false; break; }
      SubJetAlgorithm = AlgoFromString ( *parg);
    } else if ( arg == "-pj" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtJetMin = atof((parg)->data());
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtJetMax = atof((parg)->data());
    } else if ( arg == "-pc" ){      
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtConsMin = atof((parg)->data());      
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtConsMax = atof((parg)->data());
    } else if ( arg == "-psc" ){      
      cerr << "Different constituent cut for subjets currently not working, sorry." << endl;
      argsokay=false; break;
      // if (++parg ==arguments.end() ){ argsokay=false; break; }
      // PtSubConsMin = atof((parg)->data());      
      // if (++parg ==arguments.end() ){ argsokay=false; break; }
      // PtSubConsMax = atof((parg)->data());
    } else if ( arg == "-o" ){     
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      OutFileName=*parg;
    } else if ( arg == "-i" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      InputName=*parg;
    } else if ( arg == "-c" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      ChainName=*parg;
    } else if ( arg == "-intype" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      if ( *parg == "pico" ){
	intype = INPICO;
	continue;
      }
      if ( *parg == "tree" ){
	intype = INTREE;
	continue;
      }
      argsokay=false;
      break;
    } else if ( arg == "-N" ){      
      if (++parg==arguments.end() ){ argsokay=false; break; }
      NEvents=atoi(parg->data());
    } else {
      argsokay=false;
      break;
    }
  }
  if ( !argsokay ) {
    cerr << "usage: " << argv[0]
	 << " [-o OutFileName]"
      	 << " [-N Nevents (<0 for all)]"
      	 << " [-lja LargeJetAlgorithm]"
	 << " [-sja SubJetAlgorithm]"
      	 << " [-i infilepattern]"
      	 << " [-c chainname]"
      	 << " [-intype pico|{tree}]"
	 << " [-R radius]"
	 << " [-S subradius]"
	 << " [-pj PtJetMin PtJetMax]"
	 << " [-pc PtConsMin PtConsMax]"
      	 << " [-psc PtSubConsMin PtSubConsMax]"
	 << endl << endl
	 << "NOTE: Wildcarded file patterns should be in single quotes."
	 << endl;      
    return -1;
  }

  // Use the same constituent pT cut for jets and subjets
  float PtSubConsMin  = PtConsMin;
  float PtSubConsMax  = PtConsMax;  

  if ( PtJetMin<=0 ){
    cerr << "PtJetMin needs to be positive (0.001 will work)." << endl;
    return -1;
  }

  cout << " R = " << R << endl;
  cout << " SubR = " << SubR << endl;
  cout << " Original jet algorithm : "<< LargeJetAlgorithm << endl;
  cout << " Subjet algorithm       : "<< SubJetAlgorithm << endl;
  cout << " PtJetMin = " << PtJetMin << endl;
  cout << " PtJetMax = " << PtJetMax << endl;
  cout << " PtConsMin = " << PtConsMin << endl;
  cout << " PtConsMax = " << PtConsMax << endl;
  // cout << " PtSubConsMin = " << PtSubConsMin << endl;
  // cout << " PtSubConsMax = " << PtSubConsMax << endl;
  cout << " Reading tree named \""<< ChainName << "\" from " << InputName << endl;
  cout << " intype = " << ( intype==INTREE ? "INTREE" : "INPICO" ) << endl;
  cout << " Writing to " << OutFileName << endl;

  // For simple trees
  // -----------------
  TChain* Events = 0;
  TClonesArray* pFullEvent = 0;
  TClonesArray* pHardPartons = 0;
  TClonesArray* pHardPartonNames = 0;

  Events = new TChain(ChainName);
  Events->Add(InputName);

  if ( NEvents<0 ) NEvents = INT_MAX;
  if ( intype==INTREE ){
    assert ( Events->GetEntries()>0 && "Something went wrong loading events.");
    NEvents=min(NEvents,Events->GetEntries() );
    
    pFullEvent = new TClonesArray("TStarJetVector");
    Events->GetBranch("PythiaParticles")->SetAutoDelete(kFALSE);
    Events->SetBranchAddress("PythiaParticles", &pFullEvent);

    pHardPartons= new TClonesArray("TStarJetVector");
    Events->GetBranch("HardPartons")->SetAutoDelete(kFALSE);
    Events->SetBranchAddress("HardPartons", &pHardPartons);
    
    pHardPartonNames= new TClonesArray("TObjString");
    Events->GetBranch("HardPartonNames")->SetAutoDelete(kFALSE);
    Events->SetBranchAddress("HardPartonNames", &pHardPartonNames);

    // // TEMPORARY! Use MC only, no embedding
    // pFullEvent = new TClonesArray("TStarJetVector");
    // Events->GetBranch("McParticles")->SetAutoDelete(kFALSE);
    // Events->SetBranchAddress("McParticles", &pFullEvent);

  }
    
  // For picoDSTs
  // -------------
  TStarJetPicoReader* pReader=0;
  if ( intype==INPICO ){
    pReader = SetupReader( Events, TriggerName, SubjetParameters::RefMultCut );
    TStarJetPicoReader& reader = *pReader;
    reader.SetApplyFractionHadronicCorrection(kTRUE);
    reader.SetFractionHadronicCorrection(0.9999);
    reader.SetRejectTowerElectrons( kFALSE );
    //  reader.SetApplyFractionHadronicCorrection(kFALSE);
    
    // Run 11: Use centrality cut
    if ( InputName.Contains("NPE") ){
      TStarJetPicoEventCuts* evCuts = reader.GetEventCuts();    
      evCuts->SetReferenceCentralityCut (  6, 8 ); // 6,8 for 0-20%
    }
        
    // Explicitly choose bad tower list here
    TStarJetPicoTowerCuts* towerCuts = reader.GetTowerCuts();
    if ( InputName.Contains("NPE") ){
      towerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/badTowerList_y11.txt");
    } else {
      towerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/OrigY7MBBadTowers.txt");
      // towerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/Combined_y7_AuAu_Nick.txt");
      // towerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/Combined_y7_PP_Nick.txt");
    }
    // Add the following to y11 as well, once we're embedding!
    // towerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/Combined_y7_PP_Nick.txt");
    
    // DEBUG ONLY
    // towerCuts->AddBadTowers( "emptyBadTowerList.txt");
    
    // // DEBUG: KK: Reject bad phi strip  
    // towerCuts->SetPhiCut(0, -1.2);
    // TStarJetPicoTrackCuts* trackCuts = reader.GetTrackCuts();
    // trackCuts->SetPhiCut(0, -1.2);

    TStarJetPicoDefinitions::SetDebugLevel(0);
    reader.Init(NEvents);  
  }
  cout << " N = " << NEvents << endl;
 
  // Allow Embedding
  // ---------------
  TChain* EmbEvents = 0;
  TClonesArray* pEmbEvent = 0;
  Long64_t NEmbEvents=-1;
  bool Embedding= (EmbInputName !="");

  // TEMPORARY! Use MC only, no embedding
  // bool Embedding= (EmbInputName !="" && false);
  
  if ( Embedding ){
    EmbEvents = new TChain(EmbChainName);
    EmbEvents->Add(EmbInputName);
    
    if ( NEmbEvents<0 ) NEmbEvents = INT_MAX;

    if ( Embintype==INTREE ){
      assert ( EmbEvents->GetEntries()>0 && "Something went wrong loading the embedding data.");
      NEmbEvents=min(NEmbEvents,EmbEvents->GetEntries() );
      
      pEmbEvent = new TClonesArray("TStarJetVector");
      EmbEvents->GetBranch("McParticles")->SetAutoDelete(kFALSE);
      EmbEvents->SetBranchAddress("McParticles", &pEmbEvent);
    } else {
      cerr << "Embedding into PicoDSTs not yet enabled." << endl;
      return -1;
    }    
  }

  // Files and histograms
  // --------------------
  TFile* fout = new TFile( OutFileName, "RECREATE");
  
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);
  
  TH1D* JetPt   = new TH1D( "JetPt"  , "p_{T} of original jet;p_{T}^{Jet} [GeV/c]"  , 100,  0.0, 100  );
  TH1D* JetArea = new TH1D( "JetArea", "Area of original jet" , 100,  0.0, 1.0  );
  TH1D* Nsub    = new TH1D( "Nsub"   , "# of subjets"         , 100, -0.5, 99.5 );
  
  TH2D* SubVLeadPt     = new TH2D( "SubVLeadPt"     , "pT of sub-leading v. leading subjet;p_{T}^{lead} [GeV/c];p_{T}^{sub} [GeV/c];", 100, 0, 100, 100, 0, 100 );
  TH2D* AllVLeadPt     = new TH2D( "AllVLeadPt"     , "pT of all v. leading subjet;p_{T}^{lead} [GeV/c];p_{T}^{all} [GeV/c];", 100, 0, 100, 100, 0, 100 );
  TH2D* SubVLeadPtFrac = new TH2D( "SubVLeadPtFrac" , "pT fraction, sub-leading v. leading subjet;p_{T}^{lead} [GeV/c];p_{T}^{sub} [GeV/c];", 50, 0.01, 1.01, 50, 0.01, 1.01 );
  TH2D* AllVLeadPtFrac = new TH2D( "AllVLeadPtFrac" , "pT fraction, all v. leading subjet;p_{T}^{lead} [GeV/c];p_{T}^{all} [GeV/c];", 50, 0.01, 1.01, 50, 0.01, 1.01 );

  TH2D* SubVLeadArea     = new TH2D( "SubVLeadArea"     , "area of sub-leading v. leading subjet", 100, 0, 1, 100, 0, 1 );
  TH2D* AllVLeadArea     = new TH2D( "AllVLeadArea"     , "area of all v. leading subjet"        , 100, 0, 1, 100, 0, 1 );
  TH2D* SubVLeadAreaFrac = new TH2D( "SubVLeadAreaFrac" , "area fraction, sub-leading v. leading subjet", 100, 0, 1, 100, 0, 1 );
  TH2D* AllVLeadAreaFrac = new TH2D( "AllVLeadAreaFrac" , "area fraction, all v. leading subjet" , 100, 0, 1, 100, 0, 1 );

  TH2D* SubVLeadDeltaR   = new TH2D( "SubVLeadDeltaR"   , "#Delta R of sub-leading v. leading subjet", 100, 0, 1, 100, 0, 1 );
  TH2D* AllVLeadDeltaR   = new TH2D( "AllVLeadDeltaR"   , "#Delta R of all v. leading subjet"        , 100, 0, 1, 100, 0, 1 );
  
  // List of miscellaneous info
  // --------------------------
  TTree* info = new TTree("info", "Information");
  info->Branch("InputName"         , (void*)InputName.Data()     , "InputName/C" );
  info->Branch("ChainName"         , (void*)ChainName.Data()     , "ChainName/C" );
  if ( Embedding ){
    info->Branch("EmbInputName"         , (void*)EmbInputName.Data()     , "EmbInputName/C" );
    info->Branch("EmbChainName"         , (void*)EmbChainName.Data()     , "EmbChainName/C" );
  }
  info->Branch("R"                 , &R                          , "R/F" );
  info->Branch("SubR"              , &SubR                       , "SubR/F" );
  info->Branch("LargeJetAlgorithm" , (UInt_t*)&LargeJetAlgorithm , "LargeJetAlgorithm/i" );
  info->Branch("SubJetAlgorithm"   , (UInt_t*)&SubJetAlgorithm   , "SubJetAlgorithm/i" );
  info->Branch("PtJetMin"          , &PtJetMin                   , "PtJetMin/F" );
  info->Branch("PtJetMax"          , &PtJetMax                   , "PtJetMax/F" );
  info->Branch("EtaConsCut"        , &EtaConsCut                 , "EtaConsCut/F" );
  info->Branch("PtConsMin"         , &PtConsMin                  , "PtConsMin/F" );
  info->Branch("PtConsMax"         , &PtConsMax                  , "PtConsMax/F" );
  info->Branch("PtSubConsMin"      , &PtSubConsMin               , "PtSubConsMin/F" );
  info->Branch("PtSubConsMax"      , &PtSubConsMax               , "PtSubConsMax/F" );


  // Save results
  // ------------
  TTree* ResultTree=new TTree("ResultTree","Result Jets");

  TLorentzVector LeadingJet;
  TLorentzVector SubLeadingJet;
  ResultTree->Branch("LeadingJet",&LeadingJet);
  ResultTree->Branch("SubLeadingJet",&SubLeadingJet);

  TClonesArray LeadingSubjets( "TLorentzVector",1 ); 
  TClonesArray SubLeadingSubjets( "TLorentzVector",1 );
  ResultTree->Branch("LeadingSubjets", &LeadingSubjets );
  ResultTree->Branch("SubLeadingSubjets", &SubLeadingSubjets );

  vector<PseudoJet> vLeadingSubjets;
  vector<PseudoJet> vSubLeadingSubjets;

  TLorentzVector EmbLeadingJet;
  TLorentzVector EmbSubLeadingJet;
  TClonesArray EmbLeadingSubjets( "TLorentzVector",1 ); 
  TClonesArray EmbSubLeadingSubjets( "TLorentzVector",1 );

  if ( Embedding ){
    ResultTree->Branch("EmbLeadingSubjets", &EmbLeadingSubjets );
    ResultTree->Branch("EmbSubLeadingSubjets", &EmbSubLeadingSubjets );
    ResultTree->Branch("EmbLeadingJet",&EmbLeadingJet);
    ResultTree->Branch("EmbSubLeadingJet",&EmbSubLeadingJet);
  }  


  // // Initialize Analysis class
  // // -------------------------
  // DevSubjetAnalysis SubjetA( R, SubR,
  // 			     LargeJetAlgorithm, SubJetAlgorithm,
  // 			     PtJetMin, PtJetMax,
  // 			     EtaConsCut, PtConsMin, PtConsMax,
  // 			     JetPt, JetArea, Nsub, 
  // 			     SubVLeadPt,       AllVLeadPt,
  // 			     SubVLeadPtFrac,   AllVLeadPtFrac,
  // 			     SubVLeadArea,     AllVLeadArea,
  // 			     SubVLeadAreaFrac, AllVLeadAreaFrac,
  // 			     SubVLeadDeltaR,   AllVLeadDeltaR
  // 			  );
  
  // DevSubjetAnalysis *pEmbSubjetA=0;
  // if ( Embedding ){
  //   // not filling histos, just want the tree
  //   pEmbSubjetA = new DevSubjetAnalysis( R, SubR,
  // 					 LargeJetAlgorithm, SubJetAlgorithm,
  // 					 PtJetMin, PtJetMax,
  // 					 EtaConsCut, PtConsMin, PtConsMax
  // 					 );
  // }

  float EtaJetCut     = EtaConsCut - R;
  float EtaGhostCut   = EtaJetCut  + 2.0*R;
  GhostedAreaSpec AreaSpec = GhostedAreaSpec ( EtaGhostCut );
  AreaDefinition AreaDef = AreaDefinition (fastjet::active_area_explicit_ghosts, AreaSpec);
  // AreaDefinition AreaDef = AreaDefinition (fastjet::active_area, AreaSpec);

  JetDefinition JetDef    = JetDefinition( LargeJetAlgorithm, R    );
  // cambridge_algorithm
  JetDefinition SubJetDef = JetDefinition( SubJetAlgorithm  , SubR );

  // Jet candidate selectors
  // -----------------------
  Selector select_jet_eta     = SelectorAbsRapMax(EtaJetCut);
  Selector select_jet_pt      = SelectorPtRange( PtJetMin, PtJetMax );
  Selector select_jet         = select_jet_eta * select_jet_pt;


  // Go through events
  // -----------------
  cout << "Running analysis" << endl;
  Long64_t Ntot=0;
  Long64_t Naccepted=0;
  Long64_t Nrejected=0;
  vector<PseudoJet> particles;
  vector<PseudoJet> partons;
  TStarJetVector* sv;

  
  // ----------
  // Event Loop
  // ----------
  Long64_t evi =0;
  Long64_t Embevi =0; // For optional embedding
  bool done=false;
  while ( true ){    
    if ( evi> 20) break;
    switch (intype) {
      // =====================================================
    case INPICO :      
      if ( !pReader->NextEvent() ) {
	// cout << "Can't find a next event" << endl;
	done=true;
	break;
      }
      pReader->PrintStatus(10);
      // cout << pReader->GetOutputContainer()->GetEntries() << endl; 
      pFullEvent = pReader->GetOutputContainer()->GetArray();
      break;      
      // =====================================================
    case INTREE :
      if ( evi>= NEvents ) {
	done=true;
	break;
      }
      if ( !(evi%1000) ) cout << "Working on " << evi << " / " << NEvents << endl;
      Events->GetEntry(evi);
      ++evi;
      break;
      // =====================================================
    default:
      cerr << "Unknown intype " << intype << endl;
      return(-1);
    }
    if ( done ) break;

    // Fill particle container
    // -----------------------
    particles.clear();
    for ( int i=0 ; i<pFullEvent->GetEntries() ; ++i ){
      sv = (TStarJetVector*) pFullEvent->At(i);
      // Ensure kinematic similarity
      if ( sv->Pt()< PtConsMin && sv->Pt()< PtSubConsMin ) continue;
      if ( fabs( sv->Eta() )>EtaConsCut ) continue;
      particles.push_back( PseudoJet (*sv ) );
    }
    if ( particles.size()==0 ) continue;
    
    // For pythia, fill leading parton container
    // -----------------------------------------
    partons.clear();

    if ( pHardPartons ){
      for ( int i=0 ; i<pHardPartons->GetEntries() ; ++i ){
	sv = (TStarJetVector*) pHardPartons->At(i);
	PseudoJet pj = PseudoJet (*sv );
	
	// flavor info
	TString& s = ((TObjString*)(pHardPartonNames->At(i)))->String();
	int qcharge=-999;
	if ( s=="g" ) qcharge = 0;
	
	if ( s(0)=='u' || s(0)=='c' || s(0)=='t' ) qcharge  = 2;
	if ( s(0)=='d' || s(0)=='s' || s(0)=='b' ) qcharge = -1;
	if ( s.Contains("bar") ) qcharge*=-1;
	
	if ( abs ( qcharge ) >3 ) cout<< s << endl;
	
	pj.set_user_info ( new JetAnalysisUserInfo( qcharge ) );
	partons.push_back( pj );
      }
    }

    
    // Run analysis
    // ------------
    // JetAnalyzer JA( particles, JetDef ); // WITHOUT background subtraction
    // vector<PseudoJet> JAResult = sorted_by_pt( select_jet ( JA.inclusive_jets() ) );
    JetAnalyzer JA( particles, JetDef, AreaDef ); // WITH background subtraction
    Subtractor* BackgroundSubtractor =  JA.GetBackgroundSubtractor();
    vector<PseudoJet> JAResult = sorted_by_pt( select_jet ( (*BackgroundSubtractor)( JA.inclusive_jets() ) ) );

    if ( JAResult.size()==0 ) {
      continue;
    }

    if ( JAResult[0].pt() < 20 ) continue;

    // Now get subjets
    // ---------------
    vLeadingSubjets.clear();
    vSubLeadingSubjets.clear();
    for (unsigned int i = 0; i < JAResult.size(); i++) {
      contrib::Recluster recluster ( SubJetAlgorithm, SubR, false);       // "false": keep all pieces
      PseudoJet rec_jet = recluster(JAResult.at(i));
      cout << "Reclustering with: " << recluster.description() << endl
      	   << "  " << rec_jet << endl;
      // vector<PseudoJet> pieces = rec_jet.pieces();
      // cout << "   subjets: " << endl;
      // for (unsigned int i=0;i<pieces.size();i++)
      // 	if ( !pieces[i].is_pure_ghost() ) {
      // 	  cout << "    " << pieces[i] << endl;
      // 	}

      Selector SelectorNoGhosts = !SelectorIsPureGhost();
      vector<PseudoJet> subjets = SelectorNoGhosts ( rec_jet.pieces() );
      cout << "   subjets: " << endl;
      for (unsigned int i=0;i<subjets.size();i++){
	cout << "    " << subjets[i] << endl;
      }
      cout << endl;      

      // Treat leading and sub-leading jets specially
      if ( i==0 )     vLeadingSubjets=subjets;
      if ( i==1 )     vSubLeadingSubjets=subjets;
    }
    if ( evi> 20) break;

    // // Run analysis
    // // ------------
    // int ret = SubjetA.AnalyzeAndFill( particles, &partons );

    // Extract more info
    // -----------------
    LeadingSubjets.Clear();
    SubLeadingSubjets.Clear();
    LeadingJet    = TLorentzVector(0,0,0,0);
    SubLeadingJet = TLorentzVector(0,0,0,0);

    // Save if we found _something_
    if ( vLeadingSubjets.size()>0 ){
      if ( JAResult.size() >0 ){
	LeadingJet    = MakeTLorentzVector( JAResult.at(0) );
      }
      if ( JAResult.size()>1) {
	SubLeadingJet = MakeTLorentzVector( JAResult.at(1) );
      }
      
      for (int i=0; i< vLeadingSubjets.size() ; ++i ){
	new ( LeadingSubjets[i] ) TLorentzVector (MakeTLorentzVector ( vLeadingSubjets.at(i)));
      }
      for (int i=0; i< vSubLeadingSubjets.size() ; ++i ){
	new ( SubLeadingSubjets[i] ) TLorentzVector (MakeTLorentzVector ( vSubLeadingSubjets.at(i)));
      }
      
      // Embedding?
      // ----------
      if ( Embedding ){
	cout << " -------------- EMBEDDED ----------------- " << endl;
	
	// DevSubjetAnalysis& EmbSubjetA = *pEmbSubjetA;  
	switch (Embintype) {
	  // =====================================================
	case INPICO :      
	  // if ( !pReader->NextEvent() ) {
	  //   // cout << "Can't find a next event" << endl;
	  //   done=true;
	  //   break;
	  // }
	  // pReader->PrintStatus(10);
	  // // cout << pReader->GetOutputContainer()->GetEntries() << endl; 
    	  // pFullEvent = pReader->GetOutputContainer()->GetArray();
	  cerr << "Embedding into PicoDSTs not yet enabled." << endl;
	  return -1;
	  break;      
	  // =====================================================
	case INTREE :
	  if ( Embevi>= NEmbEvents ) {
	    // qnd: just cycle:
	    Embevi=0;
	    // done=true;
	    break;
	  }
	  EmbEvents->GetEntry(Embevi);
	  ++Embevi;
	  break;
	  // =====================================================
	default:
	  cerr << "Unknown intype " << intype << endl;
	  return(-1);
	}
	if ( done ) break;
	
	EmbEvents->GetEntry(Embevi);
	for ( int i=0 ; i<pEmbEvent->GetEntries() ; ++i ){
	  sv = (TStarJetVector*) pEmbEvent->At(i);
	  // Ensure kinematic similarity
	  if ( sv->Pt()< PtConsMin && sv->Pt()< PtSubConsMin ) continue;
	  if ( fabs( sv->Eta() )>EtaConsCut ) continue;
	  particles.push_back( PseudoJet (*sv ) );
	}
	
	// Run analysis
	// ------------
	cout << particles.size() << endl;
	JetAnalyzer EmbJA( particles, JetDef, AreaDef ); // WITH background subtraction
	Subtractor* EmbBackgroundSubtractor =  EmbJA.GetBackgroundSubtractor();
	vector<PseudoJet> EmbJAResult = sorted_by_pt( select_jet ( (*EmbBackgroundSubtractor)( EmbJA.inclusive_jets() ) ) );
	
	if ( EmbJAResult.size()==0 ) {
	  continue;
	}
	
	if ( EmbJAResult[0].pt() < 20 ) continue;
	
	vLeadingSubjets.clear();
	vSubLeadingSubjets.clear();
	// Now get subjets
	// ---------------
	for (unsigned int i = 0; i < EmbJAResult.size(); i++) {
	  contrib::Recluster recluster ( SubJetAlgorithm, SubR, false);       // "false": keep all pieces
	  PseudoJet rec_jet = recluster(EmbJAResult.at(i));

	  Selector SelectorNoGhosts = !SelectorIsPureGhost();
	  vector<PseudoJet> subjets = SelectorNoGhosts ( rec_jet.pieces() );
	  cout << "   subjets: " << endl;
	  for (unsigned int i=0;i<subjets.size();i++){
	    cout << "    " << subjets[i] << endl;
	  }
	  cout << endl;      

	  // Treat leading and sub-leading jets specially
	  if ( i==0 )     vLeadingSubjets=subjets;
	  if ( i==1 )     vSubLeadingSubjets=subjets;
	}
	
	EmbLeadingJet    = TLorentzVector(0,0,0,0);
	EmbSubLeadingJet = TLorentzVector(0,0,0,0);
	EmbLeadingSubjets.Clear();
	EmbSubLeadingSubjets.Clear();
	// Save if we found _something_
	if ( vLeadingSubjets.size()>0 ){
	  if ( JAResult.size() >0 ){
	    EmbLeadingJet    = MakeTLorentzVector( EmbJAResult.at(0) );
	  }
	  if ( JAResult.size()>1) {
	    EmbSubLeadingJet = MakeTLorentzVector( EmbJAResult.at(1) );
	  }
	  
	  for (int i=0; i< vLeadingSubjets.size() ; ++i ){
	    new ( EmbLeadingSubjets[i] ) TLorentzVector (MakeTLorentzVector ( vLeadingSubjets.at(i)));
	  }
	  for (int i=0; i< vSubLeadingSubjets.size() ; ++i ){
	    new ( EmbSubLeadingSubjets[i] ) TLorentzVector (MakeTLorentzVector ( vSubLeadingSubjets.at(i)));
	  }
	}
      }
      
      // Fill result tree
      ResultTree->Fill();   
    }

    // // Update counters
    // // ---------------
    // Ntot++;
    // switch ( ret ){
    // case 0 : Naccepted++;
    //   break;
    // case -1 :
    //   Nrejected++;
    //   break;
    // default :
    //   cerr << "Unknown return value " << ret << endl;
    //   return ret;
    // }
  }  
  
  info->Branch("Ntot"      , &Ntot      , "Ntot/L" );
  info->Branch("Naccepted" , &Naccepted , "Naccepted/L" );
  info->Branch("Nrejected" , &Nrejected , "Nrejected/L" );
  info->Fill();
  
  fout->Write();

  cout << "Done." << endl;
  return 0;
}
//----------------------------------------------------------------------
/// overloaded jet info output
ostream & operator<<(ostream & ostr, const PseudoJet & jet) {
  if (jet == 0) {
    ostr << " 0 ";
  } else {
    ostr << " pt = " << jet.pt()
         << " m = " << jet.m()
         << " y = " << jet.rap()
         << " phi = " << jet.phi()
         << " ClusSeq = " << (jet.has_associated_cs() ? "yes" : "no");
  }
  return ostr;
}
