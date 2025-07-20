#include "stdafx.h"
#include <video\shvideo.h>
#include <video\stdcodec.h>

#define BK_COLOR (RGB(40,0,0))

#pragma comment(lib,"ddraw.lib")

/////////////////////////////////////////////////////////////////
/////// helper functions for CDXView

HRESULT DDCopyBitmap(IDirectDrawSurface *pdds, HBITMAP hbm, int x, int y, int dx, int dy)
{
  HDC                 hdcImage;
  HDC                 hdc;
  DDSURFACEDESC       ddsd;
  HRESULT             hr;

  if (hbm == NULL || pdds == NULL) return E_FAIL;

  hdcImage = CreateCompatibleDC(NULL);
  if (!hdcImage)	TRACE("DDCopyBitmap: createcompatible dc failed\n");
  SelectObject(hdcImage, hbm);

  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
  pdds->GetSurfaceDesc(&ddsd);

  if ((hr = pdds->GetDC(&hdc)) == DD_OK)
  {
    BitBlt(hdc,0,0,ddsd.dwWidth, ddsd.dwHeight,hdcImage, 0, 0, SRCCOPY);
    pdds->ReleaseDC(hdc);
  }
  else
  {
    TRACE("DDCopyBitmap: Can't get DDDC\n");
  }
  DeleteDC(hdcImage);
  return hr;
}

HRESULT DDLoadBitmap(IDirectDraw *pdd, HBITMAP hbm, IDirectDrawSurface *&pdds)
{
  HRESULT             hr;
  DDSURFACEDESC       ddsd;
  BITMAP              bm;

  GetObject(hbm, sizeof(bm), &bm);      // get size of bitmap
  if (pdds==NULL)
  {
    //
    // create a DirectDrawSurface for this bitmap
    //
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth = bm.bmWidth;
    ddsd.dwHeight = bm.bmHeight;

    if (pdd->CreateSurface(&ddsd, &pdds, NULL) != DD_OK) return DD_FALSE;
  }
  hr=DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
  return hr;
}

/////////////////////////////////////////////////////////////////
/////// class CDXView

BEGIN_MESSAGE_MAP(CDXView, CDIBView)
  ON_WM_DESTROY()
  ON_WM_PAINT()
  ON_WM_SIZE()
END_MESSAGE_MAP()

CDXView::CDXView(LPCTSTR name):
  CDIBView(name),
  m_DD(NULL),
  m_SurfPri(NULL),
  m_SurfSec(NULL),
  m_Clipper(NULL),
  m_hBM(NULL),
  m_BkBrush(NULL),
  m_BMIH(NULL),
  m_ViewWidth(0), 
  m_ViewHeight(0)
{
}

CDXView::~CDXView(void)
{
}

BOOL CDXView::Create(CWnd* pParentWnd, DWORD dwAddStyle,UINT nID)
{
  BOOL RESULT=TRUE;
  RECT rect;

  m_BkBrush=CreateSolidBrush(BK_COLOR);

  EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS, &m_DevMode);

  pParentWnd->GetClientRect(&rect);
  m_ViewWidth=rect.right-rect.left;
  m_ViewHeight=rect.bottom-rect.top;

  LPCTSTR lpszDIBVClass =  AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW |CS_PARENTDC, LoadCursor(NULL, IDC_ARROW), (HBRUSH) ::GetStockObject(DKGRAY_BRUSH));
  RESULT =  CWnd::Create(lpszDIBVClass,"CDXView",WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|dwAddStyle,rect,pParentWnd,nID);

  if (RESULT)
  {
    SetScrollRange(SB_HORZ,0,1000);
    SetScrollRange(SB_VERT,0,1000);
    ShowScrollBar(SB_HORZ, FALSE);
    ShowScrollBar(SB_VERT, FALSE);
  }

  Invalidate();
  return RESULT;
}

void CDXView::OnDestroy()
{
  if (m_SurfSec != NULL)
  {
    m_SurfSec->Release();
    m_SurfSec=NULL;
  }
  if (m_Clipper!=NULL)
  {
    m_Clipper->Release();
    m_Clipper=NULL;
  }
  if (m_SurfPri != NULL)
  {
    m_SurfPri->Release();
    m_SurfPri=NULL;
  }
  if( m_DD != NULL )
  {
    m_DD->Release();
    m_DD = NULL;
  }
  if (m_hBM) DeleteObject(m_hBM); m_hBM=NULL;
  if (m_BkBrush) DeleteObject(m_BkBrush); m_BkBrush=NULL;
  if (m_BMIH) free(m_BMIH);
  CDIBView::OnDestroy();
}

bool CDXView::LoadFrame(const pTVFrame frame, bool bForceInvalidate)
{
  if ((!frame) || (!frame->lpBMIH))
    return false;
  if ( !m_LockOMutex.Lock( 1000
  #ifdef _DEBUG
    , "CDXView::LoadFrame"
  #endif
    ) )
    return false ;

  if (m_BMIH) 
    free(m_BMIH);
  if (frame)
  {
    m_BMIH=_decompress2rgb( frame , (m_Monochrome != 0) );
    m_Width=m_BMIH->biWidth;
    m_Height=m_BMIH->biHeight;
  }
  else 
  {
    m_BMIH=NULL;
    m_Width=0;
    m_Height=0;
  }
  m_LockOMutex.Unlock() ;
  UpdateFrame();
  return true;
}

void CDXView::OnPaint()
{
  CPaintDC dc(this); 
  if (m_DD)	
    UpdateFrame();
}

BOOL CDXView::DrawOverBitmap()
{
  HDC                 hdcImage;
  hdcImage = CreateCompatibleDC(NULL);
  if (!hdcImage)
  {
    TRACE("UpdateFrame: Create compatible DC failed\n");
    return FALSE;
  }

  HGDIOBJ oldObj=SelectObject(hdcImage,m_hBM);
  if (!oldObj)	
  {
    TRACE("UpdateFrame: Can't select bitmap to current DC\n");
    return FALSE;
  }
  FillRect(hdcImage,CRect(0,0,m_ViewWidth,m_ViewHeight),m_BkBrush);
  if (CDIBView::Draw(hdcImage,(RECT&)CRect(0,0,m_ViewWidth,m_ViewHeight)))
  {
    CDC dc; dc.Attach(hdcImage);
    if (m_Selection)
    {
      TRACE("Draw selection\n");
      CPen*    oPen;
      if ((m_Selection->LineEnable) && (m_Selection->DrawLine))
      {
        if (m_Selection->LPressed)
        {
          oPen=(CPen*)dc.SelectObject(m_Selection->Pen);
          CPoint a,b; a=m_Selection->p1; b=m_Selection->p2;
          SubPix2Scr(a); SubPix2Scr(b);
          dc.MoveTo(a); dc.LineTo(b);
          dc.SelectObject(&oPen);
        }
        else
        {
          oPen=(CPen*)dc.SelectObject(m_Selection->Pen);
          CPoint a(m_Selection->Object.left,m_Selection->Object.top),
            b(m_Selection->Object.right,m_Selection->Object.bottom);
          SubPix2Scr(a); SubPix2Scr(b);
          dc.MoveTo(a); dc.LineTo(b);
          dc.SelectObject(&oPen);
        }
      }
      else if ((m_Selection->RectEnable) && (m_Selection->DrawRect))
      {
        if (m_Selection->RPressed)
        {
          oPen=(CPen*)dc.SelectObject(m_Selection->Pen);
          CPoint a,b; a=m_Selection->p1; b=m_Selection->p2;
          SubPix2Scr(a); SubPix2Scr(b);
          CPoint pl[5]; pl[0]=a; pl[1]=CPoint(b.x,a.y); pl[2]=b; pl[3]=CPoint(a.x,b.y); pl[4]=a;
          dc.Polyline(pl,5);
          dc.SelectObject(&oPen);
        }
        else
        {
          oPen=(CPen*)dc.SelectObject(m_Selection->Pen);
          CPoint a(m_Selection->Object.left,m_Selection->Object.top),
            b(m_Selection->Object.right,m_Selection->Object.bottom);
          SubPix2Scr(a); SubPix2Scr(b);
          CPoint pl[5]; pl[0]=a; pl[1]=CPoint(b.x,a.y); pl[2]=b; pl[3]=CPoint(a.x,b.y); pl[4]=a;
          dc.Polyline(pl,5);
          dc.SelectObject(&oPen);
        }
      }
    }
    if (m_DrawEx) m_DrawEx(dc.m_hDC,(RECT&)CRect(0,0,m_ViewWidth,m_ViewHeight),this,m_DrawExParam);
    dc.Detach();
  }
  SelectObject(hdcImage,oldObj);
  DeleteDC(hdcImage); 
  return TRUE;
}

BOOL CDXView::UpdateFrame()
{
  HRESULT             ddrval;

  if (m_DD==NULL) 
    return FALSE;
  if ( !m_LockOMutex.Lock( 1000
#ifdef _DEBUG
    , "CDXView::UpdateFrame"
#endif
    ) )
    return false; 


start:
  DrawOverBitmap();
  DDLoadBitmap(m_DD, m_hBM, m_SurfSec);

  CPoint pt(0,0);
  CRect destRect(0,0,m_ViewWidth,m_ViewHeight);
  ClientToScreen( &pt );
  OffsetRect(&destRect, pt.x, pt.y);

  while( 1 )
  {
    m_LockSurf.Lock();
    ddrval = m_SurfPri->Blt( destRect, m_SurfSec,CRect(0,0,m_ViewWidth,m_ViewHeight), 0, NULL );
    m_LockSurf.Unlock();
    if( ddrval == DD_OK )
    {
      break;
    }
    else if( ddrval == DDERR_SURFACELOST )
    {
      if(!RestoreAll())
      {
        TRACE("UpdateFrame3: Can't restore.\n");
        if (!ReInit())
        {
          TRACE("UpdateFrame3: Can't ReInit.\n");
        }
        else 
          goto start;
        m_LockOMutex.Unlock() ;
        return FALSE;
      }
      TRACE("UpdateFrame3: Restored.\n");
    }
    else if( ddrval != DDERR_WASSTILLDRAWING )
    {
      TRACE("UpdateFrame3: Still drawed, skipping\n");
      m_LockOMutex.Unlock() ;
      //     m_LockDrData.Unlock();
      return FALSE;
    }
    else
    {
      TRACE("UpdateFrame3: Unexpected error, skipping\n");
      return FALSE;
    }
  }
  m_LockOMutex.Unlock() ;
  return TRUE;
}

BOOL CDXView::ReInit()
{
  FXAutolock al(m_LockSurf);

  if (m_SurfSec != NULL)
  {
    m_SurfSec->Release();
    m_SurfSec=NULL;
  }
  if (m_Clipper!=NULL)
  {
    m_Clipper->Release();
    m_Clipper=NULL;
  }
  if (m_SurfPri != NULL)
  {
    m_SurfPri->Release();
    m_SurfPri=NULL;
  }
  if( m_DD != NULL )
  {
    m_DD->Release();
    m_DD = NULL;
  }

  if (DirectDrawCreate( NULL, &m_DD, NULL )!=DD_OK) return(FALSE);
  if (m_DD->SetCooperativeLevel( this->GetSafeHwnd(), DDSCL_NORMAL)!=DD_OK) return(FALSE);

  // Creating primary surface
  DDSURFACEDESC       ddsd;
  memset(&ddsd,0,sizeof(DDSURFACEDESC));

  ddsd.dwSize = sizeof( ddsd );
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  HRESULT res = m_DD->CreateSurface( &ddsd, &m_SurfPri, NULL );

  // create a clipper for the primary surface
  res = m_DD->CreateClipper( 0, &m_Clipper, NULL );

  res = m_Clipper->SetHWnd( 0, this->GetSafeHwnd() );

  res = m_SurfPri->SetClipper( m_Clipper );

  return(DDLoadBitmap(m_DD, m_hBM, m_SurfSec)==DD_OK);
}

BOOL CDXView::RestoreAll()
{
  FXAutolock al(m_LockSurf);
  return m_SurfPri->Restore() == DD_OK &&
    m_SurfSec->Restore()     == DD_OK &&
    DDLoadBitmap(m_DD, m_hBM, m_SurfSec) == DD_OK;
}

void CDXView::OnSize(UINT nType, int cx, int cy)
{
  if (this->m_hWnd)
    UpdateBitmap(cx, cy);
  CDIBView::OnSize(nType, cx, cy);
}

BOOL CDXView::UpdateBitmap(int w, int h)
{
  m_ViewWidth=w;
  m_ViewHeight=h;
  if (m_hBM) DeleteObject(m_hBM); m_hBM=NULL;
  if (w*h)
  {
    m_hBM=::CreateBitmap(m_ViewWidth,m_ViewHeight,1,m_DevMode.dmBitsPerPel,NULL);
    DrawOverBitmap();
    ASSERT(m_hBM!=NULL);
    ReInit();
    //UpdateFrame();
    Invalidate();
    return TRUE;
  }
  return FALSE;
}