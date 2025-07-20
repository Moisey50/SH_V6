#if !defined(AFX_RENDERVIEWFRAME_H__CC048939_5AEE_420B_911D_CD082B89B770__INCLUDED_)
#define AFX_RENDERVIEWFRAME_H__CC048939_5AEE_420B_911D_CD082B89B770__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RenderViewFrame.h : header file
//

#include "Graph.h"

/////////////////////////////////////////////////////////////////////////////
// CRenderViewFrame window

class CRenderViewFrame : public CStatic
{
  CRenderGlyph* m_pGlyph;
  CWnd        * m_ViewWnd;
  BOOL m_bBuiltIn;
// Construction
public:
  CRenderViewFrame( CRenderGlyph* Glyph );
  BOOL IsBuiltIn() { return m_bBuiltIn; };
// Attributes
public:

// Operations
public:

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CRenderViewFrame)
public:
  virtual BOOL Create( const RECT& rect , CWnd* pParentWnd , BOOL m_bBuiltIn = TRUE );
  void    Attach( CWnd* view ) { m_ViewWnd = view; }
//}}AFX_VIRTUAL

// Implementation
public:
  ~CRenderViewFrame();

  // Generated message map functions
protected:
  //{{AFX_MSG(CRenderViewFrame)
  afx_msg void OnSize( UINT nType , int cx , int cy );
  afx_msg BOOL OnEraseBkgnd( CDC* pDC );
  afx_msg LRESULT OnGlyphChangeSize( WPARAM wparam , LPARAM lparam );
//}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENDERVIEWFRAME_H__CC048939_5AEE_420B_911D_CD082B89B770__INCLUDED_)
