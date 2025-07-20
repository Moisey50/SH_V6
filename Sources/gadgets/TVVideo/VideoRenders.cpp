// VideoRenders.cpp: implementation of the CBaseRenders class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VideoRenders.h"
#include "TVVideo.h"
#include <Gadgets\gadbase.h>
#include <Gadgets\RectFrame.h>
#include <imageproc\simpleip.h>
#include <imageproc\imagebits.h>
#include <gadgets\TextFrame.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void DibEvent(int Event, void *Data, void *pParam, CDIBViewBase* wParam)
{
    VideoRender* vrg=(VideoRender*)pParam;
    vrg->DibEvent(Event, Data);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(VideoRender, CRenderGadget, "Video.renderers", TVDB400_PLUGIN_NAME);

VideoRender::VideoRender():
                    m_wndOutput(NULL),
                    m_Scale(DEFAULT_SCALE),
                    m_Monochrome(DEFAULT_MONOCHROME),
                    m_PointOfInterest(-1,-1),
                    m_LineSelection(false),
                    m_RectSelection(false)
{
    m_pInput      = new CInputConnector(vframe);
    m_pOutput     = new COutputConnector(text);
    Resume();
}

void VideoRender::ShutDown()
{
    Detach();
    CRenderGadget::ShutDown();
    delete m_pInput; m_pInput=NULL;
    delete m_pOutput;m_pOutput=NULL;
}

void VideoRender::Attach(CWnd* pWnd)
{
    Detach();
    m_wndOutput=new CDIBRender(m_Monitor);
    m_wndOutput->Create(pWnd);
    m_wndOutput->SetScale((m_Scale==0)?-1:m_Scale);
    m_wndOutput->SetMonochrome(m_Monochrome);
    m_wndOutput->InitSelBlock(m_LineSelection,m_RectSelection);
    m_wndOutput->SetCallback(::DibEvent,this);
}

void VideoRender::Detach()
{
    m_Lock.LockAndProcMsgs();
    if (::IsWindow(m_wndOutput->GetSafeHwnd()))
    {
        m_wndOutput->SetCallback(NULL,NULL);
        m_wndOutput->DestroyWindow();
    }
    if (m_wndOutput) delete m_wndOutput; m_wndOutput=NULL;
    m_Lock.Unlock();
}

void VideoRender::DibEvent(int Event, void *Data)
{
    if (Event&DIBVE_LBUTTONDOWN)
    {
        //if ( m_wndOutput->GetSelStyle() == SEL_NOTHING )
        {
        CAutoLockMutex al( m_wndOutput->m_LockOMutex , 1000
        #ifdef _DEBUG
          , "VideoRender::DibEvent"
        #endif
        ) ;
        pTVFrame ptv=m_wndOutput->GetFramePntr();
            if(!ptv) return;
            m_PointOfInterest=*(LPPOINT)Data;
            if ((m_PointOfInterest.x!=-1) && (m_PointOfInterest.y!=-1))
            {
                CString outS;
                int I=0,U=0,V=0;
                DWORD offset=m_PointOfInterest.y*ptv->lpBMIH->biWidth+m_PointOfInterest.x;
                if (offset<ptv->lpBMIH->biSizeImage)
                    __getdata_IUV(ptv,m_PointOfInterest.x,m_PointOfInterest.y,I,U,V);
                outS.Format("selected=true;x=%d;y=%d;I=%d;U=%d;V=%d;",m_PointOfInterest.x, m_PointOfInterest.y, I,U-128,V-128);
                CTextFrame* pDataFrame = CTextFrame::Create();
                pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
                pDataFrame->ChangeId(NOSYNC_FRAME);
                pDataFrame->SetString(outS);
                if (!m_pOutput->Put(pDataFrame))
                    pDataFrame->Release();
                m_PointOfInterest=CPoint(-1,-1);
            }
        }
    }
    else if (Event&DIBVE_LINESELECTEVENT)
    {
        CRect rc=*(RECT*)Data;
        rc.NormalizeRect() ;
        CString outS; 
        outS.Format("Line=%d,%d,%d,%d;",rc.left,rc.top,rc.right,rc.bottom);
        CTextFrame* pDataFrame = CTextFrame::Create();
        pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
        pDataFrame->ChangeId(NOSYNC_FRAME);
        pDataFrame->SetString(outS);
        if (!m_pOutput->Put(pDataFrame))
            pDataFrame->Release();

    }
    else if (Event&DIBVE_RECTSELECTEVENT)
    {
        CRect rc=*(RECT*)Data;
        rc.NormalizeRect() ;
        CString outS; 
        outS.Format("Rect=%d,%d,%d,%d;",rc.left,rc.top,rc.right,rc.bottom);
        CTextFrame* pDataFrame = CTextFrame::Create();
        pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
        pDataFrame->ChangeId(NOSYNC_FRAME);
        pDataFrame->SetString(outS);
        if (!m_pOutput->Put(pDataFrame))
            pDataFrame->Release();
    }
    else if ( (Event & DIBVE_LBUTTONUP)  &&  (m_wndOutput->GetSelStyle() == SEL_NOTHING) ) 
    {
        CPoint pnt=*(POINT*)Data;
        CString outS;
        //         outS.Format("LButtonUp=%d,%d;",pnt.x,pnt.y);
        outS.Format("selected=false;x=%d;y=%d;", pnt.x,pnt.y );
        CTextFrame* pDataFrame = CTextFrame::Create();
        pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
        pDataFrame->ChangeId(NOSYNC_FRAME);
        pDataFrame->SetString(outS);
        if (!m_pOutput->Put(pDataFrame))
            pDataFrame->Release();
    }
    else if ( (Event & DIBVE_RBUTTONUP) && (m_wndOutput->GetSelStyle() == SEL_NOTHING) ) 
    {
        CPoint pnt=*(POINT*)Data;
        CString outS; 
        outS.Format("selected=false;x=%d;y=%d;", pnt.x,pnt.y );
        CTextFrame* pDataFrame = CTextFrame::Create();
        pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
        pDataFrame->ChangeId(NOSYNC_FRAME);
        pDataFrame->SetString(outS);
        if (!m_pOutput->Put(pDataFrame))
            pDataFrame->Release();
    }
    //   if (Event&DIBVE_MOUSEMOVEEVENT) 
    //   {
    //     m_PointOfInterest=*(LPPOINT)Data;
    //     CString outS; 
    //     outS.Format("selected=false;x=%d;y=%d;", m_PointOfInterest.x,m_PointOfInterest.y );
    //     CTextFrame* pDataFrame = CTextFrame::Create();
    //     pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
    //     pDataFrame->ChangeId(NOSYNC_FRAME);
    //     pDataFrame->SetString(outS);
    //     if (!m_pOutput->Put(pDataFrame))
    //       pDataFrame->Release();
    //   }
}

void VideoRender::Render(const CDataFrame* pDataFrame)
{
    FXAutolock al(m_Lock);
    if ((m_wndOutput) && (::IsWindow(m_wndOutput->GetSafeHwnd())))
    {
        m_wndOutput->SetRedraw(FALSE);
        int x,y;
        if ( (pDataFrame) && (pDataFrame->Attributes()->GetInt("x",x)) && (pDataFrame->Attributes()->GetInt("y",y)) )
        {
            m_wndOutput->ShiftPos(x, y);
        }
        else
            m_wndOutput->ShiftPos(-1, -1);
        m_wndOutput->Render(pDataFrame);
        m_wndOutput->SetRedraw(TRUE);
        m_wndOutput->Invalidate(FALSE);
    }
}

//////////////////////////////////////////////////////////////////////

bool VideoRender::PrintProperties(FXString& text)
{
    CRenderGadget::PrintProperties(text);
    FXPropertyKit pc;
    pc.WriteBool("Monochrome", m_Monochrome);
    pc.WriteInt ("Scale",       m_Scale);
    pc.WriteBool("LineSelection",       m_LineSelection);
    pc.WriteBool("RectSelection",       m_RectSelection);
    text+=pc;
    return true;
}

bool VideoRender::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CString tmpS;
    CRenderGadget::ScanProperties(text,Invalidate);
    FXPropertyKit pc(text);
    pc.GetInt( "Scale",     m_Scale);  
    pc.GetBool("Monochrome",m_Monochrome);
    bool bVal;
    if (pc.GetBool("LineSelection", bVal) && (bVal!=m_LineSelection))
    {
        m_LineSelection=bVal;
    }
    if (pc.GetBool("RectSelection", bVal) && (bVal!=m_RectSelection))
    {
        m_RectSelection=bVal;
    }
    if (m_wndOutput)
    {
        m_wndOutput->SetScale((m_Scale==0)?-1:m_Scale);
        m_wndOutput->SetMonochrome(m_Monochrome); 
        m_wndOutput->InitSelBlock(m_LineSelection,m_RectSelection);
    }
    return true;
}

bool VideoRender::ScanSettings(FXString& text)
{
    text="template(ComboBox(Scale(Fit_window(0),x1(1),x2(2),x4(4),x8(8),x16(16))),\
         ComboBox(Monochrome(False(false),True(true))),\
         ComboBox(LineSelection(False(false),True(true))),\
         ComboBox(RectSelection(False(false),True(true))),\
         )";
    return true;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(Render3DPlot, CRenderGadget, "Video.renderers", TVDB400_PLUGIN_NAME);

Render3DPlot::Render3DPlot():
                    m_wndOutput(NULL),
                    m_Scale(DEFAULT_SCALE),
                    m_Monochrome(DEFAULT_MONOCHROME),
                    m_PointOfInterest(-1,-1),
                    m_LineSelection(false),
                    m_RectSelection(false),
                    m_alpha(15),
                    m_beta(15),
                    m_rotZ(0),
                    m_grid(30) , 
                    m_iViewLow(0) , 
                    m_iViewHigh(255)
{
    m_pInput = new CInputConnector(vframe);
    m_pOutput = new COutputConnector(text);
    m_pControl    = new CDuplexConnector(this, transparent, transparent);
    Resume();
}

void Render3DPlot::ShutDown()
{
    Detach();
    CRenderGadget::ShutDown();
    delete m_pInput; m_pInput=NULL;
    delete m_pOutput;m_pOutput=NULL;
    delete m_pControl; m_pControl = NULL;

    if (m_wndOutput)
        m_wndOutput->DestroyWindow();
    delete m_wndOutput; m_wndOutput=NULL;
}

void Render3DPlot::Attach(CWnd* pWnd)
{
    Detach();
    m_wndOutput=new CDIBRender3D(m_Monitor);
    m_wndOutput->Create(pWnd);
    m_wndOutput->SetScale((m_Scale==0)?-1:m_Scale);
    m_wndOutput->SetMonochrome(false);
    //m_wndOutput->SetParams(&m_alpha, &m_beta, &m_grid, &m_bGray);
    m_wndOutput->InitSelBlock(m_LineSelection,m_RectSelection);
    m_wndOutput->SetCallback(::DibEvent,this);
}

void Render3DPlot::Detach()
{
    m_Lock.LockAndProcMsgs();
    if (::IsWindow(m_wndOutput->GetSafeHwnd()))
    {
        m_wndOutput->DestroyWindow();
    }
    if (m_wndOutput) delete m_wndOutput; m_wndOutput=NULL;
    m_Lock.Unlock();
}

void Render3DPlot::DibEvent(int Event, void *Data)
{
    if (Event&DIBVE_LBUTTONDOWN)
    {
        //if ( m_wndOutput->GetSelStyle() == SEL_NOTHING )
        {
          CAutoLockMutex al( m_wndOutput->m_LockOMutex , 1000
          #ifdef _DEBUG
            , "Render3DPlot::DibEvent"
          #endif
          ) ;
          pTVFrame ptv=m_wndOutput->GetFramePntr();
            if(!ptv) return;
            m_PointOfInterest=*(LPPOINT)Data;
            if ((m_PointOfInterest.x!=-1) && (m_PointOfInterest.y!=-1))
            {
                CString outS;
                int I=0,U=0,V=0;
                DWORD offset=m_PointOfInterest.y*ptv->lpBMIH->biWidth+m_PointOfInterest.x;
                if (offset<ptv->lpBMIH->biSizeImage)
                    __getdata_IUV(ptv,m_PointOfInterest.x,m_PointOfInterest.y,I,U,V);

                outS.Format("selected=true;x=%d;y=%d;I=%d;U=%d;V=%d;", 
                    m_PointOfInterest.x, m_PointOfInterest.y, I,U-128,V-128);

                // Moisey's strange addon, I fix it a bit
                HWND hParent = m_wndOutput->GetSafeHwnd() ;
                HWND hParent2 = GetParent( hParent ) ;
                while ( hParent2 )
                {
                    hParent = hParent2 ;
                    hParent2 = GetParent( hParent ) ;
                }
                if ( hParent )
                {
                    CString name;
                    CWnd::FromHandle(hParent)->GetWindowText(name);
                    int pos;
                    if ((pos=name.Find("-"))>=0)
                    {
                        name=name.Left(pos);
                    }
                    name+=CString("- ")+outS;
                    SetWindowText( hParent , name ) ;
                }
                //end of Moisey's addon

                CTextFrame* pDataFrame = CTextFrame::Create();
                pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
                pDataFrame->ChangeId(NOSYNC_FRAME);
                pDataFrame->SetString(outS);
                if (!m_pOutput->Put(pDataFrame))
                    pDataFrame->Release();
                m_PointOfInterest=CPoint(-1,-1);
            }
        }
    }
    else if (Event&DIBVE_LINESELECTEVENT)
    {
        CRect rc=*(RECT*)Data;
        rc.NormalizeRect() ;
        CString outS; 
        outS.Format("Line=%d,%d,%d,%d;",rc.left,rc.top,rc.right,rc.bottom);
        CTextFrame* pDataFrame = CTextFrame::Create();
        pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
        pDataFrame->ChangeId(NOSYNC_FRAME);
        pDataFrame->SetString(outS);
        if (!m_pOutput->Put(pDataFrame))
            pDataFrame->Release();

    }
    else if (Event&DIBVE_RECTSELECTEVENT)
    {
        CRect rc=*(RECT*)Data;
        rc.NormalizeRect() ;
        CString outS; 
        outS.Format("Rect=%d,%d,%d,%d;",rc.left,rc.top,rc.right,rc.bottom);
        CTextFrame* pDataFrame = CTextFrame::Create();
        pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
        pDataFrame->ChangeId(NOSYNC_FRAME);
        pDataFrame->SetString(outS);
        if (!m_pOutput->Put(pDataFrame))
            pDataFrame->Release();
    }
    else if ( (Event & DIBVE_LBUTTONUP)  &&  (m_wndOutput->GetSelStyle() == SEL_NOTHING) ) 
    {
        CPoint pnt=*(POINT*)Data;
        CString outS;
        //         outS.Format("LButtonUp=%d,%d;",pnt.x,pnt.y);
        outS.Format("selected=false;x=%d;y=%d;", pnt.x,pnt.y );
        CTextFrame* pDataFrame = CTextFrame::Create();
        pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
        pDataFrame->ChangeId(NOSYNC_FRAME);
        pDataFrame->SetString(outS);
        if (!m_pOutput->Put(pDataFrame))
            pDataFrame->Release();
    }
    else if ( (Event & DIBVE_RBUTTONUP) && (m_wndOutput->GetSelStyle() == SEL_NOTHING) ) 
    {
        CPoint pnt=*(POINT*)Data;
        CString outS; 
        outS.Format("selected=false;x=%d;y=%d;", pnt.x,pnt.y );
        CTextFrame* pDataFrame = CTextFrame::Create();
        pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
        pDataFrame->ChangeId(NOSYNC_FRAME);
        pDataFrame->SetString(outS);
        if (!m_pOutput->Put(pDataFrame))
            pDataFrame->Release();
    }
    //   if (Event&DIBVE_MOUSEMOVEEVENT) 
    //   {
    //     m_PointOfInterest=*(LPPOINT)Data;
    //     CString outS; 
    //     outS.Format("selected=false;x=%d;y=%d;", m_PointOfInterest.x,m_PointOfInterest.y );
    //     CTextFrame* pDataFrame = CTextFrame::Create();
    //     pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
    //     pDataFrame->ChangeId(NOSYNC_FRAME);
    //     pDataFrame->SetString(outS);
    //     if (!m_pOutput->Put(pDataFrame))
    //       pDataFrame->Release();
    //   }
}

void Render3DPlot::Render(const CDataFrame* pDataFrame)
{
    FXAutolock al(m_Lock);
    if ((m_wndOutput) && (::IsWindow(m_wndOutput->GetSafeHwnd())))
    {
        int x,y;
        if ( (pDataFrame) && (pDataFrame->Attributes()->GetInt("x",x)) && (pDataFrame->Attributes()->GetInt("y",y)) )
        {
            m_wndOutput->ShiftPos(x, y);
        }
        else
            m_wndOutput->ShiftPos(-1, -1);
        m_wndOutput->Render(pDataFrame);
    }
}

//////////////////////////////////////////////////////////////////////

bool Render3DPlot::PrintProperties(FXString& text)
{
    CRenderGadget::PrintProperties(text);
    FXPropertyKit pc;
    pc.WriteBool("Monochrome", m_Monochrome);
    pc.WriteInt ("Scale",       m_Scale);
    pc.WriteBool("LineSelection",       m_LineSelection);
    pc.WriteBool("RectSelection",       m_RectSelection);
    pc.WriteInt("Alpha", m_alpha);
    pc.WriteInt("Beta", m_beta);
    pc.WriteInt("Grid", m_grid);
    pc.WriteInt("RotZ", m_rotZ);
    FXString ViewRange ;
    ViewRange.Format( "%d,%d" , m_iViewLow , m_iViewHigh ) ;
    pc.WriteString( "ViewRange" , ViewRange ) ;
    text+=pc;
    return true;
}

bool Render3DPlot::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CString tmpS;
    CRenderGadget::ScanProperties(text,Invalidate);
    FXPropertyKit pc(text);
    pc.GetInt( "Scale",     m_Scale);  
    pc.GetBool("Monochrome",m_Monochrome);
    bool bVal;
    pc.GetBool("LineSelection", bVal); 
    if (bVal!=m_LineSelection)
    {
        m_LineSelection=bVal;
    }
    pc.GetBool("RectSelection", bVal);
    if (bVal!=m_RectSelection)
    {
        m_RectSelection=bVal;
    }
    pc.GetInt("Alpha", m_alpha);
    pc.GetInt("Beta", m_beta);
    pc.GetInt("Grid", m_grid);
    pc.GetInt("RotZ", m_rotZ);
    FXString ViewRange ;
    if ( pc.GetString( "ViewRange" , ViewRange ) )
    {
      int iLow , iHigh ;
      if ( sscanf(( LPCTSTR )ViewRange , "%d,%d" , &iLow , &iHigh ) == 2 )
      {
        if ( iLow >= -1  &&  iLow < 255 
          && iHigh > iLow && iLow < iHigh )
        {
          m_iViewLow = iLow ;
          m_iViewHigh = iHigh ;
        }
      }
    }
    

    if (m_wndOutput)
    {
        m_wndOutput->SetScale((m_Scale==0)?-1:m_Scale);
        //m_wndOutput->SetMonochrome(m_Monochrome); 
        m_wndOutput->InitSelBlock(m_LineSelection,m_RectSelection);
        m_wndOutput->SetParams(&m_alpha, &m_beta, &m_grid, &m_Monochrome, 
          &m_rotZ, &m_iViewLow , &m_iViewHigh );
    }
    return true;
}

bool Render3DPlot::ScanSettings(FXString& text)
{
    text="template(ComboBox(Scale(Fit_window(0),x1(1),x2(2),x4(3))),\
         ComboBox(Monochrome(False(false),True(true))),\
         ComboBox(LineSelection(False(false),True(true))),\
         ComboBox(RectSelection(False(false),True(true))),\
         Spin(Alpha,1,89),\
         Spin(Beta,1,89),\
         Spin(Grid,10,50),\
         Spin(RotZ,-360,360),\
         EditBox(ViewRange)\
         )";
    return true;
}

void  Render3DPlot::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
    if (!pParamFrame) return;

    CTextFrame* TextFrame = pParamFrame->GetTextFrame(DEFAULT_LABEL);
    if (TextFrame)
    {
        if (TextFrame->GetString().GetLength()==0)
        {
            FXString tmpS;
            PrintProperties(tmpS);
            CTextFrame* retV=CTextFrame::Create(tmpS);
            retV->ChangeId(NOSYNC_FRAME);
            if (!m_pControl->Put(retV))
                retV->RELEASE(retV);
        }
        else
        {
            CString pk=TextFrame->GetString();
            bool Invalidate;
            ScanProperties(pk,Invalidate);
        }
    }
    pParamFrame->RELEASE(pParamFrame);
}
