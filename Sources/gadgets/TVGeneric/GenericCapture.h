// GenericCapture.h: interface for the GenericCapture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GENERICCAPTURE_H__60372DA7_B3C4_4B5C_8BD6_E1F0235ECF76__INCLUDED_)
#define AFX_GENERICCAPTURE_H__60372DA7_B3C4_4B5C_8BD6_E1F0235ECF76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <files\FileList.h>
#include <gadgets\FigureFrame.h>
#include <helpers\PrxyWnd.h>

class GenericCapture : public CCtrlGadget  
{
    CPrxyWnd             m_Proxy;
    CButton              m_Button;
    CInputConnector     *m_pInput;
    basicdatatype        m_DataType;
    BOOL                 m_NoID;
    DWORD                m_FramesCounter;
    BOOL                 m_Manual;
    BOOL                 m_Replace;
    BOOL                 m_bSendOnStart = FALSE ;
    BOOL                 m_SetRegistredFlag;
    BOOL                 m_SetEOS ;
    int                  m_FrameRate;
    FXPropertyKit         m_Cmd;
    CFileList            m_FL;
    double               m_Quantity;
    bool                 m_Bool;
    BOOL                 m_bIgnoreInputConnected ;
    FXString             m_Message;
    CFigure              m_Figure;
    CRect                m_Rect;
    FXString             m_Attributes;
    FXString             m_Label;
    FXString             m_ButtonName;
    FXLockObject          m_Lock;
private:
    void    UpdateBtnStatus();
    CDataFrame* PrepareNextFrame();
    bool    IsCtrlReady() { return (m_Button.GetSafeHwnd()!=NULL); }
    bool    IsInputConnected() { return m_pInput->IsConnected(); }
public:
	GenericCapture();
	virtual void ShutDown();

    void    Attach(CWnd* pWnd);
    void    Detach();

    virtual int GetInputsCount() { return 1; };
    virtual CInputConnector*    GetInputConnector(int n) { return (n==0)?m_pInput:NULL; }
    virtual void    GetDefaultWndSize( RECT& rc )
    {
      rc.left = 0; rc.right = 80; rc.top = 0; rc.bottom = 26;
    }
    virtual CWnd*   GetRenderWnd() { return &m_Proxy; }
    virtual void OnStop();
    virtual void OnStart();

    bool ScanSettings(FXString& txt);
    bool ScanProperties(LPCTSTR txt, bool& Invalidate);
    bool PrintProperties(FXString& txt);

    virtual CDataFrame* GetNextFrame(double* StartTime);

    void OnCommand(UINT nID, int nCode);
    void OnInput(CDataFrame* lpData, CConnector* lpInput);
    DECLARE_RUNTIME_GADGET(GenericCapture);
};

#endif // !defined(AFX_GENERICCAPTURE_H__60372DA7_B3C4_4B5C_8BD6_E1F0235ECF76__INCLUDED_)
