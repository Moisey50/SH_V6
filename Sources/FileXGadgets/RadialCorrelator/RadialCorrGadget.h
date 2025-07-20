// RadialCorrGadget.h : Declaration of the RadialCorr

#pragma once
#include <Gadgets\gadbase.h>
#include <gadgets\videoframe.h>
#include <gadgets\quantityframe.h>
#include <gadgets\containerframe.h>
#include <gadgets\FigureFrame.h>
#include <fxfc\fxfc.h>
#include <helpers\FramesHelper.h>
#include <math\intf_sup.h>
#include <math\PlaneGeometry.h>
#include <imageproc\utilities.h>
#include "CellOutputPars.h"


#define MIN_MAX_ARRAY_SIZE 30

#define VIEW_CONTUR_AND_RESULTS 0x0001
#define VIEW_SLICES             0x0002
#define VIEW_MIN_MAX_CONTURS    0x0004
#define VIEW_INIT_VECT_MIN_MAX  0x0008
#define VIEW_TAIL_SEARCH_POINTS 0x0010
#define VIEW_LONG_SHORT_DIRS    0x0020
#define VIEW_PATTERN_CONTUR     0x0040


//************************************************************************


// End of utilities
//*****************************************************
class RadialCorr : public CFilterGadget
{
public:
    RadialCorr(void);
    void ShutDown();
//
  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);
  virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  virtual int GetOutputsCount() { return 3; } ;
  virtual COutputConnector* GetOutputConnector(int n) ;
  virtual int GetDuplexCount() { return 1 ; } ;
  virtual CDuplexConnector* GetDuplexConnector(int n) ;

  bool    SaveContur( const CONTUR_DATA& Contur , LPCTSTR FileName ) ;
  bool    BuildConturRecord( const CONTUR_DATA& Contur , FXString& ResultAsText ) ;
  bool    RestoreContur( const LPCTSTR AsText , CONTUR_DATA& Result ) ;

  double     m_dGaussSigma_pix ;
  int        m_iGaussHalfWidth_pix ;
  int        m_iSecondaryMeasZone ;
  double     m_dDiscoverThres ;
  double     m_Pattern[300] ;
  double     m_TmpPattern[300] ;
  int        m_iPatternLen ;
  int        m_iMinRadius ;
  int        m_iMaxRadius ;
   // the next is for show control
   // bit 0 ( mask 0x01 ) - view contour and size results
   // bit 1 ( mask 0x02 ) - view slices
   // bit 2 ( mask 0x04 ) - view white and black contours
   // bit 3 ( mask 0x08 ) - view initial vector, max and min corr, corr function on initial vector
   // bit 4 ( mask 0x10 ) - tail points show with data 
   // bit 6 ( mask 0x20 ) - view Long and short directions
  int        m_iViewMode ;
  int        m_iOrigImageSizeX ;
  int        m_iOrigImageSizeY ;

  
  int        m_iRealMaxRadius ;
  int        m_iMinPos , m_iLastMinPos ;
  int        m_iMaxPos , m_iLastMaxPos ;
  int        m_iZeroPt ;
  cmplx      m_LastZeroPt ;
  double     m_dZeroDistThres ;
  double     m_dLastDist ;
  double     m_dLastOverDist ;
  double     m_dLastSigma2 ;
  int        m_iNMeasuredPts ;
  FXString   m_PatternConturAsString ;

  CDuplexConnector*	m_pControl;
  COutputConnector* m_pCutControl ;
  COutputConnector* m_pROIPositionPin ;
  double     m_Signal[1000] ;
  double     m_CorrFunc[1000] ;
  double     m_CorrFuncAmpl[360] ;
  CorrExtremums     m_CorrData[360] ;
  int        m_LocalMin[MIN_MAX_ARRAY_SIZE] ;
  int        m_iMinInd ;
  int        m_LocalMax[MIN_MAX_ARRAY_SIZE] ;
  int        m_iMaxInd ;
  int        m_iSignalLen ;
  int        m_iLastCorrFuncLen ;
  int        m_iNConturSlices ;
  double     m_dInitialAngle_rad ;
  int        m_iMeasStep_deg ;
  double     m_dScale_nm ;
  double     m_dMinSize_um ;
  double     m_dMaxSize_um ;
  cmplx      m_Cent ;
  double     m_TailDetectAmpls[4] ;
  double     m_TailDetectSigmas2[4] ;
  cmplx      m_TailPoints[4] ;
  cmplx      m_Ortho1[4] ;
  cmplx      m_Ortho2[4] ;
  CONTUR_DATA m_LastContur ;
  CONTUR_DATA m_PatternContur ;
  int        m_iCellNumber ;
  int        m_iMeasurementNumber ;
  DWORD      m_dwFrameId ;
  double     m_dFrameTime ;
  cmplx      m_PrevCenter ;
  CRect      m_PrevZone ;

  int        m_iStartCorrelPos ;
  int        m_iStopCorrelPos ;
  double     m_dCorrAmpl ;
  double     m_dMaxAmpl ;
  double     m_dSignThres ;
  double     m_dLastSignSigma2 ;
  double     m_dMaxSignSigma2 ;

  CContainerFrame * m_pResult ;
  CTextFrame   * m_pTxtInfo ;
  CFigureFrame * m_pSlopeContour ;
  CFigureFrame * m_pWhiteContour ;
  CFigureFrame * m_pBlackContour ;
  inline double FindZero( double * pSignal ,
    int iSignalLen , int iMinPos , int iMaxPos )
  {
    if ( iMinPos < iMaxPos )
    {
      for ( int i = iMinPos ; i <= iMaxPos ; i++ )
      {
        if ( pSignal[i] > 0. )
          return i ;
      }
      return -1 ;
    }
    else if ( iMinPos > iMaxPos )
    {
      for ( int i = iMaxPos ; i <= iMinPos ; i++ )
      {
        if ( pSignal[i] < 0. )
          return i ;
      }
      return -1 ;
    }
    if ( pSignal[iMinPos] == 0. )
      return iMinPos ;
    return -1 ;
  }

  inline double FindZero( int iIndex )
  {
    double dRes = FindZero( m_CorrData[iIndex].m_pCorrFunc ,
      m_CorrData[iIndex].m_iCorrFuncLen , 
      m_CorrData[iIndex].m_Pt1.m_iNPos , 
      m_CorrData[iIndex].m_Pt1.m_iPPos ) ;
    if ( dRes > 0 )
    {
      m_CorrData[iIndex].m_Pt1.m_iZeroPt = m_iZeroPt = ROUND( dRes ) ;
    }
    return dRes ;

  }
  inline double FindNearestZero( double * pSignal ,
    int iSignalLen , int CentPos , int iMaxDist )
  {

    for ( int i = 0 ; i <= iMaxDist ; i++ )
    {
      if ( pSignal[ CentPos + i] > 0. )
      {
        if ( pSignal[ CentPos + i + 1 ] < 0. )
          return CentPos + i + 1 ;
      }
      else if ( pSignal[ CentPos + i] < 0. )
      {
        if ( pSignal[ CentPos + i + 1 ] > 0. )
          return CentPos + i ;
      }
      else
        return CentPos + i ;
      if ( pSignal[ CentPos - i] > 0. )
      {
        if ( pSignal[ CentPos - i - 1 ] < 0. )
          return CentPos - i - 1 ;
      }
      else if ( pSignal[ CentPos - i] < 0. )
      {
        if ( pSignal[ CentPos - i - 1 ] > 0. )
          return CentPos - i ;
      }
      else
        return CentPos - i ;
    }
    return -1 ;
  }

  inline double FindNearestZero( int iIndex )
  {
    double dRes = FindNearestZero( m_CorrData[iIndex].m_pCorrFunc ,
      m_CorrData[iIndex].m_iCorrFuncLen , 
      m_iZeroPt - m_iStartCorrelPos , 
      m_iSecondaryMeasZone ) ;
    if ( dRes > 0 )
    {
      m_iZeroPt = m_iStartCorrelPos + ROUND( dRes ) ;
      m_CorrData[iIndex].m_Pt1.m_iZeroPt = m_iZeroPt ;
    }
    return dRes + m_iStartCorrelPos ;
  }
  inline int GetIndexForCenter( 
    double dCellAngle , double dAngleStep , cmplx& Cent )
  {
    int iIndex = GetAngleIndex( dCellAngle , dAngleStep ) ;
    cmplx VectFromCent = m_CorrData[iIndex].m_Pt1.m_ZeroPt - Cent ;
    double dFirstAngle = NormTo2PI(-arg( VectFromCent )) ;
    double dCellAngle2PI = NormTo2PI( dCellAngle ) ;
    int iIndexDelta = GetAngleIndex( 
      dCellAngle2PI - dFirstAngle , dAngleStep ) ;
    iIndex += (iIndexDelta <= 180) ? iIndexDelta : 360 - iIndexDelta ;
    return iIndex ;
  }
  inline int GetDeltaForIndexes( int iInd1 , int iInd2 )
  {
    if ( iInd2 < iInd1 )
      return iInd2 + (360- iInd1) ;
    else
      return iInd2 - iInd1 ;
  }
  inline int Norm360( int i ) { return ((i+360)%360) ; } ;
  inline bool dInRange( double dNumber , double dLowLimit , double dHighLimit )
  {
    return (( dLowLimit <= dNumber ) && ( dNumber <= dHighLimit ));
  }
  inline double ScaleToPix( double dCoord_um ) 
  {
    return ( dCoord_um * 1000./m_dScale_nm ) ;
  }
  inline cmplx ScaleToPix( cmplx Coord_nm )
  {
    return (Coord_nm * 1000./m_dScale_nm) ;
  }
  inline cmplx RotateAndShift( cmplx&Pt , cmplx& Cent , cmplx& Rotation )
  {
    return (Cent + (Pt * Rotation)) ;
  }
  double BuildCorrFunction( 
    double * pPattern ,
    int iPattLen , double dPattSigma ,
    double * pSignal , int iSignalLen ,
    double * pResult , int& iMinPos,
    int& iMaxPos , double& dSignSigma2 ) ;
  double BuildCorrFunction( 
    double * pPattern ,
    int iPattLen , double dPattSigma ,
    double * pSignal , int iSignalLen ,
    int iInd , double& dSignSigma2 ) ;
  double BuildCorrFunction(
    double * pPattern ,
    int iPattLen , double dPattSigma ,
    double * pSignal , int iSignalLen ,
    double * pResult , int& iMinPos, int& iMaxPos , 
    int iPrevMin , int iPrevMax , int iEpsilon , double& dSignSigma2 );
  double BuildCorrFunction(
    double * pPattern ,
    int iPattLen , double dPattSigma ,
    double * pSignal , int iSignalLen ,
    int iInd ,
    int iPrevMin , int iPrevMax , int iEpsilon , double& dSignSigma2 ) ;
  double BuildCorrFunction(  cmplx Cent , int iIndex ,
    double * pSignal , const pTVFrame pFrame , bool bFirst = false ) ;
  double GetSigma2OnSide(  cmplx Cent , cmplx PtOnContour ,
    const pTVFrame pFrame , double& dSigma2 , cmplx& CheckPoint ,
    cmplx& Pt1 , cmplx& Pt2) ;
  double BuildCorrFunction(  cmplx Cent , cmplx PtOnContour ,
    const pTVFrame pFrame , double& dSigma2 , cmplx& CheckPoint ,
    cmplx& Pt1 , cmplx& Pt2) ;
  cmplx GetFarestPoint( cmplx& LinePt1 , cmplx& LinePt2 ,
    int iFirstIndex , int iLastIndex ) ;
  int GetSlices( cmplx Cent , double dCellAngle , 
    int iFirst , int iNSlices ) ;
  double ScanSector( cmplx& Cent , double dAngleFrom_rad ,
    double dAngleTo_rad , double dAngleStep_rad , 
    const CVideoFrame * vf ) ; //returns last measured angle
  double FindAndMeasureSector( 
    double dFirstAng , double dLastAng , double dAngleStep ,
    const CVideoFrame * vf ) ;
  CFigureFrame * GetPatternOnFoundContur(cmplx Center, 
    double Angle_Rad, const char * pColor = "0xff8080" );

 
  DECLARE_RUNTIME_GADGET(RadialCorr);
};


