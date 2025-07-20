// VFEnlarge.h: interface for the VFEnlarge class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VFENLARGE_H__19A5FDA1_7996_4CD4_8B76_B4FE07499356__INCLUDED_)
#define AFX_VFENLARGE_H__19A5FDA1_7996_4CD4_8B76_B4FE07499356__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>

class VFEnlarge : public CFilterGadget  
{
public:
	VFEnlarge();
	virtual void ShutDown();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    DECLARE_RUNTIME_GADGET(VFEnlarge);
};

#endif // !defined(AFX_VFENLARGE_H__19A5FDA1_7996_4CD4_8B76_B4FE07499356__INCLUDED_)
