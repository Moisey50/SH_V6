#include "stdafx.h"
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <array>
#include <fxfc/fxfc.h>
#include <imageproc\dibutils.h>
#include <imageproc\seekspots.h>
#include <classes\drect.h>
#include "math\intf_sup.h"
#include "math\PlaneGeometry.h"
#include "TVObjects.h"
#include "VideoObjects.h"
#include <Gadgets\gadbase.h>
#include <imageproc\cut.h>
#include <imageproc\rotate.h>
#include <imageproc\fstfilter.h>
#include <imageproc\ocr\Crcgn.h>
#include <imageproc\enchance.h>
#include <imageproc\simpleip.h>
#include <imageproc\resample.h>
#include <imageproc\LineDetector\LineDetector.h>
#include <helpers\FramesHelper.h>
#include <helpers\propertykitex.h>
#include "ViewModeOptionData.h"

#define THIS_MODULENAME "TVObjects"

ObjNameToMask DefMeasurements[] =
{
  { MEASURE_POSITION , _T( "meas_pos" ) } ,
{ MEASURE_AREA , _T( "meas_area" ) } ,
{ MEASURE_THICKNESS , _T( "meas_thick" ) } ,
{ MEASURE_PERIMETER , _T( "meas_perim" ) } ,
{ MEASURE_ANGLE_CW , _T( "meas_anglew" ) } ,
{ MEASURE_ANGLE , _T( "meas_angle" ) } ,
{ MEASURE_SUBPIXEL , _T( "meas_subpix" ) } ,
{ MEASURE_RATIO , _T( "meas_ratio" ) } ,
{ MEASURE_ARC , _T( "meas_arc" ) } ,
{ MEASURE_TEXT , _T( "meas_text" ) } ,
{ MEASURE_TXT_FAST , _T( "meas_tfast" ) } ,
{ MEASURE_XPROFILE , _T( "meas_xprof" ) } ,
{ MEASURE_DIFFRACT , _T( "meas_diffr" ) } ,
{ MEASURE_YPROFILE , _T( "meas_yprof" ) } ,
{ MEASURE_CONTUR , _T( "meas_contur" ) } ,
{ MEASURE_SUB_SIDE_BACKS , _T( "meas_sub_sides" ) } ,
{ MEASURE_DIAMETERS , _T( "meas_diam" ) } ,
{ MEASURE_IMG_MOMENTS_W , _T( "meas_weight" ) } ,
{ MEASURE_RUNS , _T( "meas_runs" ) } ,
{ MEASURE_EDGE_AS_CONTUR , _T( "edge_as_contur" ) } ,
{ 0 , NULL }
} ;

ObjNameToMask DefViews[] =
{
  { OBJ_VIEW_ROI , "view_roi" } ,
{ OBJ_VIEW_POS , "view_pos" } ,
{ OBJ_VIEW_COORD , "view_coords" } ,
{ OBJ_VIEW_ANGLE , "view_angle" } ,
{ OBJ_VIEW_DIA , "view_dia" } ,
{ OBJ_VIEW_CONT , "view_contur" } ,
{ OBJ_VIEW_DET , "view_details" } ,
{ OBJ_VIEW_PROFX , "view_profx" } ,
{ OBJ_VIEW_PROFY , "view_profy" } ,
{ OBJ_VIEW_MRECTS , "view_mgraph" } ,
{ OBJ_VIEW_SCALED , "view_scaled" } ,
{ OBJ_VIEW_TEXT , "view_text" } ,
{ OBJ_VIEW_CSV , "out_csv" } ,
{ OBJ_OUT_MOMENTS , "out_moments" } ,
{ 0 , NULL }
} ;

ComboItem ObjTypeCombo[] =
{
  { SPOT , "SPOT" } ,
{ LINE_SEGMENT , "LINE" } ,
{ EDGE , "EDGE" } ,
{ OCR , "TEXT" } ,
{ 0 , NULL }
} ;

ComboItem ObjPlacementCombo[] =
{
  { PLACE_ABS , "ABS" } ,
{ PLACE_REL , "REL" } ,
{ PLACE_REL_XY , "REL_XY" } ,
{ PLACE_COPY_REL , "COPY_REL" } ,
{ 0 , NULL }
} ;

DWORD AllMeasMaskBIts = MEASURE_POSITION | MEASURE_AREA | MEASURE_THICKNESS
| MEASURE_PERIMETER | MEASURE_ANGLE_CW | MEASURE_ANGLE | MEASURE_RATIO
| MEASURE_ARC | MEASURE_TEXT | MEASURE_XPROFILE | MEASURE_DIFFRACT
| MEASURE_YPROFILE | MEASURE_CONTUR | MEASURE_SUB_SIDE_BACKS | MEASURE_DIAMETERS
| MEASURE_IMG_MOMENTS_W | MEASURE_RUNS | MEASURE_SUBPIXEL ;

DWORD MaskForSpotMeasurement = MEASURE_POSITION | MEASURE_AREA
| MEASURE_PERIMETER | MEASURE_ANGLE_CW | MEASURE_ANGLE | MEASURE_RATIO
| MEASURE_XPROFILE | MEASURE_DIFFRACT
| MEASURE_YPROFILE | MEASURE_CONTUR | MEASURE_SUB_SIDE_BACKS | MEASURE_DIAMETERS
| MEASURE_IMG_MOMENTS_W | MEASURE_RUNS | MEASURE_SUBPIXEL ;

DWORD MaskBitsForLineMeasurements = MEASURE_POSITION | MEASURE_THICKNESS
| MEASURE_ANGLE | MEASURE_XPROFILE | MEASURE_YPROFILE | MEASURE_SUB_SIDE_BACKS ;

DWORD MaskBitsForEdgeMeasurements = MEASURE_POSITION | MEASURE_ANGLE
| MEASURE_XPROFILE | MEASURE_YPROFILE
| MEASURE_SUB_SIDE_BACKS | MEASURE_EDGE_AS_CONTUR ;

DWORD MaskBitsForOCRMeas = MEASURE_TEXT | MEASURE_TXT_FAST ;

DWORD AllViewMaskBits = OBJ_VIEW_ROI | OBJ_VIEW_POS | OBJ_VIEW_COORD
| OBJ_VIEW_ANGLE | OBJ_VIEW_DIA | OBJ_VIEW_CONT | OBJ_VIEW_DET
| OBJ_VIEW_PROFX | OBJ_VIEW_PROFY | OBJ_VIEW_MRECTS | OBJ_VIEW_SCALED | OBJ_VIEW_CSV ;

DWORD MaskForSpotView = OBJ_VIEW_ROI | OBJ_VIEW_POS | OBJ_VIEW_COORD
| OBJ_VIEW_ANGLE | OBJ_VIEW_DIA | OBJ_VIEW_CONT | OBJ_VIEW_DET
| OBJ_VIEW_PROFX | OBJ_VIEW_PROFY | OBJ_VIEW_MRECTS | OBJ_VIEW_SCALED | OBJ_VIEW_CSV
| OBJ_OUT_MOMENTS ;

DWORD MaskBitsForLineView = OBJ_VIEW_ROI | OBJ_VIEW_POS | OBJ_VIEW_COORD
| OBJ_VIEW_ANGLE | OBJ_VIEW_PROFX | OBJ_VIEW_PROFY | OBJ_VIEW_SCALED | OBJ_VIEW_CSV ;

DWORD MaskBitsForEdgeView = OBJ_VIEW_ROI | OBJ_VIEW_POS | OBJ_VIEW_COORD
| OBJ_VIEW_ANGLE | OBJ_VIEW_PROFX | OBJ_VIEW_PROFY | OBJ_VIEW_SCALED
| OBJ_VIEW_CONT | OBJ_VIEW_DET | OBJ_VIEW_CSV ;

DWORD MaskBitsForOCRView = OBJ_VIEW_ROI | OBJ_VIEW_TEXT ;

bool GetMasksForObjType( VOBJ_TYPE Type , DWORD& MeasMask , DWORD& ViewMask )
{
  switch ( Type )
  {
  case SPOT:
    MeasMask = MaskForSpotMeasurement ;
    ViewMask = MaskForSpotView ;
    return true ;
  case LINE_SEGMENT:
    MeasMask = MaskBitsForLineMeasurements ;
    ViewMask = MaskBitsForLineView ;
    return true ;
  case EDGE:
    MeasMask = MaskBitsForEdgeMeasurements ;
    ViewMask = MaskBitsForEdgeView ;
    return true ;
  case OCR:
    MeasMask = MaskBitsForOCRMeas ;
    ViewMask = MaskBitsForOCRView ;
    return true ;
  default: break ;
  }
  return false ;
}


FXString GetStatesAsString(
  DWORD State , ObjNameToMask * pArray , DWORD dwBitMask = 0xffffffff )
{
  FXString Res ;
  while ( pArray->bitmask )
  {
    if ( dwBitMask & pArray->bitmask )
    {
      Res += ( FXString( pArray->name ) + '=' ) +
        ( ( State & pArray->bitmask ) ? "1;" : "0;" ) ;
    }
    pArray++ ;
  }
  return Res ;
}

DWORD GetMeasMaskForName(
  LPCTSTR pName , ObjNameToMask * pArray , DWORD dwBitMask )
{
  while ( pArray->bitmask != 0 )
  {
    if ( _tcscmp( pName , pArray->name ) == 0 )
    {
      if ( dwBitMask & pArray->bitmask )
        return pArray->bitmask ;
      else
        return 0 ;
    }
    pArray++ ;
  }
  return 0 ;
}

LPCTSTR GetMeasNameForMask(
  DWORD Mask , ObjNameToMask * pArray , DWORD dwBitMask )
{
  while ( pArray->bitmask != 0 )
  {
    if ( pArray->bitmask == Mask )
    {
      if ( pArray->bitmask & dwBitMask )
        return pArray->name ;
      return NULL ;
    }
    pArray++ ;
  }
  return NULL ;
}

bool GetMaskFromText(
  ObjNameToMask * pArray , FXPropKit2& text , DWORD& dwResult , DWORD dwBitMask = 0xffffffff )
{
  bool bResult = false ;
  while ( pArray->bitmask != 0 )
  {
    int iVal ;
    if ( text.GetInt( pArray->name , iVal )
      && ( pArray->bitmask & dwBitMask ) )
    {
      if ( iVal )
        dwResult |= pArray->bitmask ;
      else
        dwResult &= ~( pArray->bitmask ) ;
      bResult = true ;
    }
    pArray++ ;
  }
  return bResult ;
}


COCR::COCR()
{
  if ( !OCR_Init( GetModuleHandle( "TVObjects.dll" ) ,
    MAKEINTRESOURCE( IDR_OCRLIB ) , "OCRLIB" ) )
  {
    SENDERR_0( "OCR lib didn't loaded" );
  }
}

COCR::~COCR()
{
  OCR_Free();
}

#ifdef _DEBUG
__forceinline bool saveDIB( const char* fname , const pTVFrame frame )
{
  BITMAPFILEHEADER bmfH = { BMAP_HEADER_MARKER , 0 , 0 , 0 , 0 };

  if ( ( !frame ) || ( !frame->lpBMIH ) )
    return false;
  LPBYTE src = GetData( frame );
  int SizeImage = 0;
  if ( frame->lpBMIH->biSizeImage )
    SizeImage = frame->lpBMIH->biSizeImage;
  else
  {
    switch ( frame->lpBMIH->biBitCount )
    {
    case 16:
      SizeImage = frame->lpBMIH->biHeight*frame->lpBMIH->biWidth * 2;
      break;
    case 24:
      SizeImage = frame->lpBMIH->biHeight*frame->lpBMIH->biWidth * 3;
      break;
    case 32:
      SizeImage = frame->lpBMIH->biHeight*frame->lpBMIH->biWidth * 4;
      break;
    }
  }

  int fout = _open( fname , _O_CREAT | _O_WRONLY | _O_BINARY , _S_IREAD | _S_IWRITE );
  if ( fout == -1 )
    return( FALSE );
  bmfH.bfSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + SizeImage;
  bmfH.bfOffBits = ( DWORD )sizeof( BITMAPFILEHEADER ) + frame->lpBMIH->biSize;
  _write( fout , &bmfH , sizeof( BITMAPFILEHEADER ) );
  _write( fout , frame->lpBMIH , frame->lpBMIH->biSize );
  _write( fout , src , SizeImage );
  _close( fout );
  return true;
}
#endif

bool ScanNearEdge( CColorSpot& Spot , const pTVFrame pFr ,
  int iThres , CRect& rcROI , CPoint &p , CSize& dirMove , CSize& dirToBord ,
  double& dArea , cmplx& PrevPt , cmplx& FirstPt )
{
  CmplxArray& Contur = Spot.m_Contur ;
  int Br = GetPixel( pFr , p ) ;
  bool bInvert = ( Br >= iThres ) ; // true, if contour is black
  bool bFirstPassed = false ;
  CPoint Next = NextPoint( p , dirMove ) ;
  cmplx NewPt ;
  for ( int iNRightTurns = 0 ; iNRightTurns < 4 ; iNRightTurns++ )
  {
    while ( rcROI.PtInRect( Next ) )
    {
      NewPt = cmplx( ( double ) p.x + ( dirToBord.cx * 0.5 ) , ( double ) p.y + ( dirToBord.cy * 0.5 ) ) ;
      ASSERT( !Contur.Size() || ( NewPt != Contur.GetLast() ) ) ;
      Contur.Add( NewPt ) ;
      CorrectRect( NewPt , Spot.m_EdgesAsDbl ) ;
      Spot.m_dPerimeter += abs( NewPt - PrevPt ) ;
      if ( FirstPt == NewPt )
        return false ; // we did return to first point after going over edge(s) 
      dArea += 0.5 *
        ( NewPt.imag() - PrevPt.imag() ) * ( NewPt.real() + PrevPt.real() ) ;

      if ( Other( pFr , Next , iThres , bInvert ) )
      {
        TurnRight( dirMove , dirToBord ) ;
        BOOL bMeasured = BorderByTwo( pFr , p , NewPt , iThres , dirToBord ) ;
        if ( bMeasured )
        {
          ASSERT( NewPt != Contur.GetLast() ) ;
          Contur.Add( NewPt ) ;
          CorrectRect( NewPt , Spot.m_EdgesAsDbl ) ;
          Spot.m_dPerimeter += abs( NewPt - PrevPt ) ;
          if ( FirstPt == NewPt )
            return false ; // we did return to first point after going over all edges 
          PrevPt = NewPt ;
        }
        else
          ASSERT( 0 ) ;
        return true ;
      }
      p = Next ;
      Next = NextPoint( p , dirMove ) ;
      PrevPt = NewPt ;
    } ;
    // take last before corner point
    NewPt = cmplx( ( double ) p.x + ( dirToBord.cx * 0.5 ) , ( double ) p.y + ( dirToBord.cy * 0.5 ) ) ;
    ASSERT( NewPt != Contur.GetLast() ) ;
    Contur.Add( NewPt ) ;
    CorrectRect( NewPt , Spot.m_EdgesAsDbl ) ;
    Spot.m_dPerimeter += abs( NewPt - PrevPt ) ;
    if ( FirstPt == NewPt )
      return false ; // we did return to first point after going over all edges 
    dArea += 0.5 *
      ( NewPt.imag() - PrevPt.imag() ) * ( NewPt.real() + PrevPt.real() ) ;
    PrevPt = NewPt ;
    TurnRight( dirMove , dirToBord ) ;
    Next = NextPoint( p , dirMove ) ;
  }
  //ASSERT( 0 ) ; // we should not arrive there
  return false ; // we did return to first point after going over all edges

}

// bool ScanNearEdge( CmplxArray& Contur , const pTVFrame pFr ,
//   int iThres , CRect& rcROI , CPoint &p , CSize& dirMove , CSize& dirToBord ,
//   cmplx& PrevPt , cmplx& FirstPt , double dLength )
// {
//   int Br = GetPixel( pFr , p ) ;
//   bool bInvert = (Br >= iThres) ; // true, if contour is black
//   bool bFirstPassed = false ;
//   CPoint Next = NextPoint( p , dirMove ) ;
//   cmplx NewPt ;
//   while ( rcROI.PtInRect( Next ) )
//   {
//     NewPt = cmplx( (double) p.x + (dirToBord.cx * 0.5) , (double) p.y + (dirToBord.cy * 0.5) ) ;
//     Contur.Add( NewPt ) ;
//     dLength += abs( NewPt - PrevPt ) ;
//     if ( FirstPt == NewPt )
//       return false ; // we did return to first point after going over edge(s) 
//       (NewPt.imag() - PrevPt.imag()) * (NewPt.real() + PrevPt.real()) ;
// 
//     if ( Other( pFr , Next , iThres , bInvert ) )
//     {
//       TurnRight( dirMove , dirToBord ) ;
//       BOOL bMeasured = BorderByTwo( pFr , p , NewPt , iThres , dirToBord ) ;
//       if ( bMeasured )
//       {
//         Contur.Add( NewPt ) ;
//         dLength += abs( NewPt - PrevPt ) ;
//         if ( FirstPt == NewPt )
//           return false ; // we did return to first point after going over all edges 
//         PrevPt = NewPt ;
//       }
//       else
//         ASSERT( 0 ) ;
//       return true ;
//     }
//     p = Next ;
//     Next = NextPoint( p , dirMove ) ;
//     PrevPt = NewPt ;
//   } ;
//   ASSERT( 0 ) ; // we should not arrive there
//   return false ; // we did return to first point after going over all edges
// }
// 

double _FindSubPixelEdge(
  CColorSpot& Spot , const pTVFrame pFr , int iThres , CRect& OutRect ,
  CPoint FirstPoint )
{
  double dStart = GetHRTickCount() ;
  CRect rcROI = OutRect ;
  CRect FOV( 0 , 0 , GetWidth( pFr ) , GetHeight( pFr ) ) ;
  CRect Intersection ;
  Intersection.IntersectRect( rcROI , FOV ) ;
  rcROI = Intersection ;

  int iMaxConturLength = 10 * ( rcROI.Width() + rcROI.Height() ) ;
  if ( !rcROI.PtInRect( FirstPoint ) )
    return 0.0 ;
  CmplxArray& Contur = Spot.m_Contur ;
  Contur.RemoveAll() ;
  Spot.m_dPerimeter = 0. ;
  //   CmplxArray SecondDirectionPts ;  
  CPoint XYIterator( 0 , 0 ) ;
  CPoint p( FirstPoint ) ;
  bool bWhite = GetPixel( pFr , p ) >= iThres ;
  CSize dirMove( 0 , -1 ) , dirToBord( -1 , 0 ) ; //  move dir is up and direction to border is left
  CPoint PtLeft( p + dirToBord ) , PtNextLeft ;
  while ( ( GetPixel( pFr , PtLeft ) >= iThres ) == bWhite )
  {
    if ( !rcROI.PtInRect( PtLeft ) )
      break ;
    p = PtLeft ;
    PtLeft += dirToBord ;
  }
  int Br = GetPixel( pFr , p ) ;
  bool bInvert = ( Br >= iThres ) ; // true, if first point is white

#ifdef _DEBUG
  int Bra[ 9 ] ;
  if ( p.x > 0 && p.y > 0 )
  {
    Bra[ 0 ] = GetPixel( pFr , p + CSize( -1 , -1 ) ) ;
    Bra[ 1 ] = GetPixel( pFr , p + CSize( 0 , -1 ) ) ;
    Bra[ 2 ] = GetPixel( pFr , p + CSize( 1 , -1 ) ) ;
    Bra[ 3 ] = GetPixel( pFr , p + CSize( -1 , 0 ) ) ;
    Bra[ 4 ] = GetPixel( pFr , p + CSize( 0 , 0 ) ) ;
    Bra[ 5 ] = GetPixel( pFr , p + CSize( 1 , 0 ) ) ;
    Bra[ 6 ] = GetPixel( pFr , p + CSize( -1 , 1 ) ) ;
    Bra[ 7 ] = GetPixel( pFr , p + CSize( 0 , 1 ) ) ;
    Bra[ 8 ] = GetPixel( pFr , p + CSize( 1 , 1 ) ) ;
  }
#endif
  double dArea = 0. , dSumX = 0. , dSumY = 0. ;
  // the next is direction for next border searching
  CPoint Next ;
  cmplx PrevPt , NewPt ;
  cmplx FirstPt ;
  if ( !rcROI.PtInRect( PtLeft ) ) // we are on the left edge of ROI
  {
    FirstPt = cmplx( p.x - 0.5 , ( double ) p.y ) ;
    Contur.Add( FirstPt ) ;
    //     CorrectRect( FirstPt , Spot.m_EdgesAsDbl ) ;
    CPoint PtUp( p + dirMove ) ;
    TurnRight( dirMove , dirToBord ) ;
    if ( !rcROI.PtInRect( PtUp ) ) // we are on left top corner
    {
      PrevPt = cmplx( p.x - 0.5 , ( double ) p.y - 0.5 ) ;
      ASSERT( !Contur.Size() || ( NewPt != Contur.GetLast() ) ) ;
      Contur.Add( PrevPt ) ;
      //       CorrectRect( PrevPt , Spot.m_EdgesAsDbl ) ;
      //       Spot.m_dPerimeter += abs( PrevPt - FirstPt ) ;
      if ( !ScanNearEdge( Spot , pFr , iThres , rcROI , p ,
        dirMove , dirToBord , dArea , PrevPt , FirstPt ) )
      {
        return dArea ;  // full ROI is one contour
      }
      else
        Next = p + dirMove ;
    }
    else
    {
      BOOL bMeasured = BorderByTwo( pFr , p , NewPt , iThres , CSize( 0 , -1 ) ) ;
      if ( !bMeasured )
      {
        return 0. ; //ASSERT( 0 ) ;
      }
      else
      {
        ASSERT( !Contur.Size() || ( NewPt != Contur.GetLast() ) ) ;
        Contur.Add( NewPt ) ;
        PrevPt = NewPt ;
        Next = p + dirMove ;
      }
    }
  }
  else if ( Other( pFr , PtLeft , iThres , bInvert ) ) // on left side is different spot
  {
    BOOL bMeasured = BorderByTwo( pFr , p , NewPt , iThres , dirToBord ) ;
    if ( !bMeasured )
    {
      ASSERT( 0 ) ;
    }
    else
    {
      FirstPt = NewPt ;
      ASSERT( !Contur.Size() || ( NewPt != Contur.GetLast() ) ) ;
      Contur.Add( NewPt ) ;
      PrevPt = NewPt ;
    }
    Next = p + dirMove ;
  }
  else
  {
    //     ASSERT(0) ; // there is no border on left side, it's not correct
    return 0.0 ;
  }
  int iNTurns = 0 ;
  int iNCycles = 0 ;
  double dMaxScanNearTime1 = 0. , dMaxScanNearTime2 = 0. ;
  do
  {
    iNCycles++ ;
    if ( !rcROI.PtInRect( Next ) ) // we are on the border
    {
      TurnRight( dirMove , dirToBord ) ;
      if ( !ScanNearEdge( Spot , pFr , iThres , rcROI , p ,
        dirMove , dirToBord , dArea , PrevPt , FirstPt ) )
      {
        return dArea ;  // full ROI is one contour
      }
      Next = p + dirMove ;
      continue ;
    }
    if ( TheSame( pFr , Next , iThres , bInvert ) ) // black Contur continues
    {
      PtNextLeft = Next + dirToBord ;
      if ( !rcROI.PtInRect( PtNextLeft ) )
      {
        if ( !ScanNearEdge( Spot , pFr , iThres , rcROI , p ,
          dirMove , dirToBord , dArea , PrevPt , FirstPt ) )
        {
          break ;  // full ROI is one contour
        }
        Next = p + dirMove ;
        continue ;
      }
      else
      {
        if ( TheSame( pFr , PtNextLeft , iThres , bInvert ) ) // black Contur goes to left
        {
#ifdef _DEBUG
          ASSERT( Other( pFr , p + dirToBord , iThres , bInvert ) ) ;
#endif
          p = PtNextLeft ;
          TurnLeft( dirMove , dirToBord ) ;
        }
        else
          p = Next ;
      }
    }
    else
    {
      TurnRight( dirMove , dirToBord ) ;
    }
    BOOL bMeasured = BorderByTwo( pFr , p , NewPt , iThres , dirToBord ) ;
    if ( !bMeasured )
      ASSERT( 0 ) ;
    if ( PrevPt != NewPt )
    {
      dArea += GetDeltaArea( NewPt , PrevPt ) ;
      Contur.Add( NewPt ) ;
      if ( ( NewPt == FirstPt )
        && ( Contur.Size() > 3 ) ) // protection against first pixel equal to threshold
        break ;                      // is one in first spot row (3 direction changes   
      if ( Contur.Size() >= iMaxConturLength )
        break ;
      dSumX += NewPt.real() ;
      dSumY += NewPt.imag() ;
      PrevPt = NewPt ;
    }
    Next = p + dirMove ;
  } while ( 1 ) ;
  //   TRACE( "\n INCycles = %d Npts=%d T=%.3f" , iNCycles , Contur.GetCount() ,
  //     GetHRTickCount() - dStart ) ;
  return dArea ;
}


// This function is like previous, but it result will have only 
// one segment inside ROI. It begin contouring from 
// inside frame and if coming to the frame edge, will add to contour
// points from edge without coming inside frame

double _FindSubPixelEdgeWithOneInternalSegment(
  CmplxArray& OutContur , const pTVFrame pFr , int iThres , CRect& OutRect ,
  CPoint FirstPoint , CPoint SecondPoint )
{
  double dStart = GetHRTickCount() ;
  CRect rcROI = OutRect ;
  CRect FOV( 0 , 0 , GetWidth( pFr ) , GetHeight( pFr ) ) ;
  CRect Intersection ;
  Intersection.IntersectRect( rcROI , FOV ) ;
  rcROI = Intersection ;
  // check for belonging to ROI
  if ( !rcROI.PtInRect( FirstPoint )
    || !rcROI.PtInRect( SecondPoint ) )
  {
    return 0.0 ;
  }
  // Checking for contrast
  BOOL bFirstIsWhite = WhitePixel( pFr , FirstPoint , iThres ) ; // should be "white", i.e. true
  BOOL bSecondIsWhite = WhitePixel( pFr , SecondPoint , iThres ) ; // should be "black", i.e. false
  if ( !( bFirstIsWhite ^ bSecondIsWhite ) )
    return 0.0 ;  // the same contrast, no edge between them

  OutContur.RemoveAll() ;
  CmplxArray Contur ;
  CmplxArray SecondPartOfContur ;
  double dLength = 0. ;
  //   CmplxArray SecondDirectionPts ;  
  CPoint XYIterator( 0 , 0 ) ;
  CPoint p( FirstPoint ) ;

  //  define move direction and to edge direction
  CSize dirMove , dirToBord ;

  CSize Diff = SecondPoint - p ;
  if ( Diff.cx && Diff.cy ) // diagonal point
  {
    CPoint SecondNegativePt ;
    if ( !GetNegativeNeightbour( pFr , SecondPoint , iThres , SecondNegativePt ) )
      return false ;
    CSize Diff2 = SecondPoint - SecondNegativePt ;
    ASSERT( ( Diff2.cx == 0 ) ^ ( Diff2.cy == 0 ) ) ;
    FirstPoint = p = SecondNegativePt ;
    Diff = Diff2 ;
  }

  switch ( Diff.cx )
  {
  case 0:
  {
    if ( Diff.cy == 1 )
    {
      dirMove = CSize( -1 , 0 ) ;
      dirToBord = CSize( 0 , 1 ) ;
    }
    else
    {
      dirMove = CSize( 1 , 0 ) ;
      dirToBord = CSize( 0 , -1 ) ;
    }
  }
  break ;
  case 1:
  {
    dirMove = CSize( 0 , 1 ) ;
    dirToBord = CSize( 1 , 0 ) ;
  }
  break ;
  case -1:
  {
    dirMove = CSize( 0 , -1 ) ;
    dirToBord = CSize( -1 , 0 ) ;
  }
  break ;
  default: ASSERT( 0 ) ; break ;
  }
  CSize InitialDirMove( dirMove ) ;
  CSize InitialDirToBord( dirToBord ) ;
  BOOL bWhite = WhitePixel( pFr , p , iThres ) ; // last check

#ifdef _DEBUG
  int Bra[ 9 ] ;
  if ( p.x > 0 && p.y > 0 )
  {
    Bra[ 0 ] = GetPixel( pFr , p + CSize( -1 , -1 ) ) ;
    Bra[ 1 ] = GetPixel( pFr , p + CSize( 0 , -1 ) ) ;
    Bra[ 2 ] = GetPixel( pFr , p + CSize( 1 , -1 ) ) ;
    Bra[ 3 ] = GetPixel( pFr , p + CSize( -1 , 0 ) ) ;
    Bra[ 4 ] = GetPixel( pFr , p + CSize( 0 , 0 ) ) ;
    Bra[ 5 ] = GetPixel( pFr , p + CSize( 1 , 0 ) ) ;
    Bra[ 6 ] = GetPixel( pFr , p + CSize( -1 , 1 ) ) ;
    Bra[ 7 ] = GetPixel( pFr , p + CSize( 0 , 1 ) ) ;
    Bra[ 8 ] = GetPixel( pFr , p + CSize( 1 , 1 ) ) ;
  }
#endif
  int iNTurns = 0 ;
  int iNCycles = 0 ;
  double dMaxScanNearTime1 = 0. , dMaxScanNearTime2 = 0. ;
  CPoint Next , PtNextLeft;
  cmplx PrevPt , NewPt ;
  cmplx FirstPt ;

  BOOL bMeasured = BorderByTwo( pFr , p , FirstPt , iThres , dirToBord ) ;
  if ( !bMeasured )
    ASSERT( 0 ) ;
  Contur.Add( FirstPt ) ;
  Next = p + dirMove ;
  PrevPt = NewPt ;
  do
  {
    iNCycles++ ;
    if ( !rcROI.PtInRect( Next ) ) // we are on the border
      break ;  // this branch is finished on edge
    if ( WhitePixel( pFr , Next , iThres ) == bWhite )  // black Contur continues
    {                         // i.e. pixel in DirToBrd direction is "black"
      PtNextLeft = Next + dirToBord ;
      if ( rcROI.PtInRect( PtNextLeft ) )
      {
        if ( WhitePixel( pFr , PtNextLeft , iThres ) == bWhite ) // black Contur goes to left
        {
          // #ifdef _DEBUG
          //           ASSERT( Other( pFr , p + dirToBord , iThres , bInvert ) ) ;
          // #endif
          p = PtNextLeft ;
          TurnLeft( dirMove , dirToBord ) ;
        }
        else
          p = Next ;
      }
      else
        break ;  // this branch is finished on edge
    }
    else
    {
      TurnRight( dirMove , dirToBord ) ;
    }
    BOOL bMeasured = BorderByTwo( pFr , p , NewPt , iThres , dirToBord ) ;
    if ( !bMeasured )
      ASSERT( 0 ) ;
    Contur.Add( NewPt ) ;
    double dPath = abs( NewPt - PrevPt ) ;
    dLength += dPath ;
    if ( ( NewPt == FirstPt )
      && ( Contur.GetCount() > 3 ) ) // protection against first pixel equal to threshold
    {                               // is one in first spot row (3 direction changes
      return dLength ;   // contur is closed, there is no second branch
    }
    PrevPt = NewPt ;
    Next = p + dirMove ;
  } while ( 1 ) ;
  // Now we did half of work, necessary to process second part of segment (initial point 
  // is approximately in the center
  // Now we have to process "negative" edge
  int iNPointsInFirstBranch = ( int ) Contur.Count() ;
  int iLast = iNPointsInFirstBranch - 1 ;

  bWhite ^= 1 ; // invert contrast
  p = SecondPoint ;
  dirMove = CSize( 0 , 0 ) - InitialDirMove ;
  dirToBord = CSize( 0 , 0 ) - InitialDirToBord ;
  NewPt = PrevPt = FirstPt ;
  Next = p + dirMove ;
  do
  {
    iNCycles++ ;
    if ( !rcROI.PtInRect( Next ) ) // we are on the border
      break ;  // this branch is finished on edge
    if ( WhitePixel( pFr , Next , iThres ) == bWhite )  // black Contur continues
    {                         // i.e. pixel in DirToBrd direction is "black"
      PtNextLeft = Next + dirToBord ;
      if ( rcROI.PtInRect( PtNextLeft ) )
      {
        if ( WhitePixel( pFr , PtNextLeft , iThres ) == bWhite ) // black Contur goes to left
        {
          // #ifdef _DEBUG
          //           ASSERT( Other( pFr , p + dirToBord , iThres , bInvert ) ) ;
          // #endif
          p = PtNextLeft ;
          TurnLeft( dirMove , dirToBord ) ;
        }
        else
          p = Next ;
      }
      else
        break ;  // this branch is finished on edge
    }
    else
    {
      TurnRight( dirMove , dirToBord ) ;
    }
    BOOL bMeasured = BorderByTwo( pFr , p , NewPt , iThres , dirToBord ) ;
    if ( !bMeasured )
      ASSERT( 0 ) ;
    SecondPartOfContur.Add( NewPt ) ;
    double dPath = abs( NewPt - PrevPt ) ;
    dLength += dPath ;
    if ( ( NewPt == FirstPt )
      && SecondPartOfContur.GetCount() > 3 )  // protection against exit near first pixel
    {                               // we will pass first pixel after returning to mid edge 
      //ASSERT( 0 ) ; // impossible !!!
      return dLength ;   // contur is closed
    }
    PrevPt = NewPt ;
    Next = p + dirMove ;
  } while ( 1 ) ;
  OutContur.SetSize( Contur.Count() + SecondPartOfContur.Count() ) ;
  cmplx* pOutData = OutContur.GetData() ;
  cmplx* pFirstPartData = Contur.GetData() ;
  cmplx* pSecondPartData = SecondPartOfContur.GetData() ;
  cmplx* pSecond = pSecondPartData + SecondPartOfContur.Count() - 1 ;
  while ( pSecond >= pSecondPartData )
    *( pOutData++ ) = *( pSecond-- );
  memcpy( pOutData , pFirstPartData , Contur.Count() * sizeof( cmplx ) ) ;
  return dLength ;
}


// This function is like previous, but it result will have only 
// one segment inside ROI. It begin contouring from 
// inside frame and if coming to the frame edge, will add to contour
// points from edge without coming inside frame

double _FindOneInternalSegment(
  CmplxArray& OutContur , const pTVFrame pFr , int iThres , CRect& OutRect ,
  CPoint FirstPoint , CPoint SecondPoint , cmplx& cApproxDirection )
{
  double dStart = GetHRTickCount() ;
  CRect rcROI = OutRect ;
  CRect FOV( 0 , 0 , GetWidth( pFr ) , GetHeight( pFr ) ) ;
  CRect Intersection ;
  Intersection.IntersectRect( rcROI , FOV ) ;
  rcROI = Intersection ;
  // check for belonging to ROI
  if ( !rcROI.PtInRect( FirstPoint )
    || !rcROI.PtInRect( SecondPoint ) )
  {
    return 0.0 ;
  }
  // Checking for contrast
  BOOL bFirstIsWhite = WhitePixel( pFr , FirstPoint , iThres ) ; // should be "white", i.e. true
  BOOL bSecondIsWhite = WhitePixel( pFr , SecondPoint , iThres ) ; // should be "black", i.e. false
  if ( !( bFirstIsWhite ^ bSecondIsWhite ) )
    return 0.0 ;  // the same contrast, no edge between them

  OutContur.RemoveAll() ;
  CmplxArray Contur ;
  CmplxArray SecondPartOfContur ;
  double dLength = 0. ;

  //   CmplxArray SecondDirectionPts ;  
  CPoint XYIterator( 0 , 0 ) ;
  CPoint p( FirstPoint ) ;

  //  define move direction and to edge direction
  CSize dirMove , dirToBord ;

  CSize Diff = SecondPoint - p ;
  if ( Diff.cx && Diff.cy ) // diagonal point
  {
    CPoint SecondNegativePt ;
    if ( !GetNegativeNeightbour( pFr , SecondPoint , iThres , SecondNegativePt ) )
      return false ;
    CSize Diff2 = SecondPoint - SecondNegativePt ;
    ASSERT( ( Diff2.cx == 0 ) ^ ( Diff2.cy == 0 ) ) ;
    FirstPoint = p = SecondNegativePt ;
    Diff = Diff2 ;
  }

  switch ( Diff.cx )
  {
  case 0:
  {
    if ( Diff.cy == 1 )
    {
      dirMove = CSize( -1 , 0 ) ;
      dirToBord = CSize( 0 , 1 ) ;
    }
    else
    {
      dirMove = CSize( 1 , 0 ) ;
      dirToBord = CSize( 0 , -1 ) ;
    }
  }
  break ;
  case 1:
  {
    dirMove = CSize( 0 , 1 ) ;
    dirToBord = CSize( 1 , 0 ) ;
  }
  break ;
  case -1:
  {
    dirMove = CSize( 0 , -1 ) ;
    dirToBord = CSize( -1 , 0 ) ;
  }
  break ;
  default: ASSERT( 0 ) ; break ;
  }
  CSize InitialDirMove( dirMove ) ;
  CSize InitialDirToBord( dirToBord ) ;
  BOOL bWhite = WhitePixel( pFr , p , iThres ) ; // last check

#ifdef _DEBUG
  int Bra[ 9 ] ;
  if ( p.x > 0 && p.y > 0 )
  {
    Bra[ 0 ] = GetPixel( pFr , p + CSize( -1 , -1 ) ) ;
    Bra[ 1 ] = GetPixel( pFr , p + CSize( 0 , -1 ) ) ;
    Bra[ 2 ] = GetPixel( pFr , p + CSize( 1 , -1 ) ) ;
    Bra[ 3 ] = GetPixel( pFr , p + CSize( -1 , 0 ) ) ;
    Bra[ 4 ] = GetPixel( pFr , p + CSize( 0 , 0 ) ) ;
    Bra[ 5 ] = GetPixel( pFr , p + CSize( 1 , 0 ) ) ;
    Bra[ 6 ] = GetPixel( pFr , p + CSize( -1 , 1 ) ) ;
    Bra[ 7 ] = GetPixel( pFr , p + CSize( 0 , 1 ) ) ;
    Bra[ 8 ] = GetPixel( pFr , p + CSize( 1 , 1 ) ) ;
  }
#endif
  int iNTurns = 0 ;
  int iNCycles = 0 ;
  double dMaxScanNearTime1 = 0. , dMaxScanNearTime2 = 0. ;
  CPoint Next , PtNextLeft;
  cmplx PrevPt , NewPt ;
  cmplx FirstPt ;
  cmplx cDirIterator = cApproxDirection ;

  BOOL bMeasured = BorderByTwo( pFr , p , FirstPt , iThres , dirToBord ) ;
  if ( !bMeasured )
    ASSERT( 0 ) ;
  Contur.Add( FirstPt ) ;
  Next = p + dirMove ;
  PrevPt = NewPt ;
  do
  {
    iNCycles++ ;
    if ( !rcROI.PtInRect( Next ) ) // we are on the border
      break ;  // this branch is finished on edge
    if ( WhitePixel( pFr , Next , iThres ) == bWhite )  // black Contur continues
    {                         // i.e. pixel in DirToBrd direction is "black"
      PtNextLeft = Next + dirToBord ;
      if ( rcROI.PtInRect( PtNextLeft ) )
      {
        if ( WhitePixel( pFr , PtNextLeft , iThres ) == bWhite ) // black Contur goes to left
        {
          // #ifdef _DEBUG
          //           ASSERT( Other( pFr , p + dirToBord , iThres , bInvert ) ) ;
          // #endif
          p = PtNextLeft ;
          TurnLeft( dirMove , dirToBord ) ;
        }
        else
          p = Next ;
      }
      else
        break ;  // this branch is finished on edge
    }
    else
    {
      TurnRight( dirMove , dirToBord ) ;
    }
    BOOL bMeasured = BorderByTwo( pFr , p , NewPt , iThres , dirToBord ) ;
    if ( !bMeasured )
      ASSERT( 0 ) ;
    Contur.Add( NewPt ) ;
    double dPath = abs( NewPt - PrevPt ) ;
    dLength += dPath ;
    if ( ( NewPt == FirstPt )
      && ( Contur.GetCount() > 3 ) ) // protection against first pixel equal to threshold
    {                               // is one in first spot row (3 direction changes
      return dLength ;   // contur is closed, there is no second branch
    }
    PrevPt = NewPt ;
    Next = p + dirMove ;
  } while ( 1 ) ;
  // Now we did half of work, necessary to process second part of segment (initial point 
  // is approximately in the center
  // Now we have to process "negative" edge
  int iNPointsInFirstBranch = ( int ) Contur.Count() ;
  int iLast = iNPointsInFirstBranch - 1 ;

  bWhite ^= 1 ; // invert contrast
  p = SecondPoint ;
  dirMove = CSize( 0 , 0 ) - InitialDirMove ;
  dirToBord = CSize( 0 , 0 ) - InitialDirToBord ;
  NewPt = PrevPt = FirstPt ;
  Next = p + dirMove ;
  do
  {
    iNCycles++ ;
    if ( !rcROI.PtInRect( Next ) ) // we are on the border
      break ;  // this branch is finished on edge
    if ( WhitePixel( pFr , Next , iThres ) == bWhite )  // black Contur continues
    {                         // i.e. pixel in DirToBrd direction is "black"
      PtNextLeft = Next + dirToBord ;
      if ( rcROI.PtInRect( PtNextLeft ) )
      {
        if ( WhitePixel( pFr , PtNextLeft , iThres ) == bWhite ) // black Contur goes to left
        {
          // #ifdef _DEBUG
          //           ASSERT( Other( pFr , p + dirToBord , iThres , bInvert ) ) ;
          // #endif
          p = PtNextLeft ;
          TurnLeft( dirMove , dirToBord ) ;
        }
        else
          p = Next ;
      }
      else
        break ;  // this branch is finished on edge
    }
    else
    {
      TurnRight( dirMove , dirToBord ) ;
    }
    BOOL bMeasured = BorderByTwo( pFr , p , NewPt , iThres , dirToBord ) ;
    if ( !bMeasured )
      ASSERT( 0 ) ;
    SecondPartOfContur.Add( NewPt ) ;
    double dPath = abs( NewPt - PrevPt ) ;
    dLength += dPath ;
    if ( ( NewPt == FirstPt )
      && SecondPartOfContur.GetCount() > 3 )  // protection against exit near first pixel
    {                               // we will pass first pixel after returning to mid edge 
      //ASSERT( 0 ) ; // impossible !!!
      return dLength ;   // contur is closed
    }
    PrevPt = NewPt ;
    Next = p + dirMove ;
  } while ( 1 ) ;
  OutContur.SetSize( Contur.Count() + SecondPartOfContur.Count() ) ;
  cmplx* pOutData = OutContur.GetData() ;
  cmplx* pFirstPartData = Contur.GetData() ;
  cmplx* pSecondPartData = SecondPartOfContur.GetData() ;
  cmplx* pSecond = pSecondPartData + SecondPartOfContur.Count() - 1 ;
  while ( pSecond >= pSecondPartData )
    *( pOutData++ ) = *( pSecond-- );
  memcpy( pOutData , pFirstPartData , Contur.Count() * sizeof( cmplx ) ) ;
  return dLength ;
}

double _find_spot_direction_in_rect(
  CColorSpot& Spot , const pTVFrame pFrame , int iThres ,
  CRect& OutFr , int iMin , int iMax , int iPixSize ,
  cmplx& Cent , CVideoObject * pVO = NULL )
{
  bool bFullArea = ( pVO ) ? ( pVO->m_WhatToMeasure & DET_MEASURE_FULL_FRAME_ANGLE ) != 0 : false ;
  DWORD dwFrameWidth = GetWidth( pFrame ) * iPixSize ;  // width in bytes
  bool bInverse = ( pVO && pVO->m_Contrast == BLACK_ON_WHITE ) ;
  ImgMoments Moments ;
  DWORD dwShiftToOutFr = ( dwFrameWidth * OutFr.top ) + ( OutFr.left * iPixSize ) ;
  LPBYTE pData = GetData( pFrame ) + dwShiftToOutFr ;
  for ( LONG y = OutFr.top ; y < OutFr.bottom ; y++ )
  {
    LPBYTE p = pData ;
    LPWORD pw = ( LPWORD ) p ;
    double dY = y - Cent.imag() ;
    double dX = OutFr.left - Cent.real() ;
    for ( LONG x = OutFr.left ; x < OutFr.right ; dX += 1. , x++ )
    {
      double v = ( iPixSize == 1 ) ? *( p++ ) : *( pw++ ) ;
      if ( !bInverse )
      {
        if ( !bFullArea && ( v < iThres ) )
          continue ;
      }
      else
      {
        if ( !bFullArea && ( v >= iThres ) )
          continue ;
        v = iMax - v ;
      }
      Moments.Add( dX , dY , v ) ;
    }
    pData += dwFrameWidth ;
  }
  cmplx Shift = Moments.GetCenter() ;
  //   if ( bInverse )
  //     Shift = -Shift ;
  Cent += Shift ;
  AdditionalResult NewResult( bFullArea ? DET_MEASURE_FULL_FRAME_ANGLE : 0 ,
    Cent , ( double ) iThres , Moments.GetAngle() ) ;

  return Moments.GetAngle() ;
}

double _find_spot_direction_by_runs(
  CColorSpot& Spot , const pTVFrame pFrame , int iThres ,
  CPoint& Offset , CVideoObject * pVO = NULL )
{
  int iPixSize ;
  DWORD dwCompression = pFrame->lpBMIH->biCompression ;
  if ( dwCompression == BI_Y8 || dwCompression == BI_Y800
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
    iPixSize = 1 ;
  else if ( dwCompression == BI_Y16 )
    iPixSize = 2 ;
  else
    return 360.0 ;

  int iMax = Spot.m_iMaxPixel ;
  bool bInverse = ( pVO && pVO->m_Contrast == BLACK_ON_WHITE ) ;
  LPBYTE pData = GetData( pFrame ) ;
  LPWORD pwData = ( LPWORD ) pData ;
  cmplx Cent( Spot.m_SimpleCenter.x , Spot.m_SimpleCenter.y ) ;
  DWORD dwWidth = pFrame->lpBMIH->biWidth ;
  DWORD dwInitialOffset = ( Offset.y * dwWidth ) + Offset.x ;
  ImgMoments MomentsCW , MomentsCNW ;
  for ( int irun = 0; irun < Spot.m_Runs.GetSize(); irun++ )
  {
    Runs myRun = Spot.m_Runs.GetAt( irun );
    DWORD dwOffset = dwInitialOffset + ( myRun.iY * dwWidth ) + myRun.m_iB ;
    LPBYTE pByteData = pData + dwOffset ;
    LPWORD pWordData = pwData + dwOffset ;
    double dCentrY = myRun.iY + Offset.y - Cent.imag()	;
    double dCentrX = myRun.m_iB + Offset.x - Cent.real() ;
    MomentsCNW.Add( myRun.m_iB , myRun.m_iE , myRun.iY ) ;
    for ( int ix = myRun.m_iB ; ix <= myRun.m_iE; ix++ )
    {
      double dVal = ( iPixSize == 1 ) ? *( pByteData++ ) : *( pWordData++ ) ;
      if ( bInverse )
        dVal = iMax - dVal ;
      MomentsCW.Add( dCentrX , dCentrY , dVal ) ;
      dCentrX += 1.0 ;
    }
  }
  double dAngleCNW = -MomentsCNW.GetAngle() ; // Y on image is going down
  double dAngleCW = -MomentsCW.GetAngle() ;
  cmplx dShiftCW = MomentsCW.GetCenter() ;
  cmplx dShiftCNW = MomentsCNW.GetCenter() ;

  Spot.m_dAngles[ 1 ] = RadToDeg( dAngleCW ) ;	  // to degrees
  Spot.m_dAngles[ 2 ] = RadToDeg( dAngleCNW ) ;	  // to degrees
  Spot.m_Centers[ 1 ] = dShiftCW + Cent ;
  Spot.m_Centers[ 2 ] = dShiftCNW + Cent ;
  return Spot.m_dAngles[ 1 ] ;
}

// Diameters by contour
void CalcDiameters( CColorSpot& Spot , cmplx& Cent , cmplx& Offset )
{
  if ( Spot.m_Contur.GetCount() < 3 )
    return ; // no contur data

  double dLongAngle = -DegToRad( Spot.m_dAngle ) ;
  cmplx LongDir = polar( 1.0 , dLongAngle ) ;
  cmplx ShortDir = LongDir * cmplx( 0. , 1.0 ) ;
  int iWidth = Spot.m_OuterFrame.Width() ;
  int iHeight = Spot.m_OuterFrame.Height() ;
  double dShortAngle = arg( ShortDir ) ;
  double dSumSides = ( double ) ( iWidth + iHeight ) ;

  double dDistThres = ( double ) max( iWidth , iHeight ) * 2. / Spot.m_Contur.GetCount() ;
  LongDir *= dSumSides ;
  ShortDir *= dSumSides ;
  cmplx LongPt1 = Cent + LongDir ;
  cmplx LongPt2 = Cent - LongDir ;
  CLine2d LongSegment( LongPt1 , LongPt2 ) ;

  cmplx ShortPt1 = Cent + ShortDir  ;
  cmplx ShortPt2 = Cent - ShortDir ;
  CLine2d ShortSegment( ShortPt1 , ShortPt2 ) ;

  cmplx UpLong( Cent ) , DownLong( Cent ) ,
    UpShort( Cent ) , DownShort( Cent ) ;
  double dLMaxToPlus = -DBL_MAX , dLMaxToMinus = -DBL_MAX ;
  double dSMaxToPlus = -DBL_MAX , dSMaxToMinus = -DBL_MAX ;
  cmplx PrevPt = Spot.m_Contur[ 0 ] ;
  double dPrevDistToLong = GetPtToLineDistance( PrevPt , LongPt1 , LongPt2 ) ;
  double dPrevDistToShort = GetPtToLineDistance( PrevPt , ShortPt1 , ShortPt2 ) ;
  double dPrevDistToCent = abs( PrevPt - Cent ) ;
  for ( int i = 1 ; i < Spot.m_Contur.GetCount() ; i++ )
  {
    cmplx& Pt = Spot.m_Contur[ i ] ;
    //     double dLength = abs( Pt - PrevPt ) ;
    //     Spot.m_dPerimeter += dLength ;
    double dDistToCent = abs( Pt - Cent ) ;
    double dDistToLong = GetPtToLineDistance( Pt , LongPt1 , LongPt2 ) ;
    double dDistToShort = GetPtToLineDistance( Pt , ShortPt1 , ShortPt2 ) ;

    if ( dDistToShort * dPrevDistToShort <= 0 ) // on different sides
    {
      cmplx Selected = ( fabs( dDistToShort ) < fabs( dPrevDistToShort ) ) ? Pt : PrevPt ;
      if ( dDistToLong > 0 )
      {
        if ( dDistToCent > dSMaxToPlus )
        {
          dSMaxToPlus = dDistToCent ;
          UpShort = Selected ;
        }
      }
      else
      {
        if ( dDistToCent > dSMaxToMinus )
        {
          dSMaxToMinus = dDistToCent ;
          DownShort = Selected ;
        }

      }
    }
    if ( dDistToLong * dPrevDistToLong <= 0 ) // on different sides
    {
      cmplx Selected = ( fabs( dDistToLong ) < fabs( dPrevDistToLong ) ) ? Pt : PrevPt ;
      if ( dDistToShort > 0 )
      {
        if ( dDistToCent > dLMaxToPlus )
        {
          dLMaxToPlus = dDistToCent ;
          UpLong = Selected ;
        }
      }
      else
      {
        if ( dDistToCent > dLMaxToMinus )
        {
          dLMaxToMinus = dDistToCent ;
          DownLong = Selected ;
        }
      }
    }
    dPrevDistToLong = dDistToLong ;
    dPrevDistToShort = dDistToShort ;
    dPrevDistToCent = dDistToCent ;
    PrevPt = Pt ;
  }

  Spot.m_dLongDiametr = abs( UpLong - DownLong ) ;
  Spot.m_dShortDiametr = abs( UpShort - DownShort )   ;
  Spot.m_UpLong = UpLong + Offset ;
  Spot.m_DownLong = DownLong + Offset ;
  Spot.m_UpShort = UpShort + Offset ;
  Spot.m_DownShort = DownShort + Offset ;

  Spot.m_WhatIsMeasured |= MEASURE_DIAMETERS | MEASURE_PERIMETER ;
}

void CalcDiameters( CColorSpot& Spot , const pTVFrame pFrame ,
  int iThres , bool bInverse , cmplx& Cent , cmplx& Offset )
{
  CRect rc( 0 , 0 , GetWidth( pFrame ) - 1 , GetHeight( pFrame ) - 1 ) ;
  cmplx dUpLong( Cent ) , dDownLong( Cent ) ,
    dUpShort( Cent ) , dDownShort( Cent ) ;
  // step to spot direction
  cmplx dStepLong = polar( 1. , -DegToRad( Spot.m_dAngle ) ) ;
  // step to orthogonal direction
  cmplx dStepShort = dStepLong * polar( 1. , M_PI2 ) ;
  LPBYTE pData = GetData( pFrame );

  if ( is16bit( pFrame ) )
  {
    int iBr ;
    do
    {
      dUpLong += dStepLong ;
      if ( !PtInRect( rc , dUpLong ) )
        break ;
      iBr = GetPixel16( pData , dUpLong , pFrame->lpBMIH->biWidth ) ;
    } while ( ( iBr > iThres ) ^ bInverse );
    do
    {
      dDownLong -= dStepLong ;
      if ( !PtInRect( rc , dDownLong ) )
        break ;
      iBr = GetPixel16( pData , dDownLong , pFrame->lpBMIH->biWidth ) ;
    } while ( ( iBr > iThres ) ^ bInverse ) ;
    do
    {
      dUpShort += dStepShort ;
      if ( !PtInRect( rc , dUpShort ) )
        break ;
      iBr = GetPixel16( pData , dUpShort , pFrame->lpBMIH->biWidth ) ;
    } while ( ( iBr > iThres ) ^ bInverse ) ;
    do
    {
      dDownShort -= dStepShort ;
      if ( !PtInRect( rc , dDownShort ) )
        break ;
      iBr = GetPixel16( pData , dDownShort , pFrame->lpBMIH->biWidth ) ;
    } while ( ( iBr > iThres ) ^ bInverse ) ;
  }
  else
  {
    int iBr ;
    do
    {
      dUpLong += dStepLong ;
      if ( !PtInRect( rc , dUpLong ) )
        break ;
      iBr = GetPixel8( pData , dUpLong , pFrame->lpBMIH->biWidth ) ;
    } while ( ( iBr > iThres ) ^ bInverse );
    do
    {
      dDownLong -= dStepLong ;
      if ( !PtInRect( rc , dDownLong ) )
        break ;
      iBr = GetPixel8( pData , dDownLong , pFrame->lpBMIH->biWidth ) ;
    } while ( ( iBr > iThres ) ^ bInverse ) ;
    do
    {
      dUpShort += dStepShort ;
      if ( !PtInRect( rc , dUpShort ) )
        break ;
      iBr = GetPixel8( pData , dUpShort , pFrame->lpBMIH->biWidth ) ;
    } while ( ( iBr > iThres ) ^ bInverse ) ;
    do
    {
      dDownShort -= dStepShort ;
      if ( !PtInRect( rc , dDownShort ) )
        break ;
      iBr = GetPixel8( pData , dDownShort , pFrame->lpBMIH->biWidth ) ;
    } while ( ( iBr > iThres ) ^ bInverse ) ;
  }
  Spot.m_dLongDiametr = abs( dUpLong - dDownLong ) ;
  Spot.m_dShortDiametr = abs( dUpShort - dDownShort )   ;
  Spot.m_UpLong = dUpLong + Offset ;
  Spot.m_DownLong = dDownLong + Offset ;
  Spot.m_UpShort = dUpShort + Offset ;
  Spot.m_DownShort = dDownShort + Offset ;
}

double _find_spot_direction_and_Profiles(
  CColorSpot& Spot , const pTVFrame pFrame , int iThres ,
  CPoint& Offset , CVideoObject * pVO = NULL )
{
  int iPixSize ;
  DWORD dwCompression = pFrame->lpBMIH->biCompression ;
  if ( dwCompression == BI_Y8 || dwCompression == BI_Y800
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
    iPixSize = 1 ;
  else if ( dwCompression == BI_Y16 )
    iPixSize = 2 ;
  else
    return 360.0 ;

  CRect OutFr = Spot.m_OuterFrame ;
  int width = OutFr.Width() ;
  int height = OutFr.Height() ;
  DWORD dwFrameWidth = pFrame->lpBMIH->biWidth ;
  DWORD dwFrameHeight = pFrame->lpBMIH->biHeight ;
  if ( width < 3 || height < 3 )
    return 0. ;


  cmplx Cent( Spot.m_SimpleCenter.x , Spot.m_SimpleCenter.y ) ;
  OutFr -= Offset ;
  cmplx CmplxOff( Offset.x , Offset.y ) ;

  double dXc = 0.5 * ( ( double ) OutFr.right + ( double ) OutFr.left - 1. ) ;
  double dYc = 0.5 * ( ( double ) OutFr.top + ( double ) OutFr.bottom - 1. ) ;
  CRect ScanArea ;
  if ( ( Spot.m_DiffrRadius.cx > 2 ) && ( Spot.m_DiffrRadius.cy > 2 )
    && ( Spot.m_DiffrRadius.cx * 6 > width + 3 )
    && ( Spot.m_DiffrRadius.cy * 6 > height + 3 ) )
  {
    width = Spot.m_DiffrRadius.cx * 6 ;
    height = Spot.m_DiffrRadius.cy * 6 ;
    ScanArea = CRect( CPoint( ROUND( dXc ) - ( width / 2 ) , ROUND( dYc ) - ( height / 2 ) ) , CSize( width , height ) ) ;
  }
  else
    ScanArea = OutFr ;

  Spot.m_HProfile.Realloc( width ) ;
  Spot.m_VProfile.Realloc( height ) ;
  Spot.m_HProfile.m_iProfOrigin = ScanArea.left + Offset.x ;
  Spot.m_VProfile.m_iProfOrigin = ScanArea.top + Offset.y  ;
  double avgV = 0 ;
  double * pV = Spot.m_VProfile.m_pProfData ;
  int iMin = 1000000000 ;
  int iMax = -1000000000 ;

  bool bInverse = false ;
  if ( pVO && pVO->m_Contrast == BLACK_ON_WHITE )
    bInverse = true ;

  LPBYTE pData = GetData( pFrame );

  DWORD dwWidthInBytes = dwFrameWidth * iPixSize ;
  DWORD dwShiftToAreaOrigin = ( ScanArea.top * dwWidthInBytes ) + ( ScanArea.left * iPixSize ) ;
  pData += dwShiftToAreaOrigin ;

  // Cycle for profiles calculation
  for ( LONG y = ScanArea.top ; y < ScanArea.bottom ; y++ )
  {
    LPBYTE p = pData ;
    LPWORD pw = ( LPWORD ) pData ;
    double * pH = Spot.m_HProfile.m_pProfData ;
    *pV = 0. ;
    for ( LONG x = ScanArea.left ; x < ScanArea.right ; x++ )
    {
      int iVal = ( iPixSize == 1 ) ? *( p++ ) : *( pw++ ) ;
      if ( iVal < iMin )
        iMin = iVal ;
      if ( iVal > iMax )
        iMax = iVal ;

      double v = iVal ;
      ( *pV ) += v;
      ( *( pH++ ) ) += v;
    }
    ( *pV ) /= ScanArea.Width() ;
    if ( *pV < Spot.m_VProfile.m_dMinValue )
      Spot.m_VProfile.m_dMinValue = *pV ;
    if ( *pV > Spot.m_VProfile.m_dMaxValue )
      Spot.m_VProfile.m_dMaxValue = *pV ;
    avgV += *( pV++ ) ;
    pData += dwWidthInBytes ;
  }
  Spot.m_iMinPixel = iMin ;
  Spot.m_Centers[ 0 ] = Cent ;
  int iRotThres = iThres ;
  if ( pVO && ( pVO->m_dRotationThreshold > 0. ) && ( pVO->m_dRotationThreshold < 1.0 ) )
  {
    iRotThres = Spot.m_iMinPixel
      + ROUND( ( Spot.m_iMaxPixel - Spot.m_iMinPixel ) * pVO->m_dRotationThreshold ) ;
  }

  Spot.m_dAngleAlt = _find_spot_direction_in_rect( Spot , pFrame ,
    iRotThres , OutFr , iMin , iMax , iPixSize , Spot.m_Centers[ 0 ] , pVO ) ;


  double * pH = Spot.m_HProfile.m_pProfData ;
  for ( LONG x = ScanArea.left ; x < ScanArea.right ; pH++ , x++ )
  {
    *pH /= ScanArea.Height() ;
    if ( *pH < Spot.m_HProfile.m_dMinValue )
      Spot.m_HProfile.m_dMinValue = *pH ;
    if ( *pH > Spot.m_HProfile.m_dMaxValue )
      Spot.m_HProfile.m_dMaxValue = *pH ;
  }

  Spot.m_iMaxPixel = iMax ;
  // we have Y to down directed, necessary to change
  //Spot.m_dAngle = Spot.m_dAngleAlt ; // angle for white spots
  Spot.m_dAngleAlt *= 180 / PI ;	// to degrees
  Spot.m_dAngles[ 0 ] = Spot.m_dAngleAlt ;

  Spot.m_WhatIsMeasured |= MEASURE_ANGLE | MEASURE_RATIO ;
  if ( pVO  &&  pVO->m_WhatToMeasure & ( DET_MEASURE_ANGLE_CW | DET_MEASURE_ANGLE_CNW ) )
  {
    if ( pVO->m_dRotationThreshold == pVO->m_dProfileThreshold )
    {  // there we could use the available runs.
      if ( Spot.m_dAngleWeighted != 0.0 )  // already measured
      {
      }
      else
      {
        CPoint RunsOffset = pVO->m_absAOS.TopLeft() ;
        _find_spot_direction_by_runs( Spot , pFrame , iRotThres , RunsOffset , pVO ) ;
      }
    }
    else
    {
      if ( ( ( pVO->m_Contrast == WHITE_ON_BLACK )
        && ( pVO->m_dRotationThreshold > pVO->m_dProfileThreshold ) )
        || ( ( pVO->m_Contrast == BLACK_ON_WHITE )
          && ( pVO->m_dRotationThreshold < pVO->m_dProfileThreshold ) ) )
      {
        // we can measure in the case, when rotation 
        // measurement spot is less, than size measurement spot
        CVideoObject TempVO( *pVO ) ;
        TempVO.m_Thresholds.RemoveAll() ;
        TempVO.m_RotationThresholds.RemoveAll() ;
        TempVO.m_dProfileThreshold = TempVO.m_dRotationThreshold ;
        TempVO.m_absAOS = Spot.m_OuterFrame ;
        TempVO.m_Thresholds.Add( TempVO.m_dProfileThreshold ) ;
        TempVO.m_RotationThresholds.Add( TempVO.m_dRotationThreshold ) ;
        if ( TempVO.MeasureSpot( pVO->m_pOriginalFrame )
          && TempVO.m_pSegmentation->m_ColSpots.GetCount() )
        {
          CColorSpot& TempSpot = TempVO.m_pSegmentation->m_ColSpots.GetAt( 0 ) ;
          Spot.m_dAngles[ 1 ] = TempSpot.m_dAngles[ 1 ] ;
          Spot.m_dAngles[ 2 ] = TempSpot.m_dAngles[ 2 ] ;
        }
        else
        {
          Spot.m_dAngles[ 1 ] = 360. ;	  // degrees
          Spot.m_dAngles[ 2 ] = 360. ;	  // degrees
        };

      }
      else
      {
        Spot.m_dAngles[ 1 ] = 360. ;	  // degrees
        Spot.m_dAngles[ 2 ] = 360. ;	  // degrees
      }
    }
  }

  return Spot.m_dAngleAlt ;
}

bool SpotDetailedMeasurement(
  CColorSpot& Spot , CVideoObject * vo ,
  const pTVFrame pFr , int iThres )
{
  int PrevResult = Spot.m_WhatIsMeasured ;
  Spot.m_dCentralIntegral = 0 ;
  Spot.m_dLDiffraction = 0 ;
  Spot.m_dRDiffraction = 0 ;
  Spot.m_dDDiffraction = 0 ;
  Spot.m_dUDiffraction = 0 ;

  if ( Spot.m_dBlobWidth == 0. )
  {
    Spot.m_dBlobWidth = Spot.m_OuterFrame.Width() ;
    Spot.m_dBlobHeigth = Spot.m_OuterFrame.Height() ;
  }

  CRect FullImage( 0 , 0 , GetWidth( pFr ) - 1 , GetHeight( pFr ) - 1 ) ;
  CPoint WorkOrigin( Spot.m_OuterFrame.CenterPoint() ) ;
  int iBackCalculationShift = vo->m_iHorBackCalcShift ;
  if ( !iBackCalculationShift )
    iBackCalculationShift = vo->m_DiffrRadius.cx * 5 ;
  else
  {
    if ( iBackCalculationShift < vo->m_DiffrRadius.cx * 3 )
      iBackCalculationShift = vo->m_DiffrRadius.cx * 3 ;
  }
  CSize FrameSize = ( iBackCalculationShift > 0 ) ?
    CSize( iBackCalculationShift , vo->m_DiffrRadius.cy * 3 )
    : CSize( ( Spot.m_OuterFrame.Width() + 1 ) / 2 , ( Spot.m_OuterFrame.Height() + 1 ) / 2 ) ;

  WorkOrigin -= FrameSize ;
  FrameSize.cx *= 2 ;
  FrameSize.cy *= 2 ;
  CRect WorkRect( WorkOrigin , FrameSize ) ;
  WorkRect += vo->m_absAOS.TopLeft();
  CDRect rc ;
  pTVFrame pWork = NULL ;
  CPoint Cent = Spot.m_SimpleCenter.GetCPoint() ;
  if ( FullImage.PtInRect( WorkRect.TopLeft() ) && FullImage.PtInRect( WorkRect.BottomRight() ) )
  {
    double dAngle = 360. ;
    if ( ( vo->m_WhatToMeasure & MEASURE_SUB_SIDE_BACKS ) && ( vo->m_DiffrRadius.cx > 0 ) )
    {
      // We have to create  new frame, because we need to modify image
      pWork = _cut_rect( pFr , WorkRect , FALSE ) ;
      if ( pWork )
      {
        int iNewMax = calc_and_substruct_side_background( pWork , vo->m_DiffrRadius.cx ) ;
        double dAverage = calc_profiles( pWork , &Spot.m_HProfile , &Spot.m_VProfile ) ;
        Spot.m_HProfile.m_iProfOrigin += WorkRect.left ;
        Spot.m_VProfile.m_iProfOrigin += WorkRect.top ;

        bool bOK = seekBorders( rc , Spot.m_HProfile , Spot.m_VProfile ,
          1 , 3 , vo->m_iMinContrast , vo->m_dProfileThreshold ) ;
        if ( bOK )
        {
          Spot.m_SimpleCenter.x = Spot.m_HProfile.m_iProfOrigin + ( rc.left + rc.right ) * 0.5 ;
          Spot.m_SimpleCenter.y = Spot.m_VProfile.m_iProfOrigin + ( rc.top + rc.bottom ) * 0.5 ;
          Spot.m_dBlobWidth = rc.right - rc.left ;
          Spot.m_dBlobHeigth = rc.bottom - rc.top ;
        }
      }
    }
    else // there are offset is zero  - we do use original image
    {

      dAngle = _find_spot_direction_and_Profiles(
        Spot , pFr , vo->m_iRotationThres , CPoint( 0 , 0 ) , vo ) ;
    }

//     if ( Spot.m_Centers[ 0 ].real() != 0.0 )
//       Spot.m_Centers[ 0 ] += cmplx( (double) WorkRect.left , (double) WorkRect.top ) ;
    if ( vo->m_WhatToMeasure & MEASURE_THICKNESS )
    {
      bool bOK = seekBorders( rc , Spot.m_HProfile , Spot.m_VProfile ,
        1 , 3 , vo->m_iMinContrast , vo->m_dProfileThreshold ) ;
      if ( bOK )
      {
        Spot.m_SimpleCenter.x = Spot.m_HProfile.m_iProfOrigin + ( rc.left + rc.right ) * 0.5 ;
        Spot.m_SimpleCenter.y = Spot.m_VProfile.m_iProfOrigin + ( rc.top + rc.bottom ) * 0.5 ;
        Spot.m_dBlobWidth = rc.right - rc.left ;
        Spot.m_dBlobHeigth = rc.bottom - rc.top ;
      }
    }
    CSize FullRad( Spot.m_DiffrRadius.cx * 3 , Spot.m_DiffrRadius.cy * 3 ) ;
    if ( Spot.m_DiffrRadius.cx
      &&  Spot.m_DiffrRadius.cy )
    {
      CSize MeasureSize( Spot.m_DiffrRadius.cx * 2 , Spot.m_DiffrRadius.cy * 2 ) ;
      double dFullSum = 0. ;
      if ( pWork )
      {
        CPoint Orig( Spot.m_DiffrRadius.cx * 2 , 0 ) ;
        for ( int iy = 0 ; iy < 3 ; iy++ )
        {
          for ( int ix = 0 ; ix < 3 ; ix++ )
          {
            CRect rc( Orig + CSize( MeasureSize.cx * ix , MeasureSize.cy * iy ) ,
              MeasureSize ) ;
            dFullSum += ( Spot.m_dIntegrals[ iy * 3 + ix ] = _calc_integral( pWork , rc ) ) ;
          }
        }
      }
      else if ( PtInRect( FullImage , Cent - FullRad ) // check for diffraction areas  
        && PtInRect( FullImage , Cent + FullRad ) ) // placement inside frame
      {
        CPoint Orig( Cent - FullRad ) ; // top left point of integral areas
        for ( int iy = 0 ; iy < 3 ; iy++ )
        {
          for ( int ix = 0 ; ix < 3 ; ix++ )
          {
            CRect rc( Orig + CSize( MeasureSize.cx * ix , MeasureSize.cy * iy ) ,
              MeasureSize ) ;
            dFullSum += ( Spot.m_dIntegrals[ iy * 3 + ix ] = _calc_integral( pFr , rc ) ) ;
          }
        }
      }
      if ( dFullSum > 0 )
      {
        Spot.m_dCentralIntegral = Spot.m_dIntegrals[ 4 ] / dFullSum ;
        Spot.m_dLDiffraction = ( Spot.m_dIntegrals[ 3 ] + (
          ( vo->m_WhatToMeasure & MEASURE_RATIO ) ? ( 0.5 * ( Spot.m_dIntegrals[ 0 ] + Spot.m_dIntegrals[ 6 ] ) ) : 0 ) ) / dFullSum ;
        Spot.m_dRDiffraction = ( Spot.m_dIntegrals[ 5 ] + (
          ( vo->m_WhatToMeasure & MEASURE_RATIO ) ? ( 0.5 * ( Spot.m_dIntegrals[ 2 ] + Spot.m_dIntegrals[ 8 ] ) ) : 0 ) ) / dFullSum ;
        Spot.m_dDDiffraction = ( Spot.m_dIntegrals[ 7 ] + (
          ( vo->m_WhatToMeasure & MEASURE_RATIO ) ? ( 0.5 * ( Spot.m_dIntegrals[ 6 ] + Spot.m_dIntegrals[ 8 ] ) ) : 0 ) ) / dFullSum ;
        Spot.m_dUDiffraction = ( Spot.m_dIntegrals[ 1 ] + (
          ( vo->m_WhatToMeasure & MEASURE_RATIO ) ? ( 0.5 * ( Spot.m_dIntegrals[ 0 ] + Spot.m_dIntegrals[ 2 ] ) ) : 0 ) ) / dFullSum ;
        Spot.m_dSumPower = dFullSum ;
        Spot.m_WhatIsMeasured |= MEASURE_DIFFRACT ;
      }
    }
    Spot.m_dCentral5x5Sum = _calc_integral( pFr , Cent , 2 ) ;
    Spot.m_dSumOverThreshold = _calc_sum_over_thres(
      pFr , Spot.m_OuterFrame , iThres ) ;// iMin is low threshold for range
    Spot.m_WhatIsMeasured |= MEASURE_AREA | MEASURE_POSITION ;

  }
  if ( !( Spot.m_WhatIsMeasured & MEASURE_ANGLE ) )
  {
    double dAngle = _find_spot_direction_and_Profiles(
      Spot , pFr , vo->m_iRotationThres , vo->m_absAOS.TopLeft() , vo ) ;
    if ( Spot.m_Centers[ 0 ].real() != 0.0 )
      Spot.m_Centers[ 0 ] += cmplx( ( double ) vo->m_absAOS.left , ( double ) vo->m_absAOS.top ) ;
  }
  if ( pWork )
    freeTVFrame( pWork ) ;
  vo->m_WhatIsMeasured |= Spot.m_WhatIsMeasured ;

  return ( PrevResult != Spot.m_WhatIsMeasured ) ;
}

void CVideoObject::ClearData()
{
  if ( m_pSegmentation )
  {
    delete m_pSegmentation ;
    m_pSegmentation = NULL ;
  }
}

bool CVideoObject::MeasureSpot( const CVideoFrame* vf )
{
  double dStart = GetHRTickCount() ;
  if ( m_pSegmentation == NULL )
    m_pSegmentation = new CSegmentation( m_iNObjectsMax , m_dTimeout ) ;
  m_WhatIsMeasured = 0 ;
  m_pOriginalFrame = vf ;

  pTVFrame ptv = NULL ;
  CRect FrameRect( 0 , 0 , GetWidth( vf ) , GetHeight( vf ) ) ;
  if ( m_LeaderIndex.x == -1
    && m_LeaderIndex.y == -1
    && m_AOS.left < 0 )
  {
    m_absAOS = FrameRect ;
    ptv = makecopyTVFrame( vf ) ;
  }
  else
    ptv = _cut_rect( vf , m_absAOS , FALSE );
  if ( !ptv )
    return false ;
  m_Timing.push_back( GetHRTickCount() - dStart ) ;
  int iMin , iMax ;
  if ( m_WhatToMeasure & MEASURE_PROFILE )
  {
    _find_min_max_and_profiles(
      ptv , &m_HProfile , &m_VProfile ,
      iMin , iMax ) ;
    m_WhatIsMeasured |= MEASURE_PROFILE ;
  }
  else
  {
    if ( m_dHistThres > 0. )
    {
      FXIntArray Histogram ;
      if ( GetHistogram( ptv , Histogram ) )
      {
        int dThres = ROUND( m_dHistThres * GetImageSize( ptv ) ) ;
        int iAccumFromMinus = 0 , iAccumFromPlus = 0 ;
        for ( int i = 0; i < Histogram.GetCount() ; i++ )
        {
          if ( ( iAccumFromMinus += Histogram[ i ] ) >= dThres )
          {
            iMin = i ;
            break ;
          }
        }
        for ( int i = ( int ) Histogram.GetCount() - 1 ; i >= 0 ; i-- )
        {
          if ( ( iAccumFromPlus += Histogram[ i ] ) >= dThres )
          {
            iMax = i ;
            break ;
          }
        }
      }
    }
    else
      _find_min_max( ptv , iMin , iMax ) ;
  }
  m_dMeasuredMin = iMin ;
  m_dMeasuredMax = iMax ;
  if ( iMax - iMin < m_dMinProfileContrast )
  {
    m_pSegmentation->m_ColSpots.RemoveAll() ;
    m_LineResults.RemoveAll() ;
    freeTVFrame( ptv );
    return false ;
  }
  m_Timing.push_back( GetHRTickCount() - dStart ) ;

  int iThres ;
  if ( m_dProfileThreshold > 0.  &&  m_dProfileThreshold < 1.0 )
    iThres = iMin + ( int ) ( ( iMax - iMin ) * m_dProfileThreshold ) ;
  else if ( m_dProfileThreshold < 0. )
    iThres = iMin + ( iMin + iMax ) / 2 ;
  else
    iThres = ROUND( m_dProfileThreshold ) ;
  m_pSegmentation->SetClusterColors( iThres , iMax ) ;
  if ( ( m_WhatToMeasure & ( MEASURE_ANGLE | MEASURE_DIAMETERS ) )
    && !( m_WhatToMeasure & MEASURE_IMG_MOMENTS_W ) )
    m_WhatToMeasure |= MEASURE_IMG_MOMENTS_NW ;
  int iNSpots = m_pSegmentation->FindSpots( ptv , m_WhatToMeasure ) ;
  m_Timing.push_back( GetHRTickCount() - dStart ) ;

 // m_EndOfInitialProcessing.Add( GetHRTickCount() ) ;
  if ( iNSpots )
  {
    m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_AREA | ( m_WhatToMeasure & MEASURE_RUNS );
    pSpotArray Spots = m_pSegmentation->GetSpotsInfo() ;
    FXUintArray& ActiveIndexes = m_pSegmentation->m_ActiveSpots ;
    ActiveIndexes.RemoveAll() ;
    if ( m_bNoFiltration )
    {
      freeTVFrame( ptv );
      return true ;
    }

    if ( m_bDontTouchBorder )
    {
      for ( int i = 0 ; i < Spots->GetCount() ; i++ )
      {
        CColorSpot& Spot = Spots->GetAt( i );
        if ( Spot.m_Area > 0 )
        {
          if ( ( Spot.m_OuterFrame.left <= 1
            || Spot.m_OuterFrame.top <= 1
            || Spot.m_OuterFrame.right >= ptv->lpBMIH->biWidth - 2
            || Spot.m_OuterFrame.bottom >= ptv->lpBMIH->biHeight - 2 ) )
          {
            //         Spots->RemoveAt( i-- ) ;
            Spot.m_Area = -Spot.m_Area ;
          }
        }
      }
    }

    int iWhite = ( m_Contrast == WHITE_ON_BLACK ) ;
    CRect WorkingArea = m_absAOS ;
    if ( m_DiffrRadius.cx )
    {
      WorkingArea.DeflateRect( m_DiffrRadius.cx * 3 , m_DiffrRadius.cy * 3 ) ;
      WorkingArea &= FrameRect ;
    }

    cmplx Offset( WorkingArea.left , WorkingArea.top ) ;
    for ( int i = 0 ; i < Spots->GetCount() ; i++ )
    {
      CColorSpot& Spot = Spots->GetAt( i );
      if ( Spot.m_Area > m_iAreaMin )  //not removed spot
      {
        CPoint Cent = Spot.m_SimpleCenter.GetCPoint() ;
        if ( Cent.x <= 0 )
        {
          Spot.m_SimpleCenter = Spot.m_CenterByIntense + m_absAOS.TopLeft() ;
          Cent = Spot.m_SimpleCenter.GetCPoint() ;
        }
        else
        {
          Cent.Offset( m_absAOS.TopLeft() ) ;
          Spot.m_SimpleCenter += m_absAOS.TopLeft() ;
        }
        if ( ( ( iWhite == Spot.m_iColor ) || ( m_Contrast == ANY_CONTRAST ) )
          && (/* !m_WhatToMeasure || */m_absAOS.PtInRect( Cent ) )
          && ( Spot.m_OuterFrame.Width() + 1 ) >= m_ExpectedSize.left
          && Spot.m_OuterFrame.Width() <= m_ExpectedSize.right
          && ( Spot.m_OuterFrame.Height() + 1 ) >= m_ExpectedSize.top
          && Spot.m_OuterFrame.Height() <= m_ExpectedSize.bottom
          && Spot.m_Area <= m_iAreaMax )
        {
          ActiveIndexes.Add( i ) ;
          if ( !( Spot.m_WhatIsMeasured & ( MEASURE_IMG_MOMENTS_W | MEASURE_IMG_MOMENTS_NW ) ) )
          {
            if ( Spot.m_SimpleCenter.x <= 0. )
            {
              Spot.m_SimpleCenter = CmplxToCDPoint( .5 * cmplx(
                ( double ) Spot.m_OuterFrame.left + ( double ) Spot.m_OuterFrame.right ,
                ( double ) Spot.m_OuterFrame.top + ( double ) Spot.m_OuterFrame.bottom ) )
                + m_absAOS.TopLeft() ;
            }
          }
          else
          {
            if ( Spot.m_WhatIsMeasured & MEASURE_IMG_MOMENTS_W )
            {
              Spot.m_SimpleCenter = CmplxToCDPoint( Spot.m_ImgMomentsWeighted.GetCenter() )
                + m_absAOS.TopLeft() ;
              Spot.m_dAngleWeighted = -RadToDeg( Spot.m_ImgMomentsWeighted.GetAngle() ) ;
              Spot.m_dAngle = Spot.m_dAngleWeighted ;
            }
            else
            {
              Spot.m_SimpleCenter = CmplxToCDPoint( Spot.m_ImgMoments.GetCenter() )
                + m_absAOS.TopLeft() ;
              Spot.m_dAngle = -RadToDeg( Spot.m_ImgMoments.GetAngle() ) ;
            }
          }
          Spot.m_Centers[ 0 ] = CDPointToCmplx( Spot.m_SimpleCenter ) ;
          m_WhatIsMeasured |= Spot.m_WhatIsMeasured ;
          if ( !m_bMulti )
            break ;
        }
        else
        {
          //       Spots->RemoveAt( i-- ) ;
          Spot.m_Area = -Spot.m_Area ; // mark, that spot is not matched
        }
      }
      else if ( Spot.m_Area > 0 )
        Spot.m_Area = -Spot.m_Area ;
    }
    m_Timing.push_back( GetHRTickCount() - dStart ) ;

    if ( ActiveIndexes.GetCount() && m_DiffrRadius.cx && m_DiffrRadius.cy )
    {
      for ( int i = 0 ; i < ActiveIndexes.GetCount() ; i++ )
      {
        DWORD iActive1 = ActiveIndexes[ i ] ;
        CColorSpot& Spot1 = Spots->GetAt( iActive1 );
        for ( int j = i + 1 ; j < ActiveIndexes.GetCount() ; j++ )
        {
          DWORD iActive2 = ActiveIndexes[ j ] ;
          CColorSpot& Spot2 = Spots->GetAt( iActive2 );
          if ( Spot2.m_Area <= 0 )
            continue ;
          CDPoint Diff = Spot1.m_SimpleCenter - Spot2.m_SimpleCenter ;
          if ( fabs( Diff.x ) <= ( m_DiffrRadius.cx * 3 )
            && fabs( Diff.y ) <= ( m_DiffrRadius.cy * 3 ) )
          {
            if ( Spot1.m_Area > Spot2.m_Area )
            {
              Spot1.m_OuterFrame |= Spot2.m_OuterFrame ;
              //Spots->RemoveAt( j-- ) ;
              Spot2.m_Area = -Spot2.m_Area ;
              ActiveIndexes.RemoveAt( j ) ;
            }
            else
            {
              Spot2.m_OuterFrame |= Spot1.m_OuterFrame ;
              Spot1.m_Area = -Spot1.m_Area ;
              ActiveIndexes.RemoveAt( i ) ;
              break ;
            }
          }
        }
      }
    }
    m_Timing.push_back( GetHRTickCount() - dStart ) ;

    bool bFOVposFilled = false ;
    for ( int i = 0 ; i < ActiveIndexes.GetCount() ; i++ )
    {
      DWORD iActive = ActiveIndexes[ i ] ;
      CColorSpot& Spot = Spots->GetAt( iActive );
      strcpy_s( Spot.m_szName , m_ObjectName ) ;
      if ( !( Spot.m_WhatToMeasure & MEASURE_CONTUR ) )
        m_pSegmentation->MeasureSpotSize( Spot ) ;
      if ( Spot.m_WhatIsMeasured & MEASURE_IMG_MOMENTS_W )
      {
        Spot.m_SimpleCenter = CmplxToCDPoint( Spot.m_ImgMomentsWeighted.GetCenter() )
          + m_absAOS.TopLeft() ;
        Spot.m_CenterByIntense = Spot.m_SimpleCenter ;
      }
      else if ( Spot.m_WhatIsMeasured & MEASURE_IMG_MOMENTS_NW )
      {
        Spot.m_SimpleCenter = CmplxToCDPoint( Spot.m_ImgMoments.GetCenter() )
          + m_absAOS.TopLeft() ;
        Spot.m_CenterByIntense = Spot.m_SimpleCenter ;
      }
      else if ( !Spot.m_dBlobWidth )
      {
        Spot.m_OuterFrame.OffsetRect( m_absAOS.TopLeft() ) ;
        Spot.m_SimpleCenter = CDPoint( ( double ) ( Spot.m_OuterFrame.left + Spot.m_OuterFrame.right ) / 2. ,
          ( double ) ( Spot.m_OuterFrame.top + Spot.m_OuterFrame.bottom ) / 2. );
      }
      else
      {
        Spot.m_OuterFrame.OffsetRect( m_absAOS.TopLeft() ) ;
        Spot.m_SimpleCenter = CDPoint( ( double ) ( Spot.m_OuterFrame.left + Spot.m_OuterFrame.right ) / 2. ,
          ( double ) ( Spot.m_OuterFrame.top + Spot.m_OuterFrame.bottom ) / 2. );
      }


      Spot.m_WhatIsMeasured |= MEASURE_POSITION ;
      if ( Spot.m_WhatToMeasure & ( MEASURE_CONTUR | MEASURE_DIAMETERS ) )
      {
        double dStartSubEdgeFindTime = GetHRTickCount() ;
        Spot.m_dAccurateArea = _FindSubPixelEdge( Spot , vf , iThres ,
          m_absAOS , Spot.m_FirstPoint + m_absAOS.TopLeft() ) ;
        bool bAreaOK = ( Spot.m_dAccurateArea >= m_iAreaMin )
          && ( Spot.m_dAccurateArea <= m_iAreaMax ) ;
        if ( bAreaOK )
        {
          Spot.m_WhatIsMeasured |= MEASURE_CONTUR ;
          Spot.m_Area = ROUND( Spot.m_dAccurateArea ) ;

          cmplx Offset( 0. , 0. ) ;
          cmplx Cent( Spot.m_SimpleCenter.x , Spot.m_SimpleCenter.y ) ;
          if ( Spot.m_WhatToMeasure & MEASURE_DIAMETERS )
          {
            CalcDiameters( Spot , Cent , Offset ) ;
            Spot.m_WhatIsMeasured |= MEASURE_DIAMETERS ;
          }
          if ( Spot.m_Contur.GetCount() )
          {
            Spot.m_EdgesAsDbl = CDRect( DBL_MAX , DBL_MIN , DBL_MAX , DBL_MIN ) ;
            Spot.m_dPerimeter = 0. ;
            cmplx Prev = Spot.m_Contur[ 0 ] ;
            cmplx First = Prev ;
            for ( int i = 1 ; i < Spot.m_Contur.GetCount() ; i++ )
            {
              cmplx Next = Spot.m_Contur[ i ] ;
              Spot.m_dPerimeter += abs( Next - Prev ) ;
              CorrectRect( Prev , Spot.m_EdgesAsDbl ) ;
              Prev = Next ;
            }
            Spot.m_dPerimeter += abs( First - Prev ) ;

            Spot.m_dBlobWidth = Spot.m_EdgesAsDbl.Width() ;
            Spot.m_dBlobHeigth = Spot.m_EdgesAsDbl.Height() ;
          }
        }
        else
          ActiveIndexes.RemoveAt( i-- ) ;
        m_EndSubpixelEdgeFind.Add( GetHRTickCount() - dStartSubEdgeFindTime ) ;
        if ( !bAreaOK )
          continue; // Go to next spot
      }

      if ( Spot.m_WhatToMeasure & ( MEASURE_PROFILE ) )
      {
        CRect ProfROI( m_absAOS ) ;
        // Correct sizes, because width and height are used instead of right and bottom
        ProfROI.right -= ProfROI.left ;
        ProfROI.bottom -= ProfROI.top ;
        calc_profiles( vf , &Spot.m_HProfile , &Spot.m_VProfile , &ProfROI ) ;
        Spot.m_WhatIsMeasured |= MEASURE_PROFILE ;
        //       if ( Spot.m_WhatToMeasure & MEASURE_CENT_BY_PROF )
        //       {
        //         DRECT Result ;
        //         bool bRes = seekBorders( Result , Spot.m_HProfile , Spot.m_VProfile , 
        //           Spot.m_iColor , 3 , m_iMinContrast ,
        //           m_dProfileThreshold , m_iDebouncing_pix ) ;
        //         if ( bRes )
        //           Spot.m_WhatIsMeasured |= MEASURE_CENT_BY_PROF ;
        //       }
        // 
      }

      if ( Spot.m_WhatToMeasure  & ~( MEASURE_IMG_MOMENTS_NW | MEASURE_IMG_MOMENTS_W )
        &  ~( MEASURE_CONTUR | MEASURE_ANGLE | MEASURE_POSITION | MEASURE_DIAMETERS
          | MEASURE_AREA | MEASURE_PROFILE ) )
      {
        double dSaveProfThres = m_dProfileThreshold ;
        if ( m_dProfileThreshold > 0.  &&  m_dProfileThreshold < 1.0 )
          iThres = iMin + ( int ) ( ( iMax - iMin ) * m_dProfileThreshold ) ;
        else if ( m_dProfileThreshold < 0. )
          iThres = iMin + ( iMin + iMax ) / 2 ;
        else
        {
          iThres = ROUND( m_dProfileThreshold ) ;
          m_dProfileThreshold = dSaveProfThres / ( double ) ( Spot.m_iMaxPixel - iMin ) ;
        }
        if ( ( Spot.m_WhatToMeasure & ( MEASURE_RATIO | MEASURE_DIFFRACT ) )
          && m_DiffrRadius.cx && m_DiffrRadius.cy )
        {
          Spot.m_DiffrRadius = m_DiffrRadius ;
          m_iRotationThres = iMin + ( int ) ( ( -iMin ) * m_dRotationThreshold ) ;
          SpotDetailedMeasurement( Spot , this , vf , iThres ) ;
          m_WhatIsMeasured |= Spot.m_WhatIsMeasured ;
          cmplx Cent( Spot.m_CenterByIntense.x , Spot.m_CenterByIntense.y ) ;
          //       cmplx Offset( (double)m_absAOS.left , (double)m_absAOS.top ) ;
          cmplx Offset( 0. , 0. ) ;
          //         CalcDiameters( Spot , Cent , Offset ) ;
          //CalcDiameters( Spot , vf , iThres , !(iWhite > 0) , Cent , Offset ) ;
          m_WhatIsMeasured |= MEASURE_RATIO | MEASURE_DIFFRACT;
        }
        m_dProfileThreshold = dSaveProfThres ;
      }
      if ( !bFOVposFilled )
      {
        m_InFOVPosition = Spot.m_SimpleCenter ;
        bFOVposFilled = true ;
      }
      m_WhatIsMeasured |= Spot.m_WhatIsMeasured ;

      double dTimeFromStart = GetWorkingTime() ;
      if ( !m_bMulti )
      {
        break ;
      }
      if ( IsTimeout() )
      {
        m_Status.Format( _T( "%s meas timeout %.1fms\n Nspots=%d Nact=%d" ) ,
          ( LPCTSTR ) m_ObjectName , GetWorkingTime() , Spots->GetCount() ,
          ActiveIndexes.GetCount() ) ;
        Spots->RemoveAll() ;
        m_WhatIsMeasured = 0 ;
        break ;
      }
    }
  }
  else
  {
    if ( m_pSegmentation->m_Status[ 0 ] )
      m_Status.Format( "%s spots search error:\n%s T=%.1fms" ,
      ( LPCTSTR ) m_ObjectName , m_pSegmentation->m_Status ,
        GetWorkingTime() ) ;
  }
  m_Timing.push_back( GetHRTickCount() - dStart ) ;

  freeTVFrame( ptv );
  m_EndOfFinalProcessing.Add( GetHRTickCount() ) ;
  return ( m_WhatIsMeasured != 0 );
}

bool CVideoObject::MeasureLine( const CVideoFrame* vf )
{
  //    FXString fname;
  //    fname.Format("D:\\1\\_measureSpot%04d.bmp",a); a++;
  pTVFrame ptv = _cut_rect( vf , m_absAOS , FALSE );
  if ( !ptv )
    return false;
  int width = ptv->lpBMIH->biWidth , height = ptv->lpBMIH->biHeight;
  int iFullAreaMin , iFullAreaMax ;
  _find_min_max( ptv , iFullAreaMin , iFullAreaMax ) ;
  m_dMeasuredMin = iFullAreaMin ;
  m_dMeasuredMax = iFullAreaMax ;
  //  _calc_diff_image( ptv ) ;
  CDRect rc ;

  DWORD dir = m_Direction ;
  //   switch (m_Direction)
  //   {
  //   case ANY_DIRECTION : dir=3; break;
  //   case HORIZONTAL : dir=2; break;
  //   case VERTICAL : dir=1; break;
  //   }
  m_LineResults.RemoveAll() ;
  if ( m_bMulti )
  {
    calc_profiles( ptv , &m_HProfile , &m_VProfile ) ;
    if ( m_iProfileLocalization_pix )
    {
      if ( dir & VERTICAL )
        localize( &m_HProfile , m_iProfileLocalization_pix ) ;
      if ( dir & HORIZONTAL )
        localize( &m_VProfile , m_iProfileLocalization_pix ) ;
    }
    CDRectArray Lines ;
    DWORD dwContrast = 2 ; // default - any border
    switch ( m_Contrast )
    {
    case BLACK_ON_WHITE:
    case WHITE_TO_BLACK_BRD:
      dwContrast = 1 ; break ;
    case WHITE_ON_BLACK:
    case BLACK_TO_WHITE_BRD:
      dwContrast = 0 ; break ;
    default: break ;
    }
    if ( seekMultiBorders( Lines , &m_HProfile , &m_VProfile ,
      dwContrast , dir ,
      m_dMinProfileContrast , m_dProfileThreshold , m_iDebouncing_pix ) )
    {
      if ( IsHorizDir( m_Direction ) )
        m_WhatIsMeasured |= MEASURE_YPROFILE ;
      if ( IsVertDir( m_Direction ) )
        m_WhatIsMeasured |= MEASURE_XPROFILE ;

      for ( int i = 0 ; i < Lines.GetCount() ; i++ )
      {
        CLineResult LRes ;

        if ( dir & HORIZONTAL )
        {
          if ( Lines[ i ].bottom == 0.0 )
            break ;  // no finished line, only upper border
          Lines[ i ].left = m_absAOS.left ;
          Lines[ i ].right = m_absAOS.right ;
        }
        if ( dir & VERTICAL )
        {
          if ( Lines[ i ].right == 0.0 )
            break ;  // no finished line, only left border
          Lines[ i ].top = m_absAOS.top ;
          Lines[ i ].bottom = m_absAOS.bottom ;
        }

        CDRect rc( Lines[ i ] ) ;
        if ( ( dir & VERTICAL ) && ( rc.left >= 0.0 ) && ( ( m_absAOS.Width() - rc.right ) > 2.0 ) )
        {
          m_WhatIsMeasured |= MEASURE_POSITION;

          LRes.m_Center.x = ( rc.left + rc.right ) * 0.5 ;
          if ( m_WhatToMeasure & MEASURE_POSITION )
          {
            int iCoord = ( int ) rc.left ;
            double dLeftX = 1. - ( rc.left - iCoord ) ;
            double dSum = dLeftX * m_HProfile.m_pProfData[ iCoord ] * ( rc.left + ( dLeftX / 2.0 ) ) ;
            double dAmplSum = dLeftX * m_HProfile.m_pProfData[ iCoord ] ;
            iCoord++ ;
            while ( iCoord < ( rc.right - 1. ) )
            {
              dSum += ( iCoord + 0.5 ) * m_HProfile.m_pProfData[ iCoord ] ;
              dAmplSum += m_HProfile.m_pProfData[ iCoord ] ;
              iCoord++ ;
            }
            double dRightX = ( rc.right - iCoord ) ;
            dSum += dRightX * m_HProfile.m_pProfData[ iCoord ] * ( iCoord + ( dRightX / 2. ) ) ;
            dAmplSum += dRightX * m_HProfile.m_pProfData[ iCoord ] ;
            if ( dAmplSum > 0.1 )
            {
              double dXCent = dSum / dAmplSum ;
              double dDeltaX = dXCent - LRes.m_Center.x ;
              LRes.m_Center.x = dXCent ;
            }
          }
          rc.Offset( ( double ) m_absAOS.left , 0. ) ;
        }
        else
          LRes.m_Center.x = m_absAOS.Width() * 0.5 ;

        if ( ( dir & HORIZONTAL ) && ( rc.top >= 0.0 ) && ( ( m_absAOS.Height() - rc.bottom ) > 2.0 ) )
        {
          m_WhatIsMeasured |= MEASURE_POSITION;
          LRes.m_Center.y = ( rc.top + rc.bottom ) * 0.5 ;
          if ( m_WhatToMeasure & MEASURE_POSITION )
          {
            int iCoord = ( int ) rc.top ;
            double dTopY = 1. - ( rc.top - iCoord ) ;
            double dSum = dTopY * m_VProfile.m_pProfData[ iCoord ] * ( rc.top + ( dTopY / 2.0 ) ) ;
            double dAmplSum = dTopY * m_VProfile.m_pProfData[ iCoord ] ;
            iCoord++ ;
            while ( iCoord < ( rc.top - 1. ) )
            {
              dSum += ( iCoord + 0.5 ) * m_VProfile.m_pProfData[ iCoord ] ;
              dAmplSum += m_VProfile.m_pProfData[ iCoord ] ;
              iCoord++ ;
            }
            double dBottomY = ( rc.bottom - iCoord ) ;
            dSum += dBottomY * m_VProfile.m_pProfData[ iCoord ] * ( iCoord + ( dBottomY / 2. ) ) ;
            dAmplSum += dBottomY * m_VProfile.m_pProfData[ iCoord ] ;
            if ( dAmplSum > 0.1 )
            {
              double dYCent = dSum / dAmplSum ;
              double dDeltaX = dYCent - LRes.m_Center.y ;
              LRes.m_Center.y = dYCent ;
            }
          }
          rc.Offset( 0. , ( double ) m_absAOS.top ) ;
        }
        else
          LRes.m_Center.y = m_absAOS.Height() * 0.5 ;

        CRect Area( ( int ) rc.left , ( int ) rc.top , ( int ) rc.right , ( int ) rc.bottom ) ;
        int min , max ;
        int iAverage = _find_min_max( vf , min , max , Area );
        LRes.m_dExtremalAmpl = ( iAverage < GetPixel( ptv , LRes.m_Center ) ) ? max : min ;
        LRes.m_dAverCent5x5 = _calc_integral( ptv ,
          CPoint( ROUND( LRes.m_Center.x ) , ROUND( LRes.m_Center.y ) ) , 2 );
        LRes.m_dAverCent5x5 /= 25 ;
        LRes.m_Center.x += m_absAOS.left;
        LRes.m_Center.y += m_absAOS.top ;
        LRes.m_DRect = rc ;
        LRes.m_ImageMinBrightness = iFullAreaMin ;
        LRes.m_ImageMaxBrightness = iFullAreaMax ;
        LRes.m_iIndex = i ;
        if ( (dir & VERTICAL) && m_LineResults.size() )
        {
          ASSERT( m_LineResults.Last().m_Center.x < LRes.m_Center.x ) ;
        }
        m_LineResults.Add( LRes ) ;
      }
    }
  }
  else if ( seekBorders( ptv , rc , &m_HProfile , &m_VProfile ,
    ( m_Contrast == BLACK_ON_WHITE ) , dir ,
    m_dMinProfileContrast , m_dProfileThreshold , m_iDebouncing_pix ) )
  {
    CLineResult LRes ;
    int nReallyMeasured = 0 ;
    if ( IsHorizDir( m_Direction ) )
      m_WhatIsMeasured |= MEASURE_YPROFILE ;
    if ( IsVertDir( m_Direction ) )
      m_WhatIsMeasured |= MEASURE_XPROFILE ;

    if ( ( dir & VERTICAL ) && ( rc.left > 2. ) && ( ( m_absAOS.Width() - rc.right ) > 2.0 ) )
    {
      m_WhatIsMeasured |= MEASURE_POSITION;
      LRes.m_Center.x = m_InFOVPosition.x = ( rc.left + rc.right ) * 0.5 ;
      rc.Offset( ( double ) m_absAOS.left , 0. ) ;
    }
    else
      LRes.m_Center.x = m_InFOVPosition.x = ( double ) width / 2. ;

    if ( ( dir & HORIZONTAL ) && ( rc.top > 2. ) && ( ( m_absAOS.Height() - rc.bottom ) > 2.0 ) )
    {
      m_WhatIsMeasured |= MEASURE_POSITION;
      LRes.m_Center.y = m_InFOVPosition.y = ( rc.top + rc.bottom ) * 0.5 ;
      rc.Offset( 0. , ( double ) m_absAOS.top ) ;
    }
    else
      LRes.m_Center.y = m_InFOVPosition.y = ( double ) height / 2. ;

    LRes.m_dAverCent5x5 = _calc_integral( ptv ,
      CPoint( ROUND( LRes.m_Center.x ) , ROUND( LRes.m_Center.y ) ) , 2 );
    LRes.m_dAverCent5x5 /= 25 ;

    m_InFOVPosition.x += m_absAOS.left;
    m_InFOVPosition.y += m_absAOS.top /*+ 1*/ ;

    LRes.m_Center = m_InFOVPosition ;
    LRes.m_DRect = rc ;
    LRes.m_ImageMinBrightness = iFullAreaMin ;
    LRes.m_ImageMaxBrightness = iFullAreaMax ;

    m_LineResults.Add( LRes ) ;
  }
  if ( dir & VERTICAL )
  {
    m_dMeasuredMin = m_HProfile.m_dMinValue ;
    m_dMeasuredMax = m_HProfile.m_dMaxValue ;
  }
  else  // Horizontal
  {
    m_dMeasuredMin = m_VProfile.m_dMinValue ;
    m_dMeasuredMax = m_VProfile.m_dMaxValue ;
  }
  freeTVFrame( ptv );
  for ( int i = 0 ; i < m_LineResults.GetCount() ; i++ )
  {
    CLineResult& Line = m_LineResults.GetAt( i );
    if ( m_Direction == HORIZONTAL )
    {
      double dHeight = Line.m_DRect.Height();
      if ( dHeight < m_ExpectedSize.top
        || dHeight > m_ExpectedSize.bottom )
      {
        m_LineResults.RemoveAt( i-- );
      }
    }
    else // VERTICAL
    {
      double dWidth = Line.m_DRect.Width();
      if ( dWidth < m_ExpectedSize.left
        || dWidth > m_ExpectedSize.right )
      {
        m_LineResults.RemoveAt( i-- );
      }
    }
  }
  return ( ( m_WhatIsMeasured & MEASURE_POSITION ) != 0 );
}

void _simpleFilters( pClustersInfo pCI , int minH , int maxH )
{
  for ( int i = 0; i < pCI->m_ClustersNmb; i++ )
  {

    if ( pCI->m_Clusters[ i ].size.cy > maxH ) { Cluster_RemoveAt( pCI->m_Clusters , pCI->m_ClustersNmb , i ); i--; continue; }
    if ( pCI->m_Clusters[ i ].size.cy < minH ) { Cluster_RemoveAt( pCI->m_Clusters , pCI->m_ClustersNmb , i ); i--; continue; }


    /*        ASSERT(pCI->m_Clusters[i].size.cy!=0);
    double ratio=((double)pCI->m_Clusters[i].size.cx)/pCI->m_Clusters[i].size.cy;

    if (ratio>CHAR_MAX_XYRATIO)				   { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--; continue; }
    if (ratio<CHAR_MIN_XYRATIO)				   { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--; continue; } */
  }
}

bool CVideoObject::MeasureText( const CVideoFrame* vf )
{
  double ts = GetHRTickCount();
  m_WhatIsMeasured &= ~MEASURE_TEXT ;
  int iAreaSize = m_absAOS.Width() * m_absAOS.Height() ;
  if ( ( iAreaSize == 0 )
    || ( !m_pTesseract && ( m_WhatToMeasure & MEASURE_TXT_FAST ) ) )
    return false ;  // No OCR subsystem

  int iOffset = m_absAOS.top * vf->lpBMIH->biWidth + m_absAOS.left ;
  int iBytesPerPixel = 1 ;
  switch ( vf->lpBMIH->biCompression )
  {
  case BI_Y16: iBytesPerPixel = 2 ; break ;
  case BI_RGB: iBytesPerPixel = 3 ; break ;
  case BI_Y800:
  case BI_Y8:
  case BI_YUV9:
  case BI_YUV12: break ;
  default:
    SENDERR( "OCR not supported format %s" , GetVideoFormatName( vf->lpBMIH->biCompression ) ) ;
    return false ; // not supported formats
  }
  iOffset *= iBytesPerPixel ;
  iAreaSize *= iBytesPerPixel ;
  int iTouchedSize = m_absAOS.Height() * vf->lpBMIH->biWidth * iBytesPerPixel ;
  if ( iOffset < 0
    || ( iOffset + iTouchedSize ) >( int )vf->lpBMIH->biSizeImage )
  {
    return false ; // AOS on edge
  }
  int expectedH = 0;
  pTVFrame ptv = NULL ;
  if ( !m_bMulti )
  {
    ptv = _cut_rect( vf , m_absAOS , FALSE ) ;
    if ( !ptv )
      return false;

    switch ( m_Direction )
    {
      case DIR_03:
      expectedH = m_absAOS.Height();
      break;
      case DIR_06:
      expectedH = m_absAOS.Width();
  //     _rotate_minus90_Y8_16( ptv ) ;
      _rotate90_Y8_16( ptv );
      break;
      case DIR_09:
      expectedH = m_absAOS.Height();
      _rotate180_Y8_16( ptv ) ;
      break;
      case DIR_00:
      expectedH = m_absAOS.Width();
  //     _rotate90_Y8_16( ptv );
      _rotate_minus90_Y8_16( ptv ) ;
      break;
      default:
      SENDERR_0( "Measurement for object OCR can't be performed. Reason: no direction is defined." );
    }
  }
  else // multi string recognition
  {
    FXString Result ;
    RecognizeTextByTesseract( vf , Result ) ;
    m_TextResult = Result ;

    return ( ( m_WhatIsMeasured & MEASURE_TEXT ) != 0 );
  }
  if ( m_WhatToMeasure & MEASURE_TXT_FAST ) // do use tesseract
  {
  // following is for rotation debug 
//     for ( DWORD iy = 0 ; iy < GetHeight(ptv) ; iy++ )
//     {
//       LPBYTE pSrc = GetData(ptv) + iy * GetWidth( ptv ) ;
//       LPBYTE pDst = GetData( vf ) + 20 + (40 + iy) * GetWidth( vf ) ;
//       memcpy( pDst , pSrc , GetWidth(ptv) ) ;
//     }
    FXString ResultString ;
    int iTextLen = RecognizeTextByTesseract(
      ptv , expectedH , iBytesPerPixel , ResultString ) ;
    if ( iTextLen )
      m_TextResult = ResultString ;
  }
  else
  {
#ifdef _DEBUG
    //saveDIB(fname, ptv);
#endif
    RecognizeTextByOCRLib( ptv , expectedH , iBytesPerPixel ) ;
  }

  return ( ( m_WhatIsMeasured & MEASURE_TEXT ) != 0 );
}

bool CVideoObject::SetInitialPtAndStep(
  CPoint& Origin , CSize& Step , int width , int height )
{
  switch ( m_Direction )
  {
  case DIR_00:
    Step = CSize( 0 , -1 ) ;
    Origin = CPoint( width / 2 , height - 2 ) ;
    break ;
  case DIR_01_30:
    Step = CSize( 1 , -1 ) ;
    Origin = CPoint( 1 , height - 2 ) ;
    break ;
  case DIR_03:
    Step = CSize( 1 , 0 ) ;
    Origin = CPoint( 1 , height / 2 ) ;
    break ;
  case DIR_04_30:
    Step = CSize( 1 , 1 ) ;
    Origin = CPoint( 1 , 1 ) ;
    break ;
  case DIR_06:
    Step = CSize( 0 , 1 ) ;
    Origin = CPoint( width / 2 , 1 ) ;
    break ;
  case DIR_07_30:
    Step = CSize( -1 , 1 ) ;
    Origin = CPoint( width - 2 , 1 ) ;
    break ;
  case DIR_09:
    Step = CSize( -1 , 0 ) ;
    Origin = CPoint( width - 2 , height / 2 ) ;
    break ;
  case DIR_10_30:
    Step = CSize( -1 , -1 ) ;
    Origin = CPoint( width - 2 , height - 2 ) ;
    break ;
  default:
    return false ;
  }
  return true ;
}

bool CVideoObject::MeasureEdgeAsContur( const pTVFrame ptv , CDataFrame** pDiagnostics )
{
  int iROIWidth = m_absAOS.Width() ;
  int iROIHeight = m_absAOS.Height() ;
  CPoint ROICent = m_absAOS.CenterPoint() ;

  int iMin = INT_MAX , iMax = INT_MIN ;
  if ( m_dHistThres > 0. )
  {
    FXIntArray Histogram ;
    if ( GetHistogram( ptv , Histogram , m_absAOS ) )
    {
      int dThres = ROUND( m_dHistThres * GetImageSize( ptv ) ) ;
      int iAccumFromMinus = 0 , iAccumFromPlus = 0 ;
      for ( int i = 0; i < Histogram.GetCount() ; i++ )
      {
        if ( ( iAccumFromMinus += Histogram[ i ] ) >= dThres )
        {
          iMin = i ;
          break ;
        }
      }
      for ( int i = ( int ) Histogram.GetCount() - 1 ; i >= 0 ; i-- )
      {
        if ( ( iAccumFromPlus += Histogram[ i ] ) >= dThres )
        {
          iMax = i ;
          break ;
        }
      }
    }
  }
  else
    _find_min_max( ptv , iMin , iMax , m_absAOS ) ;

  m_dMeasuredMin = iMin ;
  m_dMeasuredMax = iMax ;
  if ( iMax - iMin < m_dMinProfileContrast )
    return false ;

  int iThres ;
  if ( m_dProfileThreshold > 0.  &&  m_dProfileThreshold < 1.0 )
    iThres = iMin + ( int ) ( ( iMax - iMin ) * m_dProfileThreshold ) ;
  else if ( m_dProfileThreshold <= 0. )
    iThres = iMin + ( iMin + iMax ) / 2 ;
  else
    iThres = ROUND( m_dProfileThreshold ) ;

  cmplx cCent( ( double ) ROICent.x , ( double ) ROICent.y ) ;
  CContainerFrame * pDiagOut = ( pDiagnostics ) ? CContainerFrame::Create() : NULL ;
  if ( pDiagOut )
  {
    *pDiagnostics = pDiagOut ;
    CFigureFrame * pCentPt = CreateFigureFrame( &cCent , 1 , ( DWORD ) 0xffff00 , "Cent" ) ;
    pDiagOut->AddFrame( pCentPt ) ;
  }

  double dDir = -DegToRad( ( double ) m_iDirForEdgeContur ) ;
  cmplx cDirStep = polar( 1. , dDir ) ;
  cmplx cRightStep = GetOrthoRightOnVF( cDirStep ) ;
  double dAverSize = ( ( double ) iROIWidth + ( double ) iROIHeight ) / 2. ;
  int iNSearchSteps = ROUND( dAverSize * 2. ) ;
  cmplx cSearchStep = -cRightStep ;

  cmplx cStripCenterBegin = cCent + ( cSearchStep * ( double ) max( iROIHeight , iROIWidth ) ) ;
  CDRect dRect( m_absAOS ) ;
  straight_segment SearchLine( cStripCenterBegin , cCent ) ;
  cmplx cCrossPt ;
  if ( GetCrossOfLineAndRectangle( SearchLine , dRect , cCrossPt ) )
  {
    cStripCenterBegin = cCrossPt ;
    iNSearchSteps = ROUND( abs( cCrossPt - cCent ) ) * 2 ;
    if ( pDiagOut )
    {
      pDiagOut->AddFrame( CreateFigureFrame( &cCrossPt , 1 , ( DWORD ) 0 , "RectCross" ) ) ;
      cmplx cSecondEnd = cCrossPt - cSearchStep * ( double ) iNSearchSteps ;
      pDiagOut->AddFrame( CreateFigureFrame( &cSecondEnd , 1 , ( DWORD ) 0x00ff00 , "RectCross" ) ) ;

    }
  }
  else
    return false ;
  cmplx c1stSearchPt = cStripCenterBegin - cDirStep * ( double ) m_iEdgeFindStep * 0.5  ;
  if ( pDiagOut )
  {
    CFigureFrame * pStrip = CreateFigureFrame( &c1stSearchPt , 1 , ( DWORD ) 0xff0000 , "Strip" ) ;
    cmplx NextCorner = c1stSearchPt + cDirStep * ( double ) m_iEdgeFindStep ;
    pStrip->AddPoint( CmplxToCDPoint( NextCorner ) ) ;
    NextCorner += cRightStep * ( double ) iNSearchSteps ;
    pStrip->AddPoint( CmplxToCDPoint( NextCorner ) ) ;
    NextCorner -= cDirStep * ( double ) m_iEdgeFindStep ;
    pStrip->AddPoint( CmplxToCDPoint( NextCorner ) ) ;
    pStrip->AddPoint( CmplxToCDPoint( c1stSearchPt ) ) ;
    pDiagOut->AddFrame( pStrip ) ;

  }
#define SIGNAL_MAX_LENGTH 10000
  double pSignal[ SIGNAL_MAX_LENGTH ] ;
  //   double pStd[ SIGNAL_MAX_LENGTH ] ;
  //   double pFullAver[ SIGNAL_MAX_LENGTH ] ;
  //   double pFullStd[ SIGNAL_MAX_LENGTH ] ;

    // Get "signal" - values averaged by parallel to edge lines
  double dAverage = GetAverageSignalOnStrip(
    c1stSearchPt , cRightStep , iNSearchSteps , 5 ,
    pSignal , SIGNAL_MAX_LENGTH , ptv ) ;

  double * pdIter = pSignal ;
  double * pdEnd = pSignal + iNSearchSteps ;
  bool bInvert = ( m_Contrast == WHITE_TO_BLACK_BRD );
  if ( bInvert )
  {
    while ( pdIter < pdEnd )
    {
      if ( *pdIter < iThres )
        break ;
      pdIter++ ;
    }
  }
  else
  {
    while ( pdIter < pdEnd )
    {
      if ( *pdIter > iThres )
        break ;
      pdIter++ ;
    }
  }
  int iDist = ( int ) ( pdIter - pSignal ) ;
  if ( iDist >= iNSearchSteps - 1 )
    return false ;

  cmplx cPt = cStripCenterBegin + cRightStep * ( double ) iDist ;
  if ( pDiagOut )
  {
    CFigureFrame * p1st = CreateFigureFrame( &cPt , 1 , ( DWORD ) 0x0000ff , "1stPt" ) ;
    pDiagOut->AddFrame( p1st ) ;
  }

  CPoint FirstPt( ROUND( cPt.real() ) , ROUND( cPt.imag() ) ) ;
  CPoint Cntrst ;
  if ( !GetNegativeNeightbour( ptv , FirstPt , iThres , Cntrst ) )
  {
    cmplx cNextPt = cPt + cDirStep ;
    FirstPt.x = ROUND( cNextPt.real() ) ;
    FirstPt.y = ROUND( cNextPt.imag() ) ;
    if ( !GetNegativeNeightbour( ptv , FirstPt , iThres , Cntrst ) )
    {
      cNextPt = cPt - cDirStep ;
      FirstPt.x = ROUND( cNextPt.real() ) ;
      FirstPt.y = ROUND( cNextPt.imag() ) ;
      if ( !GetNegativeNeightbour( ptv , FirstPt , iThres , Cntrst ) )
        return false ;
    }
  }
  if ( _FindSubPixelEdgeWithOneInternalSegment( m_InternalSegment ,
    ptv , iThres , m_absAOS , FirstPt , Cntrst ) && m_InternalSegment.Count() )
  {
    // check for direction
    cmplx MainVect = m_InternalSegment.Last() - m_InternalSegment[ 0 ] ;
    cmplx cRatio = MainVect / cDirStep ;
    double dAngleDiff = arg( cRatio ) ;
    if ( ( dAngleDiff < -M_PI_2 ) || ( M_PI_2 < dAngleDiff ) )
      m_InternalSegment.Reverse() ;
    m_WhatIsMeasured |= MEASURE_EDGE_AS_CONTUR ;
    //     m_InternalSegment.Reverse() ;
    return true ;
  }

  return false ;
}

bool CVideoObject::MeasureEdgeAsConturLowResolution( const pTVFrame ptv , CDataFrame** pDiagnostics )
{
  //   int width = ptv->lpBMIH->biWidth , height = ptv->lpBMIH->biHeight;
  //   CDRect rc( 0 , width , 0 , height ) ;
  //   m_InFOVPosition.x = (double) width / 2. ;
  //   m_InFOVPosition.y = (double) height / 2. ;

  int iROIWidth = m_absAOS.Width() ;
  int iROIHeight = m_absAOS.Height() ;
  CPoint ROICent = m_absAOS.CenterPoint() ;
  CPoint Offset(m_absAOS.left, m_absAOS.top);

  int iMin = INT_MAX , iMax = INT_MIN ;
  if ( m_dHistThres > 0. )
  {
    FXIntArray Histogram ;
    if ( GetHistogram( ptv , Histogram , m_absAOS ) )
    {
      int dThres = ROUND( m_dHistThres * GetImageSize( ptv ) ) ;
      int iAccumFromMinus = 0 , iAccumFromPlus = 0 ;
      for ( int i = 0; i < Histogram.GetCount() ; i++ )
      {
        if ( ( iAccumFromMinus += Histogram[ i ] ) >= dThres )
        {
          iMin = i ;
          break ;
        }
      }
      for ( int i = ( int ) Histogram.GetCount() - 1 ; i >= 0 ; i-- )
      {
        if ( ( iAccumFromPlus += Histogram[ i ] ) >= dThres )
        {
          iMax = i ;
          break ;
        }
      }
    }
  }
  else
    _find_min_max( ptv , iMin , iMax , m_absAOS ) ;


  m_dMeasuredMin = iMin ;
  m_dMeasuredMax = iMax ;
  if ( iMax - iMin < m_dMinProfileContrast )
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , "EdgeAsConturLowResolution" , 0 ,
      "Low Contrast" ) ;
    return false ;
  }

  int iThres ;
  double dThres ;
  if ( m_dProfileThreshold > 0.  &&  m_dProfileThreshold < 1.0 )
  {
    iThres = iMin + ( int ) ( ( iMax - iMin ) * m_dProfileThreshold ) ;
    dThres = m_dProfileThreshold ;
  }
  else if ( m_dProfileThreshold <= 0. )
  {
    iThres = iMin + ( iMin + iMax ) / 2 ;
    dThres = 0. ;
  }
  else
  {
    iThres = ROUND( m_dProfileThreshold ) ;
    dThres = m_dProfileThreshold ;
  }

  cmplx cCent( ( double ) ROICent.x , ( double ) ROICent.y ) ;
  CContainerFrame * pDiagOut = ( pDiagnostics && ( m_dwViewMode & OBJ_VIEW_DET ) ) ?
    CContainerFrame::Create() : NULL ;
  if ( pDiagOut )
  {
    *pDiagnostics = pDiagOut ;
    CFigureFrame * pCentPt = CreateFigureFrame( &cCent , 1 , ( DWORD ) 0xffff00 , "Cent" ) ;
    pDiagOut->AddFrame( pCentPt ) ;
  }

  double dDir = -DegToRad( ( double ) m_iDirForEdgeContur ) ;
  cmplx cDirStep = polar( 1. , dDir ) ;
  cmplx cRightStep = GetOrthoRightOnVF( cDirStep ) ;
  double dAverSize = ( ( double ) iROIWidth + ( double ) iROIHeight ) / 2. ;
  int iNSearchSteps = ROUND( dAverSize * 2. ) ;
  cmplx cSearchStep = -cRightStep ;

  cmplx cFromCenterToBegin = ( cSearchStep * ( double ) ( max( iROIHeight , iROIWidth  )/* / 2*/ ) );
  cmplx cStripCenterBegin = cCent + cFromCenterToBegin /*+ cmplx( Offset.x , Offset.y )*/;
//   cmplx cStripCenterEnd = cCent - cFromCenterToBegin;
  CDRect dRect( m_absAOS ) ;
  straight_segment SearchLine( cStripCenterBegin , cCent ) ;
  //straight_segment SearchLine(cStripCenterBegin, cStripCenterEnd);
  cmplx cCrossPt ;
  if ( GetCrossOfLineAndRectangle( SearchLine , dRect , cCrossPt ) )
  {
    cStripCenterBegin = cCrossPt ;
    iNSearchSteps = ROUND( abs( cCrossPt - cCent ) ) * 2 ;
    if ( pDiagOut )
    {
      pDiagOut->AddFrame( CreateFigureFrame( &cCrossPt , 1 , ( DWORD ) 0 , "RectCross" ) ) ;
      cmplx cSecondEnd = cCrossPt - cSearchStep * ( double ) iNSearchSteps ;
      pDiagOut->AddFrame( CreateFigureFrame( &cSecondEnd , 1 , ( DWORD ) 0x00ff00 , "RectCross" ) ) ;

    }
  }
  else
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , "EdgeAsConturLowResolution" , 0 ,
      "Initial Edge is not found" ) ;
    return false ;
  }
  cmplx c1stSearchPt = cStripCenterBegin - cDirStep * ( double ) m_iEdgeFindStep * 0.5  ;
  //   if ( pDiagOut )
  //   {
  //     CFigureFrame * pStrip = CreateFigureFrame( &c1stSearchPt , 1 , (DWORD) 0xff0000 , "Strip" ) ;
  //     cmplx NextCorner = c1stSearchPt + cDirStep * (double) m_iEdgeFindStep ;
  //     pStrip->AddPoint( CmplxToCDPoint( NextCorner ) ) ;
  //     NextCorner += cRightStep * (double) iNSearchSteps ;
  //     pStrip->AddPoint( CmplxToCDPoint( NextCorner ) ) ;
  //     NextCorner -= cDirStep * (double) m_iEdgeFindStep ;
  //     pStrip->AddPoint( CmplxToCDPoint( NextCorner ) ) ;
  //     pStrip->AddPoint( CmplxToCDPoint( c1stSearchPt ) ) ;
  //     pDiagOut->AddFrame( pStrip ) ;
  //   }
#define SIGNAL0_MAX_LENGTH 10000
  double pSignal[ SIGNAL0_MAX_LENGTH ] ;

  // Get "signal" - values averaged by parallel to edge lines
//   double dAverage = GetAverageSignalOnStrip(
//     c1stSearchPt , cRightStep , iNSearchSteps , m_iEdgeFindStep ,
//     pSignal , SIGNAL_MAX_LENGTH , ptv ) ;
// 
//   double dPos = find_border_forw( pSignal , iNSearchSteps , dThres != 0. ? dThres : dAverage ) ;
  double dPos = GetCrossPosOnStripWithView(
    c1stSearchPt , cRightStep , cDirStep , iNSearchSteps , m_iEdgeFindStep ,
    dThres , pSignal , SIGNAL0_MAX_LENGTH , ptv , pDiagOut ) ;
  if ( dPos == 0. )
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , "EdgeAsConturLowResolution" , 0 ,
      "Initial Edge is not found" ) ;
    return false ;
  }

  cmplx cPt = cStripCenterBegin + cRightStep * dPos ;
  m_InternalSegment.RemoveAll() ;
  int iMaxLen = ( iROIWidth + iROIHeight ) * 2 ; // max edge length

  m_InternalSegment.Add( cPt ) ; // Point ~in middle of segment
  if ( pDiagOut )
  {
    CFigureFrame * p1st = CreateFigureFrame( &cPt , 1 , ( DWORD ) 0x0000ff , "1stPt" ) ;
    pDiagOut->AddFrame( p1st ) ;
  }

  // First of all, we are going to reverse direction up to ROI(m_asbAOS) end
  cmplx cDirIter = -cDirStep ;
  cStripCenterBegin = cPt + ( cDirIter * ( double ) m_iEdgeFindStep ) - ( cRightStep * ( double ) m_iEdgeFindStep * 2. ) ;
  c1stSearchPt = cStripCenterBegin - ( cDirIter * ( double ) m_iEdgeFindStep * 0.5 ) ;
  iNSearchSteps = m_iEdgeFindStep * 4 ;
  cmplx cOldDirIter = cDirIter ;
  cmplx cOldRightStep = cRightStep ;
  cmplx cOld1stPt = c1stSearchPt ;
  cmplx cOldStripCenterBegin = cStripCenterBegin ;
  CDRect cTVAllowedRect( iNSearchSteps , GetWidth( ptv ) - iNSearchSteps ,
    iNSearchSteps , GetHeight( ptv ) - iNSearchSteps ) ;
  double dAngle_m1 = 0. , dAngle_m2 = 0. ;

  do // cycle of going to back direction 
  {
    cmplx cPrevPt = cPt ;
    double dPos = GetCrossPosOnStripWithView(
      c1stSearchPt , cRightStep , cDirIter , iNSearchSteps , m_iEdgeFindStep ,
      dThres , pSignal , SIGNAL_MAX_LENGTH , ptv , pDiagOut ) ;
    if ( dPos == 0. )
    {
      cmplx cShiftFromOld = ( cOldDirIter - cOldRightStep ) * ( double ) iNSearchSteps * 0.5 ;
      cmplx cExtended1st = cOld1stPt + cShiftFromOld ;
      dPos = GetCrossPosOnStripWithView(
        cExtended1st , cOldRightStep , cOldDirIter , iNSearchSteps * 2 , m_iEdgeFindStep ,
        dThres , pSignal , SIGNAL_MAX_LENGTH , ptv , pDiagOut ) ;
      if ( dPos == 0. )
      {
        FxSendLogMsg( MSG_ERROR_LEVEL , "EdgeAsConturLowResolution" , 0 ,
          "Edge is not found on back direction" ) ;
        return false ;
      }
      else
      {
        cPt = cOldStripCenterBegin + cShiftFromOld + cRightStep * dPos ;
      }
    }
    else
      cPt = cStripCenterBegin + cRightStep * dPos ;

    CPoint IntPt( ROUND( cPt.real() ) , ROUND( cPt.imag() ) ) ;
    if ( m_absAOS.PtInRect( IntPt ) ) // main check for cycle finishing
    {
      m_InternalSegment.Add( cPt ) ; // Point ~in middle of segment
      if ( pDiagOut )
      {
        CFigureFrame * pPtOnEdgeBack = CreateFigureFrame( &cPt , 1 , ( DWORD ) 0x00ffff , "PtOnEdge" ) ;
        pDiagOut->AddFrame( pPtOnEdgeBack ) ;
      }

      cmplx cStepToNextPt = ( cPt - cPrevPt ) ;
      cmplx cNormStepToNextPt = GetNormalized( cStepToNextPt ) ;
      cOldDirIter = cDirIter ;
      cOldRightStep = cRightStep ;
      cOld1stPt = c1stSearchPt ;
      cOldStripCenterBegin = cStripCenterBegin ;

      double dAngle = arg( cNormStepToNextPt / cDirIter ) ;

      if ( m_InternalSegment.Count() <= 1
        || ( fabs( dAngle ) < M_PI / 9. )
        || ( fabs( dAngle_m1 - dAngle ) < M_PI / 9 ) )
      {
        cDirIter = cNormStepToNextPt ;
        cRightStep = GetOrthoLeftOnVF( cDirIter ) ; // Left because we are going to opposite direction
      }
      if ( m_InternalSegment.Count() >= 2 )
      {
        if ( m_InternalSegment.Count() >= 3 )
          dAngle_m2 = dAngle_m1 ;
        dAngle_m1 = dAngle ;
      }

      cStripCenterBegin = cPt + ( cDirIter * ( double ) m_iEdgeFindStep ) - ( cRightStep * ( double ) m_iEdgeFindStep * 2. ) ;
      c1stSearchPt = cStripCenterBegin - ( cDirIter * ( double ) m_iEdgeFindStep * 0.5 ) ;
    }
    else
      break ;
    // Condition is protection against too complicate edge
    if ( m_InternalSegment.size() >= iMaxLen )
    {
      m_InternalSegment.RemoveAll() ;
      break ;
    }
  } while ( 1 ) ;

  if ( m_InternalSegment.size() > 2 )
  {
    cPt = m_InternalSegment[ 0 ] ;
    cmplx cPrevPt = m_InternalSegment[ 1 ] ;
    cmplx cStepToNextPt = ( cPt - cPrevPt ) ;
    cDirIter = GetNormalized( cStepToNextPt ) ;

    m_InternalSegment.Reverse() ;
    int iNAfterCenter = 0 ;
    // Go to forward direction
    cOldDirIter = cDirIter ;
    cOldRightStep = cRightStep = GetOrthoRightOnVF( cDirIter ) ;
    cOldStripCenterBegin = cStripCenterBegin =
      cPt + ( cDirIter * ( double ) m_iEdgeFindStep ) - ( cRightStep * ( double ) m_iEdgeFindStep * 2. ) ;
    cOld1stPt = c1stSearchPt = cStripCenterBegin - ( cDirIter * ( double ) m_iEdgeFindStep * 0.5 ) ;
    iNSearchSteps = m_iEdgeFindStep * 4 ;
    bool bExtention = false ;
    cmplx cLastPtInside ;
    do  // cycle of going to forward direction 
    {
      cPrevPt = cPt ;
      double dPos = GetCrossPosOnStripWithView(
        c1stSearchPt , cRightStep , cDirIter , iNSearchSteps , m_iEdgeFindStep ,
        dThres , pSignal , SIGNAL_MAX_LENGTH , ptv , pDiagOut ) ;
      if ( dPos == 0. )
      {
        cmplx cShiftFromOld = ( cOldDirIter - cOldRightStep ) * ( double ) iNSearchSteps * 0.5 ;
        cmplx cExtended1st = cOld1stPt + cShiftFromOld ;
        dPos = GetCrossPosOnStripWithView(
          cExtended1st , cOldRightStep , cOldDirIter , iNSearchSteps * 2 , m_iEdgeFindStep ,
          dThres , pSignal , SIGNAL_MAX_LENGTH , ptv , pDiagOut ) ;
        if ( dPos == 0. )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "EdgeAsConturLowResolution" , 0 ,
            "Edge is not found on back direction" ) ;
          return false ;
        }
        else
          cPt = cOldStripCenterBegin + cShiftFromOld + cRightStep * dPos ;
      }
      else
        cPt = cStripCenterBegin + cRightStep * dPos ;

      CPoint IntPt( ROUND( cPt.real() ) , ROUND( cPt.imag() ) ) ;
      if ( !bExtention && m_absAOS.PtInRect( IntPt ) )
      {
        m_InternalSegment.Add( cPt ) ; // 
        cLastPtInside = cPt ;
        if ( pDiagOut )
        {
          CFigureFrame * ptOnEdgeForward = CreateFigureFrame( &cPt , 1 , ( DWORD ) 0x00ffff , "PtOnEdge" ) ;
          pDiagOut->AddFrame( ptOnEdgeForward ) ;
        }
      }
      else
        bExtention = true ;
      if ( bExtention )
      {
        cmplx cPtToLastInside = cPt.real() - cLastPtInside ;
        double dPartNorm = m_iExtendForward_perc / 100. ;
        if ( ( abs( cPtToLastInside.real() ) > iROIWidth * dPartNorm )
          || ( abs( cPtToLastInside.imag() ) > ( iROIHeight * dPartNorm ) )
          || !cTVAllowedRect.IsPtInside( *( ( CDPoint* ) ( &cPt ) ) ) )
        {
          break ;
        }
        m_InternalSegment.Add( cPt ) ; // 
        if ( pDiagOut )
        {
          CFigureFrame * ptOnEdgeForward = CreateFigureFrame( &cPt , 1 , ( DWORD ) 0xff00ff , "PtOnEdge" ) ;
          pDiagOut->AddFrame( ptOnEdgeForward ) ;
        }
      }
      cStepToNextPt = ( cPt - cPrevPt ) ;
      cmplx cNormStepToNextPt = GetNormalized( cStepToNextPt ) ;
      cOldDirIter = cDirIter ;
      cOldRightStep = cRightStep ;
      cOld1stPt = c1stSearchPt ;
      cOldStripCenterBegin = cStripCenterBegin ;
      double dAngle = arg( cNormStepToNextPt / cDirIter ) ;

      if ( ++iNAfterCenter <= 1
        || ( fabs( dAngle ) < M_PI / 9. )
        || ( fabs( dAngle_m1 - dAngle ) < M_PI / 9 ) )
      {
        //cDirIter = GetNormalized( (cDirIter + cNormStepToNextPt) / 2.0 ) ;
        cDirIter = cNormStepToNextPt ;
        cRightStep = GetOrthoRightOnVF( cDirIter ) ; // NOT LEFT because we are going to straight direction
      }
      if ( iNAfterCenter >= 1 )
      {
        if ( iNAfterCenter >= 2 )
          dAngle_m2 = dAngle_m1 ;
        dAngle_m1 = dAngle ;
      }
      cStripCenterBegin = cPt + ( cDirIter * ( double ) m_iEdgeFindStep ) - ( cRightStep * ( double ) m_iEdgeFindStep * 2. ) ;
      c1stSearchPt = cStripCenterBegin - ( cDirIter * ( double ) m_iEdgeFindStep * 0.5 ) ;
      if ( m_InternalSegment.size() >= iMaxLen )
      {
        m_InternalSegment.RemoveAll() ;
        break ;
      }
    } while ( 1 ) ;

    if ( m_InternalSegment.Count() )
    {
      m_WhatIsMeasured |= MEASURE_EDGE_AS_CONTUR ;
      //     m_InternalSegment.Reverse() ;
      return true ;
    }
  }
  else
    m_InternalSegment.RemoveAll() ;
  return false ;
}

bool CVideoObject::MeasureEdge( const CVideoFrame* vf )
{

  if ( m_WhatToMeasure & MEASURE_EDGE_AS_CONTUR )
  {
    // Do profiles (only by request for seeing)
    // Insert there

//     return MeasureEdgeAsContur( (pTVFrame) vf , &m_pDiagnostics ) ;
    return MeasureEdgeAsConturLowResolution( ( pTVFrame ) vf , &m_pDiagnostics ) ;
  }
  else
  {
    pTVFrame ptv = _cut_rect( vf , m_absAOS , FALSE );
    if ( !ptv )
      return false;
    int width = ptv->lpBMIH->biWidth , height = ptv->lpBMIH->biHeight;

    //  pTVFrame pEdge = makecopyTVFrame2(ptv);
    //  _calc_diff_image( ptv ) ;
    CDRect rc( 0 , width , 0 , height ) ;
    m_InFOVPosition.x = ( double ) width / 2. ;
    m_InFOVPosition.y = ( double ) height / 2. ;
    DWORD dir = m_Direction ;
    m_HProfile.Realloc( width ) ;
    m_VProfile.Realloc( height ) ;
    m_LineResults.RemoveAll() ;
    if ( !m_bMulti ) // find and measure first border
    {
      if ( seekEdges( ptv , rc , &m_HProfile , &m_VProfile ,
        !( m_Contrast == BLACK_TO_WHITE_BRD ) , dir ,
        m_dMinProfileContrast , m_dProfileThreshold , m_iDebouncing_pix ) )
      {
        switch ( m_Direction )
        {
        case DIR_LR:
          if ( rc.left > 0.1 )
          {
            m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_XPROFILE ;
            m_InFOVPosition.x = rc.left ;
            m_dMeasuredMin = m_HProfile.m_dMinValue ;
            m_dMeasuredMax = m_HProfile.m_dMaxValue ;
          }
          break ;
        case DIR_RL:
          if ( rc.right > 0.1 )
          {
            m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_XPROFILE ;
            m_InFOVPosition.x = rc.right ;
            m_dMeasuredMin = m_HProfile.m_dMinValue ;
            m_dMeasuredMax = m_HProfile.m_dMaxValue ;
          }
          break;
        case DIR_UD:
          if ( rc.top > 0.1 )
          {
            m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_YPROFILE ;
            m_InFOVPosition.y = rc.top ;
            m_dMeasuredMin = m_VProfile.m_dMinValue ;
            m_dMeasuredMax = m_VProfile.m_dMaxValue ;
          }
          break ;
        case DIR_DU:
          if ( rc.bottom > 0.1 )
          {
            m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_YPROFILE ;
            m_InFOVPosition.y = rc.bottom ;
            m_dMeasuredMin = m_VProfile.m_dMinValue ;
            m_dMeasuredMax = m_VProfile.m_dMaxValue ;
          }
          break;
        }
        m_InFOVPosition.x += m_absAOS.left ;
        m_InFOVPosition.y += m_absAOS.top ;
        CLineResult LRes ;
        LRes.m_Center = m_InFOVPosition ;
        LRes.m_DRect = rc ;

        m_LineResults.Add( LRes ) ;
      }
    }
    else  // find all borders in FOV
    {
      CDRectArray Lines ;
      DWORD dwContrast = 2 ; // default - any border
      switch ( m_Contrast )
      {
      case WHITE_TO_BLACK_BRD:
        dwContrast = 1 ; break ;
      case BLACK_TO_WHITE_BRD:
        dwContrast = 0 ; break ;
      default: break ;
      }
      if ( seekMultiBorders( ptv , Lines , ( double* ) &m_HProfile , ( double* ) &m_VProfile ,
        dwContrast , dir ,
        m_dMinProfileContrast , m_dProfileThreshold , m_iDebouncing_pix ) )
      {
        bool bVert = ( dir & VERTICAL ) || ( dir == ANY_DIRECTION )
          || ( dir == DIR_LR ) || ( dir == DIR_RL ) ;
        bool bHoriz = ( dir & HORIZONTAL ) || ( dir == ANY_DIRECTION )
          || ( dir == DIR_UD ) || ( dir == DIR_DU ) ;

        for ( int i = 0 ; i < Lines.GetCount() ; i++ )
        {
          CLineResult LRes ;
          bool bIsOK = false ;

          if ( bHoriz )
          {
            Lines[ i ].left = m_absAOS.left ;
            Lines[ i ].right = m_absAOS.right ;
            LRes.m_Center.x = m_absAOS.CenterPoint().x ;
          }
          if ( bVert )
          {
            Lines[ i ].top = m_absAOS.top ;
            Lines[ i ].bottom = m_absAOS.bottom ;
            LRes.m_Center.y = m_absAOS.CenterPoint().y ;
          }

          CDRect rc( Lines[ i ] ) ;
          if ( bVert )
          {
            if ( dir == DIR_RL )
            {
              if ( (rc.right > 1.) && (rc.right <( m_absAOS.Width() - 1.)) )
              {
                m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_XPROFILE ;
                LRes.m_Center.x = rc.right + m_absAOS.left ;
                bIsOK = true ;
              }
            }
            else if ( rc.left > 2.  )
            {
              m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_XPROFILE ;
              LRes.m_Center.x = rc.left + m_absAOS.left ;
              bIsOK = true ;
            }
          }

          if ( bHoriz )
          {
            if ( dir == DIR_DU )
            {
              if ( rc.bottom > 1. )
              {
                m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_YPROFILE ;
                LRes.m_Center.y = rc.bottom + m_absAOS.top ;
                bIsOK = true ;
              }
            }
            else if ( ( rc.top > 1. ) && ( ( m_absAOS.Height() - rc.top ) > 2.0 ) )
            {
              m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_YPROFILE ;
              LRes.m_Center.y = rc.top + m_absAOS.top ;
              bIsOK = true ;
            }
          }
          
          if ( bIsOK )
          {
            LRes.m_DRect = rc ;
            LRes.m_iIndex = ( int ) m_LineResults.GetCount() ;
            m_LineResults.Add( LRes ) ;
          }
        }
      }
    }
    m_WhatIsMeasured |= MEASURE_PROFILE ;
    freeTVFrame( ptv ); /*freeTVFrame(pEdge);*/
  }

  return ( ( m_WhatIsMeasured & ~MEASURE_POSITION ) != 0 );
}

bool  CVideoObject::DoMeasure( const CVideoFrame* vf )
{
  m_Timing.clear() ;
  DWORD dwCompr = GetCompression( vf ) ;
  bool bY8 = ( dwCompr == BI_YUV9 )
    || ( dwCompr == BI_Y8 )
    || ( dwCompr == BI_Y800 )
    || ( dwCompr == BI_YUV12 ) ;
  if ( !bY8 && ( dwCompr != BI_Y16 ) )
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , "CVideoObject::DoMeasure" , 0 ,
      "Not supported image format %s (0x%08X) " ,
      GetVideoFormatName( dwCompr ) , dwCompr ) ;
    return false ;
  }

  m_WhatIsMeasured = 0;
  m_dStartTime = GetHRTickCount() ;
  m_Status.Empty() ;

  m_EndOfInitialProcessing.RemoveAll();
  m_EndSubpixelEdgeFind.RemoveAll();
  m_EndOfFinalProcessing.RemoveAll();

  switch ( m_Type )
  {
  case SPOT:
  {
    this->m_pOriginalFrame = ( CVideoFrame* ) vf ;
    bool bRes = MeasureSpot( vf );
    this->m_pOriginalFrame = NULL ;
    return bRes ;
  }
  case LINE_SEGMENT:
    return MeasureLine( vf );
  case OCR:
    return MeasureText( vf );
  case EDGE:
    return MeasureEdge( vf );
  default:
    SENDERR_1( "Measurement for object '%s' is not implemeted yet" , obj2name( m_Type ) );
  }
  return false;
}

bool CVideoObject::PrintVideoObject( FXString& AsText )
{
  return PrintVideoObject( *this , AsText ) ;
}

bool CVideoObject::PrintVideoObject(
  CVideoObject &vo , FXString& AsText )
{
  return vo.PrintProperties( AsText ) ;
}

bool CVideoObject::FillDataAboutLeaders( FXPropKit2& pk , bool * pbInvalidate )
{
  FXParser2 PosAsText ;
  if ( pk.GetString( "pos" , PosAsText ) ) // old style
  {
    m_LeaderIndex.x = m_LeaderIndex.y = -1;
    m_LeaderName.Empty() ;
    m_LeaderNameY.Empty() ;
    m_AOS.SetRectEmpty();
    FXParser2 temp;
    int pos = 0;
    if ( PosAsText.FindElement( "abs" , temp ) )
    {
      FXString val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.left = atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.top = atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.right = m_AOS.left + atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.bottom = m_AOS.top + atoi( val );
      //      TRACE("%s\n",temp);
    }
    else if ( PosAsText.FindElement( "relxy" , temp ) )
    {
      m_Placement = PLACE_REL_XY ;
      FXString val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_LeaderName = val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_LeaderNameY = val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.left = atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.top = atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.right = m_AOS.left + atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.bottom = m_AOS.top + atoi( val );
      m_bLeaderChanged = true ;
    }
    else if ( PosAsText.FindElement( "rel" , temp ) )
    {
      m_Placement = PLACE_REL ;
      FXString val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_LeaderName = m_LeaderNameY = val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.left = atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.top = atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.right = m_AOS.left + atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.bottom = m_AOS.top + atoi( val );
      m_bLeaderChanged = true ;
    }
    else if ( PosAsText.FindElement( "copy_rel" , temp ) )
    {
      m_Placement = PLACE_COPY_REL ;
      FXString val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_LeaderName = m_LeaderNameY = val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.left = atoi( val );
      if ( temp.GetParam( pos , val , ',' ) )
        m_AOS.top = atoi( val );
//       if ( temp.GetParam( pos , val , ',' ) )
//         m_AOS.right = m_AOS.left + atoi( val );
//       if ( temp.GetParam( pos , val , ',' ) )
//         m_AOS.bottom = m_AOS.top + atoi( val );
      m_bLeaderChanged = true ;
    }
  }
  else
  {
    CRect AOS ;
    CPoint LeaderAOSCenter ;

    int LeaderStatus = GetPlacementAndLeaderOffset( AOS , LeaderAOSCenter ) ;
    bool bRelDataReady = LeaderStatus > 1 ;

    VOBJ_PLACEMENT NewPlacement ;
    if ( pk.GetInt( "placement" , ( int& ) NewPlacement ) )
    {
      if ( pbInvalidate )
        *pbInvalidate = true ;
      if ( NewPlacement != m_Placement )
      {
        switch ( NewPlacement )
        {
        case PLACE_ABS: // it means, that was not ABS
        {
          if ( LeaderStatus )
            m_AOS = AOS ;
          m_LeaderIndex.x = m_LeaderIndex.y = -1 ;
        }
        break ;
        case PLACE_REL: // was or ABS or REL_XY
        {
          switch ( m_Placement )
          {
          case PLACE_ABS:
            if ( bRelDataReady )
            {
              AOS.left -= LeaderAOSCenter.x ;
              AOS.top -= LeaderAOSCenter.y ;
              m_AOS = AOS ;
            }
            break ;
          case PLACE_REL_XY:
            if ( bRelDataReady )
            {
              AOS.left -= LeaderAOSCenter.x ;
              AOS.top -= LeaderAOSCenter.y ;
              m_AOS = AOS ;
            }
            m_LeaderIndex.y = m_LeaderIndex.x ;
            break ;
          }
        }
        break ;
        case PLACE_REL_XY: // was or ABS or REL
        {
          if ( bRelDataReady )
          {
            AOS.left -= LeaderAOSCenter.x ;
            AOS.top -= LeaderAOSCenter.y ;
            m_AOS = AOS ;
          }
        }
        break ;
        case PLACE_COPY_REL:
        {
          switch ( m_Placement )
          {
          case PLACE_ABS:
            if ( bRelDataReady )
            {
              AOS.left -= LeaderAOSCenter.x ;
              AOS.top -= LeaderAOSCenter.y ;
              m_AOS = AOS ;
            }
            break ;
          case PLACE_REL_XY:
            if ( bRelDataReady )
            {
              AOS.left -= LeaderAOSCenter.x ;
              AOS.top -= LeaderAOSCenter.y ;
              m_AOS = AOS ;
            }
            m_LeaderIndex.y = m_LeaderIndex.x ;
            break ;
          }

        }
        break ;

        }
      }
      m_Placement = NewPlacement ;
    }

    switch ( m_Placement )
    {
    case PLACE_ABS: return true ;
    case PLACE_REL:
    case PLACE_REL_XY:
    case PLACE_COPY_REL: break ;
    default:
      SENDERR( "Strange placement %d for VO '%s'" ,
        m_Placement , ( LPCTSTR ) m_ObjectName );
      return false ;
    }
    int iLeader = -1 , iLeaderY = -1 ;
    FXString RelTo ;
    if ( pk.GetString( "relativeto" , RelTo ) )
    {
      if ( isdigit( RelTo[ 0 ] ) )
      {
        m_LeaderIndex.x = atoi( RelTo ) ;
        if ( m_pVObjects && m_pVOArrayLock )
        {
          FXAutolock al( *m_pVOArrayLock , "CVideoObject::FillDataAboutLeaders-1" ) ;
          if ( ( m_LeaderIndex.x >= 0 ) && ( m_LeaderIndex.x < m_pVObjects->GetCount() ) )
            m_LeaderName = m_pVObjects->GetAt( m_LeaderIndex.x ).m_ObjectName ; //m_RelativeToNames[ m_LeaderIndex.x ] ;
          else
            m_LeaderIndex.x = -1 ;
        }
      }
      else if ( m_pVObjects && m_pVOArrayLock )
      {
        FXAutolock al( *m_pVOArrayLock , "CVideoObject::FillDataAboutLeaders-2" ) ;
        int i = 0 ;
        for ( ; i < m_pVObjects->Count() ; i++ )
        {
          if ( m_pVObjects->GetAt( i ).m_ObjectName.CompareNoCase( RelTo ) == 0 )
          {
            iLeader = m_LeaderIndex.x = i ;
            m_LeaderName = RelTo ;
          }
        }
        if ( i >= m_pVObjects->Count() )
          m_LeaderIndex.x = -1 ;
      }
      if ( m_Placement == PLACE_REL )
      {
        m_LeaderIndex.y = m_LeaderIndex.x ;
        m_LeaderNameY = m_LeaderName ;
      }
      m_bLeaderChanged = true ;
    }
    if ( pk.GetString( "relativetoy" , RelTo ) )
    {
      if ( isdigit( RelTo[ 0 ] ) )
      {
        m_LeaderIndex.y = atoi( RelTo ) ;
        if ( m_pVObjects && m_pVOArrayLock )
        {
          FXAutolock al( *m_pVOArrayLock , "CVideoObject::FillDataAboutLeaders-3" ) ;
          if ( ( m_LeaderIndex.y >= 0 ) && ( m_LeaderIndex.x < m_pVObjects->GetCount() ) )
            m_LeaderNameY = m_pVObjects->GetAt( m_LeaderIndex.y ).m_ObjectName ; //m_RelativeToNames[ m_LeaderIndex.x ] ;
          else
            m_LeaderIndex.y = -1 ;
        }
      }
      else if ( m_pVObjects && m_pVOArrayLock )
      {
        FXAutolock al( *m_pVOArrayLock , "CVideoObject::FillDataAboutLeaders-4" ) ;
        int i = 0 ;
        for ( ; i < m_pVObjects->Count() ; i++ )
        {
          if ( m_pVObjects->GetAt( i ).m_ObjectName.CompareNoCase( RelTo ) == 0 )
          {
            m_LeaderIndex.y = i ;
            m_LeaderNameY = RelTo ;
          }
        }
        if ( i >= m_pVObjects->Count() )
          m_LeaderIndex.y = -1 ;
      }
      m_bLeaderChanged = true ;
    }
  }
  if ( m_bLeaderChanged )
  {
    CPoint LeaderAOSCenter ;
    if ( GetPlacementAndLeaderOffset( m_AOS , LeaderAOSCenter ) == 2 )
    {
      m_AOS.left -= LeaderAOSCenter.x ;
      m_AOS.top -= LeaderAOSCenter.y ;
    }
    if ( pbInvalidate )
      *pbInvalidate = true ;
  }
  return m_bLeaderChanged ;
}

bool CVideoObject::InitVideoObject(
  LPCTSTR key , LPCTSTR param , bool * pInvalidate )
{
  FXParser2 tmpS;
  FXPropKit2 pk = param;

  if ( pk.GetString( "name" , m_ObjectName )
    && m_ObjectName.IsEmpty() )
  {
    SENDERR_1( "Undefined name in template description of a key '%s'" , key );
  } ;
  if ( pk.GetString( "dir" , tmpS ) )
  {
    tmpS = tmpS.Trim() ;
    if ( tmpS.CompareNoCase( "horizontal" ) == 0 )
      m_Direction = HORIZONTAL;
    else if ( tmpS.CompareNoCase( "vertical" ) == 0 )
      m_Direction = VERTICAL;
    else if ( ( tmpS.CompareNoCase( "00" ) == 0 ) || ( tmpS.CompareNoCase( "0" ) == 0 )
      || ( tmpS.CompareNoCase( "12" ) == 0 ) )
      m_Direction = DIR_00;
    else if ( ( tmpS.CompareNoCase( "03" ) == 0 ) || ( tmpS.CompareNoCase( "3" ) == 0 ) )
      m_Direction = DIR_03;
    else if ( ( tmpS.CompareNoCase( "06" ) == 0 ) || ( tmpS.CompareNoCase( "6" ) == 0 ) )
      m_Direction = DIR_06;
    else if ( ( tmpS.CompareNoCase( "09" ) == 0 ) || ( tmpS.CompareNoCase( "9" ) == 0 ) )
      m_Direction = DIR_09;
    else if ( tmpS.CompareNoCase( "any" ) == 0 )
      m_Direction = ANY_DIRECTION;
    else if ( tmpS.CompareNoCase( "vlr" ) == 0 )
      m_Direction = DIR_LR;
    else if ( tmpS.CompareNoCase( "vrl" ) == 0 )
      m_Direction = DIR_RL;
    else if ( tmpS.CompareNoCase( "hud" ) == 0 )
      m_Direction = DIR_UD;
    else if ( tmpS.CompareNoCase( "hdu" ) == 0 )
      m_Direction = DIR_DU;
    else if ( isdigit( tmpS[ 0 ] ) )
      m_Direction = ( VOBJ_DIR ) atoi( tmpS ) ;
    else
    {
      SENDERR_2( "Unknown direction '%s' in template description of a key '%s'" , tmpS , key );
    }

  }
  else
  {
    pk.GetInt( "dir_degrees" , m_iDirForEdgeContur ) ;
    pk.GetInt( "find_step" , m_iEdgeFindStep ) ;
  }

  if ( pk.GetInt( "dir" , ( int& ) m_Direction )
    && ( _tcscmp( key , _T( "ocr" ) ) == 0 ) )
  {
    switch ( m_Direction )
    {
    case 0: m_Direction = DIR_00 ; break ;
    case 3: m_Direction = DIR_03 ; break ;
    case 6: m_Direction = DIR_06 ; break ;
    case 9: m_Direction = DIR_09 ; break ;
    }
  }
  FXString ContrastAsString ;
  if ( pk.GetString( "contrast" , ContrastAsString ) )
  {
    ContrastAsString = ContrastAsString.Trim() ;
    if ( isdigit( ContrastAsString[ 0 ] ) )
      m_Contrast = ( VOBJ_CONTRAST ) atoi( ContrastAsString ) ;
    else
    {
      if ( ContrastAsString.CompareNoCase( "white_on_black" ) == 0 )
        m_Contrast = WHITE_ON_BLACK;
      else if ( ContrastAsString.CompareNoCase( "black_on_white" ) == 0 )
        m_Contrast = BLACK_ON_WHITE;
      else if ( ContrastAsString.CompareNoCase( "any" ) == 0 )
        m_Contrast = ANY_CONTRAST;
      else if ( ContrastAsString.CompareNoCase( "white_to_black_brd" ) == 0 )
        m_Contrast = WHITE_TO_BLACK_BRD;
      else if ( ContrastAsString.CompareNoCase( "black_to_white_brd" ) == 0 )
        m_Contrast = BLACK_TO_WHITE_BRD;
      else
        m_Contrast = ANY_CONTRAST;
    }
  }

  FillDataAboutLeaders( pk , pInvalidate ) ;
  bool bNewStyle = false ;
  if ( !m_bExternalModification || (m_Placement == PLACE_ABS) )
  {
    bNewStyle = pk.GetInt( "xoffset" , m_AOS.left ) ;
    bNewStyle |= pk.GetInt( "yoffset" , m_AOS.top ) ;
    bNewStyle |= pk.GetInt( "width" , m_AOS.right ) ;
    bNewStyle |= pk.GetInt( "height" , m_AOS.bottom ) ;
  }
  if ( !bNewStyle && !m_bExternalModification 
    &&  pk.GetString( "pos" , tmpS ) )
  {
    // Old placement style
    FXParser2 temp;
    int pos = 0;
    if ( tmpS.FindElement( "abs" , temp ) )
    {
      m_LeaderIndex.x = m_LeaderIndex.y = -1;
      m_LeaderName.Empty() ;
      m_LeaderNameY.Empty() ;
      m_AOS.SetRectEmpty();
      FXString val;
      if ( temp.GetParam( pos , val , ',' ) )
      {
        m_AOS.left = atoi( val );
        if ( m_AOS.left > 1000000 )
        {
          m_bForROIViewOnly = true ;
          m_dROIViewScale = ( double ) ( m_AOS.left / 1000000 ) ;
          m_AOS.left = m_AOS.left % 1000000 ;
          m_RectForROIViewOnly.left = ( ( double ) m_AOS.left ) / m_dROIViewScale ;
        }
      }
      if ( temp.GetParam( pos , val , ',' ) )
      {
        m_AOS.top = atoi( val );
        if ( m_bForROIViewOnly )
        {
          m_AOS.top %= 1000000 ;
          m_RectForROIViewOnly.top = ( ( double ) m_AOS.top ) / m_dROIViewScale ;
        }
      }
      if ( temp.GetParam( pos , val , ',' ) )
      {
        m_AOS.right = /*m_AOS.left +*/ atoi( val );
        if ( m_bForROIViewOnly )
        {
          m_AOS.right %= 1000000 ;
          m_RectForROIViewOnly.right = ( ( double ) m_AOS.right ) / m_dROIViewScale ;
        }
      }
      if ( temp.GetParam( pos , val , ',' ) )
      {
        m_AOS.bottom = /*m_AOS.top + */atoi( val );
        if ( m_bForROIViewOnly )
        {
          m_AOS.bottom %= 1000000 ;
          m_RectForROIViewOnly.bottom = ( ( double ) m_AOS.bottom ) / m_dROIViewScale ;
        }
      }

      //      TRACE("%s\n",temp);
    }
    else if ( tmpS.FindElement( "relxy" , temp ) )
    {
      m_AOS.SetRectEmpty();
      FXString val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_LeaderName = val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_LeaderNameY = val;
      FillRect( m_AOS , temp , pos ) ;
    }
    else if ( tmpS.FindElement( "rel" , temp ) )
    {
      m_AOS.SetRectEmpty();
      FXString val;
      if ( temp.GetParam( pos , val , ',' ) )
        m_LeaderName = m_LeaderNameY = val;
      FillRect( m_AOS , temp , pos ) ;
    }
  }

  if ( m_Type == SPOT )
  {
    bNewStyle = pk.GetInt( "width_min" , m_ExpectedSize.left ) ;
    bNewStyle |= pk.GetInt( "width_max" , m_ExpectedSize.right ) ;
    bNewStyle |= pk.GetInt( "height_min" , m_ExpectedSize.top ) ;
    bNewStyle |= pk.GetInt( "height_max" , m_ExpectedSize.bottom ) ;
    if ( !bNewStyle && pk.GetString( "size" , tmpS ) ) // old style
    {
      FXString val;
      FXSIZE pos = 0;
      if ( tmpS[ pos ] == '(' ) pos++;
      if ( tmpS.GetWord( pos , val ) )
      {
        m_ExpectedSize.left = atoi( val );
        if ( tmpS.GetWord( pos , val ) )
        {
          m_ExpectedSize.top = atoi( val );
          {
            if ( tmpS.GetWord( pos , val ) )
              m_ExpectedSize.right = atoi( val );
            if ( tmpS.GetWord( pos , val ) )
              m_ExpectedSize.bottom = atoi( val );
            CRect R = m_ExpectedSize ;
            if ( !R.right )
            {
              R.right = m_ExpectedSize.right = ( R.left * 5 ) / 4 ;
              R.left = m_ExpectedSize.left = ( R.left * 3 ) / 4 ;
            }
            if ( !R.bottom )
            {
              R.bottom = m_ExpectedSize.bottom = ( R.top * 5 ) / 4 ;
              R.top = m_ExpectedSize.top = ( R.top * 3 ) / 4 ;
            }
          }
        }
      }
    }
    bNewStyle = pk.GetInt( "area_min" , m_iAreaMin ) ;
    bNewStyle |= pk.GetInt( "area_max" , m_iAreaMax ) ;
    if ( pk.GetString( "area" , tmpS ) )
    {
      FXString val;
      FXSIZE pos = 0;
      if ( tmpS[ pos ] == '(' ) pos++;
      if ( tmpS.GetWord( pos , val ) )
        m_iAreaMin = atoi( val );
      if ( tmpS.GetWord( pos , val ) )
        m_iAreaMax = atoi( val );
    }
    bNewStyle = pk.GetInt( "xdiffr" , m_DiffrRadius.cx ) ;
    bNewStyle |= pk.GetInt( "ydiffr" , m_DiffrRadius.cy ) ;
    if ( !bNewStyle && pk.GetString( "diffr" , tmpS ) )
    { // old style
      FXString val;
      FXSIZE pos = 0;
      if ( tmpS[ pos ] == '(' ) pos++;
      if ( tmpS.GetWord( pos , val ) )
        m_DiffrRadius.cx = atoi( val );
      if ( tmpS.GetWord( pos , val ) )
        m_DiffrRadius.cy = atoi( val );
    }
  }
  else if ( m_Type == LINE_SEGMENT )
  {
    bNewStyle = pk.GetInt( "thick_min" , m_ExpectedSize.left ) ;
    if ( bNewStyle )
      m_ExpectedSize.top = m_ExpectedSize.left ;
    bool bNewThickMax = pk.GetInt( "thick_max" , m_ExpectedSize.right ) ;
    if ( bNewThickMax )
      m_ExpectedSize.bottom = m_ExpectedSize.right ;
    bNewStyle |= bNewThickMax ;
  }
  bool bEdgeAsConturBefore = 
    ( m_WhatToMeasure & MEASURE_EDGE_AS_CONTUR ) && ( m_Type == EDGE ) ;
  if ( !GetMaskFromText( DefMeasurements , pk , m_WhatToMeasure ) )
  { // there is old style
    if ( pk.GetString( "measure" , tmpS ) )
    {
      m_WhatToMeasure = 0;
      int pos = ( tmpS[ 0 ] == _T( '(' ) ) ? 1 : 0;
      FXString val;
      while ( tmpS.GetParam( pos , val , ',' ) )
      {
        m_WhatToMeasure |= GetMeasMaskForName( val.MakeLower() ) ;
      }
    }
    else if ( pk.GetString( "detailed" , tmpS ) )
    {
      FXString val;
      FXSIZE pos = 0;
      if ( tmpS[ pos ] == '(' ) pos++;
      if ( tmpS.GetWord( pos , val ) )
        m_WhatToMeasure = ( DWORD ) ConvToBinary( val ) ;
    }
    //     else
    //     {
    //       m_WhatToMeasure |= MEASURE_POSITION ;
    //       switch ( m_Type )
    //       {
    //       case SPOT: m_WhatToMeasure |= MEASURE_AREA ; break ;
    //       case LINE_SEGMENT: m_WhatToMeasure |= MEASURE_THICKNESS ; break ;
    //       }
    //       //     if ( m_WhatToMeasure & MEASURE_CONTUR )
    //       //       m_WhatToMeasure |= MEASURE_CONTUR ;
    //     }
  }
  if ( ( m_Type == EDGE ) && pInvalidate )
  {
    bool bEdgeAsConturAfter = ( m_WhatToMeasure & MEASURE_EDGE_AS_CONTUR ) ;
    if ( bEdgeAsConturBefore != bEdgeAsConturAfter )
      *pInvalidate = true ;
  }
  if ( pk.GetString( "view" , tmpS ) )
  {
    int pos = 0 ;
    m_ViewResCoord = TRUE ;
    tmpS.MakeLower() ;
    FXParser2 temp ;
    if ( tmpS.FindElement( "color" , temp ) )
    {
      FXString val ;
      if ( temp.GetParam( pos , val , ',' ) )
      {
        if ( val[ 0 ] == '0'  &&  val[ 1 ] == 'x' )
          val = val.Mid( 2 ) ;
        if ( !val.IsEmpty() )
        {
          COLORREF res ;
          if ( sscanf( ( LPCTSTR ) val , "%x" , &res ) )
            m_ViewColor = res ;
        }
      }
    }
    if ( tmpS.FindElement( "sz" , temp ) )
    {
      FXString val ;
      pos = 0 ;
      if ( temp.GetParam( pos , val , ',' ) )
      {
        if ( !val.IsEmpty() )
        {
          int res ;
          if ( sscanf( ( LPCTSTR ) val , "%d" , &res ) )
            m_iViewSize = res ;
        }
      }
    }
    if ( tmpS.FindElement( "offs" , temp ) )
    {
      FXString val;
      pos = 0 ;
      if ( temp.GetParam( pos , val , ',' ) ) m_ViewOffset.cx = atoi( val );
      if ( temp.GetParam( pos , val , ',' ) ) m_ViewOffset.cy = atoi( val );
    }
    if ( tmpS.FindElement( "viewmode" , temp ) )
    {
      FXString val;
      pos = 0 ;
      if ( temp.GetParam( pos , val , ',' ) )
        m_dwViewMode = ( int ) ConvToBinary( val );
    }
  }
  else
  {
    bool bViewModeDecoded = GetMaskFromText( DefViews , pk , m_dwViewMode ) ;
  }
  if ( m_WhatToMeasure & MEASURE_DIFFRACT )
    m_dwViewMode |= OBJ_VIEW_DIFFR ;
  else
    m_dwViewMode &= ~OBJ_VIEW_DIFFR ;
  if ( pk.GetString( "multi" , tmpS ) )
  {
    FXString val;
    FXSIZE pos = 0;
    if ( tmpS[ pos ] == '(' ) pos++;
    if ( tmpS.GetWord( pos , val ) ) m_bMulti = atoi( val ) ;
  }
  //   else
  //     m_bMulti = false ;

  if ( pk.GetString( "min_prof_ampl" , tmpS ) || pk.GetString( "min_ampl" , tmpS ) )
  {
    FXString Value ;
    FXSIZE iPos = 0 ;
    if ( tmpS.GetWord( iPos , Value ) )
    {
      double dVal = atof( ( LPCTSTR ) Value ) ;
      m_dMinProfileContrast = dVal ;
      m_iMinContrast = ROUND( dVal ) ;
    }
  }
  if ( pk.GetString( "prof_thres" , tmpS ) || pk.GetString( "thres" , tmpS ) )
  {
    FXSIZE iPtsPos = tmpS.Find( _T( ':' ) ) ;
    if ( iPtsPos > 0 )
    {
      double dHistThres = atof( ( LPCTSTR ) tmpS + iPtsPos + 1 ) ;
      if ( dHistThres > 0. )
      {
        m_dHistThres = dHistThres / 100. ; // Histogarm threshold is in %
      }
      tmpS = tmpS.Left( iPtsPos ) ;
    }
    else
      m_dHistThres = 0. ;

    FXSIZE iPos = 0 ;
    FXString Value = tmpS.Tokenize( _T( " ,;\n\r" ) , iPos ) ;
    CDblArray Thresholds ;
    while ( !Value.IsEmpty() )
    {
      double dVal = atof( ( LPCTSTR ) Value ) ;
      if ( dVal )
        Thresholds.Add( dVal ) ;
      Value = tmpS.Tokenize( _T( " ,;\n\r" ) , iPos ) ;
    }
    if ( Thresholds.GetCount() )
    {
      m_Thresholds.Copy( Thresholds ) ;
      m_dProfileThreshold = Thresholds[ 0 ] ;
    }
  }
  m_RotationThresholds.RemoveAll() ;
  if ( pk.GetString( "rotation_thres" , tmpS ) )
  {
    CDblArray Thresholds ;

    FXSIZE iPos = 0 ;
    FXString Value = tmpS.Tokenize( _T( " ,;\n\r" ) , iPos ) ;
    while ( !Value.IsEmpty() )
    {
      double dVal = atof( ( LPCTSTR ) Value ) ;
      if ( 0. < dVal  &&  dVal < 1.0 )
      {
        Thresholds.Add( dVal ) ;
      }
      Value = tmpS.Tokenize( _T( " ,;\n\r" ) , iPos ) ;
    }
    if ( Thresholds.GetCount() )
    {
      m_RotationThresholds.Copy( Thresholds ) ;
      m_dRotationThreshold = Thresholds[ 0 ] ;
    }
  }
  if ( ( m_Type == EDGE ) || ( m_Type == LINE_SEGMENT ) )
  {
    m_iDebouncing_pix = DEFAULT_DEBOUNCING ;
    pk.GetInt( "debounce" , m_iDebouncing_pix ) ;
    //m_iProfileLocalization_pix = 0 ;
    pk.GetInt( "local_interval" , m_iProfileLocalization_pix ) ;
  }
  BOOL iDontTouch = FALSE ;
  if ( pk.GetInt( "donttouchedge" , iDontTouch ) )
  {
    m_bDontTouchBorder = iDontTouch ;
    if ( iDontTouch )
      m_WhatToMeasure |= DONT_TOUCH_BORDER ;
    else
      m_WhatToMeasure &= ~DONT_TOUCH_BORDER ;
  }
  pk.GetInt( "hor_back_dist" , m_iHorBackCalcShift ) ;
  pk.GetDouble( "timeout_ms" , m_dTimeout ) ;
  pk.GetInt( "max_n_objects" , m_iNObjectsMax ) ;
  pk.GetInt( "color" , ( int& ) m_ViewColor ) ;
  pk.GetInt( "textsize" , m_iViewSize ) ;
  pk.GetInt( "xtextoffset" , m_ViewOffset.cx ) ;
  pk.GetInt( "ytextoffset" , m_ViewOffset.cy ) ;
  pk.GetInt( "extendforw_perc" , m_iExtendForward_perc ) ;
  pk.GetInt( "ocr_view" , m_iViewMode ) ;
  m_iResultsCounter = 0 ;

  if ( m_Type == SPOT )
  {
    m_pSegmentation = new CSegmentation( m_iNObjectsMax , m_dTimeout ) ;
  }
  return true;
}

int CVideoObject::AnalyzeAndSetViewmode( FXParser2& AsText )
{
  FXString val;
  int pos = 0 ;
  if ( AsText.GetParam( pos , val , ',' ) )
  {
    if ( isdigit( val[ 0 ] ) )
      m_dwViewMode = ( DWORD ) ConvToBinary( val );
    else
    {
      DWORD MaskForMeas , MaskForView ;
      GetMasksForObjType( m_Type , MaskForMeas , MaskForView ) ;
      pos = 0 ;
      m_dwViewMode = 0 ;
      int pos = ( AsText[ 0 ] == _T( '(' ) ) ? 1 : 0;
      FXString val;
      while ( AsText.GetParam( pos , val , ',' ) )
      {
        m_dwViewMode |= GetMeasMaskForName( val.MakeLower() , DefViews , MaskForView ) ;
      }
    }
    return m_dwViewMode ;
  }
  return 0 ;
}

int CVideoObject::AnalyzeAndSetMeasurements( FXParser2& AsText )
{
  m_WhatToMeasure = 0;
  if ( !AsText.IsEmpty() )
  {
    int pos = ( AsText[ 0 ] == _T( '(' ) ) ? 1 : 0;
    FXString val;
    while ( AsText.GetParam( pos , val , ',' ) )
    {
      m_WhatToMeasure |= GetMeasMaskForName( val.MakeLower() ) ;
    }
  }
  else
  {
    m_WhatToMeasure |= MEASURE_POSITION | MEASURE_AREA ;
    switch ( m_Type )
    {
    case SPOT: m_WhatToMeasure |= MEASURE_AREA ; break ;
    case LINE_SEGMENT: m_WhatToMeasure |= MEASURE_THICKNESS ; break ;
    }
  }
  return m_WhatToMeasure ;
}

int CVideoObject::FillRect( CRect& r , FXParser2& src , int& pos )
{
  int iRes = 0 ;
  FXString val ;
  if ( src.GetParam( pos , val , ',' ) )
  {
    r.left = atoi( val );
    iRes++ ;
  }
  if ( src.GetParam( pos , val , ',' ) )
  {
    r.top = atoi( val );
    iRes++ ;
  }
  if ( src.GetParam( pos , val , ',' ) )
  {
    r.right = atoi( val );
    iRes++ ;
  }
  if ( src.GetParam( pos , val , ',' ) )
  {
    r.bottom = atoi( val );
    iRes++ ;
  }
  return iRes ;
}

bool CVideoObject::ScanSettings( FXString& SettingsOut )
{
  FXString Settings = _T( "Template("
    "EditBox(name)," );
  FXString Addition ;
  FormComboSetup( ObjTypeCombo , Settings , _T( "Type" ) ) ;
  FormComboSetup( ObjPlacementCombo , Settings , _T( "Placement" ) ) ;
  m_RelativeToNames.RemoveAll() ;
  if ( ( m_Placement != PLACE_ABS ) && m_pVObjects
    && ( m_pVObjects->GetCount() > 1 ) ) // if only one object, no other objects as reference 
  {
    //if ( m_pVOArrayLock )
    FXAutolock al( *m_pVOArrayLock , "CVideoObject::ScanSettings" ) ;
    FXString Content ;
    int j = 0 ;
    for ( int i = 0 ; i < m_pVObjects->GetCount() ; i++ )
    {
      if ( m_pVObjects->GetAt( i ).m_ObjectName != m_ObjectName )
      {
        if ( (m_pVObjects->GetAt( i ).m_Type == m_Type) 
          || (m_Placement != PLACE_COPY_REL) )
        {
          m_RelativeToNames.Add( m_pVObjects->GetAt( i ).m_ObjectName ) ;
          Addition.Format( _T( "%c%s(%d)" ) , ( j++ > 0 ) ? _T( ',' ) : _T( '(' ) ,
            ( LPCTSTR ) m_pVObjects->GetAt( i ).m_ObjectName , i ) ;
          Content += Addition ;
        }
      }
    }
    Settings += _T( "ComboBox(relativeto" ) ;

    Settings += Content + _T( "))," ) ;
    if ( m_Placement == PLACE_REL_XY )
    {
      Settings += _T( "ComboBox(relativetoy" ) ;
      Settings += Content + _T( "))," ) ;
    }
    if ( m_Placement == PLACE_COPY_REL )
    {
      Settings += _T( "Spin(XOffset,-10000,10000),Spin(YOffset,-10000,10000)" ) ;
    }
    else
    {
      Settings += _T( "Spin(XOffset,-10000,10000),Spin(YOffset,-10000,10000),"
        "Spin(Width,-10000,10000),Spin(Height,-10000,10000)" ) ;
    }
  }
  else
    Settings += _T( "Spin(XOffset,0,100000),Spin(YOffset,0,100000),"
      "Spin(Width,0,100000),Spin(Height,0,100000)" ) ;

  if ( m_Placement != PLACE_COPY_REL )
  {
    FXString Direction , Contrast , Size , Area , Diffraction;
    DWORD MeasMask , ViewMask ;
    GetMasksForObjType( m_Type , MeasMask , ViewMask ) ;

    switch ( m_Type )
    {
    case SPOT:
      Contrast.Format(
        _T( ",ComboBox(contrast"
          "(black_on_white(%d),white_on_black(%d),any(%d)))" ) ,
        BLACK_ON_WHITE , WHITE_ON_BLACK , ANY_CONTRAST ) ;;
      Size = _T( ",Spin(width_min,1,10000)"
        ",Spin(height_min,1,10000)"
        ",Spin(width_max,1,10000)"
        ",Spin(height_max,1,10000)" ) ;
      Area = _T( ",Spin(area_min,1,10000000),Spin(area_max,1,10000000)" ) ;
      Diffraction = _T( ",Spin(xdiffr,0,1000),Spin(ydiffr,0,1000)" ) ;
      Settings += ",CheckBox(donttouchedge)" ;
      break ;
    case LINE_SEGMENT:
      Direction.Format( _T( ",ComboBox(dir(Horizontal(%d),Vertical(%d)))" ) ,
        HORIZONTAL , VERTICAL ) ;
      Contrast.Format(
        _T( ",ComboBox(contrast"
          "(black_on_white(%d),white_on_black(%d),any(%d)))" ) ,
        BLACK_ON_WHITE , WHITE_ON_BLACK , ANY_CONTRAST ) ;;
      Size = _T( ",Spin(thick_min,1,10000)"
        ",Spin(thick_max,1,10000)" ) ;
      break ;
    case EDGE:
      if ( m_WhatToMeasure & MEASURE_EDGE_AS_CONTUR )
      {
        Direction = _T( ",Spin(dir_degrees,-360,360),Spin(find_step,3,30)" )  ;
      }
      else
      {

        Direction.Format( _T( ",Spin(debounce,0,100),ComboBox(dir("
          "LeftToRight(%d),RightToLeft(%d),UpToDown(%d),DownToUp(%d)))" ) ,
          DIR_LR , DIR_RL , DIR_UD , DIR_DU ) ;
      }
      Contrast.Format(
        _T( ",ComboBox(contrast"
          "(black_to_white(%d),white_to_black(%d),any(%d)))" ) ,
        BLACK_TO_WHITE_BRD , WHITE_TO_BLACK_BRD , ANY_CONTRAST_BORDER ) ;
      break ;
    case OCR:
      Direction.Format( _T( ",ComboBox(dir("
        "up_0(%d),right_3(%d),down_6(%d),left_9(%d)))" ) ,
        DIR_00 , DIR_03 , DIR_06 , DIR_09 ) ;
      break ;
    }
    Settings += Direction ;
    Settings += Contrast ;
    Settings += Size ;
    Settings += Area ;
    Settings += Diffraction ;
    //if ( m_Type != OCR )
      Settings += ",CheckBox(Multi)" ;
    //   else
    //     Settings += ",CheckBox(Simple)" ;
    Settings += ",Spin(timeout_ms,0,100000)";
    Settings += ",Spin(max_n_objects,0,10000000)";
    Settings += _T(
      ",Spin(min_ampl,1,100000)"
      ",EditBox(Thres)" ) ;
    if ( m_Type == SPOT )
      Settings += ",EditBox(Rotation_Thres)" ;
    else if ( ( m_Type == LINE_SEGMENT ) || ( m_Type == EDGE ) )
      Settings += ",SPIN(local_interval,0,400)" ;
    if ( m_dwViewMode != 0 )
    {
      Settings += ",ComboBox(color(White(16777215),Silver(12632256),"
        "Gray(8421504),Black(0),Blue(16711680),Navy(8388608),Teal(16776960),"
        "Green(65280),Lime(8453888),Olive(32768),Yellow(65535),Red(255),"
        "Maroon(128),Fuchsia(16711975),Purple(8388736)))" ;
      Settings += _T(
        ",Spin(TextSize,4,40)"
        ",Spin(XTextOffset,-1000,1000)"
        ",Spin(YTextOffset,-1000,1000)" ) ;
      if ( m_Type == OCR )
      {
        Settings += _T( ",Spin(ocr_view,0,100)" ) ;
      } ;
    }
    if ( ( m_Type == EDGE ) && ( m_WhatToMeasure & MEASURE_EDGE_AS_CONTUR ) )
      Settings += ",Spin(ExtendForw_perc,0,70)" ;

    Settings += ",Indent(Measurements)" ;
    ObjNameToMask * pName = DefMeasurements ;
    while ( pName->bitmask )
    {
      if ( pName->bitmask & MeasMask )
      {
        Addition.Format( _T( ",CheckBox(%s)" ) , pName->name ) ;
        Settings += Addition ;
      }
      pName++ ;
    }
    Settings += ",Indent(ViewOptions) " ;
    pName = DefViews ;
    while ( pName->bitmask )
    {
      if ( pName->bitmask & ViewMask )
      {
        Addition.Format( _T( ",CheckBox(%s)" ) , pName->name ) ;
        Settings += Addition ;
      }
      pName++ ;
    }
  }
  Settings += _T( ")" ) ; // close parenthesis for template 
  SettingsOut = Settings.MakeLower() ;
  return true;
}

bool CVideoObject::ScanProperties( const FXString& SettingsIn , bool& bInvalidate )
{
  FXAutolock al( m_VObjectLock ) ;

  FXPropertyKit pk( SettingsIn ) ;

  VOBJ_TYPE OldType = m_Type ;
  bInvalidate |= pk.GetInt( "type" , ( int& ) m_Type ) ;
  if ( m_Type != OldType && m_Type == OCR )
  {
    m_WhatToMeasure = MEASURE_TEXT ;
  }

  LPCTSTR pTypeName = VOTypeToVOName( m_Type ) ;
  if ( !pTypeName )
  {
    SENDERR_3( "Unsupported object type '%d'('%s') for object '%s'" ,
      m_Type , obj2name( ( VOBJ_TYPE ) m_Type ) , m_ObjectName );
    return false ;
  }

  FXParser2 tmpS  ;
  if ( pk.GetString( "name" , tmpS ) )
  {
    if ( !m_ObjectName.IsEmpty() )
    {
      if ( m_ObjectName != tmpS )
      {
        if ( m_pVObjects )
        {
          // Check for the same name of another object
          for ( int i = 0 ; i < m_pVObjects->GetCount() ; i++ )
          {
            if ( m_pVObjects->GetAt( i ).m_ObjectName == tmpS )
            {
              FxSendLogMsg( MSG_WARNING_LEVEL , "CVideoObject" , 0 ,
                "Can't rename %s to %s because name already exists for %s type" ,
                ( LPCTSTR ) m_ObjectName , ( LPCTSTR ) tmpS , pTypeName ) ;
              return false ;
            }

          }
        }
        // If object inserted into tasks then necessary to rename in tasks
        // If object is used as leader for other objects, necessary to rename in
        // other objects. Links will not be changed, only names
        m_bRenamed = true ;
      }
    }
    m_ObjectName = tmpS ; // real renaming
  }
  bool bInitSuccess = InitVideoObject( pTypeName , SettingsIn , &bInvalidate ) ;
  if ( bInitSuccess && ( m_Placement != PLACE_ABS )
    && m_bLeaderChanged && m_pVObjects )
  {
    BOOL bWasLocked = m_pVOArrayLock->IsLockedByThisThread() ;
    if ( !bWasLocked )
      m_pVOArrayLock->Lock( 500 , "CVideoObject::ScanProperties" ) ;

    bool bNameChanged = false ;
    if (/* m_LeaderName.IsEmpty() && */m_LeaderIndex.x >= 0
      && m_LeaderIndex.x < m_pVObjects->GetCount() )
    {
      m_LeaderName = m_pVObjects->GetAt( m_LeaderIndex.x ).m_ObjectName ;
      bNameChanged = true ;
    }
    if ( /*m_LeaderNameY.IsEmpty() &&*/ m_LeaderIndex.y >= 0
      && m_LeaderIndex.y < m_pVObjects->GetCount() )
    {
      m_LeaderNameY = m_pVObjects->GetAt( m_LeaderIndex.y ).m_ObjectName ;
      bNameChanged = true ;
    }
    //     if ( !bNameChanged )
    //     {
    //       for ( int i = 0 ; i < m_pVObjects->GetCount() ; i++ )
    //       {
    //         if ( m_LeaderName == m_pVObjects->GetAt( i ).m_ObjectName )
    //           m_LeaderIndex.x = i ;
    //         if ( m_LeaderNameY == m_pVObjects->GetAt( i ).m_ObjectName )
    //           m_LeaderIndex.y = i ;
    //       }
    //     }
    m_bLeaderChanged = false ;
    if ( !bWasLocked )
      m_pVOArrayLock->Unlock( ) ;

  }
  return bInitSuccess ;
}

bool CVideoObject::PrintProperties( FXString& PropertiesOut , void * pReference )
{
  FXString NextPart = obj2name( m_Type ) ;
  if ( NextPart == "Undefined" )
  {
    SENDERR_1( "Undefined Object Type %d" , m_Type );
    return false ;
  }
  FXPropKit2 AsText ;
  AsText.Format( "%s(name=%s;type=%d;contrast=%d;placement=%d;" ,
    ( LPCTSTR ) NextPart.MakeLower() , ( LPCTSTR ) m_ObjectName ,
    ( int ) m_Type , ( int ) m_Contrast , m_Placement ) ;

  if ( m_Placement != PLACE_ABS )
  {
    if ( m_pVObjects )
    {
      FXAutolock al( *m_pVOArrayLock , "CVideoObject::PrintProperties" ) ;
      if ( ( m_LeaderIndex.x >= 0 ) && ( m_LeaderIndex.x < m_pVObjects->GetCount() ) )  // relative
      {
        if ( m_Placement == PLACE_REL || m_Placement == PLACE_COPY_REL )
        {
          //           NextPart.Format( _T( "relativeto=%d(%s);" ) , m_LeaderIndex.x ,
          //             (LPCTSTR) ((*m_pVObjects)[ m_LeaderIndex.x ].m_ObjectName) ) ;
          NextPart.Format( _T( "relativeto=%d;" ) , m_LeaderIndex.x ) ;
        }
        else if ( ( m_LeaderIndex.y >= 0 ) && ( m_LeaderIndex.y < m_pVObjects->GetCount() ) )
        {
          //           NextPart.Format( _T( "relativeto=%d(%s);relativetoy=%d(%s);" ) ,
          //             m_LeaderIndex.x ,
          //             (LPCTSTR) ((*m_pVObjects)[ m_LeaderIndex.x ].m_ObjectName) ,
          //             m_LeaderIndex.y ,
          //             (LPCTSTR) ((*m_pVObjects)[ m_LeaderIndex.y ].m_ObjectName) ) ;
          NextPart.Format( _T( "relativeto=%d;relativetoy=%d;" ) ,
            m_LeaderIndex.x , m_LeaderIndex.y ) ;
        }
        else
        {
          SENDERR_2( "Not correct leaderY %d for object %s" ,
            m_LeaderIndex.y , ( LPCTSTR ) m_ObjectName );
          NextPart.Format( _T( "relativetoy=%d;" ) ) ;
        }
      }
      else // not correct index
      {
        SENDERR_2( "Not correct leader %d for object %s" ,
          m_LeaderIndex.x , ( LPCTSTR ) m_ObjectName );
        NextPart.Format( _T( "relativeto=%d;" ) ) ;
      }
      AsText += NextPart ;
    }
  }

  if ( m_Placement == PLACE_COPY_REL )
  {
    NextPart.Format( _T( "XOffset=%d;YOffset=%d;" ) ,
      m_AOS.left , m_AOS.top ) ;
    AsText += NextPart ;
  }
  else
  {
    NextPart.Format( _T( "XOffset=%d;YOffset=%d;Width=%d;Height=%d;" ) ,
      m_AOS.left , m_AOS.top , m_AOS.right , m_AOS.bottom ) ;
    AsText += NextPart ;

    if ( m_Type == SPOT )
    {
      NextPart.Format( _T( "width_min=%d;height_min=%d;"
        "width_max=%d;height_max=%d;" ) ,
        m_ExpectedSize.left , m_ExpectedSize.top ,
        m_ExpectedSize.right , m_ExpectedSize.bottom ) ;
      AsText += NextPart ;
      NextPart.Format( _T( "area_min=%d;area_max=%d;" ) , m_iAreaMin , m_iAreaMax ) ;
      AsText += NextPart ;
      NextPart.Format( _T( "xdiffr=%d;ydiffr=%d;" ) , m_DiffrRadius.cx , m_DiffrRadius.cy ) ;
      AsText += NextPart ;
    }
    else if ( m_Type == LINE_SEGMENT )
    {
      AsText.WriteInt( "dir" , ( int ) m_Direction ) ;
      if ( m_Direction == HORIZONTAL )
      {
        NextPart.Format( _T( "thick_min=%d;thick_max=%d;" ) ,
          ( m_ExpectedSize.top < 0 ) ? ( m_ExpectedSize.top = 0 ) : m_ExpectedSize.top ,
          m_ExpectedSize.bottom <= 0 ? ( m_ExpectedSize.bottom = 10 ) : m_ExpectedSize.bottom ) ;
      }
      else
      {
        NextPart.Format( _T( "thick_min=%d;thick_max=%d;" ) ,
          ( m_ExpectedSize.left < 0 ) ? ( m_ExpectedSize.left = 0 ) : m_ExpectedSize.left ,
          m_ExpectedSize.right <= 0 ? ( m_ExpectedSize.right = 10 ) : m_ExpectedSize.right ) ;
      }
      AsText += NextPart ;
      NextPart.Format( "debounce=%d;" , m_iDebouncing_pix ) ;
      AsText += NextPart ;
    }
    else if ( m_Type == EDGE )
    {
      if ( m_WhatToMeasure & MEASURE_EDGE_AS_CONTUR )
      {
        AsText.WriteInt( "dir_degrees" , m_iDirForEdgeContur ) ;
        AsText.WriteInt( "find_step" , m_iEdgeFindStep ) ;
      }
      else
        AsText.WriteInt( "dir" , ( int ) m_Direction ) ;
      AsText.WriteInt( "debounce" , m_iDebouncing_pix ) ;
    }
    else if ( m_Type == OCR )
      AsText.WriteInt( "dir" , ( int ) m_Direction ) ;

    if ( ( ( m_Type == SPOT ) || ( m_Type == LINE_SEGMENT ) ) && ( m_WhatToMeasure != 0 ) )
    {
      NextPart.Format( _T( "detailed=0x%x;" ) , m_WhatToMeasure ) ;
      AsText += NextPart ;
    }
    NextPart.Format( _T( "multi=%d;" ) , m_bMulti ) ;
    AsText += NextPart ;

    NextPart.Format( _T( "min_ampl=%d; " ) ,
      ( m_Type == OCR ) ? m_iMinContrast : ROUND( m_dMinProfileContrast ) ) ;
    AsText += NextPart ;
    NextPart = _T( "thres=" ) ;
    FXString Addition ;
    for ( int i = 0 ; i < m_Thresholds.GetCount() ; i++ )
    {
      Addition.Format( _T( "%6.3f%c" ) ,
        m_Thresholds[ i ] , i >= ( m_Thresholds.GetCount() - 1 ) ? _T( ';' ) : _T( ',' ) ) ;
      NextPart += Addition ;
    }
    if ( m_dHistThres > 0. )
    {
      Addition.Format( _T( ":%.2f" ) , m_dHistThres * 100. ) ;
      NextPart.Insert( NextPart.GetLength() - 1 , Addition ) ;
    }
    AsText += NextPart ;
    if ( m_Type == SPOT )
    {
      if ( m_RotationThresholds.GetCount() )
      {
        NextPart = _T( "rotation_thres=" ) ;
        for ( int i = 0 ; i < m_RotationThresholds.GetCount() ; i++ )
        {
          Addition.Format( _T( "%.3f%c" ) ,
            m_RotationThresholds[ i ] , i >= ( m_RotationThresholds.GetCount() - 1 ) ? _T( ';' ) : _T( ',' ) ) ;
          NextPart += Addition ;
        }
        AsText += NextPart ;
      }
      else
      {
        Addition.Format( _T( "rotation_thres=%.3f;" ) , m_dRotationThreshold ) ;
        AsText += Addition ;
      }
      if ( m_iHorBackCalcShift )
      {
        NextPart.Format( "hor_back_dist=%d;" , m_iHorBackCalcShift ) ;
        AsText += NextPart ;
      }
      NextPart.Format( "donttouchedge=%d;" , m_bDontTouchBorder ) ;
      AsText += NextPart ;
    }
    else if ( ( m_Type == LINE_SEGMENT ) || ( m_Type == EDGE ) )
    {
      Addition.Format( _T( "local_interval=%d;" ) , m_iProfileLocalization_pix ) ;
      AsText += Addition ;
    }


    NextPart.Format( "timeout_ms=%.1f;" , m_dTimeout ) ;
    AsText += NextPart ;
    NextPart.Format( "max_n_objects=%d;" , m_iNObjectsMax ) ;
    AsText += NextPart ;

    NextPart.Format( _T( "color=%d;TextSize=%d;"
      "XTextOffset=%d;YTextOffset=%d;ocr_view=%d;ExtendForw_perc=%d;" ) ,
      m_ViewColor , m_iViewSize ,
      m_ViewOffset.cx , m_ViewOffset.cy ,
      m_iViewMode , m_iExtendForward_perc
    ) ;
    AsText += NextPart ;

    DWORD MeasMask , ViewMask ;
    GetMasksForObjType( m_Type , MeasMask , ViewMask ) ;
    AsText += GetStatesAsString( m_dwViewMode , DefViews , ViewMask ) ;

    AsText += GetStatesAsString( m_WhatToMeasure , DefMeasurements , MeasMask ) ;
  }

  PropertiesOut = AsText.MakeLower() + _T( ')' );

  return true ;
}
int CVideoObject::GetActiveNames(
  FXString& ViewDescription , DWORD Mask , ObjNameToMask * pArray )
{
  int iViewCnt = 0 ;
  if ( Mask )
  {
    while ( pArray->bitmask )
    {
      if ( m_dwViewMode & pArray->bitmask )
      {
        ViewDescription += pArray->name ;
        ViewDescription += _T( ',' ) ;
        iViewCnt++ ;
      }
      pArray++ ;
    }
    if ( iViewCnt )
      ViewDescription.Delete( ViewDescription.GetLength() - 1 ) ;
  }
  return iViewCnt ;
}

ABS_POS_STATUS CVideoObject::GetAbsPosition( CRect& rAbsPos )
{
  if ( m_Placement == PLACE_ABS )
  {
    rAbsPos = m_AOS ;
    return APS_ABS ;
  }
  if ( !m_LastUsedAOS.IsRectNull() ) // object is measured, take last ROI
  {
    rAbsPos = m_LastUsedAOS ;
    return APS_REL_MEASURED ;
  }
  if ( m_pVObjects && ( m_LeaderIndex.x >= 0 )
    && ( m_LeaderIndex.x < m_pVObjects->GetCount() ) )
  {
    CRect rLeaderX ;
    ABS_POS_STATUS Stat =
      m_pVObjects->GetAt( m_LeaderIndex.x ).GetAbsPosition( rLeaderX ) ;
    if ( Stat == APS_ABS_UNKNOWN )
    {
      ASSERT( 0 ) ;
      return APS_ABS_UNKNOWN ;
    }
    CPoint LeaderCentX = rLeaderX.CenterPoint() ;
    CRect OwnAOSRect = m_AOS ;
    //     OwnAOSRect.bottom += OwnAOSRect.top ;
    //     OwnAOSRect.right += OwnAOSRect.left ;
    if ( m_Placement == PLACE_REL )
    {
      OwnAOSRect.left += LeaderCentX.x ;
      OwnAOSRect.top += LeaderCentX.y  ; //shift for leaders abs pos
      rAbsPos = OwnAOSRect ;
      return APS_ABS_CALCULATED ;
    }
    if ( ( m_LeaderIndex.y >= 0 )
      && ( m_LeaderIndex.y < m_pVObjects->GetCount() ) )
    {
      CRect rLeaderY ;
      ABS_POS_STATUS Stat =
        m_pVObjects->GetAt( m_LeaderIndex.y ).GetAbsPosition( rLeaderY ) ;
      if ( Stat == APS_ABS_UNKNOWN )
      {
        ASSERT( 0 ) ;
        return APS_ABS_UNKNOWN ;
      }
      CPoint LeaderCentY = rLeaderY.CenterPoint() ;
      OwnAOSRect.left += LeaderCentX.x ;
      OwnAOSRect.top += LeaderCentY.y  ; //shift for leaders abs pos
      rAbsPos = OwnAOSRect ;
      return APS_ABS_CALCULATED ;
    }
    return APS_ABS_UNKNOWN ;// bad leader Y
  }
  return APS_ABS_UNKNOWN ; // bad leader X
}


int CVideoObject::GetPlacementAndLeaderOffset(
  CRect& rAOS , CPoint& LeaderOffset )
{
  ABS_POS_STATUS apsCurrent = GetAbsPosition( rAOS ) ;
  if ( apsCurrent == APS_ABS_UNKNOWN )
    return 0 ;
  if ( m_Placement == PLACE_ABS )
    return 1 ;
  int iNVObjects = m_pVObjects ? ( int ) m_pVObjects->GetCount() : 0 ;
  CVideoObject* pLeaderX = NULL ;
  CVideoObject* pLeaderY = NULL ;
  CRect rLeaderX , rLeaderY ;
  bool bRelDataReady = false ;
  if ( iNVObjects
    && ( m_LeaderIndex.x >= 0 ) && ( m_LeaderIndex.x < iNVObjects )
    && ( m_LeaderIndex.y >= 0 ) && ( m_LeaderIndex.y < iNVObjects )
    )
  {
    pLeaderX = &( m_pVObjects->GetAt( m_LeaderIndex.x ) );
    pLeaderY = &( m_pVObjects->GetAt( m_LeaderIndex.y ) ) ;
    if ( pLeaderX->GetAbsPosition( rLeaderX ) != APS_ABS_UNKNOWN
      && pLeaderY->GetAbsPosition( rLeaderY ) != APS_ABS_UNKNOWN )
    {
      LeaderOffset.x = rLeaderX.left + ( rLeaderX.right / 2 ) ;
      LeaderOffset.y = rLeaderY.top + ( rLeaderY.bottom / 2 ) ;
      return 2 ; // relative position is OK
    }
  }

  return 0; //   problems
}

CVideoObject::CVideoObject( int iNObjectsMax , double dTimeout )
  : m_bNoFiltration( false )
{
  memset( ( void* ) &m_AOS , 0 , ( size_t ) ( ( BYTE* ) &m_Thresholds - ( BYTE* ) &m_AOS ) ) ;
  m_LeaderIndex = CPoint( -1 , -1 ) ;
  m_Type = NONE ;
  m_Direction = ANY_DIRECTION ;
  m_Contrast = BLACK_ON_WHITE ;
  m_WhatToMeasure = MEASURE_POSITION ;
  m_iMinContrast = MIN_AMPL ;
  m_dMinProfileContrast = MIN_AMPL ;
  m_dProfileThreshold = DEFAULT_NORM_THRES ;
  m_Thresholds.Add( DEFAULT_NORM_THRES ) ;
  m_dRotationThreshold = DEFAULT_NORM_THRES ;
  m_RotationThresholds.Add( DEFAULT_NORM_THRES ) ;
  m_InFOVPosition = -1 , -1 ;
  m_ViewColor = RGB( 255 , 0 , 0 ) ;
  m_iViewSize = 20 ;
  m_ViewOffset = CSize( 5 , -5 ) ;
  m_dwViewMode = OBJ_VIEW_ROI | OBJ_VIEW_POS | OBJ_VIEW_COORD ;
  m_bDontTouchBorder = TRUE ;
  m_iNObjectsMax = iNObjectsMax ;
  m_dTimeout = dTimeout ;
  m_pVObjects = NULL ;
  m_iEdgeFindStep = 5 ;
  m_pVOArrayLock = NULL ;
  m_iExtendForward_perc = 50 ; // for compatibility with ConturASM
  m_ROIandIds.clear() ;
  m_iCurrentROIIndex = -1 ;
  // #ifdef _DEBUG
  //     m_iSignature = 0x55aa0002 ;
  // #endif
}

CVideoObject::CVideoObject( CVideoObject& Original )
{
  memcpy( &m_AOS , &Original.m_AOS , ( size_t ) ( ( BYTE* ) &m_pSegmentation - ( BYTE* ) &m_AOS ) ) ;
  m_LeaderName = Original.m_LeaderName ;
  m_LeaderNameY = Original.m_LeaderNameY ;
  m_ObjectName = Original.m_ObjectName;
  m_Thresholds.Copy( Original.m_Thresholds ) ;
  m_RotationThresholds.Copy( Original.m_RotationThresholds ) ;
  m_pTesseract = Original.m_pTesseract ;
  if ( Original.m_pSegmentation )
  {
    if ( !m_pSegmentation )
      m_pSegmentation = new CSegmentation(
        Original.m_pSegmentation->m_iNMaxContours , Original.m_pSegmentation->m_dTimeout ) ;
    WORD min , max ;
    Original.m_pSegmentation->GetClusterColors( &min , &max ) ;
    m_pSegmentation->SetClusterColors( min , max ) ;
  }
  else
    m_pSegmentation = NULL ;
}

CVideoObject::~CVideoObject()
{
  ClearData() ;
  CSetupObject * pSetup = GetSetupObject( );
  if ( pSetup && ( ( CObjectStdSetup* ) pSetup )->m_hWnd )
  {
    ( ( CObjectStdSetup* ) pSetup )->PostMessageA( WM_DESTROY ) ;
  }
} ;

CVideoObject& CVideoObject::operator= ( const CVideoObject& Original )
{
  memcpy( &m_AOS , &Original.m_AOS , ( size_t ) ( ( BYTE* ) &m_pSegmentation - ( BYTE* ) &m_AOS ) ) ;
  m_dTimeout = Original.m_dTimeout ;
  m_dStartTime = Original.m_dStartTime ;
  m_LeaderName = Original.m_LeaderName ;
  m_LeaderNameY = Original.m_LeaderNameY ;
  m_ObjectName = Original.m_ObjectName;

  m_Thresholds.Copy( Original.m_Thresholds ) ;
  m_RotationThresholds.Copy( Original.m_RotationThresholds ) ;
  m_pTesseract = Original.m_pTesseract ;
  if ( Original.m_pSegmentation )
  {
    if ( !m_pSegmentation )
      m_pSegmentation = new CSegmentation(
        Original.m_pSegmentation->m_iNMaxContours , Original.m_pSegmentation->m_dTimeout ) ;
    WORD min , max ;
    Original.m_pSegmentation->GetClusterColors( &min , &max ) ;
    m_pSegmentation->SetClusterColors( min , max ) ;
  }
  else
    m_pSegmentation = NULL ;
  return *this;
} ;

int CVideoObject::SaveROI( DWORD dwId )
{
//   m_VObjectLock.Lock( INFINITE , "SaveROI" ) ;
  if ( m_ROIandIds.size() < 20 )
    m_ROIandIds.push_back( InfoROIandId( dwId , m_absAOS ) ) ;
  else
    m_ROIandIds[ m_iCurrentROIIndex ] = InfoROIandId( dwId , m_absAOS ) ;
  m_iCurrentROIIndex++ ;
  m_iCurrentROIIndex %= 20 ;
//   m_VObjectLock.Unlock() ;
  return (int)m_ROIandIds.size() ;
}

bool CVideoObject::GetSavedROI( CRect& ResultROI , DWORD dwId )
{
  size_t dwSize = m_ROIandIds.size() ;
  if ( dwSize )
  {
    FXAutolock al( m_VObjectLock , "GetSavedROI" ) ;
    InfoROIandId * pData = m_ROIandIds.data() ;
    InfoROIandId * pEnd = pData + dwSize ;
    do
    {
      if ( pData->m_Id == dwId )
      {
        ResultROI = pData->m_ROI ;
        return true ;
      }
    } while ( ++pData < pEnd );
  }
  return false ;
}

bool CVideoObject::SaveNewAOS( int iPatternIndex , CRect& FrameRect )
{
  int iSaveIndex = -iPatternIndex - 1 ;
  CRect FOVRect( m_absAOS ) ;
  if ( m_InFOVPosition.x > 0
    && m_InFOVPosition.y > 0 )
  {
    FOVRect.OffsetRect(
      ( int ) m_InFOVPosition.x - m_absAOS.CenterPoint().x ,
      ( int ) m_InFOVPosition.y - m_absAOS.CenterPoint().y ) ;
  }
  CRect FutureFOV ;
  FutureFOV.IntersectRect( FOVRect , FrameRect ) ;
  if ( m_SavedAOS.GetUpperBound() < iSaveIndex )
    m_SavedAOS.SetSize( iSaveIndex + 1 ) ;
  m_SavedAOS.SetAt( iSaveIndex , FutureFOV ) ;
  return !FutureFOV.IsRectNull() ;
}

LPCTSTR CVideoObject::obj2name( VOBJ_TYPE type )
{
  switch ( type )
  {
  case   LINE_SEGMENT:
    return "LINE";
  case   BORDER:
    return "BORDER";
  case   EDGE:
    return "EDGE";
  case   AREA_BRIGHTNESS:
    return "AREA_BRIGHTNESS";
  case   SPOT:
    return "SPOT";
  case   CONTOURS_QUANTITIES:
    return "CONTOURS_QUANTITIES";
  case   OCR:
    return "OCR";
  default:
    return "Undefined";
  }
}

void CVideoObject::CopyProcessingParameters( CVideoObject& Prototype )
{

}

int CVideoObject::RecognizeTextByTesseract( 
  pTVFrame ptv , int expectedH , int iBytesPerPixel ,
  FXString& ResultString )
{
  while ( expectedH > 40 )
  {
    if ( !_diminish( ptv ) )
    {
      freeTVFrame( ptv ) ;
      return 0 ;
    }
    expectedH = GetHeight( ptv ) ;
  }
  char * pOutText = NULL ;
  if ( _check_contrast( ptv ) >= m_iMinContrast )
  {
    //       LPBYTE pData = (vf->lpData) ? vf->lpData : (LPBYTE)&(vf->lpBMIH[ 1 ]) ;
    //       m_pTesseract->SetImage( pData + iOffset ,
    //         m_absAOS.Width() , m_absAOS.Height() , iBytesPerPixel , vf->lpBMIH->biWidth ) ;
          //     }
    m_pTesseract->SetImage( GetData( ptv ) , Width( ptv ) , Height( ptv ) ,
      iBytesPerPixel , Width( ptv ) ) ;
    pOutText = m_pTesseract->GetUTF8Text() ;
    if ( pOutText )
    {
      if ( strlen( pOutText ) )
      {
        ResultString = pOutText ;
        m_WhatIsMeasured |= MEASURE_TEXT;
      }
      delete[] pOutText ;
    }
  }
  if ( ptv )
    freeTVFrame( ptv ) ;

  return ( int ) ResultString.GetLength() ;
}

int CVideoObject::RecognizeTextByOCRLib(
  pTVFrame ptv , int expectedH , int iBytesPerPixel )
{
#ifdef _DEBUG
    //saveDIB(fname, ptv);
#endif

  _enchance1( ptv , 500 );
  //ptv=_sharpen(ptv,400,true);

  _normalize( ptv );
  _simplebinarize( ptv );

  m_Clusters.SetClusterColor( BLACK_COLOR );
  m_Clusters.DiagonalConections( true );
  m_Clusters.Reset();
  m_Clusters.ParseFrame( ptv );
  Cluster_ColorFilter( m_Clusters.GetClusterInfo()->m_Clusters ,
    m_Clusters.GetClusterInfo()->m_ClustersNmb , 0 );

  _simpleFilters( m_Clusters.GetClusterInfo() , expectedH / 2 , expectedH - 4 );

  Cluster_SortX( m_Clusters.GetClusterInfo()->m_Clusters , 
    m_Clusters.GetClusterInfo()->m_ClustersNmb );

  Pattern ptrn;
  FXString tmpS;
  for ( int j = 0; j < m_Clusters.GetClustersNmb(); j++ )
  {
    pTVFrame cfr = Cluster_GetFrame( m_Clusters.GetCluster( j ) );
#ifdef _DEBUG
      //fname.Format("D:\\1\\_measureSpot%04d.bmp",a); a++;
      //saveDIB(fname, cfr);
#endif
    freeTVFrame( cfr );
    OCR_RecognizeChar( m_Clusters.GetCluster( j ) , ALPHA_ANY , ptrn );
    tmpS += m_Clusters.GetCluster( j )->rec_as;
  }

  if ( tmpS.GetLength() != 0 )
  {
    tmpS.MakeUpper();
    m_TextResult = tmpS;
    m_WhatIsMeasured |= MEASURE_TEXT;
  }
  //    TRACE("+++ Time taken %f, read string '%s'\n",GetHRTickCount()-ts, tmpS);
#ifdef _DEBUG
  //saveDIB(fname, ptv);
#endif
  freeTVFrame( ptv );

  return ( int ) m_TextResult.GetLength() ;
}

int CVideoObject::RecognizeTextByTesseract( const pTVFrame ptv_FullFrame ,
  FXString& ResultString ) 
{
  // Get profiles for 10 areas
  // Space for profiles
  Profile Profiles[ 10 ] ;
  ResultString.Empty() ;
  IntVector Rises[ 10 ] , Falls[ 10 ] ;
  CmplxVector Centers[ 10 ];
  array<ProfAnalysisData,10> AData ;
  DoubleVector Amplitudes[ 10 ] , Maximums[ 10 ] , Minimums[ 10 ] ;
  if ( !m_pDiagnostics )
    m_pDiagnostics = CContainerFrame::Create() ;
  CContainerFrame * pMarking = ( CContainerFrame* ) m_pDiagnostics ;

  switch ( m_Direction )
  {
    case DIR_03:
    {
      // Calculate vertical profiles and extract upper level slopes
      for ( int iZone = 0 ; iZone < 10 ; iZone++ )
      {
        int iX = m_absAOS.left + ( m_absAOS.Width() * iZone ) / 10 ;
        ProfAnalysisData& ADataRef = AData[iZone] ;
        ADataRef.m_ProfRect = CRect( iX , m_absAOS.top ,
          ROUND( m_absAOS.Width() / 10. ) , m_absAOS.Height() ) ;
        Profile& Prof = Profiles[ iZone ] ;
        double dAver = calc_vprofile( ptv_FullFrame ,
          &Prof , &ADataRef.m_ProfRect ) ;
        ADataRef.m_dAmpl = ( Prof.m_dMaxValue - Prof.m_dMinValue ) ;
        if ( ADataRef.m_dAmpl != 0. )
        {
          CFigureFrame * pProfView = CFigureFrame::Create() ;
          double * pProf = Prof.m_pProfData ;
          double dMaxB = -DBL_MAX , dMaxE = -DBL_MAX ;
          int iShiftE = ( ADataRef.m_ProfRect.bottom * 4 ) / 5 ;
          for ( int iY = 0 ; iY < ADataRef.m_ProfRect.bottom / 5 ; iY++ )
          {
            double dValB = pProf[ iY ] ;
            double dValE = pProf[ iY + iShiftE ] ;
            if ( dValB > dMaxB )
              dMaxB = dValB ;
            if ( dValE > dMaxE )
              dMaxE = dValE ;
          }
          ADataRef.m_dSlope = ( dMaxB - dMaxE ) / ADataRef.m_ProfRect.bottom ;
        }
      }

      double dOverallMaxAmpl = -DBL_MAX ;
      double dOverAllMinAmpl = DBL_MAX ;
      // Amplitude analysis for all profiles
      // And max amplitude calculation
      for ( int iZone = 0 ; iZone < 10 ; iZone++ )
      {
        int iX = m_absAOS.left + ( m_absAOS.Width() * iZone ) / 10 ;
        Profile& Prof = Profiles[ iZone ] ;
        ProfAnalysisData& ADataRef = AData[ iZone ] ;
        DoubleVector& Ampls = Amplitudes[ iZone ] ;
        DoubleVector& Maxes = Maximums[ iZone ] ;
        DoubleVector& Mins = Minimums[ iZone ] ;
        Ampls.resize( ADataRef.m_ProfRect.bottom ) ;
        Maxes.resize( ADataRef.m_ProfRect.bottom ) ;
        Mins.resize( ADataRef.m_ProfRect.bottom ) ;
        if ( ADataRef.m_dAmpl != 0. )
        {
          CFigureFrame * pProfView = CFigureFrame::Create() ;
          ADataRef.m_dScaleX = ADataRef.m_ProfRect.right / ADataRef.m_dAmpl ;
          double * pProf = Prof.m_pProfData ;
          double dMinInArea = DBL_MAX , dMaxInArea = -DBL_MAX ;
          CmplxVector UpperLevel , LowerLevel ;
          double dMax = -DBL_MAX , dMin = DBL_MAX ;
          for ( int iY = 0 ; iY < ADataRef.m_ProfRect.bottom ; iY++ )
          {
            double dVal = pProf[ iY ] -= ( Prof.m_dMinValue - ADataRef.m_dSlope * iY ) ;
            if ( dVal > dMax )
              dMax = dVal ;
            else
              dMax *= 0.999 ;
            Maxes[ iY ] = dMax ;

            if ( dVal < dMin )
              dMin = dVal ;
            else
              dMin = dMin + ( dMax - dMin ) * 0.001 ;
            Mins[ iY ] = dMin ;
          }
          dMax = -DBL_MAX ;
          dMin = DBL_MAX ;
          for ( int iY = ADataRef.m_ProfRect.bottom - 1 ; iY >= 0 ; iY-- )
          {
            double dVal = pProf[ iY ] ;
            CDPoint Pt( ADataRef.m_ProfRect.left + dVal * ADataRef.m_dScaleX ,
              iY + ADataRef.m_ProfRect.top ) ;
            pProfView->AddPoint( Pt ) ;

            if ( dVal > dMax )
              dMax = dVal ;
            else
              dMax *= 0.999 ;
            if ( Maxes[ iY ] < dMax )
              Maxes[ iY ] = dMax ;
            cmplx cPt( ADataRef.m_ProfRect.left + Maxes[ iY ] * ADataRef.m_dScaleX ,
              iY + ADataRef.m_ProfRect.top ) ;
            UpperLevel.push_back( cPt ) ;

            if ( dVal < dMin )
              dMin = dVal ;
            else
              dMin = dMin + ( dMax - dMin ) * 0.001 ;
            if ( Mins[ iY ] > dMin )
              Mins[ iY ] = dMin ;

            cPt = cmplx( ADataRef.m_ProfRect.left + Mins[ iY ] * ADataRef.m_dScaleX ,
              iY + ADataRef.m_ProfRect.top ) ;
            LowerLevel.push_back( cPt ) ;

            Ampls[ iY ] = Maxes[ iY ] - Mins[ iY ] ;
            SetMinMax( Ampls[ iY ] , dOverAllMinAmpl , dOverallMaxAmpl ) ;
          }
          *( pProfView->Attributes() ) = "color=0x000000ff;" ;
          if ( pMarking )
          {
            if ( m_iViewMode > 10 )
              pMarking->AddFrame( pProfView ) ;
            if ( m_iViewMode > 11 )
            {
              pMarking->AddFrame( CreateFigureFrameEx( UpperLevel.data() ,
                ( int ) UpperLevel.size() , 0x00ff0000 ) ) ;
              pMarking->AddFrame( CreateFigureFrameEx( LowerLevel.data() ,
                ( int ) LowerLevel.size() , 0x00ff0000 ) ) ;
            }
          }
        }
      }

      double dRelThres = ( dOverallMaxAmpl - dOverAllMinAmpl ) * 0.1 ;
      double dAmplThres = ( dOverallMaxAmpl - dOverAllMinAmpl ) * 0.3 ;
      int iMaxCentersIndex = -1 ;
      int iNMaxCenters = -1 ;
      int iNNoTextZones = 0 ;

      // Find threshold crosses
      for ( int iZone = 0 ; iZone < 10 ; iZone++ )
      {
        IntVector& ThisRises = Rises[ iZone ];
        IntVector& ThisFalls = Falls[ iZone ];
        Profile& Prof = Profiles[ iZone ] ;
        ProfAnalysisData& ADataRef = AData[ iZone ] ;
        DoubleVector& Ampls = Amplitudes[ iZone ] ;
        DoubleVector& Maxes = Maximums[ iZone ] ;
        double * pProf = Prof.m_pProfData ;
        bool bLastWasHigh = ( pProf[ 0 ] >= ( Maxes[ 0 ] - dRelThres ) ) ;
        int iNHigh = 0 , iNLow = 0 ;

        for ( int iY = 0 ; iY < ADataRef.m_ProfRect.bottom ; iY++ )
        {
          double dVal = pProf[ iY ] ;
          bool bSave = true ;
          if ( Ampls[ iY ] < dAmplThres )
            continue ;
          if ( dVal >= ( Maxes[ iY ] - dRelThres ) )
          {
            if ( !bLastWasHigh )
            {
              bLastWasHigh = true ;
              if ( ThisFalls.size() )
              {
                if ( ( iY - ThisFalls.back() ) < 8 )
                {
                  ThisFalls.pop_back() ;
                  bSave = false ;
                }
                else if ( ThisRises.size()
                  && ( ThisFalls.back() < ThisRises.back() ) )
                {
                  bSave = false ;
                }
              }
              if ( bSave )
                ThisRises.push_back( iY ) ;
            }
          }
          else if ( bLastWasHigh )
          {
            bLastWasHigh = false ;
            if ( ThisRises.size() )
            {
              if ( ( iY - ThisRises.back() ) < 8 )
              {
                ThisRises.pop_back() ;
                bSave = false ;
              }
              else if ( ThisFalls.size() 
                && ( ThisRises.back() < ThisFalls.back()))
              {
                bSave = false ;
              }
            }
            if ( bSave )
              ThisFalls.push_back( iY ) ;
          }
        }

        CmplxVector& ThisCenters = Centers[ iZone ];
        int iRiseIndex = 0 ;
        for ( size_t iCent = 0 ; iCent < ThisFalls.size() ; iCent++)
        {
          cmplx cPt( ADataRef.m_ProfRect.left
            + pProf[ ThisFalls[iCent] ] * ADataRef.m_dScaleX ,
            ThisFalls[iCent] + ADataRef.m_ProfRect.top ) ;
          if ( m_iViewMode > 12 )
            pMarking->AddFrame( CreatePtFrameEx( cPt , 0x00ffff00 ) ) ;
          bool bOmittedRise = false ;
          if ( iRiseIndex < (int)ThisRises.size() )
          {
            if ( ThisRises[ iRiseIndex ] < ThisFalls[ iCent ] )
            {
              bOmittedRise = true ;
              cmplx cPtOmitted( ADataRef.m_ProfRect.left
                + pProf[ ThisRises[ iRiseIndex ] ] * ADataRef.m_dScaleX ,
                ThisRises[ iRiseIndex ] + ADataRef.m_ProfRect.top ) ;
              if ( m_iViewMode > 12 )
                pMarking->AddFrame( CreatePtFrameEx( cPtOmitted , 0x00ff00ff ) ) ;
              if ( ++iRiseIndex >= ( int ) ThisRises.size() )
                continue ; // it was last rise
            }
            cmplx cCenter( cPt.real() + ADataRef.m_ProfRect.left
              + pProf[ ThisRises[ iRiseIndex ] ] * ADataRef.m_dScaleX ,
              cPt.imag() + ThisRises[ iRiseIndex ] + ADataRef.m_ProfRect.top ) ;
            cCenter /= 2. ;
            ThisCenters.push_back( cCenter ) ;
            if ( m_iViewMode > 12 )
              pMarking->AddFrame( CreatePtFrameEx( cCenter , 0x0000ff00 ) ) ;
            cmplx cPtR( ADataRef.m_ProfRect.left
              + pProf[ ThisRises[ iRiseIndex ] ] * ADataRef.m_dScaleX ,
              ThisRises[ iRiseIndex ] + ADataRef.m_ProfRect.top ) ;
            if ( m_iViewMode > 12 )
              pMarking->AddFrame( CreatePtFrameEx( cPtR , 0x00ff0000 ) ) ;
            iRiseIndex++ ;
          }
        }
        if ( ThisCenters.size() )
        {
          if ( iNMaxCenters < ( int ) ThisCenters.size() )
          {
            iNMaxCenters = ( int ) ThisCenters.size() ;
            iMaxCentersIndex = iZone ;
          }
        }
        else
          iNNoTextZones++ ;
      }

      if ( iNMaxCenters > 0 )
      {
        IntVector& ThisFalls = Falls[ iMaxCentersIndex ] ;
        IntVector& ThisRises = Rises[ iMaxCentersIndex ] ;
        CmplxVector& ThisCenters = Centers[ iMaxCentersIndex ] ;
        ProfAnalysisData& ADataRef = AData[ iMaxCentersIndex ] ;

        int iCentralStringIndex = (int)ThisCenters.size() / 2 ;
        cmplx cCentInZoneString = ThisCenters[ iCentralStringIndex ] ;
        
        int iIndexFall = 0 ;
        while ( ThisFalls[ iIndexFall ] + ADataRef.m_ProfRect.top < cCentInZoneString.imag() )
          iIndexFall++ ;
        ASSERT( iIndexFall < (int)ThisFalls.size() ) ;
        iIndexFall-- ;

        int iIndexRise = 0 ;
        while ( ThisRises[ iIndexRise ] + ADataRef.m_ProfRect.top < cCentInZoneString.imag() )
          iIndexRise++ ;
        ASSERT( iIndexRise < (int) ThisRises.size() ) ;
        int iHeight = (ThisRises[ iIndexRise ] - ThisFalls[ iIndexFall ]) /** 2 */;

        int iBytesPerPixel = 1 ;
        switch ( ptv_FullFrame->lpBMIH->biCompression )
        {
          case BI_Y16: iBytesPerPixel = 2 ; break ;
          case BI_RGB: iBytesPerPixel = 3 ; break ;
          case BI_Y800:
          case BI_Y8:
          case BI_YUV9:
          case BI_YUV12: break ;
          default:
          SENDERR( "OCR not supported format %s" , GetVideoFormatName( ptv_FullFrame->lpBMIH->biCompression ) ) ;
          return false ; // not supported formats
        }

        m_pTesseract->SetImage( GetData( ptv_FullFrame ) , 
          Width( ptv_FullFrame ) , Height( ptv_FullFrame ) ,
          iBytesPerPixel , Width( ptv_FullFrame ) ) ;

        for ( size_t iCenters = 0 ; iCenters < ThisCenters.size() ; iCenters++ )
        {
          CRect ROI( m_absAOS.left , ROUND( ThisCenters[ iCenters ].imag() ) - iHeight ,
            m_absAOS.right , ROUND( ThisCenters[ iCenters ].imag() ) + iHeight ) ;
          if ( m_iViewMode > 9 )
            pMarking->AddFrame( CreateFigureFrameEx( ROI , 0x0000ffff , 2 ) ) ;
          m_pTesseract->SetRectangle( ROI.left , ROI.top , ROI.Width() , ROI.Height() ) ;

          char * pOutText = NULL ;
          pOutText = m_pTesseract->GetUTF8Text() ;
          if ( pOutText )
          {
            if ( strlen( pOutText ) )
            {
//               if ( !ResultString.IsEmpty() )
//                 ResultString += '\n' ;
              ResultString += pOutText ;
              m_WhatIsMeasured |= MEASURE_TEXT;
            }
            delete[] pOutText ;
          }
        }
      }
    }
    break;
//     case DIR_06:
//     expectedH = m_absAOS.Width();
// //     _rotate_minus90_Y8_16( ptv ) ;
//     _rotate90_Y8_16( ptv );
//     break;
//     case DIR_09:
//     expectedH = m_absAOS.Height();
//     _rotate180_Y8_16( ptv ) ;
//     break;
//     case DIR_00:
//     expectedH = m_absAOS.Width();
// //     _rotate90_Y8_16( ptv );
//     _rotate_minus90_Y8_16( ptv ) ;
//     break;
    default:
    SENDERR_0( "Measurement for object OCR can't be performed. Reason: no direction is defined." );
  }
  return ( int ) ResultString.GetLength() ;
}

bool VOArray::DoMeasure( int nIndex , 
  const CVideoFrame* vf , int iPatternIndex )
{
  if ( nIndex > GetUpperBound() )
    return false;
  if ( nIndex == -1 )
    return false;

  bool bRes = false ;
  CRect FrameRect( 0 , 0 , vf->lpBMIH->biWidth - 1 , vf->lpBMIH->biHeight - 1 ) ;
  CVideoObject& Obj = ElementAt( nIndex ) ;
  CRect AOS = Obj.m_AOS ;
  AOS.right += AOS.left ;
  AOS.bottom += AOS.top ;
  if ( ( iPatternIndex < 0 ) ) // i.e.measurement by usual setup
  {
    if ( ( ( Obj.m_LeaderIndex.x >= 0 ) && ( Obj.m_LeaderIndex.x >= GetSize() ) )
      || ( ( Obj.m_LeaderIndex.y >= 0 ) && ( Obj.m_LeaderIndex.y >= GetSize() ) ) )
      Obj.m_LeaderIndex.x = Obj.m_LeaderIndex.y = -1 ;

    if ( Obj.m_LeaderIndex.x == -1 )
    {
      AOS.OffsetRect( m_CommonOffset ) ;
      Obj.m_absAOS.IntersectRect( AOS , FrameRect ) ;
      Obj.m_LastUsedAOS = Obj.m_absAOS ;
      bRes = Obj.DoMeasure( vf );
      if ( bRes && !Obj.m_bMulti )
        Obj.SaveNewAOS( iPatternIndex , FrameRect ) ;
    }
    else
    {
      CVideoObject& LeaderX = ElementAt( Obj.m_LeaderIndex.x ) ;
      if ( LeaderX.m_WhatIsMeasured & MEASURE_POSITION )
      {

        CPoint Offset = LeaderX.m_InFOVPosition.GetCPoint() ;
        if ( Obj.m_LeaderIndex.x != Obj.m_LeaderIndex.y )
        {
          if ( Obj.m_LeaderIndex.y >= 0 )
          {
            CVideoObject& LeaderY = ElementAt( Obj.m_LeaderIndex.y ) ;
            if ( LeaderY.m_WhatIsMeasured & MEASURE_POSITION )
              Offset.y = ROUND( LeaderY.m_InFOVPosition.y ) ;
            else
              return false ;
          }
          else
            Offset.y = 0 ;
        }
        AOS.OffsetRect( Offset ) ;
        Obj.m_absAOS.IntersectRect( AOS , FrameRect );
        Obj.m_LastUsedAOS = Obj.m_absAOS ;
        bRes = Obj.DoMeasure( vf ) ;
        if ( bRes && !Obj.m_bMulti )
          Obj.SaveNewAOS( iPatternIndex , FrameRect ) ;
      }
      else
      {
        Obj.m_absAOS.SetRectEmpty();
        return false ;
      }
    }
  }
  else // Measurement by previous results
  {
    if ( iPatternIndex > Obj.m_SavedAOS.GetUpperBound() )
      return false ;
    CRect PrevAOS = Obj.m_SavedAOS[ iPatternIndex ] ;
    PrevAOS.OffsetRect( m_CommonOffset ) ;
    AOS.IntersectRect( PrevAOS , FrameRect ) ;
    if ( ( AOS.Width() > 3 ) && ( AOS.Height() > 3 ) )
    {
      Obj.m_absAOS = AOS ;
      Obj.m_LastUsedAOS = Obj.m_absAOS ;
      bRes = Obj.DoMeasure( vf );
    }
    else
      return false ;
  }
  Obj.m_LastUsedAOS.right -= Obj.m_LastUsedAOS.left ;
  Obj.m_LastUsedAOS.bottom -= Obj.m_LastUsedAOS.top ;
  return bRes;
}
