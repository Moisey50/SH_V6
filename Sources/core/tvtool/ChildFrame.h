#if !defined(AFX_CHILDFRAME_H__0F9F5149_05F7_4FEB_8C6C_C0804494A1AC__INCLUDED_)
#define AFX_CHILDFRAME_H__0F9F5149_05F7_4FEB_8C6C_C0804494A1AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChildFrame.h : header file
//

#include <Gadgets\tview.h>
#include <Gadgets\shkernel.h>
#include <classes\drect.h>
#include "PerforamceDlg2.h"

inline void CalcRelativeRect( CRect Child , CRect Parent ,
  CDRect& Result )
{
  Result.left = (double) (Child.left - Parent.left) / (double) Parent.Width() ;
  Result.right = (double) (Child.Width()) / (double) Parent.Width() ;
  Result.top = (double) (Child.top - Parent.top) / (double) Parent.Height() ;
  Result.bottom = (double) (Child.Height()) / (double) Parent.Height() ;
}

void GuessGraphName( FXString& graphName );
void FAR __stdcall PrintMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText );

/////////////////////////////////////////////////////////////////////////////
// CChildFrame frame

class CChildFrame : public CMDIChildWnd
{
  friend class CMainFrame;
  DECLARE_DYNCREATE( CChildFrame )

  IGraphbuilder*	m_pBuilder;
  ISketchView*	m_pWndView;
  FXString		m_GraphName;
  BOOL			m_bLongTitle;
  CPerforamceDlg2 m_PerformanceDlg2;
protected:
  CChildFrame();           // protected constructor used by dynamic creation

// Attributes
public:
  IGraphbuilder*	GetBuilder()
  {
    return m_pBuilder;
  }
  ISketchView*	GetIView()
  {
    return m_pWndView;
  }
  void			SetBuilder( IGraphbuilder* pBuilder = NULL , CExecutionStatus* pES = NULL );
  void			SetGraphName( LPCTSTR graphName );
  const FXString	GetGraphName() const
  {
    return m_GraphName;
  }

  // Operations
public:
  BOOL			LoadGraph( LPCTSTR filename );
  BOOL      LoadScript( LPCTSTR Script , LPCTSTR fName );
  BOOL      GraphStart();
  void			ToggleLongName();
  BOOL			IncludePlugin( FXString& Plugin );

  // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CChildFrame)
public:
  virtual BOOL OnCmdMsg( UINT nID , int nCode , void* pExtra , AFX_CMDHANDLERINFO* pHandlerInfo );
  virtual BOOL DestroyWindow();
  virtual BOOL SetGraphPosition( CDRect& Pos ) ;
  virtual CPoint GetViewOffset() { return m_pWndView->GetViewOffset(); }
  virtual void SetViewOffset( CPoint offset )
  {
    m_pWndView->SetViewOffset( offset ) ; 
    GET_WND( m_pWndView )->Invalidate( TRUE ) ;
  } ;
  virtual double GetViewScale() { return m_pWndView->GetViewScale(); } ;
  virtual void SetViewScale( double scale ) 
  { 
    m_pWndView->SetViewScale( scale ) ; 
    GET_WND( m_pWndView )->Invalidate( TRUE ) ;
  };
protected:
  virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
  //}}AFX_VIRTUAL

// Implementation
protected:
  ~CChildFrame();
  // Generated message map functions
  //{{AFX_MSG(CChildFrame)
  afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
  afx_msg void OnDestroy();
  afx_msg void OnSetFocus( CWnd* pOldWnd );
  afx_msg void OnFileSaveGraph();
  afx_msg void OnUpdateFileSaveGraph( CCmdUI* pCmdUI );
  afx_msg void OnOperateStart();
  afx_msg void OnUpdateOperateStart( CCmdUI* pCmdUI );
  afx_msg void OnOperateStop();
  afx_msg void OnUpdateOperateStop( CCmdUI* pCmdUI );
  afx_msg void OnOperatePause();
  afx_msg void OnUpdateOperatePause( CCmdUI* pCmdUI );
  afx_msg void OnOperateStepForward();
  afx_msg void OnUpdateOperateStepForward( CCmdUI* pCmdUI );
  afx_msg void OnOperateStartstop();
  afx_msg void OnEditDelete();
  afx_msg void OnEditClearAll();
  afx_msg void OnEditAggregate();
  afx_msg void OnEditExpand();
  afx_msg BOOL OnSetCursor( CWnd* pWnd , UINT nHitTest , UINT message );
  afx_msg void OnEditSelectAll();
  afx_msg void OnSettings();
  afx_msg BOOL OnMouseWheel( UINT nFlags , short zDelta , CPoint pt );
  afx_msg void OnEditUndo();
  afx_msg void OnUpdateEditUndo( CCmdUI* pCmdUI );
  afx_msg void OnEditRedo();
  afx_msg void OnUpdateEditRedo( CCmdUI* pCmdUI );
  afx_msg void OnViewUndoRedoManager();
  afx_msg void OnUpdateViewUndoRedoManager( CCmdUI* pCmdUI );
  afx_msg void OnOperateDebug();
  afx_msg void OnUpdateOperateDebug( CCmdUI* pCmdUI );
  afx_msg void OnEditCopy();
  afx_msg void OnUpdateEditCopy( CCmdUI* pCmdUI );
  afx_msg void OnEditPaste();
  afx_msg void OnUpdateEditPaste( CCmdUI* pCmdUI );
  afx_msg BOOL OnHelpInfo( HELPINFO* pHelpInfo );
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnViewPerformancedialog2();
  afx_msg void OnUpdateViewPerformancedialog2( CCmdUI *pCmdUI );
  afx_msg void OnSize( UINT nType , int cx , int cy );
  afx_msg void OnEditResetaffinity();
  afx_msg void OnUpdateEditResetaffinity( CCmdUI *pCmdUI );
  afx_msg void OnMove( int x , int y );
  afx_msg void OnUsedgadgetsView();
  afx_msg void OnUsedgadgetsFormlist();
  afx_msg void OnFormActiveExeAndDLLCopyBatch();
  int EnumAndArrangeAllGadgets( 
    IGraphbuilder * pBuilder , FXStringArray& AllGadgets );
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRAME_H__0F9F5149_05F7_4FEB_8C6C_C0804494A1AC__INCLUDED_)
