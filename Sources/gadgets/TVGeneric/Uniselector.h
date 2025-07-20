// Uniselector.h: interface for the Uniselector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNISELECTOR_H__DB09EB99_C7DA_4D1C_AEA7_74ECFC8D43E0__INCLUDED_)
#define AFX_UNISELECTOR_H__DB09EB99_C7DA_4D1C_AEA7_74ECFC8D43E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>

class Uniselector : public CFilterGadget  
{
protected:
    CPtrArray        m_Outputs;
    int              m_OutputCnt;
    FXLockObject      m_Lock;
private:
	void DeleteOutputs();
	void InitOutputs();
public:
	         Uniselector();
	virtual void ShutDown();
    int      GetInputsCount();
    int      GetOutputsCount();
    CInputConnector*  GetInputConnector(int n);
    COutputConnector* GetOutputConnector(int n);

    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);

    bool PrintProperties(FXString& text);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool ScanSettings(FXString& text);

    DECLARE_RUNTIME_GADGET(Uniselector);
};

#endif // !defined(AFX_UNISELECTOR_H__DB09EB99_C7DA_4D1C_AEA7_74ECFC8D43E0__INCLUDED_)
