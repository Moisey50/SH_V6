#include "stdafx.h"
#include "SHMemGadget.h"
#include "fxfc/FXRegistry.h"

USER_FILTER_RUNTIME_GADGET( ShMemComm , "Video.recognition" );

ShMemComm::ShMemComm()
{
  m_OutputMode = modeReplace ;
  m_GadgetMode = SHMM_Server ;
  init() ;
}


ShMemComm::~ShMemComm()
{

}

static const char * pGadgetMode = "Server;Client;" ;

void ShMemComm::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "GadgetMode" ) , ( int * ) &m_GadgetMode ,
    SProperty::Int , pGadgetMode ) ;
  addProperty( SProperty::EDITBOX , "Area Name" , &m_ShMemAreaName , SProperty::String ) ;
  addProperty( SProperty::EDITBOX , "In Event Name" , &m_InEventName , SProperty::String ) ;
  addProperty( SProperty::EDITBOX , "Out Event Name" , &m_OutEventName , SProperty::String ) ;

  SetInvalidateOnChange( _T( "GadgetMode" ) , true ) ;
  SetInvalidateOnChange( _T( "Area Name" ) , true ) ;
  SetInvalidateOnChange( _T( "In Event Name" ) , true ) ;
  SetInvalidateOnChange( _T( "Out Event Name" ) , true ) ;

};

void ShMemComm::ConnectorsRegistration()
{
  addInputConnector( transparent , "DataInput" );
  addOutputConnector( transparent , "DataOutput" );
  addOutputConnector( text , "Diagnostics" ) ;
};

CDataFrame* ShMemComm::DoProcessing( const CDataFrame* pDataFrame )
{
  double dStart = GetHRTickCount() ;

  //#ifdef _DEBUG
  FXString DiagInfo ;
  //#endif
  return NULL  ;
}

void ShMemComm::Logger( LPCTSTR pLogString )
{
  PutFrame( GetOutputConnector( 1 ) , CreateTextFrame( pLogString , "LogMsg" ) ) ;
}

