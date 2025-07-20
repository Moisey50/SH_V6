// DebugRender.h: interface for the CDebugRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUGRENDER_H__D354ECBB_B949_4635_86BB_C7C28F753F54__INCLUDED_)
#define AFX_DEBUGRENDER_H__D354ECBB_B949_4635_86BB_C7C28F753F54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DebugViewDlg.h"
#include <gadgets\shkernel.h>
#include <gadgets\TextFrame.h>
#include <gadgets\QuantityFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\FigureFrame.h>
#include <helpers\FXParser2.h>

extern LPCTSTR debug_monitor;

class CDebugRender : public CRenderGadget
{
    CTermWindow *m_Terminal;
    RenderCallBack m_rcb;
    void*          m_wParam;
    CDebugViewDlg* m_DView;
public:
	virtual void    ShutDown();
	virtual void    Create();
    virtual void    Attach(CWnd* pWnd);
    virtual void    Detach();
    bool            SetCallBack(RenderCallBack rcb, void* cbData) { m_rcb=rcb; m_wParam=cbData; return true; }
    CWnd*           GetRenderWnd() { return m_DView; }
    void            GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=320; rc.bottom=240; }
private:
	CDebugRender();
	virtual void Render(const CDataFrame* pDataFrame);

	DECLARE_RUNTIME_GADGET(CDebugRender);
};

__forceinline CDebugRender::CDebugRender(): 
    m_Terminal(NULL),
    m_rcb(NULL),
    m_wParam(NULL),
    m_DView(NULL)
{
    m_Monitor=(char*)debug_monitor;
	m_pInput = new CInputConnector(transparent);
	Resume();
}

__forceinline void CDebugRender::ShutDown()
{
    m_Monitor=NULL;
    if (m_Terminal)
    {
        m_Terminal->DestroyWindow();
        delete m_Terminal;
        m_Terminal=NULL;
    }
    CRenderGadget::ShutDown();
	delete m_pInput;   m_pInput = NULL;
}

__forceinline void CDebugRender::Render(const CDataFrame* pDataFrame)
{
    if (m_DView)
        m_DView->Render(pDataFrame);
    return;
    if (m_rcb)
    {
        m_rcb(pDataFrame,m_wParam);
        return;
    }
	if ((!m_Terminal) || (!m_Terminal->IsValid())) return;
    if (!Tvdb400_IsEOS(pDataFrame)) //Do net render EOS
    {
        CString outp; 
		if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(), text))
		{
			const CTextFrame* TextFrame = pDataFrame->GetTextFrame(DEFAULT_LABEL);
			if (TextFrame)
      {
        FXString ViewText ;
        ConvBinStringToView( TextFrame->GetString() , ViewText ) ;
        outp.Format( "text:\t%d\t%s\r\n" , pDataFrame->GetId() , ViewText );
      }
		}
		else if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(), quantity))
		{
			CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(quantity);
			if (Iterator)
			{
				CQuantityFrame* QuantityFrame = (CQuantityFrame*)Iterator->Next(DEFAULT_LABEL);
				
                CString values;
				while (QuantityFrame)
				{
					values += QuantityFrame->ToString() + _T("; ");
					QuantityFrame = (CQuantityFrame*)Iterator->Next(DEFAULT_LABEL);
				}
				delete Iterator;
				outp.Format("quantity:\t%d\t%s\r\n", pDataFrame->GetId(), values);
			}
			else
			{
				const CQuantityFrame* QuantityFrame = pDataFrame->GetQuantityFrame(DEFAULT_LABEL);
				if (QuantityFrame)
					outp.Format("quantity:\t%d\t%s\r\n",pDataFrame->GetId(), QuantityFrame->ToString());
			}
		}
		else if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(), figure))
		{
			CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(figure);
			if (Iterator)
			{
				CFigureFrame* FigureFrame = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
				
                CString values;
				while (FigureFrame)
				{
					values += FigureFrame ->ToString() + _T("; ");
					FigureFrame  = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
				}
				delete Iterator;
				outp.Format("figure:\t%d\t%s\r\n", pDataFrame->GetId(), values);
			}
			else
			{
				const CFigureFrame * FigureFrame  = pDataFrame->GetFigureFrame(DEFAULT_LABEL);
				if (FigureFrame)
					outp.Format("quantity:\t%d\t%s\r\n",pDataFrame->GetId(), FigureFrame->ToString());
			}
            delete Iterator;
		}
		else if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(), logical))
		{
			const CBooleanFrame* BooleanFrame = pDataFrame->GetBooleanFrame(DEFAULT_LABEL);
			outp.Format("logical:\t%d\t%s\r\n",pDataFrame->GetId(), (BooleanFrame->operator bool() ? "true" : "false"));
		}
		else if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(),rectangle))
        {
			const CRectFrame* RectFrame = pDataFrame->GetRectFrame(DEFAULT_LABEL);
			if (RectFrame)
			{
				CRect rc=(LPRECT)RectFrame;
				outp.Format("rectangle:\t%d\t(%d,%d,%d,%d)\r\n",pDataFrame->GetId(),rc.left,rc.top,rc.right,rc.bottom);
			}
        }
        else
			outp.Format("%d\ttype: %s\r\n",pDataFrame->GetId(),	Tvdb400_TypeToStr(pDataFrame->GetDataType()));
	    m_Terminal->AppendText(outp); 
    }
}

__forceinline void CDebugRender::Create()
{
    CRenderGadget::Create();
}

__forceinline void CDebugRender::Attach(CWnd* pWnd)
{
    Detach();
    m_DView=(CDebugViewDlg*)pWnd;
}

__forceinline void CDebugRender::Detach()
{
    m_DView=NULL;
}

#endif // !defined(AFX_DEBUGRENDER_H__D354ECBB_B949_4635_86BB_C7C28F753F54__INCLUDED_)
