// CommandGadget.h: interface for the Command class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMANDGADGET_H__B98E0456_70DC_4385_8F0C_F141C9F2D219__INCLUDED_)
#define AFX_COMMANDGADGET_H__B98E0456_70DC_4385_8F0C_F141C9F2D219__INCLUDED_


#include <Gadgets\gadbase.h>
#include <Gadgets\TextFrame.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEFAULT_PREFIX CString("cmd /C")
#define DEFAULT_COMMAND_LINE CString("")

class Command : public CFilterGadget
{
  FXString              m_Prefix;
  FXString              m_Commandline;
  FXString              m_WindowName;
  FXString              m_SubWindowName ;
  int                   m_iDelay ;
  bool                  m_Busy;
  BOOL                  m_bAutoClick ;
public:
  Command();
  void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  bool ScanProperties( LPCTSTR text , bool& Invalidate );
  bool PrintProperties( FXString& text );
  bool ScanSettings( FXString& text );
  DECLARE_RUNTIME_GADGET( Command );
};

class PostMsg : public CFilterGadget
{
  HWND           m_hWnd ;
  double         m_dMinimalInterval ;
  double         m_dLastPostTime ;
  bool           m_bPostEnabled ;
  BOOL           m_LogMessages ;
  FXString       m_GadgetInfo ;
public:
  PostMsg();
  void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  bool ScanProperties( LPCTSTR text , bool& Invalidate );
  bool PrintProperties( FXString& text );
  bool ScanSettings( FXString& text );
  LPCTSTR GetGadgetInfo() ;
  DECLARE_RUNTIME_GADGET( PostMsg );
};

#endif // !defined(AFX_COMMANDGADGET_H__B98E0456_70DC_4385_8F0C_F141C9F2D219__INCLUDED_)
