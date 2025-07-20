// ImagesSorterGadget.h : Declaration of the ImagesSorterGadget class
#ifndef __INCLUDE__ImagesSorterGadgetGadget_H__
#define __INCLUDE__ImagesSorterGadgetGadget_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "RGBFramesSorter.h"

class ImagesSorterGadget
  : public UserBaseGadget
{
private:

#pragma region | Fields |
  FXLockObject  m_Lock;
  RGBFramesSorter m_sorter;

#pragma endregion | Fields |

  __forceinline bool SendContainerHandler(CDataFrame * pContainer)
  {
    bool bRes = true;

    bRes = PutFrame(GetOutputConnector(0), pContainer);

    return bRes;
  }

  __forceinline static bool SendContainerHandler(UserBaseGadget & source, CDataFrame * pContainer)
  {
    return ((ImagesSorterGadget&)source).SendContainerHandler(pContainer);
  }

protected:

public:
  DECLARE_RUNTIME_GADGET(ImagesSorterGadget);

	ImagesSorterGadget();
	~ImagesSorterGadget(); 


//	Mandatory functions
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);  
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
};

#endif	

