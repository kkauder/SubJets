/** @file DevSubjetAnalysis.hh
    @author Kolja Kauder
    @version Revision 0.1
    @brief Class for Subjet analysis
    @details Uses JetAnalyzer objects to perform Subjet analysis.
    @date Mar 05, 2015
*/

#ifndef __DEVSUBJETANALYSIS_HH
#define __DEVSUBJETANALYSIS_HH

#include "SubjetParameters.hh"
#include "JetAnalyzer.hh"

#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TString.h"
#include "TChain.h"


// Not needed for analysis per se
#include "TStarJetPicoReader.h"
#include "TStarJetPicoEvent.h"
#include "TStarJetPicoEventHeader.h"
#include "TStarJetPicoEventCuts.h"

#include "TStarJetPicoPrimaryTrack.h"
#include "TStarJetPicoTrackCuts.h"
#include "TStarJetPicoTowerCuts.h"

#include "TStarJetVectorContainer.h"
#include "TStarJetVector.h"
#include "TStarJetPicoTriggerInfo.h"
#include "TStarJetPicoUtils.h"

// Standard C/C++ helpers
#include <assert.h>
#include <iostream>
#include <cmath>

using std::vector;
using std::cout; using std::cerr; using std::endl;
using std::string;
using fastjet::JetDefinition;
using fastjet::ClusterSequenceActiveAreaExplicitGhosts;
using fastjet::PseudoJet;
using fastjet::Selector;
using fastjet::SelectorIsPureGhost;
using fastjet::Subtractor;
using fastjet::GhostedAreaSpec;
using fastjet::AreaDefinition;
using fastjet::sorted_by_pt;
using fastjet::antikt_algorithm;
using fastjet::kt_algorithm;

/**
   The main class
 */
class DevSubjetAnalysis {
private :
  double R;                       ///< jet radius
  double SubR;                    ///< subjet radius
  JetAlgorithm LargeJetAlgorithm; ///< fastjet::JetAlgorithm used for original jets
  JetAlgorithm SubJetAlgorithm;   ///< fastjet::JetAlgorithm used for subjets
				   
  double PtJetMin;       ///< minimum jet p<SUB>T</SUB>
  double PtJetMax;       ///< maximum jet p<SUB>T</SUB>

  double EtaConsCut;     ///< Constituent |&eta;| acceptance
  double PtConsMin;      ///< Constituent pT minimum
  double PtConsMax;      ///< Constituent pT maximum

  double EtaJetCut;      ///< jet |&eta;| acceptance, should be <= EtaConsCut - R
  double EtaGhostCut;    ///< for ghosted area, should be >= EtaJetCut + 2*R

  
  JetDefinition JetDef;         ///< Original jet definition
  JetDefinition SubJetDef;      ///< Subjet jet definition

  Selector select_cons_eta;     ///< constituent rapidity selector
  Selector select_cons_pt;      ///< constituent p<SUB>T</SUB> selector
  Selector select_cons_charge;  ///< constituent charge selector

  Selector select_cons;         ///< compound constituent selector
  
  // Relevant jet candidates
  Selector select_jet_eta;      ///< jet |&eta;| selector
  Selector select_jet_pt;       ///< jet p<SUB>T</SUB> selector
  Selector select_jet;                  ///< compound jet selector

  // Subjet selectors
  Selector select_subjet_eta;   ///< subjet |&eta;| selector
  Selector select_subjet_pt;    ///< subjet p<SUB>T</SUB> selector
  Selector select_subjet;       ///< compound subjet selector


  // Area & Background 
  GhostedAreaSpec AreaSpec;    ///< ghosted area specification
  AreaDefinition AreaDef;      ///< jet area definition

  
  vector<PseudoJet> JAResult;  ///< Unaltered clustering result

  // Histos to fill
  // --------------
  TH2D*  NsubPt;                ///< Number of subjets
  TH2D*  SubPtFrac;             ///< pT fraction carried by leading subjet
  TH2D*  OtherPtFrac;           ///< pT fraction carried by non-leading subjets
  TH2D*  SubDeltaR;             ///< &Delta;R of leading subjet
  TH2D*  OtherDeltaR;           ///< #Delta R of non-leading subjets
  TH2D*  JetArea;               ///< Area of original jet
  TH2D*  SubArea;               ///< Area of leading subjet
  TH2D*  OtherArea;             ///< Area of non-leading subjets
  TH2D*  SubAreaFrac;           ///< Area Fraction of leading subjet
  TH2D*  OtherAreaFrac;         ///< Area Fraction of non-leading subjets

  TH2D* gNsubPt;                ///< Number of subjets, leading gluon
  TH2D* gSubPtFrac;             ///< pT fraction carried by leading subjet, leading gluon
  TH2D* gOtherPtFrac;           ///< pT fraction carried by non-leading subjets, leading gluon
  TH2D* gSubDeltaR;             ///< &Delta;R of leading subjet, leading gluon
  TH2D* gOtherDeltaR;           ///< #Delta R of non-leading subjets, leading gluon
  TH2D* gJetArea;               ///< Area of original jet, leading gluon
  TH2D* gSubArea;               ///< Area of leading subjet, leading gluon
  TH2D* gOtherArea;             ///< Area of non-leading subjets, leading gluon
  TH2D* gSubAreaFrac;           ///< Area Fraction of leading subjet, leading gluon
  TH2D* gOtherAreaFrac;         ///< Area Fraction of non-leading subjets, leading gluon
  
  TH2D* qNsubPt;                ///< Number of subjets, leading quark
  TH2D* qSubPtFrac;             ///< pT fraction carried by leading subjet, leading quark
  TH2D* qOtherPtFrac;           ///< pT fraction carried by non-leading subjets, leading quark
  TH2D* qSubDeltaR;             ///< &Delta;R of leading subjet, leading quark
  TH2D* qOtherDeltaR;           ///< #Delta R of non-leading subjets, leading quark
  TH2D* qJetArea;               ///< Area of original jet, leading quark
  TH2D* qSubArea;               ///< Area of leading subjet, leading quark
  TH2D* qOtherArea;             ///< Area of non-leading subjets, leading quark
  TH2D* qSubAreaFrac;           ///< Area Fraction of leading subjet, leading quark
  TH2D* qOtherAreaFrac;         ///< Area Fraction of non-leading subjets, leading quark

public:
  /** Standard constructor. Set up analysis parameters.
      \param R: jet resolution parameter (radius)
      \param SubR: subjet resolution parameter (radius)
      \param LargeJetAlgorithm: fastjet::JetAlgorithm used for original jets
      \param SubJetAlgorithm: fastjet::JetAlgorithm used for subjets
      \param PtJetMin: minimum jet p<SUB>T</SUB>
      \param PtJetMax: maximum jet p<SUB>T</SUB>
      
      \param EtaConsCut: Constituent |&eta;| acceptance
      \param PtConsMin:  Constituent p<SUB>T</SUB> minimum
      \param PtConsMax:  Constituent p<SUB>T</SUB> maximum

      Other parameters are histograms to fill
  */
DevSubjetAnalysis ( double R = 0.4, double SubR = 0.2,
		 JetAlgorithm LargeJetAlgorithm=antikt_algorithm,
		 JetAlgorithm SubJetAlgorithm=kt_algorithm,
		 double PtJetMin  = 20.0, double PtJetMax = 100.0,
		 double EtaConsCut = 1.0, double PtConsMin = 2.0, double PtConsMax=30,
		 TH2D*  NsubPt=0, TH2D*  SubPtFrac=0, TH2D*  OtherPtFrac=0, TH2D*  SubDeltaR=0, TH2D*  OtherDeltaR=0, TH2D*  JetArea=0, TH2D*  SubArea=0, TH2D*  OtherArea=0, TH2D*  SubAreaFrac=0, TH2D*  OtherAreaFrac=0,
		 TH2D* gNsubPt=0, TH2D* gSubPtFrac=0, TH2D* gOtherPtFrac=0, TH2D* gSubDeltaR=0, TH2D* gOtherDeltaR=0, TH2D* gJetArea=0, TH2D* gSubArea=0, TH2D* gOtherArea=0, TH2D* gSubAreaFrac=0, TH2D* gOtherAreaFrac=0,
		 TH2D* qNsubPt=0, TH2D* qSubPtFrac=0, TH2D* qOtherPtFrac=0, TH2D* qSubDeltaR=0, TH2D* qOtherDeltaR=0, TH2D* qJetArea=0, TH2D* qSubArea=0, TH2D* qOtherArea=0, TH2D* qSubAreaFrac=0, TH2D* qOtherAreaFrac=0
		 );
  
  /** Main analysis routine.
   * \param particles: Current event
   * \param pPartons: optional, partons to be mnatched from original hard event
   * Return value:
   *   -  0: All went well
   *   - -1: No jets found
   */
  int AnalyzeAndFill ( vector<PseudoJet>& particles, vector<PseudoJet>* pPartons );
  
  // Getters and Setters
  // -------------------
  /// Get jet radius
  inline double GetR ( )                   { return R; };
  /// Set jet radius
  inline void   SetR ( const double newv ) { R=newv;   };
  
  // Objects will be handed by _reference_! Obviates need for setter
  /// Handle to jet definition
  inline JetDefinition& GetJetDdef () { return JetDef; }
  
  /// Handle to selector for jets
  inline Selector& GetJetSelector () { return select_jet; }
  /// Handle to selector for subjets
  inline Selector& GetSubjetSelector () { return select_subjet; }
  /// Handle to selector for constituents
  inline Selector& GetConsSelector () { return select_cons; }

  /// Handle to ghosted area specification
  inline GhostedAreaSpec& GetAreaSpec () { return AreaSpec; }
  /// Handle to jet area definition
  inline AreaDefinition& GetAreaDef () { return AreaDef; }

  /// Handle to unaltered clustering result with high pT constituents
  inline vector<PseudoJet> GetJAResult() {return JAResult; };

};  


/** Helper to set up a TStarJetPicoReader 
    \param chain: pointer to a TStarJetPicoReader chain
    \param TriggerString: trigger string in the TStarJetPicoReader. Possible values: 
    <tt>All, MB, HT, pp, ppHT, ppJP</tt>
 */
TStarJetPicoReader SetupReader ( TChain* chain, TString TriggerString );


#endif // __DEVSUBJETANALYSIS_HH
