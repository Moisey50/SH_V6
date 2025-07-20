// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "UserExampleGadget.h"

USER_FILTER_RUNTIME_GADGET(UserExampleGadget,"UserExample");	//	Mandatory

const char* UserExampleGadget::pList = "AA1; AA2; AA3; BB1; BB2; BB3; CC3";	//	Example

UserExampleGadget::UserExampleGadget()
{
	//	Example

	dProp1 = 123.5;
	lProp3 = 345678;
	iProp4 = 345;
	bProp4 = true;
	sProp5 = "ABCDEFG";
	bProp6 = true;

	//	Mandatory

	init();
}

CDataFrame* UserExampleGadget::DoProcessing(const CDataFrame* pDataFrame) 
{
	return (CDataFrame*) pDataFrame;
}

void UserExampleGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  pParamFrame->Release( pParamFrame );
};

/* 
Use:	void addProperty(SProperty::PropertyBox ePropertyBox, LPCTSTR sName, void* ptrVar, SProperty::EVariableType eVariableType, UINT uiSize, ...)

Int				
Long			
Double			
Bool			
String

EDITBOX: 
	addProperty(EDITBOX,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType)
SPIN:
	addProperty(SPIN,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax)
SPIN_BOOL:
	addProperty(SPIN_BOOL,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax, bool *ptrBool)
COMBO:
	addProperty(COMBO,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, const char *pList)
*/
void UserExampleGadget::PropertiesRegistration() 
{
	addProperty(SProperty::EDITBOX		,	_T("double1")	,	&dProp1		,	SProperty::Double		);
	addProperty(SProperty::SPIN			,	_T("long2")		,	&lProp3		,	SProperty::Long	,	3		,	9	);
	addProperty(SProperty::SPIN_BOOL	,	_T("bool_int4")	,	&iProp4		,	SProperty::SpinBool	,	-10		,	20, &bProp4	);
	addProperty(SProperty::COMBO		,	_T("combo5")	,	&sProp5		,	SProperty::String	,	pList	);
	addProperty(SProperty::EDITBOX		,	_T("bool6")		,	&bProp6		,	SProperty::Bool		);
};

/*	
Use:	addInputConnector (dataType=transparent, name="")
		addOutputConnector(dataType=transparent, name="")
		addDuplexConnector(outDataType=transparent, inDataType=transparent, name="")
For create multitype datatype, use:
		int createComplexDataType(int numberOfDataTypes, basicdatatype dataType=transparent, ...), as exampled below
*/
void UserExampleGadget::ConnectorsRegistration() 
{
	addOutputConnector( text , "OutputName1");
	addInputConnector( transparent, "InputName1");
	addInputConnector( createComplexDataType(2, rectangle, text), "InputName2");
	addOutputConnector( text , "OutputName2");
	addOutputConnector( createComplexDataType(3, rectangle, text, vframe) , "OutputName3");
	addDuplexConnector( transparent, transparent, "DuplexName1");
	addDuplexConnector( transparent, createComplexDataType(3, rectangle, text, vframe), "DuplexName2");
};




