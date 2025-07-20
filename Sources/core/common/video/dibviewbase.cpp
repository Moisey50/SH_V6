// DIBViewBase.cpp : implementation file
//

#include "stdafx.h"
#include <video\shvideo.h>
#include <imageproc\cut.h>

#define DIB_EVENT(EVENT,DATA) if (m_CallBack) m_CallBack(EVENT,(void*)DATA,m_CallBackParam,this); 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

__forceinline bool rectisempty(RECT rc)
{
  return ((rc.right==rc.left) && (rc.top=rc.bottom));
}

__forceinline void orderval( LONG& x0, LONG& x1)
{
  if (x0>x1)
  {
    int r=x0;
    x1=x0; x0=r;
  }
}

__forceinline void normalizerect(RECT& rc)
{
  orderval( rc.left, rc.right);
  orderval( rc.top,  rc.bottom);
}

/////////////////////////////////////////////////////////////////////////////
// CDIBViewBase

CDIBViewBase::CDIBViewBase(LPCTSTR name): 
  m_Frame(NULL),
  m_ScrScale(1.0),
  m_ScrOffset(0,0),
  m_IntBmOffset(0,0),
  m_pConvertedBMIH(NULL),
  m_Scale(1.0),
  m_CallBack(NULL),
  m_CallBackParam(NULL),
  m_DrawEx(NULL),
  m_DrawExParam(NULL),
  m_Selection(NULL),
  m_Width(0), m_Height(0),
  m_Name(name),
  m_Monochrome(false),
  m_SizeChanged(true),
  m_DataMonochrome(false),
  m_LockOMutex( NULL )
{
  m_Frame = makeNewY8Frame( 640 , 480 ) ; 
  TRACE("+++ Create DIBView with name %s\n",name);
}

CDIBViewBase::~CDIBViewBase()
{
  Reset();
}

void CDIBViewBase::Reset()
{
  if ( !m_LockOMutex.Lock( 1000
  #ifdef _DEBUG
    , "CDIBViewBase::Reset"
  #endif
    ) )
    return ;

    if (m_Frame) 
    {
      freeTVFrame(m_Frame); 
      m_Frame=NULL;
    }
    if (m_pConvertedBMIH) 
    {
      free(m_pConvertedBMIH); 
      m_pConvertedBMIH=NULL;
    }
    m_Width=0; m_Height=0;
    FreeSelBlock();
  m_LockOMutex.Unlock() ;
//  else
//    TRACE( "\nCDIBViewBase::Reset - can't take mutex" ) ;
}

BEGIN_MESSAGE_MAP(CDIBViewBase, CWnd)
  //{{AFX_MSG_MAP(CDIBViewBase)
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_RBUTTONDOWN()
  ON_WM_RBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_NCACTIVATE()
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDIBViewBase message handlers
BOOL CDIBViewBase::Create(CWnd* pParentWnd, DWORD dwAddStyle, 
                          UINT nID , LPCTSTR szWindowName ) 
{
  BOOL RESULT;
  RECT rect;

  pParentWnd->GetClientRect(&rect);
  LPCTSTR lpszDIBVClass =  AfxRegisterWndClass(
    CS_HREDRAW | CS_VREDRAW |CS_PARENTDC, 
    LoadCursor(NULL, IDC_ARROW), 
    (HBRUSH) ::GetStockObject(DKGRAY_BRUSH));
  RESULT =  CWnd::Create(lpszDIBVClass,szWindowName,
    WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|dwAddStyle,rect,pParentWnd,nID);
  if (RESULT) 
    UpdateWindow();
  
  return (RESULT);
}

void CDIBViewBase::OnDestroy() 
{
  Reset();	
  CWnd::OnDestroy();
}

void CDIBViewBase::InvalidateRect(CRect rc, BOOL bErase)            
{ 
  CPoint pc1(rc.left,rc.top),pc2(rc.right,rc.bottom); 
  Pic2Scr(pc1); Pic2Scr(pc2); 
  pc1.x--; pc1.y--;
  pc2.x++; pc2.y++;
  CWnd::InvalidateRect(CRect(pc1.x,pc1.y,pc2.x,pc2.y),bErase);
};


void CDIBViewBase::InitSelBlock(bool SelLine, bool SelRect)
{
  FreeSelBlock();
  if ((SelLine) || (SelRect))
  {
    m_Selection=(pSelection)malloc(sizeof(Selection));
    memset(m_Selection,0,sizeof(Selection));
    m_Selection->LineEnable=SelLine;
    m_Selection->RectEnable=SelRect;
    m_Selection->Pen=new CPen(PS_SOLID,1,RGB(255,0,0));
  }
}

void CDIBViewBase::FreeSelBlock()
{
  if (m_Selection) 
  {
    delete m_Selection->Pen; 
    free(m_Selection);
  }
  m_Selection=NULL;
}

void CDIBViewBase::GetSelRect(RECT &rc)
{
  int Style;
  if (m_Selection)
  {
    Style=(m_Selection->DrawRect)?SEL_RECT:(m_Selection->DrawLine)?SEL_LINE:SEL_NOTHING;
    if (Style==SEL_RECT)
    {
      memcpy(&rc,&(m_Selection->Object), sizeof(RECT));
    }
    else
      memset(&rc,0,sizeof(rc));
  }
  else 
  {
    memset(&rc,0,sizeof(rc));
    Style = SEL_NOTHING ;
  }
}

void CDIBViewBase::GetSelLine(RECT& rc)
{
  int Style;
  if (m_Selection)
  {
    Style=(m_Selection->DrawRect)?SEL_RECT:(m_Selection->DrawLine)?SEL_LINE:SEL_NOTHING;
    if (Style==SEL_LINE)
    {
      memcpy(&rc,&(m_Selection->Object), sizeof(RECT));
    }
    else
      memset(&rc,0,sizeof(rc));
  }
  else 
  {
    memset(&rc,0,sizeof(rc));
    Style = SEL_NOTHING ;
  }
}

int  CDIBViewBase::GetSelStyle() 
{ 
  if (m_Selection)
    return ((m_Selection->DrawRect)?SEL_RECT:(m_Selection->DrawLine)?SEL_LINE:SEL_NOTHING);
  return SEL_NOTHING ;
}

bool CDIBViewBase::IsSelected()
{
  if (m_Selection)
    return ((m_Selection->DrawLine) || (m_Selection->DrawRect));
  return false;
}

void CDIBViewBase::SelectRectangle(RECT rc)
{
  if (m_Selection)	
  {
    m_Selection->Object.left=rc.left;
    m_Selection->Object.top=rc.top;
    m_Selection->Object.right=rc.right;
    m_Selection->Object.bottom=rc.bottom;
    m_Selection->DrawRect=!rectisempty(m_Selection->Object);
    m_Selection->DrawLine=false;
    m_Selection->Object.NormalizeRect();
    DIB_EVENT(DIBVE_RECTSELECTEVENT,(void*)(&m_Selection->Object));
    Invalidate();
  }
}

void CDIBViewBase::SelectLine(RECT rc)
{
  if (m_Selection)	
  {
    m_Selection->Object.left=rc.left;
    m_Selection->Object.top=rc.top;
    m_Selection->Object.right=rc.right;
    m_Selection->Object.bottom=rc.bottom;
    m_Selection->DrawRect=!rectisempty(m_Selection->Object);
    m_Selection->DrawLine=false;
    DIB_EVENT(DIBVE_RECTSELECTEVENT,(void*)(&m_Selection->Object));
    Invalidate();
  }
}

void CDIBViewBase::SelectAll()
{
  if (m_Selection)	
  {
    m_Selection->Object.left=0;
    m_Selection->Object.top=0;
    m_Selection->Object.right=m_Width-1;
    m_Selection->Object.bottom=m_Height-1;
    m_Selection->DrawRect=!rectisempty(m_Selection->Object);
    m_Selection->DrawLine=false;
    DIB_EVENT(DIBVE_RECTSELECTEVENT,(void*)(&m_Selection->Object));
    Invalidate();
  }
}

void CDIBViewBase::ResetSelection()
{
  if (m_Selection)
    m_Selection->DrawLine=m_Selection->DrawRect=SEL_NOTHING;
}

bool CDIBViewBase::PrepareData(const pTVFrame frame, /*const void * pExtData ,*/ DWORD dwTimeOut)
{
  if ( !m_LockOMutex.Lock( dwTimeOut
  #ifdef _DEBUG
    , "CDIBViewBase::PrepareData"
  #endif
      ) )
    return false ;

  //SetExtData( pExtData ) ;
  if ((!frame) || (!frame->lpBMIH))
  {
    if (m_Frame)
    {
      freeTVFrame(m_Frame); m_Frame=NULL;
      m_Width=0; m_Height=0;
      if (m_pConvertedBMIH) 
      {
        free(m_pConvertedBMIH); 
        m_pConvertedBMIH=NULL;
      }
    }
    m_SizeChanged=true;
    m_LockOMutex.Unlock();
    return false;
  }

  m_SizeChanged |= ((frame->lpBMIH->biWidth!=m_Width) || (frame->lpBMIH->biHeight!=m_Height));
  if ((m_Frame) && (m_Frame!=frame)) 
  {
    freeTVFrame(m_Frame); m_Frame=NULL;
    m_Frame=makecopyTVFrame(frame);
  }
  else if (m_Frame==NULL)
  {
    m_Frame=makecopyTVFrame(frame);
  }
  if ((m_pConvertedBMIH) && (m_SizeChanged))
  {
    free(m_pConvertedBMIH); 
    m_pConvertedBMIH=NULL;
    m_SizeChanged = false ;
  } 
  if (m_Frame)
  {
    m_pConvertedBMIH = PrepareImage(m_Frame,m_pConvertedBMIH);
    m_Width=m_Frame->lpBMIH->biWidth; m_Height=m_Frame->lpBMIH->biHeight;
  }
  m_LockOMutex.Unlock();
  return true;
}

BITMAPINFOHEADER* CDIBViewBase::PrepareImage(pTVFrame frame, LPVOID lpBuf)
{
  if (frame->lpBMIH->biCompression==BI_Y16)
  {
    if ((!m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=true;
    }
    return y16rgb8(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if ( (frame->lpBMIH->biCompression==BI_Y8)
    || (frame->lpBMIH->biCompression==BI_Y800)
    || ((m_Monochrome) && (frame->lpBMIH->biCompression==BI_YUV9)))
  {
    if ((!m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=true;
    }
    return yuv9rgb8(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if (frame->lpBMIH->biCompression==BI_YUV9)
  {
    if ((m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=false;
    }
    return yuv9rgb24(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if ((frame->lpBMIH->biCompression==BI_YUV12) && (m_Monochrome))
  {
    if ((!m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=true;
    }
    return yuv9rgb8(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if (frame->lpBMIH->biCompression==BI_YUV12)
  {
    if ((m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=false;
    }
    return yuv12rgb24(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if (frame->lpBMIH->biCompression==BI_RGB)
  {
    if ((m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=false;
    }
    return rgbrgb24(frame->lpBMIH,frame->lpData,lpBuf);
  }
  if (lpBuf) 
    free(lpBuf);
  return NULL;
}
void CDIBViewBase::LoadFrameWait(pTVFrame frame /*, const void * pExtData*/ )
{
  PrepareData(frame /*, pExtData*/);
  if ( m_hWnd && IsWindow( m_hWnd ) )
    Invalidate( frame ? FALSE : TRUE );
  return;
}

bool CDIBViewBase::LoadFrame(const pTVFrame frame,/* const void * pExtData ,*/ bool bForceInvalidate)
{
//   bool bRes = PrepareData( frame , pExtData , 0 ) ;
  if ( (!frame) || (!frame->lpBMIH) || (!PrepareData(frame, 0))) 
  {
    return false;
  }
  if (bForceInvalidate)
  {
    if ( m_hWnd && IsWindow( m_hWnd ) )
    {
      Invalidate( frame ? FALSE : TRUE );
      PostMessage( WM_PAINT ) ;
    }
  }
  return true ;
}

bool CDIBViewBase::LoadDIB(BITMAPINFOHEADER* bmih, bool bForceInvalidate)
{
  TVFrame frame;
  frame.lpBMIH=bmih;
  frame.lpData=NULL;
  return LoadFrame(&frame, /*NULL , */bForceInvalidate);
}

bool CDIBViewBase::Draw(HDC hdc,RECT& rc)
{
  ResetData() ;
  LPBITMAPINFOHEADER lpBMIH= GetDrawData();
  if (!lpBMIH) 
    return false; 

  int x,y;

  x=(rc.right - lpBMIH->biWidth)/2;
  y=(rc.bottom - lpBMIH->biHeight)/2;
  m_ScrOffset=CPoint(x,y);
  SetStretchBltMode(hdc,STRETCH_DELETESCANS);
  bool res=SetDIBitsToDevice(hdc,x,y,lpBMIH->biWidth,lpBMIH->biHeight,
    0,0,0,lpBMIH->biHeight,
    GetData(lpBMIH), (BITMAPINFO*)lpBMIH,DIB_RGB_COLORS
    )!=0;
  if (m_SizeChanged)
  {
    Invalidate();
    m_SizeChanged=false;
  }
  return res;
}

void CDIBViewBase::SetActive()
{
  DIB_EVENT(DIBVE_ACTIVWNDCHEVENT,NULL);
}

int  CDIBViewBase::Pic2Scr(double len) 
{ 
  return (int)((len+0.5)*m_ScrScale+0.5);
}

int  CDIBViewBase::Scr2Pic(double len) 
{ 
  return (int)(len/m_ScrScale/*+0.5*/);
}

void CDIBViewBase::Scr2Pic(CPoint &point)
{
  if (m_ScrScale<0.01) { point=CPoint(0,0); return;}
  point.x-=m_ScrOffset.x;
  point.y-=m_ScrOffset.y;
  point.x=(int)(point.x/m_ScrScale/*+0.5*/);
  point.y=(int)(point.y/m_ScrScale/*+0.5*/);

  point-=m_IntBmOffset;
}

void CDIBViewBase::SubPix2Scr(CPoint &point)
{
  point+=m_IntBmOffset;
  if (m_ScrScale<0.01) { point=CPoint(0,0); return; }
  point.x=(int)((point.x+0.5)*m_ScrScale+0.5);
  point.y=(int)((point.y+0.5)*m_ScrScale+0.5);
  point.x+=m_ScrOffset.x;
  point.y+=m_ScrOffset.y;
}

void CDIBViewBase::Pic2Scr(CPoint &point)
{
  point+=m_IntBmOffset;
  if (m_ScrScale<0.01) { point=CPoint(0,0); return; }
  point.x=(int)(point.x*m_ScrScale+0.5);
  point.y=(int)(point.y*m_ScrScale+0.5);
  point.x+=m_ScrOffset.x;
  point.y+=m_ScrOffset.y;
}

bool CDIBViewBase::InFrame(CPoint pnt)
{
//   if ( !m_LockOMutex.Lock( 1000
//   #ifdef _DEBUG
//     , "CDIBViewBase::InFrame"
//   #endif
//     ) )
//     return false ;
// 
//   if (!GetDrawData()) 
//   {
//     m_LockOMutex.Unlock() ;
//     return false;
//   }
  CPoint p=pnt;
  Scr2Pic(p);
//   m_LockOMutex.Unlock() ;
  return (!((p.x<0) || (p.y<0) || (p.x>(m_Width-1)) || (p.y>(m_Height-1))));
}

// Events processing

void CDIBViewBase::OnPaint() 
{
  if ( !IsWindowVisible() )
    return ;

  CWnd * pParent = GetParent() ;
  if ( !pParent )
    return ;
  CRect ParentClientRect , ThisWindowRect ;
  pParent->GetClientRect( &ParentClientRect ) ;
  GetWindowRect( &ThisWindowRect ) ;
  if ( (ParentClientRect.Width() != ThisWindowRect.Width())
    || ParentClientRect.Height() != ThisWindowRect.Height() )
  {
    SetWindowPos( NULL , ParentClientRect.left ,
      ParentClientRect.top , ParentClientRect.right ,
      ParentClientRect.bottom , SWP_NOZORDER ) ;
  }

  CPaintDC dc(this); 
  RECT rc;  
  GetClientRect(&rc);
  if ( !m_LockOMutex.Lock( 20
  #ifdef _DEBUG
    , "CDIBViewBase::OnPaint"
  #endif
    ) )
    return ;

    if (Draw(dc.m_hDC,rc))
    {
      if (m_Selection)
      {
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
      if (m_DrawEx) m_DrawEx(dc.m_hDC,rc,this,m_DrawExParam);
    }
  m_LockOMutex.Unlock() ;
}

void CDIBViewBase::OnLButtonDown(UINT nFlags, CPoint point) 
{
  //TRACE("++++++++++++++++++++++++++ OnLButtonDown\n");
  if (m_Selection)
  {
    m_Selection->LPressed=true;
    m_Selection->RPressed=false;
    if (m_Selection->LineEnable)
    {
      m_Selection->p1=point;
      m_Selection->p2=point;
      Scr2Pic(m_Selection->p1);
      Scr2Pic(m_Selection->p2);
      m_Selection->DrawLine=true;        
    }
  }
  if (GetDrawData())
  {
    CPoint pt=point;
    Scr2Pic(pt);
    DIB_EVENT(DIBVE_LBUTTONDOWN,(void*)(&pt)); 
  }
  CWnd::OnLButtonDown(nFlags, point);
}

void CDIBViewBase::OnLButtonUp(UINT nFlags, CPoint point) 
{
  //TRACE("++++++++++++++++++++++++++ OnLButtonUp\n");
  if ((m_Selection) && (m_Selection->LPressed))
  {
    m_Selection->p2=point;
    Scr2Pic(m_Selection->p2);
    if ((m_Selection->LineEnable) && (m_Selection->p1!=m_Selection->p2))
    {
      m_Selection->DrawLine=true;
      if (m_Selection->p1.x<m_Selection->p2.x)
      {
        m_Selection->Object.left  = m_Selection->p1.x;
        m_Selection->Object.top   = m_Selection->p1.y;
        m_Selection->Object.right = m_Selection->p2.x;
        m_Selection->Object.bottom= m_Selection->p2.y;
      }
      else
      {
        m_Selection->Object.left  = m_Selection->p2.x;
        m_Selection->Object.top   = m_Selection->p2.y;
        m_Selection->Object.right = m_Selection->p1.x;
        m_Selection->Object.bottom= m_Selection->p1.y;
      }
      m_Selection->p1.x = m_Selection->Object.left;
      m_Selection->p1.y = m_Selection->Object.top;
      m_Selection->p2.x = m_Selection->Object.right;
      m_Selection->p2.y = m_Selection->Object.bottom;
      m_Selection->DrawRect=false;
    }
    else if (m_Selection->DrawRect) //just move rectangle 
    {
      m_Selection->DrawLine=false;
      CPoint opos((m_Selection->Object.left+m_Selection->Object.right)/2,
        (m_Selection->Object.top+m_Selection->Object.bottom)/2);
      CSize  off=m_Selection->p2-opos;
      m_Selection->Object.left  +=off.cx;
      m_Selection->Object.top   +=off.cy;
      m_Selection->Object.right +=off.cx;
      m_Selection->Object.bottom+=off.cy;
      // check boundaries
      if (m_Selection->Object.left<0)
      {
        m_Selection->Object.right -= m_Selection->Object.left;
        m_Selection->Object.left   = 0;
      }
      if (m_Selection->Object.right>=m_Width)
      {
        m_Selection->Object.left -= m_Selection->Object.right-m_Width-1;
        m_Selection->Object.right = m_Width-1;
      }
      if (m_Selection->Object.top<0)
      {
        m_Selection->Object.bottom -= m_Selection->Object.top;
        m_Selection->Object.top   = 0;
      }
      if (m_Selection->Object.bottom>=m_Height)
      {
        m_Selection->Object.top -= m_Selection->Object.bottom-m_Height-1;
        m_Selection->Object.bottom = m_Height-1;
      }
      m_Selection->Object.NormalizeRect();
      DIB_EVENT(DIBVE_RECTSELECTEVENT,(void*)(&m_Selection->Object));
    }
    m_Selection->LPressed=false;
    DIB_EVENT(DIBVE_LINESELECTEVENT,(void*)(&m_Selection->Object));
    Invalidate(FALSE);
  } 
  else
  {
    CPoint pt=point;
    Scr2Pic(pt);
    DIB_EVENT(DIBVE_LBUTTONUP,(void*)(&pt));
  }
  CWnd::OnLButtonUp(nFlags, point);
}

void CDIBViewBase::OnRButtonDown(UINT nFlags, CPoint point) 
{
  if ((m_Selection) && (m_Selection->RectEnable))
  {
    m_Selection->RPressed=true;
    m_Selection->DrawLine=false;
    m_Selection->LPressed=false;
    m_Selection->p1=point;
    m_Selection->p2=point;
    Scr2Pic(m_Selection->p1);
    Scr2Pic(m_Selection->p2);
    m_Selection->DrawRect=true;        
  }
  else
  {
    CPoint pt=point;
    Scr2Pic(pt);
    DIB_EVENT(DIBVE_RBUTTONDOWN,(void*)(&pt));
  }
  CWnd::OnRButtonDown(nFlags, point);
}

void CDIBViewBase::OnRButtonUp(UINT nFlags, CPoint point) 
{
  if ((m_Selection) && (m_Selection->RectEnable) && m_Selection->RPressed)
  {
    m_Selection->p2=point;
    Scr2Pic(m_Selection->p2);
    m_Selection->DrawRect=(m_Selection->p2!=m_Selection->p1)!=0;
    if (m_Selection->DrawRect)
    {
      m_Selection->Object.left  = m_Selection->p1.x;
      m_Selection->Object.top   = m_Selection->p1.y;
      m_Selection->Object.right = m_Selection->p2.x;
      m_Selection->Object.bottom= m_Selection->p2.y;
      m_Selection->Object.NormalizeRect();
    }
    m_Selection->RPressed=FALSE;
    DIB_EVENT(DIBVE_RECTSELECTEVENT,(void*)(&m_Selection->Object));
    Invalidate(FALSE);
  }
  else
  {
    CPoint pt=point;
    Scr2Pic(pt);
    DIB_EVENT(DIBVE_RBUTTONUP,(void*)(&pt));
  }
  // Pass message to parent
  GetParent()->SendMessage(WM_RBUTTONUP,nFlags,MAKELPARAM(point.x,point.y));
  CWnd::OnRButtonUp(nFlags, point);
}

void CDIBViewBase::OnMouseMove(UINT nFlags, CPoint point) 
{
  if (!InFrame(point)) // Out of FOV
  {
    if (m_Selection)
    {                   // Stop following and drawing
      if (m_Selection->RPressed) 
      {
        m_Selection->RPressed=false; 
        if (m_Selection->DrawRect) 
          m_Selection->DrawRect=false; 
        Invalidate(); 
      }
      if (m_Selection->LPressed) 
      {
        m_Selection->LPressed=false; 
        if (m_Selection->DrawLine) 
          m_Selection->DrawLine=false; 
        Invalidate(); 
      }
    }
  }
  else
  {
    if (m_Selection)
    {
      CPoint pnt=point;
      Scr2Pic(pnt);
      if (m_Selection->LineEnable)
      {
        if (m_Selection->LPressed)
        {
          m_Selection->p2=pnt;
          Invalidate(FALSE);
        }
      }
      if (m_Selection->RectEnable)
      {
        if (m_Selection->RPressed)
        {
          m_Selection->p2=pnt;
          Invalidate(FALSE);
        }
      }
    }
    if ( GetFramePntr() )
    {
      DWORD dwButton = (nFlags & MK_RBUTTON) ? DIBVE_RBUTTONDOWN : 0 ;
      if ( nFlags & MK_LBUTTON )
        dwButton += DIBVE_LBUTTONDOWN ;
      if ( nFlags & MK_MBUTTON )
        dwButton += DIBVE_MBUTTONDOWN ;

      CPoint pt=point;
      Scr2Pic(pt);
      DIB_EVENT(DIBVE_MOUSEMOVEEVENT | dwButton , (void*)(&pt) ); 

      GetParent()->SendMessage(WM_MOUSEMOVE,nFlags,MAKELPARAM(point.x,point.y));
    }
  }
  CWnd::OnMouseMove(nFlags, point);
}

BOOL CDIBViewBase::OnNcActivate(BOOL bActive) 
{
  if (bActive && m_Selection)
  {
    switch (GetSelStyle())
    {
    case SEL_NOTHING:
      break;
    case SEL_LINE:
      {
        DIB_EVENT(DIBVE_LINESELECTEVENT,(void*)(&m_Selection->Object));
      }
      break;
    case SEL_RECT:
      {
        DIB_EVENT(DIBVE_RECTSELECTEVENT,(void*)(&m_Selection->Object));
      }
      break;
    }
  }
  return CWnd::OnNcActivate(bActive);
}

#pragma comment( lib, "Vfw32.lib" )

HGLOBAL CDIBViewBase::CopyHandle()
{
  if (m_Selection)
  {
    if ((
      (m_Selection->Object.left==0) && (m_Selection->Object.top==0) &&
      (m_Selection->Object.right ==m_Frame->lpBMIH->biWidth-1) &&
      (m_Selection->Object.bottom==m_Frame->lpBMIH->biHeight-1)
      ) || (!m_Selection->DrawRect))
    {
      HANDLE h = ICImageDecompress(NULL,0,(LPBITMAPINFO)m_Frame->lpBMIH,m_Frame->lpData,NULL);
      if (h)
      {
        return h;
      }
    }
    else
    {
      pTVFrame newFrame= _cut_rect(m_Frame,m_Selection->Object);
      HANDLE h = ICImageDecompress(NULL,0,(LPBITMAPINFO)newFrame->lpBMIH,newFrame->lpData,NULL);
      freeTVFrame(newFrame);
      return h;
    }
  }
  return NULL;
}

#ifdef _DEBUG
void CDIBViewBase::Invalidate(BOOL bErase)
{
  CWnd::Invalidate(bErase);
}
#endif


