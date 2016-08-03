#include "TROOT.h"
#include "TRandom.h"
#include "TMath.h"
#include "TString.h"
#include "TObjString.h"
#include "TFile.h"
#include "TH1.h"
#include "TProfile.h"
#include "TH2.h"
#include "TH3.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TSystem.h"
#include "TTree.h"
#include "TF1.h"
#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "TParameter.h"

#include "TStarJetVectorJet.h"


using namespace std;

int Explorer()
{
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);

  TFile* f = new TFile("Results/Groom_Rhic.root","READ");
  TTree* ResultTree = (TTree*) f->Get("ResultTree");

  // TString outname = f->GetName();
  // outname->ReplaceAll( ".root", ".Zg.root");
  // TFile * out= new TFile ( outname, "RECREATE");

  TString outbase = gSystem->BaseName( f->GetName() );
  outbase.ReplaceAll( ".root", ".Exploration");
  outbase.Prepend("Plots/");

  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);

  TH1D* LeadPt = new TH1D("LeadPt","Leading jet p_{T}", 120, 0, 240);
  TH1D* EmbLeadPt = new TH1D("EmbLeadPt","Embedded Leading jet p_{T}", 120, 0, 240);
  TH1D* GroomLeadPt = new TH1D("GroomLeadPt","Groomed Leading jet p_{T}", 120, 0, 240);
  TH1D* EmbGroomLeadPt = new TH1D("EmbGroomLeadPt","Groomed Embedded Leading jet p_{T}", 120, 0, 240);

  TH1D* LeadPartonPt = new TH1D("LeadPartonPt","Leading parton p_{T}", 120, 0, 240);
  TH1D* EmbLeadPartonPt = new TH1D("EmbLeadPartonPt","Embedded Leading parton p_{T}", 120, 0, 240);

  TH1D* SubLeadPt = new TH1D("SubLeadPt","SubLeading jet p_{T}", 120, 0, 240);
  TH1D* EmbSubLeadPt = new TH1D("EmbSubLeadPt","Embedded SubLeading jet p_{T}", 120, 0, 240);
  TH1D* GroomSubLeadPt = new TH1D("GroomSubLeadPt","Groomed SubLeading jet p_{T}", 120, 0, 240);
  TH1D* EmbGroomSubLeadPt = new TH1D("EmbGroomSubLeadPt","Groomed Embedded SubLeading jet p_{T}", 120, 0, 240);

  TH1D* SubLeadPartonPt = new TH1D("SubLeadPartonPt","SubLeading parton p_{T}", 120, 0, 240);
  TH1D* EmbSubLeadPartonPt = new TH1D("EmbSubLeadPartonPt","Embedded SubLeading parton p_{T}", 120, 0, 240);

  TH1D* UngroomedLead = new TH1D("UngroomedLead","Leading jets with groomed energy < 0.2 GeV", 120, 0, 240);  
  TH1D* UngroomedSubLead = new TH1D("UngroomedSubLead","SubLeading jets with groomed energy < 0.2 GeV", 120, 0, 240);

  TH2D* FracVOrigPtLead  = new TH2D("FracVOrigPtLead","Groomed #Delta p_{T}/p_{T} vs. orig p_{T}, Leading jets;p_{T}^{Jet};#Delta p_{T}/p_{T}"   , 120, 0, 240, 200, 0, 0.5);
  TH2D* FracVOrigPtSubLead  = new TH2D("FracVOrigPtSubLead","Groomed #Delta p_{T}/p_{T} vs. orig p_{T}, SubLeading jets;p_{T}^{Jet}#Delta p_{T}/p_{T};"   , 120, 0, 240, 200, 0, 0.5);
  
  TClonesArray* pHardPartons = new TClonesArray( "TStarJetVectorJet" ); 
  ResultTree->SetBranchAddress("HardPartons", &pHardPartons );

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
  
  TStarJetVectorJet* MatchedLeadingParton;
  TStarJetVectorJet* MatchedSubLeadingParton; // matched to sub-leading jet, obv.
  TStarJetVectorJet* LeadingJet;
  TStarJetVectorJet* EmbLeadingJet;
  TStarJetVectorJet* GroomedLeadingJet;
  TStarJetVectorJet* EmbGroomedLeadingJet;
  TStarJetVectorJet* SubLeadingJet;
  TStarJetVectorJet* EmbSubLeadingJet;
  TStarJetVectorJet* GroomedSubLeadingJet;
  TStarJetVectorJet* EmbGroomedSubLeadingJet;

  int evistep=1; // make it larger than 1 to take a thinner sample without skewing the weight too much
  for ( int evi=0; evi<NEvents; evi+=evistep){
    if ( !(evi%100000) ) cout << "Working on " << evi << " / " << NEvents << endl;
    ResultTree->GetEntry(evi);

    // ===================== Leading ==========================
    LeadingJet = (TStarJetVectorJet*) pJets->At(0);
    float Leadingpt = LeadingJet->Pt();

    // Some jets are slightly farther out. Enforce.
    if ( fabs(LeadingJet->Eta()) < 1-0.4 ) {
      int leadmatch = LeadingJet->GetMatch();
      MatchedLeadingParton =
	leadmatch>=0 ?
	(TStarJetVectorJet*) pHardPartons->At( leadmatch ) :
	0;

      GroomedLeadingJet = (TStarJetVectorJet*) pGroomedJets->At(0);
      float gLeadingpt = GroomedLeadingJet->Pt();

      // For now, force match to hard parton
      if ( MatchedLeadingParton ){
	LeadPt->Fill ( Leadingpt, weight );
	LeadPartonPt->Fill ( MatchedLeadingParton->Pt(), weight );
	GroomLeadPt->Fill ( gLeadingpt, weight );
	if ( fabs(Leadingpt-gLeadingpt)<0.2 ){
	  UngroomedLead->Fill( Leadingpt, weight );
	} else {
	  FracVOrigPtLead->Fill ( Leadingpt, (Leadingpt-gLeadingpt)/Leadingpt, weight );
	}
      }
    }

    // ================== SubLeading ==========================
    SubLeadingJet = (TStarJetVectorJet*) pJets->At(0);
    float SubLeadingpt = SubLeadingJet->Pt();

    // Some jets are slightly farther out. Enforce.
    if ( fabs(SubLeadingJet->Eta()) > 1-0.4 ) {
      int leadmatch = SubLeadingJet->GetMatch();
      MatchedSubLeadingParton =
	leadmatch>=0 ?
	(TStarJetVectorJet*) pHardPartons->At( leadmatch ) :
	0;
      
      GroomedSubLeadingJet = (TStarJetVectorJet*) pGroomedJets->At(0);
      float gSubLeadingpt = GroomedSubLeadingJet->Pt();
      
      // For now, force match to hard parton
      if ( MatchedSubLeadingParton ){
	SubLeadPt->Fill ( SubLeadingpt, weight );
	SubLeadPartonPt->Fill ( MatchedSubLeadingParton->Pt(), weight );
	GroomSubLeadPt->Fill ( gSubLeadingpt, weight );
	if ( fabs(SubLeadingpt-gSubLeadingpt)<0.2 ){
	  UngroomedSubLead->Fill( SubLeadingpt, weight );
	} else { 
	  FracVOrigPtSubLead->Fill ( SubLeadingpt, (SubLeadingpt-gSubLeadingpt)/SubLeadingpt, weight );
	}
      }
    }

    // ================== Embedded ==========================
    // if ( pEmbJets->GetEntries()==0 ) continue;
    
    // EmbJet = (TStarJetVectorJet*) pEmbJets->At(0);
    // // if ( EmbJet.GetMatch()<0 || EmbJet.GetMatch()>1  ) continue;
    // EmbGroomedJet = (TStarJetVectorJet*) pEmbGroomedJets->At(0);
    // // float Embpt = Jet->Pt();
    // float Embpt = EmbJet->Pt();
    // float Embgpt = EmbGroomedJet->Pt();
    // EmbLeadPt->Fill ( Embpt, weight );
    // EmbGroomLeadPt->Fill ( Embgpt, weight );

    
  }


  TLegend* leg;
  // =========================== Leading spectra ===============================
  new TCanvas;
  leg = new TLegend( 0.58, 0.58, 0.89, 0.9, "anti-k_{T}, R=0.4" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  gPad->SetGridx(0);  gPad->SetGridy(0);
  gPad->SetLogy();

  if ( !(TString(f->GetName()).Contains("Lhc")) ){ 
    LeadPt->SetAxisRange(5,70);
  }
  LeadPt->SetTitle(";p_{T} [GeV];arb.");
  
  LeadPt->SetLineColor(kBlack);
  LeadPt->Draw();
  GroomLeadPt->SetLineColor(kRed);
  GroomLeadPt->Draw("same");
  LeadPartonPt->SetLineColor(kGray+1);
  LeadPartonPt->Draw("same");

  // EmbLeadPt->SetLineColor(kRed);
  // EmbLeadPt->Draw("same");
  // EmbGroomLeadPt->SetLineColor(kMagenta);
  // EmbGroomLeadPt->Draw("same");

  leg->AddEntry( LeadPt->GetName(), "Leading Jet");
  leg->AddEntry( GroomLeadPt->GetName(), "Groomed Leading Jet");
  leg->AddEntry( LeadPartonPt->GetName(), "Matched Original Parton");
  
  // leg->Draw("same");
  // gPad->SaveAs( outbase + "_LeadingSpectra.png");

  // // =========================== SubLeading spectra ===============================
  // new TCanvas;
  // leg = new TLegend( 0.55, 0.65, 0.89, 0.9, "anti-k_{T}, R=0.4" );
  // leg->SetBorderSize(0);
  // leg->SetLineWidth(10);
  // leg->SetFillStyle(0);
  // leg->SetMargin(0.1);

  // gPad->SetGridx(0);  gPad->SetGridy(0);
  // gPad->SetLogy();

  if ( !(TString(f->GetName()).Contains("Lhc")) ){ 
    SubLeadPt->SetAxisRange(5,70);
  }
  SubLeadPt->SetTitle(";p_{T} [GeV];arb.");
  
  SubLeadPt->SetLineColor(kBlue);
  SubLeadPt->Draw("same");
  // SubLeadPt->Draw();
  GroomSubLeadPt->SetLineColor(kMagenta);
  GroomSubLeadPt->Draw("same");
  SubLeadPartonPt->SetLineColor(kGray+1);
  SubLeadPartonPt->Draw("same");

  leg->AddEntry( SubLeadPt->GetName(), "SubLeading Jet");
  leg->AddEntry( GroomSubLeadPt->GetName(), "Groomed SubLeading Jet");
  leg->AddEntry( SubLeadPartonPt->GetName(), "Matched Original Parton");
  
  leg->Draw("same");
  // gPad->SaveAs( outbase + "_SubLeadingSpectra.png");
  gPad->SaveAs( outbase + "_BothSpectra.png");



  // =========================== Ratio Groomed/Orig ===============================
  new TCanvas;
  leg = new TLegend( 0.55, 0.7, 0.89, 0.9, "anti-k_{T}, R=0.4" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  gPad->SetGridx(0);  gPad->SetGridy(0);

  TH1D* LeadingRatio = (TH1D*) GroomLeadPt->Clone("LeadingRatio");
  LeadingRatio->Divide( LeadPt );
  LeadingRatio->SetTitle(";p_{T} [GeV]; Groomed / Original");

  TH1D* SubLeadingRatio = (TH1D*) GroomSubLeadPt->Clone("SubLeadingRatio");
  SubLeadingRatio->Divide( SubLeadPt );
  SubLeadingRatio->SetTitle(";p_{T} [GeV]; Groomed / Original");


  if ( !(TString(f->GetName()).Contains("Lhc")) ){ 
    LeadingRatio->SetAxisRange(10,70);
    SubLeadingRatio->SetAxisRange(10,70);
  }
  LeadingRatio->SetAxisRange(0,1.4, "y");
  
  LeadingRatio->Draw();
  SubLeadingRatio->Draw("same");
  
  leg->AddEntry( LeadingRatio->GetName(), "Leading Jet");
  leg->AddEntry( SubLeadingRatio->GetName(), "SubLeading Jet");
  leg->Draw("same");
  gPad->SaveAs( outbase + "_SpectraRatios.png");

  // =========================== fraction of unchanged jets ===============================
  new TCanvas;
  leg = new TLegend( 0.55, 0.7, 0.89, 0.9, "anti-k_{T}, R=0.4" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  TH1D* UngroomedLeadFraction = (TH1D*) UngroomedLead->Clone("UngroomedLeadFraction");
  UngroomedLeadFraction->Divide( LeadPt );
  UngroomedLeadFraction->SetLineColor (LeadPt->GetLineColor() );
  UngroomedLeadFraction->SetTitle(";Jet p_{T} [GeV]; Fraction of ungroomed jets");
  if ( !(TString(f->GetName()).Contains("Lhc")) ){ 
    UngroomedLeadFraction->SetAxisRange(10,70);
  }
  UngroomedLeadFraction->SetAxisRange(0, 1, "y");
  UngroomedLeadFraction->Draw();
  
  TH1D* UngroomedSubLeadFraction = (TH1D*) UngroomedSubLead->Clone("UngroomedSubLeadFraction");
  UngroomedSubLeadFraction->Divide( SubLeadPt );
  UngroomedSubLeadFraction->SetLineColor (SubLeadPt->GetLineColor() );
  if ( !(TString(f->GetName()).Contains("Lhc")) ){ 
    UngroomedSubLeadFraction->SetAxisRange(10,70);
  }
  UngroomedSubLeadFraction->Draw("same");
  
  leg->AddEntry( UngroomedLeadFraction->GetName(), "Leading Jets");
  leg->AddEntry( UngroomedSubLeadFraction->GetName(), "SubLeading Jets");
  leg->Draw("same");
  gPad->SaveAs( outbase + "_UngroomedFraction.png");

  // =========================== 2D plots of groomed fraction ===============================
  if ( !(TString(f->GetName()).Contains("Lhc")) ){ 
    FracVOrigPtLead->SetAxisRange(10,70,"x");
    FracVOrigPtSubLead->SetAxisRange(10,70,"x");
  }

  new TCanvas;
  FracVOrigPtLead->Draw("colz");  
  gPad->SaveAs( outbase + "_FracVOrigPtLead.png");

  new TCanvas;
  FracVOrigPtSubLead->Draw("colz");
  gPad->SaveAs( outbase + "_FracVOrigPtSubLead.png");

  TProfile* FracVOrigPtLead_x = (TProfile*) FracVOrigPtLead->ProfileX();
  FracVOrigPtLead_x->SetName("FracVOrigPtLead_x");
  TProfile* FracVOrigPtSubLead_x = (TProfile*) FracVOrigPtSubLead->ProfileX();
  FracVOrigPtSubLead_x->SetName("FracVOrigPtSubLead_x");

  if ( !(TString(f->GetName()).Contains("Lhc")) ){ 
    FracVOrigPtSubLead_x->SetAxisRange(0, 1, "y");
    FracVOrigPtSubLead_x->SetAxisRange(0, 1, "y");
  }

  new TCanvas;
  FracVOrigPtLead_x->SetLineColor (LeadPt->GetLineColor() );
  FracVOrigPtLead_x->Draw();
  FracVOrigPtSubLead_x->SetLineColor (SubLeadPt->GetLineColor() );
  FracVOrigPtSubLead_x->Draw("same");

  leg = new TLegend( 0.55, 0.7, 0.89, 0.9, "anti-k_{T}, R=0.4" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);
  leg->AddEntry( FracVOrigPtLead_x->GetName(), "Leading Jets");
  leg->AddEntry( FracVOrigPtSubLead_x->GetName(), "SubLeading Jets");
  leg->Draw("same");

  
  gPad->SaveAs( outbase + "_FracProfile.png");


  return 0;
}
