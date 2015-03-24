{
  gStyle->SetHistLineWidth(2);
  gStyle->SetOptStat(0);

  

  TFile *fPythia = TFile::Open("HighCutSubjetResult.root");
  TFile *fPythiaEmb = TFile::Open("HighCutPythiaEmbSubjetResult.root");
  // TFile *fPythia = TFile::Open("SubjetResult.root");
  // TFile *fPythiaEmb = TFile::Open("PythiaEmbSubjetResult.root");

  TString sPythia = gSystem->BaseName( fPythia->GetName() );
  sPythia.ReplaceAll( ".root","");
  sPythia.Prepend("plots/");
  TString sPythiaEmb = gSystem->BaseName( fPythiaEmb->GetName() );
  sPythiaEmb.ReplaceAll( ".root","");
  sPythiaEmb.Prepend("plots/");

  
  TH2D* PythiaNsubPt      = (TH2D*) fPythia->Get("NsubPt");
  TH2D* PythiaSubPtFrac   = (TH2D*) fPythia->Get("SubPtFrac");
  TH2D* PythiaOtherPtFrac = (TH2D*) fPythia->Get("OtherPtFrac");

  TH2D* PythiagNsubPt      = (TH2D*) fPythia->Get("gNsubPt");
  TH2D* PythiagSubPtFrac   = (TH2D*) fPythia->Get("gSubPtFrac");
  TH2D* PythiagOtherPtFrac = (TH2D*) fPythia->Get("gOtherPtFrac");
    
  TH2D* PythiaqNsubPt      = (TH2D*) fPythia->Get("qNsubPt");
  TH2D* PythiaqSubPtFrac   = (TH2D*) fPythia->Get("qSubPtFrac");
  TH2D* PythiaqOtherPtFrac = (TH2D*) fPythia->Get("qOtherPtFrac");

  int ptleft=1;
  int ptright=PythiaNsubPt->GetNbinsX();

  // ===========================================================================
  TH1D* yPythiaNsub = PythiaNsubPt->ProjectionY("yPythiaNsubPt",ptleft, ptright);
  yPythiaNsubPt->SetLineColor(kBlack);
  TH1D* yPythiagNsub = PythiagNsubPt->ProjectionY("yPythiagNsubPt",ptleft, ptright);
  yPythiagNsubPt->SetLineColor(kGreen+1);
  TH1D* yPythiaqNsub = PythiaqNsubPt->ProjectionY("yPythiaqNsubPt",ptleft, ptright);
  yPythiaqNsubPt->SetLineColor(kRed);

  new TCanvas("cNsub","PURE PYTHIA, # Subjets");
  yPythiaqNsubPt->SetTitle(gPad->GetTitle());
  yPythiaqNsubPt->DrawNormalized();
  yPythiaNsubPt->DrawNormalized("same");
  yPythiagNsubPt->DrawNormalized("same");
  gPad->SaveAs( sPythia + ".Nsub.png");
	      

  // ===========================================================================
  TH1D* yPythiaSubPtFrac = PythiaSubPtFrac->ProjectionY("yPythiaSubPtFrac",ptleft, ptright);
  yPythiaSubPtFrac->SetLineColor(kBlack);
  TH1D* yPythiagSubPtFrac = PythiagSubPtFrac->ProjectionY("yPythiagSubPtFrac",ptleft, ptright);
  yPythiagSubPtFrac->SetLineColor(kGreen+1);
  TH1D* yPythiaqSubPtFrac = PythiaqSubPtFrac->ProjectionY("yPythiaqSubPtFrac",ptleft, ptright);
  yPythiaqSubPtFrac->SetLineColor(kRed);

  new TCanvas("cSubPtFrac","PURE PYTHIA, p_{T} fraction of leading subjet");
  yPythiaqSubPtFrac->SetTitle(gPad->GetTitle());
  yPythiaqSubPtFrac->DrawNormalized();
  yPythiagSubPtFrac->DrawNormalized("same");
  yPythiaSubPtFrac->DrawNormalized("same");
  gPad->SaveAs( sPythia + ".SubPtFrac.png");
  
  // ===========================================================================
  TH1D* yPythiaOtherPtFrac = PythiaOtherPtFrac->ProjectionY("yPythiaOtherPtFrac",ptleft, ptright);
  yPythiaOtherPtFrac->SetLineColor(kBlack);
  TH1D* yPythiagOtherPtFrac = PythiagOtherPtFrac->ProjectionY("yPythiagOtherPtFrac",ptleft, ptright);
  yPythiagOtherPtFrac->SetLineColor(kGreen+1);
  TH1D* yPythiaqOtherPtFrac = PythiaqOtherPtFrac->ProjectionY("yPythiaqOtherPtFrac",ptleft, ptright);
  yPythiaqOtherPtFrac->SetLineColor(kRed);

  new TCanvas("cOtherPtFrac","PURE PYTHIA, p_{T} fraction of non-leading subjets");
  yPythiaqOtherPtFrac->SetTitle(gPad->GetTitle());
  yPythiaqOtherPtFrac->DrawNormalized();
  yPythiagOtherPtFrac->DrawNormalized("same");
  yPythiaOtherPtFrac->DrawNormalized("same");
  
  gPad->SaveAs( sPythia + ".OtherPtFrac.png");
  return;

  // ===========================================================================
  // ===========================================================================
  // ===========================================================================

  TH2D* PythiaEmbNsubPt      = (TH2D*) fPythiaEmb->Get("NsubPt");
  TH2D* PythiaEmbSubPtFrac   = (TH2D*) fPythiaEmb->Get("SubPtFrac");
  TH2D* PythiaEmbOtherPtFrac = (TH2D*) fPythiaEmb->Get("OtherPtFrac");

  TH2D* PythiaEmbgNsubPt      = (TH2D*) fPythiaEmb->Get("gNsubPt");
  TH2D* PythiaEmbgSubPtFrac   = (TH2D*) fPythiaEmb->Get("gSubPtFrac");
  TH2D* PythiaEmbgOtherPtFrac = (TH2D*) fPythiaEmb->Get("gOtherPtFrac");
    
  TH2D* PythiaEmbqNsubPt      = (TH2D*) fPythiaEmb->Get("qNsubPt");
  TH2D* PythiaEmbqSubPtFrac   = (TH2D*) fPythiaEmb->Get("qSubPtFrac");
  TH2D* PythiaEmbqOtherPtFrac = (TH2D*) fPythiaEmb->Get("qOtherPtFrac");

  int ptleft=1;
  int ptright=PythiaEmbNsubPt->GetNbinsX();

  // ===========================================================================
  TH1D* yPythiaEmbNsub = PythiaEmbNsubPt->ProjectionY("yPythiaEmbNsubPt",ptleft, ptright);
  yPythiaEmbNsubPt->SetLineColor(kBlack);
  TH1D* yPythiaEmbgNsub = PythiaEmbgNsubPt->ProjectionY("yPythiaEmbgNsubPt",ptleft, ptright);
  yPythiaEmbgNsubPt->SetLineColor(kGreen+1);
  TH1D* yPythiaEmbqNsub = PythiaEmbqNsubPt->ProjectionY("yPythiaEmbqNsubPt",ptleft, ptright);
  yPythiaEmbqNsubPt->SetLineColor(kRed);

  new TCanvas("cEmbNsub","PYTHIA @ Au+Au, # Subjets");
  yPythiaEmbqNsubPt->SetTitle(gPad->GetTitle());
  yPythiaEmbqNsubPt->DrawNormalized();
  yPythiaEmbNsubPt->DrawNormalized("same");
  yPythiaEmbgNsubPt->DrawNormalized("same");
  gPad->SaveAs( sPythiaEmb + ".Nsub.png");	      

  // ===========================================================================
  TH1D* yPythiaEmbSubPtFrac = PythiaEmbSubPtFrac->ProjectionY("yPythiaEmbSubPtFrac",ptleft, ptright);
  yPythiaEmbSubPtFrac->SetLineColor(kBlack);
  TH1D* yPythiaEmbgSubPtFrac = PythiaEmbgSubPtFrac->ProjectionY("yPythiaEmbgSubPtFrac",ptleft, ptright);
  yPythiaEmbgSubPtFrac->SetLineColor(kGreen+1);
  TH1D* yPythiaEmbqSubPtFrac = PythiaEmbqSubPtFrac->ProjectionY("yPythiaEmbqSubPtFrac",ptleft, ptright);
  yPythiaEmbqSubPtFrac->SetLineColor(kRed);

  new TCanvas("cEmbSubPtFrac","PYTHIA @ Au+Au, p_{T} fraction of leading subjet");
  yPythiaEmbqSubPtFrac->SetTitle(gPad->GetTitle());
  yPythiaEmbqSubPtFrac->DrawNormalized();
  yPythiaEmbgSubPtFrac->DrawNormalized("same");
  yPythiaEmbSubPtFrac->DrawNormalized("same");
  gPad->SaveAs( sPythiaEmb + ".SubPtFrac.png");
	      
  // ===========================================================================
  TH1D* yPythiaEmbOtherPtFrac = PythiaEmbOtherPtFrac->ProjectionY("yPythiaEmbOtherPtFrac",ptleft, ptright);
  yPythiaEmbOtherPtFrac->SetLineColor(kBlack);
  TH1D* yPythiaEmbgOtherPtFrac = PythiaEmbgOtherPtFrac->ProjectionY("yPythiaEmbgOtherPtFrac",ptleft, ptright);
  yPythiaEmbgOtherPtFrac->SetLineColor(kGreen+1);
  TH1D* yPythiaEmbqOtherPtFrac = PythiaEmbqOtherPtFrac->ProjectionY("yPythiaEmbqOtherPtFrac",ptleft, ptright);
  yPythiaEmbqOtherPtFrac->SetLineColor(kRed);

  new TCanvas("cEmbOtherPtFrac","PYTHIA @ Au+Au, p_{T} fraction of non-leading subjets");
  yPythiaEmbqOtherPtFrac->SetTitle(gPad->GetTitle());
  yPythiaEmbqOtherPtFrac->DrawNormalized();
  yPythiaEmbgOtherPtFrac->DrawNormalized("same");
  yPythiaEmbOtherPtFrac->DrawNormalized("same");
  gPad->SaveAs( sPythiaEmb + ".OtherPtFrac.png");
  
}



