{
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);


  TFile* fAuAu = new TFile("Results/Groom_SmallAuAu.root");
  TTree* AuAuResultTree = (TTree*) fAuAu->Get("ResultTree");
  AuAuResultTree->SetName("AuAuResultTree");

  TFile* fPp = new TFile("Results/Groom_PpEmb.root");
  TTree* PpResultTree = (TTree*) fPp->Get("ResultTree");
  PpResultTree->SetName("AuAuResultTree");

  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);

  TH1D* Mass = new TH1D( "Mass", "Mass", 60, -5, 25);
  Mass->SetLineColor(kBlack);
  TH1D* GroomedMass = new TH1D( "GroomedMass", "Groomed Mass", 60, -5, 25);
  GroomedMass->SetLineColor(kRed);
  
  TH1D* EmbMass = new TH1D( "EmbMass", "EmbMass", 60, -5, 25);
  EmbMass->SetLineColor(kBlue);
  TH1D* GroomedEmbMass = new TH1D( "GroomedEmbMass", "Groomed EmbMass", 60, -5, 25);
  GroomedEmbMass->SetLineColor(kMagenta);

  TString minPt = "20";
  TString maxPt = "40";

  TString CondString = "weight";
  CondString += " *( ";
  CondString += " Jets[0].Pt()>" +minPt;
  CondString += " || ";
  CondString += " Jets[0].Pt()<" +maxPt;
  CondString += " ) ";    


  new TCanvas;
  gPad->SetLogy();
  TString title = "DATA, Mass for " + minPt + "<p_{T,Jet}<"+maxPt;
  title += ";M;counts";
  Mass->SetTitle( title );
  GroomedMass->SetTitle( title );
  EmbMass->SetTitle( title );
  GroomedEmbMass->SetTitle( title );
    
  AuAuResultTree->Draw("Jets[0].M() >> Mass",CondString);
  AuAuResultTree->Draw("GroomedJets[0].M() >> GroomedMass",CondString, "same");

  PpResultTree->Draw("EmbJets[0].M() >> EmbMass",CondString,"same");
  PpResultTree->Draw("EmbGroomedJets[0].M() >> GroomedEmbMass",CondString, "same");

  TLegend* leg = new TLegend( 0.55, 0.55, 0.89, 0.9, "Leading jet" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  leg->AddEntry( Mass, "AuAu Jet");
  leg->AddEntry( GroomedMass, "Groomed AuAu Jet");
  leg->AddEntry( EmbMass, "pp Emb. Jet");
  leg->AddEntry( GroomedEmbMass, "Groomed pp Emb. Jet");
  leg->Draw();

  gPad->SaveAs("DataMass.png");
  
  new TCanvas;
  // gPad->SetLogy();
  TH1D* Ratio = GroomedMass->Clone("Ratio");
  Ratio->Divide( Mass );
  TH1D* EmbRatio = GroomedEmbMass->Clone("EmbRatio");
  EmbRatio->Divide( EmbMass );

  title = "DATA, GroomedMass/Mass for " + minPt + "<p_{T,Jet}<"+maxPt;
  title += ";M";
  Ratio->SetTitle( title );
  // Ratio->SetAxisRange( 1e-5, 1e5, "y");
  Ratio->Draw();
  EmbRatio->Draw("same");

  TLegend* leg = new TLegend( 0.55, 0.65, 0.89, 0.9, "Leading jet" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  leg->AddEntry( Ratio, "AuAu Jet");
  leg->AddEntry( EmbRatio, "pp Emb. Jet");
  leg->Draw();

  gPad->SaveAs("DataMassRatio.png");  
}
