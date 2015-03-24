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
  
  const double PtJetMin = 20.0;    ///< Min jet pT
  const double PtJetMax = 100.0;   ///< Max jet pT
  

  const double VzCut=30;            ///< Vertex z 
  const double RefMultCut=0;        ///< Reference multiplicity. Needs to be rethought to accomodate pp and AuAu
  const double VzDiffCut=6;         ///< |Vz(TPC) - Vz(VPD)|

  // Tracks: Some standard high quality cuts
  const double DcaCut=1.0;          ///< Constituent dca cut
  const double NFitMin=20.0;        ///< Constituent minimum fit points
  const double NFitRatio=0.52;      ///< Constituent NFit / NFitPossible cut
  const double EtaConsCut = 1.0;    ///< Constituent |&eta;| acceptance
  // const double PtConsMin=0.2;       ///< Constituent pT minimum
  const double PtConsMin=2.0;       ///< Constituent pT minimum
  const double PtConsMax=30;        ///< Constituent pT maximum

  // Towers:
  const double EtTowerMax=60;       ///< Tower E_T maximum

}
#endif // SUBJETPARAMETERS_HH
