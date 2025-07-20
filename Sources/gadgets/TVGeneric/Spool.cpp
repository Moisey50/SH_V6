// Spool.cpp : Implementation of the Spool class

#include "StdAfx.h"
#include "Spool.h"

#define THIS_MODULENAME "Spool"

class CBufferedInput: public CInputConnector
{
public:
  CBufferedInput(datatype dtype = nulltype, FN_SENDINPUTDATA fnSendInputData = NULL, void* lpHostGadget = NULL);
  int GetBufLen();
  bool SetBufLen(int len);
};

CBufferedInput::CBufferedInput(datatype dtype, FN_SENDINPUTDATA fnSendInputData, void* lpHostGadget):
  CInputConnector(dtype, fnSendInputData, lpHostGadget)
{
}

int CBufferedInput::GetBufLen()
{
  return m_FramesQueue.GetQueueSize();
}

bool CBufferedInput::SetBufLen(int len)
{
  return m_FramesQueue.ResizeQueue(len);
}

// Do replace "TODO_Unknown" to necessary gadget folder name in gadgets tree
IMPLEMENT_RUNTIME_GADGET_EX(Spool, CGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

Spool::Spool(void):
  m_pInput(NULL)
  ,m_pOutput(NULL)
  ,m_Timeout(10)
{
  m_pInput = new CBufferedInput(transparent);
  m_pOutput = new COutputConnector(transparent);
  Resume();
}

void Spool::ShutDown()
{
  CGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

int Spool::GetInputsCount()
{
  return (m_pInput)?1:0;
}

int Spool::GetOutputsCount()
{
  return (m_pOutput)?1:0;
}

CInputConnector* Spool::GetInputConnector(int n)
{
  return (!n) ? (CInputConnector*)m_pInput : NULL;
}

COutputConnector* Spool::GetOutputConnector(int n)
{
  return (!n) ? m_pOutput : NULL;
}

int Spool::DoJob()
{
  CDataFrame* pDataFrame = NULL;
  if ((m_pInput) && (m_pInput->Get(pDataFrame)))
  {
    ASSERT(pDataFrame);
    if ((m_pOutput) && (Tvdb400_IsEOS(pDataFrame))) // if EOS - just pass dataframe through without processing for common types gadgets
    {
      if (!m_pOutput->Put(pDataFrame))
        pDataFrame->Release();
      OnEOS();
    }
    else
    {
      DWORD ts=GetTickCount();
      while (!m_pOutput->Put(pDataFrame))
      {
        if ((GetTickCount()-ts)>(DWORD)m_Timeout)
        {
          pDataFrame->Release();
          break;
        }
      }
    }
  } 
  return WR_CONTINUE;
}

void Spool::OnEOS()
{
}

bool Spool::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  int i;

  FXPropertyKit pk(text);
  pk.GetInt( "Timeout" , m_Timeout ) ;
  if (pk.GetInt( "BufLen" , i ))
  {
    if (!m_pInput->SetBufLen(i))
    {
      SENDERR("Impossuble to set the new size of the buffer");
    }
  }
  return true;
}

bool Spool::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CGadget::PrintProperties(text);
  pk.WriteInt( "Timeout" , m_Timeout ) ;
  pk.WriteInt( "BufLen" , m_pInput->GetBufLen());
  text+=pk;
  return true;
}

bool Spool::ScanSettings(FXString& text)
{
  text = "template("
    "Spin(Timeout,3,40),"
    "Spin(BufLen,1,100))";
  return true;
}


