#ifndef AVERAGE_INTENSITY_GADGET
#define AVERAGE_INTENSITY_GADGET

#include <gadgets\gadbase.h>

class CCell {
public:
	int i;
	int j;
	POINT points[4];
	CCell() {};
	bool PtInRegion(POINT point);
	void GetRgnBox(LPRECT rect);
};

class AverageIntensity : public CFilterGadget
{
	CCell *m_Cells;
	long m_CellsSize;
	FXLockObject m_Lock;
	int xDim, yDim;
	FXString m_sVideoLabel;

	void RemoveCells();

	CInputConnector *m_pInputs[2];
public:
	AverageIntensity();

	void onText(CDataFrame* lpData);

	virtual int GetInputsCount() { return 2; }
	CInputConnector* GetInputConnector(int n) { return ((n<2)&&(n>=0))?m_pInputs[n]:NULL; }
	virtual int DoJob();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	virtual void ShutDown();

	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
	virtual bool ScanSettings( FXString& text );

	DECLARE_RUNTIME_GADGET(AverageIntensity);
};

#endif