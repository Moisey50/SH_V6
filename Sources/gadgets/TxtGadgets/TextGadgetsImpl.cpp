#include "stdafx.h"
#include "TextGadgetsImpl.h"
#include "TxtGadgets.h"

IMPLEMENT_RUNTIME_GADGET_EX(SimpleTextRender , CRenderGadget, LINEAGE_TEXT, TVDB400_PLUGIN_NAME);
IMPLEMENT_RUNTIME_GADGET_EX(TextRender, CRenderGadget, LINEAGE_TEXT, TVDB400_PLUGIN_NAME);
IMPLEMENT_RUNTIME_GADGET_EX(TextWriter,   CRenderGadget, LINEAGE_TEXT, TVDB400_PLUGIN_NAME);

// SimpleTextRender implementation **********************************************************

SimpleTextRender::SimpleTextRender() : m_wndOutput( NULL )
{
  m_pInput = new CInputConnector( nulltype );
  Resume();
}

void SimpleTextRender::ShutDown()
{
  Detach();
  CRenderGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
}

void SimpleTextRender::Attach( CWnd* pWnd )
{
  Detach();
  m_wndOutput = new CTextView;
  m_wndOutput->Create( pWnd );
  m_wndOutput->SetText( "" );
}

void SimpleTextRender::Detach()
{
  VERIFY( m_Lock.LockAndProcMsgs() );
  if ( m_wndOutput )
  {
    if ( m_wndOutput->IsValid() )
      m_wndOutput->DestroyWindow();
    delete m_wndOutput; m_wndOutput = NULL;
  }
  m_Lock.Unlock();
}

void SimpleTextRender::Render( const CDataFrame* pDataFrame )
{
  FXAutolock lock( m_Lock );
  if ( (m_wndOutput == NULL) || (!m_wndOutput->IsValid()) ) return;
  m_wndOutput->Render( pDataFrame );
}

void SimpleTextRender::Create()
{}


// TextRender implementation *****************************************************

TextRender::TextRender() :
  m_Terminal( NULL ) ,
  m_rcb( NULL ) ,
  m_wParam( NULL ) ,
  m_bViewLabel( 0 ) ,
  m_bViewTiming( 0 ) ,
  m_bClearWindow( 0 ) ,
  m_bTerminalProgrammed( false )
{
  m_pInput = new CInputConnector( transparent );
  m_pInput->SetQueueSize( 50 ) ;
  Resume();
}

void TextRender::Render( const CDataFrame* pDataFrame )
{
  FXAutolock al( m_TermLock );
  if ( m_rcb )
  {
    m_rcb( pDataFrame , m_wParam );
    return;
  }
  if ( !pDataFrame->IsContainer() )
  {
    const CTextFrame * pText = pDataFrame->GetTextFrame();
    if ( pText )
    {
      LPCTSTR pLabel = pText->GetLabel();
      if ( pLabel && *pLabel )
      {
        if ( strstr( pLabel , "ToggleT" ) == pLabel )
        {
          if ( pText->GetString() == _T( "Set" ) )
            m_bViewTiming = true;
          else if ( pText->GetString() == _T( "Reset" ) )
            m_bViewTiming = false;
          else
            m_bViewTiming = !m_bViewTiming;
          if ( m_Terminal )
            m_Terminal->SetViewTiming( m_bViewTiming );
          return;
        }
        else if ( strstr( pLabel , "ToggleL" ) == pLabel )
        {
          if ( pText->GetString() == _T( "Set" ) )
            m_bViewLabel = true;
          else if ( pText->GetString() == _T( "Reset" ) )
            m_bViewLabel = false;
          else
            m_bViewLabel = !m_bViewLabel;
          if ( m_Terminal )
            m_Terminal->SetViewLabel( m_bViewLabel );
          return;
        }
      }
    }
  }

  if ( (!m_Terminal) || (!m_Terminal->IsValid()) )
    return;
  if ( !m_bTerminalProgrammed )
  {
    m_Terminal->SetViewLabel( m_bViewLabel != 0 ) ;
    m_Terminal->SetViewTiming( m_bViewTiming != 0 ) ;
    m_Terminal->SetClear( m_bClearWindow != 0 ) ;
    m_bTerminalProgrammed = true ;
  }
  m_Terminal->Render( pDataFrame );
}

void TextRender::Create()
{
  CRenderGadget::Create();
}

void TextRender::ShutDown()
{
  CRenderGadget::ShutDown();
  delete m_pInput; m_pInput = NULL;
  delete m_Terminal; m_Terminal = NULL;
}

void TextRender::Attach( CWnd* pWnd )
{
  Detach();
  m_Terminal = new CTermWindow();
  m_Terminal->Create( pWnd );
  CRenderGadget::Create();
}

void TextRender::Detach()
{
  VERIFY( m_TermLock.LockAndProcMsgs() );
  if ( m_Terminal )
  {
    if ( m_Terminal->IsValid() )
      m_Terminal->DestroyWindow();
    delete m_Terminal;
    m_Terminal = NULL;
  }
  m_TermLock.Unlock();
}

bool TextRender::PrintProperties( FXString& text )
{
  CRenderGadget::PrintProperties( text ) ;
  //     if ( m_Terminal )
  //    {
  FXPropertyKit pk;
  //       int bViewLabel = m_Terminal->IsViewLabel() ? 1 : 0;
  //       int bViewTiming = m_Terminal->IsViewTiming() ? 1 : 0;

  pk.WriteInt( "ViewLabel" , m_bViewLabel );
  pk.WriteInt( "ViewTiming" , m_bViewTiming );
  m_bClearWindow = 0 ;
  text += pk;
  return true;
  //     }
  //     return false;
}

bool TextRender::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CRenderGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text ) ;

  bool bRes = pk.GetInt( "ViewLabel" , m_bViewLabel ) ;
  bRes |= pk.GetInt( "ViewTiming" , m_bViewTiming ) ;
  bRes |= pk.GetInt( "ClearWindow" , m_bClearWindow ) ;
  if ( bRes )
    m_bTerminalProgrammed = false ;
  return true;
}

bool TextRender::ScanSettings( FXString& text )
{
  text.Format( "template("
    "Spin(ViewLabel,0,1)"
    ",Spin(ViewTiming,0,1)"
    ",Spin(ClearWindow,0,1)"
    ")" );
  return true;
}



// TextWRiter implementation ******************************************************************************

TextWriter::TextWriter() :
  m_WriteSync( true )
  , m_bFigThreadRun( true )
{
  SetMonitor( SET_INPLACERENDERERMONITOR ); // Always built-in (not a floating window)
  m_pInput = new CInputConnector( transparent );
  m_SetupObject = new CTxtRenderSetupDialog( this , NULL );
  m_ErrorSent = false;
  m_FileTemplate = "%X %x.txt";
  m_bLogMode = false ;
  m_bOverwrite = false ;
  m_bLogMode = false ;
  m_bWriteFigureAsText = FALSE ;
  Resume();
  m_pFigureWriteThread = new std::thread( WriteFiguresThreadFunc , this ) ;

}

void TextWriter::ShutDown()
{
  CRenderGadget::ShutDown();

  m_FigureWriteMutex.lock() ;
  while ( !m_FiguresForWriting.empty() )
    m_FiguresForWriting.pop() ;
  m_FigureWriteMutex.unlock() ;
  delete m_pInput;
  m_pInput = NULL;
  m_bFigThreadRun = false ;
  m_QueueNotEmptyCV.notify_one() ;
  m_pFigureWriteThread->join() ;
  delete m_pFigureWriteThread ;
  m_pFigureWriteThread = NULL ;
}

void TextWriter::Render( const CDataFrame* pDataFrame )
{
  FXFileException ex;

  if ( (m_File.m_pFile == FXFile::hFileNull) && (!Tvdb400_IsEOS( pDataFrame )) )
  {
    BOOL bFileExists = FALSE ;
    if ( !m_bLogMode )
      m_OpenedFileName.Empty() ;
    if ( m_bLogMode && !m_OpenedFileName.IsEmpty() )
    {
      m_NamesLock.lock() ;
      m_CrntFileName = m_OpenedFileName ;
      m_NamesLock.unlock() ;
    }
    else
    {
      m_NamesLock.lock() ;
      m_CrntFileName = GetTxtFileName( m_FileTemplate );
      FXString PathName = m_PathName ;
      m_NamesLock.unlock() ;
      if ( !PathName.IsEmpty() && !FxVerifyCreateDirectory( PathName ) )
      {
        SENDERR_1( "Can't create directory '%s'" , (LPCTSTR)PathName );
        return;
      }
      FXFileStatus fs;
      bFileExists = FXFile::GetStatus( m_CrntFileName , fs ) ;
      if ( bFileExists && (!m_bOverwrite && !m_bLogMode) )
      {
        if ( !m_ErrorSent )
        {
          SENDERR_1( "The file '%s' is already exist" , m_CrntFileName );
          m_ErrorSent = true;
        }
        return;
      }
    }

    if ( !m_File.Open( m_CrntFileName ,
      FXFile::modeWrite | FXFile::modeCreate | FXFile::modeNoTruncate , &ex ) )
    {
      if ( !m_ErrorSent )
      {
        SENDERR_1( "Can't create or open file '%s'" , m_CrntFileName );
        m_ErrorSent = true;
      }
      return;
    }
    else if ( !bFileExists && m_OpenedFileName != m_CrntFileName )
    {
      SENDINFO_1( "File '%s' open for writing" , m_CrntFileName );
      if ( m_bLogMode )
        m_OpenedFileName = m_CrntFileName ;
    }
    if ( !m_bOverwrite || m_bLogMode )
      m_File.SeekToEnd();
  }
  FXString outp;
  if ( Tvdb400_IsEOS( pDataFrame ) )
  {
    CloseFile( true ) ;
  }
  else
  {
    CFramesIterator * pIterator = pDataFrame->CreateFramesIterator( text ) ;
    bool bCloseFile = false ;
    FXString Result ;
    if ( m_bLogMode )
      Result = GetTimeAsString_ms() + ": " ;
    if ( pIterator )
    {
      const CTextFrame* pFrame = (const CTextFrame*) pIterator->Next( NULL ) ;
      while ( pFrame )
      {
        if ( m_WriteSync )
          outp.Format( "%d\t%s\t%s\r\n" , pFrame->GetId() , pFrame->GetLabel() , pFrame->GetString() );
        else
          outp.Format( "%s\r\n" , pFrame->GetString() );
        Result += outp ;
        if ( _tcscmp( pFrame->GetLabel() , _T( "CloseFile" ) ) == 0 )
          bCloseFile = true ;
        pFrame = (const CTextFrame*) pIterator->Next( NULL ) ;
      }

      delete pIterator ;
    }
    else
    {
      const CTextFrame* pTFrame = pDataFrame->GetTextFrame( DEFAULT_LABEL );
      if ( pTFrame )
      {
        bCloseFile = (_tcscmp( pTFrame->GetLabel() , _T( "CloseFile" ) ) == 0) ;
        FXSIZE iMsgLen = pTFrame->GetString().GetLength() ;

        bool bWrite = (iMsgLen > 5) || !bCloseFile ;

        if ( bWrite )
        {
          if ( m_WriteSync )
            outp.Format( "%d\t%s\r\n" , pDataFrame->GetId() , pTFrame->GetString() );
          else
            outp.Format( "%s\r\n" , pTFrame->GetString() );
          Result += outp ;
        }
      }
    }
    m_File.Write( (LPCTSTR) Result , Result.GetLength() );

    if ( bCloseFile )
    {
      m_OpenedFileName.Empty() ;
      CloseFile( true ) ;
    }
    else if ( m_bLogMode )
        CloseFile( false ) ;
  }
  if ( m_bWriteFigureAsText )
  {
    // check for figure frames
    CFramesIterator * pIterator = pDataFrame->CreateFramesIterator( figure ) ;
    if ( pIterator )
    {
      m_NamesLock.lock() ;
      FXString FileName ( m_PathName + 
        ((m_PathName.IsEmpty()) ? _T("./MFigures") :_T("/MFigures_") )
        + pDataFrame->GetLabel() + GetTimeAsString_ms() + _T(".txt") )  ;
      m_NamesLock.unlock() ;
      const CFigureFrame* pFrame = (const CFigureFrame*) pIterator->Next( NULL ) ;
      while ( pFrame )
      {
        ((CFigureFrame*) pFrame)->AddRef() ;
        FigureForWriting NextFigure( pFrame , FileName ) ;
        m_FigureWriteMutex.lock() ;
        m_FiguresForWriting.push( NextFigure ) ;
        m_FigureWriteMutex.unlock() ;
        std::unique_lock<std::mutex> lck( m_FiguresQueueNotEmpty );
        m_QueueNotEmptyCV.notify_one() ;
        pFrame = (const CFigureFrame*) pIterator->Next( NULL ) ;
      }
      delete pIterator ;
    }
    else
    {
      const CFigureFrame* pFrame = (const CFigureFrame*) pDataFrame->GetFigureFrame( NULL ) ;
      if ( pFrame )
      {
        m_NamesLock.lock() ;
        FXString FileName( m_PathName + _T( "\\Figure_" )
          + pDataFrame->GetLabel() + GetTimeAsString_ms() + _T( ".txt" ) )  ;
        m_NamesLock.unlock() ;
        ((CFigureFrame*) pFrame)->AddRef() ;
        FigureForWriting NextFigure( pFrame , FileName ) ;
        m_FigureWriteMutex.lock() ;
        m_FiguresForWriting.push( NextFigure ) ;
        m_FigureWriteMutex.unlock() ;
        std::unique_lock<std::mutex> lck( m_FiguresQueueNotEmpty );
        m_QueueNotEmptyCV.notify_one() ;
      }
    }
  }

}

void TextWriter::CloseFile( bool bReport )
{
  if ( bReport )
  {
    SENDINFO_1( "File '%s' Closed" , m_CrntFileName );
  }
  m_ErrorSent = false;
  if ( m_File.m_pFile != FXFile::hFileNull )
    m_File.Close();
}

bool TextWriter::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  pk.WriteBool( "Overwrite" , m_bOverwrite );
  pk.WriteBool( "WriteSync" , m_WriteSync );
  pk.WriteBool( "LogMode" , m_bLogMode );
  pk.WriteBool( "WriteFigureAsText" , m_bWriteFigureAsText );
  m_NamesLock.lock() ;
  pk.WriteString( "File" , m_FileTemplate ) ;
  m_NamesLock.unlock() ;

  text = pk;
  return true;
}

bool TextWriter::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  FXString fileName;
  pk.GetBool( "Overwrite" , m_bOverwrite );
  pk.GetBool( "WriteSync" , m_WriteSync );
  pk.GetBool( "LogMode" , m_bLogMode );
  pk.GetBool( "WriteFigureAsText" , m_bWriteFigureAsText );
  if ( pk.GetString( "File" , fileName ) && (fileName != m_FileTemplate) )
  {
    CloseFile();
    m_NamesLock.lock() ;
    m_FileTemplate = fileName;
    m_CrntFileName = GetTxtFileName( m_FileTemplate );
    m_PathName = FxExtractPath( m_FileTemplate );
    if ( !m_PathName.IsEmpty() && !FxVerifyCreateDirectory( m_PathName ) )
      SENDERR_1( "Can't create directory '%s' for figures" , (LPCTSTR)m_PathName );

    m_NamesLock.unlock() ;
  }
  return true;
}

bool TextWriter::ScanSettings( FXString& text )
{
  text = "calldialog(true)";
  return true;
}

UINT WriteFiguresThreadFunc( LPVOID lpData )
{
  TextWriter * pGadget = (TextWriter*) lpData;

  while ( pGadget->m_bFigThreadRun )
  {
    std::unique_lock<std::mutex> lck( pGadget->m_FiguresQueueNotEmpty );
    pGadget->m_QueueNotEmptyCV.wait( lck ); // Wait for queue filling

    while ( pGadget->m_FiguresForWriting.size() )
    {
      pGadget->m_FigureWriteMutex.lock() ;
      FigureForWriting NextFig = pGadget->m_FiguresForWriting.front() ;
      pGadget->m_FiguresForWriting.pop() ;
      pGadget->m_FigureWriteMutex.unlock() ;

      if ( NextFig.m_pFigure )
      {
        std::ofstream outfile;

        outfile.open( NextFig.m_FilePath , std::ios_base::app ); // append instead of overwrite
        if ( outfile.is_open() )
        {
          FXString Header ;
          Header.Format( "Figure Frame '%s' Id = %d Length=%d points\n" ,
            NextFig.m_pFigure->GetLabel() , NextFig.m_pFigure->GetId() , NextFig.m_pFigure->Count() ) ;
          outfile << (LPCTSTR) Header ;
          for ( int i = 0 ; i < NextFig.m_pFigure->Count() ; i++ )
          {
            const CDPoint& Pt = NextFig.m_pFigure->GetAt( i ) ;
            FXString AsString ;
            AsString.Format( "%d,%.3f,%.3f;\n" , i , Pt.x , Pt.y ) ;
            outfile << (LPCTSTR) AsString ;
          }
          outfile.close() ;
        }
        else
          SENDERR_1( "Can't open file '%s' for figure writing" , (LPCTSTR) NextFig.m_FilePath );
        ((CFigureFrame*) (NextFig.m_pFigure))->Release() ;
      }
    }
  }

  return 0 ;
}

