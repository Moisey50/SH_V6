#ifndef GENERICSCRIPTGADGET_DLL
#define AFX_EXT_GENERIC_GADGET __declspec(dllimport)
#ifdef _DEBUG
#pragma comment(lib, "GenericScriptGadget.lib")
#else
#pragma comment(lib, "GenericScriptGadget.lib")
#endif
#else
#define AFX_EXT_GENERIC_GADGET __declspec(dllexport) // exports functionality to enable the creation of derived script runner gadgets
#endif

#pragma once

#include <Gadgets\gadbase.h>
#include <fxfc\fxfc.h>
#include "gadgets\TextFrame.h"
#include "gadgets\FigureFrame.h"
#include "gadgets\ContainerFrame.h"
#include "GenericScriptRunnerDlg.h"
#include <embedch.h>

#define FIGUREFRAME 1 
#define TEXTFRAME 2 
#define INFOFRAME 3 
#define VIDEOFRAME 4 

extern CDynLinkLibrary* pThisDll;

class TxtToScript // used to send text data from any text gadget to script [not TVObjects gadget]
{
public:
	TxtToScript() { GraphTime = 0. ; pTxtOut = NULL ; } ; // constructor of text data
	double GraphTime; // graph time in which text data was sent to script 
	char* pTxtOut ; // pointer to text 
};
typedef struct PointToScriptStruct // define points data structure used in the TVObjects data structure
{
	double x , y;
	int IsROI ; // determines if the point defines the Region Of Interest or a detected point 
}
PointToScript ;

class NodeToScript
	// defines the predetermined TVObjects data structure. 
  // each such data structure is part of a linked list (contains a pointer to 
	// the previous data structure). every data structure can contain data about one the following data frames:
	// 1. FigureFrame - data regarding all the figures found in the frame.
	// 2. TextFrame - data regarding all the texts found in the frame 
	// 3. InfoFrame – data regarding the entire linked list
{
public:
	NodeToScript ( int iType , char * pName, NodeToScript * pPrev = NULL ) // constructor of nodes
	{ 
    memset( this , 0 , sizeof(*this) ) ;
		m_iType = iType ; 
		m_pPrev = pPrev ;
		int iNameLen = (int)strlen(pName) ;
		m_pName = new char[iNameLen + 1] ;
		strcpy( m_pName , pName ) ;
	}
	~NodeToScript()  // destructor of nodes 
	{
		if (m_pName)
			delete[] m_pName ;
	} 
	int m_iType; // type of current data structure. possible values: 0 = figure frame, 1 = text frame, 2 = info frame 
	NodeToScript* m_pPrev; // pointer to the previous data structure in current linked list. 
	char* m_pName; // pointer to the label of the figure or text frame
	PointToScript* m_pPoints; // pointer to the array containing the coordinates of the detected objects 
	int m_nPoints; // number of detected points 
	char* m_pValue; // string of detected text
	int m_DoDelete; // determines whether data structure should be deleted after the data was acquired by the script
	void *pFrame; // pointer to the video frame after TVObjects gadget's analysis 
	void *pOriginalFrame; // pointer to the original video frame before TVObjects gadget's analysis	
	DWORD CameraTime; // frame creation in the camera time stamp determined by camera
	DWORD CameraFrameNumber ; // frame number determined by camera
	DWORD CameraOutP; // camera input
	DWORD CameraInP; // camera output
	DWORD CameraX; // X coordinate determined by camera
	DWORD CameraY; // Y coordinate determined by camera
	int m_iId; // frame ID
	double m_dFrameTime; // data frame creation in graph time stamp
	double m_dBeforeScriptTime; // before transmitting data structure to script time stamp
	int m_iInputIndex; // input pin index 
	int m_iNfigs; // number of figure frames in data structure
	int m_iNTexts; // number of text frames in data structure
	NodeToScript** pFigs; // array of all the figure pointers in data structure
	NodeToScript** pTxts; // array of all the text pointers in data structure
};

typedef struct tagPointers // data structure used to store pointers to nodes. example - all pointers to all figure nodes.
{
	NodeToScript * pNextNode;
}
Pointers ;

typedef struct // used to declare functions that are common for both c++ and script
{
	char * pFuncProto ;
	 ChFuncdl_t FuncCallBack ;
} chdl_function;

class GenericScriptRunner : public CFilterGadget  // Script Runner gadget is a filter gadget. meaning: responses to start and stop only if 
	// data packets received in input pin. it does however responses to Init [creating new gadget \ loading graph \ reloading script]
{
protected:
	CExecutionStatus* m_pStatus; // graph execution status. possible values: STOP, PAUSE, RUN, EXIT
	CPtrArray         m_Outputs; // array of output pins
	BOOL              m_bRun; // is graph running? possible values: True \ False
	FXLockObject       m_Lock;  // used to prevent simultaneous editing of variables 
	CPtrArray         m_Chdls; // array of all function that are common for both c++ and script 

public:
    CDuplexConnector*	m_pControl;
	void AFX_EXT_GENERIC_GADGET InitExecutionStatus(CExecutionStatus* Status); 
  virtual bool AFX_EXT_GENERIC_GADGET ScanProperties(LPCTSTR Text, bool& Invalidate);
  virtual bool AFX_EXT_GENERIC_GADGET PrintProperties(FXString& text);
	virtual int AFX_EXT_GENERIC_GADGET GetDuplexCount();
	AFX_EXT_GENERIC_GADGET virtual CDuplexConnector* GetDuplexConnector(int n);
	virtual void AFX_EXT_GENERIC_GADGET AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
	AFX_EXT_GENERIC_GADGET GenericScriptRunner(void); // constructor of script runners. AFX_EXT_GENERIC_GADGET to enable access to all derived Script gadget.
	int AFX_EXT_GENERIC_GADGET GetOutputsCount(); // gets the gadget's number of output pins 
	void InitOutputs(); // deletes gadgets outputs if exists and creates m_iNumOfOutPins outputs.
	COutputConnector* GetOutputConnector(int n) { return (COutputConnector*)m_Outputs[n] ; } ;   // gets pointer to specific output pin (counting starts from 0 - upper most output pin)
	void DeleteOutputs(); // deletes gadgets output
	void AreOutPutsReady(); // checks whether all the gadgets connected to script runner are initialized. only if true - OnInit function in the script is called.
	double GetPresentTime() { return GetGraphTime() ;} ; // gets graph time in microseconds

	virtual int AFX_EXT_GENERIC_GADGET DoJob(); // runs continuously and controls gadget's behavior  
	AFX_EXT_GENERIC_GADGET CDataFrame* DoProcessing(CDataFrame* pPacket); // handles the data processing - sends received data to relevant functions in script 
	NodeToScript * ConvertFrameToStruct( CDataFrame * pInnerNext, CFramesIterator * pIterator, int iInputIndex ); // converts the received data from TVObjects to linked list of data node defined above 
	int DestroyStruct(  NodeToScript * pOut ); // deletes the linked list of data node defined above 
  virtual void AFX_EXT_GENERIC_GADGET OnStart(); // called if start button was pressed + capture gadget is connected to script runner (directly or indirectly) 
  virtual void AFX_EXT_GENERIC_GADGET OnStop(); // called if stop button was pressed + capture gadget is connected to script runner (directly or indirectly) 
	//AFX_EXT_GENERIC_GADGET virtual void OnControl(); // called if Msg was recived in control pin
	bool AFX_EXT_GENERIC_GADGET ScanSettings(FXString& text);
	void SetNotInitialized( bool bVal ) { m_bNotInitialized = bVal ; } ; // set flag m_bNotInitialized to bVal
	void SetCallOnExitFunc( bool bVal ) { m_bCallOnExitFunc = bVal ; } ; // set flag m_bCallOnExitFunc to bVal
	int InitInterpParsScriptAndCallInitFuncInScript(); // parses the script and calls OnInit_Ch function in the script with input INIT
	int InitInterpreter(); // initializes the Script Runner
	int ParsScript(); // parses the script 
	int CallOnInit_ChFuncInScript(int iRetVal, char* InBuf, char* OutBuf ); // general call to OnInit_Ch function in the script with InBuf input
	int CallInitFuncInScript(); // calls OnInit_Ch function in the script with input INIT
	int CallStartFuncInScript(); // calls OnInit_Ch function in the script with input START
	int CallStopFuncInScript(); // calls OnInit_Ch function in the script with input STOP
	int CallOnDataStructInput_ChFuncInScript(int iRetVal, int iInputIndex, NodeToScript* pOut ); // calls OnDataStructInput_Ch function in script to process data from TVObject 
	int CallOnTxtStructInput_ChFuncInScript(int iRetVal, int iInputIndex, TxtToScript* pOut ) ; // calls OnnTxtStructInput_Ch function in script to process data from any text gadget (not TVObject)   
	int CallExitFuncInScriptAndEndInterp(); // calls OnExit_Ch function in the script and shuts down interpreter 
	int CallExitFuncInScript(); // prepares and calls CallOnExit_ChFuncInScript(int iRetVal, char* InBuf, char* OutBuf ) ;
	int CallOnExit_ChFuncInScript(int iRetVal, char* InBuf, char* OutBuf ); // calls OnExit_Ch function in the script 
	int EndInterpreter(); // shuts down interpreter 
	void displayParsContentsInFile(char *filename); // displays pars errors (occur during parsing and dont allow execution) in pop up window and text file
	void displayRunTimeContentsInFile(char *filename, char* FunctionName); // displays run time errors (occur during execution) in pop up window and text file

	virtual void AFX_EXT_GENERIC_GADGET ShutDown(); // removes the gadget 

	static friend void CALLBACK fn_ScriptIndexedReceive(CDataFrame* pDataFrame, void* pGadget, CConnector* lpInput) // call back that receives the info in input pin and puts it in processing queue
	{
		GenericScriptRunner * pGadg = ((GenericScriptRunner*)pGadget) ;
		CContainerFrame * pNewPacket = CContainerFrame::Create() ;  
		pNewPacket->AddFrame( pDataFrame ) ;
		if ( !pGadg->m_pInputQueue->PutQueueObject( pNewPacket ))
			pDataFrame->Release( pDataFrame ) ;
		SetEvent( pGadg->m_evHasData ) ;
	}
	//GenericScriptRunnerDlg*    m_pSetupDlg; // set up dialog 
	FXStaticQueue<CDataFrame*> *m_pInputQueue; // input queue 
	HANDLE										 m_evHasData; // event has data   
	ChInterp_t									 m_Interp; // Ch interpreter structure 
	double										 m_dAfterScriptTime; // graph time after script was executed 
	double										 m_dFrameTime; // graph time in which the frame was created in graph
	double										 m_dBeforeScriptTime;  // graph time in which linked list was created and the nodes are ready to be sent to script
	double										 m_dStrcutToAfterScriptProcessTime; // time duration between sending the data to script and finishing executing the script
	double										 m_dFrameToStructProcessTime; // time duration between frame creation in graph and converting the frame to data structure ready to be sent to script 
	double										 m_dTotalProcessTime; // time duration between frame creation in graph and finishing executing the script
	double										 m_LastEOSTime; // storage the last End Of Stream (EOS) time 
	double										 m_EOSSnoozeTime; // if EOS received, ignore all other EOS received with EOSSnoozeTime duration to prevent multiple calls to on stop function in script	
	int												 m_iNumOfOutPins; // num of out put pins 
	int												 m_iIndexInRunner; // the index of this instance of script runner gadget 
	FXString										 m_ScriptPath; // path to script 
	FXString										 m_EmbedChDir; // path to embedch folder
  char *                       m_EmbedChDirBuf[1000] ;
  char *                       m_EmbedChLicenseBuf[1000] ;
	FXString										 m_GadgetName; // gadget name given by graph builder
	void										  *m_fptrInit; // address of function OnInit_Ch in c++ - ch common memory space
	void										  *m_fptrExit; // address of function OnExit_Ch in c++ - ch common memory space
	void										  *m_fptrDataStructInput; // address of function OnDataStruct_Ch in c++ - ch common memory space
	void										  *m_fptrTxtStructInput; // address of function OnTxtData_Ch in c++ - ch common memory space
	bool										   m_CallOnStopFunc; // flag call OnStop_Ch function in script. possible values: true / false
	bool										   m_CallOnStartFunc; // flag call OnStart_Ch function in script. possible values: true / false
	bool										   m_bCallOnExitFunc; // flag call OnExit_Ch function in script. possible values: true / false
	bool										   m_OutputsReady; // flag are all the gadgets connected to script runner initialized? only if true - OnInit function in the script is called.
	bool										   m_bNotInitialized; // flag Script Runner gadget not initialized. possible values : true / false
	bool											 m_bNotParsed; // flag Script Runner gadget not parsed. possible values : true / false
	bool										   m_bExit; // flag exit. possible values: true / false.
};
extern AFX_EXT_GENERIC_GADGET GenericScriptRunner* GetMyRunner( int iIndex ); // gets the index of current sciprt runner gadget
