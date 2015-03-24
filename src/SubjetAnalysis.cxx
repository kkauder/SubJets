/** @file SubjetAnalysis.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Class for Subjet analysis
    @details Uses JetAnalyzer objects to perform Subjet analysis.
    @date Mar 05, 2015
*/

#include "SubjetAnalysis.hh"

// Standard ctor
SubjetAnalysis::SubjetAnalysis ( double R, double SubR,
				 JetAlgorithm LargeJetAlgorithm,
				 JetAlgorithm SubJetAlgorithm,
				 double PtJetMin, double PtJetMax,
				 double EtaConsCut, double PtConsMin, double PtConsMax,
				 TH2D*  NsubPt, TH2D*  SubPtFrac, TH2D*  OtherPtFrac, TH2D*  SubDeltaR, TH2D*  OtherDeltaR, TH2D*  JetArea, TH2D*  SubArea, TH2D*  OtherArea, TH2D*  SubAreaFrac, TH2D*  OtherAreaFrac,
				 TH2D* gNsubPt, TH2D* gSubPtFrac, TH2D* gOtherPtFrac, TH2D* gSubDeltaR, TH2D* gOtherDeltaR, TH2D* gJetArea, TH2D* gSubArea, TH2D* gOtherArea, TH2D* gSubAreaFrac, TH2D* gOtherAreaFrac,
				 TH2D* qNsubPt, TH2D* qSubPtFrac, TH2D* qOtherPtFrac, TH2D* qSubDeltaR, TH2D* qOtherDeltaR, TH2D* qJetArea, TH2D* qSubArea, TH2D* qOtherArea, TH2D* qSubAreaFrac, TH2D* qOtherAreaFrac
				 )
  : R(R), SubR(SubR),
    LargeJetAlgorithm(LargeJetAlgorithm), SubJetAlgorithm(SubJetAlgorithm),
    PtJetMin(PtJetMin), PtJetMax(PtJetMax),
    EtaConsCut (EtaConsCut), PtConsMin (PtConsMin), PtConsMax (PtConsMax),
    NsubPt ( NsubPt),   SubPtFrac ( SubPtFrac),  OtherPtFrac ( OtherPtFrac),
    SubDeltaR ( SubDeltaR ), OtherDeltaR ( OtherDeltaR ),
    JetArea ( JetArea ), SubArea ( SubArea ), OtherArea ( OtherArea ),
    SubAreaFrac ( SubAreaFrac ), OtherAreaFrac ( OtherAreaFrac ),
    // gluons
    gNsubPt ( gNsubPt),   gSubPtFrac ( gSubPtFrac),  gOtherPtFrac ( gOtherPtFrac),
    gSubDeltaR ( gSubDeltaR ), gOtherDeltaR ( gOtherDeltaR ),
    gJetArea ( gJetArea ), gSubArea ( gSubArea ), gOtherArea ( gOtherArea ),
    gSubAreaFrac ( gSubAreaFrac ), gOtherAreaFrac ( gOtherAreaFrac ),
    // quarks
    qNsubPt ( qNsubPt),   qSubPtFrac ( qSubPtFrac),  qOtherPtFrac ( qOtherPtFrac),
    qSubDeltaR ( qSubDeltaR ), qOtherDeltaR ( qOtherDeltaR ),
    qJetArea ( qJetArea ), qSubArea ( qSubArea ), qOtherArea ( qOtherArea ),
    qSubAreaFrac ( qSubAreaFrac ), qOtherAreaFrac ( qOtherAreaFrac )
  
{
  // derived rapidity cuts
  // ---------------------
  EtaJetCut     = EtaConsCut - R;
  EtaGhostCut   = EtaJetCut  + 2.0*R;

  // Constituent selectors
  // ---------------------
  select_cons_eta    = fastjet::SelectorAbsRapMax( EtaConsCut );
  select_cons_pt     = fastjet::SelectorPtRange( PtConsMin, PtConsMax );
  
  select_cons_charge = fastjet::SelectorIdentity();
  // select_cons_charge = SelectorChargeRange( 0, 0);
  select_cons         = select_cons_eta * select_cons_pt * select_cons_charge;
  cout << select_cons.description() << endl;
  // throw(0);

  // Jet candidate selectors
  // -----------------------
  select_jet_eta     = fastjet::SelectorAbsRapMax(EtaJetCut);
  select_jet_pt      = fastjet::SelectorPtRange( PtJetMin, PtJetMax );
  select_jet         = select_jet_eta * select_jet_pt;

  // Choose a jet and area definition
  // --------------------------------
  JetDef    = fastjet::JetDefinition( LargeJetAlgorithm, R    );
  SubJetDef = fastjet::JetDefinition( SubJetAlgorithm  , SubR );

  // create an area definition for the clustering
  //----------------------------------------------------------
  // ghosts should go up to the acceptance of the detector or
  // (with infinite acceptance) at least 2R beyond the region
  // where you plan to investigate jets.
  AreaSpec = GhostedAreaSpec ( EtaGhostCut );
  AreaDef = AreaDefinition (fastjet::active_area_explicit_ghosts, AreaSpec);

  cout << " ################################################### " << endl;
  cout << "Clustered with " << JetDef.description() << endl;
  cout << "AreaSpec: " << AreaSpec.description() << endl;
  cout << "AreaDef:  " << AreaDef.description() << endl;
  cout << " ################################################### " << endl;

}

// Main analysis method
// ====================
int SubjetAnalysis::AnalyzeAndFill ( vector<PseudoJet>& particles, vector<PseudoJet>* pPartons ){

  // Select particles to perform analysis on
  // ---------------------------------------
  vector<PseudoJet> cons = select_cons( particles );
  
  // run the jet clustering and get the acceptable jets
  // ---------------------------------------------------
  // JetAnalyzer JA ( cons, JetDef ); // NO background subtraction
  // JAResult = sorted_by_pt( select_jet ( JA.inclusive_jets() ) );

  JetAnalyzer JA( cons, JetDef, AreaDef ); // WITH background subtraction
  Subtractor* BackgroundSubtractor =  JA.GetBackgroundSubtractor();
  JAResult = sorted_by_pt( select_jet ( (*BackgroundSubtractor)( JA.inclusive_jets() ) ) );
    
  if ( JAResult.size()==0 ) {
    return -1;
  }

  // extract the subjets
  //--------------------
  // // a "header" for the output
  // cout << "Ran " << JetDef.description() << endl;
  // cout << "Showing the jets according to " << select_jet.description() << endl;
  // cout << "And their subjets for Rsub = " << Rsub << endl;
  // printf("%10s %15s %15s %15s %15s\n","jet #", "rapidity", "phi", "pt", "n constituents");

  for (unsigned int i = 0; i < JAResult.size(); i++) {
    // jet matched to an original hard scattering?
    // -------------------------------------------
    int IsQuark=-1; // -1 if jet is unmatched
    if ( pPartons ){
      vector<PseudoJet>& partons = *pPartons;
      for ( int p = 0; p<partons.size() ; ++p ){
	if ( IsMatched ( JAResult[i], partons[p], R) ){
	  IsQuark = ( abs(partons[p].user_info<JetAnalysisUserInfo>().GetQuarkCharge())>0 );
	}
      }
    }
    // Only filled if jet is matched
    TH2D* fNsubPt=0; TH2D* fSubPtFrac=0; TH2D* fOtherPtFrac=0;
    TH2D* fSubDeltaR=0; TH2D* fOtherDeltaR=0;
    TH2D* fJetArea=0; TH2D* fSubArea=0; TH2D* fOtherArea=0; TH2D* fSubAreaFrac=0; TH2D* fOtherAreaFrac=0;

    if ( IsQuark==0 ){
      fNsubPt        = gNsubPt;
      fSubPtFrac     = gSubPtFrac;
      fOtherPtFrac   = gOtherPtFrac;
      fSubDeltaR     = gSubDeltaR;
      fOtherDeltaR   = gOtherDeltaR;
      fJetArea       = gJetArea;
      fSubArea       = gSubArea;
      fOtherArea     = gOtherArea;
      fSubAreaFrac   = gSubAreaFrac;
      fOtherAreaFrac = gOtherAreaFrac;      
    }
    if ( IsQuark==1 ){
      fNsubPt        = qNsubPt;
      fSubPtFrac     = qSubPtFrac;
      fOtherPtFrac   = qOtherPtFrac;
      fSubDeltaR     = qSubDeltaR;
      fOtherDeltaR   = qOtherDeltaR;
      fJetArea       = qJetArea;
      fSubArea       = qSubArea;
      fOtherArea     = qOtherArea;
      fSubAreaFrac   = qSubAreaFrac;
      fOtherAreaFrac = qOtherAreaFrac;      
    }

    
    // get the jet constituents and separate the ghosts from regular particles
    //------------------------------------------------------------------------
    vector<PseudoJet> particles, ghosts;
    SelectorIsPureGhost().sift(JAResult[i].constituents(), ghosts, particles);
    double ghost_area = ghosts.size() ? ghosts[0].area() : 0.01; //check!!!!!
    
    // Recluster the particles with this set of ghosts
    //------------------------------------------------
    ClusterSequenceActiveAreaExplicitGhosts  cs_sub(particles, SubJetDef, ghosts, ghost_area);
    vector<PseudoJet> subjets = sorted_by_pt(cs_sub.inclusive_jets());


    // And fill histograms
    // -------------------
    NsubPt  -> Fill( JAResult[i].perp(), subjets.size() );
    JetArea -> Fill( JAResult[i].perp(), JAResult[i].area() );
  
    for (unsigned int j=0; j<subjets.size(); j++){
      if (j==0) { // leading subjet
	SubPtFrac    -> Fill( JAResult[i].perp(), subjets[j].perp() / JAResult[i].perp() );
	SubDeltaR    -> Fill( JAResult[i].perp(), JAResult[i].delta_R(subjets[j]) );
	SubArea      -> Fill( JAResult[i].perp(), subjets[j].area() );
	SubAreaFrac  -> Fill( JAResult[i].perp(), subjets[j].area() / JAResult[i].area() );
      } else {  // nonleading subjets
	OtherPtFrac    -> Fill( JAResult[i].perp(), subjets[j].perp() / JAResult[i].perp() );
	OtherDeltaR    -> Fill( JAResult[i].perp(), JAResult[i].delta_R(subjets[j]) );
	OtherArea      -> Fill( JAResult[i].perp(), subjets[j].area() );
	OtherAreaFrac  -> Fill( JAResult[i].perp(), subjets[j].area() / JAResult[i].area() );
      } // leading?
    }

    // Same for specific leading partons
    // ---------------------------------

    if ( fNsubPt ){
      fNsubPt  -> Fill( JAResult[i].perp(), subjets.size() );
      fJetArea -> Fill( JAResult[i].perp(), JAResult[i].area() );
  
      for (unsigned int j=0; j<subjets.size(); j++){
	if (j==0) { // leading subjet
	  fSubPtFrac    -> Fill( JAResult[i].perp(), subjets[j].perp() / JAResult[i].perp() );
	  fSubDeltaR    -> Fill( JAResult[i].perp(), JAResult[i].delta_R(subjets[j]) );
	  fSubArea      -> Fill( JAResult[i].perp(), subjets[j].area() );
	  fSubAreaFrac  -> Fill( JAResult[i].perp(), subjets[j].area() / JAResult[i].area() );
	} else {  // nonleading subjets
	  fOtherPtFrac    -> Fill( JAResult[i].perp(), subjets[j].perp() / JAResult[i].perp() );
	  fOtherDeltaR    -> Fill( JAResult[i].perp(), JAResult[i].delta_R(subjets[j]) );
	  fOtherArea      -> Fill( JAResult[i].perp(), subjets[j].area() );
	  fOtherAreaFrac  -> Fill( JAResult[i].perp(), subjets[j].area() / JAResult[i].area() );
	} // leading?
      }
    }
  }
  
  // // cleanup
  // // -------
  // delete JA;


  return 0;

}


// Helper to deal with repetitive stuff
TStarJetPicoReader GetReader ( TString ChainPattern, TString TriggerString, TString ChainName ){
  TStarJetPicoDefinitions::SetDebugLevel(0); // 10 for more output

  TStarJetPicoReader reader;
  TChain* tree = new TChain( ChainName );
  tree->Add( ChainPattern );
  reader.SetInputChain (tree);

  // Event and track selection
  // -------------------------
  TStarJetPicoEventCuts* evCuts = reader.GetEventCuts();
  evCuts->SetTriggerSelection( TriggerString ); //All, MB, HT, pp, ppHT, ppJP
  // Additional cuts
  evCuts->SetVertexZCut (SubjetParameters::VzCut);
  evCuts->SetRefMultCut (SubjetParameters::RefMultCut);
  evCuts->SetVertexZDiffCut( SubjetParameters::VzDiffCut );

  // Tracks: Some standard high quality cuts
  TStarJetPicoTrackCuts* trackCuts = reader.GetTrackCuts();
  trackCuts->SetDCACut( SubjetParameters::DcaCut );
  trackCuts->SetMinNFitPointsCut( SubjetParameters::NFitMin );
  trackCuts->SetFitOverMaxPointsCut( SubjetParameters::NFitRatio );
  trackCuts->SetMaxPtCut ( SubjetParameters::PtConsMax );

  // Towers:
  TStarJetPicoTowerCuts* towerCuts = reader.GetTowerCuts();
  towerCuts->SetMaxEtCut( SubjetParameters::EtTowerMax );
  // cout << "Using these tower cuts:" << endl;
  // cout << "  GetMaxEtCut = " << towerCuts->GetMaxEtCut() << endl;
  // cout << "  Gety8PythiaCut = " << towerCuts->Gety8PythiaCut() << endl;

  // V0s: Turn off
  reader.SetProcessV0s(false);

  return reader;
}

// Slightly different, preferred version of GetReader
TStarJetPicoReader SetupReader ( TChain* chain, TString TriggerString ){
  TStarJetPicoDefinitions::SetDebugLevel(0); // 10 for more output

  TStarJetPicoReader reader;
  reader.SetInputChain (chain);

  // Event and track selection
  // -------------------------
  TStarJetPicoEventCuts* evCuts = reader.GetEventCuts();
  evCuts->SetTriggerSelection( TriggerString ); //All, MB, HT, pp, ppHT, ppJP
  // Additional cuts
  evCuts->SetVertexZCut (SubjetParameters::VzCut);
  evCuts->SetRefMultCut (SubjetParameters::RefMultCut);
  // evCuts->SetVertexZDiffCut( SubjetParameters::VzDiffCut );

  // Tracks: Some standard high quality cuts
  // TODO: Add track, tower quality cuts
  // TStarJetPicoTrackCuts* trackCuts = reader.GetTrackCuts();
  // cout << " dca : " << trackCuts->GetDCACut(  ) << endl;
  // cout << " nfit : " <<   trackCuts->GetMinNFitPointsCut( ) << endl;
  // cout << " nfitratio : " <<   trackCuts->GetFitOverMaxPointsCut( ) << endl;
  // cout << " maxpt : " << trackCuts->GetMaxPtCut (  ) << endl;
  // throw (string("done"));

  // trackCuts->SetDCACut( 1.0 ); // maybe too high for low pT
  // trackCuts->SetMinNFitPointsCut( 20 );
  // trackCuts->SetFitOverMaxPointsCut( 0.52 );
  // trackCuts->SetMaxPtCut ( 30 ); // should it be 10? 15?

  // Towers: Don't know. Let's print out the default
  TStarJetPicoTowerCuts* towerCuts = reader.GetTowerCuts();
  cout << "Using these tower cuts:" << endl;
  cout << "  GetMaxEtCut = " << towerCuts->GetMaxEtCut() << endl;
  cout << "  Gety8PythiaCut = " << towerCuts->Gety8PythiaCut() << endl;

  // V0s: Turn off
  reader.SetProcessV0s(false);

  return reader;
}
