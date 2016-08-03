/** @file Groom.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Subjet Analysis for pythia and/or real events.
    @details Perform Subjet analysis on a chain of LorentzVectors or TStarJetPicoTrees.
    @date Sep 17, 2015
*/

#include "DevSubjetAnalysis.hh"
#include "TStarJetVectorJet.h"

#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <TChain.h>
#include <TFile.h>
#include <TBranch.h>
#include <TMath.h>
#include <TRandom.h>
#include <TParameter.h>
#include "TString.h"
#include "TObjString.h"


#include <vector>
#include <algorithm>

#include <cmath>
#include <climits>

#include "fastjet/contrib/Recluster.hh"
#include "fastjet/contrib/SoftDrop.hh"

using namespace std;
using namespace fastjet;
using namespace contrib;

// For sorting with a different key
typedef pair<PseudoJet,double> PseudoJetPt;
struct PseudoJetPtCmp {
  bool operator()( PseudoJetPt const & a, PseudoJetPt const & b) { 
    return a.second < b.second ;
  }
};

ostream & operator<<(ostream &, const PseudoJet &);

void InitializeReader(  TStarJetPicoReader* pReader, const TString InputName, const Long64_t NEvents, const int PicoDebugLevel=0 );
// DEBUG
void decluster (PseudoJet j);

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
  
  enum INTYPE{ MCTREE, INTREE, INPICO };

  // Defaults
  // --------

  // Pythia
  // ------
  TString InputName = "Data/RhicPythia/RhicPythiaOnly_10_ptHat=20_23.root";
  // TString InputName = "Data/LhcPythia/LhcPythiaAndMc_0_ptHat=80_100.root";
  INTYPE intype = INTREE;
  TString ChainName = "tree";

  // // pico
  // // ------
  // TString InputName = "Data/ppHT/*.root";
  // INTYPE intype = INPICO;
  // TString ChainName = "JetTree";

  // ----- Common ----
  // TString OutFileName = "Results/TmpSubjetResult.root";
  TString OutFileName = "Results/SomeGroomResult.root";

  float R    = 0.4;
  double z_cut = 0.10;
  double beta  = 0.0;

  JetAlgorithm LargeJetAlgorithm = fastjet::antikt_algorithm;
  // JetAlgorithm LargeJetAlgorithm = fastjet::cambridge_algorithm;
  // JetAlgorithm SubJetAlgorithm   = fastjet::antikt_algorithm;
  
  // Alternative reclustering to Cambridge/Aachen (at our own risk)
  // --------------------------------------------------------------
  // JetAlgorithm ReclusterJetAlgorithm = fastjet::cambridge_algorithm;
  bool CustomRecluster=false;
  JetAlgorithm ReclusterJetAlgorithm;
  JetDefinition ReclusterJetDef;
  Recluster * recluster = 0; 

  float PtJetMin      = 20;
  float PtJetMax      = 2000;
  // float EtaConsCut    = 3;
  float EtaConsCut    = 1;
  float PtConsMin     = 0.2;
  float PtConsMax     = 10000;

  TString TriggerName = "HT";
  TString EmbTriggerName = "MB";
    
  // Allow Embedding
  // ---------------
  // TString EmbInputName = InputName;
  TString EmbInputName = "";
  INTYPE Embintype = INTREE;
  TString EmbChainName = "tree";
  int nMix=1;

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
    } else if ( arg == "-lja" ){     
      if (++parg==arguments.end() ){ argsokay=false; break; }
      LargeJetAlgorithm = AlgoFromString ( *parg);
    } else if ( arg == "-rcja" ){
      if (++parg==arguments.end() ){ argsokay=false; break; }
      CustomRecluster=true;
      ReclusterJetAlgorithm = AlgoFromString ( *parg);
      ReclusterJetDef = JetDefinition( ReclusterJetAlgorithm, 2*R );
      recluster = new Recluster( ReclusterJetDef, 2*R );
    } else if ( arg == "-b" ){
      if (++parg==arguments.end() ){ argsokay=false; break; }
      beta = atof((parg)->data());
    } else if ( arg == "-z" ){
      if (++parg==arguments.end() ){ argsokay=false; break; }
      z_cut = atof((parg)->data());
    } else if ( arg == "-pj" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtJetMin = atof((parg)->data());
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtJetMax = atof((parg)->data());
    } else if ( arg == "-ec" ){      
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      EtaConsCut = atof((parg)->data());      
    } else if ( arg == "-pc" ){      
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtConsMin = atof((parg)->data());      
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtConsMax = atof((parg)->data());
    } else if ( arg == "-o" ){     
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      OutFileName=*parg;
    } else if ( arg == "-i" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      InputName=*parg;
    } else if ( arg == "-embi" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      EmbInputName=*parg;
      // Add a shortcut
      if (EmbInputName == "FAKERHIC" ) EmbInputName = "Data/FakeAuAu20_*root";
    } else if ( arg == "-c" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      ChainName=*parg;
    } else if ( arg == "-trig" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      TriggerName=*parg;
    } else if ( arg == "-embtrig" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      EmbTriggerName=*parg;
    } else if ( arg == "-embc" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      EmbChainName=*parg;
    } else if ( arg == "-nmix" ){      
      if (++parg==arguments.end() ){ argsokay=false; break; }
      nMix=atoi(parg->data());
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
    } else if ( arg == "-embintype" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      if ( *parg == "pico" ){
	Embintype = INPICO;
	continue;
      }
      if ( *parg == "mctree" ){
	Embintype = MCTREE;
	continue;
      }
      if ( *parg == "tree" ){
	Embintype = INTREE;
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
	 << " [-rcja ReclusteringJetAlgorithm]"
      	 << " [-i infilepattern]"
      	 << " [-c chainname]"
      	 << " [-intype pico|tree|mctree]"
	 << " [-trig trigger name (e.g. HT)]"
      	 << " [-embi embedding infilepattern]"
      	 << " [-embc embedding chainname]"
      	 << " [-embintype embedding pico|tree|mctree]"
	 << " [-nmix nMix]"
	 << " [-R radius]"
	 << " [-b beta]"
	 << " [-z z_cut]"
	 << " [-pj PtJetMin PtJetMax]"
	 << " [-ec EtaConsCut]"
	 << " [-pc PtConsMin PtConsMax]"
      	 << " [-psc PtSubConsMin PtSubConsMax]"
	 << endl << endl
	 << "NOTE: Wildcarded file patterns should be in single quotes."
	 << endl;      
    return -1;
  }
  
  // Repeat on subjets?
  bool Recursive = InputName.Contains("Pythia");

  // Use the same constituent pT cut for jets and subjets
  float PtSubConsMin  = PtConsMin;
  float PtSubConsMax  = PtConsMax;  

  if ( PtJetMin<=0 ){
    cerr << "PtJetMin needs to be positive (0.001 will work)." << endl;
    return -1;
  }

  // For simple trees
  // -----------------
  TChain* Events = 0;
  TClonesArray* pFullEvent = 0;
  TClonesArray* pHardPartons = 0;
  TClonesArray* pHardPartonNames = 0;

  Events = new TChain(ChainName);
  Events->Add(InputName);

  // Note that the following are NOT unique across MC trees
  // My fault, will try to fix with a quick and dirty trick:
  // runid (and eventid) are below 1M (itself not exactly optimal)
  // So we'll just hash the data file name and add it to the runid,
  // making sure that it stays in the right range
  int eventid;
  int runid;

  // Sigh. Some are unsigned, some aren't. Ugly kluge.
  unsigned int ueventid;
  unsigned int urunid;

  if ( NEvents<0 ) NEvents = INT_MAX;
  if ( intype==INTREE || intype==MCTREE ){
    assert ( Events->GetEntries()>0 && "Something went wrong loading events.");
    NEvents=min(NEvents,Events->GetEntries() );
    
    if ( intype==MCTREE ){
      pFullEvent = new TClonesArray("TStarJetVector");
      Events->GetBranch("PythiaParticles")->SetAutoDelete(kFALSE);
      Events->SetBranchAddress("PythiaParticles", &pFullEvent);

      Events->SetBranchAddress("eventid", &eventid);
      Events->SetBranchAddress("runid", &runid);
    }
    if ( intype==INTREE ){
      pFullEvent = new TClonesArray("TStarJetVector");
      Events->GetBranch("FullEvent")->SetAutoDelete(kFALSE);
      Events->SetBranchAddress("FullEvent", &pFullEvent);

      //TLorentzVector
      Events->SetBranchAddress("eventid", &ueventid);
      Events->SetBranchAddress("runid", &urunid);
    }

    // pFullEvent = new TClonesArray("TStarJetVector");
    // Events->GetBranch("PythiaParticles")->SetAutoDelete(kFALSE);
    // Events->SetBranchAddress("PythiaParticles", &pFullEvent);

    if ( intype==MCTREE ){
      pHardPartons= new TClonesArray("TStarJetVector");
      Events->GetBranch("HardPartons")->SetAutoDelete(kFALSE);
      Events->SetBranchAddress("HardPartons", &pHardPartons);

      pHardPartonNames= new TClonesArray("TObjString");
      Events->GetBranch("HardPartonNames")->SetAutoDelete(kFALSE);
      Events->SetBranchAddress("HardPartonNames", &pHardPartonNames);
    }
    
  }

    
  // For picoDSTs
  // -------------
  TStarJetPicoReader* pReader=0;
  int PicoDebugLevel=0;
  if ( intype==INPICO ){
    pReader = SetupReader( Events, TriggerName, SubjetParameters::RefMultCut );
    InitializeReader(  pReader, InputName, NEvents, PicoDebugLevel );
  }
  // Allow Embedding
  // ---------------
  TChain* EmbEvents = 0;
  TClonesArray* pEmbEvent = 0;
  Long64_t NEmbEvents=-1;
  bool Embedding = (EmbInputName !="" && EmbInputName!="NONE");
  Long64_t Embevi =0; // Starting point will be changed!  

  int Embeventid;
  int Embrunid;
  unsigned int uEmbeventid;
  unsigned int uEmbrunid;

  TStarJetPicoReader* pEmbReader=0;
  if ( Embedding ){
    EmbEvents = new TChain(EmbChainName);
    EmbEvents->Add(EmbInputName);
    assert ( EmbEvents->GetEntries()>0 && "Something went wrong loading the embedding data.");
    if ( NEmbEvents<0 ) NEmbEvents = EmbEvents->GetEntries();
    NEmbEvents=min(NEmbEvents,EmbEvents->GetEntries() );
    gRandom->SetSeed(0);
    Embevi = gRandom->Integer(NEmbEvents); // Start at a random point

    if ( Embintype==MCTREE ){      
      EmbEvents->SetBranchAddress("eventid", &Embeventid);
      EmbEvents->SetBranchAddress("runid", &Embrunid);            

      pEmbEvent = new TClonesArray("TStarJetVector");
      EmbEvents->GetBranch("McParticles")->SetAutoDelete(kFALSE);
      EmbEvents->SetBranchAddress("McParticles", &pEmbEvent);
    } else if ( Embintype==INTREE ){      
      EmbEvents->SetBranchAddress("eventid", &uEmbeventid);
      EmbEvents->SetBranchAddress("runid", &uEmbrunid);            

      pEmbEvent = new TClonesArray("TStarJetVector");
      EmbEvents->GetBranch("FullEvent")->SetAutoDelete(kFALSE);
      EmbEvents->SetBranchAddress("FullEvent", &pEmbEvent);
    } else if ( Embintype==INPICO ){
      // cerr << "Embedding into PicoDSTs not yet enabled." << endl;
      // return -1;
      
      pEmbReader = SetupReader( EmbEvents, EmbTriggerName, SubjetParameters::RefMultCut );
      InitializeReader(  pEmbReader, EmbInputName, NEmbEvents, PicoDebugLevel );
      // TStarJetPicoReader& Embreader = *pEmbReader;
      // Embreader.SetApplyFractionHadronicCorrection(kTRUE);
      // Embreader.SetFractionHadronicCorrection(0.9999);
      // Embreader.SetRejectTowerElectrons( kFALSE );
      // //  Embreader.SetApplyFractionHadronicCorrection(kFALSE);
    
      // // Run 11: Use centrality cut
      // if ( EmbInputName.Contains("NPE") ){
      // 	TStarJetPicoEventCuts* EmbevCuts = Embreader.GetEventCuts();    
      // 	EmbevCuts->SetReferenceCentralityCut (  6, 8 ); // 6,8 for 0-20%
      // }
      
      // // Explicitly choose bad tower list here
      // TStarJetPicoTowerCuts* EmbtowerCuts = Embreader.GetTowerCuts();
      // if ( EmbInputName.Contains("NPE") ){
      // 	EmbtowerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/badTowerList_y11.txt");
      // } else {
      // 	EmbtowerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/OrigY7MBBadTowers.txt");
      // 	// EmbtowerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/Combined_y7_AuAu_Nick.txt");
      // 	// EmbtowerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/Combined_y7_PP_Nick.txt");
      // }
      // // Add the following to y11 as well, once we're embedding!
      // // EmbtowerCuts->AddBadTowers( TString( getenv("STARPICOPATH" )) + "/Combined_y7_PP_Nick.txt");
    
      // // DEBUG ONLY
      // // EmbtowerCuts->AddBadTowers( "emptyBadTowerList.txt");
    
      // // // DEBUG: KK: Reject bad phi strip  
      // // towerCuts->SetPhiCut(0, -1.2);
      // // TStarJetPicoTrackCuts* trackCuts = reader.GetTrackCuts();
      // // trackCuts->SetPhiCut(0, -1.2);
      // Embreader.Init(INT_MAX);

      pEmbReader->ReadEvent( Embevi );
    } else {
      cerr << "Unknown embedding type." << endl;
      return -1;
    }
    cout << "Starting Embedding with event number " << Embevi << endl;
  }

  // Files and histograms
  // --------------------
  TFile* fout = new TFile( OutFileName, "RECREATE");
  
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);
  TH3::SetDefaultSumw2(true);
    
  TH1D* hzg      = new TH1D( "hzg", "z_{g}"              , 100*(R+0.1),  0.0, (R+0.1)  );
  TH1D* hEmbzg   = new TH1D( "hEmbzg", "z_{g}, embedded" , 100*(R+0.1),  0.0, (R+0.1)  );

  TH3D* ptphieta = new TH3D("ptphieta","",500, 0.2, 50.2, 100, 0, TMath::TwoPi(), 300, -3, 3);
  
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
  info->Branch("beta"              , &beta                       , "beta/F" );
  info->Branch("z_cut"             , &z_cut                      , "z_cut/D" );
  info->Branch("LargeJetAlgorithm" , (UInt_t*)&LargeJetAlgorithm , "LargeJetAlgorithm/i" );
  if ( CustomRecluster )
    info->Branch("ReclusterJetAlgorithm" , (UInt_t*)&ReclusterJetAlgorithm , "ReclusterJetAlgorithm/i" );
  info->Branch("PtJetMin"          , &PtJetMin                   , "PtJetMin/F" );
  info->Branch("PtJetMax"          , &PtJetMax                   , "PtJetMax/F" );
  info->Branch("EtaConsCut"        , &EtaConsCut                 , "EtaConsCut/F" );
  info->Branch("PtConsMin"         , &PtConsMin                  , "PtConsMin/F" );
  info->Branch("PtConsMax"         , &PtConsMax                  , "PtConsMax/F" );


  // Save results
  // ------------
  TTree* ResultTree=new TTree("ResultTree","Result Jets");
  
  TClonesArray HardPartons( "TStarJetVector" );
  ResultTree->Branch("HardPartons", &HardPartons );
  TClonesArray Jets( "TStarJetVectorJet" );
  ResultTree->Branch("Jets", &Jets );
  TClonesArray GroomedJets( "TStarJetVectorJet" ); 
  ResultTree->Branch("GroomedJets", &GroomedJets );
  TClonesArray sj1( "TStarJetVectorJet" );
  ResultTree->Branch("sj1", &sj1 );
  TClonesArray sj2( "TStarJetVectorJet" );
  ResultTree->Branch("sj2", &sj2 );
  double refmult;
  ResultTree->Branch("refmult",&refmult, "refmult/d");

  
  
  int njets=0;
  ResultTree->Branch("njets",   &njets, "njets/I" );
  double zg[1000];
  ResultTree->Branch("zg",       zg, "zg[njets]/D" );
  double delta_R[1000];
  ResultTree->Branch("delta_R",  delta_R, "delta_R[njets]/D" );
  double mu[1000];
  ResultTree->Branch("mu",       mu, "mu[njets]/D" );
  double rho=-1;
  ResultTree->Branch("rho",      &rho, "rho/D" );

  double zg1[1000];
  double zg2[1000];
  if ( Recursive){
    ResultTree->Branch("zg1",       zg1, "zg1[njets]/D" );
    ResultTree->Branch("zg2",       zg2, "zg2[njets]/D" );
  }

  TClonesArray EmbJets( "TStarJetVectorJet" ); 
  ResultTree->Branch("EmbJets", &EmbJets );
  TClonesArray EmbGroomedJets( "TStarJetVectorJet" ); 
  ResultTree->Branch("EmbGroomedJets", &EmbGroomedJets );
  TClonesArray Embsj1( "TStarJetVectorJet" );
  ResultTree->Branch("Embsj1", &Embsj1 );
  TClonesArray Embsj2( "TStarJetVectorJet" );
  ResultTree->Branch("Embsj2", &Embsj2 );
  double Embrefmult;
  ResultTree->Branch("Embrefmult",&Embrefmult, "Embrefmult/d");
  
  int Embnjets=0;
  ResultTree->Branch("Embnjets",   &Embnjets, "Embnjets/I" );
  double Embzg[1000];
  ResultTree->Branch("Embzg",       Embzg, "Embzg[njets]/D" );
  double Embdelta_R[1000];
  ResultTree->Branch("Embdelta_R",  Embdelta_R, "Embdelta_R[njets]/D" );
  double Embmu[1000];
  ResultTree->Branch("Embmu",       Embmu, "Embmu[njets]/D" );

  double Embzg1[1000];
  double Embzg2[1000];
  if ( Recursive){
    ResultTree->Branch("Embzg1",       Embzg1, "Embzg1[njets]/D" );
    ResultTree->Branch("Embzg2",       Embzg2, "Embzg2[njets]/D" );
  }

  double weight=1;
  ResultTree->Branch("weight",      &weight, "weight/D" );

  double Embrho=-1;
  ResultTree->Branch("Embrho",      &Embrho, "Embrho/D" );

  // Give each event a unique ID to compare event by event with different runs
  ResultTree->Branch("eventid",&eventid, "eventid/I");
  ResultTree->Branch("runid",&runid, "runid/I");
  // And also contracted versions to index on both
  ULong64_t runevent=0;
  ResultTree->Branch("runevent",&runevent, "runevent/l");

  ULong64_t Embrunevent=0;
  // Give each event a unique ID to compare event by event with different runs
  if ( Embedding ){
    ResultTree->Branch("Embeventid",&Embeventid, "Embeventid/I");
    ResultTree->Branch("Embrunid",&Embrunid, "Embrunid/I");
    ResultTree->Branch("Embrunevent",&Embrunevent, "Embrunevent/l");
  }

  
  // Parameters
  // ----------
  float EtaJetCut     = EtaConsCut - R;
  float EtaGhostCut   = EtaJetCut  + 2.0*R;
  int GhostRepeat = 1;
  float GhostArea = 0.005;
  GhostedAreaSpec AreaSpec = GhostedAreaSpec ( EtaGhostCut, GhostRepeat, GhostArea );
  AreaDefinition AreaDef = AreaDefinition (fastjet::active_area_explicit_ghosts, AreaSpec);
  // AreaDefinition AreaDef = AreaDefinition (fastjet::active_area, AreaSpec);

  JetDefinition JetDef    = JetDefinition( LargeJetAlgorithm, R    );

  // Jet candidate selectors
  // -----------------------
  Selector select_jet_eta     = SelectorAbsRapMax(EtaJetCut);
  Selector select_jet_pt      = SelectorPtRange( PtJetMin, PtJetMax );
  Selector select_jet         = select_jet_eta * select_jet_pt;

  
  cout << " R = " << R << endl;
  cout << " beta = " << beta << endl;
  cout << " z_cut = " << z_cut << endl;
  cout << " Original jet algorithm : "<< LargeJetAlgorithm << endl;
  if ( CustomRecluster )
    cout << " Recluster jet algorithm : "<< ReclusterJetAlgorithm << endl;
  cout << " PtJetMin = " << PtJetMin << endl;
  cout << " PtJetMax = " << PtJetMax << endl;
  cout << " PtConsMin = " << PtConsMin << endl;
  cout << " PtConsMax = " << PtConsMax << endl;
  cout << " Constituent eta cut = " << EtaConsCut << endl;
  cout << " Jet eta cut = " << EtaJetCut << endl;
  cout << " Ghosts out to eta = " << EtaGhostCut << endl;
  cout << " Reading tree named \""<< ChainName << "\" from " << InputName << endl;
  cout << " intype = " << ( intype==INTREE ? "INTREE" : intype==MCTREE ? "MCTREE" : "INPICO" ) << endl;
  cout << " Writing to " << OutFileName << endl;
  cout << " N = " << NEvents << endl;
  if ( Embedding ) {
    cout << " Embedding into tree named \""<< EmbChainName << "\" from " << EmbInputName << endl;
    cout << " Embintype = " << ( Embintype==INTREE ? "INTREE" : Embintype==MCTREE ? "MCTREE" : "INPICO" ) << endl;
    cout << " nMix = " << nMix << endl;
  }
  cout << " ----" << endl;
  cout << " Writing to " << OutFileName << endl;
  cout << " ----------------------------" << endl;

  // Go through events
  // -----------------
  cout << "Running analysis" << endl;
  Long64_t Ntot=0;
  Long64_t Naccepted=0;
  Long64_t Nrejected=0;
  vector<PseudoJet> particles;
  vector<PseudoJet> partons;
  TStarJetVector* sv;

  // For matching
  // ------------
  fastjet::Selector SelectClose = fastjet::SelectorCircle( R );

  // ----------
  // Event Loop
  // ----------
  Long64_t evi =0;
  bool done=false;
  TString cname = "";
  UInt_t filehash = 0;

  while ( true ){    
    HardPartons.Clear();
    Jets.Clear();
    GroomedJets.Clear();
    sj1.Clear();
    sj2.Clear();
    rho=-1;
    EmbJets.Clear();
    EmbGroomedJets.Clear();
    Embsj1.Clear();
    Embsj2.Clear();
    Embrho=-1;
    refmult=0;


    TStarJetPicoEventHeader* header=0;
    switch (intype) {
      // =====================================================
    case INPICO :      
      if ( !pReader->NextEvent() ) {
	// cout << "Can't find a next event" << endl;
	done=true;
	break;
      }
      // cout << "hello" << endl;
      pReader->PrintStatus(10);
      // cout << pReader->GetOutputContainer()->GetEntries() << endl; 
      pFullEvent = pReader->GetOutputContainer()->GetArray();

      header = pReader->GetEvent()->GetHeader();
      eventid = header->GetEventId();
      runid   = header->GetRunId();
      refmult=header->GetProperReferenceMultiplicity();
      
      break;      
      // =====================================================
    case INTREE :
    case MCTREE :
      if ( evi>= NEvents ) {
	done=true;
	break;
      }
      if ( !(evi%200) ) cout << "Working on " << evi << " / " << NEvents << endl;
      Events->GetEntry(evi);
      cname = Events->GetCurrentFile()->GetName();
      if (intype == MCTREE ){
	filehash = cname.Hash();
	while ( filehash > INT_MAX - 100000 ) filehash /= 10;
	if ( filehash < 1000000 ) filehash += 1000001;
	runid += filehash;
      }
      if (intype == INTREE ){
	runid = urunid;
	eventid = ueventid;
      }

      ++evi;
      break;
      // =====================================================
    default:
      cerr << "Unknown intype " << intype << endl;
      return(-1);
    }
    if ( done ) break;

    // FIXME: May (will) not work as intended unless both inputs are picoDSTs!
    runevent = ULong64_t(runid)*10000000LL + eventid;
    
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
    
    // For pythia, use cross section as weight
    // ---------------------------------------
    weight=1;
    if ( TParameter<double>* sigmaGen=(TParameter<double>*) Events->GetCurrentFile()->Get("sigmaGen") ){
      weight=sigmaGen->GetVal();
    }

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

	// Save them too
	// Update info here, may not be complete in original tree
	sv->SetType(TStarJetVector::_PYTHIA);
	sv->SetCharge(qcharge ); // Less than ideal, we're revamping charge to mean quark charge
	sv->SetMatch(-1); // Necessary for older TStarJetVectors
	new ( HardPartons[i] ) 	TStarJetVector (*sv);
      }
    }

    
    // Run analysis
    // ------------
    // JetAnalyzer JA( particles, JetDef ); // WITHOUT background subtraction
    // vector<PseudoJet> JAResult = sorted_by_pt( select_jet ( JA.inclusive_jets() ) );
    JetAnalyzer JA( particles, JetDef, AreaDef, select_jet_eta * (!fastjet::SelectorNHardest(2)) ) ;
    Subtractor& BackgroundSubtractor =  *JA.GetBackgroundSubtractor();

    // vector<PseudoJet> JAResult = sorted_by_pt( select_jet ( BackgroundSubtractor( JA.inclusive_jets() ) ) );
    // Can't subtract the background before SoftDrop
    // But we can discard jets that get eliminated by BG subtraction
    // and get the subtracted order right
    vector<PseudoJet> JAResult = sorted_by_pt( select_jet ( JA.inclusive_jets() ) );
    vector<PseudoJetPt> ResortedJAResult;
    for (unsigned ijet = 0; ijet < JAResult.size(); ijet++) {
      PseudoJet j = BackgroundSubtractor( JAResult.at(ijet) );
      if ( j.pt() > 0 ){
	ResortedJAResult.push_back ( PseudoJetPt( JAResult.at(ijet), j.pt() ) );
      }
    }
    sort(ResortedJAResult.begin(), ResortedJAResult.end(),  PseudoJetPtCmp() );
    

    if ( ResortedJAResult.size()==0 ) {
      continue;
    }
    contrib::SoftDrop sd(beta, z_cut);
    if ( CustomRecluster ) {
      sd.set_reclustering(true, recluster);
    }
    //    sd.set_reclustering(true, const Recluster *recluster=0);
    sd.set_subtractor(&BackgroundSubtractor);
    sd.set_input_jet_is_subtracted( false );
    // sd.set_input_jet_is_subtracted( true );
    // sd.set_subtractor( 0 );
    // contrib::SoftDrop::_verbose=true;

    njets = ResortedJAResult.size();
    rho = JA.GetBackgroundEstimator()->rho();
    for (unsigned ijet = 0; ijet < ResortedJAResult.size(); ijet++) {
      // cout << " Orig jet pT: " << JAResult[ijet].pt() << endl;            
      // Run SoftDrop and examine the output
      PseudoJet& CurrentJet = ResortedJAResult[ijet].first
      PseudoJet sd_jet = sd( ResortedJAResult[ijet].first );
      
      zg[ijet]=0;
      zg1[ijet]=0;
      zg2[ijet]=0;
      delta_R[ijet]=0;      
      mu[ijet]=0;

      // assert(sd_jet != 0); //because soft drop is a groomer (not a tagger), it should always return a soft-dropped jet
      // HOWEVER, it seems the background subtractor may kill the sd_jet
      if ( sd_jet == 0){
	cout <<  " FOREGROUND Original Jet:   " << ResortedJAResult[ijet].first << endl;
	cout <<  " FOREGROUND rho A: " << JA.GetBackgroundEstimator()->rho() * ResortedJAResult[ijet].area() << endl;	  
	cout <<  " FOREGROUND Subtracted Jet: " << BackgroundSubtractor( ResortedJAResult[ijet] ) << endl;	  
	cout << " --- Skipped. Something caused SoftDrop to return 0 ---" << endl;

	// Use groomed==original in this case. Not ideal, but probably the best choice.
	JAResult[ijet] = BackgroundSubtractor( JAResult[ijet] );
	new ( Jets[ijet] )        TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( JAResult[ijet] )) );
	new ( GroomedJets[ijet] ) TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( JAResult[ijet] )) );

	new ( sj1[ijet] )        TStarJetVectorJet ( );
	new ( sj2[ijet] )        TStarJetVectorJet ( );

	zg[ijet]=0;
	delta_R[ijet]=0;
	mu[ijet]=0;
	
	((TStarJetVectorJet*)Jets[ijet])->SetArea( JAResult[ijet].area() );
	((TStarJetVectorJet*)Jets[ijet])->SetArea4Vector( JAResult[ijet].area_4vector().four_mom()  );
	// Save leading pt
	((TStarJetVectorJet*)Jets[ijet])->SetLeadingParticle( 0, sorted_by_pt(JAResult[ijet].constituents()).at(0).pt() );
	
	((TStarJetVectorJet*)GroomedJets[ijet])->SetArea( JAResult[ijet].area() );
	((TStarJetVectorJet*)GroomedJets[ijet])->SetArea4Vector( JAResult[ijet].four_mom()  );

      } else {       
	hzg->Fill(sd_jet.structure_of<contrib::SoftDrop>().symmetry());
	zg[ijet]=sd_jet.structure_of<contrib::SoftDrop>().symmetry();
	delta_R[ijet]=sd_jet.structure_of<contrib::SoftDrop>().delta_R();
	mu[ijet]=sd_jet.structure_of<contrib::SoftDrop>().mu();

	  // Now do background subtraction
	JAResult[ijet] = BackgroundSubtractor( JAResult[ijet] );
	//   if ( JAResult[ijet].pt() - sd_jet.pt() < -1e-5 ) cout << " ---> " << endl;
	// 	// if ( JAResult[ijet].pt() - sd_jet.pt() < -1e-5 ){
	//   	cout << JAResult[ijet].pt() 
	//   	     << "  " << sd_jet.pt()
	// 	     << "  " << (BackgroundSubtractor(sd_jet)).pt()
	// 	     // << "         " << sd_jet.structure_of<contrib::SoftDrop>().symmetry()
	// 	     << "   " << JA.GetBackgroundEstimator()->rho() 
	//   	     // << "         " << sd_jet.pieces().at(0).pt()
	//   	     // << "  " << Embrho*sd_jet.pieces().at(0).area()*3.14159 
	//   	     // << "         " << sd_jet.pieces().at(1).pt()
	//   	     // << "  " << Embrho*sd_jet.pieces().at(1).area()*3.14159 
	//   	     << endl;
	// // }

	// save original and groomed
	new ( Jets[ijet] )        TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( JAResult[ijet] )) );
	new ( GroomedJets[ijet] ) TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( sd_jet ))  );
	
	if ( sd_jet.pieces().size() >0 ){
	  new ( sj1[ijet] ) TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( sd_jet.pieces().at(0) ))  );
	  ((TStarJetVectorJet*)sj1[ijet])->SetMatch(-1);
	  ((TStarJetVectorJet*)sj1[ijet])->SetArea( sd_jet.pieces().at(0).area() );
	  ((TStarJetVectorJet*)sj1[ijet])->SetArea4Vector( sd_jet.pieces().at(0).area_4vector().four_mom()  );

	  // Repeat on subjets?
	  if ( Recursive) {
	    sd.set_subtractor(&BackgroundSubtractor);
	    sd.set_input_jet_is_subtracted( true ); // this may be trouble...
	    PseudoJet rsd_jet = sd( sd_jet.pieces().at(0) );
	    zg1[ijet] = rsd_jet.structure_of<contrib::SoftDrop>().symmetry();
	  }

	} else {
	  new ( sj1[ijet] )        TStarJetVectorJet ( );
	}
	if ( sd_jet.pieces().size() >1 ){
	  new ( sj2[ijet] ) TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( sd_jet.pieces().at(1) ))  );
	  ((TStarJetVectorJet*)sj2[ijet])->SetMatch(-1);
	  ((TStarJetVectorJet*)sj2[ijet])->SetArea( sd_jet.pieces().at(1).area() );
	  ((TStarJetVectorJet*)sj2[ijet])->SetArea4Vector( sd_jet.pieces().at(1).area_4vector().four_mom()  );
	  // Repeat on subjets?
	  if ( Recursive) {
	    sd.set_subtractor(&BackgroundSubtractor);
	    sd.set_input_jet_is_subtracted( true ); // this may be trouble...
	    PseudoJet rsd_jet = sd( sd_jet.pieces().at(1) );
	    zg2[ijet] = rsd_jet.structure_of<contrib::SoftDrop>().symmetry();
	  }

	} else {
	  new ( sj2[ijet] )        TStarJetVectorJet ( );
	}
	
	((TStarJetVectorJet*)Jets[ijet])->SetArea( JAResult[ijet].area() );
	((TStarJetVectorJet*)Jets[ijet])->SetArea4Vector( JAResult[ijet].area_4vector().four_mom()  );
	// Save leading pt
	((TStarJetVectorJet*)Jets[ijet])->SetLeadingParticle( 0, sorted_by_pt(JAResult[ijet].constituents()).at(0).pt() );
	
	((TStarJetVectorJet*)GroomedJets[ijet])->SetArea( sd_jet.area() );
	((TStarJetVectorJet*)GroomedJets[ijet])->SetArea4Vector( sd_jet.four_mom()  );
      }
      
      ((TStarJetVectorJet*)Jets[ijet])->SetMatch(-1);
      ((TStarJetVectorJet*)GroomedJets[ijet])->SetMatch(-1);

      // Make Jets[ijet].GetMatch() give the index of a matched hard parton
      // We have a choice here. 
      // - Use each hard parton exactly once, i.e. choose the best match (highest pT) when multiple options available
      // - Or match every jet to the best possible match, i.e., possibly use the targets multiple times
      // Using the first option
      // IMPORTANT: We do this by first-come first serve! The following assumes that we're working our way down in jet pT
      // Also, the partons vector is destroyed in the process
      for ( int ip=0; ip<partons.size(); ++ip){
	if ( partons[ip].pt() == 0 ) continue;
	if ( partons[ip].delta_R( JAResult[ijet] )< R){
	  ((TStarJetVectorJet*)Jets[ijet])->SetMatch( ip );
	  ((TStarJetVectorJet*)GroomedJets[ijet])->SetMatch( ip );
	  partons[ip] = PseudoJet();
	  break;
	} 
      }      
    }
    
    // Embedding?
    // ----------
    TStarJetPicoEventHeader* Embheader=0;
    UInt_t seed;
    if ( Embedding ){
      Embrefmult =0;
      // cout << " -------------- EMBEDDED ----------------- " << endl;

      for (int mix=0; mix < nMix; ++mix ){
	switch (Embintype) {
	  // =====================================================
	case INPICO :
	  // Need a unique uint from runid and eventid
	  // runid: last 6 numbers are unique (in a given year at least), 7 to be safe
	  // eventid: up to 10M events per runid aren't impossible
	  // --> 123456710000000.
	  // That's a bit more than uint_max |-(
	  // haha! eventid*runid % UINT_MAX; works, at least for run 7
	  // Try randtester.cxx for others first!!
	  seed = (mix+1)*eventid*runid % UINT_MAX;
	  gRandom->SetSeed(seed);
	  Embevi = gRandom->Integer(NEmbEvents); // Start at a random but deterministic point
	  pEmbReader->ReadEvent( Embevi );
	  // if ( !pEmbReader->NextEvent() ) {
	  //   cout << "Can't find a next embedding event, starting over." << endl;
	  //   delete pEmbReader; pEmbReader=0;
	  //   pEmbReader = SetupReader( EmbEvents, EmbTriggerName, SubjetParameters::RefMultCut );
	  //   InitializeReader(  pEmbReader, EmbInputName, NEmbEvents, PicoDebugLevel );
	  //   if ( !pEmbReader->NextEvent() ) {
	  //     cerr << "Can't find a next embedding event after reset, bailing out." << endl;
	  //     return -1;
	  //   }
	  // }
	  // pEmbReader->PrintStatus(10);
	  pEmbEvent = pEmbReader->GetOutputContainer()->GetArray();
	
	  Embheader = pEmbReader->GetEvent()->GetHeader();
	  Embrefmult = Embheader->GetProperReferenceMultiplicity();
	  Embeventid = Embheader->GetEventId();
	  Embrunid   = Embheader->GetRunId();
	
	  // return -1;
	  break;      
	  // =====================================================
	case INTREE :
	case MCTREE :
	  if ( Embevi>= NEmbEvents ) {
	    // qnd: just cycle:
	    Embevi=0;
	    // done=true;
	    // break;
	  }
	  EmbEvents->GetEntry(Embevi);
	  // cout << Embrunid << "  " << Embeventid << "  -->  ";
	  
	  if (Embintype == MCTREE ){
	    cname = EmbEvents->GetCurrentFile()->GetName();
	    filehash = cname.Hash();
	    if ( filehash > UINT_MAX - 100000 ) filehash /= 2;
	    if ( filehash < 1000000 ) filehash += 1000001;
	    Embrunid += filehash;
	  }
	  if (Embintype == INTREE ){
	    Embrunid = uEmbrunid;
	    Embeventid = uEmbeventid;
	  }

	  // cout << Embrunid << "  " << Embeventid << endl;
	  ++Embevi;
	  break;
	  // =====================================================
	default:
	  cerr << "Unknown intype " << intype << endl;
	  return(-1);
	}
	// if ( done ) break;

	// particles.clear(); // DEBUG!!      
	for ( int i=0 ; i<pEmbEvent->GetEntries() ; ++i ){
	  sv = (TStarJetVector*) pEmbEvent->At(i);
	  // Ensure kinematic similarity
	  if ( sv->Pt()< PtConsMin && sv->Pt()< PtSubConsMin ) continue;
	  if ( fabs( sv->Eta() )>EtaConsCut ) continue;
	  particles.push_back( PseudoJet (*sv ) );
	  ptphieta->Fill(sv->Pt(),sv->Phi(),sv->Eta());
	}
	// FIXME: May (will) not work as intended unless both inputs are picoDSTs!
	Embrunevent = ULong64_t(Embrunid)*10000000LL + Embeventid;

      
	// Run analysis
	// ------------
	JetAnalyzer EmbJA( particles, JetDef, AreaDef, select_jet_eta * (!fastjet::SelectorNHardest(2)) ) ;
	Subtractor& EmbBackgroundSubtractor =  *EmbJA.GetBackgroundSubtractor();
	Embrho=EmbJA.GetBackgroundEstimator()->rho() ;

	vector<PseudoJet> EmbJAResult = sorted_by_pt( select_jet( EmbJA.inclusive_jets() )) ;      
	Embnjets = EmbJAResult.size();      
	sd.set_subtractor( &EmbBackgroundSubtractor );
	sd.set_input_jet_is_subtracted(false);

	// contrib::SoftDrop unsubsd(beta, z_cut);
	// unsubsd.set_input_jet_is_subtracted( false );

	int j=0; // Counter for jets to save
	for (unsigned ijet = 0; ijet < EmbJAResult.size(); ijet++) {
	
	  // Need to skip if this jet is unhealthy
	  if ( EmbBackgroundSubtractor( EmbJAResult[ijet] ).pt() < 1 ) continue;
	
	  // if (ijet == 0){
	  //   cout << "Original : " << EmbJAResult[ijet].pt() << endl;
	  //   cout << "Should be: " << EmbBackgroundSubtractor( EmbJAResult[ijet] ).pt() << endl;
	  // }

	  // Run SoftDrop and examine the output
	  // -----------------------------------
	  // PseudoJet sd_jet = EmbBackgroundSubtractor(sd(EmbJAResult[ijet]) );
	  PseudoJet sd_jet = sd(EmbJAResult[ijet]);

	  Embzg[j]=0;
	  Embdelta_R[j]=0;      
	  Embmu[j]=0;

	  // assert(sd_jet != 0); //because soft drop is a groomer (not a tagger), it should always return a soft-dropped jet
	  // HOWEVER, it seems the background subtractor may kill the sd_jet
	  if ( sd_jet == 0){
	    cout <<  " Original Jet:   " << EmbJAResult[ijet] << endl;
	    cout <<  " Subtracted Jet: " << EmbBackgroundSubtractor( EmbJAResult[ijet] ) << endl;	  
	    cout << " --- Skipped. Something caused SoftDrop to return 0 ---" << endl;

	    // Use groomed==original in this case. Not ideal, but probably the best choice.
	    EmbJAResult[ijet] = EmbBackgroundSubtractor( EmbJAResult[ijet] );
	    new ( EmbJets[j] )        TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( EmbJAResult[ijet] )) );
	    new ( EmbGroomedJets[j] ) TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( EmbJAResult[ijet] )) );

	    new ( Embsj1[j] )        TStarJetVectorJet ( );
	    new ( Embsj2[j] )        TStarJetVectorJet ( );

	    Embzg[j]=0;
	    Embdelta_R[j]=0;
	    Embmu[j]=0;
	
	    ((TStarJetVectorJet*)EmbJets[j])->SetArea( EmbJAResult[ijet].area() );
	    ((TStarJetVectorJet*)EmbJets[j])->SetArea4Vector( EmbJAResult[ijet].area_4vector().four_mom()  );
	    // Save leading pt
	    ((TStarJetVectorJet*)EmbJets[j])->SetLeadingParticle( 0, sorted_by_pt(EmbJAResult[ijet].constituents()).at(0).pt() );
	
	    ((TStarJetVectorJet*)EmbGroomedJets[j])->SetArea( EmbJAResult[ijet].area() );
	    ((TStarJetVectorJet*)EmbGroomedJets[j])->SetArea4Vector( EmbJAResult[ijet].four_mom()  );

	  } else {
	    // unsubsd.set_subtractor();
	    // PseudoJet unsubsd_jet = unsubsd( EmbJAResult[ijet] );
	    
	    // // Try it two different ways
	    // THIS DOESN'T WORK
	    // SoftDrop is inextricably linked to background subtraction at every step along the way
	    
	    // cout << "standard  zg = " << sd_jet.structure_of<contrib::SoftDrop>().symmetry();
	    // // cout << "   unsubs zg = " << unsubsd_jet.structure_of<contrib::SoftDrop>().symmetry();
	    // if (unsubsd_jet.pieces().size() == 0)
	    //   cout << "  By Hand -->  " << " no subjet structure?" << endl;
	    // if (unsubsd_jet.pieces().size() == 1 )
	    //   cout << "  By Hand -->  " << " 1 subjet, zg=0" << endl;
	    // if (unsubsd_jet.pieces().size() > 1 ){
	    //   PseudoJet hj0 = unsubsd_jet.pieces().at(0);
	    //   PseudoJet hj1 = unsubsd_jet.pieces().at(1);	  
	    
	    //   double pt0 = hj0.pt();
	    //   double pt1 = hj1.pt();
	    //   // subtract rho*A?
	    //   pt0 -= Embrho * hj0.area();
	    //   pt1 -= Embrho * hj1.area();
	    //   // //cap at 0?
	    //   pt0 = max(0.0, pt0);
	    //   pt1 = max(0.0, pt1);
	    
	    //   cout << "  By Hand -->  " << " 2 subjets: zg = "
	    //        << pt1 / (  pt0 + pt1 )  << endl;
	    //   cout << "Orig pT = " << EmbJAResult[ijet].pt() <<  "     sum pT = " << pt0 + pt1 << "    pt0=" << pt0 <<  "    pt1=" << pt1 << endl;
	    //   cout << "rho  A0 = " <<  Embrho*hj0.area() << "       rho A1 = " <<  Embrho*hj1.area() << endl;
	    // }
	    
	    // Now do background subtraction
	    EmbJAResult[ijet] = EmbBackgroundSubtractor( EmbJAResult[ijet] );
	    // cout << "After subtraction pT = " << EmbJAResult[ijet].pt() << endl << endl;
	    
	    hEmbzg->Fill(sd_jet.structure_of<contrib::SoftDrop>().symmetry());
	    Embzg[ijet]=sd_jet.structure_of<contrib::SoftDrop>().symmetry();
	    Embdelta_R[ijet]=sd_jet.structure_of<contrib::SoftDrop>().delta_R();
	    Embmu[ijet]=sd_jet.structure_of<contrib::SoftDrop>().mu();
	    
	    // save original and groomed
	    new ( EmbJets[j] )        TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( EmbJAResult[ijet] ) ) );
	    new ( EmbGroomedJets[j] ) TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( sd_jet ) ) );
	    
	    ((TStarJetVectorJet*)EmbJets[j])->SetMatch(-1);
	    ((TStarJetVectorJet*)EmbGroomedJets[j])->SetMatch(-1);
	    
	    ((TStarJetVectorJet*)EmbJets[j])->SetArea( EmbJAResult[ijet].area() );
	    ((TStarJetVectorJet*)EmbJets[j])->SetArea4Vector( EmbJAResult[ijet].area_4vector().four_mom()  );
	    ((TStarJetVectorJet*)EmbJets[j])->SetLeadingParticle( 0, sorted_by_pt(EmbJAResult[j].constituents()).at(0).pt() );
	    ((TStarJetVectorJet*)EmbGroomedJets[j])->SetArea( sd_jet.area() );
	    ((TStarJetVectorJet*)EmbGroomedJets[j])->SetArea4Vector( sd_jet.four_mom()  );
	    
	    if ( sd_jet.pieces().size() >0 ){
	      new ( Embsj1[j] ) TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( sd_jet.pieces().at(0) ))  );
	      ((TStarJetVectorJet*)Embsj1[j])->SetMatch(-1);
	      ((TStarJetVectorJet*)Embsj1[j])->SetArea( sd_jet.pieces().at(0).area() );
	      ((TStarJetVectorJet*)Embsj1[j])->SetArea4Vector( sd_jet.pieces().at(1).area_4vector().four_mom()  );
	    } else {
	      new ( Embsj1[j] )        TStarJetVectorJet ( );
	    }
	    if ( sd_jet.pieces().size() >1 ){
	      new ( Embsj2[j] ) TStarJetVectorJet ( TStarJetVector(MakeTLorentzVector ( sd_jet.pieces().at(1) ))  );
	      ((TStarJetVectorJet*)Embsj2[j])->SetMatch(-1);
	      ((TStarJetVectorJet*)Embsj2[j])->SetArea( sd_jet.pieces().at(0).area() );
	      ((TStarJetVectorJet*)Embsj2[j])->SetArea4Vector( sd_jet.pieces().at(1).area_4vector().four_mom()  );
	    } else {
	      new ( Embsj2[j] )        TStarJetVectorJet ( );
	    }

	  }

	  // Match to an original jet. Similar considerations apply as for parton matching
	  // The original jet vector is destroyed in the process
	  for ( int iorig=0; iorig<JAResult.size(); ++iorig){
	    if ( JAResult[iorig].pt() == 0 ) continue;
	    if ( JAResult[iorig].delta_R( EmbJAResult[j] )< R){
	      ((TStarJetVectorJet*)EmbJets[j])->SetMatch( iorig );
	      ((TStarJetVectorJet*)EmbGroomedJets[j])->SetMatch( iorig );
	      // // if (j == 0) {
	      // cout << "Is:     " << ((TStarJetVectorJet*)EmbJets[j])->Pt() << endl;
	      // cout << "Compare to:     " << 	    JAResult[iorig].pt() << endl;
	      // cout << "=============== " << endl;
	      JAResult[iorig] = PseudoJet();
	      break;
	    } 
	  }
	  j++;
	} // for #jets
	ResultTree->Fill();   
      } // for mix
    } else {
      // Fill result tree
      ResultTree->Fill();   
    }


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
//----------------------------------------------------------------------
void InitializeReader(  TStarJetPicoReader* pReader, const TString InputName, const Long64_t NEvents, const int PicoDebugLevel ){
  // pReader = SetupReader( Events, TriggerName, SubjetParameters::RefMultCut );
  TStarJetPicoReader& reader = *pReader;
  reader.SetApplyFractionHadronicCorrection(kTRUE);
  reader.SetFractionHadronicCorrection(0.9999);
  reader.SetRejectTowerElectrons( kFALSE );
    
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
  reader.Init(NEvents);

  TStarJetPicoDefinitions::SetDebugLevel(PicoDebugLevel);
}
//----------------------------------------------------------------------

// DEBUG
void decluster (PseudoJet j){
  cout << " ### Declustering ### " << endl;
  cout << j.pt() << endl;

  PseudoJet piece1, piece2;

  static int level=0;
  if ( j.has_parents(piece1, piece2) ){
    cout << "level = " << level++ << endl;
    cout << "piece1.pt() = " << piece1.pt() << endl;
    cout << "piece2.pt() = " << piece2.pt() << endl;

    if (! piece1.is_pure_ghost() ) decluster(piece1);
    if (! piece2.is_pure_ghost() ) decluster(piece2);

  } else cout << " Done with this branch" << endl;
  return;
}
