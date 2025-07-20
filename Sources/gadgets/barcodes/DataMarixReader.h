#if !defined(DataMatrixReader_INC)
#define DataMatrixReader_INC

#include <gadgets\gadbase.h>
#include <imageproc\recognition\MatrixReader.h>

class DataMatrixReader :
	public CFilterGadget
{
protected:
	CMatrixReader  m_Reader;
public:
	DataMatrixReader(void);
	virtual void ShutDown();

	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(DataMatrixReader);
};

#endif
