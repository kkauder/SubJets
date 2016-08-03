{
  gROOT->ForceStyle();
  gStyle->SetOptStat(0);
  
  TFile* freal = new TFile("rhoreal.root","root");
  TH1D* real = (TH1D*) freal->Get("htemp");
  real->SetName("real");
  real->Scale(1./real->Integral());
  
  TFile* ffake = new TFile("rhofake.root","root");
  TH1D* fake = (TH1D*) ffake->Get("htemp");
  fake->SetName("fake");
  fake->Scale(1./fake->Integral());

  real->SetTitle(";#rho [(GeV/c)/(str)]; arb. u.");
  real->SetLineColor(kRed);

  fake->SetTitle(";#rho [(GeV/c)/(str)]; arb. u.");
  fake->SetLineColor(kGreen+1);

  fake->SetAxisRange( 0,0.03, "y");
  fake->Draw();
  real->Draw("same");

  TLegend* leg = new TLegend(0.55, 0.68, 0.88, 0.88);
  leg->AddEntry( real, "#rho from 0-20\% Au+Au","l");
  leg->AddEntry( fake, "#rho from Toy MC","l");
  leg->Draw("same");

  gPad->SaveAs("plots/rhocomp.png");
  
}
