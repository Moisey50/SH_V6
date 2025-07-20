#include "StdAfx.h"
#include "GenericScriptRunner.h"
#include <fxfc\fxext.h>
#include <fxfc\FXRegistry.h>
#include <fstream> // for is exist file 


using namespace std;

#ifdef _DEBUG
#pragma comment( lib, "chsdk_mdd.lib" )
#pragma comment( lib, "embedch_mdd.lib" )
#else
#pragma comment( lib, "chsdk.lib" )
#pragma comment( lib, "embedch.lib" )
#endif

#define THIS_MODULENAME "GenericScriptRunner"
#define MAX_ERR_MSG_LEN 1000
#define SWAP4BYTES(x)(((x&0Xff)<<24)|((x&0Xff00)<<8)|((x&0Xff0000)>>8)|((x&0Xff000000)>>24)); // used for extra info from camera like ROI etc.

AFX_EXT_GENERIC_GADGET CArray<GenericScriptRunner*,GenericScriptRunner*> g_pRunner; // array of all currently existing script runner gadgets
AFX_EXT_GENERIC_GADGET GenericScriptRunner* GetMyRunner( int iIndex ) // returns the index of current script runner gadget
{ 
	if ( iIndex >= 0   &&  iIndex < g_pRunner.GetCount() )
	{
		return g_pRunner[iIndex] ;
	}
	return NULL ;
}
int TxtToGadgetOut ( int iOutPinNumber , char * pOut, int iRunnerIndex ) // enables the sending of text from script to SHStudio
{   
	GenericScriptRunner *pRunnerLocal = g_pRunner[iRunnerIndex-1];
	if (!pRunnerLocal->m_bExit)
	{
		if  ( (iOutPinNumber > pRunnerLocal->GetOutputsCount()-1 ) || ( iOutPinNumber < 0 ) )
		{    
			FXString HeadLine("Script ");
			HeadLine.Append(pRunnerLocal->m_ScriptPath); 
			HeadLine.Append(" RunTime Error In Function TxtOut_toC");
			FXString ErrContent ("Script Sends Information To Non Existing Out Pins. Fix This And Reload The Script. ");
			MessageBox(NULL, ErrContent, HeadLine, MB_ICONWARNING | MB_OK ); 
			pRunnerLocal->SetCallOnExitFunc(true);
			return 0 ; 
		}
		else
		{
			CTextFrame * pTextFr = CTextFrame::Create(); 
			if (pTextFr)
			{
				if ((pOut) && (strstr( pOut, "%T"))) // delete the %T and write instead the total Processing time = from frame capture till after script finished execution 
				{
					pRunnerLocal->m_dAfterScriptTime = pRunnerLocal->GetPresentTime(); 
					pRunnerLocal->m_dFrameToStructProcessTime = pRunnerLocal->m_dBeforeScriptTime - pRunnerLocal->m_dFrameTime ; 
					pRunnerLocal->m_dStrcutToAfterScriptProcessTime = pRunnerLocal->m_dAfterScriptTime - pRunnerLocal->m_dBeforeScriptTime ; 
					pRunnerLocal->m_dTotalProcessTime = pRunnerLocal->m_dStrcutToAfterScriptProcessTime + pRunnerLocal->m_dFrameToStructProcessTime ; 
					FXString Out = pOut ; 
					Out.Trim("%%T");
					FXString TotalProcessTime;
					TotalProcessTime.Format("%6.2f", (pRunnerLocal->m_dTotalProcessTime/1000) );
					TotalProcessTime.Append("[ms]::");
					Out.Insert(0, TotalProcessTime);          
					pOut = Out.GetBuffer();
					pTextFr->GetString() += pOut ;
					pOut = NULL;
				}
				if ((pOut)&&(strstr( pOut, "%S"))) // delete the %S and write instead the script Processing time = from CallOnDataStructInput_ChFuncInScript / CallOnTxtInput_ChFuncInScript till after script finished execution 
				{
					pRunnerLocal->m_dAfterScriptTime = pRunnerLocal->GetPresentTime(); 
					pRunnerLocal->m_dStrcutToAfterScriptProcessTime = pRunnerLocal->m_dAfterScriptTime - pRunnerLocal->m_dBeforeScriptTime ; 
					FXString Out = pOut ; 
					Out.Trim("%%S");
					FXString ScriptProcessTime;
					ScriptProcessTime.Format("%6.2f", (pRunnerLocal->m_dStrcutToAfterScriptProcessTime/1000) );
					ScriptProcessTime.Append("[ms]::");
					Out.Insert(0, ScriptProcessTime);
					pOut = Out.GetBuffer();
					pTextFr->GetString() += pOut ;
					pOut = NULL;
				}
				if ((pOut)&&(strstr( pOut, "%B"))) // delete the %B and write instead the SHStudio Processing time = from frame captured till converting it to data structure ready to be sent to script
				{
					pRunnerLocal->m_dFrameToStructProcessTime = pRunnerLocal->m_dBeforeScriptTime - pRunnerLocal->m_dFrameTime ; 
					FXString Out = pOut ; 
					Out.Trim("%%B");
					FXString SHProcessTime;
					SHProcessTime.Format("%6.2f", (pRunnerLocal->m_dFrameToStructProcessTime/1000) );
					SHProcessTime.Append("[ms]::");
					Out.Insert(0, SHProcessTime);          
					pOut = Out.GetBuffer();
					pTextFr->GetString() += pOut ;
					pOut = NULL;
				}
				if ((pOut)&&(strstr( pOut, "%C"))) // delete the %C and write instead current time 
				{
					pRunnerLocal->m_dAfterScriptTime = pRunnerLocal->GetPresentTime(); 
					FXString Out = pOut ; 
					Out.Trim("%%C");
					FXString CurrTime;
					CurrTime.Format("%6.2f", (pRunnerLocal->m_dAfterScriptTime/1000) );
					CurrTime.Append("[ms]::");
					Out.Insert(0, CurrTime);
					pOut = Out.GetBuffer();
					pTextFr->GetString() += pOut ;
					pOut = NULL;
				}
				else if (pOut)
				{
					pTextFr->GetString() += pOut ;        
					pOut = NULL;
				}
				pTextFr->SetTime(pRunnerLocal->GetPresentTime());
				pTextFr->ChangeId( 0 ) ;
#ifdef _TRACE_DATAFRAMERELEASE
				TRACE(">>>>>> (%s) frame 0x%x with ID %d\n",GADEGTNAME, pTextFr, (pTextFr)?pTextFr->GetId():-1);
#endif
				COutputConnector* pOutput = pRunnerLocal->GetOutputConnector( iOutPinNumber ); 
				if (pOutput)
				{
					pTextFr->AddRef();
					if (!pOutput->Put(pTextFr))
						pTextFr->RELEASE(pTextFr);
				}
				pTextFr->RELEASE(pTextFr);
				return 1;
			}
		}
		return 0;
	}
	else
		return 0 ;
}
int FrameToGadgetOut(  int iOutPinNumber , void * pFrame, int iRunnerIndex ) // enables the sending of frames from script to SHStudio
{   
	GenericScriptRunner *pRunnerLocal = g_pRunner[iRunnerIndex-1];
	if (!pRunnerLocal->m_bExit)
	{
		if  ( (iOutPinNumber > pRunnerLocal->GetOutputsCount()-1 ) || ( iOutPinNumber < 0 ) )
		{    
			FXString HeadLine("Script ");
			HeadLine.Append(pRunnerLocal->m_ScriptPath); 
			HeadLine.Append(" RunTime Error In Function FrameOut_toC");
			FXString ErrContent ("Script Sends Information To Non Existing Out Pins. Fix This And Reload The Script. ");
			MessageBox(NULL, ErrContent, HeadLine, MB_ICONWARNING | MB_OK ); 
			pRunnerLocal->SetCallOnExitFunc(true);
			return 0 ; 
		}
		else
		{
			CVideoFrame * pVidFr = (CVideoFrame * ) pFrame; 
			if (pVidFr)
			{
				pVidFr->AddRef();
				pVidFr->SetTime(pRunnerLocal->GetPresentTime());
				pVidFr->ChangeId( 0 ) ;
#ifdef _TRACE_DATAFRAMERELEASE
				TRACE(">>>>>> (%s) frame 0x%x with ID %d\n",GADEGTNAME, pVidFr, (pVidFr)?pVidFr->GetId():-1);
#endif
				COutputConnector* pOutput = pRunnerLocal->GetOutputConnector( iOutPinNumber ); 
				if (pOutput)
				{
					pVidFr->AddRef();
					if (!pOutput->Put(pVidFr))
						pVidFr->RELEASE(pVidFr);
				}
				pVidFr->RELEASE(pVidFr);
				return 1;
			}
		}
		return 0;
	}
	else
		return 0 ;
}
EXPORTCH int TxtOut_toC_chdl(void *varg) // enables the sending of text from script to SHStudio
{
	ChInterp_t interp;
	ChVaList_t ap;
	Ch_VaStart(interp, ap, varg);
	int iIndex = Ch_VaArg( interp, ap, int );
	char * pOut = Ch_VaArg( interp, ap, char * );
	int iRunner = Ch_VaArg( interp, ap, int);
	int retval = TxtToGadgetOut( iIndex , pOut, iRunner );
	Ch_VaEnd(interp, ap);
	return retval;
} 
EXPORTCH int FrameOut_toC_chdl(void *varg) // enables the sending of frames from script to SHStudio
{
	ChInterp_t interp;
	ChVaList_t ap;
	Ch_VaStart(interp, ap, varg);
	int iIndex = Ch_VaArg( interp, ap, int );  
	void * pFrame = Ch_VaArg( interp, ap, void * );
	int iRunner = Ch_VaArg( interp, ap, int);
	int retval = FrameToGadgetOut( iIndex , pFrame, iRunner );
	Ch_VaEnd(interp, ap);
	return retval;
}

int GenericScriptRunner::GetDuplexCount()
{
	return 1;
}

 CDuplexConnector* GenericScriptRunner::GetDuplexConnector(int n)
{
	return ((!n) ? m_pControl : NULL);
}

void GenericScriptRunner::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
	CTextFrame* TextFrame = pParamFrame->GetTextFrame(DEFAULT_LABEL);
	if (TextFrame)
	{	
		FXString NewPath = TextFrame->GetString();
		fstream fin;
		fin.open(NewPath,ios::in);
		if (fin.is_open())   // script file exists
		{
			fin.close();
			m_ScriptPath = TextFrame->GetString();
			SetCallOnExitFunc( true );    
			SetNotInitialized( true ); 
		}
		else
		{
			FXString NoFile ("There is no script in the path you entered:  ")  ;      // create a proper error massage 
			NoFile.Append(NewPath);
			NoFile.Append("          ");
			FXString NoFileHead = "Parse Error ";
			MessageBox(NULL, NoFile, NoFileHead, MB_ICONWARNING | MB_OK ); 
		}
	}
	pParamFrame->RELEASE(pParamFrame);
}
void GenericScriptRunner::InitExecutionStatus(CExecutionStatus* Status)
{
  bool Suspended=(::WaitForSingleObject(m_evResume, 0) != WAIT_OBJECT_0);
  if (!Suspended)
    Suspend();
	if (m_pStatus)
		m_pStatus->Release() ;
  m_pStatus = CExecutionStatus::Create(Status);
	if ( m_pStatus->GetStatus() == CExecutionStatus::RUN)
		m_bRun = TRUE ;
	else
		m_bRun = FALSE;
	Resume();
}
bool GenericScriptRunner::ScanProperties(LPCTSTR Text, bool& Invalidate)
{
	FXPropertyKit pk(Text);
	pk.GetString("Path", m_ScriptPath);    
	int NumOfOutPinsBefore = m_iNumOfOutPins;
	pk.GetInt("NumOfOutPins", m_iNumOfOutPins); 
	if ( NumOfOutPinsBefore > m_iNumOfOutPins )  
	{
		for (int i=NumOfOutPinsBefore; i>m_iNumOfOutPins; i--)
		{
			COutputConnector* pOutput = GetOutputConnector(i-1); 
			if (pOutput->IsConnected()) 
				pOutput->Disconnect();
			delete pOutput;
			m_Outputs.RemoveAt(i-1);
		}
	}
	if ( NumOfOutPinsBefore < m_iNumOfOutPins )  
	{
		for (int i=NumOfOutPinsBefore; i<m_iNumOfOutPins; i++)
		{
			m_Outputs.Add( new COutputConnector(transparent) );
		}
	}
	return true;
}
bool GenericScriptRunner::PrintProperties(FXString& text)
{
	FXPropertyKit pk;
	pk.WriteString("Path", m_ScriptPath);
	pk.WriteInt("NumOfOutPins", m_iNumOfOutPins);
	text += pk;
	return true;
}
GenericScriptRunner::GenericScriptRunner(void) :
	m_fptrExit(NULL),
	m_fptrInit(NULL),
	m_fptrDataStructInput(NULL),
	m_fptrTxtStructInput(NULL),
	m_Interp(NULL),
	m_GadgetName("GenericScriptRunner1"),
	m_EmbedChDir("c:\\ch\\toolkit\\embedch"),
	m_iIndexInRunner (0),
	m_CallOnStartFunc(1),
	m_CallOnStopFunc (0),
	m_bCallOnExitFunc(0),
	m_LastEOSTime(0),
	m_OutputsReady(0),
	m_bNotInitialized(1),
	m_bNotParsed(1),
	m_bRun(0),
	m_bExit(0),
	m_pStatus(NULL),
	m_iNumOfOutPins(1) //default
{
	m_iIndexInRunner = ( int ) g_pRunner.GetCount() + 1 ;
	g_pRunner.Add( this ) ;  
	m_GadgetName.Format( "GenericScriptRunner%d" , g_pRunner.GetCount() ) ;
	memset ( &m_Interp , 0 , sizeof(m_Interp) ) ;
	m_pInput = new CInputConnector(transparent , fn_ScriptIndexedReceive , this ) ;
	m_pControl = new CDuplexConnector(this, text, text);

	for ( int iOutputIterator = 0 ; iOutputIterator < m_iNumOfOutPins ; iOutputIterator++ )
	{
		m_Outputs.Add(new COutputConnector(transparent));        
	}
	m_pInputQueue = new (FXStaticQueue<CDataFrame*>)(30) ;
	m_evHasData = CreateEvent( NULL , FALSE , FALSE, FALSE ) ;
	FXRegistry Reg( "TheFileX\\SHStudio") ; 
	m_ScriptPath = Reg.GetRegiString("Paths", "ScriptPath", "C:\\Graph\\Script.cpp" ); 
	m_EmbedChDir = Reg.GetRegiString("Paths", "EmbedChPath", "C:\\Ch\\Toolkit\\Embedch");
  strcpy_s( (char*)m_EmbedChDirBuf , sizeof(m_EmbedChDirBuf) , ( LPCTSTR ) m_EmbedChDir ) ;
  strcpy_s( (char*)m_EmbedChLicenseBuf , 900 , "12345678" ) ;

	m_EOSSnoozeTime = Reg.GetRegiDouble("Const", "EOSSnoozeTime[ms]", 1000); // if EOS received, ignore all other EOS received with EOSSnoozeTime duration
	FXString FileName("");
	FileName.Format("\\extern\\lib\\chmt%d.dl", g_pRunner.GetCount() ); 
	// Shifriss 29.06.11: 
	// Problem: example: run several processes of SHStudio with several graphs each containing 1 Script Runner - works even though there is only one CHMT!
	// Solution: according to SoftIntegration - in each process, every Script Runner must have CHMT (so if there are 4 script runner, there must be 4 CHMTs) but several processes can use the same CHMT (so if there 
	// 4 Script Runners, one in each process, they all use the same CHMT).
	// Problem: example: if there are 5 CHMTs and you load a graph with 6 Script Runners, the 6th file will be created (by mechanism I implemented) and used - all 6 script runners will be initialized. 
	// if there are 5 CHMTs and you load a graph with 5 Script Runners and create another Script Runner (from gadget tree) - the 6th file will be created but the Script Runner is not initialized (CHMT will not be used)
	// Solution: according to SoftIntegration, Embedded Ch will obtain the total number of CHMTs at start-up. Start-up occurs only after you load a graph (that explains how the 6th file is created and used) but 
	// once the graph is loaded, CHMTs may be created but will not be used. 
	// * changed the number of CHMTs in the setups from 5 to 10
	// * If CHMT is duplicated and one of them is already in use [= total number of CHMTs was contained] - a pop-up window will appear asking the user to save the graph, close it and open it again. If none is in use - CHMT will be
	// duplicated and used without problem - like in loading a graph with 6 Script Runner while there are only 5 CHMTs.
	// Mutex were initially added to the code to avoid the simultaneous use of a CH resources. All mutexs were removed since, according to SoftIntegration the protection is already done within CH. Double checked
	// since several processes use the same CHMT.
	FileName.Insert(0, m_EmbedChDir);
	if (FILE * file = fopen(FileName, "r"))
	{
		fclose(file);
	}
	else
	{
		FXString ExistingFile = m_EmbedChDir;
		ExistingFile.Append("\\extern\\lib\\chmt1.dl");
		CopyFile(ExistingFile, FileName, TRUE);
		// check if one of the Script Runners is initialized - if it is, the total number of CHMT was obtained and the graph must be save, closed and reloaded. 
		// if not - CHMT file could be duplicated and used without need to restart.
		int AnyRunnerWasInitialized = 0 ; 
		for (int i=0; i< g_pRunner.GetCount(); i++)
		{
			if (GetMyRunner(i)->m_bNotInitialized == 0)
				AnyRunnerWasInitialized = 1; 
		}
		if (AnyRunnerWasInitialized)
		{
			FXString ErrHead("Script Runner Error");
			FXString ErrorContent("The number of Script Runners in current graph has exceeded the limitation. Please save the graph, close it and reload it. The maximum number of Script Runners will be adjusted so that current graph will function properly."); 
			MessageBox(NULL, ErrorContent, ErrHead, MB_ICONWARNING | MB_OK ); 
		}
	}
  m_SetupObject = new GenericScriptRunnerDlg( this , NULL ) ;

	chdl_function * Chdl1 = new chdl_function ;
	Chdl1->pFuncProto = "int TxtOut_toC ( int iOutPinIndex , char * pOut, int iInstIndex );"; 
	Chdl1->FuncCallBack = (ChFuncdl_t)TxtOut_toC_chdl; 
	m_Chdls.Add(Chdl1); 
	chdl_function * Chdl2 = new chdl_function ;
	Chdl2->pFuncProto = "int FrameOut_toC ( int iOutPinIndex , void * pFrame, int iInstIndex );"; 
	Chdl2->FuncCallBack = (ChFuncdl_t)FrameOut_toC_chdl; 
	m_Chdls.Add(Chdl2); 
}
void GenericScriptRunner::InitOutputs()
{
	DeleteOutputs();
	m_Lock.Lock();
	if ((m_Outputs.GetSize()!=0) || (m_iNumOfOutPins==0)) return;
	for (int i=0; i<m_iNumOfOutPins; i++)
		m_Outputs.Add(new COutputConnector(transparent));        
	m_Lock.Unlock();
}
int GenericScriptRunner::GetOutputsCount() 
{ 
	m_Lock.Lock();  
	int count = ( int ) m_Outputs.GetSize();
	m_Lock.Unlock();
	return count;
}
void GenericScriptRunner::DeleteOutputs()
{
	m_Lock.Lock();
	while (m_Outputs.GetSize())
	{
		COutputConnector* pOutput = (COutputConnector*)m_Outputs.GetAt(0);
		m_Outputs.RemoveAt(0);
		delete pOutput ;
	}
	m_Lock.Unlock();
}
void GenericScriptRunner::AreOutPutsReady()
{
	m_Lock.Lock();
	FXPtrArray pins;
  m_OutputsReady = false ;
	for (int i=0; i<m_Outputs.GetSize(); i++)
	{
		COutputConnector* pOutput = (COutputConnector*)m_Outputs.GetAt(i);
		int uu = pOutput->GetComplementary(pins); 
		if (uu)
    {
      m_OutputsReady = true ; 
      break ;
    }
	}
	m_Lock.Unlock();
}
// DoJob() Function Scheme: 
// if not initialized and the gadgets to which Script Runner is connected are ready to receive data packets -> initialize 
// while (!m_bExit)
// {
//   if event has data
//   {
//     - if not initialized and the gadgets to which Script Runner is connected are ready to receive data packets -> initialize 
//     - process the data frame (see explanation in ProcessData() function )
//   }
//   if not initialized and the gadgets to which Script Runner is connected are ready to receive data packets -> initialize
// }
// if (m_bExit)
// 	 call OnExit function in script and destroy ScriptRunner 
int GenericScriptRunner::DoJob()
{ 
#ifdef THIS_MODULENAME
#undef THIS_MODULENAME
#endif
#define THIS_MODULENAME m_GadgetName // this is a redefinition of THIS_MODULENAME: 
	// at first THIS_MODULENAME is set as GenericScriptGadget. after construction of the derived 
	// gadget, it is reset to be the name of the derived gadget. 
	DWORD dwWaitResult ;
	AreOutPutsReady();
	if ((m_bNotInitialized ) && (m_OutputsReady))  // initialize the ch interpreter 
	{
		InitInterpParsScriptAndCallInitFuncInScript();
	}
	while ( !m_bExit ) // No command from graph runner to exit 
	{    
		dwWaitResult = ::WaitForSingleObject(m_evHasData, 1000) ; 
		switch( dwWaitResult )
		{
		case WAIT_OBJECT_0:   // Has Data 
			{
				if ( m_bNotInitialized )              // initialize the ch interpreter 
				{
					AreOutPutsReady();
					if (m_OutputsReady)
					{
						InitInterpParsScriptAndCallInitFuncInScript();
					}
				}
				DWORD dwStatus = m_pStatus->GetStatus() ;
        switch ( dwStatus )
        {
        case CExecutionStatus::RUN :
          m_bRun = TRUE ;
          break ;
        case CExecutionStatus::EXIT:
        case CExecutionStatus::STOP:
          if ( m_bRun )
          {
            OnStop() ;
					m_bRun = FALSE ; 
          }
          continue ;
          break ;
        }
        if ( !m_bRun )
          continue ;
				if (!m_bNotInitialized)
				{
					//CDataFrame * pDataFrame = NULL ;
					CDataFrame * pPacket = NULL ;

					while ( m_pInputQueue->ItemsInQueue() )
					{            
						if (( m_CallOnStartFunc ) && ( m_bRun ))
						{
							OnStart();
						}
						if ( m_pInputQueue->GetQueueObject(pPacket) && pPacket )
						{
							switch (m_Mode)
							{
							case mode_reject:
								{
									pPacket->RELEASE(pPacket);

								}
								break;

							case mode_transmit:
								{
									for ( int i = 0 ; i < m_Outputs.GetSize() ; i++ )
									{
										CDataFrame* Temp = pPacket->Copy();
										COutputConnector* pOutput = (COutputConnector*)m_Outputs.GetAt(i);
                    if ( !pOutput->Put(Temp) )
                      Temp->RELEASE( Temp ) ;
									}
									pPacket->RELEASE(pPacket);
								}
								break;
							case mode_process:
								{
									double ts=GetHRTickCount();
									DoProcessing(pPacket); // process the data frame 
									AddCPUUsage(GetHRTickCount()-ts);		
									pPacket->RELEASE(pPacket);
								}
								break;
							}
						}
					}
        }  // !m_bNotInitialized
				break ;
      }  // Wait has data
		}
		if ( m_bNotInitialized )   // initialize the ch interpreter (for if you load a script after entering the !m_bExit loop)
		{
			AreOutPutsReady();
			if (m_OutputsReady)
				InitInterpParsScriptAndCallInitFuncInScript();
			}
		}
	if (m_bExit)       // in case graph runner changed m_bExit to be true or called shut down which changed m_bExit to true
	{
		CallExitFuncInScriptAndEndInterp(); // call OnExit function in script
		m_bRun = false ; 
		m_bExit = true ; 
		m_bNotInitialized = 1;
		return WR_EXIT ;
	}
	return WR_CONTINUE ;
}
// DoProcessing() Function Scheme: 
// if there are items in input queue
// {
//	  if (m_CallOnStartFunc) and (m_bRun) -> call OnStart() function (which calls OnInit function with "start" input in the script)
//	  if label "Sink" found in data packet -> get sink gadget input from data packet
//		if label "TVObjects" found in data packet 
//			if (EOS) -> set m_CallOnStopFunc to true
//			else -> convert the data frame to TVObejcts data structure
//		else
//			if (EOS) -> set m_CallOnStopFunc to true
//			convert the text data frame to text data structure 
//	  else 
//		if label "TVObjects" found in data packet 
//			if (EOS) -> set m_CallOnStopFunc to true
//			else -> convert the data frame to TVObejcts data structure
//		else
//			if (EOS) -> set m_CallOnStopFunc to true
//			convert the text data frame to text data structure 
//	  if exists TVObjects data struct and (m_bRun) -> call OnDataStruct function in script 
//	  if exists Text data struct and (m_bRun) -> call OnTextStruct function in script 
//	  if (m_CallOnStopFunc) and (m_bRun) -> call OnStop function (which calls OnInit function with "stop" input in the script
// }
CDataFrame* GenericScriptRunner::DoProcessing(CDataFrame* pPacket)
{

	FXString pTMPString ;
	int iRetVal = 0 ;
	if (Tvdb400_IsEOS(pPacket)) // for if the EOS is DataFrame and not TextFrame (for example if Script Runner is connected to Script Runner)
	{
		double CurrTime = GetHRTickCount();//get_current_time(); returns double milliseconds from start
		//CurrTime = CurrTime * 1000 ; // convert from sec to millisec 
		if ( CurrTime - m_LastEOSTime > m_EOSSnoozeTime ) // if EOS received, ignore all other EOS received with EOSSnoozeTime duration
		{
			m_CallOnStopFunc = 1 ; 
			m_LastEOSTime = CurrTime ; 
		}
	}			
	NodeToScript * pStructOut = NULL ;
	TxtToScript *pTxtOut = new TxtToScript;        
	CFramesIterator* pIterator =  pPacket->CreateFramesIterator( nulltype );
	if ( pIterator )
	{
		int iInputIndex ; 
		CDataFrame * pNext = pIterator->Next( "Sink" ) ; 
		if ( pNext )               
		{
			CQuantityFrame * pQuanFrame = (CQuantityFrame *) pNext ;  
			iInputIndex = pQuanFrame->_i;
			iRetVal = 0 ;
			CFramesIterator* pInnerIterator = pPacket->CreateFramesIterator( nulltype );
			if ( pInnerIterator )
			{
				CDataFrame * pInnerNext = pIterator->Next( "TVObject" ) ; 
				if ( pInnerNext )
				{
					if (Tvdb400_IsEOS(pNext)) 
					{
						double CurrTime =   GetHRTickCount();//get_current_time(); returns double milliseconds from start
						//CurrTime = CurrTime * 1000 ; // convert from sec to millisec 
						if ( CurrTime - m_LastEOSTime > m_EOSSnoozeTime ) // if EOS received, ignore all other EOS received with EOSSnoozeTime duration
						{
							m_CallOnStopFunc = 1 ; 
							m_LastEOSTime = CurrTime ; 
						}
					}
					else
						pStructOut = ConvertFrameToStruct( pPacket, pIterator, 0 ) ; 
				}
				else 
				{
					CTextFrame * pTextFrame = pPacket->GetTextFrame() ; 
					if ( pTextFrame )
					{
						if (Tvdb400_IsEOS(pTextFrame)) 
						{
							double CurrTime =  GetHRTickCount();//get_current_time(); returns double milliseconds from start
							//CurrTime = CurrTime * 1000 ; // convert from sec to millisec 
							if ( CurrTime - m_LastEOSTime > m_EOSSnoozeTime ) // if EOS received, ignore all other EOS received with EOSSnoozeTime duration
								m_CallOnStopFunc = 1 ; 
							m_LastEOSTime = CurrTime ; 
						}
						pTMPString = pTextFrame->GetString(); 
						pTxtOut->pTxtOut = pTMPString.GetBuffer() ;
						pTxtOut->GraphTime = pTextFrame->GetTime() ;						
					}
				}
				delete pInnerIterator ;
			}
		}
		else  
		{
			iInputIndex = 0 ; 
			if ( pIterator )
				delete pIterator ;
			pIterator = NULL; 
			pIterator =  pPacket->CreateFramesIterator( nulltype );
			if ( pIterator )
			{
				pNext = pIterator->Next( "TVObject" ) ;
				if ( pNext )               
				{
					if (Tvdb400_IsEOS(pNext)) 
					{
						double CurrTime =  GetHRTickCount();//get_current_time(); returns double milliseconds from start
						//CurrTime = CurrTime * 1000 ; // convert from sec to millisec 
						if ( CurrTime - m_LastEOSTime > m_EOSSnoozeTime ) // if EOS received, ignore all other EOS received with EOSSnoozeTime duration
							m_CallOnStopFunc = 1 ; 
						m_LastEOSTime = CurrTime ; 
					}
					else
						pStructOut = ConvertFrameToStruct( pPacket, pIterator, 0 ) ;
				}
				else   
				{
					CTextFrame * pTextFrame = pPacket->GetTextFrame() ; 
					if ( pTextFrame )
					{
						if (Tvdb400_IsEOS(pTextFrame)) 
						{
							double CurrTime =  GetHRTickCount();//get_current_time(); returns double milliseconds from start
							//CurrTime = CurrTime * 1000 ; // convert from sec to millisec 
							if ( CurrTime - m_LastEOSTime > m_EOSSnoozeTime ) // if EOS received, ignore all other EOS received with EOSSnoozeTime duration to prevent multiple calls to on stop function in script
								m_CallOnStopFunc = 1 ; 
							m_LastEOSTime = CurrTime ; 
						}
						pTMPString = pTextFrame->GetString();
						pTxtOut->pTxtOut = pTMPString.GetBuffer() ;
						pTxtOut->GraphTime = pTextFrame->GetTime() ;						
					}	
				}
			}
		}              
		if ( (pStructOut) && (m_bRun) )
		{
			if ( m_Interp )
			{
				iRetVal = 0 ;
				if ((m_fptrDataStructInput) && (m_bNotParsed == 0))
					CallOnDataStructInput_ChFuncInScript(iRetVal, iInputIndex, pStructOut );
			}
			else
				m_bNotInitialized = 1;
		}
		if ( (pTxtOut) && (m_bRun) )
		{
			if ( m_Interp )
			{
				iRetVal = 0 ;
				if ((m_fptrTxtStructInput) && (m_bNotParsed == 0))
					CallOnTxtStructInput_ChFuncInScript(iRetVal, iInputIndex, pTxtOut );
			}
			else
				m_bNotInitialized = 1;
		}
	}
	if (( m_CallOnStopFunc ) && ( !m_bRun ))
	{
		OnStop();
		m_CallOnStopFunc = 0 ; 
	}
	if ( pIterator )
		delete pIterator ;
	if (pTxtOut->pTxtOut)
		pTMPString.ReleaseBuffer() ;
	delete pTxtOut;
	if ( pStructOut )
		DestroyStruct(pStructOut);
	return pPacket;
}
NodeToScript * GenericScriptRunner::ConvertFrameToStruct( CDataFrame * pPacket, CFramesIterator * pIterator, int iInputIndex ) 
{
	int count=-1;
	if ( pIterator )
	{
		CDataFrame * pDataFrame = pIterator->Next(NULL) ;
		int iId = 0 ;
		LPCTSTR pName = "" ;
		datatype type = 0 ; 
		double dwTime = 0 ;
		if (pDataFrame)
		{
			int iId = pDataFrame->GetId() ;
			LPCTSTR pName = pDataFrame->GetLabel() ;
			datatype type = pDataFrame->GetDataType() ;
			double dwTime = pDataFrame->GetTime() ;
		}
		NodeToScript * m_pPrev = NULL;   
		CArray<NodeToScript*,NodeToScript*> TxtNodePointers;
		CArray<NodeToScript*,NodeToScript*> FigNodePointers;
		while ( pDataFrame )
		{
			count++;
			CFigureFrame * pFig = pDataFrame->GetFigureFrame(NULL) ;
			if ( pFig )
			{          
				NodeToScript * temp = new NodeToScript(FIGUREFRAME,  (char*)pFig->GetLabel(), m_pPrev ) ;
				m_pPrev = temp ;          
				temp->m_pPoints = new PointToScript[pFig->GetCount()] ;
				for ( int i = 0 ; i <= pFig->GetCount()-1 ; i++)
				{
					temp->m_pPoints[i].x = pFig->GetAt(i).x;
					temp->m_pPoints[i].y = pFig->GetAt(i).y;
					if (strstr(temp->m_pName, "ROI"))
						temp->m_pPoints[i].IsROI = 1 ;
					else
						temp->m_pPoints[i].IsROI = 0 ;
				}  
				temp->pOriginalFrame = pPacket ; 
				temp->pFrame = NULL ; 
				temp->m_nPoints = ( int ) pFig->GetCount() ;
				FigNodePointers.Add(temp);
				pDataFrame = pIterator->Next(NULL) ;
				continue ;
			}   
			CTextFrame * pText = pDataFrame->GetTextFrame( NULL ) ;
			if ( pText )
			{
				FXString Label = pText->GetLabel() ;
				NodeToScript * temp = new NodeToScript(TEXTFRAME, (char*)pText->GetLabel(), m_pPrev);
				m_pPrev = temp ;          
				temp->m_pValue =  pText->GetString().GetBuffer();
				temp->pOriginalFrame = pPacket ; 
				temp->pFrame = NULL ; 
 				if (strlen(temp->m_pValue) && (Label.GetLength() != 0 ))
 					TxtNodePointers.Add(temp);      
				pDataFrame = pIterator->Next(NULL) ;
				continue ;
			}
			CVideoFrame * pVid = pDataFrame->GetVideoFrame( NULL ) ;
			if ( pVid )
			{
				INT_PTR * pData ;
				if (!pVid->lpData)
					pData = (INT_PTR*)((DWORD)(pVid->lpBMIH) + pVid->lpBMIH->biSize);
				else
					pData = ( INT_PTR* )(pVid->lpData) ;
				pData[0] = SWAP4BYTES(pData[0]);
				pData[1] = SWAP4BYTES(pData[1]);
				pData[2] = SWAP4BYTES(pData[2]);
				pData[3] = SWAP4BYTES(pData[3]);
				pData[4] = SWAP4BYTES(pData[4]);

				for ( int i = 0  ; i < FigNodePointers.GetCount() ; i++ )
				{
					if (FigNodePointers[i]->pFrame == NULL )
					{
						FigNodePointers[i]->pFrame = pVid->GetVideoFrame(DEFAULT_LABEL);
						FigNodePointers[i]->CameraTime = pData[0]; 
						FigNodePointers[i]->CameraFrameNumber = pData[1]; 
						FigNodePointers[i]->CameraOutP = pData[2]; 
						FigNodePointers[i]->CameraInP = pData[3]; 
						FigNodePointers[i]->CameraX = pData[4] >> 16 ;           
						FigNodePointers[i]->CameraY = pData[4] & 0Xffff ; 
					}
				}

				for ( int i = 0  ; i < TxtNodePointers.GetCount() ; i++ )
				{
					if ( TxtNodePointers[i]->pFrame == NULL )
					{
						TxtNodePointers[i]->pFrame = pVid->GetVideoFrame(DEFAULT_LABEL);
						TxtNodePointers[i]->CameraTime = pData[0]; 
						TxtNodePointers[i]->CameraFrameNumber = pData[1]; 
						TxtNodePointers[i]->CameraOutP = pData[2]; 
						TxtNodePointers[i]->CameraInP = pData[3]; 
						TxtNodePointers[i]->CameraX = pData[4] >> 16 ;           
						TxtNodePointers[i]->CameraY = pData[4] & 0Xffff ; 
					}
				}
			}         
			pDataFrame = pIterator->Next(NULL) ;
		}
		if ( m_pPrev )
		{    
			char * pInfoName = "Info"; 
			NodeToScript * temp = new NodeToScript(INFOFRAME, pInfoName, m_pPrev ) ; 
			temp->m_iId = iId;
			temp->m_dFrameTime = dwTime;  
			m_dFrameTime = dwTime;
			temp->m_dBeforeScriptTime = GetPresentTime();      
			m_dBeforeScriptTime = GetPresentTime();
			temp->m_iInputIndex = iInputIndex;
			temp->m_iNfigs = (int)FigNodePointers.GetCount();
			temp->m_iNTexts = ( int ) TxtNodePointers.GetCount();

			if (temp->m_iNfigs > 0 )
			{
				NodeToScript ** pFigsPtrs = new NodeToScript* [temp->m_iNfigs];
				memcpy( pFigsPtrs , FigNodePointers.GetData() , temp->m_iNfigs * sizeof(NodeToScript*) ) ;
				temp->pFigs = pFigsPtrs ;                                             
			}
			else
				temp->pFigs = NULL ;
			if ( temp->m_iNTexts > 0 )
			{
				Pointers * pTxt = new Pointers[temp->m_iNTexts];
				for ( int i = 0 ; i < temp->m_iNTexts ; i++)
					pTxt[i].pNextNode = TxtNodePointers.GetAt(i);
				temp->pTxts = (NodeToScript **) pTxt ;
			}
			else
				temp->pTxts = NULL ;
			m_pPrev = temp ;                 
			return m_pPrev ;       
		}
	}
	return NULL ;
}
int GenericScriptRunner::DestroyStruct(  NodeToScript * pOut ) 
{
	if (pOut->m_iNfigs>0) 
		delete[] pOut->pFigs; 
	if (pOut->m_iNTexts>0)
		delete[] pOut->pTxts; 
	if (pOut->m_pPrev)
	{
		NodeToScript* pCurrNode = pOut->m_pPrev; 
		while ((pCurrNode) && (pCurrNode->m_iType>0))
		{
			int Type = pCurrNode->m_iType;
			NodeToScript* pTemp = pCurrNode->m_pPrev ; 
			if (Type == FIGUREFRAME)
			{
				delete pCurrNode->m_pPoints; 
				delete pCurrNode ; 
			}  
			if (Type == TEXTFRAME)
			{
				delete pCurrNode ; 
			}        
			if (Type == VIDEOFRAME)
			{
				delete pCurrNode ; 
			}                 
			pCurrNode = pTemp ; 
		}
	}
	delete pOut ;
	return 1 ; 
}
void GenericScriptRunner::OnStart()
{
	if ((m_bNotInitialized == 0) && (m_bNotParsed == 0))
	{
		CallStartFuncInScript();
		m_CallOnStartFunc = 0 ; 
    TRACE("\r\nGenericScriptRunner::OnStart() finished") ;
	}
}
void GenericScriptRunner::OnStop()
{
	CallStopFuncInScript();
	m_CallOnStartFunc = 1;
	CDataFrame* pFrame=CDataFrame::Create();
	Tvdb400_SetEOS(pFrame);
	pFrame->ChangeId(pFrame->GetId());
	pFrame->SetTime(pFrame->GetTime());
	m_Lock.Lock();  // now we need to deliver the EOS message to the rest of the graph -> send it to all outputs
	for ( int i = 0 ; i < m_Outputs.GetSize() ; i++ )
	{
		CDataFrame* Temp = pFrame->Copy();
		COutputConnector* pOutput = (COutputConnector*)m_Outputs.GetAt(i);
		if ( !pOutput->Put(Temp) )
      Temp->RELEASE( Temp ) ;
	}
	pFrame->Release(pFrame);
	m_Lock.Unlock();
  m_bNotInitialized = true ;
  while ( m_pInputQueue->ItemsInQueue() )
  {            
    if ( m_pInputQueue->GetQueueObject(pFrame) && pFrame )
      pFrame->RELEASE(pFrame) ;
  }

  TRACE("\r\nGenericScriptRunner::OnStop() called") ;
}

bool GenericScriptRunner::ScanSettings(FXString& text)
{
	text="calldialog(true)";
	return true;
}

int GenericScriptRunner::InitInterpParsScriptAndCallInitFuncInScript()
{
	m_CallOnStartFunc = 1 ; 	
	int RetValue = 0 ; 
	int iRetVal = 0;
	RetValue = InitInterpreter();
	if (RetValue == 1) 
	{
		RetValue = ParsScript();
		if (RetValue == 1 )
		{
			m_bNotParsed = 0 ; 
			RetValue = CallInitFuncInScript();
			if (RetValue == 1)
			{
				m_bNotInitialized = 0;
				return 1 ;
			}
			else
				return 0 ; 
		}
		else
			return 0 ; 			
	}
	else 
	{
		m_bNotParsed = 1; 
		return 0;
	}
}
int GenericScriptRunner::InitInterpreter()
{
	CallExitFuncInScriptAndEndInterp();
	HANDLE hOldStdErr = GetStdHandle( STD_ERROR_HANDLE ) ; // save default error output
	ChOptions_t option;
	option.shelltype = CH_REGULARCH;
	option.chhome = (char*)m_EmbedChDirBuf ; 
  option.licensestr = ( char* ) m_EmbedChLicenseBuf ;
	int Stat = NULL ;
	// creating file in script's folder comprised of gadget name and the ending _Initialization.err
	FXString ErrPath = m_ScriptPath ; 
	int DeleteFromHere = ( int ) ErrPath.ReverseFind(_T('\\'));
	ErrPath.Delete(DeleteFromHere, ErrPath.GetLength());
	ErrPath.Append("\\");
	ErrPath.Append(m_GadgetName) ; 
	ErrPath.Append("_Initialization.err");  
	remove(ErrPath); 	// if file with such name exists - remove it 
	FILE *fp = NULL;  
	fp = freopen(ErrPath, "w", stderr);  // copy the errors to the file 
	// shifriss 14.06.11: Ch_Initialize changes environmental variables. the following code saves current environmental variables, calls Ch_Initialize and then sets them to original value.
	FXString PathBefore = getenv ("PATH");
	FXString Path ("PATH=");
	Path.Append(PathBefore);
	FXString LibBefore = getenv("LIB");
	FXString Lib ("LIB=");
	Lib.Append(LibBefore);
	FXString IncBefore = getenv("INCLUDE");
	FXString Inc ("INCLUDE=");
	Inc.Append(IncBefore);
	Stat = Ch_Initialize(&m_Interp, &option);   // error handling in ch requires open file for errors, call ch func and close file       
	putenv(Path); 
	putenv(Lib);
	putenv(Inc);
	if (fp)
		fclose (fp); 
	if ( Stat ) // there were errors in initializing CH
	{
		char buffer[MAX_ERR_MSG_LEN]= {0};
		fp = fopen(ErrPath, "r");
		int iNBytes = ( int ) fread(buffer, 1, MAX_ERR_MSG_LEN-1, fp); // copy the first 1000 chars of the errors from the file to the buffer 
		fclose(fp); // close the file 
		buffer[iNBytes] = 0 ;
		FXString ErrHead = "Gadget "; // create headline for the message box 
		ErrHead.Append(m_GadgetName);          
		ErrHead.Append(" Initialization Error");
		if (strlen(buffer)+1 == sizeof(buffer) )
		{
			FXString TooManyErr = " !!!! There are more initialization errors. Fix these and retry or find full description in ";
			TooManyErr.Append(ErrPath);
			FXString ErrorContent; 
			ErrorContent = buffer;
			ErrorContent.Append( TooManyErr ); 
			MessageBox(NULL, ErrorContent, ErrHead, MB_ICONWARNING | MB_OK ); 
			CallExitFuncInScriptAndEndInterp();
			SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
			return WR_EXIT ;
		}		
		else
		{
			MessageBox(NULL, buffer, ErrHead, MB_ICONWARNING | MB_OK ); 
			CallExitFuncInScriptAndEndInterp();
			SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
			return WR_EXIT ;
		}
	}	
	else  //  initialization works 
	{   
		remove(ErrPath); 	// if file with such name exists - remove it 
		TRACE(">>>>>> CH Interpreter initialized \n" );		
		SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
		return WR_CONTINUE ;
	}   
}
int GenericScriptRunner::ParsScript() 
{
	HANDLE hOldStdErr = GetStdHandle( STD_ERROR_HANDLE ) ;  // save default error output
	int Stat = NULL ;
	// does script file exist???  
	fstream fin;
	fin.open(m_ScriptPath,ios::in);
	if (fin.is_open())   // script file exists
	{
    fin.close() ;
		for ( int i = 0 ; i < m_Chdls.GetSize() ; i++ )
		{
			Stat = Ch_DeclareFunc( m_Interp, ((chdl_function*)( m_Chdls.GetAt(i)))->pFuncProto , ((chdl_function*)( m_Chdls.GetAt(i)))->FuncCallBack);
			if ( Stat ) // function declaration failed 
			{			
				// create file in script's folder comprised of gadget name and the ending _Function_Declaration.err
				FXString ErrPath = m_ScriptPath;
				int DeleteFromHere = ( int ) ErrPath.ReverseFind(_T('\\'));
				ErrPath.Delete(DeleteFromHere, ErrPath.GetLength());
				ErrPath.Append("\\");
				ErrPath.Append(m_GadgetName) ; 
				ErrPath.Append("_Function_Declaration.err"); 
				remove(ErrPath); 	// if file with such name exists - remove it 
				FXString ErrContent ;
				FXString ErrHead ;
				ErrContent.Format( "Function: %s \n" , ((chdl_function*)( m_Chdls.GetAt(i)))->pFuncProto ) ;       
				ErrHead.Append("Gadget ");
				ErrHead.Append(m_GadgetName);
				ErrHead.Append(" Function Declaration Error");
				if (ErrContent.GetLength() < MAX_ERR_MSG_LEN )
				{
					MessageBox(NULL, ErrContent, ErrHead, MB_ICONWARNING | MB_OK ); 
					CallExitFuncInScriptAndEndInterp();
					SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
					return WR_EXIT ;
				}
				else
				{
					fstream ifin;
					ifin.open(ErrPath,ios::out);
					ifin.write(ErrContent.GetBuffer(), ErrContent.GetLength());
					ifin.close();
					FXString TooManyErr = " !!!! There are more declaration errors. Fix these and retry or find full description in ";
					TooManyErr.Append(ErrPath);
					ErrContent.Delete(MAX_ERR_MSG_LEN, ErrContent.GetLength()-MAX_ERR_MSG_LEN);
					ErrContent.Append(TooManyErr);
					MessageBox(NULL, ErrContent, ErrHead, MB_ICONWARNING | MB_OK ); 
					CallExitFuncInScriptAndEndInterp();
					SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
					return WR_EXIT ;
				}
			}
		}	// function declaration succeeded	
		Stat = Ch_InitGlobalVar(m_Interp , 1) ;
		if ( Stat ) // Init Global Var failed 
		{			
			FXString ErrContent ;
			FXString ErrHead ;
			ErrContent.Append("Global variable: m_Interp \n ") ;     
			ErrHead.Append("Gadget ");
			ErrHead.Append(m_GadgetName);
			ErrHead.Append(" Global Variable Declaration Error");
			MessageBox(NULL, ErrContent, ErrHead, MB_ICONWARNING | MB_OK ); 
			CallExitFuncInScriptAndEndInterp();
			SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
			return WR_EXIT ;
		}
		// creating file in script's folder comprised of script name and the ending _Parse.err
		FXString ErrPath = m_ScriptPath; 
		ErrPath.Replace(".cpp", "_Parse.err");
		remove(ErrPath); // if file with such name exists - remove it 
//     FXString NameForParsing ;
//     NameForParsing.Format( "LastParseScript%d.cpp" , m_iIndexInRunner ) ;
//     FXString TempName( m_ScriptPath ) ;
//     int iSlashPos = TempName.ReverseFind( '\\' ) ;
//     if ( iSlashPos >= 0 )
//     {
//       TempName = TempName.Left( iSlashPos + 1 ) ;
//       TempName += NameForParsing ;
//     }
//     else
//       TempName = NameForParsing ;
//     CopyFile( m_ScriptPath , TempName , FALSE ) ;  // copy to temporary file (replace if exists )
//     char *argvv[] = {(char*)(LPCTSTR) TempName, NULL} ;
    char *argvv[] = {(char*)(LPCTSTR) m_ScriptPath, NULL} ;
		ChFile_t fildes_stderr;
		fildes_stderr = Ch_Reopen(m_Interp, ErrPath, "w", STDERR_FILENO);            
		Stat = Ch_ParseScript(m_Interp , argvv) ;	// error handling in ch requires open file for errors, call ch func and close file
		if ( Stat ) // Pars failed 
		{			
			fildes_stderr = Ch_Close(m_Interp, fildes_stderr);
      CallExitFuncInScriptAndEndInterp();
      SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ;
			displayParsContentsInFile(ErrPath.GetBuffer());
			return WR_EXIT ;			
		} 
    else
    {
      SENDINFO_1("OK parsing script %s", m_ScriptPath);
    }
		fildes_stderr = Ch_Close(m_Interp, fildes_stderr);
		remove( ErrPath ); 
		Stat = Ch_SetVar(m_Interp, "__SCRIPTRUNNER_INDEX__", CH_INTTYPE, m_iIndexInRunner);
		if ( Stat ) // gadget index declaration failed 
		{			
			FXString ErrContent ;
			FXString ErrHead ;
			ErrContent.Append("Global variable: __SCRIPTRUNNER_INDEX__\n ") ;     
			ErrHead.Append("Gadget ");
			ErrHead.Append(m_GadgetName);
			ErrHead.Append(" Value Assignment Error");
			MessageBox(NULL, ErrContent, ErrHead, MB_ICONWARNING | MB_OK ); 
			CallExitFuncInScriptAndEndInterp();
			SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
			return WR_EXIT ;
		}
	}
	else  // script file doesnt exists 
	{
		SENDINFO_1("There is no script in path %s", m_ScriptPath);
		CallExitFuncInScriptAndEndInterp();
		SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
		return WR_EXIT ;
	}	
	m_fptrInit = Ch_SymbolAddrByName(m_Interp, "OnInit_Ch"); // obtain the address of function OnInit_Ch
	m_fptrDataStructInput = Ch_SymbolAddrByName(m_Interp, "OnDataStructInput_Ch"); // obtain the address of function OnDataStructInput_Ch
	m_fptrTxtStructInput = Ch_SymbolAddrByName(m_Interp, "OnTxtStructInput_Ch"); // obtain the address of the function OnTxtStructInput_Ch
	m_fptrExit = Ch_SymbolAddrByName(m_Interp, "OnExit_Ch"); // obtain the address of function OnExit_Ch
	SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
	TRACE(">>>>>> Script %s parsed successfully  \n", m_ScriptPath );		
	return WR_CONTINUE ;
}
int GenericScriptRunner::CallOnInit_ChFuncInScript(int iRetVal, char InBuf[200], char OutBuf[200] ) // general call to OnInit_Ch function in the script with InBuf input
{
	int Stat = NULL;    
	fstream fin;
	ChFile_t fildes_stderr;
	char OnInitErrPath[300] ;
	HANDLE hOldStdErr = GetStdHandle( STD_ERROR_HANDLE ) ;
	char *argvv[] = {(char*)(LPCTSTR) m_ScriptPath, NULL} ;
	strcpy( OnInitErrPath, m_ScriptPath );
	char* DeleteFromHere =  strrchr(OnInitErrPath, '.') ;
	if (DeleteFromHere)
		*(DeleteFromHere) = 0 ;      
	strcat( OnInitErrPath, "_OnInit.err" );
	// does error file exist ???
	remove(OnInitErrPath);
	fildes_stderr = Ch_Reopen(m_Interp, OnInitErrPath, "w", STDERR_FILENO);            
	Stat = Ch_CallFuncByAddr(m_Interp, m_fptrInit, &iRetVal, InBuf, OutBuf ); /* call Ch function func() */
	if ( Stat )
	{
		char* FunctionName = "OnInit_Ch" ;
		fildes_stderr = Ch_Close(m_Interp, fildes_stderr);
		displayRunTimeContentsInFile(OnInitErrPath, FunctionName);
	}
	else 
	{

		fildes_stderr = Ch_Close(m_Interp, fildes_stderr);  
		remove(OnInitErrPath);
		SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
		if (m_bNotInitialized = 1)
			m_bNotInitialized = 0; 
		iRetVal = 1 ; 
		return WR_CONTINUE ;
	}  
	SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
	return WR_EXIT ;
}
int GenericScriptRunner::CallInitFuncInScript()
{
	if (m_Interp != NULL)
	{
		char InBuf[200] ;
		char OutBuf[200] ;
		int iRetVal = 0;
		iRetVal = 0 ;
		int RetVal = 0 ; 
		*InBuf = NULL ;
		*OutBuf = NULL ;
		strcpy( InBuf , "INIT") ;
		OutBuf[0] = 0 ;
		if ((m_fptrInit) && (m_bNotParsed == 0))
		{
			RetVal = CallOnInit_ChFuncInScript(iRetVal, InBuf, OutBuf );
			if (RetVal == 1)
				return 1 ; 
			else
				return 0 ; 
		}
		else
			return 0 ; 
	}
	else
		return 0 ; 
}
int GenericScriptRunner::CallStartFuncInScript()
{
	if (m_Interp != NULL)
	{
		char InBuf[200] ;
		char OutBuf[200] ;
		int iRetVal = 0;
		iRetVal = 0 ;
		int RetVal = 0 ; 
		*InBuf = NULL ;
		*OutBuf = NULL ;
		strcpy( InBuf , "START") ;
		OutBuf[0] = 0 ;
		if ((m_fptrInit) && (m_bNotParsed == 0))
		{
			RetVal = CallOnInit_ChFuncInScript(iRetVal, InBuf, OutBuf );
			if (RetVal == 1)
				return 1 ; 
			else
				return 0 ; 
		}
		else
			return 0 ; 
	}
	else
		return 0 ; 
}
int GenericScriptRunner::CallStopFuncInScript()
{
	if ((m_Interp != NULL) && (m_fptrInit))
	{
		char InBuf[200] ;
		char OutBuf[200] ;
		int iRetVal = 0;
		iRetVal = 0 ;
		int RetVal = 0 ; 
		*InBuf = NULL ;
		*OutBuf = NULL ;
		strcpy( InBuf , "STOP") ;
		OutBuf[0] = 0 ;
		if ((m_fptrInit) && (m_bNotParsed == 0))
		{
			RetVal = CallOnInit_ChFuncInScript(iRetVal, InBuf, OutBuf );
			if (RetVal == 1)
				return 1 ; 
			else
				return 0 ; 
		}
		else
			return 0 ; 
	}
	else
		return 0 ; 
}
int GenericScriptRunner::CallOnDataStructInput_ChFuncInScript(
  int iRetVal, int iInputIndex, NodeToScript* pOut )
{
	if ( (!pOut->m_iNTexts && !pOut->m_iNfigs )
    || ( pOut->m_iNTexts && !pOut->pTxts ) 
    || ( pOut->m_iNfigs && !pOut->pFigs ) )
    return WR_CONTINUE ;
	int Stat = NULL;    
	fstream fin;
	ChFile_t fildes_stderr;
	char OnInputErrPath[300] ;
	HANDLE hOldStdErr = GetStdHandle( STD_ERROR_HANDLE ) ;
	char *argvv[] = {(char*)(LPCTSTR) m_ScriptPath, NULL} ;
	strcpy( OnInputErrPath, m_ScriptPath );
	char* DeleteFromHere =  strrchr(OnInputErrPath, '.') ;
	*(DeleteFromHere) = 0 ;      
	strcat( OnInputErrPath, "_OnInput.err" );
	// does error file exist ???
	fildes_stderr = Ch_Reopen(m_Interp, OnInputErrPath, "w", STDERR_FILENO);            
	Stat = Ch_CallFuncByAddr(m_Interp, m_fptrDataStructInput, &iRetVal, iInputIndex, pOut ); /* call Ch function func() */ 
	if ( Stat )
	{
		char * FunctionName = "OnDataStructInput_Ch";
		fildes_stderr = Ch_Close(m_Interp, fildes_stderr);
		displayRunTimeContentsInFile(OnInputErrPath, FunctionName);
	}
	else 
	{
		fildes_stderr = Ch_Close(m_Interp, fildes_stderr);  
		remove(OnInputErrPath);
	}
	SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
	return WR_CONTINUE ;
}
int GenericScriptRunner::CallOnTxtStructInput_ChFuncInScript(int iRetVal, int iInputIndex, TxtToScript * pOut )
{
	int Stat = NULL;    
	fstream fin;
	ChFile_t fildes_stderr;
	char OnInputErrPath[300] ;
	HANDLE hOldStdErr = GetStdHandle( STD_ERROR_HANDLE ) ;
	char *argvv[] = {(char*)(LPCTSTR) m_ScriptPath, NULL} ;
	strcpy( OnInputErrPath, m_ScriptPath );
	char* DeleteFromHere =  strrchr(OnInputErrPath, '.') ;
	if (DeleteFromHere)
		*(DeleteFromHere) = 0 ;      
	strcat( OnInputErrPath, "_OnInput.err" );
	// does error file exist ???
	remove(OnInputErrPath);
	fildes_stderr = Ch_Reopen(m_Interp, OnInputErrPath, "w", STDERR_FILENO);            
	Stat = Ch_CallFuncByAddr(m_Interp, m_fptrTxtStructInput, &iRetVal, iInputIndex, pOut ); /* call Ch function func() */ 
	if ( Stat )
	{
		char * FunctionName = "OnTxtStructInput_Ch";
		fildes_stderr = Ch_Close(m_Interp, fildes_stderr);
		displayRunTimeContentsInFile(OnInputErrPath, FunctionName);
	}
	else 
	{
		fildes_stderr = Ch_Close(m_Interp, fildes_stderr);  
		remove(OnInputErrPath);
	}
	SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
	return WR_CONTINUE ;
}
int GenericScriptRunner::CallExitFuncInScriptAndEndInterp()
{
	int RetValue = 0 ; 
	int iRetVal = 0;
	if ((m_fptrExit) && (m_bNotParsed == 0))
		CallExitFuncInScript();
	if (EndInterpreter())
	{
		m_bNotInitialized = 1;
		m_bNotParsed = 1; 
		return 1 ;
	}	
	else
		return 0;
}
int GenericScriptRunner::CallExitFuncInScript()
{
	if ((m_Interp != NULL) && (m_fptrExit) && (m_bNotParsed == 0))
	{
		char InBuf[200] ;
		char OutBuf[200] ;
		int iRetVal = 0;
		iRetVal = 0 ;
		int RetVal = 0 ; 
		*InBuf = NULL ;
		*OutBuf = NULL ;
		strcpy( InBuf , "EXIT") ;
		OutBuf[0] = 0 ;
		if (m_fptrExit)
		{
			RetVal = CallOnExit_ChFuncInScript(iRetVal, InBuf, OutBuf );
			if (RetVal == 1)
				return 1 ; 
			else
				return 0 ; 
		}
		else
			return 0 ; 
	}
	else
		return 0 ; 
}
int GenericScriptRunner::CallOnExit_ChFuncInScript(int iRetVal, char* InBuf, char* OutBuf )
{
	strcpy( InBuf , "EXIT") ;
	OutBuf[0] = 0 ;
	int Stat = 0 ;
	fstream fin;
	ChFile_t fildes_stderr;
	char OnExitErrPath[300] ;
	HANDLE hOldStdErr = GetStdHandle( STD_ERROR_HANDLE ) ;
	char *argvv[] = {(char*)(LPCTSTR) m_ScriptPath, NULL} ;
	strcpy( OnExitErrPath, m_ScriptPath );
	char* DeleteFromHere =  strrchr(OnExitErrPath, '.') ;
	if (DeleteFromHere)
		*(DeleteFromHere) = 0 ;      
	strcat( OnExitErrPath, "_OnExit.err" );
	// does error file exist ???
	remove(OnExitErrPath);
	fildes_stderr = Ch_Reopen(m_Interp, OnExitErrPath, "w", STDERR_FILENO);            
	Stat = Ch_CallFuncByAddr(m_Interp, m_fptrExit, &iRetVal, InBuf, OutBuf ); /* call Ch function func() */
	if ( Stat )
	{
		fildes_stderr = Ch_Close(m_Interp, fildes_stderr);
		char * FunctionName = "OnExit_Ch";
		displayRunTimeContentsInFile(OnExitErrPath, FunctionName);
	}
	else 
	{
		fildes_stderr = Ch_Close(m_Interp, fildes_stderr);  
		remove(OnExitErrPath);
	}
	SetStdHandle( STD_ERROR_HANDLE , hOldStdErr ) ; 
	if (m_bCallOnExitFunc = true)
		m_bCallOnExitFunc = false; 
	return WR_EXIT ;
}
int GenericScriptRunner::EndInterpreter()  
{
	int RetValue = 0 ; 
	if ( m_Interp )
	{
		// shifriss 14.06.11: Ch_End changes environmental variables. the following code saves current environmental variables, calls Ch_End and then sets them to original value.
		FXString PathBefore = getenv ("PATH");
		FXString Path ("PATH="); 
		Path.Append(PathBefore);
		FXString LibBefore = getenv("LIB");
		FXString Lib ("LIB=");
		Lib.Append(LibBefore);
		FXString IncBefore = getenv("LIB");
		FXString Inc ("INCLUDE=");
		Inc.Append(IncBefore);
		RetValue = Ch_End(m_Interp);
		putenv(Path); 
		putenv(Lib);
		putenv(Inc);
		if (RetValue == 0 ) 
		{
			TRACE(">>>>>> CH Interpreter destroyed \n" );		
			m_Interp = NULL;
			return 1 ; 
		}
		else
		{
			FXString ErrHead = "Gadget "; 
			ErrHead.Append(m_GadgetName);          
			ErrHead.Append(" Ch_End Error");
			FXString ErrContent ;
			ErrContent.Format( "Ch_End Error %d" , RetValue ) ;
			MessageBox(NULL, ErrContent, ErrHead, MB_ICONWARNING | MB_OK ); /*for GUI in Windows */   
			return 0 ; 
		}
	}
	else
		return 1 ; 
}
// displayParsContentsInFile() / displayRunTimeContentsInFile() function Scheme: 
// if the error massages are bigger than BUFSIZ - some will be written to error pop up window in addition to a remark that there are more errors and the user should fix 
// those and retry or refer to the file (whose path is given) which contains all the errors
// else - all the errors will be written to error pop up window
void GenericScriptRunner::displayParsContentsInFile(char *filename) 
{
	FILE *stream;
	char buffer[1000] = {0};
	FXString HeadLine("Script ");  
	HeadLine.Append(m_ScriptPath);
	HeadLine.Append (" Parse Error");
	int leangth; 
	FXString TooManyErr(" !!!! There are more parse errors. Fix these and retry or read all in a file named "); 
	TooManyErr.Append(filename);
	stream = fopen(filename, "r");
	if (stream != NULL)
	{
		leangth = ( int ) fread(buffer, 1, sizeof(buffer) , stream);
		if (leangth > 0 )
		{
			if (sizeof(buffer) < TooManyErr.GetLength())
			{
				MessageBox(NULL, TooManyErr, HeadLine, MB_ICONWARNING | MB_OK ); 
			}
			else if ((leangth == sizeof(buffer) ) &&  (sizeof(buffer) >= TooManyErr.GetLength()))
			{
				char bufferCopy[sizeof(buffer)] = {0}; 

				strncpy (bufferCopy, buffer, sizeof(buffer) - TooManyErr.GetLength()-2 );
				strcat(bufferCopy, TooManyErr);
				strcat(bufferCopy, "");
				MessageBox(NULL, bufferCopy, HeadLine, MB_ICONWARNING | MB_OK ); 
			}
			else
			{
				MessageBox(NULL, buffer, HeadLine, MB_ICONWARNING | MB_OK ); 
			}
			m_bNotInitialized = 1;
		}
	}
}
void GenericScriptRunner::displayRunTimeContentsInFile(char *filename, char* FunctionName) 
{
	FILE *stream;
	char buffer[1000] = {0};
	FXString HeadLine ("Script ");
	HeadLine.Append(m_ScriptPath);
	HeadLine.Append(" RunTime Error In Function ");
	HeadLine.Append(FunctionName);
	int leangth = 0 ; 
	FXString TooManyErr(" !!!! There are more run time errors. Fix these and retry or read all in a file named "); 
	TooManyErr.Append(filename);
	stream = fopen(filename, "r");    
	if (stream != NULL)
	{
		leangth = ( int ) fread(buffer, 1, sizeof(buffer)-1 , stream);
		if ( leangth > 0 )
		{
			if (sizeof(buffer) < TooManyErr.GetLength())
			{                 
				MessageBox(NULL, TooManyErr, HeadLine, MB_ICONWARNING | MB_OK ); 
			}
			else if ((leangth == sizeof(buffer) ) &&  (sizeof(buffer) >= TooManyErr.GetLength()))
			{
				char bufferCopy[sizeof(buffer)] = {0}; 
				strncpy (bufferCopy, buffer, sizeof(buffer) - TooManyErr.GetLength() );
				strcat(bufferCopy, TooManyErr);
				MessageBox(NULL, bufferCopy, HeadLine, MB_ICONWARNING | MB_OK ); 
			}
			else
			{       
				MessageBox(NULL, buffer, HeadLine, MB_ICONWARNING | MB_OK ); 
			}
		}
	}
}
void GenericScriptRunner::ShutDown()
{
	m_bExit = true;
	AfxGetApp()->WriteProfileString("ScriptRunner", "Path", m_ScriptPath);
	CGadget::ShutDown();

	delete m_pInput;
  m_pInput = NULL ;
	delete m_pControl;
	m_pControl = NULL;
  DeleteOutputs(); 

	while ( m_pInputQueue->ItemsInQueue() )
	{
		CDataFrame * pDataFrame = NULL ;
		CDataFrame * pPacket = NULL ;
		if ( m_pInputQueue->GetQueueObject( pPacket ) && pPacket )
		{ 
			pPacket->Release( pPacket ) ;
		}
	}
	if (m_pInputQueue) 
	{
		delete m_pInputQueue ;
		m_pInputQueue=NULL; 
	}
	FxReleaseHandle(m_evHasData); 
	for ( int i = 0 ; i < m_Chdls.GetCount() ; i++ )
	{    
		if (m_Chdls.GetAt(i) != NULL)
		{
			chdl_function * p = (chdl_function *) m_Chdls.GetAt(i) ;
			delete p ;       
		}
	}
	m_Chdls.RemoveAll();
	if ( m_pStatus )
	{
		m_pStatus->Release() ; 
    m_pStatus = NULL ;
	}

	g_pRunner.ElementAt(m_iIndexInRunner-1 ) = NULL ; 
	int DeleteRunnerArray = 1 ;
	for ( int i = 0 ; i < g_pRunner.GetSize() ; i++ )
	{
		if (g_pRunner.ElementAt(i) != NULL)
			DeleteRunnerArray = 0;
	}
	if (DeleteRunnerArray)
	{
		g_pRunner.RemoveAll();
		DeleteRunnerArray = 0;
	}
}
