// WatchDogGadget.h.h : Implementation of the WatchDogGadget class


#include "StdAfx.h"
#include <Windows.h>
#include "WatchDogGadget.h"
#include <helpers\FramesHelper.h>

// USER_FILTER_RUNTIME_GADGET(WatchDogGadget,"Generic");	//	Mandatory
IMPLEMENT_RUNTIME_GADGET_EX( WatchDogGadget , CFilterGadget , LINEAGE_GENERIC , TVDB400_PLUGIN_NAME );

WatchDogGadget::WatchDogGadget()
{
  m_ihowMuchDelay_ms = 1000;
  m_iPeriod_ms = 0 ;
  m_iOutFramesCount = 0;
  m_bFlagTimeOut = false;
  m_hTimer = NULL;
  m_stringMessage = NULL;  // tringMessage use for DoTextFrame() messages

  //	Mandatory
  init();
}
void WatchDogGadget::InitExecutionStatus( CExecutionStatus* Status )
{
  m_pStatus = CExecutionStatus::Create( Status );
}

WatchDogGadget::~WatchDogGadget()
{
  DeleteTimerQueueFromWatchDogGadget() ;
}

void WatchDogGadget::DoTextFrame()
{
  CTextFrame * ViewText = CTextFrame::Create( this->m_stringMessage );
  if ( ViewText )
  {
    ViewText->ChangeId( m_iOutFramesCount++ ) ;
    PutFrame( m_pOutput , ViewText ) ;
  }
}

BOOL WatchDogGadget::DeleteTimerQueueFromWatchDogGadget()
{
  if ( m_hTimer != NULL )
  {
    BOOL bDeleteTimerQueueTimer = DeleteTimerQueueTimer( NULL , m_hTimer , NULL );
    m_hTimer = NULL;
    if ( !bDeleteTimerQueueTimer )
    {
      if ( GetLastError() != ERROR_IO_PENDING )
      {
        //SEND_GADGET_ERR( _T("Can't delete Timer") ) ;
        return false;
      }
    }
  }
  return true;
}

VOID CALLBACK TimerRoutine( LPVOID lpParam , BOOLEAN TimerOrWaitFired )
{
  WatchDogGadget * pGadget = (WatchDogGadget*) lpParam;
  if ( pGadget->m_pStatus->GetStatus() != CExecutionStatus::RUN )
    pGadget->DeleteTimerQueueFromWatchDogGadget();
  pGadget->m_stringMessage = "Timeout";
  pGadget->DoTextFrame(); //to notify that the Time Out!!! 
  if ( pGadget->m_iPeriod_ms == 0 )
    pGadget->DeleteTimerQueueFromWatchDogGadget() ;
}

CDataFrame* WatchDogGadget::DoProcessing( const CDataFrame* pDataFrame )
{
  if ( !DeleteTimerQueueFromWatchDogGadget() )
  {
    //ADD COMMENT TO STREAM HANDLER that DeleteTimerQueueTimer failed 
    //this->m_stringMessage = "Delete Timer From Queue From WatchDog Faild"; 
    //this->DoTextFrame();
  }

  // 	this->m_stringMessage = "DataFrame Arrived"; //to notify that DataFrame Arrived!!! 
  // 	this->DoTextFrame(); 

  if ( !CreateTimerQueueTimer( &m_hTimer , NULL , (WAITORTIMERCALLBACK) TimerRoutine ,
    this , m_ihowMuchDelay_ms , m_iPeriod_ms , 0 ) )
  {
    m_stringMessage = "CreateTimerQueueTimer failed"; //to notify that CreateTimerQueueTimer failed
    SEND_GADGET_ERR( m_stringMessage ) ;
    //		this->DoTextFrame();	
  }
  return NULL;
}

void WatchDogGadget::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  pParamFrame->Release( pParamFrame );
};

/*
Use:	void addProperty(SProperty::PropertyBox ePropertyBox, LPCTSTR sName, void* ptrVar, SProperty::EVariableType eVariableType, UINT uiSize, ...)

Int
Long
Double
Bool
String

EDITBOX:
addProperty(EDITBOX,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType)
SPIN:
addProperty(SPIN,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax)
SPIN_BOOL:
addProperty(SPIN_BOOL,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax, bool *ptrBool)
COMBO:
addProperty(COMBO,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, const char *pList)
*/
void WatchDogGadget::PropertiesRegistration()
{
  //addProperty(SProperty::EDITBOX		,	_T("double1")	,	&dProp1		,	SProperty::Double		);
  addProperty( SProperty::SPIN , _T( "TimeOut_ms" ) , &m_ihowMuchDelay_ms
    , SProperty::Long , 1 , 10000 );
  addProperty( SProperty::SPIN , _T( "Period_ms" ) , &m_iPeriod_ms
    , SProperty::Long , 0 , 10000 );
  //addProperty(SProperty::SPIN_BOOL	,	_T("bool_int4")	,	&iProp4		,	SProperty::SpinBool	,	-10		,	20, &bProp4	);
  //addProperty(SProperty::COMBO		,	_T("combo5")	,	&sProp5		,	SProperty::String	,	pList	);
};

/*
Use:	addInputConnector (dataType=transparent, name="")
addOutputConnector(dataType=transparent, name="")
addDuplexConnector(outDataType=transparent, inDataType=transparent, name="")
For create multitype datatype, use:
int createComplexDataType(int numberOfDataTypes, basicdatatype dataType=transparent, ...), as exampled below
*/
void WatchDogGadget::ConnectorsRegistration()
{
  addOutputConnector( transparent , "Timeout" );
  addInputConnector( transparent , "Reset" );
};



