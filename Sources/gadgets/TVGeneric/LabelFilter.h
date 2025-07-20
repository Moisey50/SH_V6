#ifndef LABELFILTER_INC
#define LABELFILTER_INC
#include <gadgets\gadbase.h>
#include <gadgets\containerframe.h>

#define COMPARE_EXACT 0
#define COMPARE_WITH_PREFIX 1
#define COMPARE_BEGIN 2
#define COMPARE_ANY   3

typedef struct 
{
  int iMode ;
  LPCTSTR ModeName ;
} CompareMode ;


class LabelFilter_FramesSet: public FXArray<CDataFrame*> {};
class LabelFilter :
    public CFilterGadget
{
protected:
    FXString    m_Label;
    FXLockObject m_Protect ;
     
    FXStringArray m_LastLabels ;
    FXStringArray m_PatternLabels ;
    FXLockObject m_Lock;
    int          m_TypeForFiltering ;
    int          m_CompareMode ;
    BOOL         m_bInverseRule ;
    FXString     m_Attributes ;
    BOOL         m_bAnalyzeLabels ;

public:
    LabelFilter(void);
public:
	virtual void ShutDown();
	bool PrintProperties(FXString& text);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool ScanSettings(FXString& text);
protected:
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    bool    CheckLabel(LPCTSTR label);
    void    InspectFrame(const CDataFrame* pDataFrame, LabelFilter_FramesSet& frames, bool AppendFrames);
	DECLARE_RUNTIME_GADGET(LabelFilter);
};

#endif