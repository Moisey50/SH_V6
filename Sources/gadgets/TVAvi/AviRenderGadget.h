// AviRenderGadget.h: interface for the AviRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVIRENDERGADGET_H__335261BC_7B2E_4E85_A403_19E658DA711B__INCLUDED_)
#define AFX_AVIRENDERGADGET_H__335261BC_7B2E_4E85_A403_19E658DA711B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>
#include "files\AviFile.h"

typedef struct _tagAviFrameData
{
  CDataFrame* pDataFrame;
  CConnector* pInput;
}AviFrameData;

class AviRender : public CRenderGadget
{
  CDuplexConnector*	m_pControlPin;
  CAviFile		    m_File;
  CMapPtrToPtr	  m_Inputs;
  CMapPtrToPtr	  m_StreamsEnded;
  CPtrArray		    m_InputsIndex;
  FXString		    m_FileName;
  FXString        m_PreviousFileName ;
  BOOL			      m_bOverwrite;
  FXLockObject	  m_InputLock;
  DWORD           m_VIDC_FOURCC;
  bool            m_ErrorShown;
  FXStaticQueue<AviFrameData> m_Buffer;
  HANDLE          m_BufferReady;
  BOOL            m_CycleWritting;
  int             m_MaxFileNumber;
  int             m_MaxFileLength;
  int             m_CurrentFileNmb;
  BOOL            m_CalcFrameRate;
  BOOL            m_OverwriteFrameRate;
  int             m_iRenderId ;
  double          m_FrameRate;
  double          m_dFileOpenTime ;
  FXString        m_GadgetInfo ;
public:
  virtual void ShutDown();
  virtual int  GetInputsCount();
  virtual CInputConnector* GetInputConnector(int n);
  virtual bool PrintProperties(FXString& text);
  virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
  virtual bool ScanSettings(FXString& text);
  bool IsAviOpen();
  virtual int		    GetDuplexCount();
  CDuplexConnector* GetDuplexConnector(int n);
  int GetOutputsCount() { return 1 ; }
  COutputConnector * GetOutputConnector( int n ) { return ( n == 0 ) ? m_pOutput : NULL ; }
  LPCTSTR GetGadgetInfo() { return (LPCTSTR)m_GadgetInfo ; }
private:
  AviRender();
  virtual int DoJob();
  void    Input(CDataFrame* pDataFrame, CConnector* pInput);
  void    Write(CDataFrame* pDataFrame, CConnector* pInput);
  void	  AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pDataFrame);
  void    AddInput();
  void    RemoveInputs();
  void*   OpenStream(DWORD streamType, long id = -1);
  BOOL    OpenAvi(LPCTSTR fileName, BOOL bOverwrite);
  void    CloseAvi();
  void    ResetChunk();
  virtual bool ReceiveEOS(const CDataFrame* pDataFrame);
  friend class AviRenderDialog;
  friend void CALLBACK _fn_avi_render_input(CDataFrame* pDataFrame, void* pGadget, CConnector* lpInput)
  {
    ((AviRender*)pGadget)->Input(pDataFrame, lpInput);
  }
  DECLARE_RUNTIME_GADGET(AviRender);
};


#endif // !defined(AFX_AVIRENDERGADGET_H__335261BC_7B2E_4E85_A403_19E658DA711B__INCLUDED_)
