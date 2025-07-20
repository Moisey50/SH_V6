// Amplifier.h : Declaration of the Amplifier and ChkRange classes

#pragma once
#include <gadgets\gadbase.h>
#include <gadgets\textframe.h>
#include <gadgets\QuantityFrame.h>
#include "helpers\FramesHelper.h"


class Amplifier : public CCollectorGadget
{
protected:
  CDuplexConnector * m_pDuplexConnector ;
  double   m_dKPlus ;
  double   m_dKMinus ;
  double   m_dOffset ;
  int      m_iAverage ;
  int      m_iNAccumulated ;
  double   m_dStdDev ;
  double   m_dSumOfSquares ;
  cmplx    m_cmplxStdDev ;
  double   m_dAveragedValue ;
  double   m_dAccumulated ;
  cmplx    m_cmplxAveragedValue ;
  BOOL     m_bCalcStd ;
  FXString m_Attributes ;
  FXString m_TextFormat ;
  FXString m_LastStdAsText ;

private:
  Amplifier(void);
  virtual void ShutDown();
public:
  bool ScanProperties(LPCTSTR text, bool& Invalidate);
  bool PrintProperties(FXString& text);
  bool ScanSettings(FXString& text);
  CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);
  int GetDuplexCount();
  CDuplexConnector* GetDuplexConnector(int n);
  virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  DECLARE_RUNTIME_GADGET(Amplifier);
};

class ChkRange : public CFilterGadget
{
protected:
  CDuplexConnector * m_pDuplexConnector ;
    // Properties
  FXString m_AttributesOK ;
  FXString m_AttributesNOK ;
  FXString m_TextFormat ;
  FXString m_InputFormat ;
  double   m_dRangeLow ;
  double   m_dRangeHigh ;
  double   m_dMaxTimeInterval_ms ;
  int      m_iMinBefore ;
  int      m_iMinAfter ;
  int      m_iNBigErrors ;
  BOOL     m_bResetID ;
  
  
  int      m_iAccumulated ;
  double   m_dAveraged ;
  double   m_dAccumulator ;
  double   m_dLastDataTime ;
  
  FXDblArray m_Samples ;


private:
  ChkRange(void);
  virtual void ShutDown();
public:
  bool ScanProperties(LPCTSTR text, bool& Invalidate);
  bool PrintProperties(FXString& text);
  bool ScanSettings(FXString& text);
  CDataFrame* DoProcessing(const CDataFrame * pDataFrame);
  int GetDuplexCount();
  CDuplexConnector* GetDuplexConnector(int n);
  virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  DECLARE_RUNTIME_GADGET(ChkRange);
};

