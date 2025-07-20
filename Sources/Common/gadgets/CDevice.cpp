// CDevice.cpp: implementation of the CDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CDevice.h"
#include <gadgets\textframe.h>
#include <gadgets\stdsetup.h>

#define THIS_MODULENAME "CDevice"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

LPCTSTR GetTypeName( PropertyDataType Type )
{
  switch ( Type )
  {
    case PDTBool: return _T( "Boolean" ) ;
    case PDTInt:  return _T( "Int" ) ;
    case PDTFloat:   return _T( "Float" ) ;
    case PDTString:  return _T( "String" ) ;
    case PDTEnum:     return _T( "Enum" ) ;
    case PDTCommand:  return _T( "Command" ) ;
    case PDTCategory:  return _T( "Category" ) ;
    case PDTExternal:  return _T( "External" ) ;
    case PDTUnknown:  return _T( "Unknown" ) ;
  }
  return _T( "Undefined" ) ;
}

Device::DeviceInfo  CDevice::m_DevicesInfo[MAX_DEVICESNMB] ;
ULONG               CDevice::m_NConnectedDevices = 0 ;
CSharedMemBoxes     CDevice::m_Devices ;

IMPLEMENT_RUNTIME_GADGET_EX(CDevice, CCaptureGadget, "Video.capture", TVDB400_PLUGIN_NAME);
//////////////////////////////////////////////////////////////////////
// Protected helpers
//////////////////////////////////////////////////////////////////////

int CDevice::SerialToNmb( unsigned serial )
{
  CAutoLockMutex_H alm( m_Devices.GetMutexHandle() ) ;
  ASSERT( alm.GetStatus() == WAIT_OBJECT_0 ) ;
  Device::GlobalDeviceInfo * pInfo = (Device::GlobalDeviceInfo *)m_Devices.GetBoxesArray() ;
  for ( unsigned i = 0; i<m_NConnectedDevices; i++ )
  {
    if ( (pInfo[ i ].dwSerialNumber == serial) )
      return i;
  }
  return -1 ;
}

int CDevice::SerialToNmb( LPCTSTR pSerial )
{
  CAutoLockMutex_H alm( m_Devices.GetMutexHandle() ) ;
  ASSERT( alm.GetStatus() == WAIT_OBJECT_0 ) ;
  Device::GlobalDeviceInfo * pInfo = (Device::GlobalDeviceInfo *)m_Devices.GetBoxesArray() ;
  for ( unsigned i = 0; i<m_NConnectedDevices; i++ )
  {
    if ( !strcmp( pInfo[ i ].szSerialNumber , pSerial) )
      return i;
  }
  return -1 ;
}

int CDevice::SaveDevInfo( Device::GlobalDeviceInfo& Info , int iIndex )
{
  if ( (iIndex < 0) || (iIndex >= m_Devices.GetNBoxes()) )
    return -1 ;
  CAutoLockMutex_H alm( m_Devices.GetMutexHandle() ) ;
  ASSERT( alm.GetStatus() == WAIT_OBJECT_0 ) ;
  Device::GlobalDeviceInfo * pInfo = (Device::GlobalDeviceInfo *)m_Devices.GetBoxesArray() ;
  memcpy( pInfo + iIndex , &Info , sizeof( Device::GlobalDeviceInfo ) ) ;
  return sizeof( Device::GlobalDeviceInfo ) ;
}

unsigned CDevice::GetPropertyId(LPCTSTR name)
{
  int i;
  unsigned retV=WRONG_PROPERTY;
  for (i=0; i<m_Properties.GetSize(); i++)
  {
    if (m_Properties[i].name.CompareNoCase(name)==0)
    {
      retV=m_Properties[i].id;
      break;
    }
  }
  return retV;
}

// Checking, that property presentation is not EditBox
// i.e. property is integer number
bool CDevice::IsDigitField(unsigned ID) 
{
  FXString      key;
  FXParser  param;
  for (int i=0; i<m_Properties.GetSize(); i++)
  {
    if (m_Properties[i].id==ID)
    { 
      if (m_Properties[i].property.GetElementNo(0, key, param)) 
        return (key.CompareNoCase(SETUP_EDITBOX)!=0) ;
    }
  }
  return true;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDevice::CDevice( DWORD Inputs , int iNBoxes , int iBoxSizeBytes , LPCTSTR pAreaName ):
  m_pInputTrigger(NULL),
  m_pControl(NULL),
  m_FrameNmb(0),
  m_CurrentDevice(-1),
  m_evSWTriggerPulse(NULL),
  m_bDeviceRunning(false) ,
  m_bInScanProperties(false),
  m_dwSerialNumber (-1) ,
  m_GadgetInfo("CDevice Generic") ,
  m_bAttachedAndShouldBeRestarted( false ) ,
  m_bGadgetInputConnected( false ) ,
  m_bSoftwareTriggerMode( false ) ,
  m_bHardwareTrigger( false ) ,
  m_bInitialOK( false )
{
  m_pOutput = new COutputConnector(vframe);
  if (Inputs&SYNC_INPUT)
  {
    m_pInputTrigger = new CInputConnector(nulltype, cdevice_fn_capture_trigger, this);
    m_evSWTriggerPulse = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    m_TriggerEvents[0] = m_evExit ;
    m_TriggerEvents[1] = m_evSWTriggerPulse ;
  }
  if (Inputs&CTRL_INPUT)
  {
    m_pControl = new CDuplexConnector(this, transparent, transparent);
  }
  if ( !m_Devices.IsInitialized() )
    m_Devices.Initialize( iNBoxes , iBoxSizeBytes , pAreaName ) ;
  HANDLE hMutex = m_Devices.GetMutexHandle() ;
  if ( hMutex )
  {
    CAutoLockMutex_H al( hMutex ) ;
    if ( m_NConnectedDevices == 0 )
      memset( m_DevicesInfo , 0 , sizeof(m_DevicesInfo) ) ;
  }
}

void CDevice::ShutDown() 
{ 
  CCaptureGadget::ShutDown();
  if (m_evSWTriggerPulse)
  {
    FxReleaseHandle(m_evSWTriggerPulse); // handle will be settled to NULL
    m_TriggerEvents[1] = NULL;
  }
  delete m_pOutput;
  m_pOutput = NULL;
  if (m_pInputTrigger)
  {
    delete m_pInputTrigger;
    m_pInputTrigger = NULL;
  }
  if (m_pControl)
  {
    delete m_pControl;
    m_pControl = NULL;
  }
  DeviceClose();
}

void CDevice::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
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

    if (cmd.CompareNoCase("list")==0)
    {
      pk.Empty();
      for (FXSIZE i=0; i<m_Properties.GetSize(); i++)
      {
        //FXParser
        pk+=m_Properties[i].property; pk+="\r\n";
      }
    }
    else if ((cmd.CompareNoCase("get")==0) && (pk.GetWord(pos,cmd)))
    {
      unsigned id=GetPropertyId(cmd);
      FXSIZE value;
      bool bauto;
      if ((id!=WRONG_PROPERTY) && GetDeviceProperty(id,value,bauto))
      {
        if (IsDigitField(id))
        {
          if (bauto)
            pk="auto";
          else
            pk.Format( (bauto)? "auto %d" : "%d",value);
        }
        else
          pk=(LPCTSTR)value;
      }
      else
        pk="error";
    }
    else if ((cmd.CompareNoCase("set")==0) && (pk.GetWord(pos,cmd)) && (pk.GetParamString(pos, param)))
    {
      unsigned id=GetPropertyId(cmd);
      INT_PTR value=0;
      bool bauto=false, Invalidate=false;
      if ( IsDigitField(id) )
      {
        bauto = IsAuto( param , value ) ;
      }
      else
        value=(INT_PTR)(LPCTSTR)param;
      if ((id!=WRONG_PROPERTY) && SetDeviceProperty(id,value,bauto,Invalidate))
      {
        pk="OK";
      }
      else
        pk="error";
    }
    else
    {
      pk="List of available commands:\r\n"
        "list - return list of properties\r\n"
        "get <item name> - return current value of item\r\n"
        "set <item name>(<value>) - change an item\r\n";
    }
    CTextFrame* retV=CTextFrame::Create(pk);
    retV->ChangeId(NOSYNC_FRAME);
    if (!m_pControl->Put(retV))
      retV->RELEASE(retV);

  }
  pParamFrame->Release(pParamFrame);
}

bool CDevice::DriverValid()
{
  if ( !m_bEnumerated )
    return DriverInit() ;

  return ( m_bEnumerated  && m_NConnectedDevices );
}

bool    CDevice::DeviceStart()
{
  m_FrameNmb=0;
  return true;
}

bool CDevice::IsRunning()
{
  return (m_bRun!=FALSE);
}

bool CDevice::ReleaseDevice()
{
  CAutoLockMutex_H al( m_Devices.GetMutexHandle() ) ;
  ASSERT( al.GetStatus() == WAIT_OBJECT_0 ) ;
  for ( int i = 0 ; i < m_Devices.GetNBusyBoxes() ; i++ )
  {
    if ( m_DevicesInfo[i].serialnmb == m_dwSerialNumber )
    {
      DWORD dwProcessId = GetCurrentProcessId() ;
      if ( m_DevicesInfo[i].ulProcessId )
      {
        if ( m_DevicesInfo[i].ulProcessId != dwProcessId ) 
        {
          DEVICESENDERR_3("Attempt to release #%u in process #%u when other process #%u is owner", 
            m_dwSerialNumber , dwProcessId , m_DevicesInfo[i].ulProcessId );
        }
        else
          m_DevicesInfo[i].ulProcessId = 0 ;
      }
      else
        m_DevicesInfo[i].ulProcessId = 0 ;

      m_CurrentDevice = 0 ;
      return true ;
    }
  }
  return false ;
}

bool CDevice::TakeDevice( DWORD dwSerialNumber )
{
  CAutoLockMutex_H al( m_Devices.GetMutexHandle() ) ;
  ASSERT( al.GetStatus() == WAIT_OBJECT_0 ) ;
  for ( int i = 0 ; i < m_Devices.GetNBusyBoxes() ; i++ )
  {
    if ( m_DevicesInfo[i].serialnmb == dwSerialNumber )
    {
      DWORD dwProcessId = GetCurrentProcessId() ;
      if ( m_DevicesInfo[i].ulProcessId == 0 ) 
      {
        m_DevicesInfo[i].ulProcessId = dwProcessId ;
        return true ;
      }
      else
      {
        DEVICESENDERR_2("Device #%u busy in process #%u", dwSerialNumber , m_DevicesInfo[i].ulProcessId );
        return false ;
      }
    }
  }
  return false ;
}


bool CDevice::PrintProperties(FXString& text)
{
  FXPropertyKit pc;
  //  m_bEnumerated = false ;
  if(DriverValid())
  {
    pc.WriteInt("Device", m_CurrentDevice );
    if ( (m_CurrentDevice != -1) && IsDeviceConnected() )
    {
      for (int i=0; i<m_Properties.GetSize(); i++)
      {
        INT_PTR value; 
        bool bauto;
        if (GetDeviceProperty(m_Properties[i].id,value,bauto))
        {
          FXString key;
          FXParser param;
          m_Properties[i].property.GetElementNo(0,key,param);
          if ((key==SETUP_COMBOBOX) || (key==SETUP_SPIN)) // ints result
          {
            pc.WriteInt( m_Properties[i].name , (int)value);
          }
          else if (key==SETUP_SPINABOOL)
          {
            FXString tmpS ;
            tmpS.Format( (bauto)?"auto %d" : "%d" , (int)value);
            pc.WriteString(m_Properties[i].name,tmpS);
          }
          else if (key==SETUP_EDITBOX)
          {
            FXString svalue=(LPCTSTR)value;
            pc.WriteString(m_Properties[i].name,svalue);
          }
          else
          {
            DEVICESENDERR_1("Unsupported key '%s'in scanproperty",key);
          }

        }
      }
    }
    text=pc;
  }
  else
  {
    pc.WriteInt("Device", -1);
    text=pc;
  }
  return true;
}

bool CDevice::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  if ( m_bInScanProperties )
    return true ;
  m_bInScanProperties = true ;
  if (!DriverInit())
  {
    DEVICESENDERR_0("Fatal error: Can't load Device drivers!");
    m_bInScanProperties = false ;
    return false ;
  }

  FXString tmpS;
  FXPropertyKit pc(text);
  unsigned camSN;
  if (pc.GetInt("Device",     (int&)camSN))
  {
    bool bNoDeviceSelected = (camSN == 0)  ||  (camSN == -1) ;
    if ( !bNoDeviceSelected )
    {
      m_dwSerialNumber = camSN ;
      if ( TakeDevice( camSN ) )
      {
        if ( (SerialToNmb( m_dwSerialNumber ) >= 0) )
        {
          if ( m_CurrentDevice != m_dwSerialNumber )
          {
            m_CurrentDevice = m_dwSerialNumber ;
            bool wasRunning = IsRunning() || m_bRun ;
            if (IsRunning())
              DeviceStop();
            DeviceInit();
            if (wasRunning)
              DeviceStart();
          }
        }
        else
        {
          if ( IsDeviceConnected() )
          {
            if (IsRunning())
              DeviceStop() ;
            DeviceClose() ;
          }
          m_CurrentDevice = -1 ;
        }
      }
      else
      {
        DEVICESENDERR_1("Can't take device #%u" , camSN ) ;
      }
    }
    else
    {
      if (IsRunning())
        DeviceStop();
      DeviceClose() ;
      m_CurrentDevice = m_dwSerialNumber = camSN ;
    }
    Invalidate|=true; //update setup
  }
  if ( DriverValid() && ( (int)m_CurrentDevice > 0) )
  {
    //     bool bSToppedFlag = ShouldBeStoppedOnReprogramming() && IsDeviceRunning() ; 
    //     if ( bSToppedFlag )
    //     {
    //       DeviceStop() ;
    //       Sleep(50) ;
    //     }
    m_bWasStopped = false ;
    for (int i=0; i<m_Properties.GetSize(); i++)
    {
      FXString key;
      FXParser param;
      m_Properties[i].property.GetElementNo(0,key,param);
      if ((key==SETUP_COMBOBOX) || (key==SETUP_SPIN)) // ints result
      {
        int value; 
        bool bauto=false;
        if (pc.GetInt(m_Properties[i].name,value))
        {
          if (!SetDeviceProperty(m_Properties[i].id,value,bauto, Invalidate))
          {
            DEVICESENDERR_1("Can't set property %s",m_Properties[i].name);
          }
        }
      }
      else if (key==SETUP_SPINABOOL)
      {
        FXString tmpS; 
        INT_PTR value; 
        bool bauto=false;
        if (pc.GetString(m_Properties[i].name , tmpS))
        {
          bauto = IsAuto( tmpS , value ) ;
          if (bauto)
          {
            GetDeviceProperty(m_Properties[i].id , value , bauto );
            bauto=true;
          }
          else
            value=atoi(tmpS);
          if (!SetDeviceProperty( m_Properties[i].id, value , bauto , Invalidate ) )
          {
            DEVICESENDERR_1("Can't set property %s",m_Properties[i].name);
          }
        }
      }
      else if (key==SETUP_EDITBOX)
      {
        FXString svalue;
        INT_PTR value; 
        bool bauto=false;
        if (pc.GetString(m_Properties[i].name,svalue))
        {
          value=(INT_PTR)((LPCTSTR)svalue);
          if (!SetDeviceProperty(m_Properties[i].id,value,bauto, Invalidate))
          {
            DEVICESENDERR_2("Can't set prop %s to %s" , 
              m_Properties[i].name , svalue );
          }
        }
      }
      else
      {
        DEVICESENDERR_1("Unsupported key '%s'in scanproperty",key);
      }
    }
    if ( m_bWasStopped )
      DeviceStart() ;
  }
  m_bInScanProperties = false ;
  return true;
}

bool CDevice::ScanSettings(FXString& text)
{
  EnumDevices();
  // Prepare Devices list
  FXString camlist("Not Selected(-1),"), paramlist, tmpS;
  ULONG   ulProcessId = GetCurrentProcessId() ;
  unsigned iDevIndex = 0 ;
  for ( ; iDevIndex<m_NConnectedDevices; iDevIndex++)
  {
    LPCTSTR pMark = (m_DevicesInfo[iDevIndex].ulProcessId == 0) ? _T("+") :
      (m_DevicesInfo[iDevIndex].serialnmb == m_CurrentDevice) ? _T("!") :
      (m_DevicesInfo[iDevIndex].ulProcessId == ulProcessId) ? _T("-") : _T("--") ;
    if ( iDevIndex > 0 )
      camlist += _T(',');
    tmpS.Format("%s#%u_%s(%d)", pMark, m_DevicesInfo[iDevIndex].serialnmb,
      m_DevicesInfo[iDevIndex].m_sGivenName , m_DevicesInfo[iDevIndex].serialnmb);
    camlist += tmpS ;
  }
  if ( (m_dwSerialNumber != -1) && (m_CurrentDevice != m_dwSerialNumber) )
  {
    tmpS.Format( "?#%u(%u)", m_dwSerialNumber , m_dwSerialNumber ) ;
    camlist += tmpS ;
  }
  tmpS.Format("ComboBox(Device(%s))",camlist);
  paramlist+=tmpS;
  if ( DriverValid() && BuildPropertyList() )
  {
    for (int i=0; i<m_Properties.GetSize(); i++)
    {
      if ( !m_Properties[i].name.IsEmpty() )
      {
        paramlist += _T( "," ) ;
        paramlist += m_Properties[ i ].property;
      }
    }
  }
  text.Format("template(%s)",paramlist);
  return true;
}

void CDevice::DeviceTriggerPulse(CDataFrame* pDataFrame)
{
  if ( Tvdb400_IsEOS( pDataFrame ) )
  {
    SetSoftwareTriggerMode( false ) ;
  }
  else
  {
    if ( !m_bSoftwareTriggerMode )
    {
      SetSoftwareTriggerMode( true ) ;
    }
    ::SetEvent(m_evSWTriggerPulse);
  }
  pDataFrame->Release();
}

CVideoFrame* CDevice::DeviceDoGrab(double* StartTime)
{
  return NULL;
}

CDataFrame* CDevice::GetNextFrame( double * dStartTime )
{
  CDataFrame* df=NULL;
  if ( m_bAttachedAndShouldBeRestarted )
  {
    if ( !DeviceInit() )
      return NULL ;
    if ( IsRunning() && !DeviceStart() )
      return NULL ;
    m_bAttachedAndShouldBeRestarted = false ;
  }
  if ( IsRunning() )
  {
    if (IsTriggerByInputPin())
    {
      DWORD res = ::WaitForMultipleObjects( sizeof( m_TriggerEvents )/sizeof( m_TriggerEvents[0] ) , 
        m_TriggerEvents , FALSE, 0 ) ;
      switch ( res )
      {
      case WAIT_TIMEOUT:
      default:
        df=DeviceDoGrab(dStartTime);
        if ( df )
        {
          if ( m_bWasTriggerOnPin )
          {
            m_bWasTriggerOnPin = false ;
            break ;
          }
          df->Release() ;
        }
      case WAIT_OBJECT_0: 
        return NULL ;
      case (WAIT_OBJECT_0 + 1) :
        df=DeviceDoGrab(dStartTime);
        break ;
      } 
      //       HANDLE pEvents[] = { m_evExit, m_evSWTriggerPulse };
      //       DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
      //       DWORD res;
      //       if((res=::WaitForMultipleObjects(cEvents,pEvents, FALSE, 0))==WAIT_TIMEOUT)
      //       {
      //         if (res==WAIT_OBJECT_0) 
      //           return NULL;
      //         df=DeviceDoGrab(dStartTime);
      //         if (df) 
      //           df->Release(df);
      //         return NULL;
      //       } 
    }
    else
    {
      df=DeviceDoGrab(dStartTime);
    }
  }
  return df;
}

bool CDevice::ReastartDevice(void)
{
  return false;
}
