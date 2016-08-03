{
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);


  TFile* f = new TFile("Results/Groom_Rhic.root");
  TTree* ResultTree = (TTree*) f->Get("ResultTree");

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

  TString minPtHard = "20";
  TString maxPtHard = "40";

  TString CondString = "weight";
  CondString += " *( ";
  CondString += " HardPartons[0].Pt()>" +minPtHard;
  CondString += " || ";
  CondString += " HardPartons[0].Pt()<" +maxPtHard;
  CondString += " ) ";    


  new TCanvas;
  gPad->SetLogy();
  TString title = "PYTHIA, Mass for " + minPtHard + "<p_{T,hard}<"+maxPtHard;
  title += ";M;counts";
  Mass->SetTitle( title );
  GroomedMass->SetTitle( title );
  EmbMass->SetTitle( title );
  GroomedEmbMass->SetTitle( title );
    
  ResultTree->Draw("Jets[0].M() >> Mass",CondString);
  ResultTree->Draw("GroomedJets[0].M() >> GroomedMass",CondString, "same");

  ResultTree->Draw("EmbJets[0].M() >> EmbMass",CondString,"same");
  ResultTree->Draw("EmbGroomedJets[0].M() >> GroomedEmbMass",CondString, "same");

  TLegend* leg = new TLegend( 0.55, 0.55, 0.89, 0.9, "Leading jet" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  leg->AddEntry( Mass, "Jet");
  leg->AddEntry( GroomedMass, "Groomed Jet");
  leg->AddEntry( EmbMass, "Emb. Jet");
  leg->AddEntry( GroomedEmbMass, "Groomed Emb. Jet");
  leg->Draw();

  gPad->SaveAs("PythiaMass.png");
  
  new TCanvas;
  gPad->SetLogy();
  TH1D* Ratio = GroomedMass->Clone("Ratio");
  Ratio->Divide( Mass );
  TH1D* EmbRatio = GroomedEmbMass->Clone("EmbRatio");
  EmbRatio->Divide( EmbMass );

  title = "PYTHIA, GroomedMass/Mass for " + minPtHard + "<p_{T,hard}<"+maxPtHard;
  title += ";M";
  Ratio->SetTitle( title );
  Ratio->Draw();
  EmbRatio->Draw("same");

  TLegend* leg = new TLegend( 0.55, 0.65, 0.89, 0.9, "Leading jet" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  leg->AddEntry( Ratio, "Jet");
  leg->AddEntry( EmbRatio, "Emb. Jet");
  leg->Draw();

  gPad->SaveAs("PythiaMassRatio.png");  
}
