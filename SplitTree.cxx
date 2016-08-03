// root -l -b -q SplitTree.cxx'("AjResults/Tow0_Eff0_Fresh_NicksList_HC100_ppAj.root","AjResults/Tow0_Eff0_ppAj",12)'
// root -l -b -q SplitTree.cxx'("AjResults/Tow1_Eff0_Fresh_NicksList_HC100_ppAj.root","AjResults/Tow1_Eff0_ppAj",12)'
// root -l -b -q SplitTree.cxx'("AjResults/Tow-1_Eff0_Fresh_NicksList_HC100_ppAj.root","AjResults/Tow-1_Eff0_ppAj",12)'
// root -l -b -q SplitTree.cxx'("AjResults/Tow0_Eff1_Fresh_NicksList_HC100_ppAj.root","AjResults/Tow0_Eff1_ppAj",12)'
// root -l -b -q SplitTree.cxx'("AjResults/Tow0_Eff-1_Fresh_NicksList_HC100_ppAj.root","AjResults/Tow0_Eff-1_ppAj",12)'

int SplitTree( TString in = "AjResults/Tow0_Eff0_Fresh_NicksList_HC100_ppAj.root", TString outbase = "AjResults/Tow0_Eff0_ppAj", int split=12){
  TChain* inchain = new TChain ( "TriggeredTree" );
  inchain->Add ( in );

  Long64_t Norig = inchain->GetEntries();
  Long64_t Nsplit = Norig / split;

  int oldfile = 0;
  TFile * out=0;
  TTree * newtree = inchain->CloneTree ( 0 );
  
  for ( Long64_t i=0; i<Norig; ++i ){
    int currfile = i / Nsplit + 1;
    if ( currfile != oldfile && currfile <= split ){
      // Last one soaks up the remainder
     
      TString name = outbase + "_";
      name += currfile;
      name += "_of_";
      name += split;
      name += ".root";
      cout << "Now saving to " << name << endl;

      oldfile = currfile;
      if ( out ){
	out->Write();
	out->Close();
      }
      out = new TFile( name, "recreate");
      newtree = inchain->CloneTree ( 0 );
    }
    inchain->GetEntry( i );
    newtree->Fill();
    
  }

  if ( out ){
    out->Write();
    out->Close();
  }

}
