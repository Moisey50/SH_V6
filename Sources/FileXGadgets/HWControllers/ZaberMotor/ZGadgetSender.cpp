// zaber_device.cpp: implementation of the zaber_device class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZGadgetSender.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define THIS_MODULENAME "ZGadgetSender"
USER_FILTER_RUNTIME_GADGET(ZGadgetSender, "HWControllers");

string ZGadgetSender::RESOLUTIONS_TEXT_COLLECTION;


ZGadgetSender::ZGadgetSender()
	: /*m_pSelectedIFKDrvr(NULL)
	,*/ m_dMotionPerRevolution_um(UNDEFINED_VALUE)
	, m_iStepsPerRevolution(UNDEFINED_VALUE)
	, m_iStoredDvcID(UNDEFINED_VALUE)
	, m_iMicrostepsInStep(ZBR_DEFAULT_MICROSTEPS_IN_STEP)
  , m_bToAutosendMicrostepsInStep(true)
	, m_bAreSettingsUploaded(false)
{
    init();
}

#pragma region | Methods Private |


void ZGadgetSender::HandleFrame( )
{
	//if(m_pSelectedIFKDrvr && m_pSelectedIFKDrvr->GetInfo().GetNumAnalogs())
	//{
	//	int analogIn = m_pSelectedIFKDrvr->GetInfo().GetNumAnalogs();
	//	for(int i = 0; i < analogIn; i++)
	//	{
	//		int analogVal = m_pSelectedIFKDrvr->GetAnalogInputValue(i);
	//		//if(analogVal)
	//		//	SendOut_Analog(true, i, analogVal*0.2222-61.111);
	//	}
	//}
}
void ZGadgetSender::HandleFrameText( const CTextFrame& data )
{
	if(m_iStoredDvcID >= 0)
	{
    m_Locker.Lock();

		if(!m_Devices[0])
			m_Devices.Add(ZDeviceBase(0));
		if(!m_Devices[m_iStoredDvcID])
			m_Devices.Add(ZDeviceBase(m_iStoredDvcID, m_iStepsPerRevolution, m_dMotionPerRevolution_um));
		else
		{
			ZDeviceSettings& settings = ((ZDeviceSettings&)m_Devices[m_iStoredDvcID]->GetSettings());

			settings
				.SetProductLinearMotionPerRevolution_um(m_dMotionPerRevolution_um)
				.SetProductStepsPerRevolution(m_iStepsPerRevolution);
		}
    bool bPrevUploadedStatus = m_bAreSettingsUploaded ;
		if (!m_bAreSettingsUploaded)
		{
      Sleep( m_iStoredDvcID * 50 ); // timing distortion
      ZCommandHeader setResolution = m_CommandsHeaders[37];

			if (!ZCommandHeader::IsEmptyCommand(setResolution))
			{
				ZCommandBase cmdSetResolution;
				cmdSetResolution
					.SetHeader(&setResolution)
					.SetData(m_iMicrostepsInStep);

        if (m_bToAutosendMicrostepsInStep)
        {
          const ZPacket* pPacketSetResolution = ZPacket::Deserialize(
            cmdSetResolution.ToString(),
            m_CommandsFactory,
            m_CommandsHeaders,
            m_Devices,
            *m_Devices[m_iStoredDvcID]);

          if (pPacketSetResolution)
          {
            SendOut(m_pOutput, PACKET_LABEL_ASCII, pPacketSetResolution->ToString().c_str(), data.GetId());
            SendOut(m_pOutput, PACKET_LABEL_BYNARY, pPacketSetResolution->Serialize().c_str(), data.GetId());
          }
        }

				m_bAreSettingsUploaded = true;
				ZDeviceSettings& settings = ((ZDeviceSettings&)m_Devices[m_iStoredDvcID]->GetSettings());
				settings.SetResolution(m_iMicrostepsInStep);

				Sleep( m_iStoredDvcID * 50 ); //additional timing distortion
			}
		}
    m_Locker.Unlock();

		const ZPacket* pPacket = ZPacket::Deserialize((string)data.GetString(), m_CommandsFactory, m_CommandsHeaders, m_Devices, *m_Devices[m_iStoredDvcID]);

		if(pPacket)
		{
			SendOut(m_pOutput, PACKET_LABEL_ASCII, pPacket->ToString().c_str(), data.GetId());
			SendOut(m_pOutput, PACKET_LABEL_BYNARY, pPacket->Serialize().c_str(), data.GetId());
		}
    if ( !bPrevUploadedStatus )
    {
      Sleep( m_iStoredDvcID * 20 ) ;
    }
		delete pPacket;
	}
}
void ZGadgetSender::HandleFrameQuantity( const CQuantityFrame& data )
{
	//if(m_pSelectedIFKDrvr)
	//{
	//	FXString command = ((FXString)data.GetLabel()).Trim();
	//	long bitsIndexes = data.operator long();
	//	
	//	if(bitsIndexes < m_pSelectedIFKDrvr->GetInfo().GetNumOutputs() && FALSE == m_bOutputCmdAsMask)
	//		ChangeDigitalOutput((BYTE)bitsIndexes, command);
	//	else 
	//		ChangeDigitalOutputs(bitsIndexes, command);
	//}
}
//FXString ZGadgetSender::OnGetCommand( const FXString &getCmd, int val, FXString *pLabel /*= NULL */ )
//{
//	FXString bitsStatesTxt;
//	FXString digitalType;
//	unsigned bitsStates = 0;
//	bool isSingle = false;
//
//	//if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_INPUTS)==0)
//	//{
//	//	bitsStates = m_pSelectedIFKDrvr->GetAllStatesInputs();
//	//	digitalType = DIGITAL_TYPE_INPUT;
//	//}
//	//else if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_OUTPUTS)==0)
//	//{
//	//	bitsStates = m_pSelectedIFKDrvr->GetAllStatesOutputs();
//	//	digitalType = DIGITAL_TYPE_OUTPUT;
//	//}
//	//else if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_INPUT)==0 && val >= 0)
//	//{
//	//	bitsStates = m_pSelectedIFKDrvr->GetDigitalStateInput(val);
//	//	digitalType = DIGITAL_TYPE_INPUT;
//	//}
//	//else if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_OUTPUT)==0 && val >= 0)
//	//{
//	//	bitsStates = m_pSelectedIFKDrvr->GetDigitalStateOutput(val);
//	//	digitalType = DIGITAL_TYPE_OUTPUT;
//	//}
//
//	if(!digitalType.IsEmpty())
//	{
//		FXString bitsState;
//		bitsState.Format("BitsState=0x%x(%d);", bitsStates, bitsStates);
//		
//		if(val>=0)
//			bitsState.Format("BitIndex=%d; BitValue=%d;", val, bitsStates);
//
//		if(!pLabel)
//		{
//			bitsStatesTxt.Format("DigitalType=%s; %s", digitalType, bitsState);
//		}
//		else
//		{
//			pLabel->Format("Dig%s", digitalType);
//			if(val>=0)
//			{
//				pLabel->Format("%s%d", *pLabel, val);
//				bitsState.Format("BitValue=%d;", bitsStates);
//			}
//			bitsStatesTxt = bitsState;
//		}		
//	}
//	return bitsStatesTxt.IsEmpty() ? "Error; Enter Any key for lis of commands." : bitsStatesTxt;
//}
//int ZGadgetSender::GetCommandVal( FXParser &pk, int pos )
//{
//	int bitVal = -1;
//	FXString val;
//	if(pk.GetParamString(pos, val))
//		bitVal = atoi(val);
//	return bitVal;
//}
//FXString ZGadgetSender::GetCmdParamValDescriptor( const FXString &cmd, const FXString &paramName, const FXString &descr, const FXString &valName /*= ""*/ )
//{
//	FXString cmdParamVal, val;
//	if(!valName.IsEmpty())
//		val.Format("(<%s>)", valName);
//	cmdParamVal.Format("%s %s%s - %s;\n", cmd, paramName, valName.IsEmpty() ? "" : val, descr);//"get inputs - returns back all Digital Inputs State\r\n as mask;\n")
//	return cmdParamVal;
//}
#pragma endregion | Methods Private |

#pragma region | Methods Protected |


#pragma endregion | Methods Protected |

#pragma region | Methods Public |
void ZGadgetSender::PropertiesRegistration()
{
	//ATTENTION!
	//All properties created and registered manually
	//in Threads Safe mode
}

void ZGadgetSender::ConnectorsRegistration()
{
	addInputConnector( createComplexDataType(1, text), "TxtCmdIn");
	addDuplexConnector( transparent, transparent, "Control");
	addOutputConnector( createComplexDataType(1, text) , "BinaryCmdOut");
}

CDataFrame* ZGadgetSender::DoProcessing( const CDataFrame* pDataFrame )
{
	if(pDataFrame!=NULL)
	{			
		const CTextFrame* pText = pDataFrame->GetTextFrame();

		if(pText!=NULL)
			HandleFrameText(*pText);
		else
		{
			const CQuantityFrame* pQuantity = pDataFrame->GetQuantityFrame();
			if(pQuantity!=NULL)
				HandleFrameQuantity(*pQuantity);
			else
				HandleFrame();
		}
	}
	return (CDataFrame*) NULL;
}

void ZGadgetSender::AsyncTransaction( CDuplexConnector* pConnector, CDataFrame* pParamFrame )
{
	if (!pParamFrame)
		return;

 	CTextFrame* tf=pParamFrame->GetTextFrame(DEFAULT_LABEL);
	if (tf)
	{    
    LPCTSTR pString = (LPCTSTR)tf->GetString();
    string binaryPacket(pString);
    const ZPacket* pPacket = ZPacket::Deserialize(
      binaryPacket,
      m_CommandsFactory,
      m_CommandsHeaders,
      m_Devices);

    if (pPacket
      && pPacket->GetDevice()
      && pPacket->GetCommand()
      && pPacket->GetCommand()->GetHeader()
      && m_iStoredDvcID == pPacket->GetDevice()->GetID()
      && pPacket->GetCommand()->GetHeader()->GetID() == m_CommandsHeaders["Set_Microstep_Resolution"].GetID())
    {
      m_iMicrostepsInStep = pPacket->GetCommand()->GetData();

      ZDeviceBase* pDevice = (ZDeviceBase*)m_Devices[m_iStoredDvcID];
      bool toCreate = !pDevice;
      if (toCreate)
        m_Devices.Add(ZDeviceBase(m_iStoredDvcID, m_iStepsPerRevolution, m_dMotionPerRevolution_um));

      ZDeviceSettings& settings = ((ZDeviceSettings&)m_Devices[m_iStoredDvcID]->GetSettings());

      if (toCreate)
        settings
        .SetProductLinearMotionPerRevolution_um(m_dMotionPerRevolution_um)
        .SetProductStepsPerRevolution(m_iStepsPerRevolution);

      settings.SetResolution(m_iMicrostepsInStep);
    }

	//	FXParser pk=(LPCTSTR)tf->GetString(); 
	//	FXString cmd;
	//	FXString param;
	//	int pos=0;
	//	pk.GetWord(pos,cmd);
	//	COutputConnector* pOutConnector = NULL;
	//	FXString label;
//
	//	if ((cmd.MakeLower().CompareNoCase(DUPLEX_COMMAND_NAME_GET)==0) && pk.GetWord(pos,cmd))
	//	{
	//		pOutConnector = pConnector;
	//		pk = OnGetCommand(cmd.MakeLower(), GetCommandVal(pk, pos));
	//	}
	//	else if ((cmd.CompareNoCase(DUPLEX_COMMAND_NAME_GET_OUT)==0) && pk.GetWord(pos,cmd))
	//	{
	//		pOutConnector = m_pOutput;
	//		pk = OnGetCommand(cmd.MakeLower(), GetCommandVal(pk, pos), &label);
	//	}
	//	
	//	if(pOutConnector==NULL)
	//	{			
	//		pOutConnector = pConnector;
	//		pk.Empty();
	//		pk="List of 8 available commands:\r\n";
    //
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_INPUTS,  "returns back all Digital Inputs State\r\n as mask",  "");
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_OUTPUTS, "returns back all Digital Outputs State\r\n as mask", "");
//
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_INPUT,  "returns back the state of\r\n specified Digital Input",   "bitIndex");
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_OUTPUT, "returns back the state of\r\n specified Digital Outputs", "bitIndex");
//
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_INPUTS,  "returns all Digital Inputs State as\r\n mask to the out pin",  "");
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_OUTPUTS, "returns all Digital Outputs State as\r\n mask to the out pin", "");
//
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_INPUT,  "returns back the state of\r\n specified Digital Input",   "bitIndex");
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_OUTPUT, "returns back the state of\r\n specified Digital Outputs", "bitIndex");
	}


		string commands("List of commands:\r\n");
		SendOut(pConnector, "Help", (commands + m_CommandsHeaders.ToString()).c_str());
	//}
	pParamFrame->Release(pParamFrame);
}
#pragma endregion | Methods Public |
























