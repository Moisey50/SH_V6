// GridCalbrator.h: interface for the CGridCalbrator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRIDCALBRATOR_H__0DEAADA0_DA51_4BD4_B892_F20F4BDF439A__INCLUDED_)
#define AFX_GRIDCALBRATOR_H__0DEAADA0_DA51_4BD4_B892_F20F4BDF439A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <setka\Grid.h>

class GridCalibrator : public CFilterGadget  
{
	CGrid m_Grid;
protected:
	GridCalibrator();
public:
	virtual void ShutDown();

private:
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(GridCalibrator);
};

#endif // !defined(AFX_GRIDCALBRATOR_H__0DEAADA0_DA51_4BD4_B892_F20F4BDF439A__INCLUDED_)
