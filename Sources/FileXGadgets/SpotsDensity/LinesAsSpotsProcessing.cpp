#include "StdAfx.h"
#include "LinesAsSpotsProcessing.h"

#include "SpotStatistics.h"
#include "helpers\FramesHelper.h"
#include "imageproc\ExtractVObjectResult.h"
#include <vector>
#include "Math\FRegression.h"
#include "afxcoll.h"
#include <fxfc/FXRegistry.h>
#include <..\..\gadgets\common/imageproc/clusters/Segmentation.h>

#define MIN_DISY_FOR_MINMAX      27
#define MIN_DISY_FOR_MIN      30
#define MAX_DISY_WINDOW     18
#define X_MIN_THRESHOLD_FACTOR   70
#define X_MAX_THRESHOLD_FACTOR   30



LinesAsSpotsProcessing::LinesAsSpotsProcessing():
  m_sOutDoubleFormat( "%10.3f ")
{
}

double NormAngleFromM45toP135(double dAngle)
{
  if (dAngle < -M_PI_4)
    dAngle += M_PI;
  else if (dAngle > 3. * M_PI_4)
    dAngle -= M_PI;
  ASSERT((-M_PI_4 <= dAngle) && (dAngle <= (3. * M_PI_4)));
  return dAngle;
}

////**************************************** iStatisticsOnlyByProfile == 1************************************************************////


// void LinesAsSpotsProcessing::FindProfileCenters(
//   const NamedCDRects& ROIs, NamedCmplxVectors& Profiles, 
//   CmplxVectors& profileCentersPerSpot, CmplxVectors& spotCentersByprofilePerSpot, 
//   CContainerFrame * pMarking)
// {
// 
//   //pMarking->AddFrame(CreatePtFrame(cmplx(ROIs[0].m_Rect.top, Profiles[0].m_Data[0].imag()), "color=0xffffff;Sz=4;"));
//   FXRegistry Reg("TheFileX\\SHStudio");
//   double dProfileThreshold = Reg.GetRegiDouble(
//     "SpotStatistics", "LinesAsSpotsProfileThreshold", 0.5);
// 
//   CmplxVector::iterator cmplxVectorIterator;   // All profiles' pixels per zone
// 
//   CmplxVector localProfileMinimums;
//   IntVector localMinimumsIndex;
//   CmplxVectors LocalMinimumsPerSpot;
//   IntVectors LocalMinimumsPerSpotIndex;
// 
//   CmplxVector localProfileMaximums;
//   IntVector localMaximumsIndex;
//   CmplxVectors LocalMaximumsPerSpot;
//   IntVectors LocalMaximumsPerSpotIndex;
//   CmplxVector profileCenters;
//   CmplxVector spotCentersByprofile;
// 
// 
//   CArray<CString> sProfileBufArr;//Debug
//   CArray<CString> sProfileBufArr1;//Debug
//   CString sProfileBuf = "";  //Debug
//   CString sProfileBuf1 = "";  //Debug
//   CString sZone; //Debug
// 
// 
//   CArray<CString> sProfileRawDataBufArr;//Debug
//   CString sProfileRawsBuf = "";  //Debug
// 
// 
//   if (!Profiles.size() || Profiles.size() < 5)
//     return;
//   int iZoneWithMaxSpots = 0;
//   int iMax = -10000;
//   for (int ZonesCount = 0; ZonesCount < (int)Profiles.size(); ZonesCount++)
//   {
//     sZone.Format("Zone = %d,", ZonesCount);//Debug
// //     CmplxVector Results;   FILTRATION is not necessary - profile is averaged values
// //     MovingAverageProfile(Profiles[ZonesCount].m_Data, Results, pMarking);
// 
//     double dSpotCenterByROI = (ROIs[ZonesCount + 1].m_Rect.right + ROIs[ZonesCount + 1].m_Rect.left) / 2;
//     double dXMinThreshold = dSpotCenterByROI + X_MIN_THRESHOLD_FACTOR;
//     double dXMaxThreshold = dSpotCenterByROI + X_MAX_THRESHOLD_FACTOR;
//     int iIndexSpotsStart = 0;
// 
//     //pMarking->AddFrame(CreatePtFrame(cmplx(Profiles[ZonesCount].m_Data[0].real(), Profiles[ZonesCount].m_Data[0].imag()), "color=0xffffff;Sz=4;"));
//    // pMarking->AddFrame(CreatePtFrame(cmplx(Profiles[ZonesCount].m_Data[Profiles[ZonesCount].m_Data.size() - 1].real(), Profiles[ZonesCount].m_Data[Profiles[ZonesCount].m_Data.size() - 1].imag()), "color=0xffffff;Sz=4;"));
// 
// //     FindLocalProfileMinMax( localProfileMinimums , localMinimumsIndex ,
// //       localProfileMaximums , localMaximumsIndex ,
// //       /*Profiles[ZonesCount].m_Data*/ Results , iIndexSpotsStart , sZone ,
// //       dXMinThreshold , dXMaxThreshold , pMarking );
//     FindLocalProfileMinMax( localProfileMinimums , localMinimumsIndex ,
//       localProfileMaximums , localMaximumsIndex ,
//       Profiles[ZonesCount].m_Data , iIndexSpotsStart , sZone ,
//       dXMinThreshold , dXMaxThreshold , pMarking );
//     LocalMinimumsPerSpot.push_back(localProfileMinimums);
//     LocalMinimumsPerSpotIndex.push_back(localMinimumsIndex);
//     LocalMaximumsPerSpot.push_back(localProfileMaximums);
//     LocalMaximumsPerSpotIndex.push_back(localMaximumsIndex);
// 
//     if (localProfileMinimums.size() && localProfileMaximums.size())
//     {
//       for (int iLocalMin = 0, iLocalMax = 0;
//         (iLocalMin < (int)(localProfileMinimums.size() - 1))
//         && (iLocalMax < (int)(localProfileMaximums.size() - 1));
//         iLocalMin++, iLocalMax++)
//       {
// 
//         //pMarking->AddFrame(CreatePtFrame(cmplx(localProfileMaximums[iLocalMax].real(), localProfileMaximums[iLocalMax].imag()), "color=0xff7fff;Sz=2;"));
// 
//         double dYThesholdDistance = abs((localProfileMaximums[iLocalMax].imag() - localProfileMaximums[iLocalMax + 1].imag())) / 2;
//         double dProfileCenterY = localProfileMaximums[iLocalMax + 1].imag() - dYThesholdDistance;
// 
//         double dXDistance = (localProfileMaximums[iLocalMax].real() - localProfileMaximums[iLocalMax + 1].real());
//         double dThresholdDis = abs(localProfileMaximums[iLocalMax + 1].real() - localProfileMinimums[iLocalMin].real()) * dProfileThreshold;
//         double dProfileCenterX = localProfileMaximums[iLocalMax + 1].real() - dThresholdDis;
// 
//         cmplx ProfCent(dProfileCenterX, dProfileCenterY);
//         cmplx SpotCent(dSpotCenterByROI, dProfileCenterY);
//         if ( IsVert() )
//         {
//           swap(ProfCent._Val[_RE], ProfCent._Val[_IM]);
//           swap(SpotCent._Val[_RE], SpotCent._Val[_IM]);
//         }
// 
//         pMarking->AddFrame(CreatePtFrame( ProfCent , "color=0xffff00;Sz=1;"));
//         pMarking->AddFrame(CreatePtFrame( SpotCent , "color=0x66ffff;Sz=1;"));
// 
//         profileCenters.push_back(cmplx(dProfileCenterX, dProfileCenterY));
//         spotCentersByprofile.push_back(cmplx(dSpotCenterByROI, dProfileCenterY));
// 
//         // sProfileBuf.Format("%6.2f , %6.2f\n", dProfileCenterX, dProfileCenterY);//Debug
//         // sProfileBufArr.Add(sProfileBuf);//Debug
//       }
//       profileCentersPerSpot.push_back(profileCenters);
//       spotCentersByprofilePerSpot.push_back(spotCentersByprofile);
//     }
//   
//     profileCenters.clear();
//     spotCentersByprofile.clear();
// 
//     localProfileMinimums.clear();
//     localMinimumsIndex.clear();
//     localProfileMaximums.clear();
//     localMaximumsIndex.clear();
//     
// 
// 
//     //WriteToFile(sProfileBufArr, sZone, "profileCentersData");//Debug
//    // sProfileBufArr.RemoveAll();//Debug
// 
//     //WriteToFile(sProfileRawDataBufArr, sZone, "profileMAData");//Debug
//   //  sProfileRawDataBufArr.RemoveAll();//Debug
//   }
// 
//   ///////*debug *///////////////
//  /* CmplxVectors filteredsSpotCenters;
// 
//  /* FilterSpotsCenter(ROIs, spotCentersByprofilePerSpot, filteredsSpotCenters); // need to return after repair  - 17.11.2022
//   CentersRegressionForLinesAsSpots(filteredsSpotCenters);
// 
//   /*for (int ZonesCount = 0; ZonesCount < (int)filteredsSpotCenters.size(); ZonesCount++)
//   {
//     for (int SpotCenter = 0; SpotCenter < (int)filteredsSpotCenters[ZonesCount].size(); SpotCenter++)
//     {
//      // pMarking->AddFrame(CreatePtFrame(filteredsSpotCenters[ZonesCount][SpotCenter], "color=0x000080;Sz=3;"));
//     }
//   }*/
//   ///////*debug *///////////////
// }// end FindProfileCenter FUNCTION

void LinesAsSpotsProcessing::FindProfileCenters(
  const NamedCDRects& ROIs , NamedCmplxVectors& Profiles ,
  CContainerFrame * pMarking )
{
  if ( !Profiles.size() || Profiles.size() < 5 )
    return;

  FXRegistry Reg( "TheFileX\\SHStudio" );
  double dProfileThreshold = Reg.GetRegiDouble(
    "SpotStatistics" , "LinesAsSpotsProfileThreshold" , 0.5 );
  m_sOutDoubleFormat = Reg.GetRegiString("SpotStatistics",
    "OutDoubleFormat", "%10.4f");

  m_ProfileCenters.clear() ;
  m_SpotCentersByProfile.clear() ;

  CmplxVector::iterator cmplxVectorIterator;   // All profiles' pixels per zone

  CmplxVector localProfileMinimums;
  IntVector localMinimumsIndex;
  CmplxVectors LocalMinimumsPerSpot;
  IntVectors LocalMinimumsPerSpotIndex;

  CmplxVector localProfileMaximums;
  IntVector localMaximumsIndex;
  CmplxVectors LocalMaximumsPerSpot;
  IntVectors LocalMaximumsPerSpotIndex;
  CmplxVector profileCenters;
  CmplxVector spotCentersByprofile;

  CmplxVectors AllRises ;
  CmplxVectors AllFalls ;

  CArray<CString> sProfileBufArr;//Debug
  CArray<CString> sProfileBufArr1;//Debug
  CString sProfileBuf = "";  //Debug
  CString sProfileBuf1 = "";  //Debug
  CString sZone; //Debug

  CArray<CString> sProfileRawDataBufArr;//Debug
  CString sProfileRawsBuf = "";  //Debug

  int iZoneWithMaxSpots = 0;
  int iMax = -10000;
  for ( int ZonesCount = 0; ZonesCount < ( int ) Profiles.size(); ZonesCount++ )
  {
    sZone.Format( "Zone = %d," , ZonesCount );//Debug

    double dSpotCenterByROI = ( ROIs[ ZonesCount + 1 ].m_Rect.right + ROIs[ ZonesCount + 1 ].m_Rect.left ) / 2;
    double dProfCenterByROI = dSpotCenterByROI + ROIs[ ZonesCount + 1 ].m_Rect.Width() ;
    CmplxVector Falls , Rises ;
    IntVector   FallIndexes , RiseIndexes ;
    FindProfileCrossPoints( Rises , RiseIndexes , Falls , FallIndexes ,
      Profiles[ ZonesCount ].m_Data , sZone , pMarking ) ;

    if ( Rises.size() && Falls.size() )
    {
      AllRises.push_back( Rises ) ;
      AllFalls.push_back( Falls ) ;

      CFStatistics ZoneStatistics ;
      double dPrevYPos = 0. ;

      CmplxVector SpotCentersByProfile , CentersOnProfile ;
      size_t iCntr = ( Rises[ 0 ].imag() < Falls[ 0 ].imag() ) ;
      for ( ; iCntr < Rises.size() && iCntr < Falls.size() ; iCntr++ )
      {
        cmplx cSpotCenterByProf( dSpotCenterByROI , 0.5 * ( Rises[ iCntr ].imag() + Falls[ iCntr ].imag() ) ) ;
        cmplx cProfCenterByProf( dProfCenterByROI , 0.5 * ( Rises[ iCntr ].imag() + Falls[ iCntr ].imag() ) ) ;
        SpotCentersByProfile.push_back( cSpotCenterByProf ) ;
        CentersOnProfile.push_back( cProfCenterByProf ) ;

        if ( dPrevYPos != 0. )
          ZoneStatistics.Add( cProfCenterByProf.imag() - dPrevYPos ) ;
        dPrevYPos = cProfCenterByProf.imag() ;
      }
      if ( SpotCentersByProfile.size() )
      {
        m_ProfileCenters.push_back( CentersOnProfile ) ;
        m_SpotCentersByProfile.push_back( SpotCentersByProfile ) ;
        ZoneStatistics.Calculate() ;
        m_ProfileCentersStatistics.push_back( ZoneStatistics ) ;
      }
    }
    else
    {
      ASSERT( 0 ) ;
    }
  }

  CmplxVectors RisesLines , FallLines ;
  ExtractEdgesOfLines( AllRises , AllFalls , RisesLines , FallLines , pMarking ) ;

  //  size_t iLen = RisesLines.size() ;
  // 
  // 
  //       for ( int iLocalMin = 0 , iLocalMax = 0;
  //         ( iLocalMin < ( int ) ( localProfileMinimums.size() - 1 ) )
  //         && ( iLocalMax < ( int ) ( localProfileMaximums.size() - 1 ) );
  //         iLocalMin++ , iLocalMax++ )
  //       {
  // 
  //         //pMarking->AddFrame(CreatePtFrame(cmplx(localProfileMaximums[iLocalMax].real(), localProfileMaximums[iLocalMax].imag()), "color=0xff7fff;Sz=2;"));
  // 
  //         double dYThesholdDistance = abs( ( localProfileMaximums[ iLocalMax ].imag() - localProfileMaximums[ iLocalMax + 1 ].imag() ) ) / 2;
  //         double dProfileCenterY = localProfileMaximums[ iLocalMax + 1 ].imag() - dYThesholdDistance;
  // 
  //         double dXDistance = ( localProfileMaximums[ iLocalMax ].real() - localProfileMaximums[ iLocalMax + 1 ].real() );
  //         double dThresholdDis = abs( localProfileMaximums[ iLocalMax + 1 ].real() - localProfileMinimums[ iLocalMin ].real() ) * dProfileThreshold;
  //         double dProfileCenterX = localProfileMaximums[ iLocalMax + 1 ].real() - dThresholdDis;
  // 
  //         cmplx ProfCent( dProfileCenterX , dProfileCenterY );
  //         cmplx SpotCent( dSpotCenterByROI , dProfileCenterY );
  //         if ( IsVert() )
  //         {
  //           swap( ProfCent._Val[ _RE ] , ProfCent._Val[ _IM ] );
  //           swap( SpotCent._Val[ _RE ] , SpotCent._Val[ _IM ] );
  //         }
  // 
  //         pMarking->AddFrame( CreatePtFrame( ProfCent , "color=0xffff00;Sz=1;" ) );
  //         pMarking->AddFrame( CreatePtFrame( SpotCent , "color=0x66ffff;Sz=1;" ) );
  // 
  //         profileCenters.push_back( cmplx( dProfileCenterX , dProfileCenterY ) );
  //         spotCentersByprofile.push_back( cmplx( dSpotCenterByROI , dProfileCenterY ) );
  // 
  //         // sProfileBuf.Format("%6.2f , %6.2f\n", dProfileCenterX, dProfileCenterY);//Debug
  //         // sProfileBufArr.Add(sProfileBuf);//Debug
  //       }
  //       profileCentersPerSpot.push_back( profileCenters );
  //       spotCentersByprofilePerSpot.push_back( spotCentersByprofile );
  //     }




      //WriteToFile(sProfileBufArr, sZone, "profileCentersData");//Debug
     // sProfileBufArr.RemoveAll();//Debug

      //WriteToFile(sProfileRawDataBufArr, sZone, "profileMAData");//Debug
    //  sProfileRawDataBufArr.RemoveAll();//Debug
    //  }

}// end FindProfileCenter FUNCTION

void LinesAsSpotsProcessing::ExtractLinesOfEdges( CmplxVectors& AllCrosses ,
  CmplxVectors& Lines , double dAverDist , double& dAvAngle_deg ,
  CContainerFrame * pMarking , DWORD dwColor )
{
  Size_tVector Indexes;
  Indexes.push_back( 0 );
  dAvAngle_deg = 0 ;
  int iAccounted = 0 ;

  for ( size_t iRise = 0 ; iRise < AllCrosses[ 0 ].size() ; iRise++ )
  {
    CmplxVector NextLine ;
    cmplx Pt0 = AllCrosses[ 0 ][ Indexes[ 0 ] ] ;
    NextLine.push_back( Pt0 ) ;
    for ( size_t iZone = 1 ; iZone < AllCrosses.size() ; iZone++ )
    {
      if ( Indexes.size() <= AllCrosses.size() )
        Indexes.push_back( 0 );
      size_t iCurrIndex = Indexes[ iZone ];
      if ( iCurrIndex >= AllCrosses[ iZone ].size() )
        break;
      cmplx Pt1 = AllCrosses[ iZone ][ iCurrIndex ] ;
      if ( fabs( Pt0.imag() - Pt1.imag() ) < ( dAverDist / 3. ) )
      {
        NextLine.push_back( Pt1 ) ;
        Pt0 = Pt1 ;
      }
      else if ( iCurrIndex > 0 )
      {
        Pt1 = AllCrosses[ iZone ][ iCurrIndex - 1 ];
        if ( fabs( Pt0.imag() - Pt1.imag() ) < ( dAverDist / 3. ) )
        {
          NextLine.push_back( Pt1 );
          Pt0 = Pt1;
          Indexes[ iZone ]--;
        }
      }
      else if ( iCurrIndex < AllCrosses[ iZone ].size() - 1 )
      {
        Pt1 = AllCrosses[ iZone ][ iCurrIndex + 1 ];
        if ( fabs( Pt0.imag() - Pt1.imag() ) < ( dAverDist / 3. ) )
        {
          NextLine.push_back( Pt1 );
          Pt0 = Pt1;
          Indexes[ iZone ]++;
        }
      }
      Indexes[ iZone ]++;
    }
    if ( NextLine.size() > 3 )
      Lines.push_back( NextLine ) ;

    if ( NextLine.size() > 3 )
    {
      cmplx cBegin = NextLine.front();
      cmplx cEnd = NextLine.back();
      if ( m_bVerticalLines )
      {
        SwapReIm( cBegin );
        SwapReIm( cEnd );
      }
      double dAngle_rad = -arg( cEnd - cBegin ); // Y is going down
      dAvAngle_deg += RadToDeg( dAngle_rad ) ;
      iAccounted++ ;
      if ( m_bVerticalLines )
      {
        for ( size_t i = 0 ; i < NextLine.size() ; i++ )
          SwapReIm( *( NextLine.data() + i ) ) ;
      }
      pMarking->AddFrame( CreateFigureFrame( NextLine.data() ,
        ( int ) NextLine.size() , dwColor , "RiseEdge" ) );
    }
    Indexes[ 0 ]++;
  }
  if ( iAccounted )
    dAvAngle_deg /= iAccounted ;
}

void LinesAsSpotsProcessing::ExtractEdgesOfLines( CmplxVectors& AllRises ,
  CmplxVectors& AllFalls , CmplxVectors& AllRiseLines , CmplxVectors& AllFallLines ,
  CContainerFrame * pMarking )
{
  m_RiseStatistics.clear() ;
  m_FallStatistics.clear() ;
  AllRiseLines.clear() ;
  AllFallLines.clear() ;
  for ( size_t iZone = 0 ; iZone < AllRises.size() ; iZone++ )
  {
    CmplxVector& RiseZone = AllRises.at( iZone ) ;
    CFStatistics RiseZoneStat ;

    GetDiffYStatisticsForData( RiseZone , RiseZoneStat ) ;
    if ( RiseZoneStat.GetAverage() > 0. )
      m_RiseStatistics.push_back( RiseZoneStat ) ;
    else
      ASSERT( 0 ) ;

    CmplxVector& FallZone = AllFalls.at( iZone ) ;
    CFStatistics FallZoneStat ;
    GetDiffYStatisticsForData( FallZone , FallZoneStat ) ;
    if ( FallZoneStat.GetAverage() > 0. )
      m_FallStatistics.push_back( FallZoneStat ) ;
    else
      ASSERT( 0 ) ;
  }
  double dAvRiseDist = m_RiseStatistics[ 0 ].GetAverage();
  double dAvFallDist = m_FallStatistics[ 0 ].GetAverage();
  double dRiseAvAngle_deg = 0. , dFallAvAngle_deg = 0. ;
  ExtractLinesOfEdges( AllRises , AllRiseLines ,
    dAvRiseDist , dRiseAvAngle_deg , pMarking ) ;
  ExtractLinesOfEdges( AllFalls , AllFallLines ,
    dAvFallDist , dFallAvAngle_deg , pMarking ) ;
  m_dLastAngle_deg = 0.5 * ( dRiseAvAngle_deg + dFallAvAngle_deg ) ;
}

void LinesAsSpotsProcessing::GetDiffYStatisticsForData( CmplxVector& Data , CFStatistics& Result )
{
  Result.Reset() ;
  Result.ResetHisto() ;
  int iFail = -1 ;
  for ( size_t iCnt = 1 ; iCnt < Data.size() ; iCnt++ )
  {
    double dDiff = Data[ iCnt ].imag() - Data[ iCnt - 1 ].imag();
    Result.Add( dDiff ) ;
  }
  Result.Calculate() ;
  for ( size_t iCnt = 1 ; iCnt < Data.size() ; iCnt++ )
  {
    double dDiff = Data[ iCnt ].imag() - Data[ iCnt - 1 ].imag();
    double dDistRatio = dDiff / Result.GetAverage() ;
    if ( 0.3 <= dDistRatio && dDistRatio < 3.0 )
    {
      Result.AddToHistogram( dDiff ) ;
    }
    else
      iFail = ( int ) iCnt ; // ASSERT( 0 ) ;
  }
}

FXString ProcessHistogramAndGetAsString( StatVector& Stat , double dScale )
{
  FXString Result ;

  double dAveragePeriod = 0. , dStd = 0. , dWMin_75perc = 0. ,
    dWMax_75perc = 0. , dWMin_95perc = 0. , dWMax_95perc = 0. ;

  size_t iLen = Stat.size() ;
  for ( size_t i = 0 ; i < iLen ; i++ )
  {
    dAveragePeriod += Stat[ i ].GetAverage() ;
    dStd += Stat[ i ].GetStd() ;
    double dMin , dMax ;
    Stat[ i ].GetValueForPercentOnHistogram( 75 , dMin , dMax ) ;
    dWMin_75perc += dMin ;
    dWMax_75perc += dMax ;
    Stat[ i ].GetValueForPercentOnHistogram( 95 , dMin , dMax ) ;
    dWMin_95perc += dMin ;
    dWMax_95perc += dMax ;
  }

  Result.Format( "%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f" , dAveragePeriod / iLen ,
    dStd / iLen , dWMin_75perc / iLen , dWMax_75perc / iLen , dWMin_95perc / iLen , dWMax_95perc / iLen ) ;

  return Result ;

}

FXString LinesAsSpotsProcessing::GetStatResultsForLines()
{
  FXString ResultRise = ProcessHistogramAndGetAsString( m_RiseStatistics , m_dScale_unit_per_pixel ) ;
  FXString ResultFall = ProcessHistogramAndGetAsString( m_FallStatistics , m_dScale_unit_per_pixel ) ;
  FXString AngleAsText ;
  AngleAsText.Format( "%8.4f" , m_dLastAngle_deg ) ;
  FXString Result = AngleAsText + ',' + ResultRise + ',' + ResultFall ;

  return Result ;
}

FXString LinesAsSpotsProcessing::GetResultDescriptionForLines()
{
  return FXString( "Ang_deg,dRis_um,dRisStd,dRmin75,dRMax75,dRMin95,dRMax75,"
    "dFal_um,dFalStd,dFmin75,dFMax75,dFMin95,dFMax75," ) ;
}


FXString LinesAsSpotsProcessing::CentersRegressionForLinesAsSpots( const CmplxVectors &spotCenters )
{
  CFRegression RowsCenterReg[ 300 ];
  FXString resAsString = "";

  for ( int iZone = 0; iZone < ( int ) spotCenters.size(); iZone++ )
  {
    for ( int iCenter = 0; iCenter < ( int ) spotCenters[ iZone ].size(); iCenter++ )
    {
      RowsCenterReg[ iCenter ].Add( spotCenters[ iZone ].at( iCenter ) );
    }
  }

  for ( int iAngleSum = 0; iAngleSum < ( int ) RowsCenterReg->Size(); iAngleSum++ ) //ROWS
  {

    double dCenterAngle = 0.0;
    if ( RowsCenterReg != NULL )
    {
      dCenterAngle = CalculateAngle( RowsCenterReg[ iAngleSum ] );
    }

    //Results Format: rowNum,  CenterAngle
    FXString resultRow = m_LineAsSpotsStatistics.ToString( iAngleSum , 0.0 , 0.0 , dCenterAngle );
    resAsString += resultRow;
  }
  return resAsString;

  //Result += m_LineAsSpotsStatistics.GetCaption();
}

void LinesAsSpotsProcessing::FilterSpotsCenter( const NamedCDRects& ROIs , 
  CmplxVectors &spotCentersByprofilePerSpot , CmplxVectors &filteredsSpotCenters )
{
  CmplxVector profileCenters;
  CmplxVector spotCentersByprofile;
  vector<double> firstSpotsCenterDaltasFromTop;
  vector<double> lastSpotsCenterDaltasFromBottom;
  CmplxVector spotCenters;

  if ( !spotCentersByprofilePerSpot.size() )
    return;

  int iIndexMaxTop = 0;
  int iIndexMinBottom = 0;
  double dMaxTop = spotCentersByprofilePerSpot[ 0 ][ 0 ].imag();
  double dMinBottom = spotCentersByprofilePerSpot[ 0 ][ ( int ) spotCentersByprofilePerSpot.size() - 1 ].imag();

  for ( int iZone = 0; iZone < ( int ) spotCentersByprofilePerSpot.size() - 1; iZone++ )
  {

    if ( spotCentersByprofilePerSpot[ iZone ][ 0 ].imag() > spotCentersByprofilePerSpot[ iZone + 1 ][ 0 ].imag() )
    {
      dMaxTop = spotCentersByprofilePerSpot[ iZone ][ 0 ].imag();
      iIndexMaxTop = iZone;
    }
    if ( spotCentersByprofilePerSpot[ iZone ][ ( int ) spotCentersByprofilePerSpot[ iZone ].size() - 1 ].imag() < spotCentersByprofilePerSpot[ iZone + 1 ][ spotCentersByprofilePerSpot[ iZone + 1 ].size() - 1 ].imag() )
    {
      dMinBottom = spotCentersByprofilePerSpot[ iZone ][ ( int ) spotCentersByprofilePerSpot[ iZone ].size() - 1 ].imag();
      iIndexMinBottom = iZone;
    }
  }

  double dRefTopPointByY = spotCentersByprofilePerSpot[ iIndexMaxTop ][ 0 ].imag();
  double dRefBottomPointByY = spotCentersByprofilePerSpot[ iIndexMinBottom ][ ( int ) spotCentersByprofilePerSpot[ iIndexMinBottom ].size() - 1 ].imag();

  for ( int iZone = 0; iZone < ( int ) spotCentersByprofilePerSpot.size(); iZone++ )
  {
    int iFirstInxInZone = 0;
    int iLastInxInZone = 0;


    int iSpotCenterTop;
    int iSpotCenterBottom;
    for ( iSpotCenterTop = 0; iSpotCenterTop < ( int ) spotCentersByprofilePerSpot[ iZone ].size() - 1; iSpotCenterTop++ )
    {
      if ( abs( spotCentersByprofilePerSpot[ iZone ][ iSpotCenterTop ].imag() - dRefTopPointByY ) < 9 )
      {
        iFirstInxInZone = iSpotCenterTop;
        break;
      }
    }
    for ( iSpotCenterBottom = ( int ) spotCentersByprofilePerSpot[ iZone ].size() - 1; iSpotCenterBottom >= 0; iSpotCenterBottom-- )
    {
      if ( abs( spotCentersByprofilePerSpot[ iZone ][ iSpotCenterBottom ].imag() - dRefBottomPointByY ) < 9 )
      {
        iLastInxInZone = iSpotCenterBottom;
        break;
      }
    }

    for ( int iSpotCenter = iFirstInxInZone; iSpotCenter < iLastInxInZone; iSpotCenter++ )
    {
      spotCenters.push_back( spotCentersByprofilePerSpot[ iZone ][ iSpotCenter ] );
    }
    filteredsSpotCenters.push_back( spotCenters );
    spotCenters.clear();
  }
}

void LinesAsSpotsProcessing::FindLocalProfileMinMax( CmplxVector &localProfileMinimums/*filteredlocalProfileMin*/ , IntVector &localMinimumsIndex , CmplxVector &localProfileMaximums /*filteredlocalProfileMax*/ , IntVector &localMaximumsIndex , CmplxVector &cmplxVector , const int iIndexTostart , CString sZone , const double xMinThreshold , const double xMaxThreshold , CContainerFrame * pMarking )
{
  CArray<CString> sProfileMinimumsBufArr;//Debug
  CString sProfileMinimumsBuf = "";  //Debug
  CArray<CString> sProfileMaximumsBufArr;//Debug
  CString sProfileMaximumsBuf = "";  //Debug

  if ( cmplxVector.size() < 5 )
  {
    return;
  }
  CmplxVector ::iterator cmplxVectorIterator;


  int iIndex = 0;

  // Checking if the first point is local maximum or minimum 
  if ( cmplxVector[ 0 ].real() > cmplxVector[ 1 ].real() ) //max
  {
    localMaximumsIndex.push_back( iIndex );
    localProfileMaximums.push_back( cmplxVector[ 0 ] );
    //pMarking->AddFrame(CreatePtFrame(cmplx(cmplxVector[0].real(), cmplxVector[0].imag()), "color=0xff66ff;Sz=3;"));

    sProfileMaximumsBuf.Format( "%6.2f , %6.2f\n" , cmplxVector[ 0 ].real() , cmplxVector[ 0 ].imag() );//Debug
    sProfileMaximumsBufArr.Add( sProfileMaximumsBuf );//Debug
  }
  else if ( cmplxVector[ 0 ].real() < cmplxVector[ 1 ].real() ) //min
  {

    localProfileMinimums.push_back( cmplxVector[ 0 ] );
    localMinimumsIndex.push_back( iIndex );

    //pMarking->AddFrame(CreatePtFrame(cmplx(cmplxVector[0].real(), cmplxVector[0].imag()), "color=0xffffff;Sz=3;")); //Debug
    //sProfileMinimumsBuf.Format("%6.2f , %6.2f\n", cmplxVector[0].real(), cmplxVector[0].imag());//Debug
    //sProfileMinimumsBufArr.Add(sProfileMaximumsBuf);//Debug
  }

  for ( cmplxVectorIterator = cmplxVector.begin() + 1; cmplxVectorIterator < cmplxVector.end() - 1; cmplxVectorIterator++ , iIndex++ ) // Profile pixels coordinates     
  {

     //FindLocal minimum
    if ( ( ( cmplxVectorIterator - 1 )->real() ) > cmplxVectorIterator->real() && ( ( cmplxVectorIterator + 1 )->real() > cmplxVectorIterator->real() ) )
    {
      //if ((cmplxVectorIterator)->real() < xMinThreshold)
      {
        localMinimumsIndex.push_back( iIndex );
        localProfileMinimums.push_back( cmplx( cmplxVectorIterator->real() , cmplxVectorIterator->imag() ) );

        //pMarking->AddFrame(CreatePtFrame(cmplx(cmplxVectorIterator->real(), cmplxVectorIterator->imag()), "color=0xffffff;Sz=3;")); //Debug
        //sProfileMinimumsBuf.Format("%6.2f , %6.2f\n", cmplxVectorIterator->real(), cmplxVectorIterator->imag()); //Debug
        //sProfileMinimumsBufArr.Add(sProfileMinimumsBuf); //Debug
      }
    }
    // For plateaus case        
    else if ( ( cmplxVectorIterator - 1 )->real() > ( cmplxVectorIterator->real() ) && ( ( cmplxVectorIterator + 1 )->real() == cmplxVectorIterator->real() ) )
    {
       // if ((cmplxVectorIterator)->real() < xMinThreshold)
      {
        localMinimumsIndex.push_back( iIndex );
        localProfileMinimums.push_back( cmplx( cmplxVectorIterator->real() , cmplxVectorIterator->imag() ) );

        //pMarking->AddFrame(CreatePtFrame(cmplx(cmplxVectorIterator->real(), cmplxVectorIterator->imag()), "color=0xffffff;Sz=3;"));
        //sProfileMinimumsBuf.Format("%6.2f , %6.2f\n", cmplxVectorIterator->real(), cmplxVectorIterator->imag()); //Debug
        //sProfileMinimumsBufArr.Add(sProfileMinimumsBuf); //Debug
      }

    }
    //FindLocal maximum
    if ( ( ( cmplxVectorIterator - 1 )->real() ) < cmplxVectorIterator->real() && ( ( cmplxVectorIterator + 1 )->real() < cmplxVectorIterator->real() ) )
    {
      // if ((cmplxVectorIterator)->real() > xMaxThreshold)
      {
        localMaximumsIndex.push_back( iIndex );
        localProfileMaximums.push_back( cmplx( cmplxVectorIterator->real() , cmplxVectorIterator->imag() ) );

        //pMarking->AddFrame(CreatePtFrame(cmplx(cmplxVectorIterator->real(), cmplxVectorIterator->imag()), "color=0xff66ff;Sz=3;")); //Debug
        //sProfileMaximumsBuf.Format("%6.2f , %6.2f\n", cmplxVectorIterator->real(), cmplxVectorIterator->imag()); //Debug
        //sProfileMaximumsBufArr.Add(sProfileMaximumsBuf); //Debug
      }
    }
    // For plateaus case        
    else if ( ( cmplxVectorIterator - 1 )->real() < ( cmplxVectorIterator->real() ) && ( ( cmplxVectorIterator + 1 )->real() == cmplxVectorIterator->real() ) )
    {
      // if ((cmplxVectorIterator)->real() > xMaxThreshold)
      {
        localMaximumsIndex.push_back( iIndex );
        localProfileMaximums.push_back( cmplx( cmplxVectorIterator->real() , cmplxVectorIterator->imag() ) );

        //pMarking->AddFrame(CreatePtFrame(cmplx(cmplxVectorIterator->real(), cmplxVectorIterator->imag()), "color=0xff66ff;Sz=3;"));//Debug
        //sProfileMaximumsBuf.Format("%6.2f , %6.2f\n", cmplxVectorIterator->real(), cmplxVectorIterator->imag());//Debug
        //sProfileMaximumsBufArr.Add(sProfileMaximumsBuf);//Debug
      }
    }
  }
 /* // Checking if the last point is local maximum or minimum
  if (cmplxVector[cmplxVector.size() - 1].real() > cmplxVector[cmplxVector.size() - 2].real()) // max
  {
    localMaximumsIndex.push_back(iIndex);
    localProfileMaximums.push_back(cmplxVector[cmplxVector.size() - 1]);
    pMarking->AddFrame(CreatePtFrame(cmplx(cmplxVector[cmplxVector.size() - 1].real(), cmplxVector[cmplxVector.size() - 1].imag()), "color=0xff66ff;Sz=3;"));
  }
  else if (cmplxVector[cmplxVector.size() - 1].real() < cmplxVector[cmplxVector.size() - 2].real()) // min
  {

    localProfileMinimums.push_back(cmplxVector[cmplxVector.size() - 1]);
    localMinimumsIndex.push_back(iIndex);
    pMarking->AddFrame(CreatePtFrame(cmplx(cmplxVector[cmplxVector.size() - 1].real(), cmplxVector[cmplxVector.size() - 1].imag()), "color=0x000080;Sz=3;"));
  }*/

   //WriteToFile(sProfileMinimumsBufArr, sZone, "MinimumsDataProfile");//Debug
  sProfileMinimumsBufArr.RemoveAll();//Debug
  //WriteToFile(sProfileMaximumsBufArr, sZone, "MaximumsDataProfile");//Debug
  sProfileMaximumsBufArr.RemoveAll();//Debug


 /*if (localProfileMaximums.size() && localProfileMinimums.size())
   FilterProfileMinMax(localProfileMaximums, localProfileMinimums, filteredlocalProfileMax, filteredlocalProfileMin, sZone);*/// Removed filter for now 17.11.22


}// end FindLocalProfileMinMax FUNCTION 

void LinesAsSpotsProcessing::FindProfileCrossPoints(
  CmplxVector& Rises , IntVector& RiseIndexes ,
  CmplxVector& Falls , IntVector& FallIndexes ,
  CmplxVector &ProfileData , CString sZone ,
  CContainerFrame * pMarking )
{
  if ( ProfileData.size() < 5 )
    return;

  FXString AsText , Addition ;
  double dMin = DBL_MAX , dMax = -DBL_MAX ;
  int iMaxIndex = 0 , iMinIndex = 0 ;
  double dLocalMin = DBL_MAX , dLocalMax = -DBL_MAX ;
  int iLocalMaxIndex = 0 , iLocalMinIndex = 0 ;
  cmplx * pProf = ProfileData.data() ;
  cmplx * pProfOrig = pProf ;
  size_t iLen = ProfileData.size() ;
  cmplx * pEndScan = pProf + iLen - 2 ;
  int iToPlusCnt = 0 , iToMinusCnt = 0 ;
  int iFrontThreshold = 5 ;
  bool bPlusReady = false , bMinusReady = false ;
  double dVal = pProf->real() ;
  double dThres = 0. ;
  while ( ++pProf < pEndScan )
  {
    double dNewVal = pProf->real() ;
    if ( dNewVal > dVal )
    {
      if ( ( ++iToPlusCnt > iFrontThreshold ) && ( ( dNewVal - dMin ) > 15 ) ) // the full profile amplitude is 100
      {                                                                  // Profile is normalized to 100
        if ( !bPlusReady )
        {
          if ( bMinusReady )
          {
            dLocalMin = dMin ;
            iLocalMinIndex = iMinIndex ;
            bMinusReady = false ;

            if ( dLocalMax != -DBL_MAX )
            {
              dThres = 0.5 * ( dLocalMin + dLocalMax ) ;
              cmplx * pPt = ProfileData.data() + iLocalMaxIndex ;
              if ( pMarking )
              {
                pMarking->AddFrame( CreatePtFrame(
                  IsVert() ?
                  cmplx( pPt->imag() , pPt->real() ) : *pPt , "color=0xff00ff;Sz=1;" ) );
              }
              while ( ( ++pPt )->real() >= dThres )
              {
                if ( pPt >= pEndScan - 1 )
                  break ;
              };

              if ( pPt >= pEndScan - 1 )
                break ; // from main loop in profile

              pPt-- ;
              double dPos = dThres ;
              double dPosY = pPt->imag() + GetThresPosition( pPt->real() , ( pPt + 1 )->real() , dThres ) ;

              Falls.push_back( cmplx( dPos , dPosY ) ) ;
              FallIndexes.push_back( (int)(pProf - ProfileData.data()) ) ;

              Addition.Format( "Fall=%.2f Min=%.2f Max=%.2f Thres=%.2f Before=%.2f After=%.2f\n" ,
                dPos , dLocalMin , dLocalMax , dThres , pPt->real() , ( pPt + 1 )->real() ) ;
              AsText += Addition ;
              if ( pMarking )
              {
                pMarking->AddFrame( CreatePtFrame(
                  IsVert() ? cmplx( dPosY , dPos ) : cmplx( dPos , dPosY ) ,
                  "color=0x00ff00;Sz=1;" ) );
              }
            }
          }
          bPlusReady = true ;
        }
      }
      iMaxIndex = (int)(pProf - ProfileData.data()) ;
      dMax = dNewVal ;
      iToMinusCnt = 0 ;
    }
    else if ( dNewVal < dVal )
    {
      if ( ( ++iToMinusCnt > iFrontThreshold ) && ( ( dMax - dNewVal ) > 15 ) )// the full profile amplitude is 100
      {                                                                        // Profile is normalized to 100
        if ( !bMinusReady )
        {
          if ( bPlusReady )
          {
            dLocalMax = dMax ;
            iLocalMaxIndex = iMaxIndex ;
            bPlusReady = false ;

            if ( dLocalMin != DBL_MAX )
            {
              dThres = 0.5 * ( dLocalMin + dLocalMax ) ;
              cmplx * pPt = ProfileData.data() + iLocalMinIndex ;
              if ( pMarking )
              {
                pMarking->AddFrame( CreatePtFrame(
                  IsVert() ?
                  cmplx( pPt->imag() , pPt->real() ) : *pPt , "color=0xff00ff;Sz=1;" ) );
              }
              while ( ( ++pPt )->real() < dThres )
              {
                if ( pPt >= pEndScan - 1 )
                  break ;
              }

              if ( pPt >= pEndScan - 1 )
                break ; // from main loop in profile

              pPt-- ;
              double dPos = dThres ;
              double dPosY = pPt->imag() + GetThresPosition( pPt->real() , ( pPt + 1 )->real() , dThres ) ;
              Rises.push_back( cmplx( dPos , dPosY ) ) ;
              RiseIndexes.push_back( (int)(pProf - ProfileData.data()) ) ;

              Addition.Format( "Fall=%.2f Min=%.2f Max=%.2f Thres=%.2f Before=%.2f After=%.2f\n" ,
                dPosY , dLocalMin , dLocalMax , dThres , pPt->real() , ( pPt + 1 )->real() ) ;
              AsText += Addition ;
              if ( pMarking )
              {
                pMarking->AddFrame( CreatePtFrame(
                  IsVert() ? cmplx( dPosY , dPos ) : cmplx( dPos , dPosY ) ,
                  "color=0x00ffff;Sz=1;" ) );
              }
            }
          }
          bMinusReady = true ;
        }
      }
      iMinIndex = (int)(pProf - ProfileData.data()) ;
      dMin = dNewVal ;
      iToPlusCnt = 0 ;
    }
    dVal = dNewVal ;
  }

//   double dThres = 0.5 * ( dMin + dMax ) ;
// 
//   pProf = ProfileData.data() ;
//   pEndScan = pProf + iLen - 1 ;
//   while ( pProf < pEndScan )
//   {
//     BOOL bBigger = pProf->real() >= dThres ;
//     BOOL bNextBigger = ( pProf + 1 )->real() >= dThres ;
//     if ( bBigger ^ bNextBigger ) 
//     {
//       double dPos = pProf->imag() + GetThresPosition( pProf->real() , ( pProf + 1 )->real() , dThres ) ;
//       if ( bBigger ) // fall front
//       {
//         if ( Falls.size() && Rises.size() )
//           dThres = 0.5 * ( dMin + dMax ) ;
// 
//         Falls.push_back( dPos ) ;
//         FallIndexes.push_back( pProf - ProfileData.data() ) ;
//         
// 
//         Addition.Format( "Fall=%.2f Min=%.2f Max=%.2f Thres=%.2f Before=%.2f After=%.2f\n" ,
//           dPos , dMin , dMax , dThres , pProf->real() , ( pProf + 1 )->real() ) ;
//         AsText += Addition ;
//         if ( pMarking )
//         {
//           pMarking->AddFrame( CreatePtFrame(
//             IsVert() ? cmplx( dPos , pProf->real() ) : cmplx( pProf->real() , dPos ) ,
//             "color=0x0000ff;Sz=2;" ) );
//         }
// 
//         dMin = DBL_MAX ;
//         pProf += 2 ;
//       }
//       else // rise front
//       {
//         if ( Falls.size() && Rises.size() )
//           dThres = 0.5 * ( dMin + dMax ) ;
// 
//         Rises.push_back( dPos ) ;
//         RiseIndexes.push_back( pProf - ProfileData.data() ) ;
//         Addition.Format( "Rise=%.2f Min=%.2f Max=%.2f Thres=%.2f Before=%.2f After=%.2f\n" ,
//           dPos , dMin , dMax , dThres , pProf->real() , ( pProf + 1 )->real() ) ;
//         AsText += Addition ;
//         if ( pMarking )
//         {
//           pMarking->AddFrame( CreatePtFrame(
//             IsVert() ? cmplx( dPos , pProf->real() ) : cmplx( pProf->real() , dPos ) ,
//             "color=0x00ff00;Sz=2;" ) );
//         }
// 
//         dMax = -DBL_MAX ;
//         pProf += 2 ;
//       }
//     }
//     SetMinMax( pProf->real() , dMin , dMax ) ;
//     ++pProf ;
//   } ;
// 
  //WriteToFile(sProfileMinimumsBufArr, sZone, "MinimumsDataProfile");//Debug
  //WriteToFile(sProfileMaximumsBufArr, sZone, "MaximumsDataProfile");//Debug
}// end FindProfileCrossPoints FUNCTION  

int LinesAsSpotsProcessing::MovingAverageProfile( CmplxVector &Profile , CmplxVector &Results , CContainerFrame * pMarking )
{
  double average = 0.0;
  double sum = 0.0;

  FXRegistry Reg( "TheFileX\\SHStudio" );
  int iSlidingWindow = Reg.GetRegiInt(
    "SpotStatistics" , "Sliding Window For Profile" , 4 );

  if ( iSlidingWindow != 0 )
  {
    for ( int iPixel = 0 , left_pt = 0 , right_pt = iSlidingWindow - 1; right_pt < ( int ) Profile.size() - 1; iPixel++ , right_pt++ , left_pt++ )
    {
      int left = left_pt;
      while ( left <= right_pt )
      {
        sum += Profile[ left ].real();
        left++;
      }
      average = sum / iSlidingWindow;
      Results.push_back( cmplx( average , Profile[ left ].imag() ) );
      //pMarking->AddFrame(CreatePtFrame(cmplx(average, Profile[left].imag()), "color=0xff77ff;Sz=1;")); //Debug
     // pMarking->AddFrame(CreatePtFrame(cmplx(dProfileCenterX, dProfileCenterY), "color=0xffff00;Sz=1;")); //Debug

      if ( ( iPixel > 0 ) && ( abs( Results[ iPixel ].real() - Results[ iPixel - 1 ].real() ) > 15 ) ) { return left; } // 15 - move to registry?

      sum = 0.0;
      average = 0.0;
    }
  }
  return -1;

}// end MovingAverageProfile FUNCTION

void LinesAsSpotsProcessing::FilterProfileMinMax( CmplxVector &localProfileMaximums , CmplxVector &localProfileMinimums , CmplxVector &filteredlocalProfileMax , CmplxVector &filteredlocalProfileMin , CString sZone )
{
  CArray<CString> sProfileMinimumsBufArr;//Debug
  CString sProfileMinimumsBuf = "";  //Debug
  CArray<CString> sProfileMaximumsBufArr;//Debug
  CString sProfileMaximumsBuf = "";  //Debug



  CmplxVector ::iterator cmplxVectorIterator;

  for ( cmplxVectorIterator = localProfileMaximums.begin() + 1; cmplxVectorIterator < localProfileMaximums.end() - 1; cmplxVectorIterator++ ) // Profile pixels coordinates     
  {
    if ( abs( cmplxVectorIterator->imag() - ( cmplxVectorIterator - 1 )->imag() ) > MIN_DISY_FOR_MINMAX )
    {
      filteredlocalProfileMax.push_back( cmplx( cmplxVectorIterator->real() , cmplxVectorIterator->imag() ) );
      sProfileMaximumsBuf.Format( "%6.2f , %6.2f\n" , cmplxVectorIterator->real() , cmplxVectorIterator->imag() ); //Debug
      sProfileMaximumsBufArr.Add( sProfileMaximumsBuf ); //Debug
    }
  }
  for ( cmplxVectorIterator = localProfileMinimums.begin() + 1; cmplxVectorIterator < localProfileMinimums.end() - 1; cmplxVectorIterator++ ) // Profile pixels coordinates     
  {
    if ( abs( cmplxVectorIterator->imag() - ( cmplxVectorIterator - 1 )->imag() ) > MIN_DISY_FOR_MINMAX )
    {
      filteredlocalProfileMin.push_back( cmplx( cmplxVectorIterator->real() , cmplxVectorIterator->imag() ) );
      sProfileMinimumsBuf.Format( "%6.2f , %6.2f\n" , cmplxVectorIterator->real() , cmplxVectorIterator->imag() ); //Debug
      sProfileMinimumsBufArr.Add( sProfileMinimumsBuf ); //Debug
    }
  }



  //WriteToFile(sProfileMinimumsBufArr, sZone, "FilteredMinProfile");//Debug
  sProfileMinimumsBufArr.RemoveAll();//Debug
  //WriteToFile(sProfileMaximumsBufArr, sZone, "FilteredMaxProfile");//Debug
  sProfileMaximumsBufArr.RemoveAll();//Debug

}// end FilterMinMaxProfile FUNCTION 

 ////************** end iStatisticsOnlyByProfile == 1*************////


 ////*************** iStatisticsOnlyByProfile == 0**************////
// FXString LinesAsSpotsProcessing::RegressionForLinesAsSpots( 
//   const SpotVectors &WithMissingSpots , SpotItemVecs &WithMissingSpotsCenter )
// {
//   FXString      Result = ""; // Sum of average angles of all the regressions - Per Line
//   CFRegression RegrUpper;
//   CFRegression RegrLower;
//   CFRegression CentersRegr;
// 
//   vector<CFRegression> RegrUpperPerSpot;
//   vector<CFRegression> RegrLowerPerSpot;
// 
//   vector<vector<CFRegression>> RegrUpperVectorPerZone;
//   vector<vector<CFRegression>> RegrLowerVectorPerZone;
// 
//   vector<CFRegression> CentersRegrPerSpot;
//   vector<vector<CFRegression>> CentersRegrVectorPerZone;
// 
//   //CFRegression *RowsCenterReg = NULL;
// 
//   CFRegression RowsCenterReg[ 150 ];
// 
// 
//   CList<SpotRegressionItem> spotRegression;
//   CList<SpotRegressionItem > *pSpotRegressionList = &spotRegression;
//   CList<CList<SpotRegressionItem >*> spotRegressionsZone;
// 
// 
//   // for (theSpotsVectorIterator = sSeparatedResults.begin(); theSpotsVectorIterator != sSeparatedResults.end(); theSpotsVectorIterator++) // Zones = Spots Vector loop 
//   for ( int iZone = 0; iZone < ( int ) WithMissingSpotsCenter.size(); iZone++ )
//   {
//     ///*CFRegression **/ RowsCenterReg = new CFRegression[(int)m_WithMissingSpotsCenter[iZone].size()];
// 
//     // for (colorSpotIterator = theSpotsVectorIterator->begin(); colorSpotIterator < theSpotsVectorIterator->end(); colorSpotIterator++) // Spots = ColorSpot in Spots Vector loop
//     const SpotItemVec& ForZone = WithMissingSpotsCenter.at( iZone ) ;
// 
//     for ( size_t iSpot = 0; iSpot < ForZone.size(); iSpot++ )
//     {
//       double dSum = 0.0;
//       double dAvr = 0.0;
//       double dMin = 100000 , dMax = -100000;
//       if ( !WithMissingSpotsCenter[ iZone ][ iSpot ].Added )
//       {
//         RowsCenterReg[ iSpot ].Add( WithMissingSpotsCenter[ iZone ][ iSpot ].Center );
// 
//         CentersRegr.Add( ForZone[ iSpot ].Center );
// 
//         FindMinMax_X( ForZone[ iSpot ].Contur , dMin , dMax ); //X of the left and right parts of the contur
//           // Spots' Contur loop   
//         for ( int iCountorPoint = 0; iCountorPoint < ForZone[ iSpot ].Contur.size(); iCountorPoint++ )   
//         {
//           {   // to not add left and right parts of the contur to the regression
//             if ( ( ForZone[ iSpot ].Contur[ iCountorPoint ].real() > ( dMin + 1 ) ) 
//               && ( ForZone[ iSpot ].Contur[ iCountorPoint ].real() < ( dMax - 1 ) ) ) // 
//             {
//               if ( ForZone[ iSpot ].Center.imag() < ForZone[ iSpot ].Contur[ iCountorPoint ].imag() )
//               {
//                 RegrUpper.Add( ForZone[ iSpot ].Contur[ iCountorPoint ] );
//               }
//               if ( ForZone[ iSpot ].Center.imag() > ForZone[ iSpot ].Contur[ iCountorPoint ].imag() )
//               {
//                 RegrLower.Add( ForZone[ iSpot ].Contur[ iCountorPoint ] );
//               }
//             }
//           }
//         }
//         RegrUpperPerSpot.push_back( RegrUpper );
//         RegrLowerPerSpot.push_back( RegrLower );
//         RegrUpper.Reset();
//         RegrLower.Reset();
//       }
//     }
//     RegrUpperVectorPerZone.push_back( RegrUpperPerSpot );
//     RegrLowerVectorPerZone.push_back( RegrLowerPerSpot );
// 
//     RegrUpperPerSpot.clear();
//     RegrLowerPerSpot.clear();
//   }
//   Result += m_LineAsSpotsStatistics.GetCaption();
//   FindAnglePerRow( RegrUpperVectorPerZone , RegrLowerVectorPerZone , RowsCenterReg , Result );
//   RegrLowerVectorPerZone.clear();
//   RegrUpperVectorPerZone.clear();
// 
//   return Result;
// }// end RegressionForLinesAsSpots FUNCTION

FXString LinesAsSpotsProcessing::RegressionForLinesAsSpots(
  const SpotVectors &WithMissingSpots )
{
  FXString Result ; 
  CFRegression RegrUpper;
  CFRegression RegrLower;
  CFRegression CentersRegr;

  RegrVector RegrUpperPerSpot;
  RegrVector RegrLowerPerSpot;

  RegrVectors RegrUpperVectorPerZone;
  RegrVectors RegrLowerVectorPerZone;

  RegrVector CentersRegrPerSpot;
  RegrVectors CentersRegrVectorPerZone;

  //CFRegression *RowsCenterReg = NULL;

  CFRegression RowsCenterReg[ 150 ];


  CList<SpotRegressionItem> spotRegression;
  CList<SpotRegressionItem > *pSpotRegressionList = &spotRegression;
  CList<CList<SpotRegressionItem >*> spotRegressionsZone;


  // for (theSpotsVectorIterator = sSeparatedResults.begin(); theSpotsVectorIterator != sSeparatedResults.end(); theSpotsVectorIterator++) // Zones = Spots Vector loop 
  for ( size_t iZone = 0; iZone < ( int ) WithMissingSpots.size(); iZone++ )
  {
    ///*CFRegression **/ RowsCenterReg = new CFRegression[(int)m_WithMissingSpotsCenter[iZone].size()];

    // for (colorSpotIterator = theSpotsVectorIterator->begin(); colorSpotIterator < theSpotsVectorIterator->end(); colorSpotIterator++) // Spots = ColorSpot in Spots Vector loop
    const SpotVector& ForZone = WithMissingSpots.at( iZone ) ;
    CFStatistics& ProfileStat = m_ProfileCentersStatistics.at( iZone ) ;
    cmplx cFirstPointOnProfile = m_ProfileCenters.at(iZone).at( 0 ) ;
    for ( size_t iSpot = 0; iSpot < ForZone.size(); iSpot++ )
    {
      double dSum = 0.0;
      double dAvr = 0.0;
      //double dMin = 100000 , dMax = -100000;

      const CColorSpot& Spot = ForZone.at( iSpot ) ;
      if ( Spot.m_Contur.size() )
      {
        CmplxVector UpperSide , LowerSide ;
        cmplx cCenter = CDPointToCmplx( Spot.m_SimpleCenter ) ;
        RowsCenterReg[ iSpot ].Add( cCenter );

        CentersRegr.Add( cCenter );
        
        CmplxArray Extrems ;
        cmplx cSize ;
        cmplx CenterByExtrems = FindExtrems( Spot.m_Contur , Extrems ,
          NULL , &cSize ) ;
        
        double dMinX = Extrems[ EXTREME_INDEX_LEFT ].real() ;
        double dMaxX = Extrems[ EXTREME_INDEX_RIGHT ].real() ;
        double dMinY = Extrems[ EXTREME_INDEX_TOP ].imag() ;
        double dMaxY = Extrems[ EXTREME_INDEX_BOTTOM ].imag() ;
        if ( m_bVerticalLines )
        {
          swap(dMinX, dMinY);
          swap(dMaxX, dMaxY);
        }
        //FindMinMax_X( Spot.m_Contur , dMin , dMax ); //X of the left and right parts of the contur
          // Spots' Contur loop   
        for ( int iCountorPoint = 0; iCountorPoint < Spot.m_Contur.size(); iCountorPoint++ )
        {
          {   // to not add left and right parts of the contur to the regression
            cmplx cPt = Spot.m_Contur[ iCountorPoint ] ;
            if (m_bVerticalLines)
              SwapReIm(cPt);
            if ( ( cPt.real() > ( dMinX + 1. ) ) && ( cPt.real() < ( dMaxX - 1. ) ) ) // 
            {
              if ( cCenter.imag() > cPt.imag() )
              {
                RegrUpper.Add( cPt );
                UpperSide.push_back( cPt ) ;
              }
              else
              {
                RegrLower.Add( cPt ) ;
                LowerSide.push_back( cPt ) ;
              }
            }
          }
        }
        RegrUpper.Calculate() ;
        RegrLower.Calculate() ;

        double dR2 = RegrUpper.GetRSquared();
        double dR2_2 = RegrUpper.GetStdFromLine(UpperSide.data(), (int)UpperSide.size());
        m_LeadRaggedness_um.Add( dR2_2 * m_dScale_unit_per_pixel ) ;
        dR2 = RegrLower.GetRSquared();
        dR2_2 = RegrLower.GetStdFromLine(LowerSide.data(), (int)LowerSide.size()) ;
        m_TrailRaggedness_um.Add( dR2_2 * m_dScale_unit_per_pixel ) ;
        double dUpper = RegrUpper.GetY( Spot.m_SimpleCenter.x ) ;
        double dLower = RegrLower.GetY( Spot.m_SimpleCenter.x ) ;
        double dWidth_um = fabs( dLower - dUpper ) * m_dScale_unit_per_pixel ;
        m_Width_um.Add( dWidth_um );
        double dAngle = RegrUpper.GetAngle();
        dAngle = NormAngleFromM45toP135(dAngle);
        dAngle = RadToDeg(dAngle);
        m_LeadAngle_deg.Add( dAngle );
        dAngle = RegrLower.GetAngle();
        dAngle = NormAngleFromM45toP135(dAngle);
        dAngle = RadToDeg(dAngle); 
        m_TrailAngle_deg.Add( dAngle );

        m_BoundLeft_um.Add(( - dMinX + /*Spot.m_SimpleCenter.x*/cCenter.real())* m_dScale_unit_per_pixel ) ;
        m_BoundRight_um.Add( (dMaxX -/*Spot.m_SimpleCenter.x*/cCenter.real())* m_dScale_unit_per_pixel) ;
        m_BoundTop_um.Add(( - dMinY + /*Spot.m_SimpleCenter.x*/cCenter.imag())* m_dScale_unit_per_pixel) ;
        m_BoundBottom_um.Add(( dMaxY -  /*Spot.m_SimpleCenter.x*/cCenter.imag())* m_dScale_unit_per_pixel) ;
        m_BoundHeight_um.Add( cSize.imag() * m_dScale_unit_per_pixel) ;

        RegrUpperPerSpot.push_back( RegrUpper );
        RegrLowerPerSpot.push_back( RegrLower );
        RegrUpper.Reset();
        RegrLower.Reset();
      }
      m_Xmid_um.Add( Spot.m_OuterFrame.CenterPoint().x ) ;
      m_Ymid_um.Add(cFirstPointOnProfile.imag() - ProfileStat.GetAverage() * iSpot ) ;
//       if ( iSpot != 0 )
//         m_Dist_um.Add( Spot.m_SimpleCenter.y - ForZone.at( iSpot - 1 ).m_SimpleCenter.y ) ;

      RegrUpperVectorPerZone.push_back( RegrUpperPerSpot );
      RegrLowerVectorPerZone.push_back( RegrLowerPerSpot );

      RegrUpperPerSpot.clear();
      RegrLowerPerSpot.clear();
    }

    m_Dist_um.Add( m_ProfileCentersStatistics[ iZone ].GetAverage() * m_dScale_unit_per_pixel ) ;

   //Result += m_LineAsSpotsStatistics.GetCaption();
//     FindAnglePerRow( RegrUpperVectorPerZone , RegrLowerVectorPerZone , RowsCenterReg , Result );

    RegrLowerVectorPerZone.clear();
    RegrUpperVectorPerZone.clear();
  }
  m_LeadRaggedness_um.Calculate() ;
  m_TrailRaggedness_um.Calculate() ;
  m_Width_um.Calculate() ;
//   m_MinWidth_um.Reset() ;
//   m_MaxWidth_um.Reset() ;
//   m_StdWidth_um.Reset() ;
  m_LeadAngle_deg.Calculate() ;
  m_TrailAngle_deg.Calculate() ;
  m_BoundLeft_um.Calculate() ;
  m_BoundRight_um.Calculate() ;
  m_BoundTop_um.Calculate() ;
  m_BoundBottom_um.Calculate() ;
  m_BoundHeight_um.Calculate() ;
  m_Xmid_um.Calculate() ;
  m_Ymid_um.Calculate() ;
  m_Dist_um.Calculate() ;


  Result = GetAsString_Av_Std_Min75_Max75_Min95_Max95(
    m_LeadRaggedness_um , m_sOutDoubleFormat ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_TrailRaggedness_um , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_LeadAngle_deg , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_TrailAngle_deg , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_Width_um , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_BoundLeft_um , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_BoundRight_um , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_BoundTop_um , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_BoundBottom_um , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_BoundHeight_um , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95(
    m_Xmid_um , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_Ymid_um , m_sOutDoubleFormat, "," ) ;
  Result += GetAsString_Av_Std_Min75_Max75_Min95_Max95( 
    m_Dist_um , m_sOutDoubleFormat, "," ) ;

  return Result;
}// end RegressionForLinesAsSpots FUNCTION

void LinesAsSpotsProcessing::FindAnglePerRow( vector<vector<CFRegression>> &RegrUpperVectorPerZone , vector<vector<CFRegression>> &RegrLowerVectorPerZone , /*vector<CFRegression > RowsCenterReg*/CFRegression *RowsCenterReg , FXString &resAsString )
{
  vector<SpotRegressionItem> spotRegression;
  vector<SpotRegressionItem > *pSpotRegressionList = &spotRegression;
  vector<vector<SpotRegressionItem>> spotRegressionsZone;

  vector<double> sLowerAngles; //debug
  vector<double> sUpperAngles; //debug
  vector<vector<double>> sLowerAnglesPerZone; //debug
  vector<vector<double>> sUpperAnglesPerZone; //debug


  /*CList<SpotRegressionItem> spotRegression;
  CList<SpotRegressionItem > *pSpotRegressionList = &spotRegression;
  CList<CList<SpotRegressionItem>> spotRegressionsZone;*/

  SpotStatisticsItem spotStatisticsItem;
  SpotRegressionItem regItem;
  SpotStatisticsItem rowsLowerAngles[ 150 ] = { 0 };
  SpotStatisticsItem rowsUpperAngles[ 150 ] = { 0 };

  for ( size_t iZone = 0; iZone < RegrUpperVectorPerZone.size(); iZone++ )
  {
    vector<CFRegression>& UpRegr = RegrUpperVectorPerZone.at( iZone ) ;
    vector<CFRegression>& LowRegr = RegrLowerVectorPerZone.at( iZone ) ;
    for ( size_t iSpot = 0; iSpot < UpRegr.size(); iSpot++ )
    {
      regItem.UpperAngle = CalculateAngle( UpRegr[ iSpot ] );
      regItem.LowerAngle = CalculateAngle( LowRegr[ iSpot ] );

      if ( LowRegr[ iSpot ].m_dSumX != 0 ) // if added point need to reduce it from num of samples
      {
        rowsLowerAngles[ iSpot ].Sum += abs( regItem.LowerAngle );
        sLowerAngles.push_back( regItem.LowerAngle );
        rowsUpperAngles[ iSpot ].Sum += abs( regItem.UpperAngle );
        sUpperAngles.push_back( regItem.UpperAngle );
        rowsLowerAngles[ iSpot ].iSamples++;
        rowsUpperAngles[ iSpot ].iSamples++;
      }

      regItem.Zone = (int)iZone;
      regItem.index = (int)iSpot;
      spotRegression.push_back( regItem );
      //spotRegression.AddTail(regItem);
    }
    spotRegressionsZone.push_back( spotRegression );
    //spotRegressionsZone.AddTail(spotRegression);  
    spotRegression.clear();
    //spotRegression.RemoveAll();   
  }

  int iZoneSizes = 0;
  if ( ( int ) spotRegressionsZone.size() > 0 )
  {
    iZoneSizes = ( int ) spotRegressionsZone[ 0 ].size();
  }

  for ( int iAngleSum = 0; iAngleSum < ( int ) spotRegressionsZone[ 0 ].size(); iAngleSum++ ) //ROWS
  {
    rowsLowerAngles[ iAngleSum ].Avr = rowsLowerAngles[ iAngleSum ].Sum / rowsLowerAngles[ iAngleSum ].iSamples;
    rowsUpperAngles[ iAngleSum ].Avr = rowsUpperAngles[ iAngleSum ].Sum / rowsUpperAngles[ iAngleSum ].iSamples;

    double dCenterAngle = 0.0;
    if ( RowsCenterReg != NULL )
    {
      dCenterAngle = CalculateAngle( RowsCenterReg[ iAngleSum ] );
    }

    //Results Format: rowNum, upperAvr, LowerAvr, CenterAngle
    FXString resultRow = m_LineAsSpotsStatistics.ToString( iAngleSum , rowsUpperAngles[ iAngleSum ].Avr , rowsLowerAngles[ iAngleSum ].Avr , dCenterAngle );
    resAsString += resultRow;
  }
  spotRegressionsZone.clear();
}// end FindAnglePerRow FUNCTION

void LinesAsSpotsProcessing::FindProfileCenter( const SpotVectors& sSeparatedResults , 
  NamedCmplxVectors& Profiles , CmplxVectors &profileCentersPerSpot , CContainerFrame * pMarking )
{
  FXRegistry Reg( "TheFileX\\SHStudio" );
  double dProfileThreshold = Reg.GetRegiDouble(
    "SpotStatistics" , "LinesAsSpotsProfileThreshold" , 0.5 );

  CmplxVector ::iterator cmplxVectorIterator;   // All profiles' pixels per zone

  CmplxVector localProfileMinimums;
  IntVector localMinimumsIndex;
  CmplxVectors LocalMinimumsPerSpot;
  IntVectors LocalMinimumsPerSpotIndex;

  CmplxVector localProfileMaximums;
  IntVector localMaximumsIndex;
  CmplxVectors LocalMaximumsPerSpot;
  IntVectors LocalMaximumsPerSpotIndex;
  CmplxVector profileCenters;


  /*CArray<CString> sProfileBufArr;//Debug
  CArray<CString> sProfileBufArr1;//Debug
  CString sProfileBuf = "";  //Debug
  CString sProfileBuf1 = "";  //Debug*/
  CString sZone; //Debug

  if ( (Profiles.size() != (sSeparatedResults.size() - 1)) || Profiles.size() < 5 )
    return;
  int iZoneWithMaxSpots = 0;
  int iMax = -10000;
  for ( size_t ZonesCount = 1; ZonesCount < sSeparatedResults.size(); ZonesCount++ )
  {
    sZone.Format( "Zone = %d," , ZonesCount );//Debug
    const SpotVector& ZoneSpots = sSeparatedResults.at( ZonesCount ) ;

    int iIndexSpotsStart; //start from
    for ( iIndexSpotsStart = 0 , cmplxVectorIterator = Profiles[ ZonesCount - 1 ].m_Data.begin() + 1; 
      cmplxVectorIterator < Profiles[ ZonesCount - 1 ].m_Data.end() - 1; 
      cmplxVectorIterator++ , iIndexSpotsStart++ ) // Profile pixels coordinates     
    {
      //sProfileBuf.Format("%6.2f , %6.2f\n", cmplxVectorIterator->real(), cmplxVectorIterator->imag());//Debug
      //sProfileBufArr.Add(sProfileBuf);//Debug
      if ( ( cmplxVectorIterator - 1 )->real() - ( cmplxVectorIterator )->real() > 5 ) //Find The "Drop" point Before the spots profiles
      {
        //WriteToFile(sProfileBufArr, sZone, "RawDataProfileBeforeDrop");//Debug
        //sProfileBufArr.RemoveAll();//Debug
        break;
      }
    }

    double dXMinThreshold = ZoneSpots.at( 0 ).m_SimpleCenter.x + X_MIN_THRESHOLD_FACTOR;
    double dXMaxThreshold = ZoneSpots.at( 0 ).m_SimpleCenter.x + X_MAX_THRESHOLD_FACTOR;

    FindLocalMinMax( localProfileMinimums , localMinimumsIndex , 
      localProfileMaximums , localMaximumsIndex , 
      Profiles[ ZonesCount - 1 ].m_Data , iIndexSpotsStart , sZone ,
      dXMinThreshold , dXMaxThreshold );
    //FindMinMax(localProfileMinimums, localMinimumsIndex, localProfileMaximums,  localMaximumsIndex, Profiles[ZonesCount].m_Data, iIndexSpotsStart, sZone, dXMinThreshold, dXMaxThreshold, pMarking);
    LocalMinimumsPerSpot.push_back( localProfileMinimums );
    LocalMinimumsPerSpotIndex.push_back( localMinimumsIndex );
    LocalMaximumsPerSpot.push_back( localProfileMaximums );
    LocalMaximumsPerSpotIndex.push_back( localMaximumsIndex );

    if ( localProfileMinimums.size() && localProfileMaximums.size() )
    {
      for ( int iLocalMin = 0 , iLocalMax = 0;
        ( iLocalMin < ( int ) ( localProfileMinimums.size() - 1 ) )
        && ( iLocalMax < ( int ) ( localProfileMaximums.size() - 1 ) );
        iLocalMin++ , iLocalMax++ )
      {
        double dYThesholdDistance = abs( 
          ( localProfileMaximums[ iLocalMax ].imag() - localProfileMaximums[ iLocalMax + 1 ].imag() ) ) / 2;
        double dProfileCenterY = localProfileMaximums[ iLocalMax + 1 ].imag() - dYThesholdDistance;

        double dXDistance = 
          ( localProfileMaximums[ iLocalMax ].real() - localProfileMaximums[ iLocalMax + 1 ].real() );
        double dThresholdDis = abs( localProfileMaximums[ iLocalMax + 1 ].real() - localProfileMinimums[ iLocalMin ].real() ) 
          * dProfileThreshold;
        double dProfileCenterX = localProfileMaximums[ iLocalMax + 1 ].real() - dThresholdDis;


        pMarking->AddFrame( CreatePtFrame(
          IsVert() ? cmplx( dProfileCenterY , dProfileCenterX ) : cmplx( dProfileCenterX , dProfileCenterY ) ,
          "color=0xffff00;Sz=1;" ) );
        profileCenters.push_back( cmplx( dProfileCenterX , dProfileCenterY ) );
        //sProfileBuf.Format("%6.2f , %6.2f\n", dProfileCenterX, dProfileCenterY);//Debug
        //sProfileBufArr.Add(sProfileBuf);//Debug
      }
      profileCentersPerSpot.push_back( profileCenters );
    }
    //WriteToFile(sProfileBufArr, sZone, "profileCentersData");//Debug
    //sProfileBufArr.RemoveAll();//Debug
  }
}// end FindProfileCenter FUNCTION


// Moisey did modification in function, original is above
void LinesAsSpotsProcessing::FindMissingSpotsByProfile(
  SpotVectors& WithMissingSpots ,
  SpotVectors& sSeparatedResults ,
  CContainerFrame * pMarking )
{
  SpotItemVec WithMissingSpotsIndicator; //Added points indication ( true = added, simple center data)
  SpotVector Spots; // With whole spots data

  // Zones = Spots Vector loop, the first zone is for anchor square
  for ( size_t iZoneIterator = 1; iZoneIterator < sSeparatedResults.size(); iZoneIterator++ )
  {
    SpotVector& Zone = sSeparatedResults.at( iZoneIterator ) ;
    CmplxVector& CentersByProfile = m_SpotCentersByProfile.at( iZoneIterator - 1 ) ;
    CFStatistics& ZoneStat = m_ProfileCentersStatistics.at( iZoneIterator - 1 ) ;
    double dDist = ZoneStat.GetAverage() ;

    if ( ( ( iZoneIterator - 1 ) < m_ProfileCenters.size() )
      && ( iZoneIterator < sSeparatedResults.size() ) )
    {
      SpotItem newSpotItem;
      SpotVector SpotsInZone ;
      size_t iProfileIter = 0 ;

      for ( size_t iSpotIter = 0 ;
        (iSpotIter < Zone.size()) && (iProfileIter < CentersByProfile.size()) ;
        iSpotIter++ )
      {
        CColorSpot& Spot = Zone.at( iSpotIter ) ;
        if ( Spot.m_dBlobHeigth > dDist * 0.9 )
          continue ;

        double dYOnProfile = CentersByProfile.at( iProfileIter ).imag() ;
        double dYDiff = dYOnProfile - Spot.m_SimpleCenter.y ;
        
        CDPoint CenterByProfile( CentersByProfile[ 0 ].real() , dYOnProfile ) ;
        if ( m_bVerticalLines )
          swap( CenterByProfile.x , CenterByProfile.y ) ;
        if ( fabs( dYDiff ) > dDist / 4. )
        {
          if ( dYDiff > 0 ) // point on profile is LOWER than spot
          {                 // i.e. spot is higher; should be only once
            continue;       // do pass all spots above profile centers
          }
          else //
          {
            while ( dYDiff < -dDist * 0.75 )
            {
              CColorSpot newSpot;
              newSpot.m_SimpleCenter =CenterByProfile ;
              SpotsInZone.push_back( newSpot ) ;
              pMarking->AddFrame(
                CreatePtFrame( CDPointToCmplx( newSpot.m_SimpleCenter ) , "color=0xff8040;Sz=2;" ) );
              if ( ++iProfileIter < CentersByProfile.size() ) 
              {
                dYOnProfile = CentersByProfile.at( iProfileIter ).imag() ;
                dYDiff = dYOnProfile - Spot.m_SimpleCenter.y ;
              }
              else
                break ;
            }
          }
        }

        if ( Spot.m_dBlobWidth < m_Zones[ iZoneIterator ].Width() - 2. )
        {   // Spot is too small, doesn't go from one side of zone to another
          CColorSpot newSpot; // replace this spot to artificial
          newSpot.m_SimpleCenter = CenterByProfile ;
          SpotsInZone.push_back( newSpot ) ;
          pMarking->AddFrame(
            CreatePtFrame( CDPointToCmplx( newSpot.m_SimpleCenter ) , "color=0xffffff;Sz=2;" ) );
        }
        else if ( iProfileIter < CentersByProfile.size() )
        {   // Spot has full size
//           ASSERT( abs( dYDiff ) < dDist * 0.25 ) ;
          SpotsInZone.push_back( Spot ) ;
        }
        else
          break ;
        iProfileIter++ ;
      }
      WithMissingSpots.push_back( SpotsInZone ) ;
//       m_WithMissingSpotsCenter.push_back( WithMissingSpotsIndicator );
    }
  }
}// end FindMissingSpotsByProfile FUNCTION

void LinesAsSpotsProcessing::FindLastPointInZoneByProfile( 
  CmplxVectors &profileCentersPerSpotPerZone , 
  IntVector &LastPointInIndex , 
  int &iMinLastPointInZone , 
  CContainerFrame * pMarking )
{
  vector<double> dProfileDis;

  for ( size_t ZonesCount = 0; ZonesCount < profileCentersPerSpotPerZone.size(); ZonesCount++ )
  {
    CmplxVector& ProfCentersInZone = profileCentersPerSpotPerZone.at( ZonesCount ) ;
    for ( size_t iProfileCenter = 0; 
      iProfileCenter < ProfCentersInZone.size() - 1;
      iProfileCenter++ )
    {
      dProfileDis.push_back( abs( ProfCentersInZone[ iProfileCenter ].imag()
        - ProfCentersInZone[ iProfileCenter + 1 ].imag() ) );
    }
    for ( size_t iProfileDis = 0; iProfileDis < dProfileDis.size() - 1; iProfileDis++ )
    {
      if ( abs( dProfileDis[ iProfileDis ] - dProfileDis[ iProfileDis + 1 ] ) 
               > ( dProfileDis[ iProfileDis ] * 0.4 ) )
      {
        LastPointInIndex.push_back( ( int ) ( iProfileDis ) );
        if ( ( size_t )iMinLastPointInZone >  iProfileDis )
        {
          iMinLastPointInZone = (int)iProfileDis;
        }
        //   pMarking->AddFrame(CreatePtFrame(
//         cmplx(profileCentersPerSpotPerZone[ZonesCount][iProfileDis].real(), 
//           profileCentersPerSpotPerZone[ZonesCount][iProfileDis].imag()), 
//           "color=0x66ffff;Sz=4;"));
        break;
      }
    }

    dProfileDis.clear();
  }

  for ( size_t ZonesCount = 0; ZonesCount < ( int ) profileCentersPerSpotPerZone.size(); ZonesCount++ )
  {
    if ( ZonesCount < profileCentersPerSpotPerZone.size() )
    {
      for ( size_t iLastPoint = 0; iLastPoint < ( int ) LastPointInIndex.size(); iLastPoint++ )
      {
//    Sasha! This is your version
//         pMarking->AddFrame( CreatePtFrame(
//           profileCentersPerSpotPerZone[ ZonesCount ][ iMinLastPointInZone ] , "color=0x66ffff;Sz=4;" ) );
        if ( iLastPoint < profileCentersPerSpotPerZone[ ZonesCount ].size() )
        {
          pMarking->AddFrame( CreatePtFrame(
            profileCentersPerSpotPerZone[ ZonesCount ][ iLastPoint ] , "color=0x66ffff;Sz=4;" ) );
        }
      }
    }
  }
}// end FindLastPointInZoneByProfile FUNCTION 


void LinesAsSpotsProcessing::FindMinMax( CmplxVector &localProfileMinimums , 
  IntVector &localMinimumsIndex , 
  CmplxVector &localProfileMaximums , 
  IntVector &localMaximumsIndex , 
  CmplxVector &cmplxVector , 
  int iIndexTostart , CString sZone , 
  double xMinThreshold , double xMaxThreshold , CContainerFrame * pMarking )
{
  if ( iIndexTostart < 0 )return;
  int iIndex = iIndexTostart;

  cmplx cMin = cmplx( 10000.0 , 10000.0 );
  cmplx cMax = cmplx( -10000.0 , -10000.0 );

  for ( int iInterval = 0; 
    iInterval < ( int ) ( ( cmplxVector.size() - iIndexTostart ) / MAX_DISY_WINDOW ); 
    iInterval++ )
  {
    cMin = cmplx( 10000.0 , 10000.0 );
    cMax = cmplx( -10000.0 , -10000.0 );
    int iStartStep = iIndexTostart + ( MAX_DISY_WINDOW * iInterval );
    int iEndStep = ( iIndexTostart + MAX_DISY_WINDOW ) + ( MAX_DISY_WINDOW * iInterval );

    for ( int iStep = iStartStep; iStep < iEndStep; iStep++ , iIndex++ ) // Profile pixels coordinates     
    {
      if ( cmplxVector[ iStep ].real() < cMin.real() ) // FindMin
      {
        cMin = cmplxVector[ iStep ]; //cmplx(cmplxVector[iStep].real(), cmplxVector[iStep].imag());
      }

      if ( cmplxVector[ iStep ].real() > cMax.real() ) // FindMax
      {
        cMax = cmplxVector[ iStep ]; // cmplx(cmplxVector[iStep].real(), cmplxVector[iStep].imag());
      }
    }

    localMinimumsIndex.push_back( iIndex );
    localProfileMinimums.push_back( cMin );
    //sProfileMinimumsBuf.Format("%6.2f , %6.2f\n", 
    //  cmplxVectorIterator->real(), cmplxVectorIterator->imag()); //Debug
      //sProfileMinimumsBufArr.Add(sProfileMinimumsBuf); //Debug

    localMaximumsIndex.push_back( iIndex );
    localProfileMaximums.push_back( cMax );
    //sProfileMinimumsBuf.Format("%6.2f , %6.2f\n", 
    //  cmplxVectorIterator->real(), cmplxVectorIterator->imag()); //Debug
    //sProfileMinimumsBufArr.Add(sProfileMinimumsBuf); //Debug
  }

  for ( int iMark = 0; iMark < ( int ) localProfileMinimums.size(); iMark++ )
  {
    pMarking->AddFrame( CreatePtFrame( localProfileMinimums[ iMark ] , "color=0xffff00;Sz=1;" ) );
  }
  for ( int iMark = 0; iMark < ( int ) localProfileMaximums.size(); iMark++ )
  {
    pMarking->AddFrame( CreatePtFrame( localProfileMaximums[ iMark ] , "color=0xffff00;Sz=1;" ) );
  }

}// end FindMinMax FUNCTION 

void LinesAsSpotsProcessing::FindLocalMinMax( CmplxVector &filteredlocalProfileMin , 
  IntVector &localMinimumsIndex , 
  CmplxVector &filteredlocalProfileMax ,
  IntVector &localMaximumsIndex ,
  CmplxVector& Profile ,
  const int iIndexTostart , CString sZone , 
  const double xMinThreshold , const double xMaxThreshold )
{
  /*CArray<CString> sProfileMinimumsBufArr;//Debug
  CString sProfileMinimumsBuf = "";  //Debug
  CArray<CString> sProfileMaximumsBufArr;//Debug
  CString sProfileMaximumsBuf = "";  //Debug*/

  if ( Profile.size() < 5 )
    return;

  CmplxVector::iterator ProfileIt ;
  CmplxVector localProfileMinimums;
  CmplxVector localProfileMaximums;

  if ( iIndexTostart < 0 )
    return;
  int iIndex = iIndexTostart + 1;

  for ( ProfileIt = Profile.begin() + iIndexTostart;
    ProfileIt < Profile.end() - 1;
    ProfileIt++ , iIndex++ ) // Profile pixels coordinates     
  {
    if ( iIndex == iIndexTostart ) // First element - maximum
    {
      localMaximumsIndex.push_back( iIndex );
      localProfileMaximums.push_back( *ProfileIt );
      //sProfileMaximumsBuf.Format("%6.2f , %6.2f\n", 
      //  ProfileIt->real(), ProfileIt->imag());//Debug
      //sProfileMaximumsBufArr.Add(sProfileMaximumsBuf);//Debug
    }
    //FindLocal minimum

    if ( ( !localProfileMaximums.size() || ( ( ProfileIt - 1 )->real() ) > ProfileIt->real() )
      && ( (ProfileIt == Profile.end() - 1) || ( ProfileIt + 1 )->real() > ProfileIt->real() ) )
    {
      if ( ( ProfileIt )->real() < xMinThreshold )
      {
        localMinimumsIndex.push_back( iIndex );
        localProfileMinimums.push_back( *ProfileIt );

        //sProfileMinimumsBuf.Format("%6.2f , %6.2f\n", 
        //  ProfileIt->real(), ProfileIt->imag()); //Debug
        //sProfileMinimumsBufArr.Add(sProfileMinimumsBuf); //Debug
      }
    }
    // For plateaus case        
    else if ( ( ProfileIt - 1 )->real() > ( ProfileIt->real() )
           && ( ( ProfileIt + 1 )->real() == ProfileIt->real() ) )
    {
      if ( ( ProfileIt )->real() < xMinThreshold )
      {
        localMinimumsIndex.push_back( iIndex );
        localProfileMinimums.push_back( *ProfileIt );

        //sProfileMinimumsBuf.Format("%6.2f , %6.2f\n", 
        //  ProfileIt->real(), ProfileIt->imag()); //Debug
        //sProfileMinimumsBufArr.Add(sProfileMinimumsBuf); //Debug
      }
    }
    //FindLocal maximum
    if ( ( ( ProfileIt - 1 )->real() ) < ProfileIt->real() 
      && ( ( ProfileIt + 1 )->real() < ProfileIt->real() ) )
    {
      if ( ( ProfileIt )->real() > xMaxThreshold )
      {
        localMaximumsIndex.push_back( iIndex );
        localProfileMaximums.push_back( *ProfileIt );

        //sProfileMaximumsBuf.Format("%6.2f , %6.2f\n", 
        //  ProfileIt->real(), ProfileIt->imag()); //Debug
        //sProfileMaximumsBufArr.Add(sProfileMaximumsBuf); //Debug
      }
    }
    // For plateaus case        
    else if ( ( ProfileIt - 1 )->real() < ( ProfileIt->real() ) 
         && ( ( ProfileIt + 1 )->real() == ProfileIt->real() ) )
    {
      if ( ( ProfileIt )->real() > xMaxThreshold )
      {
        localMaximumsIndex.push_back( iIndex );
        localProfileMaximums.push_back( *ProfileIt );

        //sProfileMaximumsBuf.Format("%6.2f , %6.2f\n", 
        //  ProfileIt->real(), ProfileIt->imag());//Debug
        //sProfileMaximumsBufArr.Add(sProfileMaximumsBuf);//Debug
      }
    }
  }
  /* WriteToFile(sProfileMinimumsBufArr, sZone, "MinimumsDataProfile");//Debug
   sProfileMinimumsBufArr.RemoveAll();//Debug
   WriteToFile(sProfileMaximumsBufArr, sZone, "MaximumsDataProfile");//Debug
   sProfileMaximumsBufArr.RemoveAll();//Debug*/
  if ( localProfileMaximums.size() && localProfileMinimums.size() )
    FilterMinMaxProfile( localProfileMaximums , localProfileMinimums , 
      filteredlocalProfileMax , filteredlocalProfileMin , sZone );


}// end FindLocalMinMax FUNCTION 

void LinesAsSpotsProcessing::FilterMinMaxProfile( CmplxVector &localProfileMaximums ,
  CmplxVector &localProfileMinimums ,
  CmplxVector &filteredlocalProfileMax ,
  CmplxVector &filteredlocalProfileMin , CString sZone )
{
  /* CArray<CString> sProfileMinimumsBufArr;//Debug
   CString sProfileMinimumsBuf = "";  //Debug
   CArray<CString> sProfileMaximumsBufArr;//Debug
   CString sProfileMaximumsBuf = "";  //Debug*/

  //first Max element
  filteredlocalProfileMax.push_back( cmplx( localProfileMaximums[ 0 ].real() , localProfileMaximums[ 0 ].imag() ) );
  //sProfileMaximumsBuf.Format("%6.2f , %6.2f\n", 
  //  localProfileMaximums[0].real(), localProfileMaximums[0].imag());//Debug
  //sProfileMaximumsBufArr.Add(sProfileMaximumsBuf);//Debug

  for ( auto Iter = localProfileMaximums.begin() + 1; 
    Iter < localProfileMaximums.end() - 1; 
    Iter++ ) // Profile pixels coordinates     
  {
    if ( abs( Iter->imag() - ( Iter - 1 )->imag() ) > MIN_DISY_FOR_MINMAX )
    {
      filteredlocalProfileMax.push_back( cmplx( Iter->real() , Iter->imag() ) );
      //sProfileMaximumsBuf.Format("%6.2f , %6.2f\n", 
      //  Iter->real(), Iter->imag()); //Debug
      //sProfileMaximumsBufArr.Add(sProfileMaximumsBuf); //Debug
    }
  }
  for ( auto Iter = localProfileMinimums.begin() + 1;
    Iter < localProfileMinimums.end() - 1; 
    Iter++ ) // Profile pixels coordinates     
  {
    if ( abs( Iter->imag() - ( Iter - 1 )->imag() ) > MIN_DISY_FOR_MINMAX )
    {
      filteredlocalProfileMin.push_back( cmplx( Iter->real() , Iter->imag() ) );
      //sProfileMinimumsBuf.Format("%6.2f , %6.2f\n", 
      //  Iter->real(), Iter->imag()); //Debug
      //sProfileMinimumsBufArr.Add(sProfileMinimumsBuf); //Debug
    }
  }
  /* WriteToFile(sProfileMinimumsBufArr, sZone, "FilteredMinProfile");//Debug
   sProfileMinimumsBufArr.RemoveAll();//Debug
   WriteToFile(sProfileMaximumsBufArr, sZone, "FilteredMaxProfile");//Debug
   sProfileMaximumsBufArr.RemoveAll();//Debug*/

}// end FilterMinMaxProfile FUNCTION 

void LinesAsSpotsProcessing::FindMinMax_X(const CmplxVector &Contur , double& dMin , double& dMax )
{
  for ( size_t iIndex = 0; iIndex < Contur.size(); iIndex++ )
  {
    if ( Contur[ iIndex ].real() < dMin )
      dMin = Contur[ iIndex ].real();
    if ( Contur[ iIndex ].real() > dMax )
      dMax = Contur[ iIndex ].real();
  }
}// end FindMinMax_X FUNCTION

void LinesAsSpotsProcessing::FindMinMax_X(const CmplxArray &Contur , double& dMin , double& dMax )
{
  for ( int iIndex = 0; iIndex < Contur.size(); iIndex++ )
  {
    if ( Contur[ iIndex ].real() < dMin )
      dMin = Contur[ iIndex ].real();
    if ( Contur[ iIndex ].real() > dMax )
      dMax = Contur[ iIndex ].real();
  }
}// end FindMinMax_X FUNCTION

void LinesAsSpotsProcessing::FindPotentialMissingLinesAsSpots( SpotVectors& sSeparatedResults , 
  /*CList<CPoint> *pPotentialMissimgSpotsList,*/ 
  int potentialMissimgSpotsIndex[ ZONES_NUM ][ MAX_SPOT_NUM ] , CContainerFrame * pMarking )
{
  SpotVector::iterator colorSpotIterator;

  //CList<int> potentialMissimgSpotsIndex;
  CList<CPoint> potentialMissimgSpotsList;
  CList<CPoint> *pPotentialMissimgSpotsList = &potentialMissimgSpotsList;
  CList<CList<CPoint>*> potentialMissimgSpotsListZone;

  for ( int ZonesCount = 0; ZonesCount < ( int ) sSeparatedResults.size(); ZonesCount++ )
  {
    /* CArray<CString> sPotentialMissingLinesBufArr;//Debug
     CString sPotentialMissingLinesBuf = "";  //Debug*/

    int iIndexOfPotentialMissingSpot = 0;
    int iSpotIndex = 0;
    for ( colorSpotIterator = sSeparatedResults[ ZonesCount ].begin(); 
      colorSpotIterator < sSeparatedResults[ ZonesCount ].end() - 1; colorSpotIterator++ ,
      iIndexOfPotentialMissingSpot++ , iSpotIndex++ )
    {
      potentialMissimgSpotsList.AddTail( CPoint( ( int ) colorSpotIterator->m_SimpleCenter.x , 
        ( int ) colorSpotIterator->m_SimpleCenter.y ) );

      //potentialMissimgSpotsList.AddTail(CPoint(colorSpotIterator->m_SimpleCenter.x, 
      //                                  colorSpotIterator->m_SimpleCenter.y));
      //potentialMissimgSpots[ZonesCount][iIndexOfPotentialMissingSpot] = 
      //  CDPoint(colorSpotIterator->m_SimpleCenter.x, colorSpotIterator->m_SimpleCenter.y);

      //sPotentialMissingLinesBuf.Format("%6.2f , %6.2f, %6.2f\n", 
      //  colorSpotIterator->m_SimpleCenter.x, 
      //  colorSpotIterator->m_SimpleCenter.y, colorSpotIterator->m_dBlobHeigth);//Debug 
      //sPotentialMissingLinesBufArr.Add(sPotentialMissingLinesBuf);  //Debug 
      double dDisRatioBetweenSpots = 1.85;
      double dDisRatio = 1.85;

      if ( abs( ( colorSpotIterator->m_SimpleCenter.y ) - ( ( colorSpotIterator + 1 )->m_SimpleCenter.y ) ) 
              > ( colorSpotIterator->m_dBlobHeigth * dDisRatio ) )
      {
        int iNumOfPotentialSpots = 0;

        if ( colorSpotIterator->m_dBlobHeigth != 0 )
        {
          dDisRatioBetweenSpots = abs( ( colorSpotIterator->m_SimpleCenter.y ) 
            - ( ( colorSpotIterator + 1 )->m_SimpleCenter.y ) ) / colorSpotIterator->m_dBlobHeigth;
          iNumOfPotentialSpots = ( int ) ( dDisRatioBetweenSpots );
        }
        for ( int iSpot = 1; iSpot < iNumOfPotentialSpots + 1; iSpot++ )
        {
          //potentialMissimgSpots[ZonesCount][iIndexOfPotentialMissingSpot + iSpot] = 
          // CDPoint(colorSpotIterator->m_SimpleCenter.x, 
          // (colorSpotIterator->m_SimpleCenter.y + colorSpotIterator->m_dBlobHeigth /* iSpot*/ * dDisRatio));
          //POSITION pPosition = potentialMissimgSpotsList.FindIndex(iIndexOfPotentialMissingSpot);
          potentialMissimgSpotsList.AddTail( CPoint( ( int ) colorSpotIterator->m_SimpleCenter.x , 
            ( int ) ( colorSpotIterator->m_SimpleCenter.y + colorSpotIterator->m_dBlobHeigth /* iSpot*/ 
              * dDisRatio ) ) );
          pMarking->AddFrame( CreatePtFrame( cmplx( colorSpotIterator->m_SimpleCenter.x , 
            colorSpotIterator->m_SimpleCenter.y + colorSpotIterator->m_dBlobHeigth /* iSpot*/  
            * dDisRatio ) , "color=0xfffff0;Sz=2;" ) );
          //potentialMissimgSpots[ZonesCount][iIndexOfPotentialMissingSpot + 1] = 1;
        }
      }
    }
    potentialMissimgSpotsListZone.AddTail( pPotentialMissimgSpotsList );

    CString sZone;
    sZone.Format( "Zone = %d" , ZonesCount );
    // WriteToFile(sPotentialMissingLinesBufArr, sZone, "SimpleCentersRaw");//Debug
    // sPotentialMissingLinesBufArr.RemoveAll();//Debug
  }
}//  end FindPotentialMissingLinesAsSpots FUNCTION 
 ////**************************************** end iStatisticsOnlyByProfile == 0************************************************************////

void LinesAsSpotsProcessing::Reset()
{
  m_RiseInZoneRegressions.clear() ;
  m_FallInZoneRegressions.clear() ;
  m_RiseLinesRegressions.clear() ;
  m_FallLinesRegressions.clear() ;

  m_RiseStatistics.clear() ;
  m_FallStatistics.clear() ;
  m_ProfileCentersStatistics.clear() ;
  m_RiseLinesStatistics.clear() ;
  m_FallLinesStatistics.clear() ;

  m_ProfileCenters.clear() ;
  m_SpotCentersByProfile.clear() ;
  m_WithMissingSpots.clear();

  m_Zones.clear() ;

  m_LeadRaggedness_um.Reset() ;
  m_TrailRaggedness_um.Reset() ;
  m_Width_um.Reset() ;
//   m_MinWidth_um.Reset() ;
//   m_MaxWidth_um.Reset() ;
//   m_StdWidth_um.Reset() ;
  m_LeadAngle_deg.Reset() ;
  m_TrailAngle_deg.Reset() ;
  m_Dist_um.Reset() ;
  m_BoundLeft_um.Reset() ;
  m_BoundRight_um.Reset() ;
  m_BoundTop_um.Reset() ;
  m_BoundBottom_um.Reset() ;
  m_BoundHeight_um.Reset() ;
  m_Xmid_um.Reset() ;
  m_Ymid_um.Reset() ;

}

////**************************************** Debug ************************************************************////

void LinesAsSpotsProcessing::WriteToFile( CArray<CString> &sBufArr , CString sZone , CString sileName )
{
  FILE* SpotStatisticsDebug = fopen( "D:\\LinesAsSpotsDebug\\" + sZone + sileName + ".csv" , "a+" );
  if ( SpotStatisticsDebug )
  {
    fputs( "X, Y       \n" , SpotStatisticsDebug );

    for ( int i = 0; i < sBufArr.GetCount(); i++ )
    {
      fputs( sBufArr[ i ] , SpotStatisticsDebug );
    }
    fclose( SpotStatisticsDebug );
  }
} //end of WriteToFile FUNCTION

void LinesAsSpotsProcessing::WriteToFileProfileRawData( NamedCmplxVectors& Profiles ) //Debug
{
  CString sZone; //Debug

  CArray<CString> sProfileRawDataBufArr;//Debug
  CString sProfileRawsBuf = "";  //Debug

  for ( int ZonesCount = 0; ZonesCount < ( int ) Profiles.size(); ZonesCount++ )
  {
    sZone.Format( "Zone = %d," , ZonesCount );//Debug

    for ( int i = 0; i < ( int ) Profiles[ ZonesCount ].m_Data.size(); i++ )
    {
      sProfileRawsBuf.Format( "%6.2f , %6.2f\n" , 
        Profiles[ ZonesCount ].m_Data[ i ].real() , Profiles[ ZonesCount ].m_Data[ i ].imag() );
      sProfileRawDataBufArr.Add( sProfileRawsBuf );
    }
    WriteToFile( sProfileRawDataBufArr , sZone , "profileRawData" );//Debug
    sProfileRawDataBufArr.RemoveAll();//Debug
  }
}// end WriteToFileProfileRawData FUNCTION 


FXString LinesAsSpotsProcessing::GetAsString_Av_Std_Min75_Max75_Min95_Max95(
  CFStatistics& Stat , LPCTSTR pNumbersFormat , LPCTSTR pPrefix )
{
  FXString Result ;
  double dValMin75 = 0. , dValMax75 = 0. , dValMin95 = 0. , dValMax95 = 0. ;
  FXString OutFormat ;
  Stat.GetValueForPercentOnHistogram( 75 , dValMin75 , dValMax75 ) ;
  Stat.GetValueForPercentOnHistogram( 95 , dValMin95 , dValMax95 ) ;
  OutFormat.Format( "%s%s,%s,%s,%s,%s,%s" , ( pPrefix ) ? pPrefix : "" ,
    pNumbersFormat , pNumbersFormat , pNumbersFormat , pNumbersFormat ,
    pNumbersFormat , pNumbersFormat ) ;

  Result.Format( ( LPCTSTR ) OutFormat , Stat.GetAverage() , Stat.GetStd() ,
     dValMin75 , dValMax75 , dValMin95 , dValMax95 ) ;

  return Result ;
}

////**************************************** end Debug ************************************************************////