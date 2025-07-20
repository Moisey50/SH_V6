// Pearson.h : Declaration of the Pearson class

#pragma once
#include <gadgets\gadbase.h>
#include <video\tvframe.h>

struct PearsonData
{
    LPBYTE  m_data;
    int     m_datasize;
    double  m_mean;
    double  m_sgm;
    void    Copy(LPBYTE data, int size);
};

class Pearson : public CFilterGadget
{
protected:
	CDuplexConnector * m_pDuplexConnector ;
    int          m_FrameCntr;
    int          m_Size;
    PearsonData *m_pData;
private:
    Pearson(void);
    void     ShutDown();
    void     FreeData();
    pTVFrame CalcPearson();
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);
	int GetDuplexCount();
	CDuplexConnector* GetDuplexConnector(int n);
	virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

	DECLARE_RUNTIME_GADGET(Pearson);
};
