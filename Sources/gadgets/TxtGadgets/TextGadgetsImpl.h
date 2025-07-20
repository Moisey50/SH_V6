#ifndef TEXT_GADGEST_IMPL
#define TEXT_GADGEST_IMPL

#define THIS_MODULENAME "TextGadgetImpl"

#include <gadgets\gadbase.h>
#include <shbase\shbase.h>
#include <gadgets\TextFrame.h>
#include "TxtRenderSetupDialog.h"
#include <gadgets\QuantityFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\VideoFrame.h>
#include <gadgets\FigureFrame.h>
#include <gadgets\ContainerFrame.h>
#include <math\Intf_sup.h>
#include <helpers\FramesHelper.h>
#include <helpers\LockingQueue.h>
#include <thread>             // std::thread, std::this_thread::yield
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <iostream>
#include <fstream>

class SimpleTextRender : public CRenderGadget
{
  FXLockObject    m_Lock;
  CTextView*      m_wndOutput;
public:
  virtual void ShutDown();
  virtual void Create();
  virtual void Attach( CWnd* pWnd );
  virtual void Detach();
  CWnd* GetRenderWnd()
  {
    return m_wndOutput;
  }
  virtual void GetDefaultWndSize( RECT& rc )
  {
    rc.left = rc.top = 0; rc.right = 4 * DEFAULT_GADGET_WIDTH; rc.bottom = DEFAULT_GADGET_HEIGHT / 2;
  }
private:
  SimpleTextRender();
  virtual void Render( const CDataFrame* pDataFrame );

  DECLARE_RUNTIME_GADGET( SimpleTextRender );
};



class TextRender : public CRenderGadget
{
  CTermWindow   *m_Terminal;
  int            m_iLogFormat ;
  int            m_iTimeFormat ;
  RenderCallBack m_rcb;
  void*          m_wParam;
  FXLockObject   m_TermLock;
  int            m_bViewLabel ;
  int            m_bViewTiming ;
  int            m_bClearWindow ;
  bool           m_bTerminalProgrammed ;
public:
  virtual void Create();
  virtual void ShutDown();
  virtual void Attach( CWnd* pWnd );
  virtual void Detach();
  //    virtual int  DoJob() ;
  bool SetCallBack( RenderCallBack rcb , void* cbData )
  {
    m_rcb = rcb; m_wParam = cbData; return true;
  }
  CWnd*GetRenderWnd()
  {
    return m_Terminal;
  }
  void GetDefaultWndSize( RECT& rc )
  {
    rc.left = rc.top = 0; rc.right = 320; rc.bottom = 240;
  }
  virtual bool PrintProperties( FXString& text ) ;
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate ) ;
  virtual bool ScanSettings( FXString& text ) ;
private:
  TextRender();
  virtual void Render( const CDataFrame* pDataFrame );

  DECLARE_RUNTIME_GADGET( TextRender );
};



__forceinline FXString GetTxtFileName( LPCTSTR fmt )
{
  FXString retV = fmt;
  CTime t = CTime::GetCurrentTime();
  FXString tmpS;
  FXSIZE pos;
  while ( (pos = retV.Find( '%' )) >= 0 )
  {
    tmpS = retV.Mid( pos , 2 );
    if ( tmpS.GetLength() == 2 )
    {
      retV.Delete( pos , 2 );
      tmpS = (LPCTSTR) t.Format( tmpS );
      // remove forbidden chars
      tmpS.Replace( ':' , '-' );
      tmpS.Replace( '/' , '.' );
      tmpS.Replace( '\\' , ' ' );
      tmpS.Replace( '*' , '+' );
      tmpS.Replace( '|' , 'I' );
      tmpS.Replace( '?' , ' ' );
      tmpS.Replace( '>' , ' ' );
      tmpS.Replace( '<' , ' ' );
      tmpS.Replace( '"' , '\'' );

      retV.Insert( pos , tmpS );
    }
  }
  return retV;
}

UINT WriteFiguresThreadFunc( LPVOID lpData ) ;

class FigureForWriting
{
public:
  const CFigureFrame * m_pFigure ;
  TCHAR          m_FilePath[ 400 ] ;
  FigureForWriting( const CFigureFrame * pFig , LPCTSTR pName )
  {
    m_pFigure = pFig ;
    strcpy_s( m_FilePath , pName ) ;
  }
  FigureForWriting& operator=( FigureForWriting& Other )
  {
    m_pFigure = Other.m_pFigure ;
    memcpy( m_FilePath , Other.m_FilePath , sizeof( m_FilePath ) ) ;
    return *this ;
  }
};

class TextWriter : public CRenderGadget
{
  friend class CTxtRenderSetupDialog;
  friend  UINT     WriteFiguresThreadFunc( LPVOID lpData ) ;
protected:
  FXLockObject  m_SettingsLock ;
  bool          m_bOverwrite;
  bool          m_bLogMode ;
  bool					m_WriteSync;
  bool          m_bWriteFigureAsText ;
  bool          m_bFigThreadRun ;
  FXString      m_FileTemplate;
  FXString      m_PathName ;
  FXString      m_CrntFileName;
  FXFile        m_File;
  bool					m_ErrorSent;
  FXString      m_OpenedFileName ;
  std::mutex    m_FigureWriteMutex ;
  std::mutex    m_FiguresQueueNotEmpty ;
  std::condition_variable m_QueueNotEmptyCV ;

  std::mutex    m_NamesLock ;
  std::thread * m_pFigureWriteThread ;
  std::queue<FigureForWriting> m_FiguresForWriting ;
public:
  virtual void ShutDown();
  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ReceiveEOS( const CDataFrame* pDataFrame )
  {
    return (pDataFrame->GetId() == END_OF_STREAM_MARK) ;
  }
  bool ScanSettings( FXString& text );
  void CloseFile( bool bReporting = false );
private:
  TextWriter();
  virtual void Render( const CDataFrame* pDataFrame );

  DECLARE_RUNTIME_GADGET( TextWriter );
};


#endif