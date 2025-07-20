#ifndef LOCATORCLASS_INC
#define LOCATORCLASS_INC

#include <gadgets\shkernel.h>

#define ANGLE_180 0
#define ANGLE_360 1

class Locator :
	public CFilterGadget
{
protected:
	CPoint	       m_pnt;
    int            m_Angle;
    int            m_Radius;
private:
	Locator(void);
public:
	virtual void ShutDown();
	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
	virtual bool ScanSettings(FXString& text);
	
protected:
	DECLARE_RUNTIME_GADGET(Locator);
};

#endif //LOCATORCLASS_INC