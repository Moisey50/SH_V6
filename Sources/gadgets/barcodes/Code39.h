#if !defined(CODE39_READER_INC)
#define CODE39_READER_INC

#include <gadgets\gadbase.h>

class Code39Reader :
	public CFilterGadget
{
public:
	Code39Reader(void);

	virtual void ShutDown();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(Code39Reader);
};

#endif //CODE39_READER_INC