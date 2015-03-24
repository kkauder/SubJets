/** @file SubjetWrapper.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Subjet Analysis for pythia events.
    @details Perform Subjet analysis on a Pytha chain.
    Since this is pythia, we also separate by original hard parton.
    @date Mar 13, 2015
*/

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


#include <cmath>

using namespace std;
using namespace fastjet;


/** 
    - Parse parameters
    - Set up input tree
    - Set up output histos and tree
    - Initialize SubjetAnalysis object
    - Loop through events
    \arg argv: flags.
    Display options with
    <BR><tt>% SubjetWrapper -h </tt> 
    <BR>Note that wildcarded file patterns should be in single quotes.
*/
int main( int argc, const char** argv ){
  
  // Defaults
  // --------
  TString PythiaName = "pytest.root";
  TString ChainName = "tree";
  TString OutFileName = "Results/SubjetResult.root";

  float R    = SubjetParameters::R;
  float SubR = SubjetParameters::SubR;
  JetAlgorithm LargeJetAlgorithm = SubjetParameters::LargeJetAlgorithm;
  JetAlgorithm SubJetAlgorithm   = SubjetParameters::SubJetAlgorithm;
  
  float PtJetMin      = SubjetParameters::PtJetMin;
  float PtJetMax      = SubjetParameters::PtJetMax;
  float EtaConsCut    = SubjetParameters::EtaConsCut;
  float PtConsMin     = SubjetParameters::PtConsMin;
  float PtConsMax     = SubjetParameters::PtConsMax;
  // By default, use the same constituent pT cut for jets and subjets
  float PtSubConsMin  = SubjetParameters::PtConsMin;
  float PtSubConsMax  = SubjetParameters::PtConsMax;  
  
  // parse arguments
  // ---------------
  vector<string> arguments(argv + 1, argv + argc);
  bool argsokay=true;
  Long64_t NPythiaEvents=9999;
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
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtSubConsMin = atof((parg)->data());      
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PtSubConsMax = atof((parg)->data());
    } else if ( arg == "-o" ){     
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      OutFileName=*parg;
    } else if ( arg == "-i" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      PythiaName=*parg;
    } else if ( arg == "-c" ){
      if (++parg ==arguments.end() ){ argsokay=false; break; }
      ChainName=*parg;
    } else if ( arg == "-N" ){      
      if (++parg==arguments.end() ){ argsokay=false; break; }
      NPythiaEvents=atoi(parg->data());
    } else {
      argsokay=false;
      break;
    }
  }
  if ( !argsokay ) {
    cerr << "usage: " << argv[0]
	 << " [-o OutFileName]"
      	 << " [-lja LargeJetAlgorithm]"
	 << " [-sja SubJetAlgorithm]"
      	 << " [-i infilepattern]"
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
  cout << " Reading tree named \""<< ChainName << "\" from " << PythiaName << endl;
  cout << " Writing to " << OutFileName << endl;
  cout << " N = " << NPythiaEvents << endl;


  // read in input particles
  //----------------------------------------------------------
  TChain* PythiaJets = new TChain(ChainName);
  PythiaJets->Add(PythiaName);
  assert ( PythiaJets->GetEntries()>0 && "Something went wrong loading the Pythia jets.");
  NPythiaEvents=min(NPythiaEvents,PythiaJets->GetEntries() );
  
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
  
  cout << "Filling pythia vector" << endl;
  for ( Long64_t pythiaEv = 0; pythiaEv< NPythiaEvents ; ++ pythiaEv ){
    if ( !(pythiaEv%1000) ) cout << "Working on " << pythiaEv << " / " << NPythiaEvents << endl;
    PythiaJets->GetEntry(pythiaEv);
    CurrentPythiaEvent.clear();
    for ( int i=0 ; i<pFullEvent->GetEntries() ; ++i ){
      lv = (TLorentzVector*) pFullEvent->At(i);
      // Ensure kinematic similarity
      if ( lv->Pt()< PtConsMin && lv->Pt()< PtSubConsMin ) continue;
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
      TString& s = ((TObjString*)(pHardPartonNames->At(0)))->String();
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
 
  // Files and histograms
  // --------------------
  TFile* fout = new TFile( OutFileName, "RECREATE");
  
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);
  
  TH2D*  NsubPt        = new TH2D( "NsubPt"         , "# of subjets;p_{T}^{lead} [GeV/c];#", 100, 0 , 100, 100, -0.5, 99.5 );
  TH2D*  SubPtFrac     = new TH2D( "SubPtFrac"      , "pT fraction carried by leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 50, 0.01, 1.01 );
  TH2D*  OtherPtFrac   = new TH2D( "OtherPtFrac"    , "pT fraction carried by non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 50, 0.01, 1.01 );
  TH2D*  SubDeltaR     = new TH2D( "SubDeltaR"      , "#Delta R of leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D*  OtherDeltaR   = new TH2D( "OtherDeltaR"    , "#Delta R of non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D*  JetArea       = new TH2D( "JetArea"        , "Area of original jet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D*  SubArea       = new TH2D( "SubArea"        , "Area of leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D*  OtherArea     = new TH2D( "OtherArea"      , "Area of non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D*  SubAreaFrac   = new TH2D( "SubAreaFrac"    , "Area Fraction of leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D*  OtherAreaFrac = new TH2D( "OtherAreaFrac"  , "Area Fraction of non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );

  TH2D* gNsubPt        = new TH2D( "gNsubPt"        , "gluon-led, # of subjets;p_{T}^{lead} [GeV/c];#", 100, 0 , 100, 100, -0.5, 99.5 );
  TH2D* gSubPtFrac     = new TH2D( "gSubPtFrac"     , "gluon-led, pT fraction carried by leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 50, 0.01, 1.01 );
  TH2D* gOtherPtFrac   = new TH2D( "gOtherPtFrac"   , "gluon-led, pT fraction carried by non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 50, 0.01, 1.01 );
  TH2D* gSubDeltaR     = new TH2D( "gSubDeltaR"     , "gluon-led, #Delta R of leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* gOtherDeltaR   = new TH2D( "gOtherDeltaR"   , "gluon-led, #Delta R of non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* gJetArea       = new TH2D( "gJetArea"       , "gluon-led, Area of original jet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* gSubArea       = new TH2D( "gSubArea"       , "gluon-led, Area of leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* gOtherArea     = new TH2D( "gOtherArea"     , "gluon-led, Area of non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* gSubAreaFrac   = new TH2D( "gSubAreaFrac"   , "gluon-led, Area Fraction of leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* gOtherAreaFrac = new TH2D( "gOtherAreaFrac" , "gluon-led, Area Fraction of non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );

  TH2D* qNsubPt        = new TH2D( "qNsubPt"        , "quark-led, # of subjets;p_{T}^{lead} [GeV/c];#", 100, 0 , 100, 100, -0.5, 99.5 );
  TH2D* qSubPtFrac     = new TH2D( "qSubPtFrac"     , "quark-led, pT fraction carried by leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 50, 0.01, 1.01 );
  TH2D* qOtherPtFrac   = new TH2D( "qOtherPtFrac"   , "quark-led, pT fraction carried by non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 50, 0.01, 1.01 );
  TH2D* qSubDeltaR     = new TH2D( "qSubDeltaR"     , "quark-led, #Delta R of leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* qOtherDeltaR   = new TH2D( "qOtherDeltaR"   , "quark-led, #Delta R of non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* qJetArea       = new TH2D( "qJetArea"       , "quark-led, Area of original jet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* qSubArea       = new TH2D( "qSubArea"       , "quark-led, Area of leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* qOtherArea     = new TH2D( "qOtherArea"     , "quark-led, Area of non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* qSubAreaFrac   = new TH2D( "qSubAreaFrac"   , "quark-led, Area Fraction of leading subjet;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );
  TH2D* qOtherAreaFrac = new TH2D( "qOtherAreaFrac" , "quark-led, Area Fraction of non-leading subjets;p_{T}^{lead} [GeV/c];", 100, 0 , 100, 100, 0.0, 1.0 );

  // List of miscellaneous info
  // --------------------------
  TTree* info = new TTree("info", "Information");
  info->Branch("PythiaName"        , (void*)PythiaName.Data()    , "PythiaName/C" );
  info->Branch("ChainName"         , (void*)ChainName.Data()     , "ChainName/C" );
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


  // Initialize Analysis class
  // -------------------------
  SubjetAnalysis SubjetA( R, SubR,
			  LargeJetAlgorithm, SubJetAlgorithm,
			  PtJetMin, PtJetMax,
			  EtaConsCut, PtConsMin, PtConsMax,
			  //			  PtSubConsMin, PtSubConsMax,
			   NsubPt,  SubPtFrac,  OtherPtFrac,  SubDeltaR,  OtherDeltaR,  JetArea,  SubArea,  OtherArea,  SubAreaFrac,  OtherAreaFrac,
			  gNsubPt, gSubPtFrac, gOtherPtFrac, gSubDeltaR, gOtherDeltaR, gJetArea, gSubArea, gOtherArea, gSubAreaFrac, gOtherAreaFrac,
			  qNsubPt, qSubPtFrac, qOtherPtFrac, qSubDeltaR, qOtherDeltaR, qJetArea, qSubArea, qOtherArea, qSubAreaFrac, qOtherAreaFrac
			  );

  // Go through events
  // -----------------
  cout << "Running analysis" << endl;
  Long64_t Ntot=0;
  Long64_t Naccepted=0;
  Long64_t Nrejected=0;
  
  for ( int pythiai =0; pythiai< FullPythiaEvent.size() ; ++pythiai ){
    if ( !(pythiai%1000) ) cout << "Working on " << pythiai << " / " << FullPythiaEvent.size() << endl;
    
    vector<PseudoJet>& particles = FullPythiaEvent.at(pythiai);
    vector<PseudoJet>& partons = TriggerPartons.at(pythiai);
    
    // Run analysis
    // ------------
    int ret =SubjetA.AnalyzeAndFill( particles, &partons );

    // Update counters
    Ntot++;
    switch ( ret ){
    case 0 : Naccepted++;
      break;
    case -1 :
      Nrejected++;
      break;
    default :
      cerr << "Unknown return value " << ret << endl;
      return ret;
    }

  }  
  
  info->Branch("Ntot"      , &Ntot      , "Ntot/L" );
  info->Branch("Naccepted" , &Naccepted , "Naccepted/L" );
  info->Branch("Nrejected" , &Nrejected , "Nrejected/L" );
  info->Fill();
  
  fout->Write();
  return 0;
}
