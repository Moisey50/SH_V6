// UserExampleGadget.h : Declaration of the UserExampleGadget class
#ifndef __INCLUDE__FXControllerGadget_H__
#define __INCLUDE__FXControllerGadget_H__

#pragma once
#include <stdexcept>
#include "helpers\UserBaseGadget.h"

#include "ControllerCommandFactory.h"


using namespace std;

#define THIS_MODULENAME                  ("FXControllerGadget")

//#define LBL_CALCULATED_FPS               ("CalcFPS")
//#define LBL_CALCULATED_SHUTTER           ("CalcShutter")
//#define LBL_COLOR_CONFIG                 ("ColourConfig")
//#define LBL_DK_AND_COLORS_WITH_TICKS     ("DKandTicks")
#define LBL_STATUS_ABORT                 ("StatusAbort")
#define LBL_STATUS_VIDEO                 ("StatusVideo")
#define LBL_STATUS_SNAPSHOT              ("StatusSnapshot")
#define LBL_STATUS_PARAMS                ("StatusParams")
#define LBL_STATUS_COMMAND               ("StatusCommand")
#define LBL_FULL_PROGRAM                 ("FullProgram")

#define EXCEPTION_ID_SHUTTER_NOT_VALID   (100)

enum ConnectorsOut
{
	UNKNOWN = -1,
	COMMANDS = 0,
	STATUSES,
	//SHUTTER_OR_FPS,

	//ADD CONNECTORS BEFORE THIS LINE
	TOTAL_CONNECTORS
};

static const string CONNECTORS_OUT_NAMES[] = { "CmdsForController","Statuses","FPSorShutter" };

//commands to process
enum EXTERNAL_COMMAND
{
	GET_FPS = 0,
	ABORT = 1,
	VIDEO_CONFIG = 2,
	VIDEO_START = 3,
	SNPSHT_CONFIG = 4,
	SNPSHT_START = 5,
	VIDEO_CONF_START = 6,
	SNPSHT_CONF_START = 7
};

class FXControllerGadget
	: public UserBaseGadget
{

private:

	DWORD                          m_dwFrameCounter;
	CommandAbort                   m_cmdAbort; //standalone abort command
  CommandStartVideo              m_cmdStartVideo;
  CommandStartSnapshot           m_cmdStartSnapshot;
  string                         m_listOfPropsNames;
	COutputConnector*              m_pOutConnectorsArray[ConnectorsOut::TOTAL_CONNECTORS];
//	ControllerOperSettings         m_settings;
//	const ModeConfigRepository*    m_pModesConfigsRep;
//	const ControllerOperationBase* m_pCntrlrOperationVideo;
//	const ControllerOperationBase* m_pCntrlrOperationSnapshot;

	void HandleTextFrame(const CTextFrame * pTextConfigs);
	void HandleQuantityFrame(const CQuantityFrame * pInputCommand);
//	const CTextFrame* GetShutterTextFrame(int cameraShutter);
//	void ConfigureAndStart(const ControllerOperationBase& operation, DWORD dwDelay, const string& statusLabel = "");
//	void SetControllerOperations(const ModeConfigRepository* pModesConfigsRep, ControllerOperSettings& settings);

	//inline void Clear()
	//{
	//	if (m_pModesConfigsRep && !m_pModesConfigsRep->isEmpty())
	//	{
	//		delete m_pModesConfigsRep;
	//		m_pModesConfigsRep = NULL;
	//	}
	//	if (m_pCntrlrOperationVideo && !m_pCntrlrOperationVideo->isEmpty())
	//	{
	//		delete m_pCntrlrOperationVideo;
	//		m_pCntrlrOperationVideo = NULL;
	//	}
	//	if (m_pCntrlrOperationSnapshot && !m_pCntrlrOperationSnapshot->isEmpty())
	//	{
	//		delete m_pCntrlrOperationSnapshot;
	//		m_pCntrlrOperationSnapshot = NULL;
	//	}
	//}

	static void ConfigParamChanged(LPCTSTR pName, void* pOwner, bool& bInvalidate);

	FXControllerGadget(const FXControllerGadget&);
	FXControllerGadget& operator=(const FXControllerGadget&);

protected:

	FXControllerGadget();

	inline virtual ~FXControllerGadget() //override
	{
		Clear();
    m_listOfPropsNames = "";
	}

public:
	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

	DECLARE_RUNTIME_GADGET(FXControllerGadget);
};


#endif