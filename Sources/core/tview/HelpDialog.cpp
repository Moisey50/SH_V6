// HelpDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include <gadgets\tview.h>
#include "helpdialog.h"
#include <gadgets\shkernel.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "HelpDialog"

extern CDynLinkLibrary* pThisDll;

DlgItem DI[] =
{
  { IDC_VIEWFRAME , BOTTOM_ALIGN | RIGHT_ALIGN , { 0 , 0 , 0 , 0 } , ( DWORD ) -1 }
};

CHelpDialog _hd;

char NotFoundHttp[] = "\
<html>\
<head>\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\">\
<meta http-equiv=\"Content-Language\" content=\"en-us\">\
<link rel=\"stylesheet\" type=\"text/css\" href=\"res://tvhelp.dll/1001.css\">\
<title>Test document</title>\
</head>\
<body>\
<table width=\"100%\" id=\"table1\" class=\"pagelike\">\
	<tr>\
		<th class=\"pg\">Gadget help</th>\
	</tr>\
	<tr>\
		<td height=\"113\">\
		<dl>\
			<dd>\
			<div>\
				<p>Error: Topic for gadget \"[GadgetName]\" is not found in \"[PluginName]\" plugin.</div>\
			</dd>\
		</dl>\
		</td>\
	</tr>\
</table>\
</body>\
</html>";


typedef unsigned( __stdcall *TVDB400_GET_HELP_ITEM )( LPCTSTR );

__forceinline CString GetHelpForGadget( LPCTSTR dllname , LPCTSTR gadgetname )
{
  CString retV;
  HINSTANCE hDll = AfxLoadLibrary( dllname );
  if ( !hDll )
  {
    SENDWARN_1( "Error: plugin DLL '%s' can't be loaded. Check dependencies!" , dllname );
    return retV;
  }
  TVDB400_GET_HELP_ITEM FnHelp = ( TVDB400_GET_HELP_ITEM ) GetProcAddress( ( HMODULE ) hDll , "?GetHelpItem@@YGIPBD@Z" );
  if ( FnHelp )
  {
    unsigned itemID = FnHelp( gadgetname );
    if ( itemID != -1 )
    {
      CString resName; resName.Format( "#%d" , itemID );
      HRSRC hRC = FindResource( hDll , resName , RT_HTML );
      if ( hRC ) // OK, we found it!
      {
        retV.Format( "res://%s\\Plugins\\%s.dll/%d" , FxGetAppPath() , dllname , itemID );
      }
      else
      {
        if ( FindResource( hDll , resName , _T( "MHTML" ) ) )
        {
          retV.Format( "res://%s\\Plugins\\%s.dll/MHTML/%d.mht" , FxGetAppPath() , dllname , itemID );
        }
        if ( FindResource( hDll , resName , _T( "PDF" ) ) )
        {
          retV.Format( "res://%s\\Plugins\\%s.dll/PDF/%d.pdf" , FxGetAppPath() , dllname , itemID );
        }
      }
    }
  }
  AfxFreeLibrary( hDll );
  return retV;
}

__forceinline bool LoadHtml2Memory( LPCTSTR dllname , int itemID , CString& html )
{
  HINSTANCE hDll = AfxLoadLibrary( dllname );
  if ( !hDll )
  {
    SENDWARN_1( "Error: plugin DLL '%s' can't be loaded. Check dependencies!" , dllname );
    return false;
  }

  CString resName; resName.Format( "#%d" , itemID );
  HRSRC hRC = FindResource( hDll , resName , RT_HTML );
  if ( hRC ) // OK, we found it!
  {
    HGLOBAL hres = LoadResource( hDll , hRC );
    DWORD szres = SizeofResource( hDll , hRC );
    LPCTSTR resstr = ( LPCTSTR ) LockResource( hres );
    if ( !resstr ) { AfxFreeLibrary( hDll ); return false; }
    LPTSTR buffer = html.GetBuffer( szres + 1 );
    memcpy( buffer , resstr , szres );
    buffer[ szres ] = 0;
    html.ReleaseBuffer();
    DeleteObject( hres );
  }
  AfxFreeLibrary( hDll );
  return true;
}

bool ShowHelp( int cmd , LPCTSTR topic )
{
  if ( !::IsWindow( _hd.GetSafeHwnd() ) )
  {
    if ( !_hd.Create( MAKEINTRESOURCE( IDD_HELP_DIALOG ) , NULL ) )
    {
      AfxMessageBox( "Failed to create Help Dialog" );
      return false;
    }
  }
  //_hd.SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  switch ( cmd )
  {
    case ID_HELP_INDEX:
    {
      CString IndexPath;
      //IndexPath.Format("res://%stvhelp.dll/10000",GetStartDir());
      //_hd.Navigate(IndexPath);
      _hd.Navigate( INDEX_ALIAS );
      //_hd.MemoryNavigate(_hd.GetIndexHtm());
      _hd.ShowWindow( SW_SHOWNORMAL );
      break;
    }
    case ID_CONTEXT_HELP:
    {
      if ( _tcscmp( topic , "@SÒUDIO" ) == 0 )
      {
        CString IndexPath;
        IndexPath.Format( "res://%s\\tvhelp.dll/10100" , FxGetAppPath() );
        _hd.Navigate( IndexPath );
        _hd.ShowWindow( SW_SHOWNORMAL );
      }
      else
      {
        CString message;
        message = _hd.LookUpHelp( topic );
        if ( message.GetLength() != 0 )
        {
          _hd.Navigate( message );
          _hd.ShowWindow( SW_SHOWNORMAL );
        }
        else
        {
          message = NotFoundHttp;
          message.Replace( "[GadgetName]" , topic );
          CString plugin = _hd.LookUpPlugin( topic );
          message.Replace( "[PluginName]" , plugin );
          _hd.MemoryNavigate( message );
          _hd.ShowWindow( SW_SHOWNORMAL );
        }
      }
      break;
    }
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// CHelpDialog dialog


CHelpDialog::CHelpDialog( CWnd* pParent /*=NULL*/ )
  : CResizebleDialog( CHelpDialog::IDD , pParent ) ,
  m_pView( NULL )
{
  //{{AFX_DATA_INIT(CHelpDialog)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}

CHelpDialog::~CHelpDialog()
{}

void CHelpDialog::DoDataExchange( CDataExchange* pDX )
{
  CResizebleDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CHelpDialog)
    // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CHelpDialog , CResizebleDialog )
  //{{AFX_MSG_MAP(CHelpDialog)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_SIZE()
  ON_WM_DESTROY()
  ON_BN_CLICKED( IDC_BACK , OnBack )
  ON_BN_CLICKED( IDC_FORWARD , OnForward )
  ON_BN_CLICKED( IDC_INDEX , OnIndex )
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHelpDialog message handlers

BOOL CHelpDialog::OnInitDialog()
{
  CResizebleDialog::OnInitDialog( DI , sizeof( DI ) / sizeof( DlgItem ) );
  m_hIcon = ::LoadIcon( pThisDll->m_hModule , MAKEINTRESOURCE( IDI_HELP ) );

// Set the icon for this dialog.  The framework does this automatically
//  when the application's main window is not a dialog
  SetIcon( m_hIcon , TRUE );			// Set big icon
  SetIcon( m_hIcon , FALSE );		// Set small icon
  EnableFitting();
  ShowWindow( SW_SHOW );

  // Create HTMLVIEW

  m_pClass = RUNTIME_CLASS( CHelpView );
  if ( m_pView != NULL ) return TRUE;
  m_pView = ( CHelpView* ) m_pClass->CreateObject();
  if ( !m_pView ) return FALSE;
  if ( !m_pView->Create( GetDlgItem( IDC_VIEWFRAME ) , 0 ) )
    return FALSE;
  CString rootPath;
  rootPath.Format( "res://%s\\tvhelp.dll/" , FxGetAppPath() );
  m_pView->SetDefaultBrowseDirectory( rootPath );

  PrepareDynamicPart();

  return TRUE;  // return TRUE unless you set the focus to a control
              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHelpDialog::OnDestroy()
{
  m_pView->DestroyWindow();
  CResizebleDialog::OnDestroy();
}

HCURSOR CHelpDialog::OnQueryDragIcon()
{
  return ( HCURSOR ) m_hIcon;
}

void CHelpDialog::OnPaint()
{
  if ( IsIconic() )
  {
    CPaintDC dc( this ); // device context for painting

    SendMessage( WM_ICONERASEBKGND , ( WPARAM ) dc.GetSafeHdc() , 0 );

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics( SM_CXICON );
    int cyIcon = GetSystemMetrics( SM_CYICON );
    CRect rect;
    GetClientRect( &rect );
    int x = ( rect.Width() - cxIcon + 1 ) / 2;
    int y = ( rect.Height() - cyIcon + 1 ) / 2;

    // Draw the icon
    dc.DrawIcon( x , y , m_hIcon );
  }
  else
  {
    CResizebleDialog::OnPaint();
  }
}

void CHelpDialog::OnSize( UINT nType , int cx , int cy )
{
  CResizebleDialog::OnSize( nType , cx , cy );
  CRect rc;
  if ( m_pView )
  {
    GetDlgItem( IDC_VIEWFRAME )->GetClientRect( rc );
    m_pView->MoveWindow( 0 , 0 , rc.right , rc.bottom );
  }
}

void CHelpDialog::PrepareDynamicPart()
{
  CString Index_htm;
  if ( LoadHtml2Memory( "tvhelp.dll" , 10000 , Index_htm ) )
  {
    IGraphbuilder*  pGraphBuilder = Tvdb400_CreateBuilder();
    CString gdgtListhtm;
    if ( pGraphBuilder )
    {
      IPluginLoader*	pPluginLoader = pGraphBuilder->GetPluginLoader();
      pPluginLoader->RegisterPlugins( pGraphBuilder );

      CStringArray Classes;
      CStringArray Plugins;
      pGraphBuilder->EnumGadgetClassesAndPlugins( Classes , Plugins );
      for ( int i = 0; i < Classes.GetSize(); i++ )
      {
          //TRACE("%s-->%s\n",Classes[i],Plugins[i]);
        m_Plugins.SetAt( Classes[ i ] , Plugins[ i ] );
        CString gadgetname = Classes[ i ].Mid( Classes[ i ].ReverseFind( '.' ) + 1 );
        //TRACE("%s\n", GetHelpForGadget(Plugins[i], gadgetname));
        if ( Plugins[ i ].GetLength() )
        {
          CString hlp = GetHelpForGadget( Plugins[ i ] , gadgetname );
          CString item;
          if ( hlp.GetLength() )
          {
            m_PluginHelps.SetAt( Classes[ i ] , hlp );
            item.Format( "<li><a href=\"%s\">%s</a></li>\n" , hlp , Classes[ i ] );
          }
          else
          {
            item.Format( "<p>%s\n" , Classes[ i ] );
          }
          gdgtListhtm += item;
        }
        else
        {
          TRACE( "!!!Warrning! Gadget %s has no corresponding dll\n" , Classes[ i ] );
        }
      }
      if ( Index_htm.Find( "[gadgets]" ) != -1 )
      {
        Index_htm.Replace( "[gadgets]" , gdgtListhtm );
        CString hlpdllpath; hlpdllpath.Format( "res://%s\\tvhelp.dll/1001.css" , FxGetAppPath() );
        Index_htm.Replace( "1001.css" , hlpdllpath );
        m_pView->CreateAliasedURL( INDEX_ALIAS , Index_htm );
      }
      pGraphBuilder->Release();
    }
  }
}

CString CHelpDialog::LookUpPlugin( LPCTSTR gadgetclass )
{
  CString retV;
  m_Plugins.Lookup( gadgetclass , retV );
  return retV;
}

CString CHelpDialog::LookUpHelp( LPCTSTR gadgetclass )
{
  CString retV;
  m_PluginHelps.Lookup( gadgetclass , retV );
  return retV;
}

void CHelpDialog::OnBack()
{
  if ( m_pView ) m_pView->GoBack();
}

void CHelpDialog::OnForward()
{
  if ( m_pView ) m_pView->GoForward();
}

void CHelpDialog::OnIndex()
{
  _hd.Navigate( INDEX_ALIAS );
}
