// TextCaptureCtrl.h: interface for the TextCaptureCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTCAPTURECTRL_H__2AA043DE_3407_41EF_8C12_DEDD8F4ABB05__INCLUDED_)
#define AFX_TEXTCAPTURECTRL_H__2AA043DE_3407_41EF_8C12_DEDD8F4ABB05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include "TextCaptureDlg.h"

class TextCaptureCtrl : public CCtrlGadget  
{
protected:
    CTextCaptureDlg   m_CtrlDlg;
    TextCaptureCtrl();
    CInputConnector * m_pInput ;
    DWORD             m_FramesCounter;
    FXString          m_Message;
    BOOL              m_SendQuantity;
    BOOL              m_NoID;
    BOOL              m_Manual;
    int               m_FrameRate;
    double            m_dFramePeriod_ms ;
    BOOL              m_bSendOnStart ;
    BOOL              m_StringSeparate ;
    BOOL              m_Loop ;
    int               m_iStringCount ;
    FXStringArray     m_Strings ;
    FXLockObject      m_TextProcessLock ;
    bool              m_bChanged ;
public:
  virtual int GetInputsCount()                { return 1;}
  virtual CInputConnector* GetInputConnector(int n)   { return (n == 0) ? m_pInput : NULL; }
    void    ShutDown();
    void    Attach(CWnd* pWnd);
    void    Detach();
    CWnd*   GetRenderWnd() { return &m_CtrlDlg; }

    CDataFrame* GetNextFrame(double* StartTime);

    void    GetDefaultWndSize (RECT& rc) 
    { 
        m_CtrlDlg.GetClientRect(&rc);
    }
    bool    IsCtrlReady() { return (m_CtrlDlg.GetSafeHwnd()!=NULL); }
    void    OnStop() { m_FramesCounter=0; }
    void    OnStart() ;

    bool    ScanSettings(FXString& text);
    bool    ScanProperties(LPCTSTR text, bool& Invalidate);
    bool    PrintProperties(FXString& text);

    void    OnCommand(UINT nID, int nCode);
    void    OnInput(CDataFrame* lpData, CConnector* lpInput);

    DECLARE_RUNTIME_GADGET(TextCaptureCtrl);
    int CheckAndFormStrings(void);
};

#endif // !defined(AFX_TEXTCAPTURECTRL_H__2AA043DE_3407_41EF_8C12_DEDD8F4ABB05__INCLUDED_)
