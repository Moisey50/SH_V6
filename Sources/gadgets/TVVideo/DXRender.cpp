// VideoRenders.cpp: implementation of the CBaseRenders class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXRender.h"
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

void DXVideoRender_DibEvent(int Event, void *Data, void *pParam, CDIBViewBase* wParam)
{
    DXVideoRender* vrg=(DXVideoRender*)pParam;
    vrg->DibEvent(Event, Data);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(DXVideoRender, CRenderGadget, "Video.renderers", TVDB400_PLUGIN_NAME);

DXVideoRender::DXVideoRender():
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

void DXVideoRender::ShutDown()
{
    Detach();
    CRenderGadget::ShutDown();
    delete m_pInput; m_pInput=NULL;
    delete m_pOutput;m_pOutput=NULL;
}

void DXVideoRender::Attach(CWnd* pWnd)
{
    Detach();
    m_wndOutput=new CDXRender(m_Monitor);
    m_wndOutput->Create(pWnd);
    m_wndOutput->SetScale((m_Scale==0)?-1:m_Scale);
    m_wndOutput->SetMonochrome(m_Monochrome);
    m_wndOutput->InitSelBlock(m_LineSelection,m_RectSelection);
    m_wndOutput->SetCallback(::DXVideoRender_DibEvent,this);
}

void DXVideoRender::Detach()
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

void DXVideoRender::DibEvent(int Event, void *Data)
{
    if (Event&DIBVE_LBUTTONDOWN)
    {
        //if ( m_wndOutput->GetSelStyle() == SEL_NOTHING )
        {
            CAutoLockMutex al( m_wndOutput->m_LockOMutex , 1000
            #ifdef _DEBUG
              , "DXVideoRender::DibEvent"
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

void DXVideoRender::Render(const CDataFrame* pDataFrame)
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

bool DXVideoRender::PrintProperties(FXString& text)
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

bool DXVideoRender::ScanProperties(LPCTSTR text, bool& Invalidate)
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

bool DXVideoRender::ScanSettings(FXString& text)
{
    text="template(ComboBox(Scale(Fit_window(0),x1(1),x2(2),x4(4),x8(8),x16(16))),\
         ComboBox(Monochrome(False(false),True(true))),\
         ComboBox(LineSelection(False(false),True(true))),\
         ComboBox(RectSelection(False(false),True(true))),\
         )";
    return true;
}


