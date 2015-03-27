#include<vector>
#include<string>
#include<sstream>

Int_t QuickOverwiew(  TString SJA="antikt",  TString CMIN="0.2" )
{
  // TString Pattern="Results/Pythia_antikt_*_kt_*pscmin2.0*.root";
  TString Pattern="Results/Pythia_antikt_R0.4_pcminXCMINX_XSJAX_SRXSRX.root";
  TString SR="*";
  Pattern.ReplaceAll("XSJAX",SJA);
  Pattern.ReplaceAll("XCMINX",CMIN);
  Pattern.ReplaceAll("XSRX",SR);

  TString plotbase = "Plots/Sja_"+SJA + "_CMIN_"+CMIN;


  Color_t col  = kBlack;
  Color_t gcol = kGreen+1;
  Color_t qcol = kRed;

  Style_t mark  = kFullDiamond;
  Style_t gmark = kFullTriangleUp;
  Style_t qmark = kFullTriangleDown;

  gStyle->SetHistLineWidth(2);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1);
  float PerLine=0.05; // Line height in NDC for legend
  

  // Load files from a pattern
  // --------------------------
  cout << "Loading files." << endl;
  vector<TFile*> files;
  stringstream ss;
  string line;
  ss << gSystem->GetFromPipe(TString("/bin/ls ") + Pattern );
  while(getline(ss,line)){
    cout << line.data() << endl;
    TFile * file = new TFile(line.data(), "READ");
    files.push_back( file);
  }
  cout << "Loaded " << files.size() << " files for reading" << endl;

  // Project, then sort histos into standard, quark-, and gluon-led
  vector<TH1D*> histos;
  vector<TH1D*> qhistos;
  vector<TH1D*> ghistos;

  // Info
  vector<TTree*> infos;
  
 
  for ( int f = 0 ; f<files.size() ; ++f ){
    TIter nextkey(files.at(f)->GetListOfKeys());
    TKey *key;
    TH1D* h;
    while (key = (TKey*)nextkey() ) {
      TObject *object = (TObject*)key->ReadObj();
      TString classname=object->ClassName();
      TString name=object->GetName();
      name +="_"; name+=f;

      // intercept info tree
      if ( classname.Contains("TTree") ) {
	TTree* t = (TTree*)object;
	t->SetName(name);
	infos.push_back( t );
      }

      // On to the histos
      if ( !classname.Contains("TH2") ) continue;     
      h=((TH2D*)object)->ProjectionY(name);

      // Nortmalize to unity
      h->Scale(1./h->Integral());
      
      if ( name.BeginsWith("q") ){
	h->SetLineColor  ( qcol );
	h->SetMarkerColor( qcol );
	h->SetMarkerStyle( qmark );
	qhistos.push_back( h );
      } else if ( name.BeginsWith("g") ){
	h->SetLineColor  ( gcol );
	h->SetMarkerColor( gcol );
	h->SetMarkerStyle( gmark );
	ghistos.push_back( h );
      } else {
	h->SetLineColor  ( col );
	h->SetMarkerColor( col );
	h->SetMarkerStyle( mark );
	histos.push_back( h );
      }
      
    }
  }


  // Now draw histos
  // ---------------  
  TString title;
  TLegend* leg;
  TCanvas* canvas;
  for ( int i=0; i< histos.size(); ++ i){
    h=histos.at(i);
    TString hName = h->GetName();
    TString cName = "c";   cName+= hName(0, hName.Index("_") );
    int f=TString( hName( hName.Index("_") +1, hName.Length() ) ).Atoi();
    h->SetLineColor(f+1);
    h->SetMarkerColor(f+1);

    if ( hName.Contains("_0") ){
      canvas= new TCanvas (cName, cName);

      // some customization
      // Order is important
      if ( hName.Contains("Area") ) h->SetAxisRange(0,0.2,"x");
      if ( hName.Contains("OtherAreaFrac") ) h->SetAxisRange(0,0.35,"x");
      if ( hName.Contains("SubAreaFrac") ) h->SetAxisRange(0,0.4,"x");
      if ( hName.Contains("JetArea") ) h->SetAxisRange(0.35,0.65,"x");

      if ( hName.Contains("SubDeltaR") ) h->SetAxisRange(0.,0.3,"x");
      if ( hName.Contains("SubDeltaR") ) h->SetAxisRange(0.,0.2,"y");

      if ( hName.Contains("OtherDeltaR") ) h->SetAxisRange(0.,0.6,"x");
      if ( hName.Contains("OtherDeltaR") ) h->SetAxisRange(0.,0.16,"y");
      
      // if ( hName.Contains("SubPtFrac") ) h->SetAxisRange(0,1.0,"x");
      // if ( hName.Contains("SubPtFrac") ) h->SetAxisRange(0,0.1,"y");
      if ( hName.Contains("SubPtFrac") ) h->SetAxisRange(0,0.12,"y");

      if ( hName.Contains("OtherPtFrac") ) h->SetAxisRange(0,0.51,"x");
      if ( hName.Contains("OtherPtFrac") ) h->SetAxisRange(0,0.4,"y");

      if ( hName.Contains("Nsub") ) h->SetAxisRange(0,70,"x");
      if ( hName.Contains("Nsub") ) h->SetAxisRange(0,0.5,"y");
      

      // Construct title
      title  = "Subjets with ";  title += SJA;
      title += ", p_{T}^{C}>" ;  title += CMIN;
      title += ", " ;  title += cName;
      h->SetTitle ( title );
      
      // Construct legend
      leg = new TLegend ( 0.7, 0.75 - 0.5*PerLine*files.size(), 0.88, 0.75 + 0.5*PerLine*files.size(), "", "brNDC");
      leg->SetName("leg");
      
      // infos.at(0)->GetLeaf( "R"
      
      // Draw
      h->Draw();

      // Jet area: Just fit a gaussian and skip the legend
      if ( hName.Contains("JetArea") ) {
	h->Fit("gaus","","same");
	continue;
      }

      // construct legend entry
      infos.at(f)->GetEntry(0);
      title = TString::Format(" SubR = %0.2f", infos.at(f)->GetLeaf("SubR")->GetValue());      
      leg->AddEntry(h->GetName(), title, "lp");
      leg->Draw();

    } else {
      if ( hName.Contains("JetArea") )	continue;
      
      canvas= (TCanvas*)gROOT->GetListOfCanvases()->FindObject(cName);
      canvas->cd();
      h->Draw("same");

      // construct legend entry
      infos.at(f)->GetEntry(0);
      title = TString::Format(" SubR = %0.2f", infos.at(f)->GetLeaf("SubR")->GetValue());      
      leg = (TLegend*) canvas->FindObject("leg");
      leg->AddEntry(h->GetName(), title, "lp");
      leg->Draw();
    }
    // ghistos.at(i)->Draw("same");
    // qhistos.at(i)->Draw("same");

  }

  // and save all canvases
  TSeqCollection* canvases = gROOT->GetListOfCanvases();
  for (int i=0; i<canvases->GetEntries() ; ++i ){
    canvas = (TCanvas*) canvases->At(i);
        
    canvas->SaveAs(plotbase + "." + canvas->GetName()+".png");

  }
  

  return 0;
}



