
#ifndef __INCLUDE__LinesAsSpotsProcessing_H__
#define __INCLUDE__LinesAsSpotsProcessing_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets\QuantityFrame.h"
#include <imageproc\clusters\segmentation.h>
#include "helpers\FramesHelper.h"
#include "imageproc\ExtractVObjectResult.h"
#include "SpotsDataProcessing.h"
#include "Math\FRegression.h"

#ifndef UINT8
#define UINT8 unsigned char
#endif

#ifndef UINT16_MAX
#define UINT16_MAX 0xffff
#define UINT8_MAX  0xff
#endif

#define MAX_SPOT_NUM      1000
#define ZONES_NUM        15


struct SpotItem {
  int iIndx = -1;
  cmplx Center /*= 0.0*/; // cmplx constructor does zeroing
  bool Added = false;
  CmplxArray Contur;
};

typedef vector<SpotItem> SpotItemVec ;
typedef vector<SpotItemVec> SpotItemVecs ;


class LinesAsSpotsProcessing
{
private:
  RegrVector m_RiseInZoneRegressions ;
  RegrVector m_FallInZoneRegressions ;
  RegrVector m_RiseLinesRegressions ;
  RegrVector m_FallLinesRegressions ;

  StatVector m_RiseStatistics ;
  StatVector m_FallStatistics ;
  StatVector m_ProfileCentersStatistics ;
  StatVector m_RiseLinesStatistics ;
  StatVector m_FallLinesStatistics ;

  
  bool m_bVerticalLines = false;
  double m_dLastAngle_deg = 0. ;

public:
  double m_dScale_unit_per_pixel = 1. ;
  CmplxVectors m_ProfileCenters ;
  CmplxVectors m_SpotCentersByProfile ;
  LinesAsSpotsStatistics m_LineAsSpotsStatistics;
//  SpotItemVecs m_WithMissingSpotsCenter;
  SpotVectors m_WithMissingSpots;
  vector<CDRect> m_Zones ;

  CFStatistics m_LeadRaggedness_um ;
  CFStatistics m_TrailRaggedness_um ;
  CFStatistics m_Width_um ;
//   CFStatistics m_MinWidth_um ;
//   CFStatistics m_MaxWidth_um ;
//  CFStatistics m_StdWidth_um ;
  CFStatistics m_LeadAngle_deg ;
  CFStatistics m_TrailAngle_deg ;
  CFStatistics m_Dist_um ;
  CFStatistics m_BoundLeft_um ;
  CFStatistics m_BoundRight_um ;
  CFStatistics m_BoundTop_um ;
  CFStatistics m_BoundBottom_um ;
  CFStatistics m_BoundHeight_um ;
  CFStatistics m_Xmid_um ;
  CFStatistics m_Ymid_um ;

  FXString m_sOutDoubleFormat;
  
  LinesAsSpotsProcessing();
  
  FXString CentersRegressionForLinesAsSpots(const CmplxVectors &spotCenters);
  void ExtractEdgesOfLines( 
    CmplxVectors& AllRises , CmplxVectors& AllFalls , 
    CmplxVectors& AllRiseLines , CmplxVectors& AllFallLines ,
    CContainerFrame * pMarking = NULL ) ;
  void ExtractLinesOfEdges( CmplxVectors& AllCrosses ,
    CmplxVectors& Lines , double dAverDist , double& dAvAngle ,
    CContainerFrame * pMarking , DWORD dwColor = 0x00ff00 ) ;
  void FilterMinMaxProfile( CmplxVector &localProfileMaximums ,
    CmplxVector &localProfileMinimums, CmplxVector &filteredlocalProfileMax, 
    CmplxVector &filteredlocalProfileMin, CString sZone);
  void FilterProfileMinMax(CmplxVector &localProfileMaximums, 
    CmplxVector &localProfileMinimums, 
    CmplxVector &filteredlocalProfileMax, 
    CmplxVector &filteredlocalProfileMin, CString sZone);
  void FilterSpotsCenter(const NamedCDRects& ROIs, 
    CmplxVectors &spotCentersByprofilePerSpot, 
    CmplxVectors &filteredsSpotCenters);
  void FindAnglePerRow(vector<vector<CFRegression>> &RegrUpperVectorPerZone, 
    vector<vector<CFRegression>> &RegrLowerVectorPerZone, 
    CFRegression *RowsCenterReg , FXString &resAsString);
  void FindLastPointInZoneByProfile(
    CmplxVectors &profileCentersPerSpotPerZone, 
    IntVector &LastPointInIndex, int &iMinLastPointInZone, 
    CContainerFrame * pMarking);
  void FindLocalMinMax(CmplxVector &filteredlocalProfileMin, 
    IntVector &localMinimumsIndex, CmplxVector &filteredlocalProfileMax, 
    IntVector &localMaximumsIndex, CmplxVector &cmplxVector, 
    const int iIndexTostart, CString sZone, 
    const double xMinThreshold, const double xMaxThreshold);
  void FindLocalProfileMinMax(
    CmplxVector &filteredlocalProfileMin, 
    IntVector &localMinimumsIndex, CmplxVector &filteredlocalProfileMax, 
    IntVector &localMaximumsIndex, 
    CmplxVector &cmplxVector, const int iIndexTostart, 
    CString sZone, const double xMinThreshold, const double xMaxThreshold, 
    CContainerFrame * pMarking);
 void FindMinMax(CmplxVector &localProfileMin,
    IntVector &localMinimumsIndex, CmplxVector &localProfileMax, 
    IntVector &localMaximumsIndex, CmplxVector &cmplxVector, 
    int iIndexTostart, CString sZone, double xMinThreshold, 
    double xMaxThreshold, CContainerFrame * pMarking);
 void FindMinMax_X( const CmplxVector &ConturCmplxArray , double& dMin , double& dMax );
 void FindMinMax_X( const CmplxArray& ConturCmplxArray , double& dMin , double& dMax );
 void FindMissingSpotsByProfile( SpotVectors& m_WithMissingSpots ,
    SpotVectors& sSeparatedResults, CContainerFrame * pMarking);
  void FindPotentialMissingLinesAsSpots( SpotVectors& sSeparatedResults , 
    int potentialMissimgSpotsIndex[ ZONES_NUM ][ MAX_SPOT_NUM ] , 
    CContainerFrame * pMarking );
  void FindProfileCenter( const SpotVectors& sSeparatedResults ,
    NamedCmplxVectors& Profiles, CmplxVectors &profileCentersPerSpotPerZone, 
    CContainerFrame * pMarking);
  void FindProfileCenters( const NamedCDRects& ROIs , NamedCmplxVectors& Profiles ,
    CmplxVectors& profileCentersPerSpot , CmplxVectors& spotCentersByprofilePerSpot ,
    CContainerFrame * pMarking );
  void FindProfileCenters( const NamedCDRects& ROIs , NamedCmplxVectors& Profiles ,
    CContainerFrame * pMarking );
  void FindProfileCrossPoints(
    CmplxVector& Rises , IntVector& RiseIndexes ,
    CmplxVector& Falls , IntVector& FallIndexes ,
    CmplxVector &ProfileData , CString sZone ,
    CContainerFrame * pMarking ) ;
  FXString GetAsString_Av_Std_Min75_Max75_Min95_Max95( CFStatistics& Stat ,
    LPCTSTR pNumbersFormat = "%.3f" , LPCTSTR pPrefix = NULL ) ;
  void GetDiffYStatisticsForData( CmplxVector& Data , CFStatistics& Result ) ;
  FXString LinesAsSpotsProcessing::GetResultDescriptionForLines() ;
  FXString GetStatResultsForLines() ;
  void GetStatisticsForData( CmplxVector& Data , CFStatistics& Result ) ;
  int MovingAverageProfile( CmplxVector &Profile , CmplxVector &Results , CContainerFrame * pMarking );

//   FXString RegressionForLinesAsSpots( const SpotVectors &sSeparatedResults ,
//     const SpotVectors &WithMissingSpots , vector<vector<SpotItem>> &WithMissingSpotsCenter );
  FXString RegressionForLinesAsSpots( const SpotVectors &WithMissingSpots );
  void Reset() ;

  void WriteToFile(CArray<CString> &sBufArr, CString sZone, CString sileName); //Debug
  void WriteToFileProfileRawData(NamedCmplxVectors& Profiles); //Debug
  
  void SetVertical(bool bVert) { m_bVerticalLines = bVert;  }
  bool IsVert() { return m_bVerticalLines; }

  struct SpotRegressionItem {
    CString name;
    int index;
    int Zone;
    bool added = false;
    double UpperAngle = 0.0;
    double LowerAngle = 0.0;
    double CenterAngle = 0.0;
  };

  struct SpotStatisticsItem {
    int iSamples = 0;
    double Sum = 0.0;
    double Avr = 0.0;

  };

  double CalculateAngle(CFRegression  &Regression)
  {
    double dAngle = 0.0;
    Regression.Calculate();

    CLine2d line2d = Regression.GetCLine2d();
    dAngle = RadToDeg(line2d.get_angle());

    if (RadToDeg(line2d.get_angle()) < -90)
    {
      dAngle = RadToDeg(line2d.get_angle()) + 180;
    }
    else if (RadToDeg(line2d.get_angle()) > 90)
    {
      dAngle = RadToDeg(line2d.get_angle()) - 180;
    }
    return dAngle;
  }

  bool ClearSpotStatisticsArr(SpotStatisticsItem *spotStatisticsItemArr, int iSize)
  {
    try {
      for (int iItem = 0; iItem < iSize; iItem++)
      {
        spotStatisticsItemArr[iItem].iSamples = 0;
        spotStatisticsItemArr[iItem].Sum = 0.0;
        spotStatisticsItemArr[iItem].Avr = 0.0;
      }
      return true;
    }
    catch(exception ex)
    {
      return false;
    }
  }
};

#endif // __INCLUDE__LinesAsSpotsProcessing_H__
