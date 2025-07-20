// NullGadget.cpp: implementation of the Null class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NullGadget.h"
#include "helpers/FramesHelper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(Null, CGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

Null::Null()
{
  m_pInput = new CInputConnector(transparent,Null_inData,this);
  m_pOutput = new COutputConnector(transparent);
  Destroy(); // No thread required
}

void Null::ShutDown()
{
  CGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

IMPLEMENT_RUNTIME_GADGET_EX(NullIzo, CFilterGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);
NullIzo::NullIzo()
{
  m_pInput = new CInputConnector(transparent);
  m_pOutput = new COutputConnector(transparent);
  Resume() ;
}

void NullIzo::ShutDown()
{
  CGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

CDataFrame* NullIzo::DoProcessing(const CDataFrame* pDataFrame)
{
  if ( pDataFrame )
  {
    const CDataFrame * pIsolatedFrame = pDataFrame->Copy() ;
    return ( CDataFrame* ) pIsolatedFrame ;
  }
  return (CDataFrame*)pDataFrame ;
};

IMPLEMENT_RUNTIME_GADGET_EX(Cap, CGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

// Data exchange ports 
// Implementation
AllPortals Port::m_AllPortals ; 

Port::Port()
{
  m_PortalName = "Default" ;
  m_pMyPortal = NULL ;
  init();
  RegisterPort() ;
}

void Port::ShutDown()
{
  UnregisterPort() ;
  UserGadgetBase::ShutDown() ;
}

int Port::UnregisterPort()
{
  if ( m_pMyPortal )
  {
    m_pMyPortal->m_Lock.Lock( INFINITE , "Port::UnregisterPort" ) ;
    for ( auto PortIt = m_pMyPortal->m_PortsOnPortal.begin() ;
      PortIt < m_pMyPortal->m_PortsOnPortal.end() ; PortIt++ )
    {
      if ( *PortIt == this ) // already registered
      { // remove from portal, because name is changed
        m_pMyPortal->m_PortsOnPortal.erase( PortIt ) ;
        if ( m_pMyPortal->m_PortsOnPortal.size() == 0 )
        { // No ports on portal, necessary to remove from portals list
          string Who( "Port::UnregisterPort - " ) ;
          FXAutolock al( m_AllPortals.m_Lock , (Who += m_pMyPortal->m_PortalName).c_str() ) ;
          for ( auto Iter = m_AllPortals.m_Portals.begin() ; 
            Iter < m_AllPortals.m_Portals.end() ; Iter++ )
          {
            if ( *Iter == m_pMyPortal )
            {
              m_AllPortals.m_Portals.erase( Iter ) ;
              m_pMyPortal->m_Lock.Unlock() ;
              delete m_pMyPortal ;
              m_pMyPortal = NULL ;
              m_OldPortalName.Empty() ;
              return 0 ;
            }
          }
          ASSERT( 0 ) ; // Can't be "No portal in all portals"
          size_t iRest = m_pMyPortal->m_PortsOnPortal.size() ;
          m_OldPortalName.Empty() ;
          m_pMyPortal->m_Lock.Unlock() ;
          return (int)iRest ;
        }
        int iNRestPortsOnPortal = (int)m_pMyPortal->m_PortsOnPortal.size() ;
        m_AdditionalInfo.Format( "%s_%d" , ( LPCTSTR ) m_OldPortalName ,
          iNRestPortsOnPortal ) ;
        for ( int i = 0 ; i < iNRestPortsOnPortal ; i++ )
        {
          Port * pPort = m_pMyPortal->m_PortsOnPortal[ i ] ;
          pPort->m_AdditionalInfo = m_AdditionalInfo ;
          pPort->Status().WriteBool( STATUS_REDRAW , true ) ;
          if ( pPort->m_pModifiedUIDs )
            pPort->m_pModifiedUIDs->push( pPort->m_Name ) ;
        }
        m_pMyPortal->m_Lock.Unlock() ;
        m_pMyPortal = NULL ;
        m_OldPortalName.Empty() ;
        return iNRestPortsOnPortal ;
      }
    }
    ASSERT( 0 ) ; // Can't be "No port in my portals"
    m_pMyPortal->m_Lock.Unlock() ;
  }
  return 0 ;
}


int Port::RegisterPort()
{
  if ( m_pMyPortal )
    UnregisterPort() ;

  if ( m_PortalName.IsEmpty() )
  {
    m_AdditionalInfo = "Not Connected" ;
    Status().WriteBool( STATUS_REDRAW , true ) ;
    return 0 ;
  }

  m_AllPortals.m_Lock.Lock() ;
  for ( auto Iter = m_AllPortals.m_Portals.begin() ;
    Iter < m_AllPortals.m_Portals.end() ; Iter++ )
  {
    if ( (*Iter)->m_PortalName == m_PortalName )
    { // Portal with necessary name exists
      Portal * pFoundPortal = (*Iter) ;
      pFoundPortal->m_Lock.Lock() ;
      for ( size_t j = 0 ; j < pFoundPortal->m_PortsOnPortal.size() ; j++ )
      {
        if ( pFoundPortal->m_PortsOnPortal[j] == this )
        { // Already exists
          ASSERT( 0 ) ;
        }
      }
      pFoundPortal->m_PortsOnPortal.push_back( this ) ;
      m_pMyPortal = pFoundPortal ;
      pFoundPortal->m_Lock.Unlock() ;
      break ;
    }
  }
  if ( !m_pMyPortal )
  {
    Portal * pNewPortal = new Portal( m_PortalName ) ;
    pNewPortal->m_PortsOnPortal.push_back( this ) ;
    m_pMyPortal = pNewPortal ;
    m_AllPortals.m_Portals.push_back( pNewPortal ) ;
  }
  if ( m_pMyPortal )
  {
    m_AdditionalInfo.Format( "%s_%d" , ( LPCTSTR ) m_PortalName ,
      ( int ) m_pMyPortal->m_PortsOnPortal.size() ) ;
    m_pMyPortal->m_Lock.Lock() ;
    for ( size_t i = 0 ; i < m_pMyPortal->m_PortsOnPortal.size() ; i++ )
    {
      Port * pPort = m_pMyPortal->m_PortsOnPortal[ i ] ;
      pPort->m_AdditionalInfo = m_AdditionalInfo ;
      pPort->Status().WriteBool( STATUS_REDRAW , true ) ;
      if ( pPort->m_pModifiedUIDs )
        pPort->m_pModifiedUIDs->push( pPort->m_Name ) ;
    }
    m_pMyPortal->m_Lock.Unlock() ;
    m_OldPortalName = m_PortalName ;
  }
  m_AllPortals.m_Lock.Unlock() ;
  
  return ( int ) ( m_pMyPortal ? m_pMyPortal->m_PortsOnPortal.size() : 0 ) ;
}
void Port::ConfigParamChange( LPCTSTR pName ,
  void* pObject , bool& bInvalidate , bool& bInitRescan )
{
  Port * pGadget = ( Port* ) pObject;
  if ( pGadget )
  {
    LPCTSTR pFoundName = NULL ;
    if ( !_tcsicmp( pName , _T( "PortalName" ) ) )
    {
      pGadget->RegisterPort() ;
      pGadget->Status().WriteBool( STATUS_REDRAW , TRUE ) ;
    }
    if ( !_tcsicmp( pName , _T( "PortMode" ) ) )
    {
      if ( pGadget->m_OldPortMode != pGadget->m_PortMode )
      {
        pGadget->Suspend() ;
        UINT OldPortModeMask = pGadget->m_OldPortMode + 1 ; // bit 0 - In, Bit 1 - Out
        UINT PortModeMask = pGadget->m_PortMode + 1 ; // bit 0 - In, Bit 1 - Out
        if ( OldPortModeMask & 1 )
        {
          if ( !( PortModeMask & 1 ) )
          {
            if ( pGadget->GetInputConnector( 0 ) )
            {
              pGadget->GetInputConnector( 0 )->Disconnect( pGadget->GetInputConnector( 0 ) ) ;
              Sleep( 20 ) ;
              while (pGadget->GetInputConnector( 0 )->GetNFramesInQueue())
              {
                CDataFrame * pFrame = NULL ;
                if ( pGadget->GetInputConnector( 0 )->Get( pFrame ) )
                  pFrame->Release() ;
              }
              pGadget->GetInputConnector( 0 )->SetVisible( false ) ;
            }
          }
        }
        else
        {
          if ( PortModeMask & 1 )
          {
            if ( !pGadget->GetInputConnector( 0 ) )
              pGadget->addInputConnector( transparent , "PortalInput" ) ;
            else
              pGadget->GetInputConnector( 0 )->SetVisible( true ) ;
          }
        }
        if ( OldPortModeMask & 2 )
        {
          if ( !( PortModeMask & 2 ) )
          {
            if ( pGadget->GetOutputConnector( 0 ) )
            {
              pGadget->GetOutputConnector( 0 )->Disconnect( pGadget->GetOutputConnector( 0 ) ) ;
              Sleep( 20 ) ;
              pGadget->GetOutputConnector( 0 )->SetVisible( false ) ;
            }
          }
        }
        else
        {
          if ( PortModeMask & 2 )
          {
            if ( !pGadget->GetOutputConnector( 0 ) )
              pGadget->addOutputConnector( transparent , "PortalOutput" ) ;
            else
              pGadget->GetOutputConnector( 0 )->SetVisible( true ) ;
          }
        }
        pGadget->m_OldPortMode = pGadget->m_PortMode ;
        pGadget->Resume() ;
      }
      pGadget->Status().WriteBool( STATUS_REDRAW , TRUE ) ;
    }
  }
}

void Port::SetGroupSelected( BOOL bSelected )
{ 
  if ( m_bGroupSelected != bSelected )
  {
    m_pMyPortal->m_Lock.Lock() ;
    for ( size_t i = 0 ; i < m_pMyPortal->m_PortsOnPortal.size() ; i++ )
    {
      Port * pPort = m_pMyPortal->m_PortsOnPortal[ i ] ;
      pPort->Status().WriteBool( STATUS_REDRAW , true ) ;
      pPort->m_bGroupSelected = bSelected ;
      if ( pPort->m_pModifiedUIDs )
        pPort->m_pModifiedUIDs->push( pPort->m_Name ) ;
    }
    m_pMyPortal->m_Lock.Unlock() ;
  }
}


static const char * pPortMode = "In;Out;InOut;"; // 0 , 1 , 2

void Port::PropertiesRegistration()
{
  addProperty( SProperty::EDITBOX , "PortalName" , &m_PortalName ,
    SProperty::String );
  SetChangeNotificationForLast( ConfigParamChange , this );
  addProperty( SProperty::COMBO , "PortMode" , &m_PortMode ,
    SProperty::Int , pPortMode );
  SetChangeNotificationForLast( ConfigParamChange , this );
}

void Port::ConnectorsRegistration()
{
    addInputConnector( transparent , "PortalInput" );
    if ( !( ( m_PortMode + 1 ) & 1 ) )
      GetInputConnector( 0 )->SetVisible( false ) ;
    addOutputConnector( transparent , "PortalOutput" );
    if ( !( ( m_PortMode + 1 ) & 2 ) )
      GetOutputConnector( 0 )->SetVisible( false ) ;

};

CDataFrame * Port::DoProcessing( const CDataFrame * pDataFrame )
{
  if ( m_pMyPortal )
  {
    FXString Who( "Port::DoProcessing - " ) ;
    FXAutolock al( m_pMyPortal->m_Lock , Who += m_pMyPortal->m_PortalName ) ;
    int iNOut = 0 , iNIn = 0 ;
    for ( auto Iter = m_pMyPortal->m_PortsOnPortal.begin() ;
      Iter < m_pMyPortal->m_PortsOnPortal.end() ; Iter++ )
    {
      COutputConnector * pPinOut = (( UserBaseGadget* ) ( *Iter ) )->GetOutputConnector( 0 ) ;
      if ( pPinOut )
      {
        ( ( CDataFrame* ) pDataFrame )->AddRef() ;
        PutFrame( pPinOut , ( CDataFrame* ) pDataFrame ) ;
        iNOut++ ;
      }
      iNIn += (( ( UserBaseGadget* ) ( *Iter ) )->GetInputConnector( 0 ) != NULL) ;
    }
//     m_AdditionalInfo.Format( "%s_%d" , ( LPCTSTR ) m_PortalName , iNIn , iNOut ) ;
  }
  return NULL ;
}

LPCTSTR Port::GetAdditionalInfo() { return ( LPCTSTR ) m_AdditionalInfo ; } ;
USER_PORT_RUNTIME_GADGET( Port ,"Generic" )
// 
// USER_PORT_RUNTIME_GADGET( Port , "Generic" );
