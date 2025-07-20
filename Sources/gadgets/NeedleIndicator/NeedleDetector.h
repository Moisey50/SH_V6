#ifndef NeedleDetector_INC
#define NeedleDetector_INC

#include <gadgets\shkernel.h>

class NeedleDetector :
	public CFilterGadget
{
protected:
	int				  m_delta;
	COutputConnector* m_pTextOutput;
	int				  m_ScanLine;
	double			  m_FstPointValue;
	int				  m_FstPointX;
	double			  m_SdPointValue;
	int				  m_SdPointX;
private:
			NeedleDetector(void);
	int		SeekMax(const CVideoFrame* VideoFrame);
	double	GetValue(int pos);
public:
	virtual void		ShutDown();
	virtual int			GetOutputsCount() { return 2; }
	COutputConnector*	GetOutputConnector(int n) { return (n==0)?m_pOutput:m_pTextOutput; }
	CDataFrame*			DoProcessing(const CDataFrame* pDataFrame);

	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
	virtual bool ScanSettings(FXString& text);

protected:
	DECLARE_RUNTIME_GADGET(NeedleDetector);
};

#endif