#include "stdafx.h"
#include "ZGadgetReceiver.h"


#define THIS_MODULENAME "ZGadgetReceiver"

USER_FILTER_RUNTIME_GADGET(ZGadgetReceiver, "HWControllers"); //LINEAGE_GENERIC);


string ZGadgetReceiver::COMMANDS_TEXT_COLLECTION;


ZGadgetReceiver::ZGadgetReceiver()
  : /*m_iStoredSN(-1)//PROP_INDEX_NOT_SELECTED)
  ,*/ m_sPacketRecieved()
  , m_sCmdToFilter( NO_COMMAND_TO_FILTER )
  , m_iStoredDvcID( 0 )
  //, m_dLastTime(-1)
  //, m_dDeltaTime(0)
  //, m_packetsQueue()
  //, m_zPacketsQueue()
{
  init();
}

#pragma region | Methods Private |

//void Zaber_Motor::SendOut_Digital( bool isDigitalInput, int bitIndx, BYTE bitVal )
//{
//	FXString txtData;
//	txtData.Format(/*"BitVal=*/"%d", bitVal);
//	FXString label;
//	label.Format("Dig%s%d", (isDigitalInput ? DIGITAL_TYPE_INPUT : DIGITAL_TYPE_OUTPUT), bitIndx);
//	SendOut(m_pOutput, label, txtData);
//}
//void Zaber_Motor::SendOut_Analog( bool isAnalogInput, int analogIndx, double analogVal )
//{
//	FXString txtData;
//	txtData.Format(/*"AnlgVal=*/"%f", analogVal);
//	FXString label;
//	label.Format("Analog%s%d", (isAnalogInput ? DIGITAL_TYPE_INPUT : DIGITAL_TYPE_OUTPUT), analogIndx);
//	SendOut(m_pOutput, label, txtData);
//}
//int Zaber_Motor::GetNewValue(const FXString& command)
//{
//	int bitVal = -1;
//	if(command.CompareNoCase(DIGITAL_OUTPUT_COMMAND_ON)==0)
//		bitVal=1;
//	else if(command.CompareNoCase(DIGITAL_OUTPUT_COMMAND_OFF)==0)
//		bitVal=0;
//	return bitVal;
//}
//void Zaber_Motor::ChangeDigitalOutput(BYTE bitIndx, const FXString& command)
//{
//	int bitVal = GetNewValue(command);
//	if(m_pSelectedIFKDrvr && bitIndx >= 0 && bitIndx < m_pSelectedIFKDrvr->GetInfo().GetNumOutputs())
//	{
//		if(bitVal>=0)
//			m_pSelectedIFKDrvr->SetOutput(bitIndx, bitVal);
//		else if(command.CompareNoCase(DIGITAL_OUTPUT_COMMAND_TOGGLE)==0)
//			m_pSelectedIFKDrvr->ToggelOutput(bitIndx);
//	}
//}
//void Zaber_Motor::ChangeDigitalOutputs(int bitsIndxMask, const FXString& command)
//{
//	if(m_pSelectedIFKDrvr && TRUE == m_bOutputCmdAsMask)
//	{
//		int bitIndex = 0;
//		while (bitIndex < m_pSelectedIFKDrvr->GetInfo().GetNumOutputs())
//		{
//			int bitMask = 1 << (1 * bitIndex);
//			if(bitsIndxMask & bitMask)
//				ChangeDigitalOutput(bitIndex, command);
//			bitIndex++;
//		}
//	}
//}

//void Zaber_Motor::HandleFrame( )
//{
//	if(m_pSelectedIFKDrvr && m_pSelectedIFKDrvr->GetInfo().GetNumAnalogs())
//	{
//		int analogIn = m_pSelectedIFKDrvr->GetInfo().GetNumAnalogs();
//		for(int i = 0; i < analogIn; i++)
//		{
//			int analogVal = m_pSelectedIFKDrvr->GetAnalogInputValue(i);
//			//if(analogVal)
//			//	SendOut_Analog(true, i, analogVal*0.2222-61.111);
//		}
//	}
//}
//void Zaber_Motor::HandleFrameText( const CTextFrame& data )
//{
//	FXString command = ((FXString)data.GetLabel()).Trim();
//
//	int delimiterIndx = 0;
//	FXString bitsIndexes = data.GetString();
//	if(!bitsIndexes.GetLength())
//		HandleFrame();
//	else if(bitsIndexes.Find("0x")==0
//		|| bitsIndexes.Find("0X")==0)
//	{
//		char *eptr;
//		ChangeDigitalOutputs(strtoul(bitsIndexes, &eptr, 16), command);
//	}
//	else
//	{
//		while(bitsIndexes.GetLength()>delimiterIndx)
//		{
//			FXString bitIndxTxt = bitsIndexes.Tokenize(LIST_DELIMITER, delimiterIndx);
//			ChangeDigitalOutput(atoi(bitIndxTxt), command);
//		}
//	}
//}
//void Zaber_Motor::HandleFrameQuantity( const CQuantityFrame& data )
//{
//	if(m_pSelectedIFKDrvr)
//	{
//		FXString command = ((FXString)data.GetLabel()).Trim();
//		long bitsIndexes = data.operator long();
//
//		if(bitsIndexes < m_pSelectedIFKDrvr->GetInfo().GetNumOutputs() && FALSE == m_bOutputCmdAsMask)
//			ChangeDigitalOutput((BYTE)bitsIndexes, command);
//		else 
//			ChangeDigitalOutputs(bitsIndexes, command);
//	}
//}
//FXString Zaber_Motor::OnGetCommand( const FXString &getCmd, int val, FXString *pLabel /*= NULL */ )
//{
//	FXString bitsStatesTxt;
//	FXString digitalType;
//	unsigned bitsStates = 0;
//	bool isSingle = false;
//
//	if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_INPUTS)==0)
//	{
//		bitsStates = m_pSelectedIFKDrvr->GetAllStatesInputs();
//		digitalType = DIGITAL_TYPE_INPUT;
//	}
//	else if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_OUTPUTS)==0)
//	{
//		bitsStates = m_pSelectedIFKDrvr->GetAllStatesOutputs();
//		digitalType = DIGITAL_TYPE_OUTPUT;
//	}
//	else if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_INPUT)==0 && val >= 0)
//	{
//		bitsStates = m_pSelectedIFKDrvr->GetDigitalStateInput(val);
//		digitalType = DIGITAL_TYPE_INPUT;
//	}
//	else if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_OUTPUT)==0 && val >= 0)
//	{
//		bitsStates = m_pSelectedIFKDrvr->GetDigitalStateOutput(val);
//		digitalType = DIGITAL_TYPE_OUTPUT;
//	}
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
//int Zaber_Motor::GetCommandVal( FXParser &pk, int pos )
//{
//	int bitVal = -1;
//	FXString val;
//	if(pk.GetParamString(pos, val))
//		bitVal = atoi(val);
//	return bitVal;
//}
//FXString Zaber_Motor::GetCmdParamValDescriptor( const FXString &cmd, const FXString &paramName, const FXString &descr, const FXString &valName /*= ""*/ )
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
void ZGadgetReceiver::PropertiesRegistration()
{
	//ATTENTION!
	//All properties created and registered manually
	//in Threads Safe mode
}

void ZGadgetReceiver::ConnectorsRegistration()
{
	UserBaseGadget::addInputConnector( createComplexDataType(1, text), "SerialData");
	//UserBaseGadget::addDuplexConnector( transparent, transparent, "Control");
	UserBaseGadget::addOutputConnector( createComplexDataType(1, text) , "ZaberPacket");
	//UserBaseGadget::addOutputConnector( createComplexDataType(1, text) , "ZaberPacketDescr");
}
#include <ostream>
CDataFrame* ZGadgetReceiver::DoProcessing( const CDataFrame* pDataFrame )
{
  if ( pDataFrame != NULL )
  {
    const CTextFrame* pText = pDataFrame->GetTextFrame();

    if ( pText )
    {
      const ZPacket* pPacket = ZPacket::Deserialize( (const string) pText->GetString() , m_CommandsFactory , m_CommandsHeaders , m_Devices );
      if ( pPacket )
      {
        string outTxt = pPacket->ToString();
        string lbl = "";

        if ( m_iStoredDvcID == pPacket->GetDevice()->GetID() && m_sCmdToFilter == pPacket->GetCommand()->GetHeader()->GetInstructionName() )
        {
          ostringstream oss;
          ZCommandPosition *pCmdPos = (ZCommandPosition*) pPacket->GetCommand();
          if ( pCmdPos )
            oss << fixed << setprecision( 4 ) << pCmdPos->GetPosition();
          else
            oss << pPacket->GetCommand()->GetData();

          outTxt = oss.str();

          lbl = pPacket->GetDevice()->ToString() + ";" + pPacket->GetCommand()->GetHeader()->GetInstructionName();
        }
        else if ( m_iStoredDvcID > 0 || m_sCmdToFilter != NO_COMMAND_TO_FILTER )
          outTxt.clear();

        if ( !outTxt.empty() )
          SendOut( m_pOutput , lbl.c_str() , outTxt.c_str() , pText->GetId() );

        delete pPacket;
      }
    }
  }
  return (CDataFrame*) NULL;
}

void ZGadgetReceiver::AsyncTransaction( CDuplexConnector* pConnector, CDataFrame* pParamFrame )
{
	if (!pParamFrame)
		return;

	CTextFrame* tf=pParamFrame->GetTextFrame(DEFAULT_LABEL);
	if (tf)
	{
		//FXParser pk=(LPCTSTR)tf->GetString(); 
		//FXString cmd;
		//FXString param;
		//int pos=0;
		//pk.GetWord(pos,cmd);
		//COutputConnector* pOutConnector = NULL;
		//FXString label;

		//if ((cmd.MakeLower().CompareNoCase(DUPLEX_COMMAND_NAME_GET)==0) && pk.GetWord(pos,cmd))
		//{
		//	pOutConnector = pConnector;
		//	pk = OnGetCommand(cmd.MakeLower(), GetCommandVal(pk, pos));
		//}
		//else if ((cmd.CompareNoCase(DUPLEX_COMMAND_NAME_GET_OUT)==0) && pk.GetWord(pos,cmd))
		//{
		//	pOutConnector = m_pOutput;
		//	pk = OnGetCommand(cmd.MakeLower(), GetCommandVal(pk, pos), &label);
		//}

		//if(pOutConnector==NULL)
		//{			
		//	pOutConnector = pConnector;
		//	pk.Empty();
		//	pk="List of 8 available commands:\r\n";

		//	pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_INPUTS,  "returns back all Digital Inputs State\r\n as mask",  "");
		//	pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_OUTPUTS, "returns back all Digital Outputs State\r\n as mask", "");

		//	pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_INPUT,  "returns back the state of\r\n specified Digital Input",   "bitIndex");
		//	pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_OUTPUT, "returns back the state of\r\n specified Digital Outputs", "bitIndex");

		//	pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_INPUTS,  "returns all Digital Inputs State as\r\n mask to the out pin",  "");
		//	pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_OUTPUTS, "returns all Digital Outputs State as\r\n mask to the out pin", "");

		//	pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_INPUT,  "returns back the state of\r\n specified Digital Input",   "bitIndex");
		//	pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_OUTPUT, "returns back the state of\r\n specified Digital Outputs", "bitIndex");
		//}

		//SendOut(pOutConnector, label, pk);
	}
	pParamFrame->Release(pParamFrame);
}



#pragma endregion | Methods Public |