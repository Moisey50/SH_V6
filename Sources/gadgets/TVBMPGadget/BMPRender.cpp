// BMPRender.cpp: implementation of the BMPRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVBMPGadget.h"
#include "BMPRender.h"
#include "files\imgfiles.h"
#include "gadgets\VideoFrame.h"
#include <gadgets/textframe.h>
#include <video\shvideo.h>
#include <WinError.h>
//#include <helpers\FramesHelper.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "BMPRender"

IMPLEMENT_RUNTIME_GADGET_EX( BMPRender , CRenderGadget , "Files.Render" , TVDB400_PLUGIN_NAME );

//////////////////////////////////////////////////////////////
//  Save ImageAsText
//////////////////////////////////////////////////////////////

bool saveImageAsCSV( LPCTSTR fname , LPBITMAPINFOHEADER bmih , LPBYTE pData = NULL )
{
  if ( !bmih )
    return false;

  LPBYTE src = ( pData ) ? pData : ( ( LPBYTE ) bmih ) + bmih->biSize;

  int iWidth = bmih->biWidth ;
  int iHeight = bmih->biHeight ;
  switch ( bmih->biCompression )
  {
    case BI_Y8:
    case BI_Y800:
    {
      FILE * pOutFile = _tfopen( fname , "w" ) ;
      if ( !pOutFile )
        return( FALSE );
      for ( int iY = 0 ; iY < bmih->biHeight ; iY++ )
      {
        LPBYTE pRow = src + iY * iWidth ;
        for ( int iX = 0 ; iX < iWidth ; iX++ )
        {
          fprintf_s( pOutFile , "%d," , *( pRow++ ) ) ;
        }
        fprintf_s( pOutFile , "\n" ) ;
      }
      fclose( pOutFile ) ;
      return true ;
    }
    break;
    case BI_Y16:
    {
      FILE * pOutFile = _tfopen( fname , "w" ) ;
      if ( !pOutFile )
        return( FALSE );
      for ( int iY = 0 ; iY < bmih->biHeight ; iY++ )
      {
        LPWORD pRow = ( ( LPWORD ) src ) + iY * iWidth ;
        for ( int iX = 0 ; iX < iWidth ; iX++ )
        {
          fprintf_s( pOutFile , "%d," , *( pRow++ ) ) ;
        }
        fprintf_s( pOutFile , "\n" ) ;
      }
      fclose( pOutFile ) ;
      return true ;
    }
  }
  return false;
}

bool saveBMP( const LPCTSTR fname , const CVideoFrame* vfr )
{
  bool isDone = false;
  bool toFreeMemory = true;
  LPBITMAPINFOHEADER lpBMP = NULL;

  SetLastError( ERROR_SUCCESS );

  if ( vfr )
  {
    switch ( vfr->lpBMIH->biCompression )
    {
      case BI_RGB:
        lpBMP = vfr->lpBMIH;
        toFreeMemory = false;
        break;
      case BI_YUV9:
        lpBMP = yuv9rgb24( vfr->lpBMIH , GetData( vfr ) );
        break;
      case BI_YUV12:
        lpBMP = yuv12rgb24( vfr->lpBMIH , GetData( vfr ) );
        break;
      case BI_Y8:
      case BI_Y800:
        lpBMP = y8rgb8( vfr->lpBMIH , GetData( vfr ) );
  //       lpBMP->biCompression = vfr->lpBMIH->biCompression ;
        lpBMP->biCompression = 0 ;
        lpBMP->biClrUsed = 256 ;
        break;
      case BI_Y16:
        lpBMP = y16rgb8( vfr->lpBMIH , GetData( vfr ) );
        //lpBMP->biCompression = BI_Y8 ;
        lpBMP->biCompression = 0 ;
        lpBMP->biClrUsed = 256 ;
        break;
    }
  }

  if ( !lpBMP )
    SetLastError( ERROR_INVALID_PARAMETER );
  else
  {
    isDone = saveDIB( fname , lpBMP );

    if ( toFreeMemory )
      free( lpBMP );

    lpBMP = NULL;
  }
  return isDone;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BMPRender::BMPRender()
  : m_FramesCntr( 0 )
  , m_Standard( StandardBMP )
  , m_AddLabel( NoLabel )
  , m_FileNameWithoutExt()
  , m_FileNameSuffix( FileNameSuffix::Counter )
{
  m_pInput = new CInputConnector( vframe );
  m_pDuplex = new CDuplexConnector( this , transparent , transparent );
  m_pOutput = new COutputConnector( transparent );
  SetMonitor( SET_INPLACERENDERERMONITOR );
  m_PathName = DEFAULT_FOLDER_PATH; //".\\BMP\\";
  m_FileExt = GetFileExtention( m_Standard );
  m_FNTemplate = CombineFileName( m_FileNameWithoutExt , m_FileNameSuffix ) + GetFileExtention( m_Standard ); // "%06d.bmp";
  Resume();
}

void BMPRender::ShutDown()
{
  CRenderGadget::ShutDown();

  delete m_pDuplex ;
  m_pDuplex = NULL ;
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;

}

FXString BMPRender::CombineFileName( const FXString& coreName , FileNameSuffix counterOrTimestmpTmp ) const
{
  FXString res;
  res = coreName + GetSuffixFormatStr( counterOrTimestmpTmp );

  return res;
}
FXString BMPRender::CombineFullFileName( const CVideoFrame* vf ,
  const FXString& folderPath , const FXString& coreFileName ,
  FileNameSuffix cntrOrTimestampTempl , DWORD framesCntr ,
  InsertLabel lblPos , const FXString& extention ) const
{
  FXString res;
  FXString fullFN;
  if ( vf )
  {
    if ( m_AddLabel == LabelIsPath )
    {
      FXString Label = vf->GetLabel() ;
      FXString DirsOnly = Label ;
      int iSlashPos = ( int ) DirsOnly.ReverseFind( _T( '\\' ) ) ;
      int iSlashPos2 = ( int ) DirsOnly.ReverseFind( _T( '/' ) ) ;
      if ( iSlashPos2 > iSlashPos )
        iSlashPos = iSlashPos2 ;
      DirsOnly = DirsOnly.Left( iSlashPos ) ;
      if ( !DirsOnly.IsEmpty() )
      {
        if ( !FxVerifyCreateDirectory( DirsOnly ) )
        {
          SENDERR( "Can't create '%s' folder , Label is %s." ,
            ( LPCTSTR ) DirsOnly , ( LPCTSTR ) Label );
        }
        else
          res = Label ;
      }
      else
        res = Label ;

      return res ;
    }
    else if ( !FxVerifyCreateDirectory( folderPath ) )
    {
      SENDERR_1( "Can't create directory the '%s' folder or sub-folder." , folderPath );
      return res ;
    }

    res = folderPath;

    if ( m_Standard == NoHeaders || m_Standard == CSV )
    {
      fullFN.Format( "X%d_Y%d_%s"
        , vf->lpBMIH->biWidth
        , vf->lpBMIH->biHeight
        , GetVideoFormatName( vf->lpBMIH->biCompression )
      );
    }

    fullFN += coreFileName;

    FXString suffix;
    switch ( cntrOrTimestampTempl )
    {
      case FileNameSuffix::Counter:
        suffix.Format( GetSuffixFormatStr( cntrOrTimestampTempl ) , framesCntr );
        break;
      case FileNameSuffix::TimeStamp:
        suffix = GetTimeStamp() ;
        break;
    }
    fullFN += suffix;

    FXString nonLbledName = fullFN;
    const FXString tmplt = "%s_%s";
    switch ( m_AddLabel )
    {
      case NoLabel: /*Template = FNTemplate;*/ break;
      case NumberFirst:
        fullFN.Format( tmplt
          , nonLbledName
          , vf->GetLabel() );
        break;
      case LabelFirst:
      case LabelFirstWithPath:
        fullFN.Format( tmplt
          , vf->GetLabel()
          , nonLbledName );
        break;
      case LabelIsPath:  // Not used (checked before)
        fullFN = vf->GetLabel() ;
        break ;
    }

    res += ( fullFN + extention );
  }
  return res;
}

bool PutFrameForBMP( COutputConnector * pPin , CDataFrame * pData , int iTimeout_ms = -1 )
{
  if ( iTimeout_ms >= 0 )
  {
    FXLockObject& Locker = pPin->GetLocker() ;
    if ( Locker.Lock( iTimeout_ms , "PutFrame" ) )
      Locker.Unlock() ;
    else
    {
      TRACE( "\nPutFrame: Can't lock pin ""%s"" " , pPin->GetName() ) ;
      pData->Release() ;
      return false ;
    }
  }
  if ( !pPin->Put( pData ) )
  {
    TRACE( "\nCan't send frame ""%s"" to pin ""%s"" " , pData->GetLabel() , pPin->GetName() ) ;
    pData->Release() ;
    return false ;
  }
  return true ;
}

void BMPRender::Render( const CDataFrame* pDataFrame )
{
  if ( Tvdb400_IsEOS( pDataFrame ) )
  {
    m_FramesCntr = 0;
  }
  else
  {
    const CVideoFrame* vf = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
    if ( vf )
    {
      FXString fullFilePath = CombineFullFileName(
        vf ,
        GetFolderPath() ,
        m_FileNameWithoutExt ,
        m_FileNameSuffix ,
        m_FramesCntr ,
        m_AddLabel ,
        m_FileExt
      );

      if ( !fullFilePath.IsEmpty() )
      {
        LPBITMAPINFOHEADER lpBMIH = vf->lpBMIH ;
        bool isDone = false;
        switch ( m_Standard )
        {
          case StandardBMP:
          {
            if ( ( lpBMIH->biCompression == BI_Y800 )
              || ( lpBMIH->biCompression == BI_Y8 )
              || ( lpBMIH->biCompression == BI_RGB )
              || ( lpBMIH->biCompression == BI_Y16 ) )
            {
              isDone = saveSH2BMP( fullFilePath , lpBMIH , vf->lpData ) ;
            }
            else
              isDone = saveBMP( fullFilePath , vf );
          }
          break;
          case Headers_And_Data:
          {
            isDone = saveDIB( fullFilePath , vf );
          }
          break;
          case Tiff:
          {
            isDone = saveSH2Tiff( fullFilePath , vf->lpBMIH , vf->lpData );
          }
          break;
          case JPG:
          {
            isDone = saveSH2StandardEncoder( SE_JPG , fullFilePath , vf->lpBMIH , vf->lpData );
          }
          break;
          case PNG:
          {
            isDone = saveSH2StandardEncoder( SE_PNG , fullFilePath , vf->lpBMIH , vf->lpData );
          }
          break;
          case NoHeaders:
          {
            isDone = saveNoHeaders( fullFilePath , vf->lpBMIH , vf->lpData );
          }
          break;
          case CSV:
          {
            isDone = saveImageAsCSV( fullFilePath , vf->lpBMIH , vf->lpData );
          }
          break;
        }

        FXString result;
        FXString outputStatusTmpl = "IsRendered=%s; Result=%s;";
        FXString outputStatus;
        result.Format( "Can't save the '%s' file; %s" , fullFilePath , FxLastErr2Mes( "The reason is '" , "'" ) );

        if ( !isDone )
          SENDERR( result );
        else
          result = fullFilePath;

        m_FramesCntr++;

        if ( m_pOutput && m_pOutput->IsConnected() )
        {
          outputStatus.Format( outputStatusTmpl , isDone ? "true" : "false" , result );
          CTextFrame* tfSavedFilePath = CTextFrame::Create( outputStatus );
          tfSavedFilePath->ChangeId( vf->GetId() );
          tfSavedFilePath->SetLabel( vf->GetLabel() );
          PutFrameForBMP( m_pOutput , tfSavedFilePath ) ;
        }
      }
    }
  }
}

int BMPRender::GetPathDelimiterLast( const FXString& filePath ) const
{
  FXSIZE pos = filePath.ReverseFind( _T( '\\' ) );
  FXSIZE iPos2 = filePath.ReverseFind( _T( '/' ) );
  if ( iPos2 > pos )
    pos = iPos2;

  return ( int ) pos;
}

const FXString& BMPRender::GetFolderPath()
{
  if ( m_PathName.IsEmpty() )
    m_PathName = DEFAULT_FOLDER_PATH;

  TCHAR LastChar = m_PathName[ m_PathName.GetLength() - 1 ];
  if ( LastChar != _T( '\\' ) && LastChar != _T( '/' ) )
    m_PathName += _T( '/' );
  return m_PathName;
}

bool BMPRender::HasFileName( const FXString& fn ) const
{
  return !GetTokenFileName( fn , true ).IsEmpty();
}
FXString BMPRender::GetTokenFileExt( const FXString& fn ) const
{
  FXString retV;
  FXSIZE pos = fn.ReverseFind( '.' );
  if ( pos > 0 )
    retV = fn.Mid( pos + 1 );

  return retV;
}
FXString BMPRender::GetTokenFileName( const FXString& fn , bool withExt ) const
{
  FXString retV = fn;
  FXSIZE pos = GetPathDelimiterLast( fn ); /*fn.ReverseFind( '\\' );
  int iPos2 = fn.ReverseFind( _T( '/' ) ) ;
  if ( iPos2 > pos )
    pos = iPos2 ;*/
  if ( pos >= 0 )
    retV = fn.Mid( pos + 1 );
  FXSIZE extPos = retV.ReverseFind( '.' );
  if ( !withExt && extPos > 0 )
    retV = retV.Left( extPos );

  return retV;
}
FXString BMPRender::GetTokenFolderPath( const FXString& filePath ) const
{
  FXString retV = filePath;
  //FXString fname = GetTokenFileName(filePath, true);
  if ( HasFileName( filePath ) )
  {
    FXSIZE pos = GetPathDelimiterLast( filePath );/* fn.ReverseFind('\\');
    FXSIZE iPos2 = fn.ReverseFind(_T('/'));
    if (iPos2 > pos)
      pos = iPos2;*/
    if ( pos >= 0 )
      retV = filePath.Left( pos + 1 );
    //else
    //  retV = fn;
  }
  return retV;
}

FXString BMPRender::GetSuffixFormatStr( FileNameSuffix fns )const
{
  FXString res;
  switch ( fns )
  {
    case FileNameSuffix::Counter:
      res = "%06d";
      break;
    case FileNameSuffix::TimeStamp:
      res = "%04d%02d%02d%02d%02d%02d%03d";
      break;
  }
  return res;
}
FXString BMPRender::GetFileExtention( BMPFormat fileFormat )const
{
  FXString res;
  FXString ext;
  switch ( fileFormat )
  {
    case BMPFormat::Headers_And_Data:
      ext = "bmp";
      break;
    case BMPFormat::StandardBMP:
      ext = "bmp";
      break;
    case BMPFormat::Tiff:
      ext = "tiff";
      break;
    case BMPFormat::JPG:
      ext = "jpg";
      break;
    case BMPFormat::PNG:
      ext = "png";
      break;
    case BMPFormat::NoHeaders:
      ext = "raw";
      break;
    case BMPFormat::CSV:
      ext = "csv";
      break;
  }

  if ( !ext.IsEmpty() )
    res.Format( ".%s" , ext );

  return res;
}

bool BMPRender::ScanSettings( FXString& text )
{
  text.Format(
    "template("
    "EditBox(%s)"//FileName
    ",ComboBox(%s(BMIH&Data[%s](%d),RGB[%s](%d),Tiff[%s](%d)"
    ",JPEG[%s](%d),PNG[%s](%d),RawNoHeader[%s](%d),CSV[%s](%d)))" //FormatBMP
    ",ComboBox(%s(No(%d),#First(%d),LabelFirst(%d),Label1stWithPath(%d),LabelIsPath(%d)))"//AddLabel
    ",EditBox(%s)"//FolderPath
    ",EditBox(%s)"//FileName_NoExt
    ",ComboBox(%s(Counter(%d),Timestamp(%d)))"//AddFileNameSuffix
    ")"
    , PROP_NAME_FILE_NAME
    , PROP_NAME_FORMAT_BMP
    , GetFileExtention( BMPFormat::Headers_And_Data )
    , BMPFormat::Headers_And_Data
    , GetFileExtention( BMPFormat::StandardBMP )
    , BMPFormat::StandardBMP
    , GetFileExtention( BMPFormat::Tiff )
    , BMPFormat::Tiff
    , GetFileExtention( BMPFormat::JPG )
    , BMPFormat::JPG
    , GetFileExtention( BMPFormat::PNG)
    , BMPFormat::PNG
    , GetFileExtention( BMPFormat::NoHeaders )
    , BMPFormat::NoHeaders
    , GetFileExtention( BMPFormat::CSV )
    , BMPFormat::CSV
    , PROP_NAME_ADD_LABEL
    , InsertLabel::NoLabel
    , InsertLabel::NumberFirst
    , InsertLabel::LabelFirst
    , InsertLabel::LabelFirstWithPath
    , InsertLabel::LabelIsPath
    , PROP_NAME_FOLDER_PATH
    , PROP_NAME_FILE_NAME_NO_EXT
    , PROP_NAME_ADD_FN_SUFFIX
    , FileNameSuffix::Counter
    , FileNameSuffix::TimeStamp
  );
  return true;
}

bool BMPRender::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  BOOL res = FALSE;
  CRenderGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text );
  FXString pathname;
  FXString folderPath;
  bool bNameChange = false ;

  if ( pk.GetInt( PROP_NAME_FORMAT_BMP , ( int& ) m_Standard ) )
  {
    res = TRUE ;
    Invalidate = true ;
  }
  res |= ( BOOL ) pk.GetInt( PROP_NAME_ADD_LABEL , ( int& ) m_AddLabel );
  bNameChange |= pk.GetInt( PROP_NAME_ADD_FN_SUFFIX , ( int& ) m_FileNameSuffix );
  BOOL isFNWOE = pk.GetString( PROP_NAME_FILE_NAME_NO_EXT , m_FileNameWithoutExt );
  res |= isFNWOE;
  bNameChange |= ( isFNWOE != FALSE ) ;

  m_FileExt = GetFileExtention( m_Standard );
  m_FNTemplate = CombineFileName( m_FileNameWithoutExt , m_FileNameSuffix ) + m_FileExt;

  bool isFPATH = pk.GetString( PROP_NAME_FOLDER_PATH , folderPath );
  bNameChange |= isFPATH ;
  Invalidate |= bNameChange ;
  if ( isFNWOE || isFPATH || !m_FileNameWithoutExt.IsEmpty() )
  {
    if ( isFPATH )
    {
      if ( folderPath.IsEmpty() )
        folderPath = DEFAULT_FOLDER_PATH;
      else
      {
        FXString Unregularized = FxUnregularize( folderPath ) ;
        Unregularized.Replace( "\\" , "/" ) ;
        folderPath = Unregularized ;
      }
      m_PathName = folderPath;
    }

    FXAutolock al( m_NameLock );
    m_FileNameWithoutExt = GetTokenFileName( m_FileNameWithoutExt , false );
    m_FNTemplate = m_FileNameWithoutExt + GetSuffixFormatStr( m_FileNameSuffix ) + m_FileExt;
  }

  if ( !isFNWOE /*&& !isFPATH*/ && pk.GetString( PROP_NAME_FILE_NAME , pathname ) )
  {
    FXAutolock al( m_NameLock );
    m_PathName = DEFAULT_FOLDER_PATH;
    if ( HasFileName( pathname ) )
    {
      m_PathName = GetTokenFolderPath( pathname );
      m_FNTemplate = GetTokenFileName( pathname , true );
      m_FileNameWithoutExt = GetTokenFileName( m_FNTemplate , false );

      if ( m_FNTemplate.Find( '%' ) < 0 )
        m_FNTemplate = m_FileNameWithoutExt + GetSuffixFormatStr( m_FileNameSuffix ) + m_FileExt;
    }
    else if ( !pathname.IsEmpty() )
    {
      m_PathName = pathname;
    }
    res |= TRUE;
  }

  return res;
}

bool BMPRender::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  CRenderGadget::PrintProperties( text );
  /*TCHAR LastChar = m_PathName[ m_PathName.GetLength() - 1 ] ;
  if ( LastChar != _T('\\') && LastChar != _T( '/' ) )
    m_PathName += _T( '\\' ) ;*/
  FXString fn = GetFolderPath() + m_FileNameWithoutExt ;

  fn += ( m_FileNameSuffix == Counter ) ? _T( "<Counter>" ) : _T( "<TimeStamp>" ) ;
  fn += m_FileExt ;
  pk.WriteString( PROP_NAME_FILE_NAME , fn );
  pk.WriteInt( PROP_NAME_FORMAT_BMP , ( int ) m_Standard );
  pk.WriteInt( PROP_NAME_ADD_LABEL , ( int ) m_AddLabel );
  pk.WriteString( PROP_NAME_FOLDER_PATH , GetFolderPath() );
  pk.WriteString( PROP_NAME_FILE_NAME_NO_EXT , m_FileNameWithoutExt );
  pk.WriteInt( PROP_NAME_ADD_FN_SUFFIX , ( int ) m_FileNameSuffix );
  text += pk;
  return true;
}

void BMPRender::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return;
  CTextFrame* tf = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  const FXString cmdList = "list";
  const FXString cmdGet = "get";
  const FXString cmdSet = "set";

  if ( tf )
  {
    FXParser pk = ( LPCTSTR ) tf->GetString();
    FXString cmd;
    FXString param;
    FXSIZE pos = 0;
    pk.GetWord( pos , cmd );

    if ( cmd.CompareNoCase( cmdList ) == 0 )
    {
      FXString list;
      list.Format( "%s %s = <File Name>;\r\n%s %s"
        , cmdSet
        , PROP_NAME_FILE_NAME
        , cmdGet
        , PROP_NAME_FILE_NAME );
      pk = list  ;
    }
    else
    {
      const FXString* pKey = GetKey( pk );

      if ( cmd.CompareNoCase( cmdSet ) == 0 )
      {
        FXSIZE iEquPos = pk.Find( *pKey , cmdSet.GetLength() + 1 );
        if ( iEquPos >= 0 && ( ( iEquPos += ( *pKey ).GetLength() ) < ( pk.GetLength() - 2 ) ) )
        {
          pk = pk.Mid( iEquPos );
          pk = pk.Trim( _T( " \t;\r\n" ) );
          if ( pk.GetLength() > 2 )
          {
            pk.Insert( 0 , *pKey );
            pk += _T( ';' );
            bool bDummy = false;
            ScanProperties( pk , bDummy );
            pk = "";
            /*TCHAR EndOfPath = m_PathName[ m_PathName.GetLength() - 1 ] ;
            if ( (EndOfPath != _T('\\')) && (EndOfPath != _T('/')) )
            m_PathName += _T('/');*/
            //FXString fn = GetFolderPath() + m_FNTemplate;
            //pk.Format("%s%s", key, fn);
          }
        }
      }
      else if ( cmd.CompareNoCase( cmdGet ) == 0 )
      {
        if ( pk.GetWord( pos , cmd )
          && cmd.CompareNoCase( *pKey ) == 0 )
          pk = "";
        else
          pk = "error";
        //{
        //  FXAutolock al(m_NameLock);
        //  //if ( m_PathName[ m_PathName.GetLength() - 1 ] != '\\' )
        //  //  m_PathName += '\\';
        //  FXString fn = GetFolderPath() + m_FNTemplate;
        //  pk.Format("%s=%s"
        //    , PROP_NAME_FILE_NAME
        //    , fn);
        //}          
      }
      else
      {
        pk = "List of available commands [all names are case sensitive]:\r\n"
          "list - returns list of changeable properties\r\n"
          "set <item name>=<value> - changes value of specified property\r\n"
          "get <item name> - returns current value of specified property\r\n";
      }

      if ( pk.IsEmpty() )
      {
        FXAutolock al( m_NameLock );
        FXString fn = GetFolderPath() + m_FNTemplate;
        pk.Format( "%s=%s" , *pKey , fn );
      }
    }
    CTextFrame* retV = CTextFrame::Create( pk );
    retV->ChangeId( NOSYNC_FRAME );
    if ( !m_pDuplex->Put( retV ) )
      retV->RELEASE( retV );

  }
  pParamFrame->Release( pParamFrame );
}
