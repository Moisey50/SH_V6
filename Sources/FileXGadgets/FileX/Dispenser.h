#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>      // std::stringstream

class Dispenser
{
public:
  //string m_Name ;
  char m_Name[ 100 ] = { '\0' } ;
  double m_dIDmin_um = 39. ; // Target values
  double m_dIDmax_um = 43. ;
  double m_dIDMax_diff_um = 3. ;
  double m_dBlankIDmin_um = 18. ; // min hole diameter before grinding
  double m_dBlankIDmax_um = 33. ; // max hole diameter before grinding
  double m_dODmin_um = 211. ;
  double m_dODmax_um = 231. ;
  double m_dConuseAngleMin_deg = 29. ;
  double m_dConuseAngleMax_deg = 31. ;
  double m_dInternalConuseAngle_deg = 20. ;
  double m_dMaxEccentricity_um = 2.5 ;

  int    m_iFrontForWhiteExposure_us = 5000 ;
  int    m_iFrontForBlackExposure_us = 400 ;
  int    m_iSideExposure_us = 400 ;
  int    m_iDiameterAverage = 5 ;
  int    m_iGainForWhite_dBx10 = 370; // may be some other units known by camera
  int    m_iGainForBlack_dBx10 = 450; // may be some other units known by camera

  double m_dResultConuseAngle_deg = 0. ;
  // Averaged results
  double m_dIDWhiteResultSum_um ;   // White Processing result
  double m_dIDWhiteResultMinSum_um ; 
  double m_dIDWhiteResultMaxSum_um ;
  double m_dTIRw_Sum_um ;
  int    m_iWhiteResultCounter  ;

  double m_dIDWhiteRotationResultSum_um;   // White Processing result
  double m_dIDWhiteRotationResultMinSum_um;
  double m_dIDWhiteRotationResultMaxSum_um;
  double m_dTIRwRotation_Sum_um;
  int    m_iWhiteRotationResultCounter;

  double m_dIDWhiteResultWithIdleSum_um;   // White Processing result
  double m_dIDWhiteResultWithIdleMinSum_um;
  double m_dIDWhiteResultWithIdleMaxSum_um;
  int    m_iWhiteResultWithIdleCounter;


  double m_dIDBlackResultSum_um ;  // Black Processing result
  double m_dIDBlackResultMinSum_um ; 
  double m_dIDBlackResultMaxSum_um ;
  double m_dODBlackResultSum_um ;
  double m_dTIRb_Sum_um ;
  int    m_iBlackResultCounter ;

  double m_dODSideResultSum_um ;    // Side processing results
  double m_dResultConuseAngleSum_deg ;
  int    m_iSideResultCounter ;

  cmplx m_cCenter ;

  Dispenser()
  { 
    ResetAllResults() ;
  }
  Dispenser( LPCTSTR pName , double dIDmin_um , double dIDmax_um ,
    double dODmin_um , double dODmax_um , double dConuseAngle_deg ,
    double dConusAngleTol_deg = 1. , int iGain = 230 )
  {
//    m_Name = pName ;
    strcpy_s( m_Name , pName ) ;
    m_dIDmin_um = dIDmin_um ;
    m_dIDmax_um = dIDmax_um ;
    m_dBlankIDmin_um = dIDmin_um * 0.75 ;
    m_dBlankIDmax_um = dIDmin_um * 0.95 ;
    m_dIDMax_diff_um = 3. ;
    m_dODmin_um = dODmin_um ;
    m_dODmax_um = dODmax_um ;
    m_dConuseAngleMin_deg = dConuseAngle_deg - dConusAngleTol_deg ;
    m_dConuseAngleMax_deg = dConuseAngle_deg + dConusAngleTol_deg ;
    m_dInternalConuseAngle_deg = dConuseAngle_deg - 10. ;
    m_iDiameterAverage = 5 ;
    m_iGainForWhite_dBx10 = iGain;
    m_iGainForBlack_dBx10 = iGain;

    m_iFrontForBlackExposure_us = 400;
    m_iFrontForWhiteExposure_us = 5000;
    m_iSideExposure_us = 400;
    ResetAllResults() ;
  }
  Dispenser( LPCTSTR pPartName ) ;
  void SaveDispenserData( LPCTSTR pPartName = NULL ) ;
  void RestoreDispenserData( LPCTSTR pPartName = NULL ) ;
  int RestorePartDataFromRegistry(
    LPCTSTR pFolderForParts , LPCTSTR pPartName , bool bSetDefault = false ) ;
  Dispenser& operator=( Dispenser& Orig )
  {
//    m_Name = Orig.m_Name ;
//    memcpy( &m_dIDmin_um , &Orig.m_dIDmin_um , sizeof( Dispenser ) - sizeof( string ) ) ;
    memcpy( &m_Name , &Orig.m_Name , sizeof( Dispenser ) ) ;
    return *this ;
  }
  double GetNominalHoleDia_um() { return 0.5 * (m_dIDmin_um + m_dIDmax_um); }
  double GetNominalTipDia_um() { return 0.5 * (m_dODmin_um + m_dODmax_um); }
  void ResetAllResults()
  {
    memset( &m_dIDWhiteResultSum_um , 0 , ( LPBYTE ) &m_cCenter - ( LPBYTE ) &m_dIDWhiteResultSum_um ) ;
  }
  void ResetWhiteResults()
  {
    memset(&m_dIDWhiteResultSum_um, 0,
      (LPBYTE)&m_dIDWhiteRotationResultSum_um - (LPBYTE)&m_dIDWhiteResultSum_um);
  }
  void ResetWhiteRotationResults()
  {
    memset( &m_dIDWhiteRotationResultSum_um , 0 ,
      ( LPBYTE ) &m_dIDWhiteResultWithIdleSum_um - ( LPBYTE ) &m_dIDWhiteRotationResultSum_um );
  }
  void ResetWhiteResultsWithIdle()
  {
    memset( &m_dIDWhiteResultWithIdleSum_um , 0 ,
      ( LPBYTE ) &m_dIDBlackResultSum_um - ( LPBYTE ) &m_dIDWhiteResultWithIdleSum_um );
  }
  void ResetBlackResults()
  {
    memset( &m_dIDBlackResultSum_um , 0 , ( LPBYTE ) &m_dODSideResultSum_um - ( LPBYTE ) &m_dIDBlackResultSum_um ) ;
  }
  void ResetSideResults()
  {
    memset( &m_dODSideResultSum_um , 0 , ( LPBYTE ) &m_cCenter - ( LPBYTE ) &m_dODSideResultSum_um ) ;
  }

  int AddToWhite( double dAver , double dMin , double dMax , double dTIRw )
  {
    m_dIDWhiteResultSum_um += dAver ;
    m_dIDWhiteResultMinSum_um += dMin ;
    m_dIDWhiteResultMaxSum_um += dMax ;
    m_dTIRw_Sum_um += dTIRw ;
    return ++m_iWhiteResultCounter ;
  }
  // returns average white diameter or zero if no results
  double GetDIWhiteAverageResults( double& dIDWhiteMin_um , double& dIDWhiteMax_um , double& dTIRw )
  {
    if ( m_iWhiteResultCounter )
    {
      dIDWhiteMin_um = m_dIDWhiteResultMinSum_um / m_iWhiteResultCounter ;
      dIDWhiteMax_um = m_dIDWhiteResultMaxSum_um / m_iWhiteResultCounter ;
      dTIRw = m_dTIRw_Sum_um / m_iWhiteResultCounter ;
      return m_dIDWhiteResultSum_um / m_iWhiteResultCounter ;
    }
    return 0. ;
  }

  int AddToWhiteRotation(double dAver, double dMin, double dMax, double dTIRw)
  {
    m_dIDWhiteRotationResultSum_um += dAver;
    m_dIDWhiteRotationResultMinSum_um += dMin;
    m_dIDWhiteRotationResultMaxSum_um += dMax;
    m_dTIRwRotation_Sum_um += dTIRw;
    return ++m_iWhiteRotationResultCounter;
  }
  // returns average white diameter or zero if no results
  double GetDIWhiteRotationAverageResults(double& dIDWhiteRotationMin_um,
    double& dIDWhiteRotationMax_um, double& dTIRwRotation_um)
  {
    if (m_iWhiteResultCounter)
    {
      dIDWhiteRotationMin_um = m_dIDWhiteRotationResultMinSum_um / m_iWhiteRotationResultCounter;
      dIDWhiteRotationMax_um = m_dIDWhiteRotationResultMaxSum_um / m_iWhiteRotationResultCounter;
      dTIRwRotation_um = m_dTIRwRotation_Sum_um / m_iWhiteRotationResultCounter;
      return m_dIDWhiteRotationResultSum_um / m_iWhiteRotationResultCounter;
    }
    return 0.;
  }

  int AddToWhiteWithIdle( double dAver , double dMin , double dMax )
  {
    m_dIDWhiteResultWithIdleSum_um += dAver;
    m_dIDWhiteResultWithIdleMinSum_um += dMin;
    m_dIDWhiteResultWithIdleMaxSum_um += dMax;
    return ++m_iWhiteResultWithIdleCounter;
  }
  // returns average white diameter or zero if no results
  double GetDIWhiteWithIdleAverageResults( double& dIDWhiteWithIdleMin_um ,
    double& dIDWhiteWithIdleMax_um )
  {
    if ( m_iWhiteResultWithIdleCounter )
    {
      dIDWhiteWithIdleMin_um = m_dIDWhiteRotationResultMinSum_um / m_iWhiteRotationResultCounter;
      dIDWhiteWithIdleMax_um = m_dIDWhiteRotationResultMaxSum_um / m_iWhiteRotationResultCounter;
      return m_dIDWhiteResultWithIdleSum_um / m_iWhiteRotationResultCounter;
    }
    return 0.;
  }

  int AddToBlack( double dIDAver , double dIDMin ,
    double dIDMax , double dDO , double dTIRb )
  {
    m_dIDBlackResultSum_um += dIDAver ;
    m_dIDBlackResultMinSum_um += dIDMin ;
    m_dIDBlackResultMaxSum_um += dIDMax ;
    m_dODBlackResultSum_um += dDO ;
    m_dTIRb_Sum_um += dTIRb ;
    return ++m_iBlackResultCounter ;
  }
  // returns average black internal diameter or zero if no results
  double GetBlackAverageResults( double& dIDBlackMin_um , 
    double& dIDBlackMax_um , double& dODBlack_um , double& dTIRb )
  {
    if ( m_iBlackResultCounter )
    {
      dIDBlackMin_um = m_dIDBlackResultMinSum_um / m_iBlackResultCounter ;
      dIDBlackMax_um = m_dIDBlackResultMaxSum_um / m_iBlackResultCounter ;
      dODBlack_um = m_dODBlackResultSum_um / m_iBlackResultCounter ;
      dTIRb = m_dTIRb_Sum_um / m_iBlackResultCounter ;
      return m_dIDBlackResultSum_um / m_iBlackResultCounter ;
    }
    return 0. ;
  }

  int AddToSide( double dODAver , double dAngle_deg )
  {
    m_dODSideResultSum_um += dODAver ;
    m_dResultConuseAngleSum_deg += dAngle_deg ;
    return ++m_iSideResultCounter ;
  }

  // returns average cone (tip) diameter or zero if no results
  double GetSideAverageResults( double& dConusAngle_deg )
  {
    if ( m_iSideResultCounter )
    {
      dConusAngle_deg = m_dResultConuseAngleSum_deg / m_iSideResultCounter ;
      return m_dODSideResultSum_um / m_iSideResultCounter ;
    }
    return 0. ;
  }

  FXString ResultsToString() ;
};

typedef vector<Dispenser> Dispensers ;

class DispenserProcessingResults
{
public:
  FXString m_ProcessingStartTimeStamp ;
  FXString m_FinalMeasurementFinishedTimeStamp ;

  int    m_iDispenserNumber ;
  double m_dBlankInitialDI_um ;
  double m_dBlankInitialMinDI_um ;
  double m_dBlankInitialMaxDI_um ;
  double m_dBlankHeightDiffFromMaster_um ;

  double m_dBlankDOAfterCoarse_um ;
  double m_dBlankAfterSecondCenteringDI_um ;
  double m_dBlankAfterSecondCenteringMinDI_um ;
  double m_dBlankAfterSecondCenteringMaxDI_um ;

  double m_dPartAfterFrontGrindingDI_um ;
  double m_dPartAfterFrontGrindingMinDI_um ;
  double m_dPartAfterFrontGrindingMaxDI_um ;
  int    m_iNGrindingCycles ;

  double m_dPartDOAfterFine_um ;

  double m_dPartAfterPolishingDI_um ;
  double m_dPartAfterPolishingMinDI_um ;
  double m_dPartAfterPolishingMaxDI_um ;
  int    m_iNPolishingCycles ;

  double m_dPartFinalDOSide_um ;
  double m_dPartFinalDOFront_um ;
  double m_dPartFinalWDI_um ;
  double m_dPartFinalWMinDI_um ;
  double m_dPartFinalWMaxDI_um ;

  double m_dPartFinalBDI_um ;
  double m_dPartFinalBMinDI_um ;
  double m_dPartFinalBMaxDI_um ;

  double m_dTirB_um ;
  double m_dTirW_um ;

  DispenserProcessingResults() { Reset(); m_iDispenserNumber = 0 ; }

  void Reset() 
  {
    m_ProcessingStartTimeStamp.Empty() ;
    m_FinalMeasurementFinishedTimeStamp.Empty() ;
    memset( &m_dBlankInitialDI_um , 0 , &m_dTirW_um - &m_dBlankInitialDI_um + sizeof(m_dTirW_um) ) ;
  }

  FXString ToCSVString() ;
  FXString GetCaption() ;

};