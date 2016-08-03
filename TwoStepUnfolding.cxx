
#include "../software/RooUnfold/src/RooUnfoldResponse.h"
#include "../software/RooUnfold/src/RooUnfoldBayes.h"
#include "../software/RooUnfold/src/RooUnfoldSvd.h"
#include "../software/RooUnfold/src/RooUnfoldTUnfold.h"

#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TArrayD.h>
#include <TObjArray.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TLegend.h>



int TwoStepUnfolding( bool coarse=true, TString inname = "Results/GroomUnfoldingTest.root") {
  
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);

  TFile* f = new TFile( inname, "READ");
  TString outbase = gSystem->BaseName( f->GetName() );
  if (coarse) outbase.ReplaceAll( ".root", "_CoarseTwoStep");
  else        outbase.ReplaceAll( ".root", "_FineTwoStep");
  outbase.Prepend("Plots/");
  
  TH2D* TestTruth2D = (TH2D*) f->Get("TestTruth2D");
  TH2D* TestMeas2D  = (TH2D*) f->Get("TestMeas2D");
  TH2D* TrainMeas2D  = (TH2D*) f->Get("Meas2D");
  TH2D* TrainTruth2D  = (TH2D*) f->Get("Truth2D");


  // pT Unfolding
  // ------------
  RooUnfoldResponse* LeadPtResponse = (RooUnfoldResponse*) f->Get("LeadPtResponse");
  TH1D* PtTestMeas  = (TH1D*) TestMeas2D->ProjectionX("PtTestMeas");
  int nPtBayes1D=3;
  RooUnfoldBayes    PtBayesUnfold ( LeadPtResponse, PtTestMeas, nPtBayes1D);
  // PtBayesUnfold.PrintTable( cout, (TH1D*) TestTruth2D->ProjectionX() );
  // return 0;
  TH1D* PtBayesUnfolded= (TH1D*) PtBayesUnfold.Hreco();
  PtBayesUnfolded->SetName("PtBayesUnfolded");

  // Get Bins
  // --------
  TArrayD* PtBinBounds=0;
  TObjArray* ToaLeadZgResponse=0;
  if (coarse) {
    PtBinBounds = (TArrayD*) f->Get("CoarsePtBinBounds");
    ToaLeadZgResponse = (TObjArray*) f->Get("ToaCoarseLeadZgResponse");
  } else {
    PtBinBounds = (TArrayD*) f->Get("FinePtBinBounds");
    ToaLeadZgResponse = (TObjArray*) f->Get("ToaFineLeadZgResponse");
  }
  int nPtBins = PtBinBounds->GetSize()-1;
  if ( ToaLeadZgResponse->GetEntries() != nPtBins ){
    cerr << "Incompatible sizes." << endl;
    return -1;
  }

  // Unfoled bin-wise
  // ----------------
  TH1D* ZgTestMeas  = (TH1D*) TestMeas2D->ProjectionY("ZgTestMeas");
  TObjArray ToaZgBayesUnfolded;
  int nZgBayes1D=3;
  for ( int i=0; i<nPtBins; ++i ){
    cout << " ================== " << endl;
    cout << " Working on bin " << i << endl;
    cout << " ================== " << endl;
    RooUnfoldResponse* zgResp = (RooUnfoldResponse*) ToaLeadZgResponse->At(i);

    if ( zgResp->Hresponse()->GetEntries()< 100 ) {
      cout << "SKIPPED" << endl;
      ToaZgBayesUnfolded.Add( 0 );
      continue;
    }
    RooUnfoldBayes* ZgBayesUnfold = new RooUnfoldBayes( zgResp, ZgTestMeas, nZgBayes1D );
    TH1D* ZgBayesUnfolded= (TH1D*) ZgBayesUnfold->Hreco();
    TString name = "ZgBayesUnfolded_";
    name += i;
    ZgBayesUnfolded->SetName("name");
    ToaZgBayesUnfolded.Add( ZgBayesUnfolded );
  }

  // And combine
  // -----------
  // First rebin pt unfolding
  TH1D* RebinnedPtBayes = PtBayesUnfolded->Rebin( nPtBins, "RebinnedPtBayes", PtBinBounds->GetArray() );

  // Now normalize spectrum and use as weights
  // -----------------------------------------
  RebinnedPtBayes->Scale ( 1./RebinnedPtBayes->Integral() );

  TH1D* WeightedZgBayesUnfolded = (TH1D*) TestTruth2D->ProjectionY("WeightedZgBayesUnfolded");  
  // TH1D* WeightedZgBayesUnfolded = (TH1D*) ToaZgBayesUnfolded.At(0)->Clone("WeightedZgBayesUnfolded");
  WeightedZgBayesUnfolded->Reset();
  WeightedZgBayesUnfolded->SetLineColor(kBlue);
    
  for ( int i=0; i< ToaZgBayesUnfolded.GetEntries(); ++i ){
    // new TCanvas;
    // ((TH1D*)ToaZgBayesUnfolded.At(i))->DrawNormalized();
    // WeightedZgBayesUnfolded->DrawNormalized("same");
    cout << RebinnedPtBayes->GetBinContent( i+1 ) << endl;
    TH1D* h = (TH1D*) ToaZgBayesUnfolded.At(i);
    if ( !h ) continue;
    
    // Constrain pT_true range here
    // ----------------------------
    // CAREFUL! pT bins MUST overlap with cuts
    if ( RebinnedPtBayes->GetBinLowEdge( i+1 )+0.01<10 ) continue;
    if ( RebinnedPtBayes->GetBinLowEdge( i+2 )-0.01>20 ) continue;
    
    

  
    WeightedZgBayesUnfolded->Add( h, RebinnedPtBayes->GetBinContent( i+1 ) );
    // new TCanvas;
    // h->DrawCopy();
    // new TCanvas;
    // WeightedZgBayesUnfolded->DrawCopy();
    // if ( i>3 ){
    //   new TCanvas;
    //   WeightedZgBayesUnfolded->Draw();
    // return 0;
    //}
    
  }

  
  // =========================== Draw Spectra ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  gPad->SetLogy();
  TLegend* leg = new TLegend( 0.55, 0.7, 0.89, 0.9, "Leading Jet p_{T}" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  TH1D* truthpt = (TH1D*) TestTruth2D->ProjectionX("truthpt");
  truthpt->SetLineColor(kRed);
  truthpt->SetTitle( ";p_{T};arb. units" );
  truthpt->SetMinimum( 1E-5 );
  truthpt->Draw();
  leg->AddEntry("truthpt", "Truth");
      
  TH1D* measpt = (TH1D*) TestMeas2D->ProjectionX("measpt");
  measpt->SetLineColor(kBlue);
  measpt->Draw("same");
  leg->AddEntry("measpt", "Measured");

  PtBayesUnfolded->SetLineColor(kBlack);
  PtBayesUnfolded->Draw("same");
  leg->AddEntry("PtBayesUnfolded", "1D Unfolded");    

  leg->Draw();
  gPad->SaveAs( outbase + "_UnfoldedSpectra.png");

  // new TCanvas;
  // RebinnedPtBayes->Draw();
  // return 0;
  
  // =========================== Draw Zg ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);

  leg = new TLegend( 0.55, 0.65, 0.89, 0.9, "Leading Jet z_{g} NORMALIZED" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);
    
  TH1D* truth1 = (TH1D*) TestTruth2D->ProjectionY("truth1", TestTruth2D->GetXaxis()->FindBin( 20+0.01), TestTruth2D->GetXaxis()->FindBin( 30-0.01));
  truth1->SetLineColor(kRed);
  truth1->SetTitle( ";z_{g};arb. units" );
  truth1->DrawNormalized();
  leg->AddEntry("truth1", "Truth, 20 < p_{T} < 30");

  // TH1D* truth2 = (TH1D*) TestTruth2D->ProjectionY("truth2");
  // truth2->SetLineColor(kMagenta);
  // truth2->SetTitle( ";z_{g};arb. units" );
  // truth2->DrawNormalized("same");
  // leg->AddEntry("truth1", "Truth, No p_{T} cut");

  TH1D* truth3 = (TH1D*) TestTruth2D->ProjectionY("truth3", TestTruth2D->GetXaxis()->FindBin( 10+0.01), TestTruth2D->GetXaxis()->FindBin( 20-0.01));
  truth3->SetLineColor(kGreen);
  truth3->SetTitle( ";z_{g};arb. units" );
  truth3->DrawNormalized("same");
  leg->AddEntry("truth1", "Truth, 10 < p_{T} < 20");

  // TH1D* ZgBayesUnfolded_5 = (TH1D*) ToaZgBayesUnfolded.At(5);
  // ZgBayesUnfolded_5->SetLineColor(kBlue);
  // ZgBayesUnfolded_5->DrawNormalized("same");
  // leg->AddEntry("ZgBayesUnfolded_5", "ZgBayesUnfolded_5");

  WeightedZgBayesUnfolded->SetLineColor(kBlue);
  WeightedZgBayesUnfolded->DrawNormalized("same");
  leg->AddEntry("WeightedZgBayesUnfolded", "1D Unfolded, 10 < p_{T} < 20");


  leg->Draw();
  gPad->SaveAs( outbase + "_UnfoldedZg.png");

  new TCanvas;
  WeightedZgBayesUnfolded->DrawNormalized();
  return 0;

  // =========================== Draw unnormalized Zg ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);

  leg = new TLegend( 0.55, 0.65, 0.89, 0.9, "Leading Jet z_{g}" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);
    
  truth1->Draw();
  leg->AddEntry("truth1", "Truth, 20 < p_{T} < 30");

  unfold1->Draw("same");
  leg->AddEntry("unfold1", "Unfolded, 20 < p_{T} < 30");

  truth2->Draw("same");
  leg->AddEntry("truth2", "Truth, 30 < p_{T} < 40");

  unfold2->Draw("same");
  leg->AddEntry("unfold2", "Unfolded, 30 < p_{T} < 40");
  leg->Draw();
  
  gPad->SaveAs( outbase + "_VarUnfoldedZg.png");  

  // =========================== Draw Ratio ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);

  leg = new TLegend( 0.25, 0.75, 0.59, 0.9, "z_{g} Ratio Unfolded / Truth" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  TH1D* ratio1 = (TH1D*) unfold1->Clone("ratio1");
  ratio1->Divide(truth1);
  // ratio1->SetLineColor(kRed);
  ratio1->SetTitle( ";z_{g};unfolded / truth" );
  ratio1->SetAxisRange( 0.5, 1.6, "y" );
  ratio1->Draw();
  leg->AddEntry("ratio1", "20 < p_{T} < 30");
  
  TH1D* ratio2 = (TH1D*) unfold2->Clone("ratio2");
  ratio2->Divide(truth2);
  // ratio2->SetLineColor(kBlue);
  ratio2->SetAxisRange( 0.5, 1.6, "y" );
  ratio2->Draw("same");
  leg->AddEntry("ratio2", "30 < p_{T} < 40");

  leg->Draw();
  gPad->SaveAs( outbase + "_UnfoldedZgRatio.png");  


  // DEBUG
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  gPad->SetLogy();

  truthpt->Draw();
  unfoldpt->Draw("same");
  PtBayesUnfolded->Draw("same");
  

  return 0;
}
