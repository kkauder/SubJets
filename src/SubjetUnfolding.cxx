/** @file SubjetUnfolding.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Play with unfolding of pythia in MC subjets
    @date Mar 04, 2015
*/

// #include "AjParameters.hh"
// #include "ktTrackEff.hh"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#include "RooUnfoldSvd.h"
#include "RooUnfoldTUnfold.h"

#include "TStarJetVectorContainer.h"
#include "TStarJetVector.h"
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

  // Parameters
  // ----------
  float LeadMin = 20;
  float LeadMax = 30;

  // Load input
  // ----------
  // TString InputName = "Results/Subjets_PythiaAndMc_0.root";
  TString InputName = "Results/OldWrong_Subjets_PythiaAndMc_0.root";
  TChain* InputChain = new TChain("ResultTree");
  InputChain->Add(InputName);
  assert ( InputChain->GetEntries()>0 && "Something went wrong loading input.");
  
  // TLorentzVector* LeadingJet;
  // TLorentzVector* SubLeadingJet;
  // InputChain->SetBranchAddress("LeadingJet", &LeadingJet);
  // // InputChain->SetBranchAddress("SubLeadingJet", &SubLeadingJet);

  TLorentzVector* LeadingJet = new TLorentzVector();
  InputChain->SetBranchAddress("LeadingJet", &LeadingJet);
  TLorentzVector* SubLeadingJet = new TLorentzVector();
  InputChain->SetBranchAddress("SubLeadingJet", &SubLeadingJet);

  TClonesArray* pLeadingSubjets = 0;
  pLeadingSubjets = new TClonesArray("TStarJetVector");
  InputChain->SetBranchAddress("LeadingSubjets", &pLeadingSubjets);

  TClonesArray* pSubLeadingSubjets = 0;
  pSubLeadingSubjets = new TClonesArray("TStarJetVector");
  InputChain->SetBranchAddress("SubLeadingSubjets", &pSubLeadingSubjets);

  TLorentzVector* EmbLeadingJet = new TLorentzVector();
  InputChain->SetBranchAddress("EmbLeadingJet", &EmbLeadingJet);
  TLorentzVector* EmbSubLeadingJet = new TLorentzVector();
  InputChain->SetBranchAddress("EmbSubLeadingJet", &EmbSubLeadingJet);

  TClonesArray* pEmbLeadingSubjets = 0;
  pEmbLeadingSubjets = new TClonesArray("TStarJetVector");
  InputChain->SetBranchAddress("EmbLeadingSubjets", &pEmbLeadingSubjets);
  TClonesArray* pEmbSubLeadingSubjets = 0;
  pEmbSubLeadingSubjets = new TClonesArray("TStarJetVector");
  InputChain->SetBranchAddress("EmbSubLeadingSubjets", &pEmbSubLeadingSubjets);

  // Output
  // ------
  TString OutFileName = "Results/SubjetUnfoldingTest.root";
  TFile* fout = new TFile( OutFileName, "RECREATE");
  
  // Histograms
  // ----------
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);

  TH1D* Truth_LeadingZ1 = new TH1D( "Truth_LeadingZ1", "p_{T}^{sj1} / p_{T}^{jet}, Truth", 110, 0, 1.1);
  TH1D* Truth_LeadingZ2 = new TH1D( "Truth_LeadingZ2", "p_{T}^{sj2} / p_{T}^{jet}, Truth", 110, 0, 1.1);
  TH1D* Truth_DeltaS = new TH1D( "Truth_DeltaS", "p_{T}^{sj1} - p_{T}^{sj2}, Truth", 110, 0, 55);
  TH1D* Truth_DeltaZ = new TH1D( "Truth_DeltaZ", "(p_{T}^{sj1} - p_{T}^{sj2}) / p_{T}^{jet}, Truth", 110, 0, 1);
  
  TH1D* MC_LeadingZ1 = new TH1D( "MC_LeadingZ1", "p_{T}^{sj1} / p_{T}^{jet}, Pythia in MC", 110, 0, 1.1);
  TH1D* MC_LeadingZ2 = new TH1D( "MC_LeadingZ2", "p_{T}^{sj2} / p_{T}^{jet}, Pythia in MC", 110, 0, 1.1);
  TH1D* MC_DeltaS = new TH1D( "MC_DeltaS", "p_{T}^{sj1} - p_{T}^{sj2}, Pythia in MC", 110, 0, 55);
  TH1D* MC_DeltaZ = new TH1D( "MC_DeltaZ", "(p_{T}^{sj1} - p_{T}^{sj2}) / p_{T}^{jet}, Pythia in MC", 110, 0, 1);
  
  // TH2D* RecoVsTruePt = new TH2D( "RecoVsTruePt","Reco Vs True p_{T};p_{T}^{true} [GeV/c];p_{T}^{reco} [GeV/c]", 60, 0 , 60, 120, 0, 60 );
  // TH1D* TestTruePt = new TH1D( "TestTruePt","TestTruePt;p_{T}^{true} [GeV/c]", 60, 0 , 60);
  // TH1D* TestDistortedPt = new TH1D( "TestDistortedPt","TestDistortedPt;p_{T}^{reco} [GeV/c]", 120, 0 , 60);

  // Helpers
  // -------
  // vector<PseudoJet> TrueParticles;
  // vector<PseudoJet> DistortedParticles;
  int nJets = 0;
  
  // Set up response matrix
  // ----------------------
  RooUnfoldResponse LeadPtResponse (120, 0 , 60, 60, 0 , 60);

  // Cycle through events
  // --------------------  
  int nEvents=InputChain->GetEntries();
  // nEvents=100;
  TStarJetVector* sv;
  cout << "Performing analysis." << endl;

  for ( int ev=0; ev<nEvents  ; ++ev ){
    if ( !(ev%1000) ) cerr << "Event " << ev << " / " << nEvents << endl;

    // TrueParticles.clear();
    // DistortedParticles.clear();
    LeadingJet->SetPtEtaPhiM(0,0,0,0);
    pLeadingSubjets->Clear();

    EmbLeadingJet->SetPtEtaPhiM(0,0,0,0);
    pEmbLeadingSubjets->Clear();

    InputChain->GetEntry(ev);

    // pT cuts on the TRUTH level
    // --------------------------
    float t_lead_pt = LeadingJet->Pt();
    if ( t_lead_pt < LeadMin || t_lead_pt > LeadMax ) continue;
    
    // ======
    // PYTHIA
    // ======
    if ( pLeadingSubjets->GetEntries() >0 ){
      float t_lead_sub1_pt = ((TStarJetVector*) pLeadingSubjets->At(0))->Pt();
      Truth_LeadingZ1->Fill( t_lead_sub1_pt / t_lead_pt );
    } // else ?

    if ( pLeadingSubjets->GetEntries() >1 ){
      float t_lead_sub1_pt = ((TStarJetVector*) pLeadingSubjets->At(0))->Pt();
      float t_lead_sub2_pt = ((TStarJetVector*) pLeadingSubjets->At(1))->Pt();
      Truth_LeadingZ2->Fill( t_lead_sub2_pt / t_lead_pt );
      Truth_DeltaS->Fill( t_lead_sub1_pt - t_lead_sub2_pt );
      Truth_DeltaZ->Fill( (t_lead_sub1_pt - t_lead_sub2_pt) / t_lead_pt );
    } // else ?
    
    // ============
    // PYTHIA in MC
    // ============
    float mc_lead_pt = EmbLeadingJet->Pt();

    if ( pEmbLeadingSubjets->GetEntries() > 0) {
      float mc_lead_sub1_pt = ((TStarJetVector*) pEmbLeadingSubjets->At(0))->Pt();
      MC_LeadingZ1->Fill( mc_lead_sub1_pt / mc_lead_pt );
    } // else?

    if ( pEmbLeadingSubjets->GetEntries() > 1) {
      float mc_lead_sub1_pt = ((TStarJetVector*) pEmbLeadingSubjets->At(0))->Pt();
      float mc_lead_sub2_pt = ((TStarJetVector*) pEmbLeadingSubjets->At(1))->Pt();
      MC_LeadingZ2->Fill( mc_lead_sub2_pt / mc_lead_pt );
      MC_DeltaS->Fill( mc_lead_sub1_pt - mc_lead_sub2_pt );
      MC_DeltaZ->Fill( (mc_lead_sub1_pt - mc_lead_sub2_pt) / mc_lead_pt );
    } // else?
    
    // // RECO:
    // // -----
    // ClusterSequence csaHiDistorted ( pHiDistorted, jet_def );
    // vector<PseudoJet> HiResultDistorted = fastjet::sorted_by_pt( sjet ( csaHiDistorted.inclusive_jets() ) );
    // if ( HiResultDistorted.size() < 1 ){
    //   cerr << "DISTORTED: NOTHING FOUND." << endl;
    //   if ( ev %2 )       response.Miss ( HiResultTrue.at(0).pt() );
    //   continue;
    // }
    
    // // find MATCHING jet
    // // -----------------
    // fastjet::Selector SelectClose = fastjet::SelectorCircle( R );
    // SelectClose.set_reference( HiResultTrue.at(0) );
    // std::vector<fastjet::PseudoJet> MatchedToTruth = sorted_by_pt(SelectClose( HiResultDistorted ));
    // if ( MatchedToTruth.size() < 1 ){
    //   cerr << "DISTORTED: NOTHING MATCHED." << endl;
    //   if ( ev %2 )       response.Miss ( HiResultTrue.at(0).pt() );
    //   continue;
    // }

    // // Use half for training, half for testing
    // if ( ev %2 ){
    //   RecoVsTruePt->Fill( HiResultTrue.at(0).pt(), MatchedToTruth.at(0).pt() );
    //   response.Fill( MatchedToTruth.at(0).pt(), HiResultTrue.at(0).pt() );
    // } else {
    //   TestTruePt->Fill( HiResultTrue.at(0).pt() );
    //   TestDistortedPt->Fill( MatchedToTruth.at(0).pt() );
    // }

    
  } // nEvents
    

  // Close up shop
  // -------------
  // response.SetName("response");
  fout->cd();
  // response.Write();
  fout->Write();

  // cout << "In " << nEvents << " events, found " << endl
  //      << nHardDijets << " dijets with constituents above 2 GeV," << endl
  //      << nCorrespondingLowDijets << " corresponding dijets with constituents above 0.2 GeV," << endl
  //      << " of which " <<  nMatchedDijets << " could be matched." << endl;

  cout << "Wrote to " << fout->GetName() << endl;
  cout << "Bye." << endl;
  
  return 0;
  
}
