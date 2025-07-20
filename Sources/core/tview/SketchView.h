#if !defined(AFX_SKETCHVIEW_H__3563E7F0_8E6C_4EDD_8E50_9A9E9B9C279E__INCLUDED_)
#define AFX_SKETCHVIEW_H__3563E7F0_8E6C_4EDD_8E50_9A9E9B9C279E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SketchView.h : header file
//

#include "pointsmap.h"
#include "graph.h"
#include "mouseactions.h"
#include <gadgets\shkernel.h>
#include <shbase\shbase.h>
#include <gadgets\tview.h>
#include "gadgetstreedlg.h"
#include "floatwndmanager.h"
#include "tvtooltip.h"
#include "userinterface\workspacebar.h"
#include "undomanager.h"
#include "debugviewdlg.h"
#include "dragdrop.h"

#define SCR_VERT 1
#define SCR_HORZ 2

/////////////////////////////////////////////////////////////////////////////
// CSketchView window
class CSketchView;
class CSketchView : public CWnd
{
protected:
  CWnd*			m_pTargetWnd;	// window for messaging
  CGDIBank        m_GDI; // collection of pens and brushes
  CPointsMap      m_Map;
  CCompositeGlyph m_Graph;
  CCollectionGlyph m_SelectedGlyphs;
  CCollectionGlyph m_VirtualGlyphs;
  CRgn			m_CopyRgn;
  CMouseActions   m_MouseAction;
  IGraphbuilder*  m_pGraphBuilder;
  CPtrArray       m_FlashingGlyphs;
  CFloatWndManager m_FloatWndManager;
  CFont           m_GadgetsFont;
  CTVToolTip      m_ToolTip;
  CImageList      m_GlyphIcons , m_ModesIcons;
  CRect           m_ViewRect;
  UINT            m_BarsEnabled;
  UINT            m_XPage , m_YPage;
  CPoint          m_LastMousePoint;
  CGlyph*         m_pLastActiveGlyph;
  CUndoManager    m_UndoManager;
  BOOL            m_bDebugging;
  CDebugViewDlg*  m_DebugWnd;
  HACCEL          m_hAccel;
  DWORD           m_LastShownChange;
  bool            m_ViewActivity;
  CDropTarget m_DropTarget;
  CString m_sCurrGadgetName;
  CString m_sCurrGadgetInfo;
  TRACKMOUSEEVENT m_tme;
  FXString		m_LastGraphScript;
  FXString		m_LastGraphProps;

  struct tagCONTEXTMENUINFO
  {
    CGlyph* pGlyph;
    CPoint	pt;
  }m_ContextMenuInfo;
  // Construction
public:
  BOOL			m_bRescaled;
  BOOL			m_bWasRescaled;
  CSketchView();
  virtual ~CSketchView();

  void     SetBuilder( IGraphbuilder* Builder );
  void     SetTargetWnd( CWnd* pWnd )
  {
    m_pTargetWnd = pWnd;
  };
  void     ClearView();   // Clears just Glyphs, not graph in a builder
  void     DeleteGraph();
  void     SelectAll();
  void     ComposeGraph( CStringArray* SrcGadgets = NULL , CStringArray* DstGadgets = NULL );
  void     UpdateGraph( CStringArray& NewGadgets );
  void	   UpdateProperties();
  bool     InsertGadget();
  void     RegisterSketchDragDrop();
  bool     InsertGadget( LPCTSTR lpszClassName , CPoint& ptScreen , LPCTSTR params = NULL , LPCTSTR uidopt = NULL );
  void     DeleteSelected();
  BOOL     Disconnect( FXString& uid );
  BOOL     Disconnect( FXString& uid1 , FXString &uid2 );
  void     AggregateSelected( LPCTSTR uid = NULL , LPCTSTR name = NULL );
  void     ExpandSelected();
  void     InitUndoManager();
  void     ShowGadgetsBar( BOOL bShow );
  void     ShowAffinityDlg( FXString& uid , CPoint& point );
  void     FlashGlyph( CGlyph* pGlyph );
  void	   RenameGlyph( CGlyph* pGlyph , LPCTSTR uid );
  CWnd*    CreateRenderFrame( LPRECT rc , CRenderGlyph* pHost , CString& Monitor );
  int      CanUndo( CStringArray* IDs = NULL )
  {
    return m_UndoManager.CanUndo( IDs );
  };
  int      CanRedo( CStringArray* IDs = NULL )
  {
    return m_UndoManager.CanRedo( IDs );
  };
  BOOL     Undo( int steps = 1 ); // negative steps -> Redo
  BOOL     IsDebugging()
  {
    return m_bDebugging;
  };
  void     SetDebugMode( BOOL on ); // { m_bDebugging = on; if (!on) DisconnectDebugRender(); else CreateDebugRender(this); };
  BOOL     CanCopy();
  void     Copy();
  BOOL     CanPaste();
  void     Paste();
  void     EmptyCopyBuffer();
  void     Map( CPoint& pt )
  {
    m_Map.AbsToView( &pt );
  };
  void	 Map( CRect& rect )
  {
    m_Map.AbsToView( &rect );
  }

  // Attributes
public:
  IGraphbuilder* GetBuilder()
  {
    return m_pGraphBuilder;
  };
  // Operations
public:
  void    DrawRect( CDC* pDC , LPRECT rc , COLORREF penColor = RGB( 0 , 0 , 0 ) , 
    COLORREF bgColor = RGB( 255 , 255 , 255 ) );
  void    DrawRoundRect( CDC* pDC , LPRECT rc , COLORREF penColor = RGB( 0 , 0 , 0 ) ,
    COLORREF bgColor = RGB( 255 , 255 , 255 ) , CPoint * pEllipse = NULL );
  void    DrawBigRoundRect( CDC* pDC , LPRECT rc , COLORREF penColor = RGB( 0 , 0 , 0 ) ,
    COLORREF bgColor = RGB( 255 , 255 , 255 ) );
  void	  DrawEdgeRect( CDC* pDC , LPRECT rc , COLORREF penColor = RGB( 0 , 0 , 0 ) , COLORREF bgColor = RGB( 255 , 255 , 255 ) );
  void    DrawLine( CDC* pDC , LPRECT rc , LOGPEN* pen = NULL , int rop = R2_COPYPEN );
  void    DrawLine( CDC* pDC , CPoint& pt1 , CPoint& pt2 , LOGPEN* pen = NULL , int rop = R2_COPYPEN );
  void    DrawText( CDC* pDC , LPCTSTR text , LPRECT rc , COLORREF color , LOGFONT* font );
  void    DrawIcon( CDC* pDC , int idIcon , LPRECT rc , 
    COLORREF* penColor = NULL , COLORREF* bgColor = NULL , LPRECT rcResult = NULL );
  void	  DrawModeIcon( CDC* pDC , int idIcon , LPRECT rc );
  BOOL    GetMyPen( void* user , LOGPEN* pen ); // FALSE -> user should not draw
  BOOL    GetMyRectColors( void* user , COLORREF* crBorder , COLORREF* crBody , int isDataPassed = NO_DATA_PASSED ); // FALSE -> user should not draw
  BOOL    GetMyFont( void* user , COLORREF* crText , LOGFONT* font ); // FALSE -> user should not draw
  int     GetMyIcon( void* user ); // -1 -> user should not draw
  int		  GetMyModeIcon( void* user ); // -1 -> user should not draw
  void    ShowScrollBar( UINT nBar , BOOL bShow = TRUE );
  BOOL    SetScrollInfo( int nBar , LPSCROLLINFO lpScrollInfo , BOOL bRedraw = TRUE );

  virtual void Paint();
  virtual void SetViewType( ViewType vt )
  {
    return;
  };
  virtual ViewType GetViewType()
  {
    ViewType vt; return vt;
  }
  virtual void CascadeFloatWindows();//{return;}
  bool GetViewActivity()
  {
    return m_ViewActivity;
  }
  virtual CPoint GetViewOffset() { return m_Map.GetOffset() ; }
  virtual void SetViewOffset( CPoint offset ) { m_Map.SetOffset( offset ) ; Invalidate( TRUE ) ; };
  virtual double GetViewScale() const { return m_Map.GetScale(); };
  virtual void SetViewScale( double scale ) { m_Map.SetScale( scale ) ; Invalidate( TRUE ) ; };
  void SetViewActivity( bool set );
  // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSketchView)
public:
  virtual BOOL PreTranslateMessage( MSG* pMsg );
protected:
  virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
  virtual BOOL OnCommand( WPARAM wParam , LPARAM lParam );
  virtual LRESULT WindowProc( UINT message , WPARAM wParam , LPARAM lParam );
  //}}AFX_VIRTUAL

// Implementation
public:
  void       RemoveGadget( CGlyph* pGlyph , FXString& uid );
  CString    GetCurrentGadgetName();
  CString    GetCurrentGadgetInfo();
  // Generated message map functions
protected:
  virtual CGlyph* InsertGadgetAt( CPoint pt , UINT* type = NULL , LPCTSTR uidopt = NULL );
  void    RedrawMouseAction();
  void    DrawNewGlyph( CGlyph* pGlyph );
  BOOL	ConnectGlyphs( CGlyph* pStartGlyph , CGlyph* pEndGlyph );
  void    ProcMouseEntry( UINT nFlags , CPoint point );
  BOOL    IsSelectedGlyph( void* user );
  BOOL    IsVirtualGlyph( void* user );
  BOOL    IsInvaldeGlyph( void* user );
  void	TrackMouseEvent();
  void    UpdateActivity();
public:
  void    AddGlyphToGraph( CGlyph* pGlyph );
  void    RemoveGlyphFromGraph( CGlyph *pGlyph );
  void	ToggleDebugRender( const FXString& uid , BOOL bDestroy = FALSE );
protected:
  CString AutoBlockName();
  int     DefineGlyphIcon( LPCTSTR strGadgetClass , UINT uGadgetType );
  void    RedrawGlyph( CGlyph* pGlyph );
  void    FixCurrentState( LPCTSTR name = "" );
  BOOL    CreateDebugRender( CWnd* pParentWnd );
  void    DeleteDebugRender();
  void    DisconnectDebugRender();
  BOOL    ConnectDebugRender( FXString& uidOutput );
  void    ClearSelected();
  BOOL    DoPaste();

  //{{AFX_MSG(CSketchView)
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown( UINT nFlags , CPoint point );
  afx_msg void OnLButtonUp( UINT nFlags , CPoint point );
  afx_msg void OnMouseMove( UINT nFlags , CPoint point );
  afx_msg void OnDestroy();
  afx_msg void OnRButtonDown( UINT nFlags , CPoint point );
  afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  afx_msg void OnLButtonDblClk( UINT nFlags , CPoint point );
  afx_msg void OnVScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar );
  afx_msg void OnHScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar );
  afx_msg BOOL OnMouseWheel( UINT nFlags , short zDelta , CPoint pt );
  afx_msg void OnRButtonUp( UINT nFlags , CPoint point );
  afx_msg BOOL OnEraseBkgnd( CDC* pDC );
  afx_msg LRESULT OnMouseLeave( WPARAM wparam , LPARAM lparam );
  afx_msg LRESULT OnFltWndChanged( WPARAM wparam , LPARAM lparam );
  afx_msg void OnContextRename();
  afx_msg void OnContextSetup();
  afx_msg void OnContextAffinity();
  afx_msg void OnContextCmplxLocal();
  afx_msg void OnPinSetLabel();
  afx_msg void OnPinLogging();
  afx_msg void OnModeReject();
  afx_msg void OnModeTransmit();
  afx_msg void OnModeProcess();
  afx_msg void OnModeAppend();
  afx_msg void OnModeReplace();
  afx_msg void OnThreadsIncrease();
  afx_msg void OnThreadsDecrease();


  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CGadgetsDockbar window

class CGadgetsDockbar : public CWorkspaceBar
{
  CTabCtrl			m_Tab;
  CGadgetsTreeDlg*	m_pDlg;
  CGadgetsTreeDlg*	m_pLibDlg;
  CStringArray		m_ComplexPaths;
  const enum
  {
    TAB_GADGETSTREE , TAB_USERLIBRARY
  };
  // Construction
public:
  CGadgetsDockbar();

  // Attributes
public:
  void SetView( ISketchView* IView )
  {
    if ( m_pDlg ) m_pDlg->SetView( IView ); if ( m_pLibDlg ) m_pLibDlg->SetView( IView );
  };
  BOOL IsTypeShown()
  {
    CGadgetsTree* pTree = GetTree(); return (pTree && pTree->IsGroupByMedia());
  };

  // Operations
public:
  BOOL Create( CWnd* pParent , LPCTSTR title = "" , DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_LEFT
    | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC , UINT uID = TVDB_WORKSPACE_BAR_ID_FIRST );
  void PopulateTree( IGraphbuilder* pBuilder , BOOL bDeleteExisting = TRUE );
  void SetShowType( BOOL bShow , IGraphbuilder* pBuilder );
  void PopulateUserLibrary( LPCTSTR path , LPCTSTR ext = _T( "cog" ) );

  // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CGadgetsDockbar)
    //}}AFX_VIRTUAL

  // Implementation
public:
  ~CGadgetsDockbar();

  // Generated message map functions
protected:
  void AddGadgetToTree( UINT type , LPCTSTR Class , LPCTSTR Lineage );
  CGadgetsTree* GetTree();
  CGadgetsTree* GetLibTree();

  //{{AFX_MSG(CGadgetsDockbar)
  afx_msg void OnSize( UINT nType , int cx , int cy );
  afx_msg void OnTabSelChange( NMHDR* pNMHDR , LRESULT* pResult );
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKETCHVIEW_H__3563E7F0_8E6C_4EDD_8E50_9A9E9B9C279E__INCLUDED_)
