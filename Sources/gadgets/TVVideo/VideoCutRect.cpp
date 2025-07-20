#include "stdafx.h"
#include "TVVideo.h"
#include "VideoCutRect.h"
#include <gadgets\RectFrame.h>
#include <imageproc\cut.h>
#include <gadgets\ContainerFrame.h>
#include "VideoCutRectDialog.h"

#define THIS_MODULENAME "VideoCutRect"

// inline DWORD GetCompression(const pTVFrame fr )
// {
//   if ( fr->lpBMIH )
//     return fr->lpBMIH->biCompression ;
//   else
//     return 0xffffffff ;
// }
// 

// VideoCutRect
IMPLEMENT_RUNTIME_GADGET_EX(VideoCutRect, CControledFilter,  "Video.cut", TVDB400_PLUGIN_NAME);

VideoCutRect::VideoCutRect():
  CControledFilter(vframe * rectangle,vframe * rectangle,rectangle)
{
  m_Rect = CRect( 0 , 0 , 100 , 80 ) ;
  m_bAllowROIControlFromVideoInput = FALSE ;
  m_b4PixelsStep = TRUE ;
  m_LastInputRect = CRect(0,0,0,0) ;
  m_SetupObject = new VideoCutRectDialog(this, NULL);
  NeverSync(true);
}

void VideoCutRect::ShutDown()
{
  CControledFilter::ShutDown();
}

CDataFrame* VideoCutRect::DoProcessing(const CDataFrame* pDataFrame, const CDataFrame* pParamFrame)
{
  const CVideoFrame* VideoFrame = NULL;
  if (pDataFrame)
    VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
  const CRectFrame* rf = NULL;
  const CTextFrame* tf = NULL;
  if (pParamFrame)
  {
    rf = pParamFrame->GetRectFrame(DEFAULT_LABEL);
    if (!rf) 
      tf = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  }
  if((!rf) && (!tf) && (pDataFrame && m_bAllowROIControlFromVideoInput) )
    rf = pDataFrame->GetRectFrame(DEFAULT_LABEL);
  if (IsSetupOn())
    ((VideoCutRectDialog*)m_SetupObject)->LoadFrame(VideoFrame);
  CRect rc( m_Rect );
  if (rf)
    rc = rf ;
  else if (tf)
  {
    FXPropertyKit pk=tf->GetString();
    if ((pk.CompareNoCase(_T("?"))==0) || (pk.CompareNoCase(_T("help"))==0))
    {
      CTextFrame* retV=CTextFrame::Create(_T("Control pin can accept data in PropertyKit format.\r\nPossible variables are:\r\n\r\nROI,Rect - defines new rectangle\r\nCenter - shifts the rectanle to new position\r\n"));
      retV->ChangeId(NOSYNC_FRAME);
      if (!m_pControl->Put(retV))
        retV->RELEASE(retV);
      return NULL;
    }
    FXString res;
    if (pk.GetString("Rect",res) || pk.GetString("ROI",res)) // Rect=
    {
      if (sscanf(res,"%d,%d,%d,%d",&rc.left, &rc.top, &rc.right, &rc.bottom)!=4)
        return NULL;

    }
    else if (pk.GetString("Center",res)) // ROI=
    {
      CPoint CenterPt ;
      if (sscanf(res,"%d,%d", &CenterPt.x , &CenterPt.y )!=2)
        return NULL;
      CPoint CurrentCent = rc.CenterPoint() ;
      rc.OffsetRect( CenterPt - CurrentCent ) ;
      if ( m_LastInputRect.Width() )
      {
        rc.IntersectRect( m_LastInputRect , rc ) ;
      }
    }
    else
    {
      CTextFrame* retV=CTextFrame::Create(_T("Error"));
      retV->ChangeId(NOSYNC_FRAME);
      if (!m_pControl->Put(retV))
        retV->RELEASE(retV);
      return NULL;
    }
  }
  if ( rf || tf )
  {
    int iWidthError = rc.Width() & 0x3 ;
    switch ( iWidthError )
    {
    case 0 : break;
    case 1 : rc.right-- ; break ;
    case 2 : rc.right++ ;
      rc.left-- ;
      break ;
    case 3: rc.right++ ; break ;
    }
    m_Rect=rc;
  }

  if (m_Rect.IsRectEmpty())
    return NULL;


  if (!VideoFrame)
    return NULL;
  //TRACE("+++ cut_rect1(%d,%d,%d,%d)\n",rc.left,rc.top,rc.right,rc.bottom);
  m_LastInputRect.right = GetWidth( VideoFrame ) ;
  m_LastInputRect.bottom = GetHeight( VideoFrame ) ;

  BOOL bStep1IsPossible = (GetCompression( VideoFrame ) == BI_Y8) 
    || (GetCompression( VideoFrame ) == BI_Y16)
    || (GetCompression( VideoFrame ) == BI_Y800) ;
  pTVFrame res = _cut_rect(VideoFrame, rc , m_b4PixelsStep || !bStep1IsPossible );
  if (!res)
  {
    return NULL;
  }
  //TRACE("+++ cut_rect2(%d,%d,%d,%d)\n",rc.left,rc.top,rc.right,rc.bottom);
  m_Rect=rc;
  CContainerFrame* retVal = CContainerFrame::Create();
      retVal->CopyAttributes(pDataFrame);;

  CVideoFrame* cutVal = CVideoFrame::Create(res);
  cutVal->CopyAttributes(pDataFrame);
  retVal->AddFrame(cutVal);

  CRectFrame* rectFrame = CRectFrame::Create(&rc);
  rectFrame->CopyAttributes(pDataFrame);
  retVal->AddFrame(rectFrame);

  return retVal;
}

bool VideoCutRect::PrintProperties(FXString& text)
{
  FXPropertyKit pc;
  pc.WriteInt("Offset.x",m_Rect.left);
  pc.WriteInt("Offset.y",m_Rect.top); 
  pc.WriteInt("Width",m_Rect.right - m_Rect.left);
  pc.WriteInt("Height",m_Rect.bottom - m_Rect.top);
  pc.WriteInt("4PixStep",m_b4PixelsStep);
  pc.WriteInt("AllowROIInput" , m_bAllowROIControlFromVideoInput ) ;
  text=pc;
  return true;
}

bool VideoCutRect::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pc(text);
  RECT rc;
  bool res=true;
  res&=pc.GetInt("Offset.x",(int&)rc.left);
  res&=pc.GetInt("Offset.y",(int&)rc.top);
  res&=pc.GetInt("Width",(int&)rc.right);
  res&=pc.GetInt("Height",(int&)rc.bottom);
  if (res)
  {
    rc.right+=rc.left;
    rc.bottom+=rc.top;
    m_Rect=rc;
  }
  pc.GetInt("4PixStep",m_b4PixelsStep);
  pc.GetInt("AllowROIInput" , m_bAllowROIControlFromVideoInput ) ;
  return true;
}

bool VideoCutRect::ScanSettings(FXString& text)
{
  text="calldialog(true)";
  return true;
}
