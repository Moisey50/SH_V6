// phidget_0_16_16Kit.h: interface for the phidget_0_16_16 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_phidget_0_16_16Kit_H__INCLUDED_)
#define AFX_phidget_0_16_16KIT_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "helpers\UserBaseGadget.h"
#include "helpers\UserIsoFilterGadget.h"
#include "phidget21.h"
#include "PhidgetsDriver.h"
#include <assert.h>

#pragma region | Defines |
#define THIS_MODULENAME "phidget_0_16_16Kit"

#pragma region | Logger Defines |
#define PHDrvr_SENDLOG_3(vrbsty,sz,a,b,c)  FxSendLogMsg(vrbsty,THIS_MODULENAME,0,sz,a,b,c)

#define PHDrvr_SENDINFO_3(sz,a,b,c)  PHDrvr_SENDLOG_3(MSG_INFO_LEVEL,sz,a,b,c)
#define PHDrvr_SENDINFO_2(sz,a,b)    PHDrvr_SENDINFO_3(sz,a,b,"")
#define PHDrvr_SENDINFO_1(sz,a)      PHDrvr_SENDINFO_2(sz,a,"")
#define PHDrvr_SENDINFO_0(sz)        PHDrvr_SENDINFO_1(sz,"")

#define PHDrvr_SENDTRACE_3(sz,a,b,c)  PHDrvr_SENDLOG_3(MSG_DEBUG_LEVEL,sz,a,b,c)
#define PHDrvr_SENDTRACE_2(sz,a,b)    PHDrvr_SENDTRACE_3(sz,a,b,"")
#define PHDrvr_SENDTRACE_1(sz,a)      PHDrvr_SENDTRACE_2(sz,a,"")
#define PHDrvr_SENDTRACE_0(sz)        PHDrvr_SENDTRACE_1(sz,"")

#define PHDrvr_SENDWARN_3(sz,a,b,c)  PHDrvr_SENDLOG_3(MSG_WARNING_LEVEL,sz,a,b,c)
#define PHDrvr_SENDWARN_2(sz,a,b)    PHDrvr_SENDWARN_3(sz,a,b,"")
#define PHDrvr_SENDWARN_1(sz,a)      PHDrvr_SENDWARN_2(sz,a,"")
#define PHDrvr_SENDWARN_0(sz)        PHDrvr_SENDWARN_1(sz,"")

#define PHDrvr_SENDERR_3(sz,a,b,c)  PHDrvr_SENDLOG_3(MSG_ERROR_LEVEL,sz,a,b,c)
#define PHDrvr_SENDERR_2(sz,a,b)    PHDrvr_SENDERR_3(sz,a,b,"")
#define PHDrvr_SENDERR_1(sz,a)      PHDrvr_SENDERR_2(sz,a,"")
#define PHDrvr_SENDERR_0(sz)        PHDrvr_SENDERR_1(sz,"")

#define PHDrvr_SENDFAIL_3(sz,a,b,c)  PHDrvr_SENDLOG_3(MSG_CRITICAL_LEVEL,sz,a,b,c)
#define PHDrvr_SENDFAIL_2(sz,a,b)    PHDrvr_SENDFAIL_3(sz,a,b,"")
#define PHDrvr_SENDFAIL_1(sz,a)      PHDrvr_SENDFAIL_2(sz,a,"")
#define PHDrvr_SENDFAIL_0(sz)        PHDrvr_SENDFAIL_1(sz,"")
#pragma endregion | Logger Defines |

#define DIGITAL_TYPE_OUTPUT                 ("OUT")
#define DIGITAL_TYPE_INPUT                  ("IN")

#define DIGITAL_OUTPUT_COMMAND_ON           ("ON")
#define DIGITAL_OUTPUT_COMMAND_OFF          ("OFF")
#define DIGITAL_OUTPUT_COMMAND_TOGGLE       ("TOGGLE")

#define DUPLEX_COMMAND_NAME_GET             ("get")
#define DUPLEX_COMMAND_NAME_GET_OUT         ("getOut")

#define DUPLEX_COMMAND_PARAM_INPUT          ("input")
#define DUPLEX_COMMAND_PARAM_INPUTS         ("inputs")
#define DUPLEX_COMMAND_PARAM_OUTPUT         ("output")
#define DUPLEX_COMMAND_PARAM_OUTPUTS        ("outputs")

#define LIST_DELIMITER                      (";")

#define PROP_NAME_PHDGT_DVC                 ("Phidget_Device")
#define PROP_NAME_PHDGT_INPUT_STATE_AS_MASK ("Input_State_As_Mask")
#define PROP_NAME_PHDGT_OTPUT_STATE_AS_MASK ("Output_State_As_Mask")
#define PROP_NAME_PHDGT_OTPUT_CMMND_AS_MASK ("Output_Cmmnd_As_Mask")

#define PROP_INDEX_NOT_SELECTED             (-1)
#pragma endregion | Defines |


class Phidget_0_16_16 
	: public UserIsoFilterGadget
	, public IPhidgetManagerListener
	, public IPhIFKDriverListener
{
#pragma region | Fields |

protected:
	IFKDriver              *m_pSelectedIFKDrvr;
	bool                    m_bInputStateAsMask;
	bool                    m_bOutputStateAsMask;
	bool                    m_bOutputCmdAsMask;
	int                     m_iStoredSN;
	
#pragma endregion | Fields |

#pragma region | Constructors |
private:
	Phidget_0_16_16(const Phidget_0_16_16&);
	Phidget_0_16_16& operator=(const Phidget_0_16_16&);
protected:
	Phidget_0_16_16();
public:
	DECLARE_RUNTIME_GADGET(Phidget_0_16_16);	

	virtual ~Phidget_0_16_16(void)
	{
		if(m_pSelectedIFKDrvr)
		{
			m_pSelectedIFKDrvr->DetachPhidgetLisener(this);
			//m_pSelectedIFKDrvr->SetHost(NULL);
			m_pSelectedIFKDrvr = NULL;
		}

		if(g_pPhidgetDrvr)
		{
			g_pPhidgetDrvr->DetachPhidgetLisener(this);
			if(!g_pPhidgetDrvr->IsInUse())
			{
				delete g_pPhidgetDrvr;
				g_pPhidgetDrvr = NULL;
			}
		}

	}
#pragma endregion | Constructors |

#pragma region | Methods Private |
private:
	void SendOut(  const FXString& data , const FXString * pLabel = NULL );
	void SendOut_Digital(bool isDigitalInput, int bitIndx, BYTE bitVal);
	void SendOut_Analog(bool isAnalogInput, int analogIndx, double analogVal);
	int GetNewValue(const FXString& command);
	void ChangeDigitalOutput(BYTE bitIndx, const FXString& command);
	void ChangeDigitalOutputs(int bitsIndxMask, const FXString& command);
	void HandleFrame();
	void HandleFrameText(const CTextFrame& data);
	void HandleFrameQuantity( const CQuantityFrame& data );
	FXString OnGetCommand(const FXString &getCmd, int val, FXString *pLabel = NULL );
	int GetCommandVal( FXParser &pk, FXSIZE pos );
	FXString GetCmdParamValDescriptor( const FXString &cmd, const FXString &paramName, const FXString &descr, const FXString &valName = "");
	//void RemoveFromHost(IFKDriver *pDrvr)
	//{
	//	if(pDrvr)
	//	{
	//		Phidget_0_16_16 * gdgtHost = reFXSIZEerpret_cast<Phidget_0_16_16*>(pDrvr->GetHost());
	//		if(gdgtHost)
	//		{
	//			gdgtHost->m_pSelectedIFKDrvr->DetachPhidgetLisener(gdgtHost);
	//			gdgtHost->m_pSelectedIFKDrvr = NULL;
	//			gdgtHost->m_iStoredSN = PROP_INDEX_NOT_SELECTED;
	//		}
	//		pDrvr->SetHost(NULL);
	//	}
	//}
#pragma endregion | Methods Private |

#pragma region | Methods Protected |
protected:
	virtual void /*IPhidgetBaseListener::*/OnPhidgetError( int deviceSN, int errCode, const string& errMsg ){}
	virtual void /*IPhidgetManagerListener::*/OnPhidgetBusChanged( /*const vector<IFKDriver*> &devices*/ )
	{
		SelectDriver();
	}

	virtual void /*IPhIFKDriverListener::*/OnPhIFKAttachmentChanged( int deviceSN, bool isAttached )
	{

	}
	virtual void /*IPhIFKDriverListener::*/OnPhIFKChanged_Digital( int deviceSN, PHIDGET_EVENT eventId, DWORD dwStateMask, DWORD dwFeet )
	{
		switch (eventId)
		{
		case PHIDGET_EVENT_CHANGED_INPUT:
			SendOut_Digital(true, (BYTE)dwFeet, (dwStateMask & (1<<dwFeet))>>dwFeet);
			break;
		case PHIDGET_EVENT_CHANGED_OUTPUT:
			SendOut_Digital(false, (BYTE)dwFeet, (dwStateMask & (1<<dwFeet))>>dwFeet);
			break;
		case PHIDGET_EVENT_REQUESTED_CHANGE_OUTPUT:
			PhidgetKit_OutputChangeRequested(dwStateMask, dwFeet);
			break;
		default:
			assert(eventId);
		}
	}
	virtual void /*IPhIFKDriverListener::*/OnPhIFKChanged_Analog( int deviceSN, PHIDGET_EVENT eventId, double dValue, DWORD dwFeet )
	{
		switch (eventId)
		{
		case PHIDGET_EVENT_CHANGED_INPUT:
			SendOut_Analog(true, (BYTE)dwFeet, dValue);
			break;
		case PHIDGET_EVENT_CHANGED_OUTPUT:
			//SendOut(false, (BYTE)dwFeet, (dwStateMask & (1<<dwFeet))>>dwFeet);
			break;
		case PHIDGET_EVENT_REQUESTED_CHANGE_OUTPUT:
			//PhidgetKit_OutputChangeRequested(dwStateMask, dwFeet);
			break;
		default:
			assert(eventId);
		}
	}

	virtual void PhidgetKit_OutputChangeRequested( DWORD dwStateMask, DWORD dwBitId );

	bool ScanProperties(LPCTSTR text, bool& Invalidate)
	{
		UserGadgetBase::ScanProperties(text, Invalidate);
		
		FXPropertyKit pc(text);
		
		if(pc.GetInt(PROP_NAME_PHDGT_DVC, m_iStoredSN))
			SelectDriver();

		bool boolValue = false;

		if(pc.GetBool(PROP_NAME_PHDGT_INPUT_STATE_AS_MASK, boolValue) && boolValue != m_bInputStateAsMask)
			m_bInputStateAsMask = boolValue;
		boolValue = false;

		if(pc.GetBool(PROP_NAME_PHDGT_OTPUT_STATE_AS_MASK, boolValue) && boolValue != m_bOutputStateAsMask)
			m_bOutputStateAsMask = boolValue;
		boolValue = false;

		if(pc.GetBool(PROP_NAME_PHDGT_OTPUT_CMMND_AS_MASK, boolValue) && boolValue != m_bOutputCmdAsMask)
			m_bOutputCmdAsMask = boolValue;

		return true;
	}

	void SelectDriver()
	{
		if(m_iStoredSN <= 0)
		{
			if(m_pSelectedIFKDrvr)
			{
				m_pSelectedIFKDrvr->DetachPhidgetLisener(this);
				//m_pSelectedIFKDrvr->SetHost(NULL);
				m_pSelectedIFKDrvr = NULL;
			}
		}
		else if (g_pPhidgetDrvr)
		{
			IFKDriver *pCurrentDrvr = m_pSelectedIFKDrvr;
			int currentSN = m_pSelectedIFKDrvr ? m_pSelectedIFKDrvr->GetInfo().GetSerialNum() : m_iStoredSN;
			IFKDriver *pRequestedDrvr = g_pPhidgetDrvr->GetDeviceBySerialNum(m_iStoredSN);
			if(!pRequestedDrvr)
				m_pSelectedIFKDrvr = NULL;

			if(m_pSelectedIFKDrvr && m_pSelectedIFKDrvr->GetInfo().GetSerialNum()!=m_iStoredSN)
			{
				m_pSelectedIFKDrvr->DetachPhidgetLisener(this);
				//m_pSelectedIFKDrvr->SetHost(NULL);
				m_pSelectedIFKDrvr = NULL;
			}

			if(!m_pSelectedIFKDrvr && pRequestedDrvr)
			{
				m_pSelectedIFKDrvr = pRequestedDrvr;
				if(m_pSelectedIFKDrvr->IsInUse())
				{
					PHDrvr_SENDWARN_1
						( "The '%s' device is already in use by other gadget."
						, m_pSelectedIFKDrvr->ToString().c_str());

					//string newGdgtName("other");
					//Phidget_0_16_16 * gdgtHost = reinterpret_cast<Phidget_0_16_16*>(m_pSelectedIFKDrvr->GetHost());
					//FXString name;
					//if(gdgtHost && gdgtHost->GetGadgetName(name))
					//	newGdgtName = name;
					
					//FXString msg;
					//msg.Format("The '%s' device is already in use by %s gadget.\nClick 'OK' to re-attach it to this gadget, otherwise - 'Cancel'.
					//	, m_pSelectedIFKDrvr->ToString().c_str()
					//	, newGdgtName);

					//if(IDOK==MessageBox(NULL, msg, "Phidget Device Select", MB_APPLMODAL | MB_ICONWARNING | MB_OKCANCEL ))
					//{
					//	//RemoveFromHost(m_pSelectedIFKDrvr);
					//}
					//else
					//{
					//	m_iStoredSN = currentSN;
					//	m_pSelectedIFKDrvr = pCurrentDrvr;							
					//}
				}
				
				if(m_pSelectedIFKDrvr)
				{
					//m_pSelectedIFKDrvr->SetHost(this);
					m_pSelectedIFKDrvr->Init((LPIPhIFKListener)this);
					m_pSelectedIFKDrvr->Start();
				}
			}
		}
	}

	bool PrintProperties(FXString& text)
	{
		UserGadgetBase::PrintProperties(text);

		FXPropertyKit pc;
		
		pc.WriteInt (PROP_NAME_PHDGT_DVC,                 m_iStoredSN);
		pc.WriteBool(PROP_NAME_PHDGT_INPUT_STATE_AS_MASK, m_bInputStateAsMask);
		pc.WriteBool(PROP_NAME_PHDGT_OTPUT_STATE_AS_MASK, m_bOutputStateAsMask);
		pc.WriteBool(PROP_NAME_PHDGT_OTPUT_CMMND_AS_MASK, m_bOutputCmdAsMask);

		text+=pc;

		return true;
	}

	bool ScanSettings(FXString& text)
	{
		FXString devicesInCombo;
		devicesInCombo.Format("%s(%d)", "NOT_SELECTED", PROP_INDEX_NOT_SELECTED);
		FXString devices;
		
		if(g_pPhidgetDrvr)
		{
			devices = g_pPhidgetDrvr->ToString().c_str();
			if(m_iStoredSN > 0 && !g_pPhidgetDrvr->GetDeviceBySerialNum(m_iStoredSN))
			{
				devicesInCombo+=LIST_DELIMITER;
				devicesInCombo.Format("%sMISSING_DEVICE_SN%d(%d)", devicesInCombo, m_iStoredSN, m_iStoredSN);
			}
		}

		if(!devices.IsEmpty())
			devicesInCombo+=LIST_DELIMITER+devices;

		text.Format("template(%s(%s(%s)),"
			"%s(%s(False(false),True(true))),"
			"%s(%s(False(false),True(true))),"
			"%s(%s(False(false),True(true)))"
			")"
			, SETUP_COMBOBOX, PROP_NAME_PHDGT_DVC, devicesInCombo
			, SETUP_COMBOBOX, PROP_NAME_PHDGT_INPUT_STATE_AS_MASK
			, SETUP_COMBOBOX, PROP_NAME_PHDGT_OTPUT_STATE_AS_MASK
			, SETUP_COMBOBOX, PROP_NAME_PHDGT_OTPUT_CMMND_AS_MASK);
		return true;
	}
  virtual int ProcessData( CDataFrame * pDataFrame )
  {
    if ( pDataFrame && m_pOutput && m_pOutput->Put(pDataFrame) )
      return TRUE ;
    pDataFrame->Release( pDataFrame ) ;
    return FALSE ;
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
#endif // !defined(AFX_phidget_0_16_16KIT_H__INCLUDED_)