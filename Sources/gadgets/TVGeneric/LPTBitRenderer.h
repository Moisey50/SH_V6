// LPTBitRenderer.h: interface for the LPTBitRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LPTBITRENDERER_H__CDF01DF9_57B9_4621_88BD_CB374B385E1C__INCLUDED_)
#define AFX_LPTBITRENDERER_H__CDF01DF9_57B9_4621_88BD_CB374B385E1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>
#include <helpers\DirectIO.h>

class LPTBitRender: public CRenderGadget
{
private:
    CDirectIO * m_directIO;
    int         m_Port;
public:
	LPTBitRender();
	virtual void ShutDown();
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    virtual bool ScanSettings(FXString& text);
private:
	virtual void Render(const CDataFrame* pDataFrame);
    DECLARE_RUNTIME_GADGET(LPTBitRender);
};

#endif // !defined(AFX_LPTBITRENDERER_H__CDF01DF9_57B9_4621_88BD_CB374B385E1C__INCLUDED_)
