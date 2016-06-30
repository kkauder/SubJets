/** @file SubjetParameters.hh
    @author Kolja Kauder
    @brief Common parameters
    @details Used to quickly include the same parameters into different macros.
    @date Mar 05, 2015
   
 */

#ifndef SUBJETPARAMETERS_HH
#define SUBJETPARAMETERS_HH

#include "JetAnalyzer.hh"

using fastjet::JetAlgorithm;
using fastjet::antikt_algorithm;
using fastjet::kt_algorithm;
using fastjet::cambridge_algorithm;


namespace SubjetParameters{
  const double R = 0.4;            ///< Resolution parameter ("radius").
  const double SubR = 0.2;         ///< Resolution parameter for subjet finding.

  /// Jet algorithm for the original jets
  const JetAlgorithm LargeJetAlgorithm=antikt_algorithm;
  /// Jet algorithm for subjets
  const JetAlgorithm SubJetAlgorithm=kt_algorithm;
  
  /// Repetitions in the background. Anything other than 1 WILL NOT WORK because
  /// a) we're using explicit ghosts (though we don't have to)
  /// b) more importantly, the background subtractor contains fastjet::SelectorNHardest(2)
  ///    which doesn't work jet-by-jet and throws an error
  const int ghost_repeat = 1;
  //const double ghost_area = 0.01;    ///< ghost area
  const double ghost_area = 0.0005;    ///< ghost area

  // const double PtJetMin = 20.0;    ///< Min jet pT
  const double PtJetMin = 5.0;    ///< Min jet pT
  const double PtJetMax = 1000.0;   ///< Max jet pT
    
  const double EtaConsCut = 1.0;    ///< Constituent |&eta;| acceptance
  const double PtConsMin=0.2;       ///< Constituent pT minimum
  // const double PtConsMin=2.0;       ///< Constituent pT minimum
  const double PtConsMax=1000;        ///< Constituent pT maximum
  
  const double RefMultCut=0;        ///< Reference multiplicity. Needs to be rethought to accomodate pp and AuAu
  
  const double VzCut=30;            ///< Vertex z 
  // const double VzDiffCut=6;         ///< |Vz(TPC) - Vz(VPD)| <-- NOT WORKING in older data (no VPD)
  const double VzDiffCut=1000;      ///< |Vz(TPC) - Vz(VPD)|
  
  const double DcaCut=1.0;          ///< track dca
  const double NFitMin=20.0;        ///< Constituent minimum fit points
  const double NFitRatio=0.52;      ///< Constituent NFit / NFitPossible cut

  // ************************************
  // Do NOT cut high tracks and towers!
  // Instead, reject the whole event when
  // of these is found
  // ************************************
  const double EtTowerMax=1000;       ///< tower ET cut
  const double PtTrackMax=1000;       ///< tower ET cut

  
  // EVENT rejection cuts
  const double MaxEventPtCut=30;       ///< track pT cut for event
  const double MaxEventEtCut=30;       ///< tower ET cut for event

  
  // const double VzCut=30;            ///< Vertex z 
  // const double VzDiffCut=6;         ///< |Vz(TPC) - Vz(VPD)|

  // // Tracks: Some standard high quality cuts
  // const double DcaCut=1.0;          ///< Constituent dca cut



}
#endif // SUBJETPARAMETERS_HH
