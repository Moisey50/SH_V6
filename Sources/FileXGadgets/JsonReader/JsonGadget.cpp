#include "stdafx.h"
#include "JsonGadget.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



USER_FILTER_RUNTIME_GADGET(JsonGadget, LINEAGE_GENERIC);


void JSNSendLogMessage(const string& message, bool isError)
{
	if (isError)
		JSN_SENDERR_0(message.c_str());
	else
		JSN_SENDINFO_0(message.c_str());
}

FXLockObject                     JsonGadget::m_Locker;

JsonGadget::JsonGadget(void)
	: m_bIsJsonLoaded(false)
	, m_jsonPath()
	, m_jsonFigures()
	, m_toAutoSaveJsonFigures(false)
{
	init();
}


JsonGadget::~JsonGadget(void)
{
	m_jsonFigures.SerializeToFile(m_jsonPath.GetString(), JSNSendLogMessage);
	//ShutDown();
}

#pragma region | Methods Private |
void JsonGadget::HandleFrame()
{

}
void JsonGadget::HandleFrameText(const CTextFrame& data)
{
	m_Locker.Lock();

	if (m_jsonFigures.GetFigures().empty() || !m_bIsJsonLoaded)
	{
		m_jsonFigures.DeserializeFromFile(m_jsonPath.GetString(), JSNSendLogMessage);
		m_bIsJsonLoaded = m_jsonFigures.IsValid();
	}

	string keyId(data.GetString());

	if (keyId.empty())
		JSN_SENDERR_1("Bad request (empty key) from '%s' file.", m_jsonPath);
	else
	{
		const JFigure* cpJf = m_jsonFigures.GetFigure(keyId);
		if (cpJf)
			SendOut(m_pOutput, "", cpJf->SerializeToTransfer().c_str(), data.GetId());
		else
		{
			JFigure jf("");
			jf.DeserializeFromTransfer(keyId.c_str(), JSNSendLogMessage);

			if (jf.GetId().empty())
				JSN_SENDERR_2("There is NO '%s' member in the '%s' file.", keyId.c_str(), m_jsonPath);
			else
			{
				const JFigure* cpJfBeforeUpdate = m_jsonFigures.GetFigure(jf.GetId());
				stringstream ss;
				ss << "figure '" << jf.GetId() << "' is missing in the '" << m_jsonPath << "' file.";
				string missingJf(ss.str());

				if (cpJfBeforeUpdate)
				{
					ss.str("");
					ss.clear();
					ss << "figure '" << cpJfBeforeUpdate->GetId() << "' before update looks like " << cpJfBeforeUpdate->ToString();
				}

				SendOut(m_pOutput, "", ss.str().c_str(), data.GetId());

				m_jsonFigures.Add(jf);
				const JFigure* cpJfAfterUpdate = m_jsonFigures.GetFigure(jf.GetId());
				ss.str("");
				ss.clear();

				if (!cpJfAfterUpdate)
					ss << missingJf;
				else
					ss << "figure '" << cpJfAfterUpdate->GetId() << "' after update looks like " << cpJfAfterUpdate->ToString();

				SendOut(m_pOutput, "", ss.str().c_str(), data.GetId());

				ss.str("");
				ss.clear();
				ss << "figure '" << jf.GetId() << "' received from transfer is " << jf.ToString();
				SendOut(m_pOutput, "", ss.str().c_str(), data.GetId());

				if (m_toAutoSaveJsonFigures)
					m_jsonFigures.SerializeToFile(m_jsonPath.GetString(), JSNSendLogMessage);
			}
		}
		m_Locker.Unlock();
	}

	//if (!m_jsonPath.IsEmpty())
	//{
	//	try
	//	{
	//		if (!m_documentFromFile.IsArray())
	//			ParseScript(string(m_jsonPath));
	//	}
	//	catch (exception ())
	//	{

	//	}

	//	string key(data.GetString());

	//	if (key.empty())
	//		JSN_SENDERR_1("Bad request (empty key) from '%s' file.", m_jsonPath);
	//	else if (!m_documentFromFile.HasMember(key.c_str()))
	//		JSN_SENDERR_2("There is NO '%s' member in the '%s' file.", key.c_str(), m_jsonPath);
	//	else
	//		SendOut(m_pOutput, "", m_documentFromFile[key.c_str()].GetString(), data.GetId());
	//	
	//	//m_Locker.Lock();
	//	//
	//	//if (!m_Devices[0])
	//	//	m_Devices.Add(ZDeviceBase(0));
	//	//if (!m_Devices[m_iStoredDvcID])
	//	//	m_Devices.Add(ZDeviceBase(m_iStoredDvcID, m_iStepsPerRevolution, m_dMotionPerRevolution_um));
	//	//else
	//	//{
	//	//	ZDeviceSettings& settings = ((ZDeviceSettings&)m_Devices[m_iStoredDvcID]->GetSettings());
	//	//
	//	//	settings
	//	//		.SetProductLinearMotionPerRevolution_um(m_dMotionPerRevolution_um)
	//	//		.SetProductStepsPerRevolution(m_iStepsPerRevolution);
	//	//}
	//	//
	//	//if (!m_bAreSettingsUploaded)
	//	//{
	//	//	ZCommandHeader setResolution = m_CommandsHeaders[37];
	//	//
	//	//	if (!ZCommandHeader::IsEmptyCommand(setResolution))
	//	//	{
	//	//		ZCommandBase cmdSetResolution;
	//	//		cmdSetResolution
	//	//			.SetHeader(&setResolution)
	//	//			.SetData(m_iMicrostepsInStep);
	//	//
	//	//		const ZPacket* pPacketSetResolution = ZPacket::Deserialize(cmdSetResolution.ToString(), m_CommandsFactory, m_CommandsHeaders, m_Devices, *m_Devices[m_iStoredDvcID]);
	//	//
	//	//		if (pPacketSetResolution)
	//	//		{
	//	//			SendOut(m_pOutput, "", pPacketSetResolution->ToString().c_str(), data.GetId());
	//	//			SendOut(m_pOutput, "", pPacketSetResolution->Serialize().c_str(), data.GetId());
	//	//		}
	//	//
	//	//		m_bAreSettingsUploaded = true;
	//	//		ZDeviceSettings& settings = ((ZDeviceSettings&)m_Devices[m_iStoredDvcID]->GetSettings());
	//	//		settings.SetResolution(m_iMicrostepsInStep);
	//	//
	//	//		Sleep(500);
	//	//	}
	//	//}
	//	//m_Locker.Unlock();
	//	//
	//	//const ZPacket* pPacket = ZPacket::Deserialize((string)data.GetString(), m_CommandsFactory, m_CommandsHeaders, m_Devices, *m_Devices[m_iStoredDvcID]);
	//	//
	//	//if (pPacket)
	//	//{
	//	//	SendOut(m_pOutput, "", pPacket->ToString().c_str(), data.GetId());
	//	//	SendOut(m_pOutput, "", pPacket->Serialize().c_str(), data.GetId());
	//	//}
	//	//
	//	//delete pPacket;
	//}
}
void JsonGadget::HandleFrameQuantity(const CQuantityFrame& data)
{

}
//FXString JsonGadget::OnGetCommand(const FXString &getCmd, int val, FXString *pLabel /*= NULL */)
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
//	if (!digitalType.IsEmpty())
//	{
//		FXString bitsState;
//		bitsState.Format("BitsState=0x%x(%d);", bitsStates, bitsStates);
//
//		if (val >= 0)
//			bitsState.Format("BitIndex=%d; BitValue=%d;", val, bitsStates);
//
//		if (!pLabel)
//		{
//			bitsStatesTxt.Format("DigitalType=%s; %s", digitalType, bitsState);
//		}
//		else
//		{
//			pLabel->Format("Dig%s", digitalType);
//			if (val >= 0)
//			{
//				pLabel->Format("%s%d", *pLabel, val);
//				bitsState.Format("BitValue=%d;", bitsStates);
//			}
//			bitsStatesTxt = bitsState;
//		}
//	}
//	return bitsStatesTxt.IsEmpty() ? "Error; Enter Any key for lis of commands." : bitsStatesTxt;
//}
//int JsonGadget::GetCommandVal(FXParser &pk, int pos)
//{
//	int bitVal = -1;
//	FXString val;
//	if (pk.GetParamString(pos, val))
//		bitVal = atoi(val);
//	return bitVal;
//}
//FXString JsonGadget::GetCmdParamValDescriptor(const FXString &cmd, const FXString &paramName, const FXString &descr, const FXString &valName /*= ""*/)
//{
//	FXString cmdParamVal, val;
//	if (!valName.IsEmpty())
//		val.Format("(<%s>)", valName);
//	cmdParamVal.Format("%s %s%s - %s;\n", cmd, paramName, valName.IsEmpty() ? "" : val, descr);//"get inputs - returns back all Digital Inputs State\r\n as mask;\n")
//	return cmdParamVal;
//}
#pragma endregion | Methods Private |

#pragma region | Methods Protected |


#pragma endregion | Methods Protected |

#pragma region | Methods Public |
void JsonGadget::PropertiesRegistration()
{
	//ATTENTION!
	//All properties created and registered manually
	//in Threads Safe mode
}

void JsonGadget::ConnectorsRegistration()
{
	addInputConnector(createComplexDataType(1, text), "TxtCmdIn");
	addDuplexConnector(transparent, transparent, "Control");
	addOutputConnector(createComplexDataType(1, text), "BinaryCmdOut");
}

CDataFrame* JsonGadget::DoProcessing(const CDataFrame* pDataFrame)
{
	if (pDataFrame != NULL)
	{
		const CTextFrame* pText = pDataFrame->GetTextFrame();

		if (pText != NULL)
			HandleFrameText(*pText);
		else
		{
			const CQuantityFrame* pQuantity = pDataFrame->GetQuantityFrame();
			if (pQuantity != NULL)
				HandleFrameQuantity(*pQuantity);
			else
				HandleFrame();
		}
	}
	return (CDataFrame*)NULL;
}

void JsonGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
	if (!pParamFrame)
		return;

	//CTextFrame* tf=pParamFrame->GetTextFrame(DEFAULT_LABEL);
	//if (tf)
	//{
	//	FXParser pk=(LPCTSTR)tf->GetString(); 
	//	FXString cmd;
	//	FXString param;
	//	int pos=0;
	//	pk.GetWord(pos,cmd);
	//	COutputConnector* pOutConnector = NULL;
	//	FXString label;

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

	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_INPUTS,  "returns back all Digital Inputs State\r\n as mask",  "");
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_OUTPUTS, "returns back all Digital Outputs State\r\n as mask", "");

	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_INPUT,  "returns back the state of\r\n specified Digital Input",   "bitIndex");
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_OUTPUT, "returns back the state of\r\n specified Digital Outputs", "bitIndex");

	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_INPUTS,  "returns all Digital Inputs State as\r\n mask to the out pin",  "");
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_OUTPUTS, "returns all Digital Outputs State as\r\n mask to the out pin", "");

	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_INPUT,  "returns back the state of\r\n specified Digital Input",   "bitIndex");
	//		pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_OUTPUT, "returns back the state of\r\n specified Digital Outputs", "bitIndex");
	//	}


	string commands("List of commands:\r\n");
	//SendOut(pConnector, "Help", (commands + m_CommandsHeaders.ToString()).c_str());
	//}
	pParamFrame->Release(pParamFrame);
}
#pragma endregion | Methods Public |
