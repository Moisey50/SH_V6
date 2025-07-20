// SetVersionInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SetVersionInfo.h"
#include "SetVersionInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetVersionInfoDlg dialog

CSetVersionInfoDlg::CSetVersionInfoDlg( CWnd* pParent /*=NULL*/ )
  : CDialog( CSetVersionInfoDlg::IDD , pParent )
{
  //{{AFX_DATA_INIT(CSetVersionInfoDlg)
  m_CompanyName = _T( "" );
  m_FileVersion = _T( "" );
  m_ProductVersion = _T( "" );
  m_Comments = _T( "" );
  m_LegalTradeMark = _T( "" );
  m_LegalCopyright = _T( "" );
  m_PrivateBuild = _T( "" );
  m_SpecialBuild = _T( "" );
  //}}AFX_DATA_INIT
  // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

void CSetVersionInfoDlg::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CSetVersionInfoDlg)
  DDX_Text( pDX , IDC_COMPANYNAME , m_CompanyName );
  DDX_Text( pDX , IDC_FILEVERSION , m_FileVersion );
  DDX_Text( pDX , IDC_PRODUCVERSION , m_ProductVersion );
  DDX_Text( pDX , IDC_COMMENTS , m_Comments );
  DDX_Text( pDX , IDC_LEGAL_TRADEMARK , m_LegalTradeMark );
  DDX_Text( pDX , IDC_LEGALCOPYRIGHT , m_LegalCopyright );
  DDX_Text( pDX , IDC_PRIVATEBUILD , m_PrivateBuild );
  DDX_Text( pDX , IDC_SPECILBUILD , m_SpecialBuild );
  //}}AFX_DATA_MAP
  DDX_Control( pDX , IDC_CHECK_FILES , m_FilesList );
}

BEGIN_MESSAGE_MAP( CSetVersionInfoDlg , CDialog )
  //{{AFX_MSG_MAP(CSetVersionInfoDlg)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED( IDC_PROCESS , OnProcess )
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
  ON_BN_CLICKED( IDC_CHECK_ALL , &CSetVersionInfoDlg::OnBnClickedCheckAll )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetVersionInfoDlg message handlers

BOOL CSetVersionInfoDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  SetIcon( m_hIcon , TRUE );
  SetIcon( m_hIcon , FALSE );

  m_FileVersion = AfxGetApp()->GetProfileString( _T( "root" ) , _T( "fileversion" ) , _T( "3,1,0,1" ) );
  m_ProductVersion = AfxGetApp()->GetProfileString( _T( "root" ) , _T( "productversion" ) , _T( "3,1,0,1" ) );
  m_CompanyName = AfxGetApp()->GetProfileString( _T( "root" ) , _T( "companyname" ) , _T( "File X Ltd." ) );
  m_Comments = AfxGetApp()->GetProfileString( _T( "root" ) , _T( "comments" ) , _T( "" ) );
  m_LegalTradeMark = AfxGetApp()->GetProfileString( _T( "root" ) , _T( "legaltrademark" ) , _T( "Trademark (R) File X Ltd. 2008" ) );
  m_LegalCopyright = AfxGetApp()->GetProfileString( _T( "root" ) , _T( "legalcopyright" ) , _T( "Copyright (C) File X Ltd. 2008-2017" ) );
  m_PrivateBuild = AfxGetApp()->GetProfileString( _T( "root" ) , _T( "privatebuild" ) , _T( "File X Ltd." ) );
  m_SpecialBuild = AfxGetApp()->GetProfileString( _T( "root" ) , _T( "specialbuild" ) , _T( "File X Ltd." ) );
  UpdateData( FALSE );

  CFileFind ff;
  if ( ff.FindFile( "SetVersionInfo.exe" ) )
  {
    ff.FindNextFile();
    m_ParentDir = ff.GetFilePath();
    int pos = m_ParentDir.ReverseFind( '\\' );
    m_ParentDir = m_ParentDir.Left( pos + 1 );
  }
  if ( !SeekFiles( "." ) )
    GetDlgItem( IDC_CHECK_FILES )->EnableWindow( FALSE );

  return TRUE;
}

void CSetVersionInfoDlg::OnDestroy()
{
  UpdateData( TRUE );
  VERIFY( AfxGetApp()->WriteProfileString( _T( "root" ) , _T( "fileversion" ) , m_FileVersion ) );
  VERIFY( AfxGetApp()->WriteProfileString( _T( "root" ) , _T( "productversion" ) , m_ProductVersion ) );
  VERIFY( AfxGetApp()->WriteProfileString( _T( "root" ) , _T( "companyname" ) , m_CompanyName ) );
  VERIFY( AfxGetApp()->WriteProfileString( _T( "root" ) , _T( "comments" ) , m_Comments ) );
  VERIFY( AfxGetApp()->WriteProfileString( _T( "root" ) , _T( "legaltrademark" ) , m_LegalTradeMark ) );
  VERIFY( AfxGetApp()->WriteProfileString( _T( "root" ) , _T( "legalcopyright" ) , m_LegalCopyright ) );
  VERIFY( AfxGetApp()->WriteProfileString( _T( "root" ) , _T( "privatebuild" ) , m_PrivateBuild ) );
  VERIFY( AfxGetApp()->WriteProfileString( _T( "root" ) , _T( "specialbuild" ) , m_SpecialBuild ) );
  CDialog::OnDestroy();
}

void CSetVersionInfoDlg::OnPaint()
{
  if ( IsIconic() )
  {
    CPaintDC dc( this );

    SendMessage( WM_ICONERASEBKGND , ( WPARAM )dc.GetSafeHdc() , 0 );

    int cxIcon = GetSystemMetrics( SM_CXICON );
    int cyIcon = GetSystemMetrics( SM_CYICON );
    CRect rect;
    GetClientRect( &rect );
    int x = ( rect.Width() - cxIcon + 1 ) / 2;
    int y = ( rect.Height() - cyIcon + 1 ) / 2;

    dc.DrawIcon( x , y , m_hIcon );
  }
  else
  {
    CDialog::OnPaint();
  }
}

HCURSOR CSetVersionInfoDlg::OnQueryDragIcon()
{
  return ( HCURSOR )m_hIcon;
}

void CSetVersionInfoDlg::OnProcess()
{
  //    SeekFiles(".");
  for ( int i = 0; i < m_FilesList.GetCount(); i++ )
  {
    if ( m_FilesList.GetCheck( i ) )
    {
      CString fileName;
      m_FilesList.GetText( i , fileName );
      ProcessFile( fileName );
    }
  }
}

inline CString GetFileExtension( const CString& fName )
{
  CString Ext;
  int pntPos = fName.ReverseFind( '.' );
  if ( ( pntPos > 0 ) && ( pntPos < fName.GetLength() ) )
  {
    Ext = fName.Mid( pntPos + 1 );
  }
  return Ext;
}

bool CSetVersionInfoDlg::SeekFiles( const char *path )
{
  CFileFind finder;
  CString fp = path;
  fp += "\\*.*";
  BOOL bWorking = finder.FindFile( fp );
  while ( bWorking )
  {
    bWorking = finder.FindNextFile();
    if ( finder.IsDots() ) continue;
    if ( finder.IsDirectory() )
    {
      SeekFiles( finder.GetFilePath() );
    }
    else
    {
      CString Extension = GetFileExtension( finder.GetFileName() );
      if ( Extension.CompareNoCase( "rc" ) == 0 )
      {
        //ProcessFile(finder.GetFilePath());
        CString fileName = finder.GetFilePath();
        fileName = fileName.Mid( m_ParentDir.GetLength() );
        m_FilesList.AddString( fileName );
      }
    }
  }
  finder.Close();
  return true;
}

void CSetVersionInfoDlg::ProcessFile( const char *path )
{
  TRACE( _T( "+++ Process file 's'\n" ) , path );
  SetFileName( path );
  UpdateData( TRUE );
  CFile fl;
  CFileException ex;
  TRACE( _T( "Process file %s\n" ) , path );
  if ( !fl.Open( path , CFile::modeReadWrite , &ex ) )
  {
    CString mes; mes.Format( _T( "Error: Can't open file: %s" ) , path );
    AfxMessageBox( mes );
    AddResult( _T( "Error" ) );
    return;
  }
  FSIZE len = fl.GetLength();
  if ( len )
  {
    char* data = ( char* )malloc( ( DWORD )len + 1 );
    data[ len ] = 0;
    char* eod = data + ( DWORD )len;
    fl.Read( data , ( DWORD )len );
    char* pntr = strstr( data , _T( "VS_VERSION_INFO" ) );
    if ( pntr )
    {
      char * pInfoBlockBegin = pntr ;
      CString Result ;
      if ( !ProcessVersionInfoBlock( fl , pntr , eod , &Result ) || Result.IsEmpty() )
        AddResult( _T( "Fail" ) );
      else
      {
        fl.SetLength( 0 );
        fl.Write( data , (UINT)(pInfoBlockBegin - data) );
        fl.Write( ( LPCTSTR )Result , (UINT) Result.GetLength() ) ;
        CString tmpS; 
        tmpS.Format( _T( "Success(%d)" ) , m_FieldChanged );
        AddResult( tmpS );
        fl.Write( pntr , (UINT)(eod - pntr) );
      }
    }
    else
      AddResult( _T( "Fail" ) );
    free( data );
  }
  fl.Close();
}

CString GetString( char *&pntr , char *eod )
{
  CString retVal;
  char eos[] = { 0x0D , 0x0A , 0x00 };

  char * e = NULL ;
  char * p = pntr ;
  while ( p < eod )
  {
    if ( *p == '\n' || *p == '\r' || *p == 0 )
    {
      e = p ;
      while ( p < eod && (*p == '\n' || *p == '\r' || *p == 0) )
        p++ ;
      break ;
    }
    retVal += *p ;
    p++ ;
  }
  if ( e == NULL )
  {
    pntr = eod ;
  }
  else if ( e < eod )
  {
    *e = 0;
    pntr = p ;
  }
  return retVal;
}

CString GetWord( CString& str )
{
  CString retVal = str;
  retVal.TrimLeft( _T( "\t ," ) );
  if ( retVal.GetLength() == 0 ) return retVal;
  int pos;
  if ( retVal[ 0 ] == _T( '\"' ) )
  {
    pos = retVal.Find( _T( "\"" ) , 1 );
    if ( pos >= 0 ) pos++;
  }
  else
    pos = retVal.FindOneOf( _T( "\t ," ) );
  if ( pos == -1 ) return retVal; // Return whole string
  retVal = retVal.Left( pos );
  retVal.TrimRight( _T( "\t ," ) );
  pos = str.Find( retVal );
  ASSERT( pos >= 0 );
  pos += retVal.GetLength();
  str = str.Right( str.GetLength() - pos );
  str.TrimLeft( _T( "\t " ) );
  return retVal;
}

void TrimOnEnd( CString& s , char* wrd )
{
  int pos = s.Find( wrd , 0 );
  ASSERT( pos >= 0 );
  pos += (int)strlen( wrd );
  while ( ( s[ pos ] == _T( ' ' ) ) && ( pos < s.GetLength() ) )
    pos++;
  s = s.Left( pos );
}

typedef struct _VALUES
{
  DWORD ID;
  char  Format[ 512 ];
}SValues;

SValues Values[] = {
  { COMPANYNAME_DONE , _T( "            VALUE \"CompanyName\", \"%s\\0\"" ) } ,
  { FILEVERSION_DONE , _T( "            VALUE \"FileVersion\", \"%s\\0\"" ) } ,
  { PRODUCTVERSION_DONE , _T( "            VALUE \"ProductVersion\", \"%s\\0\"" ) } ,
  { COMMENTS_DONE , _T( "            VALUE \"Comments\", \"%s\\0\"" ) } ,
  { LEGALTRADEMARK_DONE , _T( "            VALUE \"LegalTrademarks\", \"%s\\0\"" ) } ,
  { LEGALCOPYRIGHT_DONE , _T( "            VALUE \"LegalCopyright\", \"%s\\0\"" ) } ,
  { PRIVATEBUILD_DONE , _T( "            VALUE \"PrivateBuild\", \"%s\\0\"" ) } ,
  { SPECIALBUILD_DONE , _T( "            VALUE \"SpecialBuild\", \"%s\\0\"" ) }
};


bool CSetVersionInfoDlg::ProcessVersionInfoBlock( 
  CFile& fl , char *&pntr , char *eod , CString * pResult )
{
  TRACE( "Start processing file '%s'\n" , fl.GetFileName() );
  m_FieldChanged = 0;
  m_DoneBM = 0;
  bool end = false;
  CString endString;
  while ( !end )
  {
    if ( ( pntr >= eod ) || ( *pntr == 0 ) )
    {
      break;
    }
    CString str = GetString( pntr , eod );
    CString scn = str;
    CString cmd = GetWord( scn );
    if ( cmd.CompareNoCase( _T( "END" ) ) == 0 )
    {
      endString = str;
      break;
    }
    if ( cmd.CompareNoCase( _T( "FILEVERSION" ) ) == 0 )
    {
      TrimOnEnd( str , _T( "FILEVERSION" ) );
      CString Modified = m_FileVersion.Trim();
      Modified.Replace( _T('.') , _T(',') ) ;
      str = (cmd + _T(' ')) + Modified  ;
    }
    else if ( cmd.CompareNoCase( _T( "PRODUCTVERSION" ) ) == 0 )
    {
      TrimOnEnd( str , _T( "PRODUCTVERSION" ) );
      CString Modified = m_ProductVersion.Trim();
      Modified.Replace( _T( '.' ) , _T( ',' ) );
      str = ( cmd + _T( ' ' ) ) + Modified;
    }
    else if ( cmd.CompareNoCase( _T( "VALUE" ) ) == 0 )
    {
      CString nxtWrd = GetWord( scn );
      if ( nxtWrd.CompareNoCase( _T( "\"CompanyName\"" ) ) == 0 )
      {
        CString Val = GetWord( scn );
        TRACE( _T( "%s\n" ) , Val );
        str.Replace( Val , _T( "\"" ) + m_CompanyName + _T( "\\0\"" ) );
        m_FieldChanged++;
        m_DoneBM |= COMPANYNAME_DONE;
      }
      else if ( nxtWrd.CompareNoCase( _T( "\"FileVersion\"" ) ) == 0 )
      {
        CString Val = GetWord( scn );
        TRACE( _T( "%s\n" ) , Val );
        str.Replace( Val , _T( "\"" ) + m_FileVersion + _T( "\\0\"" ) );
        m_FieldChanged++;
        m_DoneBM |= FILEVERSION_DONE;
      }
      else if ( nxtWrd.CompareNoCase( _T( "\"ProductVersion\"" ) ) == 0 )
      {
        CString Val = GetWord( scn );
        TRACE( _T( "%s\n" ) , Val );
        str.Replace( Val , _T( "\"" ) + m_ProductVersion + _T( "\\0\"" ) );
        m_FieldChanged++;
        m_DoneBM |= PRODUCTVERSION_DONE;
      }
      else if ( nxtWrd.CompareNoCase( _T( "\"Comments\"" ) ) == 0 )
      {
        CString Val = GetWord( scn );
        TRACE( _T( "%s\n" ) , Val );
        str.Replace( Val , _T( "\"" ) + m_Comments + _T( "\\0\"" ) );
        m_FieldChanged++;
        m_DoneBM |= COMMENTS_DONE;
      }
      else if ( nxtWrd.CompareNoCase( _T( "\"LegalCopyright\"" ) ) == 0 )
      {
        CString Val = GetWord( scn );
        TRACE( _T( "%s\n" ) , Val );
        str.Replace( Val , _T( "\"" ) + m_LegalCopyright + _T( "\\0\"" ) );
        m_FieldChanged++;
        m_DoneBM |= LEGALCOPYRIGHT_DONE;
      }
      else if ( nxtWrd.CompareNoCase( _T( "\"LegalTrademarks\"" ) ) == 0 )
      {
        CString Val = GetWord( scn );
        TRACE( _T( "%s\n" ) , Val );
        str.Replace( Val , _T( "\"" ) + m_LegalTradeMark + _T( "\\0\"" ) );
        m_FieldChanged++;
        m_DoneBM |= LEGALTRADEMARK_DONE;
      }
      else if ( nxtWrd.CompareNoCase( _T( "\"PrivateBuild\"" ) ) == 0 )
      {
        CString Val = GetWord( scn );
        TRACE( _T( "%s\n" ) , Val );
        str.Replace( Val , _T( "\"" ) + m_PrivateBuild + _T( "\\0\"" ) );
        m_FieldChanged++;
        m_DoneBM |= PRIVATEBUILD_DONE;
      }
      else if ( nxtWrd.CompareNoCase( _T( "\"SpecialBuild\"" ) ) == 0 )
      {
        CString Val = GetWord( scn );
        TRACE( _T( "%s\n" ) , Val );
        str.Replace( Val , _T( "\"" ) + m_SpecialBuild + _T( "\\0\"" ) );
        m_FieldChanged++;
        m_DoneBM |= SPECIALBUILD_DONE;
      }
    }
    str += _T( "\r\n" );
    if ( pResult )
      pResult->Append( str ) ;
    else
      fl.Write( ( LPCTSTR )str , str.GetLength() );
  }
  // Insert fields originally missed
  for ( int i = 0; i < sizeof( Values ) / sizeof( SValues ); i++ )
  {
    if ( ( m_DoneBM & Values[ i ].ID ) == 0 )
    {
      CString tmpS;
      tmpS = GetValue( Values[ i ].ID );
      if ( tmpS.GetLength() != 0 )
      {
        tmpS.Format( Values[ i ].Format , GetValue( Values[ i ].ID ) );
        TRACE( "+++ %s\n" , tmpS );
        tmpS += _T( "\r\n" );
        if ( pResult )
          pResult->Append( tmpS ) ;
        else
          fl.Write( ( LPCTSTR )tmpS , tmpS.GetLength() );
        m_FieldChanged++;
      }
    }
  }
  endString += _T( "\r\n" );
  if ( pResult )
    pResult->Append( endString ) ;
  else
    fl.Write( ( LPCTSTR )endString , endString.GetLength() );
  
  while ( !end )
  {
    if ( ( pntr >= eod ) || ( *pntr == 0 ) )
    {
      break;
    }
    CString str = GetString( pntr , eod );
    str += _T( "\r\n" );
    if ( pResult )
      pResult->Append( str ) ;
    else
      fl.Write( ( LPCTSTR )str , str.GetLength() );
  }
  return true;
}

void CSetVersionInfoDlg::SetFileName( LPCTSTR name )
{
  CString tmpS;

  GetDlgItem( IDC_LISTFILES )->GetWindowText( tmpS );
  if ( tmpS.GetLength() ) tmpS += _T( "\r\n" );

  tmpS += name;
  GetDlgItem( IDC_LISTFILES )->SetWindowText( tmpS );
}

void CSetVersionInfoDlg::AddResult( LPCTSTR res )
{
  CString tmpS;
  GetDlgItem( IDC_LISTFILES )->GetWindowText( tmpS );
  tmpS += CString( _T( " " ) ) + res;
  GetDlgItem( IDC_LISTFILES )->SetWindowText( tmpS );
}

LPCTSTR CSetVersionInfoDlg::GetValue( DWORD id )
{
  switch ( id )
  {
    case COMPANYNAME_DONE:
      return m_CompanyName;
    case FILEVERSION_DONE:
      return m_FileVersion;
    case PRODUCTVERSION_DONE:
      return m_ProductVersion;
    case COMMENTS_DONE:
      return m_Comments;
    case LEGALTRADEMARK_DONE:
      return m_LegalTradeMark;
    case LEGALCOPYRIGHT_DONE:
      return m_LegalCopyright;
    case PRIVATEBUILD_DONE:
      return m_PrivateBuild;
    case SPECIALBUILD_DONE:
      return m_SpecialBuild;
    default:
      return _T( "Error" );
  }
}

void CSetVersionInfoDlg::OnBnClickedCheckAll()
{
  for ( int i = 0; i < m_FilesList.GetCount(); i++ )
  {
    m_FilesList.SetCheck( i , BST_CHECKED );
  }
}