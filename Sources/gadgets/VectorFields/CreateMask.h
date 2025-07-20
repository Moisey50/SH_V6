// CreateMask.h: interface for the CreateMask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CREATEMASK_H__7816B67B_BA1F_45C5_B098_D0875544D0F0__INCLUDED_)
#define AFX_CREATEMASK_H__7816B67B_BA1F_45C5_B098_D0875544D0F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>

class CreateMask : public CFilterGadget  
{
public:
	CreateMask();
	virtual void ShutDown();
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    DECLARE_RUNTIME_GADGET(CreateMask);
};

#endif // !defined(AFX_CREATEMASK_H__7816B67B_BA1F_45C5_B098_D0875544D0F0__INCLUDED_)
