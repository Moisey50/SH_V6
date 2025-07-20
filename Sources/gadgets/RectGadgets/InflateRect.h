#ifndef InflateRect_INC
#define InflateRect_INC
#include <gadgets\gadbase.h>

class InflateRect : 
	public CFilterGadget
{
protected:
	int	m_InflateX,m_InflateY;
private:
	InflateRect(void);
public:
	virtual void ShutDown();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
	virtual bool ScanSettings(FXString& text);

	DECLARE_RUNTIME_GADGET(InflateRect);
};

#endif