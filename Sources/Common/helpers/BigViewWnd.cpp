// BigViewWnd.cpp : implementation file
//

#include "stdafx.h"
#include "BigViewWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBigViewWnd

CBigViewWnd::CBigViewWnd()
{
  m_BigText = "EMPTY" ;
  m_iNStrings = 1 ;
}

CBigViewWnd::~CBigViewWnd()
{
}


BEGIN_MESSAGE_MAP(CBigViewWnd, CWnd)
	//{{AFX_MSG_MAP(CBigViewWnd)
	ON_WM_PAINT()
    ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBigViewWnd message handlers

void CBigViewWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
//   CFont Font;
//   Font.CreateFont( 16 ,       // Height
//    0,                         // nWidth
//    0,                         // nEscapement
//    0,                         // nOrientation
//    FW_SEMIBOLD,               // nWeight
//    FALSE,                     // bItalic
//    FALSE,                     // bUnderline
//    0,                         // cStrikeOut
//    ANSI_CHARSET,              // nCharSet
//    OUT_DEFAULT_PRECIS,        // nOutPrecision
//    CLIP_DEFAULT_PRECIS,       // nClipPrecision
//    DEFAULT_QUALITY,           // nQuality
//    DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
//    "Arial");                  // lpszFacename

  CFont Font;
  LOGFONT lf;
  memset( &lf , 0 , sizeof( LOGFONT ) ); // zero out structure
  lf.lfHeight = 18;                // request a 12-pixel-height font
  _tcsncpy_s( lf.lfFaceName , LF_FACESIZE ,
    _T( "Arial" ) , 7 );           // request a face name "Arial"
  VERIFY( Font.CreateFontIndirect( &lf ) ); // create the font

  CRect r ;
  
  GetClientRect( &r ) ;
  dc.FillSolidRect( &r , 0x00ffffff ) ;

  CFont * pOldFont = (CFont*)dc.SelectObject( &Font ) ;
  CPen * pOldPen = (CPen*)dc.SelectStockObject( WHITE_PEN ) ;

  CString Ori( m_BigText ) ;
  CString s ;

  int iStringCount = 0 ;
  int iPos1 = 1 ;
  while ( iPos1 >= 0 )
  {
    iPos1  = Ori.Find( ';' ) ;
    if ( iPos1 >= 0 )
    {
      s = Ori.Left( iPos1 ) ;
      Ori = Ori.Right( Ori.GetLength() - iPos1 - 1 ) ;
    }
    else
      s = Ori ;

    COLORREF defColor =  RGB(0,0,0);

    if (s.Find("Las")>-1)
      dc.SetTextColor(m_LaserTempColor);
    if (s.Find("Pol")>-1)
      dc.SetTextColor(m_PolygonTempColor);
    if (s.Find("Win")>-1)
      dc.SetTextColor(m_WindowTempColor);
    if (s.Find("Col")>-1)
      dc.SetTextColor(m_CollimTempColor);
    if (s.Find("Eos")>-1)
      dc.SetTextColor(m_EosSideTempColor);
    if (s.Find("Sos")>-1)
      dc.SetTextColor(m_SosSideTempColor);
    if (s.Find("T =")>-1 || s.Find("X =")>-1)
      dc.SetTextColor(defColor);

    
    dc.TextOut( 15 , 15 + iStringCount * 20 , s ) ;
    iStringCount++ ;
  }

  dc.SelectObject( pOldFont ) ;
  dc.SelectObject( pOldPen ) ;

	// Do not call CWnd::OnPaint() for painting messages
}

int CBigViewWnd::SetOutputText(const char *OutText)
{
  m_BigText = OutText ;

  m_iNStrings = 1 ;
  for ( int i = 0 ; i < m_BigText.GetLength() ; i++ )
    m_iNStrings += (m_BigText[ i ] == ';') ;

  if ( ShowWindow( SW_SHOWNA ) )
  {
    SetWindowPos( NULL , 0 , 0 , 500 , 40 + 20 * m_iNStrings ,
      SWP_NOMOVE | SWP_NOZORDER  ) ;
  }
  
  Invalidate() ;

  return 1 ;
}

void  CBigViewWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
 this->ShowWindow(SW_HIDE);
 
}