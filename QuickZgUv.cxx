{
  gStyle->SetOptStat(0);
  gStyle->SetOptDate(0);

  // Quark Jets: contributions from Pqq and Pgq, but
  // PbarQQ == PbarGQ = Pqq + Pgq
  TF1* PbarQjet = new TF1("PbarQjet","1./[0] * 4./3. * ( (1+pow(x,2))/(1-x) + (1 + pow(1-x,2))/x )", 0.1, 0.5);
  PbarQjet->SetParameter( 0,4.2593);
  PbarQjet->SetLineColor(kGreen);
  
  TF1* FUVQjet = new TF1("FUVQjet", "[0]*(PbarQjet)", 0.1,0.5);
  FUVQjet->SetParameter( 0,1);
  FUVQjet->SetLineColor(kBlack);
  new TCanvas;
  TH1D* dummy = new TH1D("dummy","Quark Jets;z_g",100, 0, 0.6);
  dummy->SetAxisRange(0,10,"y");

  dummy->Draw();
  FUVQjet->Draw("same");
  PbarQjet->Draw("same");
  TLegend* leg = new TLegend( 0.6, 0.66, 0.8, 0.85 );
  leg->AddEntry(PbarQjet, "#bar{P_{qq}} = #bar{P_{gq}} (q->qg + q->gq)","l");
  leg->AddEntry(FUVQjet, "Sum","l");
  leg->Draw();



  // Gluon Jets: contributions from Pqg and Pgg

  // QG
  TF1* PbarQG = new TF1("PbarQG","1./[0] * ( x*x + (1-x)*(1-x) )", 0.1, 0.5);
  PbarQG->SetParameter( 0,0.2427 / (1./6.));
  PbarQG->SetLineColor(kRed);

  // GG
  TF1* PbarGG = new TF1("PbarGG","1./[0] * 6.0 * 2.0 * ( x/(1-x) + (1-x)/x + x*(1-x))", 0.1, 0.5);
  PbarGG->SetParameter( 0, 17.7107 / (1.-1./6.));
  PbarGG->SetLineColor(kGreen);



  TF1* FUVGjet = new TF1("FUVGjet", "PbarGG + PbarQG", 0.1,0.5);
  // FUVGjet->SetParameters( 1.-1./6., 1./6);
  FUVGjet->SetLineColor(kBlack);
  new TCanvas;
  TH1D* dummy2 = new TH1D("dummy2",";z_g",100, 0, 0.6);
  dummy2->SetAxisRange(0,10,"y");

  dummy2->Draw();
  FUVGjet->Draw("same");
  FUVQjet->SetLineColor(kMagenta);
  FUVQjet->Draw("same");
  PbarGG->Draw("same");
  PbarQG->Draw("same");

  TLegend* leg = new TLegend( 0.6, 0.6, 0.8, 0.85 );
  leg->AddEntry(PbarGG, "#bar{P_{gg}} (g->gg)","l");
  leg->AddEntry(PbarQG, "#bar{P_{qg}} (g->q#bar{q})","l");
  leg->AddEntry(FUVGjet, "Gluon Jets","l");
  leg->AddEntry(FUVQjet, "Quark Jets","l");
  leg->Draw();
  

  // qg: 0.2427
  // qq: 4.2593
  // gg: 17.7107
  

}
