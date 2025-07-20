#ifndef  GenericRender_INC
#define  GenericRender_INC

#include <gadgets\gadbase.h>

class GenericRender;

class CGenericView : public CWnd
{
    GenericRender* m_GR;
public:
	CGenericView(GenericRender* gr);
// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenericView)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGenericView();
	// Generated message map functions
protected:
	//{{AFX_MSG(CGenericView)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class GenericRender : public CRenderGadget
{
    CGenericView*   m_View;
    CDataFrame*     m_Frame;
    FXLockObject    m_Lock;
public:
	virtual void    ShutDown();
	virtual void    Attach(CWnd* pWnd);
	virtual void    Detach();
            CWnd*   GetRenderWnd() { return m_View; }
    virtual void    GetDefaultWndSize (RECT& rc);
            bool    Draw(CDC * dc, RECT rc);
private:
    GenericRender(void);
	virtual void Render(const CDataFrame* pDataFrame);
    DECLARE_RUNTIME_GADGET(GenericRender);
};

#endif 
