// HTMLDlg.cpp: implementation of the CHTMLDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HTMLDlg.h"

#include <tchar.h>

#include <atlconv.h>

#ifdef __AFX_H__
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTMLDlg::CHTMLDlg() :
	m_bstrUrl( NULL ),
	m_hMSHTML( NULL ),
	m_hWnd( NULL ),
	m_Options(),
	m_pfn( NULL ),
	m_pMoniker( NULL ),
	m_vResult()
{
	::memset( &m_vResult, 0, sizeof(VARIANT) );
	m_vResult.vt = VT_EMPTY;
}

CHTMLDlg::~CHTMLDlg()
{
	Destroy();
}

CHTMLDlg::CHTMLDlgOptions::CHTMLDlgOptions() :
	m_dwHeight( 0xffffffff ),
	m_dwWidth( 0xffffffff ),
	m_dwX( 0xffffffff ),
	m_dwY( 0xffffffff ),
	m_bCenter( true ),
	m_bHelp( true ),
	m_bResize( false ),
	m_bScroll( true ),
	m_bStatus( true ),
	m_bSunken( false ),
	m_bUnadorned( false )
{
}

void CHTMLDlg::CHTMLDlgOptions::GetString(WCHAR * pwszBuffer, DWORD dwMaxLen)
{
	if (dwMaxLen > 0)
	{
		DWORD dw = 0;
		WCHAR wsz[ 100 ];

		*pwszBuffer = L'\0';

		if (m_dwHeight != 0xffffffff)
		{
			::swprintf( wsz, L"dialogHeight: %dpx;", m_dwHeight );
			dw += ::wcslen( wsz );
			if (dw < dwMaxLen)
				::wcscat( pwszBuffer, wsz );
		}

		if (m_dwWidth != 0xffffffff)
		{
			::swprintf( wsz, L"dialogWidth: %dpx;", m_dwWidth );
			dw += ::wcslen( wsz );
			if (dw < dwMaxLen)
				::wcscat( pwszBuffer, wsz );
		}

		if (m_dwX != 0xffffffff)
		{
			::swprintf( wsz, L"dialogLeft: %dpx;", m_dwX );
			dw += ::wcslen( wsz );
			if (dw < dwMaxLen)
				::wcscat( pwszBuffer, wsz );
		}

		if (m_dwY != 0xffffffff)
		{
			::swprintf( wsz, L"dialogTop: %dpx;", m_dwY );
			dw += ::wcslen( wsz );
			if (dw < dwMaxLen)
				::wcscat( pwszBuffer, wsz );
		}

		::swprintf( wsz, L"center: %s;", (m_bCenter ? L"yes" : L"no") );
		dw += ::wcslen( wsz );
		if (dw < dwMaxLen)
			::wcscat( pwszBuffer, wsz );

		::swprintf( wsz, L"help: %s;", (m_bHelp ? L"yes" : L"no") );
		dw += ::wcslen( wsz );
		if (dw < dwMaxLen)
			::wcscat( pwszBuffer, wsz );

		::swprintf( wsz, L"resizable: %s;", (m_bResize ? L"yes" : L"no") );
		dw += ::wcslen( wsz );
		if (dw < dwMaxLen)
			::wcscat( pwszBuffer, wsz );

		::swprintf( wsz, L"scroll: %s;", (m_bScroll ? L"yes" : L"no") );
		dw += ::wcslen( wsz );
		if (dw < dwMaxLen)
			::wcscat( pwszBuffer, wsz );

		::swprintf( wsz, L"status: %s;", (m_bStatus ? L"yes" : L"no") );
		dw += ::wcslen( wsz );
		if (dw < dwMaxLen)
			::wcscat( pwszBuffer, wsz );

		::swprintf( wsz, L"edge: %s;", (m_bSunken ? L"sunken" : L"raised") );
		dw += ::wcslen( wsz );
		if (dw < dwMaxLen)
			::wcscat( pwszBuffer, wsz );

		::swprintf( wsz, L"unadorned: %s;", (m_bUnadorned ? L"yes" : L"no") );
		dw += ::wcslen( wsz );
		if (dw < dwMaxLen)
			::wcscat( pwszBuffer, wsz );
	}
}

bool CHTMLDlg::Init(LPCTSTR pszPath, HWND hwndParent)
{
	Destroy();

	bool bResult = true;
	USES_CONVERSION;

	try
	{
		m_hMSHTML = ::LoadLibrary( _T("MSHTML.DLL") );
		if (!m_hMSHTML)
			throw 0;

		m_pfn = reinterpret_cast< SHOWHTMLDIALOG_PFN >
			(::GetProcAddress( m_hMSHTML, "ShowHTMLDialog" ));
		if (!m_pfn)
			throw 1;

		m_bstrUrl = ::SysAllocString( T2COLE( pszPath ) );
		if (!m_bstrUrl)
			throw 2;

		if (FAILED(::CreateURLMoniker( NULL, m_bstrUrl, &m_pMoniker )))
			throw 3;

		m_hWnd = hwndParent;
	}
	catch (int)
	{
		Destroy();
		bResult = false;
	}

	return bResult;
}

void CHTMLDlg::Destroy()
{
	if (m_bstrUrl)
	{
		::SysFreeString( m_bstrUrl);
		m_bstrUrl = NULL;
	}

	if (m_hMSHTML)
	{
		::FreeLibrary( m_hMSHTML );
		m_hMSHTML = NULL;
	}

	if (m_pfn)
		m_pfn = NULL;

	if (m_pMoniker)
	{
		m_pMoniker->Release();
		m_pMoniker = NULL;
	}

	if (m_vResult.vt == VT_BSTR)
		::SysFreeString( m_vResult.bstrVal );

	::VariantClear( &m_vResult );
}

bool CHTMLDlg::Show(VARIANT * pvArgs)
{
	if (!m_bstrUrl || !m_hMSHTML || !m_pfn || !m_pMoniker)
		return false;

	WCHAR wszBuf[ 1024 ];
	m_Options.GetString( wszBuf );

	return (SUCCEEDED((*m_pfn) ( m_hWnd, m_pMoniker, pvArgs, wszBuf, &m_vResult )) ? true : false);
}

const VARIANT & CHTMLDlg::GetResult()
{
	return m_vResult;
}