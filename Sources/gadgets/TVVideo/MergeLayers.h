#pragma once
#include <gadgets\gadbase.h>
#include <gadgets\MetafileFrame.h>

/*
class CMFHelpers: public CArray<CMFHelper,CMFHelper&>
{
public:
    ~CMFHelpers() { RemoveAll(); }
};
*/

class MergeLayers :
	public CFilterGadget
{
	CMFHelpers  m_Metafiles;
	FXLockObject m_Lock;
	int m_Width,m_Height;
public:
	        MergeLayers(void);
	void    ShutDown();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	CVideoFrame* DoDraw(const CVideoFrame* vf);
	void    DrawMetafiles(HDC hdc);
	DECLARE_RUNTIME_GADGET(MergeLayers);
};

