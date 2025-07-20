#if !defined(AFX_GRIDLISTCTRL_H__AF56AAD3_DC09_49E0_BA30_9716EC14FB96__INCLUDED_)
#define AFX_GRIDLISTCTRL_H__AF56AAD3_DC09_49E0_BA30_9716EC14FB96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridListCtrl.h : header file
//

#include <afxtempl.h>
#include <gadgets\spinctrl.h>

#ifdef TRACE_ALL
#define TRACE_GRID TRACE
#else
#define TRACE_GRID
#endif

#define SC_BUTTON       1
#define SC_SELCHANGED   2
#define SC_EDITLSTFOCUS 3
#define SC_INTCHANGE    4
#define SC_INTCHKCHANGE 5
#define SC_CHKCHANGE    6

typedef struct _intdata
{
  int     value;
  int     max , min;
  bool    enable;
}intdata;

typedef struct _cbdata
{
  LPTSTR  strlst;
  LPTSTR  inivalue;
  bool    reloadonchange;
}cbdata;

typedef struct _checkdata
{
  char    name[20];
  bool    state;
} checkdata ;

class FX_EXT_STDSETUP CGridCell
{
  friend class CGridListCtrl;
public:
  enum CellType
  {
    typeNull = 0x0000 ,
    typeIndent = 0x0001 ,
    typeString = 0x0002 ,
    typeButton = 0x0003 ,
    typeComboBox = 0x0004 ,
    typeInt = 0x0005 ,
    typeIntChk = 0x0006 ,
    typeChk    = 0x0007
  };
private:
  bool     m_Editable;
  bool     m_bDisabled ;
  bool     m_Changed;
  CellType m_Type;
  union
  {
    LPTSTR      m_pText;
    intdata     m_pInt;
    cbdata      m_CBData;
    checkdata   m_Check ;
  };
public:
  CWnd*  m_Ctrl;
  int    m_uData;
protected:
  void Clear();
  void Copy( CGridCell& Src );
public:
  CGridCell();
  CGridCell( CGridCell& Src );
  ~CGridCell();
  CGridCell& operator =( CGridCell& Src )
  {
    Copy( Src );
    return *this;
  }
  void DefineString( LPCTSTR src , bool Editable = false , int uData = -1 );
  void DefineIndent( LPCTSTR src );
  void DefineButton( LPCTSTR src , int uData );
  void DefineListBox( LPCTSTR src , LPCTSTR ititialVal , int uData );
  void DefineInt( int value , int min , int max , int uData );
  void DefineIntChk( int value , int min , int max , bool enable , int uData );
  void DefineChk( LPCTSTR name , bool value , int uData );
  LPCTSTR  GetText();
  int      GetInt();
  bool    GetIntChk( int& val , bool& enable , BOOL * pEnabled = NULL );
  bool    GetChk( bool& enable );
  bool    SetText( LPCTSTR pText );
  bool    SetInt( int iValue );
  bool    SetIntChk( int val , bool enable );
  bool    SetChk( bool enable );
  cbdata* GetCBData();
  CellType GetType()
  {
    return m_Type;
  }
  bool     IsEditable()
  {
    return m_Editable;
  }
  void     SetChanged()
  {
    m_Changed = true;
  }
  bool     IsChanged()
  {
    return m_Changed;
  }
  void    SetDisabled( bool bDisabled ) ;
  bool    IsDisabled() { return m_bDisabled ;  }
};

class CGridRow : public FXArray<CGridCell , CGridCell&>
{
  void Copy( CGridRow& Src )
  {
    RemoveAll();
    for ( int i = 0; i < (int) Src.GetSize(); i++ )
    {
      this->Add( Src[ i ] );
    }
    m_bGrayed = Src.m_bGrayed ;
  }
public:
  CGridRow()
  {
    m_bGrayed = false ;
  };
  CGridRow( CGridRow& Src )
  {
    Copy( Src );
  }
  CGridRow& operator =( CGridRow& Src )
  {
    Copy( Src );
    return *this;
  }

  bool m_bGrayed ;
};

typedef void( *GridEvent )(int Event , void *wParam , int col , int row , int uData);

/////////////////////////////////////////////////////////////////////////////
// CGridListCtrl window
class FX_EXT_STDSETUP CGridListCtrl : public CListCtrl
{
private:
  CPen    m_FramePen;
  CBrush  m_Highlight , m_BckGrnd , m_IndentBrush;
  int     m_VLastScrollPos;
  CFont   m_GridFont , m_BigFont;
  FXArray<CGridRow , CGridRow&> m_Data;
  GridEvent m_CB;
  LPVOID    m_CBwParam;
  bool      m_IsLastItem , m_IsFirstItem;
  CPoint    m_CurSel;
public:
  CGridListCtrl();
  // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CGridListCtrl)
public:
  virtual BOOL PreTranslateMessage( MSG* pMsg );
protected:
  virtual BOOL OnCommand( WPARAM wParam , LPARAM lParam = FALSE );
  //}}AFX_VIRTUAL
public:
  virtual ~CGridListCtrl();
  void    InitCtrl( GridEvent callback , LPVOID wParam );
  int     InsertColumn( int nCol , LPCTSTR lpszColumnHeading , 
    int nFormat = LVCFMT_LEFT , int nWidth = -1 , int nSubItem = -1 );
  int     AddRow( CGridRow& row );
  bool    HasElement( int c , int r );
  bool    SelectItem( int col , int row );
  bool    IsItemActive( int c , int r );
  FXString GetItemText( int col , int row );
  FXString GetItemData( int col , int row );
  int     GetItemInt( int col , int row );
  bool    GetItemIntChk( int col , int row , 
    int&val , bool& enable , BOOL * pEnabled = NULL );
  bool    GetItemChk( int col , int row , bool& enable );

  bool    SetCellText( LPCTSTR pName , LPCTSTR pText );
  bool    SetCellInt( LPCTSTR pName , int iValue );
  bool    SetCellChk( LPCTSTR pName , bool enable );
  int     SetCellIntValue( LPCTSTR pName , LPCTSTR pValueAsString );

  BOOL    DeleteAllItems();
  //  Edit ListCtr functions
  CGridRow* GetRow( int row )
  {
    if ( (row < 0) || (row >= (int) m_Data.GetSize()) ) return NULL;
    return (&(m_Data[ row ]));
  }
  BOOL    DeleteItem( int nItem );
protected:
  bool    NextItem( bool Forward );
  BOOL    EnsureVisible( int nItem );
  int     GetColumnOffset( int col );
  int     GetColumnCount()
  {
    return GetHeaderCtrl()->GetItemCount();
  }
  void    GetCellRect( int nItem , int nColumn , CRect& rectItem );
  virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
  bool    IsItemVisible( int Item );
  void    GetRowRectangle( int nItem , CRect&rc );
  int     GetCellID( int col , int row );
  CGridCell* GetGridCell( int c , int r )
  {
    if ( (r < 0) || (c < 0) ) return NULL;
    if ( r >= (int) m_Data.GetSize() ) return NULL;
    if ( c >= (int) m_Data[ r ].GetSize() ) return NULL;
    return (&(m_Data[ r ][ c ]));
  }

  CGridCell* GetGridCell( int nID )
  {
    int r = nID / GetColumnCount();
    int c = nID % GetColumnCount();
    return GetGridCell( c , r );
  }

  CGridCell* GetGridCell( LPCTSTR pRowName )
  {
    FXString Small( pRowName ) ;
    Small.MakeLower() ;
    for ( int i = 0 ; i < m_Data.GetCount() ; i++ )
    {
      CGridRow& Row = m_Data[ i ] ;
      FXString Name = Row[ 0 ].GetText() ;
      Name.MakeLower() ;
      CGridCell::CellType Type = Row[ 0 ].m_Type ;
      if ( Name == Small )
        return GetGridCell( ( Type != CGridCell::typeChk ) ? 1 : 2 , i ) ;
    }
    return NULL ;
  }

  int     GetNextItemID()
  {
    int pos = m_CurSel.y*GetColumnCount() + m_CurSel.x + 1;
    while ( (pos < GetColumnCount()*GetItemCount()) && (!IsItemActive( pos%GetColumnCount() , pos / GetColumnCount() )) )
      pos++;
    if ( pos < GetColumnCount()*GetItemCount() ) return pos;
    return -1;
  }
  void HideCtrls();
  //{{AFX_MSG(CGridListCtrl)
  afx_msg BOOL OnEraseBkgnd( CDC* pDC );
  afx_msg void OnDestroy();
  afx_msg void OnSetFocus( CWnd* pOldWnd );
  afx_msg void OnChar( UINT nChar , UINT nRepCnt , UINT nFlags );
  afx_msg void OnKeyDown( UINT nChar , UINT nRepCnt , UINT nFlags );
  afx_msg void OnKeyUp( UINT nChar , UINT nRepCnt , UINT nFlags );
  afx_msg void OnVScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar );
  //}}AFX_MSG

  afx_msg void OnButton( UINT nID );
  afx_msg void OnSelChange( UINT nID );
  afx_msg LRESULT OnNextItem( WPARAM wParam , LPARAM lParam );

  DECLARE_MESSAGE_MAP()
public:
  int InsertRow( CGridRow& row , int line );
  afx_msg void OnNcCalcSize( BOOL bCalcValidRects , NCCALCSIZE_PARAMS* lpncsp );
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDLISTCTRL_H__AF56AAD3_DC09_49E0_BA30_9716EC14FB96__INCLUDED_)
