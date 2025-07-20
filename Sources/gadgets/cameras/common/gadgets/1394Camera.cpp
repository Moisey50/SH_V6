// 1394Camera.cpp: implementation of the C1394Camera class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "1394Camera.h"
#include <gadgets\textframe.h>
#include <gadgets\stdsetup.h>

#define THIS_MODULENAME "1394Camera"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX(C1394Camera, CCaptureGadget, "Video.capture", TVDB400_PLUGIN_NAME);
//////////////////////////////////////////////////////////////////////
// Protected helpers
//////////////////////////////////////////////////////////////////////

unsigned C1394Camera::SerialToNmb(unsigned serial)
{
  unsigned retV=-1;
  for (unsigned i=0; i<m_CamerasOnBus; i++)
  {
    if (m_CamerasInfo[i].serialnmb==serial)
      return i;
  }
  return retV;
}

unsigned C1394Camera::GetPropertyId(LPCTSTR name)
{
  FXSIZE i;
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

bool C1394Camera::IsDigitField(unsigned ID)
{
  FXString      key;
  FXParser  param;
  for (FXSIZE i=0; i<m_Properties.GetSize(); i++)
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

C1394Camera::C1394Camera(DWORD Inputs):
m_pInputTrigger(NULL),
  m_pControl(NULL),
  m_FrameNmb(0),
  m_CamerasOnBus(0),
  m_CurrentCamera(-1),
  m_evSWTriggerPulse(NULL),
  m_bCameraRunning(false) ,
  m_bInScanProperties(false),
  m_bInsertCamera(true),
  m_GadgetInfo("C1394 Generic") ,
  m_bGadgetInputConnected (false) ,
  m_bSoftwareTriggerMode( false ) ,
  m_bHardwareTrigger( false )
{
  m_pOutput = new COutputConnector(vframe);
  if (Inputs&SYNC_INPUT)
  {
    m_pInputTrigger = new CInputConnector(transparent, c1394c_fn_capture_trigger, this);
    m_evSWTriggerPulse = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    m_TriggerEvents[0] = m_evExit ;
    m_TriggerEvents[1] = m_evSWTriggerPulse ;
  }
  if (Inputs&CTRL_INPUT)
  {
    m_pControl = new CDuplexConnector(this, transparent, transparent);
  }
}

void C1394Camera::ShutDown() 
{ 
  CCaptureGadget::ShutDown();
  if (m_evSWTriggerPulse)
  {
    FxReleaseHandle(m_evSWTriggerPulse); // handle will be settled to NULL
    m_TriggerEvents[1] = NULL;
  }
  if ( m_pOutput )
  {
  delete m_pOutput;
  m_pOutput = NULL;
  }
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
  CameraClose();
}

void C1394Camera::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
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
      for (INT_PTR i=0; i<m_Properties.GetSize(); i++)
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
      if ((id!=WRONG_PROPERTY) && GetCameraProperty(id,value,bauto))
      {
        if (IsDigitField(id))
        {
          if (bauto)
            pk="auto";
          else
            pk.Format("%d",value);
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
      FXSIZE value=0;
      bool bauto=false, Invalidate=false;
      if (IsDigitField(id))
      {
        if (param.CompareNoCase("auto")==0)
          bauto=true;
        else
          value=atoi(param);
      }
      else
        value=(FXSIZE)(LPCTSTR)param;
      if ((id!=WRONG_PROPERTY) && SetCameraProperty(id,value,bauto,Invalidate))
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

bool C1394Camera::DriverValid()
{
  m_bGadgetInputConnected = ( m_pInputTrigger && m_pInputTrigger->IsConnected() ) ;
  
  return ( /*m_CurrentCamera  && */ m_CurrentCamera </*=*/ m_CamerasOnBus );
}

bool    C1394Camera::CameraStart()
{
  m_FrameNmb=0;
  return true;
}

bool C1394Camera::IsRunning()
{
  return (m_bRun!=FALSE);
}

bool C1394Camera::PrintProperties(FXString& text)
{
  FXPropertyKit pc;
  if(DriverValid())
  {
    if ( m_bInsertCamera )
    pc.WriteInt("Camera", m_CamerasInfo[m_CurrentCamera].serialnmb);
    for (FXSIZE i=0; i<m_Properties.GetSize(); i++)
    {
      FXSIZE value; 
      bool bauto;
      if (GetCameraProperty(m_Properties[i].id,value,bauto))
      {
        FXString key;
        FXParser param;
        m_Properties[i].property.GetElementNo(0,key,param);
        if ((key==SETUP_COMBOBOX) || (key==SETUP_SPIN)) // ints result
        {
          pc.WriteInt(m_Properties[i].name,(int)value);
        }
        else if (key==SETUP_SPINABOOL)
        {
          FXString tmpS;
          if (bauto)
            tmpS="auto";
          else
            tmpS.Format("%d",(int)value);
          pc.WriteString(m_Properties[i].name,tmpS);
        }
        else if (key==SETUP_EDITBOX)
        {
          FXString svalue=(LPCTSTR)value;
          pc.WriteString(m_Properties[i].name,svalue);
        }
        else
        {
          C1394_SENDERR_1("Unsupported key '%s'in scanproperty",key);
        }

      }
    }
    text += pc;
  }
  else
  {
    pc.WriteInt("Camera", -1);
    text=pc;
  }
  pc.Empty() ;

  CGadget::PrintProperties( pc ) ;
  text += pc ;
  return true;
}

bool C1394Camera::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  if ( m_bInScanProperties )
    return true ;
  m_bInScanProperties = true ;
  if (!DriverInit())
    C1394_SENDERR_0("Fatal error: Can't load camera drivers!");

  FXString tmpS;
  FXPropertyKit pc(text);
  unsigned camSN;
  if (pc.GetInt("Camera",     (int&)camSN))
  {
    unsigned newCamnmb= SerialToNmb(camSN);;
    if (m_CurrentCamera!=newCamnmb)
    {
      m_CurrentCamera=newCamnmb;
      bool wasRunning = IsRunning() || m_bRun ;
      if (IsRunning())
        CameraStop();
      CameraInit();
      if (wasRunning)
        CameraStart();
      Invalidate|=true; //update setup
    }
  }
  if (DriverValid())
  {
    bool bSToppedFlag = ShouldBeStoppedOnReprogramming() && IsCameraRunning() ; 
    if ( bSToppedFlag )
    {
      CameraStop() ;
      Sleep(50) ;
    }

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
          FXSIZE xValue = value ;
          if (!SetCameraProperty(m_Properties[i].id,xValue , bauto, Invalidate))
          {
            C1394_SENDERR_1("Can't set property %s",m_Properties[i].name);
          }
        }
      }
      else if (key==SETUP_SPINABOOL)
      {
        FXString tmpS; 
        FXSIZE value; 
        bool bauto=false;
        if (pc.GetString(m_Properties[i].name,tmpS))
        {
          bauto=tmpS.CompareNoCase("auto")==0;
          if (bauto)
          {
            GetCameraProperty(m_Properties[i].id,value,bauto);
            bauto=true;
          }
          else
            value=atoi(tmpS);
          if (!SetCameraProperty(m_Properties[i].id,value,bauto,Invalidate))
          {
            C1394_SENDERR_1("Can't set property %s",m_Properties[i].name);
          }
        }
      }
      else if (key==SETUP_EDITBOX)
      {
        FXString svalue;
        FXSIZE value; bool bauto=false;
        if (pc.GetString(m_Properties[i].name,svalue))
        {
          value=(FXSIZE)((LPCTSTR)svalue);
          if (!SetCameraProperty(m_Properties[i].id,value,bauto, Invalidate))
          {
            C1394_SENDERR_2("Can't set prop %s to %s" , 
              m_Properties[i].name , svalue );
          }
        }
      }
      else
      {
        C1394_SENDERR_1("Unsupported key '%s'in scanproperty",key);
      }
    }
    if ( bSToppedFlag )
      CameraStart() ;
  }
  m_bInScanProperties = false ;
  return true;
}

bool C1394Camera::ScanSettings(FXString& text)
{
    EnumCameras();
  // Prepare cameras list
  FXString camlist("Not Selected(-1),"), paramlist, tmpS;
  for (unsigned i=0; i<m_CamerasOnBus; i++)
  {
    tmpS.Format("%s_SN%d(%d)",m_CamerasInfo[i].name,m_CamerasInfo[i].serialnmb,m_CamerasInfo[i].serialnmb);
    camlist+=tmpS+',';
  }
  tmpS.Format("ComboBox(Camera(%s)),",camlist);
  paramlist+=tmpS;
  if (DriverValid())
  {
    for (FXSIZE i=0; i<m_Properties.GetSize(); i++)
    {
      paramlist+=m_Properties[i].property+',';
    }
  }
  text.Format("template(%s)",paramlist);
  return true;
}

void C1394Camera::CameraTriggerPulse(CDataFrame* pDataFrame)
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
    m_bTriggerOnPinReceived = true ;
  ::SetEvent(m_evSWTriggerPulse);
  }
  pDataFrame->Release();
}

CVideoFrame* C1394Camera::CameraDoGrab(double* StartTime)
{
  return NULL;
}

CDataFrame* C1394Camera::GetNextFrame( double * dStartTime )
{
  CDataFrame* df=NULL;
  if (IsRunning())
  {
    if (IsTriggerByInputPin())
    {
      DWORD res = ::WaitForMultipleObjects( sizeof( m_TriggerEvents )/sizeof( m_TriggerEvents[0] ) , 
        m_TriggerEvents , FALSE, 50 ) ;
      switch ( res )
      {
      case WAIT_TIMEOUT:
      default:
        df=CameraDoGrab(dStartTime);
        if (df)
        {
          if ( m_bSoftwareTriggerMode )
            break ;
          df->Release() ;
        }

      case WAIT_OBJECT_0: 
        return NULL ;
      case (WAIT_OBJECT_0 + 1) :
        df=CameraDoGrab(dStartTime);
        break ;
      } 
//       HANDLE pEvents[] = { m_evExit, m_evSWTriggerPulse };
//       DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
//       DWORD res;
//       if((res=::WaitForMultipleObjects(cEvents,pEvents, FALSE, 0))==WAIT_TIMEOUT)
//       {
//         if (res==WAIT_OBJECT_0) 
//           return NULL;
//         df=CameraDoGrab(dStartTime);
//         if (df) 
//           df->Release(df);
//         return NULL;
//       } 
    }
    else
    {
      df=CameraDoGrab(dStartTime);
    }
  }
  return df;
}
