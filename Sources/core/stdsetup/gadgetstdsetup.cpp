// GadgetStdSetup.cpp : implementation file
//

#include "stdafx.h"
#include <gadgets\stdsetup.h>
#include <fxfc/FXRegistry.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "GadgetStdSetup"


int StdSetupShowDebugInfo = 0 ;

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
      NewItem.Format( _T( "%c%s(%lld)" ) ,
        iNItems ? _T( ',' ) : _T( '(' ) , pItems->psItemName , pItems->iItemId ) ;
      Result += NewItem ;
      pItems++ ;
      iNItems++ ;
    }
    Result += _T( "))," ) ;
  }
  return iNItems ;
}

LPCTSTR GetNameForId( ComboItem * pItems , __int64 iId )
{
  while ( pItems && pItems->psItemName )
  {
    if ( pItems->iItemId == iId )
      return pItems->psItemName ;
    pItems++ ;
  }
  return NULL ;
}

bool GetIdForName( ComboItem * pItems , LPCTSTR pName , __int64& iId )
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
// 
// BEGIN_MESSAGE_MAP( CStdSetupDlg , CDialog )
//   ON_WM_TIMER()
// END_MESSAGE_MAP()
// 
// 
// void CStdSetupDlg::OnTimer( UINT_PTR nIDEvent )
// {
// 
// 
//   CDialog::OnTimer( nIDEvent );
// }

/////////////////////////////////////////////////////////////////////////////
// CStdSetupDlg dialog - base for CGadgetStdSetup and CObjectStdSetup

CStdSetupDlg::CStdSetupDlg( UINT IDD , CWnd * pParent ) :
  CDialog( IDD , pParent ) ,
  m_iNRequiredCnt( 12 ) ,
  m_iSpareHeight( 0 ) ,
  m_ItemsCnt( 0 ) ,
  m_iNLastCheckBoxes( 0 ) ,
  m_pItemsData( NULL ) ,
  m_bInvalidateProcess( false ) ,
  m_ReadOnlyState( RO_Normal )
{
  //{{AFX_DATA_INIT(CStdSetupDlg)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  m_pItemsData = new ITEM_DATA_ARRAY;
  m_DlgMode = SDM_Unknown ;
} ;
CStdSetupDlg::~CStdSetupDlg()
{
  if ( m_pItemsData )
  {
    ITEM_DATA( m_pItemsData )->RemoveAll();
    delete ITEM_DATA( m_pItemsData );
    m_pItemsData = NULL;
  }
  m_row.RemoveAll() ;
}

ReadOnlyForStdSetupState CStdSetupDlg::GetROState( FXString& ItemName )
{
  int iSlashPos = (int)ItemName.Find( _T( '/' ) ) ;
  if ( (int)iSlashPos >= 0 )
  {
    int iRPos = ( iSlashPos > 0 ) ? (int)ItemName.ReverseFind( _T( 'R' ) ) : -1 ;
    if (iRPos == iSlashPos + 1) // read only item
    {
      if (m_ReadOnlyState == RO_DoNotShow)
        return RO_DoNotShow ; // not necessary to show this parameter
      if (m_ReadOnlyState == RO_Disabled)
        return RO_NoGrayed ; // view no grayed
      return RO_Normal ; // view grayed
    }
  }
  return RO_NoGrayed ;
}

void CStdSetupDlg::ResetData()
{
  m_iNLastCheckBoxes = m_iNRequiredCnt = 0;
  m_LastSettings.Empty() ;
  m_LastParams.Empty();
  m_LastKeys.RemoveAll() ;
  m_LastLineParams.RemoveAll() ;
}


int CStdSetupDlg::CntRequired( bool bROCheck )
{
  if ( bROCheck )
  {
    FXRegistry Regp( "TheFileX\\SHStudio" ) ;
    m_ReadOnlyState =
      (ReadOnlyForStdSetupState) Regp.GetRegiInt( "GadgetTree" , "SetupReadOnlyState" , 0 ) ;
  }
  FXString  key;
  int itemNo = 0;
  if ( (!m_LastSettings.GetElementNo( itemNo , key , m_LastParams )) || (key != "template") )
  {
    ASSERT( FALSE ); // wrong syntax of template
    return 0;
  }

  FXParser lineParam;
  FXSIZE i = 0 ;
  while ( m_LastParams.GetElementNo( i++ , key , lineParam ) )
  {
    m_LastKeys.Add( key ) ;
    m_LastLineParams.Add( lineParam ) ;
    bool bCheckBox = key.CompareNoCase( SETUP_CHECKBOX ) == 0 ;

    if ( !bCheckBox )
    { // new row necessary is not check box or 2 check boxes

      m_iNRequiredCnt++ ;
      if ( m_iNLastCheckBoxes )
        m_iNRequiredCnt++ ; // was one check box in row
      m_iNLastCheckBoxes = 0 ;
    }
    else if ( ++m_iNLastCheckBoxes >= 2 )
    {
      m_iNRequiredCnt++ ;
      m_iNLastCheckBoxes = 0 ;
    }
  }

  if ( m_iNLastCheckBoxes )
    m_iNRequiredCnt++ ;

  return m_iNRequiredCnt ;
}

BOOL CStdSetupDlg::ParseSetupData()
{
  m_ItemsCnt = 0;
  m_pk.Empty() ;

  for ( int i = 0 ; i < m_LastKeys.Count() ; i++ )
  {
    if ( !InsertLine( m_LastKeys[ i ] , m_LastLineParams[ i ] ) )
      break ;
  }

  SetDlgSizeAndPos() ;
  return (m_ItemsCnt > 0) ;
}

bool CStdSetupDlg::InsertLine( LPCTSTR key , LPCTSTR params )
{
  if ( StdSetupShowDebugInfo )
    TRACE( "CStdSetupDlg %s insert line %s(%s)\n" , (LPCTSTR)m_UID , key , params );

  FXParser item_param( params );

  if ( m_InitialPropValues.IsEmpty() )
  {
    m_InitialPropValues = m_pk ;
    m_SavedPropValues = m_pk ;
  }

  if ( !key && !params )
  {
    EndLineIfNotEmpty() ;
    return true ;
  }
  CGridCell cell;
  bool bDisabled = false ;
  FXSIZE iPos = 0 ;
  FXString ItemName = item_param.Tokenize( _T( ",(" ) , iPos ) ;
  if ( !ItemName.IsEmpty() )
  {
    ReadOnlyForStdSetupState ROState = GetROState( ItemName ) ;
    if ( ROState == RO_DoNotShow )
      return true ; // not necessary to show this parameter
    m_row.m_bGrayed = (ROState != RO_NoGrayed) ;
  }
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
        (LPCTSTR)m_UID , lpName );
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
      //m_pBuilder->PrintProperties(m_UID, pk);
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
        m_row.Add( cell );

        itemData iData;
        iData.ctrlId = SC_INTCHANGE;
        iData.itemId = "this";
        iData.cmdId = lpName;
        int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

        cell.DefineInt( iniVal , min , max , p );
        m_row.Add( cell );
        m_SetupGrid.AddRow( m_row );
        if ( m_row.m_bGrayed )
        {
          CheckAndAddToGreyedList( lpName ) ;
          if ( !m_iTimerID )
            m_iTimerID = SetTimer( TIMER_ID_FOR_GREYED , 1000 , NULL ) ;
        }
        m_row.RemoveAll();
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
        FXSIZE iAutoPos = SmallLetters.Find( "auto" ) ;
        if ( iAutoPos >= 0 )
        {
          autoV = true;
          SmallLetters.Delete( 0 , iAutoPos + 4 ) ;
          SmallLetters = SmallLetters.Trim() ;
          if ( SmallLetters.GetLength() > 0 )
            iniVal = atoi( SmallLetters ) ;

        }
        else
          iniVal = atoi( iniSVal );

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
        if ( autoV )
        {
          CheckAndAddToGreyedList( lpName ) ;
          if ( !m_iTimerID )
            m_iTimerID = SetTimer( TIMER_ID_FOR_GREYED , 1000 , NULL ) ;
        }

        m_row.RemoveAll();
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
        m_row.Add( cell );

        itemData iData;
        iData.ctrlId = SC_EDITLSTFOCUS;
        iData.itemId = "this";
        iData.cmdId = lpName;
        int p = (int) ITEM_DATA( m_pItemsData )->Add( iData );

        cell.DefineString( iniVal , true , p );
        m_row.Add( cell );
        m_SetupGrid.AddRow( m_row );
        if ( m_row.m_bGrayed )
        {
          CheckAndAddToGreyedList( lpName ) ;
          if ( !m_iTimerID )
            m_iTimerID = SetTimer( TIMER_ID_FOR_GREYED , 1000 , NULL ) ;
        }
        m_row.RemoveAll();
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
  }
  return true;
}

int CStdSetupDlg::SetDlgSizeAndPos() // returns num of shown strings
{
  CRect GridRect , ItemRect , DlgRect ;
  m_SetupGrid.GetWindowRect( &GridRect ) ;
  GetWindowRect( &DlgRect ) ;
  m_SetupGrid.GetItemRect( 0 , ItemRect , LVIR_BOUNDS );
  // +1 because parameters caption string
  int iActiveHeight = (m_iNRequiredCnt + 1) * ItemRect.Height() ;
  int iDiffDlgToGridHeight = DlgRect.Height() - GridRect.Height() ;

  GridRect.bottom = GridRect.top + iActiveHeight ;
  m_SetupGrid.SetWindowPos( NULL , GridRect.left , GridRect.top ,
    GridRect.Width() , GridRect.Height() , SWP_NOMOVE | SWP_NOZORDER ) ;
  DlgRect.bottom = DlgRect.top + GridRect.Height() + iDiffDlgToGridHeight ;
  SetWindowPos( NULL , 0 , 0 , DlgRect.Width() , DlgRect.Height() ,
    SWP_NOMOVE | SWP_NOZORDER ) ;
  UpdateWindow() ;
  return m_iNRequiredCnt ;
}

BOOL CStdSetupDlg::OnInitDialog()
{

  CDialog::OnInitDialog();

  CRect GridClientRect;
  m_SetupGrid.GetClientRect( GridClientRect ) ;
  ASSERT( m_DlgMode != SDM_Unknown ) ;
  m_SetupGrid.InitCtrl( 
    (m_DlgMode == SDM_VideoObject) ? ::OnObjectSetupEvent : ::OnGadgetSetupEvent , this );
  m_SetupGrid.InsertColumn( 0 , "Parameter" , LVCFMT_LEFT , GridClientRect.Width() / 3 );
  m_SetupGrid.InsertColumn( 1 , "Value" , LVCFMT_LEFT , 2 * GridClientRect.Width() / 3 );

  CRect GridRect , ItemRect , DlgRect ;
  m_SetupGrid.GetWindowRect( &GridRect ) ;
  GetWindowRect( &DlgRect ) ;
  m_SetupGrid.GetItemRect( 0 , ItemRect , LVIR_BOUNDS );

  m_iSpareHeight = DlgRect.Height() - GridRect.Height() ;
  return TRUE;
  //return LoadSetup();
}

bool CStdSetupDlg::EndLineIfNotEmpty()
{
  if ( m_row.GetCount() )
  {
    m_SetupGrid.AddRow( m_row );
    m_row.RemoveAll();
    if ( StdSetupShowDebugInfo )
      TRACE( "\n............CStdSetupDlg::EndLineIfNotEmpty  " ) ;
    return true ;
  }
  return false ;
}
/////////////////////////////////////////////////////////////////////////////
// CGadgetStdSetup dialog


CGadgetStdSetup::CGadgetStdSetup( IGraphbuilder* pBuilder , LPCTSTR uid , CWnd* pParent /*=NULL*/ ) :
  CStdSetupDlg( IDD_STDDIALOG , pParent ) ,
  m_pBuilder( pBuilder ) 
{
  m_UID = uid ;
  m_DlgMode = SDM_Gadget ;
}

CGadgetStdSetup::~CGadgetStdSetup()
{
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
  ON_WM_TIMER()
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
  LoadSetup();
  SetWindowPos( NULL , point.x , point.y , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
  ShowWindow( SW_SHOWNORMAL );
  Sleep( 30 ) ;
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

BOOL CGadgetStdSetup::LoadSetup()
{
  ResetData() ;
  CGadget * pGadget = m_pBuilder->GetGadget( m_UID ) ;
  if ( !pGadget || !pGadget->ScanSettings( m_LastSettings ) )
  {
    DestroyWindow();
    return FALSE;
  }

  if ( !CntRequired( true ) )
    return FALSE;

  m_ItemsCnt = 0;
  m_pk.Empty() ;
  m_GreyedItems.RemoveAll() ;
  for ( int i = 0 ; i < m_LastKeys.Count() ; i++ )
  {
    if ( !InsertLine( m_LastKeys[i] , m_LastLineParams[i] ) )
      break ;
  }

  SetDlgSizeAndPos() ;
  return (m_ItemsCnt > 0) ;
}

void CGadgetStdSetup::OnDestroy()
{
  CDialog::OnDestroy();
}

bool CGadgetStdSetup::InsertLine( LPCTSTR key , LPCTSTR params )
{
  if ( m_pk.IsEmpty() )
    m_pBuilder->PrintProperties( m_UID , m_pk );

  return CStdSetupDlg::InsertLine( key , params ) ;
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
      bool bAuto;
      BOOL bEnabled = TRUE ;
      m_SetupGrid.GetItemIntChk( col , row , val , bAuto , &bEnabled );
      LPCTSTR pParameterName = ( *ITEM_DATA( m_pItemsData ) )[ uData ].cmdId ;
      if ( bAuto )
      {
        pk.WriteString( pParameterName , "auto" );
        m_pBuilder->ScanProperties( m_UID , pk , Invalidate );
        CheckAndAddToGreyedList( pParameterName ) ;
        if ( !m_iTimerID )
          m_iTimerID = SetTimer( TIMER_ID_FOR_GREYED , 1000 , NULL ) ;
      }
      else if ( bEnabled )
      {
        pk.WriteInt( pParameterName , val );
        m_pBuilder->ScanProperties( m_UID , pk , Invalidate );
        for ( FXSIZE i = 0 ; i < m_GreyedItems.Size() ; i++ )
        {
          if ( m_GreyedItems[ i ] == pParameterName )
          {
            m_GreyedItems.RemoveAt( i-- ) ;
            if ( !m_GreyedItems.Size() && m_iTimerID )
            {
              KillTimer( m_iTimerID ) ;
              m_iTimerID = NULL ;
            }
            break ;
          }
        }
      }
      break;
    }
  case SC_CHKCHANGE:
    {
      FXPropertyKit pk;
      bool enable = false ;
      m_SetupGrid.GetItemChk( col , row , enable );
      pk.WriteInt( ( *ITEM_DATA( m_pItemsData ) )[ uData ].cmdId , enable ? 1 : 0 );
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
    int iPos = m_SetupGrid.GetScrollPos( SB_VERT ) - 12 ;
    if ( iPos < 0 )
      iPos = 0 ;
    m_SetupGrid.DeleteAllItems();
    LoadSetup();
    CRect rect ;
    m_SetupGrid.GetWindowRect( &rect ) ;
    int iRotation = -iPos * WHEEL_DELTA ;
    SetCursorPos( rect.left , rect.top ) ;
    m_SetupGrid.PostMessage( WM_MOUSEWHEEL , (iRotation << 16) , (rect.right - 7) + (rect.CenterPoint().y << 16) ) ;
  }
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
    LoadSetup();
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
    LoadSetup();
  }
}

void CGadgetStdSetup::OnBnClickedApply()
{
  // TODO: Add your control notification handler code here
}

void CGadgetStdSetup::OnTimer( UINT_PTR nIDEvent )
{
  if ( m_GreyedItems.Size() && IsWindowVisible() )
  {
    FXString Parameter( _T( "Properties=" ) ) ;
    for ( FXSIZE i = 0 ; i < m_GreyedItems.Size() ; i++ )
    {
      Parameter += m_GreyedItems[ i ] + _T( ';' ) ;
      m_pBuilder->PrintProperties( m_UID , Parameter ) ;
      FXSIZE iPos = Parameter.Find( '=' ) ;
      Parameter = Parameter.Mid( iPos + 1 ) ;
      m_SetupGrid.SetCellIntValue( m_GreyedItems[ i ] , Parameter ) ;
    }
  }
  CDialog::OnTimer( nIDEvent );
}

#undef THIS_MODULENAME

#define THIS_MODULENAME "ObjectStdSetup"

/////////////////////////////////////////////////////////////////////////////
// CObjectStdSetup dialog


CObjectStdSetup::CObjectStdSetup( CVideoObjectBase* pObject , LPCTSTR uid , CWnd* pParent /*=NULL*/ ) :
  CStdSetupDlg( IDD_STDDIALOG , pParent ) ,
  m_pObject( pObject ) 
{
  m_UID = uid ;
  m_DlgMode = SDM_VideoObject ;

  //{{AFX_DATA_INIT(CObjectStdSetup)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  m_pItemsData = new ITEM_DATA_ARRAY;
}

CObjectStdSetup::~CObjectStdSetup()
{
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

BOOL CObjectStdSetup::LoadSetup()
{
  m_ItemsCnt = 0;
  FXParser settings;
  ResetData() ;
  if ( !m_pObject->ScanSettings( m_LastSettings ) )
  {
    DestroyWindow();
    return FALSE;
  }
  
  if ( !CntRequired( false ) )
    return FALSE;

  m_pk.Empty() ;
  m_row.RemoveAll();

  for ( int i = 0 ; i < m_LastKeys.Count() ; i++ )
  {
    if ( !InsertLine( m_LastKeys[ i ] , m_LastLineParams[ i ] ) )
      break ;
  }
  EndLineIfNotEmpty() ;
  SetDlgSizeAndPos() ;
  return (m_ItemsCnt > 0) ;
}

void CObjectStdSetup::OnDestroy()
{
  CDialog::OnDestroy();
}

bool CObjectStdSetup::InsertLine( LPCTSTR key , LPCTSTR params )
{
  FXParser item_param( params );

  if ( !key && !params )
  {
    EndLineIfNotEmpty() ;
    return true ;
  }
  if ( m_pk.IsEmpty() )
    m_pObject->PrintProperties( m_pk );

  return CStdSetupDlg::InsertLine( key , params ) ;
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
    m_SetupGrid.DeleteAllItems();
    LoadSetup();
    CRect rect ;
    m_SetupGrid.GetWindowRect( &rect ) ;
    int iRotation = -iPos * WHEEL_DELTA ;
    SetCursorPos( rect.left , rect.top ) ;
    m_SetupGrid.PostMessage( WM_MOUSEWHEEL , (iRotation << 16) , (rect.right - 7) + (rect.CenterPoint().y << 16) ) ;
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



int CStdSetupDlg::CheckAndAddToGreyedList( LPCTSTR pParameterName )
{
  FXSIZE i = 0 ;
  for ( ; i < m_GreyedItems.Size() ; i++ )
  {
    if ( m_GreyedItems[ i ] == pParameterName )
      return 0 ;
  }
  return (int)m_GreyedItems.Add( pParameterName ) ;
}


int CStdSetupDlg::RemoveFromGreyedList( LPCTSTR pParameterName )
{
  // TODO: Add your implementation code here.
  return 0;
}
