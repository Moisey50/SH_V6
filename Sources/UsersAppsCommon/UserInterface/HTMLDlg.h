// HTMLDlg.h: interface for the CHTMLDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTMLDLG_H__FB2519D0_85B0_4745_ABD4_6225C0231DF7__INCLUDED_)
#define AFX_HTMLDLG_H__FB2519D0_85B0_4745_ABD4_6225C0231DF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef HRESULT (STDAPICALLTYPE *SHOWHTMLDIALOG_PFN) (HWND hwndParent, IMoniker *pmk, VARIANT *pvarArgIn, WCHAR* pchOptions, VARIANT *pvArgOut);

class CHTMLDlg  
{
	class CHTMLDlgOptions
	{
		CHTMLDlgOptions(void);

		void GetString(WCHAR * pwszBuffer, DWORD dwMaxLen = 1024);

		DWORD m_dwWidth,
			m_dwHeight,
			m_dwX,
			m_dwY;
		bool m_bCenter,
			m_bSunken,
			m_bHelp,
			m_bResize,
			m_bScroll,
			m_bStatus,
			m_bUnadorned;

		friend class CHTMLDlg;
	};

public:
	CHTMLDlg(void);
	virtual ~CHTMLDlg(void);

// Operations
public:
	bool Init(LPCTSTR pszPath, HWND hwndParent = NULL);    // Initialize the object
	void Destroy(void);                                    // Destroy the object
	bool Show(VARIANT * pvArgs = NULL);                    // Show the dialog
	const VARIANT& GetResult(void);                        // Get the return value of the dialog

// Attributes
public:
	DWORD GetHeight(void) const
	{
		return m_Options.m_dwHeight;
	}
	DWORD GetWidth(void) const
	{
		return m_Options.m_dwWidth;
	}
	DWORD GetTop(void) const
	{
		return m_Options.m_dwY;
	}
	DWORD GetLeft(void) const
	{
		return m_Options.m_dwX;
	}
	bool IsCentered(void) const
	{
		return m_Options.m_bCenter;
	}
	bool IsEdgeSunken(void) const
	{
		return m_Options.m_bSunken;
	}
	bool IsEdgeRaised(void) const
	{
		return !IsEdgeSunken();
	}
	bool HasHelpButton(void) const
	{
		return m_Options.m_bHelp;
	}
	bool IsResizable(void) const
	{
		return m_Options.m_bResize;
	}
	bool IsScrollable(void) const
	{
		return m_Options.m_bScroll;
	}
	bool HasStatusBar(void) const
	{
		return m_Options.m_bStatus;
	}
	bool IsUnadorned(void) const
	{
		return m_Options.m_bUnadorned;
	}

	void SetHeight(DWORD dw)
	{
		m_Options.m_dwHeight = dw;
	}
	void SetWidth(DWORD dw)
	{
		m_Options.m_dwWidth = dw;
	}
	void SetTop(DWORD dw)
	{
		m_Options.m_dwY = dw;
	}
	void SetLeft(DWORD dw)
	{
		m_Options.m_dwX = dw;
	}
	void SetCentered(bool bCentered = true)
	{
		m_Options.m_bCenter = bCentered;
	}
	void SetEdgeSunken(void)
	{
		m_Options.m_bSunken = true;
	}
	void SetEdgeRaised(void)
	{
		m_Options.m_bSunken = false;
	}
	void SetHelpButton(bool bHelp = true)
	{
		m_Options.m_bHelp = bHelp;
	}
	void SetResizable(bool bResize = true)
	{
		m_Options.m_bResize = bResize;
	}
	void SetScrollable(bool bScroll = true)
	{
		m_Options.m_bScroll = bScroll;
	}
	void SetStatusBar(bool bStatus = true)
	{
		m_Options.m_bStatus = bStatus;
	}
	void SetUnadorned(bool bUnadorned = true)
	{
		m_Options.m_bUnadorned = bUnadorned;
	}

private:
	HINSTANCE m_hMSHTML;
	SHOWHTMLDIALOG_PFN m_pfn;
	struct IMoniker * m_pMoniker;
	HWND m_hWnd;
	BSTR m_bstrUrl;
	CHTMLDlgOptions m_Options;
	VARIANT m_vResult;
};

#endif // !defined(AFX_HTMLDLG_H__FB2519D0_85B0_4745_ABD4_6225C0231DF7__INCLUDED_)
