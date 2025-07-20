#pragma once
#include <fxfc\fxfc.h>
#include "helpers\UserBaseGadget.h"
#include "gadgets\videoframe.h"
#include "gadgets\textframe.h"
#include "gadgets\QuantityFrame.h"
#include <gadgets\FigureFrame.h>
#include "imageproc\LineData.h"
#include <imageproc/ImageProfile.h>
#include <Math\PlaneGeometry.h>
#include <helpers\FramesHelper.h>
#include <Math\FRegression.h>
#include <Math/FigureProcessing.h>
#include "SecondaryCheckForObjects.h"

#include <deque>

enum RulTrackerGadgetMode
{
  RTGM_Unknown = 0 ,
  RTGM_Side ,
  RTGM_Top 
};

enum RT_OutputConn
{
  RTOC_MainOutput = 0 ,
  RTOC_ResultView ,
  RTOC_CamControl ,
};


class DistAndScale
{
private:
  double m_dDist_Orig;
  double m_dDist_Expected;
  double m_dScale;

public:
  double GetDistOrig( ) const
  {
    return m_dDist_Orig;
  }

  double GetDistExpected( ) const
  {
    return m_dDist_Expected;
  }

  double GetScale( ) const
  {
    return m_dScale;
  }

public:
  DistAndScale( double dDist_Orig = 0 , double dDist_Expected = 10. )
    : m_dDist_Orig( dDist_Orig )
    , m_dDist_Expected( dDist_Expected )
  {
    SetScale( );
  }

  DistAndScale& SetDistOrig( double dDistOrig )
  {
    m_dDist_Orig = dDistOrig;
    SetScale( );
    return *this;
  }

  DistAndScale& SetDistExpected( double dDistExpected )
  {
    m_dDist_Expected = dDistExpected;
    SetScale( );
    return *this;
  }
  DistAndScale& Reset( )
  {
    m_dDist_Expected = m_dDist_Orig = 0;
    SetScale( );
    return *this;
  }

private:
  double GetScale( double dDistOrig , double dDistExpected )
  {
    double dScale = 0;
    if ( dDistOrig > 1.e-10)
      dScale = dDistExpected / dDistOrig;
    return dScale;
  }
  DistAndScale& SetScale( )
  {
    m_dScale = GetScale( m_dDist_Orig , m_dDist_Expected );

    return *this;
  }
};

class LineInfo
{
public:
  
  //LineInfo* m_pLinePrev;
  //LineInfo* m_pLineNext;
private:
  
  double m_dAbsPos_mm ;
  cmplx  m_cPt ;
  double m_dWidth_px ;
  int    m_iIndex ;

  //const double MMperPOSITION = 10.; 
  
  int    m_iMatchedInNextBatch = -1 ; // index of line in next image
  int    m_iMatchedInPrevBatch = -1;  // index of line in prev image
  
  double m_dBaseHorDist_mm;

  double m_dTime_ms ;
//  double m_dReplacedAbsPos_mm ; // if position replaced by reenumeration
//  double m_dReplacedAbsPos2_mm ; // if twice replaced by reenumeration

  LineInfo* m_pLinePrev;
  LineInfo* m_pLineNext;

  DistAndScale m_distAndScaleToPrev;
  DistAndScale m_distAndScaleToNext;
  
  double m_dZeroPosX_px;
  double m_dDistToZeroPosX_px;

  double m_dROICenterX_px;
  double m_dAbsDisToROICntrX_px;

  double m_dSiftFromPrevBatchActual_px;

  bool m_bIsThick;
  bool m_bIsBeforNumber;

public:
  LineInfo( ) { memset( this , 0 , sizeof( *this ) ); }
  LineInfo(
    const FXString& AsString ,
    int          iIndex ,
    double       dTime ,
    double       dROICenterX_px ,
    double       dZeroPosX_px ,
    double       dBaseHorDist_mm )
  {
    cmplx cCntrPt;
    double dW_px = -1;

    if (FromString( AsString , __out cCntrPt , __out dW_px )
      && dW_px > 0)
    {
      Reset( cCntrPt , dW_px , iIndex , dTime , dROICenterX_px , dZeroPosX_px , dBaseHorDist_mm );
    }
  }

  LineInfo(
    const cmplx& cPt ,
    double       dWidth ,
    int          iIndex ,
    double       dTime ,
    double       dROICenterX_px ,
    double       dZeroPosX_px ,
    double       dBaseHorDist_mm )
  {
    Reset( cPt , dWidth , iIndex , dTime , dROICenterX_px , dZeroPosX_px , dBaseHorDist_mm );
  }
  
  bool FromString( const FXString& AsString , __out cmplx& cPt , __out double dWidth_px )
  {
    bool res = false;

    cmplx cCntrPt;

    double dW_px = -1;

    int iXPos = (int)AsString.Find( 'X' );
    int iYPos = ( int ) AsString.Find( 'Y' );
    int iWPos = ( int ) AsString.Find( "W=" );

    if (( iXPos >= 0 ) && ( iYPos > 0 ) && ( iWPos > 0 ))
    {
      cCntrPt._Val[ _RE ] = atof( ( ( LPCTSTR )AsString ) + iXPos + 1 );
      cCntrPt._Val[ _IM ] = atof( ( ( LPCTSTR )AsString ) + iYPos + 1 );
      dW_px = atof( ( ( LPCTSTR )AsString ) + iWPos + 2 );
    }

    if (cCntrPt.real( ) > 0 &&
      cCntrPt.imag( ) > 0 &&
      dW_px > 0)
    {
      cPt = cCntrPt;
      dWidth_px = dW_px;
      res = true;
    }

    return res;
  }

  LineInfo& operator=( const LineInfo& other )
  {
    if (this != &other )
    {
      memcpy_s( this , sizeof( *this ) , &other , sizeof( other ) );
    }
    return *this;
  }

  double GetAbsPos_mm( ) const
  {
    return m_dAbsPos_mm;
  }
  LineInfo& SetAbsPos_mm( double dAbsPos_mm )
  {
    ASSERT( dAbsPos_mm > 100 );
    double dDelta = 0;
    if (m_pLinePrev != NULL && m_pLinePrev->HasAbsPos_mm( ))
    {
      dDelta = dAbsPos_mm - m_pLinePrev->GetAbsPos_mm( );
      ASSERT( dDelta==0|| dDelta == m_dBaseHorDist_mm || dDelta == m_dBaseHorDist_mm * 2 || dDelta == m_dBaseHorDist_mm * 3 );
    }

    if (m_pLineNext != NULL && m_pLineNext->HasAbsPos_mm( ))
    {
      dDelta = dAbsPos_mm - m_pLineNext->GetAbsPos_mm( );
      ASSERT( dDelta == 0 || dDelta == m_dBaseHorDist_mm * -1 || dDelta == m_dBaseHorDist_mm * -2 || dDelta == m_dBaseHorDist_mm * -3 );
    }

    m_dAbsPos_mm = dAbsPos_mm;
    return *this;
  }

  bool HasAbsPos_mm( ) const
  {
    return m_dAbsPos_mm != DBL_MAX && m_dAbsPos_mm < 1.e5;
  }

  const cmplx& GetCenterPos( ) const
  {
    return m_cPt;
  }

  double GetCenterPosX_px( ) const
  {
    return m_cPt.real( );
  }

  double GetWidth_px( ) const
  {
    return m_dWidth_px;
  }
  //LineInfo& SetWidth_px( double dWidth_px )
  //{
  //  m_dWidth_px = dWidth_px;
  //  return *this;
  //}

  LineInfo& SetAsThick( bool bIsThick )
  {
    m_bIsThick = bIsThick;
    return *this;
  }
  bool IsThick( ) const
  {
    return m_bIsThick;
  }

  int GetMatchedInPrevBatch( ) const
  {
    return m_iMatchedInPrevBatch;
  }
  LineInfo& SetMatchedInPrevBatch( int iIndxInPrevBatch )
  {
    m_iMatchedInPrevBatch = iIndxInPrevBatch;
    return *this;
  }
  int GetMatchedInNextBatch( ) const
  {
    return m_iMatchedInNextBatch;
  }
  LineInfo& SetMatchedInNextBatch( int iIndxInNextBatch )
  {
    m_iMatchedInNextBatch = iIndxInNextBatch;
    return *this;
  }

  LineInfo& SetPrev( LineInfo* pLinePrev )
  {
    if (m_pLinePrev != pLinePrev)
    {
      if (m_pLinePrev != NULL)
      {
        m_distAndScaleToPrev.Reset( );
        m_distAndScaleToPrev.SetDistExpected( m_dBaseHorDist_mm );

        if (this == m_pLinePrev->m_pLineNext)
        {
          LineInfo* pTmpLinePrev = m_pLinePrev;
          m_pLinePrev = NULL;
          pTmpLinePrev->SetNext( NULL );
        }
      }

      m_pLinePrev = pLinePrev;

      if (m_pLinePrev != NULL)
      {
        m_distAndScaleToPrev.SetDistOrig( GetCenterPosX_px( ) - m_pLinePrev->GetCenterPosX_px( ) );
   
        if (this != m_pLinePrev->m_pLineNext)
          m_pLinePrev->SetNext( this );
      }
    }
    return *this;
  }

  LineInfo& SetNext( LineInfo* pLineNext )
  {
    if (m_pLineNext != pLineNext)
    {
      if (m_pLineNext != NULL)
      {
        m_distAndScaleToNext.Reset( );
        m_distAndScaleToNext.SetDistExpected( m_dBaseHorDist_mm );

        if (this == m_pLineNext->m_pLinePrev)
        {
          LineInfo* pTmpLineNext = m_pLineNext;
          m_pLineNext = NULL;
          pTmpLineNext->SetPrev( NULL );
        }
      }

      m_pLineNext = pLineNext;

      if (m_pLineNext != NULL)
      {
        m_distAndScaleToNext.SetDistOrig( m_pLineNext->GetCenterPosX_px( ) - GetCenterPosX_px( ) );

        if (this != m_pLineNext->m_pLinePrev)
          m_pLineNext->SetPrev( this );
      }
    }
    return *this;
  }

  double GetAbsDistToPrev_px( ) const
  {
    return m_distAndScaleToPrev.GetDistOrig( );
  }

  double GetAbsDistToNext_px( ) const
  {
    return m_distAndScaleToNext.GetDistOrig( );
  }

  double GetShortestAbsDist_px(__out bool& bIsPrev ) const
  {
    double res = 0;
    
    double dAbsDistToPrev_px = GetAbsDistToPrev_px( );
    double dAbsDistToNext_px = GetAbsDistToNext_px( );
    

    res = dAbsDistToNext_px;
    bIsPrev = false;

    if (res == 0 || ( dAbsDistToPrev_px > 0 && dAbsDistToPrev_px < res ))
    {
      res = dAbsDistToPrev_px;
      bIsPrev = true;
    }

    return res;
  }

  double GetAbsDistToROICntrX_px( ) const
  {
    return m_dAbsDisToROICntrX_px;
  }

  double GetDistToZeroPosX_px( ) const
  {
    return m_dDistToZeroPosX_px;
  }

  double GetROICenterX_px( ) const
  {
    return m_dROICenterX_px;
  }

  double GetZeroPosX_px( ) const
  {
    return m_dZeroPosX_px;
  }


  LineInfo& CopyROICenterX_px( LineInfo& srcLineInfo )
  {
    SetROICenterX_px( srcLineInfo.m_dROICenterX_px );

    return *this;
  }

  LineInfo& CopyZeroPosX_px( LineInfo& srcLineInfo )
  {
    SetZeroPosX_px( srcLineInfo.m_dZeroPosX_px );

    return *this;
  }

  LineInfo& SetSiftFromPrevBatchActual_px( double dShiftFromPrevBatch_px )
  {
    m_dSiftFromPrevBatchActual_px = dShiftFromPrevBatch_px;
    return *this;
  }


private:
  LineInfo& SetROICenterX_px(double dROICenterX_px )
  {
    ASSERT( dROICenterX_px > 0 );
    
    double dLineCntrPosByX_px = GetCenterPosX_px( );

    if (dLineCntrPosByX_px > 0)
    {
      m_dROICenterX_px = dROICenterX_px;
      if (m_dROICenterX_px > 0)
        m_dAbsDisToROICntrX_px = fabs( dLineCntrPosByX_px - m_dROICenterX_px);
    }
    return *this;
  }

  LineInfo& SetZeroPosX_px( double dZeroPosX_px )
  {
    //ASSERT( dZeroPosX_px > 0 );

    double dLineCntrPosByX_px = GetCenterPosX_px( );

    if (dLineCntrPosByX_px > 0 )
    {
      m_dZeroPosX_px = dZeroPosX_px;
      if (m_dZeroPosX_px > 0)
        m_dDistToZeroPosX_px = dLineCntrPosByX_px - m_dZeroPosX_px;
    }
    return *this;
  }

  LineInfo& Reset(
    const cmplx& cPt ,
    double       dWidth ,
    int          iIndex ,
    double       dTime ,
    double       dROICenterX_px ,
    double       dZeroPosX_px,
    double       dBaseHorDist_mm )
  {
    if (fabs( m_cPt.real( ) - cPt.real( ) ) > 0.001)
      m_cPt = cPt;

    m_dBaseHorDist_mm = dBaseHorDist_mm;

    SetROICenterX_px( dROICenterX_px );

    SetZeroPosX_px( dZeroPosX_px );

    m_bIsThick = false;
    m_bIsBeforNumber = false;


    m_dWidth_px = dWidth;
    m_iIndex = iIndex; //index in raw data
    m_dTime_ms = dTime;
    m_dAbsPos_mm = DBL_MAX;

    m_pLineNext = NULL;
    m_pLinePrev = NULL;

    m_distAndScaleToPrev.Reset( );
    m_distAndScaleToPrev.SetDistExpected( m_dBaseHorDist_mm );
    m_distAndScaleToNext.Reset( );
    m_distAndScaleToNext.SetDistExpected( m_dBaseHorDist_mm );

    //m_dReplacedAbsPos_mm = m_dReplacedAbsPos2_mm = 0.;
    m_iMatchedInNextBatch = m_iMatchedInPrevBatch = -1;

    m_dSiftFromPrevBatchActual_px = DBL_MAX;

    return *this;
  }
};


class LineInfoMatched
{
private:
  LineInfo* m_pLine;

  double m_dDeltaX_px;
  double m_dDeltaTime_ms;
};

//typedef vector<LineInfo> LinesV ;
//typedef vector<LinesV> LinesSequence ;

class OneLinesSample
{
#define COLLECTION_SIZE 10
  LineInfo m_Lines[ COLLECTION_SIZE ];
public:
  const LineInfo* GetCollection( ) const
  {
    return m_Lines;
  }
  int GetSize( ) const
  {
    return COLLECTION_SIZE;
  }

  OneLinesSample( ) { memset( this , 0 , sizeof( *this ) ) ;}

  //OneLinesSample& UpdateCollection( const LinesBatch& batch )
  //{
  //  //// copy last information about processed lines to history
  //  //int iNCopyToOrigLines = batch.GetSize( ) > COLLECTION_SIZE-1 ? COLLECTION_SIZE-1 : batch.GetSize( );

  //  //memcpy( &m_OriginLinesHistory[ m_iNextHistoryIndex ] ,
  //  //  m_LastLines.data( ) , iNCopyToOrigLines * sizeof( LineInfo ) );
  //  //// set to zero the history item after copied information
  //  //memset( &m_OriginLinesHistory[ m_iNextHistoryIndex ].m_Lines[ iNCopyToOrigLines ] ,
  //  //  0 , sizeof( LineInfo ) );

  //  return *this;
  //}
};

class LinesBatch
{
private:
  string          m_sName;
  deque<LineInfo> m_batch;
  DWORD           m_dwFrameID = 0;
  int             m_iExpectedQty = -1;
  string          m_sRawData;
  double          m_dTimestamp_ms = -1;
  
  double          m_dPixelSize_mm = 0;

  double m_dWidthMax_pix = 0.;
  double m_dWidthMin_pix = 1000.;
  int    m_iWMaxIndex = -1;

  int    m_iNearestToROICntrXIndx = -1;
  int    m_iNearestToZeroPosIndx = -1;
  int    m_iBeforeWideSpaceIndx = -1;


  bool   m_bHasDigitsRegion = false;

  //double m_dROICntrX_px = DBL_MAX;
  double m_dAbsMinDistToROICntrX_px = DBL_MAX;
  double m_dMinDistToZeroPos_px = DBL_MAX;  //Minimal distance by module

  int    m_iNIntervals = 0;
  double m_dTotalDist_px = 0;

  double m_dBaseHorDist_px = DBL_MAX;

public:
  LinesBatch& SetExpectedQty( int iExpectedQty )
  {
    m_iExpectedQty = iExpectedQty;
    return *this;
  }

  LinesBatch& SetRawData( const string& sRawData )
  {
    m_sRawData = sRawData;
    return *this;
  }

  int GetSize( )const
  {
    return ( int ) m_batch.size( );
  }

  bool IsEmpty( ) const
  {
    return m_batch.empty( ); // GetSize( ) == 0;
  }

  double GetTimeStamp_ms( ) const
  {
    return m_dTimestamp_ms;
  }

  bool HasThickLine( ) const
  {
    return m_iWMaxIndex > 0;
  }

  bool HasAllAbsPos_mm( ) const
  {
    bool res = !IsEmpty( );

    for (std::deque<LineInfo>::const_iterator ci_li = m_batch.begin( ); ci_li != m_batch.end( ) && res; ci_li++)
    {
      res = ci_li->HasAbsPos_mm( );
    }

    return res;
  }

  bool HasWideSpace( ) const
  {
    return m_iBeforeWideSpaceIndx >= 0;
  }

  int GetBeforeWideSpaceIndx( ) const
  {
    return m_iBeforeWideSpaceIndx;
  }

  LineInfo* GetLineBeforeWideSpace( ) const
  {
    return ( LineInfo* )( !HasWideSpace( ) ? NULL : &( m_batch[ m_iBeforeWideSpaceIndx ] ) );
  }

  int GetThickLineIndx( )const
  {
    return m_iWMaxIndex;
  }

  double GetThickLineCntrPosX_px( ) const
  {
    return !HasThickLine( ) ? -1. : m_batch[ m_iWMaxIndex ].GetCenterPosX_px( );
  }

  LineInfo* GetThickLine( ) const
  {
    return ( LineInfo* )( !HasThickLine( ) ? NULL : &( m_batch[ m_iWMaxIndex ] ) );
  }


  const deque<LineInfo>& GetCollection( ) const
  {
    return m_batch;
  }

  deque<LineInfo>& GetCollection( )
  {
    return m_batch;
  }

  int GetNearestToROICntrXIndx( ) const
  {
    return m_iNearestToROICntrXIndx;
  }

  LineInfo* GetLineNearestToROICntrX( ) const
  {
    return GetLineByIndex( m_iNearestToROICntrXIndx );
  }

  int GetNearestToZeroPosIndx( ) const
  {
    return m_iNearestToZeroPosIndx;
  }
  LineInfo* GetLineNearestToZeroPos( ) const
  {
    return GetLineByIndex( m_iNearestToZeroPosIndx );
  }

  double GetBaseHorDist_px( ) const
  {
    return m_dBaseHorDist_px;
  }

  LineInfo* GetLineByIndex( int iIndex ) const
  {
    return ( LineInfo* )( ( iIndex < 0 || iIndex >= ( GetSize( ) ) )
      ? NULL : &( m_batch[ iIndex ] ) );
  }

  LinesBatch(
    const string& sName = string( ) ,
    double dTimestamp_ms = -1 ,
    DWORD dwFrameID = 0 ,
    double dPixelSize_mm = 0 )
  {
    Reset( sName , dTimestamp_ms , dwFrameID, dPixelSize_mm /*, dBaseHorDist_mm*/ );
  }
  LinesBatch& operator=( const LinesBatch& other )
  {
    if (this != &other)
    {
      //memcpy_s( this , sizeof( *this ) , &other , sizeof( other ) );

      m_sName = other.m_sName;
      m_sRawData = other.m_sRawData;
      m_dwFrameID = other.m_dwFrameID;
      m_dTimestamp_ms = other.m_dTimestamp_ms;
      SetExpectedQty( other.m_iExpectedQty );
      m_dTimestamp_ms = other.m_dTimestamp_ms;

      m_dWidthMax_pix = other.m_dWidthMax_pix;
      m_dWidthMin_pix = other.m_dWidthMin_pix;
      m_iWMaxIndex = other.m_iWMaxIndex;
      m_iNearestToROICntrXIndx = other.m_iNearestToROICntrXIndx;
      m_iNearestToZeroPosIndx = other.m_iNearestToZeroPosIndx;

      m_bHasDigitsRegion = other.m_bHasDigitsRegion;

      m_dAbsMinDistToROICntrX_px = other.m_dAbsMinDistToROICntrX_px;
      m_dMinDistToZeroPos_px = other.m_dMinDistToZeroPos_px;

      m_iNIntervals = other.m_iNIntervals;
      m_dTotalDist_px = other.m_dTotalDist_px;

      m_dBaseHorDist_px = other.m_dBaseHorDist_px;

      
      m_dPixelSize_mm = other.m_dPixelSize_mm;

      m_batch = other.m_batch;

      if (!IsEmpty( )) //update references
      {
        for (deque<LineInfo>::iterator i = m_batch.begin( ); i != m_batch.end( ) - 1; i++)
        {
          LineInfo* pLineInfo = &*i;
          LineInfo* pLineInfoNext = &*( i + 1 );

          pLineInfo->SetNext( pLineInfoNext );
        }
      }
    }
    return *this;
  }


  LinesBatch& Reset(
    const string& sName = string( ) ,
    double dTimestamp_ms = -1 ,
    DWORD dwFrameID = 0 ,
    double dPixelSize_mm = 0,
    double dMasterBaseHorDist_px = DBL_MAX )
  {
    m_sName = sName;
    m_sRawData = string( );
    m_dwFrameID = dwFrameID;
    m_dTimestamp_ms = dTimestamp_ms;
    SetExpectedQty( -1 );
    m_dTimestamp_ms = dTimestamp_ms;
    m_dPixelSize_mm = dPixelSize_mm;

    m_batch.clear( );

    m_dWidthMax_pix = 0.;
    m_dWidthMin_pix = 1000.;
    m_iWMaxIndex = -1;

    m_iNearestToROICntrXIndx = -1;
    m_iNearestToZeroPosIndx = -1;
    m_iBeforeWideSpaceIndx = -1;


    m_bHasDigitsRegion = false;

    m_dAbsMinDistToROICntrX_px = DBL_MAX;
    m_dMinDistToZeroPos_px = DBL_MAX;  //Minimal distance by module


    m_iNIntervals = 0;
    m_dTotalDist_px = 0;

    m_dBaseHorDist_px = dMasterBaseHorDist_px;
    
    return *this;
  }

  bool AddLine( LineInfo& newLine , double dMasterBaseHorDist_px , double dROIWidth_px , bool isLast = false )
  {

    bool bRes = true;

    if (IsEmpty( ))
      Reset( m_sName , m_dTimestamp_ms , m_dwFrameID , m_dPixelSize_mm , dMasterBaseHorDist_px );

    LineInfo*  pPrev = ( IsEmpty( ) ? ( LineInfo* )NULL : &*( m_batch.end( ) - 1 ) );

    m_batch.push_back( newLine );

    LineInfo& newLast = *( m_batch.end( ) - 1 );

    ASSERT( &newLine != &newLast );

    if (pPrev != NULL)
    {
      pPrev->SetNext( &newLast );

      //      ASSERT( pPrev->m_pLineNext == &newLast );

            //ASSERT( m_dROIRight_px != DBL_MAX );

      bRes = InvalidateLineInfo( newLast , dMasterBaseHorDist_px > 0 ? dMasterBaseHorDist_px : dROIWidth_px * 0.4 );

      //double dAbsDistToPrev = newLast.GetAbsDistToPrev_px( );
      //
      //m_dTotalDist_px += dAbsDistToPrev;
      //m_iNIntervals++;
      //
      //int iAdditionalIntervals = 0;
      //
      //if (!TryGetAdditionalIntervals( dAbsDistToPrev , dMasterBaseHorDist_px > 0 ? dMasterBaseHorDist_px : dROIWidth_px * 0.4 , __out iAdditionalIntervals ))
      //{
      //  bRes = false;
      //}
      //else
      //{
      //  m_iNIntervals += iAdditionalIntervals;
      //
      //  if (iAdditionalIntervals == 1 && m_iBeforeWideSpaceIndx < 0)
      //    m_iBeforeWideSpaceIndx = GetSize( ) - 2;
      //}
      //
      //if (/*isLast*/m_iNIntervals > 0 && bRes)
      //  m_dBaseHorDist_px = m_dTotalDist_px / m_iNIntervals;

    }

    if (bRes)
    {
      InvalidateMinDistToROICenter_px( newLast , GetSize( ) - 1 );

      InvalidateMinDistToZeroPos_px( newLast , GetSize( ) - 1 );

      InvalidateThickLineByLineWidtInPx( newLast.GetWidth_px( ) );
    }

    return bRes;
  }

  bool TrySetThickLineAbsPos_mm( double dNewAbsPos_mm )
  {
    return TrySetSpecialLineAbsPos_mm( dNewAbsPos_mm , GetThickLineIndx( ) , true );
  }

  bool TrySetBfrWideSpaceLineAbsPos_mm( double dNewAbsPos_mm )
  {
    return TrySetSpecialLineAbsPos_mm( dNewAbsPos_mm , GetBeforeWideSpaceIndx( ) , false );
  }

  bool TryMatchByPrevBatch( LinesBatch& prevLinesBatch ,
    double dShiftExpected_px ,
    double dBaseHorDist_mm ,
    double dZeroPos_mm,
    __out double& dAvgShiftActual_px
  )
  {
    //ASSERT( fabs( dShiftExpected_px ) < 300 );

    bool res = false;

    if (m_dBaseHorDist_px != 0
      && !prevLinesBatch.IsEmpty( )
      && !IsEmpty( ))
    {
      int iNMatched = 0;
      int iFrstMtchdIndxInThis = -1;
      double dTotalShiftsActual_px = 0;
      int iCountBeforeMatching = GetSize( );


      for (int i = 0; i < prevLinesBatch.GetSize( ); i++)
      {
        LineInfo& theLine_Prev = prevLinesBatch.GetCollection( )[ i ];

        double dPrevX_pix = theLine_Prev.GetCenterPosX_px( );
        double dNewPosExpected_pix = dPrevX_pix + dShiftExpected_px; // Expected position

        for (int j = 0; j < GetSize( ); j++)
        {
          LineInfo& theLine_Here = m_batch[ j ];
          double dShiftActual_px = theLine_Here.GetCenterPosX_px( ) - dPrevX_pix;


          double dPosPredictionErr_px = theLine_Here.GetCenterPosX_px( ) - dNewPosExpected_pix;
          if (fabs( dPosPredictionErr_px ) < m_dBaseHorDist_px * 0.5)//0.25)
          {
            ASSERT( theLine_Prev.HasAbsPos_mm( ) );
            theLine_Here
              .SetSiftFromPrevBatchActual_px(dShiftActual_px )
              .SetAbsPos_mm( theLine_Prev.GetAbsPos_mm( ) )
              .SetMatchedInPrevBatch( i);

            theLine_Prev.SetMatchedInNextBatch( j);

            //dXForSpeed += theLine_Here.GetCenterPosX_px( ) - dPrevX_pix;
            dTotalShiftsActual_px += dShiftActual_px;

            if (( i != 0 ) && ( j != 0 ) && prevLinesBatch.GetCollection( )[ i - 1 ].GetMatchedInNextBatch() == -1)
            {
              double dPrevIntervalLen_px = theLine_Here.GetAbsDistToPrev_px( );
              int iNMissingLines = ROUND( dPrevIntervalLen_px / m_dBaseHorDist_px ) - 1;

              if (iNMissingLines > 0 && TryInsertMissingLines( j - 1 , iNMissingLines , dPrevIntervalLen_px / ( iNMissingLines + 1 ) , dBaseHorDist_mm ))
              {
                dTotalShiftsActual_px = 0;
                iFrstMtchdIndxInThis = -1;
                i = -1;
                iNMatched = -1;
              }
            }

            if (iFrstMtchdIndxInThis < 0 && iNMatched >= 0)
              iFrstMtchdIndxInThis = j;

            iNMatched++;

            break;
          }
        }
      }

      if (iCountBeforeMatching != GetSize( ))
      {
        m_dWidthMax_pix = 0;
        m_dWidthMin_pix = DBL_MAX;
        m_iWMaxIndex = -1;

        InvalidateThickLine( );

        m_iBeforeWideSpaceIndx = -1;
        m_iNIntervals = -1;
        m_dTotalDist_px = 0;

        InvalidateLineInfo( );

      }

      if (!HasAllAbsPos_mm( ) && iFrstMtchdIndxInThis >= 0)
      {
        double dAbsPos_mm = m_batch[ iFrstMtchdIndxInThis ].GetAbsPos_mm( );

        if (HasThickLine( ))
        {
          if (GetThickLine( )->HasAbsPos_mm( ))
            dAbsPos_mm = GetThickLine( )->GetAbsPos_mm( );
          else
            dAbsPos_mm += ( dBaseHorDist_mm * ( m_iWMaxIndex - iFrstMtchdIndxInThis ) );
         
          if (dAbsPos_mm < 100 || dAbsPos_mm > dZeroPos_mm + dBaseHorDist_mm)
            dAbsPos_mm = dZeroPos_mm;

          ASSERT( dAbsPos_mm < DBL_MAX && dAbsPos_mm > 100 );
          
          TrySetThickLineAbsPos_mm( dAbsPos_mm );
        }
        else if (HasWideSpace( ))
        {
          if(GetLineBeforeWideSpace( )->HasAbsPos_mm())
            dAbsPos_mm = GetLineBeforeWideSpace( )->GetAbsPos_mm( );
          else
            dAbsPos_mm += ( dBaseHorDist_mm * ( m_iBeforeWideSpaceIndx - iFrstMtchdIndxInThis - 1 ) );


          TrySetBfrWideSpaceLineAbsPos_mm( dAbsPos_mm );
        }
        else
        {
          ReenumerateAbsPos_mm( iFrstMtchdIndxInThis , dAbsPos_mm , m_dBaseHorDist_px , dBaseHorDist_mm );
        }
      }

      ASSERT( HasAllAbsPos_mm( ) );

      res = HasAllAbsPos_mm( ) && iNMatched > 0 && iNMatched <= prevLinesBatch.GetSize( );

      if (res)
        dAvgShiftActual_px = dTotalShiftsActual_px / iNMatched;
    }

    return res;
  }

  bool TryGetFinalPos_mm( __out double& dFinalPos_mm ) const
  {
    bool res = false;
    LineInfo* pLine = GetLineNearestToZeroPos( );
    if (m_dPixelSize_mm > 0 &&
      m_dMinDistToZeroPos_px != DBL_MAX &&
      pLine != NULL)
    {
      dFinalPos_mm = pLine->GetAbsPos_mm( ) - ( m_dMinDistToZeroPos_px * m_dPixelSize_mm );

      res = dFinalPos_mm >= 0;
    }
    return res;
  }

private:
  bool TryFindWidthMaxIndx(
    double dWidthNew_px ,
    int iIndxOfNew  ,
    double& dWidthMin_px ,
    double& dWidthMax_px,
    __out int& iWidthMaxIndx )
  {
    bool hasRes = false;

    int res = -1;
    int iWMinIndex = -1;
    int iWMaxIndex = -1;

    SetMinMax( dWidthNew_px , dWidthMin_px , dWidthMax_px ,
      iIndxOfNew , iWMinIndex , iWMaxIndex );
    if (iWMaxIndex >= 0)
    {
      hasRes = true;
      iWidthMaxIndx = iWMaxIndex;
    }

    if (!hasRes && iWMinIndex >= 0)
      hasRes = true;

    return hasRes;
  }

  bool TryGetAdditionalIntervals( double dAbsDistToPrev_px , double dBaseHorDist_px , __out int& iAdditionalIntervals )const
  {
    bool bRes = false;

    const double c_dTolerance = 0.1;

    double dIntervals = fabs( dAbsDistToPrev_px / dBaseHorDist_px );
    int iIntervals = ( int )( 0.5 + dIntervals );
    double dReminder = fabs( dIntervals - iIntervals );

    if (dReminder < c_dTolerance)
    {
      bRes = true;
      iAdditionalIntervals = iIntervals == 0 ? 0 : iIntervals - 1;
    }

    return bRes;
  }


  double InvalidateAbsPos_mm( double dAbsPos_mm , bool bIsForThick = true )
  {
    const int iReminderThickLine_mm = 50;
    const int iReminderBeforeWideSpace_mm = 90;
    const int iFullIntervalBtwnThickLinesOnly_mm = 2 * iReminderThickLine_mm;

    double res = dAbsPos_mm;

    int iQtyOfFullIntervalBtwnThickLinesOnly = ( int )( res / iFullIntervalBtwnThickLinesOnly_mm );
    int iTotalLenOfAllFullIntervalsOnly_mm = iQtyOfFullIntervalBtwnThickLinesOnly * iFullIntervalBtwnThickLinesOnly_mm;
    int iReminderCalculated_mm = ( int )( res - iTotalLenOfAllFullIntervalsOnly_mm );

    int iReminderExpected_mm = iReminderBeforeWideSpace_mm;

    if (bIsForThick /*&& iReminderCalculated_mm != iReminderThickLine_mm*/)
      iReminderExpected_mm = iReminderThickLine_mm;

    res = iTotalLenOfAllFullIntervalsOnly_mm + iReminderExpected_mm;

    if (!bIsForThick && iReminderCalculated_mm < iReminderThickLine_mm)
      res -= iFullIntervalBtwnThickLinesOnly_mm;

    return res;
  }

  bool InvalidateMinDistToROICenter_px( const LineInfo& theLine , int iLineIndx )
  {
    bool res = false;
    double dDist_px = theLine.GetAbsDistToROICntrX_px( );
    if (InvalidateDistAsMinDist (dDist_px , m_dAbsMinDistToROICntrX_px))
    {
      m_dAbsMinDistToROICntrX_px = dDist_px;
      m_iNearestToROICntrXIndx = iLineIndx;

      res = true;
    }

    return res;
  }
  bool InvalidateMinDistToZeroPos_px( const LineInfo& theLine , int iLineIndx )
  {
    bool res = false;
    double dDist_px = theLine.GetDistToZeroPosX_px( );
    if ( InvalidateDistAsMinDist( fabs( dDist_px ) , fabs( m_dMinDistToZeroPos_px ) ))
    {
      m_dMinDistToZeroPos_px = dDist_px;
      m_iNearestToZeroPosIndx = iLineIndx;

      res = true;
    }

    return res;
  }
  bool InvalidateDistAsMinDist( double dCrntDist , double dLastMinDist )
  {
    return dCrntDist < dLastMinDist;
  }

  bool InvalidateThickLineByLineWidtInPx( double dWidth_px )
  {
    bool res = false;

    if (!HasThickLine( ))
    {
      int iWidthMaxIndx = -1;
      if (TryFindWidthMaxIndx( dWidth_px , GetSize( ) - 1 , m_dWidthMin_pix , m_dWidthMax_pix , iWidthMaxIndx )
        && iWidthMaxIndx >= 0)
      {
        double dDeltaW = m_dWidthMax_pix - m_dWidthMin_pix;

        if (( dDeltaW / m_dWidthMin_pix ) > 0.5)
        {
          m_iWMaxIndex = iWidthMaxIndx;
          m_batch[ m_iWMaxIndex ].SetAsThick( HasThickLine( ) );
          res = true;
        }
      }

    }

    return res;
  }

  bool InvalidateThickLine( )
  {
    bool res = false;

    if (!HasThickLine( ))
    {

      for (deque<LineInfo>::const_iterator ci = m_batch.begin( ); !res && ci != m_batch.end( ); ci++)
        res = InvalidateThickLineByLineWidtInPx( ci->GetWidth_px() );
    }

    return res;
  }

  bool InvalidateLineInfo(const LineInfo& lineInfo , double dBaseHorDist_px )
  {
    bool bRes = true;

    double dAbsDistToPrev = lineInfo.GetAbsDistToPrev_px( );

    m_dTotalDist_px += dAbsDistToPrev;
    m_iNIntervals++;

    int iAdditionalIntervals = 0;

    if (!TryGetAdditionalIntervals( dAbsDistToPrev , dBaseHorDist_px , __out iAdditionalIntervals ))
    {
      bRes = false;
    }
    else
    {
      m_iNIntervals += iAdditionalIntervals;

      if (iAdditionalIntervals == 1 && m_iBeforeWideSpaceIndx < 0)
      {
        m_iBeforeWideSpaceIndx = m_iNIntervals - 2;

        m_bHasDigitsRegion = true;
      }
    }

    if (m_iNIntervals > 0 && bRes)
      m_dBaseHorDist_px = m_dTotalDist_px / m_iNIntervals;

    return bRes;
  }

  bool InvalidateLineInfo( )
  {
    bool bRes = false;

    for (deque<LineInfo>::const_iterator ci = m_batch.begin( ); !bRes && ci != m_batch.end( ); ci++)
    {
      bRes = InvalidateLineInfo( *ci , m_dBaseHorDist_px );
    }

    return bRes;
  }

  LinesBatch& ReenumerateAbsPos_mm( int iKnownIndex , double dKnownAbsPosition_mm , double dBaseHorDist_px , double dBaseHorDist_mm = 10. )
  {
    double dKnownPos_px = -1;
    if (iKnownIndex >= 0 && iKnownIndex < GetSize( ))
      dKnownPos_px = m_batch[ iKnownIndex ].GetCenterPosX_px( );

    for (int i = 0; i < GetSize( ) && dKnownPos_px>0; i++)
    {
      ASSERT( dBaseHorDist_px > 0 );

      LineInfo& thisLine = m_batch[ i ];

      double dDistFromThisToKnown_px = thisLine.GetCenterPosX_px( ) - dKnownPos_px;

      int iIntervalsInDistFromThisToKnownPx = ( int )( 0.5 * ( dDistFromThisToKnown_px < 0 ? -1 : 1 ) + dDistFromThisToKnown_px / dBaseHorDist_px );

      double newAbsPos_mm = dKnownAbsPosition_mm + dBaseHorDist_mm * iIntervalsInDistFromThisToKnownPx;

      thisLine.SetAbsPos_mm( newAbsPos_mm );
    }

    return *this;
  }

  bool TrySetSpecialLineAbsPos_mm( double dNewAbsPos_mm , int iSpecialLineIndx , bool bIsThickLine )
  {
    bool res = false;

    if (iSpecialLineIndx >= 0 && dNewAbsPos_mm > 0)
    {
      double dValidAbsPos_mm = InvalidateAbsPos_mm( dNewAbsPos_mm , bIsThickLine );

      ReenumerateAbsPos_mm( iSpecialLineIndx , dValidAbsPos_mm , GetBaseHorDist_px( ) );

      res = true;
    }

    return res;
  }

  bool TryInsertMissingLines(
    int iFrstKnownIndx ,
    int iNMissingLinesToInsert ,
    double dCalcDistToNextLine_px ,
    double dBaseHorDist_mm )
  {
    bool res = true;

    if (iFrstKnownIndx >= 0 && iFrstKnownIndx < GetSize( ))
    {
      LineInfo& knwnLine_frst = m_batch[ iFrstKnownIndx ];
      LineInfo& knwnLine_last = m_batch[ iFrstKnownIndx + 1 ];
      int iAbsPos_mm_Frst = -1;
      if (knwnLine_frst.HasAbsPos_mm( ))
        iAbsPos_mm_Frst = ( int )knwnLine_frst.GetAbsPos_mm();
      else
        iAbsPos_mm_Frst = ( int )knwnLine_last.GetAbsPos_mm() - iNMissingLinesToInsert - 1;

      double dMissingLineWidth_px = knwnLine_frst.GetWidth_px( );
      if (knwnLine_frst.IsThick( ))
        dMissingLineWidth_px /= 2;

      for (int i = 1; i <= iNMissingLinesToInsert; i++)
      {
        cmplx cMssngPt = knwnLine_frst.GetCenterPos( );
        cMssngPt += cmplx( dCalcDistToNextLine_px * i , 0 );
        double dLineWidth_px = dMissingLineWidth_px;

        int iExpectedAbsPos_mm = iAbsPos_mm_Frst + ( int )dBaseHorDist_mm * i;

        if (iExpectedAbsPos_mm % 100 == 50)
          dLineWidth_px *= 2;

        LineInfo missingLine( cMssngPt , dMissingLineWidth_px , -1 , 0 , knwnLine_frst.GetROICenterX_px( ), knwnLine_frst.GetZeroPosX_px(), dBaseHorDist_mm );

        //missingLine.CopyZeroPosX_px( knwnLine_frst );

        res &= TryInsertMissingLine( missingLine , iFrstKnownIndx + i );

        if (res)
        {
          int iInsertedIndx = iFrstKnownIndx + i;

          LineInfo* pInsertedLine = &m_batch[ iInsertedIndx ];

          m_batch[ iInsertedIndx - 1 ].SetNext( pInsertedLine );

          if (i == iNMissingLinesToInsert)
          {
            //LineInfo* pAfterLasInsertedLine = &m_batch[ iInsertedIndx + 1 ];

            //pInsertedLine->SetNext( pAfterLasInsertedLine );
            knwnLine_last
              .SetPrev(NULL) //Reset prev. line reference and distance to this line;
              .SetPrev( pInsertedLine );
          }

          InvalidateMinDistToROICenter_px( *pInsertedLine , iInsertedIndx );

          InvalidateMinDistToZeroPos_px( *pInsertedLine , iInsertedIndx );
        }
      }
    }

    return res;
  }

  bool TryInsertMissingLine( const LineInfo& missingLine , int iNdxFromStart )
  {
    bool res = false;

    int iSizeBefore = GetSize( );

    if (iSizeBefore > 0)
      m_batch.insert( m_batch.begin( ) + iNdxFromStart , missingLine );

    if (GetSize( ) - iSizeBefore > 0)
    {
      res = true;
      TRACE( "\nLine %d inserted Pt(%.1f,%.1f) W=%.2f" ,
        iNdxFromStart ,
        missingLine.GetCenterPos( ) ,
        missingLine.GetWidth_px() );
    }
    return res;
  }
};

typedef deque<LinesBatch> LinesSequence;

#include <map>

class LinesBatchStatistics
{



private:
  map<int , deque<LineInfo>> m_Hisrogramm;

  double m_dMasterBaseInterval_px;

public:
  LinesBatchStatistics& Reset( )
  {
    m_Hisrogramm.clear( );
    return *this;
  }

  LinesBatchStatistics& Add( const LineInfo& li )
  {
    int iLinePos_px = ( int )li.GetCenterPosX_px();
    m_Hisrogramm[ iLinePos_px ].push_front(li);
    return *this;
  }

  int GetCount( ) const
  {
    int iRes = 0;

    for (map<int , deque<LineInfo>>::const_iterator ci = m_Hisrogramm.begin( ); ci != m_Hisrogramm.end( ); ci++)
    {
      if (iRes < (int)(ci->second.size()))
      {
        iRes = ( int )(ci->second.size());
      }
    }

    return iRes;
  }

  bool TryGetMasterBaseInterval_px(int iQtyToAnalize, __out double& dMasterBaseInterval_px )
  {
    bool bRes = false;
    int iQty = GetCount( );
    if (iQty < iQtyToAnalize)
      dMasterBaseInterval_px = 0;
    else
    {
      int iPos_px_Frst = 0;
      int iPos_px_Scnd = 0;
      int iNmbrOfIntervals = 0;
      double dTotalDist_px = 0;

      for (map<int , deque<LineInfo>>::const_iterator ci = m_Hisrogramm.begin( ); ci != m_Hisrogramm.end( ); ci++)
      {
        if (iQty == ( int )( ci->second.size( ) ))
        {
          iPos_px_Frst = iPos_px_Scnd;

          iPos_px_Scnd = ci->first;
          if (iPos_px_Frst == 0)
            iPos_px_Frst = iPos_px_Scnd;
          else if(abs(iPos_px_Scnd-iPos_px_Frst )>0)
          {
            dTotalDist_px += abs( iPos_px_Scnd - iPos_px_Frst );
            iNmbrOfIntervals++;
          }

        }
      }

      if (iNmbrOfIntervals > 0)
        dMasterBaseInterval_px = dTotalDist_px / iNmbrOfIntervals;
    }

    return bRes;
  }
};

class RulTracker :
  public UserBaseGadget
{
private:
  static const int       msc_iDigitsPeriod_mm;
  static const double    msc_dDigitsRegionWidth_mm;
  static const Segment1d msc_DigitsRegionInPeriod_mm;
  static const LinesBatch msc_LinesBatchEmpty;

  static double          ms_dBaseHorDist_mm;
  static int             ms_iViewMode;
  
  static       double    ms_dZeroPos_mm;
  static       int       ms_iMaskToSetZeroPos; // bit 0 - set side camera, bit 1 - set top camera

  static   DWORD  ms_dLastVFIdPerGdgtMode[ 2 ];
  static   double ms_dLastFrameTimePerGdgtMode_ms[ 2 ];

  FXString m_sName;



  RulTrackerGadgetMode m_GadgetMode;

public:
  RulTracker( );
  ~RulTracker( );


  // Angle calculation data
  static   double ms_dAngle_deg;
  static   double ms_dSideAngle_deg;
  static   double ms_dTopAngle_deg;
  static   Segment1d ms_SideEdges_deg , m_OldSideEdges_deg ;
  static   Segment1d ms_TopEdges_deg , m_OldTopEdges_deg;
  static   double ms_dAngleShift_deg ;
  // End of angle calculation data


  double   m_dZeroPosOnImage_pix = 0.; // per camera
  double   m_dPixelSize_mm = 1.e6;
  double   m_dLastX_mm = -100000.;
  double   m_dLastAngle_deg = 0.;
  double   m_dLastSpaceSize_deg = 20.;
  double   m_dLastV_pps; // speed in pix per sec
  double   m_dMaxV_pps; // Max speed in pix per sec
  double   m_dMaxSpeedSetTime_ms;
  double   m_dAcceleration_pps2;
  int      m_iNNearestToCenterIndex = -1;
  double   m_dTolerance_um;
  double   m_dSearchWidth_pix = 30.;
  double   m_dLineSearchThres = 0.25;
  double   m_dMinLineWidth = 3. , m_dMaxLineWidth = 35.;
  double   m_dMinimalContrast = 60.; // in brightness units
  int      m_iRecordPeriod_sec = 0;
  FXString m_sThresholdForLog;
  FXString m_sRecalibrateCause;
  //FXString m_sSavedLinePositions;

  double   m_dMasterBaseHorDist_px = 0;
  LinesBatchStatistics  m_Statistics;


  const CVideoFrame * m_pLastVideoFrame = NULL;
  double              m_dLastFrameOrderTime = 0.;
  CRect    m_LastROI_px;
  cmplx    m_cLastROI_Cntr_px;

  cmplx    m_cInitialSearchPt_pix;
  cmplx    m_cInitOrthoSearchPt_pix;

  LinesBatch* m_pLastLines = NULL; //(LinesBatch());
  LinesBatch* m_pPrevLines = NULL; //( LinesBatch ());
  LinesBatch* m_pLL = NULL;

  LinesSequence m_LineBatches;

#define LinesHistorySize 200
  OneLinesSample m_OriginLinesHistory[ 200 ];
  OneLinesSample m_ProcessedLinesHistory[ LinesHistorySize ];
  int   m_iNextHistoryIndex = 0;

  CLineResult m_BodyInfo;

  Segment1d   m_ROIofZeroPos_mm;
  //Segment1d   m_DigitsArea_pix;
  //Segment1d   m_DigitsArea_mm;
  Segment1d   m_PartOfDigitsRegionInROI_px_Predicted;

  // Exposure control
  int          m_iCurrentExposure_us;

  cmplx        m_cCrossesAverage;

  double       m_dWidthMax_pix;
  double       m_dWidthMin_pix;
  int          m_iWMinIndex;

  Segments1d  m_AllSegments;
  char * m_pAngleSelectedBy = "UN";
  // Result view parameters

  DWORD   m_BorderResultColor = 0xffc0c0;
  int     m_iPageBorderThickness = 3;
  int     m_PageBorderStyle = PS_DASH;
  DWORD   m_PageColor = 0xc0c0c0;

  // Power status data

  FXLockObject        m_LockTimer;
  int                 m_dwTimerPeriod = 100;
  int                 m_iTimeCount = 0;
  double              m_dTimerDeleteOrderTime = 0.;
  SYSTEM_POWER_STATUS m_SystemPowerStatus;
  FXString            m_PowerStatusAsString;
  CContainerFrame *   m_pLastResultViewWithoutPower = NULL;
  double              m_dLastResultViewTime = 0.;
  //double              ms_dLastFrameTimePerGdgtMode_ms;
  double              m_dTimeBetweenFrames_ms;

  // Capture mode: 0 - pause, -1 - live 10 fps, n>0 - take n frames
  static int          m_iCaptureMode;
  // Mode switching rectangles
  CRect               m_LiveProcessingRect;
  CRect               m_LiveViewRect;
  CRect               m_SingleFrameProcessingRect;
  CRect               m_QuitRect;
  int                 m_iLastSelectedByUI = 0;
  //int                 m_iNearestToCenter = 0;

  // Log variables
  FXString            m_LogFileName;
  double              m_dLastLoggedTime_ms = 0.;

  double              m_dPrevSkew_um = 0.;
  double              m_dPrevHError_um = 0.;
  double              m_dPrevVError_um = 0.;
  double              m_dPrevHLineDist_um = 0.;
  


  //   void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );

  void PropertiesRegistration( );
  void ConnectorsRegistration( );
  static void ConfigParamChange( LPCTSTR pName ,
    void* pObject , bool& bInvalidate , bool& bInitRescan );
  virtual void ShutDown( );

  BOOL CameraControl( LPCTSTR pCommand );
  //int ExtractData( LinesV& FoundLines , const CDataFrame * pData );
  
  int FormAndSendResultView( );
  int GetMarkerExtrems( const CVideoFrame * pVF , LineInfo& Marker ,
    double& dUpperExtrem , double& dLowerExtrem , CContainerFrame * pMarking );
  bool IsDigitsInFOV( double dCurrPos , const Segment1d& FOVofZeroPos_mm , __out Segment1d& DigitsSegment );
  //int TryMatchLines( ); // compares previous and last lines and do transfer coordinates
  int SetCameraExposure( int iExposure_us );

  //RulTracker& InvalidateAbsPosDit_mm( double dAbsPos_mm_Frst , double dAbsPos_mm_Scnd )
  //{
  //  double dDist_mm = dAbsPos_mm_Frst - dAbsPos_mm_Scnd;
  //  if (dDist_mm > ms_dBaseHorDist_mm * 1.2)
  //  {
  //    //ASSERT( dDist_mm > m_dLastBaseHorDist_pix * 1.2 );
  //  }
  //  return *this;
  //}

  DECLARE_RUNTIME_GADGET( RulTracker );
  //int ReenumerateAbsPositions( int iKnownIndex , double dKnownAbsPosition );

private:
  double GetAngle() ;
  
  double GetTimestamp_ms( ) const
  {
    return ms_dLastFrameTimePerGdgtMode_ms[ ( int )m_GadgetMode - 1 ];
  }
  const RulTracker& SetTimestamp_ms( double dTimestamp_ms )const
  {
    ms_dLastFrameTimePerGdgtMode_ms[ ( int )m_GadgetMode - 1 ] = dTimestamp_ms;
    return *this;
  }

  DWORD GetFrameId( ) const
  {
    return ms_dLastVFIdPerGdgtMode[ ( int )m_GadgetMode - 1 ];
  }
  const RulTracker& SetFrameId( DWORD dwFrameId ) const
  {
    ms_dLastVFIdPerGdgtMode[ ( int )m_GadgetMode - 1 ] = dwFrameId;
    return *this;
  }

  double GetFrameToFrameTime_ms(double dCrntFrameTime_ns )const
  {
    double dFrameToFrameTime_ms = 0;
    double dCrntFrameTime_ms = 1.e-3 * dCrntFrameTime_ns;
    double dPrevFrameTime_ms = GetTimestamp_ms( );

    dFrameToFrameTime_ms = dCrntFrameTime_ms - dPrevFrameTime_ms;

    SetTimestamp_ms( dCrntFrameTime_ms );

    return dFrameToFrameTime_ms;
  }

  int GetFrameToFrameIDs(DWORD dwCrntFrameId )const
  {
    int dFrameToFrameIDs = 0;
    DWORD dCrntFrameID = dwCrntFrameId;
    DWORD dPrevFrameID = GetFrameId();

    dFrameToFrameIDs = (int)(dCrntFrameID - dPrevFrameID);

    SetFrameId( dCrntFrameID );

    return dFrameToFrameIDs;
  }

  bool TryExtractZeroPos_mm( const CDataFrame* pDataFrame , __out double& dZeroPos_mm ) const
  {
    bool res = false;

    if (!pDataFrame->IsContainer( ))
    {
      const CTextFrame * pTF = pDataFrame->GetTextFrame( );
      if (pTF)
      {
        FXPropertyKit pk( pTF->GetString( ) );
        if (_tcscmp( pTF->GetLabel( ) , _T( "SetAnchor" ) ) == 0)
        {
          double dAnchorVal;
          if (pk.GetDouble( "AnchorValue" , dAnchorVal ))
          {
            dZeroPos_mm = dAnchorVal;
            //ms_iMaskToSetZeroPos = 3; // for both cameras

            res = dZeroPos_mm > 0;
          }
        }
        else
        {
          int iX = 0 , iY = 0;
          if (pk.GetInt( "x" , iX ) && pk.GetInt( "y" , iY )
            && ( pk.Find( "selected" ) >= 0 ))
          {
          }
        }

      }
    }

    return res;
  }
  
  bool TryExtractData(
    CFramesIterator * pData ,
    const CRect& ROI_px ,
    double dMasterBaseHorDist_px ,
    double dZeroPosX_px ,
    double dBaseHorDist_mm ,
    const Segment1d& digitsRegionInROI_px ,
    __out LinesBatch& FoundLines ,
    __out LinesBatchStatistics* pStatistics );

  bool IsZeroPosCalibrated( ) const
  {
    return ms_dZeroPos_mm > 0
      && m_dZeroPosOnImage_pix > 0
      && m_dPixelSize_mm > 0.
      && ( ms_iMaskToSetZeroPos & m_GadgetMode ) != m_GadgetMode;
  }

  bool IsReadyToCabrateZeroPosInROI_px(const LinesBatch& linesBatch, double dZeroPos_mm, RulTrackerGadgetMode crntGdgtMode ) const
  {
    return linesBatch.HasThickLine( )
      && dZeroPos_mm > 0
      && ( ms_iMaskToSetZeroPos & crntGdgtMode ) == crntGdgtMode;
  }

  bool TryCalibrateZeroPosInROI_px(
    LinesBatch& linesBatch ,
    double dZeroPos_mm ,
    bool bIsReadyToCalibateZeroPos,
   // RulTrackerGadgetMode crntGdgtMode,
    __out double& dZeroPosInROI_px ,
    __out double& dAbsShortestDist_px,
    __out bool& bIsShortestDistIsPrev)
  {
    bool res = false;
    double dTmpZeroPosInROI_px;
    double dTmpAbsShortestDist_px = 0;
    bool bIsPrev = true;;
    
    LinesBatch tmpLinesBatch = linesBatch;

    if (bIsReadyToCalibateZeroPos &&
      tmpLinesBatch.TrySetThickLineAbsPos_mm( dZeroPos_mm ) &&
      tmpLinesBatch.HasAllAbsPos_mm())
    {
      linesBatch.TrySetThickLineAbsPos_mm( dZeroPos_mm );

      LineInfo& thickLine = *linesBatch.GetThickLine( );
      
      dTmpAbsShortestDist_px = thickLine.GetShortestAbsDist_px(bIsPrev);
      
      dTmpZeroPosInROI_px = thickLine.GetCenterPosX_px( );
        //double dDistToRight_px = thickLine.GetAbsDistToNext_px( );
        //double dDistToLeft_px = thickLine.GetAbsDistToPrev_px( );
//
        //dShortestDist_px = dDistToRight_px;
//
        //if (dShortestDist_px == 0 || ( dDistToLeft_px > 0 && dDistToLeft_px < dShortestDist_px ))
        //  dShortestDist_px = dDistToLeft_px;
        //
        //m_dZeroPosOnImage_pix = dThickLinePos_px;
        //
        //if (dShortestDist_px > 0.)
        //  m_dPixelSize_mm = ms_dBaseHorDist_mm / dShortestDist_px;
      if (linesBatch.HasAllAbsPos_mm() &&
        dTmpZeroPosInROI_px > 0 &&
        dTmpAbsShortestDist_px > 0)
      {
        //linesBatch.

        dZeroPosInROI_px = dTmpZeroPosInROI_px;
        dAbsShortestDist_px = dTmpAbsShortestDist_px;
        bIsShortestDistIsPrev = bIsPrev;

        res = true;

      }

      //if (dAbsShortestDist_px > 0)
      //{
      //  m_ROIofZeroPos_mm.m_dBegin = ms_dZeroPos_mm - ( m_dZeroPosOnImage_pix * m_dPixelSize_mm );
      //  m_ROIofZeroPos_mm.m_dEnd = ms_dZeroPos_mm + ( ( m_LastROI_px.Width( ) - m_dZeroPosOnImage_pix ) * m_dPixelSize_mm );
      //  //        m_FOVofZeroPos_mm = m_CurrentViewArea_mm;
      //  double dAnchorSegmentLength_px = dAbsShortestDist_px;
      //  double dNearestDigitsViewCenter_mm = ms_dZeroPos_mm + 50;// ms_dDigitsAreaShift_mm;
      //  double dNearestDigitsViewCenter_pix = dNearestDigitsViewCenter_mm / m_dPixelSize_mm;
      //  //        m_DigitsArea_mm.m_dBegin = ms_dDigitsAreaShift_mm - msc_dDigitsAreaWidth_mm / 2.;
      //  //        m_DigitsArea_mm.m_dEnd = ms_dDigitsAreaShift_mm + msc_dDigitsAreaWidth_mm / 2.;
      //
      //          //m_DigitsArea_pix.m_dBegin = msc_DigitsArea_mm.m_dBegin / m_dPixelSize_mm;
      //          //m_DigitsArea_pix.m_dEnd = msc_DigitsArea_mm.m_dEnd / m_dPixelSize_mm;
      //
      //  m_DigitsRegionInROI_px.m_dBegin = dNearestDigitsViewCenter_pix - dAnchorSegmentLength_px / 2.;
      //  m_DigitsRegionInROI_px.m_dEnd = dNearestDigitsViewCenter_pix + dAnchorSegmentLength_px / 2.;
      //  
      //}

      ms_iMaskToSetZeroPos &= ~m_GadgetMode;
    }


    return res;
  }

  double GetPixelSize_mm( double dDistInMM , double dDistInPx ) const
  {
    return dDistInPx == 0 ? 0 : dDistInMM / dDistInPx;
  }

  bool IsPixelSizeInMmValid( ) const
  {
    return m_dPixelSize_mm != 0;
  }

  double GetDistOfPixelsInMM( double dBegin_px , double dEnd_px , double dPixelSize_mm ) const
  {
    return ( dEnd_px - dBegin_px ) * dPixelSize_mm;
  }

  double GetDistOfMMInPx( double dBegin_mm , double dEnd_mm , double dPixelSize_mm ) const
  {
    return ( dEnd_mm - dBegin_mm ) / dPixelSize_mm;
  }

  bool TryMatchLines(
    LinesBatch& linesBatchPrev ,
    LinesBatch& linesBatchNew ,
    double dShiftExpected_px ,
    double dBaseHorDist_mm ,
    double dZeroPos_mm ,
    __out double& dAvgShiftActual_px ); // compares previous and last lines and do transfer coordinates
  
  Segment1d GetROIforAbsPos_mm( double dAbsPos_mm )const
  {
    Segment1d res = m_ROIofZeroPos_mm;
    res.Offset( dAbsPos_mm - ms_dZeroPos_mm );

    return res;
  }

  Segment1d GetPartOfDigitsRegionInROI_mm( double dAbsPos_mm , const Segment1d& digitsRegionInPeriod_mm ) const
  {
    return GetPartOfDigitsRegionInROI_mm( GetROIforAbsPos_mm( dAbsPos_mm ) , digitsRegionInPeriod_mm );
  }

  Segment1d GetPartOfDigitsRegionInROI_mm(const Segment1d& ROIofAbsPos_mm , const Segment1d& digitsRegionInPeriod_mm ) const
  {
    Segment1d res;

    double dDigitsPeriodIDofAbsPos_mm =
      ( ( int )ROIofAbsPos_mm.m_dBegin / msc_iDigitsPeriod_mm ) -
      ( fmod( ROIofAbsPos_mm.m_dBegin , msc_iDigitsPeriod_mm ) < ( msc_dDigitsRegionWidth_mm / 2 ) ? 1 : 0 );

    Segment1d digitsRegionInPeriodIDofAbsPos_mm = digitsRegionInPeriod_mm;
    digitsRegionInPeriodIDofAbsPos_mm.Offset( dDigitsPeriodIDofAbsPos_mm * msc_iDigitsPeriod_mm );

    ASSERT( digitsRegionInPeriodIDofAbsPos_mm.GetLength( ) == msc_dDigitsRegionWidth_mm );

    res = ROIofAbsPos_mm;

    if (!res.Intersection( digitsRegionInPeriodIDofAbsPos_mm ))
      res.m_dBegin = res.m_dEnd = 0;

    ASSERT( res.GetLength( ) >= 0 && res.GetLength( ) <= msc_dDigitsRegionWidth_mm );

    return res;
  }

  Segment1d GetPartOfDigitsRegionInROI_px( double dAbsPosOfLeftROI_mm , const Segment1d& partOfDigitsRegionInROI_mm , double dPixelSize_mm ) const
  {
    Segment1d res;

    if (dAbsPosOfLeftROI_mm > 0 && partOfDigitsRegionInROI_mm.GetLength( ) > 0)
    {
      ASSERT( dAbsPosOfLeftROI_mm <= partOfDigitsRegionInROI_mm.m_dBegin );
      ASSERT( dAbsPosOfLeftROI_mm <= partOfDigitsRegionInROI_mm.m_dEnd );

      res.m_dBegin = GetDistInPixels( dAbsPosOfLeftROI_mm , partOfDigitsRegionInROI_mm.m_dBegin , dPixelSize_mm );
      res.m_dEnd = GetDistInPixels( dAbsPosOfLeftROI_mm , partOfDigitsRegionInROI_mm.m_dEnd , dPixelSize_mm );

      ASSERT( res.GetLength( ) >= 0 );
    }

    return res;
  }

  Segment1d GetPartOfDigitsRegionInROI_px( const Segment1d& digitsRegionInPeriod_mm , double dAbsPos_mm , double dPixelSize_mm ) const
  {
    Segment1d res;

    Segment1d ROIforAbsPos_mm = GetROIforAbsPos_mm( dAbsPos_mm );
    double dAbsPosOfLeftROI_mm = ROIforAbsPos_mm.m_dBegin;
    if (ROIforAbsPos_mm.GetLength( ) > 0 && dAbsPosOfLeftROI_mm > 0)
    {
      res = GetPartOfDigitsRegionInROI_px( dAbsPosOfLeftROI_mm , GetPartOfDigitsRegionInROI_mm( ROIforAbsPos_mm , digitsRegionInPeriod_mm ) , dPixelSize_mm );
    }

    return res;
  }

  double GetDistInPixels( double dBegin_mm , double dEnd_mm , double dPixelSize_mm ) const
  {
    double res = 0;
    if (dPixelSize_mm != 0)
    {
      res = ( dEnd_mm - dBegin_mm ) / dPixelSize_mm;
    }
    return res;
  }

  bool TryGetCurrentPosRelativeToZero_mm( LineInfo* pLineNearestToROICntr ,
    double dZeroPosOnImage_px ,
    double dPixelSize_mm ,
    __out double& dCurrentPosRelativeToZero_mm )
  {
    bool bIsDone = false;
    double res = -1;

    //LineInfo* pLineNearestToROICntr = m_LastLines.GetLineNearestToROICntrX( );
    if (pLineNearestToROICntr != NULL
      && dZeroPosOnImage_px > 0
      && dPixelSize_mm > 0)
    {
      double dDistToZeroOnImage_px = pLineNearestToROICntr->GetCenterPosX_px( ) - dZeroPosOnImage_px;
      double dDistToZero_mm = dDistToZeroOnImage_px * dPixelSize_mm;
      res = pLineNearestToROICntr->GetAbsPos_mm( ) - dDistToZero_mm;

      dCurrentPosRelativeToZero_mm = res;

      bIsDone = res > 0;
    }

    return bIsDone;
  }

};
