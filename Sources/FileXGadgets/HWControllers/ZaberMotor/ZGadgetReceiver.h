#if !defined(AFX_zaber_commutator_H__INCLUDED_)
#define AFX_zaber_commutator_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZGadgetBase.h"
#include <string>
#include <queue>
#include "ZPacket.h"

using namespace std;

#pragma region | Defines |
#define PROP_NAME_ZBR_DVC_ID					             ("Device_ID")
#define PROP_NAME_ZBR_CMD_TO_FILTER  		       ("Cmd_To_Filter")

#define NO_COMMAND_TO_FILTER            ("NO_COMMAND_TO_FILTER")
#pragma endregion | Defines |

class ZGadgetReceiver
	: public ZGadgetBase
{
#pragma region | Fields |
  static string COMMANDS_TEXT_COLLECTION;
	

protected:
	string  m_sPacketRecieved;
  int			m_iStoredDvcID;
  string  m_sCmdToFilter;
	//double                  m_dLastTime;
	//double                  m_dDeltaTime;
	//queue<string>           m_packetsQueue;
	//queue<const ZPacket*>   m_zPacketsQueue;


#pragma endregion | Fields |

#pragma region | Constructors |
private:
	ZGadgetReceiver(const ZGadgetReceiver&);
	ZGadgetReceiver& operator=(const ZGadgetReceiver&);
protected:
	ZGadgetReceiver();
public:
	DECLARE_RUNTIME_GADGET(ZGadgetReceiver);	

	virtual ~ZGadgetReceiver(void)
	{

	}
#pragma endregion | Constructors |

#pragma region | Methods Private |
private:


	//void SendOut_Digital(bool isDigitalInput, int bitIndx, BYTE bitVal);
	//void SendOut_Analog(bool isAnalogInput, int analogIndx, double analogVal);
	//int GetNewValue(const FXString& command);
	//void ChangeDigitalOutput(BYTE bitIndx, const FXString& command);
	//void ChangeDigitalOutputs(int bitsIndxMask, const FXString& command);
	//void HandleFrame();
	//void HandleFrameText(const CTextFrame& data);
	//void HandleFrameQuantity( const CQuantityFrame& data );
	//FXString OnGetCommand(const FXString &getCmd, int val, FXString *pLabel = NULL );
	//int GetCommandVal( FXParser &pk, int pos );
	//FXString GetCmdParamValDescriptor( const FXString &cmd, const FXString &paramName, const FXString &descr, const FXString &valName = "");

  static const string& GetCommandsCollectionAsText()
  {

    if ( COMMANDS_TEXT_COLLECTION.empty() )
    {
      ostringstream oss;

      ZCommandsHeadersRepository cmds;
      ZCommandsHeadersRepository::RepositorySet::const_iterator ci = cmds.GetRepository().begin();
      ZCommandsHeadersRepository::RepositorySet::const_iterator ce = cmds.GetRepository().end();

      for ( ; ci != ce; ci++ )
      {
        string cmdName = NO_COMMAND_TO_FILTER;

        if ( oss.str().empty() )
          oss << cmdName << "(" << cmdName << ")";

        if ( !oss.str().empty() )
          oss << ",";

        cmdName = (ci)->GetInstructionName();

        oss << cmdName
          << "("
          << cmdName
          << ")";
      }

      if ( !oss.str().empty() )
        COMMANDS_TEXT_COLLECTION = oss.str();
    }
    return COMMANDS_TEXT_COLLECTION;
  }
#pragma endregion | Methods Private |

#pragma region | Methods Protected |
protected:
	//virtual void /*IPhidgetBaseListener::*/OnPhidgetError( int deviceSN, int errCode, const string& errMsg ){}
	//virtual void /*IPhidgetManagerListener::*/OnPhidgetBusChanged( /*const vector<IFKDriver*> &devices*/ )
	//{
	//	SelectDriver();
	//}

	//virtual void /*IPhIFKDriverListener::*/OnPhIFKAttachmentChanged( int deviceSN, bool isAttached )
	//{

	//}
	//virtual void /*IPhIFKDriverListener::*/OnPhIFKChanged_Digital( int deviceSN, PHIDGET_EVENT eventId, DWORD dwStateMask, DWORD dwFeet )
	//{
	//	switch (eventId)
	//	{
	//	case PHIDGET_EVENT_CHANGED_INPUT:
	//		SendOut_Digital(true, (BYTE)dwFeet, (dwStateMask & (1<<dwFeet))>>dwFeet);
	//		break;
	//	case PHIDGET_EVENT_CHANGED_OUTPUT:
	//		SendOut_Digital(false, (BYTE)dwFeet, (dwStateMask & (1<<dwFeet))>>dwFeet);
	//		break;
	//	case PHIDGET_EVENT_REQUESTED_CHANGE_OUTPUT:
	//		PhidgetKit_OutputChangeRequested(dwStateMask, dwFeet);
	//		break;
	//	default:
	//		assert(eventId);
	//	}
	//}
	//virtual void /*IPhIFKDriverListener::*/OnPhIFKChanged_Analog( int deviceSN, PHIDGET_EVENT eventId, double dValue, DWORD dwFeet )
	//{
	//	switch (eventId)
	//	{
	//	case PHIDGET_EVENT_CHANGED_INPUT:
	//		SendOut_Analog(true, (BYTE)dwFeet, dValue);
	//		break;
	//	case PHIDGET_EVENT_CHANGED_OUTPUT:
	//		//SendOut(false, (BYTE)dwFeet, (dwStateMask & (1<<dwFeet))>>dwFeet);
	//		break;
	//	case PHIDGET_EVENT_REQUESTED_CHANGE_OUTPUT:
	//		//PhidgetKit_OutputChangeRequested(dwStateMask, dwFeet);
	//		break;
	//	default:
	//		assert(eventId);
	//	}
	//}

	//virtual void PhidgetKit_OutputChangeRequested( DWORD dwStateMask, DWORD dwBitId );

	bool ScanProperties(LPCTSTR text, bool& Invalidate)
	{
		UserGadgetBase::ScanProperties(text, Invalidate);

		FXPropertyKit pc(text);
    FXString cmdToFilter = "";
    pc.GetInt( PROP_NAME_ZBR_DVC_ID , m_iStoredDvcID );
    if ( pc.GetString( PROP_NAME_ZBR_CMD_TO_FILTER , cmdToFilter ) )
      m_sCmdToFilter = cmdToFilter;

		//if(pc.GetInt(PROP_NAME_PHDGT_DVC, m_iStoredSN))
		//	SelectDriver();

		//bool boolValue = false;

		//if(pc.GetBool(PROP_NAME_PHDGT_INPUT_STATE_AS_MASK, boolValue) && boolValue != m_bInputStateAsMask)
		//	m_bInputStateAsMask = boolValue;
		//boolValue = false;

		//if(pc.GetBool(PROP_NAME_PHDGT_OTPUT_STATE_AS_MASK, boolValue) && boolValue != m_bOutputStateAsMask)
		//	m_bOutputStateAsMask = boolValue;
		//boolValue = false;

		//if(pc.GetBool(PROP_NAME_PHDGT_OTPUT_CMMND_AS_MASK, boolValue) && boolValue != m_bOutputCmdAsMask)
		//	m_bOutputCmdAsMask = boolValue;

		return true;
	}

	//void SelectDriver()
	//{
	//	if(m_iStoredSN <= 0)
	//	{
	//		if(m_pSelectedIFKDrvr)
	//		{
	//			m_pSelectedIFKDrvr->DetachPhidgetLisener(this);
	//			//m_pSelectedIFKDrvr->SetHost(NULL);
	//			m_pSelectedIFKDrvr = NULL;
	//		}
	//	}
	//	else if (g_pPhidgetDrvr)
	//	{
	//		IFKDriver *pCurrentDrvr = m_pSelectedIFKDrvr;
	//		int currentSN = m_pSelectedIFKDrvr ? m_pSelectedIFKDrvr->GetInfo().GetSerialNum() : m_iStoredSN;
	//		IFKDriver *pRequestedDrvr = g_pPhidgetDrvr->GetDeviceBySerialNum(m_iStoredSN);
	//		if(!pRequestedDrvr)
	//			m_pSelectedIFKDrvr = NULL;
  //
	//		if(m_pSelectedIFKDrvr && m_pSelectedIFKDrvr->GetInfo().GetSerialNum()!=m_iStoredSN)
	//		{
	//			m_pSelectedIFKDrvr->DetachPhidgetLisener(this);
	//			//m_pSelectedIFKDrvr->SetHost(NULL);
	//			m_pSelectedIFKDrvr = NULL;
	//		}
  //
	//		if(!m_pSelectedIFKDrvr && pRequestedDrvr)
	//		{
	//			m_pSelectedIFKDrvr = pRequestedDrvr;
	//			if(m_pSelectedIFKDrvr->IsInUse())
	//			{
	//				ZBR_SENDWARN_1
	//					( "The '%s' device is already in use by other gadget."
	//					, m_pSelectedIFKDrvr->ToString().c_str());
  //
	//				//string newGdgtName("other");
	//				//Zaber_Motor * gdgtHost = reinterpret_cast<Za*>(m_pSelectedIFKDrvr->GetHost());
	//				//FXString name;
	//				//if(gdgtHost && gdgtHost->GetGadgetName(name))
	//				//	newGdgtName = name;
  //
	//				//FXString msg;
	//				//msg.Format("The '%s' device is already in use by %s gadget.\nClick 'OK' to re-attach it to this gadget, otherwise - 'Cancel'.
	//				//	, m_pSelectedIFKDrvr->ToString().c_str()
	//				//	, newGdgtName);
  //
	//				//if(IDOK==MessageBox(NULL, msg, "Phidget Device Select", MB_APPLMODAL | MB_ICONWARNING | MB_OKCANCEL ))
	//				//{
	//				//	//RemoveFromHost(m_pSelectedIFKDrvr);
	//				//}
	//				//else
	//				//{
	//				//	m_iStoredSN = currentSN;
	//				//	m_pSelectedIFKDrvr = pCurrentDrvr;							
	//				//}
	//			}
  //
	//			if(m_pSelectedIFKDrvr)
	//			{
	//				//m_pSelectedIFKDrvr->SetHost(this);
	//				m_pSelectedIFKDrvr->Init((LPIPhIFKListener)this);
	//				m_pSelectedIFKDrvr->Start();
	//			}
	//		}
	//	}
	//}

	bool PrintProperties(FXString& text)
	{
		UserGadgetBase::PrintProperties(text);

		FXPropertyKit pc;

		pc.WriteInt (PROP_NAME_ZBR_DVC_ID           , m_iStoredDvcID);
    pc.WriteString( PROP_NAME_ZBR_CMD_TO_FILTER , m_sCmdToFilter.c_str() );
		//pc.WriteBool(PROP_NAME_PHDGT_INPUT_STATE_AS_MASK, m_bInputStateAsMask);
		//pc.WriteBool(PROP_NAME_PHDGT_OTPUT_STATE_AS_MASK, m_bOutputStateAsMask);
		//pc.WriteBool(PROP_NAME_PHDGT_OTPUT_CMMND_AS_MASK, m_bOutputCmdAsMask);

		text+=pc;

		return true;
	}

	bool ScanSettings(FXString& text)
	{ 
    FXString resolutions = GetCommandsCollectionAsText().c_str();
    text.Format( "template"
      "(%s(%s, %d, %d)"
      ",%s(%s(%s))"
      ")"
      , SETUP_SPIN , PROP_NAME_ZBR_DVC_ID , -1 , 10
      , SETUP_COMBOBOX , PROP_NAME_ZBR_CMD_TO_FILTER , resolutions
    );
    return true ;
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

#endif // !defined(AFX_zaber_commutator_H__INCLUDED_)