{
  double R = 0.4; // for geom. matching

  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);

  TString fname = "Results/Groom_pp.root";
  // TString fname = "Results/Groom_ppEmb.root";
  // TString fname = "Results/HighConsGroom_SmallAuAu.root";
  // TString fname = "Results/Groom_AuAu.root";

  TFile* f = new TFile(fname,"READ");

  bool UseEmb = fname.Contains("Emb");

  TTree* ResultTree = (TTree*) f->Get("ResultTree");

  TString outbase = gSystem->BaseName( f->GetName() );
  outbase.ReplaceAll( ".root", "");
  outbase.Prepend("Plots/");

  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);
  TH1D* LeadPt = new TH1D("LeadPt","Leading jet p_{T}", 60, 0, 60);
  TH1D* GroomLeadPt = new TH1D("GroomLeadPt","Groomed Leading jet p_{T}", 60, 0, 60);
  TH1D* SubLeadPt = new TH1D("SubLeadPt","SubLeading jet p_{T}", 60, 0, 60);
  TH1D* GroomSubLeadPt = new TH1D("GroomSubLeadPt","Groomed SubLeading jet p_{T}", 60, 0, 60);
 
  int nzgBins=20;
  TH1D* zgLead2030 = new TH1D("zgLead2030","Leading jet with 20<p_{T}<30", nzgBins, 0.05, 0.55);
  TH1D* zgLead3040 = new TH1D("zgLead3040","Leading jet with 30<p_{T}<40", nzgBins, 0.05, 0.55);
  TH1D* zgLead4060 = new TH1D("zgLead4060","Leading jet with 40<p_{T}<60", nzgBins, 0.05, 0.55);

  TH1D* zgSubLead1020 = new TH1D("zgSubLead1020","SubLeading jet with 10<p_{T}<20", nzgBins, 0.05, 0.55);
  TH1D* zgSubLead2030 = new TH1D("zgSubLead2030","SubLeading jet with 20<p_{T}<30", nzgBins, 0.05, 0.55);
  TH1D* zgSubLead3040 = new TH1D("zgSubLead3040","SubLeading jet with 30<p_{T}<40", nzgBins, 0.05, 0.55);
  TH1D* zgSubLead4060 = new TH1D("zgSubLead4060","SubLeading jet with 40<p_{T}<60", nzgBins, 0.05, 0.55);

  
  TClonesArray* pJets = new TClonesArray( "TStarJetVectorJet" ); 
  TClonesArray* pGroomedJets = new TClonesArray( "TStarJetVectorJet" ); 
  if ( UseEmb ) {
    ResultTree->SetBranchAddress("EmbJets", &pJets );
    ResultTree->SetBranchAddress("EmbGroomedJets", &pGroomedJets );
  } else {
    ResultTree->SetBranchAddress("Jets", &pJets );
    ResultTree->SetBranchAddress("GroomedJets", &pGroomedJets );
  }

  double weight=1;

  int njets=0;
  ResultTree->SetBranchAddress("njets", &njets);
  double zg[1000];
  ResultTree->SetBranchAddress("zg", zg);

  Long64_t NEvents = ResultTree->GetEntries();
  // NEvents = 10000;
  
  TStarJetVectorJet* LeadJet;
  TStarJetVectorJet* GroomedLeadJet;
  TStarJetVectorJet* SubLeadJet;
  TStarJetVectorJet* GroomedSubLeadJet;
  for ( int evi=0; evi<NEvents; ++evi){
    if ( !(evi%10000) ) cout << "Working on " << evi << " / " << NEvents << endl;
    ResultTree->GetEntry(evi);

    LeadJet = (TStarJetVectorJet*) pJets->At(0);
    // if ( Jet.GetMatch()<0 ) continue;
    float pt = LeadJet->Pt();
    GroomedLeadJet = (TStarJetVectorJet*) pGroomedJets->At(0);
    float gpt = GroomedLeadJet->Pt();
    
    if ( pt<20 ) continue;

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
    
    // Now look for a di-jet partner
    // -----------------------------
    SubLeadJet=0;
    int SubLeadIndex=-1;
    for ( int i =1; i<pJets->GetEntries() ; ++i ){      
      TStarJetVectorJet* Jet = (TStarJetVectorJet*) pJets->At(i);
      // cout << Jet->DeltaPhi( -(*LeadJet) ) << endl;

      if ( fabs ( Jet->DeltaPhi( -(*LeadJet) ) ) < R ){
	if (SubLeadJet) {
	  if ( Jet->Pt() >SubLeadJet->Pt() ){
	    SubLeadJet = Jet;
	    SubLeadIndex = i;
	  }
	} else {	  
	  SubLeadJet = Jet;
	  SubLeadIndex = i;
	}
      }

    }

    if (SubLeadJet) {
      float pt = SubLeadJet->Pt();
    
      SubLeadPt->Fill ( pt, weight );
      
      if ( pt >= 10 && pt < 20 ){
        zgSubLead1020->Fill ( zg[SubLeadIndex], weight );
      }
      if ( pt >= 20 && pt < 30 ){
        zgSubLead2030->Fill ( zg[SubLeadIndex], weight );
      }
      if ( pt >= 30 && pt < 40 ){
        zgSubLead3040->Fill ( zg[SubLeadIndex], weight );
      }
      if ( pt >= 40 && pt < 60 ){
        zgSubLead4060->Fill ( zg[SubLeadIndex], weight );
      }
    }

  }

  TLegend* leg;
  // =========================== Draw spectra ===============================
  new TCanvas;
  leg = new TLegend( 0.55, 0.7, 0.89, 0.9, "pp HT" );
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
  SubLeadPt->SetLineColor(kGreen+1);
  SubLeadPt->Draw("same");

  leg->AddEntry( LeadPt->GetName(), "Leading Jet");
  leg->AddEntry( GroomLeadPt->GetName(), "Groomed Leading Jet");
  leg->AddEntry( SubLeadPt->GetName(), "Matching SubLeading Jet");
  
  leg->Draw("same");
  gPad->SaveAs("Plots/PpGroomSpectra.png");

  // =========================== Draw Leading zg ===============================
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

  // zgLead3040->SetLineColor(kMagenta);
  // zgLead3040->Draw("same");
  // leg->AddEntry( zgLead3040, "30<p_{T}<40");

  // zgLead4060->SetLineColor(kRed);
  // zgLead4060->Draw("same");
  // leg->AddEntry( zgLead4060, "40<p_{T}<60");
  

  leg->Draw();
  gPad->SaveAs("Plots/LeadZg.png");
    
  // =========================== Draw SubLeading zg ===============================
  new TCanvas;
  gPad->SetGridx(0);  gPad->SetGridy(0);
  TLegend* leg = new TLegend( 0.6, 0.65, 0.89, 0.9, "pp HT, SubLeading jet" );
  leg->SetBorderSize(0);
  leg->SetLineWidth(10);
  leg->SetFillStyle(0);
  leg->SetMargin(0.1);

  zgSubLead1020->Scale ( 1./ zgSubLead1020->Integral() / zgSubLead1020->GetXaxis()->GetBinWidth(1));
  zgSubLead2030->Scale ( 1./ zgSubLead2030->Integral() / zgSubLead2030->GetXaxis()->GetBinWidth(1));
  zgSubLead3040->Scale ( 1./ zgSubLead3040->Integral() / zgSubLead3040->GetXaxis()->GetBinWidth(1));
  zgSubLead4060->Scale ( 1./ zgSubLead4060->Integral() / zgSubLead4060->GetXaxis()->GetBinWidth(1));
    
  zgSubLead1020->SetTitle(";z_{g};arb.");
  zgSubLead1020->SetAxisRange(0,10,"y");

  zgSubLead1020->SetLineColor(kGreen+1);
  zgSubLead1020->Draw();
  leg->AddEntry( zgSubLead1020, "10<p_{T}<20");

  zgSubLead2030->SetTitle(";z_{g};arb.");
  zgSubLead2030->SetAxisRange(0,10,"y");

  zgSubLead2030->SetLineColor(kBlack);
  zgSubLead2030->Draw("same");
  leg->AddEntry( zgSubLead2030, "20<p_{T}<30");

  zgSubLead3040->SetLineColor(kMagenta);
  zgSubLead3040->Draw("same");
  leg->AddEntry( zgSubLead3040, "30<p_{T}<40");

  // zgSubLead4060->SetLineColor(kRed);
  // zgSubLead4060->Draw("same");
  // leg->AddEntry( zgSubLead4060, "40<p_{T}<60");
  

  leg->Draw();
  gPad->SaveAs("Plots/SubLeadZg.png");

  
}
