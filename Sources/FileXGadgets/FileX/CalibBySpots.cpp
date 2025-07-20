// CalibBySpots.h.h : Implementation of the CalibBySpots class


#include "StdAfx.h"
#include "CalibBySpots.h"

#define THIS_MODULENAME "VideoCalib"

USER_FILTER_RUNTIME_GADGET(CalibBySpots,"VideoCalib");


CalibBySpots::CalibBySpots(void)
{
    init();	// Mandatory in constructor
}


/*
	DoProcessing function processing data received/send by Input/Output connectors (do not relating to Duplex connector)
*/
CDataFrame* CalibBySpots::DoProcessing(const CDataFrame* pDataFrame)
{
  switch ( m_iWorkingMode )
  {
  case MODE_CONVERT:
    break ;
  case MODE_CALIBRATE:
    break ;
  default:
    SENDERR_1( _T("Unknown working mode %d") , m_iWorkingMode );
    return NULL ;

  }
  
  return NULL;
}


/*
	AsyncTransaction function associated with Duplex connector data processing
*/
void CalibBySpots::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  pParamFrame->Release( pParamFrame );
}


/*
	PropertiesRegistration() function define shown and editable gadget properties
*/
void CalibBySpots::PropertiesRegistration() 
{
/* 
	To fill PropertiesRegistration() Use: addProperty(SProperty::PropertyBox ePropertyBox, LPCTSTR sName, void* ptrVar, SProperty::EVariableType eVariableType, ...) function as described below

		EDITBOX property type: 
			addProperty(EDITBOX,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType)
		SPIN property type: 
			addProperty(SPIN,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax)
		SPIN_BOOL property type: 
			addProperty(SPIN_BOOL,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax, bool *ptrBool)
		COMBO property type: 
			addProperty(COMBO,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, const char *pList)

		Avaliable variable types: 

			SProperty::Int				
			SProperty::Long			
			SProperty::Double			
			SProperty::Bool			
			SProperty::String

		Important: variable type should be matched to property type
	
	Example:
	
		static const char* pList = "AA1; AA2; AA3; BB1; BB2; BB3; CC3";
		double		dProp1;
		long		lProp3;
		int			iProp4;
		bool		bProp4;
		FXString	sProp5;
		bool		bProp6;
	
		void UserExampleGadget::PropertiesRegistration() 
		{
			addProperty(SProperty::EDITBOX		,	"double1"	,	&dProp1		,	SProperty::Double		);
			addProperty(SProperty::SPIN			,	"long2"		,	&lProp3		,	SProperty::Long	,	3		,	9	);
			addProperty(SProperty::SPIN_BOOL	,	"bool_int4"	,	&iProp4		,	SProperty::SpinBool	,	-10		,	20, &bProp4	);
			addProperty(SProperty::COMBO		,	"combo5"	,	&sProp5		,	SProperty::String	,	pList	);
			addProperty(SProperty::EDITBOX		,	"bool6"		,	&bProp6		,	SProperty::Bool		);
		};	
*/
  addProperty(SProperty::COMBO		,	_T("WorkingMode")	,	&m_iWorkingMode		,	SProperty::Int	,	_T("Convert; Calibrate;")	);
  m_iWorkingMode = MODE_CONVERT ;

}


/*
	ConnectorsRegistration() function define Input/Output/Duplex connectors 
  (Duplex connectors will be processed by AsyncTransaction function)
*/
void CalibBySpots::ConnectorsRegistration() 
{
/* 
		To fill ConnectorsRegistration() use next functions:

			addInputConnector (int dataType, LPCTSTR name)
			addOutputConnector(int dataType, LPCTSTR name)
			addDuplexConnector(int outDataType, int inDataType, LPCTSTR name)

		Use valiable datatypes:

			transparent // unspecified format, needs run-time compatibility checking
			nulltype    // bare time-stamp (synchronization data)
			vframe      // video frame
			text        // text
			wave        // sound
			quantity    // quantity (integer, double etc)
			logical     // logical value (boolean true/false)
			rectangle   // rectangle
			figure      // figure - set of points connected with segments
			metafile    // windows metafile drawing
			userdata	// specific user-defined data
			arraytype	// array

		for define connector to send/receive different datatypes, please use the function create and register datatype called "complex"

			int createComplexDataType(int numberOfDataTypes, basicdatatype dataType, ...) that return complex datatype ID

	Example:
	
		addOutputConnector( text , "OutputName1");
		addInputConnector( transparent, "InputName1");
		addInputConnector( createComplexDataType(2, rectangle, text), "InputName2");
		addOutputConnector( text , "OutputName2");
		addOutputConnector( createComplexDataType(3, rectangle, text, vframe) , "OutputName3");
		addDuplexConnector( transparent, transparent, "DuplexName1");
		addDuplexConnector( transparent, createComplexDataType(3, rectangle, text, vframe), "DuplexName2");
*/
  addOutputConnector( transparent , "Converted");
  addInputConnector( transparent, "PointsData");

}




CDataFrame * CalibBySpots::ConvertToWorld(const CDataFrame * pFOVData)
{
  return NULL;
}


LPCTSTR CalibBySpots::Calibrate(const CDataFrame * pData)
{
  return LPCTSTR();
}
