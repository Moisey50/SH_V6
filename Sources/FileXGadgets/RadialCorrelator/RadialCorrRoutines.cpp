// RadialCorrGadget.h : Implementation of the RadialCorr class


#include "StdAfx.h"
#include "RadialCorrGadget.h"

double RadialCorr::BuildCorrFunction( 
  double * pPattern ,
  int iPattLen , double dPattSigma ,
  double * pSignal , int iSignalLen ,
  double * pResult , int& iMinPos,
  int& iMaxPos , double& dSignSigma2 )
{
  if ( iPattLen >= iSignalLen )
    return 0. ;

  int iResultLen = iSignalLen - iPattLen ;
  double dMax = -1e30 ;
  double dMin = 1e30 ;
  int iCmpStat = 0 , iPrevCmpStat = 0 ;
  m_iMaxInd = m_iMinInd = 0 ;
  double dPrevVal ;
  dSignSigma2 = 0 ;
  for ( int i = 0 ; i < iResultLen ; i++ )
  {
    double dSigma2 ;
    double dR = pResult[i] = Correlator( pPattern , iPattLen ,
      dPattSigma , pSignal + i , dSigma2 ) ;
    if ( dSigma2 > dSignSigma2 )
      dSignSigma2 = dSigma2 ;
    m_CorrFunc[i + m_iMinRadius] = dR ;
    if ( dR > dMax )
    {
      iMaxPos = i ;
      dMax = dR ;
    }
    if ( dR < dMin )
    {
      iMinPos = i ;
      dMin = dR ;
    }
    if ( i > 0 )
    {
      iCmpStat = ( dR > dPrevVal ) ;
      if ( !iCmpStat )
        iCmpStat = (dR == dPrevVal ) << 1 ;

      if ( i > 1 )
      {
        if ( iPrevCmpStat ) // prev was grow or equal
        {
          if (i > 2)
          {
            if ( !(iCmpStat & 1) )
            {
              if ( m_iMaxInd >= MIN_MAX_ARRAY_SIZE )
                return 0. ;
              m_LocalMax[ m_iMaxInd++ ] = i - 1 ;
            }
          }
        }
        else if (i > 2) 
        {
          if ( iCmpStat & 1 )
          {
            if ( m_iMinInd >= MIN_MAX_ARRAY_SIZE )
              return 0. ;
            m_LocalMin[ m_iMinInd++ ] = i - 1 ;
          }
        }
        iPrevCmpStat = iCmpStat ;
      }
    }
    dPrevVal = dR ;
  }
  double dAmpl = dMax - dMin ;
  int iMaxMinInd = 0 , iMaxMaxInd = 0 ;
  if ( m_iMinInd && m_iMaxInd ) // we have maximums and minimums 
  {
    int iMaxIndex = min(m_iMinInd , m_iMaxInd) ;
    double dMaxAmpl = pResult[m_LocalMax[0]] - pResult[m_LocalMin[0]] ;
    if ( m_LocalMin[0] > m_LocalMax[0] )
    {
      for ( int i = 0 ; i < iMaxIndex ; i++ )
      {
        double dAmpl1 = pResult[m_LocalMax[i]] - pResult[m_LocalMin[i]] ;
        if (i+1 < m_iMaxInd)
        {
          double dAmpl2 = pResult[m_LocalMax[i+1]] - pResult[m_LocalMin[i]] ;
          if ( (dAmpl2 > dAmpl1)    )
          {
            if (dAmpl2 > dMaxAmpl) 
            {
              iMaxMinInd = i ;
              iMaxMaxInd = i + 1 ;
              dMaxAmpl = dAmpl2 ;
            }
          }
          else if ( dAmpl1 > dMaxAmpl )
          {
            {
              iMaxMinInd = i ;
              iMaxMaxInd = i ;
              dMaxAmpl = dAmpl1 ;
            }
          }
        }
      }
    }
    else
    {
      for ( int i = 0 ; i < iMaxIndex ; i++ )
      {
        double dAmpl1 = pResult[m_LocalMax[i]] - pResult[m_LocalMin[i]] ;
        if (i+1 < m_iMinInd)
        {
          double dAmpl2 = pResult[m_LocalMax[i]] - pResult[m_LocalMin[i+1]] ;
          if ( (dAmpl2 > dAmpl1)    )
          {
            if (dAmpl2 > dMaxAmpl) 
            {
              iMaxMinInd = i + 1 ;
              iMaxMaxInd = i ;
              dMaxAmpl = dAmpl2 ;
            }
          }
          else if ( dAmpl1 > dMaxAmpl )
          {
            {
              iMaxMinInd = i ;
              iMaxMaxInd = i ;
              dMaxAmpl = dAmpl1 ;
            }
          }
        }
      }
    }
    dAmpl = dMaxAmpl ;
    m_iLastMinPos = iMinPos = m_LocalMin[ iMaxMinInd ] ;
    m_iLastMaxPos = iMaxPos = m_LocalMax[ iMaxMaxInd ] ;
  }
  else
  {
    m_iLastMinPos = iMinPos ;
    m_iLastMaxPos = iMaxPos ;
  }

  return dAmpl ;
}

double RadialCorr::BuildCorrFunction( 
  double * pPattern ,
  int iPattLen , double dPattSigma ,
  double * pSignal , int iSignalLen ,
  int iInd , double& dSignSigma2 )
{
  double dRes = BuildCorrFunction( 
    pPattern , iPattLen , dPattSigma , pSignal , iSignalLen ,
    m_CorrData[iInd].m_pCorrFunc , 
    m_CorrData[iInd].m_Pt1.m_iNPos , m_CorrData[iInd].m_Pt1.m_iPPos , 
    dSignSigma2 ) ;

  if ( dRes > 0. )
  {
    m_CorrData[iInd].m_Pt1.m_dMaxNCorr = 
      m_CorrData[iInd].m_pCorrFunc[m_CorrData[iInd].m_Pt1.m_iNPos] ;
    m_CorrData[iInd].m_Pt1.m_dMaxPCorr = 
      m_CorrData[iInd].m_pCorrFunc[m_CorrData[iInd].m_Pt1.m_iPPos] ;
    m_CorrData[iInd].m_Pt1.m_dMaxSignSigma2 = dSignSigma2 ;
  }
  return dRes ;
}

double RadialCorr::BuildCorrFunction(
  double * pPattern ,
  int iPattLen , double dPattSigma ,
  double * pSignal , int iSignalLen ,
  double * pResult , int& iMinPos, int& iMaxPos , 
  int iPrevMin , int iPrevMax , int iEpsilon , double& dSignSigma2 )
{
  if ( iPattLen >= iSignalLen )
    return 0. ;

  double dMax = -1e30 ;
  double dMin = 1e30 ;
  int iResultLen = iSignalLen - iPattLen ;
  dSignSigma2 = 0. ;
  for ( int i = 0 ; i < iResultLen ; i++ )
  {
    double dSigma2 ;
    double dR = pResult[i] = Correlator( pPattern , iPattLen ,
      dPattSigma , pSignal + i , dSigma2 ) ;
    if ( dSigma2 > dSignSigma2 )
      dSignSigma2 = dSigma2 ;
    m_CorrFunc[i + m_iMinRadius + m_iStartCorrelPos] = dR ;
    if ( i >= (iPrevMax - iEpsilon) 
      && i <= (iPrevMax + iEpsilon) )
    {
      if ( dR > dMax )
      {
        iMaxPos = i ;
        dMax = dR ;
      }
    }
    if ( i >= (iPrevMin - iEpsilon) 
      && i <= (iPrevMin + iEpsilon) )
    {
      if ( dR < dMin )
      {
        iMinPos = i ;
        dMin = dR ;
      }
    }
  }

  return dMax - dMin ;
}

double RadialCorr::BuildCorrFunction(
  double * pPattern ,
  int iPattLen , double dPattSigma ,
  double * pSignal , int iSignalLen ,
  int iInd ,
  int iPrevMin , int iPrevMax , int iEpsilon , double& dSignSigma2 )
{

  double dRes = BuildCorrFunction(
    pPattern , iPattLen , dPattSigma , pSignal , iSignalLen ,
    m_CorrData[iInd].m_pCorrFunc , 
    m_CorrData[iInd].m_Pt1.m_iNPos , m_CorrData[iInd].m_Pt1.m_iPPos ,
    iPrevMin , iPrevMax , iEpsilon , dSignSigma2 ) ;
  if ( dRes > 0. )
  {
    m_CorrData[iInd].m_Pt1.m_dMaxNCorr = 
      m_CorrData[iInd].m_pCorrFunc[m_CorrData[iInd].m_Pt1.m_iNPos] ;
    m_CorrData[iInd].m_Pt1.m_dMaxPCorr = 
      m_CorrData[iInd].m_pCorrFunc[m_CorrData[iInd].m_Pt1.m_iPPos] ;
    m_CorrData[iInd].m_Pt1.m_dMaxSignSigma2 = dSignSigma2 ;
  }
  return dRes ;
}



double RadialCorr::BuildCorrFunction(  cmplx Cent , int iIndex ,
        double * pSignal , const pTVFrame pFrame , bool bFirst )
{
  double dAngle_rad = DegToRad(m_CorrData[iIndex].m_dAngle) ;
  m_iSignalLen = GetRadiusPixels( Cent , -dAngle_rad ,
    m_iMinRadius , m_iRealMaxRadius , m_Signal , pFrame ) ;
  int iCorrFuncLen = m_iSignalLen - m_iPatternLen ;
  m_CorrData[iIndex].bCheckAndRealloc( iCorrFuncLen ) ;
  double * pCorrFunc = m_CorrData[iIndex].m_pCorrFunc ;
  if ( m_iSignalLen <= m_iPatternLen )
    return 0. ;
  if ( bFirst )
  {
    m_dCorrAmpl = BuildCorrFunction( m_Pattern , m_iPatternLen , 
      m_dGaussSigma_pix , m_Signal , m_iSignalLen ,
      iIndex , m_CorrData[iIndex].m_Pt1.m_dMaxSignSigma2 ) ;
  }
  else
  {
    m_iStartCorrelPos = min( m_iMaxPos , m_iMinPos )
      - ((m_iGaussHalfWidth_pix * 2) / 2) ;
    if ( m_iStartCorrelPos < 0 )
      m_iStartCorrelPos = 0 ;
    m_iStopCorrelPos = max( m_iMaxPos , m_iMinPos ) 
      + ((m_iGaussHalfWidth_pix * 2) / 2) ;
    if ( m_iStopCorrelPos > m_iSignalLen - m_iPatternLen )
      m_iStopCorrelPos = m_iSignalLen - m_iPatternLen ;
    m_dCorrAmpl = BuildCorrFunction( m_Pattern , m_iPatternLen , 
      m_dGaussSigma_pix , m_Signal + m_iStartCorrelPos , 
      m_iStopCorrelPos - m_iStartCorrelPos + m_iPatternLen , 
      iIndex , 
      m_iMinPos - m_iStartCorrelPos , m_iMaxPos - m_iStartCorrelPos , 
      m_iSecondaryMeasZone , m_CorrData[iIndex].m_Pt1.m_dMaxSignSigma2) ;
    m_iLastMinPos = m_CorrData[iIndex].m_Pt1.m_iNPos + m_iStartCorrelPos ;
    m_iLastMaxPos = m_CorrData[iIndex].m_Pt1.m_iPPos + m_iStartCorrelPos ;
  }

  if ( m_dCorrAmpl > 0. )
  {
    cmplx MaxPt = Cent + polar( 
      double(m_iMinRadius + m_CorrData[iIndex].m_Pt1.m_iPPos 
      + m_iStartCorrelPos + m_iGaussHalfWidth_pix) , 
      -dAngle_rad ) ;
    m_CorrData[iIndex].m_Pt1.m_MaxPt = MaxPt ;
    cmplx MinPt = Cent + polar( 
      double(m_iMinRadius + m_CorrData[iIndex].m_Pt1.m_iNPos 
      + m_iStartCorrelPos + m_iGaussHalfWidth_pix) , 
      -dAngle_rad ) ;
    m_CorrData[iIndex].m_Pt1.m_MinPt = MinPt ;
  }
  if ( m_dCorrAmpl > m_dMaxAmpl )
    m_dMaxAmpl = m_dCorrAmpl ;
  return m_dCorrAmpl ;
}

double RadialCorr::GetSigma2OnSide(  cmplx Cent , cmplx PtOnContour ,
  const pTVFrame pFrame , double& dSigma2 , cmplx& CheckPoint ,
  cmplx& Pt1 , cmplx& Pt2)
{
  cmplx VectorToCent = (PtOnContour - Cent ) ;
  CheckPoint = Cent + VectorToCent * 1.3 ;
  cmplx OrthoStep = polar( 1.0 , arg(VectorToCent) + M_PI2 ) ;
  double dOrthoDist = 35. ;
  do 
  {
    if ( (--dOrthoDist) <= 10 )
      return 0. ;
    Pt1 = CheckPoint + (OrthoStep * dOrthoDist) ;
    Pt2 = CheckPoint - (OrthoStep * dOrthoDist) ;
  } while ( !IsPtInFrame( Pt1 , pFrame ) || !IsPtInFrame( Pt2 , pFrame ) ) ;

  m_iSignalLen = GetPixelsOnLine( Pt1 , Pt2 ,
    m_Signal , 1000 , pFrame ) ;
  dSigma2 = GetSigma2( m_Signal , m_iSignalLen ) ;
  return dSigma2 ;
}

double RadialCorr::BuildCorrFunction(  cmplx Cent , cmplx PtOnContour ,
  const pTVFrame pFrame , double& dSigma2 , cmplx& CheckPoint ,
  cmplx& Pt1 , cmplx& Pt2)
{
  cmplx VectorToCent = (PtOnContour - Cent ) ;
  CheckPoint = Cent + VectorToCent * 1.3 ;
  cmplx OrthoStep = polar( 1.0 , arg(VectorToCent) + M_PI2 ) ;
  double dOrthoDist = 35. ;
  do 
  {
    if ( (--dOrthoDist) <= 10 )
      return 0. ;
    Pt1 = CheckPoint + (OrthoStep * dOrthoDist) ;
    Pt2 = CheckPoint - (OrthoStep * dOrthoDist) ;
  } while ( !IsPtInFrame( Pt1 , pFrame ) || !IsPtInFrame( Pt2 , pFrame ) ) ;

  m_iSignalLen = GetPixelsOnLine( Pt1 , Pt2 ,
    m_Signal , 1000 , pFrame ) ;
  if ( m_iSignalLen <= m_iPatternLen )
    return 0. ;
  m_dCorrAmpl = BuildCorrFunction( m_Pattern , m_iPatternLen , 
    m_dGaussSigma_pix , m_Signal , m_iSignalLen ,
    m_CorrFunc , m_iLastMinPos , m_iLastMaxPos ,
    m_dLastSigma2 ) ;
  dSigma2  = m_dLastSigma2 ;
  return m_dCorrAmpl ;
}

cmplx RadialCorr::GetFarestPoint( cmplx& LinePt1 , cmplx& LinePt2 ,
                                 int iFirstIndex , int iLastIndex )
{
  double dMaxDist = 0 ;
  cmplx FoundPt ;
  iFirstIndex %= 360 ;
  iLastIndex %= 360 ;
  for ( int i = iFirstIndex ; i != iLastIndex ; i++ )
  {
    i = Norm360( i ) ;
    if ( m_CorrData[i].m_Pt1.m_iZeroPt == 0 )
      continue ;
    double dDist = GetPtToLineDistance( m_CorrData[i].m_Pt1.m_ZeroPt ,
      LinePt1 , LinePt2 ) ;
    dDist = fabs(dDist) ;
    if ( dDist > dMaxDist )
    {
      dMaxDist = dDist ;
      FoundPt = m_CorrData[i].m_Pt1.m_ZeroPt ;
    }
  }
  return FoundPt ;
}



int RadialCorr::GetSlices( cmplx Cent , double dCellAngle , 
                          int iFirst , int iNSlices )
{
  cmplx MainAxisDir = polar( 500. , -dCellAngle ) ;  // minus, because coord system
  cmplx Extr1 = Cent + MainAxisDir ;
  cmplx Extr2 = Cent - MainAxisDir ;
  CLine2d MainAxis( Extr1 , Extr2 ) ;

  cmplx CrossPt1 , CrossPt2 ;
  int i = iFirst - 16 ;
  while ( (m_CorrData[ Norm360(i) ].m_Pt1.m_iZeroPt == 0)
    && (i < (iFirst + 16)) ) 
    i++ ;
  cmplx SegmPt1 = m_CorrData[ Norm360(i) ].m_Pt1.m_ZeroPt ;
  i++ ;
  for (  ; i < iFirst + 16 ; i++ )
  {
    while ( (m_CorrData[ Norm360(i) ].m_Pt1.m_iZeroPt == 0)
      && (i < (iFirst + 16)) ) 
      i++ ;
    cmplx SegmPt2 = m_CorrData[ Norm360( i ) ].m_Pt1.m_ZeroPt ;
    double dDist1 = MainAxis.dist( SegmPt1 ) ;
    double dDist2 = MainAxis.dist( SegmPt2 ) ;
    if ( intersect( SegmPt1 , SegmPt2 , Extr1 , Extr2 , CrossPt1 , CrossPt2) )
    {
      iFirst = i ;
      break ;
    }
    SegmPt1 = SegmPt2 ;
  }
  if ( i >= iFirst + 16 )
    return 0 ;
  cmplx Pt1 = CrossPt1 ;
  i += 10 ;

  while ( (m_CorrData[ Norm360(i) ].m_Pt1.m_iZeroPt == 0)
    && (i < (iFirst + 16)) ) 
    i++ ;
  SegmPt1 = m_CorrData[ Norm360(i) ].m_Pt1.m_ZeroPt ;
  i++ ;

  for ( ; i < iFirst + 270 ; i++ )
  {
    while ( (m_CorrData[ Norm360(++i) ].m_Pt1.m_iZeroPt == 0)
      &&    (i < (iFirst + 16)) ) ;
    cmplx SegmPt2 = m_CorrData[ Norm360( i ) ].m_Pt1.m_ZeroPt ;
    if ( intersect( SegmPt1 , SegmPt2 , Extr1 , Extr2 , CrossPt1 , CrossPt2) )
    {
      break ;
    }
    SegmPt1 = SegmPt2 ;
  }
  if ( i >= iFirst + 270 )
    return 0 ;

  cmplx Pt5 = CrossPt1 ;
  cmplx FromHeadToTail = Pt5 - Pt1 ;
  cmplx ConturCent = (Pt1 + Pt5)/2. ;
  m_LastContur.Center = ConturCent ; // middle between extrem points, not weight center
  m_LastContur.dLength_um = abs( FromHeadToTail ) * m_dScale_nm * 0.001 ;
  m_LastContur.dWidth_um = 0.0 ;
  cmplx Step = FromHeadToTail / (iNSlices + 1.) ;
  cmplx Ortho1 = polar( 150. , arg(FromHeadToTail) + M_PI2 ) ;
  int iIndexRight = iFirst + 2 ;
  int iIndexLeft = iFirst -2 ;
  int iRight1 = iIndexRight , iRight2 ;
  int iLeft1 = iIndexLeft , iLeft2 ;
  cmplx Right1 , Right2 , Left1 , Left2 ;
  memset( m_LastContur.Slices , 0 , 
    sizeof(m_LastContur.Slices) + sizeof(m_LastContur.Presentation) ) ;
  m_LastContur.iNConturSlices = 0 ;
  int iNRightSlices = 0 , iNLeftSlices = 0 ;
  for ( int i = 0 ; (i < iNSlices) && (abs(iIndexLeft - iIndexRight) > 3) ; i++ )
  {
    cmplx SliceCenter = Pt1 + (Step * (double)(i + 1)) ;
    cmplx SidePt1 = SliceCenter + Ortho1 ;
    cmplx SidePt2 = SliceCenter - Ortho1 ;
    while ( m_CorrData[iRight1].m_Pt1.m_iZeroPt == 0 )  
    {
      if ( abs(iRight1 - iIndexLeft) < 4 )
        return 0 ;
      iRight1 = Norm360(iRight1 + 1);
    }
    Right1 = m_CorrData[iRight1].m_Pt1.m_ZeroPt ;
    while ( abs(iIndexLeft - iIndexRight) > 3 )
    {
      iRight2 = Norm360(iRight1 + 1) ;
      while ( m_CorrData[iRight2].m_Pt1.m_iZeroPt == 0 )  
      {
        if ( abs(iRight2 - iIndexLeft) < 4 )
          return 0 ;
        iRight2 = Norm360(iRight2 + 1);
      }
      Right2 = m_CorrData[iRight2].m_Pt1.m_ZeroPt ;
      cmplx CrossRight , CrossRight2 ;
      if ( intersect( Right1 , Right2 , SidePt1 , SidePt2 , CrossRight , CrossRight2) )
      {
        m_LastContur.Slices[ i ].dRightSide_um = 
          abs( SliceCenter - CrossRight ) * m_dScale_nm / 1000. ;
        m_LastContur.Slices[ i ].RightPoint = CrossRight ;
        iNRightSlices++ ;
        break ;
      }
      iRight1 = iIndexRight = Norm360(iRight2 + 1) ;
      Right1 = Right2 ;
    }
  }
  int iWidestSlice = -1 ;
  for ( int i = 0 ; (i < iNSlices) && (abs(iIndexLeft - iIndexRight) > 3) ; i++ )
  {
    cmplx SliceCenter = Pt1 + (Step * (double)(i + 1)) ;
    cmplx SidePt1 = SliceCenter + Ortho1 ;
    cmplx SidePt2 = SliceCenter - Ortho1 ;
    while ( m_CorrData[iLeft1].m_Pt1.m_iZeroPt == 0 )  
    {
      if ( abs(iLeft1 - iIndexRight) < 4 )
        return 0 ;
      iLeft1 = Norm360(iLeft1 - 1);
    }
    Left1 = m_CorrData[iLeft1].m_Pt1.m_ZeroPt ;
    while ( abs(iIndexLeft - iIndexRight) > 3 )
    {
      iLeft2 = Norm360(iLeft1 - 1) ;
      while ( m_CorrData[ Norm360(iLeft2) ].m_Pt1.m_iZeroPt == 0  )
      {
        if ( abs(iLeft2 - iIndexRight) < 4 )
          return 0 ;
        iLeft2 = Norm360( iLeft2 - 1 ) ;
      }
      Left2 = m_CorrData[iLeft2].m_Pt1.m_ZeroPt ;
      cmplx CrossLeft , CrossLeft2 ;
      if ( intersect( Left1 , Left2 , SidePt1 , SidePt2 , CrossLeft , CrossLeft2 ) )
      {
        m_LastContur.Slices[ i ].dLeftSide_um = 
          abs( SliceCenter - CrossLeft ) * m_dScale_nm / 1000. ;
        m_LastContur.Slices[ i ].LeftPoint = CrossLeft ;
        if ( m_LastContur.Slices[ i ].dRightSide_um )
        {
          double dSliceWidth = m_LastContur.Slices[ i ].dLeftSide_um
            + m_LastContur.Slices[ i ].dRightSide_um ;
          if ( dSliceWidth > m_LastContur.dWidth_um )
          {
            m_LastContur.dWidth_um = dSliceWidth ;
            iWidestSlice =  i  ;
          }
        }
        m_LastContur.iNConturSlices++ ;
        break ;
      }
      iLeft1 = iIndexLeft = Norm360(iLeft2 - 1) ;
      Left1 = Left2 ;
    }
  }
  if ( m_LastContur.iNConturSlices )
  {
    m_LastContur.dAngle_deg = RadToDeg( dCellAngle ) ;
    if ( m_iViewMode & 2 )
    {
      FXString Label ;
      for ( int i = 0 ; i < m_LastContur.iNConturSlices ; i++ )
      {
        Label.Format( "Slice%d" , i ) ;
        m_LastContur.Presentation[ i ] = 
          CreateLineFrame( m_LastContur.Slices[i].RightPoint , 
          m_LastContur.Slices[i].LeftPoint , 
          (i == iWidestSlice)? "0xff0000" : "0xffffff" , Label , m_dwFrameId ) ;
      }

    }
  }
  return m_LastContur.iNConturSlices ;
}

double RadialCorr::ScanSector( cmplx& Cent , double dAngleFrom_rad ,
  double dAngleTo_rad , double dAngleStep_rad , 
  const CVideoFrame * vf )  //returns last measured angle
{
  int iBreakCnt = 0 ;
  double dAngle = dAngleFrom_rad ;
  double dAngLow = ( dAngleStep_rad > 0. ) ? dAngleFrom_rad : dAngleTo_rad ;
  double dAngHigh = ( dAngleStep_rad > 0. ) ? dAngleTo_rad : dAngleFrom_rad ;
  double dLastAngle = dAngle ;
  for ( ;  dInRange( dAngle , dAngLow , dAngHigh ) ;  dAngle += dAngleStep_rad  )
  {
    double dAngleNorm = fmod( dAngle , M_2PI ) ;
    double dAngledeg = RadToDeg( dAngleNorm ) ;
    int iInd = GetAngleIndex( dAngleNorm , fabs(dAngleStep_rad) ) ;
    m_CorrData[iInd].m_dAngle = dAngledeg ;
    if ( BuildCorrFunction( Cent , iInd , m_Signal , vf ) == 0. )
    {
      if ( ++iBreakCnt <= 7 )
        continue ;
      return dLastAngle ;
    }
    if ( m_iZeroPt - m_iStartCorrelPos <= 0 )
    {
      m_CorrData[iInd].m_Pt1.m_iZeroPt = 0 ;
      if ( ++iBreakCnt <= 7 )
        continue ;
      return dLastAngle ;
    }
    double dCorrZeroPos = FindNearestZero( iInd ) ;
    if ( m_dCorrAmpl * 100. * m_dGaussSigma_pix > m_dDiscoverThres )
    {
      if ( ((dCorrZeroPos > m_iLastMinPos)  &&  (dCorrZeroPos > m_iLastMaxPos))
        || ((dCorrZeroPos < m_iLastMinPos)  &&  (dCorrZeroPos < m_iLastMaxPos)) )
      {
        if ( ++iBreakCnt <= 7 )
          continue ;
        return dLastAngle ;
      }
      m_iMaxPos = m_iLastMaxPos ;
      m_iMinPos = m_iLastMinPos ;

      if ( m_pWhiteContour )
      {
        m_pWhiteContour->Add( CDPoint( m_CorrData[iInd].m_Pt1.m_MaxPt.real() , 
          m_CorrData[iInd].m_Pt1.m_MaxPt.imag() ) ) ;
      }
      if ( m_pBlackContour )
      {
        m_pBlackContour->Add( CDPoint( m_CorrData[iInd].m_Pt1.m_MinPt.real() , 
          m_CorrData[iInd].m_Pt1.m_MinPt.imag() ) ) ;
      }
      if ( dCorrZeroPos > 0. )
      {
        m_iZeroPt = m_CorrData[iInd].m_Pt1.m_iZeroPt = ROUND(dCorrZeroPos) ;
        cmplx ZeroPt = Cent + polar(
          double(m_iMinRadius + m_iZeroPt + m_iGaussHalfWidth_pix) ,
          -dAngleNorm ) ;
        m_dLastDist = abs( m_LastZeroPt - ZeroPt ) ;
        m_CorrData[iInd].m_Pt1.m_dDist = m_dLastDist ;
        m_CorrData[iInd].m_Pt1.m_ZeroPt = ZeroPt ;
        m_LastZeroPt = ZeroPt ;
        double dThresInPixels = m_dZeroDistThres * abs( ZeroPt - Cent ) ;
        if ( (dAngle != dAngleFrom_rad)  &&  (m_dLastDist > dThresInPixels) )
        {
          m_dLastOverDist = m_dLastDist ;
        }
        dLastAngle = dAngle ;
        m_iNMeasuredPts++ ;
      }
      else
      {
        m_CorrData[iInd].m_Pt1.m_dDist = -1 ;
        m_CorrData[iInd].m_Pt1.m_ZeroPt = cmplx() ;
      }
    }
    else
    {
      m_CorrData[iInd].Reset() ;
      m_CorrData[iInd].m_dAngle = dAngleNorm ;
      m_CorrData[iInd].m_Pt1.m_iZeroPt = 0 ;
      if ( ++iBreakCnt <= 7 )
        continue ;
      return dLastAngle ;
    }
  }
  return dLastAngle ;
}

double RadialCorr::FindAndMeasureSector( 
   double dFirstAng , double dLastAng , double dAngleStep ,
   const CVideoFrame * vf )
{
  double dAngleNorm = fmod( dFirstAng , M_2PI ) ;
  double dAngledeg = RadToDeg( dAngleNorm ) ;
  int iInd = GetAngleIndex( dAngleNorm , dAngleStep ) ;
  m_CorrData[iInd].m_dAngle = dAngledeg ;
  double dLastMeasuredAngle = dFirstAng ;
  // first correlations
  double dAmpl1 = BuildCorrFunction(  m_Cent , 
    iInd , m_Signal , vf , true ) ;
  m_dMaxSignSigma2 = m_CorrData[iInd].m_Pt1.m_dMaxSignSigma2 ;
  m_dCorrAmpl = dAmpl1 ;
  m_iLastCorrFuncLen = m_CorrData[iInd].m_iCorrFuncLen ;
  if ( m_dCorrAmpl * 100. * m_dGaussSigma_pix > m_dDiscoverThres 
    && m_iLastCorrFuncLen > 0 
    && m_dMaxSignSigma2 > m_dSignThres )
  {
    m_iMaxPos = m_iLastMaxPos ;
    m_iMinPos = m_iLastMinPos ;

    cmplx VectorEnd = m_Cent + polar( 
      double(m_iRealMaxRadius) , -dFirstAng ) ;

    double dCorrZeroPos = FindZero( iInd ) ;
    m_iZeroPt = ROUND( dCorrZeroPos ) ;
    cmplx ZeroPt = m_Cent + polar(
      m_iMinRadius + dCorrZeroPos + m_iGaussHalfWidth_pix ,
      -dFirstAng ) ;
    m_CorrData[iInd].m_Pt1.m_ZeroPt = ZeroPt ;
    m_CorrData[iInd].m_Pt1.m_dDist = 0. ;
    int iFirstMeasuredIndex = iInd ;
    int iLastMeasuredIndex = iInd ;

    if ( m_iViewMode & 8 )
    {
      CFigureFrame * pVector = CreateLineFrame( m_Cent , VectorEnd , 
        "0xffff00" , "InitVector" , vf->GetId() , vf->GetTime() ) ;
      m_pResult->AddFrame( pVector ) ;
      CFigureFrame * pMaxPt = CreatePtFrame( 
        m_CorrData[iInd].m_Pt1.m_MaxPt , 
        vf->GetTime() , "0x00ffff" , "MaxCorrelation" ) ;
      m_pResult->AddFrame( pMaxPt ) ;
      CFigureFrame * pMinPt = CreatePtFrame( 
        m_CorrData[iInd].m_Pt1.m_MinPt , 
        vf->GetTime() , "0x00ffff" , "MinCorrelation" ) ;
      m_pResult->AddFrame( pMinPt ) ;
      CFigureFrame * pCorrFuncShow = CFigureFrame::Create() ;
      pCorrFuncShow->Attributes()->WriteString( "color" , "0xff00ff" ) ;
      pCorrFuncShow->SetLabel( "Contour" ) ;
      pCorrFuncShow->SetTime( vf->GetTime() ) ;
      pCorrFuncShow->ChangeId( vf->GetId() ) ;
      double * pCorrFunc = m_CorrData[iInd].m_pCorrFunc ;
      for ( int i = m_iMinRadius ; i < m_iRealMaxRadius - m_iPatternLen ; i++ )
      {
        pCorrFuncShow->Add( CDPoint( 
          m_Cent.real() + m_iGaussHalfWidth_pix + i + 1 , 
          m_Cent.imag() - pCorrFunc[i- m_iMinRadius] * 100 ) ) ;
      }
      m_pResult->AddFrame( pCorrFuncShow ) ;
    }
    if ( m_iViewMode & 1 )
    {
      if ( m_pTxtInfo == NULL )
      {
        m_pTxtInfo = CTextFrame::Create() ;
        m_pTxtInfo->Attributes()->WriteInt( "x" , 10 ) ;
        m_pTxtInfo->Attributes()->WriteInt( "y" , 10 ) ;
        m_pTxtInfo->Attributes()->WriteInt("Sz" , 10 ) ;
        m_pTxtInfo->SetLabel( "Data" );
        m_pTxtInfo->ChangeId(m_dwFrameId) ;
        m_pTxtInfo->SetTime(m_dFrameTime) ;
      }
    }

    if ( m_iViewMode & 4 )
    {
      if ( m_pWhiteContour == NULL )
      {
        m_pWhiteContour = CreatePtFrame( 
          m_CorrData[iInd].m_Pt1.m_MaxPt , vf->GetTime() , 
          "0x000000" , "MinCont" , m_dwFrameId ) ;
      }
      if ( m_pBlackContour == NULL )
      {
        m_pBlackContour = CreatePtFrame( 
          m_CorrData[iInd].m_Pt1.m_MinPt , vf->GetTime() , 
          "0xffffff" , "MaxCont" , m_dwFrameId ) ;
      }
    }
    m_iNMeasuredPts++ ;
    double dLastAngle = dFirstAng + M_2PI - dAngleStep/2. ;
    int iLastMeasured = 0 ;
    double dAngle = dFirstAng + dAngleStep ;
    dLastMeasuredAngle = ScanSector ( m_Cent , dAngle , 
      dLastAngle , dAngleStep , vf ) ;
  }
  return dLastMeasuredAngle ;
}

bool RadialCorr::BuildConturRecord
(const CONTUR_DATA& Contur , FXString& ResultAsText ) 
{
  FXString AsText ;
  m_LastContur.dTime = GetHRTickCount()/1000. ; // in ms
  AsText.Format( 
    _T("%6d,%6d,%10.1f,%6.2f,%6.2f,%6.2f,%8.1f,%d") ,
    m_iCellNumber , m_iMeasurementNumber++ , Contur.dTime ,
    Contur.dLength_um , Contur.dWidth_um , Contur.dAngle_deg ,
    0. /* Square */ , Contur.iNConturSlices ) ;
  for ( int i = 0 ; i < Contur.iNConturSlices ; i++ )
  {
    double dLeftDist = Contur.Slices[i].dLeftSide_um ;
    double dRightDist = Contur.Slices[i].dRightSide_um ;
    FXString Addition ; 
    Addition.Format( _T(",%6.2f,%6.2f") , dLeftDist , dRightDist ) ;
    AsText += Addition ;
  }
  ResultAsText = AsText ;
  return true ;
}

bool RadialCorr::RestoreContur( LPCTSTR AsText , CONTUR_DATA& Result )
{
  CONTUR_DATA TmpContur ;
  double dMaxWidthPos ;  // dummy
  int iNScanned = _stscanf_s( AsText , _T("%6d,%6d,%lf,%lf,%lf,%lf,%lf,%lf,%d") ,
    &TmpContur.iCellNumber , &TmpContur.iMeasurementNumber , 
    &TmpContur.dTime , &TmpContur.dLength_um , &TmpContur.dWidth_um , &TmpContur.dAngle_deg ,
    &TmpContur.dSquare_um2 , &dMaxWidthPos , &TmpContur.iNConturSlices ) ;
  if (  iNScanned < 9  
    ||  TmpContur.iNConturSlices <= 0
    ||  TmpContur.iNConturSlices > MAX_N_CONTUR_SLICES )
    return false ;
  TmpContur.dTime = GetHRTickCount() ;
  TmpContur.Center = cmplx(0,0) ;
  int iCommaNumber = 0 ;
  LPCTSTR pComma = _tcschr( AsText , _T(',') ) ;
  for (  ; iCommaNumber < 8 ; iCommaNumber++ )
  {
    if ( !pComma )
      return false ;
    pComma = _tcschr( pComma + 1 , _T(',') ) ;
  }
  double dSliceSize_um = TmpContur.dLength_um / (TmpContur.iNConturSlices + 1) ;
  double dHeadPos_um = (TmpContur.dLength_um / 2) ;
  for ( int i = 0 ; i < TmpContur.iNConturSlices ; i++ )
  {
    iNScanned = _stscanf_s( pComma + 1 , _T("%lf,%lf") ,
      &TmpContur.Slices[i].dLeftSide_um , &TmpContur.Slices[i].dRightSide_um ) ;
    if ( iNScanned < 2 )
      return false ;
    double dSlicePosition_um = dHeadPos_um - ((i + 1) * dSliceSize_um) ;
    // Right point is under symmetry line, for this line direction 0 it will be negative
    cmplx RightPt( dSlicePosition_um , -TmpContur.Slices[i].dRightSide_um ) ;
    cmplx LeftPt( dSlicePosition_um , TmpContur.Slices[i].dLeftSide_um ) ;
    TmpContur.Slices[i].RightPoint = RightPt ;
    TmpContur.Slices[i].LeftPoint = LeftPt ;
    pComma = _tcschr( pComma + 1 , _T(',') ) ;
    if ( !pComma )
      return false ;
    pComma = _tcschr( pComma + 1 , _T(',') ) ;
    if ( !pComma  &&  i < TmpContur.iNConturSlices - 1 )
      return false ;
  }
  Result = TmpContur ;
  return true ;
}

CFigureFrame *
RadialCorr::GetPatternOnFoundContur(
  cmplx Center_pix, double Angle_Rad, const char * pColor )
{
  if ( m_PatternContur.iNConturSlices && m_PatternContur.dLength_um 
    && m_PatternContur.dWidth_um && (m_dScale_nm != 0.) )
  {
    cmplx Rotation = polar( 1. , Angle_Rad ) ;
    double dHeadPos = (m_PatternContur.dLength_um / 2) ;
    // Step 1: Add head point (Pt1 in Benny's terminology)
    cmplx HeadPt( ScaleToPix( dHeadPos ) , 0. ) ;
    cmplx HeadPtPix = RotateAndShift( HeadPt , Center_pix , Rotation ) ;
    CDPoint HdCDPt( CDPoint(HeadPtPix.real(),HeadPtPix.imag())) ;
    CFigureFrame * pResult = CreatePtFrame( HeadPtPix , GetHRTickCount() , 
      pColor , "PatternContur" ) ;
    if ( !pResult )
      return NULL ;
    // Step 2: Build right side of contur in pixels
    for ( int i = 0 ; i < m_iNConturSlices ; i++ )
    {
      cmplx PtPix = RotateAndShift( 
        ScaleToPix(m_PatternContur.Slices[i].RightPoint) ,
        Center_pix , Rotation ) ;
      CDPoint Pt(PtPix.real(),PtPix.imag()) ;
      pResult->AddPoint( Pt ) ;
    }
    // Step 3: add tail point (Pt 3 in Benny's terminology)
    cmplx TailPtPix = RotateAndShift( -HeadPt , Center_pix , Rotation ) ;
    pResult->AddPoint( CDPoint(TailPtPix.real(),TailPtPix.imag()) );
    // Step 4: Build left side of contur in pixels
    for ( int i = m_iNConturSlices - 1 ; i >= 0 ; i-- )
    {
      cmplx PtPix = RotateAndShift( 
        ScaleToPix(m_PatternContur.Slices[i].LeftPoint) ,
        Center_pix , Rotation ) ;
      CDPoint Pt(PtPix.real(),PtPix.imag()) ;
      pResult->AddPoint( Pt ) ;
    }
    // Step 5: Add head point for contur closing
    pResult->AddPoint( HdCDPt ) ;
    return pResult ;
  }
  else
    return NULL ;
}
