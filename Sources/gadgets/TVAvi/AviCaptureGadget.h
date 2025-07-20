// AviCaptureGadget.h: interface for the AviCapture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVICAPTUREGADGET_H__45865FF2_0808_41FA_9F00_5121576C9F02__INCLUDED_)
#define AFX_AVICAPTUREGADGET_H__45865FF2_0808_41FA_9F00_5121576C9F02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>
#include "files\AviFile.h"

typedef struct _tagchunk
{
  FXString fName;
  FILETIME fTime;
  DWORD    dwOffset;
  DWORD    dwLength;
  double   dTimeOffset;
  double   dTimeLength;
  double   dRate;
}chunk;

class CAVIChunk
{
protected:
  FXArray<chunk,chunk> m_List;
  FXString             m_Path;
  int                  m_CurPos;
  DWORD                m_Length;
  double               m_TimeLength;
public:
  CAVIChunk();
  ~CAVIChunk();
  bool        List(LPCTSTR dir);
  FXString    GetNext();
  FXString    GetForPosition(DWORD pos);
  bool        IsEOD();
  void        Rewind() { m_CurPos=0; }
  DWORD       GetChunkOffset();
  DWORD       GetLength() { return m_Length; }
protected:
  bool        GetInfo(LPCTSTR fName, AVIFILEINFO& afi);
};

class AviCapture : public CCaptureGadget
{
  CAVIChunk         *m_AVIChunks;
  CAviFile			    m_AviFile;
  CMapPtrToPtr		  m_OutputStreams;
  CPtrArray			    m_Outputs;
  FXLockObject		  m_OutputLock;
  BOOL				      m_bSoftwareTrigger;
  BOOL              m_LoopFilm;
  BOOL              m_bMultiFile ;
  CInputConnector*	m_pInputTrigger;
  CDuplexConnector*	m_pControlPin;
  DWORD				      m_dwFrameRate;
  bool              m_ChunkMode;
  bool				      m_AddEOS;
  bool              m_ReserveOutputs;
  int               m_OutputsNumber;
  HANDLE            m_SWTriggerEvent ;
protected:
  AviCapture();
public:
  virtual void	ShutDown();
  virtual int		GetInputsCount();
  CInputConnector* GetInputConnector(int n);
  virtual int		GetOutputsCount();
  COutputConnector* GetOutputConnector(int n);
  virtual int		GetDuplexCount();
  CDuplexConnector* GetDuplexConnector(int n);
  virtual void	OnStop();
  virtual bool	PrintProperties(FXString& text);
  virtual bool	ScanProperties(LPCTSTR text, bool& Invalidate);
  bool			ScanSettings(FXString& text);
  void			AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pDataFrame);
private:
  virtual int DoJob();
  void		    SendData( bool bEOSOnly = false );
  BOOL		    OpenAvi(LPCTSTR fileName);
  void		    AddOutput(void* pStream);
  CConnector* GetOutput(DWORD aviType, int nOutput);
  void        RemoveOutput(int pos);
  void		    RemoveOutputs();
  void		    ToggleSoftwareTrigger();
  void		    TriggerPulse(CDataFrame* pDataFrame);
  CDataFrame* GetNextFrame(void* pStream);
  void		    StampDataFrame(CDataFrame* pDataFrame);
  bool        NextChunk();
  DWORD       GetLength();
  DWORD       GetPosition();
  void        SetPosition(DWORD pos);
  LPCTSTR     StepViaFile( int iDelta ) ;
protected:
  static friend void CALLBACK _fn_capture_trigger(CDataFrame* pDataFrame, void* pGadget, CConnector* lpInput)
  {
    ((AviCapture*)pGadget)->TriggerPulse(pDataFrame);
  }
  friend class AviCaptureDialog;
  DECLARE_RUNTIME_GADGET(AviCapture);
};

#endif // !defined(AFX_AVICAPTUREGADGET_H__45865FF2_0808_41FA_9F00_5121576C9F02__INCLUDED_)
