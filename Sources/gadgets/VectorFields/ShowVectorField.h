// ShowVectorField.h: interface for the ShowVectorField class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHOWVECTORFIELD_H__EF837AEA_B4D0_4019_8BD6_7A1853F470B9__INCLUDED_)
#define AFX_SHOWVECTORFIELD_H__EF837AEA_B4D0_4019_8BD6_7A1853F470B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>
#include <classes\dpoint.h>

#define SCALE 10

class ShowVectorField : public CFilterGadget  
{
public:
	ShowVectorField();
	virtual void ShutDown();
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    bool    Pic2Scr(DPOINT& point, CPoint& res);
    DECLARE_RUNTIME_GADGET(ShowVectorField);
};

#endif // !defined(AFX_SHOWVECTORFIELD_H__EF837AEA_B4D0_4019_8BD6_7A1853F470B9__INCLUDED_)
