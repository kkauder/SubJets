/** @file DevSubjetAnalysis.cxx
    @author Kolja Kauder
    @version Revision 0.1
    @brief Class for Subjet analysis
    @details Uses JetAnalyzer objects to perform Subjet analysis.
    @date Mar 05, 2015
*/

#include "DevSubjetAnalysis.hh"

// Standard ctor
DevSubjetAnalysis::DevSubjetAnalysis ( double R, double SubR,
				       JetAlgorithm LargeJetAlgorithm,
				       JetAlgorithm SubJetAlgorithm,
				       double PtJetMin, double PtJetMax,
				       double EtaConsCut, double PtConsMin, double PtConsMax,
				       TH1D* JetPt, TH1D*  JetArea, TH1D*  Nsub, 
				       TH2D* SubVLeadPt,       TH2D* AllVLeadPt,
				       TH2D* SubVLeadPtFrac,   TH2D* AllVLeadPtFrac,
				       TH2D* SubVLeadArea,     TH2D* AllVLeadArea,
				       TH2D* SubVLeadAreaFrac, TH2D* AllVLeadAreaFrac,
				       TH2D* SubVLeadDeltaR,   TH2D* AllVLeadDeltaR
				       )
: R(R), SubR(SubR),
  LargeJetAlgorithm(LargeJetAlgorithm), SubJetAlgorithm(SubJetAlgorithm),
  PtJetMin(PtJetMin), PtJetMax(PtJetMax),
  EtaConsCut (EtaConsCut), PtConsMin (PtConsMin), PtConsMax (PtConsMax),
  JetPt (JetPt), JetArea (JetArea), Nsub(Nsub),
  SubVLeadPt ( SubVLeadPt)             ,  AllVLeadPt ( AllVLeadPt),
  SubVLeadPtFrac ( SubVLeadPtFrac)     ,  AllVLeadPtFrac ( AllVLeadPtFrac),
  SubVLeadArea ( SubVLeadArea)         ,  AllVLeadArea ( AllVLeadArea),
  SubVLeadAreaFrac ( SubVLeadAreaFrac) ,  AllVLeadAreaFrac ( AllVLeadAreaFrac),
  SubVLeadDeltaR ( SubVLeadDeltaR)     ,  AllVLeadDeltaR ( AllVLeadDeltaR)
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
int DevSubjetAnalysis::AnalyzeAndFill ( vector<PseudoJet>& particles, vector<PseudoJet>* pPartons ){

  // Reset members
  // -------------
  LeadingSubjets.clear();
  SubLeadingSubjets.clear();
  
  // Select particles to perform analysis on
  // ---------------------------------------
  vector<PseudoJet> cons = select_cons( particles );
  
  // run the jet clustering and get the acceptable jets
  // ---------------------------------------------------
  // JetAnalyzer JA ( cons, JetDef ); // NO background subtraction
  // JAResult = sorted_by_pt( select_jet ( JA.inclusive_jets() ) );

  fastjet::Selector selector_bkgd = fastjet::SelectorAbsRapMax( EtaJetCut ) * (!fastjet::SelectorNHardest(2));
  JetAnalyzer JA( cons, JetDef, AreaDef, selector_bkgd ); // WITH background subtraction
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
    
    // get the jet constituents and separate the ghosts from regular particles
    //------------------------------------------------------------------------
    vector<PseudoJet> particles, ghosts;
    SelectorIsPureGhost().sift(JAResult[i].constituents(), ghosts, particles);
    // double ghost_area = ghosts.size() ? ghosts[0].area() : 0.01; //check!!!!!
    double ghost_area = ghosts.size() ? ghosts[0].area() : 0.001; //check!!!!!
      
    // Recluster the particles with this set of ghosts
    //------------------------------------------------
    ClusterSequenceActiveAreaExplicitGhosts  cs_sub(particles, SubJetDef, ghosts, ghost_area);
    // vector<PseudoJet> subjets = sorted_by_pt(cs_sub.inclusive_jets()); // ALL subjets, including ghosts
    Selector SelectorNoGhosts = !SelectorIsPureGhost();
    // vector<PseudoJet> subjets = sorted_by_pt( SelectorNoGhosts ( cs_sub.inclusive_jets()) );
    
    // Need a new background estimator ?
    // Then work on these lines.
    // But the old one should be right.
    // --------------------------------- 
    // fastjet::JetDefinition* SubJetDefBkgd = new fastjet::JetDefinition (fastjet::kt_algorithm, SubJetDef.R());
    // fastjet::JetMedianBackgroundEstimator* SubJetBkgdEstimator = new fastjet::JetMedianBackgroundEstimator(selector_bkgd, *SubJetDefBkgd, *area_def_bkgd);
    // bkgd_subtractor = new fastjet::Subtractor(bkgd_estimator);
    // Subtractor* SubBackgroundSubtractor =  JA.GetBackgroundSubtractor();
    vector<PseudoJet> subjets = sorted_by_pt( (*BackgroundSubtractor)( SelectorNoGhosts ( cs_sub.inclusive_jets()) ));

    // Copy to hand to caller
    if ( i==0 )     LeadingSubjets=subjets;
    if ( i==1 )     SubLeadingSubjets=subjets;
    
    // And fill histograms
    // -------------------
    float vJetPt   = JAResult[i].pt();
    float vJetArea = JAResult[i].area();
    if ( JetPt )     JetPt -> Fill( vJetPt   );
    if ( JetArea ) JetArea -> Fill( vJetArea );
    if ( Nsub )    Nsub    -> Fill( subjets.size() );

    float vLeadPt     = -1;
    float vLeadArea   = -1;
    float vLeadDeltaR = -1;
    float vSubPt      = -1;
    float vSubArea    = -1;
    float vSubDeltaR  = -1;
    float vAllPt      = -1;
    float vAllArea    = -1;
    float vAllDeltaR  = -1;

    
    if ( subjets.size() <= 0 ) throw string("Found a jet with no subjets. Abort." ) ;
    // Leading definitely exists
    vLeadPt     = subjets.at(0).pt();
    vLeadArea   = subjets.at(0).area();
    vLeadDeltaR = subjets.at(0).delta_R( JAResult[i] );

    // More than 1 subjet?
    if ( subjets.size() >=2 ){      
      vSubPt     = subjets.at(1).pt();
      vSubArea   = subjets.at(1).area();
      vSubDeltaR = subjets.at(1).delta_R( JAResult[i] );      
    }
    
    // Fill both regardless! Handle existence of more than 1 subjet  via underflow management
    if ( SubVLeadPt ){
      SubVLeadPt       ->Fill ( vLeadPt, vSubPt );
      SubVLeadPtFrac   ->Fill ( vLeadPt/vJetPt, vSubPt/vJetPt );
      SubVLeadArea     ->Fill ( vLeadArea, vSubArea );
      SubVLeadAreaFrac ->Fill ( vLeadArea/vJetArea, vSubArea/vJetArea );
      SubVLeadDeltaR   ->Fill ( vLeadDeltaR, vSubDeltaR );
    }
    
    for (unsigned int j=0; j<subjets.size(); j++) {      
      vAllPt     = subjets.at(j).pt();
      vAllArea   = subjets.at(j).area();
      vAllDeltaR = subjets.at(j).delta_R( JAResult[i] );      
      
      if (  AllVLeadPt ){
	AllVLeadPt       ->Fill ( vLeadPt, vAllPt );
	AllVLeadPtFrac   ->Fill ( vLeadPt/vJetPt, vAllPt/vJetPt );
	AllVLeadArea     ->Fill ( vLeadArea, vAllArea );
	AllVLeadAreaFrac ->Fill ( vLeadArea/vJetArea, vAllArea/vJetArea );
	AllVLeadDeltaR   ->Fill ( vLeadDeltaR, vAllDeltaR );
      }
    }


    
  }
  // // cleanup
  // // -------
  // delete JA;
    
    
  return 0;

}


// Helper to deal with repetitive stuff
TStarJetPicoReader* SetupReader ( TChain* chain, TString TriggerString, const double RefMultCut ){
  TStarJetPicoDefinitions::SetDebugLevel(0); // 10 for more output

  // Create on the heap
  TStarJetPicoReader* pReader = new TStarJetPicoReader();
  TStarJetPicoReader& reader = *pReader;
  reader.SetInputChain (chain);

  // Event and track selection
  // -------------------------
  TStarJetPicoEventCuts* evCuts = reader.GetEventCuts();
  evCuts->SetTriggerSelection( TriggerString ); //All, MB, HT, pp, ppHT, ppJP
  // Additional cuts 
  evCuts->SetVertexZCut ( SubjetParameters::VzCut);
  evCuts->SetRefMultCut ( RefMultCut );
  evCuts->SetVertexZDiffCut( SubjetParameters::VzDiffCut );
  evCuts->SetMaxEventPtCut ( SubjetParameters::MaxEventPtCut );
  evCuts->SetMaxEventEtCut ( SubjetParameters::MaxEventEtCut );
  // evCuts->SetReferenceCentralityCut (  6, 8 ); // 6,8 for 0-20%
  // evCuts->SetMinEventEtCut ( -1.0 );
  // evCuts->SetMinEventEtCut ( 6.0 );


  // Tracks cuts
  TStarJetPicoTrackCuts* trackCuts = reader.GetTrackCuts();
  trackCuts->SetDCACut( SubjetParameters::DcaCut );
  trackCuts->SetMinNFitPointsCut( SubjetParameters::NFitMin );
  trackCuts->SetFitOverMaxPointsCut( SubjetParameters::NFitRatio );
  trackCuts->SetMaxPtCut ( SubjetParameters::PtTrackMax );

  std::cout << "Using these track cuts:" << std::endl;
  std::cout << " dca : " << trackCuts->GetDCACut(  ) << std::endl;
  std::cout << " nfit : " <<   trackCuts->GetMinNFitPointsCut( ) << std::endl;
  std::cout << " nfitratio : " <<   trackCuts->GetFitOverMaxPointsCut( ) << std::endl;
  std::cout << " maxpt : " << trackCuts->GetMaxPtCut (  ) << std::endl;
  
  // Towers
  TStarJetPicoTowerCuts*  towerCuts = reader.GetTowerCuts();
  towerCuts->SetMaxEtCut(SubjetParameters::EtTowerMax);

  std::cout << "Using these tower cuts:" << std::endl;
  std::cout << "  GetMaxEtCut = " << towerCuts->GetMaxEtCut() << std::endl;
  std::cout << "  Gety8PythiaCut = " << towerCuts->Gety8PythiaCut() << std::endl;

  // V0s: Turn off
  reader.SetProcessV0s(false);

  return pReader;
}
