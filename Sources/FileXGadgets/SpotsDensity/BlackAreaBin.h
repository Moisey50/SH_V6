// BlackAreaBin.h : Declaration of the BlackAreaBin class

#ifndef __INCLUDE__BlackArea_H__
#define __INCLUDE__BlackArea_H__

#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets\videoframe.h"
#include "gadgets\QuantityFrame.h"

#ifndef UINT8
#define UINT8 unsigned char
#endif

class BlackAreaBin : public UserBaseGadget
{
private:

	double	countSpotsDensity			(UINT8* pProcessingPicture, unsigned int uiProcessingPictureSize);

protected:

	BlackAreaBin();

public:

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

	DECLARE_RUNTIME_GADGET(BlackAreaBin);
};

#endif	// __INCLUDE__BlackArea_H__

