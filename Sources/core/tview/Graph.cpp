// Graph.cpp: implementation of the CGraph class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "graph.h"
#include "sketchview.h"
#include "sketchviewmod.h"
#include "renderviewframe.h"
#include <gadgets\shkernel.h>
#include "floatwnd.h"
#include <math.h>
#include "debugenvelopdlg.h"
#include <gadgets\stdsetup.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define CLICK_MARGIN            10  // Distance of sensitivity for object selection
#define GLYPH_CONNECTOR_WIDTH	5
#define GLYPH_CONNECTOR_HEIGHT	5
#define GLYPH_GADGET_WIDTH		80
#define GLYPH_GADGET_HEIGHT		80
#define GLYPH_CLIENT_MARGIN		5
#define GLYPH_CAPTION_HEIGHT	10
// #define GLYPH_CONNECTOR_OFFSET_X (2)
// #define GLYPH_CONNECTOR_OFFSET_Y (2)
#define GLYPH_VERT_SPACE_TO_FIRST_PIN 16
#define GLYPH_VERT_SPACE_BETWEEN_PINS 14

#define GLYPH_MIN_HEIGHT 10
#define GLYPH_MIN_WIDTH 10

#define SEARCH_UP 1
#define SEARCH_LEFT 2
#define SEARCH_DOWN 3
#define SEARCH_RIGHT 4

#define MY_EVADE_SIZE 6
#define COUNTER_LIMIT_LEVEL1 3000
#define COUNTER_LIMIT_LEVEL2 500
#define COUNTER_LIMIT_LEVEL3 100

#ifdef TVDB400_TRACE_GLYPHS

CGlyphsHeap* CGlyphsHeap::m_pThis = NULL;

CGlyphsHeap::CGlyphsHeap() :
  m_Refs( 0 )
{}

CGlyphsHeap::~CGlyphsHeap()
{
  m_Lock.Lock();
  if ( !m_Glyphs.GetCount() )
  {
    TRACE( " * * *  GLYPHS HEAP IS EMPTY  * * *\n" );
  }
  else
  {
    TRACE( " ! ! !  GLYPHS HEAP IS NOT EMPTY  ! ! !\n" );
    POSITION pos = m_Glyphs.GetStartPosition();
    while ( pos )
    {
      LPVOID pGlyph , dummy;
      m_Glyphs.GetNextAssoc( pos , pGlyph , dummy );
      TRACE( "  : : :  Glyph 0x%x remained undeleted\n" , pGlyph );
    }
    m_Glyphs.RemoveAll();
  }
  m_pThis = NULL;
  m_Lock.Unlock();
}

CGlyphsHeap* CGlyphsHeap::Get()
{
  if ( !m_pThis )
    m_pThis = new CGlyphsHeap();
  return m_pThis;
}

BOOL CGlyphsHeap::Register( void* pGlyph )
{
  m_Lock.Lock();
  BOOL bResult = TRUE;
  void* dummy;
  if ( !m_Glyphs.Lookup( pGlyph , dummy ) )
  {
    m_Glyphs.SetAt( pGlyph , NULL );
//    TRACE( " + + + (%d) Glyph 0x%p added\n" , m_Glyphs.GetCount() , pGlyph );
  }
  else
  {
    bResult = FALSE;
    TRACE( " +!+!+! (%d) Glyph 0x%p can not be added: already exists\n" , m_Glyphs.GetCount() , pGlyph );
  }
  m_Lock.Unlock();
  return bResult;
}

BOOL CGlyphsHeap::Unregister( void* pGlyph )
{
  m_Lock.Lock();
  BOOL bResult = TRUE;
  void* dummy;
  if ( m_Glyphs.Lookup( pGlyph , dummy ) )
  {
    m_Glyphs.RemoveKey( pGlyph );
    //TRACE( " - - - (%d) Glyph 0x%p removed\n" , m_Glyphs.GetCount() , pGlyph );
  }
  else
  {
    bResult = FALSE;
    TRACE( " -!-!-! (%d) Glyph 0x%p can not be removed: doesn't exist\n" , m_Glyphs.GetCount() , pGlyph );
  }
  m_Lock.Unlock();
  return bResult;
}

#endif


//
// (CGlyph)
//   CWireGlyph
//   CCompositeGlyph
//    CGadgetGlyph
//     CRenderGlyph
//    CCollectionGlyph
//

__forceinline int roundI( double a )
{
  return ((a < 0) ? (int) (a - .5) : (int) (a + .5));
}

__forceinline double calcDT( CRect rcLink )
{
  double dt;
  if ( rcLink.left - rcLink.right == 0 )
  {
    dt = 1.0 / (rcLink.bottom - rcLink.top);
  }
  else if ( rcLink.top - rcLink.bottom == 0 )
  {
    dt = 1.0 / (rcLink.right - rcLink.left);
  }
  else
  {
    double dt1 , dt2;
    dt1 = 1.0 / (rcLink.right - rcLink.left);
    dt2 = 1.0 / (rcLink.bottom - rcLink.top);
    if ( dt1 < 0 ) dt1 = -dt1;
    if ( dt2 < 0 ) dt2 = -dt2;
    dt = min( dt1 , dt2 );
  }
  if ( dt < 0 ) dt = -dt;
  return dt;
}

// CGlyph
BOOL CGlyph::SearchEvadedGlyph( void *gl )
{
  for ( int i = 0; i < m_EvadedGlyphs.GetSize(); i++ )
  {
    if ( gl == m_EvadedGlyphs.GetAt( i ) )
      return TRUE;
  }
  return FALSE;
}

void CGlyph::OffsetPos( CPoint& offset )
{
  FXString UID;
  m_Pos += offset;
  if ( (IsGadgetGlyph()) && (GetUID( UID )) )
    m_View->GetBuilder()->SetGlyph( UID , m_Pos.x , m_Pos.y );
}

// CWireGlyph

CWireGlyph::CWireGlyph( CSketchView *view , LPCTSTR uid ) :
  CGlyph( view ) ,
  m_UID( uid ) ,
  pInput( NULL ) ,
  pOutput( NULL )
{}

CWireGlyph::CWireGlyph( CSketchView *view , LPCTSTR uid , ViewType vt ) :
  CGlyph( view ) ,
  m_UID( uid ) ,
  pInput( NULL ) ,
  pOutput( NULL )
{
  SetViewType( vt );
}

BOOL CWireGlyph::SetConnection( CGlyph* pInput1 , CGlyph *pOutput1 )
{
  if ( (!pInput1) || (!pOutput1) )
    return FALSE;

  pInput = pInput1;
  pOutput = pOutput1;
  FXString uid1 , uid2;
  pInput->GetUID( uid1 ); pOutput->GetUID( uid2 );
  if ( uid1.Find( ">>" ) > 0 || uid2.Find( "<<" ) > 0 )
  {
    FXString tmp = uid1;
    uid1 = uid2;
    uid2 = tmp;
  }
  else if ( uid1.Find( "<>" ) > 0 && uid2.Find( "<>" ) > 0 && uid2.Compare( uid1 ) > 0 )
  {
    FXString tmp = uid1;
    uid1 = uid2;
    uid2 = tmp;
  }
  m_UID = uid2 + "," + uid1;
  // ((CConnectorGlyph*)pInput)->ConnectTo(pOutput1);
  //  ((CConnectorGlyph*)pOutput)->ConnectTo(pInput);
  return TRUE;
}

BOOL CWireGlyph::SetInputConnection( CGlyph *input )
{
  if ( !input )
    return FALSE;
  pInput = input;
  ((CConnectorGlyph*) pInput)->ConnectTo( pOutput );
  ((CConnectorGlyph*) pOutput)->ConnectTo( pInput );
  return TRUE;
}

BOOL CWireGlyph::SetOutputConnection( CGlyph *output )
{
  if ( !output )
    return FALSE;
  pOutput = output;
  ((CConnectorGlyph*) pInput)->ConnectTo( pOutput );
  ((CConnectorGlyph*) pOutput)->ConnectTo( pInput );
  return TRUE;
}

void CWireGlyph::Disconnect()
{
  if ( pInput )
    ((CConnectorGlyph*) pInput)->DisconnectWire( this );

  if ( pOutput )
    ((CConnectorGlyph*) pOutput)->DisconnectWire( this );
  m_View->RemoveGlyphFromGraph( this );
  delete this;
}

void CWireGlyph::Draw( CDC* pDC , bool bActivityOnly )
{
  if ( !bActivityOnly )
  {
    LOGPEN pen;
    if ( m_View->GetMyPen( this , &pen ) )
    {
      CRect rcLink( ((CConnectorGlyph*) pInput)->GetConnectionPoint() , ((CConnectorGlyph*) pOutput)->GetConnectionPoint() );
      m_View->DrawLine( pDC , rcLink , &pen );
    }
  }
}

BOOL CWireGlyph::GetRgn( CRgn* pRgn , BOOL bSelf )
{
  const int marg = CLICK_MARGIN;
  CPoint pt[ 4 ];
  pt[ 0 ] = ((CConnectorGlyph*) pInput)->GetConnectionPoint();
  pt[ 1 ] = ((CConnectorGlyph*) pOutput)->GetConnectionPoint();
  CPoint v = pt[ 1 ] - pt[ 0 ];
  int dx = (v.x > 0) ? v.x : -v.x;
  int dy = (v.y > 0) ? v.y : -v.y;
  int d = (dx > dy) ? dx : dy;
  if ( !d ) return TRUE;
  CPoint n( -(v.y * marg) / d , (v.x * marg) / d );
  pt[ 2 ] = pt[ 1 ] - n;
  pt[ 3 ] = pt[ 0 ] - n;
  pt[ 0 ] += n;
  pt[ 1 ] += n;
  CRgn rgn;
  rgn.CreatePolygonRgn( pt , 4 , ALTERNATE );
  pRgn->CombineRgn( pRgn , &rgn , RGN_OR );
  rgn.DeleteObject();
  return TRUE;
}

CGlyph* CWireGlyph::Intersect( CPoint& pt , BOOL bSelfRgn )
{
  CGlyph* pGlyph = NULL;
  CRgn rgn;
  rgn.CreateRectRgnIndirect( CRect( ((CConnectorGlyph*) pInput)->GetPos() , CSize( 0 , 0 ) ) );
  GetRgn( &rgn , bSelfRgn/*(m_ConnectedGlyphs.GetSize() > 1)*/ );
  if ( rgn.PtInRegion( pt ) )
    pGlyph = this;
  rgn.DeleteObject();
  return pGlyph;
}

void CWireGlyph::DestroyIn()
{
  FXString uid1 , uid2;
  pInput->GetUID( uid1 ); pOutput->GetUID( uid2 );
  if ( m_View->Disconnect( uid1 , uid2 ) || m_View->Disconnect( uid2 , uid1 ) )
  {
    m_View->RemoveGlyphFromGraph( this );
    Disconnect();
  }
}

void CWireGlyph::CleanUp()
{}

BOOL CWireGlyph::SwitchConnectionTo( CGlyph *gl )
{
  pOutput = gl;
  return TRUE;
}

//CEvadeWireGlyph

CEvadeWireGlyph::CEvadeWireGlyph( CSketchView *view , LPCTSTR uid ) :
  CWireGlyph( view , uid )
{}

CEvadeWireGlyph::CEvadeWireGlyph( CSketchView *view , LPCTSTR uid , ViewType vt ) :
  CWireGlyph( view , uid )
{
  SetViewType( vt );
}

void CEvadeWireGlyph::Draw( CDC* pDC , bool bActivityOnly )
{
  if ( !bActivityOnly )
  {
    if ( GetViewType().m_EvType == ViewType::EVADE_CLASSIC )
      CWireGlyph::Draw( pDC );
    else if ( GetViewType().m_EvType == ViewType::EVADE_MODERN )
      DrawGlyph( pDC );
  }
}

bool CEvadeWireGlyph::CheckMins( int a1 , int a2 , int delta , int min )
{
  if ( (abs( a1 - a2 ) < delta) && ((a1 == min) || (a2 == min)) )
    return true;
  else
    return false;
}

__forceinline int __dist( CPoint pt1 , CPoint pt2 )
{
  return (int) sqrt( (double) (pt1.x - pt2.x)*(pt1.x - pt2.x) + (pt1.y - pt2.y)*(pt1.y - pt2.y) );
}

int CEvadeWireGlyph::DistToRect( CRect rc , CPoint pt )
{
  if ( rc.PtInRect( pt ) ) return 0;
  if ( (pt.x > rc.right) && (pt.y < rc.top) )
    return __dist( pt , CPoint( rc.right , rc.top ) );
  else if ( (pt.x < rc.left) && (pt.y < rc.top) )
    return __dist( pt , CPoint( rc.left , rc.top ) );
  else if ( (pt.x < rc.left) && (pt.y > rc.bottom) )
    return __dist( pt , CPoint( rc.left , rc.bottom ) );
  else if ( (pt.x > rc.right) && (pt.y > rc.bottom) )
    return __dist( pt , CPoint( rc.right , rc.bottom ) );
  else
  {
    if ( (pt.x > rc.left) && (pt.x < rc.right) )
      return min( abs( pt.y - rc.top ) , abs( rc.bottom - pt.y ) );
    if ( (pt.y > rc.top) && (pt.y < rc.bottom) )
      return min( abs( pt.x - rc.left ) , abs( rc.right - pt.x ) );
  }
  return -1;
}

CRect CEvadeWireGlyph::FindSuitableCornerDirection( CRect rcLink , CRect res , CRect res_measured , int evCnt , int xc , int yc , int start , int *increm , CGlyph *gl , bool tryDifferent )
{
  CRect rcLinkTemp( rcLink );
  int a1 , a2 , a3 , a4;
  a1 = abs( yc - res_measured.top );
  a2 = abs( res_measured.right - xc );
  a3 = abs( res_measured.bottom - yc );
  a4 = abs( xc - res_measured.left );
  res.top -= (evCnt + 1)*GetViewType().offset.y;
  res.bottom += (evCnt + 1)*GetViewType().offset.y;
  res.left -= (evCnt + 1)*GetViewType().offset.x;
  res.right += (evCnt + 1)*GetViewType().offset.x;
  int minim = min( min( a1 , a2 ) , min( a3 , a4 ) );
  int delta = max( GetViewType().offset.x , GetViewType().offset.y );
  CRect rc1;
  //   if (endArray.GetSize()>2)
  //     rc1=CRect(endArray.GetAt(endArray.GetSize()-2), endArray.GetAt(endArray.GetSize()-3));
  if ( start > 1 )
    rc1 = CRect( endArray.GetAt( start - 2 ) , endArray.GetAt( start - 1 ) );
  else
    rc1 = CRect();
  rc1.NormalizeRect();
  bool flag1 = false;
  CPoint prev;
  if ( start > 2 )
  {
    prev = endArray.GetAt( start - 2 );
    int temp = DistToRect( res , prev );
    if ( DistToRect( res , prev ) < 2 ) flag1 = true;
  }
  //  flag1=true;

  int incremental;
  if ( CheckMins( a1 , a2 , delta , minim )/* && flag1*/ ) //top-right
  {
    if ( flag1 )
    {
      if ( rc1.Width() > rc1.Height() )
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.right;
        incremental = 2;
      }
      else
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.left;
        incremental = 0;
      }
    }
    else
    {
      rcLinkTemp.bottom = res.top;
      rcLinkTemp.right = res.right;
      incremental = 1;
    }
  }
  else if ( CheckMins( a1 , a4 , delta , minim )/* && flag1*/ ) //top-left
  {
    if ( flag1 )
    {
      if ( rc1.Width() > rc1.Height() )
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.left;
        incremental = 3;
      }
      else
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.right;
        incremental = 1;
      }
    }
    else
    {
      rcLinkTemp.bottom = res.top;
      rcLinkTemp.right = res.left;
      incremental = 0;
    }
  }
  else if ( CheckMins( a2 , a3 , delta , minim )/* && flag1*/ ) //bottom-right
  {
    if ( flag1 )
    {
      if ( rc1.Width() > rc1.Height() )
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.right;
        incremental = 1;
      }
      else
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.left;
        incremental = 3;
      }
    }
    else
    {
      rcLinkTemp.bottom = res.bottom;
      rcLinkTemp.right = res.right;
      incremental = 2;
    }
  }
  else if ( CheckMins( a3 , a4 , delta , minim )/* && flag1*/ ) //bottom-left
  {
    if ( flag1 )
    {
      if ( rc1.Width() > rc1.Height() )
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.left;
        incremental = 0;
      }
      else
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.right;
        incremental = 2;
      }
    }
    else
    {
      rcLinkTemp.bottom = res.bottom;
      rcLinkTemp.right = res.left;
      incremental = 3;
    }
  }
  else if ( minim == a1 ) // 1 - top
  {
    if ( flag1 )
    {
      if ( (rcLinkTemp.left < xc && !tryDifferent) || (rcLinkTemp.left >= xc && tryDifferent) )
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.right;
        incremental = 1;
      }
      else
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.left;
        incremental = 0;
      }
    }
    else
    {
      bool flag = ((rcLinkTemp.right > rcLinkTemp.left) && (rcLinkTemp.bottom > rcLinkTemp.top)) || ((rcLinkTemp.right < rcLinkTemp.left) && (rcLinkTemp.bottom < rcLinkTemp.top));
      if ( (flag && !tryDifferent) || (!flag && tryDifferent) )
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.right;
        incremental = 1;
      }
      else
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.left;
        incremental = 0;
      }
    }
  }
  else if ( minim == a3 ) // 3 - bottom
  {
    if ( flag1 )
    {
      if ( (rcLinkTemp.left < xc && !tryDifferent) || (rcLinkTemp.left >= xc && tryDifferent) )
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.right;
        incremental = 2;
      }
      else
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.left;
        incremental = 3;
      }
    }
    else
    {
      bool flag = (((rcLinkTemp.right > rcLinkTemp.left) && (rcLinkTemp.bottom < rcLinkTemp.top)) || ((rcLinkTemp.right < rcLinkTemp.left) && (rcLinkTemp.bottom > rcLinkTemp.top)));
      if ( (flag && !tryDifferent) || (!flag && tryDifferent) )
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.right;
        incremental = 2;
      }
      else
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.left;
        incremental = 3;
      }
    }
  }
  else if ( minim == a2 ) // 2 - right
  {
    if ( flag1 )
    {
      if ( (rcLinkTemp.top < yc && !tryDifferent) || (rcLinkTemp.top >= yc && tryDifferent) )
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.right;
        incremental = 2;
      }
      else
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.right;
        incremental = 1;
      }
    }
    else
    {
      bool flag = (((rcLinkTemp.bottom > rcLinkTemp.top) && (rcLinkTemp.right < rcLinkTemp.left)) || ((rcLinkTemp.bottom < rcLinkTemp.top) && (rcLinkTemp.right > rcLinkTemp.left)));
      if ( (flag && !tryDifferent) || (!flag && tryDifferent) )
      {
        rcLinkTemp.right = res.right;
        rcLinkTemp.bottom = res.bottom;
        incremental = 2;
      }
      else
      {
        rcLinkTemp.right = res.right;
        rcLinkTemp.bottom = res.top;
        incremental = 1;
      }
    }
  }
  else if ( minim == a4 ) // 4 - left
  {
    if ( flag1 )
    {
      if ( (rcLinkTemp.top < yc && !tryDifferent) || (rcLinkTemp.top >= yc && tryDifferent) )
      {
        rcLinkTemp.bottom = res.bottom;
        rcLinkTemp.right = res.left;
        incremental = 3;
      }
      else
      {
        rcLinkTemp.bottom = res.top;
        rcLinkTemp.right = res.left;
        incremental = 0;
      }
    }
    else
    {
      bool flag = (((rcLinkTemp.bottom > rcLinkTemp.top) && (rcLinkTemp.right > rcLinkTemp.left)) || ((rcLinkTemp.bottom < rcLinkTemp.top) && (rcLinkTemp.right < rcLinkTemp.left)));
      if ( (flag && !tryDifferent) || (!flag && tryDifferent) )
      {
        rcLinkTemp.right = res.left;
        rcLinkTemp.bottom = res.bottom;
        incremental = 3;
      }
      else
      {
        rcLinkTemp.right = res.left;
        rcLinkTemp.bottom = res.top;
        incremental = 0;
      }
    }
  }
  gl->IncCornerCounter( incremental );
  *increm = incremental;
  return rcLinkTemp;
}

int CEvadeWireGlyph::CheckForRepeatCorners( int cornerCounter[ 4 ] )
{
  for ( int i = 0; i < 4; i++ )
  {
    if ( cornerCounter[ i ] >= 2 )
    {
      int count = 0;
      int j;
      for ( j = 0; j < 4; j++ )
      {
        if ( cornerCounter[ j ] > 0 )
        {
          count += cornerCounter[ j ] - 1;
        }
      }
      return count;
    }
  }
  return 0;
}

__forceinline double CEvadeWireGlyph::calcDTDummy( CRect rcLink )
{
  double dt;
  if ( rcLink.left - rcLink.right == 0 )
  {
    dt = 1.0 / (rcLink.bottom - rcLink.top);
  }
  else if ( rcLink.top - rcLink.bottom == 0 )
  {
    dt = 1.0 / (rcLink.right - rcLink.left);
  }
  else
  {
    double dt1 , dt2;
    dt1 = 1.0 / (rcLink.right - rcLink.left);
    dt2 = 1.0 / (rcLink.bottom - rcLink.top);
    if ( dt1 < 0 ) dt1 = -dt1;
    if ( dt2 < 0 ) dt2 = -dt2;
    dt = min( dt1 , dt2 );
  }
  if ( dt < 0 ) dt = -dt;
  return dt;
}

__forceinline double CEvadeWireGlyph::calcDTStraight( CRect rcLink )
{
  double dt;
  CRect rc_t = rcLink;
  rc_t.NormalizeRect();
  dt = 1.0 / (rc_t.right - rc_t.left + rc_t.bottom - rc_t.top);
  return dt;
}

int CEvadeWireGlyph::GetNextXcYcStraight_yFirst( CRect rcLink , double t , int &xc , int &yc )
{
  CRect rc_t = rcLink;
  rc_t.NormalizeRect();
  int halfperimeter = rc_t.right - rc_t.left + rc_t.bottom - rc_t.top;
  if ( t < (double) (rc_t.bottom - rc_t.top) / halfperimeter )
  {
    yc = roundI( halfperimeter*t / (rc_t.bottom - rc_t.top)*(rcLink.bottom - rcLink.top) + rcLink.top );
    xc = rcLink.left;
    return 0;
  }
  else
  {
    yc = rcLink.bottom;
    xc = roundI( halfperimeter*(t - (double) (rc_t.bottom - rc_t.top) / halfperimeter) / (rc_t.right - rc_t.left)*(rcLink.right - rcLink.left) + rcLink.left );
    return 1;
  }
  return 0;
}

int CEvadeWireGlyph::GetNextXcYcStraight_xFirst( CRect rcLink , double t , int &xc , int &yc )
{
  CRect rc_t = rcLink;
  rc_t.NormalizeRect();
  int halfperimeter = rc_t.right - rc_t.left + rc_t.bottom - rc_t.top;
  if ( t < (double) (rc_t.right - rc_t.left) / halfperimeter )
  {
    xc = roundI( halfperimeter*t / (rc_t.right - rc_t.left)*(rcLink.right - rcLink.left) + rcLink.left );
    yc = rcLink.top;
    return 0;
  }
  else
  {
    xc = rcLink.right;
    yc = roundI( halfperimeter*(t - (double) (rc_t.right - rc_t.left) / halfperimeter) / (rc_t.bottom - rc_t.top)*(rcLink.bottom - rcLink.top) + rcLink.top );
    return 1;
  }
  return 0;
}


int CEvadeWireGlyph::GetNextXcYcDummy( CRect rcLink , double t , int &xc , int &yc )
{
  xc = roundI( (rcLink.right - rcLink.left)*t + rcLink.left );
  yc = roundI( (rcLink.bottom - rcLink.top)*t + rcLink.top );
  return 0;
}

int sgn( int a )
{
  if ( a > 0 ) return 1;
  else if ( a < 0 ) return -1;
  else return 0;
}

int DistBetweenPointsStraight( CPoint p1 , CPoint p2 )
{
  return (abs( p1.x - p2.x ) + abs( p1.y - p2.y ));
}

void CEvadeWireGlyph::DummySearch()
{
  ((CSketchViewMod*) m_View)->ResetCornerCounters();
  endArray.RemoveAll();
  int start = 1;
  endArray.Add( ((CConnectorGlyph*) pInput)->GetConnectionPoint() );
  endArray.Add( ((CConnectorGlyph*) pOutput)->GetConnectionPoint() );
  CPoint pos = ((CConnectorGlyph*) pInput)->GetConnectionPoint();
  int sign;
  if ( ((CConnectorGlyph*) pInput)->GetFather()->GetPos().x < pos.x ) //output pin (located on the right)
    sign = 1;
  else
    sign = -1;
  for ( int k = GetViewType().offset.x; k > 0; k-- )
  {
    if ( !((CSketchViewMod*) m_View)->IsPointInGlyphRect( pos.x + sign * k , pos.y , NULL , GetViewType().offset.x , GetViewType().offset.y ) )
    {
      endArray.InsertAt( start , CPoint( pos.x + sign * k , pos.y ) );
      start++;
      break;
    }
  }
  CRect rcLink( endArray.GetAt( start - 1 ) , ((CConnectorGlyph*) pOutput)->GetConnectionPoint() );

  CRect rcLinkN;
  rcLinkN = rcLink; //normalized rectangle in case something gone bad.
  int xc , yc;
  xc = rcLink.left;
  yc = rcLink.right;
  CGlyph* endGlyph , *startGlyph;
  startGlyph = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLink.left , rcLink.top , NULL , GetViewType().offset.x , GetViewType().offset.y );
  endGlyph = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLink.right , rcLink.bottom , NULL , GetViewType().offset.x , GetViewType().offset.y , ((CConnectorGlyph*) pInput)->GetFather() );
  CGlyph *gl_old = NULL;
  int cornerCounter[ 4 ]; //evasion counter for each corner, 0 - UL, 1 - UR, 2 - LR, 3 - LL
  for ( int i = 0; i < 4; i++ ) cornerCounter[ i ] = 0;
  int start0;
  bool FirstX = false;
  bool ItWasX = false;

  do
  {
    double t = 0;
    double dt = calcDTStraight( rcLink );
    bool FirstTry = true;
    while ( t <= 1.0 )
    {
      CRect rc_t = rcLink;
      rc_t.NormalizeRect();
      rc_t.OffsetRect( -rc_t.TopLeft() );
      if ( rc_t.IsRectNull() )
      {
        t = 2.0;
        continue;
      }

      if ( abs( rcLink.left - rcLink.right )*GLYPH_GADGET_HEIGHT >
        GLYPH_GADGET_WIDTH*abs( rcLink.top - rcLink.bottom ) )
        FirstX = true;
      else
        FirstX = false;

      if ( FirstX )
        GetNextXcYcStraight_xFirst( rcLink , t , xc , yc );
      else
        GetNextXcYcStraight_yFirst( rcLink , t , xc , yc );
      CRect res , res_measured;
      CGlyph *gl;
      if ( (gl = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( xc , yc , &res_measured , GetViewType().offset.x , GetViewType().offset.y )) && (!FirstTry) )
      {

        int ev = gl->GetEvadeCount();
        if ( DistBetweenPointsStraight( rcLink.BottomRight() , CPoint( xc , yc ) ) <
          ev*max( GetViewType().offset.x , GetViewType().offset.y ) )
        {
          t += dt;
          continue;
        }

        if ( gl != gl_old )
        {
          for ( int i = 0; i < 4; i++ ) cornerCounter[ i ] = 0;
          start0 = start;
        }

        double cur_dt = -dt;
        CRgn rgn;
        gl->GetRgn( &rgn , TRUE );
        while ( (rgn.PtInRegion( CPoint( xc , yc ) )) && (t >= 0) )
        {
          if ( FirstX )
            GetNextXcYcStraight_xFirst( rcLink , t , xc , yc );
          else
            GetNextXcYcStraight_yFirst( rcLink , t , xc , yc );
          t += cur_dt;
        }

        res = CRect( ((CGadgetGlyph*) gl)->GetPos() , ((CGadgetGlyph*) gl)->GetSize() );
        if ( endGlyph != NULL )
        {
          FXString s1 , s2;
          endGlyph->GetUID( s1 );
          gl->GetUID( s2 );

          //           if (DistBetweenPointsStraight(rcLink.BottomRight(), CPoint(xc,yc)) <
          //             ev*max(GetViewType().offset.x, GetViewType().offset.y))
          //           {
          //             t+=dt;
          //             continue;
          //           }

          if ( !s1.Compare( s2 ) )
          {
            t = 2.0;
            gl_old = gl;
            continue;
          }
        }
        if ( !SearchEvadedGlyph( gl ) )
        {
          AddEvadedGlyph( (void*) gl );
          gl->AddEvadedGlyph( (void*) this );
          //gl->IncEvadeCount();
        }

        int a1 , a2 , a3 , a4;
        a1 = abs( yc - res_measured.top );
        a2 = abs( res_measured.right - xc );
        a3 = abs( res_measured.bottom - yc );
        a4 = abs( xc - res_measured.left );
        int minim = min( min( a1 , a2 ) , min( a3 , a4 ) );
        if ( minim == a1 || minim == a3 )
        {
          ItWasX = false;
        }
        else
        {
          ItWasX = true;
        }

        CRect rcLinkTemp1 , rcLinkTemp2;
        int inc1 , inc2;
        rcLinkTemp1 = FindSuitableCornerDirection( rcLink , res , res_measured , gl->GetEvadeCount() , xc , yc , start , &inc1 , gl , false );
        if ( !((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLinkTemp1.BottomRight() , NULL , GetViewType().offset.x , GetViewType().offset.y ) )
        {
          rcLink = rcLinkTemp1;
        }
        else
        {
          rcLinkTemp2 = FindSuitableCornerDirection( rcLink , res , res_measured , gl->GetEvadeCount() , xc , yc , start , &inc2 , gl , true );
          if ( !((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLinkTemp2.BottomRight() , NULL , GetViewType().offset.x , GetViewType().offset.y ) )
          {
            if ( (rcLink.IsRectEmpty()) || (gl->GetCornerCounter( inc2 ) > 1) )
            {
              rcLink = rcLinkTemp1;
              gl->DecCornerCounter( inc2 );
            }
            else
            {
              rcLink = rcLinkTemp2;
              gl->DecCornerCounter( inc1 );
            }
          }
          else
          {
            rcLink = rcLinkTemp1;
          }
        }
        endArray.InsertAt( start , rcLink.BottomRight() );

        if ( CheckForRepeatCorners( cornerCounter ) )
        {
          for ( int j = 0; j < CheckForRepeatCorners( cornerCounter ); j++ )
            endArray.RemoveAt( endArray.GetSize() - 2 - j );
          t = 2.0;
          continue;
        }

        startGlyph = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLink.left , rcLink.top , NULL , GetViewType().offset.x , GetViewType().offset.y );
        endGlyph = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLink.right , rcLink.bottom , NULL , GetViewType().offset.x , GetViewType().offset.y );
        FirstTry = true;
        dt = calcDTStraight( rcLink );
        t = -dt;
      }
      else
      {
        if ( (gl == NULL) && (FirstTry) )
          FirstTry = false;
        else if (/*(gl_old!=NULL) &&*/ (gl != startGlyph) && (FirstTry) )
        {
          FirstTry = false;
        }
      }
      if ( gl != NULL )
        gl_old = gl;
      t += dt;
    }
    //view->DrawLine(pDC, rcLink, &pen);
    if ( !FirstX )
      endArray.InsertAt( start , CPoint( rcLink.left , rcLink.bottom ) );
    else
      endArray.InsertAt( start , CPoint( rcLink.right , rcLink.top ) );
    start++;

    rcLink.top = rcLink.bottom;
    rcLink.left = rcLink.right;
    if ( start + 1 < endArray.GetSize() )
    {
      CPoint tempP = endArray.GetAt( start + 1 );
      start++;
      //endArray.RemoveAt(endArray.GetSize()-1);
      rcLink.bottom = tempP.y;
      rcLink.right = tempP.x;
      startGlyph = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLink.left , rcLink.top , NULL , GetViewType().offset.x , GetViewType().offset.y );
      if ( start == endArray.GetSize() - 1 )
      {
        if ( rcLink.left - (((CConnectorGlyph*) pInput)->GetFather()->GetPos()).x < 0 )
          endGlyph = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLink.right , rcLink.bottom , NULL , GetViewType().offset.x , GetViewType().offset.y , false );
        else
          endGlyph = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLink.right , rcLink.bottom , NULL , GetViewType().offset.x , GetViewType().offset.y , (((CConnectorGlyph*) pOutput)->GetFather()) );
      }
      else
      {
        ((CSketchViewMod*) m_View)->IsPointInGlyphRect( rcLink.right , rcLink.bottom , NULL , GetViewType().offset.x , GetViewType().offset.y , false );
      }
      //      endGlyph=((CSketchViewMod*)view)->IsPointInGlyphRect(rcLink.right, rcLink.bottom, GLYPH_CONNECTOR_OFFSET_X, GLYPH_CONNECTOR_OFFSET_Y, (start==endArray.GetSize()-1) ? (((CEvadeWireGlyph*)(pComplementary))->father) : (false) );
    }

    //     if (abs(rcLink.right-rcLink.left)>abs(rcLink.bottom-rcLink.top))
    //       FirstX=false;
    //     else
    //       FirstX=true;

    /*    FirstX=!FirstX;*/

    //     if (ItWasX)
    //       FirstX=true;
    //     else
    //       FirstX=false;

    if ( endArray.GetSize() > 1000 )
    {
      while ( endArray.GetSize() > 2 )
        endArray.RemoveAt( 1 );
      break;
    }

    rcLinkN = rcLink;
    rcLinkN.NormalizeRect();
    rcLinkN -= CPoint( rcLinkN.left , rcLinkN.top );
  } while ( !rcLinkN.IsRectNull() );

  for ( int l = 0; l < GetEvadedGlyphSize(); l++ )
  {
    CGlyph *gl;
    gl = (CGlyph*) GetEvadedGlyph( l );
    gl->IncEvadeCount();
  }

  CPoint p1 , p2 , p3;
  int i1 = 2;
  while ( i1 < endArray.GetSize() )
  {
    p1 = endArray.GetAt( i1 - 2 );
    p2 = endArray.GetAt( i1 - 1 );
    p3 = endArray.GetAt( i1 );
    if ( p1.y == p2.y && p2.y == p3.y )
    {
      if ( sgn( p1.x - p2.x ) != sgn( p2.x - p3.x ) )
      {
        endArray.RemoveAt( i1 - 1 );
        i1--;
      }
    }
    else if ( p1.x == p2.x && p2.x == p3.x )
    {
      if ( sgn( p1.y - p2.y ) != sgn( p2.y - p3.y ) )
      {
        endArray.RemoveAt( i1 - 1 );
        i1--;
      }
    }
    i1++;
  }
}

//track the way, using simple decision-making, obstacle-piercing and, afterwards, track optimization
//description of algorithm can be found in teamlead's correspondence
void CEvadeWireGlyph::MySearch()
{
  int counter;
  CGlyph *pGlyph = NULL;
  CPoint input = ((CConnectorGlyph*) pInput)->GetConnectionPoint();
  CPoint output = ((CConnectorGlyph*) pOutput)->GetConnectionPoint();
  CPoint temp;

  //artifact
  ((CSketchViewMod*) m_View)->ResetCornerCounters();

  //clear massive of waypoints
  endArray.RemoveAll();

  //if input or output are blocked - push them to the open space
  pGlyph = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( input , NULL , GetViewType().offset.x , GetViewType().offset.y );
  if ( pGlyph )
    input = PierceObstacleShortest( (CSketchViewMod*) m_View , &input );
  //insert changed input
  endArray.Add( input );

  pGlyph = NULL;
  pGlyph = ((CSketchViewMod*) m_View)->IsPointInGlyphRect( output , NULL , GetViewType().offset.x , GetViewType().offset.y );
  if ( pGlyph )
    output = PierceObstacleShortest( (CSketchViewMod*) m_View , &output );

  //start
  counter = 0;
  while ( true )
  {
    //terminating cycle: can't find path fast enough
    if ( counter > COUNTER_LIMIT_LEVEL3 )
    {
      //simply coonect to output
      temp = endArray.GetAt( endArray.GetCount() - 1 );
      temp.x = output.x;
      endArray.Add( temp );
      break;
    }
    //get to final horizon
    counter += GetToHorizon( (CSketchViewMod*) m_View , endArray.GetAt( endArray.GetCount() - 1 ) , output );
    //attempt to reach output
    counter += SlideToPoint( (CSketchViewMod*) m_View , endArray.GetAt( endArray.GetCount() - 1 ) , output );

    if ( endArray.GetAt( endArray.GetCount() - 1 ) == output )
      break;
    //if we're here, means we had to pierce an impenetrable obstacle, so now
    //we're in another connected component, and need to start from the beginning
    counter++;
  }

  //optimize the track
  OptimizeTrack( (CSketchViewMod*) m_View );

  //if we moved start/endpoint in the beginning, or if that was a termination, we need to insert real input and ountput
  if ( input != ((CConnectorGlyph*) pInput)->GetConnectionPoint() )
    endArray.InsertAt( 0 , ((CConnectorGlyph*) pInput)->GetConnectionPoint() );
  if ( output != ((CConnectorGlyph*) pOutput)->GetConnectionPoint() || endArray.GetAt( endArray.GetCount() - 1 ) != ((CConnectorGlyph*) pOutput)->GetConnectionPoint() )
    endArray.Add( ((CConnectorGlyph*) pOutput)->GetConnectionPoint() );

  //only AFTER completing the track we increse evadecounters to prevent unnecessary cycling etc.
  for ( int l = 0; l < GetEvadedGlyphSize(); l++ )
  {
    CGlyph *gl;
    gl = (CGlyph*) GetEvadedGlyph( l );
    gl->IncEvadeCount();
  }
}

//this function attempts to deliver current trackpoint to the final (output) horizon
int CEvadeWireGlyph::GetToHorizon( CSketchViewMod *view , CPoint input , CPoint output )
{
  int iMoveDir = 0;
  int iOldMoveDir = 0;
  int counter = 0;
  CPoint pos = input;
  CGlyph * pGlyph = NULL;
  bool bClockwise;
  while ( true )
  {
    //emergency termiation: possible endless cycling
    if ( counter > COUNTER_LIMIT_LEVEL2 )
      return COUNTER_LIMIT_LEVEL2;

    //move vertically till we reach output horizon or an obstacle
    iMoveDir = pos.y < output.y ? SEARCH_DOWN : SEARCH_UP;
    pos = MoveVertToObstacle( (CSketchViewMod*) view , &pos , iMoveDir , output.y );
    endArray.Add( pos );

    if ( pos.y != output.y )
    {
      //we've met an obstacle: bypass it from "inner" side
      bClockwise = (iMoveDir == SEARCH_UP && pos.x >= output.x) || (iMoveDir == SEARCH_DOWN && pos.x < output.x) ? TRUE : FALSE;
      iMoveDir = pos.x < output.x ? SEARCH_RIGHT : SEARCH_LEFT;
      if ( iOldMoveDir != 0 )
      {
        //to avoid "pendulum", direction of bypassing must be chosen once and forever
        if ( iMoveDir != iOldMoveDir )
          bClockwise = !bClockwise;
        iMoveDir = iOldMoveDir;
      }
      else
        iMoveDir = pos.x < output.x ? SEARCH_RIGHT : SEARCH_LEFT;
      iOldMoveDir = iMoveDir;
      pos = BypassObstacleVert( (CSketchViewMod*) view , &pos , iMoveDir , bClockwise , output );
    }
    else
      break;
    endArray.Add( pos );
    counter++;
  }
  return 0;
}

//function attempts to pave to the output point.
//it is assumed that current position and output have the same y-coordinate
int CEvadeWireGlyph::SlideToPoint( CSketchViewMod *view , CPoint input , CPoint output )
{
  int iMoveDir = 0;
  int iOldMoveDir = 0;
  CPoint pos = input;
  CGlyph * pGlyph = NULL;
  bool bClockwise;
  CPoint next;
  int counter = 0;

  while ( true )
  {
    if ( counter > COUNTER_LIMIT_LEVEL2 )
      return COUNTER_LIMIT_LEVEL2;
    //if we are not on the final horizon - means we've just pierced an obstacle and need to get back
    //to moving vertically
    if ( pos.y != output.y )
      return 0;

    if ( pos.x == output.x )
      //we made it!
      break;

    iMoveDir = pos.x < output.x ? SEARCH_RIGHT : SEARCH_LEFT;
    pos = MoveHorzToObstacle( (CSketchViewMod*) view , &pos , iMoveDir , output.x );
    endArray.Add( pos );

    if ( pos.x != output.x )
    {
      //we found an obstacle - we need to bypass it (till we get back to final horizon)
      if ( iOldMoveDir != 0 )
        //to avoid "pendulum", direction of bypassing must be chosen once and forever
        iMoveDir = iOldMoveDir;
      else
        iMoveDir = input.y < output.y ? SEARCH_UP : SEARCH_DOWN;
      bClockwise = (iMoveDir == SEARCH_DOWN && pos.x > output.x) || (iMoveDir == SEARCH_UP && pos.x < output.x) ? TRUE : FALSE;
      next = BypassObstacleHorz( (CSketchViewMod*) view , &pos , iMoveDir , bClockwise , output );
      if ( (pos.x > output.x && next.x < output.x) || (pos.x < output.x && next.x > output.x) )
      {
        //overshoot: we perform another attempt, and either get closer, or detect a cycle (unpenetrable obstacle)
        endArray.Add( next );
        iMoveDir = iMoveDir == SEARCH_UP ? SEARCH_DOWN : SEARCH_UP;
        pos = BypassObstacleHorz( (CSketchViewMod*) view , &next , iMoveDir , bClockwise , output );
      }
      else
        pos = next;
    }
    else
      break;
    endArray.Add( pos );
    counter++;
  }
  return 0;
}

//function performs possible track optimizing by removing unnecessary turns, flips, redundant points etc.
void CEvadeWireGlyph::OptimizeTrack( CSketchViewMod * view )
{
  int startPoint = 0;
  int iMoveDir = 0;
  CPoint temp;
  CPoint temp2;
  bool bSuccess;
  while ( startPoint + 2 < endArray.GetCount() )
  {
    CPoint p0 = endArray.GetAt( startPoint );
    CPoint p1 = endArray.GetAt( startPoint + 1 );
    CPoint p2 = endArray.GetAt( startPoint + 2 );
    CPoint p3;
    //redundant point
    if ( (p0.x == p1.x && p1.x == p2.x) || (p0.y == p1.y && p1.y == p2.y) )
    {
      endArray.RemoveAt( startPoint + 1 );
      if ( startPoint > 0 )
        startPoint--;
      continue;
    }

    bSuccess = FALSE;
    //patch: another redundant point: need to check to avoid mistakes
    if ( startPoint + 3 < endArray.GetCount() )
    {
      p3 = endArray.GetAt( startPoint + 3 );
      if ( (p1.x == p2.x && p2.x == p3.x) || (p1.y == p2.y && p2.y == p3.y) )
      {
        endArray.RemoveAt( startPoint + 2 );
        continue;
      }
    }

    //remove unnecessary turns and flips: the idea is simple;
    //pensil and paper will help if you're stuck here
    if ( p0.x == p1.x )
    {

      iMoveDir = p0.x < p2.x ? SEARCH_RIGHT : SEARCH_LEFT;
      temp = MoveHorzToObstacle( (CSketchViewMod*) view , &p0 , iMoveDir , p2.x );
      if ( temp.x == p2.x )
      {
        iMoveDir = p0.y > p2.y ? SEARCH_UP : SEARCH_DOWN;
        temp2 = MoveVertToObstacle( (CSketchViewMod*) view , &temp , iMoveDir , p2.y );
        if ( temp2.y == p2.y )
          bSuccess = TRUE;
      }
    }
    else
    {
      if ( p1.y == p2.y )
      {
        endArray.RemoveAt( startPoint + 1 );
        continue;
      }
      iMoveDir = p0.y > p2.y ? SEARCH_UP : SEARCH_DOWN;
      temp = MoveVertToObstacle( (CSketchViewMod*) view , &p0 , iMoveDir , p2.y );
      if ( temp.y == p2.y )
      {
        iMoveDir = p0.x < p2.x ? SEARCH_RIGHT : SEARCH_LEFT;
        temp2 = MoveHorzToObstacle( (CSketchViewMod*) view , &temp , iMoveDir , p2.x );
        if ( temp2.x == p2.x )
          bSuccess = TRUE;
      }
    }
    if ( bSuccess )
    {
      if ( (startPoint + 3) != endArray.GetCount() )
        endArray.RemoveAt( startPoint + 2 );
      endArray.RemoveAt( startPoint + 1 );
      if ( startPoint != 0 )
      {
        endArray.RemoveAt( startPoint );
        endArray.InsertAt( startPoint , temp );
        startPoint--;
      }
      else
      {
        endArray.InsertAt( startPoint + 1 , temp );
        if ( startPoint + 3 == endArray.GetCount() )
          break;
      }
    }
    else
      startPoint++;
  }
}

//the function simply moves up or down till it meets an obstacle
CPoint CEvadeWireGlyph::MoveVertToObstacle( CSketchViewMod *view , CPoint *pPos , int startDir , LONG limit )
{
  CPoint end( pPos->x , pPos->y );
  CGadgetGlyph *pGlyph = NULL;
  CRgn rgn;
  if ( startDir == SEARCH_DOWN )
    while ( true )
    {
      end.y += GLYPH_MIN_HEIGHT;
      if ( end.y > limit )
        end.y = limit;
      pGlyph = (CGadgetGlyph *) view->IsPointInGlyphRect( end , NULL , GetViewType().offset.x , GetViewType().offset.y );
      if ( pGlyph )
      {
        end = PierceObstacle( view , &end , SEARCH_UP );
        break;
      }
      if ( end.y == limit )
        break;
    }
  else
    while ( true )
    {
      end.y -= GLYPH_MIN_HEIGHT;
      if ( end.y < limit )
        end.y = limit;
      pGlyph = (CGadgetGlyph *) view->IsPointInGlyphRect( end , NULL , GetViewType().offset.x , GetViewType().offset.y );
      if ( pGlyph )
      {
        end = PierceObstacle( view , &end , SEARCH_DOWN );
        break;
      }
      if ( end.y == limit )
        break;
    }
  return end;
}

//the function simply moves left or right till it meets an obstacle
CPoint CEvadeWireGlyph::MoveHorzToObstacle( CSketchViewMod *view , CPoint *pPos , int startDir , LONG limit )
{
  CPoint end( pPos->x , pPos->y );
  CGadgetGlyph *pGlyph = NULL;
  CRgn rgn;
  if ( startDir == SEARCH_RIGHT )
    while ( true )
    {
      end.x += GLYPH_MIN_WIDTH;
      if ( end.x > limit )
        end.x = limit;
      pGlyph = (CGadgetGlyph *) view->IsPointInGlyphRect( end , NULL , GetViewType().offset.x , GetViewType().offset.y );
      if ( pGlyph )
      {
        end = PierceObstacle( view , &end , SEARCH_LEFT );
        break;
      }
      if ( end.x == limit )
        break;
    }
  else //left
    while ( true )
    {
      end.x -= GLYPH_MIN_WIDTH;
      if ( end.x < limit )
        end.x = limit;
      pGlyph = (CGadgetGlyph *) view->IsPointInGlyphRect( end , NULL , GetViewType().offset.x , GetViewType().offset.y );
      if ( pGlyph )
      {
        end = PierceObstacle( view , &end , SEARCH_RIGHT );
        break;
      }
      if ( end.x == limit )
        break;
    }
  return end;
}

//the function bypasses ah obstacle, moving bClockwise, starting in startDir
//it returns, when the obstacle is bypassed and horizon is unchanged
//emergency exit: detected cycle and pierced an obstacle, or termination
CPoint CEvadeWireGlyph::BypassObstacleHorz( CSketchViewMod *view , CPoint *pPos , int startDir , bool bClockwise , CPoint output )
{
  CPoint end( pPos->x , pPos->y );
  CPoint next;
  CGadgetGlyph *pGlyph = NULL;
  CGadgetGlyph *pOldGlyph = NULL;
  int curDir = startDir;
  int counter = 0;
  while ( true )
  {
    //emergency termination
    counter++;
    if ( counter > COUNTER_LIMIT_LEVEL1 )
      return end;

    next = end;
    //check if we can move further
    MovePoint( &next , curDir );
    pGlyph = (CGadgetGlyph *) view->IsPointInGlyphRect( next , NULL , GetViewType().offset.x , GetViewType().offset.y );
    if ( pGlyph )
    {
      //means we need to turn !bClockwise
      if ( pGlyph != pOldGlyph )
      {
        pOldGlyph = pGlyph;
        AddEvadedGlyph( pGlyph );
      }
      curDir = GetNextDirection( curDir , !bClockwise );
      pGlyph = NULL;
      endArray.Add( end );
      //check for running cycles
      next = CheckForCycles( view , output );
      if ( next != end )
        return next;
      continue;
    }
    //make a step
    end = next;
    //check if we're done bypassing
    if ( end.y == pPos->y && curDir == GetNextDirection( GetNextDirection( startDir , TRUE ) , TRUE ) )
      break;
    //check if we nned to turn - to stay near the obstacle
    next = end;
    MovePoint( &next , GetNextDirection( curDir , bClockwise ) );
    pGlyph = (CGadgetGlyph *) view->IsPointInGlyphRect( next , NULL , GetViewType().offset.y , GetViewType().offset.y );
    if ( !pGlyph )
    {
      curDir = GetNextDirection( curDir , bClockwise );
      endArray.Add( end );
      //check for running cycles
      next = CheckForCycles( view , output );
      if ( next != end )
        return next;
      continue;
    }
    if ( pGlyph != pOldGlyph )
    {
      pOldGlyph = pGlyph;
      AddEvadedGlyph( pGlyph );
    }
    pGlyph = NULL;
  }
  return end;
}

//the function bypasses ah obstacle, moving bClockwise, starting in startDir
//it returns, when the obstacle is bypassed and moving direction is the same (like falling water, if startDir was DOWN, for example)
//emergency exit: detected cycle and pierced an obstacle, or termination
CPoint CEvadeWireGlyph::BypassObstacleVert( CSketchViewMod *view , CPoint *pPos , int startDir , bool bClockwise , CPoint output )
{
  CPoint end( pPos->x , pPos->y );
  CPoint next;
  CGadgetGlyph *pGlyph = NULL;
  CGadgetGlyph *pOldGlyph = NULL;
  int curDir = startDir;
  int counter = 0;
  while ( true )
  {
    counter++;
    if ( counter > COUNTER_LIMIT_LEVEL1 )
      return end;
    next = end;

    //Check if we can move further
    MovePoint( &next , curDir );
    pGlyph = (CGadgetGlyph *) view->IsPointInGlyphRect( next , NULL , GetViewType().offset.x , GetViewType().offset.y );
    //if not, we need to turn !bClockwise
    if ( pGlyph )
    {
      if ( pGlyph != pOldGlyph )
      {
        pOldGlyph = pGlyph;
        AddEvadedGlyph( pGlyph );
      }
      curDir = GetNextDirection( curDir , !bClockwise );
      pGlyph = NULL;
      endArray.Add( end );
      //check for running cycles
      next = CheckForCycles( view , output );
      if ( next != end )
        return next;
      continue;
    }
    //make a step
    end = next;
    //check if we're done bypassing: flow round, the same direction
    if ( curDir == startDir )
    {
      MovePoint( &next , GetNextDirection( curDir , bClockwise ) );
      pGlyph = (CGadgetGlyph *) view->IsPointInGlyphRect( next , NULL , GetViewType().offset.x , GetViewType().offset.y );
      //means we're free to move vertically in the desired direction
      if ( !pGlyph )
        break;
      pGlyph = NULL;
    }
    //check if we need to turn
    next = end;
    MovePoint( &next , GetNextDirection( curDir , bClockwise ) );
    pGlyph = (CGadgetGlyph *) view->IsPointInGlyphRect( next , NULL , GetViewType().offset.x , GetViewType().offset.y );
    if ( !pGlyph )
    {
      curDir = GetNextDirection( curDir , bClockwise );
      endArray.Add( end );
      //check for running cycles
      next = CheckForCycles( view , output );
      if ( next != end )
        return next;
      continue;
    }
    if ( pGlyph != pOldGlyph )
    {
      pOldGlyph = pGlyph;
      AddEvadedGlyph( pGlyph );
    }
    pGlyph = NULL;

  }
  return end;
}

//pierce an obstacle in the shortest possible direction
CPoint CEvadeWireGlyph::PierceObstacleShortest( CSketchViewMod *view , CPoint *pPos )
{
  int length = 0;
  CPoint end;
  CPoint temp;
  temp = PierceObstacle( view , pPos , SEARCH_UP );
  length = pPos->y - temp.y;
  end = temp;
  temp = PierceObstacle( view , pPos , SEARCH_DOWN );
  if ( temp.y - pPos->y < length )
  {
    length = temp.y - pPos->y;
    end = temp;
  }
  temp = PierceObstacle( view , pPos , SEARCH_LEFT );
  if ( pPos->x - temp.x < length )
  {
    length = pPos->x - temp.x;
    end = temp;
  }
  temp = PierceObstacle( view , pPos , SEARCH_RIGHT );
  if ( temp.x - pPos->x < length )
    end = temp;
  return end;
}

//pierce an obstacle in the chosen direction
CPoint CEvadeWireGlyph::PierceObstacle( CSketchViewMod *view , CPoint *pPos , int startDir )
{
  CGlyph *pGlyph = NULL;
  CPoint end( pPos->x , pPos->y );
  switch ( startDir )
  {
    case SEARCH_LEFT:
      while ( true )
      {
        end.x--;
        pGlyph = view->IsPointInGlyphRect( end , NULL , GetViewType().offset.x , GetViewType().offset.y );
        if ( !pGlyph )
          break;
      }
      break;
    case SEARCH_DOWN:
      while ( true )
      {
        end.y++;
        pGlyph = view->IsPointInGlyphRect( end , NULL , GetViewType().offset.x , GetViewType().offset.y );
        if ( !pGlyph )
          break;
      }
      break;
    case SEARCH_UP:
      while ( true )
      {
        end.y--;
        pGlyph = view->IsPointInGlyphRect( end , NULL , GetViewType().offset.x , GetViewType().offset.y );
        if ( !pGlyph )
          break;
      }
      break;
    case SEARCH_RIGHT:
      while ( true )
      {
        end.x++;
        pGlyph = view->IsPointInGlyphRect( end , NULL , GetViewType().offset.x , GetViewType().offset.y );
        if ( !pGlyph )
          break;
      }
      break;
  }
  return end;
}

//check, if endArray contains cycles
CPoint CEvadeWireGlyph::CheckForCycles( CSketchViewMod *view , CPoint output )
{
  int i = 0;
  int j = 0;
  int iDirection = 0;
  INT_PTR last = endArray.GetCount() - 1;
  CPoint lastPoint = endArray.GetAt( last );
  CPoint next;
  CPoint nearest;
  int iNearest;
  for ( i = 0; i < endArray.GetCount(); i++ )
    if ( endArray.GetAt( i ) == endArray.GetAt( last ) )
      break;
  if ( i < last )
  {
    //found a cycle
    nearest = endArray.GetAt( i );
    iNearest = i;
    //we want to determine the best point to start piercing: nearest to the output
    for ( j = i; j < endArray.GetCount(); j++ )
      if ( abs( nearest.x - output.x ) + abs( nearest.y - output.y ) > abs( endArray.GetAt( j ).x - output.x ) + abs( endArray.GetAt( j ).y - output.y ) )
      {
        nearest = endArray.GetAt( j );
        iNearest = j;
      }
    //remove all the points after nearest
    while ( endArray.GetCount() > iNearest + 1 )
      endArray.RemoveAt( iNearest + 1 );
    //determine direction of piercing
    next = nearest;
    if ( abs( nearest.x - output.x ) > abs( nearest.y - output.y ) )
      iDirection = nearest.x < output.x ? SEARCH_RIGHT : SEARCH_LEFT;
    else
      iDirection = nearest.y < output.y ? SEARCH_DOWN : SEARCH_UP;
  }
  if ( iDirection != 0 )
    //cycle found and cleant: now piercing
    return PierceObstacle( view , &nearest , iDirection );
  else
    //no cycles, do nothing
    return lastPoint;
}

//move a point in the chosen direction
void CEvadeWireGlyph::MovePoint( CPoint *pPt , int iDir )
{
  switch ( iDir )
  {
    case SEARCH_LEFT:
      pPt->x--;
      break;
    case SEARCH_DOWN:
      pPt->y++;
      break;
    case SEARCH_UP:
      pPt->y--;
      break;
    case SEARCH_RIGHT:
      pPt->x++;
      break;
  }
}

//get next direction to chosen - eithe clockwise or counterclockwise
int CEvadeWireGlyph::GetNextDirection( int iDir , bool bClockwise )
{
  switch ( iDir )
  {
    case SEARCH_LEFT:
      return bClockwise ? SEARCH_UP : SEARCH_DOWN;
      break;
    case SEARCH_DOWN:
      return bClockwise ? SEARCH_LEFT : SEARCH_RIGHT;
      break;
    case SEARCH_UP:
      return bClockwise ? SEARCH_RIGHT : SEARCH_LEFT;
      break;
    case SEARCH_RIGHT:
      return bClockwise ? SEARCH_DOWN : SEARCH_UP	;
      break;
  }
  return 0;
}

void CEvadeWireGlyph::StraightSearch()
{
  const int CDX = GLYPH_GADGET_WIDTH / 2 , CDY = GLYPH_GADGET_HEIGHT / 2;
  int dx , dy;
  enum tDir
  {
    UP , LEFT , DOWN , RIGHT
  };
  CArray <int , int> dir; //0 - up, 1 - left, 2 - down, 3 - right
  dir.Add( UP );
  dx = 0; dy = -CDY;
  CPoint pc = endArray.GetAt( 0 );
  while ( 1 )
  {
    pc += (CPoint) (dx , dy);
    {

    }
  }

}

void CEvadeWireGlyph::DrawGlyph( CDC* pDC )
{
  LOGPEN pen;
  if ( m_View->GetMyPen( this , &pen ) )
  {
    if ( !(GetEvadeCount()) )
    {
      MySearch();
      IncEvadeCount();
    }

    for ( int i = 0; i < endArray.GetSize() - 1; i++ )
    {
      m_View->DrawLine( pDC , endArray.GetAt( i ) , endArray.GetAt( i + 1 ) , &pen );
    }

  }
}

BOOL CEvadeWireGlyph::GetRgn( CRgn* pRgn , BOOL bSelfRgn /* = FALSE */ )
{
  if ( !pRgn->m_hObject )
  {
    pRgn->CreateRectRgnIndirect( CRect( ((CConnectorGlyph*) pInput)->GetPos() , CSize( 0 , 0 ) ) );
  }
  if ( GetViewType().m_EvType == ViewType::EVADE_CLASSIC )
    return CWireGlyph::GetRgn( pRgn , bSelfRgn );
  else if ( GetViewType().m_EvType == ViewType::EVADE_MODERN )
    return _GetRgn( pRgn , bSelfRgn );
  return FALSE;
}

BOOL CEvadeWireGlyph::_GetRgn( CRgn* pRgn , BOOL bSelfRgn /* = FALSE */ )
{
  bSelfRgn = FALSE;
  if ( !bSelfRgn )
  {
    int i = 0;
    CArray<CPoint , CPoint> *endArray_loc = NULL;
    if ( endArray.GetSize() > 0 )
    {
      endArray_loc = &endArray;
    }

    if ( endArray_loc != NULL )
    {
      while ( i + 1 < endArray_loc->GetSize() )
      {
        const int marg = CLICK_MARGIN;
        CPoint pt[ 4 ];
        pt[ 0 ] = endArray_loc->GetAt( i );
        pt[ 1 ] = endArray_loc->GetAt( i + 1 );
        CPoint v = pt[ 1 ] - pt[ 0 ];
        int dx = (v.x > 0) ? v.x : -v.x;
        int dy = (v.y > 0) ? v.y : -v.y;
        int d = (dx > dy) ? dx : dy;
        if ( !d ) return TRUE;
        CPoint n( -(v.y * marg) / d , (v.x * marg) / d );
        pt[ 2 ] = pt[ 1 ] - n;
        pt[ 3 ] = pt[ 0 ] - n;
        pt[ 0 ] += n;
        pt[ 1 ] += n;
        CRgn rgn;
        rgn.CreatePolygonRgn( pt , 4 , ALTERNATE );
        pRgn->CombineRgn( pRgn , &rgn , RGN_OR );
        rgn.DeleteObject();
        i++;
      }
    }
    else
    {
      const int marg = CLICK_MARGIN;
      CPoint pt[ 4 ];
      pt[ 0 ] = ((CConnectorGlyph*) pInput)->GetConnectionPoint(); //endArray_loc->GetAt(i);
      pt[ 1 ] = ((CConnectorGlyph*) pOutput)->GetConnectionPoint(); //endArray_loc->GetAt(i+1);
      CPoint v = pt[ 1 ] - pt[ 0 ];
      int dx = (v.x > 0) ? v.x : -v.x;
      int dy = (v.y > 0) ? v.y : -v.y;
      int d = (dx > dy) ? dx : dy;
      if ( !d ) return TRUE;
      CPoint n( -(v.y * marg) / d , (v.x * marg) / d );
      pt[ 2 ] = pt[ 1 ] - n;
      pt[ 3 ] = pt[ 0 ] - n;
      pt[ 0 ] += n;
      pt[ 1 ] += n;
      CRgn rgn;
      rgn.CreatePolygonRgn( pt , 4 , ALTERNATE );
      pRgn->CombineRgn( pRgn , &rgn , RGN_OR );
      rgn.DeleteObject();
      i++;
    }
  }
  return TRUE;
}

void CEvadeWireGlyph::SetEndArrayRev( CArray<CPoint , CPoint> *eA )
{
  endArray.RemoveAll();
  for ( int i = 0; i < eA->GetSize(); i++ )
  {
    endArray.Add( eA->GetAt( eA->GetSize() - 1 - i ) );
  }
}

BOOL CEvadeWireGlyph::GetLocalRgn( CRgn *pRgn , int i , BOOL bSelf )
{
  if ( !bSelf )
  {
    //    int i=0;
    if ( i + 1 < endArray.GetSize() )
    {
      const int marg = CLICK_MARGIN;
      CPoint pt[ 4 ];
      pt[ 0 ] = endArray.GetAt( i );
      pt[ 1 ] = endArray.GetAt( i + 1 );
      CPoint v = pt[ 1 ] - pt[ 0 ];
      int dx = (v.x > 0) ? v.x : -v.x;
      int dy = (v.y > 0) ? v.y : -v.y;
      int d = (dx > dy) ? dx : dy;
      if ( !d ) return TRUE;
      CPoint n( -(v.y * marg) / d , (v.x * marg) / d );
      pt[ 2 ] = pt[ 1 ] - n;
      pt[ 3 ] = pt[ 0 ] - n;
      pt[ 0 ] += n;
      pt[ 1 ] += n;
      CRgn rgn;
      pRgn->CreatePolygonRgn( pt , 4 , ALTERNATE );
      //pRgn->CopyRgn(&rgn);
      rgn.DeleteObject();
      i++;
    }
  }
  return TRUE;
}

// CConnectorGlyph
CConnectorGlyph::CConnectorGlyph( CSketchView *view , LPCTSTR uid ) :
  CCompositeGlyph( view ) ,
  m_UID( uid ) ,
  m_Size( GLYPH_CONNECTOR_WIDTH , GLYPH_CONNECTOR_HEIGHT ) ,
  m_bHotSpot( false ) ,
  m_pDebugWnd( NULL ) ,
  m_pConnector( NULL )
{
  ASSERT( uid && *uid ) ;
}

CConnectorGlyph::CConnectorGlyph( CSketchView *view , LPCTSTR uid , CGlyph *f ) :
  CCompositeGlyph( view ) ,
  m_UID( uid ) ,
  father( f ) ,
  m_Size( GLYPH_CONNECTOR_WIDTH , GLYPH_CONNECTOR_HEIGHT ) ,
  m_bHotSpot( false ) ,
  m_pDebugWnd(NULL),
  m_pConnector(NULL)
{
  ASSERT( uid && *uid ) ;
}

CConnectorGlyph::CConnectorGlyph( CSketchView *view , LPCTSTR uid , ViewType vt ) :
  CCompositeGlyph( view ) ,
  m_UID( uid ) ,
  m_Size( GLYPH_CONNECTOR_WIDTH , GLYPH_CONNECTOR_HEIGHT ) ,
  m_bHotSpot( false ) ,
  m_pDebugWnd(NULL) ,
  m_pConnector(NULL)
{
  ASSERT( uid && *uid ) ;
  SetViewType( vt );
}

CConnectorGlyph::CConnectorGlyph( CSketchView *view , LPCTSTR uid , ViewType vt , CGlyph *f ) :
  CCompositeGlyph( view ) ,
  m_UID( uid ) ,
  father( f ) ,
  m_Size( GLYPH_CONNECTOR_WIDTH , GLYPH_CONNECTOR_HEIGHT ) ,
  m_bHotSpot( false ) ,
  m_pDebugWnd(NULL),
  m_pConnector(NULL)
{
  ASSERT( uid && *uid ) ;
  SetViewType( vt );
}

void CConnectorGlyph::Draw( CDC* pDC , bool bActivityOnly )
{
  if ( GetConnector()->IsVisible() )
  {
    //if( !bActivityOnly )
    //{
        CRect rc( GetPos() , m_Size );
        if ( m_bHotSpot )
          rc.InflateRect( 2 , 2 );
        COLORREF crBorder , crBody;
        if ( m_View->GetMyRectColors( this , &crBorder , &crBody , GetDataPassed() ) )
        {
        #ifdef GLYPHS_DRAW_PLAIN
          m_View->DrawRect( pDC , rc , crBorder , crBody );
        #elif defined GLYPHS_DRAW_ROUND
          rc.InflateRect( 1 , 1 );
          m_View->DrawRoundRect( pDC , rc , crBorder , crBody );
        #elif defined GLYPHS_DRAW_EDGE
          rc.InflateRect( 1 , 1 );
          m_View->DrawEdgeRect( pDC , rc , crBorder , crBody );
        #endif
          if ( m_pDebugWnd && ::IsWindow( m_pDebugWnd->m_hWnd ) && m_pDebugWnd->IsWindowVisible() )
          {
            CPoint pt = rc.BottomRight();
            m_View->Map( pt );
            CRect rcWnd;
            m_pDebugWnd->GetWindowRect( rcWnd );
            m_pDebugWnd->MoveWindow( pt.x , pt.y , rcWnd.Width() , rcWnd.Height() );
          }
        }
  //  }
    CCompositeGlyph::Draw( pDC , bActivityOnly );
  }
}

BOOL CConnectorGlyph::DisconnectWire( CGlyph *pGlyph )
{
  if ( !pGlyph )
  {
    while ( m_Components.GetSize() )
    {
      CWireGlyph* Wire = (CWireGlyph*) m_Components.GetAt( 0 );
      ASSERT( Wire );
      Wire->Disconnect();
    }
    return TRUE;
  }
  else
  {
    for ( int i = 0; i < m_Components.GetSize(); i++ )
    {
      if ( m_Components.GetAt( i ) == pGlyph )
      {
        m_Components.RemoveAt( i );
        return TRUE;
      }
    }
    return FALSE;
  }
}

CGlyph* CConnectorGlyph::Intersect( CPoint& pt , BOOL bSelfRgn )
{
  CRect rc( GetPos() , m_Size );
  rc.InflateRect( CLICK_MARGIN , CLICK_MARGIN );
  BOOL res = rc.PtInRect( pt );
  if ( res )
  {
    m_bHotSpot = true;
    return this;
  }
  else
  {
    m_bHotSpot = false;
    return NULL;
  }

}

BOOL CConnectorGlyph::GetRgn( CRgn* pRgn , BOOL bSelfRgn /* = FALSE */ )
{
  CRect rc( GetPos() , m_Size );
  rc.InflateRect( 4 , 4 );
  pRgn->CreateRectRgnIndirect( rc );
  return TRUE;
}

CGlyph* CConnectorGlyph::GetFather()
{
  return father;
}

CWnd* CConnectorGlyph::GetDebugWnd()
{
  if ( !m_pDebugWnd )
  {
    m_pDebugWnd = new CDebugEnvelopDlg( m_View );
    ((CDebugEnvelopDlg*) m_pDebugWnd)->Create( IDD_DEBUG_ENVELOP , m_View );
  }
  ASSERT( m_pDebugWnd );
  return m_pDebugWnd;
}

void CConnectorGlyph::EnumOpenDebugWnds( CStringArray& uids )
{
  if ( m_pDebugWnd && m_pDebugWnd->IsWindowVisible() )
    uids.Add( m_UID );
}

BOOL CConnectorGlyph::SwitchConnectionTo( CGlyph *glyph )
{
  return ConnectTo( glyph );
}

CGlyph *CConnectorGlyph::FindByUID( LPCTSTR uid )
{
  if ( uid == m_UID )
    return this;
  return CCompositeGlyph::FindByUID( uid );

}

CPoint CConnectorGlyph::GetConnectionPoint()
{
  return GetPos() + CPoint( m_Size.cx / 2 , m_Size.cy / 2 );
}

void CConnectorGlyph::CleanUp()
{
  if ( m_pDebugWnd )
  {
    m_View->ToggleDebugRender( m_UID , TRUE );
    m_pDebugWnd = NULL;
  }
}

void CConnectorGlyph::StopDebuggingPins()
{
  if ( m_pDebugWnd )
  {
    m_View->ToggleDebugRender( m_UID , TRUE );
    m_pDebugWnd = NULL;
  }
}

void CConnectorGlyph::DestroyIn()
{
  m_pConnector = NULL;
  if ( m_pDebugWnd )
  {
    m_View->ToggleDebugRender( m_UID , TRUE );
    m_pDebugWnd = NULL;
  }
  CCompositeGlyph::DestroyIn( FALSE );
  m_UID.Empty();
}

BOOL CConnectorGlyph::ConnectTo( CGlyph *pGlyph )
{
  for ( int i = 0; i < m_Components.GetSize(); i++ )
    if ( ((((CWireGlyph*) (m_Components.GetAt( i )))->GetInput() == this) && (((CWireGlyph*) (m_Components.GetAt( i )))->GetOutput() == pGlyph)) ||
      ((((CWireGlyph*) (m_Components.GetAt( i )))->GetInput() == pGlyph) && (((CWireGlyph*) (m_Components.GetAt( i )))->GetOutput() == this)) )

      return FALSE;

  CEvadeWireGlyph *pGl = new CEvadeWireGlyph( m_View , "" , GetViewType() );
  pGl->SetConnection( this , pGlyph );
  m_Components.Add( pGl );
  if ( m_View )
    m_View->AddGlyphToGraph( pGl );
  ((CConnectorGlyph*) pGlyph)->m_Components.Add( pGl );
  return TRUE;
}

void CConnectorGlyph::FillContextMenu( CMenu* menu )
{
  menu->AppendMenu(MF_STRING, ID_PIN_SETLABEL, "Set label");
  if ( m_pConnector && m_pConnector->GetLogMode() )
    menu->AppendMenu(MF_STRING, ID_PIN_LOG_OFF, "Logging Off");
  else
    menu->AppendMenu(MF_STRING, ID_PIN_LOG_ON, "Logging On");
}

//#define NO_DATA_PASSED 0
//#define DATA_PASSED    1
//#define DATA_SKIPPED   2
CGadget * CConnectorGlyph::GetGadgetAndPin( EnumConnectorType& ConnType , int& iPin )
{
  IGraphbuilder* pGB = m_View->GetBuilder();
  if (!pGB)
    return NULL;
  FXString GadgetUID = m_UID;
  ConnType = Enum_CT_Unknow;
  iPin = -1;
  int pos = -1;
  if ((pos = (int) GadgetUID.Find("<<")) >= 0)
  {
    CString cnmbs = (LPCTSTR)GadgetUID.Mid(pos + 2);
    if (cnmbs.IsEmpty() || cnmbs.SpanIncluding("0123456789").Compare(cnmbs))
      return NULL;
    iPin = atoi(cnmbs);
    GadgetUID = GadgetUID.Left(pos);
    ConnType = Enum_CT_Input;
  }
  else if ((pos = (int) GadgetUID.Find(">>")) >= 0)
  {
    FXString cnmbs = GadgetUID.Mid(pos + 2);
    if (cnmbs.IsEmpty() || cnmbs.SpanIncluding("0123456789").Compare(cnmbs))
      return NULL;
    iPin = atoi(cnmbs);
    GadgetUID = GadgetUID.Left(pos);
    ConnType = Enum_CT_Output;
  }
  else if ((pos = (int)GadgetUID.Find("<>")) >= 0)
  {
    FXString cnmbs = GadgetUID.Mid(pos + 2);
    if (cnmbs.IsEmpty() || cnmbs.SpanIncluding("0123456789").Compare(cnmbs))
      return NULL;
    iPin = atoi(cnmbs);
    GadgetUID = GadgetUID.Left(pos);
    ConnType = Enum_CT_Control;
  }
  else
  {
    //ASSERT(FALSE);
    TRACE( "\nGetGadgetAndPin: can't get gadget ID  " ) ;
    return NULL ;
  }
  CGadget* pG = pGB->GetGadget(GadgetUID);
  return pG;
}

CConnector * CConnectorGlyph::GetConnector()
{
  if (!m_pConnector)
  {
    int connectrorn;
    CGadget * pG = GetGadgetAndPin(m_ConnectorType, connectrorn);
    if (!pG)
      return NULL;
    switch (m_ConnectorType)
    {
    case Enum_CT_Input:
    {
      m_pConnector = pG->GetInputConnector(connectrorn);
      break;
    }
    case Enum_CT_Output:
    {
      m_pConnector = pG->GetOutputConnector(connectrorn);
      break;
    }
    case Enum_CT_Control:
    {
      m_pConnector = pG->GetDuplexConnector(connectrorn);
      break;
    }
    default:
      ASSERT(FALSE);
    }
  }
  return m_pConnector;
}
int CConnectorGlyph::GetDataPassed()
{
  if ( !m_View->GetViewActivity() ) 
    return NO_DATA_PASSED;
//   IGraphbuilder* pGB = m_View->GetBuilder();
//   if ( !pGB )
//     return NO_DATA_PASSED;
//   CString GadgetUID = m_UID;
//   EnumConnectorType ConnectorType = Enum_CT_Unknow;
//   int connectrorn = -1;
//   int pos = -1;
//   if ( (pos = GadgetUID.Find( "<<" )) >= 0 )
//   {
//     CString cnmbs = GadgetUID.Mid( pos + 2 );
//     if ( cnmbs.IsEmpty() || cnmbs.SpanIncluding( "0123456789" ).Compare( cnmbs ) )
//       return NO_DATA_PASSED;
//     connectrorn = atoi( cnmbs );
//     GadgetUID = GadgetUID.Left( pos );
//     ConnectorType = Enum_CT_Input;
//   }
//   else if ( (pos = GadgetUID.Find( ">>" )) >= 0 )
//   {
//     CString cnmbs = GadgetUID.Mid( pos + 2 );
//     if ( cnmbs.IsEmpty() || cnmbs.SpanIncluding( "0123456789" ).Compare( cnmbs ) )
//       return NO_DATA_PASSED;
//     connectrorn = atoi( cnmbs );
//     GadgetUID = GadgetUID.Left( pos );
//     ConnectorType = Enum_CT_Output;
//   }
//   else if ( (pos = GadgetUID.Find( "<>" )) >= 0 )
//   {
//     CString cnmbs = GadgetUID.Mid( pos + 2 );
//     if ( cnmbs.IsEmpty() || cnmbs.SpanIncluding( "0123456789" ).Compare( cnmbs ) )
//       return NO_DATA_PASSED;
//     connectrorn = atoi( cnmbs );
//     GadgetUID = GadgetUID.Left( pos );
//     ConnectorType = Enum_CT_Control;
//   }
//   else
//   {
//     ASSERT( FALSE );
//   }
//   CGadget* pG = pGB->GetGadget( GadgetUID );
 
  if ( !m_pConnector && !GetConnector() )
    return NO_DATA_PASSED;
  switch (m_ConnectorType)
  {
  case Enum_CT_Input:
  {
    if (m_pConnector->GetFramesPassed() != 0)
      return DATA_PASSED;
    break;
  }
  case Enum_CT_Output:
  {
    if (m_pConnector->GetFramesSkipped())
    {
      m_pConnector->GetFramesPassed(); // reset frames counter
      return DATA_SKIPPED;
    }
    if (m_pConnector->GetFramesPassed())
      return DATA_PASSED;
    break;
  }
  case Enum_CT_Control:
  {
    if (m_pConnector->GetFramesPassed())
      return DATA_PASSED;
    break;
  }
  default:
    ASSERT(FALSE);
  }
  return NO_DATA_PASSED;
}

// CCompositeGlyph
CGlyph* CCompositeGlyph::FindByUID( LPCTSTR uid )
{
  //	if (_tcsstr(uid,"->")!=NULL)
  //	{
  //		int lll=0;
  //	}
  for ( int i = 0; i < m_Components.GetSize(); i++ )
  {
    CGlyph* CompGlyph = m_Components.GetAt( i );
    if ( !CompGlyph ) 
      return NULL;
    CGlyph* Glyph = CompGlyph->FindByUID( uid );
    if ( Glyph ) 
      return Glyph;
  }
  return NULL;
}

void CCompositeGlyph::EnumOpenDebugWnds( CStringArray& uids )
{
  for ( int i = 0; i < m_Components.GetSize(); i++ )
  {
    CGlyph* Glyph = m_Components.GetAt( i );
    Glyph->EnumOpenDebugWnds( uids );
  }
}

void CCompositeGlyph::Draw( CDC* pDC , bool bActivityOnly )
{
  //ResetEvadeCount();
  for ( int i = 0; i < m_Components.GetSize(); i++ )
    m_Components.GetAt( i )->Draw( pDC , bActivityOnly );
}

void CCompositeGlyph::ResetEvadeCount()
{
  CGlyph::ResetEvadeCount();
  for ( int i = 0; i < m_Components.GetSize(); i++ )
  {
    m_Components.GetAt( i )->ResetEvadeCount();
  }
}

void CCompositeGlyph::SetViewType( ViewType vt )
{
  CGlyph::SetViewType( vt );
  for ( int i = 0; i < m_Components.GetSize(); i++ )
  {
    m_Components.GetAt( i )->SetViewType( vt );
  }
}

BOOL CCompositeGlyph::GetRgn( CRgn* pRgn , BOOL bSelf )
{
  BOOL bFirst = TRUE;
  for ( int i = 0; i < m_Components.GetSize(); i++ )
  {
    if ( bFirst )
    {
      bFirst = !m_Components.GetAt( i )->GetRgn( pRgn , bSelf );
    }
    else
    {
      CRgn rgn;
      if ( m_Components.GetAt( i )->GetRgn( &rgn , bSelf ) )
      {
        pRgn->CombineRgn( pRgn , &rgn , RGN_OR );
        rgn.DeleteObject();
      }
    }
  }
  return !bFirst;
}

CGlyph* CCompositeGlyph::Intersect( CPoint& pt , BOOL bSelfRgn )
{
  CGlyph* pResult = NULL;
  for ( INT_PTR i = m_Components.GetSize() - 1; i >= 0; i-- )
  {
    CGlyph* pGlyph = m_Components.GetAt( i );
    CGlyph* pHit = pGlyph->Intersect( pt , bSelfRgn );
    if ( pHit )
    {
      if ( pHit->IsSelectionPriorityHigh() )
        return pHit;
      else if ( !pResult )
        pResult = pHit;
    }
  }
  return pResult;
}

BOOL CCompositeGlyph::Intersect( CGlyph* gl )
{
  CRgn r1 , r2 , r3;
  GetRgn( &r1 , TRUE );
  gl->GetRgn( &r2 , TRUE );
  r3.CreateRectRgnIndirect( CRect( 0 , 0 , 0 , 0 ) );
  int nCombineResult = r3.CombineRgn( &r1 , &r2 , RGN_AND );
  if ( nCombineResult == NULLREGION )
    return FALSE;
  else
    return TRUE;
}

void CCompositeGlyph::OffsetPos( CPoint& offset )
{
  for ( int i = 0; i < m_Components.GetSize(); i++ )
    m_Components.GetAt( i )->OffsetPos( offset );
  CGlyph::OffsetPos( offset );
}

void CCompositeGlyph::DestroyIn()
{
  while ( m_Components.GetSize() )
  {
    CGlyph* pGlyph = m_Components.GetAt( 0 );
    pGlyph->DestroyIn();
    delete pGlyph;
    m_Components.RemoveAt( 0 );
  }
}

void CCompositeGlyph::DestroyIn( BOOL delete_from_components /* = FALSE */ )
{
  while ( m_Components.GetSize() )
  {
    CGlyph* pGlyph = m_Components.GetAt( 0 );
    pGlyph->DestroyIn();
    if ( delete_from_components )
    {
      delete pGlyph;
      m_Components.RemoveAt( 0 );
    }
  }
}

void CCompositeGlyph::CleanUp()
{
  while ( m_Components.GetSize() )
  {
    CGlyph* pGlyph = m_Components.GetAt( 0 );
    pGlyph->CleanUp();
    delete pGlyph;
    m_Components.RemoveAt( 0 );
  }
}

void CCompositeGlyph::StopDebuggingPins()
{
  int i;
  for ( i = 0; i < GetChildrenCount(); i++ )
  {
    CGlyph* pGlyph = GetChildAt( i );
    ASSERT( pGlyph );
    pGlyph->StopDebuggingPins();
  }
}

bool CCompositeGlyph::Remove( CGlyph* Component , bool bFreeMemory )
{
  for ( int i = 0; i < GetChildrenCount(); i++ )
  {
    if ( GetChildAt( i ) == Component )
    {
      if ( bFreeMemory )
        delete Component;
      m_Components.RemoveAt( i );
      return true;
    }
  }
  return false;
}


// CGadgetGlyph

CGadgetGlyph::CGadgetGlyph( CSketchView *view , LPCTSTR uid , 
  CStringArray& inputs , CStringArray& outputs , CStringArray& duplex ) :
  CCompositeGlyph( view ) ,
  m_UID( uid ) ,
  m_bNeedUpdate( TRUE ) ,
  m_nInputs( 0 ) ,
  m_nOutputs( 0 ) ,
  m_nDuplex( 0 )
{
  int iNVertPins = (int) max( inputs.GetCount() , outputs.GetCount() ) ;
  int iHeight = 2 * GLYPH_VERT_SPACE_TO_FIRST_PIN + (iNVertPins - 1) * GLYPH_VERT_SPACE_BETWEEN_PINS ;
  m_Size = CSize( GLYPH_GADGET_WIDTH , iHeight ) ;

  UpdatePins( inputs , outputs , duplex );
  m_ModesMenu.m_hMenu = NULL;
  m_ThreadsMenu.m_hMenu = NULL;
}

CGadgetGlyph::CGadgetGlyph( CSketchView *view , LPCTSTR uid , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex , ViewType vt ) :
  CCompositeGlyph( view ) ,
  m_UID( uid ) ,
  m_bNeedUpdate( TRUE ) ,
  m_nInputs( 0 ) ,
  m_nOutputs( 0 ) ,
  m_nDuplex( 0 )
{
  int iNVertPins = (int) max( inputs.GetCount() , outputs.GetCount() ) ;
  int iHeight = 2 * GLYPH_VERT_SPACE_TO_FIRST_PIN + (iNVertPins - 1) * GLYPH_VERT_SPACE_BETWEEN_PINS ;
  m_Size = CSize( GLYPH_GADGET_WIDTH , iHeight ) ;

  SetViewType( vt );
  UpdatePins( inputs , outputs , duplex );
  m_ModesMenu.m_hMenu = NULL;
}

CGlyph* CGadgetGlyph::FindByUID( LPCTSTR uid )
{
  if ( uid == m_UID )
    return this;
  return CCompositeGlyph::FindByUID( uid );
}

BOOL CGadgetGlyph::GetUID( FXString& uid )
{
  uid = m_UID;
  return TRUE;
}

void CGadgetGlyph::SetUID( LPCTSTR uid )
{
  m_UID = uid;
}

void CGadgetGlyph::Draw( CDC* pDC , bool bActivityOnly )
{
  bool NeedUpdate = false;
  if ( (m_View->GetBuilder()->GetGadget( m_UID )) &&
    (m_View->GetBuilder()->GetGadget( m_UID )->Status().GetBool( STATUS_REDRAW , NeedUpdate )) &&
    NeedUpdate )
  {
    m_View->GetBuilder()->GetGadget( m_UID )->Status().WriteBool( STATUS_REDRAW , false );
  }
  if ( (m_bNeedUpdate) || (NeedUpdate) )
  {
    CStringArray inputs , outputs , duplex;
    if ( m_View->GetBuilder()->ListGadgetConnectors( m_UID , inputs , outputs , duplex ) )
    {
      UpdatePins( inputs , outputs , duplex );
      m_bNeedUpdate = m_View->GetBuilder()->IsGadgetSetupOn( m_UID );
    }
  }

  CRect rc( GetPos() , m_Size );
  COLORREF crBorder , crBody;
  if ( m_View->GetMyRectColors( this , &crBorder , &crBody ) )
  {
    if ( GetBodyColor() != 0xffffff )
      crBody = GetBodyColor() ;
    if ( !bActivityOnly )
    {
    #ifdef GLYPHS_DRAW_PLAIN
      m_View->DrawRect( pDC , rc , crBorder , crBody );
    #elif defined GLYPHS_DRAW_ROUND
      m_View->DrawBigRoundRect( pDC , rc , crBorder , crBody );
    #elif defined GLYPHS_DRAW_EDGE
      m_View->DrawEdgeRect( pDC , rc , crBorder , crBody );
    #endif
      CRect rcIcon;
      m_View->DrawIcon( pDC , m_View->GetMyIcon( this ) , rc , &crBorder , &crBody , rcIcon );
      CRect rcMode = rc;
      rcMode.left = rcMode.right - 6; rcMode.right--;
      rcMode.bottom = rcMode.top + 6; rcMode.top++;
      m_View->DrawModeIcon( pDC , m_View->GetMyModeIcon( this ) , rcMode );
      rc.DeflateRect( 1 , 1 );
      LOGFONT font;
      FXString uid;
      if ( GetUID( uid ) && m_View->GetMyFont( this , &crBorder , &font ) )
      {
        m_View->DrawText( pDC , uid , rc , crBorder , &font );
        int nThreads;
        if ( m_View->GetBuilder()->GetGadgetThreadsNumber( uid , nThreads ) && (nThreads > 1) )
        {
          CRect rcThreads;
          rcThreads.left = rcIcon.right + 3;
          rcThreads.right = rc.right;
          rcThreads.top = rcIcon.top;
          rcThreads.bottom = rcIcon.bottom;
          CString text;
          text.Format( "%d" , nThreads );
          m_View->DrawText( pDC , text , rcThreads , crBorder , &font );
        }
      }
    }
    CCompositeGlyph::Draw( pDC , bActivityOnly );
  }
}

void CGadgetGlyph::ResetEvadeCount()
{
  CGlyph::ResetEvadeCount();
  CCompositeGlyph::ResetEvadeCount();
}

BOOL CGadgetGlyph::GetRgn( CRgn* pRgn , BOOL bSelf )
{
  CRect rc( GetPos() , m_Size );
  CRgn rgn;
  rgn.CreateRectRgnIndirect( rc );
  if ( bSelf || !CCompositeGlyph::GetRgn( pRgn ) )
    pRgn->CreateRectRgnIndirect( rc );
  else
    pRgn->CombineRgn( pRgn , &rgn , RGN_OR );
  rgn.DeleteObject();
  return TRUE;
}

CGlyph* CGadgetGlyph::Intersect( CPoint& pt , BOOL bSelfRgn )
{
  CGlyph* pHit = CCompositeGlyph::Intersect( pt , bSelfRgn );
  if ( pHit )
    return pHit;
  CRect rc( GetPos() , m_Size );
  return (rc.PtInRect( pt )) ? this : NULL;
}

void CGadgetGlyph::DestroyIn()
{
  CCompositeGlyph::DestroyIn();
  m_nInputs = 0;
  m_nOutputs = 0;
  m_nDuplex = 0;

  m_View->RemoveGadget( this , m_UID );
  m_UID.Empty();
}

void CGadgetGlyph::CleanUp()
{
  CCompositeGlyph::CleanUp();
  m_UID.Empty();
}

void CGadgetGlyph::ShowSetupDlgAt( CPoint& point )
{
  m_bNeedUpdate = TRUE;
  Tvdb400_ShowGadgetSetupDlg( m_View->GetBuilder() , m_UID , point );
  //m_View->Invalidate();
  //m_View->UpdateWindow();
}

void CGadgetGlyph::ShowAffinityDlgAt( CPoint& point )
{
  m_bNeedUpdate = TRUE;
  m_View->ShowAffinityDlg( m_UID , point );
  //m_View->Invalidate();
  //m_View->UpdateWindow();
}


void CGadgetGlyph::FillContextMenu( CMenu* menu )
{
  menu->AppendMenu( MF_STRING , ID_CONTEXT_RENAME , "Rename" );
  menu->AppendMenu( MF_STRING , ID_CONTEXT_SETUP , "Setup" );
  menu->AppendMenu( MF_STRING , ID_CONTEXT_AFFINITY , "Affinity" );
  DestroyContextSubmenus();
  if ( m_ThreadsMenu.CreatePopupMenu() )
  {
    int nThreads = 0;
    FXString uid;
    if (
      GetUID( uid ) &&
      m_View->GetBuilder()->GetGadgetIsMultyCoreAllowed( uid )
      )
    {
      if (
        m_View->GetBuilder() &&
        m_View->GetBuilder()->GetGadgetThreadsNumber( uid , nThreads )
        )
      {
        m_ThreadsMenu.AppendMenu( MF_STRING , ID_THREADS_INCREASE , "Increase" );
        m_ThreadsMenu.AppendMenu( MF_STRING , ID_THREADS_DECREASE , "Decrease" );
        if ( nThreads < 2 )
        {
          m_ThreadsMenu.EnableMenuItem( ID_THREADS_DECREASE , MF_BYCOMMAND | MF_GRAYED );
        }
      }
      menu->AppendMenu( MF_POPUP , (UINT_PTR) m_ThreadsMenu.m_hMenu , "Threads" );
    }
  }
  FXString uid;
  int mode;
  if ( GetUID( uid ) && m_View->GetBuilder() && m_View->GetBuilder()->GetGadgetMode( uid , mode ) && m_ModesMenu.CreatePopupMenu() )
  {
    m_ModesMenu.AppendMenu( MF_STRING , ID_MODE_REJECT , "Reject" );
    m_ModesMenu.AppendMenu( MF_STRING , ID_MODE_TRANSMIT , "Transmit" );
    m_ModesMenu.AppendMenu( MF_STRING , ID_MODE_PROCESS , "Process" );
    UINT nChecked = ((mode == CGadget::mode_reject) ? ID_MODE_REJECT : ((mode == CGadget::mode_transmit) ? ID_MODE_TRANSMIT : ID_MODE_PROCESS));
    m_ModesMenu.CheckMenuItem( nChecked , MF_CHECKED | MF_BYCOMMAND );
    menu->AppendMenu( MF_POPUP , (UINT_PTR) m_ModesMenu.m_hMenu , "Modes" );
  }
  CFilterGadget::OutputMode outputmode;
  if ( GetUID( uid ) && m_View->GetBuilder() && m_View->GetBuilder()->GetOutputMode( uid , outputmode ) && m_OutputModeMenu.CreatePopupMenu() )
  {
    m_OutputModeMenu.AppendMenu( MF_STRING , ID_MODE_APPEND , "Append" );
    m_OutputModeMenu.AppendMenu( MF_STRING , ID_MODE_REPLACE , "Replace" );
    UINT nChecked = ((outputmode == CFilterGadget::modeAppend) ? ID_MODE_APPEND : ID_MODE_REPLACE);
    m_OutputModeMenu.CheckMenuItem( nChecked , MF_CHECKED | MF_BYCOMMAND );
    menu->AppendMenu( MF_POPUP , (UINT_PTR) m_OutputModeMenu.m_hMenu , "OutputMode" );
  }
}

void CGadgetGlyph::DestroyContextSubmenus()
{
  if ( m_ModesMenu.m_hMenu )
  {
    m_ModesMenu.DestroyMenu();
    m_ModesMenu.m_hMenu = NULL;
  }
  if ( m_ThreadsMenu.m_hMenu )
  {
    m_ThreadsMenu.DestroyMenu();
    m_ThreadsMenu.m_hMenu = NULL;
  }
  if ( m_OutputModeMenu.m_hMenu )
  {
    m_OutputModeMenu.DestroyMenu();
    m_OutputModeMenu.m_hMenu = NULL;
  }
}

void CGadgetGlyph::ArrangeChildren()
{
  if ( !GetChildrenCount() )
    return;
  CRect rc( CPoint( 0 , 0 ) , CSize( GLYPH_CONNECTOR_WIDTH , GLYPH_CONNECTOR_HEIGHT ) );
  if ( m_nInputs )
  {
    int space = m_Size.cy / m_nInputs;
    for ( int i = 0; i < m_nInputs; i++ )
    {
      CPoint offset( -rc.Width() , (space - rc.Height()) / 2 + space * i );
      CPoint delta = offset - GetChildAt( i )->GetPos() + GetPos();
      GetChildAt( i )->OffsetPos( delta );
    }
  }
  if ( m_nOutputs )
  {
    int space = m_Size.cy / m_nOutputs;
    for ( int i = 0; i < m_nOutputs; i++ )
    {
      CPoint offset( m_Size.cx , (space - rc.Height()) / 2 + space * i );
      CPoint delta = offset - GetChildAt( i + m_nInputs )->GetPos() + GetPos();
      GetChildAt( i + m_nInputs )->OffsetPos( delta );
    }
  }
  if ( m_nDuplex )
  {
    int space = m_Size.cx / m_nDuplex;
    for ( int i = 0; i < m_nDuplex; i++ )
    {
      CPoint offset( (space - rc.Width()) / 2 + space * i , m_Size.cy );
      CPoint delta = offset - GetChildAt( i + m_nInputs + m_nOutputs )->GetPos() + GetPos();
      GetChildAt( i + m_nInputs + m_nOutputs )->OffsetPos( delta );
    }
  }
}

void CGadgetGlyph::UpdatePins( CStringArray& inputs , CStringArray& outputs , CStringArray& duplex )
{
  int i;
  int nTmpInputs = m_nInputs;
  int nTmpOutputs = m_nOutputs;
  int nTmpDuplex = m_nDuplex;
  bool bNewInputsGoFirst = true , bNewOutputsGoFirst = true , bNewDuplexGoFirst = true;
  FXString uid;
  if ( inputs.GetSize() )
  {
    uid = inputs.GetAt( 0 );
    bNewInputsGoFirst = (FindByUID( uid ) == NULL);
  }
  if ( outputs.GetSize() )
  {
    uid = outputs.GetAt( 0 );
    bNewOutputsGoFirst = (FindByUID( uid ) == NULL);
  }
  if ( duplex.GetSize() )
  {
    uid = duplex.GetAt( 0 );
    bNewDuplexGoFirst = (FindByUID( uid ) == NULL);
  }
  for ( i = 0; i < GetChildrenCount(); i++ )
  {
    CGlyph* pGlyph = GetChildAt( i );
    if ( (pGlyph) && (pGlyph->GetUID( uid )) )
    {
      int j;
      bool bExisting = false;
      for ( j = 0; j < inputs.GetSize(); j++ )
      {
        if ( uid == (LPCTSTR) inputs[ j ] )
        {
          bExisting = true;
          inputs.RemoveAt( j );
          break;
        }
      }
      if ( bExisting )
        continue;
      for ( j = 0; j < outputs.GetSize(); j++ )
      {
        if ( uid == (LPCTSTR) outputs[ j ] )
        {
          bExisting = true;
          outputs.RemoveAt( j );
          break;
        }
      }
      for ( j = 0; j < duplex.GetSize(); j++ )
      {
        if ( uid == (LPCTSTR) duplex[ j ] )
        {
          bExisting = true;
          duplex.RemoveAt( j );
          break;
        }
      }
      if ( bExisting )
        continue;
      pGlyph->DisconnectWire( NULL );
      Remove( pGlyph , true );
      if ( i < m_nInputs )
        m_nInputs--;
      else if ( i < m_nInputs + m_nOutputs )
        m_nOutputs--;
      else
        m_nDuplex--;
      i--;
    }
  }
  for ( i = 0; i < inputs.GetSize(); i++ )
  {
    if ( bNewInputsGoFirst )
    {
      Insert( new CConnectorGlyph( m_View , inputs.GetAt( i ) , GetViewType() , this ) , i );
      m_nInputs++;
    }
    else
      Insert( new CConnectorGlyph( m_View , inputs.GetAt( i ) , GetViewType() , this ) , m_nInputs++ );
  }
  for ( i = 0; i < outputs.GetSize(); i++ )
  {
    if ( bNewOutputsGoFirst )
    {
      Insert( new CConnectorGlyph( m_View , outputs.GetAt( i ) , GetViewType() , this ) , i + m_nInputs );
      m_nOutputs++;
    }
    else
      Insert( new CConnectorGlyph( m_View , outputs.GetAt( i ) , GetViewType() , this ) , m_nInputs + m_nOutputs++ );
  }
  for ( i = 0; i < duplex.GetSize(); i++ )
  {
    if ( bNewDuplexGoFirst )
    {
      Insert( new CConnectorGlyph( m_View , duplex.GetAt( i ) , GetViewType() , this ) , i + m_nInputs + m_nOutputs );
      m_nDuplex++;
    }
    else
      Insert( new CConnectorGlyph( m_View , duplex.GetAt( i ) , GetViewType() , this ) , m_nInputs + m_nOutputs + m_nDuplex++ );
  }
  ArrangeChildren();
  //Invalidate only if number of pin has been changed
  //for CSink && CAggregate
  if ( ((nTmpOutputs != m_nOutputs) || (nTmpInputs != m_nInputs) || (nTmpDuplex != m_nDuplex)) )
  {
    m_View->Invalidate();
  }
}


// CComplexGlyph

void CComplexGlyph::Draw( CDC* pDC , bool bActivityOnly )
{
  if ( !bActivityOnly )
  {
    CRect rc( GetPos() , GetSize() );
    COLORREF crBorder , crBody;
    if ( m_View->GetMyRectColors( this , &crBorder , &crBody ) )
    {
    #ifdef GLYPHS_DRAW_PLAIN
      rc.OffsetRect( 4 , 4 );
      m_View->DrawRect( pDC , rc , crBorder , crBody );
      rc.OffsetRect( -2 , -2 );
      m_View->DrawRect( pDC , rc , crBorder , crBody );
    #elif defined GLYPHS_DRAW_ROUND
      rc.OffsetRect( 4 , 4 );
      m_View->DrawBigRoundRect( pDC , rc , crBorder , crBody );
      rc.OffsetRect( -2 , -2 );
      m_View->DrawBigRoundRect( pDC , rc , crBorder , crBody );
    #elif defined GLYPHS_DRAW_EDGE
      rc.OffsetRect( 4 , 4 );
      m_View->DrawEdgeRect( pDC , rc , crBorder , crBody );
      rc.OffsetRect( -2 , -2 );
      m_View->DrawEdgeRect( pDC , rc , crBorder , crBody );
    #endif
    }
  }
  CStringArray uidsModified;
  FXString uid;
  if ( GetUID( uid ) && m_View->GetBuilder()->EnumModifiedGadgets( uidsModified ) )
  {
    for ( int i = 0; i < uidsModified.GetSize(); i++ )
    {
      if ( uidsModified[ i ] == (LPCTSTR) uid )
      {
        SetNeedUpdate( TRUE );
        break;
      }
    }
  }
  CGadgetGlyph::Draw( pDC , bActivityOnly );
}

BOOL CComplexGlyph::GetRgn( CRgn* pRgn , BOOL bSelfRgn )
{
  if ( bSelfRgn )
    return CGadgetGlyph::GetRgn( pRgn , bSelfRgn );
  CSize sz = GetSize() + CSize( 4 , 4 );
  CRect rc( GetPos() , sz );
  CRgn rgn;
  rgn.CreateRectRgnIndirect( rc );
  if ( !CGadgetGlyph::GetRgn( pRgn , bSelfRgn ) )
    pRgn->CreateRectRgnIndirect( rc );
  else
    pRgn->CombineRgn( pRgn , &rgn , RGN_OR );
  rgn.DeleteObject();
  return TRUE;
}

void CComplexGlyph::FillContextMenu( CMenu* menu )
{
  menu->AppendMenu( MF_STRING , ID_CONTEXT_RENAME , "Rename" );
  FXString uid;
  if ( GetUID( uid ) )
  {
    if ( m_View->GetBuilder()->IsLibraryComplexGadget( uid ) )
      menu->AppendMenu( MF_STRING , ID_CONTEXT_CMPLXLOCAL , "Move from library to local" );
  }
}

// CRenderGlyph

CRenderGlyph::CRenderGlyph( CSketchView *view , LPCTSTR uid , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex ) :
  CGadgetGlyph( view , uid , inputs , outputs , duplex ) ,
  m_pWndFrame( NULL ) ,
  m_pRenderGadget( NULL ) ,
  m_constructed( true )
{
  CRect rc( GetPos() , GetSize() );
  CString Monitor;
  m_pWndFrame = (CRenderViewFrame*) view->CreateRenderFrame( rc , this , Monitor );

  if ( m_pWndFrame == NULL )
    m_constructed = false;

  if ( (m_pWndFrame) && (Monitor != LINEAGE_DEBUG) )
  {
    if ( !view->GetBuilder()->ConnectRendererAndMonitor( uid , m_pWndFrame , Monitor , m_pRenderGadget ) )
    {
      m_pWndFrame->DestroyWindow();
      delete m_pWndFrame;
      m_pWndFrame = NULL;
    }
    else
    {
      m_pRenderGadget->GetDefaultWndSize( rc );
      rc.InflateRect( GLYPH_CLIENT_MARGIN , GLYPH_CLIENT_MARGIN + GLYPH_CAPTION_HEIGHT , GLYPH_CLIENT_MARGIN , GLYPH_CLIENT_MARGIN );
      m_defaultWndSize = rc.Size();
      UpdateSize( m_defaultWndSize );
    }
    if ( m_pWndFrame && (!m_pRenderGadget || !m_pRenderGadget->GetRenderWnd()) )
    {
      m_pWndFrame->DestroyWindow();
      delete m_pWndFrame;
      m_pWndFrame = NULL;
    }
  }
  if ( (m_pWndFrame) && m_pRenderGadget->GetRenderWnd() )
    m_pWndFrame->Attach( m_pRenderGadget->GetRenderWnd() );
}

CRenderGlyph::CRenderGlyph( CSketchView *view , LPCTSTR uid , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex , ViewType vt ) :
  CGadgetGlyph( view , uid , inputs , outputs , duplex , vt ) ,
  m_pWndFrame( NULL ) ,
  m_pRenderGadget( NULL ) ,
  m_constructed( true )
{
  CRect rc( GetPos() , GetSize() );
  CString Monitor;
  m_pWndFrame = (CRenderViewFrame*) view->CreateRenderFrame( rc , this , Monitor );

  if ( m_pWndFrame == NULL )
    m_constructed = false;

  if ( (m_pWndFrame) && (Monitor != LINEAGE_DEBUG) )
  {
    if ( !view->GetBuilder()->ConnectRendererAndMonitor( uid , m_pWndFrame , Monitor , m_pRenderGadget ) )
    {
      m_pWndFrame->DestroyWindow();
      delete m_pWndFrame;
      m_pWndFrame = NULL;
    }
    else
    {
      m_pRenderGadget->GetDefaultWndSize( rc );
      rc.InflateRect( GLYPH_CLIENT_MARGIN , GLYPH_CLIENT_MARGIN + GLYPH_CAPTION_HEIGHT , GLYPH_CLIENT_MARGIN , GLYPH_CLIENT_MARGIN );
      m_defaultWndSize = rc.Size();
      UpdateSize( m_defaultWndSize );
    }
    if ( m_pWndFrame && (!m_pRenderGadget || !m_pRenderGadget->GetRenderWnd()) )
    {
      m_pWndFrame->DestroyWindow();
      delete m_pWndFrame;
      m_pWndFrame = NULL;
    }
  }
  if ( (m_pWndFrame) && m_pRenderGadget->GetRenderWnd() )
    m_pWndFrame->Attach( m_pRenderGadget->GetRenderWnd() );
}

CRenderGlyph::~CRenderGlyph()
{
  if ( m_pWndFrame )
  {
    if ( m_pRenderGadget )
      m_pRenderGadget->Detach();
    m_pRenderGadget = NULL;
    if ( IsWindow( m_pWndFrame->m_hWnd ) )
    {
      m_pWndFrame->DestroyWindow();
      delete m_pWndFrame;
    }
    m_pWndFrame = NULL;
  }
}

void CRenderGlyph::DestroyIn()
{
  CGadgetGlyph::DestroyIn();
  m_pRenderGadget = NULL;
}

void CRenderGlyph::Draw( CDC* pDC , bool bActivityOnly )
{
  CGadgetGlyph::Draw( pDC , bActivityOnly );
  if ( m_pWndFrame && IsWindow(m_pWndFrame->m_hWnd) && m_pWndFrame->IsBuiltIn() )
  {
    CPoint pt( GetPos().x , GetPos().y );
    m_View->Map( pt );
    CRect tmpRc( CPoint( 0 , 0 ) , m_defaultWndSize );
    m_View->Map( tmpRc );
    tmpRc.OffsetRect( CPoint( pt.x - tmpRc.left , pt.y - tmpRc.top ) );
    tmpRc.DeflateRect( GLYPH_CLIENT_MARGIN , 
      GLYPH_CAPTION_HEIGHT + GLYPH_CLIENT_MARGIN , 
      GLYPH_CLIENT_MARGIN , GLYPH_CLIENT_MARGIN );
    m_pWndFrame->MoveWindow( tmpRc );

    if ( !m_pWndFrame->IsWindowVisible() )
    {
      m_pWndFrame->ShowWindow( SW_SHOW );
      if ( !m_pRenderGadget->GetRenderWnd()->IsWindowVisible() )
        m_pRenderGadget->GetRenderWnd()->ShowWindow( SW_SHOW );
    }
  }
}

int CRenderGlyph::GetIcon()
{
  if ( (m_pWndFrame) && (m_pWndFrame->IsBuiltIn()) )
    return -1;
  return m_idIcon;
}

void CRenderGlyph::UpdateSize( CSize newSz )
{
  if ( !m_pWndFrame )
    return;
  CSize sz( GLYPH_GADGET_WIDTH , GLYPH_GADGET_HEIGHT );
  CRect rcView;
  if ( m_pWndFrame->IsBuiltIn() )
  {
    SetIcon( -1 );

    TRACE( "+++ New size(%d,%d)\n" , newSz.cx , newSz.cy );

    if ( (m_pRenderGadget) && (m_pRenderGadget->GetRenderWnd()) )
    {
      CRect frRect;

      if ( newSz != m_defaultWndSize )
        SetNewDefaultWndSize( newSz );

      rcView = CRect( CPoint( 0 , 0 ) , m_defaultWndSize );

      frRect = CRect( CPoint( 0 , 0 ) , m_defaultWndSize );
      sz = frRect.Size();

      //to remove upper-left corner flicker
      m_pWndFrame->ShowWindow( SW_HIDE );
      m_pRenderGadget->GetRenderWnd()->ShowWindow( SW_HIDE );

      m_pWndFrame->MoveWindow( rcView );
      m_pWndFrame->GetClientRect( rcView );
      m_pRenderGadget->GetRenderWnd()->MoveWindow( rcView , FALSE );
      m_View->Invalidate();
    }
    else //
    {
      m_pWndFrame->GetWindowRect( rcView );
      rcView.InflateRect( GLYPH_CLIENT_MARGIN , GLYPH_CLIENT_MARGIN + GLYPH_CAPTION_HEIGHT , GLYPH_CLIENT_MARGIN , GLYPH_CLIENT_MARGIN );
      sz = rcView.Size();
    }
    SetSize( sz );
  }
  else //!IsBuiltIn
  {
    m_pWndFrame->GetWindowRect( rcView );
    rcView.InflateRect( GLYPH_CLIENT_MARGIN , GLYPH_CLIENT_MARGIN + GLYPH_CAPTION_HEIGHT , GLYPH_CLIENT_MARGIN , GLYPH_CLIENT_MARGIN );
    CRect rcView;
    m_pWndFrame->GetClientRect( rcView );
    if ( (m_pRenderGadget) && (m_pRenderGadget->GetRenderWnd()) )
      m_pRenderGadget->GetRenderWnd()->MoveWindow( rcView , TRUE );
  }
}

CWnd* CRenderGlyph::GetRenderWnd()
{
  return m_pWndFrame;
}


// CCollectionGlyph

CCollectionGlyph::CCollectionGlyph( CSketchView *view , const CCollectionGlyph* pCollection ) :
  CCompositeGlyph( view )
{
  Select( (CCompositeGlyph*) pCollection );
}

CCollectionGlyph::CCollectionGlyph( CSketchView *view , const CCollectionGlyph* pCollection , ViewType vt ) :
  CCompositeGlyph( view )
{
  Select( (CCompositeGlyph*) pCollection );
  SetViewType( vt );
}

void CCollectionGlyph::Select( CCompositeGlyph* pGlyph )
{
  for ( int i = 0; i < pGlyph->GetChildrenCount(); i++ )
    Add( pGlyph->GetChildAt( i ) );
}

void CCollectionGlyph::IntersectRect( LPRECT rc )
{
  CRgn rgn;
  for ( int i = 0; i < GetChildrenCount(); i++ )
  {
    CGlyph* pGlyph = GetChildAt( i );
    pGlyph->GetRgn( &rgn , TRUE );
    if ( !rgn.RectInRegion( rc ) )
    {
      Remove( pGlyph );
      i--;
    }
    rgn.DeleteObject();
  }
}

BOOL CCollectionGlyph::IsDraggable()
{
  for ( int i = 0; i < GetChildrenCount(); i++ )
    if ( !GetChildAt( i )->IsDraggable() )
      return FALSE;
  return TRUE;
}

void CCollectionGlyph::DestroyIn()
{
  while ( GetChildrenCount() )
    Remove( GetChildAt( 0 ) );
}


// CAggregateGlyph

CAggregateGlyph::CAggregateGlyph( CSketchView *view , CCollectionGlyph* pInternals , LPCTSTR uid , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex ) :
  CGadgetGlyph( view , uid , inputs , outputs , duplex )
{
  m_pInternalGlyphs = new CCollectionGlyph( view , pInternals );
  SwitchConnections( TRUE );
}

CAggregateGlyph::CAggregateGlyph( CSketchView *view , CCollectionGlyph* pInternals , LPCTSTR uid , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex , ViewType vt ) :
  CGadgetGlyph( view , uid , inputs , outputs , duplex , vt )
{
  m_pInternalGlyphs = new CCollectionGlyph( view , pInternals );
  SwitchConnections( TRUE );
}

void CAggregateGlyph::OffsetPos( CPoint& offset )
{
  CCompositeGlyph::OffsetPos( offset );
}

CCollectionGlyph* CAggregateGlyph::Expand()
{
  SwitchConnections( FALSE );
  CCollectionGlyph* pInternals = m_pInternalGlyphs;
  m_pInternalGlyphs = NULL;
  return pInternals;
}

void CAggregateGlyph::DestroyIn()
{
  if ( m_pInternalGlyphs )
  {
    SwitchConnections( FALSE );
    TRACE( "CAggregateGlyph::DestroyIn - %d internal glyphs\n" , m_pInternalGlyphs->GetChildrenCount() );
    while ( m_pInternalGlyphs->GetChildrenCount() )
    {
      CGlyph* pChild = m_pInternalGlyphs->GetChildAt( 0 );
      pChild->DestroyIn();
      m_pInternalGlyphs->Remove( pChild , true );
    }
    delete m_pInternalGlyphs;
    m_pInternalGlyphs = NULL;
  }
  while ( GetChildrenCount() )
    Remove( GetChildAt( 0 ) , true );
}

void CAggregateGlyph::ShowSetupDlgAt( CPoint& point )
{}

void CAggregateGlyph::SwitchConnections( BOOL bOnVirtualPins )
{
  if ( !m_pInternalGlyphs )
    return;
  int cPins = GetChildrenCount();
  while ( cPins-- )
  {
    CWireGlyph* pConnector = (CWireGlyph*) GetChildAt( cPins );
    FXString uidConnector;
    if ( !pConnector || !pConnector->GetUID( uidConnector ) )
      continue;
    CWireGlyph* Prototype = (CWireGlyph*) m_pInternalGlyphs->FindByUID( uidConnector );
    if ( !Prototype )
      continue;
    if ( bOnVirtualPins )
      Prototype->SwitchConnectionTo( pConnector );
    else
      pConnector->SwitchConnectionTo( Prototype );
  }
}

// CPortalGlyph

CPortalGlyph::CPortalGlyph( CSketchView *view , LPCTSTR uid ,
  CStringArray& inputs , CStringArray& outputs , CStringArray& dummy ) :
  CGadgetGlyph( view , uid , inputs , outputs , dummy ) 
{
  int iHeight = 2 * GLYPH_VERT_SPACE_TO_FIRST_PIN ;// (3 * GLYPH_VERT_SPACE_TO_FIRST_PIN) / 2;
  m_Size = CSize( GLYPH_GADGET_WIDTH , iHeight ) ;

  UpdatePins( inputs , outputs );
  m_ModesMenu.m_hMenu = NULL;
}

CPortalGlyph::CPortalGlyph( CSketchView *view , LPCTSTR uid ,
  CStringArray& inputs , CStringArray& outputs ,
  CStringArray& dummy , ViewType vt ) :
  CGadgetGlyph( view , uid , inputs , outputs , dummy , vt ) 
{
  int iHeight = ( 3 * GLYPH_VERT_SPACE_TO_FIRST_PIN ) / 2;
  m_Size = CSize( GLYPH_GADGET_WIDTH , iHeight ) ;

  SetViewType( vt );
  UpdatePins( inputs , outputs );
  m_ModesMenu.m_hMenu = NULL;
}

void CPortalGlyph::UpdatePins( CStringArray& inputs , CStringArray& outputs )
{
  int i;
  int nTmpInputs = m_nInputs;
  int nTmpOutputs = m_nOutputs;
  bool bNewInputsGoFirst = true , bNewOutputsGoFirst = true ;
  FXString uid;
  if ( inputs.GetSize() && !inputs[0].IsEmpty() )
  {
    uid = inputs[ 0 ] ;
    bNewInputsGoFirst = ( FindByUID( uid ) == NULL );
  }
  else if ( outputs.GetSize() && !outputs[0].IsEmpty() )
  {
    uid = outputs[ 0 ] ;
    bNewOutputsGoFirst = ( FindByUID( uid ) == NULL );
  }
  bool bInputExists = false , bOutputExists = false ;
  for ( i = 0; i < GetChildrenCount(); i++ )
  {
    CGlyph* pGlyph = GetChildAt( i );
    if ( pGlyph && ( pGlyph->GetUID( uid ) ) )
    {
      if ( inputs.GetSize() && (uid == ( LPCTSTR ) inputs[0]) )
      {
        bInputExists = true;
        continue ;
      }
      if ( outputs.GetSize() && (uid == ( LPCTSTR ) outputs[0]) )
      {
        bOutputExists = true;
        continue;
      }
      pGlyph->DisconnectWire( NULL );
      Remove( pGlyph , true );
      i--;
    }
  }
//   m_nInputs = m_nOutputs = 0 ;
  if ( inputs.GetSize() && !bInputExists )
  {
    Insert( new CConnectorGlyph( m_View , inputs[0] , GetViewType() , this ) , 0 );
    m_nInputs = 1;
  }
  if ( outputs.GetSize() && !bOutputExists )
  {
    Insert( new CConnectorGlyph( m_View , outputs[0] , GetViewType() , this ) , i + m_nInputs );
    m_nOutputs = 1 ;
  }
  ArrangeChildren();
  //Invalidate only if number of pin has been changed
  //for CSink && CAggregate
  if ( ( ( nTmpOutputs != m_nOutputs ) || ( nTmpInputs != m_nInputs ) ) )
  {
    m_View->Invalidate();
  }
}

void CPortalGlyph::ArrangeChildren()
{
  if ( !GetChildrenCount() )
    return;
  CRect rc( CPoint( 0 , 0 ) , CSize( GLYPH_CONNECTOR_WIDTH , GLYPH_CONNECTOR_HEIGHT ) );
  int space = m_Size.cy / 2 ;
  if ( m_nInputs )
  {
    CPoint offset( -rc.Width() , space - (rc.Height() / 2) );
    CPoint delta = offset - GetChildAt( 0 )->GetPos() + GetPos();
    GetChildAt( 0 )->OffsetPos( delta );
  } ;
  if ( m_nOutputs )
  {
    CPoint offset( m_Size.cx , space - ( rc.Height() / 2 ) );
    CPoint delta = offset - GetChildAt(m_nInputs )->GetPos() + GetPos();
    GetChildAt( m_nInputs )->OffsetPos( delta );
  }
}

void CPortalGlyph::ResetEvadeCount()
{
  CGlyph::ResetEvadeCount();
  CCompositeGlyph::ResetEvadeCount();
}

void CPortalGlyph::Draw( CDC* pDC , bool bActivityOnly )
{
  bool NeedUpdate = false;
  CGadget * pGadget = m_View->GetBuilder()->GetGadget( m_UID ) ;
  if ( !pGadget )
    return ;

  if ( pGadget && pGadget->Status().GetBool( STATUS_REDRAW , NeedUpdate )  
    && NeedUpdate )
  {
    pGadget->Status().WriteBool( STATUS_REDRAW , false );
  }
  if ( ( m_bNeedUpdate ) || ( NeedUpdate ) )
  {
    CStringArray inputs , outputs , duplex;
    if ( m_View->GetBuilder()->ListGadgetConnectors( m_UID , inputs , outputs , duplex ) )
    {
      UpdatePins( inputs , outputs );
      m_bNeedUpdate = m_View->GetBuilder()->IsGadgetSetupOn( m_UID );
    }
  }

  CRect rc( GetPos() , m_Size );
  COLORREF crBorder , crBody;
  if ( m_View->GetMyRectColors( this , &crBorder , &crBody ) )
  {
    if ( crBody == RGB( 255 , 255 , 255 ) )
      crBody = RGB( 220 , 220 , 220 ) ;
    if ( pGadget->GetGroupSelected() )
      crBody = pGadget->GetBodyColor() ;
    if ( !bActivityOnly )
    {
#ifdef GLYPHS_DRAW_PLAIN
      m_View->DrawRect( pDC , rc , crBorder , crBody );
#elif defined GLYPHS_DRAW_ROUND
      m_View->DrawRoundRect( pDC , rc , crBorder , crBody , &CPoint(10,10));
#elif defined GLYPHS_DRAW_EDGE
      m_View->DrawEdgeRect( pDC , rc , crBorder , crBody );
#endif
//       CRect rcIcon;
//       m_View->DrawIcon( pDC , m_View->GetMyIcon( this ) , rc , &crBorder , &crBody , rcIcon );
      CRect rcMode = rc;
      rcMode.left = rcMode.right - 8; 
      rcMode.bottom = rcMode.top + 6; rcMode.top++;
      m_View->DrawModeIcon( pDC , m_View->GetMyModeIcon( this ) , rcMode );
      LOGFONT font;
      if ( pGadget && m_View->GetMyFont( this , &crBorder , &font ) )
      {
        CRect rcText = rc ;
        rcText.OffsetRect( 4 , 0 );
//         rcText.bottom -= GLYPH_VERT_SPACE_TO_FIRST_PIN  ;
        m_View->DrawText( pDC , m_UID , rcText , crBorder , &font );
        FXString AddInfo = pGadget->GetAdditionalInfo() ;
        if ( AddInfo.GetLength() )
        {
          rcText.OffsetRect( 0 , GLYPH_VERT_SPACE_TO_FIRST_PIN - 6 ) ;
          m_View->DrawText( pDC , AddInfo , rcText , crBorder , &font );
        }
      }
    }
    CCompositeGlyph::Draw( pDC , bActivityOnly );
  }
}

BOOL CPortalGlyph::GetRgn( CRgn* pRgn , BOOL bSelf )
{
  CRect rc( GetPos() , m_Size );
  CRgn rgn;
  rgn.CreateRectRgnIndirect( rc );
  if ( bSelf || !CCompositeGlyph::GetRgn( pRgn ) )
    pRgn->CreateRectRgnIndirect( rc );
  else
    pRgn->CombineRgn( pRgn , &rgn , RGN_OR );
  rgn.DeleteObject();
  return TRUE;
}

CGlyph* CPortalGlyph::Intersect( CPoint& pt , BOOL bSelfRgn )
{
  CGlyph* pHit = CCompositeGlyph::Intersect( pt , bSelfRgn );
  if ( pHit )
    return pHit;
  CRect rc( GetPos() , m_Size );
  return ( rc.PtInRect( pt ) ) ? this : NULL;
}

void CPortalGlyph::DestroyIn()
{
  CCompositeGlyph::DestroyIn();
  m_nInputs = 0;
  m_nOutputs = 0;

  m_View->RemoveGadget( this , m_UID );
  m_UID.Empty();
}

void CPortalGlyph::CleanUp()
{
  CCompositeGlyph::CleanUp();
  m_UID.Empty();
}

void CPortalGlyph::ShowSetupDlgAt( CPoint& point )
{
  m_bNeedUpdate = TRUE;
  Tvdb400_ShowGadgetSetupDlg( m_View->GetBuilder() , m_UID , point );
  //m_View->Invalidate();
  //m_View->UpdateWindow();
}

void CPortalGlyph::FillContextMenu( CMenu* menu )
{
  menu->AppendMenu( MF_STRING , ID_CONTEXT_RENAME , "Rename" );
  menu->AppendMenu( MF_STRING , ID_CONTEXT_SETUP , "Setup" );
  DestroyContextSubmenus();

  FXString uid;
  int mode;
  if ( GetUID( uid ) && m_View->GetBuilder() && m_View->GetBuilder()->GetGadgetMode( uid , mode ) && m_ModesMenu.CreatePopupMenu() )
  {
    m_ModesMenu.AppendMenu( MF_STRING , ID_MODE_REJECT , "Reject" );
    m_ModesMenu.AppendMenu( MF_STRING , ID_MODE_TRANSMIT , "Transmit" );
    m_ModesMenu.AppendMenu( MF_STRING , ID_MODE_PROCESS , "Process" );
    UINT nChecked = ( ( mode == CGadget::mode_reject ) ? ID_MODE_REJECT : ( ( mode == CGadget::mode_transmit ) ? ID_MODE_TRANSMIT : ID_MODE_PROCESS ) );
    m_ModesMenu.CheckMenuItem( nChecked , MF_CHECKED | MF_BYCOMMAND );
    menu->AppendMenu( MF_POPUP , ( UINT_PTR ) m_ModesMenu.m_hMenu , "Modes" );
  }
}

void CPortalGlyph::DestroyContextSubmenus()
{
  if ( m_ModesMenu.m_hMenu )
  {
    m_ModesMenu.DestroyMenu();
    m_ModesMenu.m_hMenu = NULL;
  }
}
