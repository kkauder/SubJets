{
  
  // //  ======================================  Pythia spectrum
  //  TFile * in=new TFile("Results/Groom_Rhic.Zg.root","READ");
  // TH1D* h= (TH1D*) in->Get("LeadPt");
  // h->SetAxisRange(0, 60,"x");

  // TCanvas* c = new TCanvas( "c","",400, 500 );
  // gPad->SetLogy();
  // h->Draw();

  // gPad->SaveAs("tmp/PythiaSpec.png");

  // //  ====================================== rho
  // // TFile * in=new TFile("Results/Groom_AuAu.root","READ");
  // // TFile * in=new TFile("Results/Groom_Rhic.root","READ");
  // TFile * in=new TFile("../PaperAj/BasicAj/AjResults/Fresh_NicksList_HC100_AuAu.root","READ");
  // TCanvas* c = new TCanvas( "c","",500, 500 );

  // TH1D* h= new TH1D ( "h","", 60, 0, 120 );
  // ResultTree->Draw("rho>>h","refmult>=269","");
  // h->SetTitle(";#rho [GeV/c]");
  
  // h->Draw();
  // gPad->SaveAs("tmp/Rho.png");

  // //  ====================================== rhoerr
  // TFile * in=new TFile("../PaperAj/BasicAj/AjResults/Fresh_NicksList_HC100_AuAu.root","READ");
  // TCanvas* c = new TCanvas( "c","",500, 500 );

  // TH1D* h= new TH1D ( "h","", 40, 0, 20);
  // ResultTree->Draw("rhoerr>>h","refmult>=269","");
  // h->SetTitle(";#sigma ( #rho ) [GeV/c]");
  
  // h->Draw();
  // gPad->SaveAs("tmp/RhoErr.png");

  // //  ====================================== delta pt
  // TFile * in=new TFile("Results/Groom_Rhic.root","READ");
  // TCanvas* c = new TCanvas( "c","",500, 500 );
  // gPad->SetLogy();

  // TH1D* h= new TH1D ( "h","", 60, -30, 30 );
  // ResultTree->Draw("-Jets[0].Pt() + EmbJets[0].Pt()>>h");
  // h->SetTitle(";#Delta p_{T} [GeV/c]");
  
  // h->Draw();
  // gPad->SaveAs("tmp/DeltaPt.png");

  // //  ====================================== delta pt
  // TFile * in=new TFile("Results/GroomUnfoldingTest.root","READ");
  // TCanvas* c = new TCanvas( "c","",550, 500 );
  // gPad->SetLogz();
  

  // TH2D* h  = (TH2D*) LeadPtResponse->Hresponse();
  // h->SetTitle("; p_{T}^{meas} [GeV/c];p_{T}^{meas} [GeV/c]");
  // h->SetAxisRange(10, 40, "x");
  // h->SetAxisRange( 0, 40, "y");

  // gStyle->SetOptStat(0);
  // h->Draw("colz");
  // gPad->SaveAs("tmp/response.png");



}
