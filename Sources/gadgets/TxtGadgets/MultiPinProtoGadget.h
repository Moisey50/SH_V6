#ifndef _MULTIPINPROTOGADGET_H
#define _MULTIPINPROTOGADGET_H


#pragma once
#include "gadgets\gadbase.h"
#include <gadgets\TextFrame.h>

class CDependentInput : public CConnector
{
  friend class CWire;
  friend class CSetOfInputConnectors ;
protected:
  HANDLE * m_pevHasData ;
  bool     m_bDataAvailable ;
public:
   CDependentInput(
    datatype dtype = nulltype , HANDLE * pEvHasData = NULL );
   virtual ~CDependentInput();
  bool Get(CDataFrame*& pDataFrame); // indirect input interface: gadget <- pin

private:
  void SetDataAvailable(bool bSet);
};

class CSetOfInputConnectors 
{
  friend class CMultiPinProtoGadget ;
private:
  CArray<CInputConnector*,CInputConnector*> m_Inputs ;
  HANDLE m_evClose;
  HANDLE m_evHasData;
public:
  CSetOfInputConnectors() ;
  virtual ~CSetOfInputConnectors() ;
  virtual int Add( CDependentInput * pNewInput ) ;
  virtual HANDLE GetHasDataEvent(){ return m_evHasData; } ;
  inline int GetInputCount() { return (int) m_Inputs.GetSize() ;}
};


class CMultiPinProtoGadget :
  public CGadget
{
private:
  CSetOfInputConnectors m_InArray ;
  CArray<COutputConnector*,COutputConnector*> m_Outputs ;
public:
  CMultiPinProtoGadget(void);
  virtual ~CMultiPinProtoGadget(void);
  virtual void OnStart();
  virtual void OnStop();
  inline int GetInputsCount() { return m_InArray.GetInputCount(); } ;
  virtual int GetOutputsCount();
  CInputConnector* GetInputConnector(int n);
  COutputConnector* GetOutputConnector(int n);
  void InitExecutionStatus(CExecutionStatus* Status);
private:
  virtual int DoJob();
  virtual CDataFrame* DoProcessing(CDataFrame* pDataFrame);
  DECLARE_RUNTIME_GADGET(CMultiPinProtoGadget);
};


#endif