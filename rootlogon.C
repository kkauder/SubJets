{
  // gSystem->Load("./libMuMCEvent.so");

  // Style options
  gROOT->SetStyle("Plain");// Default white background for all plots
  gStyle->SetPalette(1);

  // The modes below are provided by Nick van Eijndhoven <Nick@phys.uu.nl>
  // from Alice.
  gStyle->SetCanvasColor(10);
  gStyle->SetStatColor(10);
  gStyle->SetTitleFillColor(10);
  gStyle->SetPadColor(10);

  // Settings for statistics information
  gStyle->SetOptFit(0);
  // gStyle->SetOptStat(0);

  // SetPaperSize wants width & height in cm: A4 is 20,26 & US is 20,24
  gStyle->SetPaperSize(20,24);

  // Positioning of axes labels
  gStyle->SetTitleOffset(1.2);
  // grid
  gStyle->SetPadGridX(0);
  gStyle->SetPadGridY(0);

  //  Set date/time for plot
  gStyle->SetOptDate(0);

  // gSystem->SetIncludePath(" -I.");
  // gSystem->AddIncludePath(" -I./.$STAR_HOST_SYS/include -I./StRoot -I$STAR/.$STAR_HOST_SYS/include -I$STAR/StRoot -I/usr/include/mysql");
  // if (gSystem->Getenv("QTDIR"))
  //    gSystem->AddIncludePath(" -I$QTDIR/include -I$QTDIR/include/Qt -I$QTDIR/include/QtCore -I$QTDIR/include/QtGui");

  gSystem->AddIncludePath(" -I${STARPICOPATH}");

  
  // To read and deal with TStarJetPicoDst's
  Int_t ierr = 0;
  char *clibs[] = {
    "$ROOTSYS/lib/libPhysics.so",
    //    "$ROOTSYS/lib/libRIO.so",
    "$ROOTSYS/lib/libHist.so",
    "$ROOTSYS/lib/libEG.so",
    "$ROOTSYS/lib/libTree.so",    
    "$STARPICOPATH/libTStarJetPico.so",
    "~/software/RooUnfold/libRooUnfold.so",
    0
  };

  Int_t i = 0;
  while ( clibs[i++] ) {
    ierr = gSystem->Load(clibs[i-1]);
    if ( ierr != 0) {
      cerr <<  "Unable to load " << clibs[i-1] << endl;
    }
  }

 
}
