#include<vector>
#include<string>
#include<sstream>

Int_t DevQuickOverwiew(  TString SJA="antikt",  TString CMIN="0.2", TString JSEL="2030" )
{
  TString Pattern="Results/devPythiaXJSELX_antikt_R0.4_pcminXCMINX_XSJAX_SRXSRX.root";
  TString SR="*";
  Pattern.ReplaceAll("XSJAX",SJA);
  Pattern.ReplaceAll("XCMINX",CMIN);
  Pattern.ReplaceAll("XSRX",SR);
  Pattern.ReplaceAll("XJSELX",JSEL);

  TString plotbase = "Plots/Jsel_"+JSEL+"Sja_"+SJA + "_CMIN_"+CMIN;

  Color_t col  = kBlack;
  Style_t mark  = kFullDiamond;

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

  // Info
  vector<TTree*> infos;  
  TString lname; 
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
      if ( classname.Contains("TH1") ){      
	h=(TH1D*)object->Clone(name);
	h->Scale(1./h->Integral());     	// Normalize to unity
	h->SetLineColor  ( col );
	h->SetMarkerColor( col );
	h->SetMarkerStyle( mark );
	histos.push_back( h );
      }
	
      if ( classname.Contains("TH2") ){
	lname = name; lname.ReplaceAll("_","x_");
	h=((TH2D*)object)->ProjectionX(lname);
	h->Scale(1./h->Integral());     	// Normalize to unity
	h->SetLineColor  ( col );
	h->SetMarkerColor( col );
	h->SetMarkerStyle( mark );
	histos.push_back( h );

	lname = name; lname.ReplaceAll("_","y_");
	h=((TH2D*)object)->ProjectionY(lname);
	h->Scale(1./h->Integral());     	// Normalize to unity
	h->SetLineColor  ( col );
	h->SetMarkerColor( col );
	h->SetMarkerStyle( mark );
	histos.push_back( h );
      }
      
      h->GetYaxis()->SetTitle("fraction");
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
    
    if ( hName.Contains("AreaFrac") ) continue;
    if ( hName.Contains("_0") ){
      // some customization
      // Order is important
      if ( hName.Contains("JetPt") ) h->SetAxisRange(10,50,"x");
	    
      if ( hName.Contains("Area") ) h->SetAxisRange(0,0.2,"x");
      if ( hName.Contains("JetArea") ) h->SetAxisRange(0.35,0.65,"x");

      if ( hName.Contains("AreaFrac") ) h->SetAxisRange(0,0.35,"x");


      
      if ( hName.Contains("SubDeltaR") ) h->SetAxisRange(0.,0.3,"x");
      if ( hName.Contains("SubDeltaR") ) h->SetAxisRange(0.,0.2,"y");

      if ( hName.Contains("DeltaR") ) h->SetAxisRange(0.,0.5,"x");
      if ( hName.Contains("DeltaR") ) h->SetAxisRange(0.,0.22,"y");
      
      // if ( hName.Contains("SubPtFrac") ) h->SetAxisRange(0,1.0,"x");
      // if ( hName.Contains("SubPtFrac") ) h->SetAxisRange(0,0.1,"y");
      if ( hName.Contains("SubPtFrac") ) h->SetAxisRange(0,0.12,"y");

      if ( hName.Contains("OtherPtFrac") ) h->SetAxisRange(0,0.51,"x");
      if ( hName.Contains("OtherPtFrac") ) h->SetAxisRange(0,0.4,"y");

      if ( hName.Contains("Nsub") ) h->SetAxisRange(0,70,"x");
      if ( hName.Contains("Nsub") ) h->SetAxisRange(0,0.5,"y");
      
      canvas= new TCanvas (cName, cName);
      
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

      // Jet pT: Fit an exponential, skip legend
      if ( hName.Contains("JetPt") ) {
	h->GetXaxis()->SetTitle("p_{T}^{Jet} [GeV/c]");
	gPad->SetLogy();
	h->SetMaximum(1.5*h->GetMaximum());
	h->Fit("expo","","0");
	h->GetFunction("expo")->SetLineColor(kGray+1);
	h->GetFunction("expo")->SetLineWidth(1);
	h->GetFunction("expo")->SetLineStyle(2);
	h->GetFunction("expo")->Draw("same");
	continue;
      }

      // construct legend entry
      infos.at(f)->GetEntry(0);
      title = TString::Format(" SubR = %0.2f", infos.at(f)->GetLeaf("SubR")->GetValue());      
      leg->AddEntry(h->GetName(), title, "lp");
      leg->Draw();

    } else {
      if ( hName.Contains("JetArea") || hName.Contains("JetPt") )	continue;
      
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



