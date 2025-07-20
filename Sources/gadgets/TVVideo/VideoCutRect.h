#ifndef VideoCutRect_INC
#define VideoCutRect_INC

#include <Gadgets\VideoFrame.h>
#include <Gadgets\vftempl.h>
#include <gadgets\ControledFilter.h>

class VideoCutRect : public CControledFilter
{
    CRect m_Rect; 
    CRect m_LastInputRect ;
    BOOL m_b4PixelsStep ;
    BOOL m_bAllowROIControlFromVideoInput ;
protected:
	VideoCutRect();
private:
	virtual void ShutDown();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame, const CDataFrame* pParamFrame);
    bool ScanSettings(FXString& txt);
    bool ScanProperties(LPCTSTR txt, bool& Invalidate);
    bool PrintProperties(FXString& txt);
	DECLARE_RUNTIME_GADGET(VideoCutRect);
};

#endif
