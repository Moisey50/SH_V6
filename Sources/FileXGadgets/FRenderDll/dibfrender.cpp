// DIBFRender.cpp: implementation of the CDIBFRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include <gadgets\ContainerFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\VideoFrame.h>
#include <imageproc\draw_over.h>
#include <imageproc\imagebits.h>
#include <helpers\FramesHelper.h>
#include "DIBFRender.h"
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")



#define BOUNDE(x,min,max)        ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define SCALEF 1.5
#define DIB_EVENT(EVENT,DATA) if (m_CallBack) m_CallBack(EVENT,(void*)DATA,m_CallBackParam,this); 

__forceinline void _swapRB( LPBYTE ptr )
{
  BYTE t = ptr[ 2 ];
  ptr[ 2 ] = ptr[ 0 ];
  ptr[ 0 ] = t;
}

__forceinline cmplx GetCenter( const pTVFrame frame )
{
  return cmplx( 0.5 * (double) GetWidth( frame ) , 0.5 * (double) GetHeight( frame ) ) ;
}

char FrenderHelpText[] =
"Left Button - measure length;"
"Left Button when 'I' is down - toggle show pixel info as tool tip;"
"Left Button when 'R' is down - toggle RGB info;"
"Right Button - mark rectangle;"
"Middle Button (Wheel) - switch zoom;"
"  sequence:fit, x1, x2, x4, x8, x16, fit...;"
"Wheel Scroll: vertical scroll;"
"Wheel Scroll + SHIFT: horizontal scroll;"
"Wheel Scroll + CTRL: zoom control;"
;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDIBFRender::CDIBFRender( LPCTSTR name ) :
  CDIBView( name ) ,
  m_OrgFrame( NULL ) ,
  m_pTmpFrame( NULL ) ,
  m_RectanglePen( PS_SOLID , 1 , RGB( 255 , 0 , 0 ) ) ,
  m_bCutOverlapMode( FALSE ) ,
  m_iCursorForm( CURSOR_NO_SELECTION ) ,
  m_LastCalculatedCursor( NULL ) ,
  m_CurrentCursor( NULL ) ,
  m_LastCursorPt( -1 , -1 ) ,
  m_pView( NULL ) ,
  m_bShowLabel( SL_DISABLE ) ,
  m_dLastLineLength( 0. ) ,
  m_sUnits( "pix" ) ,
  m_bTrackingTT( false ) ,
  m_bTrackingSwitch( false ) ,
  m_bShowRGB( false ) ,
  m_dLastUpdateTime( 0. ) ,
  m_iMouseMoveTimer( 0 ) ,
  m_iFrameInterval( 1000 ) ,
  m_dwLastFrameTime( 0 ) ,
  m_PointOfInterest( -1 , -1 ) ,
  m_bSomeSelected( false ) ,
  m_iSaveImage( 0 ) ,
  m_iSendOutImage( 0 ) ,
  m_NewImageCenter( 0 , 0 )
{
  m_Rect.SetRectEmpty();
  //   CreateFontWithSize(12);
  memset( m_ScrollOffsets , 0 , sizeof( m_ScrollOffsets ) ) ;
}

CDIBFRender::~CDIBFRender()
{
  if ( m_pView )
    m_pView->Detach() ;
  m_Lock.Lock( INFINITE , "CDIBFRender::~CDIBRender" ) ;
  if ( m_pTmpFrame )
  {
    ((CDataFrame*) m_pTmpFrame)->Release() ;
    m_pTmpFrame = NULL ;

  }
  if ( m_OrgFrame )
  {
    ((CDataFrame*) m_OrgFrame)->Release() ;
    m_OrgFrame = NULL;
  }
  m_Lock.Unlock() ;

  for ( int i = 0; i <= m_Fonts.GetUpperBound(); i++ )
  {
    if ( m_Fonts[ i ] )
    {
      m_Fonts[ i ]->DeleteObject();
      delete m_Fonts[ i ];
      m_Fonts[ i ] = NULL;
    }
  }
}


BEGIN_MESSAGE_MAP( CDIBFRender , CDIBView/*CDIBViewBase*/ )
  //{{AFX_MSG_MAP(CDIBFRender)
  ON_WM_MOUSEMOVE()
  ON_WM_SETFOCUS()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_MOUSEWHEEL()
  ON_WM_KEYDOWN()
  //   ON_WM_RBUTTONDOWN()
  //   ON_WM_RBUTTONUP()
  ON_WM_MBUTTONDOWN()
  ON_WM_LBUTTONDBLCLK()
  //}}AFX_MSG_MAP
  ON_WM_SIZE()
  ON_WM_MOUSELEAVE()
  ON_WM_KEYUP()
  ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDIBFRender::Create( CWnd* pParentWnd , DWORD dwAddStyle , UINT nID , LPCTSTR szWindowName )
{
  BOOL res = CDIBView::Create( pParentWnd , dwAddStyle | WS_CLIPCHILDREN , nID , szWindowName );

  if ( res )
  {
    UpdateWindow();

    LPCTSTR pClass = AfxRegisterWndClass( NULL ) ;
    CRect rPos( 220 , 50 , 800 , 200 ) ;
    if ( !m_HelpWnd.CreateEx( WS_EX_TOPMOST , pClass , "FRender Help" ,
      WS_MINIMIZEBOX | WS_OVERLAPPED | WS_CAPTION | WS_VISIBLE | WS_SYSMENU ,
      rPos , this , 0 ) )
    {
      DWORD ErrCode = GetLastError() ;
      ASSERT( 0 ) ;
    }
    m_HelpWnd.ShowWindow( SW_HIDE ) ;
  }

  SetCursor( LoadCursor( NULL , IDC_CROSS ) ) ; // set real cursor

  return (res);
}

void CDIBFRender::ShowScrollBar( UINT nBar , BOOL bShow )
{
  UINT mask = (nBar == SB_VERT) ? FL_VERT : (nBar == SB_HORZ) ? FL_HORZ : 0;
  if ( bShow )
    m_ScrBarEnabled |= mask;
  else
    m_ScrBarEnabled &= ~mask;
  CDIBViewBase::ShowScrollBar( nBar , bShow );
}

bool CDIBFRender::DoScrolling( int& x , int& y , RECT& rc )
{
  UINT oldScrBarEnabled = m_ScrBarEnabled;
  ShowScrollBar( SB_HORZ , x < 0 );
  ShowScrollBar( SB_VERT , y < 0 );
  if ( x < 0 )
  {
    int max , min;
    GetScrollRange( SB_VERT , &min , &max );
    x = ((rc.right - (int) (m_Scale* (m_Width/* - rc.right + rc.left*/)))*GetScrollPos( SB_HORZ )) / (max - min);
    //    x=GetScrollPos(SB_HORZ) ;
  }
  if ( y < 0 )
  {
    int max , min;
    GetScrollRange( SB_VERT , &min , &max );
    y = ((rc.bottom - (int) (m_Scale* (m_Height/* - rc.bottom + rc.top*/)))*GetScrollPos( SB_VERT )) / (max - min);
    //       y=GetScrollPos(SB_VERT) ;
  }
  m_ScrOffset = CPoint( x , y );
  return (oldScrBarEnabled != m_ScrBarEnabled);
}

bool CDIBFRender::Draw( HDC hdc , RECT& rc )
{
  m_dLastUpdateTime = GetHRTickCount() ;
  if ( m_iMouseMoveTimer )
  {
    KillTimer( m_iMouseMoveTimer ) ;
    m_iMouseMoveTimer = 0 ;
  }

  SetMapMode( hdc , MM_TEXT ) ;
  HDC hMemDC = CreateCompatibleDC( hdc ) ;
  if ( !hMemDC )
    return false ;
  HBITMAP hMemBitmap = CreateCompatibleBitmap(
    hdc , rc.right - rc.left , rc.bottom - rc.top ) ;
  if ( !hMemBitmap )
  {
    DeleteDC( hMemDC ) ;
    return false ;
  }
  HGDIOBJ hOld = SelectObject( hMemDC , hMemBitmap ) ;
  //bool res=CDIBView::Draw(hdc,rc);
  bool res = CDIBView::Draw( hMemDC , rc ) ;
  bool bImageReplaced = false ;
  m_dPictureDrawTime = GetHRTickCount() - m_dLastUpdateTime ;
  if ( res )
  {
    {
      FXAutolock al( m_Lock , "CDIBFRender::Draw" ) ;
      if ( m_pTmpFrame )
      {
        int iNLines = 0 , iNLinesNow = 0 , iNTmpNow = 0 ;;
        if ( m_OrgFrame )
          ((CDataFrame*) m_OrgFrame)->Release() ;
        m_OrgFrame = m_pTmpFrame ;
        m_OrgFrameContent = m_TmpFrameContent ;
        m_pTmpFrame = NULL ;

        m_GraphicsData.RemoveData( true ) ;
        bImageReplaced = true ;
      }

      if ( m_OrgFrame )
      {
        DrawData( hMemDC );
      }
    }
    BitBlt( hdc , 0 , 0 , rc.right - rc.left , rc.bottom - rc.top ,
      hMemDC , 0 , 0 , SRCCOPY ) ;

    if ( bImageReplaced ) // from this moment the image is on screen
    {
      GetCursorPos( &m_CursorPtWhenUpdated ) ;
    }

    SelectObject( hMemDC , hOld ) ;
    if ( m_iSaveImage || m_iSendOutImage )
    {
      BITMAPINFO bmih ;
      memset( &bmih , 0 , sizeof( bmih ) ) ;
      bmih.bmiHeader.biSize = sizeof( BITMAPINFOHEADER ) ;
      int iRes = GetDIBits( hMemDC , hMemBitmap ,
        0 , 0 , NULL , &bmih , DIB_RGB_COLORS ) ;
      if ( iRes )
      {
        TVFrame FrameForSave ;

        FrameForSave.lpBMIH = (BITMAPINFOHEADER*) malloc( bmih.bmiHeader.biSize + bmih.bmiHeader.biSizeImage ) ;
        FrameForSave.lpData = NULL ;
        memcpy( FrameForSave.lpBMIH , &bmih.bmiHeader , bmih.bmiHeader.biSize ) ;
        LPBYTE pData = (LPBYTE) (FrameForSave.lpBMIH + 1) ;
        FrameForSave.lpBMIH->biCompression = BI_RGB ;
        FrameForSave.lpBMIH->biBitCount = 24 ;
        FrameForSave.lpBMIH->biWidth &= ~(3) ;
        FrameForSave.lpBMIH->biSizeImage = FrameForSave.lpBMIH->biWidth *  FrameForSave.lpBMIH->biHeight * 3 ;
        int iY = bmih.bmiHeader.biHeight ;
        iRes = GetDIBits( hMemDC , hMemBitmap ,
          0 , iY , pData , (BITMAPINFO*)FrameForSave.lpBMIH , DIB_RGB_COLORS ) ;
        if ( m_iSendOutImage > 0 )
        {
          CVideoFrame * pOutImage = CVideoFrame::Create() ;
          int iAllocSize = sizeof( *( pOutImage->lpBMIH ) )
            + FrameForSave.lpBMIH->biSizeImage ;
          pOutImage->lpBMIH = ( BITMAPINFOHEADER* ) malloc( iAllocSize ) ;
          memcpy( pOutImage->lpBMIH , FrameForSave.lpBMIH , iAllocSize ) ;
          pOutImage->SetLabel( "Image+Graphics" ) ;
          pOutImage->ChangeId( m_OrgFrame->GetId() ) ;
          DIB_EVENT( DIBVE_OUT_VFRAME , pOutImage ) ;
          m_iSendOutImage-- ;
        }
        if ( m_iSaveImage > 0 )
        {
          if ( FxVerifyCreateDirectory( m_ImagesDir ) )
          {
            FXString Prefix( m_FileNamePrefix ) ;
            Prefix.MakeLower() ;
            if ( Prefix == "<label>" )
            {
              int iDotPos = ( int ) m_sVideoFrameLabel.ReverseFind( '.' ) ;
              int iSlashPos = ( int ) m_sVideoFrameLabel.ReverseFind( '\\' ) ;
              int iSlashPos2 = ( int ) m_sVideoFrameLabel.ReverseFind( '/' ) ;
              if ( iDotPos >= 0 )
              {
                if ( iSlashPos >= 0 )
                {
                  if ( iSlashPos < iSlashPos2 )
                    iSlashPos = iSlashPos2 ;
                  Prefix = m_sVideoFrameLabel.Mid( iSlashPos + 1 , iDotPos - iSlashPos - 1 ) ;
                }
                else if ( iSlashPos2 >= 0 )
                  Prefix = m_sVideoFrameLabel.Mid( iSlashPos2 + 1 , iDotPos - iSlashPos2 - 1 ) ;
                else
                  Prefix = m_sVideoFrameLabel.Left( iDotPos ) ;
              }
              else
                Prefix = m_sVideoFrameLabel ;
            }
            else
              Prefix = m_FileNamePrefix ;
            FXString CombineFileName = m_ImagesDir
              + Prefix + GetTimeStamp( "-" , m_FileNameSuffix ) + ".bmp" ;
            saveSH2BMP( ( LPCTSTR ) CombineFileName , FrameForSave.lpBMIH ) ;
          }
          m_iSaveImage-- ;
        }
        free( FrameForSave.lpBMIH ) ;
      }
    }
  }
  DeleteObject( hMemBitmap ) ;
  DeleteDC( hMemDC ) ;
  m_dFullDrawTime = GetHRTickCount() - m_dLastUpdateTime ;
  return res;
}

void CDIBFRender::Render( const CDataFrame* pDataFrame )
{
  if ( (pDataFrame) && (!Tvdb400_IsEOS( pDataFrame )) )
  {
    double dSTartLoadTime = GetHRTickCount() ;

    const CTextFrame * pCenter = pDataFrame->GetTextFrame( _T( "SetCenter" ) );
    if ( pCenter )
    {
      FXPropKit2 Content = pCenter->GetString() ;
      CPoint NewCenter ;
      if ( Content.GetInt( "Xc" , NewCenter.x ) && Content.GetInt( "Yc" , NewCenter.y ) )
        m_NewImageCenter = NewCenter ;
      int iScale ;
      if ( Content.GetInt( "Scale" , iScale ) )
          m_iNewScale = iScale ;
    }

    const CTextFrame * pSaveImage = pDataFrame->GetTextFrame( _T( "SaveImage" ) );
    if ( pSaveImage )
    {
      if ( pSaveImage->GetString().IsEmpty() )
        m_iSaveImage = 0 ;
      else
      {
        FXString DirNameAndOrNumber = pSaveImage->GetString();
        DirNameAndOrNumber.Trim( " \t\r\n" ) ;
        if ( isdigit(DirNameAndOrNumber[0]) )
        {
          m_iSaveImage = atoi( DirNameAndOrNumber ) ;
          FXSIZE iPos = DirNameAndOrNumber.FindOneOf( " ,;\t(_" ) ;
          if ( iPos >= 0 )
          {
            DirNameAndOrNumber.Delete( 0 , iPos + 1 ) ;
            if ( DirNameAndOrNumber.GetLength() >= 3 )
              m_ImagesDir = DirNameAndOrNumber ;
          }
        }
        else
        {
          if (DirNameAndOrNumber.GetLength() >= 3)
          {
            m_ImagesDir = DirNameAndOrNumber ;
            m_iSaveImage = 1 ;
          }
        }
        TCHAR cLastChar = m_ImagesDir[ m_ImagesDir.GetLength() - 1 ] ;
        if ( ( cLastChar != _T( '\\' ) ) && ( cLastChar != _T( '/' ) ) )
          m_ImagesDir += _T( '/' ) ;
      }
    }

    const CTextFrame * pSaveImagePrefix = pDataFrame->GetTextFrame( _T( "ImageSavePrefix" ) );
    if (pSaveImagePrefix)
    {
      m_FileNamePrefix = pSaveImagePrefix->GetString() ;
      m_FileNamePrefix.Trim() ;
    }
    const CTextFrame * pSaveImageSuff = pDataFrame->GetTextFrame( _T( "ImageSaveSuffix" ) );
    if ( pSaveImageSuff )
      m_FileNameSuffix = pSaveImageSuff->GetString() ;

    bool bContainer = pDataFrame->IsContainer() ;
    if ( !bContainer && !pCenter
      && (pDataFrame->GetDataType() == text) )
    {
      const CTextFrame * pCommand = pDataFrame->GetTextFrame();
      if ( pCommand )
      {
        LPCTSTR pLabel = pCommand->GetLabel() ;
        if ( _tcscmp( pLabel , _T( "Resize" ) ) == NULL )
        {
          CRect cr ;
          GetParent()->GetClientRect( &cr ) ;
          PostMessage( WM_SIZE , SIZE_RESTORED ,
            (LPARAM) ((cr.Height() << 16) + (cr.Width())) ) ;
          Invalidate() ;
        }
        else
        {
          FXPropKit2 pk( pCommand->GetString() );

          UINT hUI;
          int x , y , w , h;
          int iRes = (pk.GetUIntOrHex( "wh" , hUI ) != false) ;
          if ( pk.GetInt( "x" , x ) && pk.GetInt( "y" , y )
            && pk.GetInt( "w" , w ) && pk.GetInt( "h" , h ) )
          {
            FXString wn;
            iRes += (pk.GetString( "wn" , wn ) != false);
            CWnd * pParent = GetParent();
            HWND hParent = NULL ;
            CRect WinPos( 0 , 0 , 0 , 0 ) , ParentPos( 0 , 0 , 0 , 0 ) ;
            if ( pParent )
            {
              hParent = pParent->m_hWnd;
              if ( IsWindow( hParent ) )
              {
                WINDOWPLACEMENT WPl;
                ::GetWindowPlacement( (HWND) hParent , &WPl );
                ParentPos = WPl.rcNormalPosition;
                SetWindowPos( NULL , 0 , 0 , w , h ,
                  SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
                TRACE( "\n%s(%d,%d,%d,%d) OldPos(0x%08X)=(%d,%d,%d,%d) "
                  "NewPos=(%d,%d,%d,%d) " , (LPCTSTR) wn ,
                  x , y , w , h , hParent ,
                  ParentPos.left , ParentPos.top , ParentPos.Width() , ParentPos.Height() ,
                  0 , 0 , w , h );
              }
            }
          }
        }
        return;
      } // pCommand

    }

    if ( !pDataFrame->GetVideoFrame() )
    {
      if ( m_NewImageCenter.x )
      {
        if ( m_iNewScale != -2 )
          SetScale( m_iNewScale , m_NewImageCenter ) ;
        else
          SetNewViewCenter( m_NewImageCenter ) ;
      }
      else if ( m_iNewScale != -2 )
        SetScale( m_iNewScale ) ;

      m_NewImageCenter.x = m_NewImageCenter.y = 0 ;
      m_iNewScale = -2 ;
      return ;
    }

    DWORD dwNow = GetTickCount() ;
    m_iFrameInterval = dwNow - m_dwLastFrameTime ;
    m_dwLastFrameTime = dwNow ;

    CFramesIterator* Iterator = (m_bCutOverlapMode) ?
      pDataFrame->CreateFramesIterator( vframe ) : NULL ;
    if ( Iterator )
    {
      const CRectFrame* rectFrame = pDataFrame->GetRectFrame();
      if ( !rectFrame->Attributes()->IsEmpty() )
        rectFrame = NULL ;
      CVideoFrame* cutFrame = (CVideoFrame*) Iterator->Next();
      CVideoFrame* bgFrame = (CVideoFrame*) Iterator->Next();
      if ( rectFrame && cutFrame && bgFrame )
      {
        if ( !_tcschr( rectFrame->GetLabel() , _T( ':' ) ) )
          m_Rect = *rectFrame;
        CVideoFrame* Frame = CVideoFrame::Create( _draw_over( bgFrame , cutFrame , m_Rect.left , m_Rect.top ) );
        if ( (Frame) && (Frame->lpBMIH) )
        {
          LoadFrame( Frame , pDataFrame );
          SetExtData( pDataFrame ) ;
        }
        else
          LoadFrame( NULL );
        Frame->Release( Frame );
      }
      else if ( cutFrame && cutFrame->lpBMIH )
      {
        if ( rectFrame && !_tcschr( rectFrame->GetLabel() , _T( ':' ) ) )
          m_Rect = *rectFrame;
        else
          m_Rect.SetRectEmpty();
        LoadFrame( cutFrame , pDataFrame );
      }
      else
      {
        m_Rect.SetRectEmpty();
        LoadFrame( NULL , pDataFrame );
      }
      delete Iterator;
    }
    else
    {
      m_Rect.SetRectEmpty();
      const CVideoFrame* Frame = pDataFrame->GetVideoFrame();
      if ( (Frame) && (Frame->lpBMIH) )
      {
        // !!!!!!!!!!!!!!!!!!!    Removed work with IP cameras
        //
        //if (Frame->lpBMIH->biCompression == BI_H264)
        //{
         // DWORD dwPacketSize =  Frame->lpBMIH->biSizeImage;
         // unsigned char*  pImage = new unsigned char[dwPacketSize]();
         // memcpy(pImage, (unsigned char*)&Frame->lpBMIH[1], dwPacketSize);
         // H264_PLAY_InputData( m_nIndex , pImage, dwPacketSize);
         // delete pImage;
        //}
      //  else
        m_sVideoFrameLabel = Frame->GetLabel() ;
        LoadFrame( Frame , pDataFrame );
        SetExtData( pDataFrame ) ;
      }
    }
    double dLoadFrameTime = GetHRTickCount() - dSTartLoadTime ;
    if ( m_NewImageCenter.x )
    {
      SetNewViewCenter( m_NewImageCenter ) ;
      m_NewImageCenter.x = m_NewImageCenter.y = 0 ;
    }
  }
  //else
  //  LoadFrame(NULL);
}

void CDIBFRender::DrawData( HDC hdc )
{
  m_iLastNearest = 100000 ;
  LoadGraphics( m_OrgFrame ) ;
  CheckCursorForm( false , m_LastCursorPt ) ;
  bool res = DrawRectangles( CDC::FromHandle( hdc ) );
  res = DrawFigures( CDC::FromHandle( hdc ) ) || res;
  res = DrawTexts( CDC::FromHandle( hdc ) ) || res;
  if ( m_bTrackingTT )
    DrawToolTip( CDC::FromHandle( hdc ) ) ;
}

bool CDIBFRender::Pic2Scr( DPOINT& pt , CPoint& res )
{
  if ( m_ScrScale < 0.01 )
    res = CPoint( 0 , 0 );
  else
  {
    CDPoint point( pt ) ;
    point.x += m_IntBmOffset.x + 0.5 ;
    point.y += m_IntBmOffset.y + 0.5 ;
    //     point.x *= m_ScrScale ;
    //     point.y *= m_ScrScale ;
    point = point * m_ScrScale ;
    point.x += m_ScrOffset.x ;
    point.y += m_ScrOffset.y ;
    //     if ( point.y < 0. )
    //       point.y = 0. ;
    res = CPoint( ROUND( point.x ) , ROUND( point.y ) ) ;
  }
  return true;
}

bool CDIBFRender::Pic2Scr( POINT& pt , CPoint& res )
{
  CDPoint point( pt ) ;
  return Pic2Scr( point , res );
}

bool CDIBFRender::CreateFontWithSize( UINT iSize )
{
  CFont * pFont = new CFont;
  if ( pFont->CreateFont(
    (int) (iSize*SCALEF + 0.5) ,              // nHeight
    0 ,                         // nWidth
    0 ,                         // nEscapement
    0 ,                         // nOrientation
    FW_NORMAL ,                 // nWeight
    FALSE ,                     // bItalic
    FALSE ,                     // bUnderline
    0 ,                         // cStrikeOut
    ANSI_CHARSET ,              // nCharSet
    OUT_DEFAULT_PRECIS ,        // nOutPrecision
    CLIP_DEFAULT_PRECIS ,       // nClipPrecision
    DEFAULT_QUALITY ,           // nQuality
    DEFAULT_PITCH | FF_SWISS ,  // nPitchAndFamily
    _T( "Arial" ) ) )              // lpszFacename 
  {
    m_Fonts.Add( pFont );
    m_Sizes.Add( iSize );
    return true;
  }
  return false;
}

bool CDIBFRender::LoadGraphics( const CDataFrame * df )
{
  if ( !m_GraphicsData.IsFilled() && df )
  {
    double dSTartTime = GetHRTickCount() ;
    CFramesIterator* rectIter = df->CreateFramesIterator( rectangle );
    if ( rectIter )
    {
      FXString atribS;
      CPoint Cursor( ROUND( m_LastCursor.real() ) , ROUND( m_LastCursor.imag() ) ) ;
      CRectFrame* rectFrame = (CRectFrame*) rectIter->Next( DEFAULT_LABEL );
      int iCurSelect = -1 ;
      LPCSTR hCursor = NULL ;
      for ( int i = 0 ; i < m_GraphicsData.m_Rects.GetCount() ; i++ )
        m_GraphicsData.m_Rects[ i ].SetMatched( false ) ;
      bool bEmptyFilled = false ;
      while ( rectFrame )
      {
        if ( !rectFrame->Attributes()->IsEmpty() )
        {
          int i = 0 ;
          for ( ; i < m_GraphicsData.m_Rects.GetCount() ; i++ )
          {
             FXRectangle& Rect = m_GraphicsData.m_Rects.GetAt( i ) ;
            if ( Rect.m_ObjectName == rectFrame->GetLabel() )
            {
              if ( Rect.m_ObjectName.IsEmpty() )
              {
                if ( bEmptyFilled )
                  continue ;
                else
                  bEmptyFilled = true ;
              }
              Rect.SetMatched( true ) ;
              FXSIZE color = 0x000000ff ;
              if ( rectFrame->Attributes()->GetString( "color" , atribS ) )
                ConvToBinary( atribS , color ) ;
              Rect.m_Color = (DWORD)color ;
              int iThickness = 1 ;
              rectFrame->Attributes()->GetInt( "thickness" , iThickness ) ;
              Rect.m_dwLineWidth = iThickness ;
              int iStyle = 0 ;
              rectFrame->Attributes()->GetInt( "style" , iStyle ) ;
              Rect.m_Style = iStyle ;
              if ( rectFrame->Attributes()->GetString( "back" , atribS ) )
                Rect.m_Back = ( int ) ConvToBinary( atribS ) ;
              if ( Rect.m_ObjectName == m_GraphicsData.m_SelectedForAdjustmentName )
                Rect.m_bSelectedForAdjustment = true ;
              if ( (m_GraphicsData.m_iSelectedIndex == i)
                && (m_GraphicsData.m_iSelectedType == SELECTED_RECT) )
              {
                FXString SelectedName = Rect.m_ObjectName ;
              }
              else
              {
                int bSelectable = false ;
                if ( rectFrame->Attributes()->GetInt( "Selectable" , bSelectable ) )
                  Rect.SetSelectable( bSelectable != 0 ) ;
                Rect = *(RECT*) rectFrame ;  // only coordinates will be copied
              }
              break ;
            }
            //             }
          }
          if ( i >= m_GraphicsData.m_Rects.GetCount() ) // the same object is not found
          {
            FXSIZE color = 0x000000ff ;
            if (rectFrame->Attributes()->GetString( "color" , atribS ))
              ConvToBinary( atribS , color ) ;
            int iThickness = 1 ;
            rectFrame->Attributes()->GetInt( "thickness" , iThickness ) ;
            FXRectangle NewRect( ( DWORD ) color , iThickness ) ;
            CRect * pRC = (CRect*) rectFrame ;
            NewRect = CRect( rectFrame ) ;
            int bSelectable = false ;
            if ( rectFrame->Attributes()->GetInt( "Selectable" , bSelectable ) )
              NewRect.SetSelectable( bSelectable != 0 ) ;
            NewRect.SetObjectName( rectFrame->GetLabel() ) ;
            if ( NewRect.m_ObjectName.IsEmpty() )
              bEmptyFilled = true ;
            if ( NewRect.m_ObjectName == m_GraphicsData.m_SelectedForAdjustmentName )
              NewRect.m_bSelectedForAdjustment = true ;
            int iStyle = 0 ;
            if ( rectFrame->Attributes()->GetInt( "style" , iStyle ) )
              NewRect.m_Style = iStyle ;
            if ( rectFrame->Attributes()->GetString( "back" , atribS ) )
              NewRect.m_Back = ( int ) ConvToBinary( atribS ) ;
            
            m_GraphicsData.m_Rects.Add( NewRect ) ;
          }
        }
        rectFrame = (CRectFrame*) rectIter->Next( DEFAULT_LABEL );
      }
      delete rectIter;
    }
    for ( int i = 0 ; i < m_GraphicsData.m_Rects.GetCount() ; i++ )
    {
      FXRectangle& Rect = m_GraphicsData.m_Rects.GetAt( i ) ;
      if ( !Rect.GetMatched() )
      {
        m_GraphicsData.m_Rects.RemoveAt( i ) ;
        i-- ;
      }
    }
    CFramesIterator* figIter = df->CreateFramesIterator( figure );
    const CFigureFrame* pFigFrame = NULL ;
    if ( (figIter != NULL) || ((pFigFrame = df->GetFigureFrame()) != NULL) )
    {
      FXString atribS;
      CPen* cstPen = NULL;
      if ( figIter )
        pFigFrame = (CFigureFrame*) figIter->Next();
      while ( pFigFrame )
      {
        if ( !pFigFrame->Attributes()->IsEmpty() )
        {
          FXSIZE fxscolor = 0x000000ff ;
          if (pFigFrame->Attributes()->GetString( "color" , atribS ))
            ConvToBinary( atribS , fxscolor ) ;
          unsigned color = (DWORD) fxscolor ;
          int iThickness = 1 ;
          int iSize = 1 ;
          pFigFrame->Attributes()->GetInt( "thickness" , iThickness ) ;
          pFigFrame->Attributes()->GetInt( "Sz" , iSize ) ;
          int iStyle = PS_SOLID ;
          pFigFrame->Attributes()->GetInt( "style" , iStyle ) ;

          int iNPoints = (int) pFigFrame->GetCount() ;
          if ( iNPoints )
          {
            if ( iNPoints > 1 )
            {
              FXGFigure NewFigure( color , iThickness ) ;
              int bSelectable = false ;
              if ( pFigFrame->Attributes()->GetInt( "Selectable" , bSelectable ) )
                NewFigure.SetSelectable( bSelectable != 0 ) ;
              NewFigure.Copy( *((CDPointArray*) pFigFrame) ) ;
              NewFigure.SetObjectName( pFigFrame->GetLabel() ) ;
              NewFigure.m_Style = iStyle ;
              m_GraphicsData.m_Figures.Add( NewFigure ) ;
            }
            else   // separate point case
            {
              CGPoint NewPoint( pFigFrame->GetAt( 0 ) , color , iThickness ) ;
              NewPoint.m_iSizeMult = iSize ;
              int bSelectable = false ;
              if ( pFigFrame->Attributes()->GetInt( "Selectable" , bSelectable ) )
                NewPoint.SetSelectable( bSelectable != 0 ) ;
              NewPoint.SetObjectName( pFigFrame->GetLabel() ) ;
              NewPoint.m_Style = iStyle ;
              m_GraphicsData.m_Points.Add( NewPoint ) ;
            }
          }

        }
        if ( figIter )
          pFigFrame = (CFigureFrame*) figIter->Next();
        else
          break ;
      }
      if ( figIter )
        delete figIter;
    }
    CFramesIterator* TextIter = df->CreateFramesIterator( text );
    const CTextFrame* TextFrame = NULL ;
    if ( (TextIter != NULL) || ((TextFrame = df->GetTextFrame()) != NULL) )
    {
      if ( TextIter )
        TextFrame = (CTextFrame*) TextIter->Next();
      while ( TextFrame )
      {
        const FXPropertyKit * pProp = TextFrame->Attributes();
        if ( !pProp->IsEmpty() )
        {
          FXString Text = TextFrame->GetString();
          int color = 0x00ff00 , x , y , iSz , back = 1 ; // 1 means, that there is no background
          int iSize = 12;
          if ( pProp->GetInt( "x" , x ) && pProp->GetInt( "y" , y ) )
          {
            FXString t;
            FXSIZE fxscolor = 0x000000ff ;
            if (TextFrame->Attributes()->GetString( "color" , t ))
              ConvToBinary( t , fxscolor ) ;
            unsigned color = ( DWORD ) fxscolor ;
            if ( pProp->GetInt( "Sz" , iSz ) )
              iSize = iSz ;
            if ( pProp->GetString( "back" , t ) )
              back = (int)ConvToBinary( t ) ;
            if ( !Text.IsEmpty() || pProp->GetString( "message" , Text ) )
            {
              FXGText NewText( Text , CPoint( x , y ) , iSize , color , back ) ;
              NewText.SetObjectName( TextFrame->GetLabel() ) ;
              m_GraphicsData.m_Texts.Add( NewText ) ;
            }
          }
        }
        if ( !TextIter )
          break ;
        TextFrame = (CTextFrame*) TextIter->Next();
      }
      if ( TextIter )
        delete TextIter;
    }
    CFramesIterator* qIter = df->CreateFramesIterator( quantity );
    const CQuantityFrame* qFrame = NULL ;
    if ( (qIter != NULL) || ((qFrame = df->GetQuantityFrame()) != NULL) )
    {
      if ( qIter )
        qFrame = (CQuantityFrame*) qIter->Next() ;
      while ( qFrame )
      {
        const FXPropertyKit * pProp = qFrame->Attributes();
        if ( !pProp->IsEmpty() )
        {
          int x , y , iSz;
          int iSize = 12;
          if ( pProp->GetInt( "x" , x ) && pProp->GetInt( "y" , y ) )
          {
            FXString Text( qFrame->ToString() ) ;

            FXString t;
            FXSIZE fxscolor = 0x000000ff ;
            if ( pProp->GetString( "color" , t ) )
              ConvToBinary( t , fxscolor ) ;
            unsigned color = ( DWORD ) fxscolor ;

            if ( pProp->GetInt( "Sz" , iSz ) )
              iSize = iSz ;
            FXGText NewText( qFrame->ToString() ,
              CPoint( x , y ) , iSize , color ) ;
            NewText.SetObjectName( qFrame->GetLabel() ) ;
            m_GraphicsData.m_Texts.Add( NewText ) ;
          }
        }
        if ( !qIter )
          break ;
        qFrame = (CQuantityFrame*) qIter->Next() ;
      }  ;
      if ( qIter )
        delete qIter;
    }
    m_GraphicsData.SetFilled( true ) ;

    static double dLoadGraphicsTime = GetHRTickCount() - dSTartTime ;
    return true ;
  }
  return false ;
}

bool CDIBFRender::DrawRectangles( CDC*  dc )
{
  if ( m_GraphicsData.m_Rects.GetCount() )
  {
    CPen * oP = NULL; // Old pen
    CBrush * oB = NULL ; // Old brush
    CPoint pnt;
    CFont* pOldFont = NULL;
    COLORREF OldColor = 0 ;
    COLORREF OldBack = 0 ;
    int OldBackMode = 0 ;
    for ( int i = 0 ; i < m_GraphicsData.m_Rects.GetCount() ; i++ )
    {
      FXRectangle& Rect = m_GraphicsData.m_Rects.ElementAt( i ) ;
      CPen Pen( Rect.m_Style , Rect.m_dwLineWidth *
       ( Rect.m_bSelectedForAdjustment ) ? 3 : ( ( Rect.m_bSelected ) ? 2 : 1 ) ,
        Rect.m_Color );
      if ( oP == NULL )
        oP = dc->SelectObject( &Pen );
      else
        dc->SelectObject( &Pen );
      CBrush Brush( Rect.m_Back ) ;
      if ( oB == NULL )
      {
        if ( Rect.m_Back == 1 )
          oB = ( CBrush* ) dc->SelectStockObject( NULL_BRUSH ) ;
        else
          oB = dc->SelectObject( &Brush ) ;
      }
      else
      {
        if ( Rect.m_Back == 1 )
          dc->SelectStockObject( NULL_BRUSH ) ;
        else
          dc->SelectObject( &Brush ) ;
      }

      CPoint tl( Rect.TopLeft() );
      CPoint br( Rect.BottomRight() );
      CDIBViewBase::Pic2Scr( tl );
      CDIBViewBase::Pic2Scr( br );
      Rectangle( dc->m_hDC , tl.x , tl.y , br.x , br.y ) ;
      int iROIPos = (int)Rect.m_ObjectName.Find( "ROI:" ) ;
      if ( iROIPos >= 0 ) 
      {
        FXString Text = Rect.m_ObjectName.Mid(iROIPos + 4) ;
        if (!pOldFont)
        {
          pOldFont = dc->SelectObject( GetFont( 8 ) );
          OldColor = dc->GetTextColor() ;
          OldBack = dc->GetBkColor() ;
          OldBackMode = dc->GetBkMode() ;
        }
        dc->SetTextColor( Rect.m_Color );
        dc->SetBkMode( TRANSPARENT ) ;

        CPoint toff( Rect.left + 1 , Rect.bottom + 1 );
        CDIBViewBase::Pic2Scr( toff );
        CRect rc( toff , CSize( CDIBViewBase::Pic2Scr( 50 ) , CDIBViewBase::Pic2Scr( 30 ) ) );
        dc->DrawText( Text , ( int ) Text.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
      }
    }
    if ( pOldFont )
    {
      dc->SetTextColor( OldColor );
      dc->SetBkColor( OldBack ) ;
      dc->SetBkMode( OldBackMode ) ;
      dc->SelectObject( pOldFont );
    }
    if ( oP )
      dc->SelectObject( oP );
    if ( oB )
      dc->SelectObject( oB ) ;

    return true;
  }
  return false ;
}

bool CDIBFRender::DrawFigures( CDC*  dc )
{
  CPen * pOldPen = NULL ;
  HPEN hOldPen = NULL , hSelectedPen = NULL ;
  CPoint pnt;
  bool bSomethingDrawn = false ;
  int iMapMode = dc->GetMapMode() ;
  dc->SetMapMode( MM_TEXT ) ;
  if ( m_GraphicsData.m_Figures.GetCount() )
  {
    bSomethingDrawn = true ;
    for ( int i = 0 ; i < m_GraphicsData.m_Figures.GetCount() ; i++ )
    {
      FXGFigure& Fig = m_GraphicsData.m_Figures.ElementAt( i ) ;

//       LOGPEN PenData =
//       {
//         (UINT)Fig.m_Style | PS_GEOMETRIC ,
//         CPoint( Fig.m_dwLineWidth , 0 ) ,
//         Fig.m_Color
//       } ;
// 
//       CPen Pen ;
//       EXTLOGPEN ExtPenData ;
//       Pen.GetExtLogPen( &ExtPenData ) ;
//       ExtPenData.elpPenStyle = Fig.m_Style | PS_GEOMETRIC | PS_ENDCAP_ROUND ;
//       ExtPenData.elpWidth = Fig.m_dwLineWidth ;
//       //ExtPenData.elpBrushStyle = BS_HATCHED ;
//       ExtPenData.elpColor = Fig.m_Color ;
// // 
//       Pen.CreatePenIndirect( &PenData );
      LOGBRUSH logBrush;
      logBrush.lbStyle = BS_SOLID;
      logBrush.lbColor = Fig.m_Color ;

      CPen Pen( Fig.m_Style | PS_GEOMETRIC | PS_ENDCAP_ROUND ,
        Fig.m_dwLineWidth , &logBrush ) ;
      if ( pOldPen == NULL )
        pOldPen = dc->SelectObject( &Pen );
      else
        dc->SelectObject( &Pen );

      switch ( Fig.GetSize() )
      {
      case 1:
        {
          Pic2Scr( Fig.GetAt( 0 ) , pnt );
          int iShift = 2 * Fig.m_iSizeMult ;
          dc->MoveTo( pnt.x - iShift , pnt.y - iShift );
          dc->LineTo( pnt.x + iShift , pnt.y + iShift );
          dc->MoveTo( pnt.x + iShift , pnt.y - iShift );
          dc->LineTo( pnt.x - iShift , pnt.y + iShift );
          break;
        }
      case 2:
        {
          Pic2Scr( Fig.GetAt( 0 ) , pnt );
          dc->MoveTo( pnt.x , pnt.y );
          Pic2Scr( Fig.GetAt( 1 ) , pnt );
          dc->LineTo( pnt.x , pnt.y );
          break;
        }
      default:
        {
          CPointArray ScreenFig ;
          ScreenFig.SetSize( Fig.GetSize() ) ;
          for ( int i = 0 ; i < Fig.GetSize() ; i++ )
          {
            Pic2Scr( Fig.GetAt( i ) , pnt );
            ScreenFig.SetAt( i , pnt );
          }
          dc->Polyline( ScreenFig.GetData() , (int) ScreenFig.GetCount() ) ;
        }
        break ;
      }
    }
  }
  // Draw selected line for output (to file or image frame)
  // Selected line is overlay, which is not visible on image
  if ( m_GraphicsData.m_Points.GetCount() )
  {
    bSomethingDrawn = true ;
    for ( int i = 0 ; i < m_GraphicsData.m_Points.GetCount() ; i++ )
    {
      CGPoint& Pt = m_GraphicsData.m_Points.ElementAt( i ) ;

//       LOGBRUSH logBrush;
//       logBrush.lbStyle = Pt.m_Style | PS_GEOMETRIC | PS_ENDCAP_ROUND ;
//       logBrush.lbColor = Pt.m_Color;
//       logBrush.lbHatch = 0 ;
//       HPEN NewPen = ::ExtCreatePen( Pt.m_Style , Pt.m_dwLineWidth ,
//         &logBrush , 0 , NULL ) ;
//       if ( hOldPen == NULL )
//         hOldPen = ( HPEN ) SelectObject( dc->m_hDC , NewPen );
//       else
//         SelectObject( dc->m_hDC , NewPen );

      CPen NewPen( Pt.m_Style ,
        Pt.m_dwLineWidth * ((Pt.IsSelected()) ? 2 : 1) , Pt.m_Color );
      if ( pOldPen == NULL )
        pOldPen = dc->SelectObject( &NewPen );
      else
        dc->SelectObject( &NewPen );

      Pic2Scr( (CDPoint) Pt , pnt );
      int iShift = 2 * Pt.m_iSizeMult ;
      dc->MoveTo( pnt.x - iShift , pnt.y - iShift );
      dc->LineTo( pnt.x + iShift , pnt.y + iShift );
      dc->MoveTo( pnt.x + iShift , pnt.y - iShift );
      dc->LineTo( pnt.x - iShift , pnt.y + iShift );
    }
  }

  if ( (m_iSaveImage > 0) || (m_iSendOutImage > 0) 
    && m_dLastLineLength )
  {
    CPen NewPen( PS_SOLID , 1 , RGB(255,0,0) );
    if ( pOldPen == NULL )
      pOldPen = dc->SelectObject( &NewPen );
    else
      dc->SelectObject( &NewPen );
    CPoint pnt ;
    Pic2Scr( m_rcLastSelectedLine.TopLeft() , pnt );
    dc->MoveTo( pnt.x , pnt.y );
    Pic2Scr( m_rcLastSelectedLine.BottomRight() , pnt );
    dc->LineTo( pnt.x , pnt.y );

  }
  if ( pOldPen )
    dc->SelectObject( pOldPen ) ;
  if ( hOldPen )
    SelectObject( dc->m_hDC , hOldPen ) ;

  dc->SetMapMode( iMapMode ) ;
  return bSomethingDrawn ;
}

bool CDIBFRender::DrawTexts( CDC*  dc )
{
  if ( m_GraphicsData.m_Texts.GetCount() || m_bShowLabel )
  {
    CPoint pnt;
    CFont* pOldFont = NULL;
    COLORREF OldColor = dc->GetTextColor() ;
    COLORREF OldBack = dc->GetBkColor() ;
    int OldBackMode = dc->GetBkMode() ;
    for ( int i = 0 ; i < m_GraphicsData.m_Texts.GetCount() ; i++ )
    {
      FXGText& Text = m_GraphicsData.m_Texts.GetAt( i ) ;
      if ( !pOldFont )
        pOldFont = dc->SelectObject( GetFont( Text.m_iSz ) );
      else
        dc->SelectObject( GetFont( Text.m_iSz ) );
      {
        dc->SetTextColor( Text.m_Color );
        if ( Text.m_bUseBackGround )
        {
          dc->SetBkMode( OPAQUE ) ;
          dc->SetBkColor( Text.m_Back ) ;
        }
        else
          dc->SetBkMode( TRANSPARENT ) ;

        CPoint toff( Text.m_Coord );
        CDIBViewBase::Pic2Scr( toff );
        CRect rc( toff , CSize( CDIBViewBase::Pic2Scr( 50 ) , CDIBViewBase::Pic2Scr( 30 ) ) );
        dc->DrawText( Text , (int) Text.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
      }
    }
//     LPCTSTR pLabel = m_OrgFrame->GetLabel() ;
//     if ( m_bShowLabel && pLabel && (*pLabel) )
      LPCTSTR pLabel = m_OrgFrame->GetLabel() ;
    if ( m_bShowLabel && !m_sVideoFrameLabel.IsEmpty() )
    {
      if ( !pOldFont )
        pOldFont = dc->SelectObject( GetFont( 12 ) );
      else
        dc->SelectObject( GetFont( 12 ) );
      {
        dc->SetTextColor( 0x0000ff );
        dc->SetBkMode( OPAQUE ) ;
        dc->SetBkColor( 0 ) ;

        CPoint Offset( 10 , 10 ) ;
        CDIBViewBase::Pic2Scr( Offset ) ;
        CRect rc( Offset , CSize( CDIBViewBase::Pic2Scr( 70 ) , CDIBViewBase::Pic2Scr( 30 ) ) );
        dc->DrawText( m_sVideoFrameLabel ,
          (int)m_sVideoFrameLabel.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
      }

    }

    if ( m_dLastLineLength != 0. && (m_LengthViewMode != LVM_Disabled) )
    {
      if ( !pOldFont )
        pOldFont = dc->SelectObject( GetFont( 12 ) );
      else
        dc->SelectObject( GetFont( 12 ) );
      dc->SetTextColor( 0x0000ff );
      dc->SetBkMode( OPAQUE ) ;
      dc->SetBkColor( 0 ) ;

      CPoint Offset( m_LastLineLengthTextPos ) ;
      CDIBViewBase::Pic2Scr( Offset ) ;
      CRect rc( Offset , CSize( CDIBViewBase::Pic2Scr( 70 ) , CDIBViewBase::Pic2Scr( 30 ) ) );
      if ( rc.left < 0 )
        rc.OffsetRect( -rc.left , 0 ) ;
      if ( rc.right > ( int )GetWidth( m_Frame ) )
        rc.OffsetRect( -((int) rc.right - (int) GetWidth( m_Frame )) , 0 ) ;

      FXString LengthAsText ;
      if ( m_LengthViewMode != LVM_ViewTenth )
      {
        LengthAsText.Format( "Len=%.3f%s" , m_dLastLineLength ,
          m_sUnits.IsEmpty() ? "" : ( LPCTSTR ) m_sUnits ) ;
      }
      if ( m_dScaleTenthPerUnit != 0. )
      {
        if ( ( m_LengthViewMode == LVM_ViewTenth ) )
          LengthAsText.Format( "Len=%.2fT" , m_dLastLineLength * m_dScaleTenthPerUnit ) ;
        else if ( m_LengthViewMode == LVM_ViewBoth )
        {
          FXString Addition ;
          Addition.Format( " / %.2fT" , m_dLastLineLength * m_dScaleTenthPerUnit ) ;
          LengthAsText += Addition ;
        }
      }
      dc->DrawText( ( LPCTSTR ) LengthAsText , ( int ) LengthAsText.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
    }
    int iIndex = m_GraphicsData.m_iSelectedForAdjustmentsIndex ;
    if ( iIndex >= 0 )
    {
      FXString AboutSelectedRect ;
      Rectangles& Rects = m_GraphicsData.m_Rects ;
      AboutSelectedRect.Format( "Selected %s(%d) %s " ,
        ( LPCTSTR ) ( Rects[ iIndex ].m_ObjectName ) , iIndex ,
        GetSelectedSideName( Rects[ iIndex ].m_iSelectedSide ) ) ;
      CPoint Offset( 10 , 70 ) ;
      CDIBViewBase::Pic2Scr( Offset ) ;
      CRect rc( Offset , CSize( CDIBViewBase::Pic2Scr( 100 ) , CDIBViewBase::Pic2Scr( 30 ) ) );
      dc->DrawText( ( LPCTSTR ) AboutSelectedRect , 
        ( int ) AboutSelectedRect.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
    }

    dc->SetTextColor( OldColor );
    dc->SetBkColor( OldBack ) ;
    dc->SetBkMode( OldBackMode ) ;
    if ( pOldFont )
      dc->SelectObject( pOldFont );
  }
  return true;
}

void CDIBFRender::OnSetFocus( CWnd* pOldWnd )
{
  CDIBView::OnSetFocus( pOldWnd );

  SetCursor( LoadCursor( NULL , IDC_CROSS ) ) ;
}

int CDIBFRender::CheckCursorForm( bool bMouseMove , CPoint& CursorPos )
{
  if ( CursorPos.x <= 0 || CursorPos.y <= 0 )
    return 0 ;
  CPoint GlobalCurs ;
  GetCursorPos( &GlobalCurs ) ;
  ScreenToClient( &GlobalCurs ) ;
  CRect cr ;
  GetClientRect( &cr ) ;
  if ( !cr.PtInRect( GlobalCurs ) )
    return 0 ;
  m_LastCursorPt = CursorPos ;
  m_LastCursor = cmplx( CursorPos.x , CursorPos.y ) ;
  m_LastCalculatedCursor = NULL ;
  m_iLastNearest = 100000 ;
  LPCSTR CursorForm = 0 ;
  for ( int i = 0 ; i < m_GraphicsData.m_Points.GetCount() ; i++ )
  {
    CGPoint& pt = m_GraphicsData.m_Points.GetAt( i ) ;
    if ( pt.IsSelectable() )  // could be commented for debugging
    {
      cmplx cmplxpt( pt.x , pt.y ) ;
      int iDist = ROUND( abs( cmplxpt - m_LastCursor ) ) ;
      if ( iDist <= CURSOR_TOLERANCE && iDist < m_iLastNearest )
      {
        if ( !pt.IsSelected() )
        {
          pt.SetSelected( true ) ;
          m_LastCalculatedCursor = IDC_SIZEALL ;
        }
        pt.m_iDistToCursor = iDist ;
        m_iLastNearest = iDist ;
      }
      else if ( pt.IsSelected() )
      {
        pt.SetSelected( false ) ;
        CursorForm = m_LastCalculatedCursor = IDC_CROSS ;
      }
    }
  }
  for ( int i = 0 ; i < m_GraphicsData.m_Rects.GetCount() ; i++ )
  {
    FXRectangle& Rect = m_GraphicsData.m_Rects.GetAt( i ) ;
    if ( Rect.IsSelectable() &&   // could be commented for debugging
      ( (m_GraphicsData.m_iSelectedForAdjustmentsIndex < 0)
      || ( Rect.m_bSelectedForAdjustment ) ) )
    {
      int iDist = 100000 ;
      int iCursSelect = Rect.CheckCursor( m_LastCursorPt ,
        CURSOR_TOLERANCE /*/ m_ScrScale*/ , &iDist ) ;
      if ( (iCursSelect != CURSOR_NO_SELECTION) && (iDist < m_iLastNearest) )
      {
        m_iLastNearest = iDist ;
        Rect.m_iDistToCursor = iDist ;
        switch ( iCursSelect )
        {
          case CURSOR_ON_LEFT_BORDER:
          case CURSOR_ON_RIGHT_BORDER:  CursorForm = IDC_SIZEWE ; break;
          case CURSOR_ON_TOP_BORDER:
          case CURSOR_ON_BOTTOM_BORDER: CursorForm = IDC_SIZENS ; break;
          case CURSOR_ON_LTCORNER:
          case CURSOR_ON_RBCORNER:      CursorForm = IDC_SIZENWSE ; break;
          case CURSOR_ON_TRCORNER:
          case CURSOR_ON_BLCORNER:      CursorForm = IDC_SIZENESW ; break;
          case CURSOR_ON_WHOLE_SELECTION:CursorForm = IDC_SIZEALL ; break;
        }
        Rect.m_CursorForm = CursorForm ;
        Rect.m_iSelectedSide = iCursSelect ;
      }
    }
  }
  if ( m_iLastNearest <= 2 * CURSOR_TOLERANCE )
  {
    for ( int i = 0 ; i < m_GraphicsData.m_Points.GetCount() ; i++ )
    {
      CGPoint& pt = m_GraphicsData.m_Points.GetAt( i ) ;
      if ( (m_LastCalculatedCursor == NULL) && (pt.m_iDistToCursor < CURSOR_TOLERANCE)
        && (pt.m_iDistToCursor == m_iLastNearest) )
      {
        pt.SetSelected( true ) ;
        CursorForm = m_LastCalculatedCursor = IDC_SIZEALL ;
        m_GraphicsData.m_iSelectedType = SELECTED_POINT ;
        m_GraphicsData.m_iSelectedIndex = i ;
        break ;
      }
      else
      {
        pt.SetSelected( false ) ;
        pt.m_iDistToCursor = 100001 ;
      }
    }
    for ( int i = 0 ; i < m_GraphicsData.m_Rects.GetCount() ; i++ )
    {
      FXRectangle& Rect = m_GraphicsData.m_Rects.GetAt( i ) ;
      if ( m_LastCalculatedCursor == NULL
        && Rect.m_iDistToCursor == m_iLastNearest 
        /*&& i == m_GraphicsData.m_iSelectedForAdjustmentsIndex*/ )
      {
        Rect.SetSelected( true ) ;
        m_LastCalculatedCursor = Rect.m_CursorForm ;
        m_GraphicsData.m_iSelectedType = SELECTED_RECT ;
        m_GraphicsData.m_iSelectedSide = Rect.m_iSelectedSide ;
        m_GraphicsData.m_iSelectedIndex = i ;
        m_GraphicsData.m_SelectedName = Rect.m_ObjectName ;
        m_GraphicsData.m_LastSelectedRectName = Rect.m_ObjectName ;
      }
      else
      {
        Rect.SetSelected( false ) ;
        Rect.m_iDistToCursor = 100001 ;
        Rect.m_iSelectedSide = CURSOR_NO_SELECTION ;
        m_GraphicsData.m_SelectedName.Empty() ;
      }
    }
  }
  if ( !m_LastCalculatedCursor )
  {
    HCURSOR hCrossCursor = LoadCursor( NULL , IDC_CROSS ) ;
    if ( hCrossCursor != GetCursor() )
      m_LastCalculatedCursor = IDC_CROSS ;
    m_GraphicsData.m_iSelectedIndex = -1 ;
  }
  if ( bMouseMove )
  {
    if ( m_LastCalculatedCursor )
    {
      Invalidate( FALSE ) ;
    }
  }
  else
  {
    SetCursor( LoadCursor( NULL , m_LastCalculatedCursor ) ) ;
    m_CurrentCursor = m_LastCalculatedCursor ;
  }
  return 0 ;
}

void CDIBFRender::OnMouseMove( UINT nFlags , CPoint point )
{
  m_LastMousePos = point ;
  CPoint rPoint = point;
  bool bInFrame = InFrame( point ) ;
  if ( bInFrame )
    Scr2Pic( rPoint );

  bool bKbCntrlPressed = (GetAsyncKeyState( VK_LCONTROL ) & 0x8000) != 0 ;
  if ( !bKbCntrlPressed )
  {
//     if ( !m_bTrackingTT && bInFrame )
//     {
//       SetFocus() ;
//       TRACKMOUSEEVENT tme = { sizeof( TRACKMOUSEEVENT ) };
//       tme.hwndTrack =m_hWnd;
//       tme.dwFlags = TME_LEAVE;
//       TrackMouseEvent( &tme );
// 
//       m_bTrackingTT = true ;
//       if ( (GetHRTickCount() - m_dLastUpdateTime) > 100. )
//         Invalidate( FALSE ) ;
//       else if ( m_iFrameInterval > MOUSE_MOVE_TIMEOUT && !m_iMouseMoveTimer )
//         m_iMouseMoveTimer = SetTimer( TMR_MOUSE_MOVE , MOUSE_MOVE_TIMEOUT , NULL ) ;
//    }

    bool bIPressed = (GetAsyncKeyState( 'I' ) & 0x8000) != 0 ;
    bool bRPressed = (GetAsyncKeyState( 'R' ) & 0x8000) != 0 ;
    if ( !bIPressed && !bRPressed && !m_bTrackingSwitch )
      CDIBViewBase::OnMouseMove( nFlags , point ) ;
    else
      CWnd::OnMouseMove( nFlags , point ) ;
    return ;
  }
  else
  {
    CDIBViewBase::OnMouseMove( nFlags , point ) ;

//     FXAutolock al( m_Lock , "DIBFrender::OnMouseMove" ) ;
//     if ( m_bSomeSelected && ( m_PointOfInterest.x >= 0 )
//       && m_GraphicsData.m_iSelectedIndex >= 0 &&
//       ( m_GraphicsData.m_iSelectedIndex < m_GraphicsData.m_Rects.GetCount() ) )
//     {
//       FXRectangle& SelectedRect =
//         m_GraphicsData.m_Rects.GetAt( m_GraphicsData.m_iSelectedIndex ) ;
//       FXString PureObjectName( SelectedRect.m_ObjectName ) ;
//       int iColonPos = ( int ) PureObjectName.Find( _T( ':' ) ) ;
//       if ( iColonPos >= 0 )
//         PureObjectName.Delete( 0 , iColonPos + 1 ) ;
// 
//       bool bCorrected = m_GraphicsData.CorrectRect(
//         SelectedRect , m_PointOfInterest , point ,
//         ( SelectionByCursor ) SelectedRect.m_iSelectedSide ) ;
//       CRect ForWrite( SelectedRect ) ;
//       // Replace right to width and bottom to height
//       ForWrite.right = SelectedRect.Width() ;
//       ForWrite.bottom = SelectedRect.Height() ;
//       FXPropertyKit SetROICommand ;
//       WriteRect( SetROICommand , PureObjectName , ForWrite ) ;
//       SetROICommand.Insert( 0 , _T( "setroi " ) ) ;
//       TRACE( " %s" , ( LPCTSTR ) SetROICommand ) ;
//       CTextFrame * pTF = CTextFrame::Create( SetROICommand ) ;
//       pTF->SetLabel( SelectedRect.m_ObjectName ) ;
//       pTF->SetTime( GetHRTickCount() );
//       pTF->ChangeId( NOSYNC_FRAME );
//       DIB_EVENT( DIBVE_OUT_FRAME , ( void* ) pTF );
//     }
  }

  if ( !bInFrame ) // Out of FOV
  {
    if ( m_Selection )
    {                   // Stop following and drawing
      if ( m_Selection->RPressed )
      {
        m_Selection->RPressed = false;
        if ( m_Selection->DrawRect )
          m_Selection->DrawRect = false;
        Invalidate();
      }
      if ( m_Selection->LPressed )
      {
        m_Selection->LPressed = false;
        if ( m_Selection->DrawLine )
          m_Selection->DrawLine = false;
        Invalidate();
      }
    }
  }
  else
  {
    if ( (GetHRTickCount() - m_dLastUpdateTime) > 100. )
      Invalidate( FALSE ) ;
    else if ( m_iFrameInterval > MOUSE_MOVE_TIMEOUT && !m_iMouseMoveTimer )
      m_iMouseMoveTimer = SetTimer( TMR_MOUSE_MOVE , MOUSE_MOVE_TIMEOUT , NULL ) ;
    GetParent()->SendMessage(WM_MOUSEMOVE,nFlags,MAKELPARAM(point.x,point.y));
  }
  CWnd::OnMouseMove( nFlags , point );
}

void CDIBFRender::OnLButtonUp( UINT nFlags , CPoint point )
{
  bool bKbCntrlPressed = (GetAsyncKeyState( VK_LCONTROL ) & 0x8000) != 0 ;
  bool bInFrame = InFrame( point ) ;
  if ( m_bTrackingSwitch )
  {
    m_bTrackingSwitch = false ;
    CWnd::OnLButtonUp( nFlags , point ) ;
    return ;
  }
  CPoint rPoint = point;
  if ( !bKbCntrlPressed )
  {
    CDIBViewBase::OnLButtonUp( nFlags , point ) ;
    return ;
  }
  CPoint pt = point;
  Scr2Pic( pt );
  DIB_EVENT( DIBVE_LBUTTONUP , (void*) (&pt) );
}
void CDIBFRender::OnLButtonDown( UINT nFlags , CPoint point )
{
  bool bIPressed = (GetAsyncKeyState( 'I' ) & 0x8000) != 0 ;
  bool bFPressed = (GetAsyncKeyState( 'F' ) & 0x8000) != 0 ;
  bool bHPressed = ( GetAsyncKeyState( 'H' ) & 0x8000 ) != 0 ;
  if ( bHPressed )
  {
    CRect FRenderRC ;
    GetWindowRect( &FRenderRC ) ;
    m_HelpWnd.SetWindowPos( &wndTopMost ,
      FRenderRC.left + 30 , FRenderRC.top + 40 , 0 , 0 , SWP_SHOWWINDOW | SWP_NOSIZE ) ;
    m_HelpWnd.ShowWindow( SW_RESTORE ) ;
    m_HelpWnd.SetOutputText( FrenderHelpText ) ;

    return ;
  }
  if ( bIPressed || bFPressed )
  {
    if ( bIPressed )
    {
      if ( !m_bTrackingTT )
      {
        TRACKMOUSEEVENT tme = { sizeof( TRACKMOUSEEVENT ) };
        tme.hwndTrack = m_hWnd;
        tme.dwFlags = TME_LEAVE;
        TrackMouseEvent( &tme );

        m_bTrackingTT = m_bTrackingSwitch = true ; ;
        if ( (GetHRTickCount() - m_dLastUpdateTime) > 100. )
          Invalidate( FALSE ) ;
        else if ( m_iFrameInterval > MOUSE_MOVE_TIMEOUT && !m_iMouseMoveTimer )
          m_iMouseMoveTimer = SetTimer( TMR_MOUSE_MOVE , MOUSE_MOVE_TIMEOUT , NULL ) ;
      }
      else
      {
        m_bTrackingTT = false ;
        if ( m_iMouseMoveTimer )
        {
          KillTimer( m_iMouseMoveTimer ) ;
          m_iMouseMoveTimer = 0 ;
          Invalidate( FALSE ) ;
        }
      }
      CWnd::OnLButtonUp( nFlags , point ) ;
      TRACE( "\nLButtonUp: m_bTrackingSwitch=%d m_bTrackingTT=%d" , m_bTrackingSwitch , m_bTrackingTT ) ;

      return ;
    }
    if ( bFPressed )
    {
      m_bShowRGB = !m_bShowRGB ;
      return ;
    }
    CWnd::OnLButtonDown( nFlags , point ) ;
    return ;
  }
  bool bKbCntrlPressed = (GetAsyncKeyState( VK_LCONTROL ) & 0x8000) != 0 ;
  if ( !bKbCntrlPressed )
  {
    if ( nFlags & MK_RBUTTON )
    {
      CPoint pt = point;
      Scr2Pic( pt );
      DIB_EVENT( DIBVE_LBUTTONDBL , (void*) (&pt) );
    }
    else
      CDIBViewBase::OnLButtonDown( nFlags , point ) ;
    return ;
  }
  CPoint pt = point;
  Scr2Pic( pt );
  DIB_EVENT( DIBVE_LBUTTONDOWN , (void*) (&pt) );
}

void CDIBFRender::OnHScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar )
{
  int     iMax;
  int     iMin;
  int     iPos;
  int     dn;
  RECT    rc;

  GetScrollRange( SB_HORZ , &iMin , &iMax );
  iPos = GetScrollPos( SB_HORZ );
  GetClientRect( &rc );
  switch ( nSBCode )
  {
  case SB_LINEDOWN:
    dn = 1;
    break;
  case SB_LINEUP:
    dn = -1;
    break;
  case SB_PAGEDOWN:
    dn = rc.right / 4 + 1;
    break;
  case SB_PAGEUP:
    dn = -rc.right / 4 + 1;
    break;
  case SB_THUMBTRACK:
  case SB_THUMBPOSITION:
    dn = nPos - iPos;
    break;
  default:
    dn = 0;
    break;
  }
  UpdateWindow();
  if ( dn = BOUNDE( iPos + dn , iMin , iMax ) - iPos )
  {
    CDIBViewBase::ScrollWindow( -dn , 0 , NULL , NULL );
    SetScrollPos( SB_HORZ , iPos + dn , TRUE );
    UpdateWindow();
    Invalidate( FALSE );
  }
  CDIBViewBase::OnHScroll( nSBCode , nPos , pScrollBar );
}

void CDIBFRender::OnVScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar )
{
  int     iMax;
  int     iMin;
  int     iPos;
  int     dn;
  RECT    rc;

  GetScrollRange( SB_VERT , &iMin , &iMax );
  iPos = GetScrollPos( SB_VERT );
  GetClientRect( &rc );
  switch ( nSBCode )
  {
  case SB_LINEDOWN:
    dn = 1;
    break;
  case SB_LINEUP:
    dn = -1;
    break;
  case SB_PAGEDOWN:
    dn = rc.bottom / 4 + 1;
    break;
  case SB_PAGEUP:
    dn = -rc.bottom / 4 + 1;
    break;
  case SB_THUMBTRACK:
  case SB_THUMBPOSITION:
    dn = nPos - iPos;
    break;
  default:
    dn = 0;
    break;
  }
  UpdateWindow();
  if ( dn = BOUNDE( iPos + dn , iMin , iMax ) - iPos )
  {
    CDIBViewBase::ScrollWindow( 0 , -dn , NULL , NULL );
    SetScrollPos( SB_VERT , iPos + dn , TRUE );
    UpdateWindow();
    Invalidate( FALSE );
  }
  CDIBViewBase::OnVScroll( nSBCode , nPos , pScrollBar );
}

void CDIBFRender::ShiftPos( int dx , int dy )
{
  m_MoveX = dx; m_MoveY = dy;
}

BOOL CDIBFRender::OnMouseWheel( UINT nFlags , short zDelta , CPoint pt )
{
  if ( !(nFlags & MK_MBUTTON) )
  {
    int iDelta = (4 * zDelta) / WHEEL_DELTA ;
    if ( iDelta )
    {
      bool bCNTRL = (GetAsyncKeyState( VK_LCONTROL ) & 0x8000) != 0 ;
      bool bShift = (GetAsyncKeyState( VK_SHIFT ) & 0x8000) != 0 ;
      if ( m_LockOMutex.Lock( 1000
#ifdef _DEBUG
        , "CDIBFRender::OnMouseWheel"
#endif
        ) )
      {
        if ( !bCNTRL )
        {
          if ( bShift )
          {
            if ( m_ScrBarEnabled&FL_HORZ )
              OnHScroll( SB_THUMBTRACK , GetScrollPos( SB_HORZ ) - 4 * zDelta / WHEEL_DELTA , NULL );
          }
          else
          {
            if ( m_ScrBarEnabled&FL_VERT )
              OnVScroll( SB_THUMBTRACK , GetScrollPos( SB_VERT ) - 4 * zDelta / WHEEL_DELTA , NULL );
          }
        }
        else
        {
          CRect wr ;
          GetWindowRect( &wr ) ;
          pt -= wr.TopLeft() ;
          if ( zDelta < 0 )
          {
            if ( m_Scale < 16 )
            {
              SetScale( m_Scale == -1 ? 1. : m_Scale * 2. , pt ) ;
            }
          }
          else
          {
            if ( m_Scale == 1 )
              SetScale( -1. ) ;
            else if ( m_Scale > 1. )
              SetScale( m_Scale * 0.5 , pt ) ;
          }
        }
        m_LockOMutex.Unlock() ;
      }
    }
  }
  return CDIBViewBase::OnMouseWheel( nFlags , zDelta , pt );
}

void CDIBFRender::SetScale( double dNewScale )
{
  int iScale = ROUND( m_Scale ) ;
  if ( iScale < 0 )
    iScale = 0 ;
  if ( iScale > 16 )
    iScale = 16 ;
  m_ScrollOffsets[ iScale ] = m_ScrOffset ;


  if ( dNewScale < 0 )
    m_Scale = -1.;
  else if ( dNewScale <= 1. )
    m_Scale = 1.;
  else if ( dNewScale <= 2. )
    m_Scale = 2.;
  else if ( dNewScale <= 4. )
    m_Scale = 4.;
  else if ( dNewScale <= 8. )
    m_Scale = 8.;
  else
    m_Scale = 16.;
  iScale = ROUND( m_Scale ) ;
  if ( iScale < 0 )
    iScale = 0 ;
  if ( iScale > 16 )
    iScale = 16 ;
  m_ScrOffset = m_NewScrollOffset ;
}

void CDIBFRender::SetScale( double dNewScale , CPoint CursorPt )
{
  CSize ImageSize ;
  GetImageSize( ImageSize.cx , ImageSize.cy ) ;
  CRect rc ;
  GetClientRect( &rc ) ;
  cmplx cViewSize( (double) rc.Width() , (double) rc.Height() ) ; // for old view area
  cmplx cImageSize( (double) ImageSize.cx , (double) ImageSize.cy ) ;
  double dXRatio = cViewSize.real() / cImageSize.real() ;
  double dYRatio = cViewSize.imag() / cImageSize.imag() ;
  double dXYRatio = dXRatio / dYRatio ;

  cmplx cOldSpare ;
  if ( m_Scale > 0. )
    cOldSpare = cViewSize - cImageSize * m_Scale ;

  cmplx ScrollRanges = cImageSize - cViewSize / m_Scale ;
  cmplx cOldShift ;
  if ( cOldSpare.real() < 0. )
  {
    int max , min;
    GetScrollRange( SB_HORZ , &min , &max );
    int iRange = max - min ;
    int iPos = GetScrollPos( SB_HORZ ) ;
    m_ScrOffset.x = ROUND( (double) iPos * ScrollRanges.real() / (double) iRange ) - min ;
  }
  else
  {
    m_ScrOffset.x = 0 ;
    cOldShift._Val[ _RE ] = cOldSpare.real() / 2. ;
  }
  if ( cOldSpare.imag() < 0. )
  {
    int max , min;
    GetScrollRange( SB_VERT , &min , &max );
    int iRange = max - min ;
    int iPos = GetScrollPos( SB_VERT ) ;
    m_ScrOffset.y = ROUND( (double) iPos * ScrollRanges.imag() / (double) iRange ) - min ;
  }
  else
  {
    m_ScrOffset.y = 0 ;
    cOldShift._Val[ _IM ] = cOldSpare.imag() / 2. ;
  }
  cmplx cCursorOnView( (double) CursorPt.x , (double) CursorPt.y ) ;
  cmplx cCurrentOffsetOnImage( (double) m_ScrOffset.x , (double) m_ScrOffset.y ) ;
  cCursorOnView -= cOldShift ;
  cmplx CursorOnImage ;
  cmplx cViewOffset ;
  if ( m_Scale > 0. )
  {
    CursorOnImage = cCurrentOffsetOnImage + (cCursorOnView / m_Scale) ;
  }
  else
  {
    double dZoom ;
    if ( dXYRatio >= 1. ) // width is more than height => spaces on left and right
    {
      cViewOffset._Val[ _RE ] = cViewSize.real() * (dXYRatio - 1.0) / 2. ; // /2 because spaces on left and right
      dZoom = cViewSize.imag() / cImageSize.imag() ;
    }
    else  // width is less than height => spaces on up and down
    {
      cViewOffset._Val[ _IM ] = cViewSize.imag() * ((1.0 / dXYRatio) - 1.0) / 2. ; // /2 because spaces on up and down
      dZoom = cViewSize.real() / cImageSize.real() ;
    }
    CursorOnImage = (cCursorOnView - cViewOffset) / dZoom ;
  }
  double dXNormCursorShift = CursorPt.x / cViewSize.real() ;
  double dYNormCursorShift = CursorPt.y / cViewSize.imag() ;

  TRACE( "\n Old ViewSize[%d,%d] , m_ScrOffset[%d,%d] OnView[%d,%d]" ,
    rc.Width() , rc.Height() , m_ScrOffset.x , m_ScrOffset.y , CursorPt.x , CursorPt.y ) ;
  TRACE( "\n OldSpare[%5.0f,%5.0f] OldShift[%5.0f,%5.0f] CursOnIm[%5.0f,%5.0f]" ,
    cOldSpare.real() , cOldSpare.imag() , cOldShift.real() , cOldShift.imag() ,
    CursorOnImage.real() , CursorOnImage.imag() ) ;


  cmplx cNewSpare = cViewSize - cImageSize * dNewScale ;
  BOOL bScrollX = cNewSpare.real() < 0. ;
  BOOL bScrollY = cNewSpare.imag() < 0. ;
  ShowScrollBar( SB_HORZ , bScrollX );
  ShowScrollBar( SB_VERT , bScrollY );

  CRect prc ;
  GetParent()->GetClientRect( prc );
  GetClientRect( &rc ) ; // we take new view area: scrolls could appear
  cViewSize = cmplx( (double) rc.Width() , (double) rc.Height() ) ;
  cNewSpare = cViewSize - cImageSize * dNewScale ;
  cmplx NewSize = cViewSize / dNewScale ;
  cmplx cNewShift ;
  if ( dNewScale > 0. ) // there is no fitting
  {
    int iRangeX = 0 , iRangeY = 0 ;
    if ( !bScrollX )
    {
      NewSize._Val[ _RE ] = cImageSize.real() * dNewScale ;
      cNewShift._Val[ _RE ] = cNewSpare.real() / 2. ;
    }
    if ( !bScrollY )
    {
      NewSize._Val[ _IM ] = cImageSize.imag() * dNewScale ;
      cNewShift._Val[ _IM ] = cNewSpare.imag() / 2. ;
    }
    cmplx NewOffset( CursorOnImage.real() - NewSize.real() * dXNormCursorShift ,
      CursorOnImage.imag() - NewSize.imag() * dYNormCursorShift );
    cmplx RightBottomViewCorner = NewOffset + NewSize ;
    TRACE( "\n OZ=%d NZ=%d NewSize[%5.0f,%5.0f] NewOffset[%5.0f,%5.0f] RBCorner[%5.0f,%5.0f]" ,
      ROUND( m_Scale ) , ROUND( dNewScale ) ,
      NewSize.real() , NewSize.imag() , NewOffset.real() , NewOffset.imag() ,
      RightBottomViewCorner.real() , RightBottomViewCorner.imag() ) ;
    if ( NewOffset.real() < 0. )
    {
      NewOffset._Val[ _RE ] = 0. ;
      RightBottomViewCorner._Val[ _RE ] = NewSize.real() ;
    }
    else if ( RightBottomViewCorner.real() > cImageSize.real() )
      NewOffset._Val[ _RE ] = cImageSize.real() * dNewScale - cViewSize.real() ;
    if ( NewOffset.imag() < 0. )
    {
      NewOffset._Val[ _IM ] = 0. ;
      RightBottomViewCorner._Val[ _IM ] = NewSize.imag() ;
    }
    else if ( RightBottomViewCorner.imag() > cImageSize.imag() )
      NewOffset._Val[ _IM ] = cImageSize.imag() * dNewScale - cViewSize.imag() ;
    m_NewScrollOffset.x = ROUND( NewOffset.real() ) ;
    m_NewScrollOffset.y = ROUND( NewOffset.imag() ) ;
    TRACE( "\n Final NewOffset[%5.0f,%5.0f] RBCorner[%5.0f,%5.0f]" ,
      NewOffset.real() , NewOffset.imag() , RightBottomViewCorner.real() , RightBottomViewCorner.imag() ) ;
    int iPosX = 0 , iPosY = 0 ;
    int iXMin = 0 , iXMax = 1000 , iYMin = 0 , iYMax = 1000 ;
    if ( bScrollX )
    {
      GetScrollRange( SB_HORZ , &iXMin , &iXMax );
      int iRange = iXMax - iXMin ;
      iPosX = ROUND( iRange * NewOffset.real() / (cImageSize.real() - NewSize.real()) ) + iXMin ;
      SetScrollPos( SB_HORZ , iPosX ) ;
    }
    if ( bScrollY )
    {
      GetScrollRange( SB_VERT , &iYMin , &iYMax );
      int iRange = iYMax - iYMin ;
      iPosY = ROUND( iRange * NewOffset.imag() / (cImageSize.imag() - NewSize.imag()) ) + iYMin ;
      SetScrollPos( SB_VERT , iPosY ) ;
    }
    TRACE( "\n NS=%3.0f ImSize[%5.0f,%5.0f] MAXs[%d,%d] SCrollPos[%d,%d]" ,
      dNewScale , cImageSize.real() , cImageSize.imag() , iXMax , iYMax , iPosX , iPosY ) ;
  }
  else // image is fitted into view
  {
    m_NewScrollOffset.x = m_NewScrollOffset.y = 0 ;
    ShowScrollBar( SB_HORZ , FALSE );
    ShowScrollBar( SB_VERT , FALSE );
    TRACE( "\n Scale fit" ) ;

  }
  SetScale( dNewScale ) ;
  Invalidate();
}

void CDIBFRender::SetNewViewCenter( CPoint Center )
{
  if ( m_Scale < 0 )  // scaled, nothing to do
    return ;

  CSize ImageSize ;
  GetImageSize( ImageSize.cx , ImageSize.cy ) ;
  CRect rc ;
  GetClientRect( &rc ) ;
  cmplx cViewSize( (double) rc.Width() , (double) rc.Height() ) ; // for old view area
  cmplx cImageSize( (double) ImageSize.cx , (double) ImageSize.cy ) ;
  double dXRatio = cViewSize.real() / cImageSize.real() ;
  double dYRatio = cViewSize.imag() / cImageSize.imag() ;
  double dXYRatio = dXRatio / dYRatio ;


  cmplx cOldSpare ;
  if ( m_Scale > 0. )
    cOldSpare = cViewSize - cImageSize * m_Scale ;

  cmplx cViewedPart = cViewSize / m_Scale ;
  cmplx ScrollRanges = cImageSize - cViewedPart ;
  cmplx cViewedCenter( Center.x , Center.y ) ;
  cmplx cLeftTopView = cViewedCenter - cViewedPart * 0.5 ;
  cmplx cBottomRightView = cViewedCenter + cViewedPart * 0.5 ;
  if ( cLeftTopView.real() < 0 )
  {
    cBottomRightView += cmplx( -cLeftTopView.real() , 0. ) ;
    cLeftTopView += cmplx( -cLeftTopView.real() , 0. ) ;
  }
  if ( cLeftTopView.imag() < 0 )
  {
    cBottomRightView += cmplx( 0. , -cLeftTopView.imag() ) ;
    cLeftTopView += cmplx( 0. , -cLeftTopView.imag() ) ;
  }
  cmplx cBottomRightDiff = cBottomRightView - cImageSize ;
  if ( cBottomRightDiff.real() > 0. )
  {
    cLeftTopView += cmplx( -cBottomRightDiff.real() , 0. ) ;
    cBottomRightView += cmplx( -cBottomRightDiff.real() , 0. ) ;
  }
  if ( cBottomRightDiff.imag() > 0. )
  {
    cLeftTopView += cmplx( 0. , -cBottomRightDiff.imag() ) ;
    cBottomRightView += cmplx( 0. , -cBottomRightDiff.imag() ) ;
  }

  int max , min;
  if ( ScrollRanges.real() > 1 )
  {
    GetScrollRange( SB_HORZ , &min , &max );
    int iRange = max - min ;
    m_ScrOffset.x = ROUND( cLeftTopView.real() ) ;
    int iXScrollPos = ROUND( iRange * cLeftTopView.real() / ScrollRanges.real() ) ;
    SetScrollPos( SB_HORZ , iXScrollPos , 1 ) ;
  }

  if ( ScrollRanges.imag() > 1 )
  {
    GetScrollRange( SB_VERT , &min , &max );
    int iRange = max - min ;
    m_ScrOffset.y = ROUND( iRange * cLeftTopView.imag() / ScrollRanges.imag() ) ;
    int iYScrollPos = ROUND( iRange * cLeftTopView.imag() / ScrollRanges.imag() ) ;
    SetScrollPos( SB_VERT , iYScrollPos , 1 ) ;
  }

  //   TRACE( "\n SetViewCenter ViewSize[%d,%d] , m_ScrOffset[%d,%d] Center[%d,%d]" ,
  //     rc.Width() , rc.Height() , m_ScrOffset.x , m_ScrOffset.y , Center.x , Center.y ) ;
}

void CDIBFRender::OnKeyDown( UINT nChar , UINT nRepCnt , UINT nFlags )
{
  bool bControlPressed = (GetAsyncKeyState( VK_LCONTROL ) & 0x8000) != 0 ;
  switch ( nChar )
  {
  case _T( '+' ):
  case VK_ADD:
    {
      if ( m_Scale < 16. )
      {
        if ( m_Scale == -1. )
          m_Scale = 1. ;
        else
          m_Scale *= 2. ;
        CRect rc ;
        GetClientRect( rc ) ;
        SetScale( m_Scale , rc.CenterPoint() ) ;
      }
    }
    break ;
  case _T( '-' ):
  case VK_SUBTRACT:
    {
      if ( m_Scale > -1. )
      {
        if ( m_Scale <= 1. )
          m_Scale = -1. ;
        else
          m_Scale /= 2. ;
        CRect rc ;
        GetClientRect( rc ) ;
        SetScale( m_Scale , rc.CenterPoint() ) ;
      }
    }
    break ;
  case _T('i') :
  case _T( 'I' ): 
    m_bTrackingTT = !m_bTrackingTT ; 
    TRACE( "\nKeyDown: m_bTrackingSwitch=%d m_bTrackingTT=%d" , m_bTrackingSwitch , m_bTrackingTT ) ;
    break ;
  case _T( 'r' ):
  case _T( 'R' ): m_bShowRGB = !m_bShowRGB ; break ;
  case _T( 'h' ) :
  case _T( 'H' ) :
  case VK_F1:
    {
      CRect FRenderRC ;
      GetWindowRect( &FRenderRC ) ;
      m_HelpWnd.SetWindowPos( &wndTopMost ,
        FRenderRC.left + 30 , FRenderRC.top + 40 , 0 , 0 , SWP_SHOWWINDOW | SWP_NOSIZE ) ;
      m_HelpWnd.SetOutputText( FrenderHelpText ) ;
    }
    break ;
  }
  CDIBView::OnKeyDown( nChar , nRepCnt , nFlags );
}


void CDIBFRender::OnMButtonDown( UINT nFlags , CPoint point )
{
  bool bMPressed = ( GetAsyncKeyState( VK_RCONTROL ) & 0x8000 ) != 0 ;
  if ( bMPressed )
  {
    DIB_EVENT( DIBVE_DO_MAXIMIZE , (VOID*)(&point) ) ;
    return ;
  }
  else
  {
    double dNewScale = ( m_Scale >= 16. ) ?
      -1. : ( m_Scale < 1. ) ? 1. : m_Scale * 2. ;
    SetScale( dNewScale , point ) ;
  }

  CDIBView::OnMButtonDown( nFlags , point );
}

void CDIBFRender::SetExtData( const void * pDataFrameWithGraphics )
{
  m_Lock.Lock();
  if ( m_pTmpFrame )
    ((CDataFrame*) m_pTmpFrame)->Release() ;
  if ( pDataFrameWithGraphics )
  {
    ((CDataFrame*) pDataFrameWithGraphics)->AddRef() ;
    m_pTmpFrame = (const CDataFrame*) pDataFrameWithGraphics ;
#ifdef _DEBUG
    m_iTmpFrameId = m_pTmpFrame->GetId();
    m_TmpFrameLabel = m_pTmpFrame->GetLabel();
    m_iTmpUserCnt = m_pTmpFrame->GetUserCnt();
#endif
  }
  else
    m_pTmpFrame = NULL ;
  m_Lock.Unlock();
}


// BOOL CDIBFRender::PreTranslateMessage(MSG* pMsg)
// {
//   switch( pMsg->message )
//   {
//   case WM_MOUSEWHEEL:
//     if ( pMsg->hwnd == m_hWnd )
//     {
//       TRACE("\n My WheelMsg") ;
//     }
//     break ;
//   }
// 
// 
//   return CDIBView::PreTranslateMessage(pMsg);
// }
// 
// 
// LRESULT CDIBFRender::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
// {
//   // TODO: Add your specialized code here and/or call the base class
// 
//   return CDIBView::DefWindowProc(message, wParam, lParam);
// }


void CDIBFRender::OnLButtonDblClk( UINT nFlags , CPoint point )
{
  CPoint pt = point;
  Scr2Pic( pt );
  DIB_EVENT( DIBVE_LBUTTONDBL , (void*) (&pt) );


  CDIBView::OnLButtonDblClk( nFlags , point );
}


void CDIBFRender::OnSize( UINT nType , int cx , int cy )
{
  HWND hWnd = m_hWnd;
  CRect MyRC;
  GetWindowRect( &MyRC );
  CWnd * pParent = GetParent();
  HWND hParentWnd = (pParent) ? pParent->m_hWnd : NULL;
  CRect ParRC( 0 , 0 , 0 , 0 ) ;
  if ( hParentWnd )
    ::GetWindowRect( hParentWnd , &ParRC );

  FXString Info;
  Info.Format( "hWnd=0x%08X[%d,%d,%d,%d]->[%d,%d] hParent=0x%08X[%d,%d,%d,%d] Mode=%d " ,
    hWnd , MyRC.left , MyRC.top , MyRC.Width() , MyRC.Height() , cx , cy ,
    ParRC.left , ParRC.top , ParRC.Width() , ParRC.Height() , nType );
  TRACE( "%s" , (LPCTSTR) Info );

  CDIBView::OnSize( nType , cx , cy );

  // TODO: Add your message handler code here
}

HWND CDIBFRender::CreateTrackingToolTip( int toolID , TCHAR * pText )
{
//   memset( &m_ToolTipInfo , 0 , sizeof( m_ToolTipInfo ));
//   m_ToolTipInfo.cbSize = sizeof( m_ToolTipInfo );
//   m_ToolTipInfo.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
//   m_ToolTipInfo.hwnd = m_hWnd;
//   m_ToolTipInfo.uId = (UINT_PTR) m_hWnd;
//   m_ToolTipInfo.hinst = GetModuleHandle( NULL ) ;
//   ::GetClientRect( m_hWnd , &m_ToolTipInfo.rect );
//   ::GetClientRect( m_hWnd , &m_ToolTipInfo.rect );
//   CPoint pt( m_ToolTipInfo.rect.left , m_ToolTipInfo.rect.top ) ;
//   ClientToScreen( &pt ) ;
// 
//   // Create a tooltip.
//   HWND hwndTT = CreateWindowEx( WS_EX_TOPMOST , /*TOOLTIPS_CLASS*/ WC_STATIC , NULL ,
//     /*WS_POPUP*/ WS_CHILD /*| TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON*/ ,
//     pt.x , pt.y , 100 , 40 ,
//     m_hWnd , NULL , NULL , NULL );
// 
//   if ( !hwndTT )
//   {
//     return NULL;
//   }
// 
//   ::SetWindowPos( hwndTT , HWND_TOPMOST , 0 , 0 , 0 , 0 ,
//     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
// 
  // Set up the tool information. In this case, the "tool" is the entire parent window.

//   // Associate the tooltip with the tool window.
//   DWORD dwErr = 0 ;
//   DWORD dwRes = ::SendMessage( hwndTT , TTM_ADDTOOL , 0 , (LPARAM) &m_ToolTipInfo );
//   FXString ErrString ;
//   if ( !dwRes )
//     ErrString = FxLastErr2Mes( "AddTooltip Error: ", "" ) ;
//   dwRes = ::SendMessage( hwndTT , TTM_TRACKACTIVATE , (WPARAM) TRUE , (LPARAM) &m_ToolTipInfo );
//   if ( !dwRes )
//     ErrString = FxLastErr2Mes( "Track Acitvate Error: " , "" ) ;
//   dwRes = ::SendMessage( hwndTT , TTM_GETTOOLINFO , 0 , (LPARAM) &m_ToolTipInfo );
//   if ( !dwRes )
//     ErrString = FxLastErr2Mes( "Get Tool Info Error: " , "" ) ;
//   dwRes = ::SendMessage( hwndTT , TTM_SETDELAYTIME , TTDT_AUTOMATIC , (LPARAM)100 );
//   EnableToolTips( TRUE ) ;
//   EnableTrackingToolTips( TRUE ) ;
  return/* hwndTT*/ NULL;
}

void CDIBFRender::OnMouseLeave()
{
  m_bTrackingTT = false ;
  m_bTrackingSwitch = false ;
  TRACE( "\nMouseLeave: m_bTrackingSwitch=%d m_bTrackingTT=%d" , m_bTrackingSwitch , m_bTrackingTT ) ;

  Invalidate( FALSE ) ;
  CDIBView::OnMouseLeave();
}


int CDIBFRender::DrawToolTip( CDC * dc , LPCTSTR pTip )
{
  if ( m_bTrackingTT )
  {
    CPoint TipPoint( m_LastMousePos ) ;
    CRect cr ;
    GetClientRect( &cr ) ;

    if ( cr.PtInRect( TipPoint ) )
    {
      CPoint TipPos ;
      if ( (TipPoint.x - cr.left) > (cr.right - TipPoint.x) )
        TipPos.x = TipPoint.x - 100 ;
      else
        TipPos.x = TipPoint.x + 20 ;
      if ( (TipPoint.y - cr.top) > (cr.bottom - TipPoint.y) )
        TipPos.y = TipPoint.y - 50 ;
      else
        TipPos.y = TipPoint.y + 10 ;

      CFont * pOldFont = dc->SelectObject( GetFont( 10 ) );
      COLORREF PrevTextColor = dc->SetTextColor( 0xffffff );
      int OldBackMode = dc->SetBkMode( OPAQUE ) ;
      COLORREF PrevBkColor = dc->SetBkColor( RGB( 60 , 60 , 40 ) ) ;

      CRect rc( TipPos , CSize( 70 , 40 ) );
      if ( rc.left < 0 )
        rc.OffsetRect( -rc.left , 0 ) ;
      if ( rc.right > ( int )GetWidth( m_Frame ) )
        rc.OffsetRect( -((int) rc.right - (int) GetWidth( m_Frame )) , 0 ) ;

      if ( !pTip )
      {
        cmplx cTipPnt = Scr2PicCmplx( TipPoint ) ;
        FXString OutString ;
        if ( FormToolTipText( TipPoint , 0 , OutString , false , m_bShowRGB ) )
        {
          if ( (m_cScale.real() != 1.) || (m_cScale.imag() != 0.) )
          {
            LONG iWidth , iHeight ;
            GetImageSize( iWidth , iHeight ) ;
            cTipPnt -= cmplx( (DOUBLE) iWidth / 2. , (double) iHeight / 2. ) ;
            cmplx cScaledPt = ConvertCoordsRelativeToCenter( cTipPnt ) ;
            TCHAR Buf[ 40 ] ;
            sprintf_s( Buf , "\nRel(%.2f,%.2f)%s" ,
              cScaledPt.real() , cScaledPt.imag() , (LPCTSTR)m_sUnits ) ;
            OutString += Buf ;
          }
          dc->DrawText( (LPCTSTR) OutString , (int) OutString.GetLength() ,
            &rc , DT_LEFT | DT_NOCLIP );
        }
      }
      else
        dc->DrawText( pTip , (int)_tcslen( pTip ) , &rc , DT_LEFT | DT_NOCLIP );
      dc->SelectObject( pOldFont ) ;
      dc->SetTextColor( PrevTextColor ) ;
      dc->SetBkColor( PrevBkColor ) ;
      dc->SetBkMode( OldBackMode ) ;
      return 1 ;
    }
  }
  return 0;
}

bool CDIBFRender::FormToolTipText(
  CPoint& OnImagePt , DWORD dwBitMask , FXString& OutS , 
  bool bExt , bool bViewRGB , bool bOneString )
{
  OutS.Empty() ;

  if ( bExt )
  {
    bool bLocked = m_LockOMutex.Lock( 200
#ifdef _DEBUG
      , "CDIBFRender::FormToolTipText"
#endif
    ) ;
    if ( !bLocked )
      return false ;
  }
  pTVFrame ptv = GetFramePntr() ;
  if ( ptv && __verifyInFrame( ptv , OnImagePt ) )
  {
    int I = 0 , U = 0 , V = 0;
    DWORD dwCompr = GetCompression( ptv ) ;
    bool bBW = (dwCompr == BI_Y8 || dwCompr == BI_Y800 || dwCompr == BI_Y16) ;
    if ( ptv->lpBMIH->biCompression != BI_RGB )
    {
      if ( __getdata_IUV( ptv , OnImagePt.x , OnImagePt.y , I , U , V ) )
      {
        if ( bViewRGB && !bBW )
        {
          int    u = (U - 128) << 13 ;
          int    v = (V - 128) << 13 ;
          int    R = I + v / 7185 ;
          int    G = I - (u / 20687 + v / 14100) ;
          int    B = I + u / 4037 ;
          BOUND( R , 0 , 255 ) ;
          BOUND( G , 0 , 255 ) ;
          BOUND( B , 0 , 255 ) ;
          if ( R < 0 )
            R = 0 ;
          else if ( R > 255 )
            R = 255 ;

          LPCTSTR pFormat = (!bOneString) ?
            "x=%d;y=%d;\nR=%d;G=%d;B=%d"
            : "x=%d;y=%d;R=%d;G=%d;B=%d;Keys=%d;" ;
          OutS.Format( pFormat ,
            OnImagePt.x , OnImagePt.y ,
            R , G , B , dwBitMask ) ;
        }
        else if ( !bBW ) // not grey scale image
        {
          LPCTSTR pFormat = (!bOneString) ?
            "x=%d;y=%d;\nI=%d;U=%d;V=%d;"
            : "x=%d;y=%d;I=%d;U=%d;V=%d;Keys=%d;" ;
          OutS.Format( pFormat ,
            OnImagePt.x , OnImagePt.y , I , U - 128 , V - 128 , dwBitMask );
        }
        else // Grey scale image
        {
          LPCTSTR pFormat = (!bOneString) ?
            "x=%d;y=%d;\nI=%d"
            : "x=%d;y=%d;I=%d;Keys=%d;" ;
          OutS.Format( pFormat ,
            OnImagePt.x , OnImagePt.y , I , dwBitMask );
        }
      }
    }
    else  // BI_RGB
    {
      int R = 0 , G = 0 , B = 0 ;
      if ( __getdata_RGB( ptv , OnImagePt.x , OnImagePt.y , R , G , B ) )
      {
        LPCTSTR pFormat = (!bOneString) ?
          "x=%d;y=%d;\nR=%d;G=%d;B=%d;"
          : "selected=true;x=%d;y=%d;R=%d;G=%d;B=%d;Keys=%d;" ;
        OutS.Format( pFormat ,
          OnImagePt.x , OnImagePt.y , R , G , B , dwBitMask );
      }
    }
  }
  if ( bExt )
    m_LockOMutex.Unlock() ;

  return !OutS.IsEmpty() ;
}


void CDIBFRender::OnKeyUp( UINT nChar , UINT nRepCnt , UINT nFlags )
{

  CDIBView::OnKeyUp( nChar , nRepCnt , nFlags );
}


void CDIBFRender::OnTimer( UINT_PTR nIDEvent )
{
  switch ( m_iMouseMoveTimer )
  {
  case TMR_MOUSE_MOVE: 
    Invalidate( FALSE ) ;
    KillTimer( TMR_MOUSE_MOVE ) ;
    m_iMouseMoveTimer = 0 ;
    break ;
  }

  CDIBView::OnTimer( nIDEvent );
}

cmplx CDIBFRender::Scr2PicCmplx( CPoint &point )
{
  if ( m_ScrScale < 0.01 )
  {
    point = CPoint( 0 , 0 );
    return cmplx();
  }
  point.x -= m_ScrOffset.x;
  point.y -= m_ScrOffset.y;
  cmplx OnPic( point.x / m_ScrScale , point.y / m_ScrScale ) ;
  point.x = ROUND( OnPic.real() );
  point.y = ROUND( OnPic.imag() );
  point -= m_IntBmOffset;
  OnPic -= cmplx( m_IntBmOffset.x + 0.5 , m_IntBmOffset.y + 0.5 ) ;
  return OnPic ;
}

cmplx CDIBFRender::ConvertCoordsRelativeToCenter( cmplx& cRelToCenter )
{
  cmplx cConverted = cRelToCenter * m_cScale ;
  if ( m_bConjugateScaleConvert )
    cConverted._Val[ _IM ] = -cConverted._Val[ _IM ] ;

  return cConverted;
}
