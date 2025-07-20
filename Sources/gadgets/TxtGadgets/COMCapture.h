// COMCapture.h: interface for the COMCapture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMCAPTURE_H__3E7C8C46_44A2_4C3C_BADC_DDD14FA6AC84__INCLUDED_)
#define AFX_COMCAPTURE_H__3E7C8C46_44A2_4C3C_BADC_DDD14FA6AC84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <gadgets\TextFrame.h>
#include "helpers/SerialPort.h"

enum InOutMode
{
  IOM_Normal = 0 , // CR as Flag char
  IOM_HEX = 1 ,    // No flag
  IOM_CSV = 2 ,    // No flag
  IOM_NoFlag = 3 , // No flag char
  IOM_SpecialFlag = 4 // Flag char is not zero and not CR
};

class DataSamples
{
public:
  DataSamples( double dSampleTime = 0. , int iSampleLength = 0 )
  {
    m_dSampleTime = dSampleTime ;
    m_iSampleLength = iSampleLength ;
  }
  double m_dSampleTime ;
  int    m_iSampleLength ;
};


class COMCapture : public CCaptureGadget,
  public CSerialPort
{
protected:
  FXString            m_Buffer;
  FXString            m_RestData ;
  BYTE                m_ByteBuffer[4096] ;
  int                 m_iInByteBuffer ;
  InOutMode           m_InOutMode;
  volatile bool       m_IsRunning;
  HANDLE              m_DataReadyEvent;
  CRITICAL_SECTION    m_BufferCritSect;               
  CInputConnector   * m_pInputForTx ;
  bool                m_bClose ;
  int                 m_iCRPosInBuffer ;
  double              m_dLastSent ;
  double              m_dLastReceivedTime ;
  FXString            m_SelectedPortAsString;

  DataSamples         m_Samples[100] ;
  int                 m_iNSamples ;
public:
  COMCapture( int iFlagChar = '\n' );
  virtual void ShutDown();
  virtual void OnStart();
  virtual void OnStop();
  virtual CDataFrame* GetNextFrame(double* StartTime);
  bool ScanSettings(FXString& text);
  virtual bool ScanProperties( LPCTSTR text, bool& Invalidate) ;
  virtual bool PrintProperties(FXString& text) ;
  BOOL RxProc(LPVOID pParam, char *Data , int iLen);
  virtual int GetInputsCount() { return 1; } ;
  CInputConnector* GetInputConnector(int n) { return (n==0) ? m_pInputForTx : NULL ; } ;
  DECLARE_RUNTIME_GADGET(COMCapture);

  friend void CALLBACK fn_ToComPort( CDataFrame* pDataFrame ,
    void* pGadget , CConnector* lpInput = NULL ) ;

protected:
  bool ShowSetupDialog(CPoint& point);
//   bool GetComPortNames( FXStringArray& Ports ) ;
public:
  void TransmitText( LPCTSTR Text , int iLen );
};

#endif // !defined(AFX_COMCAPTURE_H__3E7C8C46_44A2_4C3C_BADC_DDD14FA6AC84__INCLUDED_)
