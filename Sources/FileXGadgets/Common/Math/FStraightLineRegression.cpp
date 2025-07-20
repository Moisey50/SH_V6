// FRegression.cpp: implementation of the CFRegression class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include <gadgets/figureframe.h>
#include "Math/FRegression.h"
#include "FStraightLineRegression.h"
#include "Math/PlaneGeometry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// function extracts from figure segment with 
// central point near cCentPt with all hearest points in range 
// dRangeFromCenter and calculate regression 

double StraightLineRegression::GetAndCalc( const CFigure * pFig ,
  cmplx cCentPt , double dAngleFromCenter ,
  double dRangeFromCenter , double dLineDirectionAngle , 
  cmplx* pFirst , cmplx * pSecond , bool bYCW)
{
  if ( !pFig )
  {
    return 0.;
  }
  m_cCenter = cCentPt ;
  cmplx * pData = (cmplx*) pFig->GetData() ;
  m_iCentIndex = FindNearestToDirection( pFig , cCentPt , dAngleFromCenter ) ;
  int iOneSideIndex = FindFigurePtOnDist( pFig , m_iCentIndex , 1 , dRangeFromCenter ) ;
  int iOppositSideIndex = FindFigurePtOnDist( pFig , m_iCentIndex , -1 , dRangeFromCenter ) ;
  cmplx cFirst = pData[ iOneSideIndex ] ;
  cmplx cSecond = pData[ iOppositSideIndex ] ;
  cmplx cVect = cSecond - cFirst ;
  if ( pFirst )
    *pFirst = cFirst ;
  if ( pSecond )
    *pSecond = cSecond ;

  cmplx LineDir = polar(1., dLineDirectionAngle);
  cmplx VectDiv = cVect / LineDir;
  double dAng1 = arg(VectDiv) ;
  if ( (abs( dAng1 ) < M_PI_4) ^ bYCW )
  {
    m_iLowerIndex = iOneSideIndex ;
    m_iUpperIndex = iOppositSideIndex ;
  }
  else
  {
    if ( pFirst && pSecond )
      swapcmplx(*pFirst, *pSecond);
    m_iLowerIndex = iOppositSideIndex ;
    m_iUpperIndex = iOneSideIndex ;
  }

  if (m_iUpperIndex == pFig->GetCount() - 1)
    m_iUpperIndex--;

  Reset() ;
  AddPtsToRegression( pData , (int)pFig->GetCount() , m_iLowerIndex , m_iUpperIndex ) ;
  Calculate() ;
  
  m_dStd = GetStdFromLine(
    m_Pts.data() , (int)m_Pts.size() , -1 , -1 , &m_Dists ) ;
  return m_dStd ;
}

double StraightLineRegression::DoFiltering( double dThresInStdUnits )
{
  CFilterRegression RegrFiltered ;
  for ( FXSIZE i = 0 ; i < m_Dists.Count() ; i++ )
  {
    if ( fabs( m_Dists[ i ] ) < m_dStd * dThresInStdUnits )
      RegrFiltered.Add( m_Pts[ i ] ) ;
  }
  RegrFiltered.Calculate() ;
  m_FilteredLine = RegrFiltered.GetCLine2d() ;
  m_dFa = RegrFiltered.m_da ;
  m_dFb = RegrFiltered.m_db ;
  m_dFaY = RegrFiltered.m_daY;
  m_dFbY = RegrFiltered.m_dbY;

  m_dStdFiltered = GetStdFromLine( 
    m_Pts.data() , (int)m_Pts.size() , -1 , -1 , &RegrFiltered.m_Dists) ;
//   for ( FXSIZE i = 0 ; i < RegrFiltered.m_Pts.Count() ; i++ )
//   {
//     double dDistToLine = m_FilteredLine.GetDistFromPoint( m_Pts[ i ] ) ;
//     RegrFiltered.m_Dists.Add( dDistToLine ) ;
//     m_dStdFiltered += dDistToLine * dDistToLine ;
//   }
//   if ( m_Pts.Count() )
//   {
//     m_dStdFiltered = m_dStdFiltered / m_Pts.Count() ;
//     m_dStdFiltered = sqrt( m_dStdFiltered ) ;
//   }
  return m_dStdFiltered ;
}

double StraightLineRegression::DoOneSideFiltering( bool bSelectFromCenter )
{
  CLine2d AsClassic = GetCLine2d() ;
  double dDistToCenter = AsClassic.GetDistFromPoint( m_cCenter ) ;
  bool bFromCenter = (dDistToCenter >= 0.) ^ bSelectFromCenter ;
  CFilterRegression RegrFiltered ;
  for ( FXSIZE i = 0 ; i < m_Dists.Count() ; i++ )
  {
    if ( (m_Dists[ i ] < 0.) ^ bFromCenter )
      RegrFiltered.Add( m_Pts[ i ] ) ;
  }
  RegrFiltered.Calculate() ;
  m_FilteredLine = RegrFiltered.GetCLine2d() ;
  m_dFa = RegrFiltered.m_da ;
  m_dFb = RegrFiltered.m_db ;
  m_dFaY = RegrFiltered.m_daY;
  m_dFbY = RegrFiltered.m_dbY;

  m_dStdFiltered = GetStdFromFilteredLine( RegrFiltered ) ;
//   for ( FXSIZE i = 0 ; i < RegrFiltered.m_Pts.Count() ; i++ )
//   {
//     double dDistToLine = m_FilteredLine.GetDistFromPoint( RegrFiltered.m_Pts[ i ] ) ;
//     RegrFiltered.m_Dists.Add( dDistToLine ) ;
//     m_dStdFiltered += dDistToLine * dDistToLine ;
//   }
//   if ( RegrFiltered.m_Pts.Count() )
//   {
//     m_dStdFiltered = m_dStdFiltered / RegrFiltered.m_Pts.Count() ;
//     m_dStdFiltered = sqrt( m_dStdFiltered ) ;
//   }
  return m_dStdFiltered ;
}

double StraightLineRegression::DoOneSideFiltering(
  bool bSelectFromCenter , int iNIntervals ,
  CFilterRegression& RegrFiltered , CmplxVector * pEdgePts )
{
  CLine2d AsClassic = GetCLine2d() ;
  double dDistToCenter = AsClassic.GetDistFromPoint( m_cCenter ) ;
  bool bFromCenter = (dDistToCenter >= 0.) ^ bSelectFromCenter ;

  double dNPtsPerInterval = m_Pts.size() / (double) iNIntervals ;
  int iIter = 0 ;
  double dNextStop = dNPtsPerInterval - 1 ;
  CFilterRegression LocalRegr;
  for ( int iIntCount = 0 ; iIntCount < iNIntervals ; iIntCount++ )
  {
    if ( pEdgePts )
      pEdgePts->push_back( m_Pts[ iIter ] );

    double dFar = -DBL_MAX ;
    int iFarestIndex = -1 ;
    while ( iIter < dNextStop )
    {
      if ( (m_Dists[ iIter ] < 0.) ^ bFromCenter )
      {
        double dAbs = fabs( m_Dists[ iIter ] ) ;
        if ( dAbs > dFar )
        {
          dFar = dAbs ;
          iFarestIndex = iIter ;
        }
      }
      iIter++ ;
    }
    if ( iFarestIndex >= 0 )
      LocalRegr.Add( m_Pts[ iFarestIndex ] ) ;
    dNextStop += dNPtsPerInterval ;
    if ( dNextStop > m_Pts.size() - 1. )
      dNextStop = m_Pts.size() - 1. ;
  }
  LocalRegr.Calculate() ;
  m_FilteredLine = LocalRegr.GetCLine2d() ;
  m_dFa = LocalRegr.m_da ;
  m_dFb = LocalRegr.m_db ;
  m_dFaY = LocalRegr.m_daY;
  m_dFbY = LocalRegr.m_dbY;
  IntVector IsGoodSide;
  vector<double> DistsFromLine;
  double dStd2 = 0.;
  for ( size_t i = 0 ; i < LocalRegr.m_Pts.size() ; i++ )
  {
    double dDistToLine = m_FilteredLine.GetDistFromPoint( LocalRegr.m_Pts[ i ] ) ;
    DistsFromLine.push_back( dDistToLine );
    dStd2 += dDistToLine * dDistToLine;
    if ( !((dDistToLine > 0.) ^ bFromCenter) )
      IsGoodSide.push_back( (int) i );
  }
  double dStd = sqrt( dStd2 );
  if ( IsGoodSide.size() >= LocalRegr.m_Pts.size() / 2 )
  {
    for ( size_t i = 0 ; i < LocalRegr.m_Pts.size() ; i++ )
    {
      if ( fabs( DistsFromLine[ i ] ) <= dStd * 1.5 )
        RegrFiltered.Add( LocalRegr.m_Pts[ i ] );
    }
  }
  else
  {
    for ( size_t i = 0; i < IsGoodSide.size() ; i++ )
      RegrFiltered.Add( LocalRegr.m_Pts[ IsGoodSide[ i ] ] );
  }
  RegrFiltered.Calculate() ;
  m_FilteredLine = RegrFiltered.GetCLine2d() ;
  m_dFa = RegrFiltered.m_da ;
  m_dFb = RegrFiltered.m_db ;
  m_dFaY = RegrFiltered.m_daY;
  m_dFbY = RegrFiltered.m_dbY;

  m_dStdFiltered = GetStdFromFilteredLine( RegrFiltered ) ;
  return m_dStdFiltered ;
}

double StraightLineRegression::DoFilteringByDeviation( 
  double dMaxDeviation , CFilterRegression& RegrFiltered , 
  int& iNGood )
{
  CLine2d AsClassic = GetCLine2d() ;
  vector<bool> bIsGood1st( m_Pts.size() ) ;
  vector<bool> bIsGood2nd( m_Pts.size() ) ;

  iNGood = 0 ;
  auto BoolIt = bIsGood1st.begin() ;
  for ( auto It = m_Pts.begin() ; It != m_Pts.end() ; BoolIt++ , It++ )
  {
    double dDist = AsClassic.GetDistFromPoint( *It );
    if ( *BoolIt = (dDist <= dMaxDeviation) )
    {
      iNGood += (int)(*BoolIt) ;
      RegrFiltered.Add( *It ) ;
    }
  }
  RegrFiltered.Calculate() ;
  m_FilteredLine = RegrFiltered.GetCLine2d() ;

  RegrFiltered.Reset() ;
  auto BoolIt2 = bIsGood2nd.begin() ;
  for ( auto It = m_Pts.begin() ; It != m_Pts.end() ; BoolIt2++ , It++ )
  {
    double dDist = m_FilteredLine.GetDistFromPoint( *It );
    if ( *BoolIt2 = (dDist <= dMaxDeviation * 2. ) )
      RegrFiltered.Add( *It ) ;
  }
  RegrFiltered.Calculate() ;
  m_FilteredLine = RegrFiltered.GetCLine2d() ;

  m_dStdFiltered = GetStdFromFilteredLine( RegrFiltered ) ;
  return m_dStdFiltered ;
}

double StraightLineRegression::GetStdFromFilteredLine( CFilterRegression& RegrFiltered )
{
  if ( !RegrFiltered.m_Pts.size() )
    return 0. ;

  double dStdFiltered = 0. ;
  for (size_t i = 0 ; i < RegrFiltered.m_Pts.size() ; i++ )
  {
    double dDistToLine = m_FilteredLine.GetDistFromPoint( RegrFiltered.m_Pts[ i ] ) ;
    RegrFiltered.m_Dists.Add( dDistToLine ) ;
    dStdFiltered += dDistToLine * dDistToLine ;
  }
  dStdFiltered = dStdFiltered / RegrFiltered.m_Pts.size() ;
  dStdFiltered = sqrt( dStdFiltered ) ;
  return dStdFiltered ;
}

double CFRegression::GetStdFromLine( const cmplx * pData , 
  int iDataLen , int iBeginIndex , int iEndIndex , FXDblArray * pDists )
{
  if ( iBeginIndex < 0 )
    iBeginIndex = 0 ;
  if ( iEndIndex  < 0 )
    iEndIndex = iDataLen - 1 ;
  if ( iBeginIndex >= iDataLen || iEndIndex >= iDataLen )
    return 0. ;
  CLine2d AsClassic = GetCLine2d() ;
  double dStd = 0. ;
  int iNPts = 0 ;
  if (pDists)
    pDists->RemoveAll();

  for ( int i = iBeginIndex ; i != iEndIndex ; i = ((++i) % iDataLen) )
  {
    double dDistToLine = AsClassic.GetDistFromPoint( pData[i] ) ;
    if (pDists)
      pDists->Add(dDistToLine);
    dStd += dDistToLine * dDistToLine ;
    iNPts++ ;
  }
  dStd = sqrt( dStd/iNPts ) ;
  return dStd ;
}

double StraightLineRegression::DoFilterFarestFromLine(
  CLine2d& Line , double dTolerance_pix ,
  CFilterRegression& RegrFiltered , CmplxVector * pEdgePts )
{
  CLine2d AsClassic = GetCLine2d() ;
  
  CFilterRegression LocalRegr;
  double dFar1st = 0. , dFar2nd = 0. ;
  int i1stIndex = -1 , i2ndIndex = -1 ;
  cmplx c1st , c2nd ;
  int OneThird = ROUND( m_Pts.size() / 3. ) ;
  for ( auto It = m_Pts.begin() ; It - m_Pts.begin() <= OneThird ; It++ )
  {
    double dDist = fabs( Line.GetDistFromPoint( *It ) ) ;
    if ( dFar1st < dDist )
    {
      dFar1st = dDist ;
      i1stIndex = (int) (It - m_Pts.begin()) ;
      c1st = *It ;

    }
  }
  int TwoThirds = ROUND( 2. * m_Pts.size() / 3. ) ;

  for ( auto It = m_Pts.begin() + TwoThirds ; It < m_Pts.end() ; It++ )
  {
    double dDist = fabs( Line.GetDistFromPoint( *It ) ) ;
    if ( dFar2nd < dDist )
    {
      dFar2nd = dDist ;
      i2ndIndex = (int) (It - m_Pts.begin()) ;
      c2nd = *It ;
    }
  }
  if ( pEdgePts )
  {
    pEdgePts->push_back( c1st ) ;
    pEdgePts->push_back( c2nd ) ;
  }

  CLine2d FarEdge( c1st , c2nd ) ;

  RegrFiltered.Reset() ;
  double dDistLimit = 2. * dTolerance_pix ;
  for ( auto It = m_Pts.begin() ; It != m_Pts.end() ; It++  )
  {
    double dDistFromFarLine = FarEdge.GetDistFromPoint( *It ) ;
    if ( fabs( dDistFromFarLine ) <= dDistLimit )
    {
      RegrFiltered.Add( *It ) ;
      if ( pEdgePts )
        pEdgePts->push_back( *It ) ;
    }
  }
  RegrFiltered.Calculate() ;
  m_FilteredLine = RegrFiltered.GetCLine2d() ;

  m_dStdFiltered = GetStdFromFilteredLine( RegrFiltered ) ;
  return m_dStdFiltered ;
}

