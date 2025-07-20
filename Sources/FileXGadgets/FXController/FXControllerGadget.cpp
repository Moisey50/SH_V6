// UserExampleGadget.h : Implementation of the UserExampleGadget class
#include "StdAfx.h"
#include "FXControllerGadget.h"
#include <corecrt_memcpy_s.h>
#include <helpers\FramesHelper.h>

USER_FILTER_RUNTIME_GADGET( FXControllerGadget , "HWControllers" );	//	Mandatory


FXControllerGadget::FXControllerGadget()
	: m_pOutConnectorsArray()
	//, m_pModesConfigsRep()
	//, m_pCntrlrOperationVideo()
	//, m_pCntrlrOperationSnapshot()
  , m_listOfPropsNames()
	, m_dwFrameCounter(0)
	, m_cmdAbort()
  , m_cmdStartVideo()
  , m_cmdStartSnapshot()
	//, m_settings()
{
	//Code


	//	Mandatory

	init();
	
	//m_settings.setSettingsChangedListener(FXControllerGadget::ConfigParamChanged, this);
	
	for (int i = ConnectorsOut::UNKNOWN+1; i < ConnectorsOut::TOTAL_CONNECTORS; i++)
		m_pOutConnectorsArray[i] = GetOutputConnector(i);
}

void FXControllerGadget::ConfigParamChanged(LPCTSTR pName, void* pOwner, bool& bInvalidate)
{
	FXControllerGadget* pThisGadget = (FXControllerGadget*)pOwner;
	if (pThisGadget)
	{
		//if (pThisGadget->m_pModesConfigsRep && !pThisGadget->m_pModesConfigsRep->isEmpty())
		//	pThisGadget->SetControllerOperations(pThisGadget->m_pModesConfigsRep, pThisGadget->m_settings);
	}
}

//void FXControllerGadget::SetControllerOperations(const ModeConfigRepository* pModesConfigsRep, ControllerOperSettings& settings)
//{
//	if (pModesConfigsRep->getConfVideo())
//	{
//		delete m_pCntrlrOperationVideo;
//		m_pCntrlrOperationVideo = NULL;
//		m_pCntrlrOperationVideo = ControllerOperationFactory::create(*pModesConfigsRep->getConfVideo(), settings);
//	}
//
//	if (pModesConfigsRep->getConfSnapshot())
//	{
//		delete m_pCntrlrOperationSnapshot;
//		m_pCntrlrOperationSnapshot = NULL;
//		m_pCntrlrOperationSnapshot = ControllerOperationFactory::create(*pModesConfigsRep->getConfSnapshot(), settings);
//	}
//}

void FXControllerGadget::HandleTextFrame(const CTextFrame * pTextConfigs)
{
	ASSERT(pTextConfigs);

	CTextFrame* pOutputTextCommand = NULL;
	string inputConf = pTextConfigs->GetString().GetString();
	FXString errMsg;

	Clear();

	//m_pModesConfigsRep = ConfigurationFactory::deserialize(inputConf, __out errMsg);

	//if (m_pModesConfigsRep && !m_pModesConfigsRep->isEmpty() && errMsg.empty())
	//	SetControllerOperations(m_pModesConfigsRep, m_settings);
  FXString errMsgHdr;
  errMsgHdr.Format("Configuration is (%s) is in wrong format;", inputConf.c_str());
  
  int sbPrgrDelimiterIndx = (int)inputConf.find('&');
  if (sbPrgrDelimiterIndx < 0)
    errMsg.Format("%s The subprograms delimiter (&) is missing;", errMsgHdr, inputConf.c_str());

  FXString subprgrm_v = inputConf.substr(0, sbPrgrDelimiterIndx).c_str();
  FXString subprgrm_s = inputConf.substr(sbPrgrDelimiterIndx+1).c_str();

  int t = 0;
  for (int i = 0; i < subprgrm_v.GetLength(); i++)
  {
    if (subprgrm_v[i] == ';')
      t++;
  }

  if (t != 9)
  {
    FXString err_v;
    err_v.Format("Video subprogram is missing fields (%d instead %d);", t, 9);
    if (errMsg.IsEmpty())
      errMsg.Format("%s %s", errMsgHdr, err_v);
    else
      errMsg = errMsg + " " + err_v;
  }
  t = 0;
  for (int i = 0; i < subprgrm_s.GetLength(); i++)
  {
    if (subprgrm_s[i] == ';')
      t++;
  }

  if (t != 9)
  {
    FXString err_v;
    err_v.Format("Snapshot subprogram is missing fields (%d instead %d);", t, 9);
    if (errMsg.IsEmpty())
      errMsg.Format("%s %s", errMsgHdr, err_v);
    else
      errMsg = errMsg + " " + err_v;
  }

  FXString fullCmdData;
	if (!errMsg.IsEmpty())
		SENDERR("Error! %s", errMsg);
  else
  {
    FXString coeffs("0;-1000;1000;0;");
    fullCmdData.Format("%s%s&%s%s"
      , coeffs
      , subprgrm_v
      , coeffs
      , subprgrm_s
    );
  }

  CommandSetData cmdSetData = CommandSetData(string(fullCmdData));

  string cmdSetDataForMsgOnly = cmdSetData.getCmdText().substr(0, cmdSetData.getCmdText().length() - strlen("\n\r"));

	FXString msg;
  msg.Format
  (
    "Configurations (%s) were%s applied;%s"
    , !cmdSetData.isEmpty() ? cmdSetDataForMsgOnly.c_str() : inputConf.c_str()
    , errMsg.IsEmpty() ? "" : " not"
    , errMsg.IsEmpty() ? "" : " See errors before."
  );
	
  SENDINFO(msg);
	
  msg.Format("IsLoaded=%s; Value=%s;"
    , !cmdSetData.isEmpty() ? "true" : "false"
    , !cmdSetData.isEmpty() ? cmdSetData.getCmdText().c_str() : inputConf.c_str()
  );

	if (!errMsg.IsEmpty())
	{
		msg.Append(" Cause=");
		msg.Append(errMsg);
		msg.Append(";");
	}
	
  pOutputTextCommand = CreateTextFrame(msg, LBL_STATUS_PARAMS);
	PutFrame(m_pOutConnectorsArray[ConnectorsOut::STATUSES], pOutputTextCommand);

  pOutputTextCommand = CreateTextFrame(cmdSetData.getCmdText().c_str(), LBL_FULL_PROGRAM);
  PutFrame(m_pOutConnectorsArray[ConnectorsOut::COMMANDS], (CTextFrame*)pOutputTextCommand);
}

void FXControllerGadget::HandleQuantityFrame( const CQuantityFrame * pInputCommand )
{
  ASSERT( pInputCommand );

  const CTextFrame* pOutputTextCommand = NULL ;
  COutputConnector* pConnector = m_pOutConnectorsArray[ConnectorsOut::STATUSES];

  int nInputCmdId = (long)*pInputCommand;

  FXString errPrefix;

  //choose which command is pDataFrame and creating output accordingly	
  switch (nInputCmdId)
  {
    //  case GET_FPS://command getFps
    //  {
    //    FXString fpsResponce;
    //    bool isCalculated = m_pCntrlrOperationVideo && ((ControllerOperationVideo*)m_pCntrlrOperationVideo)->getCalculatedFPS() > 0;
    //    fpsResponce.Format( "IsFPS=%s; Value=%d;" , isCalculated ? "true" : "false" , ((ControllerOperationVideo*)m_pCntrlrOperationVideo)->getCalculatedFPS());
    //
    //    if ( !isCalculated )
    //      fpsResponce.Append( " Cause=Program configuration is missing or wrong!;" );
    //
    //    pOutputTextCommand = CreateTextFrame( fpsResponce , LBL_CALCULATED_FPS );
     //pConnector = m_pOutConnectorsArray[ConnectorsOut::SHUTTER_OR_FPS];
    //  }
    //  break;
  case ABORT://command abort
  {
    pOutputTextCommand = CreateTextFrame(m_cmdAbort.getCmdText().c_str(), LBL_STATUS_ABORT);
    pConnector = m_pOutConnectorsArray[ConnectorsOut::COMMANDS];
  }
  break;
  //case VIDEO_CONFIG://command configure videoMode (and send video mode shutter to camera)   
  //  ConfigureAndStartMode(true, cameraShutterVideoMode , FXString( videoModeConfigCommands.c_str() ) , 10 );
  //  break;
  //case VIDEO_START://command start videoMode
  //{
  //  configStartCmds = FXString( videoModeStartCommands.c_str() );
  //  dwDelay = 10;
  //}
  //break;
  //case SNPSHT_CONFIG://command configure snapshotMode (and send snapshotMode shutter to camera)
  //  ConfigureAndStartMode(false, cameraShutterSnapshotMode , FXString( snapshotStartCommands.c_str() ) , 5 );
  //  break;
  //case SNPSHT_START://command start snapshotMode
  //{
  //  configStartCmds = FXString( snapshotStartCommands.c_str() );
  //  dwDelay = 5;
  //}
  //break;
  case VIDEO_CONF_START://command start videomode 
  {
    //if (m_pCntrlrOperationVideo)
    //  ConfigureAndStart(*m_pCntrlrOperationVideo, 7, LBL_STATUS_VIDEO);
    pOutputTextCommand = CreateTextFrame(m_cmdStartVideo.getCmdText().c_str(), LBL_STATUS_VIDEO);
    pConnector = m_pOutConnectorsArray[ConnectorsOut::COMMANDS];
  }
  break;
  case SNPSHT_CONF_START://command start snapshotMode  
  {
    //if (m_pCntrlrOperationSnapshot)
    //  ConfigureAndStart(*m_pCntrlrOperationSnapshot, 5, LBL_STATUS_SNAPSHOT);
    pOutputTextCommand = CreateTextFrame(m_cmdStartSnapshot.getCmdText().c_str(), LBL_STATUS_SNAPSHOT);
    pConnector = m_pOutConnectorsArray[ConnectorsOut::COMMANDS];
  } break;
  default://unknoun command
  {
    FXString errMsg;
    errMsg.Format("The (%d) is unknown command!", nInputCmdId);
    SENDERR("Error! %s", errMsg);
    FXString statusMsg;
    statusMsg.Format("IsReceived = false; Cause=%s;", errMsg);
    pOutputTextCommand = CreateTextFrame(statusMsg, LBL_STATUS_COMMAND);
  }
  break;
  }
  
  if ( pOutputTextCommand != NULL )
  {
    PutFrame( pConnector , (CTextFrame*) pOutputTextCommand );
    m_dwFrameCounter++ ;
  }
}

//void FXControllerGadget::ConfigureAndStart(const ControllerOperationBase& operation, DWORD dwDelay , const string& statusLabel /*=""*/ )
//{
//  bool isStarted = false;
//  const CTextFrame* pOutputTextCommand = NULL;
//  FXString errMsg;
//  FXString msg;
//  FXString colConfigMsg = operation.getColorConfigDescr().c_str();
//  FXString modeName = operation.getModeName().c_str();
//  try
//  {
//	  CTextFrame* pColourConfOutputTextCommand = CreateTextFrame(colConfigMsg, LBL_COLOR_CONFIG );
//	  PutFrame(m_pOutConnectorsArray[ConnectorsOut::STATUSES], pColourConfOutputTextCommand);
//
//	  pOutputTextCommand = GetShutterTextFrame( /*sutter*/operation.getCameraShutter());
//
//	  if (pOutputTextCommand != NULL)
//		  PutFrame(m_pOutConnectorsArray[ConnectorsOut::SHUTTER_OR_FPS], (CTextFrame*)pOutputTextCommand);
//    
//    pOutputTextCommand = CreateTextFrame(operation.getDKandELandTicksAsTxt().c_str(), LBL_DK_AND_COLORS_WITH_TICKS );
//    if (pOutputTextCommand != NULL)
//      PutFrame(m_pOutConnectorsArray[ConnectorsOut::SHUTTER_OR_FPS], (CTextFrame*)pOutputTextCommand);
//
//    if (!statusLabel.empty())
//	  {
//      const int DELAY_TO_EMPTY_CAMERA_FRAME_us = 25;
//		  FXString configStartCmds = operation.getCmdsAsTxt().c_str();
//		  if (!operation.isEmpty())
//			  PutTextsSeparated(m_pOutConnectorsArray[ConnectorsOut::COMMANDS], (FXString&)configStartCmds, _T("\n"), NULL, dwDelay, &m_dwFrameCounter, CommandClear().getCmdText().c_str(), DELAY_TO_EMPTY_CAMERA_FRAME_us);
//		  else
//		  {
//			  errMsg = modeName; 
//			  errMsg.Append(" mode command is terminated!");
//		  }
//
//		  isStarted = errMsg.GetLength() == 0;
//	  }
//  }
//  catch ( const exception& ex )
//  {
//	  errMsg = modeName;
//    errMsg.Append( ex.what() );
//  }
//
//  if ( !statusLabel.empty() )
//  {
//    msg.Format( "IsStarted=%s;" , isStarted ? "true" : "false" );
//    if ( errMsg.GetLength() > 0 )
//    {
//      msg.Append( " Cause=" );
//      msg.Append( errMsg );
//      msg.Append( "; " );
//	    msg.Append(modeName);
//      msg.Append( " configuration program is wrong or missing!;" );
//    }
//
//    CTextFrame * pStatusMsg = CreateTextFrame( msg , statusLabel.c_str() );
//    PutFrame(m_pOutConnectorsArray[ConnectorsOut::STATUSES], pStatusMsg );
//  }
//}

//const CTextFrame* FXControllerGadget::GetShutterTextFrame( int cameraShutter )
//{
//  CTextFrame* res = NULL;
//
//  if ( cameraShutter < 1 )
//  {
//    FXString errMsg;
//    errMsg.Format( "Shutter (%d) is not valid! Super Cause: Shutter is not initialized yet!" , cameraShutter );
//    throw  logic_error( string( errMsg ) );
//  }
//
//  FXString shutterAsTxt;
//  shutterAsTxt.Format( "%d" , cameraShutter );
//
//  res = CreateTextFrame( shutterAsTxt , LBL_CALCULATED_SHUTTER);
//
//  return res;
//}

CDataFrame* FXControllerGadget::DoProcessing(const CDataFrame* pDataFrame)
{
	if (pDataFrame != NULL)
	{
		try
		{
			//figuring out if the 'pDataFrame', is it a Configuration or Command?
			const CTextFrame * inputTextCommand = pDataFrame->GetTextFrame();
			if ((inputTextCommand != NULL))//pDataFrame is Configuration    
				HandleTextFrame(inputTextCommand);
			else
			{
				const CQuantityFrame * inputNumberCommand = pDataFrame->GetQuantityFrame();
				if ((inputNumberCommand != NULL)) //pDataFrame is Command
					HandleQuantityFrame(inputNumberCommand);
			}
		}
		catch (const std::exception& ex)
		{
			SENDERR("Error! %s", ex.what());
		}
	}
	return (CDataFrame*)NULL;
}

void FXControllerGadget::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{

};


void FXControllerGadget::PropertiesRegistration()
{
  //addProperty(SProperty::EDITBOX		,	_T("double1")	,	&dProp1		,	SProperty::Double		);
  //addProperty(SProperty::SPIN			,	_T("long2")		,	&lProp3		,	SProperty::Long	,	3		,	9	);
  //addProperty(SProperty::SPIN_BOOL	,	_T("bool_int4")	,	&iProp4		,	SProperty::SpinBool	,	-10		,	20, &bProp4	);
  //addProperty(SProperty::EDITBOX		,	_T("bool6")		,	&bProp6		,	SProperty::Bool		);

 // m_listOfPropsNames =  m_settings.getAllValuesNamesForReset();

	//addProperty(SProperty::COMBO, m_settings.getPropertyIndxToReset().getName().c_str(), &m_settings.getPropertyIndxToReset().m_currentRef, SProperty::Int, m_listOfPropsNames.c_str());
	//SetChangeNotification(m_settings.getPropertyIndxToReset().getName().c_str(), m_settings.getPropertyIndxToReset().getPropertyChangedHandler(), &m_settings.getPropertyIndxToReset());

	//const char* pList = "false;true";
	//addProperty(SProperty::COMBO, m_settings.getIsSixPinsConnectors().getName().c_str(), &m_settings.getIsSixPinsConnectors().m_currentRef, SProperty::Int, pList);
	//SetChangeNotification(m_settings.getIsSixPinsConnectors().getName().c_str(), m_settings.getIsSixPinsConnectors().getPropertyChangedHandler(), &m_settings.getIsSixPinsConnectors());
	//
 // addProperty(SProperty::COMBO, m_settings.getToEnforce3TicksInVideo().getName().c_str(), &m_settings.getToEnforce3TicksInVideo().m_currentRef, SProperty::Int, pList);
 // SetChangeNotification(m_settings.getToEnforce3TicksInVideo().getName().c_str(), m_settings.getToEnforce3TicksInVideo().getPropertyChangedHandler(), &m_settings.getToEnforce3TicksInVideo());


	//vector<ControllerOperSettingValueInt*> spinProps = m_settings.getIntValuesCollection();

	//for (vector<ControllerOperSettingValueInt*>::iterator it = spinProps.begin(); it != spinProps.end(); it++)
	//{
	//	if (*it )
	//	{
	//		ControllerOperSettingValueInt& val = **it;
	//		addProperty(SProperty::SPIN, val.getName().c_str(), &(**it).m_currentRef, SProperty::Long, val.getLowerBound(), val.getUpperBound());
	//		SetChangeNotification(val.getName().c_str(), val.getPropertyChangedHandler(), *it);
	//	}
	//}
};


void FXControllerGadget::ConnectorsRegistration()
{
  addInputConnector( transparent , "ConfigurationsAndCommands" );

  for (int i = ConnectorsOut::UNKNOWN + 1; i < ConnectorsOut::TOTAL_CONNECTORS; i++)
	  addOutputConnector(transparent, CONNECTORS_OUT_NAMES[i].c_str());
};