// GRaphSettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include <gadgets\stdsetup.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME _T("GraphSettingsDialog")

typedef struct
{
  FXString m_Name ;
  double   m_dTime ;
} GadgetSample ;

typedef struct  
{
  double m_dTime;
  TCHAR  m_Key[20];
  TCHAR m_Name[50];
  TCHAR m_Params[1000];
} ParseSample ;

static GadgetSample Timing[ 300 ] ;
static ParseSample ParseTiming[300];
static int iParseIndex = 0;



/////////////////////////////////////////////////////////////////////////////
// CGraphSettingsDialog dialog

void OnGridEvent( int Event , void *wParam , int col , int row , int uData )
{
  ((CGraphSettingsDialog*) wParam)->OnGridEvent( Event , col , row , uData );
}


CGraphSettingsDialog::CGraphSettingsDialog( IGraphbuilder* gb , CWnd* pParent /*=NULL*/ )
  : CDialog( IDD_SETTINGS_DIALOG/*CGraphSettingsDialog::IDD*/ , pParent ) ,
  IDD( IDD_SETTINGS_DIALOG ) ,
  m_Builder( gb )
{
  //{{AFX_DATA_INIT(CGraphSettingsDialog)
  //}}AFX_DATA_INIT
}

BOOL CGraphSettingsDialog::Create( CWnd* pParentWnd )
{
//   int IDD = IDD_STDDIALOG;
  LPCTSTR lpszTemplateName = MAKEINTRESOURCE( IDD );
  //m_hInstSave = afxCurrentInstanceHandle;

  ASSERT( HIWORD( lpszTemplateName ) == 0 ||
    AfxIsValidString( lpszTemplateName ) );

  m_lpszTemplateName = lpszTemplateName;  // used for help
  if ( HIWORD( m_lpszTemplateName ) == 0 && m_nIDHelp == 0 )
    m_nIDHelp = LOWORD( (DWORD) (size_t)m_lpszTemplateName );
  HINSTANCE hInst = GetModuleHandle( STDSETUP_DLL_NAME );
  HRSRC hResource = ::FindResource( hInst , lpszTemplateName , RT_DIALOG );
  HGLOBAL hTemplate = LoadResource( hInst , hResource );
  BOOL bResult = CDialog::CreateIndirect( hTemplate , pParentWnd , hInst );
  FreeResource( hTemplate );

  return bResult;
}

bool CGraphSettingsDialog::Show( CPoint point , LPCTSTR uid )
{
  if ( !m_hWnd )
  {
    if ( !Create( NULL ) )
    {
      SENDERR_0( _T( "Failed to create Setup Dialog" ) );
      return false;
    }
  }
  m_SetupGrid.DeleteAllItems();
  if ( uid && *uid )
  {
    FXString Name( "Graph " ) ;
    Name += uid ;
    Name += " Settings";
    SetWindowText( Name );
  }

  LoadSetup();
  SetWindowPos( NULL , point.x , point.y , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
  ShowWindow( SW_SHOWNORMAL );
  return true;
}

void CGraphSettingsDialog::Delete()
{
  if ( GetSafeHwnd() )
    DestroyWindow();
  delete this;
}

BOOL CGraphSettingsDialog::LoadSetup()
{
  m_ItemsCnt = 0;
  iParseIndex = 0;
  FXParser val;
  m_SetupGrid.InitCtrl( ::OnGridEvent , this );

  m_SetupGrid.InsertColumn( 0 , _T( "Column1" ) , LVCFMT_LEFT , 200 );
  m_SetupGrid.InsertColumn( 1 , _T( "Column2" ) , LVCFMT_LEFT , 150 );

  CGridRow row;
  CGridCell cell;

  FXSIZE iPos = 0;
  int itemNo = 0 ;
  CString name ;
  //TRACE( "%s\n===\n" , (LPCTSTR) m_Data );

  double dStart = GetHRTickCount() ;
  FXSIZE iLen = m_Data.GetLength() ;
  while ( iPos >= 0
    && m_Data.GetNextElement( _T( "settings" ) , iPos , val ) )
  {
    name.Empty() ;
    ParseGadgetItem( val , name );
    Timing[ itemNo ].m_Name = (LPCTSTR) name ;
    double dStop = GetHRTickCount() ;
    Timing[ itemNo ].m_dTime = dStop - dStart ;
    dStart = dStop ;
    itemNo++;
    while ( iPos >= 0 && iPos < iLen 
      && (m_Data[ iPos ] == _T( ')' ) || m_Data[ iPos ] == _T( '\n' )) )
    {
      iPos++ ;
    }
    iParseIndex = 0;
  }
  return TRUE;
}

void CGraphSettingsDialog::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CGraphSettingsDialog)
  DDX_Control( pDX , IDC_SETUP_GRID , m_SetupGrid );
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP( CGraphSettingsDialog , CDialog )
  //{{AFX_MSG_MAP(CGraphSettingsDialog)
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphSettingsDialog message handlers

// BOOL CGraphSettingsDialog::OnInitDialog()
// {
//   m_ItemsCnt = 0;
//   CDialog::OnInitDialog();
// //   FXParser val;
// //   m_SetupGrid.InitCtrl( ::OnGridEvent , this );
// // 
// //   m_SetupGrid.InsertColumn( 0 , _T( "Column1" ) , LVCFMT_LEFT , 200 );
// //   m_SetupGrid.InsertColumn( 1 , _T( "Column2" ) , LVCFMT_LEFT , 150 );
// // 
// //   CGridRow row;
// //   CGridCell cell;
// // 
// //   int iPos = 0;
// //   int itemNo = 0 ;
// //   CString name ;
// //   TRACE( "%s\n===\n" , (LPCTSTR)m_Data );
// // 
// //   double dStart = GetHRTickCount() ;
// //   while ( iPos >= 0
// //     && m_Data.GetNextElement( _T( "settings" ) , iPos , val ) )
// //   {
// //     name.Empty() ;
// //     ParseGadgetItem( val , name );
// //     Timing[ itemNo ].m_Name = (LPCTSTR)name ;
// //     double dStop = GetHRTickCount() ;
// //     Timing[ itemNo ].m_dTime = dStop - dStart ;
// //     dStart = dStop ;
// //     itemNo++;
// //   }
//   return TRUE;
// }

bool CGraphSettingsDialog::ParseGadgetItem( FXParser& ip , CString& cname )
{
  FXParser    params;
  FXString    key;
  FXSIZE iPos = 0 ;
  double dStart = GetHRTickCount();

  while ( ip.GetNextElement( iPos , key , params ) )
  {
    if ( key == "name" )
    {
      cname += params;
    }
    else if ( key == "template" )
    {
      InsertStdDlg( cname , key , params );
    }
    else if ( key == "settings" )
    {
      CString Tmp = cname + '.' ;
      ParseGadgetItem( params , Tmp );
    }
    else
    {
      InsertSimple( cname , key , params );
    }
#ifdef _DEBUG
    FXstrcpy_s( ParseTiming[iParseIndex].m_Key , sizeof(ParseTiming[iParseIndex].m_Key) , key);
    FXstrcpy_s( ParseTiming[iParseIndex].m_Name , sizeof(ParseTiming[iParseIndex].m_Name), cname ) ;
    FXstrcpy_s( ParseTiming[iParseIndex].m_Params, sizeof(ParseTiming[iParseIndex].m_Params) , params );
    double dStop = GetHRTickCount();
    ParseTiming[iParseIndex].m_dTime = dStop - dStart;
    dStart = dStop;
    if ( ++iParseIndex > 299 )
      iParseIndex = 299 ;
#endif
  }
  return true;
}

void CGraphSettingsDialog::OnDestroy()
{
  CDialog::OnDestroy();
}


void CGraphSettingsDialog::SetData( LPCTSTR data )
{
  m_Data = data;
}

bool CGraphSettingsDialog::RemoveStdDlg( LPCTSTR gadgetname )
{
  CString name( gadgetname );
  TRACE( "Removing stdsetup for '%s' gadget!\n" , name );
  int startPos = -1;
  int i;
  CGridRow* gr = NULL;
  for ( i = 0; i < m_SetupGrid.GetItemCount(); i++ )
  {
    gr = m_SetupGrid.GetRow( i );
    if ( (gr) && ((*gr)[ 0 ].GetType() == CGridCell::typeIndent) )
    {
      if ( _tcscmp( (*gr)[ 0 ].GetText() , name ) == 0 )
      {
        startPos = i;
        TRACE( "Found indent %s\n" , (*gr)[ 0 ].GetText() );
        break;
      }
    }
  }
  i++;
  gr = m_SetupGrid.GetRow( i );
  while ( (gr) && ((*gr)[ 0 ].GetType() != CGridCell::typeIndent) )
  {
    TRACE( "Remove item %s\n" , (*gr)[ 0 ].GetText() );
    gr = m_SetupGrid.GetRow( i );
    itemData freeData;
    freeData.ctrlId = -1;
    m_ItemsData.SetAt( (*gr)[ 1 ].m_uData , freeData ); // mark free
    m_SetupGrid.DeleteItem( i );
  }

  // Load new one

  FXString key;
  FXParser param;
  FXParser settings;
  m_Builder->ScanSettings( name , settings );
  if ( settings.GetElementNo( 0 , key , param ) )
  {
    InsertStdDlg( name , key , param , startPos );
  }
  return true;
}

bool CGraphSettingsDialog::InsertStdDlg( LPCTSTR name , LPCTSTR key , 
  LPCTSTR params , int line )
{
  if ( _tcsicmp( key , _T( "template" ) ) == 0 )
  {
    CGridRow row;
    CGridCell cell;
    CGadget * pGadget = m_Builder->GetGadget( name ) ;
    FXString GadgetNameAndType = (
      (FXString(name) + _T(" (")) 
      + pGadget->GetRuntimeGadget()->m_lpszClassName) + _T(')') ;
    if ( line == -1 )
    {

      cell.DefineIndent( GadgetNameAndType );
      row.Add( cell );
      m_SetupGrid.AddRow( row );
      row.RemoveAll();
    }

    int iItemId = 0;
    FXSIZE iPos = 0 ;
    FXParser ip( params );
    FXString itemID;
    FXParser item_param;
    FXPropertyKit pk;
    m_Builder->PrintProperties( name , pk );
    while ( ip.GetNextElement( iPos , itemID , item_param ) )
    {
      if ( line != -1 )
        line++;
      if ( _tcsicmp( itemID , SETUP_COMBOBOX ) == 0 )
      {
        FXString tmpS;
        FXString lpName;
        FXParser lbItems;

        FXSIZE i = 0;
        item_param.GetWord( i , lpName );

        item_param.GetElementNo( 0 , lpName , lbItems );

        FXString iniVal;

        if ( (i != 0) && (pk.GetString( lpName , iniVal )) )
        {
          cell.DefineString( lpName , false );
          row.Add( cell );
          itemData iData;
          iData.ctrlId = SC_SELCHANGED;
          iData.itemId = name;
          iData.cmdId = lpName;
          int p = (int)m_ItemsData.Add( iData );

          cell.DefineListBox( lbItems , iniVal , p );
          row.Add( cell );
          if ( line == -1 )
            m_SetupGrid.AddRow( row );
          else
            m_SetupGrid.InsertRow( row , line );
          row.RemoveAll();
        }
      }
      else if ( _tcsicmp( itemID , SETUP_SPIN ) == 0 )
      {
        FXString tmpS;
        FXString lpName;
        int min , max;

        FXSIZE i = 0;
        item_param.GetWord( i , lpName );
        item_param.GetWord( i , tmpS );
        min = _ttoi( tmpS );
        item_param.GetWord( i , tmpS );
        max = _ttoi( tmpS );

        //m_Builder->PrintProperties(name,pk);
        int iniVal;

        if ( (i != 0) && (pk.GetInt( lpName , iniVal )) )
        {
          cell.DefineString( lpName , false );
          row.Add( cell );

          itemData iData;
          iData.ctrlId = SC_INTCHANGE;
          iData.itemId = name;
          iData.cmdId = lpName;
          int p = (int) m_ItemsData.Add( iData );

          cell.DefineInt( iniVal , min , max , p );
          row.Add( cell );
          if ( line == -1 )
            m_SetupGrid.AddRow( row );
          else
            m_SetupGrid.InsertRow( row , line );
          row.RemoveAll();
          m_ItemsCnt++;
        }
      }
      else if ( _tcsicmp( itemID , SETUP_SPINABOOL ) == 0 )
      {
        FXString tmpS;
        FXString lpName;
        int min , max;

        FXSIZE i = 0;
        item_param.GetWord( i , lpName );
        item_param.GetWord( i , tmpS );
        min = _ttoi( tmpS );
        item_param.GetWord( i , tmpS );
        max = _ttoi( tmpS );

        //m_Builder->PrintProperties(name,pk);
        FXString iniSVal;

        if ( (i != 0) && (pk.GetString( lpName , iniSVal )) )
        {
          bool autoV = false;
          int iniVal = 0;

          FXString SmallLetters = iniSVal.MakeLower() ;
          if ( SmallLetters.Find( _T( "auto" ) ) >= 0 )
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
            iniVal = _ttoi( iniSVal );

          cell.DefineString( lpName , false );
          row.Add( cell );

          itemData iData;
          iData.ctrlId = SC_INTCHKCHANGE;
          iData.itemId = name;
          iData.cmdId = lpName;
          int p = (int) m_ItemsData.Add( iData );

          cell.DefineIntChk( iniVal , min , max , autoV , p );
          row.Add( cell );
          if ( line == -1 )
            m_SetupGrid.AddRow( row );
          else
            m_SetupGrid.InsertRow( row , line );
          row.RemoveAll();
          m_ItemsCnt++;
        }
      }
      else if ( _tcsicmp( itemID , SETUP_EDITBOX ) == 0 )
      {
        FXString lpName;
        FXString iniVal;
        FXSIZE i = 0;

        item_param.GetWord( i , lpName );
        //m_Builder->PrintProperties(name,pk);
        if ( (i != 0) && (pk.GetString( lpName , iniVal )) )
        {
          cell.DefineString( lpName , false );
          row.Add( cell );

          itemData iData;
          iData.ctrlId = SC_EDITLSTFOCUS;
          iData.itemId = name;
          iData.cmdId = lpName;
          int p = (int) m_ItemsData.Add( iData );

          cell.DefineString( iniVal , true , p );
          row.Add( cell );
          if ( line == -1 )
            m_SetupGrid.AddRow( row );
          else
            m_SetupGrid.InsertRow( row , line );
          row.RemoveAll();
          m_ItemsCnt++;
        }
      }
      else
      {
        SENDERR_1( _T( "Error: Unknown control for StdSetupDlg '%s'" ) , itemID );
      }
      iItemId++;
    }
  }
  return false;
}


bool CGraphSettingsDialog::InsertSimple( LPCTSTR name , LPCTSTR key , LPCTSTR params )
{
  if ( (_tcsicmp( key , _T( "calldialog" ) ) == 0) && ((_tcsicmp( params , _T( "true" ) ) == 0) || (_ttoi( params ) != 0)) )
  {
    CGridRow row;
    CGridCell cell;
    cell.DefineIndent( name );
    row.Add( cell );
    m_SetupGrid.AddRow( row );

    row.RemoveAll();
    cell.DefineString( _T( "Call gadget setup dialog" ) , false );
    row.Add( cell );
    itemData iData;
    iData.ctrlId = CALL_FITTEDSETUP;
    iData.itemId = name;
    iData.cmdId = "";
    int p = (int) m_ItemsData.Add( iData );
    //int p=m_uData.Add(tmpS);
    cell.DefineButton( _T( "Gadget setup" ) , p );
    row.Add( cell );
    m_SetupGrid.AddRow( row );
  }
  return false;
}

BOOL CGraphSettingsDialog::PreTranslateMessage( MSG* pMsg )
{
  // TODO: Add your specialized code here and/or call the base class
  if ( pMsg->message == WM_KEYDOWN )
  {
    TRACE( "!!! %d\n" , pMsg->wParam );
    //m_ScrollWnd.SendMessage( WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
  }
  return CDialog::PreTranslateMessage( pMsg );
}

void CGraphSettingsDialog::OnGridEvent( int Event , int col , int row , int uData )
{
  bool Invalidate = false;
  if ( uData >= m_ItemsData.GetSize() )
  {
    SENDERR_0( _T( "Error: Unexpected error in OnGridEvent" ) );
    return;
  }
  switch ( m_ItemsData[ uData ].ctrlId )
  {
    case CALL_FITTEDSETUP:
    {
      RECT rc;
      GetWindowRect( &rc );
      CPoint point( rc.left , rc.top );
      FXString uid( m_ItemsData[ uData ].itemId );
      Tvdb400_ShowGadgetSetupDlg( m_Builder , uid , point );
      break;
    }
    case SC_SELCHANGED:
    {
      FXPropertyKit pk;
      pk.WriteString( m_ItemsData[ uData ].cmdId , m_SetupGrid.GetItemData( col , row ) );
      m_Builder->ScanProperties( m_ItemsData[ uData ].itemId , pk , Invalidate );
      break;
    }
    case SC_INTCHANGE:
    {
      FXPropertyKit pk;
      pk.WriteInt( m_ItemsData[ uData ].cmdId , m_SetupGrid.GetItemInt( col , row ) );
      m_Builder->ScanProperties( m_ItemsData[ uData ].itemId , pk , Invalidate );
      break;
    }
    case SC_INTCHKCHANGE:
    {
      FXPropertyKit pk;
      int val;
      bool enable;
      m_SetupGrid.GetItemIntChk( col , row , val , enable );
      if ( enable )
        pk.WriteString( m_ItemsData[ uData ].cmdId , _T( "auto" ) );
      else
        pk.WriteInt( m_ItemsData[ uData ].cmdId , val );
      m_Builder->ScanProperties( m_ItemsData[ uData ].itemId , pk , Invalidate );
      break;
    }
    case SC_EDITLSTFOCUS:
    {
      FXPropertyKit pk;
      pk.WriteString( m_ItemsData[ uData ].cmdId , m_SetupGrid.GetItemText( col , row ) );
      m_Builder->ScanProperties( m_ItemsData[ uData ].itemId , pk , Invalidate );
      break;
    }
    default:
      SENDERR_2( _T( "Error: Unsupported control in OnGridEvent ID: %d(0x%x)" ) , m_ItemsData[ uData ].ctrlId , m_ItemsData[ uData ].ctrlId );
  }
  if ( Invalidate ) // Setup for gadget have to be updated
  {
    CGraphSettingsDialog::RemoveStdDlg( m_ItemsData[ uData ].itemId );
  }
}


