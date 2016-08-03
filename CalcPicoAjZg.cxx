
double CalcAj ( TLorentzVector* j1, TLorentzVector* j2, bool nofabs=true );
  
//int CalcPicoAjZg(  TString sFile = "~/PaperAj/BasicAj/AjResults/Tow0_Eff0_Groom_ppInAuAuAj.root", float FabsAjCut=0.3, int dir=-1 ){
int CalcPicoAjZg(  TString sFile = "~/PaperAj/BasicAj/AjResults/Groom_Fresh_NicksList_AuAu.root", float FabsAjCut=0.3, int dir=-1 ){
  char c[100];
  TString sFabsAjCut="";
  if ( fabs(FabsAjCut)>1e-4 ){
    sprintf (c,"%0.1f",FabsAjCut);    
    if ( dir > 0 ){
      sFabsAjCut="_AbsAjGt";
    } else if ( dir < 0 ){
      sFabsAjCut="_AbsAjLt";
    } else return -1;

    sFabsAjCut+=c;
    // sFabsAjCut+="_";
  }
  
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);
  
  int RebinZg=2;
  int MinRefmult = 269;
  if ( sFile.Contains("ppAj") )   MinRefmult = 0;
  
  
  TString spp = ;
  
  // --------------------------------------------------------------
  // ------------------------ Load AuAu ---------------------------
  // --------------------------------------------------------------
  TFile * f = new TFile( sFile, "READ");  
  
  TTree* ResultTree = (TTree*) f->Get("ResultTree");
  ResultTree->SetName("ResultTree");
  
  double refmult;
  ResultTree->SetBranchAddress("refmult", &refmult );
  float zg1, zg2, zgm1, zgm2;
  ResultTree->SetBranchAddress("zg1",&zg1);
  ResultTree->SetBranchAddress("zg2",&zg2);
  ResultTree->SetBranchAddress("zgm1",&zgm1);
  ResultTree->SetBranchAddress("zgm2",&zgm2);
    
  TLorentzVector *pJ1 = new TLorentzVector();
  TLorentzVector *pJ2 = new TLorentzVector();
  TLorentzVector *pJM1 = new TLorentzVector();
  TLorentzVector *pJM2 = new TLorentzVector();
  double refmult;

  ResultTree->SetBranchAddress("j1", &pJ1);
  ResultTree->SetBranchAddress("j2", &pJ2);
  ResultTree->SetBranchAddress("jm1", &pJM1);
  ResultTree->SetBranchAddress("jm2", &pJM2);

  // Open file for writing
  TString outbase = gSystem->BaseName( sFile );
  outbase.ReplaceAll( ".root", "");
  TFile* fOut = new TFile("Results/" + outbase + ".PicoAjZg" + sFabsAjCut + ".root","RECREATE");

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

  // Histos
  // -------
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);

  TH2D* LeadPtLoVPtHi = new TH2D("LeadPtLoVPtHi", ";p_{T,Hi};p_{T,Lo}",100, 0, 60, 100, 0, 60 );
  TH1D* LeadPtLoMinusPtHi = new TH1D("LeadPtLoMinusPtHi", ";p_{T,Lo}-p_{T,Hi}",120, -30, 30 );

  TH1D* hLeadPtHi = new TH1D( "hLeadPtHi","p_{T}^{C} > 2 GeV/c;p_{T}^{lead} [GeV/c]", 100, 10 , 60 );
  TH1D* hLeadPtLo = new TH1D( "hLeadPtLo","p_{T}^{C} > 0.2 GeV/c;p_{T}^{lead} [GeV/c]", 100, 10 , 60 );
  TH1D* hSubLeadPtHi = new TH1D( "hSubLeadPtHi","p_{T}^{C} > 2 GeV/c;p_{T}^{sublead} [GeV/c]", 100, 0 , 50 );
  TH1D* hSubLeadPtLo = new TH1D( "hSubLeadPtLo","p_{T}^{C} > 0.2 GeV/c;p_{T}^{sublead} [GeV/c]", 100, 0 , 50 );
  // TH1D* LeadPt = new TH1D("LeadPt","Leading jet p_{T}", 120, 0, 240);
  TH1D* EmbLeadPt = new TH1D("EmbLeadPt","Embedded Leading jet p_{T}", 120, 0, 240);
  TH1D* GroomLeadPt = new TH1D("GroomLeadPt","Groomed Leading jet p_{T}", 120, 0, 240);
  TH1D* EmbGroomLeadPt = new TH1D("EmbGroomLeadPt","Groomed Embedded Leading jet p_{T}", 120, 0, 240);

  int nzgBins=40;
  TH1D* zgLead1020Hi = new TH1D("zgLead1020Hi","Leading Hi jet with 10<p_{T}<20", nzgBins, 0.05, 0.55);
  TH1D* zgLead2030Hi = new TH1D("zgLead2030Hi","Leading Hi jet with 20<p_{T}<30", nzgBins, 0.05, 0.55);
  TH1D* zgLead3040Hi = new TH1D("zgLead3040Hi","Leading Hi jet with 30<p_{T}<40", nzgBins, 0.05, 0.55);
  TH1D* zgLead4060Hi = new TH1D("zgLead4060Hi","Leading Hi jet with 40<p_{T}<60", nzgBins, 0.05, 0.55);

  TH1D* zgLead1020Lo = new TH1D("zgLead1020Lo","Leading Lo jet with 10<p_{T}<20", nzgBins, 0.05, 0.55);
  TH1D* zgLead2030Lo = new TH1D("zgLead2030Lo","Leading Lo jet with 20<p_{T}<30", nzgBins, 0.05, 0.55);
  TH1D* zgLead3040Lo = new TH1D("zgLead3040Lo","Leading Lo jet with 30<p_{T}<40", nzgBins, 0.05, 0.55);
  TH1D* zgLead4060Lo = new TH1D("zgLead4060Lo","Leading Lo jet with 40<p_{T}<60", nzgBins, 0.05, 0.55);

  TH2D* SubLeadPtLoVPtHi = new TH2D("SubLeadPtLoVPtHi", ";p_{T,Hi};p_{T,Lo}",100, 0, 60, 100, 0, 60 );
  TH1D* SubLeadPtLoMinusPtHi = new TH1D("SubLeadPtLoMinusPtHi", ";p_{T,Lo}-p_{T,Hi}",120, -30, 30 );

  TH1D* SubLeadPt = new TH1D("SubLeadPt","SubLeading jet p_{T}", 120, 0, 240);
  TH1D* EmbSubLeadPt = new TH1D("EmbSubLeadPt","Embedded SubLeading jet p_{T}", 120, 0, 240);
  TH1D* GroomSubLeadPt = new TH1D("GroomSubLeadPt","Groomed SubLeading jet p_{T}", 120, 0, 240);
  TH1D* EmbGroomSubLeadPt = new TH1D("EmbGroomSubLeadPt","Groomed Embedded SubLeading jet p_{T}", 120, 0, 240);

  TH1D* zgSubLead1020Hi = new TH1D("zgSubLead1020Hi","SubLeading Hi jet with 10<p_{T}<20", nzgBins, 0.05, 0.55);
  TH1D* zgSubLead2030Hi = new TH1D("zgSubLead2030Hi","SubLeading Hi jet with 20<p_{T}<30", nzgBins, 0.05, 0.55);
  TH1D* zgSubLead3040Hi = new TH1D("zgSubLead3040Hi","SubLeading Hi jet with 30<p_{T}<40", nzgBins, 0.05, 0.55);
  TH1D* zgSubLead4060Hi = new TH1D("zgSubLead4060Hi","SubLeading Hi jet with 40<p_{T}<60", nzgBins, 0.05, 0.55);

  TH1D* zgSubLead1020Lo = new TH1D("zgSubLead1020Lo","SubLeading Lo jet with 10<p_{T}<20", nzgBins, 0.05, 0.55);
  TH1D* zgSubLead2030Lo = new TH1D("zgSubLead2030Lo","SubLeading Lo jet with 20<p_{T}<30", nzgBins, 0.05, 0.55);
  TH1D* zgSubLead3040Lo = new TH1D("zgSubLead3040Lo","SubLeading Lo jet with 30<p_{T}<40", nzgBins, 0.05, 0.55);
  TH1D* zgSubLead4060Lo = new TH1D("zgSubLead4060Lo","SubLeading Lo jet with 40<p_{T}<60", nzgBins, 0.05, 0.55);

  TH1D* hdphiHi = new TH1D( "hdphiHi","#Delta#phi for hard constituent jets", 200, -2, 2 );
  TH1D* hdphiLo = new TH1D( "hdphiLo","#Delta#phi for soft constituent jets", 200, -2, 2 );

  TH2D* AJ_hi = new TH2D( "AJ_hi","A_{J} for hard constituent jets;A_{J};Refmult;fraction", 50, -0.6, 0.9, 800, -0.5, 799.5 );
  TH2D* AJ_lo = new TH2D( "AJ_lo","A_{J} for soft constituent jets;A_{J};Refmult;fraction", 50, -0.6, 0.9, 800, -0.5, 799.5 );

  // Fill Histos
  // -----------
 
  Long64_t NEvents = ResultTree->GetEntries();
  // NEvents = 10000;

  Long64_t nAccepted=0;  
  for ( int evi=0; evi<NEvents; ++evi){
    ResultTree->GetEntry( evi );
    if ( !(evi%10000) ) cout << "Working on " << evi << " / " << NEvents << endl;

    if ( refmult<MinRefmult ) continue;
    
    // Could also cut on ajLo :-/
    float ajHi = CalcAj ( pJ1, pJ2 );
    if ( fabs(FabsAjCut)>1e-4 ){
      if ( dir > 0 && ajHi < FabsAjCut )	continue;
      if ( dir < 0 && ajHi > FabsAjCut )	continue;
    }
    
    nAccepted++;
    float ptHi = pJ1->Pt();

    if ( ptHi >= 10 && ptHi < 20 ){
      zgLead1020Hi->Fill ( zg1 );
      zgLead1020Lo->Fill ( zgm1 );
    }
    if ( ptHi >= 20 && ptHi < 30 ){
      zgLead2030Hi->Fill ( zg1 );
      zgLead2030Lo->Fill ( zgm1 );
    }
    if ( ptHi >= 30 && ptHi < 40 ){
      zgLead3040Hi->Fill ( zg1 );
      zgLead3040Lo->Fill ( zgm1 );
    }
    if ( ptHi >= 40 && ptHi < 60 ){
      zgLead4060Hi->Fill ( zg1 );
      zgLead4060Lo->Fill ( zgm1 );
    }


    float SubLeadingptHi = pJ2->Pt();

    if ( SubLeadingptHi >= 10 && SubLeadingptHi < 20 ){
      zgSubLead1020Hi->Fill ( zg2 );
      zgSubLead1020Lo->Fill ( zgm2 );
    }
    if ( SubLeadingptHi >= 20 && SubLeadingptHi < 30 ){
      zgSubLead2030Hi->Fill ( zg2 );
      zgSubLead2030Lo->Fill ( zgm2 );
    }
    if ( SubLeadingptHi >= 30 && SubLeadingptHi < 40 ){
      zgSubLead3040Hi->Fill ( zg2 );
      zgSubLead3040Lo->Fill ( zgm2 );
    }
    if ( SubLeadingptHi >= 40 && SubLeadingptHi < 60 ){
      zgSubLead4060Hi->Fill ( zg2 );
      zgSubLead4060Lo->Fill ( zgm2 );
    }

    
  }
  new TCanvas; zgLead2030Hi->Draw();
  new TCanvas; zgSubLead1020Hi->Draw();

  new TCanvas; zgLead2030Lo->Draw();
  new TCanvas; zgSubLead1020Lo->Draw();

  fOut->Write();
  cout << " Accepted " << nAccepted << " dijet pairs." << endl; 
  cout << " Saved to " << endl << fOut->GetName() << endl; 
  return 0;

}
// ===========================================================================
double CalcAj ( TLorentzVector* j1, TLorentzVector* j2, bool nofabs ){
  if ( nofabs )  return ( j1->Pt()-j2->Pt() ) / ( j1->Pt()+j2->Pt() );
  return fabs( ( j1->Pt()-j2->Pt() ) / ( j1->Pt()+j2->Pt() ) );

  return -999;
}
