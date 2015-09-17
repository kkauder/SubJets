/** @file SubjetWrapper.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Subjet Analysis for pythia events.
    @details Perform Subjet analysis on a Pytha chain.
    Since this is pythia, we also separate by original hard parton.
    @date Mar 13, 2015
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
  Long64_t NPythiaEvents=50000;
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
    //    if ( CurrentPythiaEvent.size() ) cout << sorted_by_pt(CurrentPythiaEvent).at(0).pt() << endl;
    
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

    // Skip empty events (not that there should be any)
    if ( CurrentPythiaEvent.size()<1 ) continue;
    // And events where we can't identify the trigger
    if ( CurrentTriggerPartons.size()<2 ) continue;
    
    // Skip events that have no chance to fulfill the minimum jet pT
    if ( sorted_by_pt(CurrentTriggerPartons).at(0).pt() < PtJetMin - 5 ) continue;
    FullPythiaEvent.push_back ( CurrentPythiaEvent );
    TriggerPartons.push_back( CurrentTriggerPartons );  
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
  DevSubjetAnalysis SubjetA( R, SubR,
			     LargeJetAlgorithm, SubJetAlgorithm,
			     PtJetMin, PtJetMax,
			     EtaConsCut, PtConsMin, PtConsMax,
			     JetPt, JetArea, Nsub, 
			     SubVLeadPt,       AllVLeadPt,
			     SubVLeadPtFrac,   AllVLeadPtFrac,
			     SubVLeadArea,     AllVLeadArea,
			     SubVLeadAreaFrac, AllVLeadAreaFrac,
			     SubVLeadDeltaR,   AllVLeadDeltaR
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

  cout << "Done." << endl;
  return 0;
}
