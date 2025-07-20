// FLWRenderGadget.h: interface for the FLWRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLWRENDERGADGET_H__74E24A1B_7F40_4079_9F5F_56F98C7700B4__INCLUDED_)
#define AFX_FLWRENDERGADGET_H__74E24A1B_7F40_4079_9F5F_56F98C7700B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "flwgadget.h"
#include <gadgets\gadbase.h>
#include <gadgets\flwarchive.h>
#include "FLWLogRecord.h"


class FLWRenderStp;
class FLWRender : public CRenderGadget
{
private:
  CFLWArchive     m_Archive;
  FXString        m_FileNameTemplate;
  unsigned        m_LastError;
  FLWLog          m_Log ;
  BOOL            m_bDoLog ;
  COutputConnector * m_pOutput ;
  int             m_iNBuffers ;
  int             m_iBufferSize_KB ;
  int             m_iNOmittedFrames ;
  double          m_dLastErrorIndicatedTime ;
  int             m_iLastViewedNFrames ;
public:
  FLWRender();
  virtual void ShutDown();
  bool IsFileOpen();
  void CloseFile();
  virtual void Render( const CDataFrame* pDataFrame );
  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ScanSettings( FXString& text );
  virtual int GetOutputsCount() { return 1 ; }
  virtual COutputConnector * GetOutputConnector( int n ) { return ( n == 0 ) ? m_pOutput : NULL ; }
private:
  DECLARE_RUNTIME_GADGET( FLWRender );
};

#endif // !defined(AFX_FLWRENDERGADGET_H__74E24A1B_7F40_4079_9F5F_56F98C7700B4__INCLUDED_)
