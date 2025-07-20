#include "StdAfx.h"
#include "CSlideView.h"
#include <imageproc\resample.h>

__forceinline LPBITMAPINFOHEADER __createLPBMIH(int width, int height)
{
  int isize=3*(width*height);

  LPBITMAPINFOHEADER lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+isize);
  memset(lpBMIH,0,sizeof(BITMAPINFOHEADER)+isize);
  lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
  lpBMIH->biWidth=width;
  lpBMIH->biHeight=height;
  lpBMIH->biPlanes=1;
  lpBMIH->biBitCount=24;
  lpBMIH->biCompression=0;
  lpBMIH->biSizeImage=0;
  lpBMIH->biXPelsPerMeter=11800;
  lpBMIH->biYPelsPerMeter=11800;
  lpBMIH->biClrUsed=0;
  lpBMIH->biClrImportant = 0;
  return lpBMIH;
}

__forceinline void *memcpy_8_24(void *dest, const void *src, size_t count)
{
  LPBYTE Dest = (LPBYTE)dest;
  LPBYTE Src  = (LPBYTE)src;
  while(count)
  {
    *Dest=*Src; Dest++;
    *Dest=*Src; Dest++;
    *Dest=*Src; Dest++;
    Src++;
    count--;
  }
  return Dest;
}

void CSlideView_DipPPEvent(int Event, void *Data, void *pParam, CDIBViewBase* wParam)
{
  ((CSlideView*)pParam)->DipPPEvent(Event, Data);
}

bool CSlideView_DrawExFunc(HDC hdc, RECT& rc, CDIBViewBase* view, LPVOID lParam)
{
  return ((CSlideView*)lParam)->DrawExFunc(hdc,rc);
}

/////////////////////////////////////////////////////////////////////////////
CSlideView::CSlideView(LPCTSTR name):
  CDIBView(name),
  m_OrgWidth(-1), 
  m_OrgHeight(-1),
  m_TargetWidth(200),
  m_TargetHeight(0),
  m_FramesLen(16),
  m_FramesInRow(4),
  m_Rows(4),
  m_Cntr(0),
  m_SelectedItem(-1),
  m_Rescale(true),
  m_Shift(false) ,
  m_bResetData( false )
{
  SetCallback(CSlideView_DipPPEvent,this);
  SetDrawExFunc(CSlideView_DrawExFunc,this);
}

CSlideView::~CSlideView(void)
{
}

bool CSlideView::PrepareData(const pTVFrame frame, DWORD dwTimeOut)
{
  CAutoLockMutex al( m_LockOMutex , 1000
  #ifdef _DEBUG
    , "CSlideView::PrepareData"
  #endif
  ) ;
  if ( !al.IsLocked() ) 
  {
//     TRACE("CDIBViewBase::PrepareData skips frame\n");
    return false; 
  } 
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
    m_SelectedItem=-1;
    m_SizeChanged=true;
    return false;
  }

  m_SizeChanged |= ((frame->lpBMIH->biWidth!=m_OrgWidth) || (frame->lpBMIH->biHeight!=m_OrgHeight));
  if (m_SizeChanged)
    OnBufferReset();
  if (m_Rescale)
  {
    m_TargetHeight=m_TargetWidth*GetHeight(frame)/GetWidth(frame);
  }
  else
  {
    m_TargetWidth  = GetWidth(frame);
    m_TargetHeight = GetHeight(frame);
  }
  LPBITMAPINFOHEADER srcH=NULL;
  {
    pTVFrame Frame=makecopyTVFrame(frame);
    if (_resample(Frame,m_TargetWidth,m_TargetHeight))
    {
      srcH=(LPBITMAPINFOHEADER) PrepareImage(Frame);
      freeTVFrame(Frame);
    }
    else
    {
      TRACE("!!!!! Error! _resample fails\n");
      freeTVFrame(Frame);
      return false;
    }

  }
  ASSERT(srcH!=NULL);

  if (!m_pConvertedBMIH)
  {
    m_Rows  = (m_FramesLen/m_FramesInRow+((m_FramesLen%m_FramesInRow)?1:0));
    m_pConvertedBMIH=__createLPBMIH(m_TargetWidth*m_FramesInRow,m_TargetHeight*m_Rows);
    m_OrgWidth=frame->lpBMIH->biWidth; 
    m_OrgHeight=frame->lpBMIH->biHeight;
    m_Width=((LPBITMAPINFOHEADER)m_pConvertedBMIH)->biWidth;
    m_Height=((LPBITMAPINFOHEADER)m_pConvertedBMIH)->biHeight;
  }
  if (m_Shift)
    SetShift((LPBITMAPINFOHEADER)m_pConvertedBMIH, srcH);
  else
    SetCicle((LPBITMAPINFOHEADER)m_pConvertedBMIH, srcH);
  free(srcH);
  m_Cntr++;
  m_Cntr=m_Cntr%m_FramesLen;
  return true;
}

bool CSlideView::CopyFrame(int from, int to, LPBYTE data)
{
  LPBYTE src, dst;
  src=data+(m_Rows-1-from/m_FramesInRow)*m_Width*m_TargetHeight*3+((from%m_FramesInRow)*m_TargetWidth*3);
  dst=data+(m_Rows-1-to/m_FramesInRow)*m_Width*m_TargetHeight*3+((to%m_FramesInRow)*m_TargetWidth*3);
  for (int y=0; y<m_TargetHeight; y++)
  {
    memcpy(dst,src,m_TargetWidth*3);
    dst+=m_Width*3;
    src+=m_Width*3;
  }
  return true;
}

bool CSlideView::SetShift(LPBITMAPINFOHEADER dst, LPBITMAPINFOHEADER src)
{
  LPBYTE lpDst=(LPBYTE)dst+dst->biSize;

  for (int i=m_FramesLen-2; i>=0; i--)
  {
    CopyFrame(i, i+1, lpDst);
  }
  lpDst+=(m_Rows-1)*m_Width*m_TargetHeight*3;
  LPBYTE lpSrc=(LPBYTE)src+src->biSize+GetPaletteSize(src);
  if (src->biBitCount==24)
  {
    for (int y=0; y<src->biHeight; y++)
    {
      memcpy(lpDst,lpSrc,src->biWidth*3);
      lpDst+=m_Width*3;
      lpSrc+=src->biWidth*3;
    }
  }
  else if (src->biBitCount==8)
  {
    for (int y=0; y<src->biHeight; y++)
    {
      memcpy_8_24(lpDst,lpSrc,src->biWidth);
      lpDst+=m_Width*3;
      lpSrc+=src->biWidth;
    }
  }
  return true;
}

bool CSlideView::SetCicle(LPBITMAPINFOHEADER dst, LPBITMAPINFOHEADER src)
{

  LPBYTE lpDst=(LPBYTE)dst+dst->biSize;
  lpDst+=(m_Rows-1-m_Cntr/m_FramesInRow)*m_Width*m_TargetHeight*3+((m_Cntr%m_FramesInRow)*m_TargetWidth*3);
  LPBYTE lpSrc=(LPBYTE)src+src->biSize+GetPaletteSize(src);

  if (src->biBitCount==24)
  {
    for (int y=0; y<src->biHeight; y++)
    {
      memcpy(lpDst,lpSrc,src->biWidth*3);
      lpDst+=m_Width*3;
      lpSrc+=src->biWidth*3;
    }
  }
  else if (src->biBitCount==8)
  {
    for (int y=0; y<src->biHeight; y++)
    {
      memcpy_8_24(lpDst,lpSrc,src->biWidth);
      lpDst+=m_Width*3;
      lpSrc+=src->biWidth;
    }
  }
  return true;
}

void CSlideView::OnBufferReset()
{
  m_SelectedItem=-1;
  if (m_pConvertedBMIH)
  {
    free(m_pConvertedBMIH);
    m_pConvertedBMIH=NULL;
    m_Cntr=0;
  }
}

void CSlideView::OnItemSelected()
{
}

void CSlideView::DipPPEvent(int Event, void *Data)
{
  if ((Event==DIBVE_LBUTTONDOWN) && (m_pConvertedBMIH) && (m_TargetWidth) && (m_TargetHeight))
  {
    POINT pt=*(POINT*)Data;
    CRect rc(0,0,((LPBITMAPINFOHEADER)m_pConvertedBMIH)->biWidth,((LPBITMAPINFOHEADER)m_pConvertedBMIH)->biHeight);
    if (rc.PtInRect(pt))
    {
      pt.x/=m_TargetWidth;
      pt.y/=m_TargetHeight;
      m_SelectedItem=pt.x+pt.y*m_FramesInRow;
      OnItemSelected();
      Invalidate();
    }
  }
}

bool    CSlideView::DrawExFunc(HDC hdc, RECT& rc)
{
  if (m_SelectedItem>=0)
  {
    CPen RectanglePen(PS_SOLID,1,RGB(255,0,0));
    HGDIOBJ oP=NULL;

    CPoint tl,br;
    tl.x=(m_SelectedItem%m_FramesInRow)*m_TargetWidth;
    tl.y=(m_SelectedItem/m_FramesInRow)*m_TargetHeight;
    br.x=tl.x+m_TargetWidth-1;
    br.y=tl.y+m_TargetHeight-1;
    Pic2Scr(tl);
    Pic2Scr(br);

    CPoint pl[5];
    pl[0]=tl;
    pl[1]=CPoint(br.x, tl.y);
    pl[2]=br;
    pl[3]=CPoint(tl.x, br.y);
    pl[4]=tl;
    oP=::SelectObject(hdc,RectanglePen);
    ::Polyline(hdc,pl,5);
    ::SelectObject(hdc,oP);
  }
  return true;
}