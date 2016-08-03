/** @file PythiaInMcAj.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Similar to ppInAuAuAj, but use mock BG and pythia events
    @date Mar 04, 2015
*/

#include "AjAnalysis.hh"
// #include "AjParameters.hh"
#include "ktTrackEff.hh"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#include "RooUnfoldSvd.h"
#include "RooUnfoldTUnfold.h"


#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>

#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <TChain.h>
#include <tBranch.h>
#include <TMath.h>
#include <TRandom.h>

#include <iostream>
#include <cmath>

using namespace std;
using namespace fastjet;

/** 
    - Set up vector of pythia and MC events
    - Set up output histos and tree
    - "Truth": Can use 
    --- jetfinder result without cuts and with BG subtraction
    --- jetfinder result with cuts, with or without BG subtraction
    --- hard parton pT
    - "Distorted": Sources:
    --- tracking efficiency  --> lose charged particles
    --- pT resolution        --> smear charged pT  (phi, eta?)
    --- Tower Scale          --> smear neutral pT  (phi, eta?)
    --- BG fluctuations      --> Add pseudo Heavy Ion Background, with or without subtraction
*/
Int_t main(int argc, char **argv) {
  

  // Load input
  // ----------
  TString InputName = "Data/PythiaAndMc_0.root";
  TChain* InputChain = new TChain("tree");
  InputChain->Add(InputName);
  assert ( InputChain->GetEntries()>0 && "Something went wrong loading input.");
  
  TClonesArray* pPythia = new TClonesArray("TStarJetVector");
  InputChain->GetBranch("PythiaParticles")->SetAutoDelete(kFALSE);
  InputChain->SetBranchAddress("PythiaParticles", &pPythia);

  TClonesArray* pMc = new TClonesArray("TStarJetVector");
  InputChain->GetBranch("McParticles")->SetAutoDelete(kFALSE);
  InputChain->SetBranchAddress("McParticles", &pMc);

  int eventid;
  InputChain->SetBranchAddress("eventid", &eventid);
  int runid;
  InputChain->SetBranchAddress("runid", &runid);

  // Output
  // ------
  TString OutFileName = "UnfoldingTest.root";
  int randomoff=0;
  if ( argc>1 ){
    randomoff = TString( argv[1][0] ).Atoi(); // defaults to zero
  }
  randomoff *= 10000000;
  TFile* fout = new TFile( OutFileName, "RECREATE");

  // jet resolution parameter
  // ------------------------
  float R = 0.4;

  // soft constituent cut
  // --------------------
  float PtConsLo=0.2;

  cout << " ################################################### " << endl;
  cout << "Triggering with R=" << R << endl;
  cout << "Low pT cut =" << PtConsLo << endl;
  cout << " ################################################### " << endl;

  cout << " ################################################### " << endl;
  cout << "   FastJet random seeds offset by" << randomoff << endl;
  cout << " ################################################### " << endl;
  
  
  // Histograms
  // ----------
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);
  
  TH2D* RecoVsTruePt = new TH2D( "RecoVsTruePt","Reco Vs True p_{T};p_{T}^{true} [GeV/c];p_{T}^{reco} [GeV/c]", 60, 0 , 60, 120, 0, 60 );
  TH1D* TestTruePt = new TH1D( "TestTruePt","TestTruePt;p_{T}^{true} [GeV/c]", 60, 0 , 60);
  TH1D* TestDistortedPt = new TH1D( "TestDistortedPt","TestDistortedPt;p_{T}^{reco} [GeV/c]", 120, 0 , 60);

  // Helpers
  // -------
  vector<PseudoJet> TrueParticles;
  vector<PseudoJet> DistortedParticles;
  int nJets = 0;

  // Parameters
  // ----------
  const int ghost_repeat = 1;
  const double ghost_area = 0.01;

  const double jet_ptmin = 10.0;    ///< Min jet pT
  const double jet_ptmax = 1000.0;   ///< DEBUG
  
  // const double LeadPtMin=20.0;      ///< Min leading jet pT 
  const double LeadPtMin=16.0;      ///< Min leading jet pT 
  const double dPhiCut = 0.4;       ///< Dijet acceptance angle,  |&phi;1 - &phi;2 -&pi;| < dPhiCut
  
  const double max_track_rap = 1.0; ///< Constituent &eta; acceptance
  const double PtConsHi=2.0;        ///< High constituent pT cut  

  // derived rapidity cuts
  // ---------------------
  double max_rap      = max_track_rap-R;
  double ghost_maxrap = max_rap + 2.0 * R;

  // Constituent selectors
  // ---------------------
  Selector select_track_rap = fastjet::SelectorAbsRapMax(max_track_rap);
  Selector select_lopt      = fastjet::SelectorPtMin( PtConsLo );
  Selector select_hipt      = fastjet::SelectorPtMin( PtConsHi );
  
  Selector slo = select_track_rap * select_lopt;
  Selector shi = select_track_rap * select_hipt;

  // Jet candidate selectors
  // -----------------------
  Selector select_jet_rap     = fastjet::SelectorAbsRapMax(max_rap);
  Selector select_jet_pt_min  = fastjet::SelectorPtMin( jet_ptmin );
  Selector select_jet_pt_max  = fastjet::SelectorPtMax( jet_ptmax );
  Selector sjet = select_jet_rap && select_jet_pt_min && select_jet_pt_max;
  
  // Choose a jet and area definition
  // --------------------------------
  JetDefinition jet_def = fastjet::JetDefinition(fastjet::antikt_algorithm, R);
  
  // create an area definition for the clustering
  //----------------------------------------------------------
  // ghosts should go up to the acceptance of the detector or
  // (with infinite acceptance) at least 2R beyond the region
  // where you plan to investigate jets.
  GhostedAreaSpec area_spec = fastjet::GhostedAreaSpec( ghost_maxrap, ghost_repeat, ghost_area );
  AreaDefinition  area_def = fastjet::AreaDefinition(fastjet::active_area_explicit_ghosts, area_spec);

  // For random seed
  // ---------------
  fastjet::GhostedAreaSpec TmpArea; // for access to static random seed
  vector<int> SeedStatus;

  // Initialize tracking efficiency
  // ------------------------------
  ktTrackEff* tEff=0;
  int mEffUn=0;
  tEff = new ktTrackEff();
  tEff->SetSysUncertainty(mEffUn);
  cout<<endl;
  tEff->PrintInfo();
  cout<<endl;

  // Gaussian used for smearing
  // --------------------------
  TF1* smear = new TF1("smear","gaus",-100, 100 );  


  // Set up response matrix
  // ----------------------
  RooUnfoldResponse response (120, 0 , 60, 60, 0 , 60);

  // Cycle through events
  // --------------------  
  int nEvents=InputChain->GetEntries();
  // nEvents=100;
  TStarJetVector* sv;
  cout << "Performing analysis." << endl;

  for ( int ev=0; ev<nEvents  ; ++ev ){
    if ( !(ev%1000) ) cerr << "Event " << ev << " / " << nEvents << endl;

    TrueParticles.clear();
    DistortedParticles.clear();
    InputChain->GetEntry(ev);

    // Jet particles
    // -------------

    for ( int i=0; i < pPythia->GetEntries() ; ++i ){
      sv = (TStarJetVector*) pPythia->At(i);

      // TRUTH:
      TrueParticles.push_back( MakePseudoJet ( sv ) );

      // Distort CHARGED particles
      // -------------------------
      if (sv->GetCharge()!=0 ) {

	// efficiency
	// ----------
	Double_t reff=tEff->EffRatio_20(sv->Eta(),sv->Pt());
	Double_t mran=gRandom->Uniform(0,1);
	if (mran>reff)  {
	  continue;
	}

	// Smearing
	// --------
	float ptRes = 0.01;
	float pt = sv->Pt();
	float sigmapt = pt*ptRes;
	float eta = sv->eta();
	float phi = sv->phi();
	smear->SetParameters ( 1, pt, sigmapt );
	smear->SetRange ( pt - 5*sigmapt, pt + 5*sigmapt );
	sv->SetPtEtaPhiM( smear->GetRandom(), eta, phi, 0);	
	
      } 
      
      // 	if (sv->GetCharge()==0 ) (*sv) *= fTowScale; // for systematics
      // 	pj=MakePseudoJet( sv );
      // 	pj.set_user_info ( new JetAnalysisUserInfo( 3*sv->GetCharge() ) );

      // 	if ( sv->GetCharge()!=0 && tEff ) {
      // 	  Double_t reff=tEff->EffRatio_20(sv->Eta(),sv->Pt());
      // 	  Double_t mran=gRandom->Uniform(0,1);
      // 	  // cout << reff << "  " << mran << endl;
      // 	  if (mran<reff)  {
      // 	    particles.push_back ( pj );
      // 	  }
      // 	} else { // no charge or no efficiency class
      // 	  particles.push_back ( pj );
      // 	}	      

      DistortedParticles.push_back( MakePseudoJet ( (TLorentzVector*)pPythia->At(i)) ); 

    }
    
    // Fill up with pseudo Au+Au
    // -------------------------
    for ( int i=0; i < pMc->GetEntries() ; ++i ){
      DistortedParticles.push_back( MakePseudoJet ( (TLorentzVector*)pMc->At(i)) );
    }
    
    // random seed for fastjet
    // -----------------------
    TmpArea.get_random_status(SeedStatus);
    if ( SeedStatus.size() !=2 ) {
      throw std::string("SeedStatus.size() !=2");
      return -1;
    }
    SeedStatus.at(0) = runid   + randomoff;
    SeedStatus.at(1) = eventid + randomoff;
    TmpArea.set_random_status(SeedStatus);


    // Background selector
    // -------------------
    Selector selector_bkgd = fastjet::SelectorAbsRapMax( max_rap ) * (!fastjet::SelectorNHardest(2));
    
    // Analysis
    // --------
    // vector<PseudoJet> pLo = slo( TrueParticles );
    vector<PseudoJet> pHiTrue = shi( TrueParticles );
    vector<PseudoJet> pHiDistorted = shi( DistortedParticles );

    // find high constituent pT jets
    // -----------------------------
    // TRUTH:
    // ------
    ClusterSequence csaHiTrue ( pHiTrue, jet_def );
    vector<PseudoJet> HiResultTrue = fastjet::sorted_by_pt( sjet ( csaHiTrue.inclusive_jets() ) );
    if ( HiResultTrue.size() < 1 )                 {     continue; }
    if ( HiResultTrue.at(0).pt() < LeadPtMin )     {     continue; }

    // RECO:
    // -----
    ClusterSequence csaHiDistorted ( pHiDistorted, jet_def );
    vector<PseudoJet> HiResultDistorted = fastjet::sorted_by_pt( sjet ( csaHiDistorted.inclusive_jets() ) );
    if ( HiResultDistorted.size() < 1 ){
      cerr << "DISTORTED: NOTHING FOUND." << endl;
      if ( ev %2 )       response.Miss ( HiResultTrue.at(0).pt() );
      continue;
    }
    
    // find MATCHING jet
    // -----------------
    fastjet::Selector SelectClose = fastjet::SelectorCircle( R );
    SelectClose.set_reference( HiResultTrue.at(0) );
    std::vector<fastjet::PseudoJet> MatchedToTruth = sorted_by_pt(SelectClose( HiResultDistorted ));
    if ( MatchedToTruth.size() < 1 ){
      cerr << "DISTORTED: NOTHING MATCHED." << endl;
      if ( ev %2 )       response.Miss ( HiResultTrue.at(0).pt() );
      continue;
    }

    // Use half for training, half for testing
    if ( ev %2 ){
      RecoVsTruePt->Fill( HiResultTrue.at(0).pt(), MatchedToTruth.at(0).pt() );
      response.Fill( MatchedToTruth.at(0).pt(), HiResultTrue.at(0).pt() );
    } else {
      TestTruePt->Fill( HiResultTrue.at(0).pt() );
      TestDistortedPt->Fill( MatchedToTruth.at(0).pt() );
    }

    
  } // nEvents
    

  // Close up shop
  // -------------
  response.SetName("response");
  fout->cd();
  response.Write();
  fout->Write();

  // cout << "In " << nEvents << " events, found " << endl
  //      << nHardDijets << " dijets with constituents above 2 GeV," << endl
  //      << nCorrespondingLowDijets << " corresponding dijets with constituents above 0.2 GeV," << endl
  //      << " of which " <<  nMatchedDijets << " could be matched." << endl;

  cout << "Wrote to " << fout->GetName() << endl;
  cout << "Bye." << endl;
  
  return 0;
  
}
