// NullGadget.h: interface for the Null class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NULLGADGET_H__81168301_B13A_49C8_8298_7880FAE5BE5B__INCLUDED_)
#define AFX_NULLGADGET_H__81168301_B13A_49C8_8298_7880FAE5BE5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "TVGeneric.h"
#include <Gadgets\gadbase.h>
#include <math/Intf_sup.h>

class Null : public CGadget
{
private:
  CInputConnector*  m_pInput;
  COutputConnector* m_pOutput;
public:
  Null();
  virtual void ShutDown();
  virtual int GetInputsCount() { return 1; }
  virtual int GetOutputsCount() { return 1; }
  virtual CInputConnector*    GetInputConnector( int n ) { return ( n == 0 ) ? m_pInput : NULL; }
  virtual COutputConnector*   GetOutputConnector( int n ) { return ( n == 0 ) ? m_pOutput : NULL; }
  static void CALLBACK Null_inData( CDataFrame* lpData , void* lpParam , CConnector* lpInput )
  {
    Null * qc = ( Null * ) lpParam;
    if ( !qc->m_pOutput->Put( lpData ) )
      lpData->Release( lpData );
  }
  DECLARE_RUNTIME_GADGET( Null );
};

class NullIzo : public CFilterGadget
{
private:
//   CInputConnector*  m_pInput;
//   COutputConnector* m_pOutput;
public:
  NullIzo();
  virtual void ShutDown();
  virtual int GetInputsCount() { return 1; }
  virtual int GetOutputsCount() { return 1; }
  virtual CInputConnector*    GetInputConnector( int n ) { return ( n == 0 ) ? m_pInput : NULL; }
  virtual COutputConnector*   GetOutputConnector( int n ) { return ( n == 0 ) ? m_pOutput : NULL; }
protected:
  virtual CDataFrame* DoProcessing( const CDataFrame* pDataFrame ) ;
  DECLARE_RUNTIME_GADGET( NullIzo );
};

class Cap : public CGadget
{
private:
  CInputConnector*  m_pInput;
public:
  Cap()
  {
    m_pInput = new CInputConnector( transparent , Cap_inData , this );
    Destroy();
  }
  virtual void ShutDown()
  {
    CGadget::ShutDown();
    delete m_pInput;
    m_pInput = NULL;
  }
  virtual int GetInputsCount() { return 1; }
  virtual int GetOutputsCount() { return 0; }
  virtual CInputConnector* GetInputConnector( int n ) { return ( n == 0 ) ? m_pInput : NULL; }
  virtual COutputConnector* GetOutputConnector( int n ) { return NULL; }
  static void CALLBACK Cap_inData( CDataFrame* lpData , void* lpParam , CConnector* lpInput )
  {
    lpData->Release( lpData );
  }
  DECLARE_RUNTIME_GADGET( Cap );
};

#include "helpers/UserBaseGadget.h"


class Port ;

typedef vector<Port*> Ports ;

class Portal
{ 
public:
  FXString       m_PortalName ;
  FXLockObject m_Lock ;
  Ports        m_PortsOnPortal ;

  Portal( LPCTSTR pPortalName ) 
  {
    m_PortalName = pPortalName ;
  }
};

typedef vector<Portal*> Portals ;

class AllPortals
{
public:
  FXLockObject m_Lock ;
  Portals      m_Portals ;
};

enum PortMode
{
  PMode_In = 0 ,
  PMode_Out = 1 ,
  PMode_InOut = 2
};

class Port : public UserPortBaseGadget 
{
private:
  FXString   m_PortalName ;
  FXString   m_OldPortalName ;
  FXString   m_AdditionalInfo ;
  PortMode   m_PortMode = PMode_InOut ;
  PortMode   m_OldPortMode = PMode_InOut ;
  Portal *   m_pMyPortal ;
  DWORD           m_GroupBorderColor = 0x00c000 ;
  DWORD           m_GroupBodyColor = 0x80c080 ;
  BOOL            m_bGroupSelected = FALSE ;
  static AllPortals m_AllPortals ;  

public:
  Port();
  virtual void ShutDown();
  void PropertiesRegistration();
  void ConnectorsRegistration();
  int RegisterPort() ;
  int UnregisterPort() ;
  static void ConfigParamChange( LPCTSTR pName ,
    void* pObject , bool& bInvalidate , bool& bInitRescan ) ;
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  virtual LPCTSTR GetAdditionalInfo() ;
  virtual BOOL CanBeGrouped() { return TRUE; }
  virtual DWORD GetBorderColor() { return m_GroupBorderColor ; }
  virtual DWORD GetBodyColor() { return m_GroupBodyColor ; }
  virtual void SetGroupSelected( BOOL bSelected ) ;
  virtual BOOL GetGroupSelected() { return m_bGroupSelected ; }
  DECLARE_RUNTIME_GADGET( Port );
};



#endif // !defined(AFX_NULLGADGET_H__81168301_B13A_49C8_8298_7880FAE5BE5B__INCLUDED_)
