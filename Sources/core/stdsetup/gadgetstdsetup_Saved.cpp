// GadgetStdSetup.cpp : implementation file
//

#include "stdafx.h"
#include <gadgets\stdsetup.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "GadgetStdSetup"
#define ITEM_DATA_ARRAY FXArray<itemData,itemData>
#define ITEM_DATA(p) ((ITEM_DATA_ARRAY*)p)

void OnGadgetSetupEvent( int Event , void *wParam , int col , int row , int uData )
{
  ((CGadgetStdSetup*) wParam)->OnGridEvent( Event , col , row , uData );
}

void OnObjectSetupEvent( int Event , void *wParam , int col , int row , int uData )
{
  ((CObjectStdSetup*) wParam)->OnGridEvent( Event , col , row , uData );
}

int FormComboSetup( ComboItem * pItems , FXString& Result , LPCTSTR pName )
{
  int iNItems = 0 ;
  if ( pItems && pItems->psItemName )
  {
    Result += _T( "ComboBox(" ) ;
    Result += pName ;

    FXString NewItem ;
    while ( pItems->psItemName )
    {
      NewItem.Format( _T( "%c%s(%d)" ) ,
        iNItems ? _T( ',' ) : _T( '(' ) , pItems->psItemName , pItems->iItemId ) ;
      Result += NewItem ;
      pItems++ ;
      iNItems++ ;
    }
    Result += _T( "))," ) ;
  }
  return iNItems ;
}

LPCTSTR GetNameForId( ComboItem * pItems , int iId )
{
  while ( pItems && pItems->psItemName )
  {
    if ( pItems->iItemId == iId )
      return pItems->psItemName ;
    pItems++ ;
  }
  return NULL ;
}

bool GetIdForName( ComboItem * pItems , LPCTSTR pName , int& iId )
{
  while ( pItems && pItems->psItemName )
  {
    if ( !_tcsicmp( pName , pItems->psItemName ) )
    {
      iId = pItems->iItemId ;
      return true ;
    }
    pItems++ ;
  }
  return false ;
}

/////////////////////////////////////////////////////////////////////////////
// CGadgetStdSetup dialog


CGadgetStdSetup::CGadgetStdSetup( IGraphbuilder* pBuilder , LPCTSTR uid , CWnd* pParent /*=NULL*/ ) :
  CDialog( IDD_STDDIALOG , pParent ) ,
  m_pBuilder( pBuilder ) ,
  m_UID( uid ) ,
  m_bInvalidateProcess (false)
{
  //{{AFX_DATA_INIT(CGadgetStdSetup)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  m_pItemsData = new ITEM_DATA_ARRAY;
}

CGadgetStdSetup::~CGadgetStdSetup()
{
  ITEM_DATA( m_pItemsData )->RemoveAll();
  delete ITEM_DATA( m_pItemsData );
  m_pItemsData = NULL;
}

void CGadgetStdSetup::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CGadgetStdSetup)
  DDX_Control( pDX , IDC_LIST , m_SetupGrid );
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CGadgetStdSetup , CDialog )
  //{{AFX_MSG_MAP(CGadgetStdSetup)
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
  //    ON_WM_SIZE()
  ON_BN_CLICKED( ID_RESTORE_INITIAL , &CGadgetStdSetup::OnBnClickedRestoreInitial )
  ON_BN_CLICKED( ID_SAVE_SET , &CGadgetStdSetup::OnBnClickedSaveSet )
  ON_BN_CLICKED( ID_RESTORE_SET , &CGadgetStdSetup::OnBnClickedRestoreSet )
  ON_BN_CLICKED( ID_APPLY , &CGadgetStdSetup::OnBnClickedApply )
  ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGadgetStdSetup message handlers

bool CGadgetStdSetup::Show( CPoint point , LPCTSTR uid )
{
  if ( !m_hWnd )
  {
    if ( !Create( NULL ) )
    {
      SENDERR_0( _T( "Failed to create Setup Dialog" ) );
      return false;
    }
  }
  FXString Name( uid ) ;
  Name += " Parameters";
  SetWindowText( Name );
  m_SetupGrid.DeleteAllItems();
  EnableWindow( FALSE ) ;
  LoadSetup();
  EnableWindow( TRUE ) ;
  SetWindowPos( NULL , point.x , point.y , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
  ShowWindow( SW_SHOWNORMAL );
  return true;
}

void CGadgetStdSetup::Delete()
{
  if ( GetSafeHwnd() )
    DestroyWindow();
  delete this;
}

BOOL CGadgetStdSetup::Create( CWnd* pParentWnd )
{
  int IDD = IDD_STDDIALOG;
  LPCTSTR lpszTemplateName = MAKEINTRESOURCE( IDD );
  //m_hInstSave = afxCurrentInstanceHandle;

  ASSERT( HIWORD( lpszTemplateName ) == 0 ||
    AfxIsValidString( lpszTemplateName ) );

  m_lpszTemplateName = lpszTemplateName;  // used for help
  if ( HIWORD( m_lpszTemplateName ) == 0 && m_nIDHelp == 0 )
    m_nIDHelp = LOWORD( (DWORD) (size_t) m_lpszTemplateName );
  HINSTANCE hInst = GetModuleHandle( STDSETUP_DLL_NAME );
  HRSRC hResource = ::FindResource( hInst , lpszTemplateName , RT_DIALOG );
  HGLOBAL hTemplate = LoadResource( hInst , hResource );
  BOOL bResult = CDialog::CreateIndirect( hTemplate , pParentWnd , hInst );
  FreeResource( hTemplate );

  return bResult;
}

BOOL CGadgetStdSetup::OnInitDialog()
{
  CRect rc;

  CDialog::OnInitDialog();

  m_SetupGrid.GetClientRect( rc );
  m_SetupGrid.InitCtrl( ::OnGadgetSetupEvent , this );
  m_SetupGrid.InsertColumn( 0 , "Parameter" , LVCFMT_LEFT , rc.Width() / 3 );
  m_SetupGrid.InsertColumn( 1 , "Value" , LVCFMT_LEFT , 2 * rc.Width() / 3 );
  return TRUE;
  //return LoadSetup();
}

BOOL CGadgetStdSetup::LoadSetup()
{
  m_ItemsCnt = 0;
  FXParser settings;
  if ( !m_pBuilder->ScanSettings( m_UID , settings ) )
  {
    DestroyWindow();
    return FALSE;
  }
  m_pk.Empty() ;

  FXString  key;
  FXParser params;
  int itemNo = 0;
  if ( (!settings.GetElementNo( itemNo , key , params )) || (key != "template") )
  {
    ASSERT( FALSE ); // wrong syntax of template
    return FALSE;
  }

  FXParser lineParam;
  while ( (params.GetElementNo( m_ItemsCnt , key , lineParam )) )
  {
    if ( !InsertLine( key , lineParam ) )
      break ;
  }
  if ( m_ItemsCnt == 0 )
  {
    return FALSE;
  }
  return TRUE;
}

void CGadgetStdSetup::OnDestroy()
{
  CDialog::OnDestroy();
}

bool CGadgetStdSetup::InsertLine( LPCTSTR key , LPCTSTR params )
{
  TRACE( "It's about to insert std line %s(%s)\n" , key , params );
  //CInfoParser item_param(params);
  FXParser item_param( params );
  //FXPropertyKit pk;
  if ( m_pk.IsEmpty() )
    m_pBuilder->PrintProperties( m_UID , m_pk );

  if ( m_InitialPropValues.IsEmpty() )
  {
    m_InitialPropValues = m_pk ;
    m_SavedPropValues = m_pk ;
  }

  CGridRow row;
  CGridCell cell;

  if ( stricmp( key , SETUP_COMBOBOX ) == 0 )
  {
    FXString tmpS;
    FXString lpName;
    FXSIZE i = 0;
    item_param.GetWord( i , lpName );
    //CInfoParser lbItems;
    FXParser lbItems;

    item_param.GetElementNo( 0 , lpName , lbItems );

    //m_pBuilder->PrintProperties(m_UID, pk);
    FXString iniVal;

    if ( (i != 0) && (m_pk.GetString( lpName , iniVal )) )
    {
      cell.DefineString( lpName , false );
      row.Add( cell );
      itemData iData;
      iData.ctrlId = SC_SELCHANGED;
      iData.itemId = "this";
      iData.cmdId = lpName;
      int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

      cell.DefineListBox( lbItems , iniVal , p );
      row.Add( cell );
      m_SetupGrid.AddRow( row );
      row.RemoveAll();
      m_ItemsCnt++;
    }
    else
    {
      SENDERR_1( "Error: Can't get default value for '%s' value" , lpName );
      return false;
    }
  }
  else if ( stricmp( key , SETUP_SPIN ) == 0 )
  {
    FXString tmpS;
    FXString lpName;
    int min , max;

    FXSIZE i = 0;
    item_param.GetWord( i , lpName );
    item_param.GetWord( i , tmpS );
    min = atoi( tmpS );
    item_param.GetWord( i , tmpS );
    max = atoi( tmpS );

    //m_pBuilder->PrintProperties(m_UID, pk);
    int iniVal;

    if ( (i != 0) && (m_pk.GetInt( lpName , iniVal )) )
    {
      cell.DefineString( lpName , false );
      row.Add( cell );

      itemData iData;
      iData.ctrlId = SC_INTCHANGE;
      iData.itemId = "this";
      iData.cmdId = lpName;
      int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

      cell.DefineInt( iniVal , min , max , p );
      row.Add( cell );
      m_SetupGrid.AddRow( row );
      row.RemoveAll();
      m_ItemsCnt++;
    }
    else
    {
      SENDERR_1( "Error: Can't get default value for '%s' value" , lpName );
      return false;
    }
  }
  else if ( stricmp( key , SETUP_SPINABOOL ) == 0 )
  {
    FXString tmpS;
    FXString lpName;
    int min , max;

    FXSIZE i = 0;
    item_param.GetWord( i , lpName );
    item_param.GetWord( i , tmpS );
    min = atoi( tmpS );
    item_param.GetWord( i , tmpS );
    max = atoi( tmpS );

    //m_pBuilder->PrintProperties(m_UID, pk);
    FXString iniSVal;

    if ( (i != 0) && (m_pk.GetString( lpName , iniSVal )) )
    {
      bool autoV = false;
      int iniVal = 0;

      FXString SmallLetters = iniSVal.MakeLower() ;
      if ( SmallLetters.Find( "auto" ) == 0 )
      {
        autoV = true;
        SmallLetters.Delete( 0 , 4 ) ;
        SmallLetters = SmallLetters.Trim() ;
        if ( SmallLetters.GetLength() > 0 )
        {
          iniVal = atoi( SmallLetters ) ;
        }
      }
      else
      {
        iniVal = atoi( iniSVal );
      }
      cell.DefineString( lpName , false );
      row.Add( cell );

      itemData iData;
      iData.ctrlId = SC_INTCHKCHANGE;
      iData.itemId = "this";
      iData.cmdId = lpName;
      int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

      cell.DefineIntChk( iniVal , min , max , autoV , p );
      row.Add( cell );
      m_SetupGrid.AddRow( row );
      row.RemoveAll();
      m_ItemsCnt++;
    }
    else
    {
      SENDERR_1( "Error: Can't get default value for '%s' value" , lpName );
      return false;
    }
  }
  else if ( stricmp( key , SETUP_EDITBOX ) == 0 )
  {
    FXString lpName;
    FXString iniVal;
    FXSIZE i = 0;

    item_param.GetWord( i , lpName );
    //m_pBuilder->PrintProperties(m_UID, pk);
    if ( (i != 0) && (m_pk.GetString( lpName , iniVal )) )
    {
      cell.DefineString( lpName , false );
      row.Add( cell );

      itemData iData;
      iData.ctrlId = SC_EDITLSTFOCUS;
      iData.itemId = "this";
      iData.cmdId = lpName;
      int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

      cell.DefineString( iniVal , true , p );
      row.Add( cell );
      m_SetupGrid.AddRow( row );
      row.RemoveAll();
      m_ItemsCnt++;
    }
    else
    {
      SENDERR_1( "Error: Can't get default value for '%s' value" , lpName );
      return false;
    }
  }
  else
  {
    SENDERR_1( "Error: Unknown control for StdSetupDlg '%s'" , key );
    return false;
  }
  return true;
}

void CGadgetStdSetup::OnGridEvent( int Event , int col , int row , int uData )
{
  CWaitCursor wc;
  bool Invalidate = false;
  switch ( Event )
  {
  case SC_SELCHANGED:
    {
      FXPropertyKit pk;
      pk.WriteString( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , m_SetupGrid.GetItemData( col , row ) );
      m_pBuilder->ScanProperties( m_UID , pk , Invalidate );
      break;
    }
  case SC_INTCHANGE:
    {
      FXPropertyKit pk;
      pk.WriteInt( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , m_SetupGrid.GetItemInt( col , row ) );
      m_pBuilder->ScanProperties( m_UID , pk , Invalidate );
      break;
    }
  case SC_INTCHKCHANGE:
    {
      FXPropertyKit pk;
      int val;
      bool enable;
      m_SetupGrid.GetItemIntChk( col , row , val , enable );
      if ( enable )
        pk.WriteString( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , "auto" );
      else
        pk.WriteInt( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , val );
      m_pBuilder->ScanProperties( m_UID , pk , Invalidate );
      break;
    }
  case SC_EDITLSTFOCUS:
    {
      FXPropertyKit pk;
      pk.WriteString( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , m_SetupGrid.GetItemText( col , row ) );
      m_pBuilder->ScanProperties( m_UID , pk , Invalidate );
      break;
    }
  default:
    SENDERR_2( "Error: Unsupported control in OnGridEvent ID: %d(0x%x)" , Event , Event );
  }
  if ( Invalidate )
  {
    int iPos = m_SetupGrid.GetScrollPos( SB_VERT ) /*- 12*/ ;
    int iScrollMin , iScrollMax ;
    m_SetupGrid.GetScrollRange( SB_VERT , &iScrollMin , &iScrollMax ) ;
//     if ( iPos < 0 )
//       iPos = 0 ;
    m_bInvalidateProcess = true ;
    m_SetupGrid.DeleteAllItems();
//     EnableWindow( FALSE ) ;
    LoadSetup();
    m_SetupGrid.SetScrollPos( SB_VERT , iPos ) ;
//     EnableWindow( TRUE ) ;
    //     if ( iPos > 0 )
//     {
//       CRect rect ;
//       m_SetupGrid.GetWindowRect( &rect ) ;
//       int iRotation = -iPos * WHEEL_DELTA ;
      //SetCursorPos( rect.left - 5 , rect.top ) ;
//       m_bInvalidateProcess = TRUE ;
//       m_SetupGrid.PostMessage( WM_MOUSEWHEEL , (iRotation << 16) ,
//         (rect.right - 7) + (rect.CenterPoint().y << 16) ) ;
//     }
//     else
//       EnableWindow( TRUE ) ;
  }
}

BOOL CGadgetStdSetup::OnMouseWheel( UINT nFlags , short zDelta , CPoint pt )
{
  BOOL bRes = __super::OnMouseWheel( nFlags , zDelta , pt );
//   if ( m_bInvalidateProcess )
//   {
//     EnableWindow( TRUE ) ;
//     m_bInvalidateProcess = false ;
//   }
  return bRes ;
}

FXPropertyKit& CGadgetStdSetup::GetSavedProperties()
{
  return m_SavedPropValues ;
}

void CGadgetStdSetup::SetSavedProperties( FXString& Properties )
{
  m_SavedPropValues = Properties ;
}

void CGadgetStdSetup::Update()
{
  Invalidate() ;
}

void CGadgetStdSetup::OnBnClickedRestoreInitial()
{
  bool bInvalidate = false ;
  if ( !m_InitialPropValues.IsEmpty()
    && m_pBuilder->ScanProperties( m_UID , m_InitialPropValues , bInvalidate )
    && bInvalidate )
  {
    m_SetupGrid.DeleteAllItems();
    EnableWindow( FALSE ) ;
    LoadSetup();
    EnableWindow( TRUE ) ;
  }
}

void CGadgetStdSetup::OnBnClickedSaveSet()
{
  m_pBuilder->PrintProperties( m_UID , m_SavedPropValues ) ;
}

void CGadgetStdSetup::OnBnClickedRestoreSet()
{
  bool bInvalidate = false ;
  if ( !m_SavedPropValues.IsEmpty()
    && m_pBuilder->ScanProperties( m_UID , m_SavedPropValues , bInvalidate )
    && bInvalidate )
  {
    m_SetupGrid.DeleteAllItems();
    EnableWindow( FALSE ) ;
    LoadSetup();
    EnableWindow( TRUE ) ;
  }
}

void CGadgetStdSetup::OnBnClickedApply()
{
  // TODO: Add your control notification handler code here
}

#undef THIS_MODULENAME

#define THIS_MODULENAME "ObjectStdSetup"

/////////////////////////////////////////////////////////////////////////////
// CObjectStdSetup dialog


CObjectStdSetup::CObjectStdSetup( CVideoObjectBase* pObject , LPCTSTR uid , CWnd* pParent /*=NULL*/ ) :
  CDialog( IDD_STDDIALOG , pParent ) ,
  m_pObject( pObject ) ,
  m_UID( uid )
{
  //{{AFX_DATA_INIT(CObjectStdSetup)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  m_pItemsData = new ITEM_DATA_ARRAY;
}

CObjectStdSetup::~CObjectStdSetup()
{
  m_row.RemoveAll() ;
  ITEM_DATA( m_pItemsData )->RemoveAll();
  delete ITEM_DATA( m_pItemsData );
  m_pItemsData = NULL;
}

void CObjectStdSetup::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CObjectStdSetup)
  DDX_Control( pDX , IDC_LIST , m_SetupGrid );
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CObjectStdSetup , CDialog )
  //{{AFX_MSG_MAP(CObjectStdSetup)
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
  //    ON_WM_SIZE()
  ON_BN_CLICKED( ID_RESTORE_INITIAL , &CObjectStdSetup::OnBnClickedRestoreInitial )
  ON_BN_CLICKED( ID_SAVE_SET , &CObjectStdSetup::OnBnClickedSaveSet )
  ON_BN_CLICKED( ID_RESTORE_SET , &CObjectStdSetup::OnBnClickedRestoreSet )
  ON_BN_CLICKED( ID_APPLY , &CObjectStdSetup::OnBnClickedApply )
  ON_MESSAGE( MSG_UPDATE , OnUpdateSetup )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectStdSetup message handlers

bool CObjectStdSetup::Show( CPoint point , LPCTSTR ObjectName )
{
  if ( !m_hWnd )
  {
    if ( !Create( NULL ) )
    {
      SENDERR_0( _T( "Failed to create Setup Dialog" ) );
      return false;
    }
  }
  FXString Name( "Object " ) ;
  Name += ObjectName ;
  Name += " Parameters";
  SetWindowText( Name );
  m_SetupGrid.DeleteAllItems();
  LoadSetup();
  SetWindowPos( NULL , point.x , point.y , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
  ShowWindow( SW_SHOWNORMAL );
  return true;
}

BOOL CObjectStdSetup::IsOn()
{
  return (::IsWindow( GetSafeHwnd() ) && IsWindowVisible());
}

void CObjectStdSetup::Delete()
{
  if ( GetSafeHwnd() )
    DestroyWindow();
  m_pObject->ResetSetupObject() ;
  delete this;
}

BOOL CObjectStdSetup::Create( CWnd* pParentWnd )
{
  int IDD = IDD_STDDIALOG;
  LPCTSTR lpszTemplateName = MAKEINTRESOURCE( IDD );
  //m_hInstSave = afxCurrentInstanceHandle;

  ASSERT( HIWORD( lpszTemplateName ) == 0 ||
    AfxIsValidString( lpszTemplateName ) );

  m_lpszTemplateName = lpszTemplateName;  // used for help
  if ( HIWORD( m_lpszTemplateName ) == 0 && m_nIDHelp == 0 )
    m_nIDHelp = LOWORD( (DWORD) (size_t) m_lpszTemplateName );
  HINSTANCE hInst = GetModuleHandle( STDSETUP_DLL_NAME );
  HRSRC hResource = ::FindResource( hInst , lpszTemplateName , RT_DIALOG );
  HGLOBAL hTemplate = LoadResource( hInst , hResource );
  BOOL bResult = CDialog::CreateIndirect( hTemplate , pParentWnd , hInst );
  FreeResource( hTemplate );

  return bResult;
}

BOOL CObjectStdSetup::OnInitDialog()
{
  CRect rc;

  CDialog::OnInitDialog();

  m_SetupGrid.GetClientRect( rc );
  m_SetupGrid.InitCtrl( ::OnObjectSetupEvent , this );
  m_SetupGrid.InsertColumn( 0 , "Parameter" , LVCFMT_LEFT , rc.Width() / 3 );
  m_SetupGrid.InsertColumn( 1 , "Value" , LVCFMT_LEFT , 2 * rc.Width() / 3 );
  return TRUE;
  //return LoadSetup();
}

BOOL CObjectStdSetup::LoadSetup()
{
  m_ItemsCnt = 0;
  FXParser settings;
  if ( !m_pObject->ScanSettings( settings ) )
  {
    DestroyWindow();
    return FALSE;
  }
  m_pk.Empty() ;

  FXString  key;
  FXParser params;
  int itemNo = 0;
  if ( (!settings.GetElementNo( itemNo , key , params )) || (key != "template") )
  {
    ASSERT( FALSE ); // wrong syntax of template
    return FALSE;
  }

  FXParser lineParam;
  while ( (params.GetElementNo( m_ItemsCnt , key , lineParam )) )
  {
    if ( !InsertLine( key , lineParam ) )
      break ;
  }
  InsertLine( NULL , NULL ) ; // insert last line, if odd check boxes number 
  if ( m_ItemsCnt == 0 )
  {
    return FALSE;
  }
  return TRUE;
}

void CObjectStdSetup::OnDestroy()
{
  CDialog::OnDestroy();
}

bool CObjectStdSetup::EndLineIfNotEmpty()
{
  if ( m_row.GetCount() )
  {
    m_SetupGrid.AddRow( m_row );
    m_row.RemoveAll();
    TRACE( "\n............CObjectStdSetup::EndLineIfNotEmpty  " ) ;
    return true ;
  }
  return false ;
}

bool CObjectStdSetup::InsertLine( LPCTSTR key , LPCTSTR params )
{
  TRACE( "It's about to insert std line %s(%s)\n" , key , params );
  //CInfoParser item_param(params);
  FXParser item_param( params );
  //FXPropertyKit pk;

  if ( !key && !params )
  {
    EndLineIfNotEmpty() ;
    return true ;
  }
  if ( m_pk.IsEmpty() )
    m_pObject->PrintProperties( m_pk );

  if ( m_InitialPropValues.IsEmpty() )
  {
    m_InitialPropValues = m_pk ;
    m_SavedPropValues = m_pk ;
  }

  CGridCell cell;
  if ( stricmp( key , SETUP_INDENT ) == 0 )
  {
    EndLineIfNotEmpty() ;
    FXString IndentName;
    FXSIZE i = 0;
    item_param.GetWord( i , IndentName );
    cell.DefineIndent( IndentName );
    m_row.Add( cell );
    m_SetupGrid.AddRow( m_row );
    m_row.RemoveAll();
    m_ItemsCnt++ ;
  }
  else if ( stricmp( key , SETUP_CHECKBOX ) == 0 )
  {
    FXString tmpS;
    FXString lpName;

    FXSIZE i = 0;
    item_param.GetWord( i , lpName );
    FXString iniSVal;

    if ( (i != 0) && (m_pk.GetString( lpName , iniSVal )) )
    {
      bool bCheckVal = false;
      int iniVal = 0;

      FXString SmallLetters = iniSVal.MakeLower() ;
      if ( SmallLetters.Find( "true" ) == 0 )
        bCheckVal = true;
      else
      {
        iniVal = atoi( iniSVal );
        if ( iniVal != 0 )
          bCheckVal = true ;
      }

      itemData iData;
      iData.ctrlId = SC_CHKCHANGE;
      iData.itemId = "this";
      iData.cmdId = lpName;
      int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

      cell.DefineChk( (LPCTSTR) lpName , bCheckVal , p );
      m_row.Add( cell );
      if ( m_row.GetCount() > 1 )
      {
        m_SetupGrid.AddRow( m_row );
        m_row.RemoveAll();
      }
      m_ItemsCnt++;
    }
    else
    {
      SENDERR( "Error in Object '%s': Can't get default value for '%s' " ,
        (LPCTSTR) m_pObject->m_ObjectName , lpName );
      return false;
    }
  }
  else
  {
    EndLineIfNotEmpty() ;
    if ( stricmp( key , SETUP_COMBOBOX ) == 0 )
    {
      FXString tmpS;
      FXString lpName;
      FXSIZE i = 0;
      item_param.GetWord( i , lpName );
      //CInfoParser lbItems;
      FXParser lbItems;

      item_param.GetElementNo( 0 , lpName , lbItems );

      //m_pObject->PrintProperties(m_UID, pk);
      FXString iniVal;

      if ( (i != 0) && (m_pk.GetString( lpName , iniVal )) )
      {
        cell.DefineString( lpName , false );
        m_row.Add( cell );
        itemData iData;
        iData.ctrlId = SC_SELCHANGED;
        iData.itemId = "this";
        iData.cmdId = lpName;
        int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

        cell.DefineListBox( lbItems , iniVal , p );
        m_row.Add( cell );
        m_SetupGrid.AddRow( m_row );
        m_row.RemoveAll();
        m_ItemsCnt++;
      }
      else
      {
        SENDERR( "Error in Object '%s': Can't get default value for '%s' " ,
          (LPCTSTR) m_pObject->m_ObjectName , lpName );
        return false;
      }
    }
    else if ( stricmp( key , SETUP_SPIN ) == 0 )
    {
      FXString tmpS;
      FXString lpName;
      int min , max;

      FXSIZE i = 0;
      item_param.GetWord( i , lpName );
      item_param.GetWord( i , tmpS );
      min = atoi( tmpS );
      item_param.GetWord( i , tmpS );
      max = atoi( tmpS );

      //m_pObject->PrintProperties(m_UID, pk);
      int iniVal;

      if ( (i != 0) && (m_pk.GetInt( lpName , iniVal )) )
      {
        cell.DefineString( lpName , false );
        m_row.Add( cell );

        itemData iData;
        iData.ctrlId = SC_INTCHANGE;
        iData.itemId = "this";
        iData.cmdId = lpName;
        int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

        cell.DefineInt( iniVal , min , max , p );
        m_row.Add( cell );
        m_SetupGrid.AddRow( m_row );
        m_row.RemoveAll();
        m_ItemsCnt++;
      }
      else
      {
        SENDERR( "Error in Object '%s': Can't get default value for '%s' " ,
          (LPCTSTR) m_pObject->m_ObjectName , lpName );
        return false;
      }
    }
    else if ( stricmp( key , SETUP_SPINABOOL ) == 0 )
    {
      FXString tmpS;
      FXString lpName;
      int min , max;

      FXSIZE i = 0;
      item_param.GetWord( i , lpName );
      item_param.GetWord( i , tmpS );
      min = atoi( tmpS );
      item_param.GetWord( i , tmpS );
      max = atoi( tmpS );

      //m_pObject->PrintProperties(m_UID, pk);
      FXString iniSVal;

      if ( (i != 0) && (m_pk.GetString( lpName , iniSVal )) )
      {
        bool autoV = false;
        int iniVal = 0;

        FXString SmallLetters = iniSVal.MakeLower() ;
        if ( SmallLetters.Find( "auto" ) == 0 )
        {
          autoV = true;
          SmallLetters.Delete( 0 , 4 ) ;
          SmallLetters = SmallLetters.Trim() ;
          if ( SmallLetters.GetLength() > 0 )
          {
            iniVal = atoi( SmallLetters ) ;
          }
        }
        else
        {
          iniVal = atoi( iniSVal );
        }
        cell.DefineString( lpName , false );
        m_row.Add( cell );

        itemData iData;
        iData.ctrlId = SC_INTCHKCHANGE;
        iData.itemId = "this";
        iData.cmdId = lpName;
        int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

        cell.DefineIntChk( iniVal , min , max , autoV , p );
        m_row.Add( cell );
        m_SetupGrid.AddRow( m_row );
        m_row.RemoveAll();
        m_ItemsCnt++;
      }
      else
      {
        SENDERR( "Error in Object '%s': Can't get default value for '%s' " ,
          (LPCTSTR) m_pObject->m_ObjectName , lpName );
        return false;
      }
    }
    else if ( stricmp( key , SETUP_EDITBOX ) == 0 )
    {
      FXString lpName;
      FXString iniVal;
      FXSIZE i = 0;

      item_param.GetWord( i , lpName );
      //m_pObject->PrintProperties(m_UID, pk);
      if ( (i != 0) && (m_pk.GetString( lpName , iniVal )) )
      {
        cell.DefineString( lpName , false );
        m_row.Add( cell );

        itemData iData;
        iData.ctrlId = SC_EDITLSTFOCUS;
        iData.itemId = "this";
        iData.cmdId = lpName;
        int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

        cell.DefineString( iniVal , true , p );
        m_row.Add( cell );
        m_SetupGrid.AddRow( m_row );
        m_row.RemoveAll();
        m_ItemsCnt++;
      }
      else
      {
        SENDERR( "Error in Object '%s': Can't get default value for '%s' " ,
          (LPCTSTR) m_pObject->m_ObjectName , lpName );
        return false;
      }
    }
    else
    {
      SENDERR( "Error in Object '%s': Unknown control for ObjectSetupDlg '%s'" ,
        (LPCTSTR) m_pObject->m_ObjectName , key );
      return false;
    }
  }
  return true;
}

void CObjectStdSetup::OnGridEvent( int Event , int col , int row , int uData )
{
  CWaitCursor wc;
  FXPropertyKit pk;
  bool Invalidate = false;
  switch ( Event )
  {
  case SC_SELCHANGED:
    {
      pk.WriteString( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , m_SetupGrid.GetItemData( col , row ) );
      m_pObject->ScanProperties( pk , Invalidate );
      break;
    }
  case SC_INTCHANGE:
    {
      pk.WriteInt( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , m_SetupGrid.GetItemInt( col , row ) );
      m_pObject->ScanProperties( pk , Invalidate );
      break;
    }
  case SC_INTCHKCHANGE:
    {
      int val;
      bool enable;
      m_SetupGrid.GetItemIntChk( col , row , val , enable );
      if ( enable )
        pk.WriteString( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , "auto" );
      else
        pk.WriteInt( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , val );
      m_pObject->ScanProperties( pk , Invalidate );
      break;
    }
  case SC_CHKCHANGE:
    {
      bool enable = false ;
      m_SetupGrid.GetItemChk( col , row , enable );
      pk.WriteInt( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , enable ? 1 : 0 );
      m_pObject->ScanProperties( pk , Invalidate );
      break;
    }
  case SC_EDITLSTFOCUS:
    {
      pk.WriteString( (*ITEM_DATA( m_pItemsData ))[ uData ].cmdId , m_SetupGrid.GetItemText( col , row ) );
      m_pObject->ScanProperties( pk , Invalidate );
      break;
    }
  default:
    SENDERR_2( "Error: Unsupported control in OnGridEvent ID: %d(0x%x)" , Event , Event );
  }
  if ( Invalidate )
  {
    int iPos = m_SetupGrid.GetScrollPos( SB_VERT ) ;
    EnableWindow( FALSE ) ;
    m_SetupGrid.DeleteAllItems();
    LoadSetup();
    CRect rect ;
    m_SetupGrid.GetWindowRect( &rect ) ;
    int iRotation = -iPos * WHEEL_DELTA ;
    SetCursorPos( rect.left , rect.top ) ;
    m_bInvalidateProcess = true ;
    m_SetupGrid.PostMessage( WM_MOUSEWHEEL , (iRotation << 16) , 
      (rect.right - 7) + (rect.CenterPoint().y << 16) ) ;
//     m_SetupGrid.GetItemRect( 0 , rect , LVIR_BOUNDS );
//     int iScrollMin , iScrollMax ;
//     m_SetupGrid.GetScrollRange( SB_VERT , &iScrollMin , &iScrollMax ) ;
//     if ( iPos > iScrollMax - 2 )
//       iPos = iScrollMax - 2 ;
//     int scrollPos = (iPos + 2)* rect.Height() ;
//     PostMessage( MSG_UPDATE , 2 , NULL ) ;
// //    m_SetupGrid.SetScrollPos( SB_VERT , iPos , TRUE ) ;
//     m_SetupGrid.Scroll( CSize( 0 , scrollPos) ) ;
// //     this->Invalidate() ;
// //     m_SetupGrid.Invalidate() ;
// //     m_SetupGrid.Scroll( CSize( 0 , -10 ) ) ;
   }
}

FXPropertyKit& CObjectStdSetup::GetSavedProperties()
{
  return m_SavedPropValues ;
}

void CObjectStdSetup::SetSavedProperties( FXString& Properties )
{
  m_SavedPropValues = Properties ;
}

void CObjectStdSetup::OnBnClickedRestoreInitial()
{
  bool bInvalidate = false ;
  if ( !m_InitialPropValues.IsEmpty()
    && m_pObject->ScanProperties( m_InitialPropValues , bInvalidate )
    && bInvalidate )
  {
    m_SetupGrid.DeleteAllItems();
    LoadSetup();
  }
}

void CObjectStdSetup::OnBnClickedSaveSet()
{
  m_pObject->PrintProperties( m_SavedPropValues ) ;
}

void CObjectStdSetup::OnBnClickedRestoreSet()
{
  bool bInvalidate = false ;
  if ( !m_SavedPropValues.IsEmpty()
    && m_pObject->ScanProperties( m_SavedPropValues , bInvalidate )
    && bInvalidate )
  {
    m_SetupGrid.DeleteAllItems();
    LoadSetup();
  }
}

void CObjectStdSetup::OnBnClickedApply()
{
  // TODO: Add your control notification handler code here
}

LRESULT CObjectStdSetup::OnUpdateSetup( WPARAM wPar , LPARAM lPar )
{
  int iScrollMin , iScrollMax ;
  m_SetupGrid.GetScrollRange( SB_VERT , &iScrollMin , &iScrollMax ) ;
  CRect rect ;
  m_SetupGrid.GetItemRect( 0 , rect , LVIR_BOUNDS );
  int iHeight = rect.Height() ;
  int scrollPos = iHeight * (int) wPar ;
  m_SetupGrid.Scroll( CSize( 0 , scrollPos) ) ;
  return 1 ;
}


CVideoObjectBase::~CVideoObjectBase()
{
  if ( m_SetupObject )
    m_SetupObject->Delete();
  m_SetupObject = NULL;
}

