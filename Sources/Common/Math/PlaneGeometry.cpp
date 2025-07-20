// PlaneGeometry.cpp - functions for on plane operations
#include "stdafx.h"
#include <Math/PlaneGeometry.h>


// From screen to world
bool ConvertBy3Pts( CoordsCorresp& Known1 , CoordsCorresp& Known2 ,
  CoordsCorresp Known3 , CoordsCorresp& Measured )
{

// 1. Find far corner in triangle
double ToPt1 = abs( Measured.FOV - Known1.FOV ) ;
if ( ToPt1 < EPSILON )
{
  Measured.World = Known1.World ;
  return true ;
}
double ToPt2 = abs( Measured.FOV - Known2.FOV ) ;
if ( ToPt2 < EPSILON )
{
  Measured.World = Known2.World ;
  return true ;
}
double ToPt3 = abs( Measured.FOV - Known3.FOV ) ;
if ( ToPt3 < EPSILON )
{
  Measured.World = Known3.World ;
  return true ;
}

// Now we are looking for two nearest calibration pts to Measured pt
// There are points 1 and 2 (OnLine1 and OnLine2)
// The third calibration pt called NotOnLine

CoordsCorresp OnLine1 , OnLine2 , NotOnLine ;
if ( ToPt1 > ToPt2 )
{
  if ( ToPt3 > ToPt1 )
  {
    OnLine1 = Known1 ;
    OnLine2 = Known2 ;
    NotOnLine = Known3 ;
  }
  else
  {
    OnLine1 = Known2 ;
    OnLine2 = Known3 ;
    NotOnLine = Known1 ;
  }
}
else if ( ToPt2 > ToPt3 )
{
  OnLine1 = Known1 ;
  OnLine2 = Known3 ;
  NotOnLine = Known2 ;
}
else
{
  OnLine1 = Known1 ;
  OnLine2 = Known2 ;
  NotOnLine = Known3 ;
}

// 2. Find cross point between nearest triangle side (pt1 and pt2)
//    and vector from far corner to measured point

CLine2d FromFarToMeasured( NotOnLine.FOV , Measured.FOV ) ;
CLine2d Line( OnLine1.FOV , OnLine2.FOV ) ;
cmplx CrossLineInFOV ;
if ( !Line.intersect( FromFarToMeasured , CrossLineInFOV ) )
{
  return false ;
}

// 3. Find world coordinates of cross point
cmplx LineVector = OnLine2.FOV - OnLine1.FOV ;
double Dist1to2InFOV = abs( LineVector ) ;
cmplx CrossVector = CrossLineInFOV - OnLine1.FOV ;
double CrossTo1Dist = abs( CrossVector ) ;
cmplx CrossToPt2Vector = CrossLineInFOV - OnLine2.FOV ;
double CrossTo2Dist = abs( CrossToPt2Vector ) ;
cmplx CrossInWorld ;
cmplx WorldVect1to2 = OnLine2.World - OnLine1.World ;
double RatioInFOV = 1.0 ;
// there is case, when cross pt is out of segment between pt1 and pt2
// close to pt1
if ( Dist1to2InFOV < CrossTo2Dist )
{  // cross out of segment close to pt1
  RatioInFOV = CrossTo1Dist / Dist1to2InFOV ;
  CrossInWorld = OnLine1.World - (WorldVect1to2 * RatioInFOV) ;
}
else
{   // cross inside line segment or cross out of segment close to pt2
    // simple Convert cross point to World coordinate
  RatioInFOV = CrossTo1Dist / Dist1to2InFOV ;
  CrossInWorld = OnLine1.World + (WorldVect1to2 * RatioInFOV) ;
}

// 4. Find world coordinates of Measured point
// There we are using known world coordinates of NotOnLine pt and Cross pt
cmplx VectFromNotOnLineToCross = CrossLineInFOV - NotOnLine.FOV ;
double DistFromCrossToNotOnLine = abs( VectFromNotOnLineToCross ) ;
cmplx VectFromMeasToNotOnLine = Measured.FOV - NotOnLine.FOV ;
double DistFromMeasurdToNotOnLine = abs( VectFromMeasToNotOnLine ) ;
RatioInFOV = DistFromMeasurdToNotOnLine / DistFromCrossToNotOnLine ;
cmplx VectFromNotOnLineToCrossInWorld = CrossInWorld - NotOnLine.World ;
Measured.World = NotOnLine.World + VectFromNotOnLineToCrossInWorld * RatioInFOV ;

return true ;
}

cmplx FindNearest( CFigure * pFigure , cmplx& Pt , int& iFoundIndex ,
  bool bIsSmoothCurve , int iEXtremIndex  , bool bGoPlus  )
{
  double dMinDist = DBL_MAX ;
  iFoundIndex = -1 ;
  cmplx * AsCmplx = (cmplx*) pFigure->GetData() ;
  int iStep = (bGoPlus) ? 1 : -1 ;
  int iCurrIndex = (iEXtremIndex >= 0) ? iEXtremIndex : 0 ;
  if ( iFoundIndex >= 0 )
    iCurrIndex = iFoundIndex ;

  iCurrIndex %= pFigure->GetCount() ;
  int iStartIndex = iCurrIndex ;
  do
  {
    double dDist = abs( AsCmplx[ iCurrIndex ] - Pt ) ;
    if ( dDist < dMinDist )
    {
      iFoundIndex = iCurrIndex ;
      dMinDist = dDist ;
    }
    else if ( bIsSmoothCurve )
    {
      if ( dDist > 1.3 * dMinDist )
        return AsCmplx[ iFoundIndex ] ;
    }
    iCurrIndex += iStep + (int) pFigure->GetCount() ;
    iCurrIndex %= pFigure->GetCount() ;
  } while ( iCurrIndex != iStartIndex ) ;
  return (iFoundIndex >= 0) ? AsCmplx[ iFoundIndex ] : cmplx( 0. , 0. ) ;
}


cmplx FindNearestToY( const CFigure * pFigure , cmplx& Pt , bool bdXPositive )
{
  double dTargetY = Pt.imag() ;
  double dCenterX = Pt.real() ;
  double dMinDist = DBL_MAX ;
  int iIndex = -1 ;
  for ( int i = 0; i < pFigure->GetCount() ; i++ )
  {
    double dXDiff = pFigure->GetAt( i ).x - dCenterX ;
    BOOL bYes = (dXDiff > 0) ^ (bdXPositive == false) ;
    if ( ((dXDiff > 0) && (bdXPositive == true))
      || ((dXDiff < 0) && (bdXPositive == false)) )
    {
      double dYDiff = fabs( dTargetY - pFigure->GetAt( i ).y ) ;
      if ( dYDiff < dMinDist )
      {
        dMinDist = dYDiff ;
        iIndex = i ;
      }
    }
  }
  return (iIndex >= 0) ? CDPointToCmplx( pFigure->GetAt( iIndex ) ) : cmplx( 0. , 0. ) ;
}


int FindNearestToYIndex(
  const CFigure * pFigure , const cmplx& Pt , bool bdXPositive )
{
  double dTargetY = Pt.imag() ;
  double dCenterX = Pt.real() ;
  double dMinDist = DBL_MAX ;
  int iIndex = -1 ;
  for ( int i = 0; i < pFigure->GetCount() ; i++ )
  {
    double dXDiff = pFigure->GetAt( i ).x - dCenterX ;
    BOOL bYes = (dXDiff > 0) ^ (bdXPositive == false) ;
    if ( ((dXDiff > 0) && (bdXPositive == true))
      || ((dXDiff < 0) && (bdXPositive == false)) )
    {
      double dYDiff = fabs( dTargetY - pFigure->GetAt( i ).y ) ;
      if ( dYDiff < dMinDist )
      {
        dMinDist = dYDiff ;
        iIndex = i ;
      }
    }
  }
  return iIndex ;
}

int FindNearestToDirection(
  const CFigure * pFigure , cmplx& cCentPt , double dDirFromCent )
{
  double dMinAngleDiff = DBL_MAX ;
  int iIndex = -1 ;
  cmplx * pData = (cmplx*) pFigure->GetData() ;
  for ( int i = 0; i < pFigure->GetCount() ; i++ )
  {
    cmplx cPt = pData[i] ;
    cmplx cVectFromCent = cPt - cCentPt ; // Y is inverted
    double dAng = arg( cVectFromCent ) ;
    double dAngleDiff = fabs( dAng - dDirFromCent ) ;
    if ( dAngleDiff < dMinAngleDiff )
    {
      dMinAngleDiff = dAngleDiff ;
      iIndex = i ;
    }
  }
  return iIndex ;
}

cmplx FindNearestToX( CFigure * pFigure , cmplx& Pt , bool bdYPositive )
{
  double dCenterY = Pt.imag() ;
  double dTargetX = Pt.real() ;
  double dMinDist = DBL_MAX ;
  int iIndex = -1 ;
  for ( int i = 0; i < pFigure->GetCount() ; i++ )
  {
    double dYDiff = pFigure->GetAt( i ).y - dCenterY ;
    BOOL bYes = (dYDiff > 0) ^ (bdYPositive == false) ;
    if ( ((dYDiff > 0) && (bdYPositive == true))
      || ((dYDiff < 0) && (bdYPositive == false)) )
    {
      double dYDiff = fabs( dTargetX - pFigure->GetAt( i ).x ) ;
      if ( dYDiff < dMinDist )
      {
        dMinDist = dYDiff ;
        iIndex = i ;
      }
    }
  }
  return (iIndex >= 0) ? CDPointToCmplx( pFigure->GetAt( iIndex ) ) : cmplx( 0. , 0. ) ;
}

cmplx FindNearestToX( CFigure * pFigure , int& iInitIndex , double dTargetX , bool bToPlus  )
{
  int iIndex = iInitIndex ;
  cmplx * pcOrigin = ( cmplx* ) ( pFigure->GetData() ) ;
  cmplx * pcEnd = pcOrigin + pFigure->GetCount() ;
  cmplx * pcIter = pcOrigin + iIndex ;
  double dPrevDist = pcIter->real() - dTargetX ;
  double dDist ;
  for ( int i = 0 ; i < pFigure->GetCount() ; i++ )
  {
    if ( bToPlus )
    {
      if ( ++pcIter >= pcEnd )
        pcIter = pcOrigin ;
    }
    else
    {
      if ( --pcIter < pcOrigin )
        pcIter = pcEnd - 1 ; ;
    }
    dDist = pcIter->real() - dTargetX ;
    if ( ( dDist == 0. ) || ( dDist * dPrevDist < 0. ) )
    {
      iInitIndex = (int)( pcIter - pcOrigin );
      return *pcIter ;
    }
  }
  
  return cmplx( 0. , 0. ) ;
}

size_t TrackToTargetX( CFigure * pFigure , CmplxVector& Pts , 
  int iInitIndex , double dStopX , bool bToPlus )
{
  int iIndex = iInitIndex ;
  cmplx * pcOrigin = ( cmplx* ) ( pFigure->GetData() ) ;
  cmplx * pcEnd = pcOrigin + pFigure->GetCount() ;
  cmplx * pcIter = pcOrigin + iIndex ;
  Pts.push_back( *pcIter ) ;
  cmplx cPrev = *pcIter ;
  double dPrevDist = pcIter->real() - dStopX ;
  double dDist ;
  for ( int i = 0 ; i < pFigure->GetCount() ; i++ )
  {
    if ( bToPlus )
    {
      if ( ++pcIter >= pcEnd )
        pcIter = pcOrigin ;
    }
    else
    {
      if ( --pcIter < pcOrigin )
        pcIter = pcEnd - 1 ; ;
    }
    if ( cPrev.real() >= pcIter->real() )
      continue ;
    dDist = pcIter->real() - dStopX ;
    if ( ROUND( fabs( cPrev.real() - pcIter->real() ) ) != 0 )
    {
      cPrev = *pcIter ;
      Pts.push_back( cPrev ) ;
    }
    if ( ( dDist == 0. ) || ( dDist * dPrevDist < 0. ) )
      break ;
  }

  return Pts.size() ;
}

size_t FindNearestToPts( const CFigure * pFigure , 
  cmplx * pcTargetPts , int iNPts ,IntVector& Indexes , int iInitialIndex )
{
  double dPrevDist = DBL_MAX ;
  int iIndex = -1 ;
  bool bFindNearestX = pcTargetPts->real() != 0. ;
  for (int i = iInitialIndex; i < pFigure->GetCount() ; i++)
  {
    cmplx NextPt = CDPointToCmplx( pFigure->GetAt( i ) ) ;
    double dNextDist = fabs( ( bFindNearestX ) ?
      NextPt.real() - pcTargetPts->real() : NextPt.imag() - pcTargetPts->imag() ) ;
    if (dPrevDist < dNextDist)
    {
      Indexes.push_back( i ) ;
      if (--iNPts == 0)
        break ;
      pcTargetPts++ ;
      dPrevDist = DBL_MAX ;
      bFindNearestX = pcTargetPts->real() != 0. ;
    }
    else
      dPrevDist = dNextDist ;
  }
  return Indexes.size() ;
}

cmplx FindExtrems( 
  const cmplx * pData , size_t iSize ,
  Extremes_s& Extrems ,        // Array of 4 values for extreme points coordinates
  ExtrIndexes_s * pIndexes ,  // = NULL Array of 4 values for extreme indexes
  cmplx * pSize )   // = NULL 
{
  if ( !iSize )
    return cmplx() ;

  int iIndexXMin = -1 , iIndexYMin = -1 , iIndexXMax = -1 , iIndexYMax = -1 ;
  double dXMin = DBL_MAX , dXMax = -DBL_MAX ;
  double dYMin = DBL_MAX , dYMax = -DBL_MAX ;
  const cmplx * pIter = pData ;
  for ( int i = 0; i < ( int ) iSize ; i++ , pIter++ )
  {
    double dX = pIter->real() ;
    SetMinAndIndex( dX , dXMin , iIndexXMin , i ) ;
    SetMaxAndIndex( dX , dXMax , iIndexXMax , i ) ;
    double dY = pIter->imag() ;
    SetMinAndIndex( dY , dYMin , iIndexYMin , i ) ;
    SetMaxAndIndex( dY , dYMax , iIndexYMax , i ) ;
  }
  Extrems.m_cLeft = *( pData + iIndexXMin )  ;
  Extrems.m_cTop = *( pData + iIndexYMin ) ;
  Extrems.m_cRight = *( pData + iIndexXMax ) ;
  Extrems.m_cBottom = *( pData + iIndexYMax ) ;

  if ( pIndexes )
  {
    pIndexes->m_iLeft = iIndexXMin ;
    pIndexes->m_iTop = iIndexYMin  ;
    pIndexes->m_iRight  = iIndexXMax  ;
    pIndexes->m_iBottom = iIndexYMax  ;
  }

  if ( pSize )
    *pSize = Extrems.GetSize() ;
  return Extrems.GetCenter() ;
}


cmplx FindExtrems( const cmplx * pData , size_t iSize ,
  CmplxArray& Extrems , FXIntArray * pIndexes , cmplx * pSize )
{
  if ( !iSize )
    return cmplx() ;
  Extrems.SetSize( 4 ) ;
  return FindExtrems( pData , iSize , (Extremes_s&) *Extrems.GetData() ,
    pIndexes ? ( ExtrIndexes_s*)pIndexes->GetData() : NULL , pSize ) ;
}
 

cmplx FindExtrems( const CmplxArray& Data ,
  CmplxArray& Extrems , FXIntArray * pIndexes , cmplx * pSize )
{
  return FindExtrems( Data.GetData() , Data.size() , Extrems , pIndexes , pSize ) ;
}

cmplx FindExtrems( const CmplxVector& Data ,
  CmplxArray& Extrems , FXIntArray * pIndexes , cmplx * pSize )
{
  return FindExtrems( Data.data() , Data.size() , Extrems , pIndexes , pSize ) ;
}

cmplx FindExtrems( const CFigure * pFigure ,
  CmplxArray& Extrems , FXIntArray * pIndexes , cmplx * pSize )
{

  if ( !pFigure || !pFigure->GetCount() )
    return cmplx() ;

  const cmplx * pData = ( const cmplx * ) ( pFigure->GetData() ) ;
  return FindExtrems( pData , pFigure->size() , Extrems , pIndexes , pSize ) ;
}

cmplx FindExtrems( const CFigure * pFigure ,
  CmplxVector& Extrems , IntVector * pIndexes , cmplx * pSize )
{
  if (pFigure->GetCount() == 0)
    return cmplx() ;
  cmplx * pAsCmplx = ( cmplx* ) pFigure->GetData() ;
  int iIndexXMin = -1 , iIndexYMin = -1 , iIndexXMax = -1 , iIndexYMax = -1 ;
  double dXMin = DBL_MAX , dXMax = -DBL_MAX ;
  double dYMin = DBL_MAX , dYMax = -DBL_MAX ;
  for (int i = 0; i < pFigure->GetCount() ; i++)
  {
    double dX = pAsCmplx[ i ].real() ;
    double dY = pAsCmplx[ i ].imag() ;
    SetMinAndIndex( dX , dXMin , iIndexXMin , i ) ;
    SetMaxAndIndex( dX , dXMax , iIndexXMax , i ) ;
    SetMinAndIndex( dY , dYMin , iIndexYMin , i ) ;
    SetMaxAndIndex( dY , dYMax , iIndexYMax , i ) ;
  }
  Extrems.clear() ;
  Extrems.push_back( pAsCmplx[ iIndexXMin ] ) ;
  Extrems.push_back( pAsCmplx[ iIndexYMin ] ) ;
  Extrems.push_back( pAsCmplx[ iIndexXMax ] ) ;
  Extrems.push_back( pAsCmplx[ iIndexYMax ] ) ;

  if (pIndexes)
  {
    pIndexes->push_back( iIndexXMin ) ;
    pIndexes->push_back( iIndexYMin ) ;
    pIndexes->push_back( iIndexXMax ) ;
    pIndexes->push_back( iIndexYMax ) ;
  }

  if (pSize)
  {
    *pSize = cmplx( Extrems[ 2 ].real() - Extrems[ 0 ].real() ,
      Extrems[ 3 ].imag() - Extrems[ 1 ].imag() ) ;
  }
  return cmplx( ( Extrems[ 0 ].real() + Extrems[ 2 ].real() ) * 0.5 ,
    ( Extrems[ 1 ].imag() + Extrems[ 3 ].imag() ) * 0.5 ) ;
}

cmplx FindLeftPt( const CFigure * pFigure , int& iIndex )
{
  if ( pFigure->GetCount() == 0 )
    return cmplx() ;
  double dXMin = DBL_MAX ;
  for ( int i = 0; i < pFigure->GetCount() ; i++ )
  {
    double dX = pFigure->GetAt( i ).x ;
    SetMinAndIndex( dX , dXMin , iIndex , i ) ;
  }
  return CDPointToCmplx( pFigure->GetAt( iIndex ) ) ;
}
cmplx FindFarthestPt( cmplx& cBase , const CFigure * pFigure , int& iIndex )
{
  cmplx * pData = (cmplx*) pFigure->GetData() ;
  cmplx * pIter = pData ;
  cmplx * pEnd = pData + pFigure->Count() ;
  double dMaxDist = 0.  ;
  int iIndexMax = -1 ;

  do
  {
    double dNewDist = abs( *pIter - cBase ) ;
    if ( dNewDist > dMaxDist )
    {
      iIndexMax = (int)(pIter - pData) ;
      dMaxDist = dNewDist ;
    }
  } while ( ++pIter < pEnd ) ;
  iIndex = iIndexMax ;
  return pData[ iIndexMax ] ;
}

cmplx FindFarthestPtFromFigSegment( int iIndexBegin , int iIndexEnd ,
  const CFigure * pFigure , int& iFarestIndex , double& dFarestDist )
{
  cmplx * pData = ( cmplx* ) pFigure->GetData() ;
  cmplx cBegin = pData[ iIndexBegin ] ;
  cmplx cEnd = pData[ iIndexEnd ] ;
  cmplx * pIter = pData + iIndexBegin + 1 ;
  cmplx * pEnd = pData + iIndexEnd ;
  dFarestDist = 0.  ;
  int iIndexMax = -1 ;

  do
  {
    double dNewDist = GetPtToLineDistance( *pIter , cBegin , cEnd ) ;
    double dAbsDist = fabs( dNewDist ) ;
    if ( dAbsDist > dFarestDist )
    {
      iIndexMax = ( int ) ( pIter - pData ) ;
      dFarestDist = dAbsDist ;
    }
  } while ( ++pIter < pEnd ) ;
  iFarestIndex = iIndexMax ;
  return pData[ iIndexMax ] ;
}


// from screen to world
bool ConvertBy3Pts( CoordsCorresp& Known1 , CoordsCorresp& Known2 ,
  CoordsCorresp& Known3 , cmplx& Screen , cmplx& World ) // 
{

  // 1. Find far corner in triangle
  double ToPt1 = abs( Screen - Known1.FOV ) ;
  double ToPt2 = abs( Screen - Known2.FOV ) ;
  double ToPt3 = abs( Screen - Known3.FOV ) ;
  if ( ToPt1 < EPSILON )
  {
    World = Known1.World ;
    return true ;
  }
  if ( ToPt2 < EPSILON )
  {
    World = Known2.World ;
    return true ;
  }
  if ( ToPt3 < EPSILON )
  {
    World = Known3.World ;
    return true ;
  }

  // Now we are looking for two nearest calibration pts to Measured pt
  // There are points 1 and 2 (OnLine1 and OnLine2)
  // The third calibration pt called NotOnLine

  CoordsCorresp OnLine1 , OnLine2 , NotOnLine ;
  if ( ToPt1 > ToPt2 )
  {
    if ( ToPt3 > ToPt1 )
    {
      OnLine1 = Known1 ;
      OnLine2 = Known2 ;
      NotOnLine = Known3 ;
    }
    else
    {
      OnLine1 = Known2 ;
      OnLine2 = Known3 ;
      NotOnLine = Known1 ;
    }
  }
  else if ( ToPt2 > ToPt3 )
  {
    OnLine1 = Known1 ;
    OnLine2 = Known3 ;
    NotOnLine = Known2 ;
  }
  else
  {
    OnLine1 = Known1 ;
    OnLine2 = Known2 ;
    NotOnLine = Known3 ;
  }

  // 2. Find cross point between nearest triangle side (pt1 and pt2)
  //    and vector from far corner to measured point

  CLine2d FromFarToMeasured( NotOnLine.FOV , Screen ) ;
  CLine2d Line( OnLine1.FOV , OnLine2.FOV ) ;
  cmplx CrossLineInFOV ;
  if ( !Line.intersect( FromFarToMeasured , CrossLineInFOV ) )
  {
    return false ;
  }

  // 3. Find world coordinates of cross point
  cmplx LineVector = OnLine2.FOV - OnLine1.FOV ;
  double Dist1to2InFOV = abs( LineVector ) ;
  cmplx CrossVector = CrossLineInFOV - OnLine1.FOV ;
  double CrossTo1Dist = abs( CrossVector ) ;
  cmplx CrossToPt2Vector = CrossLineInFOV - OnLine2.FOV ;
  double CrossTo2Dist = abs( CrossToPt2Vector ) ;
  cmplx CrossInWorld ;
  cmplx WorldVect1to2 = OnLine2.World - OnLine1.World ;
  double RatioInFOV = 1.0 ;
  // there is case, when cross pt is out of segment between pt1 and pt2
  // close to pt1
  if ( Dist1to2InFOV < CrossTo2Dist )
  {  // cross out of segment close to pt1
    RatioInFOV = CrossTo1Dist / Dist1to2InFOV ;
    CrossInWorld = OnLine1.World - (WorldVect1to2 * RatioInFOV) ;
  }
  else
  {   // cross inside line segment or cross out of segment close to pt2
    // simple Convert cross point to World coordinate
    RatioInFOV = CrossTo1Dist / Dist1to2InFOV ;
    CrossInWorld = OnLine1.World + (WorldVect1to2 * RatioInFOV) ;
  }

  // 4. Find world coordinates of Measured point
  // There we are using known world coordinates of NotOnLine pt and Cross pt
  cmplx VectFromNotOnLineToCross = CrossLineInFOV - NotOnLine.FOV ;
  double DistFromCrossToNotOnLine = abs( VectFromNotOnLineToCross ) ;
  cmplx VectFromMeasToNotOnLine = Screen - NotOnLine.FOV ;
  double DistFromMeasurdToNotOnLine = abs( VectFromMeasToNotOnLine ) ;
  RatioInFOV = DistFromMeasurdToNotOnLine / DistFromCrossToNotOnLine ;
  cmplx VectFromNotOnLineToCrossInWorld = CrossInWorld - NotOnLine.World ;
  World = NotOnLine.World + VectFromNotOnLineToCrossInWorld * RatioInFOV ;

  return true ;
}


// from world to screen
bool ConvertWtoSBy3Pts( CoordsCorresp& Known1 , CoordsCorresp& Known2 ,
  CoordsCorresp Known3 , cmplx& Screen , cmplx& World ) // 
{

  // 1. Find far corner in triangle
  double ToPt1 = abs( World - Known1.World ) ;
  double ToPt2 = abs( World - Known2.World ) ;
  double ToPt3 = abs( World - Known3.World ) ;
  if ( ToPt1 < EPSILON )
  {
    Screen = Known1.FOV ;
    return true ;
  }
  if ( ToPt2 < EPSILON )
  {
    Screen = Known2.FOV ;
    return true ;
  }
  if ( ToPt3 < EPSILON )
  {
    Screen = Known3.FOV ;
    return true ;
  }

  // Now we are looking for two nearest calibration pts to Measured pt
  // There are points 1 and 2 (OnLine1 and OnLine2)
  // The third calibration pt called NotOnLine

  CoordsCorresp OnLine1 , OnLine2 , NotOnLine ;
  if ( ToPt1 > ToPt2 )
  {
    if ( ToPt3 > ToPt1 )
    {
      OnLine1 = Known1 ;
      OnLine2 = Known2 ;
      NotOnLine = Known3 ;
    }
    else
    {
      OnLine1 = Known2 ;
      OnLine2 = Known3 ;
      NotOnLine = Known1 ;
    }
  }
  else if ( ToPt2 > ToPt3 )
  {
    OnLine1 = Known1 ;
    OnLine2 = Known3 ;
    NotOnLine = Known2 ;
  }
  else
  {
    OnLine1 = Known1 ;
    OnLine2 = Known2 ;
    NotOnLine = Known3 ;
  }

  // 2. Find cross point between nearest triangle side (pt1 and pt2)
  //    and vector from far corner to measured point

  CLine2d FromFarToMeasured( NotOnLine.World , World ) ;
  CLine2d Line( OnLine1.World , OnLine2.World ) ;
  cmplx CrossLineInWorld ;
  if ( !Line.intersect( FromFarToMeasured , CrossLineInWorld ) )
  {
    return false ;
  }

  // 3. Find world coordinates of cross point
  cmplx LineVector1to2 = OnLine2.World - OnLine1.World ;
  double Dist1to2InWorld = abs( LineVector1to2 ) ;
  cmplx CrossToPt1Vector = CrossLineInWorld - OnLine1.World ;
  double CrossTo1Dist = abs( CrossToPt1Vector ) ;
  cmplx CrossToPt2Vector = CrossLineInWorld - OnLine2.World ;
  double CrossTo2Dist = abs( CrossToPt2Vector ) ;
  cmplx CrossInFOV ;
  cmplx FOVVect1to2 = OnLine2.FOV - OnLine1.FOV ;
  double RatioInWorld = 1.0 ;
  // there is case, when cross pt is out of segment between pt1 and pt2
  // close to pt1
  if ( Dist1to2InWorld < CrossTo2Dist )
  {  // cross out of segment close to pt1
    RatioInWorld = CrossTo1Dist / Dist1to2InWorld ;
    CrossInFOV = OnLine1.FOV - (FOVVect1to2 * RatioInWorld) ;
  }
  else
  {   // cross inside line segment or cross out of segment close to pt2
      // simple Convert cross point to World coordinate
    RatioInWorld = CrossTo1Dist / Dist1to2InWorld ;
    CrossInFOV = OnLine1.FOV + (FOVVect1to2 * RatioInWorld) ;
  }

  // 4. Find world coordinates of Measured point
  // There we are using known world coordinates of NotOnLine pt and Cross pt
  cmplx VectFromNotOnLineToCross = CrossLineInWorld - NotOnLine.World ;
  double DistFromCrossToNotOnLine = abs( VectFromNotOnLineToCross ) ;
  cmplx VectFromMeasToNotOnLine = World - NotOnLine.World ;
  double DistFromMeasurdToNotOnLine = abs( VectFromMeasToNotOnLine ) ;
  RatioInWorld = DistFromMeasurdToNotOnLine / DistFromCrossToNotOnLine ;
  cmplx VectFromNotOnLineToCrossInFOV = CrossInFOV - NotOnLine.FOV ;
  Screen = NotOnLine.FOV + VectFromNotOnLineToCrossInFOV * RatioInWorld ;

  return true ;
}


double GetDiffFromStraight( cmplx * pPts , int iLen ,
  int iInd1st , int iInd2nd ,
  double& dLengthRatio , double * pdStd )
{
  int iNPts = abs( iInd2nd - iInd1st ) - 2 ;
  if ( iNPts < 1 )
    return 0. ;
  int iDir = (iInd2nd > iInd1st) ? 1 : -1 ;


  iInd1st = (int) GetIndexInRing( iInd1st , iLen ) ;
  iInd2nd = (int) GetIndexInRing( iInd2nd , iLen ) ;


  cmplx& Pt2 = pPts[ iInd1st ] ;
  cmplx& Pt3 = pPts[ iInd2nd ] ;
  double dX = Pt3.real() - Pt2.real() ;
  double dY = Pt3.imag() - Pt2.imag() ;

  double dLength = abs( Pt3 - Pt2 ) ;


  cmplx Pt = Pt2 ;
  int iIndex = iInd1st + iDir ;
  double dLen = 0. ;
  double dMaxDistCross = 0. ;
  double dSum2 = 0 ;
  while ( iIndex != iInd2nd )
  {
    cmplx NextPt = pPts[ iIndex ] ;
    double dDistOnLine = abs( NextPt - Pt ) ;
    dLen += dDistOnLine ;
    //double dS = (dY * (NextPt.real() - Pt2.real())
    //  - dX * (NextPt.imag() - Pt2.imag())) / 2. ;
    double dDist = GetPtToLineDistance( NextPt , Pt2 , Pt3 ) ;
    if ( fabs( dDist ) > EPSILON )
    {
      double dAbsDistCross = fabs( dDist ) ;
      if ( dAbsDistCross > dMaxDistCross )
        dMaxDistCross = dAbsDistCross ;
      if ( pdStd )
        dSum2 += dAbsDistCross * dAbsDistCross ;
    }
    iIndex += iDir ;
    iIndex = (int) GetIndexInRing( iIndex , iLen ) ;

    Pt = NextPt ;
  }

  dLengthRatio = dLen / dLength ;

  if ( pdStd )
  {
    *pdStd = sqrt( dSum2 / iNPts ) ;
  }
  return dMaxDistCross ;
}


// function returns maximal angle difference between neightbour segments
double CheckStraightness( cmplx * pPts , int iLen , // array of pts and it's length
  int iInd1st , int iInd2nd , int iCheckStep , // part for analysis begin and end, step for analysis 
  double& dMinDist , double& dMaxDist ) // Minimal and maximal subsegment lengths
{
  int iNPts = abs( iInd2nd - iInd1st ) - 2 ;
  if ( iNPts < (iCheckStep * 2) - 2 )
    return 0. ;
  int iDir = (iInd2nd > iInd1st) ? iCheckStep : -iCheckStep ;

  iInd1st = (int) GetIndexInRing( iInd1st , iLen ) ;
  iInd2nd = (int) GetIndexInRing( iInd2nd , iLen ) ;

  cmplx Pt1 = pPts[ iInd1st ] ;
  int iIndex = (int) GetIndexInRing( iInd1st + iDir , iLen ) ;
  cmplx Pt2 = pPts[ iIndex ] ;
  cmplx cVect = Pt2 - Pt1 ;
  double dAngle = arg( cVect ) ;
  double dMaxTurn = 0. ;
  dMinDist = dMaxDist = abs( cVect ) ;
  for ( int iPath = iCheckStep * 2 ; iPath < iNPts ; iPath += iDir )
  {
    Pt1 = Pt2 ;
    iIndex = (int) GetIndexInRing( iIndex + iDir , iLen ) ;
    Pt2 = pPts[ iIndex ] ;
    cmplx cNewVect = Pt2 - Pt1 ;
    double dDAngle = GetAngleBtwVects( cNewVect , cVect ) ;
    if ( fabs( dMaxTurn ) < fabs( dDAngle ) )
      dMaxTurn = dDAngle ;
    double dLen = abs( cNewVect ) ;
    if ( dLen < dMinDist )
      dMinDist = dLen ;
    if ( dLen > dMaxDist )
      dMaxDist = dLen ;
  }
  return dMaxTurn ;
}

cmplx GetCenterForFigure( cmplx * pFigure , int iFigLen , ImgMoments& Moments )
{
  Moments.Reset() ;
  cmplx * pEnd = pFigure + iFigLen ;
  cmplx * p = pFigure ;
  cmplx cPt = *p ;
  cmplx cSum ;
  double dAccLen = 0. ;
  while ( ++p < pEnd )
  {
    cmplx cNext = *p ;
    cmplx cCent = 0.5 * (cPt + cNext) ;
    double dLen = abs( cNext - cPt ) ;
    cSum += dLen * cCent ;
    dAccLen += dLen ;
    Moments.Add( cCent.real() , cCent.imag() , dLen ) ;
  }
  return (dAccLen > EPSILON) ? cSum / dAccLen : cSum ;
}

cmplx GetCenterForFigure( cmplx * pFigure , int iFigLen ,
  cmplx cCenter , ImgMoments& Moments )
{
  Moments.Reset() ;
  cmplx * pEnd = pFigure + iFigLen ;
  cmplx * p = pFigure ;
  cmplx cPt = *p ;
  cmplx cPrev = cPt ;
  while ( ++p < pEnd )
  {
    cmplx cNext = *p ;
    double dArea = fabs(GetTriangleArea( cCenter , cPrev , cNext )) ;
    cmplx cWeightCent = GetTriangleWeightCenter( cCenter , cPrev , cNext ) ;
    Moments.Add( cWeightCent , dArea ) ;
    cPrev = cNext ;
  }
  return Moments.GetCenter() + cPt ;
}

//for approximate circle or ellipse, don't use for random contur
double GetMinMaxDia( const cmplx * pContur , int iConturLen ,
  double& dMinDia , double& dMaxDia , int iAverage )
{
  double Diameters[ 20000 ] ;
  const cmplx * pEnd = pContur + iConturLen ;
  int iNDiameters = iConturLen / 2;
  int iOppositeIndex = iNDiameters ;
  int iScanRange = iConturLen / 12 ;
  int iDiaIndex = 0 ;
  dMinDia = DBL_MAX ;
  dMaxDia = 0. ;
  double dDiaSum = 0. ;
  const cmplx * p = pContur ;
  do 
  {
    const cmplx * pOpposite = p + iOppositeIndex - iScanRange/2 ;
    if (pOpposite >= pEnd)
      pOpposite -= iConturLen ;
    const cmplx * pScanEnd = pOpposite + iScanRange - 1 ;
    if (pScanEnd >= pEnd)
      pScanEnd -= iConturLen ;
    double dLocalDiaMax = 0. ;
    do 
    {
      double dDist = abs( *p - *pOpposite ) ;
      if (dDist > dLocalDiaMax)
        dLocalDiaMax = dDist ;
      if (++pOpposite >= pEnd)
        pOpposite -= iConturLen ;
    } while ( pOpposite < pScanEnd );
    Diameters[ iDiaIndex++ ] = dLocalDiaMax ;
    dDiaSum += dLocalDiaMax ;
    if ( iAverage == 0 )
    {
      if (dLocalDiaMax < dMinDia)
        dMinDia = dLocalDiaMax ;
      if (dLocalDiaMax > dMaxDia)
        dMaxDia = dLocalDiaMax ;
    }
  } while (++p < pEnd);

  if ( iAverage > 0 )
  {
    double * pLow = &Diameters[ (iDiaIndex - iAverage + iConturLen) % iConturLen] ;
    double * pDiaEnd = Diameters + iDiaIndex ;
    double dAverage = 0. ;
    double * pIt = pLow ;
    int iNSamples = 1 + iAverage * 2;
    int iCnt = 0;
    do
    {
      dAverage += *( pIt++ ) ;
      if (pIt >= pDiaEnd)
        pIt -= iDiaIndex ;
    } while ( ++iCnt < iNSamples ) ;
    double dMultiplier = 1. / iNSamples ;
    dAverage *= dMultiplier ;
    
    double * pMinus = pLow ;
    iCnt = 0;
    do 
    {
      if (dAverage < dMinDia)
        dMinDia = dAverage ;
      if (dAverage > dMaxDia)
        dMaxDia = dAverage ;

      dAverage += (*pIt - *pLow) * dMultiplier  ;
      if (++pLow >= pDiaEnd)
        pLow -= iDiaIndex ;
      if (++pIt >= pDiaEnd)
        pIt -= iDiaIndex ;
    } while ( ++iCnt < iNSamples );
  }
  return (dDiaSum / iDiaIndex) ;
}


// for approximate circle or ellipse, don't use for random contur
double GetMinMaxDia( const cmplx * pContur , int iConturLen , cmplx cCenter ,
  double NPointsPerCircle , double& dMinDia , double& dMaxDia , int iAverage )
{
  double Diameters[ 20000 ] ;
  int iOppositeIndex = -1 ;
  int iDiaIndex = 0 ;
  dMinDia = DBL_MAX ;
  dMaxDia = 0. ;
  const cmplx * p = pContur ;
  const cmplx *pIter = p + 1 ;
  const cmplx * pEnd = p + iConturLen ;
  cmplx cVector1 = *p - cCenter ;
  const cmplx * pOppositeCandidate = NULL ;
  double dMinAngle = M_2PI ;
  const cmplx * pMin = NULL ;
  double dMinAngle_deg , /*dMinModAng_deg , */dAng_deg ;
  double dStepBetweenPoints_rad = M_2PI / NPointsPerCircle ;
  if ( iConturLen > NPointsPerCircle )
    iConturLen = ROUND( NPointsPerCircle ) ;
  // 1. Find opposite vector by angle
  do 
  {
    cmplx cVector2 = *pIter - cCenter ;
    double dAngle = arg( -cVector2/cVector1 ) ; // Angle between vectors
    dAng_deg = RadToDeg( dAngle ) ;
    double dAngleMod = abs( dAngle ) ;

    if ( dAngleMod < dMinAngle )
    {
      pMin = pIter ;
      dMinAngle = dAngleMod ;
      dMinAngle_deg = RadToDeg( dMinAngle ) ;
    }
    else if ( dAngleMod - dMinAngle > M_PI_8 )
      break ;
  } while ( ++pIter < pEnd );

  pOppositeCandidate = pMin ;
  p = pContur ;
  double dAngleThres = dStepBetweenPoints_rad / 2. ;
  do 
  {
    cVector1 = ( *p - cCenter ) ;
    cmplx cVector2 = *pOppositeCandidate - cCenter ;
    double dAngle = arg( -cVector2 / cVector1 ) ; // Angle between vectors
    double dAngleMod = abs( dAngle ) ;
    if ( dAngleMod < dAngleThres ) // are point opposite?
    {
      Diameters[ iDiaIndex++ ] = abs( *p - *pOppositeCandidate ) ; // yes, save diameter
      p++ ;
      pOppositeCandidate++ ;
    }
    else // check, may be neighbors are opposite
    {
      if ( dAngle < 0 )
        pOppositeCandidate++ ;
      else
        p++ ;
//       double dAngleModMin = dAngleMod ;
//       dMinModAng_deg = RadToDeg( dAngleModMin ) ;
//       const cmplx * pOppositeCandidateP = pOppositeCandidate + 1 ;
//       while ( pOppositeCandidateP < pEnd )
//       {
//         cmplx cVector2P = *pOppositeCandidateP - cCenter ;
//         double dAngleP = arg( -cVector2P / cVector1 ) ;
//         double dAngleModP = abs( dAngleP ) ;
//         cmplx cVector2PPlus = *(pOppositeCandidateP + 1) - cCenter ;
//         double dAnglePPlus = arg( -cVector2PPlus / cVector1 ) ;
//         double dAngleModPlus = abs( dAnglePPlus ) ;
//         cmplx cVector2PMinus = *(pOppositeCandidateP - 1 ) - cCenter ;
//         double dAnglePMinus = arg( -cVector2PMinus / cVector1 ) ;
//         double dAngleModPMinus = abs( dAnglePMinus ) ;
//         if ( dAngleModP < dAngleModMin )
//         {
//           if ( dAngleModP < dAngleThres )
//           {
//             Diameters[ iDiaIndex++ ] = abs( *p - *pOppositeCandidateP ) ; // yes, save diameter
//             pOppositeCandidate = pOppositeCandidateP ;
//             break ;
//           }
//           dAngleModMin = dAngleModP ;
//           ++pOppositeCandidateP ;
//         }
//         else
//         {
//           pOppositeCandidate = pOppositeCandidateP - 1 ;
//           break ;
//         }
//       }
    }
  } while ( ( p < pMin ) && ( pOppositeCandidate < pEnd ) );
  
  int iDiaLen = iDiaIndex ;
  if ( iAverage <= 0 )
    iAverage = 1 ;
  double dDiaSum = 0. , dLastOut = Diameters[ 0 ] ;
  double dFullDiaSum = 0. ;
  int i = 0 ;
  for ( ; i < iAverage ; i++ )
  {
    dDiaSum += Diameters[ i ] ;
    dFullDiaSum += Diameters[ i ] ;
  }
  dMinDia = dMaxDia = dDiaSum / iAverage ;

  for ( ; i < iDiaLen ; i++ )
  {
    dDiaSum += Diameters[ i ] - dLastOut ;
    dFullDiaSum += Diameters[ i ] ;
    dLastOut = Diameters[ i - iAverage ] ;
    double dAverDia = dDiaSum / iAverage ;
    SetMinMax( dAverDia , dMinDia , dMaxDia ) ;
  }
  for ( ; i < iDiaLen + iAverage ; i++ )
  {
    dDiaSum += Diameters[ i - iDiaLen ] - dLastOut ;
    dLastOut = Diameters[ i - iAverage ] ;
    double dAverDia = dDiaSum / iAverage ;
    SetMinMax( dAverDia , dMinDia , dMaxDia ) ;
  }
  return dFullDiaSum / iDiaLen ;
}


// cmplx GetCenterForFigure( cmplx * pFigure , int iFigLen , ImgMoments& Moments )
// {
//   Moments.Reset() ;
//   cmplx * pEnd = pFigure + iFigLen ;
//   cmplx * p = pFigure ;
//   cmplx cPt = *p ;
//   cmplx cPrev = cPt ;
//   cmplx cSum ;
//   double dAccLen = 0. ;
//   while (++p < pEnd)
//   {
//     cmplx cNext = *p ;
//     cmplx cCent = 0.5 * ( cPrev + cNext ) ;
//     double dLen = cCent.real() - cPt.real() ;/*abs( cNext - cPt ) ;*/
//     double dHeight = abs( cPrev.imag() - cNext.imag() ) ;
//     double dArea = dLen * dHeight ;
//     cSum += dArea * cmplx( dLen * 0.5 , cCent.imag() ) ; // weight center of area  /*dLen * cCent ;*/
//     dAccLen += abs( cNext - cPrev ) ;
//     cPrev = cNext ;
//     Moments.Add( dLen * 0.5 , cCent.imag() , dArea ) ;
//   }
//   return ( dAccLen > EPSILON ) ? cSum / dAccLen : cSum ;
// }


int FindFigurePtOnDist( const CFigure * pFig , int iInitialIndex ,
  int iStep , double dDist )
{
  int iFigLen = (int)pFig->Count() ;
  int iEdgePt = (iInitialIndex + iStep + iFigLen) % iFigLen;
  int iStepOnRing = iStep + iFigLen ;
  cmplx * pCmplx = (cmplx*) pFig->GetData() ;
  cmplx cInitPt = pCmplx[ iInitialIndex ] ;
  while ( abs( pCmplx[ iEdgePt ] - cInitPt ) < dDist )
  {
    iEdgePt = (iEdgePt + iStepOnRing) % iFigLen ;
    if ( iEdgePt == iInitialIndex )
      return -1 ;
  }
  return iEdgePt ;
}

bool GetCrossOfLineAndRectangle( straight_segment& Segment , CDRect& Rect , cmplx& cCross )
{
  cmplx PtLT( Rect.left , Rect.top ) ;
  cmplx PtRT( Rect.right , Rect.top ) ;
  cmplx PtRB( Rect.right , Rect.bottom ) ;
  cmplx PtLB( Rect.left , Rect.bottom ) ;

  cmplx c2ndCross ;

  if ( intersect( PtLT , PtRT , Segment.m_Pt1 , Segment.m_Pt2 , cCross , c2ndCross ) )
    return true ;
  if ( intersect( PtRB , PtRT , Segment.m_Pt1 , Segment.m_Pt2 , cCross , c2ndCross ) )
    return true ;
  if ( intersect( PtLT , PtLB , Segment.m_Pt1 , Segment.m_Pt2 , cCross , c2ndCross ) )
    return true ;
  if ( intersect( PtLB , PtRB , Segment.m_Pt1 , Segment.m_Pt2 , cCross , c2ndCross ) )
    return true ;

  return false ;
}

bool GetCrossOfDirectionAndRectangle( CRect& Rect , double dDir_Rad, cmplx& cCross )
{
  cmplx cFrom( Rect.CenterPoint().x , Rect.CenterPoint().y ) ;
  cmplx PtLT( Rect.left , Rect.top ) ;
  cmplx PtRT( Rect.right , Rect.top ) ;
  cmplx PtRB( Rect.right , Rect.bottom ) ;
  cmplx PtLB( Rect.left , Rect.bottom ) ;

  cmplx cDir = polar( 1. , dDir_Rad ) ;
  cmplx cSecondPt = cFrom + (cDir * 3000.) ;
  cmplx c2ndCrossPt ; //for parallel lines
  cmplx c1stCross , c2ndCross , c3rdCross , c4thCross ;

  bool b1 = intersect( PtLT , PtRT , cFrom , cSecondPt , c1stCross , c2ndCrossPt ) ; // upper edge
  bool b2 = intersect( PtRB , PtRT , cFrom , cSecondPt , c2ndCross , c2ndCrossPt ) ; // right edge
  if ( b1 && b2 )
  {
      cCross = ( abs( c1stCross - cFrom ) < abs( c2ndCross - cFrom ) ) ? c1stCross : c2ndCross ;
      return true ;
  }
  bool b3 = intersect( PtLB , PtLT , cFrom , cSecondPt , c3rdCross , c2ndCrossPt ) ; // left edge
  if ( b3 && b1 )
  {
    cCross = ( abs( c1stCross - cFrom ) < abs( c3rdCross - cFrom ) ) ? c1stCross : c3rdCross ;
    return true ;
  }
  else if ( b1 )
  {
    cCross = c1stCross ;
    return true ;
  }
  bool b4 = intersect( PtLB , PtRB , cFrom , cSecondPt , c4thCross , c2ndCrossPt ) ; // bottom edge

  if ( b4 )
  {
    if ( b2 )
      cCross = ( abs( c1stCross - cFrom ) < abs( c4thCross - cFrom ) ) ? c1stCross : c4thCross ;
    else
      cCross = c4thCross ;
    return true ;
  }
  if ( b2 )
  {
    cCross = c2ndCross ;
    return true ;
  }
  if ( b3 )
  {
    cCross = c3rdCross ;
    return true ;
  }
  return false ;
}

bool ConvertByNearestAndSegment( CoordsCorresp& Nearest ,
  CoordsCorresp& Known2 , CoordsCorresp& Known3 ,
  cmplx& Screen , cmplx& World ) // 
{

  // 1. Find far corner in triangle
  double ToNearest = abs( Screen - Nearest.FOV ) ;
  double ToPt2 = abs( Screen - Known2.FOV ) ;
  double ToPt3 = abs( Screen - Known3.FOV ) ;
  if ( ToNearest < EPSILON )
  {
    World = Nearest.World ;
    return true ;
  }

  // Now we are looking for two nearest calibration pts to Measured pt
  // There are points 1 and 2 (OnLine1 and OnLine2)
  // The third calibration pt called NotOnLine

  CoordsCorresp& OnLine1 = Nearest ;
  CoordsCorresp& OnLine2 = (ToPt2 > ToPt3) ? Known3 : Known2 ;
  CoordsCorresp& NotOnLine = (ToPt2 > ToPt3) ? Known2 : Known3 ;

  // 2. Find cross point between nearest triangle side (pt1 and pt2)
  //    and vector from far corner to measured point

  CLine2d FromFarToMeasured( NotOnLine.FOV , Screen ) ;
  CLine2d Line( OnLine1.FOV , OnLine2.FOV ) ;
  cmplx CrossLineInFOV ;
  if ( !Line.intersect( FromFarToMeasured , CrossLineInFOV ) )
  {
    return false ;
  }

  // 3. Find world coordinates of cross point
  cmplx LineVector = OnLine2.FOV - OnLine1.FOV ;
  double Dist1to2InFOV = abs( LineVector ) ;
  cmplx CrossVector = CrossLineInFOV - OnLine1.FOV ;
  double CrossTo1Dist = abs( CrossVector ) ;
  cmplx CrossToPt2Vector = CrossLineInFOV - OnLine2.FOV ;
  double CrossTo2Dist = abs( CrossToPt2Vector ) ;
  cmplx CrossInWorld ;
  cmplx WorldVect1to2 = OnLine2.World - OnLine1.World ;
  double RatioInFOV = 1.0 ;
  // there is case, when cross pt is out of segment between pt1 and pt2
  // close to pt1
  if ( Dist1to2InFOV < CrossTo2Dist )
  {  // cross out of segment close to pt1
    RatioInFOV = CrossTo1Dist / Dist1to2InFOV ;
    CrossInWorld = OnLine1.World - (WorldVect1to2 * RatioInFOV) ;
  }
  else
  {   // cross inside line segment or cross out of segment close to pt2
    // simple Convert cross point to World coordinate
    RatioInFOV = CrossTo1Dist / Dist1to2InFOV ;
    CrossInWorld = OnLine1.World + (WorldVect1to2 * RatioInFOV) ;
  }

  // 4. Find world coordinates of Measured point
  // There we are using known world coordinates of NotOnLine pt and Cross pt
  cmplx VectFromNotOnLineToCross = CrossLineInFOV - NotOnLine.FOV ;
  double DistFromCrossToNotOnLine = abs( VectFromNotOnLineToCross ) ;
  cmplx VectFromMeasToNotOnLine = Screen - NotOnLine.FOV ;
  double DistFromMeasurdToNotOnLine = abs( VectFromMeasToNotOnLine ) ;
  RatioInFOV = DistFromMeasurdToNotOnLine / DistFromCrossToNotOnLine ;
  cmplx VectFromNotOnLineToCrossInWorld = CrossInWorld - NotOnLine.World ;
  World = NotOnLine.World + VectFromNotOnLineToCrossInWorld * RatioInFOV ;

  return true ;
}

inline int GetMinMaxDistances( cmplx * pFigure , int iFigLen ,
  cmplx cCenter , ImgMoments& Moments ,
  CIntArray& Maxes , CIntArray& Mins , double dMinRelDiff ,
  int iWindowSize )
{
  int NExtremums = 0 ;
  CDblArray Distances ;
  for ( int i = 0; i < iFigLen; i++ )
  {
    double dDist = abs( pFigure[ i ] - cCenter ) ;
    Distances.Add( dDist ) ;
  }

  Mins.RemoveAll() ;
  Maxes.RemoveAll() ;

  double dMin = DBL_MAX ;
  double dMax = -DBL_MAX ;
  CDblArray AveragedDists ;
  int iHalfWindow = iWindowSize / 2 ;
  int i = -iHalfWindow ;
  double dAccum = 0 ;
  for ( ; i < iWindowSize / 2; i++ )
    dAccum = Distances[ (i + iFigLen) % iFigLen ] ;
  for ( int j = 0 ; j < iFigLen ; )
  {
    AveragedDists[ j++ ] = dAccum / iWindowSize ;
    dAccum += Distances[ (j + iHalfWindow + iFigLen) % iFigLen ]
      - Distances[ (j - iHalfWindow + iFigLen) % iFigLen ] ;
    SetMinMax( dAccum , dMin , dMax ) ;
  }
  double dAmpl = dMax - dMin ;
  double dRelAmpl = dAmpl * 2. / (dMin + dMax) ;
  if ( dRelAmpl < dMinRelDiff * 0.5 )
    return 0 ;
  double dThres = 0.5 * (dMin + dMax) ;
#define RingIndex(i) ((i+AveragedDists.GetCount()) % AveragedDists.GetCount())
#define AAmpl(j) (AveragedDists[RingIndex(j)])
  double dLocalMin = AveragedDists[ 0 ] ;
  double dLocalMax = dLocalMin ;
  double dCurrVal = dLocalMin ;
  double dNextVal  ;
  bool bHigh = (dCurrVal > dThres) ;
  int iIndex = 1 ;
  int iZoneBegin = -1 ;
  int iBackZoneBegin = -1 ;
  int iLocalMaxIndex = 0 , iLocalMinIndex = 0 ;
  while ( iIndex < AveragedDists.GetCount() )
  {
    while ( bHigh && iIndex < AveragedDists.GetCount() )
    {
      dNextVal = AAmpl( iIndex )  ;
      if ( dNextVal >= dThres )
      {
        if ( dLocalMax < dCurrVal )
        {
          dLocalMax = dCurrVal ;
          iLocalMaxIndex = iIndex - 1 ;
        }
        iIndex++ ;
        dCurrVal = dNextVal ;
        continue ;
      }
      else
      {
        if ( iZoneBegin >= 0 )
        {
          Maxes.Add( iLocalMaxIndex ) ;
          iZoneBegin = iIndex ;
          dCurrVal = dNextVal ;
          bHigh = false ;
        }
        else // case, of scan begin
        {
          int iBackIndex = -1 ;
          dNextVal = AAmpl( iBackIndex )  ;
          while ( dNextVal >= dThres )
          {
            if ( dLocalMax < dNextVal )
            {
              dLocalMax = dNextVal ;
              iLocalMaxIndex = iBackIndex ;
            }
            iBackIndex-- ;
            continue ;
          }
          Maxes.Add( iLocalMaxIndex ) ;
          iBackZoneBegin = iBackIndex ;
          bHigh = false ;
        }
      }
    }
    dLocalMin = dCurrVal ;
    iLocalMinIndex = iIndex ;
    while ( !bHigh && iIndex < AveragedDists.GetCount() )
    {
      dNextVal = AAmpl( iIndex )  ;
      if ( dNextVal < dThres )
      {
        if ( dLocalMin > dCurrVal )
        {
          dLocalMin = dCurrVal ;
          iLocalMinIndex = iIndex - 1 ;
        }
        iIndex++ ;
        dCurrVal = dNextVal ;
        continue ;
      }
      else
      {
        if ( iZoneBegin >= 0 )
        {
          Mins.Add( iLocalMinIndex ) ;
          iZoneBegin = iIndex ;
          dCurrVal = dNextVal ;
          bHigh = true ;
        }
        else // case, of scan begin
        {
          int iBackIndex = -1 ;
          dNextVal = AAmpl( iBackIndex )  ;
          while ( dNextVal < dThres )
          {
            if ( dLocalMin < dNextVal )
            {
              dLocalMin = dNextVal ;
              iLocalMinIndex = iBackIndex ;
            }
            iBackIndex-- ;
            continue ;
          }
          Maxes.Add( iLocalMinIndex ) ;
          iBackZoneBegin = iBackIndex ;
          bHigh = true ;
        }
      }
    }
    dLocalMax = dCurrVal ;
    iLocalMaxIndex = iIndex ;
  }
  if ( Maxes[ 0 ] < 0 )
  {
    Maxes.Add( Maxes[ 0 ] + (int) AveragedDists.GetCount() ) ;
    Maxes.RemoveAt( 0 ) ;
  }
  if ( Mins[ 0 ] < 0 )
  {
    Mins.Add( Mins[ 0 ] + (int) AveragedDists.GetCount() ) ;
    Mins.RemoveAt( 0 ) ;
  }

  return (int) (Maxes.GetCount() + Mins.GetCount()) ;
}

int RemoveSmallOutGrowths( CFigure& Orig , CFigure& Result ,
  int iNeckWidth , int iNLookForward )
{

  return 0 ;
}

