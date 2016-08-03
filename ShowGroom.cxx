{
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);

  // TFile* f = new TFile("Results/Groom_Lhc.root","READ");
  // TFile* f = new TFile("Results/Groom_Rhic.root","READ");
  TFile* f = new TFile("Results/Hi_Groom_Rhic.root","READ");
  // TFile* f = new TFile("Results/R05Groom_Rhic.root","READ");
  TTree* ResultTree = (TTree*) f->Get("ResultTree");

  TString outname = f->GetName();
  outname->ReplaceAll( ".root", ".Zg.root");
  TFile * out= new TFile ( outname, "RECREATE");

  TString outbase = gSystem->BaseName( f->GetName() );
  outbase.ReplaceAll( ".root", "");
  outbase.Prepend("Plots/");

  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);
  TH1D* LeadPt = new TH1D("LeadPt","Leading jet p_{T}", 120, 0, 240);
  TH1D* EmbLeadPt = new TH1D("EmbLeadPt","Embedded Leading jet p_{T}", 120, 0, 240);
  TH1D* GroomLeadPt = new TH1D("GroomLeadPt","Groomed Leading jet p_{T}", 120, 0, 240);
  TH1D* EmbGroomLeadPt = new TH1D("EmbGroomLeadPt","Groomed Embedded Leading jet p_{T}", 120, 0, 240);

  int nzgBins=40;
  TH1D* zgLead1020 = new TH1D("zgLead1020","Leading jet with 10<p_{T}<20", nzgBins, 0.05, 0.55);
  TH1D* zgLead2030 = new TH1D("zgLead2030","Leading jet with 20<p_{T}<30", nzgBins, 0.05, 0.55);
  TH1D* zgLead3040 = new TH1D("zgLead3040","Leading jet with 30<p_{T}<40", nzgBins, 0.05, 0.55);
  TH1D* zgLead4060 = new TH1D("zgLead4060","Leading jet with 40<p_{T}<60", nzgBins, 0.05, 0.55);
  TH1D* zgLead6080 = new TH1D("zgLead6080","Leading jet with 60<p_{T}<80", nzgBins, 0.05, 0.55);
  TH1D* zgLead80100 = new TH1D("zgLead80100","Leading jet with 80<p_{T}<nzgBins", nzgBins, 0.05, 0.55);
  TH1D* zgLead100Plus = new TH1D("zgLead100Plus","Leading jet with 100<p_{T}", nzgBins, 0.05, 0.55);

  TH1D* EmbzgLead1020 = new TH1D("EmbzgLead1020","Matched jet to 10<p_{T}<20", nzgBins, 0.05, 0.55);
  TH1D* EmbzgLead2030 = new TH1D("EmbzgLead2030","Matched jet to 20<p_{T}<30", nzgBins, 0.05, 0.55);
  TH1D* EmbzgLead3040 = new TH1D("EmbzgLead3040","Matched jet to 30<p_{T}<40", nzgBins, 0.05, 0.55);
  TH1D* EmbzgLead4060 = new TH1D("EmbzgLead4060","Matched jet to 40<p_{T}<60", nzgBins, 0.05, 0.55);
  TH1D* EmbzgLead6080 = new TH1D("EmbzgLead6080","Matched jet to 60<p_{T}<80", nzgBins, 0.05, 0.55);
  TH1D* EmbzgLead80100 = new TH1D("EmbzgLead80100","Matched jet to 80<p_{T}<nzgBins", nzgBins, 0.05, 0.55);
  TH1D* EmbzgLead100Plus = new TH1D("EmbzgLead100Plus","Matched jet to 100<p_{T}", nzgBins, 0.05, 0.55);


  
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
  
  TStarJetVectorJet* MatchedParton;
  TStarJetVectorJet* Jet;
  TStarJetVectorJet* EmbJet;
  TStarJetVectorJet* GroomedJet;
  TStarJetVectorJet* EmbGroomedJet;
  for ( int evi=0; evi<NEvents; ++evi){
    if ( !(evi%10000) ) cout << "Working on " << evi << " / " << NEvents << endl;
    ResultTree->GetEntry(evi);

    Jet = (TStarJetVectorJet*) pJets->At(0);
    if ( Jet.GetMatch()<0 ) continue;
    MatchedParton = (TStarJetVectorJet*) pHardPartons->At(Jet.GetMatch());
    // if ( MatchedParton->GetCharge() ==0 ) continue;
    

    // if ( Jet.GetMatch()<0 ) continue;
    float pt = Jet->Pt();
    GroomedJet = (TStarJetVectorJet*) pGroomedJets->At(0);
    float gpt = GroomedJet->Pt();
    
    LeadPt->Fill ( pt, weight );
    GroomLeadPt->Fill ( gpt, weight );

    if ( pt >= 10 && pt < 20 ){
      zgLead1020->Fill ( zg[0], weight );
    }

    if ( pt >= 20 && pt < 30 ){
      zgLead2030->Fill ( zg[0], weight );
    }
    if ( pt >= 30 && pt < 40 ){
      zgLead3040->Fill ( zg[0], weight );
    }
    if ( pt >= 40 && pt < 60 ){
      zgLead4060->Fill ( zg[0], weight );
    }
    if ( pt >= 60 && pt < 80 ){
      zgLead6080->Fill ( zg[0], weight );
    }
    if ( pt >= 80 && pt < 100 ){
      zgLead80100->Fill ( zg[0], weight );
    }
    if ( pt >= 100 ){
      zgLead100Plus->Fill ( zg[0], weight );
    }

    if ( pEmbJets->GetEntries()==0 ) continue;
    
    EmbJet = (TStarJetVectorJet*) pEmbJets->At(0);
    // if ( EmbJet.GetMatch()<0 || EmbJet.GetMatch()>1  ) continue;
    EmbGroomedJet = (TStarJetVectorJet*) pEmbGroomedJets->At(0);
    // float Embpt = Jet->Pt();
    float Embpt = EmbJet->Pt();
    float Embgpt = EmbGroomedJet->Pt();
    EmbLeadPt->Fill ( Embpt, weight );
    EmbGroomLeadPt->Fill ( Embgpt, weight );

    if ( pt >= 10 && pt < 20 ){
      EmbzgLead1020->Fill ( Embzg[0], weight );
    }
    if ( pt >= 20 && pt < 30 ){
      EmbzgLead2030->Fill ( Embzg[0], weight );
    }
    if ( pt >= 30 && pt < 40 ){
      EmbzgLead3040->Fill ( Embzg[0], weight );
    }
    if ( pt >= 40 && pt < 60 ){
      EmbzgLead4060->Fill ( Embzg[0], weight );
    }
    if ( pt >= 60 && pt < 80 ){
      EmbzgLead6080->Fill ( Embzg[0], weight );
    }
    if ( pt >= 80 && pt < 100 ){
      EmbzgLead80100->Fill ( Embzg[0], weight );
    }
    if ( pt >= 100 ){
      EmbzgLead100Plus->Fill ( Embzg[0], weight );
    }

    
  }

  // Also show theory
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


  TLegend* leg;
  // =========================== Draw spectra ===============================
  new TCanvas;
  leg = new TLegend( 0.55, 0.65, 0.89, 0.9, "" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  gPad->SetGridx(0);  gPad->SetGridy(0);
  gPad->SetLogy();

  if ( !(TString(f->GetName()).Contains("Lhc")) ){ 
    LeadPt->SetAxisRange(0,100);
  }
  LeadPt->SetTitle(";p_{T} [GeV];arb.");
  
  LeadPt->SetLineColor(kBlack);
  LeadPt->Draw();
  GroomLeadPt->SetLineColor(kGray+1);
  GroomLeadPt->Draw("same");

  EmbLeadPt->SetLineColor(kRed);
  EmbLeadPt->Draw("same");
  EmbGroomLeadPt->SetLineColor(kMagenta);
  EmbGroomLeadPt->Draw("same");

  leg->AddEntry( LeadPt->GetName(), "Leading Jet");
  leg->AddEntry( GroomLeadPt->GetName(), "Groomed Leading Jet");
  leg->AddEntry( EmbLeadPt->GetName(), "Embedded Leading Jet");
  leg->AddEntry( EmbGroomLeadPt->GetName(), "Groomed Embedded Leading Jet");
  
  leg->Draw("same");
  gPad->SaveAs( outbase + "_GroomSpectra.png");

  // =========================== Draw original ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  TLegend* leg = new TLegend( 0.55, 0.55, 0.89, 0.9, "Leading jet" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  zgLead1020->Scale ( 1./ zgLead1020->Integral() / zgLead1020->GetXaxis()->GetBinWidth(1));
  zgLead2030->Scale ( 1./ zgLead2030->Integral() / zgLead2030->GetXaxis()->GetBinWidth(1));
  zgLead3040->Scale ( 1./ zgLead3040->Integral() / zgLead3040->GetXaxis()->GetBinWidth(1));
  zgLead4060->Scale ( 1./ zgLead4060->Integral() / zgLead4060->GetXaxis()->GetBinWidth(1));
  zgLead6080->Scale ( 1./ zgLead6080->Integral() / zgLead6080->GetXaxis()->GetBinWidth(1));
  zgLead80100->Scale ( 1./ zgLead80100->Integral() / zgLead80100->GetXaxis()->GetBinWidth(1));
  zgLead100Plus->Scale ( 1./ zgLead100Plus->Integral() / zgLead100Plus->GetXaxis()->GetBinWidth(1));
    
  zgLead1020->SetTitle(";z_{g};arb.");
  zgLead1020->SetAxisRange(0,10,"y");

  zgLead2030->SetTitle(";z_{g};arb.");
  zgLead2030->SetAxisRange(0,10,"y");

  // zgLead1020->SetLineColor(kYellow+1);
  // zgLead1020->Draw();
  // leg->AddEntry( zgLead1020, "10<p_{T}<20");

  // zgLead2030->SetLineColor(kBlack);
  // zgLead2030->Draw("same");
  // leg->AddEntry( zgLead2030, "20<p_{T}<30");

  zgLead2030->SetLineColor(kBlack);
  zgLead2030->Draw("");
  leg->AddEntry( zgLead2030, "20<p_{T}<30");

  zgLead3040->SetLineColor(kRed);
  zgLead3040->Draw("same");
  leg->AddEntry( zgLead3040, "30<p_{T}<40");

  zgLead4060->SetLineColor(kGreen+1);
  zgLead4060->Draw("same");
  leg->AddEntry( zgLead4060, "40<p_{T}<60");
  
  if ( TString(f->GetName()).Contains("Lhc") ){ 
    zgLead6080->SetLineColor(kMagenta);
    zgLead6080->Draw("same");
    leg->AddEntry( zgLead6080, "60<p_{T}<80");
    
    zgLead80100->SetLineColor(kBlue);
    zgLead80100->Draw("same");
    leg->AddEntry( zgLead80100, "80<p_{T}<100");
    
    zgLead100Plus->SetLineColor(kGray+2);
    zgLead100Plus->Draw("same");
    leg->AddEntry( zgLead100Plus, "100<p_{T}");
  }

  FUVQjet->Draw("same");
  leg->AddEntry( FUVQjet, "F_{UV} (quarks)","l");
  // leg->AddEntry( FUVQjet, "F_{UV}, quarks");
  // FUVGjet->Draw("same");
  // leg->AddEntry( FUVGjet, "F_{UV}, gluons");

  leg->Draw();
  gPad->SaveAs(outbase + "_zg.png");

  // =========================== Draw matched ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  leg = new TLegend( 0.55, 0.55, 0.89, 0.9, "Leading after Embedding" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  EmbzgLead1020->Scale ( 1./ EmbzgLead1020->Integral() / EmbzgLead1020->GetXaxis()->GetBinWidth(1));
  EmbzgLead2030->Scale ( 1./ EmbzgLead2030->Integral() / EmbzgLead2030->GetXaxis()->GetBinWidth(1));
  EmbzgLead3040->Scale ( 1./ EmbzgLead3040->Integral() / EmbzgLead3040->GetXaxis()->GetBinWidth(1));
  EmbzgLead4060->Scale ( 1./ EmbzgLead4060->Integral() / EmbzgLead4060->GetXaxis()->GetBinWidth(1));
  EmbzgLead6080->Scale ( 1./ EmbzgLead6080->Integral() / EmbzgLead6080->GetXaxis()->GetBinWidth(1));
  EmbzgLead80100->Scale ( 1./ EmbzgLead80100->Integral() / EmbzgLead80100->GetXaxis()->GetBinWidth(1));
  EmbzgLead100Plus->Scale ( 1./ EmbzgLead100Plus->Integral() / EmbzgLead100Plus->GetXaxis()->GetBinWidth(1));
    
  EmbzgLead2030->SetTitle(";z_{g};arb.");
  EmbzgLead2030->SetAxisRange(0,10,"y");

  EmbzgLead2030->SetLineColor(kBlack);
  EmbzgLead2030->Draw();
  leg->AddEntry( EmbzgLead2030, "20<p_{T}<30");

  // EmbzgLead1020->SetLineColor(kYellow+1);
  // EmbzgLead1020->Draw("same");
  // leg->AddEntry( EmbzgLead1020, "10<p_{T}<20");

  EmbzgLead3040->SetLineColor(kRed);
  EmbzgLead3040->Draw("same");
  leg->AddEntry( EmbzgLead3040, "30<p_{T}<40");

  EmbzgLead4060->SetLineColor(kGreen+1);
  EmbzgLead4060->Draw("same");
  leg->AddEntry( EmbzgLead4060, "40<p_{T}<60");  
  
  if ( TString(f->GetName()).Contains("Lhc") ){ 
    EmbzgLead6080->SetLineColor(kGreen+1);
    EmbzgLead6080->Draw("same");
    leg->AddEntry( EmbzgLead6080, "60<p_{T}<80");
    
    EmbzgLead80100->SetLineColor(kBlue);
    EmbzgLead80100->Draw("same");
    leg->AddEntry( EmbzgLead80100, "80<p_{T}<100");
    
    EmbzgLead100Plus->SetLineColor(kGray+2);
    EmbzgLead100Plus->Draw("same");
    leg->AddEntry( EmbzgLead100Plus, "100<p_{T}");
  }      

  FUVQjet->Draw("same");
  leg->AddEntry( FUVQjet, "F_{UV} (quarks)", "l");
  // leg->AddEntry( FUVQjet, "F_{UV}, quarks");
  // FUVGjet->Draw("same");
  // leg->AddEntry( FUVGjet, "F_{UV}, gluons");

  leg->Draw();
  gPad->SaveAs(outbase + "_Embzg.png");
    

  // =========================== Draw Comparison ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  leg = new TLegend( 0.5, 0.55, 0.89, 0.9, "Leading Jet z_{g}" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);


  zgLead2030->SetTitle(";z_{g};arb.");
  zgLead2030->SetAxisRange(0,10,"y");
  zgLead2030->Draw();

  zgLead2030->SetMarkerStyle(21);
  zgLead2030->SetMarkerColor(kBlack);
  zgLead2030->SetLineColor(kBlack);
  zgLead2030->Draw("same");
  leg->AddEntry( zgLead2030, "20<p_{T}<30, Pythia");

  zgLead3040->SetMarkerStyle(20);
  zgLead3040->SetMarkerColor(kRed);
  zgLead3040->SetLineColor(kRed);
  zgLead3040->Draw("same");
  leg->AddEntry( zgLead3040, "30<p_{T}<40, Pythia");

  EmbzgLead2030->SetMarkerStyle(25);
  EmbzgLead2030->SetMarkerColor(kGray+1);
  EmbzgLead2030->SetLineColor(kGray+1);
  EmbzgLead2030->Draw("same");
  leg->AddEntry( EmbzgLead2030, "20<p_{T}<30, Pythia #oplus AuAu");

  EmbzgLead3040->SetMarkerStyle(24);
  EmbzgLead3040->SetMarkerColor(kMagenta);
  EmbzgLead3040->SetLineColor(kMagenta);
  EmbzgLead3040->Draw("same");
  leg->AddEntry( EmbzgLead3040, "30<p_{T}<40, Pythia #oplus AuAu");


  FUVQjet->Draw("same");
  leg->AddEntry( FUVQjet, "F_{UV} for quark jets");
  // FUVGjet->Draw("same");
  // leg->AddEntry( FUVGjet, "F_{UV}, gluons");

  leg->Draw();
  gPad->SaveAs(outbase + "_ComboZg.png");

  out->Write();
  
}
