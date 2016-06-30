/** @file GroomUnfolding.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Play with unfolding of pythia in MC subjets
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
  // float LeadMin = 20;
  // float LeadMax = 30;

  // Load input
  // ----------
  TString InputName = "Results/Groom_Rhic.root";
  // TString InputName = "Results/Groom_Lhc.root";
  TChain* ResultTree = new TChain("ResultTree");
  ResultTree->Add(InputName);
  assert ( ResultTree->GetEntries()>0 && "Something went wrong loading input.");
  
  TClonesArray* pJets = new TClonesArray( "TStarJetVectorJet" ); 
  ResultTree->SetBranchAddress("Jets", &pJets );
  TClonesArray* pGroomedJets = new TClonesArray( "TStarJetVectorJet" ); 
  ResultTree->SetBranchAddress("GroomedJets", &pGroomedJets );

  TClonesArray* pEmbJets = new TClonesArray( "TStarJetVectorJet" ); 
  ResultTree->SetBranchAddress("EmbJets", &pEmbJets );
  TClonesArray* pEmbGroomedJets = new TClonesArray( "TStarJetVectorJet" ); 
  ResultTree->SetBranchAddress("EmbGroomedJets", &pEmbGroomedJets );

  double weight=1;
  ResultTree->SetBranchAddress("weight", &weight );

  int njets=0;
  ResultTree->SetBranchAddress("njets", &njets);
  double zg[1000];
  ResultTree->SetBranchAddress("zg", zg);

  int Embnjets=0;
  ResultTree->SetBranchAddress("Embnjets", &Embnjets);
  double Embzg[1000];
  ResultTree->SetBranchAddress("Embzg", Embzg);

  Long64_t NEvents = ResultTree->GetEntries();
  // NEvents = 10000;
  
  // Output
  // ------
  TString OutFileName = "Results/GroomUnfoldingTest.root";
  TFile* fout = new TFile( OutFileName, "RECREATE");
  
  // Histograms
  // ----------
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);

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

  // int nPtBinsTrue = 50;
  // float ptminTrue = 50;
  // float ptmaxTrue = 100;
  // int nPtBinsMeas = 150;
  // float ptminMeas = 0;
  // float ptmaxMeas = 150;

  int nZgBinsTrue = 20;
  float zgminTrue = 0;
  float zgmaxTrue = 0.5;
  int nZgBinsMeas = 20;
  float zgminMeas = 0;
  float zgmaxMeas = 0.5;

  TH2D* Truth2D = new TH2D( "Truth2D", "TRAIN z_{g}^{lead} vs. p_{T}^{lead}, Pythia", nPtBinsTrue, ptminTrue, ptmaxTrue, nZgBinsTrue, zgminTrue, zgmaxTrue);
  TH2D* Meas2D  = new TH2D( "Meas2D", "TRAIN z_{g}^{lead} vs. p_{T}^{lead}, Pythia in MC", nPtBinsMeas, ptminMeas, ptmaxMeas, nZgBinsMeas, zgminMeas, zgmaxMeas);
  TH2D* TestTruth2D = new TH2D( "TestTruth2D", "TEST z_{g}^{lead} vs. p_{T}^{lead}, Pythia", nPtBinsTrue, ptminTrue, ptmaxTrue, nZgBinsTrue, zgminTrue, zgmaxTrue);
  TH2D* TestMeas2D  = new TH2D( "TestMeas2D", "TEST z_{g}^{lead} vs. p_{T}^{lead}, Pythia in MC", nPtBinsMeas, ptminMeas, ptmaxMeas, nZgBinsMeas, zgminMeas, zgmaxMeas);
  
  // Helpers
  // -------
  // vector<PseudoJet> TrueParticles;
  // vector<PseudoJet> DistortedParticles;
  // int nJets = 0;
  
  // Set up response matrix
  // ----------------------
  RooUnfoldResponse LeadPtResponse ( nPtBinsMeas, ptminMeas, ptmaxMeas, nPtBinsTrue, ptminTrue, ptmaxTrue );
  RooUnfoldResponse LeadZgResponse ( nZgBinsMeas, zgminMeas, zgmaxMeas, nZgBinsTrue, zgminTrue, zgmaxTrue);

  // 2D unfolding:
  TH2D* hTrue= new TH2D ("hTrue", "Truth", nPtBinsTrue, ptminTrue, ptmaxTrue, nZgBinsTrue, zgminTrue, zgmaxTrue);
  TH2D* hMeas= new TH2D ("hMeas", "Measured", nPtBinsMeas, ptminMeas, ptmaxMeas, nZgBinsMeas, zgminMeas, zgmaxMeas);
  RooUnfoldResponse LeadPtZgResponse2D;
  LeadPtZgResponse2D.Setup (hMeas, hTrue );

  // Alternative: Two step unfolding:
  // --------------------------------
  // Have an array of Zg responses in N bins of pT_Truth
  // then unfold pT in 1D,
  // then unfold the zg spectrum N times with the different responses and
  // combine them by the weights obtained from the first steps
  // Trying coarse (5 GeV bins) and fine (1 GeV) bins.
  // Fine is obviously  preferrable but may very well be statistics-limited
  
  TString NameString;
  float dPtCoarse = 5;
  TObjArray ToaCoarseLeadZgResponse;
  int nCoarsePt = (ptmaxTrue - ptminTrue) / dPtCoarse;
  if ( fabs ( ptminTrue + nCoarsePt*dPtCoarse - ptmaxTrue ) > 1e-3 ){
    cerr << "Coarse Boundaries incompatible" << endl;
    return -1;
  }
  double *CoarsePtBinBounds = new double[nCoarsePt+1];
  RooUnfoldResponse* CoarseLeadZgResponse  = new RooUnfoldResponse[nCoarsePt];
  for ( int i=0 ; i<=nCoarsePt; ++ i ){
    CoarsePtBinBounds[i] = ptminTrue + i*dPtCoarse;
    if ( i<nCoarsePt ){
      CoarseLeadZgResponse[i].Setup (nZgBinsMeas, zgminMeas, zgmaxMeas, nZgBinsTrue, zgminTrue, zgmaxTrue);
      NameString = "CoarseLeadZgResponse_"; NameString+=i;
      CoarseLeadZgResponse[i].SetName(NameString);
      ToaCoarseLeadZgResponse.Add( &(CoarseLeadZgResponse[i]));
    }
  }
  TArrayD aCoarsePtBinBounds (nCoarsePt+1,  &(CoarsePtBinBounds[0]));
  
  
  float dPtFine = 1;
  TObjArray ToaFineLeadZgResponse;
  int nFinePt = (ptmaxTrue - ptminTrue) / dPtFine;
  if ( fabs ( ptminTrue + nFinePt*dPtFine - ptmaxTrue ) > 1e-3 ){
    cerr << "Fine Boundaries incompatible" << endl;
    return -1;
  }
  double *FinePtBinBounds = new double[nFinePt+1];
  RooUnfoldResponse* FineLeadZgResponse  = new RooUnfoldResponse[nFinePt];
  for ( int i=0 ; i<=nFinePt; ++ i ){
    FinePtBinBounds[i] = ptminTrue + i*dPtFine;
    if ( i<nFinePt ){
      FineLeadZgResponse[i].Setup (nZgBinsMeas, zgminMeas, zgmaxMeas, nZgBinsTrue, zgminTrue, zgmaxTrue);
      NameString = "FineLeadZgResponse_"; NameString+=i;
      FineLeadZgResponse[i].SetName(NameString);
      ToaFineLeadZgResponse.Add( &(FineLeadZgResponse[i]));
    }
  }
  TArrayD aFinePtBinBounds (nFinePt+1,  &(FinePtBinBounds[0]));
    

  // Update: Let's also try that in bins of pt_meas :-/
  // --- Wait no, that doesn't work. That way we cannot collect misses.
  // float dPtEmbCoarse = 5;
  // TObjArray ToaEmbCoarseLeadZgResponse;
  // int nEmbCoarsePt = (ptmaxMeas - ptminMeas) / dPtEmbCoarse;
  // if ( fabs ( ptminTrue + nEmbCoarsePt*dPtEmbCoarse - ptmaxTrue ) > 1e-3 ){
  //   cerr << "EmbCoarse Boundaries incompatible" << endl;
  //   return -1;
  // }
  // double *EmbCoarsePtBinBounds = new double[nEmbCoarsePt+1];
  // RooUnfoldResponse* EmbCoarseLeadZgResponse  = new RooUnfoldResponse[nEmbCoarsePt];
  // for ( int i=0 ; i<=nEmbCoarsePt; ++ i ){
  //   EmbCoarsePtBinBounds[i] = ptminMeas + i*dPtEmbCoarse;
  //   if ( i<nEmbCoarsePt ){
  //     EmbCoarseLeadZgResponse[i].Setup (nZgBinsMeas, zgminMeas, zgmaxMeas, nZgBinsTrue, zgminTrue, zgmaxTrue);
  //     NameString = "EmbCoarseLeadZgResponse_"; NameString+=i;
  //     EmbCoarseLeadZgResponse[i].SetName(NameString);
  //     ToaEmbCoarseLeadZgResponse.Add( &(EmbCoarseLeadZgResponse[i]));
  //   }
  // }
  // TArrayD aEmbCoarsePtBinBounds (nEmbCoarsePt+1,  &(EmbCoarsePtBinBounds[0]));

  // float dPtEmbFine = 5;
  // TObjArray ToaEmbFineLeadZgResponse;
  // int nEmbFinePt = (ptmaxMeas - ptminMeas) / dPtEmbFine;
  // if ( fabs ( ptminTrue + nEmbFinePt*dPtEmbFine - ptmaxTrue ) > 1e-3 ){
  //   cerr << "EmbFine Boundaries incompatible" << endl;
  //   return -1;
  // }
  // double *EmbFinePtBinBounds = new double[nEmbFinePt+1];
  // RooUnfoldResponse* EmbFineLeadZgResponse  = new RooUnfoldResponse[nEmbFinePt];
  // for ( int i=0 ; i<=nEmbFinePt; ++ i ){
  //   EmbFinePtBinBounds[i] = ptminMeas + i*dPtEmbFine;
  //   if ( i<nEmbFinePt ){
  //     EmbFineLeadZgResponse[i].Setup (nZgBinsMeas, zgminMeas, zgmaxMeas, nZgBinsTrue, zgminTrue, zgmaxTrue);
  //     NameString = "EmbFineLeadZgResponse_"; NameString+=i;
  //     EmbFineLeadZgResponse[i].SetName(NameString);
  //     ToaEmbFineLeadZgResponse.Add( &(EmbFineLeadZgResponse[i]));
  //   }
  // }
  // TArrayD aEmbFinePtBinBounds (nEmbFinePt+1,  &(EmbFinePtBinBounds[0]));














  // Cycle through events
  // --------------------  
  TStarJetVectorJet* sv;
  cout << "Performing analysis." << endl;

  for ( int ev=0; ev<NEvents  ; ++ev ){
    if ( !(ev%1000) ) cerr << "Event " << ev << " / " << NEvents << endl;

    pJets->Clear();
    pEmbJets->Clear();
    ResultTree->GetEntry(ev);

    // Start with Leading jet
    // ----------------------
    if ( pJets->GetEntries() ==0 ) continue;
    int tjet = 0; // Prepare for running over multiple jets. Using [0] for now
    TStarJetVectorJet* TrueJet = (TStarJetVectorJet*) pJets->At(tjet);
    float t_lead_pt = TrueJet->Pt();
    
    if ( t_lead_pt < ptminTrue || t_lead_pt > ptmaxTrue ) continue;

    // Can we find it in embedding?
    // ----------------------------
    TStarJetVectorJet* MatchedJet = 0;
    int MatchIndex=-1;
    for (int i=0; i<pEmbJets->GetEntries(); ++i ){
      if ( ((TStarJetVectorJet*) pEmbJets->At(i))->GetMatch() == tjet ){
	MatchedJet = (TStarJetVectorJet*) pEmbJets->At(i);
	MatchIndex=i;
      }
    }
    
    // pT cuts on the TRUTH level
    // --------------------------
    // if ( t_lead_pt < LeadMin || t_lead_pt > LeadMax ) continue;

    // Use half for training, half for testing
    if ( ev %2 ){
      // ======
      // TRAIN
      // ======

      // ======
      // PYTHIA
      // ======
      Truth2D->Fill( t_lead_pt, zg[tjet], weight );
      
      // ============
      // PYTHIA in MC
      // ============
      // find pt bins
      int ptCoarse = -1;
      if ( t_lead_pt >= CoarsePtBinBounds[nCoarsePt] ||
	   t_lead_pt >= FinePtBinBounds[nFinePt] ){
	cerr << "t_lead_pt = " << t_lead_pt << " is out of range" << endl;
	return -1;
      }
      
      for ( int i=nCoarsePt-1; i>=0 ; --i ){
	if ( t_lead_pt >= CoarsePtBinBounds[i] ){
	  ptCoarse = i;
	  break;
	}
      }
      if ( ptCoarse <0 ){
	cerr << "Could not find coarse pt bin" << endl;
	return -1;
      }
      int ptFine = -1;
      for ( int i=nFinePt-1; i>=0 ; --i ){
	if ( t_lead_pt >= FinePtBinBounds[i] ){
	  ptFine = i;
	  break;
	}
      }
      if ( ptFine <0 ){
	cerr << "Could not find fine pt bin" << endl;
	return -1;
      }
      
      if ( MatchedJet ){
	Meas2D->Fill( MatchedJet->Pt(), Embzg[MatchIndex], weight );
	LeadPtResponse.Fill( MatchedJet->Pt(), t_lead_pt, weight );
	LeadZgResponse.Fill( Embzg[MatchIndex], zg[tjet], weight );
	LeadPtZgResponse2D.Fill( MatchedJet->Pt(), Embzg[MatchIndex], t_lead_pt, zg[tjet], weight );

	CoarseLeadZgResponse[ptCoarse].Fill( Embzg[MatchIndex], zg[tjet], weight );
	// cout << "filling " << ptFine << endl;
	// cout << "t_lead_pt = " << t_lead_pt << "  Bin boundaries " << FinePtBinBounds[ptFine] << " -- " << FinePtBinBounds[ptFine+1] << endl;
	FineLeadZgResponse[ptFine].Fill( Embzg[MatchIndex], zg[tjet], weight );

	// int ptEmbCoarse = -1;
	// if ( t_lead_pt >= EmbCoarsePtBinBounds[nEmbCoarsePt] ||
	//      t_lead_pt >= FinePtBinBounds[nFinePt] ){
	//   cerr << "t_lead_pt = " << t_lead_pt << " is out of range" << endl;
	//   return -1;
	// }
	
	// for ( int i=nEmbCoarsePt-1; i>=0 ; --i ){
	//   if ( t_lead_pt >= EmbCoarsePtBinBounds[i] ){
	//     ptEmbCoarse = i;
	//     break;
	//   }
	// }
	// if ( ptEmbCoarse <0 ){
	//   cerr << "Could not find coarse pt bin" << endl;
	//   return -1;
	// }

      
      } else {
	// Miss
	LeadPtResponse.Miss( t_lead_pt, weight );
	LeadZgResponse.Miss( zg[tjet], weight );
	LeadPtZgResponse2D.Miss( t_lead_pt, zg[tjet], weight );

	CoarseLeadZgResponse[ptCoarse].Miss( zg[tjet], weight );
	FineLeadZgResponse[ptFine].Miss( zg[tjet], weight );
      }
    } else {
      // ======
      // TEST
      // ======
      // ======
      // PYTHIA
      // ======
      TestTruth2D->Fill( t_lead_pt, zg[tjet], weight );
      
      // ============
      // PYTHIA in MC
      // ============
      if ( MatchedJet ){
	TestMeas2D->Fill( MatchedJet->Pt(), Embzg[MatchIndex], weight );
      } else {
	// Miss
      }

    }
        
  } // NEvents

  // Close up shop
  // -------------
  LeadPtResponse.SetName("LeadPtResponse");
  LeadZgResponse.SetName("LeadZgResponse");
  LeadPtZgResponse2D.SetName("LeadPtZgResponse2D");
  fout->cd();
  LeadPtResponse.Write();
  LeadZgResponse.Write();
  LeadPtZgResponse2D.Write();

  ToaCoarseLeadZgResponse.Write("ToaCoarseLeadZgResponse", TObject::kSingleKey);
  ToaFineLeadZgResponse.Write("ToaFineLeadZgResponse", TObject::kSingleKey );
  fout->WriteObject ( &aCoarsePtBinBounds, "CoarsePtBinBounds");
  fout->WriteObject ( &aFinePtBinBounds, "FinePtBinBounds");
  
  // for ( int i=0 ; i<nCoarsePt; ++ i )   CoarseLeadZgResponse[i].Write();
  // for ( int i=0 ; i<nFinePt; ++ i )     FineLeadZgResponse[i].Write();

  fout->Write();

  cout << "Wrote to " << fout->GetName() << endl;
  cout << "Bye." << endl;
  
  return 0;
  
}
