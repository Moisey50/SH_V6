// zaber_deviceKit.h: interface for the zaber_device class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_zaber_deviceKit_H__INCLUDED_)
#define AFX_zaber_deviceKIT_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZGadgetBase.h"
#include "ZPacket.h"
#include "ZaberDriver.h"
#include "ZCommandsHeadersRepository.h"

#include <string>
#include <sstream>
#include <assert.h>

using namespace std;


#pragma region | Defines |


//#define DIGITAL_TYPE_OUTPUT                 ("OUT")
//#define DIGITAL_TYPE_INPUT                  ("IN")
//
//#define DIGITAL_OUTPUT_COMMAND_ON           ("ON")
//#define DIGITAL_OUTPUT_COMMAND_OFF          ("OFF")
//#define DIGITAL_OUTPUT_COMMAND_TOGGLE       ("TOGGLE")
//
//#define DUPLEX_COMMAND_NAME_GET             ("get")
//#define DUPLEX_COMMAND_NAME_GET_OUT         ("getOut")
//
//#define DUPLEX_COMMAND_PARAM_INPUT          ("input")
//#define DUPLEX_COMMAND_PARAM_INPUTS         ("inputs")
//#define DUPLEX_COMMAND_PARAM_OUTPUT         ("output")
//#define DUPLEX_COMMAND_PARAM_OUTPUTS        ("outputs")
//
//
//

#define ZBR_DEFAULT_MICROSTEPS_IN_STEP			         (64)

#define PROP_NAME_ZBR_DVC_ID					               ("Device_ID")
#define PROP_NAME_ZBR_STEPS_PER_REVOLUTION		       ("Steps_Per_Revol")
#define PROP_NAME_ZBR_MOTION_PER_REVOLUTION_UM       ("Motion_Per_Revol_um")
#define PROP_NAME_ZBR_MICROSTEPS_IN_STEP		         ("Microsteps_In_Step")
#define PROP_NAME_ZBR_TO_AUTOSEND_MICROSTEPS_IN_STEP ("Autosend_Microsteps_In_Step")

#define PROP_INDEX_NOT_SELECTED					             (-1)

#define PACKET_LABEL_BYNARY                          ("PacketBinary")
#define PACKET_LABEL_ASCII                           ("PacketASCII")
#pragma endregion | Defines |


class ZGadgetSender 
	: public ZGadgetBase
{
#pragma region | Fields |



protected:
//	IFKDriver	*m_pSelectedIFKDrvr;
	int			m_iStepsPerRevolution;
	double	m_dMotionPerRevolution_um;
	int			m_iMicrostepsInStep;
	int			m_iStoredDvcID;
  bool    m_bToAutosendMicrostepsInStep;
	bool		m_bAreSettingsUploaded;

	static string RESOLUTIONS_TEXT_COLLECTION;
	
#pragma endregion | Fields |

#pragma region | Constructors |
private:
	ZGadgetSender(const ZGadgetSender&);
	ZGadgetSender& operator=(const ZGadgetSender&);
protected:
	ZGadgetSender();
public:
	DECLARE_RUNTIME_GADGET(ZGadgetSender);	

	virtual ~ZGadgetSender(void)
	{

	}
#pragma endregion | Constructors |

#pragma region | Methods Private |
private:
	//void SendOut_Digital(bool isDigitalInput, int bitIndx, BYTE bitVal);
	//void SendOut_Analog(bool isAnalogInput, int analogIndx, double analogVal);
	//int GetNewValue(const FXString& command);

	void HandleFrame();
	void HandleFrameText(const CTextFrame& data);
	void HandleFrameQuantity( const CQuantityFrame& data );
	//FXString OnGetCommand(const FXString &getCmd, int val, FXString *pLabel = NULL );
	//int GetCommandVal( FXParser &pk, int pos );
	//FXString GetCmdParamValDescriptor( const FXString &cmd, const FXString &paramName, const FXString &descr, const FXString &valName = "");

	static const string& GetResolutionsCollectionAsText()
	{
		int resolutionMin = 1;
		int resolutionMax = 128;
		int index = 0;
		if (RESOLUTIONS_TEXT_COLLECTION.empty())
		{
			ostringstream oss;

			for (int i = resolutionMin; i <= resolutionMax; i *= 2)
			{
				if (!oss.str().empty())
					oss << ",";

				oss << i
					<< "("
					<< i
					<< ")";
			}

			if (!oss.str().empty())
				RESOLUTIONS_TEXT_COLLECTION = oss.str();
		}
		return RESOLUTIONS_TEXT_COLLECTION;
	}

#pragma endregion | Methods Private |

#pragma region | Methods Protected |
protected:

	bool ScanProperties(LPCTSTR text, bool& Invalidate)
	{
    UserGadgetBase::ScanProperties(text, Invalidate);

		FXPropertyKit pc(text);

		int cacheDvcId = m_iStoredDvcID;
		int cacheStepsPerRevolution = m_iStepsPerRevolution;
		double cacheMotionPerRevolution_um = m_dMotionPerRevolution_um;
		int cacheMicrostepsInStep = m_iMicrostepsInStep;
    bool bToAutosendMicrostepsInStep = m_bToAutosendMicrostepsInStep;

		pc.GetInt(PROP_NAME_ZBR_DVC_ID, m_iStoredDvcID);
		pc.GetInt(PROP_NAME_ZBR_STEPS_PER_REVOLUTION, m_iStepsPerRevolution);
		pc.GetDouble(PROP_NAME_ZBR_MOTION_PER_REVOLUTION_UM, m_dMotionPerRevolution_um);
		pc.GetInt(PROP_NAME_ZBR_MICROSTEPS_IN_STEP, m_iMicrostepsInStep);
    pc.GetBool(PROP_NAME_ZBR_TO_AUTOSEND_MICROSTEPS_IN_STEP, m_bToAutosendMicrostepsInStep);

		if (m_bAreSettingsUploaded && (cacheDvcId != m_iStoredDvcID ||
			cacheStepsPerRevolution != m_iStepsPerRevolution ||
			abs(cacheMotionPerRevolution_um - m_dMotionPerRevolution_um) > 0.0001 ||
			cacheMicrostepsInStep != m_iMicrostepsInStep))
			m_bAreSettingsUploaded = false;
		return true;
	}

	bool PrintProperties(FXString& text)
	{
		UserGadgetBase::PrintProperties(text);

		FXPropertyKit pc;
		
		pc.WriteInt(PROP_NAME_ZBR_DVC_ID,     m_iStoredDvcID);
		pc.WriteInt(PROP_NAME_ZBR_STEPS_PER_REVOLUTION, m_iStepsPerRevolution);
		pc.WriteDouble(PROP_NAME_ZBR_MOTION_PER_REVOLUTION_UM,   m_dMotionPerRevolution_um);
		pc.WriteInt(PROP_NAME_ZBR_MICROSTEPS_IN_STEP, m_iMicrostepsInStep);
    pc.WriteBool(PROP_NAME_ZBR_TO_AUTOSEND_MICROSTEPS_IN_STEP, m_bToAutosendMicrostepsInStep);

		text+=pc;

		return true;
	}

	bool ScanSettings(FXString& text)
	{
		FXString resolutions = GetResolutionsCollectionAsText().c_str();
		text.Format("template"
			"(%s(%s, %d, %d)"
			",%s(%s, %d, %d)"
			",%s(%s, %f)"
			",%s(%s(%s))"
      ",%s(%s(YES(true),NO(false)))"
			")"
			, SETUP_SPIN, PROP_NAME_ZBR_DVC_ID, -1, 10
			, SETUP_SPIN, PROP_NAME_ZBR_STEPS_PER_REVOLUTION, -1, 1000
			, SETUP_EDITBOX, PROP_NAME_ZBR_MOTION_PER_REVOLUTION_UM, m_dMotionPerRevolution_um
			, SETUP_COMBOBOX, PROP_NAME_ZBR_MICROSTEPS_IN_STEP, resolutions
      , SETUP_COMBOBOX, PROP_NAME_ZBR_TO_AUTOSEND_MICROSTEPS_IN_STEP
			);
		return true;
	}
#pragma endregion | Methods Protected |

#pragma region | Methods Public |
public:
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);	
#pragma endregion | Methods Public |
};

#endif // !defined(AFX_zaber_deviceKIT_H__INCLUDED_)