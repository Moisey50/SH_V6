// FloatWndManager.h: interface for the CFloatWndManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLOATWNDMANAGER_H__12D74A49_0B1C_494B_A65D_DCCEAC6EC9EE__INCLUDED_)
#define AFX_FLOATWNDMANAGER_H__12D74A49_0B1C_494B_A65D_DCCEAC6EC9EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFloatWnd;
class CSketchView;
class CFloatWndManager  
{
	CPtrArray	m_Monitors;
public:
	        CFloatWndManager();
	        ~CFloatWndManager();

	CFloatWnd* CreateChannel(CSketchView* pSketchView, CString& monitor, LPCTSTR uid, CRect& rect);
    void    EnumMonitors(CStringArray& Names);
    void    DestroyChannel(LPCTSTR uid);
	void    SetChannelFrame(LPCTSTR monitor, LPCTSTR uid, CWnd* pGlyphFrame);
	bool    PtInside(CPoint& pt);
	CFloatWnd* FindMonitor(LPCTSTR name);
  CString RenameChannel( LPCTSTR pOldChannel , LPCTSTR pNewChannel ) ;
private:
	CString GenerateNewMonitorName();
};

#endif // !defined(AFX_FLOATWNDMANAGER_H__12D74A49_0B1C_494B_A65D_DCCEAC6EC9EE__INCLUDED_)
