#ifndef Code128Reader_INC
#define Code128Reader_INC

#include <gadgets\gadbase.h>

class Code128Reader :
	public CFilterGadget
{
public:
	Code128Reader(void);
	virtual void ShutDown();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(Code128Reader);
};

#endif // Code128Reader_INC