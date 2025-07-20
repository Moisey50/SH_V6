// DisassembleGadget.h: interface for the Disassemble class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISASSEMBLEGADGET_H__042763F3_B299_45F6_ABCE_016783F6D5C8__INCLUDED_)
#define AFX_DISASSEMBLEGADGET_H__042763F3_B299_45F6_ABCE_016783F6D5C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets/gadbase.h>
#include <gadgets\ContainerFrame.h>

class Disassemble : public CFilterGadget  
{
protected:
  CPtrArray        m_Outputs;
  int              m_OutputCnt;
  FXLockObject      m_Lock;
private:
  void DeleteOutputs();
  void InitOutputs();
public:
  Disassemble();
  virtual void ShutDown();
  int      GetInputsCount();
  int      GetOutputsCount();
  CInputConnector*  GetInputConnector(int n);
  COutputConnector* GetOutputConnector(int n);

  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);

  bool PrintProperties(FXString& text);
  bool ScanProperties(LPCTSTR text, bool& Invalidate);
  bool ScanSettings(FXString& text);

  DECLARE_RUNTIME_GADGET(Disassemble);
};

class SplitConts: public Disassemble
{
public:
  SplitConts() {} ;
  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
  DECLARE_RUNTIME_GADGET(SplitConts);

};
#endif // !defined(AFX_DISASSEMBLEGADGET_H__042763F3_B299_45F6_ABCE_016783F6D5C8__INCLUDED_)
