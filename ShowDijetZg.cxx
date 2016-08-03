{
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);

  int RebinZg=2;

  // TString sAuAu = "Results/Groom_AuAu.DijetZg.root";
  // TString spp = "Results/Groom_pp.DijetZg.root";
  
  // TString sAuAu = "Results/Groom_Fresh_NicksList_AuAu.PicoAjZg.root";
  // TString spp = "Results/Tow0_Eff0_Groom_ppInAuAuAj.PicoAjZg.root";
  // TString spp = "Results/Tow0_Eff0_Groom_Fresh_NicksList_HC100_ppAj.PicoAjZg.root";

  // TString sAuAu = "Results/Groom_Fresh_NicksList_AuAu.PicoAjZg_AbsAjGt0.3.root";
  // TString spp = "Results/Tow0_Eff0_Groom_ppInAuAuAj.PicoAjZg_AbsAjGt0.3.root";
  TString sAuAu = "Results/Groom_Fresh_NicksList_AuAu.PicoAjZg_AbsAjLt0.3.root";
  TString spp = "Results/Tow0_Eff0_Groom_ppInAuAuAj.PicoAjZg_AbsAjLt0.3.root";

  // TString sAuAu = "Results/Badrefmult_Groom_AuAu.DijetZg.root";
  // TString spp = "Results/Badrefmult_Groom_pp.DijetZg.root";
  
  // TString helperstring="";
  // if ( sAuAu.Contains("AbsAjGt") ){
  //   if ( spp.Contains("AbsAjGt") ){
  //     cerr << "Inconsistent Aj cuts!" << endl;
  //     return -1;
  //   }
  //   helperstring +="AbsAjGt";

  TString outbase = gSystem->BaseName(sAuAu);
  outbase->ReplaceAll (".root","");
  outbase+="___";
  outbase+=gSystem->BaseName(spp);
  outbase->ReplaceAll (".root","");

  
  // --------------------------------------------------------------
  // ------------------------ Load AuAu ---------------------------
  // --------------------------------------------------------------
  TFile * fAuAu = new TFile( sAuAu, "READ");
  TH1D* AuAu_zgLead2030Hi  = fAuAu->Get("zgLead2030Hi");
  AuAu_zgLead2030Hi->SetName("AuAu_zgLead2030Hi");
  TH1D* AuAu_zgLead3040Hi  = fAuAu->Get("zgLead3040Hi");
  AuAu_zgLead3040Hi->SetName("AuAu_zgLead3040Hi");

  TH1D* AuAu_zgSubLead1020Hi  = fAuAu->Get("zgSubLead1020Hi");
  AuAu_zgSubLead1020Hi->SetName("AuAu_zgSubLead1020Hi");
  TH1D* AuAu_zgSubLead2030Hi  = fAuAu->Get("zgSubLead2030Hi");
  AuAu_zgSubLead2030Hi->SetName("AuAu_zgSubLead2030Hi");
  TH1D* AuAu_zgSubLead3040Hi  = fAuAu->Get("zgSubLead3040Hi");
  AuAu_zgSubLead3040Hi->SetName("AuAu_zgSubLead3040Hi");

  TH1D* AuAu_zgLead2030Lo  = fAuAu->Get("zgLead2030Lo");
  AuAu_zgLead2030Lo->SetName("AuAu_zgLead2030Lo");
  TH1D* AuAu_zgLead3040Lo  = fAuAu->Get("zgLead3040Lo");
  AuAu_zgLead3040Lo->SetName("AuAu_zgLead3040Lo");

  TH1D* AuAu_zgSubLead1020Lo  = fAuAu->Get("zgSubLead1020Lo");
  AuAu_zgSubLead1020Lo->SetName("AuAu_zgSubLead1020Lo");
  TH1D* AuAu_zgSubLead2030Lo  = fAuAu->Get("zgSubLead2030Lo");
  AuAu_zgSubLead2030Lo->SetName("AuAu_zgSubLead2030Lo");
  TH1D* AuAu_zgSubLead3040Lo  = fAuAu->Get("zgSubLead3040Lo");
  AuAu_zgSubLead3040Lo->SetName("AuAu_zgSubLead3040Lo");

  // --------------------------------------------------------------
  // ------------------------- Load pp ----------------------------
  // --------------------------------------------------------------
  TFile * fpp = new TFile( spp, "READ");
  TH1D* pp_zgLead2030Hi  = fpp->Get("zgLead2030Hi");
  pp_zgLead2030Hi->SetName("pp_zgLead2030Hi");
  TH1D* pp_zgLead3040Hi  = fpp->Get("zgLead3040Hi");
  pp_zgLead3040Hi->SetName("pp_zgLead3040Hi");

  TH1D* pp_zgSubLead1020Hi  = fpp->Get("zgSubLead1020Hi");
  pp_zgSubLead1020Hi->SetName("pp_zgSubLead1020Hi");
  TH1D* pp_zgSubLead2030Hi  = fpp->Get("zgSubLead2030Hi");
  pp_zgSubLead2030Hi->SetName("pp_zgSubLead2030Hi");
  TH1D* pp_zgSubLead3040Hi  = fpp->Get("zgSubLead3040Hi");
  pp_zgSubLead3040Hi->SetName("pp_zgSubLead3040Hi");

  TH1D* pp_zgLead2030Lo  = fpp->Get("zgLead2030Lo");
  pp_zgLead2030Lo->SetName("pp_zgLead2030Lo");
  TH1D* pp_zgLead3040Lo  = fpp->Get("zgLead3040Lo");
  pp_zgLead3040Lo->SetName("pp_zgLead3040Lo");

  TH1D* pp_zgSubLead1020Lo  = fpp->Get("zgSubLead1020Lo");
  pp_zgSubLead1020Lo->SetName("pp_zgSubLead1020Lo");
  TH1D* pp_zgSubLead2030Lo  = fpp->Get("zgSubLead2030Lo");
  pp_zgSubLead2030Lo->SetName("pp_zgSubLead2030Lo");
  TH1D* pp_zgSubLead3040Lo  = fpp->Get("zgSubLead3040Lo");
  pp_zgSubLead3040Lo->SetName("pp_zgSubLead3040Lo");

  // Prep Theory curve
  TF1* PbarQjet = new TF1("PbarQjet","1./[0] * 4./3. * ( (1+pow(x,2))/(1-x) + (1 + pow(1-x,2))/x )", 0.1, 0.5);
  PbarQjet->SetParameter( 0,4.2593);
  PbarQjet->SetLineColor(kGreen);
  
  TF1* FUVQjet = new TF1("FUVQjet", "[0]*(PbarQjet)", 0.1,0.5);
  FUVQjet->SetParameter( 0,1);

  FUVQjet->SetLineColor(kGray+2);
  FUVQjet->SetLineWidth(1);
  FUVQjet->SetLineStyle(2);
  FUVQjet->SetParameter( 0,1);

  // --------------------------------------------------------------
  // --------------------- Prettifications ------------------------
  // --------------------------------------------------------------
  TObjArray toa;
  toa.Add(AuAu_zgLead2030Hi);
  toa.Add(AuAu_zgLead3040Hi);
  toa.Add(AuAu_zgSubLead1020Hi);
  toa.Add(AuAu_zgSubLead2030Hi);
  toa.Add(AuAu_zgSubLead3040Hi);

  toa.Add(pp_zgLead2030Hi);
  toa.Add(pp_zgLead3040Hi);
  toa.Add(pp_zgSubLead1020Hi);
  toa.Add(pp_zgSubLead2030Hi);
  toa.Add(pp_zgSubLead3040Hi);

  toa.Add(AuAu_zgLead2030Lo);
  toa.Add(AuAu_zgLead3040Lo);
  toa.Add(AuAu_zgSubLead1020Lo);
  toa.Add(AuAu_zgSubLead2030Lo);
  toa.Add(AuAu_zgSubLead3040Lo);

  toa.Add(pp_zgLead2030Lo);
  toa.Add(pp_zgLead3040Lo);
  toa.Add(pp_zgSubLead1020Lo);
  toa.Add(pp_zgSubLead2030Lo);
  toa.Add(pp_zgSubLead3040Lo);

  TH1D* h;
  for (int i=0 ; i<toa.GetEntries() ; ++i ){
    h=(TH1D*) toa.At(i);

    h->Scale ( 1./ h->Integral() / h->GetXaxis()->GetBinWidth(1));

    h->Rebin( RebinZg );
    h->Scale(1./RebinZg );

    h->SetAxisRange( 0,14, "y" );
    h->SetLineWidth( 2 );

    h->SetTitle(";z_{g};arb.");
    h->GetXaxis()->SetTitleFont( 42 ); // 42: helvetica, 62: helvetica bold
    h->GetXaxis()->SetLabelFont( 42 ); // 42: helvetica, 62: helvetica bold
    h->GetYaxis()->SetTitleFont( 42 ); // 42: helvetica, 62: helvetica bold
    h->GetYaxis()->SetLabelFont( 42 ); // 42: helvetica, 62: helvetica bold

    if ( TString(h->GetName()).Contains("AuAu") ){
      h->SetMarkerStyle( 21 );
    } else if ( TString(h->GetName()).Contains("pp") ){
      h->SetMarkerStyle( 25 );
    }

    if ( TString(h->GetName()).Contains("Hi") ){
      h->SetLineColor( kRed );
      h->SetMarkerColor( kRed );
    } else if ( TString(h->GetName()).Contains("Lo") ){
      h->SetLineColor( kBlack );
      h->SetMarkerColor( kBlack );
    }
    
  }


  // --------------------------------------------------------------
  // ----------------------- Draw Leading -------------------------
  // --------------------------------------------------------------

  // 20-30
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  TLegend* leg = new TLegend( 0.55, 0.55, 0.89, 0.9, "Leading jet, 20-30 GeV" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  AuAu_zgLead2030Hi->Draw("");
  leg->AddEntry( AuAu_zgLead2030Hi, "Au+Au, Hi Cut");
  
  pp_zgLead2030Hi->Draw("same");
  leg->AddEntry( pp_zgLead2030Hi, "pp #oplus Au+Au, Hi Cut");

  AuAu_zgLead2030Lo->Draw("same");
  leg->AddEntry( AuAu_zgLead2030Lo, "Au+Au, Lo Cut");
  
  pp_zgLead2030Lo->Draw("same");
  leg->AddEntry( pp_zgLead2030Lo, "pp #oplus Au+Au, Lo Cut");

  FUVQjet->Draw("same");
  leg->AddEntry( FUVQjet, "F_{UV} (quarks)","l");
  leg->Draw();
  cout << outbase << endl;

  gPad->SaveAs( "Plots/"+  outbase + ".DijetLeadZg2030.png");

  if ( false ){
    // 30-40
    new TCanvas;
    gPad->SetGridx(0);  gPad->SetGridy(0);
    TLegend* leg = new TLegend( 0.55, 0.55, 0.89, 0.9, "Leading jet, 30-40 GeV" );
    leg->SetBorderSize(0);
    leg->SetLineWidth(10);
    leg->SetFillStyle(0);
    leg->SetMargin(0.1);
    
    AuAu_zgLead3040Hi->Draw("");
    leg->AddEntry( AuAu_zgLead3040Hi, "Au+Au, Hi Cut");
    
    pp_zgLead3040Hi->Draw("same");
    leg->AddEntry( pp_zgLead3040Hi, "pp #oplus Au+Au, Hi Cut");
    
    AuAu_zgLead3040Lo->Draw("same");
    leg->AddEntry( AuAu_zgLead3040Lo, "Au+Au, Lo Cut");
    
    pp_zgLead3040Lo->Draw("same");
    leg->AddEntry( pp_zgLead3040Lo, "pp #oplus Au+Au, Lo Cut");
    
    FUVQjet->Draw("same");
    leg->AddEntry( FUVQjet, "F_{UV} (quarks)","l");
    leg->Draw();
    gPad->SaveAs( "Plots/"+  outbase + ".DijetLeadZg3040.png");
  }

  // --------------------------------------------------------------
  // ----------------------- Draw SubLeading -------------------------
  // --------------------------------------------------------------

  // 10-20
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  TLegend* leg = new TLegend( 0.55, 0.55, 0.89, 0.9, "SubLeading jet, 10-20 GeV" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  AuAu_zgSubLead1020Hi->Draw("");
  leg->AddEntry( AuAu_zgSubLead1020Hi, "Au+Au, Hi Cut");
  
  pp_zgSubLead1020Hi->Draw("same");
  leg->AddEntry( pp_zgSubLead1020Hi, "pp #oplus Au+Au, Hi Cut");

  AuAu_zgSubLead1020Lo->Draw("same");
  leg->AddEntry( AuAu_zgSubLead1020Lo, "Au+Au, Lo Cut");
  
  pp_zgSubLead1020Lo->Draw("same");
  leg->AddEntry( pp_zgSubLead1020Lo, "pp #oplus Au+Au, Lo Cut");

  FUVQjet->Draw("same");
  leg->AddEntry( FUVQjet, "F_{UV} (quarks)","l");
  leg->Draw();
  gPad->SaveAs( "Plots/"+  outbase + ".DijetSubLeadZg1020.png");

  // 20-30
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  TLegend* leg = new TLegend( 0.55, 0.55, 0.89, 0.9, "SubLeading jet, 20-30 GeV" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  AuAu_zgSubLead2030Hi->Draw("");
  leg->AddEntry( AuAu_zgSubLead2030Hi, "Au+Au, Hi Cut");
  
  pp_zgSubLead2030Hi->Draw("same");
  leg->AddEntry( pp_zgSubLead2030Hi, "pp #oplus Au+Au, Hi Cut");

  AuAu_zgSubLead2030Lo->Draw("same");
  leg->AddEntry( AuAu_zgSubLead2030Lo, "Au+Au, Lo Cut");
  
  pp_zgSubLead2030Lo->Draw("same");
  leg->AddEntry( pp_zgSubLead2030Lo, "pp #oplus Au+Au, Lo Cut");

  FUVQjet->Draw("same");
  leg->AddEntry( FUVQjet, "F_{UV} (quarks)","l");
  leg->Draw();
  gPad->SaveAs( "Plots/"+  outbase + ".DijetSubLeadZg2030.png");

}
