// phidget_0_16_16.cpp: implementation of the phidget_0_16_16 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "phidget_0_16_16.h"
#include "phidget_0_16_16Kit.h"
#include <assert.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment( lib, "phidget21.lib" )

//IMPLEMENT_RUNTIME_GADGET_EX(Phidget_0_16_16, CPhIKBaseGadget, /*"Video.capture"*/LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

USER_FILTER_RUNTIME_GADGET(Phidget_0_16_16, "HWControllers"); //LINEAGE_GENERIC);


Phidget_0_16_16::Phidget_0_16_16()
  : m_pSelectedIFKDrvr(NULL)
  , m_bInputStateAsMask(false)
  , m_bOutputStateAsMask(false)
  , m_bOutputCmdAsMask(false)
  , m_iStoredSN(PROP_INDEX_NOT_SELECTED)
{
  init();
  if(!g_pPhidgetDrvr)
    g_pPhidgetDrvr = new CPhidgetsDriver();
  g_pPhidgetDrvr->Init((LPIPhidgetManagerListener)this);
  if(!g_pPhidgetDrvr->IsRunning())
    g_pPhidgetDrvr->Start();
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  GadgetName += _T("IsoThread") ;
  SetNameAndStart( (LPCTSTR) GadgetName ) ;
}

#pragma region | Methods Private |

void Phidget_0_16_16::SendOut( const FXString& data , 
  const FXString * pLabel )
{
  CTextFrame* pTxtFrm=CTextFrame::Create(data);
  pTxtFrm->ChangeId(NOSYNC_FRAME);
  if ( pLabel )
    pTxtFrm->SetLabel( *pLabel);
  pTxtFrm->SetTime(GetGraphTime() * 1.e-3);

  if ( !PutForOutput( pTxtFrm ) )
      pTxtFrm->RELEASE(pTxtFrm);
}
void Phidget_0_16_16::SendOut_Digital( bool isDigitalInput, int bitIndx, BYTE bitVal )
{
  FXString txtData;
  txtData.Format(/*"BitVal=*/"%d", bitVal);
  FXString label;
  label.Format("Dig%s%d", (isDigitalInput ? DIGITAL_TYPE_INPUT : DIGITAL_TYPE_OUTPUT), bitIndx);
  SendOut(txtData , &label );
}
void Phidget_0_16_16::SendOut_Analog( bool isAnalogInput, int analogIndx, double analogVal )
{
  FXString txtData;
  txtData.Format(/*"AnlgVal=*/"%f", analogVal);
  FXString label;
  label.Format("Analog%s%d", (isAnalogInput ? DIGITAL_TYPE_INPUT : DIGITAL_TYPE_OUTPUT), analogIndx);
  SendOut(txtData , &label );
}
int Phidget_0_16_16::GetNewValue(const FXString& command)
{
  int bitVal = -1;
  if(command.CompareNoCase(DIGITAL_OUTPUT_COMMAND_ON)==0)
    bitVal=1;
  else if(command.CompareNoCase(DIGITAL_OUTPUT_COMMAND_OFF)==0)
    bitVal=0;
  return bitVal;
}
void Phidget_0_16_16::ChangeDigitalOutput(BYTE bitIndx, const FXString& command)
{
  int bitVal = GetNewValue(command);
  if(m_pSelectedIFKDrvr && bitIndx >= 0 && bitIndx < m_pSelectedIFKDrvr->GetInfo().GetNumOutputs())
  {
    if(bitVal>=0)
      m_pSelectedIFKDrvr->SetOutput(bitIndx, bitVal);
    else if(command.CompareNoCase(DIGITAL_OUTPUT_COMMAND_TOGGLE)==0)
      m_pSelectedIFKDrvr->ToggelOutput(bitIndx);
  }
}
void Phidget_0_16_16::ChangeDigitalOutputs(int bitsIndxMask, const FXString& command)
{
  if(m_pSelectedIFKDrvr && TRUE == m_bOutputCmdAsMask)
  {
    int bitIndex = 0;
    while (bitIndex < m_pSelectedIFKDrvr->GetInfo().GetNumOutputs())
    {
      int bitMask = 1 << (1 * bitIndex);
      if(bitsIndxMask & bitMask)
        ChangeDigitalOutput(bitIndex, command);
      bitIndex++;
    }
  }
}

void Phidget_0_16_16::HandleFrame( )
{
  if(m_pSelectedIFKDrvr && m_pSelectedIFKDrvr->GetInfo().GetNumAnalogs())
  {
    int analogIn = m_pSelectedIFKDrvr->GetInfo().GetNumAnalogs();
    for(int i = 0; i < analogIn; i++)
    {
      int analogVal = m_pSelectedIFKDrvr->GetAnalogInputValue(i);
      //if(analogVal)
      //	SendOut_Analog(true, i, analogVal*0.2222-61.111);
    }
  }
}
void Phidget_0_16_16::HandleFrameText( const CTextFrame& data )
{
  FXString command = ((FXString)data.GetLabel()).Trim();

  FXSIZE delimiterIndx = 0;
  FXString bitsIndexes = data.GetString();
  if(!bitsIndexes.GetLength())
    HandleFrame();
  else if(bitsIndexes.Find("0x")==0
    || bitsIndexes.Find("0X")==0)
  {
    char *eptr;
    ChangeDigitalOutputs(strtoul(bitsIndexes, &eptr, 16), command);
  }
  else
  {
    while(bitsIndexes.GetLength()>delimiterIndx)
    {
      FXString bitIndxTxt = bitsIndexes.Tokenize(LIST_DELIMITER, delimiterIndx);
      ChangeDigitalOutput(atoi(bitIndxTxt), command);
    }
  }
}
void Phidget_0_16_16::HandleFrameQuantity( const CQuantityFrame& data )
{
  if(m_pSelectedIFKDrvr)
  {
    FXString command = ((FXString)data.GetLabel()).Trim();
    long bitsIndexes = data.operator long();

    if(bitsIndexes < m_pSelectedIFKDrvr->GetInfo().GetNumOutputs() && FALSE == m_bOutputCmdAsMask)
      ChangeDigitalOutput((BYTE)bitsIndexes, command);
    else 
      ChangeDigitalOutputs(bitsIndexes, command);
  }
}
FXString Phidget_0_16_16::OnGetCommand( const FXString &getCmd, int val, FXString *pLabel /*= NULL */ )
{
  FXString bitsStatesTxt;
  FXString digitalType;
  unsigned bitsStates = 0;
  bool isSingle = false;

  if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_INPUTS)==0)
  {
    bitsStates = m_pSelectedIFKDrvr->GetAllStatesInputs();
    digitalType = DIGITAL_TYPE_INPUT;
  }
  else if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_OUTPUTS)==0)
  {
    bitsStates = m_pSelectedIFKDrvr->GetAllStatesOutputs();
    digitalType = DIGITAL_TYPE_OUTPUT;
  }
  else if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_INPUT)==0 && val >= 0)
  {
    bitsStates = m_pSelectedIFKDrvr->GetDigitalStateInput(val);
    digitalType = DIGITAL_TYPE_INPUT;
  }
  else if(getCmd.CompareNoCase(DUPLEX_COMMAND_PARAM_OUTPUT)==0 && val >= 0)
  {
    bitsStates = m_pSelectedIFKDrvr->GetDigitalStateOutput(val);
    digitalType = DIGITAL_TYPE_OUTPUT;
  }

  if(!digitalType.IsEmpty())
  {
    FXString bitsState;
    bitsState.Format("BitsState=0x%x(%d);", bitsStates, bitsStates);

    if(val>=0)
      bitsState.Format("BitIndex=%d; BitValue=%d;", val, bitsStates);

    if(!pLabel)
    {
      bitsStatesTxt.Format("DigitalType=%s; %s", digitalType, bitsState);
    }
    else
    {
      pLabel->Format("Dig%s", digitalType);
      if(val>=0)
      {
        pLabel->Format("%s%d", *pLabel, val);
        bitsState.Format("BitValue=%d;", bitsStates);
      }
      bitsStatesTxt = bitsState;
    }		
  }
  return bitsStatesTxt.IsEmpty() ? "Error; Enter Any key for lis of commands." : bitsStatesTxt;
}
int Phidget_0_16_16::GetCommandVal( FXParser &pk, FXSIZE pos )
{
  int bitVal = -1;
  FXString val;
  if(pk.GetParamString(pos, val))
    bitVal = atoi(val);
  return bitVal;
}
FXString Phidget_0_16_16::GetCmdParamValDescriptor( const FXString &cmd, const FXString &paramName, const FXString &descr, const FXString &valName /*= ""*/ )
{
  FXString cmdParamVal, val;
  if(!valName.IsEmpty())
    val.Format("(<%s>)", valName);
  cmdParamVal.Format("%s %s%s - %s;\n", cmd, paramName, valName.IsEmpty() ? "" : val, descr);//"get inputs - returns back all Digital Inputs State\r\n as mask;\n")
  return cmdParamVal;
}
#pragma endregion | Methods Private |

#pragma region | Methods Protected |

void Phidget_0_16_16::PhidgetKit_OutputChangeRequested( DWORD dwStateMask, DWORD dwBitId )
{

}
#pragma endregion | Methods Protected |

#pragma region | Methods Public |
void Phidget_0_16_16::PropertiesRegistration()
{
  //ATTENTION!
  //All properties created and registered manually
  //in Threads Safe mode
}

void Phidget_0_16_16::ConnectorsRegistration()
{
  addInputConnector( createComplexDataType(2, text, quantity), "DigOutCmd");
  addDuplexConnector( transparent, transparent, "Control");
  addOutputConnector( createComplexDataType(2, text, quantity) , "DigitalState");
}

CDataFrame* Phidget_0_16_16::DoProcessing( const CDataFrame* pDataFrame )
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

void Phidget_0_16_16::AsyncTransaction( CDuplexConnector* pConnector, CDataFrame* pParamFrame )
{
  if (!pParamFrame)
    return;

  CTextFrame* tf=pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (tf)
  {
    FXParser pk=(LPCTSTR)tf->GetString(); 
    FXString cmd;
    FXString param;
    FXSIZE pos=0;
    pk.GetWord(pos,cmd);
    COutputConnector* pOutConnector = NULL;
    FXString label;

    if ((cmd.MakeLower().CompareNoCase(DUPLEX_COMMAND_NAME_GET)==0) && pk.GetWord(pos,cmd))
    {
      pOutConnector = pConnector;
      pk = OnGetCommand(cmd.MakeLower(), GetCommandVal(pk, pos));
    }
    else if ((cmd.CompareNoCase(DUPLEX_COMMAND_NAME_GET_OUT)==0) && pk.GetWord(pos,cmd))
    {
      pOutConnector = m_pOutput;
      pk = OnGetCommand(cmd.MakeLower(), GetCommandVal(pk, pos), &label);
    }

    if(pOutConnector==NULL)
    {			
      pOutConnector = pConnector;
      pk.Empty();
      pk="List of 8 available commands:\r\n";

      pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_INPUTS,  "returns back all Digital Inputs State\r\n as mask",  "");
      pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_OUTPUTS, "returns back all Digital Outputs State\r\n as mask", "");

      pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_INPUT,  "returns back the state of\r\n specified Digital Input",   "bitIndex");
      pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET, DUPLEX_COMMAND_PARAM_OUTPUT, "returns back the state of\r\n specified Digital Outputs", "bitIndex");

      pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_INPUTS,  "returns all Digital Inputs State as\r\n mask to the out pin",  "");
      pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_OUTPUTS, "returns all Digital Outputs State as\r\n mask to the out pin", "");

      pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_INPUT,  "returns back the state of\r\n specified Digital Input",   "bitIndex");
      pk += GetCmdParamValDescriptor(DUPLEX_COMMAND_NAME_GET_OUT, DUPLEX_COMMAND_PARAM_OUTPUT, "returns back the state of\r\n specified Digital Outputs", "bitIndex");
    }

    SendOut( pk , &label );
  }
  pParamFrame->Release(pParamFrame);
}
#pragma endregion | Methods Public |