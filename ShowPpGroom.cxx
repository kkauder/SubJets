{
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);

  int MinRefmult = 269;
  bool UseEmb = true;

  TFile* f = new TFile("Results/Groom_pp.root","READ");
  TTree* ResultTree = (TTree*) f->Get("ResultTree");

  TString outname = f->GetName();
  outname->ReplaceAll( ".root", ".Zg.root");
  TFile * out= new TFile ( outname, "RECREATE");

  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);
  TH1D* LeadPt = new TH1D("LeadPt","Leading jet p_{T}", 60, 0, 60);
  TH1D* GroomLeadPt = new TH1D("GroomLeadPt","Groomed Leading jet p_{T}", 60, 0, 60);
 
  int nzgBins=40;
  TH1D* zgLead2030 = new TH1D("zgLead2030","Leading jet with 20<p_{T}<30", nzgBins, 0.05, 0.55);
  TH1D* zgLead3040 = new TH1D("zgLead3040","Leading jet with 30<p_{T}<40", nzgBins, 0.05, 0.55);
  TH1D* zgLead4060 = new TH1D("zgLead4060","Leading jet with 40<p_{T}<60", nzgBins, 0.05, 0.55);

  
  TClonesArray* pJets = new TClonesArray( "TStarJetVectorJet" ); 
  TClonesArray* pGroomedJets = new TClonesArray( "TStarJetVectorJet" ); 

  double weight=1;

  int njets=0;
  double zg[1000];

  double Embrefmult;
  ResultTree->SetBranchAddress("Embrefmult", &Embrefmult );

  if ( UseEmb ) { 
    ResultTree->SetBranchAddress("Embnjets", &njets);
    ResultTree->SetBranchAddress("Embzg", zg);
    ResultTree->SetBranchAddress("EmbJets", &pJets );
    ResultTree->SetBranchAddress("EmbGroomedJets", &pGroomedJets );
    
  } else {
    ResultTree->SetBranchAddress("Jets", &pJets );
    ResultTree->SetBranchAddress("njets", &njets);
    ResultTree->SetBranchAddress("zg", zg);
    ResultTree->SetBranchAddress("GroomedJets", &pGroomedJets );
  }


  Long64_t NEvents = ResultTree->GetEntries();
  // NEvents = 10000;
  
  TStarJetVectorJet* Jet;
  TStarJetVectorJet* GroomedJet;
  for ( int evi=0; evi<NEvents; ++evi){
    if ( !(evi%10000) ) cout << "Working on " << evi << " / " << NEvents << endl;
    ResultTree->GetEntry(evi);

    //if ( UseEmb && Embrefmult<MinRefmult ) continue;
    if ( pJets->GetEntries()<1 ) continue;
    
    Jet = (TStarJetVectorJet*) pJets->At(0);
    float pt = Jet->Pt();

    GroomedJet = (TStarJetVectorJet*) pGroomedJets->At(0);
    float gpt = GroomedJet->Pt();
    
    LeadPt->Fill ( pt, weight );
    GroomLeadPt->Fill ( gpt, weight );

    if ( pt >= 20 && pt < 30 ){
      zgLead2030->Fill ( zg[0], weight );
    }
    if ( pt >= 30 && pt < 40 ){
      zgLead3040->Fill ( zg[0], weight );
    }
    if ( pt >= 40 && pt < 60 ){
      zgLead4060->Fill ( zg[0], weight );
    }
    
  }

  TLegend* leg;
  // =========================== Draw spectra ===============================
  new TCanvas;
  leg = new TLegend( 0.55, 0.75, 0.89, 0.9, "pp HT" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  gPad->SetGridx(0);  gPad->SetGridy(0);
  gPad->SetLogy();

  LeadPt->SetTitle(";p_{T} [GeV];arb.");
  
  LeadPt->SetLineColor(kBlack);
  LeadPt->Draw();
  GroomLeadPt->SetLineColor(kGray+1);
  GroomLeadPt->Draw("same");

  leg->AddEntry( LeadPt->GetName(), "Leading Jet");
  leg->AddEntry( GroomLeadPt->GetName(), "Groomed Leading Jet");
  
  leg->Draw("same");
  gPad->SaveAs("Plots/PpGroomSpectra.png");

  // =========================== Draw original ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  TLegend* leg = new TLegend( 0.6, 0.65, 0.89, 0.9, "pp HT, Leading jet" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  zgLead2030->Scale ( 1./ zgLead2030->Integral() / zgLead2030->GetXaxis()->GetBinWidth(1));
  zgLead3040->Scale ( 1./ zgLead3040->Integral() / zgLead3040->GetXaxis()->GetBinWidth(1));
  zgLead4060->Scale ( 1./ zgLead4060->Integral() / zgLead4060->GetXaxis()->GetBinWidth(1));
    
  zgLead2030->SetTitle(";z_{g};arb.");
  zgLead2030->SetAxisRange(0,10,"y");

  zgLead2030->SetLineColor(kBlack);
  zgLead2030->Draw();
  leg->AddEntry( zgLead2030, "20<p_{T}<30");

  zgLead3040->SetLineColor(kMagenta);
  zgLead3040->Draw("same");
  leg->AddEntry( zgLead3040, "30<p_{T}<40");

  // zgLead4060->SetLineColor(kRed);
  // zgLead4060->Draw("same");
  // leg->AddEntry( zgLead4060, "40<p_{T}<60");
  

  leg->Draw();
  gPad->SaveAs("Plots/PpZg.png");
    
  out->Write();
  
}
