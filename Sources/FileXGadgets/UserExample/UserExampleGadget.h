// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__UserExampleGadget_H__
#define __INCLUDE__UserExampleGadget_H__


#pragma once
#include "helpers\UserBaseGadget.h"

class UserExampleGadget : public UserBaseGadget
{
protected:

	UserExampleGadget();

public:

	//	Example variables

	double		dProp1;
	long		lProp3;
	int			iProp4;
	bool		bProp4;
	FXString	sProp5;
	bool		bProp6;

	static const char* pList;

	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

	DECLARE_RUNTIME_GADGET(UserExampleGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

