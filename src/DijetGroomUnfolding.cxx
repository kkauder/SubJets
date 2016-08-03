/** @file DijetGroomUnfolding.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief AJ type dijets unfolding
    @date Nov 12, 2015
*/

// #include "AjParameters.hh"
// #include "ktTrackEff.hh"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#include "RooUnfoldSvd.h"
#include "RooUnfoldTUnfold.h"

#include "TStarJetVectorContainer.h"
#include "TStarJetVector.h"
#include "TStarJetVectorJet.h"
#include "TStarJetPicoTriggerInfo.h"
#include "TStarJetPicoUtils.h"

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
#include <TSystem.h>

#include <iostream>
#include <cmath>
#include <assert.h>

using namespace std;
//using namespace fastjet;

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

  // // Parameters
  // // ----------
  int RefmultCut = 269;  // 269 for 0-20%, 399 for 0-10%
  float openingAngle=0.4;
  float MatchingR=0.4;

  // Load input
  // ----------
  // PROBLEM: HI and LO are not embedded into the same events...
  // And if (when) we reuse pp events, the eventid/runid matching no longer works.
  // Sigh......
  // Got to make sure that
  // a) embedded event gets assigned to pre-determined (set of) mixing events
  // b) any pp+AuAu combination gets a unique (and deterministic) identifier.
  // Urghhh

  TFile* fLo = new TFile("Results/Groom_pp.root","READ");
  TTree* RLo = (TTree*) fLo->Get("ResultTree");
  RLo->SetName("RLo");

  TString outbase = gSystem->BaseName( fLo->GetName() );
  // outbase.ReplaceAll( ".root", "");
  // outbase.Prepend("Plots/");

  TFile* fHi = new TFile("Results/Hi_Groom_pp.root","READ");
  TTree* RHi = (TTree*) fHi->Get("ResultTree");
  RHi->SetName("RHi");

  RLo->BuildIndex("runid","eventid");
  RHi->BuildIndex("runid","eventid");

  // To match, use runid and eventid
  int runidLo;
  RLo->SetBranchAddress("runid", &runidLo );
  int eventidLo;
  RLo->SetBranchAddress("eventid", &eventidLo );

  // Interesting measurables
  TClonesArray* pJetsLo = new TClonesArray( "TStarJetVectorJet" ); 
  RLo->SetBranchAddress("Jets", &pJetsLo );
  double weightLo=1;
  RLo->SetBranchAddress("weight", &weightLo );

  int njetsLo=0;
  RLo->SetBranchAddress("njets", &njetsLo);
  double zgLo[1000];
  RLo->SetBranchAddress("zg", zgLo);

  int EmbnjetsLo=0;
  RLo->SetBranchAddress("Embnjets", &EmbnjetsLo);
  TClonesArray* pEmbJetsLo = new TClonesArray( "TStarJetVectorJet" );
  RLo->SetBranchAddress("EmbJets", &pEmbJetsLo );
  double EmbzgLo[1000];
  RLo->SetBranchAddress("Embzg", EmbzgLo);
  
  // To match, use runid and eventid
  // --------------------------------
  int runidHi;
  RHi->SetBranchAddress("runid", &runidHi );
  int eventidHi;
  RHi->SetBranchAddress("eventid", &eventidHi );

  // Interesting measurables
  TClonesArray* pJetsHi = new TClonesArray( "TStarJetVectorJet" ); 
  RHi->SetBranchAddress("Jets", &pJetsHi );
  double weightHi=1;
  RHi->SetBranchAddress("weight", &weightHi );

  int njetsHi=0;
  RHi->SetBranchAddress("njets", &njetsHi);
  double zgHi[1000];
  RHi->SetBranchAddress("zg", zgHi);

  int EmbnjetsHi=0;
  RHi->SetBranchAddress("Embnjets", &EmbnjetsHi);
  TClonesArray* pEmbJetsHi = new TClonesArray( "TStarJetVectorJet" ); 
  RHi->SetBranchAddress("EmbJets", &pEmbJetsHi );
  double EmbzgHi[1000];
  RHi->SetBranchAddress("Embzg", EmbzgHi);

  // Output
  // ------
  TString OutFileName = outbase;
  OutFileName.ReplaceAll( ".root", "DijetUnfolded.root");
  OutFileName.Prepend("Results/");
  TFile* fout = new TFile( OutFileName, "RECREATE");
  
  // Histograms
  // ----------
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);
  TH3::SetDefaultSumw2(true);

  // int nPtBinsTrue = 200;
  // float ptminTrue = 0;
  // float ptmaxTrue = 200;
  // int nPtBinsMeas = 200;
  // float ptminMeas = 0;
  // float ptmaxMeas = 200;

  int nPtBinsTrue =  60;
  float ptminTrue =   0;
  float ptmaxTrue =  60;
  int nPtBinsMeas =  50;
  float ptminMeas =  10;
  float ptmaxMeas =  60;

  int nZgBinsTrue = 20;
  float zgminTrue = 0;
  float zgmaxTrue = 0.5;
  int nZgBinsMeas = 20;
  float zgminMeas = 0;
  float zgmaxMeas = 0.5;

  TH3D* LeadTruth3D = new TH3D( "LeadTruth2D", "TRAIN pp;p_{T}^{lead};p_{T}^{sublead};z_{g}^{lead}", nPtBinsTrue, ptminTrue, ptmaxTrue, nPtBinsTrue, ptminTrue, ptmaxTrue, nZgBinsTrue, zgminTrue, zgmaxTrue);
  TH3D* LeadMeas3D = new TH3D( "LeadMeas2D", "TRAIN pp;p_{T}^{lead};p_{T}^{sublead};z_{g}^{lead}", nPtBinsMeas, ptminMeas, ptmaxMeas, nPtBinsMeas, ptminMeas, ptmaxMeas, nZgBinsMeas, zgminMeas, zgmaxMeas);
  
  TH3D* LeadTestTruth3D = new TH3D( "LeadTestTruth2D", "TEST pp;p_{T}^{lead};p_{T}^{sublead};z_{g}^{lead}", nPtBinsTrue, ptminTrue, ptmaxTrue, nPtBinsTrue, ptminTrue, ptmaxTrue, nZgBinsTrue, zgminTrue, zgmaxTrue);
  TH3D* LeadTestMeas3D = new TH3D( "LeadTestMeas2D", "TEST pp;p_{T}^{lead};p_{T}^{sublead};z_{g}^{lead}", nPtBinsMeas, ptminMeas, ptmaxMeas, nPtBinsMeas, ptminMeas, ptmaxMeas, nZgBinsMeas, zgminMeas, zgmaxMeas);
  
  // Helpers
  // -------
  
  // // Set up response matrix
  // // ----------------------
  // RooUnfoldResponse LeadPtResponse ( nPtBinsMeas, ptminMeas, ptmaxMeas, nPtBinsTrue, ptminTrue, ptmaxTrue );
  // RooUnfoldResponse LeadZgResponse ( nZgBinsMeas, zgminMeas, zgmaxMeas, nZgBinsTrue, zgminTrue, zgmaxTrue);

  // 3D unfolding:
  TH3D* hTrueHi= new TH3D ("hTrueHi", "Truth Hi", nPtBinsTrue, ptminTrue, ptmaxTrue, nPtBinsTrue, ptminTrue, ptmaxTrue, nZgBinsTrue, zgminTrue, zgmaxTrue);
  TH3D* hMeasHi= new TH3D ("hMeasHi", "Measured Hi", nPtBinsMeas, ptminMeas, ptmaxMeas, nPtBinsMeas, ptminMeas, ptmaxMeas, nZgBinsMeas, zgminMeas, zgmaxMeas);
  RooUnfoldResponse LeadPtZgResponse3DHi;
  LeadPtZgResponse3DHi.Setup (hMeasHi, hTrueHi );

  TH3D* hTrueLo= new TH3D ("hTrueLo", "Truth Lo", nPtBinsTrue, ptminTrue, ptmaxTrue, nPtBinsTrue, ptminTrue, ptmaxTrue, nZgBinsTrue, zgminTrue, zgmaxTrue);
  TH3D* hMeasLo= new TH3D ("hMeasLo", "Measured Lo", nPtBinsMeas, ptminMeas, ptmaxMeas, nPtBinsMeas, ptminMeas, ptmaxMeas, nZgBinsMeas, zgminMeas, zgmaxMeas);
  RooUnfoldResponse LeadPtZgResponse3DLo;
  LeadPtZgResponse3DLo.Setup (hMeasLo, hTrueLo );

  // Cycle through events
  // --------------------  
  Long64_t NEvents = RHi->GetEntries();
  // NEvents = 10000;
  TStarJetVectorJet* JetLo;
  TStarJetVectorJet* JetHi;
  TStarJetVectorJet* EmbJetLo;
  TStarJetVectorJet* EmbJetHi;

  cout << "Performing analysis." << endl;
  for ( int evi=0; evi<NEvents; ++evi){
    if ( !(evi%10000) ) cout << "Working on " << evi << " / " << NEvents << endl;
    RHi->GetEntry(evi);
    RLo->GetEntryWithIndex( runidHi, eventidHi );

    if ( fabs(weightLo / weightHi - 1) > 1e-4 ) {
      cerr << "weightLo / weightHi = " << weightLo / weightHi  << endl;
      cerr << "weightLo = " <<  weightLo  << "    weightHi = " <<  weightHi << endl;
      cout << "HI: runid = " << runidHi << "  eventid = " << eventidHi << endl;
      cout << "HI: evi = " << evi << endl;
      cout << "LO: evi = " << RLo->GetEntryNumberWithIndex( runidHi, eventidHi) << endl;
      return -1;
    }

    // Leading jets
    JetHi = (TStarJetVectorJet*) pJetsHi->At(0);
    
    // Did we have a sub-leading jet at TRUTH level?
    if ( pJetsHi->GetEntries() < 2 )      continue;
    TStarJetVectorJet* SubLeadingJetHi = (TStarJetVectorJet*) pJetsHi->At(1);
    
    // Back-to-back?
    float dPhiHi = TMath::Pi() - fabs ( SubLeadingJetHi->DeltaPhi( *JetHi ) );
    if ( dPhiHi > openingAngle ) continue;

    TStarJetVectorJet* SubLeadingJetLo = (TStarJetVectorJet*) pJetsLo->At(1);
    // Matched?
    if ( pJetsLo->GetEntries() < 2 )      continue;
    JetLo = (TStarJetVectorJet*) pJetsLo->At(0);

    TStarJetVectorJet* LeadingMatch=0;
    TStarJetVectorJet* SubLeadingMatch=0;
    if ( JetHi->DeltaR( *JetLo ) < MatchingR ) LeadingMatch = JetLo;
    if ( JetHi->DeltaR( *SubLeadingJetLo ) < MatchingR ) LeadingMatch = SubLeadingJetLo;
    if ( SubLeadingJetHi->DeltaR( *JetLo ) < MatchingR ) SubLeadingMatch = JetLo;
    if ( SubLeadingJetHi->DeltaR( *SubLeadingJetLo ) < MatchingR ) SubLeadingMatch = SubLeadingJetLo;

    if ( !SubLeadingMatch || !LeadingMatch ){
      // cout << "Couldn't match both sides" << endl;
      continue;
    }
    
    // ---------------------------------
    // Found a dijet pair at TRUTH level
    // ---------------------------------
    // IMPORTANT: Have NOT yet imposed pT cut
    // For the time being, let's not
    // because in the analysis that cut happens on the measured level
    float t_lead_pt_hi = JetHi->Pt();
    float t_sublead_pt_hi = SubLeadingJetHi->Pt();
    float t_lead_zg_hi = zgHi[0];
    float t_sublead_zg_hi = zgHi[1];

    float t_lead_pt_lo = JetLo->Pt();
    float t_sublead_pt_lo = SubLeadingJetLo->Pt();
    float t_lead_zg_lo = zgLo[0];
    float t_sublead_zg_lo = zgLo[1];

    // float LeadMin = 20;
    // float SubLeadMin = 10;

    // FIXME: In the following, there are multiple ways to miss which
    // FIXME: are all folded into the same "miss" routine.

    // Do we still have a dijet pair?    
    if ( pEmbJetsHi->GetEntries() < 2 ) {
      LeadPtZgResponse3DHi.Miss( t_lead_pt_hi, t_sublead_pt_hi, t_lead_zg_hi, weightHi );
      continue;
    }
    //    cout << "hello" << endl;

    // FIXME: The following probably doesn't make much sense...
    if ( pEmbJetsLo->GetEntries() < 2 ) {
      LeadPtZgResponse3DLo.Miss( t_lead_pt_lo, t_sublead_pt_lo, t_lead_zg_lo, weightLo );
      continue;
    }

    EmbJetHi = (TStarJetVectorJet*) pEmbJetsHi->At(0);

    TStarJetVectorJet* SubLeadingEmbJetHi = (TStarJetVectorJet*) pEmbJetsHi->At(1);
    float EmbdPhiHi = TMath::Pi() - fabs ( SubLeadingEmbJetHi->DeltaPhi( *EmbJetHi ) );
    if ( EmbdPhiHi > openingAngle ) {
      LeadPtZgResponse3DHi.Miss( t_lead_pt_hi, t_sublead_pt_hi, t_lead_zg_hi, weightHi );
      continue;
    }

    if ( pEmbJetsLo->GetEntries() < 2 ){
      LeadPtZgResponse3DLo.Miss( t_lead_pt_lo, t_sublead_pt_lo, t_lead_zg_lo, weightLo );
      continue;
    }
    
    EmbJetLo = (TStarJetVectorJet*) pEmbJetsLo->At(0);
    TStarJetVectorJet* SubLeadingEmbJetLo = (TStarJetVectorJet*) pEmbJetsLo->At(1);

    TStarJetVectorJet* EmbLeadingMatch=0;
    TStarJetVectorJet* EmbSubLeadingMatch=0;
    if ( EmbJetHi->DeltaR( *EmbJetLo ) < MatchingR ) LeadingMatch = JetLo;
    if ( EmbJetHi->DeltaR( *SubLeadingEmbJetLo ) < MatchingR ) EmbLeadingMatch = SubLeadingEmbJetLo;
    if ( SubLeadingEmbJetHi->DeltaR( *EmbJetLo ) < MatchingR ) EmbSubLeadingMatch = EmbJetLo;
    if ( SubLeadingEmbJetHi->DeltaR( *SubLeadingEmbJetLo ) < MatchingR ) EmbSubLeadingMatch = SubLeadingEmbJetLo;

    if ( !EmbSubLeadingMatch || !EmbLeadingMatch ){
      LeadPtZgResponse3DHi.Miss( t_lead_pt_hi, t_sublead_pt_hi, t_lead_zg_hi, weightHi );
      LeadPtZgResponse3DLo.Miss( t_lead_pt_lo, t_sublead_pt_lo, t_lead_zg_lo, weightLo );
      // cout << "Couldn't match both sides" << endl;
      continue;
    }

    // TEST:
    float m_lead_pt_hi = EmbJetHi->Pt();
    float m_sublead_pt_hi = SubLeadingEmbJetHi->Pt();
    float m_lead_zg_hi = EmbzgHi[0];
    float m_sublead_zg_hi = EmbzgHi[1];

    float m_lead_pt_lo = EmbJetLo->Pt();
    float m_sublead_pt_lo = SubLeadingEmbJetLo->Pt();
    float m_lead_zg_lo = EmbzgLo[0];
    float m_sublead_zg_lo = EmbzgLo[1];

    float m_LeadMin = 20;
    float m_SubLeadMin = 10;
    if ( m_lead_pt_hi < m_LeadMin || m_sublead_pt_hi < m_SubLeadMin ) continue;
    cout << "Got something!" << endl;
    cout << "Truth:" << endl;
    cout << "  HI cut:" << endl;
    cout << "     pT    lead = " << t_lead_pt_hi << endl;
    cout << "     pT sublead = " << t_sublead_pt_hi << endl;
    cout << "     zg    lead = " << t_lead_zg_hi << endl;
    
    cout << "  LO cut, matched:" << endl;
    cout << "     pT    lead = " << t_lead_pt_lo << endl;
    cout << "     pT sublead = " << t_sublead_pt_lo << endl;
    cout << "     zg    lead = " << t_lead_zg_lo << endl;

    // float t_lead_pt_lo = LeadingMatch->Pt();
    // if ( t_lead_pt_lo < ptminTrue || t_lead_pt_lo > ptmaxTrue ) continue;
    


    // // Start with Leading jet
    // // ----------------------
    // if ( pJets->GetEntries() ==0 ) continue;
    // int tjet = 0; // Prepare for running over multiple jets. Using [0] for now
    // TStarJetVectorJet* TrueJet = (TStarJetVectorJet*) pJets->At(tjet);
    // float t_lead_pt = TrueJet->Pt();
    
    // if ( t_lead_pt < ptminTrue || t_lead_pt > ptmaxTrue ) continue;

    // // Can we find it in embedding?
    // // ----------------------------
    // TStarJetVectorJet* MatchedJet = 0;
    // int MatchIndex=-1;
    // for (int i=0; i<pEmbJets->GetEntries(); ++i ){
    //   if ( ((TStarJetVectorJet*) pEmbJets->At(i))->GetMatch() == tjet ){
    // 	MatchedJet = (TStarJetVectorJet*) pEmbJets->At(i);
    // 	MatchIndex=i;
    //   }
    // }
    
    // // pT cuts on the TRUTH level
    // // --------------------------
    // // if ( t_lead_pt < LeadMin || t_lead_pt > LeadMax ) continue;

    // // Use half for training, half for testing
    // if ( ev %2 ){
    //   // ======
    //   // TRAIN
    //   // ======

    //   // ======
    //   // PYTHIA
    //   // ======
    //   Truth2D->Fill( t_lead_pt, zg[tjet], weight );
      
    //   // ============
    //   // PYTHIA in MC
    //   // ============
    //   // find pt bins
    //   int ptCoarse = -1;
    //   if ( t_lead_pt >= CoarsePtBinBounds[nCoarsePt] ||
    // 	   t_lead_pt >= FinePtBinBounds[nFinePt] ){
    // 	cerr << "t_lead_pt = " << t_lead_pt << " is out of range" << endl;
    // 	return -1;
    //   }
      
    //   for ( int i=nCoarsePt-1; i>=0 ; --i ){
    // 	if ( t_lead_pt >= CoarsePtBinBounds[i] ){
    // 	  ptCoarse = i;
    // 	  break;
    // 	}
    //   }
    //   if ( ptCoarse <0 ){
    // 	cerr << "Could not find coarse pt bin" << endl;
    // 	return -1;
    //   }
    //   int ptFine = -1;
    //   for ( int i=nFinePt-1; i>=0 ; --i ){
    // 	if ( t_lead_pt >= FinePtBinBounds[i] ){
    // 	  ptFine = i;
    // 	  break;
    // 	}
    //   }
    //   if ( ptFine <0 ){
    // 	cerr << "Could not find fine pt bin" << endl;
    // 	return -1;
    //   }
      
    //   if ( MatchedJet ){
    // 	Meas2D->Fill( MatchedJet->Pt(), Embzg[MatchIndex], weight );
    // 	LeadPtResponse.Fill( MatchedJet->Pt(), t_lead_pt, weight );
    // 	LeadZgResponse.Fill( Embzg[MatchIndex], zg[tjet], weight );
    // 	LeadPtZgResponse2D.Fill( MatchedJet->Pt(), Embzg[MatchIndex], t_lead_pt, zg[tjet], weight );

    // 	CoarseLeadZgResponse[ptCoarse].Fill( Embzg[MatchIndex], zg[tjet], weight );
    // 	// cout << "filling " << ptFine << endl;
    // 	// cout << "t_lead_pt = " << t_lead_pt << "  Bin boundaries " << FinePtBinBounds[ptFine] << " -- " << FinePtBinBounds[ptFine+1] << endl;
    // 	FineLeadZgResponse[ptFine].Fill( Embzg[MatchIndex], zg[tjet], weight );

    // 	// int ptEmbCoarse = -1;
    // 	// if ( t_lead_pt >= EmbCoarsePtBinBounds[nEmbCoarsePt] ||
    // 	//      t_lead_pt >= FinePtBinBounds[nFinePt] ){
    // 	//   cerr << "t_lead_pt = " << t_lead_pt << " is out of range" << endl;
    // 	//   return -1;
    // 	// }
	
    // 	// for ( int i=nEmbCoarsePt-1; i>=0 ; --i ){
    // 	//   if ( t_lead_pt >= EmbCoarsePtBinBounds[i] ){
    // 	//     ptEmbCoarse = i;
    // 	//     break;
    // 	//   }
    // 	// }
    // 	// if ( ptEmbCoarse <0 ){
    // 	//   cerr << "Could not find coarse pt bin" << endl;
    // 	//   return -1;
    // 	// }

      
    //   } else {
    // 	// Miss
    // 	LeadPtResponse.Miss( t_lead_pt, weight );
    // 	LeadZgResponse.Miss( zg[tjet], weight );
    // 	LeadPtZgResponse2D.Miss( t_lead_pt, zg[tjet], weight );

    // 	CoarseLeadZgResponse[ptCoarse].Miss( zg[tjet], weight );
    // 	FineLeadZgResponse[ptFine].Miss( zg[tjet], weight );
    //   }
    // } else {
    //   // ======
    //   // TEST
    //   // ======
    //   // ======
    //   // PYTHIA
    //   // ======
    //   TestTruth2D->Fill( t_lead_pt, zg[tjet], weight );
      
    //   // ============
    //   // PYTHIA in MC
    //   // ============
    //   if ( MatchedJet ){
    // 	TestMeas2D->Fill( MatchedJet->Pt(), Embzg[MatchIndex], weight );
    //   } else {
    // 	// Miss
    //   }

    // }
        
  } // NEvents

  // Close up shop
  // -------------
  fout->cd();
  LeadPtZgResponse3DHi.SetName("LeadPtZgResponse3DHi");
  LeadPtZgResponse3DHi.Write();
  LeadPtZgResponse3DLo.SetName("LeadPtZgResponse3DLo");
  LeadPtZgResponse3DLo.Write();


  fout->Write();

  cout << "Wrote to " << fout->GetName() << endl;
  cout << "Bye." << endl;
  
  return 0;
  
}
