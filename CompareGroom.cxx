{
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);

  TFile* fPythia = new TFile("Results/Groom_Rhic.Zg.root","READ");
  TFile* fpp     = new TFile("Results/ppHtGroom.Zg.root","READ");
  // TFile* fpp     = new TFile("Results/Groom_pp.Zg.root","READ");
  TFile* fAuAu   = new TFile("Results/Groom_AuAu.Zg.root","READ");

  TString outbase = gSystem->BaseName( fAuAu->GetName() );
  outbase.ReplaceAll( ".root", ".Compared");
  outbase.Prepend("Plots/");

  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);

  TH1D* PythiaZgLead2030 = (TH1D*) fPythia->Get("zgLead2030");
  TH1D* PythiaZgLead3040 = (TH1D*) fPythia->Get("zgLead3040");
  PythiaZgLead2030->SetName("PythiaZgLead2030");
  PythiaZgLead3040->SetName("PythiaZgLead3040");

  TH1D* ppZgLead2030 = (TH1D*) fpp->Get("zgLead2030");
  TH1D* ppZgLead3040 = (TH1D*) fpp->Get("zgLead3040");
  ppZgLead2030->SetName("ppZgLead2030");
  ppZgLead3040->SetName("ppZgLead3040");

  TH1D* AuAuZgLead2030 = (TH1D*) fAuAu->Get("zgLead2030");
  TH1D* AuAuZgLead3040 = (TH1D*) fAuAu->Get("zgLead3040");
  AuAuZgLead2030->SetName("AuAuZgLead2030");
  AuAuZgLead3040->SetName("AuAuZgLead3040");


  // =========================== Also show theory ===============================
  TF1* PbarQjet = new TF1("PbarQjet","1./[0] * 4./3. * ( (1+pow(x,2))/(1-x) + (1 + pow(1-x,2))/x )", 0.1, 0.5);
  PbarQjet->SetParameter( 0,4.2593);
  PbarQjet->SetLineColor(kGreen);
  
  TF1* FUVQjet = new TF1("FUVQjet", "[0]*(PbarQjet)", 0.1,0.5);
  FUVQjet->SetParameter( 0,1);
  FUVQjet->SetLineColor(kBlack);

  // Gluon Jets: contributions from Pqg and Pgg
  // QG
  TF1* PbarQG = new TF1("PbarQG","1./[0] * ( x*x + (1-x)*(1-x) )", 0.1, 0.5);
  PbarQG->SetParameter( 0,0.2427 / (1./6.));
  PbarQG->SetLineColor(kRed);

  // GG
  TF1* PbarGG = new TF1("PbarGG","1./[0] * 6.0 * 2.0 * ( x/(1-x) + (1-x)/x + x*(1-x))", 0.1, 0.5);
  PbarGG->SetParameter( 0, 17.7107 / (1.-1./6.));
  PbarGG->SetLineColor(kGreen);

  // Sum
  TF1* FUVGjet = new TF1("FUVGjet", "PbarGG + PbarQG", 0.1,0.5);
  FUVGjet->SetLineColor(kGray+1);

  // =========================== Draw Comparison -- 20-30 GeV ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  TLegend* leg = new TLegend( 0.5, 0.65, 0.89, 0.9, "20<p_{T}<30, Leading Jet z_{g}" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);


  PythiaZgLead2030->SetTitle(";z_{g};arb.");
  PythiaZgLead2030->SetAxisRange(0,10,"y");
  PythiaZgLead2030->Draw();

  PythiaZgLead2030->SetMarkerStyle(21);
  PythiaZgLead2030->SetMarkerColor(kGray+1);
  PythiaZgLead2030->SetLineColor(kGray+1);
  PythiaZgLead2030->Draw("same");
  leg->AddEntry( PythiaZgLead2030, "20<p_{T}<30, Pythia");



  ppZgLead2030->SetTitle(";z_{g};arb.");
  ppZgLead2030->SetAxisRange(0,10,"y");

  ppZgLead2030->SetMarkerStyle(21);
  ppZgLead2030->SetMarkerColor(kBlack);
  ppZgLead2030->SetLineColor(kBlack);
  ppZgLead2030->Draw("same");
  leg->AddEntry( ppZgLead2030, "20<p_{T}<30, pp");

  AuAuZgLead2030->SetTitle(";z_{g};arb.");
  AuAuZgLead2030->SetAxisRange(0,10,"y");

  AuAuZgLead2030->SetMarkerStyle(21);
  AuAuZgLead2030->SetMarkerColor(kRed);
  AuAuZgLead2030->SetLineColor(kRed);
  AuAuZgLead2030->Draw("same");
  leg->AddEntry( AuAuZgLead2030, "20<p_{T}<30, AuAu");


  FUVQjet->Draw("same");
  leg->AddEntry( FUVQjet, "F_{UV} for quark jets");

  // FUVGjet->Draw("same");
  // leg->AddEntry( FUVGjet, "F_{UV}, gluons");

  leg->Draw();
  gPad->SaveAs(outbase + "_2030Zg.png");

  // =========================== Draw Comparison -- 30-40 GeV ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  TLegend* leg = new TLegend( 0.5, 0.65, 0.89, 0.9, "30<p_{T}<40, Leading Jet z_{g}" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);


  PythiaZgLead3040->SetTitle(";z_{g};arb.");
  PythiaZgLead3040->SetAxisRange(0,10,"y");
  PythiaZgLead3040->Draw();

  PythiaZgLead3040->SetMarkerStyle(21);
  PythiaZgLead3040->SetMarkerColor(kGray+1);
  PythiaZgLead3040->SetLineColor(kGray+1);
  PythiaZgLead3040->Draw("same");
  leg->AddEntry( PythiaZgLead3040, "30<p_{T}<40, Pythia");



  ppZgLead3040->SetTitle(";z_{g};arb.");
  ppZgLead3040->SetAxisRange(0,10,"y");

  ppZgLead3040->SetMarkerStyle(21);
  ppZgLead3040->SetMarkerColor(kBlack);
  ppZgLead3040->SetLineColor(kBlack);
  ppZgLead3040->Draw("same");
  leg->AddEntry( ppZgLead3040, "30<p_{T}<40, pp");

  AuAuZgLead3040->SetTitle(";z_{g};arb.");
  AuAuZgLead3040->SetAxisRange(0,10,"y");

  AuAuZgLead3040->SetMarkerStyle(21);
  AuAuZgLead3040->SetMarkerColor(kRed);
  AuAuZgLead3040->SetLineColor(kRed);
  AuAuZgLead3040->Draw("same");
  leg->AddEntry( AuAuZgLead3040, "30<p_{T}<40, AuAu");


  FUVQjet->Draw("same");
  leg->AddEntry( FUVQjet, "F_{UV} for quark jets");

  // FUVGjet->Draw("same");
  // leg->AddEntry( FUVGjet, "F_{UV}, gluons");

  leg->Draw();
  gPad->SaveAs(outbase + "_3040Zg.png");
  
}
