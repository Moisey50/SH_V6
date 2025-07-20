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
  bQuit = FALSE;
}

CBigViewWnd::~CBigViewWnd()
{
}


BEGIN_MESSAGE_MAP(CBigViewWnd, CWnd)
	//{{AFX_MSG_MAP(CBigViewWnd)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBigViewWnd message handlers

void CBigViewWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
  CFont Font;
  Font.CreateFont( 70 ,      // Height
   0,                         // nWidth
   0,                         // nEscapement
   0,                         // nOrientation
   FW_SEMIBOLD,               // nWeight
   FALSE,                     // bItalic
   FALSE,                     // bUnderline
   0,                         // cStrikeOut
   ANSI_CHARSET,              // nCharSet
   OUT_DEFAULT_PRECIS,        // nOutPrecision
   CLIP_DEFAULT_PRECIS,       // nClipPrecision
   DEFAULT_QUALITY,           // nQuality
   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
   "Arial");                  // lpszFacename

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

    dc.TextOut( 15 , 15 + iStringCount * 80 , s ) ;
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
    SetWindowPos( NULL , 0 , 0 , 700 , 40 + 80 * m_iNStrings ,
      SWP_NOMOVE | SWP_NOZORDER  ) ;
  }
  
  Invalidate() ;

  return 1 ;
}
