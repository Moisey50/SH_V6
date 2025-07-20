#include "StdAfx.h"
#include <shbase\shbase.h>
#include <scriptdefinitions.h>
#include <fxfc/CSystemMonitorsEnumerator.h>

static bool   s_DoGlyphs = true;
static bool   s_DoFloatWnds = true;


CViewSectionParser::CViewSectionParser()
{
  m_bIsViewed = FALSE ;
  m_ViewOffset.x = m_ViewOffset.y = 0 ;
  m_dViewScale = 1.0 ;
}

CViewSectionParser::~CViewSectionParser( void )
{
  RemoveAll();
}

void    CViewSectionParser::SetConfig( bool DoGlyphs , bool DoFloatWnds )
{
  s_DoGlyphs = DoGlyphs;
  s_DoFloatWnds = DoFloatWnds;
}

void    CViewSectionParser::RemoveAll()
{
  m_Glyphs.RemoveAll();
  m_FloatWnds.RemoveAll();
}

int    CViewSectionParser::FindGlyph( LPCTSTR name )
{
  for ( int i = 0; i < (int) m_Glyphs.GetSize(); i++ )
  {
    if ( m_Glyphs[ i ].name == name )
      return i;
  }
  return -1;
}

int    CViewSectionParser::FindFloatWnd( LPCTSTR name )
{
  for ( int i = 0; i < (int) m_FloatWnds.GetSize(); i++ )
  {
    if ( m_FloatWnds[ i ].name == name )
      return i;
  }
  return -1;
}

bool CViewSectionParser::Parse( LPCTSTR script )
{
  if ( strlen(script) > 3800 )
  {
    TCHAR Buf[ 3801 ] ;
    memcpy( Buf , script , 3795 * sizeof( TCHAR ) ) ;
    memcpy( &Buf[ 3795 ] , _T( "...\0" ) , 4 * sizeof( TCHAR ) ) ;
    TRACE( _T( "%s\n" ) , Buf ) ;
  }
  else
    /*TRACE( _T( "%s\n" ) , script )*/;
  FXParser ip( script );
  FXSIZE pos = 0;
  FXString word;
  while ( (ip.GetWord( pos , word )) && (word != VIEW_SECTION_BEGIN) );
  if ( word != VIEW_SECTION_BEGIN ) return false;
  ip.TrimSeparators( pos );
  while ( ip.GetWord( pos , word ) )
  {
    if ( word == VIEW_SECTION_END )
      return true;
    else if ( word == VIEW_SECTION_MOVE )
    {
      FXString name;
      int x , y;
      FXPropertyKit param;
      ip.GetParamString( pos , param );
      if (
        (param.GetString( _T( "gadget" ) , name )) &&
        (param.GetInt( _T( "x" ) , x )) &&
        (param.GetInt( _T( "y" ) , y ))
        )
      {
        SetGlyph( name , x , y );
      }
      continue;
    }
    else if ( word == VIEW_SECTION_FLOATWND )
    {
      FXString name;
      double x , y , w , h;
      FXPropertyKit param;
      ip.GetParamString( pos , param );
      if (
        (param.GetString( _T( "name" ) , name )) &&
        (param.GetDouble( _T( "x" ) , x )) &&
        (param.GetDouble( _T( "y" ) , y )) &&
        (param.GetDouble( _T( "w" ) , w )) &&
        (param.GetDouble( _T( "h" ) , h ))
        )
      {
        FXString Selected ;
        param.GetString( _T( "Selected" ) , Selected ) ;
        SetFloatWnd( name , x , y , w , h , Selected );
      }
      continue;
    }
    else if ( word == VIEW_SECTION_GRAPH )
    {
      FXString name;
      double x , y , w , h;
      FXPropertyKit param;
      ip.GetParamString( pos , param );
      if (
        //             (param.GetString( _T( "name" ) , name )) &&
        (param.GetDouble( _T( "x" ) , x ) ) &&
        (param.GetDouble( _T( "y" ) , y ) ) &&
        (param.GetDouble( _T( "w" ) , w ) ) &&
        (param.GetDouble( _T( "h" ) , h ) ) &&
        (w > 0)  &&  (h > 0) 
        )
      {
        CDRect GraphPos( x , w , y < 0 ? 0 : y , h ) ;
        SetGraphViewPos( GraphPos );
        param.GetInt( _T( "x_view_off" ) , (int&) m_ViewOffset.x ) ;
        param.GetInt( _T( "y_view_off" ) , (int&) m_ViewOffset.y ) ;
        param.GetDouble( _T( "scale" ) , m_dViewScale );
      }
      param.GetInt( _T( "Viewed" ) , m_bIsViewed ) ;
      continue;
    }
    else if ( word == VIEW_SECTION_GRAPH )
    {
      FXString name;
      double x , y , w , h;
      FXPropertyKit param;
      ip.GetParamString( pos , param );
      if (
        //             (param.GetString( _T( "name" ) , name )) &&
        ( param.GetDouble( _T( "x" ) , x ) ) &&
        ( param.GetDouble( _T( "y" ) , y ) ) &&
        ( param.GetDouble( _T( "w" ) , w ) ) &&
        ( param.GetDouble( _T( "h" ) , h ) ) &&
        ( w > 0 ) && ( h > 0 )
        )
      {
        CDRect GraphPos( x , w , y < 0 ? 0 : y , h ) ;
        SetGraphViewPos( GraphPos );
        param.GetInt( _T( "x_view_off" ) , ( int& ) m_ViewOffset.x ) ;
        param.GetInt( _T( "y_view_off" ) , ( int& ) m_ViewOffset.y ) ;
        param.GetDouble( _T( "scale" ) , m_dViewScale );
      }
      param.GetInt( _T( "Viewed" ) , m_bIsViewed ) ;
      continue;
    }
    else if ( word == VIEW_SECTION_SHSTUDIO )
    {
      FXString name;
      int x , y , w , h;
      FXPropertyKit param;
      ip.GetParamString( pos , param );
      if (
        //             (param.GetString( _T( "name" ) , name )) &&
        ( param.GetInt( _T( "x" ) , x ) ) &&
        ( param.GetInt( _T( "y" ) , y ) ) &&
        ( param.GetInt( _T( "w" ) , w ) ) &&
        ( param.GetInt( _T( "h" ) , h ) ) &&
        ( w > 0 ) && ( h > 0 )
        )
      {
        CWnd * pMainWnd = AfxGetApp()->GetMainWnd() ;
        if ( pMainWnd )
        {
          CRect rcMain ;
          pMainWnd->GetWindowRect( &rcMain ) ;

          const CSystemMonitorsEnumerator * pMonitorEnum = CSystemMonitorsEnumerator::GetMonitorEnums() ;
          CRect rcDesktop = pMonitorEnum->GetFullDesktopRectangle() ;
          CRect rcSHStudio( x , y , x + w , y + h ) , rcIntersect ;

          if ( IntersectRect( &rcIntersect , &rcDesktop , &rcSHStudio ) )
          {
            pMainWnd->SetWindowPos( &CWnd::wndNoTopMost , x , y , w , h , SWP_DRAWFRAME | SWP_SHOWWINDOW ) ;
          }
          else
          {
            rcSHStudio = pMonitorEnum->GetMonitorRect( 0 ) ;
            pMainWnd->SetWindowPos( &CWnd::wndNoTopMost , rcSHStudio.left + 50 , rcSHStudio.top + 50 ,
              rcSHStudio.Width() - 100 , rcSHStudio.Height() - 100 , SWP_DRAWFRAME | SWP_SHOWWINDOW ) ;
          }
        }
      }
      continue;
    }
    else
      return false;
  }
  return false;
}

bool    CViewSectionParser::SetGlyph( LPCTSTR gadget , int x , int y )
{
  if ( s_DoGlyphs )
  {
    int pos = FindGlyph( gadget );
    if ( pos != -1 )
    {
      m_Glyphs[ pos ].pnt.x = x;
      m_Glyphs[ pos ].pnt.y = y;
      return true;
    }
    GlyphPosition gp = {gadget,x,y};
    m_Glyphs.Add( gp );
    return true;
  }
  return false;
}

bool    CViewSectionParser::SetFloatWnd( LPCTSTR name , 
  double x , double y , double w , double h , LPCTSTR pszSelected )
{
//   if ( s_DoFloatWnds )
//   {
    int pos = FindFloatWnd( name );
    if ( pos != -1 )
    {
      const CSystemMonitorsEnumerator * pMonitorsEnum =
        CSystemMonitorsEnumerator::GetMonitorEnums() ;

      x *= pMonitorsEnum->m_FullDesktopRect.Width() ;
      x += pMonitorsEnum->m_FullDesktopRect.left ;
      y *= pMonitorsEnum->m_FullDesktopRect.Height() ;
      y += pMonitorsEnum->m_FullDesktopRect.top ;
      w *= pMonitorsEnum->m_FullDesktopRect.Width() ;
      h *= pMonitorsEnum->m_FullDesktopRect.Height() ;
      CRect Allowed( pMonitorsEnum->m_FullDesktopRect ) ;
      Allowed.DeflateRect( 30 , 30 ) ;
      CPoint LeftTop( (int)x , (int)y ) ;
      if ( !Allowed.PtInRect( LeftTop ) )
      {
        x = Allowed.left ;
        y = Allowed.top ; 
      }
      int iCaptionHeight = GetSystemMetrics( SM_CYCAPTION );
      if ( h < iCaptionHeight * 2 )
        h = iCaptionHeight * 2 ;
      if ( w < 60 )
        w = 60 ;

      m_FloatWnds[ pos ].pnt.x = x;
      m_FloatWnds[ pos ].pnt.y = y;
      m_FloatWnds[ pos ].sz.cx = w;
      m_FloatWnds[ pos ].sz.cy = h;
      if ( pszSelected && *pszSelected )
        m_FloatWnds[ pos ].selected = pszSelected ;
      else
        m_FloatWnds[ pos ].selected.Empty() ;

      return true;
    }
    FloatWndPosition fwp( name,x,y,w,h, pszSelected) ;
    m_FloatWnds.Add( fwp );
    return true;
//   }
  return false;
}

bool    CViewSectionParser::GetGlyph( LPCTSTR name , int& x , int& y )
{
  if ( s_DoGlyphs )
  {
    int pos = FindGlyph( name );
    if ( pos != -1 )
    {
      x = m_Glyphs[ pos ].pnt.x;
      y = m_Glyphs[ pos ].pnt.y;
      return true;
    }
  }
  return false;
}

bool    CViewSectionParser::GetFloatWnd( 
  LPCTSTR name , double& x , double& y , double& w , double& h , FXString * pSelected )
{
  if ( s_DoFloatWnds )
  {
    int pos = FindFloatWnd( name );
    if ( pos != -1 )
    {
      x = m_FloatWnds[ pos ].pnt.x;
      y = m_FloatWnds[ pos ].pnt.y;
      w = m_FloatWnds[ pos ].sz.cx;
      h = m_FloatWnds[ pos ].sz.cy;
      if ( pSelected )
        *pSelected = m_FloatWnds[ pos ].selected ;
      return true;
    }
  }
  return false;
}

bool    CViewSectionParser::RenameGlyph( LPCTSTR oldname , LPCTSTR newname )
{
  int pos = FindGlyph( oldname );
  if ( pos == -1 ) return false;
  m_Glyphs[ pos ].name = newname;
  return true;
}

bool    CViewSectionParser::RemoveGlyph( LPCTSTR name )
{
  int pos = FindGlyph( name );
  if ( pos == -1 ) return false;
  m_Glyphs.RemoveAt( pos );
  return true;
}

FXString CViewSectionParser::GetViewSection()
{
  FXString retV( "" ) , tmpS;
  int strings = 0;
  retV += VIEW_SECTION_BEGIN; retV += EOL;
  if ( s_DoGlyphs )
  {
    for ( int i = 0; i < (int) m_Glyphs.GetSize(); i++ )
    {
      FXPropertyKit param;
      param.WriteString( _T( "gadget" ) , m_Glyphs[ i ].name );
      param.WriteInt( _T( "x" ) , m_Glyphs[ i ].pnt.x );
      param.WriteInt( _T( "y" ) , m_Glyphs[ i ].pnt.y );
      tmpS.Format( _T( "MOVE(%s)" )EOL , (LPCTSTR)param );
      retV += tmpS;
      strings++;
    }
  }
  if ( s_DoFloatWnds )
  {
    for ( int i = 0; i < (int) m_FloatWnds.GetSize(); i++ )
    {
      FXPropertyKit param;
      param.WriteString( _T( "name" ) , m_FloatWnds[ i ].name );

      param.WriteDouble( _T( "x" ) , m_FloatWnds[ i ].pnt.x /*/ ( double ) rcDesktop.Width()*/ );
      param.WriteDouble( _T( "y" ) , m_FloatWnds[ i ].pnt.y /*/ ( double ) rcDesktop.Height()*/ );
      param.WriteDouble( _T( "w" ) , m_FloatWnds[ i ].sz.cx /*/ ( double ) rcDesktop.Width()*/ );
      param.WriteDouble( _T( "h" ) , m_FloatWnds[ i ].sz.cy /*/ ( double ) rcDesktop.Height()*/ );
      param.WriteString( _T( "Selected" ) , m_FloatWnds[ i ].selected ) ;
      tmpS.Format( _T( VIEW_SECTION_FLOATWND"(%s)" )EOL , (LPCTSTR)param );
      retV += tmpS;
      strings++;
    }
  }
  { // Graph view saving
    FXPropertyKit param;
    param.WriteString( _T( "name" ) , _T( "GraphView" ) );
    param.WriteDouble( _T( "x" ) , m_GraphViewPos.left );
    param.WriteDouble( _T( "y" ) , m_GraphViewPos.top );
    param.WriteDouble( _T( "w" ) , m_GraphViewPos.right );
    param.WriteDouble( _T( "h" ) , m_GraphViewPos.bottom );
    param.WriteInt( _T( "x_view_off" ) , m_ViewOffset.x ) ;
    param.WriteInt( _T( "y_view_off" ) , m_ViewOffset.y ) ;
    param.WriteDouble( _T( "scale" ) , m_dViewScale );
    param.WriteInt( _T( "Viewed" ) , m_bIsViewed ) ;
    tmpS.Format( _T( VIEW_SECTION_GRAPH"(%s)" )EOL , (LPCTSTR)param );
    retV += tmpS;
    strings++;
  }
  { // SHStudio position saving
    CWnd * pMainWnd = AfxGetApp()->GetMainWnd() ;
    CRect rcMain ;
    pMainWnd->GetWindowRect( &rcMain ) ;
    CRect rcDesktop = CSystemMonitorsEnumerator::GetMonitorEnums()
      ->GetFullDesktopRectangle() ;
    FXPropertyKit param;
    param.WriteString( _T( "name" ) , _T( "SHStudioView" ) );
    param.WriteDouble( _T( "x" ) , rcMain.left );
    param.WriteDouble( _T( "y" ) , rcMain.top );
    param.WriteDouble( _T( "w" ) , rcMain.Width() ) ;
    param.WriteDouble( _T( "h" ) , rcMain.Height() ) ;
    tmpS.Format( _T( VIEW_SECTION_SHSTUDIO"(%s)" )EOL , ( LPCTSTR ) param );
    retV += tmpS;
    strings++;
  }
  if ( strings )
  {
    retV += VIEW_SECTION_END; retV += EOL;
    return retV;
  }
  retV.Empty();
  return retV;
}

void CViewSectionParser::GetGraphViewPos( CDRect& Pos )
{
  Pos = m_GraphViewPos ;
}

void CViewSectionParser::SetGraphViewPos( const CDRect& Pos )
{
  m_GraphViewPos = Pos ;
}


bool    CViewSectionParser::GetViewPropertyString( FXString& prop )
{
  if ( s_DoGlyphs )
  {
    int count = 0;
    FXPropertyKit retV;

    for ( int i = 0; i < (int) m_Glyphs.GetSize(); i++ )
    {
      FXPropertyKit param;
      param.WriteInt( _T( "x" ) , m_Glyphs[ i ].pnt.x );
      param.WriteInt( _T( "y" ) , m_Glyphs[ i ].pnt.y );
      retV.WriteString( m_Glyphs[ i ].name , param );
      count++;
    }
    prop = retV;
    return count != 0;
  }
  return false;
}