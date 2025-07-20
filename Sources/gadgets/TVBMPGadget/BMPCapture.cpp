// BMPCapture.cpp: implementation of the BMPCapture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <helpers/FramesHelper.h>
#include "BMPCapture.h"
#include <files\imgfiles.h>
#include <gadgets\VideoFrame.h>
#include <gadgets/textframe.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define THIS_MODULENAME "BMPCapture"

IMPLEMENT_RUNTIME_GADGET_EX( BMPCapture , CCaptureGadget , "Files.Capture" , TVDB400_PLUGIN_NAME );

BMPCapture::BMPCapture() :
m_ffWorking( FALSE ) ,
m_FrameRate( 25 ) ,
m_Loop( FALSE ) ,
m_LabelFullPath( FALSE ) ,
m_WatchNew( FALSE ) ,
m_ColorOrBW( 1 ) ,
m_DeleteRead( FALSE ) ,
m_RemoveNumberInlabel( 0 ) ,
m_IterationMode( FFIM_Native ) 
{
  m_pOutput = new COutputConnector( vframe ) ;
  m_pOutput->SetName( "BMPCapture output" ) ;
  m_pInputTrigger = new CInputConnector( nulltype , BMPCapture_capture_trigger , this );
  m_pDuplex = new CDuplexConnector(this, transparent, transparent);
  m_evSWTriggerPulse = ::CreateEvent( NULL , FALSE , FALSE , NULL );
  UseFileFilters();
}

void BMPCapture::ShutDown()
{
  CCaptureGadget::ShutDown();
  if ( m_pOutput )
    delete m_pOutput;
  m_pOutput = NULL;
  if ( m_evSWTriggerPulse )
  {
    FxReleaseHandle( m_evSWTriggerPulse );
    CloseHandle( m_evSWTriggerPulse );
    m_evSWTriggerPulse = NULL;
  }
  if ( m_pInputTrigger )
  {
    delete m_pInputTrigger;
    m_pInputTrigger = NULL;
  }
  FileFiltersDone();
}

void BMPCapture::OnStart()
{
  m_ffWorking = m_FF.FindFile( m_PathName , m_IterationMode );
  SetTicksIdle( IsTriggerByInputPin() ? 5 : 1000 / m_FrameRate );
  CCaptureGadget::OnStart();
}

void BMPCapture::OnStop()
{
  CCaptureGadget::OnStop();
}

CDataFrame* BMPCapture::GetNextFrame( double* pdStartTime )
{
  if ( !m_bRun || !pdStartTime)
    return NULL ;

  if ( IsTriggerByInputPin() )
  {
    HANDLE pEvents[] = { m_evExit , m_evSWTriggerPulse };
    DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );

    DWORD Res = ::WaitForMultipleObjects( cEvents , pEvents , FALSE , INFINITE ) ;
    if (Res != ( WAIT_OBJECT_0 + 1 ))
      return NULL ;
  }
  *pdStartTime = GetHRTickCount() ;
  CVideoFrame* vf = NULL;
  FXString NormalPath = FxUnregularize( m_PathName ) ;
  BOOL bFindMode = ( NormalPath.Find(_T('*')) >= 0
    || m_PathName.Find(_T('?')) >= 0);
  if ( m_bNameChanged )
  {
    m_ffWorking = m_FF.FindFile( m_PathName , m_IterationMode );
    m_bNameChanged = FALSE ;
  }
  else
  {
    if (!m_ffWorking || !bFindMode)
    {
      if (m_WatchNew)
        m_ffWorking = m_FF.FindNew();
      else
      {
        //       if (m_Loop || !bFindMode)
        if (m_Loop /*|| bFindMode*/)
          m_ffWorking = m_FF.FindFile( m_PathName , m_IterationMode );
      }
    }
  }
  if ( !m_ffWorking ) 
    return NULL;
  do
  {
    m_ffWorking = m_FF.FindNextFile();
  } while ( ( m_FF.IsDots() || m_FF.IsDirectory() ) && ( m_ffWorking ) );
  if ( !m_FF.IsDots() && !m_FF.IsDirectory() )
  {
    FILETIME WriteTime ;
    m_FF.GetLastWriteTime( &WriteTime ) ;
    CTime WTime( WriteTime ) ;
    CString Ext = m_FF.GetFileExt() ;
    LPBITMAPINFOHEADER bmih = NULL ;
    pTVFrame frame = NULL ;

    if ( (Ext == _T("tiff")) || Ext == _T("tif") )
    {
      frame = GetFromTiff( m_FF.GetFilePath() ) ;
    }
    else
    {
      bmih = ::loadDIB( m_FF.GetFilePath() );
      if ( bmih )
      {
        frame = new TVFrame;
        frame->lpBMIH = bmih;
        frame->lpData = NULL;
      }
    }
    if ( frame )
    {
      if ( m_ColorOrBW == 0 )
      {
        if ( frame->lpBMIH->biCompression == BI_YUV9 
          || frame->lpBMIH->biCompression == BI_YUV12)
        {
          frame->lpBMIH->biCompression = BI_Y8 ;
          frame->lpBMIH->biBitCount = 8 ;
          frame->lpBMIH->biSizeImage = frame->lpBMIH->biWidth * frame->lpBMIH->biHeight ;
        }
      }
      vf = CVideoFrame::Create( frame );
      if ( m_LabelFullPath )
        vf->SetLabel( m_FF.GetFilePath() );
      else
      {
        FXString Label = m_FF.GetFileName() ;
        if ( m_RemoveNumberInlabel )
        {
          if ( _istdigit( Label[ 0 ] ) )
          { 
            FXSIZE i = 1 ;
            for ( ; i < Label.GetLength() ; i++  )
            {
              if ( !_istdigit( Label[ i ] ) )
                break ;
            }
            if ( Label[ i ] == _T( '_' ) )
              i++ ;
            Label = Label.Mid( i ) ;
          }
        }
        
        vf->SetLabel( Label );
      }
      if ( m_WatchNew && m_DeleteRead )
      {
        if ( !DeleteFile( m_FF.GetFilePath() ) )
        {
          SENDWARN_1( "Can't delete file \"%s\"" , m_FF.GetFilePath() );
        }
      }
    }
    else
    {
      SENDWARN_1( "Can't read file \"%s\"" , m_FF.GetFilePath() );
    }
  }
  return vf;
}

bool BMPCapture::ScanSettings( FXString& text )
{
  FXString pattern( "template(" ) ;
  pattern += MainSettings ;

  pattern += (m_WatchNew) ? SubPattern1 : SubPattern2 ;
  pattern += ')' ;

  text.Format(
    pattern,
    TRUE, FALSE,
    TRUE, FALSE,
    TRUE, FALSE
  );

  /*if ( m_WatchNew )
    text.Format( "template(EditBox(FilePath)"
    ",Spin(FrameRate,1,25)"
    ",ComboBox(WatchNew(true(%d),false(%d)))"
    ",ComboBox(DeleteRead(true(%d),false(%d)))"
    ",ComboBox(LabelFullPath(true(%d),false(%d))))" ,
    TRUE , FALSE , TRUE , FALSE , TRUE , FALSE );
  else
    text.Format( "template(EditBox(FilePath)"
    ",Spin(FrameRate,1,25)"
    ",ComboBox(WatchNew(true(%d),false(%d)))"
    ",ComboBox(Loop(true(%d),false(%d)))"
    ",ComboBox(LabelFullPath(true(%d),false(%d)))"
    ",ComboBox(Remove#(No(0),Yes(1)))", 
    TRUE , FALSE , TRUE , FALSE , TRUE , FALSE );*/
  return true;
}

bool BMPCapture::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CCaptureGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text );
  FXString tmpS = m_PathName;
  if (pk.GetString( "FilePath", tmpS, false))
  {
    m_PathName = tmpS.Trim();
    m_ffWorking = FALSE;
    m_bNameChanged = TRUE ;
  }
  if (pk.GetString( "FileName", tmpS, false))
  {
    m_FileName = tmpS.Trim();
    FXString Unregularized = FxUnregularize( m_FileName ) ;
    Unregularized.Replace( "\\" , "/" ) ;
    m_FileName = Unregularized ;
    m_ffWorking = FALSE;
    m_bNameChanged = TRUE ;
  }
  if (pk.GetString( "Directory" , tmpS, false))
  {
    m_Directory = tmpS.Trim( " \t\\/\n\r" );
    FXString Unregularized = FxUnregularize( m_Directory ) ;
    Unregularized.Replace( "\\" , "/" ) ;
    m_Directory = Unregularized ;
    m_ffWorking = FALSE;
  }
  if ( m_FileName.IsEmpty() && m_Directory.IsEmpty() && !m_PathName.IsEmpty() )
  {
    FXString DirFromPath = FxExtractPath(m_PathName);
    if ( !DirFromPath.IsEmpty() )
      m_Directory = DirFromPath.Trim(" \t\\/\n\r");
    FXString FileNameFromPath = FxGetFileName(m_PathName);
    if ( !FileNameFromPath.IsEmpty() )
      m_FileName = (LPCTSTR)FileNameFromPath.Trim();
  }
  if ( !m_ffWorking && !m_FileName.IsEmpty() ) // form new path
  {
    if (!m_Directory.IsEmpty())
    {
      m_PathName = (m_Directory + '/') + m_FileName;
    }
    else
      m_PathName = m_FileName;
  }
  pk.GetInt( "FrameRate" , m_FrameRate );
  pk.GetInt( "Loop" , m_Loop );
  pk.GetInt( "LabelFullPath" , m_LabelFullPath );
  BOOL oldWatchNew = m_WatchNew;
  pk.GetInt( "WatchNew" , m_WatchNew );
  pk.GetInt( "Output" , m_ColorOrBW );
  pk.GetInt( "DeleteRead" , m_DeleteRead );
  pk.GetInt( "Remove#" , m_RemoveNumberInlabel );
  Invalidate = ( oldWatchNew != m_WatchNew );
  SetTicksIdle( IsTriggerByInputPin() ? 5 : 1000 / m_FrameRate );
  Status().WriteBool( STATUS_REDRAW , true );
  if (pk.GetInt("IterationMode", (int&)m_IterationMode))
    m_ffWorking = FALSE;
  int iGrab = 0 ;
  if ( pk.GetInt( "Grab" , iGrab) )
  {
    ::SetEvent( m_evSWTriggerPulse );
  }
  return true;
}

bool BMPCapture::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  CCaptureGadget::PrintProperties( text );
  pk.WriteString("FileName", m_FileName);
  pk.WriteString("Directory", m_Directory);
  pk.WriteInt( "FrameRate" , m_FrameRate );
  pk.WriteInt( "Loop" , m_Loop );
  pk.WriteInt( "LabelFullPath" , m_LabelFullPath );
  pk.WriteInt( "WatchNew" , m_WatchNew );
  pk.WriteInt( "Output" , m_ColorOrBW );
  pk.WriteInt( "DeleteRead" , m_DeleteRead );
  pk.WriteInt( "Remove#" , m_RemoveNumberInlabel );
  pk.WriteInt("IterationMode", (int)m_IterationMode);
  text += pk;
  return true;
}

void BMPCapture::CameraTriggerPulse( CDataFrame* pDataFrame )
{
  ::SetEvent( m_evSWTriggerPulse );
  pDataFrame->Release();
}

void BMPCapture::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if (!pParamFrame)
    return;
  CTextFrame* tf = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  const FXString cmdList = "list";
  const FXString cmdGet = "get";
  const FXString cmdSet = "set";

  if (tf)
  {
    FXParser pk = (LPCTSTR)tf->GetString();
    FXString cmd;
    FXString param;
    FXSIZE pos = 0;
    pk.GetWord(pos, cmd);
    CTextFrame* retV = CTextFrame::Create();
    retV->ChangeId( pParamFrame->GetId() );

    if (cmd.CompareNoCase(cmdList) == 0)
    {
      pk = MainSettings ;
      pk += ( m_WatchNew ) ? SubPattern1 : SubPattern2 ;
    }
    else
    {
      bool bSet = ( cmd.CompareNoCase( cmdSet ) == 0 ) ;
      bool bGet = ( cmd.CompareNoCase( cmdGet ) == 0 ) ;
      pk.Delete( 0 , 4 ) ;

      if ( bSet || bGet )
      {
        FXString Key = GetKey(pk);

        if ( !Key.IsEmpty() )
        {
          if ( bSet )
          {
            bool bDummy = false;
            retV->GetString() = ( ScanProperties( pk , bDummy ) ) ? "ok" : "error" ;
          }
          else if (cmd.CompareNoCase(cmdGet) == 0)
          {
            FXPropertyKit GetPK ;
            PrintProperties( GetPK ) ;
            FXString Value ;
            if ( GetPK.GetString( Key , Value ) )
            {
              Value.Insert( 0 , '=' ) ;
              retV->GetString() = Key + Value ;
            }
          }
          else
          {
            retV->GetString() = "List of available commands [all names are case sensitive]:\r\n"
              "list - returns list of changeable properties\r\n"
              "set <item name>=<value> - changes value of specified property\r\n"
              "get <item name> - returns current value of specified property\r\n";
          }
        }
      }
    }
    if ( retV->GetString().IsEmpty() )
      retV->GetString() = "error" ;
    PutFrame( m_pDuplex , retV ) ;
  }
  pParamFrame->Release(pParamFrame);
}

