// TVHistogram.h : Implementation of the TVHistogram class


#include "StdAfx.h"
#include "TVHistogram.h"
#include <imageproc\imagebits.h>
#include <gadgets\FigureFrame.h>
#include <gadgets\VideoFrame.h>
#include <gadgets\TextFrame.h>
#include <gadgets\ContainerFrame.h>
#include <helpers\propertykitEx.h>
#include <helpers\\FramesHelper.h>

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
    if (!(vfr) || ((vfr)->IsNullFrame()))	    \
        {	                                        \
        return NULL;			                    \
        }                                           \
}

#define BASE_Y (260.)
#define BASE_X (10.)
#define HISTOGRAM_LENGTH (256)

inline double GetHistogram( const pTVFrame frame , 
  CRect& prc , CFigureFrame& DataBuffer , double * pdStd = NULL )
{
  DataBuffer.RemoveAll();
  CRect rc( prc ) ;
  int iImWidth = GetWidth( frame ) ;
  int iImHeigth = GetHeight( frame ) ;
  if ( rc.top >= iImHeigth  || rc.left >= iImWidth )
    return 0. ;
  if ( rc.top < 0 )
    rc.top = 0 ;
  if ( rc.bottom >= iImWidth )
    rc.bottom = iImHeigth - 1 ;
  if ( rc.left < 0 )
    rc.left = 0 ;
  if ( rc.right >= iImWidth )
    rc.right = iImWidth - 1 ;

  int iWidth = rc.Width() ;
  int iHeight = rc.Height() ;
  int iInterlace = iImWidth ;
  int iNSamples = iWidth * iHeight;
  double dAverage = 0.;
  
  if ( iWidth && iHeight )
  {
    switch ( frame->lpBMIH->biCompression )
    {
      case BI_YUV12:
      case BI_YUV9:
      case BI_Y8:
      case BI_Y800:
      case BI_Y16:
      {
        if ( DataBuffer.GetCount() != 256 )
        {
          DataBuffer.SetSize( 256 ) ;
          for ( int i = 0; i < 256; i++ )
          {
            DataBuffer[ i ].x = i + BASE_X;
            DataBuffer[ i ].y = 0 ;
          }
        }
        else
        {
          for ( int i = 0 ; i < 256 ; i++ )
            DataBuffer[ i ].y = 0 ;
        }
      }
      break ;
      default:
        TRACE( "\nUnsupported data type 0x%08X in Histogram " , frame->lpBMIH->biCompression ) ;
        return 0. ; // other types we are not recognize
    }
  }
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      LPBYTE pData = GetData( frame ) + rc.left ;
      for ( int iY = rc.top ; iY <= rc.bottom ; iY++ )
      {
        LPBYTE pRow = pData + iInterlace * iY ;
        LPBYTE pEnd = pRow + iWidth ;
        while (pRow < pEnd)
        {
          DataBuffer[*pRow].y++;
          dAverage += *(pRow++);
        }
      }
      dAverage /= iNSamples ;
      if ( pdStd )
      {
        double dSum2 = 0.;
        pData = GetData(frame) + rc.left;
        for (int iY = rc.top; iY <= rc.bottom; iY++)
        {
          LPBYTE pRow = pData + iInterlace * iY;
          LPBYTE pEnd = pRow + iWidth;
          while (pRow < pEnd)
          {
            double dVal = (*(pRow++)) - dAverage ;
            dSum2 += dVal * dVal;
          }
        }
        dSum2 /= iNSamples ;
        *pdStd = sqrt(dSum2);
      }
    }
    break ;
    case BI_Y16:
    {
      LPWORD pData = GetData16( frame ) + rc.left ;
      for ( int iY = rc.top ; iY <= rc.bottom ; iY++ )
      {
        LPWORD pRow = pData + iInterlace * iY ;
        LPWORD pEnd = pRow + iWidth ;
        while (pRow < pEnd)
        {
          DataBuffer[(*pRow) >> 8].y++;
          dAverage += *(pRow++);
        }
      }
      dAverage /= iNSamples ;
      if (pdStd)
      {
        double dSum2 = 0.;
        pData = GetData16(frame) + rc.left;
        for (int iY = rc.top; iY <= rc.bottom; iY++)
        {
          LPWORD pRow = pData + iInterlace * iY;
          LPWORD pEnd = pRow + iWidth;
          while (pRow < pEnd)
          {
            double dVal = (*(pRow++)) - dAverage;
            dSum2 += dVal * dVal;
          }
        }
        dSum2 /= iNSamples;
        *pdStd = sqrt(dSum2);
      }

    }
    break ;
  }
  return dAverage ;
}

IMPLEMENT_RUNTIME_GADGET_EX( TVHistogram , CFilterGadget , "Video.statistics" , TVDB400_PLUGIN_NAME );

TVHistogram::TVHistogram( void )
{
  m_RectSel = CRect( 0 , 0 , 639 , 479 ) ;
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( transparent );
  m_pControl = new CDuplexConnector( this , text , text );
  m_iLastImageDepth = 8 ;
  m_iRangeBegin = 0 ;
  m_iRangeEnd = 255 ;
  m_iCutLevelx10_perc = 500 ; //50%
  m_bViewCut = 1 ;
  Resume();
}

void TVHistogram::ShutDown()
{
  //TODO: Add all destruction code here
  CFilterGadget::ShutDown();
  delete m_pControl;
  m_pControl = NULL;
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

void TVHistogram::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  FXAutolock al( m_TransactionLock );
  CTextFrame* TextFrame = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( TextFrame )
  {
    FXPropertyKit pk = TextFrame->GetString();
    CRect res ;
    if ( GetRect( pk , _T( "ROI" ) , res ) || GetRect( pk , _T( "Rect" ) , res ) )
      m_RectSel = res ;
  }
  pParamFrame->RELEASE( pParamFrame );
}

DWORD  GetHeight2( pTVFrame frame )
{
  return frame->lpBMIH->biHeight;
}

CDataFrame* TVHistogram::DoProcessing( const CDataFrame* pDataFrame )
{
  FXAutolock al( m_TransactionLock );
  CFigureFrame* retVal = NULL;
  const CVideoFrame* vFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( vFrame , pDataFrame );
  int iLeftCutEdge = -1 , iRightCutEdge = -1 ;
  double dAverage = 0. , dStd = 0. ;
  if ( !m_RectSel.IsRectEmpty() )
  {
    CRect m_ImageRect( 0 , 0 , GetWidth( vFrame ) , GetHeight( vFrame ) ) ;
    m_ImageRect &= m_RectSel ;
    if ( !m_ImageRect.IsRectEmpty() )
    {
      retVal = CFigureFrame::Create();
      dAverage = GetHistogram( vFrame , m_RectSel , *retVal , &dStd );
      if (dAverage == 0.)
      {
        retVal->Release();
        return NULL;
      }
      int iMin = INT_MAX;
      int iMax = -iMin ;
      int iMaxIndex = -1 ;
      for ( int i = 0 ; i < retVal->GetCount() ; i++ )
      {
        int iVal = ( int )( *retVal )[ i ].y ;
        if ( iVal > iMax )
        {
          iMax = iVal ;
          iMaxIndex = i ;
        }
        if ( iVal < iMin )
          iMin = iVal ;
      }
      if ( m_iCutLevelx10_perc )
      {
        iLeftCutEdge = iMaxIndex ;
        iRightCutEdge = iMaxIndex ;
        int iAccum = ( int ) (( *retVal )[ iMaxIndex ].y) ;
        int iArea = m_RectSel.Width() * m_RectSel.Height() ;
        int iThres = iArea * m_iCutLevelx10_perc / 1000 ; // 100 percent and x10
        while ( iThres > iAccum )
        {
          if (iLeftCutEdge > 0)
            iAccum += ( int ) ( ( *retVal )[ --iLeftCutEdge ].y ) ;
          if ( iRightCutEdge < HISTOGRAM_LENGTH - 1 )
            iAccum += ( int ) ( ( *retVal )[ ++iRightCutEdge ].y ) ;
        }
      }
      if ( iMax != 0 )
      {
        double dYScale = 256. / iMax ;
//         for ( int i = 0 ; i < retVal->GetCount() ; i++ )
//         {
//           int iVal = ( int )( *retVal )[ i ].y ;
//           iVal = 256 - ( int )( iVal * dStep ) ;
//           ( *retVal )[ i ].y = ( double )iVal ;
//         }
        double dIndexTo = 0. ;
        int iIndexTo = -1 , iOldIndexTo ;
        int iRangeBegin = m_iRangeBegin ;
        int iRangeEnd = m_iRangeEnd ;
        if ( vFrame->lpBMIH->biCompression == BI_Y16 )
        {
          iRangeBegin /= 256 ;
          iRangeEnd /= 256 ;
        }
        
        double dXStep = 256. / ( iRangeEnd - iRangeBegin /*+ 1.*/ ) ;
        double dValues[ 256 ] ;
        for ( int i = iRangeBegin ; i <= iRangeEnd ; i++ )
        {
          iOldIndexTo = iIndexTo ;
          iIndexTo = ROUND( dIndexTo ) ;
          if ( iIndexTo > 255 )
            iIndexTo = 255 ;
          
          CDPoint& PtFrom = (CDPoint&)retVal->GetAt( i ) ;
          DOUBLE dY = BASE_Y - ( PtFrom.y * dYScale ) ; 
          //CDPoint * pPtTo= &( ( CDPoint& )retVal->GetAt( iOldIndexTo + 1) ) ;
         
          for ( int j = iOldIndexTo+1 ; j <= iIndexTo ; j++  )
            dValues[j] = dY ; 
          dIndexTo += dXStep ;
        }
        for ( int i = 0 ; i < 256 ; i++ )
          (*retVal)[ i ].y = dValues[ i ] ;
        
      }
      retVal->CopyAttributes( pDataFrame );
      retVal->Attributes()->WriteString( "color" , "0x0000ff" );
    }
  }
  if ( retVal )
  {
    if ( !pDataFrame->IsContainer() )
    {
      CContainerFrame* cVal = CContainerFrame::Create();
      cVal->CopyAttributes( pDataFrame );
      cVal->AddFrame( pDataFrame );
      cVal->AddFrame( retVal );
      cmplx LeftMarkerPt( BASE_X , BASE_Y + 3. ) ;
      cmplx RightMarkerPt( BASE_X + HISTOGRAM_LENGTH - 20. , BASE_Y + 3. ) ;
      FXString LeftMarkerAsDigits , RightMarkerAsDigits ;
      LeftMarkerAsDigits.Format( _T( "%d" ) , m_iRangeBegin ) ;
      RightMarkerAsDigits.Format( _T( "%d" ) , m_iRangeEnd ) ;
      CTextFrame * pLeftMarker = CreateTextFrame(
        LeftMarkerPt , ( LPCTSTR )LeftMarkerAsDigits , _T( "0x0000ff" ) ) ;
      CTextFrame * pRightMarker = CreateTextFrame(
        RightMarkerPt , ( LPCTSTR )RightMarkerAsDigits , _T( "0x0000ff" ) ) ;
      if ( pLeftMarker )
        cVal->AddFrame( pLeftMarker ) ;
      if ( pRightMarker )
        cVal->AddFrame( pRightMarker ) ;
      CFigureFrame * pROI = CreateFigureFrame( 
        m_RectForView , 5 , GetHRTickCount() , _T( "0xff0000" ) ) ;
      if ( pROI )
        cVal->AddFrame( pROI ) ;
      if ( m_bViewCut && (iLeftCutEdge >= 0) && (iRightCutEdge <= 255) )
      {
        cmplx LeftCut( BASE_X + iLeftCutEdge , BASE_Y + 8. ) ;
        cmplx RightCut( BASE_X + iRightCutEdge , BASE_Y + 8. ) ;
        CFigureFrame * pCutEdge = CreateLineFrame( LeftCut , RightCut ,
          RGB( 0 , 255 , 255 ) ) ;
        pCutEdge->Attributes()->WriteInt( "thickness" , 3 ) ;
        cVal->AddFrame( pCutEdge ) ;
        LeftCut._Val[ _IM ] += 35. ;
        int iWidth = iRightCutEdge - iLeftCutEdge + 1;
        if (iWidth > 0)
        {
          cVal->AddFrame(CreateTextFrame(LeftCut, "0xff00c0", 14, "HistoWidth",
            pDataFrame->GetId(), "Width=%d Std=%.2f\nAver=%.2f\nAver/Width=%.3f",
            iWidth, dStd ,dAverage, dAverage / iWidth));
        }
      }
      return cVal;
    }
    else
      return retVal;
  }
  else
  {
    ( ( CDataFrame* )pDataFrame )->AddRef();
    return ( CDataFrame* )pDataFrame;
  }
}

bool TVHistogram::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  CRect TmpRect ;
  if ( GetRect( pk , _T( "ROI" ) , TmpRect ) )
  {
    m_RectSel = TmpRect ;
    m_RectSel.right += m_RectSel.left ;
    m_RectSel.bottom += m_RectSel.top ;
    m_RectForView[ 0 ] = m_RectForView[ 4 ] = cmplx( m_RectSel.left , m_RectSel.top ) ;
    m_RectForView[ 1 ] = cmplx( m_RectSel.right , m_RectSel.top ) ;
    m_RectForView[ 2 ] = cmplx( m_RectSel.right , m_RectSel.bottom ) ;
    m_RectForView[ 3 ] = cmplx( m_RectSel.left , m_RectSel.bottom ) ;
  }
  pk.GetInt( _T( "RangeBegin" ) , m_iRangeBegin ) ;
  pk.GetInt( _T( "RangeEnd" ) , m_iRangeEnd ) ;
  pk.GetInt( _T( "CutLevel_x10_perc" ) , m_iCutLevelx10_perc ) ;
  pk.GetInt( _T( "ViewCut" ) , m_bViewCut ) ;

  if ( m_iRangeEnd <= m_iRangeBegin )
  {
    if ( m_iRangeBegin < 255 )
      m_iRangeEnd = m_iRangeBegin + 1 ;
    else
      m_iRangeBegin = m_iRangeEnd - 1 ;
  }
  
  return true;
}

bool TVHistogram::PrintProperties( FXString& text )
{
  FXPropertyKit pk ;
  CRect ForWrite( m_RectSel ) ;
  ForWrite.right -= ForWrite.left ;
  ForWrite.bottom -= ForWrite.top ;
  WriteRect( pk , _T( "ROI" ) , ForWrite ) ;
  pk.WriteInt( _T( "RangeBegin" ) , m_iRangeBegin ) ;
  pk.WriteInt( _T( "RangeEnd" ) , m_iRangeEnd ) ;
  pk.WriteInt( _T( "CutLevel_x10_perc" ) , m_iCutLevelx10_perc ) ;
  pk.WriteInt( _T( "ViewCut" ) , m_bViewCut ) ;
  text += pk ;
  return true;
}
bool TVHistogram::ScanSettings( FXString& text )
{
//   text.Format( "template(EditBox(ROI),"
//     "Spin(RangeBegin,0,%d),Spin(RangeEnd,0,%d)))" ,
//     ( m_iLastImageDepth <= 8 ) ? 255 : 655355 ,
//     ( m_iLastImageDepth <= 8 ) ? 255 : 655355 ) ;
  text = _T( "template(EditBox(ROI),"
    "Spin(RangeBegin,0,255),"
    "Spin(RangeEnd,0,65535),"
    "Spin(CutLevel_x10_perc,0,10000),"
    "Spin(ViewCut,0,1))" ) ;
    return true;
}


int TVHistogram::GetDuplexCount()
{
  return 1;
}

CDuplexConnector* TVHistogram::GetDuplexConnector( int n )
{
  return ( ( !n ) ? m_pControl : NULL );
}

