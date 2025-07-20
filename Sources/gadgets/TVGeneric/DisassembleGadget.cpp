// DisassembleGadget.cpp: implementation of the Disassemble class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DisassembleGadget.h"
#include "TVGeneric.h"

IMPLEMENT_RUNTIME_GADGET_EX(Disassemble, CFilterGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);
IMPLEMENT_RUNTIME_GADGET_EX(SplitConts, CFilterGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Disassemble::Disassemble():
m_OutputCnt(0)
{
  m_pInput = new CInputConnector(transparent);
  Resume();
}

void Disassemble::ShutDown()
{
  CFilterGadget::ShutDown();
  if (m_pInput)
    delete m_pInput;
  m_pInput = NULL;
  DeleteOutputs();
}

int Disassemble::GetInputsCount() 
{ 
  return (m_pInput)?1:0;
}

CInputConnector*  Disassemble::GetInputConnector(int n) 
{ 
  return (n) ? NULL : m_pInput;
}

int Disassemble::GetOutputsCount() 
{ 
  m_Lock.Lock();
  int count = (int) m_Outputs.GetSize();
  m_Lock.Unlock();
  return count;
}

COutputConnector* Disassemble::GetOutputConnector(int n) 
{ 
  COutputConnector* pOutput = NULL;
  m_Lock.Lock();
  if ((n >= 0) && (n < m_Outputs.GetSize()))
    pOutput = (COutputConnector*)m_Outputs.GetAt(n);
  m_Lock.Unlock();
  return pOutput;
}

bool Disassemble::PrintProperties(FXString& text)
{
  CFilterGadget::PrintProperties(text);
  FXPropertyKit pc;
  pc.WriteInt("OutputPins", m_OutputCnt);
  text+=pc;
  return true;

}

bool Disassemble::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  CString tmpS;
  CFilterGadget::ScanProperties(text, Invalidate);
  FXPropertyKit pc(text);
  m_OutputCnt=1;
  pc.GetInt( "OutputPins", m_OutputCnt);  
  InitOutputs();
  Status().WriteBool(STATUS_REDRAW, true);
  return true;
}

bool Disassemble::ScanSettings(FXString& text)
{
  text="template(Spin(OutputPins,1,999))";
  return true;
}

CDataFrame* Disassemble::DoProcessing(const CDataFrame* pDataFrame)
{
  CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(transparent);
  if (Iterator)
  {
    CDataFrame* dFrame = (CDataFrame*)Iterator->Next(DEFAULT_LABEL);
    int cnt=0;
    m_Lock.Lock();
    while (dFrame)
    {
      if (cnt<m_Outputs.GetSize())
      {
        dFrame->AddRef();
        if (!((COutputConnector*)m_Outputs.GetAt(cnt))->Put(dFrame))
          dFrame->RELEASE(dFrame);
        cnt++;
      }
      dFrame = (CDataFrame*)Iterator->Next(DEFAULT_LABEL);
    }
    delete Iterator;
    m_Lock.Unlock();
  }
  else 
  {
    int cnt=0;
    m_Lock.Lock();
    while (cnt<m_Outputs.GetSize())
    {
      ((CDataFrame*)pDataFrame)->AddRef();
      if (!((COutputConnector*)m_Outputs.GetAt(cnt))->Put((CDataFrame*)pDataFrame))
        ((CDataFrame*)pDataFrame)->Release();
      cnt++;
    }
    m_Lock.Unlock();
    return NULL;
  }
  return NULL;
}

void Disassemble::InitOutputs()
{
  m_Lock.Lock();
  if (m_Outputs.GetSize()>m_OutputCnt)
  {
    while (m_Outputs.GetSize()>m_OutputCnt)
    {
      int last = (int) m_Outputs.GetSize()-1;
      COutputConnector* pOutput = (COutputConnector*)m_Outputs.GetAt(last);
      m_Outputs.RemoveAt(last);
      delete pOutput;
    }
  }
  else if (m_Outputs.GetSize()<m_OutputCnt)
  {
    int i = (int)m_Outputs.GetSize();
    for (; i<m_OutputCnt; i++)
      m_Outputs.Add(new COutputConnector(transparent));        
  }
  m_Lock.Unlock();
}

void Disassemble::DeleteOutputs()
{
  m_Lock.Lock();
  while (m_Outputs.GetSize())
  {
    COutputConnector* pOutput = (COutputConnector*)m_Outputs.GetAt(0);
    m_Outputs.RemoveAt(0);
    delete pOutput ;
  }
  m_Lock.Unlock();
}

CDataFrame* SplitConts::DoProcessing(const CDataFrame* pDataFrame)
{
  CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(nulltype);
  if (Iterator)
  {
    CDataFrame* dFrame = (CDataFrame*)Iterator->NextChild(DEFAULT_LABEL);
    int cnt=0;
    m_Lock.Lock();
    while (dFrame)
    {
      if (cnt < (int) m_Outputs.GetSize())
      {
        dFrame->AddRef();
        if (!((COutputConnector*)m_Outputs.GetAt(cnt))->Put(dFrame))
          dFrame->RELEASE(dFrame);
        cnt++;
      }
      dFrame = (CDataFrame*)Iterator->NextChild(DEFAULT_LABEL);
    }
    delete Iterator;
    m_Lock.Unlock();
  }
  return NULL;
}
