// GridListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include <gadgets\stdsetup.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_CELLS_IN_CONTROL 10000

/////////////////////////////////////////////////////////////////////////////
// CGridCell

CGridCell::CGridCell()
{
  m_Type = typeNull;
  memset( &m_CBData , 0 , sizeof( m_CBData ) );
  m_Ctrl = NULL;
  m_uData = -1;
  m_Changed = false;
  m_Editable = false;
}

CGridCell::CGridCell( CGridCell& Src )
{
  m_Type = typeNull;
  m_pText = NULL;
  m_Ctrl = NULL;
  m_uData = -1;
  Copy( Src );
}

CGridCell::~CGridCell()
{
  Clear();
}

void CGridCell::Clear()
{
  switch ( m_Type )
  {
    case typeString:
    case typeIndent:
    case typeButton:
      if ( m_pText ) 
        free( m_pText );
      m_pText = NULL;
      break;
    case typeComboBox:
      if ( m_CBData.strlst ) 
        free( m_CBData.strlst );
      m_CBData.strlst = NULL;
      if ( m_CBData.inivalue ) 
        free( m_CBData.inivalue );
      m_CBData.inivalue = NULL;
      break;
    case typeChk:
      memset( &m_Check , 0 , sizeof( m_Check ) ) ;
      break ;
    case typeInt:
    case typeIntChk:
      memset( &m_pInt , 0 , sizeof( m_pInt ) ) ;
      break;
  }
  m_Type = typeNull;
  m_uData = 0;
  if ( m_Ctrl )
  {
    if ( m_Ctrl->GetSafeHwnd() ) 
      m_Ctrl->DestroyWindow();
    delete m_Ctrl;
    m_Ctrl = NULL;
  }
}

void CGridCell::Copy( CGridCell& Src )
{
  Clear();
  m_Type = Src.m_Type;
  switch ( m_Type )
  {
    case typeString:
    case typeIndent:
    case typeButton:
      m_pText = (LPTSTR) malloc( _tcsclen( Src.m_pText ) + 1 );
      _tcscpy( m_pText , Src.m_pText );
      break;
    case typeComboBox:
      m_CBData.strlst = (LPTSTR) malloc( _tcsclen( Src.m_CBData.strlst ) + 1 );
      _tcscpy( m_CBData.strlst , Src.m_CBData.strlst );
      m_CBData.inivalue = (LPTSTR) malloc( _tcsclen( Src.m_CBData.inivalue ) + 1 );
      _tcscpy( m_CBData.inivalue , Src.m_CBData.inivalue );
      break;
    case typeInt:
      m_pInt.value = Src.m_pInt.value;
      m_pInt.max = Src.m_pInt.max;
      m_pInt.min = Src.m_pInt.min;
      break;
    case typeIntChk:
      m_pInt.value = Src.m_pInt.value;
      m_pInt.max = Src.m_pInt.max;
      m_pInt.min = Src.m_pInt.min;
      m_pInt.enable = Src.m_pInt.enable ;
      break ;
    case typeChk:
      memcpy( &m_Check , &Src.m_Check , sizeof( m_Check ) ) ;
      break;
  }
  m_uData = Src.m_uData;
  m_Editable = Src.m_Editable;
  m_Ctrl = Src.m_Ctrl; 
  Src.m_Ctrl = NULL;
}

void CGridCell::DefineString( LPCTSTR src , bool Editable , int uData )
{
  Clear();
  m_Type = typeString;
  m_pText = (LPTSTR) malloc( sizeof( _TCHAR )*(_tcsclen( src ) + 1) );
  _tcscpy( m_pText , src );
  m_Editable = Editable;
  m_uData = uData;
}

void CGridCell::DefineIndent( LPCTSTR src )
{
  Clear();
  m_Type = typeIndent;
  m_pText = (LPTSTR) malloc( _tcsclen( src ) + 1 );
  _tcscpy( m_pText , src );
  m_Editable = false;
}

void CGridCell::DefineButton( LPCTSTR src , int uData )
{
  Clear();
  m_Type = typeButton;
  m_pText = (LPTSTR) malloc( sizeof( _TCHAR )*(_tcsclen( src ) + 1) );
  _tcscpy( m_pText , src );
  m_Editable = false;
  m_uData = uData;
}

void CGridCell::DefineListBox( LPCTSTR src , LPCTSTR initialVal , int uData )
{
  Clear();
  m_Type = typeComboBox;
  m_CBData.strlst = (LPTSTR) malloc( _tcsclen( src ) + 1 );
  _tcscpy( m_CBData.strlst , src );
  m_CBData.inivalue = (LPTSTR) malloc( sizeof( _TCHAR )*(_tcsclen( initialVal ) + 1) );
  _tcscpy( m_CBData.inivalue , initialVal );
  m_Editable = false;
  m_uData = uData;
}

void CGridCell::DefineInt( int value , int min , int max , int uData )
{
  Clear();
  m_Type = typeInt;
  m_pInt.value = value;
  m_pInt.max = max;
  m_pInt.min = min;
  m_uData = uData;
  m_Editable = true;
  m_bDisabled = false ;
}

void CGridCell::DefineIntChk( int value , 
  int min , int max , bool enable , int uData )
{
  Clear();
  m_Type = typeIntChk;
  m_pInt.value = value;
  m_pInt.max = max;
  m_pInt.min = min;
  m_pInt.enable = enable;
  m_uData = uData;
  m_Editable = !enable ;
}

void CGridCell::DefineChk( LPCTSTR name , bool value , int uData )
{
  Clear();
  m_Type = typeChk;
  _tcscpy_s( m_Check.name , name ) ;
  m_Check.state = value ;
  m_uData = uData;
  m_Editable = false;
}

LPCTSTR  CGridCell::GetText()
{
  if ( m_Type == typeString )  return m_pText ;
  if ( m_Type == typeIndent )  return m_pText ;
  if ( m_Type == typeButton )  return m_pText ;
  if ( m_Type == typeChk )     
    return m_Check.name ;
  if ( m_Type == typeComboBox ) return NULL;
  return NULL;
}
bool  CGridCell::SetText( LPCTSTR pText )
{
  switch ( m_Type )
  {
  case typeString:
  case typeIndent:
  case typeButton:
    {
      free( m_pText ) ;
      m_pText = (LPTSTR) malloc( sizeof( _TCHAR )*(_tcsclen( pText ) + 1) );
      _tcscpy( m_pText , pText );
      HWND hWnd = m_Ctrl->GetSafeHwnd() ;
      if ( hWnd )
      {
        SetWindowText( hWnd , m_pText ) ;
        return true ;
      }
      return false ;
    }
  case typeChk:
    {
      strcpy_s(m_Check.name , pText ) ;
      return true ;
    }
  default: return false ;
  }
}
bool  CGridCell::SetInt( int iValue )
{
  if ( m_Ctrl )
  {
    int iMin , iMax ;
    switch ( m_Type )
    {
    case typeInt:
      {
        ((CSpinCtrl*) m_Ctrl)->GetRange( iMin , iMax ) ;
        if ( iValue < iMin )
          iValue = iMin ;
        if ( iMax < iValue )
          iValue = iMax ;
//          ((CSpinCtrl*) m_Ctrl)->SetPos( iValue ) ;
         ( ( CSpinCtrl* ) m_Ctrl )->PostMessage( UDM_SETPOS32 , 0 , iValue ) ;
         //         TCHAR Buf[ 20 ] ;
//         _stprintf_s( Buf , "%d" , iValue ) ;
//         m_Ctrl->SetWindowText( Buf ) ;
        return true ;
      }
    case typeIntChk:
      {
        ((CSpinChkCtrl*) m_Ctrl)->GetRange( iMin , iMax ) ;
        if ( iValue < iMin )
          iValue = iMin ;
        if ( iMax < iValue )
          iValue = iMax ;
        m_pInt.value = iValue ;
        ((CSpinChkCtrl*) m_Ctrl)->SetPos( iValue ) ;
//         TCHAR Buf[ 20 ] ;
//         _stprintf_s( Buf , "%d" , iValue ) ;
//         m_Ctrl->SetWindowText( Buf ) ;
//        ( ( CSpinChkCtrl* ) m_Ctrl )->PostMessage( UDM_SETPOS32 , 0 , iValue ) ;
        return true ;
      }
    }
  }
  return false ;
}

bool  CGridCell::SetIntChk( int iValue , bool enable )
{
  if ( m_Ctrl )
  {
    int iMin , iMax ;
    switch ( m_Type )
    {
    case typeIntChk:
      {
        ((CSpinChkCtrl*) m_Ctrl)->GetRange( iMin , iMax ) ;
        if ( iValue < iMin )
          iValue = iMin ;
        if ( iMax < iValue )
          iValue = iMax ;
        ( ( CSpinCtrl* ) m_Ctrl )->PostMessage( UDM_SETPOS32 , 0 , iValue ) ;
//        ((CSpinChkCtrl*) m_Ctrl)->SetPos( iValue ) ;
        ((CSpinChkCtrl*) m_Ctrl)->SetCheck( enable != false ) ;
        return true ;
      }
    }
  }
  return false ;
}

bool  CGridCell::SetChk( bool enable )
{
  if ( m_Ctrl )
  {
    switch ( m_Type )
    {
    case typeIntChk:
      {
        ((CSpinChkCtrl*) m_Ctrl)->SetCheck( enable ) ;
        return true ;
      }
    case typeChk:
      {
        ((CChkCtrl*) m_Ctrl)->SetCheck( enable ) ;
        return true ;
      }
    }
  }
  return false ;
}


int      CGridCell::GetInt()
{
  if ( (m_Type == typeInt) && (m_Ctrl) )  return ((CSpinCtrl*) m_Ctrl)->GetPos();
  return -1;
}

bool    CGridCell::GetIntChk( int& val , bool& bAuto , BOOL * pEnabled )
{
  if ( (m_Type == typeIntChk) && (m_Ctrl) )
  {
    val = ((CSpinChkCtrl*) m_Ctrl)->GetPos();
    bAuto = ( ( CSpinChkCtrl* ) m_Ctrl )->GetCheck();
//    enable = m_pInt.enable ;
    ( ( CSpinChkCtrl* ) m_Ctrl )->Enable( !bAuto ) ;
    if ( pEnabled )
      *pEnabled = ( ( CSpinChkCtrl* ) m_Ctrl )->bIsCntlEnabled() ;
    return true;
  }
  return false;
}

bool    CGridCell::GetChk( bool& enable )
{
  if ( (m_Type == typeChk) && (m_Ctrl) )
  {
    enable = ((CChkCtrl*) m_Ctrl)->GetCheck();
    return true;
  }
  return false;
}

cbdata* CGridCell::GetCBData()
{
  if ( m_Type == typeComboBox ) return &m_CBData;
  return NULL;
}

void    CGridCell::SetDisabled( bool bDisabled )
{
  m_bDisabled = bDisabled ;
  switch ( m_Type )
  {
  case CGridCell::typeIntChk:
    ((CSpinChkCtrl*) m_Ctrl)->Enable( !bDisabled ) ;
    break;
  case CGridCell::typeChk:
    ((CChkCtrl*) m_Ctrl)->Enable( !bDisabled ) ;
    break;
  case CGridCell::typeInt:
    ((CSpinCtrl*) m_Ctrl)->Enable( !bDisabled ) ;
    break;
  default:
    break;
  }
}


/////////////////////////////////////////////////////////////////////////////
// CGridListCtrl

CGridListCtrl::CGridListCtrl() :
  m_CB( NULL ) ,
  m_IsLastItem( false ) ,
  m_IsFirstItem( false )
{
  m_FramePen.CreatePen( PS_SOLID , 1 , RGB( 200 , 200 , 200 ) );
  m_IndentBrush.CreateSolidBrush( RGB( 90 , 90 , 90 ) );
  m_Highlight.CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT ) );
  m_BckGrnd.CreateSolidBrush( ::GetSysColor( COLOR_BTNFACE ) );
  m_CurSel = CPoint( -1 , -1 );
}

CGridListCtrl::~CGridListCtrl()
{
  m_FramePen.DeleteObject();
  m_IndentBrush.DeleteObject();
  m_Highlight.DeleteObject();
  m_BckGrnd.DeleteObject();
}

BEGIN_MESSAGE_MAP( CGridListCtrl , CListCtrl )
  //{{AFX_MSG_MAP(CGridListCtrl)
  ON_WM_ERASEBKGND()
  ON_WM_DESTROY()
  ON_WM_SETFOCUS()
  ON_WM_CHAR()
  ON_WM_KEYDOWN()
  ON_WM_KEYUP()
  ON_WM_VSCROLL()
  //}}AFX_MSG_MAP
  ON_MESSAGE( WM_NEXTDLGCTL , OnNextItem )
  ON_CONTROL_RANGE( BN_CLICKED , 0 , MAX_CELLS_IN_CONTROL , OnButton )
  ON_CONTROL_RANGE( CBN_SELENDOK , 0 , MAX_CELLS_IN_CONTROL , OnSelChange )
  ON_WM_NCCALCSIZE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGridListCtrl message handlers

void CGridListCtrl::InitCtrl( GridEvent callback , LPVOID wParam )
{
  LOGFONT lf = {0};
  VERIFY( GetFont()->GetLogFont( &lf ) != 0 );
  VERIFY( m_GridFont.CreateFontIndirect( &lf ) );

  m_CB = callback; 
  m_CBwParam = wParam;

  lf.lfHeight = (int) (lf.lfHeight * 1.46);
  lf.lfWidth = (int) (lf.lfWidth);

  VERIFY( m_BigFont.CreateFontIndirect( &lf ) );

  CListCtrl::SetFont( &m_BigFont );
  ModifyStyle( 0 , LVS_OWNERDRAWFIXED );
  GetHeaderCtrl()->SetFont( &m_GridFont );
  CListCtrl::SetFont( &m_GridFont );
}

void CGridListCtrl::OnDestroy()
{
  while ( m_Data.GetSize() )
  {
    m_Data[ 0 ].RemoveAll();
    m_Data.RemoveAt( 0 );
  }
  m_GridFont.DeleteObject();
  m_BigFont.DeleteObject();
  CListCtrl::OnDestroy();
}

FXString CGridListCtrl::GetItemText( int col , int row )
{
  FXString retV;
  CGridCell* gc = GetGridCell( col , row );
  if ( (gc) && (gc->m_Ctrl) )
  {
    int len = gc->m_Ctrl->GetWindowTextLength();
    gc->m_Ctrl->GetWindowText( retV.GetBuffer( len + 1 ) , len + 1 );
  }
  else
  {
    LVITEM Item;
    Item.mask = LVIF_TEXT;
    Item.iItem = row;
    Item.iSubItem = col;
    CListCtrl::GetItem( &Item );
    retV = Item.pszText;
  }
  return retV;
}

bool CGridListCtrl::SetCellText( LPCTSTR pName , LPCTSTR pText )
{
  CGridCell* gc = GetGridCell( pName );
  if ( gc )
    return gc->SetText( pText ) ;
  return false ;
}

int CGridListCtrl::GetItemInt( int col , int row )
{
  CGridCell* gc = GetGridCell( col , row );
  if ( (gc) && (gc->m_Ctrl) )
  {
    if ( gc->GetType() == CGridCell::typeInt )
    {
      return gc->GetInt();
    }
  }
  return -1;
}

bool CGridListCtrl::SetCellInt( LPCTSTR pName , int iValue )
{
  CGridCell* gc = GetGridCell( pName );
  if ( (gc) && (gc->m_Ctrl) )
    return gc->SetInt( iValue ) ;
  return false ;
}
bool CGridListCtrl::GetItemIntChk( int col , int row , 
  int &val , bool &bAuto , BOOL * pEnabled )
{
  CGridCell* gc = GetGridCell( col , row );
  if ( (gc) && (gc->m_Ctrl) )
  {
    if ( gc->GetType() == CGridCell::typeIntChk )
      return gc->GetIntChk( val , bAuto , pEnabled );
    }
  return false;
}

int CGridListCtrl::SetCellIntValue( LPCTSTR pName , LPCTSTR pValueAsString )
{
  CGridCell* gc = GetGridCell( pName );
  if ( ( gc ) && ( gc->m_Ctrl ) )
  {
    if ( gc->GetType() == CGridCell::typeIntChk 
      || gc->GetType() == CGridCell::typeInt )
    {
      const char * pAuto = _tcsstr( pValueAsString , "auto" ) ;
      int iVal = atoi( pAuto ? pAuto + 5 : pValueAsString ) ;
      gc->SetInt( iVal ) ;
    }
  }
  return 0;
}

bool CGridListCtrl::GetItemChk( int col , int row , bool &enable )
{
  CGridCell* gc = GetGridCell( col , row );
  if ( (gc) && (gc->m_Ctrl) )
  {
    if ( gc->GetType() == CGridCell::typeChk )
      return gc->GetChk( enable );
    }
  return false;
  }

bool CGridListCtrl::SetCellChk( LPCTSTR pName , bool enable )
{
  CGridCell* gc = GetGridCell( pName );
  if ( (gc) && (gc->m_Ctrl) )
    return gc->SetChk( enable ) ;

  return false;
}

FXString CGridListCtrl::GetItemData( int col , int row )
{
  FXString retV;
  CGridCell* gc = GetGridCell( col , row );
  if ( (gc) && (gc->m_Ctrl) )
  {
    if ( gc->GetType() == CGridCell::typeComboBox )
    {
      int sel = ((CComboBox*) (gc->m_Ctrl))->GetCurSel();
      FXParser dt( m_Data[ row ][ col ].m_CBData.strlst );
      FXString key;
      FXParser param;
      dt.GetElementNo( sel , key , param );
      retV = param;
      return retV;
    }
  }
  return retV;
}

int CGridListCtrl::GetCellID( int col , int row )
{
  return row * GetColumnCount() + col;
}

int CGridListCtrl::InsertColumn( int nCol , LPCTSTR lpszColumnHeading , int nFormat , int nWidth , int nSubItem )
{
  ASSERT( GetItemCount() == 0 );
  return CListCtrl::InsertColumn( nCol , lpszColumnHeading , nFormat , nWidth , nSubItem );
}

int CGridListCtrl::AddRow( CGridRow& row )
{
  int retV;
  if ( GetColumnCount()*(GetItemCount() + 1) > MAX_CELLS_IN_CONTROL 
    || row.GetSize() == 0 ) 
  {
    return -1;
  }
  for ( int i = 0; ((i < (int) row.GetSize()) && (i < GetColumnCount())); i++ )
  {
    if ( row[ i ].GetType() == CGridCell::typeString )
    {
      if ( row[ i ].IsEditable() )
      {
        if ( i == 0 )
          retV = InsertItem( GetItemCount() , _T( "" ) );
        else
          SetItemText( retV , i , _T( "" ) );
        row[ i ].m_Ctrl = new CEdit;
        ((CEdit*) row[ i ].m_Ctrl)->Create( 
          ES_LEFT | ES_AUTOHSCROLL | ES_WANTRETURN | WS_CHILD | WS_TABSTOP , 
          CRect( 0 , 0 , 0 , 0 ) , 
          this , 
          GetCellID( i , GetItemCount() - 1 ) );
        ((CEdit*) row[ i ].m_Ctrl)->SetFont( &m_GridFont );
        ((CEdit*) row[ i ].m_Ctrl)->SetWindowText( row[ i ].GetText() );
      }
      else
      {
        if ( i == 0 )
          retV = InsertItem( GetItemCount() , row[ i ].GetText() );
        else
          SetItemText( retV , i , row[ i ].GetText() );
      }
    }
    else if ( row[ i ].GetType() == CGridCell::typeButton )
    {
      if ( i == 0 )
        retV = InsertItem( GetItemCount() , _T( "" ) );
      else
        SetItemText( retV , i , _T( "" ) );
      row[ i ].m_Ctrl = new CButton;
      ((CButton*) row[ i ].m_Ctrl)->Create( row[ i ].GetText() , 
        WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP , 
        CRect( 0 , 0 , 0 , 0 ) , 
        this , 
        GetCellID( i , GetItemCount() - 1 ) );
      ((CButton*) row[ i ].m_Ctrl)->SetFont( &m_GridFont );
    }
    else if ( row[ i ].GetType() == CGridCell::typeComboBox )
    {
      if ( i == 0 )
        retV = InsertItem( GetItemCount() , _T( "" ) );
      else
        SetItemText( retV , i , _T( "" ) );
      row[ i ].m_Ctrl = new CComboBox;
      ((CComboBox*) row[ i ].m_Ctrl)->Create( 
        CBS_DROPDOWNLIST | WS_CHILD , CRect( 0 , 0 , 100 , 800 ) , 
        this , 
        GetCellID( i , GetItemCount() - 1 ) );
      ((CComboBox*) row[ i ].m_Ctrl)->SetFont( &m_GridFont );
      FXParser cmd = row[ i ].GetCBData()->strlst , param;
      FXString key;
      int pos = 0;
      int sel = -1;
      while ( cmd.GetElementNo( pos , key , param ) )
      {
        ((CComboBox*) row[ i ].m_Ctrl)->AddString( FxUnregularize( key ) );
        if ( param == row[ i ].GetCBData()->inivalue )
          sel = pos;
        pos++;
      }
      if ( pos == 0 )
      {
        CRect rctComboBox , rctDropDown;
        ((CComboBox*) row[ i ].m_Ctrl)->GetClientRect( &rctComboBox );
        ((CComboBox*) row[ i ].m_Ctrl)->GetDroppedControlRect( &rctDropDown );

        int itemHeight = ((CComboBox*) row[ i ].m_Ctrl)->GetItemHeight( -1 );
        ((CComboBox*) row[ i ].m_Ctrl)->GetParent()->ScreenToClient( &rctDropDown );
        rctDropDown.bottom = rctDropDown.top + rctComboBox.Height() + itemHeight * 4;
        ((CComboBox*) row[ i ].m_Ctrl)->MoveWindow( &rctDropDown );
      }
      if ( sel >= 0 )
        ((CComboBox*) row[ i ].m_Ctrl)->SetCurSel( sel );
    }
    else if ( row[ i ].GetType() == CGridCell::typeInt )
    {
      if ( i == 0 )
        retV = InsertItem( GetItemCount() , _T( "" ) );
      else
        SetItemText( retV , i , _T( "typeInt" ) );

      row[ i ].m_Ctrl = new CSpinCtrl;
      ((CSpinCtrl*) row[ i ].m_Ctrl)->Create( WS_CHILD | WS_TABSTOP | SS_NOTIFY , 
        CRect( 0 , 0 , 0 , 0 ) , 
        this , 
        GetCellID( i , GetItemCount() - 1 ) );
      ((CSpinCtrl*) row[ i ].m_Ctrl)->SetRange( row[ i ].m_pInt.min , row[ i ].m_pInt.max );
      ((CSpinCtrl*) row[ i ].m_Ctrl)->SetPos( row[ i ].m_pInt.value );
    }
    else if ( row[ i ].GetType() == CGridCell::typeIntChk )
    {
      if ( i == 0 )
        retV = InsertItem( GetItemCount() , _T( "" ) );
      else
        SetItemText( retV , i , _T( "typeIntChk" ) );

      row[ i ].m_Ctrl = new CSpinChkCtrl;
      ((CSpinChkCtrl*) row[ i ].m_Ctrl)->Create(
        WS_CHILD | WS_TABSTOP | SS_NOTIFY ,
        CRect( 0 , 0 , 0 , 0 ) ,
        this ,
        GetCellID( i , GetItemCount() - 1 ) );
      ((CSpinChkCtrl*) row[ i ].m_Ctrl)->SetRange( row[ i ].m_pInt.min , row[ i ].m_pInt.max );
      ((CSpinChkCtrl*) row[ i ].m_Ctrl)->SetPos( row[ i ].m_pInt.value );
      ((CSpinChkCtrl*) row[ i ].m_Ctrl)->SetCheck( row[ i ].m_pInt.enable );
      ( ( CSpinChkCtrl* ) row[ i ].m_Ctrl )->Enable( !row[ i ].m_pInt.enable );

    }
    else if ( row[ i ].GetType() == CGridCell::typeChk )
    {
      if ( i == 0 )
        retV = InsertItem( GetItemCount() , _T( "" ) );
      else
        SetItemText( retV , i , _T( "typeChk" ) );

      row[ i ].m_Ctrl = new CChkCtrl;
      LPCTSTR pName = row[ i ].m_Check.name ;
      ((CChkCtrl*) row[ i ].m_Ctrl)->Create(
        WS_CHILD | WS_TABSTOP | SS_NOTIFY ,
        CRect( 0 , 0 , 0 , 0 ) ,
        this ,
        GetCellID( i , GetItemCount() - 1 ) , row[ i ].GetText() );
      ((CChkCtrl*) row[ i ].m_Ctrl)->SetCheck( row[ i ].m_Check.state );
    }
    else if ( row[ i ].GetType() == CGridCell::typeIndent )
    {
      if ( i == 0 )
        retV = InsertItem( GetItemCount() , row[ i ].GetText() );
      else
        SetItemText( retV , i , row[ i ].GetText() );
    }
    if ( (i > 0) && row.m_bGrayed )
    {
      row[ i ].SetDisabled( row.m_bGrayed ) ;
      row[ i ].m_Ctrl->EnableWindow( FALSE ) ;
    }
  }
  m_Data.Add( row );
  return retV;
}

int CGridListCtrl::InsertRow( CGridRow &row , int line )
{
  int retV;
  if ( GetColumnCount()*(GetItemCount() + 1) > MAX_CELLS_IN_CONTROL ) return -1;
  for ( int i = 0; ((i < (int) row.GetSize()) && (i < GetColumnCount())); i++ )
  {
    if ( row[ i ].GetType() == CGridCell::typeString )
    {
      if ( row[ i ].IsEditable() )
      {
        if ( i == 0 )
          retV = InsertItem( line , _T( "" ) );
        else
          SetItemText( retV , i , _T( "" ) );
        row[ i ].m_Ctrl = new CEdit;
        ((CEdit*) row[ i ].m_Ctrl)->Create( 
          ES_LEFT | ES_AUTOHSCROLL | ES_WANTRETURN | WS_CHILD | WS_TABSTOP , 
          CRect( 0 , 0 , 0 , 0 ) , 
          this , 
          GetCellID( i , line ) );
        ((CEdit*) row[ i ].m_Ctrl)->SetFont( &m_GridFont );
        ((CEdit*) row[ i ].m_Ctrl)->SetWindowText( row[ i ].GetText() );
      }
      else
      {
        if ( i == 0 )
          retV = InsertItem( line , row[ i ].GetText() );
        else
          SetItemText( retV , i , row[ i ].GetText() );
      }
    }
    else if ( row[ i ].GetType() == CGridCell::typeButton )
    {
      if ( i == 0 )
        retV = InsertItem( line , _T( "" ) );
      else
        SetItemText( retV , i , _T( "" ) );
      row[ i ].m_Ctrl = new CButton;
      ((CButton*) row[ i ].m_Ctrl)->Create( row[ i ].GetText() , 
        WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP , 
        CRect( 0 , 0 , 0 , 0 ) , 
        this , 
        GetCellID( i , line ) );
      ((CButton*) row[ i ].m_Ctrl)->SetFont( &m_GridFont );
    }
    else if ( row[ i ].GetType() == CGridCell::typeComboBox )
    {
      if ( i == 0 )
        retV = InsertItem( line , _T( "" ) );
      else
        SetItemText( retV , i , _T( "" ) );
      row[ i ].m_Ctrl = new CComboBox;
      ((CComboBox*) row[ i ].m_Ctrl)->Create( CBS_DROPDOWNLIST | WS_CHILD , 
        CRect( 0 , 0 , 100 , 800 ) , 
        this , 
        GetCellID( i , line ) );
      ((CComboBox*) row[ i ].m_Ctrl)->SetFont( &m_GridFont );
      FXParser cmd = row[ i ].GetCBData()->strlst , param;
      FXString key;
      int pos = 0;
      int sel = -1;
      while ( cmd.GetElementNo( pos , key , param ) )
      {
        ((CComboBox*) row[ i ].m_Ctrl)->AddString( key );
        if ( param == row[ i ].GetCBData()->inivalue )
          sel = pos;
        pos++;
      }
      if ( sel >= 0 )
        ((CComboBox*) row[ i ].m_Ctrl)->SetCurSel( sel );
    }
    else if ( row[ i ].GetType() == CGridCell::typeInt )
    {
      if ( i == 0 )
        retV = InsertItem( line , _T( "" ) );
      else
        SetItemText( retV , i , _T( "typeInt" ) );

      row[ i ].m_Ctrl = new CSpinCtrl;
      ((CSpinCtrl*) row[ i ].m_Ctrl)->Create( 
        WS_CHILD | WS_TABSTOP | SS_NOTIFY , 
        CRect( 0 , 0 , 0 , 0 ) , 
        this , 
        GetCellID( i , line ) );
      ((CSpinCtrl*) row[ i ].m_Ctrl)->SetRange( row[ i ].m_pInt.min , row[ i ].m_pInt.max );
      ((CSpinCtrl*) row[ i ].m_Ctrl)->SetPos( row[ i ].m_pInt.value );
    }
    else if ( row[ i ].GetType() == CGridCell::typeIntChk )
    {
      if ( i == 0 )
        retV = InsertItem( line , _T( "" ) );
      else
        SetItemText( retV , i , _T( "typeIntChk" ) );

      row[ i ].m_Ctrl = new CSpinChkCtrl;
      ((CSpinChkCtrl*) row[ i ].m_Ctrl)->Create( 
        WS_CHILD | WS_TABSTOP | SS_NOTIFY , 
        CRect( 0 , 0 , 0 , 0 ) , 
        this , 
        GetCellID( i , line ) );
      ((CSpinChkCtrl*) row[ i ].m_Ctrl)->SetRange( row[ i ].m_pInt.min , row[ i ].m_pInt.max );
      ((CSpinChkCtrl*) row[ i ].m_Ctrl)->SetPos( row[ i ].m_pInt.value );
      ((CSpinChkCtrl*) row[ i ].m_Ctrl)->SetCheck( row[ i ].m_pInt.enable );
    }
    else if ( row[ i ].GetType() == CGridCell::typeChk )
    {
      if ( i == 0 )
        retV = InsertItem( line , _T( "" ) );
      else
        SetItemText( retV , i , _T( "typeChk" ) );

      row[ i ].m_Ctrl = new CChkCtrl;
      ((CChkCtrl*) row[ i ].m_Ctrl)->Create( 
        WS_CHILD | WS_TABSTOP | SS_NOTIFY , 
        CRect( 0 , 0 , 0 , 0 ) , 
        this , 
        GetCellID( i , line ) );

      ((CChkCtrl*) row[ i ].m_Ctrl)->SetCheck( row[ i ].m_Check.state );
    }
    else if ( row[ i ].GetType() == CGridCell::typeIndent )
    {
      if ( i == 0 )
        retV = InsertItem( line , row[ i ].GetText() );
      else
        SetItemText( retV , i , row[ i ].GetText() );
    }
  }
  m_Data.InsertAt( line , row );
  return retV;
}

bool CGridListCtrl::IsItemVisible( int Item )
{
  CRect wRect , iRect , intrsctRct;
  GetClientRect( wRect );
  GetRowRectangle( Item , iRect );
  if ( !intrsctRct.IntersectRect( iRect , wRect ) ) return false;
  if ( intrsctRct.Height() < iRect.Height() ) return false;
  return true;
}

int CGridListCtrl::GetColumnOffset( int col )
{
  int retV = 0;

  if ( col > GetColumnCount() ) return -1;

  for ( int i = 0; i < col; i++ )
  {
    retV += GetColumnWidth( i );
  }
  return retV;
}

void CGridListCtrl::GetRowRectangle( int nItem , CRect &rc )
{
  int lastCol = GetColumnCount() - 1;
  if ( lastCol < 0 )
  {
    rc.SetRectEmpty();
    return;
  }
  GetItemRect( nItem , rc , LVIR_BOUNDS );
  rc.left = 0;
  rc.right = GetColumnOffset( lastCol ) + GetColumnWidth( lastCol );
}


void CGridListCtrl::GetCellRect( int nItem , int nColumn , CRect& rectCell )
{
  int nWidth = 0;
  int cx = 0;

  GetItemRect( nItem , rectCell , LVIR_LABEL );

  if ( 0 == nColumn )
  {
    nWidth = GetColumnWidth( 0 );
    cx = 0;
  }
  else
  {
    cx = GetColumnOffset( nColumn );
    nWidth = GetColumnWidth( nColumn );
  }
  rectCell.left = cx;
  rectCell.right = rectCell.left + nWidth + 1;
  rectCell.bottom += 1;
}

void CGridListCtrl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
  CDC*		pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
  void*		MyVoid = NULL;
  CWnd*		pWnd = NULL;
  CRect		rcItem;

  int ItemNo = lpDrawItemStruct->itemID;
  //TRACE_GRID("+++ It's about to update line %d\n",ItemNo);
  for ( int i = 0; i < GetColumnCount(); i++ )
  {
    if ( GetGridCell( i , ItemNo ) 
      && (GetGridCell( i , ItemNo )->GetType() == CGridCell::typeIndent) )
    {
      GetRowRectangle( ItemNo , rcItem );
      CBrush* oldBrush = pDC->SelectObject( &m_IndentBrush );
      pDC->Rectangle( rcItem );
      FXString ItemText = (LPCTSTR)CListCtrl::GetItemText( ItemNo , i );
      COLORREF oldCol = pDC->SetTextColor( RGB( 255 , 255 , 255 ) );
      pDC->DrawText( (LPCTSTR) ItemText , (int) ItemText.GetLength() , 
        rcItem , DT_SINGLELINE | DT_LEFT | DT_VCENTER );
      pDC->SetTextColor( oldCol );
      pDC->SelectObject( oldBrush );
      break;
    }
    else if ( GetGridCell( i , ItemNo ) 
      && (GetGridCell( i , ItemNo )->GetType() == CGridCell::typeString) 
      && (GetGridCell( i , ItemNo )->m_Ctrl != NULL) )
    {
      if ( IsItemVisible( ItemNo ) )
      {
        GetCellRect( ItemNo , i , rcItem );
        CPen* oldPen = pDC->SelectObject( &m_FramePen );
        pDC->Rectangle( rcItem );
        pDC->SelectObject( oldPen );
        rcItem.DeflateRect( 1 , 1 );
        TRACE_GRID( "+++ show CEdit (%d:%d) at %d,%d\n" , ItemNo , i , rcItem.top , rcItem.bottom );
        if ( m_Data[ ItemNo ][ i ].m_Ctrl->GetSafeHwnd() )
        {
          m_Data[ ItemNo ][ i ].m_Ctrl->MoveWindow( rcItem );
          m_Data[ ItemNo ][ i ].m_Ctrl->ShowWindow( SW_SHOW );
        }
      }
    }
    else if ( GetGridCell( i , ItemNo ) 
      && (GetGridCell( i , ItemNo )->GetType() == CGridCell::typeButton) 
      && (GetGridCell( i , ItemNo )->m_Ctrl != NULL) )
    {
      GetCellRect( ItemNo , i , rcItem );
      if ( IsItemVisible( ItemNo ) )
      {
        TRACE_GRID( "+++ show CButton (%d:%d) at %d,%d\n" , ItemNo , i , rcItem.top , rcItem.bottom );
        if ( m_Data[ ItemNo ][ i ].m_Ctrl->GetSafeHwnd() )
        {
          m_Data[ ItemNo ][ i ].m_Ctrl->MoveWindow( rcItem );
          m_Data[ ItemNo ][ i ].m_Ctrl->ShowWindow( SW_SHOW );
        }
      }
    }
    else if ( GetGridCell( i , ItemNo ) 
      && (GetGridCell( i , ItemNo )->GetType() == CGridCell::typeInt) 
      && (GetGridCell( i , ItemNo )->m_Ctrl != NULL) )
    {
      GetCellRect( ItemNo , i , rcItem );
      if ( IsItemVisible( ItemNo ) )
      {
        TRACE_GRID( "+++ show CSpinButtonCtrl (%d:%d) at %d,%d\n" , ItemNo , i , rcItem.top , rcItem.bottom );
        if ( m_Data[ ItemNo ][ i ].m_Ctrl->GetSafeHwnd() )
        {
          rcItem.DeflateRect( 0 , 1 );
          m_Data[ ItemNo ][ i ].m_Ctrl->MoveWindow( rcItem );
          m_Data[ ItemNo ][ i ].m_Ctrl->ShowWindow( SW_SHOW );
        }
      }
    }
    else if ( GetGridCell( i , ItemNo )
      && (GetGridCell( i , ItemNo )->GetType() == CGridCell::typeIntChk)
      && (GetGridCell( i , ItemNo )->m_Ctrl != NULL) )
    {
      GetCellRect( ItemNo , i , rcItem );
      if ( IsItemVisible( ItemNo ) )
      {
        TRACE_GRID( "+++ show CSpinButtonCtrl (%d:%d) at %d,%d\n" , ItemNo , i , rcItem.top , rcItem.bottom );
        if ( m_Data[ ItemNo ][ i ].m_Ctrl->GetSafeHwnd() )
        {
          rcItem.DeflateRect( 0 , 1 );
          m_Data[ ItemNo ][ i ].m_Ctrl->MoveWindow( rcItem );
          m_Data[ ItemNo ][ i ].m_Ctrl->ShowWindow( SW_SHOW );
        }
      }
    }
    else if ( GetGridCell( i , ItemNo )
      && (GetGridCell( i , ItemNo )->GetType() == CGridCell::typeChk)
      && (GetGridCell( i , ItemNo )->m_Ctrl != NULL) )
    {
      GetCellRect( ItemNo , i , rcItem );
      if ( IsItemVisible( ItemNo ) )
      {
        TRACE_GRID( "+++ show CCheckButtonCtrl (%d:%d) at %d,%d\n" , ItemNo , i , rcItem.top , rcItem.bottom );
        if ( m_Data[ ItemNo ][ i ].m_Ctrl->GetSafeHwnd() )
        {
          rcItem.DeflateRect( 0 , 1 );
          m_Data[ ItemNo ][ i ].m_Ctrl->MoveWindow( rcItem );
          m_Data[ ItemNo ][ i ].m_Ctrl->ShowWindow( SW_SHOW );
        }
      }
    }
    else if ( GetGridCell( i , ItemNo )
      && (GetGridCell( i , ItemNo )->GetType() == CGridCell::typeComboBox) 
      && (GetGridCell( i , ItemNo )->m_Ctrl != NULL) )
    {
      GetCellRect( ItemNo , i , rcItem );
      if ( IsItemVisible( ItemNo ) )
      {
        TRACE_GRID( "+++ show CComboBox (%d:%d) at %d,%d\n" , ItemNo , i , rcItem.top , rcItem.bottom );
        if ( m_Data[ ItemNo ][ i ].m_Ctrl->GetSafeHwnd() )
        {
          CRect rc;
          m_Data[ ItemNo ][ i ].m_Ctrl->GetClientRect( rc );
          rcItem.bottom = rcItem.top + rc.Height();
          m_Data[ ItemNo ][ i ].m_Ctrl->MoveWindow( rcItem );
          m_Data[ ItemNo ][ i ].m_Ctrl->ShowWindow( SW_SHOW );
        }
      }
    }
    else
    {
      GetCellRect( ItemNo , i , rcItem );
      CBrush* oldBrush;
      if ( GetItemState( ItemNo , LVIS_SELECTED ) )
      {
        oldBrush = pDC->SelectObject( &m_Highlight );
      }
      else
      {
        oldBrush = pDC->SelectObject( &m_BckGrnd );
      }

      CPen* oldPen = pDC->SelectObject( &m_FramePen );
      pDC->Rectangle( rcItem );

      pDC->SelectObject( oldPen );
      pDC->SelectObject( oldBrush );

      rcItem.left += 2;
      rcItem.top += 1;
      FXString ItemText = (LPCTSTR) CListCtrl::GetItemText( ItemNo , i );

      pDC->DrawText( (LPCTSTR) ItemText , (int) ItemText.GetLength() , rcItem , DT_SINGLELINE | DT_LEFT | DT_VCENTER );
    }
  }
}

void CGridListCtrl::OnButton( UINT nID )
{
  int col = nID % GetColumnCount();
  int row = nID / GetColumnCount();
  if ( row < GetItemCount() )
  {
    if ( ((int) m_Data[ row ].GetSize() > col) && (m_Data[ row ][ col ].m_Type == CGridCell::typeButton) )
    {
      if ( m_CB ) m_CB( SC_BUTTON , m_CBwParam , col , row , m_Data[ row ][ col ].m_uData );
    }
  }
}

void CGridListCtrl::OnSelChange( UINT nID )
{
  int col = nID % GetColumnCount();
  int row = nID / GetColumnCount();
  if ( row < GetItemCount() )
  {
    if ( ((int) m_Data[ row ].GetSize() > col) && (m_CBwParam , m_Data[ row ][ col ].m_Ctrl) )
    {
      int nVal = ((CComboBox*) (m_CBwParam , m_Data[ row ][ col ].m_Ctrl))->GetCurSel();
      TRACE_GRID( "OnSelChange %d %d changed to %d\n" , col , row , nVal );
      if ( m_CB ) m_CB( SC_SELCHANGED , m_CBwParam , col , row , m_Data[ row ][ col ].m_uData );
    }
  }
}

BOOL CGridListCtrl::EnsureVisible( int nItem )
{
  if ( !IsItemVisible( nItem ) )
  {
    TRACE_GRID( "Call EnsureVisible item %d \n" , nItem );
    CListCtrl::EnsureVisible( nItem , FALSE );
    Invalidate();
  }
  return TRUE;
}

BOOL CGridListCtrl::OnEraseBkgnd( CDC* pDC )
{
  CRect rc;
  GetClientRect( rc );
  pDC->FillRect( rc , &m_BckGrnd );
  return TRUE;
}

bool CGridListCtrl::HasElement( int c , int r )
{
  if ( (r < 0) || (c < 0) ) return false;
  if ( r >= (int) m_Data.GetSize() ) return false;
  if ( c >= (int) m_Data[ r ].GetSize() ) return false;
  return true;
}


bool CGridListCtrl::SelectItem( int col , int row )
{
  if ( !HasElement( col , row ) )
  {
    return false;
  }
  if ( (m_Data[ row ][ col ].m_Ctrl) && (m_Data[ row ][ col ].m_Ctrl->GetSafeHwnd()) )
  {
    m_Data[ row ][ col ].m_Ctrl->SetFocus();
    m_CurSel.x = col; m_CurSel.y = row;
    m_IsLastItem = (GetNextItemID() == -1);
  }
  return true;
}

void CGridListCtrl::OnSetFocus( CWnd* pOldWnd )
{
  TRACE_GRID( "+++ OnSetFocus()\n" );
  CListCtrl::OnSetFocus( pOldWnd );
  if ( (m_CurSel.x < 0) || (m_CurSel.y < 0) )
  {
    EnsureVisible( 0 );
    HideCtrls();
  }
  if ( !HasElement( m_CurSel.x , m_CurSel.y ) )
    NextItem( true );
  if ( !HasElement( m_CurSel.x , m_CurSel.y ) )
    NextItem( false );
  SelectItem( m_CurSel.x , m_CurSel.y );
}

void CGridListCtrl::OnChar( UINT nChar , UINT nRepCnt , UINT nFlags )
{
  TRACE_GRID( ">>>On char : %d\n" , nChar );
  CListCtrl::OnChar( nChar , nRepCnt , nFlags );
}

void CGridListCtrl::OnKeyDown( UINT nChar , UINT nRepCnt , UINT nFlags )
{
  switch ( nChar )
  {
    case VK_RIGHT:
      TRACE_GRID( "+++ Right captured\n" );
      NextItem( true );
      return;	// Do not allow scroll
    case VK_LEFT:
      TRACE_GRID( "+++ Left captured\n" );
      NextItem( false );
      return;	// Do not allow scroll
    case VK_DOWN:
      TRACE_GRID( "+++ Down captured\n" );
      NextItem( true );
      return;	// Do not allow scroll
    case VK_UP:
      TRACE_GRID( "+++ Up captured\n" );
      NextItem( true );
      return;	// Do not allow scroll
    case VK_TAB:
      TRACE_GRID( "+++ Tab captured\n" );
      NextItem( true );
      return;
    case VK_NEXT:
      TRACE_GRID( "+++ Next captured\n" );
      NextItem( true );
      return;
  }
  CListCtrl::OnKeyDown( nChar , nRepCnt , nFlags );
}

void CGridListCtrl::OnKeyUp( UINT nChar , UINT nRepCnt , UINT nFlags )
{
  CListCtrl::OnKeyUp( nChar , nRepCnt , nFlags );
}

bool CGridListCtrl::NextItem( bool Forward )
{
  if ( (m_CurSel.x < 0) || (m_CurSel.y < 0) )
  {
    m_CurSel.x = -1;
    m_CurSel.y = 0;
  }
  int pos = GetNextItemID();
  if ( pos != -1 )
  {
    SelectItem( pos%GetColumnCount() , pos / GetColumnCount() );
    EnsureVisible( pos / GetColumnCount() );
    m_IsLastItem = (GetNextItemID() == -1);
  }
  TRACE_GRID( "Selected item %d, %d\n" , pos / GetColumnCount() , pos%GetColumnCount() );
  return true;
}

BOOL CGridListCtrl::PreTranslateMessage( MSG* pMsg )
{
  if ( (pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_TAB) )
  {
    TRACE_GRID( "WM_KEYDOWN\n" );
    if ( !m_IsLastItem )
    {
      NextItem( true );
      return TRUE;
    }
    m_CurSel.x = -1;
    m_CurSel.y = 0;
    m_IsLastItem = false;
  }
  return CListCtrl::PreTranslateMessage( pMsg );
}

bool CGridListCtrl::IsItemActive( int c , int r )
{
  if ( (r < 0) || (c < 0) ) return false;
  if ( r >= (int) m_Data.GetSize() ) return false;
  if ( c >= (int) m_Data[ r ].GetSize() ) return false;
  return (m_Data[ r ][ c ].m_Ctrl != NULL);
}

BOOL CGridListCtrl::OnCommand( WPARAM wParam , LPARAM lParam )
{
  int nID = LOWORD( wParam );
  int msg = HIWORD( wParam );

  CGridCell* gc = GetGridCell( nID );

  TRACE_GRID( "+++ CGridListCtrl::OnCommand ID=0x%x msg=0x%x ctrl=0x%x\n" , nID , msg , lParam );
  if ( (gc) && (gc->GetType() == CGridCell::typeString) && (gc->m_Ctrl->m_hWnd == (HWND) lParam) )
  {
    switch ( msg )
    {
      case EN_SETFOCUS:
      {
        int r = nID / GetColumnCount();
        int c = nID % GetColumnCount();
        m_CurSel.x = c; m_CurSel.y = r;
        m_IsLastItem = (GetNextItemID() == -1);
        break;
      }
      case EN_CHANGE:
        gc->SetChanged();
        break;
      case EN_KILLFOCUS:
      {
        int r = nID / GetColumnCount();
        int c = nID % GetColumnCount();
        if ( (gc->IsChanged()) && (m_CB) )
          m_CB( SC_EDITLSTFOCUS , m_CBwParam , c , r , gc->m_uData );
        break;
      }
    }
  }
  else if ( (gc) && (gc->GetType() == CGridCell::typeInt) && (gc->m_Ctrl->m_hWnd == (HWND) lParam) )
  {
    switch ( msg )
    {
      case EN_SETFOCUS:
      {
        int r = nID / GetColumnCount();
        int c = nID % GetColumnCount();
        m_CurSel.x = c; m_CurSel.y = r;
        m_IsLastItem = (GetNextItemID() == -1);
        break;
      }
      case EN_CHANGE:
      {
        int r = nID / GetColumnCount();
        int c = nID % GetColumnCount();
        if ( m_CB )
          m_CB( SC_INTCHANGE , m_CBwParam , c , r , gc->m_uData );
        break;
      }
    }
  }
  else if ( (gc) && (gc->GetType() == CGridCell::typeIntChk) && (gc->m_Ctrl->m_hWnd == (HWND) lParam) )
  {
    switch ( msg )
    {
    case EN_SETFOCUS:
      {
        int r = nID / GetColumnCount();
        int c = nID % GetColumnCount();
        m_CurSel.x = c; m_CurSel.y = r;
        m_IsLastItem = (GetNextItemID() == -1);
        break;
      }
    case EN_CHANGE:
    case BN_CLICKED:
      {
        int r = nID / GetColumnCount();
        int c = nID % GetColumnCount();
        if ( m_CB )
          m_CB( SC_INTCHKCHANGE , m_CBwParam , c , r , gc->m_uData );
        break;
      }
    }
  }
  else if ( (gc) && (gc->GetType() == CGridCell::typeChk) && (gc->m_Ctrl->m_hWnd == (HWND) lParam) )
  {
    switch ( msg )
    {
    case EN_SETFOCUS:
      {
        int r = nID / GetColumnCount();
        int c = nID % GetColumnCount();
        m_CurSel.x = c; m_CurSel.y = r;
        m_IsLastItem = (GetNextItemID() == -1);
        break;
      }
    case BN_CLICKED:
      {
        int r = nID / GetColumnCount();
        int c = nID % GetColumnCount();
        if ( m_CB )
          m_CB( SC_CHKCHANGE , m_CBwParam , c , r , gc->m_uData );
        break;
      }
    }
  }
  if ( ::IsWindow( (HWND) lParam ) )
    return CListCtrl::OnCommand( wParam , lParam );
  return 0;
}

LRESULT CGridListCtrl::OnNextItem( WPARAM wParam , LPARAM lParam )
{
  NextItem( TRUE );
  return TRUE;
}

void CGridListCtrl::OnVScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar )
{
  int pos = GetScrollPos( SB_VERT );
  TRACE_GRID( "<<< OnVScroll %d - %d\n" , pos , (int) nPos );
  if ( pos != m_VLastScrollPos )
  {
    m_VLastScrollPos = pos;
    HideCtrls();
  }
  CListCtrl::OnVScroll( nSBCode , nPos , pScrollBar );
}

void CGridListCtrl::HideCtrls()
{
  for ( int r = (int) m_Data.GetSize() - 1; r >= 0; r-- )
  {
    for ( int c = 0; c < (int) m_Data[ r ].GetSize(); c++ )
    {
      if ( m_Data[ r ][ c ].m_Ctrl )
      {
        m_Data[ r ][ c ].m_Ctrl->ShowWindow( SW_HIDE );
      }
    }
  }
  TRACE_GRID( "+++ Controls are hidden\n" );
}


void CGridListCtrl::OnNcCalcSize( BOOL bCalcValidRects , NCCALCSIZE_PARAMS* lpncsp )
{
  CListCtrl::OnNcCalcSize( bCalcValidRects , lpncsp );
  CRect rc;
  GetClientRect( rc );
  SetColumnWidth( 0 , rc.Width() / 3 );
  SetColumnWidth( 1 , 2 * rc.Width() / 3 );
}

BOOL CGridListCtrl::DeleteAllItems()
{
  while ( m_Data.GetSize() )
  {
    m_Data[ 0 ].RemoveAll();
    m_Data.RemoveAt( 0 );
  }
  return CListCtrl::DeleteAllItems();
}


BOOL CGridListCtrl::DeleteItem( int nItem )
{
  BOOL res = CListCtrl::DeleteItem( nItem );
  for ( int i = 0; i < (int) m_Data[ nItem ].GetSize(); i++ )
  {
    m_Data[ nItem ][ i ].Clear();
  }
  m_Data[ nItem ].RemoveAll();
  m_Data.RemoveAt( nItem );
  Invalidate();
  return res;
}

