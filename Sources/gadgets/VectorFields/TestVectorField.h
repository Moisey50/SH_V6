// TestVectorField.h: interface for the TestVectorField class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTVECTORFIELD_H__0BD1A9A4_4F3B_4784_B236_F903DB8EC1CB__INCLUDED_)
#define AFX_TESTVECTORFIELD_H__0BD1A9A4_4F3B_4784_B236_F903DB8EC1CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>

class TestVectorField : public CFilterGadget  
{
    TestVectorField();
public:
	virtual void ShutDown();
private:
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(TestVectorField);
};

#endif // !defined(AFX_TESTVECTORFIELD_H__0BD1A9A4_4F3B_4784_B236_F903DB8EC1CB__INCLUDED_)
