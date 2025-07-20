// avt3_11Camera.cpp: implementation of the avt3_11 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "avt1394.h"
#include "avt1394Camera.h"
#include <gadgets\stdsetup.h>
#include <video\shvideo.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "AVTFCamera"
#define MARLINCAMERAMAXNAMELENGTH 128

#pragma comment( lib, "FGCamera.lib" )

IMPLEMENT_RUNTIME_GADGET_EX(AVTF, CDevice, "Video.capture", TVDB400_PLUGIN_NAME);

typedef struct tagVideoformats
{
  FG_COLORMODE        vm;
  const char *        name; 
  bool                supported;
}Videoformats;

typedef struct tagFramerates
{
  FG_FRAMERATE        fr;
  const char *        name; 
  const char *        m7name;
}Framerates;

typedef struct tagResolutions
{
  FG_RESOLUTION       res;
  const char *        name; 
}Resolutions;

// typedef struct tagCamProperties
// {
//   FG_PARAMETER        pr;
//   const char *        name; 
// }CamProperties;

LPCTSTR GetErrorMessage(UINT32 errCode)
{
  switch (errCode)
  {
  case FCE_NOERROR:       return "No Error";
  case FCE_ALREADYOPENED: return "Something already opened";
  case FCE_NOTOPENED:     return "Need open before";
  case FCE_NODETAILS:     return "No details";
  case FCE_DRVNOTINSTALLED: return "Driver not installed";
  case FCE_MISSINGBUFFERS: return "Don't have buffers";
  case FCE_INPARMS:       return "Parameter error";
  case FCE_CREATEDEVICE:  return "Error creating WinDevice";
  case FCE_WINERROR:      return "Internal Windows error";
  case FCE_IOCTL:         return "Error DevIoCtl";
  case FCE_DRVRETURNLENGTH: return "Wrong length return data";
  case FCE_INVALIDHANDLE: return "Wrong handle";
  case FCE_NOTIMPLEMENTED: return "Function not implemented";
  case FCE_DRVRUNNING:    return "Driver runs already";
  case FCE_STARTERROR:    return "Couldn't start";
  case FCE_INSTALLERROR:  return "Installation error";
  case FCE_DRVVERSION:    return "Driver has wrong version";
  case FCE_NODEADDRESS:   return "Wrong node address";
  case FCE_PARTIAL:       return "Partial info. copied";
  case FCE_NOMEM:         return "No memory";
  case FCE_NOTAVAILABLE:  return "Requested function not available";
  case FCE_NOTCONNECTED:  return "Not connected to target";
  case FCE_ADJUSTED:      return "A parameter had to be adjusted";
  case HALER_NOCARD:      return "Card is not present";
  case HALER_NONTDEVICE:  return "No logical Device";
  case HALER_NOMEM:       return "Not enough memory";
  case HALER_MODE:        return "Not allowed in this mode";
  case HALER_TIMEOUT:     return "Timeout";
  case HALER_ALREADYSTARTED: return "Something is started";
  case HALER_NOTSTARTED:  return "Not started";
  case HALER_BUSY:        return "Busy at the moment";
  case HALER_NORESOURCES: return "No resources available";
  case HALER_NODATA:      return "No data available";
  case HALER_NOACK:       return "Didn't get acknowledge";
  case HALER_NOIRQ:       return "Interruptinstallerror";
  case HALER_NOBUSRESET:  return "Error waiting for busreset";
  case HALER_NOLICENSE:   return "No license";
  case HALER_RCODEOTHER:  return "RCode not RCODE_COMPLETE";
  case HALER_PENDING:     return "Something still pending";
  case HALER_INPARMS:     return "Input parameter range";
  case HALER_CHIPVERSION: return "Unrecognized chipversion";
  case HALER_HARDWARE:    return "Hardware error";
  case HALER_NOTIMPLEMENTED: return "Not implemented";
  case HALER_CANCELLED:   return "Cancelled";
  case HALER_NOTLOCKED:   return "Memory is not locked";
  case HALER_GENERATIONCNT: return "Bus reset in between";
  case HALER_NOISOMANAGER: return "No IsoManager present";
  case HALER_NOBUSMANAGER: return "No BusManager present";
  case HALER_UNEXPECTED:  return "Unexpected value";
  case HALER_REMOVED:     return "Target was removed";
  case HALER_NOBUSRESOURCES: return "No ISO resources available";
  case HALER_DMAHALTED:   return "DMA halted";
  }
  return "Unknown error";
}
LPCTSTR GetHALErrorMessage(UINT32 errCode , FXString& Msg)
{
  Msg.Empty() ;
  if ( errCode & HALERF_RXHLTISO0 )
    Msg += "ISO-Channel 0 rx halted;" ;
  if ( errCode & HALERF_RXHLTISO1 )
    Msg += "ISO-Channel 1 rx halted;" ;
  if ( errCode & HALERF_RXHLTISO2 )
    Msg += "ISO-Channel 2 rx halted;" ;
  if ( errCode & HALERF_RXHLTISO3 )
    Msg += "ISO-Channel 3 rx halted;" ;
  if ( errCode & HALERF_RXHLTISO4 )
    Msg += "ISO-Channel 4 rx halted;" ;
  if ( errCode & HALERF_RXHLTISO5 )
    Msg += "ISO-Channel 5 rx halted;" ;
  if ( errCode & HALERF_RXHLTISO6 )
    Msg += "ISO-Channel 6 rx halted;" ;
  if ( errCode & HALERF_RXHLTISO7 )
    Msg += "ISO-Channel 7 rx halted;" ;

  if ( errCode & HALERF_ISORXACK )
    Msg += "Error in iso rx ack code;" ;
  if ( errCode & HALERF_ISORX )
    Msg += "Error in iso rx ack code;" ;
  if ( errCode & HALERF_TXRESPONSE )
    Msg += "Error in iso rx ack code;" ;
  if ( errCode & HALERF_ASYRX )
    Msg += "Error in iso rx ack code;" ;
  if ( errCode & HALERF_ASYTX )
    Msg += "Error in iso rx ack code;" ;
  if ( errCode & HALERF_PHYTIMEOUT )
    Msg += "Error in iso rx ack code;" ;
  if ( errCode & HALERF_HDRERROR )
    Msg += "Error in iso rx ack code;" ;
  if ( errCode & HALERF_TCERROR )
    Msg += "Error in iso rx ack code;" ;
  if ( errCode & HALERF_ATSTUCK )
    Msg += "Error in iso rx ack code;" ;

  if ( errCode & HALERF_GRFOVERFLOW    )
    Msg += "General rx fifo overflow;" ;
  if ( errCode & HALERF_ITFUNDERFLOW   )
    Msg += "Isochr. Tx underflow;" ;
  if ( errCode & HALERF_ATFUNDERFLOW   )
    Msg += "Asynchr. Tx underflowe;" ;
  if ( errCode & HALERF_PCIERROR       )
    Msg += "Error accessing PCI-Bus;" ;
  if ( errCode & HALERF_ASYRXRESTART   )
    Msg += "Asy. Rx DMA was restarted;" ;
  if ( errCode & HALERF_NOACCESSINFO   )
    Msg += "Ext. access not stored;" ;
  if ( errCode & HALERF_SELFID         )
    Msg += "Error within SelfId packet;" ;
  if ( errCode & HALERF_DMPORT         )
    Msg += "Data mover port error;" ;
  if ( errCode & HALERF_ISOTX          )
    Msg += "Iso TX error;" ;
  if ( errCode & HALERF_SNOOP          )
    Msg += "Snoop error;" ;

  if ( !Msg.IsEmpty() ) 
    return (LPCTSTR)Msg ;

  return "Unknown Error" ;

}

LPCTSTR AVTF::GetErrorMessage(UINT32 errCode)
{
  return ::GetErrorMessage( errCode ) ;
}

Videoformats vFormats[]=
{
  {CM_Y8,"Y8",true},
  {CM_YUV411,"YUV411", true},
  {CM_YUV422, "YUV422", true},
  {CM_YUV444, "YUV444",true},
  {CM_RGB8, "RGB8",true},
  {CM_Y16,"Y16",true},
  {CM_RGB16,"RGB16",true},
  {CM_SY16, "SY16",false},
  {CM_SRGB16, "SRGB16",false},
  {CM_RAW8, "RAW8",true},
  {CM_RAW16, "RAW16",true},
};

Framerates fRates[]= 
{
  {FR_1_875,"1.875 fps.","0"},
  {FR_3_75,"3.75 fps.","1"},
  {FR_7_5,"7.5 fps.","2"},
  {FR_15,"15 fps.","3"},
  {FR_30,"30 fps.","4"},
  {FR_60,"60 fps.","5"},
  {FR_120,"120 fps.","6"},
  {FR_240,"240 fps.","7"}
};

Resolutions cResolutions[]=
{
  {RES_160_120,"160x120"},
  {RES_320_240,"320x240"},
  {RES_640_480,"640x480"},
  {RES_1024_768,"1024x768"},
  {RES_1280_960,"1280x960"},
  {RES_1600_1200,"1600x1200"},
  {RES_SCALABLE,"Scalable"}
};

#define FGP_FRAMERATE  ((FG_PARAMETER)(FGP_IMAGEFORMAT-1))
#define FGP_RESOLUTION ((FG_PARAMETER)(FGP_IMAGEFORMAT-2))
#define FGP_ROI        ((FG_PARAMETER)(FGP_IMAGEFORMAT-3))
#define FGP_TRIGGERONOFF ((FG_PARAMETER)(FGP_IMAGEFORMAT-4))
#define FGP_EXTSHUTTER ((FG_PARAMETER)(FGP_IMAGEFORMAT-5))
#define FGP_SEND_FINFO ((FG_PARAMETER)(FGP_IMAGEFORMAT-6))
#define FGP_TRIGGERDELAY ((FG_PARAMETER)(FGP_IMAGEFORMAT-7))
#define FGP_GRAB       ((FG_PARAMETER)(FGP_IMAGEFORMAT-8))
#define SAVE_SETTINGS ((FG_PARAMETER)(FGP_IMAGEFORMAT-10))

CamProperties cProperties[] =
{
  {(DWORD) FGP_IMAGEFORMAT, "Format"},
  {(DWORD) FGP_ROI,"ROI"},
  {(DWORD) FGP_GAIN,"Gain"},
  {(DWORD) FGP_EXTSHUTTER, "Shutt_uS"},
  {(DWORD) FGP_GAMMA,"Gamma"},
  {(DWORD) FGP_TRIGGERONOFF, "Trigger"},
  {(DWORD) FGP_FRAMERATE, "FrameRate_x10"},
  {(DWORD) FGP_PACKETSIZE , "PacketSize"} ,
  {(DWORD) FGP_TRIGGERDELAY , "TrigDelay_uS"} ,
  {(DWORD) FGP_BRIGHTNESS,"Brightness"},
  {(DWORD) FGP_AUTOEXPOSURE,"Exposure"},
  {(DWORD) FGP_SHARPNESS,"Sharpness"},
  {(DWORD) FGP_WHITEBALCB,"HWWhitebalanceB"},
  {(DWORD) FGP_WHITEBALCR,"HWWhitebalanceR"},
  {(DWORD) FGP_HUE,"Hue"},
  {(DWORD) FGP_SATURATION,"Saturation"},
  {(DWORD) FGP_TILT,"Tilt"},
  {(DWORD) FGP_IRIS,"Iris"},
  {(DWORD) FGP_FOCUS,"Focus"},
  {(DWORD) FGP_ZOOM,"Zoom"},
  {(DWORD) FGP_PAN,"Pan"} ,
  {(DWORD) FGP_GRAB , "Grab"} ,
  {(DWORD) SAVE_SETTINGS,"SaveSettings"}
//   {FGP_SEND_FINFO , "FrameInfo" } ,
};

__forceinline int GetFormatId(int format)
{
  for (int i=0; i<(sizeof(vFormats)/sizeof(Videoformats)); i++)
  {
    if (format==vFormats[i].vm) return i;
  }
  return -1;
}

__forceinline int GetFramerateId(int framerate)
{
  for (int i=0; i<(sizeof(fRates)/sizeof(Framerates)); i++)
  {
    if (framerate==fRates[i].fr) return i;
  }
  return -1;
}

__forceinline int GetResolutionId(int resolution)
{
  for (int i=0; i<(sizeof(cResolutions)/sizeof(Resolutions)); i++)
  {
    if (resolution==cResolutions[i].res) return i;
  }
  return -1;
}



__forceinline unsigned GetXSize(CFGCamera& Camera)
{
  FGPINFO Info;
  UINT LastResult=Camera.GetParameterInfo(FGP_XSIZE,&Info);
  if(LastResult==FCE_NOERROR)
  {
    return Info.IsValue;
  }
  return 0;
}

__forceinline unsigned GetYSize(CFGCamera& Camera)
{
  FGPINFO Info;
  UINT LastResult=Camera.GetParameterInfo(FGP_YSIZE,&Info);
  if(LastResult==FCE_NOERROR)
  {
    return Info.IsValue;
  }
  return 0;
}

__forceinline unsigned GetXOffset(CFGCamera& Camera)
{
  FGPINFO Info;
  UINT LastResult=Camera.GetParameterInfo(FGP_XPOSITION,&Info);
  if(LastResult==FCE_NOERROR)
  {
    return Info.IsValue;
  }
  return 0;
}

__forceinline unsigned GetYOffset(CFGCamera& Camera)
{
  FGPINFO Info;
  UINT LastResult=Camera.GetParameterInfo(FGP_YPOSITION,&Info);
  if(LastResult==FCE_NOERROR)
  {
    return Info.IsValue;
  }
  return 0;
}

__forceinline unsigned GetMaxWidth(CFGCamera& Camera)
{
  FGPINFO Info;
  UINT LastResult=Camera.GetParameterInfo(FGP_XSIZE,&Info);
  if(LastResult==FCE_NOERROR)
  {
    return Info.MaxValue;
  }
  return 0;
}

__forceinline unsigned GetMaxHeight(CFGCamera& Camera)
{
  FGPINFO Info;
  UINT LastResult=Camera.GetParameterInfo(FGP_YSIZE,&Info);
  if(LastResult==FCE_NOERROR)
  {
    return Info.MaxValue;
  }
  return 0;
}


__forceinline bool IsFormatSupported(CFGCamera& Camera, FG_COLORMODE& vf)
{
  UINT32 Result,ImageFormat;
  Camera.SetParameter(FGP_ENUMIMAGEFORMAT,0);
  do
  {
    Result=Camera.GetParameter(FGP_ENUMIMAGEFORMAT,&ImageFormat);
    if(Result==FCE_NOERROR)
    {
      if ((unsigned)vf==IMGCOL(ImageFormat))
        return true;
    }
  }while(Result==FCE_NOERROR);
  return false;
}

__forceinline bool IsModeSupported(CFGCamera& Camera, FG_COLORMODE vf,FG_FRAMERATE fr,FG_RESOLUTION rs)
{
  UINT32 Result,ImageFormat;
  Camera.SetParameter(FGP_ENUMIMAGEFORMAT,0);
  do
  {
    Result=Camera.GetParameter(FGP_ENUMIMAGEFORMAT,&ImageFormat);
    if(Result==FCE_NOERROR)
    {
      if (ImageFormat==MAKEIMAGEFORMAT(rs,vf,fr))
        return true;
    }
  }while(Result==FCE_NOERROR);
  return false;
}


// Static variables for AVTF

FXLockObject AVTF::m_avtInitLock;
bool AVTF::m_avtDrvReady=false;
FXArray<AVTF* , AVTF*> AVTF::m_AVTGadgets ;
bool                   CDevice::m_bInBusCallback = false ; 

// Callback function for bus events processing
void __stdcall AVTSystemCallBack( void* pContext , UINT32 uiParam , void * lParam )
{
  AVTF::BusCallbackProcess( pContext , uiParam , lParam ) ;
}

FGINIT AVTF::m_InitData = { NULL , 0 , AVTSystemCallBack , (void*)&AVTF::m_AVTGadgets , 0 };

void AVTF::BusCallbackProcess( void* pContext , UINT32 uiParam , void * lParam )
{
  HANDLE hMutex = m_Devices.GetMutexHandle() ;
  if ( uiParam == WPARAM_NODELISTCHANGED )
  {
    m_bInBusCallback = true ;
    FGNODEINFO Nodes[MAX_DEVICESNMB] ;
    ULONG iNNodes = 0 ;
    FXArray <DWORD> DisconnectedDevices ;
    FXArray<int> OldMatched ;
    FXArray<int> NewMatched ;
    FXArray<DWORD> NewDevices ;
    if ( hMutex )
    {
      CAutoLockMutex_H al( hMutex ) ;
      UINT32 Result = FGGetNodeList(Nodes,sizeof(Nodes)/sizeof(FGNODEINFO),(unsigned long*)&iNNodes);

      if (Result!=FCE_NOERROR)
      {
        TRACE("!!! FGGetNodeList error\n");
        FxSendLogMsg( 7 , "AVTSYS_CB" , 0 , "FGGetNodeList error %s" , ::GetErrorMessage( Result ) );
        m_bInBusCallback = false ;
        return ;
      }
      int iNOldNodes = m_Devices.GetNBusyBoxes() ;
      if ( iNOldNodes < 0 )
      {
        TRACE("!!! GetNBusyBoxes error %d\n" , iNOldNodes );
        FxSendLogMsg( 7 , "AVTSYS_CB" , 0 , "GetNBusyBoxes error %d\n" , iNOldNodes );
        m_bInBusCallback = false ;
        return ;
      }
      Device::GlobalDeviceInfo * pOldNodes = 
        (Device::GlobalDeviceInfo *) m_Devices.GetBoxesArray() ;
      int iNActiveNodes = iNOldNodes ;
      if ( iNOldNodes )  //  necessary to check, may be some are disconnected now
      {
        for ( int i = 0 ; i < iNOldNodes ; i++ )
          pOldNodes[i].iMatchedWithNew = -1 ;
        for ( ULONG i = 0 ; i < iNNodes ; i++ )
        {
          DWORD dwSerial = Nodes[i].Guid.Low ;
          int j = 0 ;
          for (  ; j < iNOldNodes ; j++ )
          {
            ASSERT ( pOldNodes[ j ].dwSerialNumber > 0 ) ;
            if ( pOldNodes[ j ].dwSerialNumber == dwSerial )
            {
              pOldNodes[j].iMatchedWithNew = i ;
              break ;
            }
          }
          if ( j >= iNOldNodes )
            NewDevices.Add(i) ; // index of new device, not found in old array
        }
        // NOw we will try to find disappeared devices (i.e. found 
        //   in old devices array  and not found in new Nodes)
        for ( int j = 0 ; j < iNActiveNodes ; j++ )
        {
          if ( pOldNodes[j].iMatchedWithNew < 0 )
          {
            DisconnectedDevices.Add( pOldNodes[j].dwSerialNumber ) ;
            if ( j < iNActiveNodes - 1 )
            {
              memcpy( pOldNodes + j , pOldNodes + j + 1 , 
                sizeof( Device::GlobalDeviceInfo ) * (iNActiveNodes - j - 1) ) ;
            }
            j-- ;
            iNActiveNodes-- ;
          }
        }
        if ( DisconnectedDevices.GetCount() )
        {
          memset( pOldNodes + iNActiveNodes , 0 , 
            sizeof( Device::GlobalDeviceInfo ) * DisconnectedDevices.GetCount() ) ;
        }
      }
      else // there are no old devices
      {  // put all found devices into new devices array
        for ( ULONG i = 0 ; i < iNNodes ; i++ )
          NewDevices.Add( i ) ; // add index in array
      }
      // Now we have to disconnect from disappeared devices
      for ( int i = 0 ; i < DisconnectedDevices.GetCount() ; i++ )
      {
        DWORD dwSerial = DisconnectedDevices[i] ;
        FxSendLogMsg( 1 , "AVTSYS_CB" , 0 , "Disconnected Device #%u" , dwSerial );
        for ( int j = 0 ; j < m_AVTGadgets.GetCount() ; j++ )
        {
          AVTF * pGadget = m_AVTGadgets[j] ;
          pGadget->m_bEnumerated = false ;
          if ( pGadget->m_dwSerialNumber == dwSerial )
          {
            pGadget->DeviceClose() ;
            break ;
          }
        }
      }
      if ( NewDevices.GetCount() )
      {
        int iNewDevicesCnt = 0 ;
        for ( int i = iNActiveNodes ; i < (int)iNNodes ; i++ )
        {
          if ( iNewDevicesCnt >= NewDevices.GetCount() )
            break ;
          FGNODEINFO * pNewNode = &Nodes[ NewDevices[iNewDevicesCnt++] ] ;
          pOldNodes[i].Guid = pNewNode->Guid ;
          pOldNodes[i].dwSerialNumber = pNewNode->Guid.Low ;
          pOldNodes[i].iCardNumber = pNewNode->CardNumber ;
          pOldNodes[i].dwProcessId = 0 ;
          pOldNodes[i].iMatchedWithNew = -1 ;
          pOldNodes[i].szModelName[0] = 0 ;
//           FxSendLogMsg( 1 , "AVTSYS_CB" , 0 , "Device #%u is connected" , pOldNodes[i].dwSerialNumber );
        }
        iNewDevicesCnt = 0 ;
        for ( int i = iNActiveNodes ; i < (INT)iNNodes ; i++ )
        {
          if ( iNewDevicesCnt >= NewDevices.GetCount() )
            break ;
          FGNODEINFO * pNewNode = &Nodes[ NewDevices[iNewDevicesCnt++] ] ;
          DWORD dwSerial = pNewNode->Guid.Low ;
          FxSendLogMsg( 1 , "AVTSYS_CB" , 0 , "New connected Device #%u" , dwSerial );
          for ( int j = 0 ; j < m_AVTGadgets.GetCount() ; j++ )
          {
            AVTF * pGadget = m_AVTGadgets[j] ;
            if ( pGadget->m_dwSerialNumber == dwSerial )
            {
//               pGadget->DeviceInit() ;
//               if ( pGadget->IsRunning() )
//                 pGadget->DeviceStart() ;
              DWORD ProcessId = GetCurrentProcessId() ;
              pOldNodes[i].dwProcessId = ProcessId ;
              pGadget->m_dRestartRequested = GetHRTickCount() + 50. ;
              pGadget->m_bAttachedAndShouldBeRestarted = true ;
            }
          }
        }
      }
      m_Devices.SetNBusyBoxes( iNNodes ) ;
      m_bInBusCallback = false ;
    }
  }
  else if ( uiParam == WPARAM_ERROR )
  {
    FxSendLogMsg( 1 , "AVTSYS_CB" , 0 , "Cards error mask  0x%x" , lParam );
  }
  else if ( WPARAM_ERRORFLAGSCARD0 <= uiParam 
    && uiParam <= WPARAM_ERRORFLAGSCARD9 )
  {
    int iCardNumber = uiParam - WPARAM_ERRORFLAGSCARD0 ;
    FXString Msg ;
    GetHALErrorMessage((UINT32)(size_t)lParam , Msg) ;
    FxSendLogMsg( 7 , "AVTSYS_CB" , 0 , "Card %d error 0x%08X: %s" , iCardNumber , 
      (UINT32) (size_t) lParam , (LPCTSTR)Msg );
  }
  else if ( uiParam == WPARAM_BUSRESET )
  {
    FxSendLogMsg( 1 , "AVTSYS_CB" , 0 , "Bus Reset" );
    if ( hMutex )
    {
//       CAutoLockMutex_H al( hMutex ) ;
//       for ( int i = 0 ; i < m_AVTGadgets.GetCount() ; i++ )
//       {
//         AVTF * pGadget = m_AVTGadgets[i] ;
//         pGadget->m_dRestartRequested = GetHRTickCount() + 50. + (100. * i) ;
//         pGadget->m_iRestartRetryCnt = 0 ;
//       }
    }
    
  }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AVTF::AVTF():
  m_bFrameInfoOn(false) ,
  CDevice( SYNC_INPUT|CTRL_INPUT , MAX_DEVICESNMB , sizeof(Device::GlobalDeviceInfo) , _T("AVT1394") )
  , m_dRestartRequested(0.)
  , m_dRestartedTime( 0. )
{
  m_GadgetInfo="AVTF gadget";
  m_bEnumerated = false ;
  avtInit( this );
  memset(&m_BMIH,0,sizeof(BITMAPINFOHEADER));
  m_Resolution = RES_SCALABLE ;
  m_dAverageConstant = 0.02 ; // 1/50
  m_dAverageFrameInterval = 0. ;
  m_dLastFrameTime = 0. ;
  m_iRestartRetryCnt = 0 ;
  m_iNLastMessagesInBurst = 0 ;
}

void AVTF::ShutDown()
{
  if (IsTriggerByInputPin())
    SetTriggerMode(0); // no trigger
  FXAutolock al(m_Lock);
  CDevice::ShutDown();
  avtDone( this );
}

bool AVTF::avtInit( AVTF * pGadget )
{
  FXAutolock al(m_avtInitLock);
  if ( m_AVTGadgets.GetCount() == 0 )
  {
    UINT32 LastResult=FGInitModule( &m_InitData );
    if (LastResult!=FCE_NOERROR)
    {
      TRACE("!!! FGInit error\n");
      FxSendLogMsg( MSG_ERROR_LEVEL , "AVT Driver Init" , 0 ,
        "Error %s", GetErrorMessage(LastResult));
      return false;
    }
    m_avtDrvReady=true;
  }
  if ( m_avtDrvReady )
  {
    for ( int i = 0 ; i < m_AVTGadgets.GetCount() ; i++ )
    {
      if ( m_AVTGadgets[i] == pGadget )
        return true ;
    }
    m_AVTGadgets.Add( pGadget ) ;
  }
  return m_avtDrvReady;
}
void AVTF::avtDone( AVTF * pGadget )
{
  FXAutolock al(m_avtInitLock);
  for ( int i = 0 ; i < m_AVTGadgets.GetCount() ; i++ )
  {
    if ( m_AVTGadgets[i] == pGadget )
    {
      m_AVTGadgets.RemoveAt( i ) ;
      if ( m_AVTGadgets.GetCount() == 0 )
      {
        FGExitModule();
        m_avtDrvReady=false;
      }
    }
  }
  TRACE("\navtDone for %s, #cameras=%u" , (LPCTSTR)pGadget->m_CameraID , m_AVTGadgets.GetCount() ) ;
}


bool AVTF::DriverInit()
{
  if (!m_avtDrvReady) 
    return false;
  return EnumDevices();
}

bool AVTF::EnumDevices()
{
  FXAutolock al(m_avtInitLock);

  if ( m_bEnumerated )
    return true ;
  if ( !m_bInBusCallback && !m_Devices.GetNBusyBoxes() )
    BusCallbackProcess( this , WPARAM_NODELISTCHANGED , NULL ) ;

//   Device::GlobalDeviceInfo Info[ MAX_DEVICESNMB ] ;
  HANDLE hMutex = m_Devices.GetMutexHandle() ;
  CAutoLockMutex_H alm( hMutex , 5000 ) ;
  int iNDevices = m_Devices.GetNBusyBoxes() ;/*m_Devices.CopyBusyBoxes( Info , MAX_DEVICESNMB ) ;*/
  LPBYTE pInfoAsBytes = m_Devices.GetBoxesArray() ;
  Device::GlobalDeviceInfo * pInfo = (Device::GlobalDeviceInfo *) pInfoAsBytes ;
  char cName[MARLINCAMERAMAXNAMELENGTH];
  for (int i=0; i<iNDevices; i++)
  {
    if ( IsDeviceConnected() && ( pInfo[i].dwSerialNumber == m_CurrentDevice ) )
      m_Camera.GetDeviceName(cName,MARLINCAMERAMAXNAMELENGTH);
    else
    {
      CFGCamera cC;
      m_LastError=cC.Connect(&pInfo[i].Guid); 
      cName[0]=0;
      if (m_LastError!=FCE_NOERROR)
      {
//         DEVICESENDERR_3("EnumDevices: m_Camera.Connect ERROR %s, Card %d , SN %u", 
//           GetErrorMessage(m_LastError) , Info[i].iCardNumber , Info[i].dwSerialNumber );
//         m_DevicesInfo[i].name.Empty();
      }
      else
      {
        cC.GetDeviceName(cName,MARLINCAMERAMAXNAMELENGTH);
        cC.Disconnect();
      }
    }
    m_DevicesInfo[i].serialnmb = pInfo[i].dwSerialNumber ;
    m_DevicesInfo[i].ulCardId = pInfo[i].iCardNumber ;
    if ( m_LastError == FCE_NOERROR )
    {
      strcpy_s( m_DevicesInfo[i].m_sGivenName , cName ) ;
      strcpy_s( pInfo[i].szCameraName , cName ) ;
    }
    m_DevicesInfo[i].Guid = pInfo[i].Guid ;
  }
  m_bEnumerated = (m_avtDrvReady && (m_NConnectedDevices = iNDevices)) ;
  return m_bEnumerated ;
}

bool AVTF::DeviceInit()
{
  if (m_NConnectedDevices==0) 
  {
    DEVICESENDERR_0("Error: No AVT Cameras found on a bus");
    return false;
  }
  if ( IsDeviceConnected() )
    DeviceClose();
//   if ( !TakeDevice( m_dwSerialNumber ) )
//   {
//     DEVICESENDERR_1("Can't take device #%u" , m_dwSerialNumber );
//     if ( m_dwSerialNumber != 0  &&  m_dwSerialNumber != -1 )
//     {
//       ReleaseDevice() ;
//       m_dwSerialNumber = -1 ;
//     }
//     return false;
//   }

  m_CurrentDevice = m_dwSerialNumber ;
  int iIndex = 0 ;
//   int iIndex = SerialToNmb( m_CurrentDevice ) ;
  {
    CAutoLockMutex_H alm( m_Devices.GetMutexHandle() ) ;
    if ( alm.GetStatus() != WAIT_OBJECT_0 ) 
    {
      DEVICESENDERR_0( "DeviceInit: Can't take device mutex" ) ;
      return false ;
    } ;
    int iNBusyBoxes = m_Devices.GetNBoxes() ;
    for ( ; iIndex < iNBusyBoxes ; iIndex++ )
    {
      if ( m_dwSerialNumber == m_DevicesInfo[iIndex].Guid.Low )
        break ;
    }
    if ( iIndex >= iNBusyBoxes )
    {
      DEVICESENDERR_1( "DeviceInit: Camera %d is not found" , m_dwSerialNumber ) ;
      return false ;
    }
  }
  if ( iIndex < m_Devices.GetNBusyBoxes() )
  {
    m_LastError=m_Camera.Connect(&m_DevicesInfo[iIndex].Guid);
    if (m_LastError!=FCE_NOERROR)
    {
      DEVICESENDERR_1("m_Camera.Connect in DeviceInit error %s", GetErrorMessage(m_LastError));
      ReleaseDevice() ;
      return false;
    }
    m_Status.Format("DeviceInit(): %s",GetErrorMessage(m_LastError));
    m_GadgetInfo.Format("#%u_%s" , m_DevicesInfo[iIndex].serialnmb , m_DevicesInfo[iIndex].m_sGivenName );
    m_CameraID = m_GadgetInfo ;

    //     m_LastError = m_Camera.GetParameter( FGP_COLORFORMAT , (UINT32*)&m_ColorModeIndex ) ;
    UINT32 CurrentFormat ;
    m_LastError = m_Camera.GetParameter( FGP_IMAGEFORMAT , &CurrentFormat ) ;
    if ( m_LastError != FCE_NOERROR )
    {
      DEVICESENDERR_1("DeviceInit error - Can't get image format %s",GetErrorMessage( m_LastError) );
    }
    else
    {
      if ( ISDCAMFORMAT(CurrentFormat) )
      {
        UINT32 Format = DCAMFORMAT(CurrentFormat);
        UINT32 Mode = DCAMMODE(CurrentFormat);
        m_ColorMode = (FG_COLORMODE)DCAMRATE(CurrentFormat); 
        if ( Format != 7 || Mode != 0 )
        {
          //           UINT32 DCAM_Image_Format = MAKEDCAMFORMAT( 7 , 0 , m_ColorMode ) ;
          //           m_LastError = m_Camera.SetParameter( FGP_IMAGEFORMAT , DCAM_Image_Format ) ;
          UINT32 ImageFormat = MAKEIMAGEFORMAT( RES_SCALABLE , m_ColorMode , 0 ) ; // last zero is mode0
          m_LastError = m_Camera.SetParameter( FGP_IMAGEFORMAT , ImageFormat ) ;
          if ( m_LastError != FCE_NOERROR )
          {
            DEVICESENDERR_1("DeviceInit error - Can't reprogram format 7 for image %s",GetErrorMessage( m_LastError) );
          }
        }
      }
      else
      {
        FG_RESOLUTION Resolution = (FG_RESOLUTION)IMGRES( CurrentFormat ) ;
        m_ColorMode = (FG_COLORMODE) IMGCOL(CurrentFormat) ; // color info
        FG_FRAMERATE FrameRate = (FG_FRAMERATE)IMGRATE(CurrentFormat) ;
        UINT32 ImageFormat = MAKEIMAGEFORMAT( RES_SCALABLE , m_ColorMode , 0 ) ;   // last zero is mode0
        //         UINT32 DCAM_Image_Format = MAKEDCAMFORMAT( 7 , 0 , m_ColorMode ) ;
        m_LastError = m_Camera.SetParameter( FGP_IMAGEFORMAT , ImageFormat ) ;
        if ( m_LastError != FCE_NOERROR )
        {
          DEVICESENDERR_1("DeviceInit error - Can't set format 7 for image %s",GetErrorMessage( m_LastError) );
        }
      }
    }

    DEVICESENDINFO_0(m_Status);
    bool bRes = BuildPropertyList();    

    return bRes ;
  }
  return false ;
}

void AVTF::DeviceClose()
{
  if ( IsDeviceConnected() )
  {
    m_LastError = m_Camera.Disconnect();
    m_Status.Format("DeviceClose(): %s",GetErrorMessage(m_LastError));
    DEVICESENDINFO_0(m_Status);
    m_CameraID="AVTF gadget";
  }

  ReleaseDevice() ;
}

bool AVTF::BuildPropertyList()
{
  if ( !m_Camera.GetPtrDCam() )
    return false ;

  //   UINT32 ImageFormat;
  //   m_LastError=m_Camera.GetParameter(FGP_IMAGEFORMAT, &ImageFormat);
  // 
  //   m_ColorMode    = (FG_COLORMODE)IMGCOL(ImageFormat);
  //   m_FrameRate = (FG_FRAMERATE)IMGRATE(ImageFormat);
  //   m_Resolution= (FG_RESOLUTION)IMGRES(ImageFormat);

  m_Properties.RemoveAll();
  int propCount=sizeof(cProperties)/sizeof(CamProperties);
  Device::Property P;
  for (int i=0; i<propCount; i++)
  {
    if ((int)cProperties[i].pr<=FGP_IMAGEFORMAT)
    {
      switch (cProperties[i].pr)
      {
      case FGP_IMAGEFORMAT:
        {
          FXString items , tmpS ;
          int formatCount=sizeof(vFormats)/sizeof(Videoformats);
          int iActiveCnt = 0 ;
          for (int j=0; j<formatCount; j++)
          {
            if ((vFormats[j].supported) && (IsFormatSupported(m_Camera,vFormats[j].vm)))
            {
              if ( iActiveCnt++ > 0 )
                items += _T(',') ;
              tmpS.Format("%s(%d)",vFormats[j].name,vFormats[j].vm);
              items += tmpS;
            }
          }
          if (items.GetLength())
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("ComboBox(%s(%s))",P.name,items);
            m_Properties.Add(P);
          }
          break;
        }
      case FGP_FRAMERATE:
        {
          if ( !IsTriggerByInputPin() )
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("EditBox(%s)",P.name);
            m_Properties.Add(P);
            UpdateBMIH() ;
          }
          break;
        }
      case FGP_RESOLUTION:
        {
          //           FXString items,tmpS;
          //           int resCount=sizeof(cResolutions)/sizeof(Resolutions);
          //           for (int j=0; j<resCount; j++)
          //           {
          //             if (cResolutions[j].res!=RES_SCALABLE)
          //             {
          //               if (IsModeSupported(m_Camera,m_ColorMode,FR_15, cResolutions[j].res))
          //               {
          //                 tmpS.Format("%s(%d),",cResolutions[j].name,cResolutions[j].res);
          //                 items+=tmpS;
          //               }
          //             }
          //             else
          //             {
          //               if (IsModeSupported(m_Camera,m_ColorMode,(FG_FRAMERATE)0, cResolutions[j].res))
          //               {
          //                 tmpS.Format("%s(%d),",cResolutions[j].name,cResolutions[j].res);
          //                 items+=tmpS;
          //               }
          //             }
          //           }
          //           if (items.GetLength())
          //           {
          //             P.name=cProperties[i].name;
          //             P.id=(unsigned)cProperties[i].pr;
          //             P.property.Format("ComboBox(%s(%s))",P.name,items);
          //             m_Properties.Add(P);
          //           }
          break;
        }
      case FGP_ROI:
        if (m_Resolution == RES_SCALABLE)
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("EditBox(%s)",P.name);
          m_Properties.Add(P);
        }
        break;
      case FGP_TRIGGERONOFF:
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("ComboBox(%s(Off(0),Tr1(1),Tr1Inv(2),Tr2(3),Tr2Inv(4)))",
            P.name);
          m_Properties.Add(P);
          break;
        }
      case FGP_EXTSHUTTER:
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("%s(%s,%d,%d)",SETUP_SPINABOOL,P.name,10,67000000);
          m_Properties.Add(P);
          break ;
        }
      case FGP_SEND_FINFO:
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("ComboBox(%s(Off(0),On(1)))", P.name );
          m_Properties.Add(P);
          break;
        }
      case FGP_TRIGGERDELAY:
        {
          if ( IsTriggerByInputPin() )
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("Spin(%s,%d,%d)", P.name , 0 , 30000000 );
            m_Properties.Add(P);
          }
          break;
        }
      case SAVE_SETTINGS:
        {
          ULONG iNSettingsChannels = 0 ;
          if ( m_Camera.ReadRegister( 0xf1000550 , &iNSettingsChannels) == FCE_NOERROR )
          {
            P.name = cProperties[i].name ;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("Spin(%s,0,%d)", P.name , iNSettingsChannels );
            m_Properties.Add(P);
          }
        }
        break ;
      case FGP_GRAB:
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        P.property.Format("Spin(%s,%d,%d)", P.name , -1 , 30000000 );
        m_Properties.Add(P);
        break ; // not shown property
      default:
        {
          DEVICESENDERR_1("Error: unsupported property %s", cProperties[i].name);
          break;
        }
      }
    }
    else // std property
    {
      FGPINFO fInfo;
      if (m_Camera.GetParameterInfo((USHORT)cProperties[i].pr,&fInfo)==FCE_NOERROR)
      {
        switch( cProperties[i].pr )
        {
        case FGP_PACKETSIZE:
          if ( m_Resolution==RES_SCALABLE && IsTriggerByInputPin() )
          {
            UINT Result = m_Camera.GetParameterInfo( FGP_PACKETSIZE , &m_PacketSize ) ;
            if ( Result == FCE_NOERROR )
            {
              P.name=cProperties[i].name;
              P.id=(unsigned)cProperties[i].pr;
              P.property.Format("Spin(%s,%d,%d)", P.name , m_PacketSize.MinValue , m_PacketSize.MaxValue );
              m_Properties.Add(P);
            }
          }
          break ;
        default:
          bool autocap=((fInfo.Specific.Type==1) && (fInfo.Specific.Data.FeatureInfo.AutoCap!=0));

          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("%s(%s,%d,%d)",(autocap)?SETUP_SPINABOOL:SETUP_SPIN,P.name,fInfo.MinValue,fInfo.MaxValue);
          m_Properties.Add(P);
          break ;
        }
      }
    }
  }  

  return (true);
}


bool AVTF::GetDeviceProperty(unsigned i, INT_PTR &value, bool& bauto)
{
  bauto=false;
  if ((int)i<=FGP_IMAGEFORMAT)
  {
    switch (i)
    {
    case FGP_IMAGEFORMAT:
      {
        UINT32 res;
        m_Camera.GetParameter(FGP_IMAGEFORMAT,&res);
        value=IMGCOL(res);
        return true;
      }
    case FGP_FRAMERATE:
      {
        UINT Result = m_Camera.GetParameter( FGP_PACKETSIZE , &m_PacketSize.IsValue ) ;
        if ( Result == FCE_NOERROR )
        {
          int iFRameSize = m_BMIH.biSizeImage ;
          int iDataSize = m_PacketSize.IsValue * 8000 ; // 8000 packets per second
          double dFPS = (double)iDataSize / (double)iFRameSize ;
          static FXString sFps_x10 ;
          sFps_x10.Format( _T("%d") , ROUND(dFPS * 10.) ) ;
          value = (INT_PTR)(LPCTSTR)sFps_x10 ;
          return true ;
        }
        //       }
        return false ;

        //         if (m_Resolution==RES_SCALABLE) 
        //           return false;
        //         UINT32 res;
        //         m_Camera.GetParameter(FGP_IMAGEFORMAT,&res);
        //         value=IMGRATE(res);
        return true;
      }
    case FGP_RESOLUTION:
      {
        UINT32 res;
        m_Camera.GetParameter(FGP_IMAGEFORMAT,&res);
        value=IMGRES(res);
        return true;
      }
    case FGP_ROI:
      {
        static FXString sROI;
        if (m_Resolution != RES_SCALABLE) 
          return false;
        CRect rc;
        GetROI(rc);
        sROI.Format("%d,%d,%d,%d",rc.left,rc.top,rc.right,rc.bottom);
        value=(INT_PTR)(LPCTSTR)sROI;
        return true;
      }
    case FGP_TRIGGERONOFF:
      {
        value = GetTriggerMode() ;
        return true;
      }
    case FGP_EXTSHUTTER:
      {
        UINT32 uiCurrentValue ;
        if ( m_Camera.GetParameter( FGP_SHUTTER , &uiCurrentValue )==FCE_NOERROR )
          bauto = ( uiCurrentValue == PVAL_AUTO ) ;
        value = GetLongExposure() ;
        return true ;
      }
    case FGP_SEND_FINFO:
      {
        value = (INT_PTR)m_bFrameInfoOn;
        //value = (IsFrameInfoAvailable() != 0) ;
        return true ;
      }
    case FGP_TRIGGERDELAY:
      {
        value = GetTriggerDelay() ;
        return true ;
      }
    case SAVE_SETTINGS:
      value = 0 ;
      return true ;
    case FGP_GRAB:
      {
        bool bContinuous ;
        bool bRes = GetGrabConditions( bContinuous , value ) ;
        return true;
      }
    case FGP_PACKETSIZE:
      {
        UINT Result = m_Camera.GetParameter( FGP_PACKETSIZE , (UINT32*)&m_PacketSize.IsValue ) ;
        if ( Result == FCE_NOERROR )
        {
          value = m_PacketSize.IsValue ;
          return true ;
        }
      }
      return false ;
    default:
      break;
    }
  }
  else
  {
    UINT32  val;
    if (m_Camera.GetParameter(i,&val)!=FCE_NOERROR)
      return false;
    if (val==PVAL_AUTO)
      bauto=true;

    UINT32 uiRealValue ;
    switch ( i )
    {
    case FGP_WHITEBALCR :
    case FGP_WHITEBALCB :
      {
//         if ( bauto )
//         {
          UINT32 Err = m_Camera.ReadRegister( 0xf0f0080c , &uiRealValue ) ;
          if ( Err == FCE_NOERROR )
            value = (( i == FGP_WHITEBALCR ) ? uiRealValue  : (uiRealValue >> 12)) & 0xfff ;
//           else
//             return false ;
//         }
      }
      break ;
    case FGP_GAIN :
      {
//         if ( bauto )
//         {
          UINT32 Err = m_Camera.ReadRegister( 0xf0f00820 , &uiRealValue ) ;
          if ( Err == FCE_NOERROR )
            value = uiRealValue & 0xfff ;
//         }
      }
      break ;
    default:
      value=val;
      break ;
    }
    return true;
  }
  return false;
}

bool AVTF::SetDeviceProperty(unsigned i, INT_PTR value, bool& bauto, bool& Invalidate)
{
  if ((int)i<=FGP_IMAGEFORMAT)
  {

    switch (i)
    {
    case FGP_IMAGEFORMAT:
      {
        bool wasRunning=IsRunning();
        if (wasRunning)
        {
          OnStop();
          Sleep(80) ;
          m_bWasStopped = true ;
        }
        m_ColorMode=(FG_COLORMODE)value;
        //         UINT32 DCAM_Image_Format = MAKEDCAMFORMAT( 7 , 0 , m_ColorMode ) ;
        UINT32 ImageFormat = MAKEIMAGEFORMAT( RES_SCALABLE , m_ColorMode , 0 ) ; // last zero is mode0
        m_LastError = m_Camera.SetParameter( FGP_IMAGEFORMAT , ImageFormat ) ;
        bool res= ( m_LastError==FCE_NOERROR ) ;
        Invalidate=true;
        if ( wasRunning && !m_bInScanProperties )
          OnStart();
        return res;
      }
    case FGP_FRAMERATE:
      {
        double dFPS_x10 ;
        int iNFields = sscanf( (LPCTSTR)value , _T("%lf") , &dFPS_x10 ) ;
        if ( iNFields && (dFPS_x10 > 1.0 ) )
        {
          UINT Result = m_Camera.GetParameterInfo( FGP_PACKETSIZE , &m_PacketSize ) ;
          if ( Result == FCE_NOERROR )
          {
            int iFRameSize = m_BMIH.biSizeImage ;
            double dBytesPerSecond = iFRameSize * dFPS_x10 * 0.1 ;
            double dPacketSize = dBytesPerSecond / 8000. ;
            double dUnits = dPacketSize / m_PacketSize.Unit ;
            int iPacketSize = ROUND( dUnits ) * m_PacketSize.Unit  ;
            m_PacketSize.IsValue = iPacketSize ;
            bool wasRunning=IsRunning();
            if (wasRunning)
            {
              DeviceStop();
              Sleep( 80 ) ;
              m_bWasStopped = true ;
              //               UINT Result = m_Camera.SetParameter( FGP_RESIZE , 1 ) ;
              //               if ( Result != FCE_NOERROR )
              //               {
              //                 DEVICESENDERR_1( _T("SetProperty FPS_x10: Can't entry into resize mode: %s") , GetErrorMessage( Result) ) ;
              //                 return false ;
              //               }
            }
            ULONG ulAvailableBandwidth ; 
            Result = m_Camera.GetParameter( FGP_IRMFREEBW , &ulAvailableBandwidth ) ;
            if ( Result == FCE_NOERROR )
            {
              if ( ulAvailableBandwidth < m_PacketSize.IsValue )
              {
                m_PacketSize.IsValue = (ulAvailableBandwidth / m_PacketSize.Unit) * m_PacketSize.Unit ;
                DEVICESENDWARN_1( _T("SetProperty FPS_x10: packet size restricted to %d") , m_PacketSize.IsValue ) ;
              }
            }
            Result = m_Camera.SetParameter( FGP_PACKETSIZE , m_PacketSize.IsValue ) ;
            if ( Result != FCE_NOERROR )
            {
              DEVICESENDERR_1( _T("SetProperty FPS_x10: Can't set packet size: %s") , GetErrorMessage( Result) ) ;
            }
            //             if ( wasRunning && !m_bInScanProperties )
            //               OnStart();
            if ( wasRunning && !m_bInScanProperties )
            {
              DeviceStart();
              m_bWasStopped = true ;
              //               UINT Result = m_Camera.SetParameter( FGP_RESIZE , 0 ) ;
              //               if ( Result != FCE_NOERROR )
              //               {
              //                 DEVICESENDERR_1( _T("SetProperty FPS_x10: Can't exit from resize mode: %s") , GetErrorMessage( Result) ) ;
              //                 return false ;
              //               }
            }
            return true ;
          }
          else
            DEVICESENDERR_1( _T("SetProperty FPS_x10: Can't get frame rate: %s") , GetErrorMessage( Result) ) ;
        }
        else
          DEVICESENDERR_1( _T("SetProperty FPS_x10: Error in data: %s") , (LPCTSTR)value ) ;
        return false ;
      }
    case FGP_RESOLUTION:
      {
        bool wasRunning=IsRunning();
        if (wasRunning)
        {
          OnStop();
          Sleep( 80 ) ;
          m_bWasStopped = true ;
        }
        m_Resolution=(FG_RESOLUTION)value;
        m_FrameRate=FR_15;
        bool res=
          (m_Camera.SetParameter(FGP_IMAGEFORMAT,
          MAKEIMAGEFORMAT(
          m_Resolution,
          m_ColorMode,
          (m_Resolution==RES_SCALABLE)?0:m_FrameRate
          )
          )==FCE_NOERROR);
        Invalidate=true;
        if ( wasRunning && !m_bInScanProperties )
          OnStart();
        return res;

      }
    case FGP_ROI:
      {
        if (m_Resolution != RES_SCALABLE) return false;
        CRect rc;
        if (sscanf((LPCTSTR)value,"%d,%d,%d,%d",&rc.left,&rc.top,&rc.right,&rc.bottom)==4)
        {
          bool res=SetROI(rc);
          return res;
        }
        return false;
      }
    case FGP_TRIGGERONOFF:
      {
        bool bIsTriggerByInputPin = IsTriggerByInputPin() ;
        SetTriggerMode( (int)value );
        Invalidate = ( bIsTriggerByInputPin != (value != 0) ) ;
        return true;
      }
    case FGP_EXTSHUTTER:
      {
        UINT32 uiCurrentValue ;
        if ( m_Camera.GetParameter( FGP_SHUTTER , &uiCurrentValue )==FCE_NOERROR )
        {
          if ( uiCurrentValue == PVAL_AUTO )
          {
            UINT32 OutValue = PVAL_OFF ;
            if ( !bauto )
            {
              //               m_LastError = m_Camera.SetParameter( FGP_SHUTTER , PVAL_OFF ) ;
              //               if ( m_LastError != FCE_NOERROR )
              //                 TRACE("\nExt Shutter Set auto ERROR: %s" , GetErrorMessage( m_LastError) ) ;
              DWORD dwRegValue ;
              m_LastError = m_Camera.ReadRegister( 0xf0f0081c , &dwRegValue ) ;
              if ( m_LastError != FCE_NOERROR )
                TRACE("\nRead Shutter auto ERROR: %s" , GetErrorMessage( m_LastError) ) ;
              else
                TRACE("\nRead Shutter auto value 0x%08x" , dwRegValue ) ;

              dwRegValue |= 0x02000000 ;
              dwRegValue &= ~( 0x01000000 ) ;
              m_LastError = m_Camera.WriteRegister( 0xf0f0081c , dwRegValue ) ;
              if ( m_LastError != FCE_NOERROR )
              {
                TRACE("\nWrite Shutter auto ERROR: %s" , GetErrorMessage( m_LastError) ) ;
              }
              //              m_Camera.WriteRegister( 0xF0F0081c , 0x82000000 + (value/20)) ;
              //              m_Camera.WriteRegister( 0xF100020c , 0x80000000 + value ) ;
            }
            else
              return true ;
          }
          else if ( bauto )
          {
            m_Camera.SetParameter( FGP_SHUTTER , PVAL_AUTO ) ;
            return true ;
          }
        }
        SetLongExposure( (int)value ) ;
        return true ;
      }
    case FGP_SEND_FINFO:
      {
        if (!IsFrameInfoAvailable())
        {
          m_bFrameInfoOn = false;
          return false;
        }
        SetSendFrameInfo( (int)value ) ;
        m_bFrameInfoOn = (value!=0);
        return true ;
      }
    case FGP_TRIGGERDELAY:
      {
        SetTriggerDelay( (int)value );
        return true;
      }
    case SAVE_SETTINGS:
      {
        UINT32 Result = m_Camera.WriteRegister( 0xf1000550 , 0 ) ;  // reset error code
        UINT32 RegisterValue = 0x80800001 ;
        Result = m_Camera.WriteRegister( 0xf1000550 , RegisterValue ) ;  // Save to memory channel (profile) 1
        if ( m_LastError != FCE_NOERROR )
        {
          DEVICESENDERR_1( _T("Write Profile error: %s") , GetErrorMessage( Result) ) ;
          return false ;
        }
        UINT32 ReadValue = 0 ;
        Result = m_Camera.ReadRegister( 0xf1000550 , &ReadValue ) ;
        if ( Result != FCE_NOERROR )
        {
          DEVICESENDERR_1( _T("Read Profile status error: %s") , GetErrorMessage( Result) ) ;
          return false ;
        }
        else
        {
          if ( ReadValue & 0x40000000 ) // error
          {
            UINT32 ErrCode = (ReadValue >> 20) & 0xf ;
            DEVICESENDERR_1( _T("Write Profile status error: %d") , ErrCode ) ;
            return false ;
          }
          Result = m_Camera.WriteRegister( 0xf1000550 , 0x80200001 ) ;  // set profile as default
          if ( Result != FCE_NOERROR )
          {
            DEVICESENDERR_1( _T("Set default profile error: %s") , GetErrorMessage( Result) ) ;
            return false ;
          }
          Result = m_Camera.ReadRegister( 0xf1000550 , &ReadValue ) ;
          if ( ReadValue & 0x40000000 ) // error
          {
            UINT32 ErrCode = (ReadValue >> 20) & 0xf ;
            DEVICESENDERR_1( _T("Set default profile error: %d") , ErrCode ) ;
            return false ;
          }
        }
        return true ;
      }
      break ;
    case FGP_GRAB:
      {
        SetGrab( (int)value ) ;
        return true;
      }
    default:
      break;
    }
  }
  else
  {
    switch ( i )
    {
    case FGP_PACKETSIZE:
      if ( m_Resolution==RES_SCALABLE )
      {
        UINT uiSizeInUnits = ROUND((double)value / (double)m_PacketSize.Unit) ;
        UINT uiNewValue = uiSizeInUnits * m_PacketSize.Unit ;
        if ( uiNewValue != m_PacketSize.IsValue ) // is necessary to change packet size?
        {
          bool wasRunning=IsRunning();
          if (wasRunning)
          {
            OnStop();
            Sleep( 80 ) ;
            m_bWasStopped = true ;
          }
          ULONG ulAvailableBandwidth ; 
          UINT32 Result = m_Camera.GetParameter( FGP_IRMFREEBW , &ulAvailableBandwidth ) ;
          if ( Result == FCE_NOERROR )
          {
            if ( ulAvailableBandwidth < m_PacketSize.IsValue )
            {
              m_PacketSize.IsValue = (ulAvailableBandwidth / uiNewValue ) * m_PacketSize.Unit ;
              DEVICESENDWARN_1( _T("SetProperty PacketSize: packet size restricted to %d") , uiNewValue ) ;
            }
          }
          Result = m_Camera.SetParameter( FGP_PACKETSIZE , uiNewValue ) ;
          if ( Result == FCE_NOERROR )
          {
            m_PacketSize.IsValue = uiNewValue ;
            if ( wasRunning && !m_bInScanProperties )
              OnStart();
            return true ;
          }
        }
        else // not necessary
          return true ;
      }
      return false ;
    case FGP_WHITEBALCB:
    case FGP_WHITEBALCR:
      {
        UINT32 lValue = 0 ;
        if ( m_Camera.GetParameter( i , &lValue) == FCE_NOERROR )
        {
          if ( (lValue == PVAL_AUTO) && !bauto )
          {
            UINT32 uiRealValue ;
            UINT32 Err = m_Camera.ReadRegister( 0xf0f0080c , &uiRealValue ) ;
            if ( Err == FCE_NOERROR )
            {
              value = (( i == FGP_WHITEBALCR ) ?
                uiRealValue  : (uiRealValue >> 12)) & 0xfff ;
            }
//             Invalidate = true ;
          }
          UINT32  val=(bauto)?PVAL_AUTO:(UINT32)value;
          bool bRes = (m_Camera.SetParameter(i,val)==FCE_NOERROR);
          return bRes ;
        }
      }
    default:
      break;
    }
    UINT32  val=(bauto)?PVAL_AUTO: (UINT32) value;
    bool bRes = (m_Camera.SetParameter(i,val)==FCE_NOERROR);

    return bRes ;
  }
  return false;
}

bool AVTF::DeviceStart()
{
  UpdateBMIH() ;
  m_LastError=m_Camera.OpenCapture(); 
  if (m_LastError!=FCE_NOERROR)
  {
    DEVICESENDERR_1("Error in m_Camera.OpenCapture(): %s", GetErrorMessage(m_LastError));
    //     return false;
  }
  m_LastError=m_Camera.StartDevice();
  if (m_LastError!=FCE_NOERROR)
  {
    DEVICESENDERR_1("Error in m_Camera.StartDevice(): %s", GetErrorMessage(m_LastError));
    m_Camera.CloseCapture() ;
    //     return false;
  }
  /*	FXString Prop ;
  bool Invalidate ;
  if ( PrintProperties( Prop ) )
  ScanProperties( Prop , Invalidate ); */
  //     for ( int i = 0 ; i < m_Properties.GetCount() ; i++ )
  //     {
  //       SetDeviceProperty( i , m_Properties[i].)
  //     }
  m_bWasStopped = false ;
  return true;
}

void AVTF::DeviceStop()
{
  m_Camera.StopDevice(); 
  m_Camera.CloseCapture(); 
}

CVideoFrame* AVTF::DeviceDoGrab(double* StartTime)
{
  double dStart = GetHRTickCount() ;
  double dGetTime , dToRGBTime, dToYUV9Time ;
  if (m_BMIH.biSize==0) 
  {
    Sleep(10);
    *StartTime=GetHRTickCount();
    return NULL;
  }
  FXAutolock al(m_Lock);
  *StartTime = ::GetHRTickCount();
  FGFRAME    CurFrame;
  pTVFrame    frame = NULL ;
  m_LastError=m_Camera.GetFrame(&CurFrame,1000); 
  if(m_LastError==FCE_NOERROR) 
  {
    m_dRestartRequested = m_dRestartedTime = 0. ;
    m_iRestartRetryCnt = 0 ;
    double dCurrentTime = GetHRTickCount() ;
    *StartTime = dCurrentTime ;
    if ( m_dLastFrameTime != 0. )
    {
      double dLastFrameInterval = dCurrentTime - m_dLastFrameTime ;
      if ( m_dAverageFrameInterval != 0. )
      {
        m_dAverageFrameInterval = m_dAverageFrameInterval * ( 1 - m_dAverageConstant)
          + dLastFrameInterval * m_dAverageConstant ;
      }
      else
        m_dAverageFrameInterval = dLastFrameInterval ;
    }
    m_dLastFrameTime = dCurrentTime ;
    memcpy(&m_RealBMIH,&m_BMIH,m_BMIH.biSize);
    frame = (pTVFrame)malloc(sizeof(TVFrame));
    frame->lpBMIH = NULL ;
    frame->lpData = NULL ;

    /*if (m_StartTime.QuadPart==-1)
    {
    m_StartTime.LowPart=CurFrame.RxTime.Low;
    m_StartTime.HighPart=CurFrame.RxTime.High;
    }*/
    double dSizeCoeff ;
    bool bCropped=false;
    LPBYTE data = CurFrame.pData;
    int newWidth = m_BMIH.biWidth & ~(0x3) ;
    int newHeight = m_BMIH.biHeight & ~(0x3) ;
    bCropped = true;
    if ( GetBitCountAndSizeCoeff( m_ColorMode , m_RealBMIH.biBitCount , dSizeCoeff ) )
    {
      int size = ROUND(newHeight * newWidth * dSizeCoeff) ;
      m_RealBMIH.biSizeImage = size;
      m_RealBMIH.biWidth = newWidth;
      m_RealBMIH.biHeight = newHeight;
      bool bMono = (m_ColorMode == CM_Y8) || (m_ColorMode == CM_Y16) ;
      bool b8bits = (m_ColorMode == CM_Y8) || (m_ColorMode == CM_RAW8) ;
      bool b16bits = (m_ColorMode == CM_Y16) || (m_ColorMode == CM_RAW16) ;
      if ( b8bits || b16bits )
      {
        if ( bMono )
        {
          frame->lpBMIH = (LPBITMAPINFOHEADER)malloc(
            sizeof(BITMAPINFOHEADER) + size ) ;
          memcpy( frame->lpBMIH , &m_RealBMIH , m_RealBMIH.biSize ) ;
          char * src = (char*)CurFrame.pData;
          char * dst = (char*)(frame->lpBMIH + 1);
          if ( m_BMIH.biWidth != newWidth )
          {
            //memset(data,0,size);
            int iRowCopySize = ROUND(newWidth * dSizeCoeff) ;
            int iSrcInterval = ROUND(m_BMIH.biWidth * dSizeCoeff) ;
            for (int i=0; i<newHeight; i++ )
            {
              if ( b8bits )
                memcpy( dst , src , iRowCopySize );
              else
                _swab( dst ,src , iRowCopySize ) ;
              dst += iRowCopySize ;
              src += iSrcInterval ;
            }
          }
          else
          {
            char * pEnd = src + size;
            if (b16bits)
            {
              while ( src < pEnd )
              {
                *(dst++) = *(src + 1);
                *(dst++) = *src;
                src += 2;
              }
            }
            else
            memcpy( dst , src , size ) ;
          }
        }
        else if ( m_ColorMode == CM_RAW8 )
        {
          VmbImage Src , Dest ;
          Src.Size = Dest.Size = sizeof( Dest ) ;
          Src.Data = CurFrame.pData ;
          dGetTime = GetHRTickCount() - dStart ;
          VmbError_t Res = VmbSetImageInfoFromPixelFormat( 
            VmbPixelFormatBayerRG8 , newWidth , newHeight , &Src ) ;
          if ( Res == VmbErrorSuccess )
          {
            Res = VmbSetImageInfoFromPixelFormat( VmbPixelFormatBgr8 , newWidth , newHeight , &Dest ) ;
            if ( Res == VmbErrorSuccess )
            {
              VmbTransformInfo info;
              // set the debayering algorithm to simple 2 by 2
              Res = VmbSetDebayerMode ( VmbDebayerMode2x2 , &info );
              if ( Res == VmbErrorSuccess )
              {
                int iSize = (3 * size) ;
                LPBITMAPINFOHEADER lpBMIH=
                  (LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER) + iSize);
                Dest.Data = malloc( iSize ) ;
                Res = VmbImageTransform ( &Src , &Dest , &info , 1 ); 
                if ( Res == VmbErrorSuccess )
                {
                  dToRGBTime = GetHRTickCount() - dStart ;
                  if ( lpBMIH )
                  {
                    LPBYTE pInv = (LPBYTE)&lpBMIH[1] ;
                    int iCopySize = newWidth * 3 ;
                    for ( int i = 0 ; i < newHeight ; i++ )   // reverse rows
                      memcpy( pInv + i * iCopySize , 
                      (LPBYTE)Dest.Data + (newHeight - i - 1) * iCopySize , iCopySize ) ;
                    memcpy( lpBMIH , &m_BMIH , sizeof(BITMAPINFOHEADER) );
                    lpBMIH->biWidth = newWidth ;
                    lpBMIH->biHeight = newHeight ;
                    lpBMIH->biSizeImage=iSize;
                    lpBMIH->biCompression = BI_RGB ;
                    lpBMIH->biBitCount = 24 ;
                    lpBMIH->biPlanes=1;
                    frame->lpBMIH = lpBMIH ;
                    //                    frame->lpData = (LPBYTE) Dest.Data ;

                    //                     LPBITMAPINFOHEADER lpYUV9 = rgb24yuv9( lpBMIH , pInv ) ;
                    //                     if ( lpYUV9 )
                    //                       frame->lpBMIH = lpYUV9 ;
                    dToYUV9Time = GetHRTickCount() - dStart ;
                    //                     free( lpBMIH ) ;
                  }
                }
                else
                {
                  SENDERR_1(  "Can't perform image transform to RGB: %s " , 
                    ErrCodeToMessage(Res) ) ;
                }
                free( Dest.Data ) ;
              }
              else
              {
                SENDERR_1(  "Can't set Debayer mode to VmbDebayerMode2x2: %s " , ErrCodeToMessage(Res) ) ;
              }
            }
            else
            {
              SENDERR_1(  "Can't set DEST pixel format to VmbPixelFormatBgr8: %s " , ErrCodeToMessage(Res) ) ;
            }
          }
          else
          {
            SENDERR_1(  "Can't set SRC pixel format VmbPixelFormatBayerRG8: %s " , ErrCodeToMessage(Res) ) ;
          }
        }
      }
      else if ( m_ColorMode == CM_YUV411 )
      {
        frame->lpBMIH = yuv411yuv9( &m_RealBMIH , data );
      }
      else if ( m_ColorMode == CM_RGB8 )
      {

      }
      else if ( m_ColorMode == CM_YUV422 )
      {
        TVFrame Tmp ;
        Tmp.lpBMIH = &m_RealBMIH ;
        m_RealBMIH.biCompression = BI_UYVY ;
        Tmp.lpData = CurFrame.pData ;
        frame->lpBMIH = _convertUYVY2YUV12( &Tmp );
      }

    }
    m_LastError=m_Camera.PutFrame(&CurFrame);
  }
  else if ( !IsTriggerByInputPin() && m_iNGrabSet < 0 )
  {
    double dCurrentTime = GetHRTickCount() ;
    if ( !m_dRestartRequested && m_iRestartRetryCnt < 3 )
    {
      if ( ( m_dRestartedTime == 0.)  ||  ((m_dRestartedTime - dCurrentTime) > 2000) )
      {
      if ( dCurrentTime - m_dLastFrameTime > 3./m_dAverageConstant )
      {
        m_dRestartRequested = dCurrentTime - 1.0 ;
      }
    }
    }
    if ( m_dRestartRequested != 0. && m_iRestartRetryCnt < 3 )
    {
      if ( dCurrentTime > m_dRestartRequested )
      {
        if ( RestartDevice() )
        {
          m_dRestartRequested = 0. ;
          m_dRestartedTime = dCurrentTime ;
          SetGrab( -1 ) ;// forever
        }
        else
          m_dRestartRequested = dCurrentTime + 1000. ;
      }
    }
  }
  if ((frame)&&(frame->lpBMIH))
  {
    CVideoFrame* vf=CVideoFrame::Create(frame);
    vf->SetLabel(m_CameraID);
    return vf;
  }
  else if (frame)
    free(frame);
  return NULL;
}

bool AVTF::IsTriggerByInputPin()
{
  UINT32 TriggerValue;
  // Get current trigger mode
  m_Camera.GetParameter(FGP_TRIGGER,&TriggerValue);
  return (TRGON(TriggerValue)!=0);
}
int AVTF::GetTriggerMode()
{
  UINT32 TriggerValue , IO1 , IO2 ;
  m_Camera.ReadRegister( 0xf0f00830 , &TriggerValue ) ;
  // Get current trigger mode
  //m_Camera.GetParameter(FGP_TRIGGER,&TriggerValue);
  int iOn = TRGON(TriggerValue) ;
  if ( !iOn )
    return 0 ;
  m_Camera.ReadRegister( 0xf1000300 , &IO1 ) ;
  m_Camera.ReadRegister( 0xf1000304 , &IO2 ) ;
  int iPol = TRGPOL(TriggerValue) ;
  int iSrc = ( IO2 & 0x00020000 ) ? 1 : 0 ;
  if ( iSrc )
    return (iPol) ? 4 : 3 ;
  else
    return (iPol) ? 2 : 1 ;
}

void AVTF::SetTriggerMode(int iMode)
{
  FGPSTRIGGER TriggerStatus ;
  m_Camera.GetParameter( FGP_TRIGGER , (unsigned long*)&TriggerStatus ) ;
  // iMode : 0 - no trigger, 1 - channel 1 no inverse , 2 - channel 1 inverse
  //                         3 - channel 2 no inverse , 4 - channel 2 inverse
  UINT32 nOn=(iMode)?1:0; // 0=ext. trigger off, else ext. trigger on
  if ( !nOn )
  {
    UINT32 TriggerValue = MAKETRIGGER(0, 0, TriggerStatus.TriggerSrc , 
      TriggerStatus.TriggerMode , TriggerStatus.TriggerParameter );
    TriggerStatus.OnOffState = 0 ;
    m_Camera.SetParameter(FGP_TRIGGER, TriggerValue ) ;
    return ;
  }
  else
  {
    //UINT32 nMode=0; // 0=edge mode, 1=level mode, 15=bulk mode
    //UINT32 nParm=0; // not currently applicable to AVT cameras, as of February 2008 - parameter inherited from IIDC/DCAM
//     UINT32 TrigReg , IO1 , IO2 ;
//     m_Camera.ReadRegister( 0xf0f00830 , &TrigReg ) ;
//     m_Camera.ReadRegister( 0xf1000300 , &IO1 ) ;
//     m_Camera.ReadRegister( 0xf1000304 , &IO2 ) ;
    UINT32 nSrc=(iMode > 2) ? 1 : 0; // not currently applicable to AVT cameras, as of February 2008 - parameter inherited from IIDC/DCAM
//     UINT32 uiAddrOn = 0xf1000300 + (nSrc * 4) ;
//     UINT32 uiAddrOff = 0xf1000300 + ((nSrc ^ 1) * 4) ;
    UINT32 iPol = (iMode % 2) == 0 ;
    // Check and set polarity together with trigger mode
//     UINT32 uiControlIO = 0x00020000 ;

    UINT32 TriggerValue=MAKETRIGGER( 1 , iPol , nSrc , 0 , 0 ) ;
    m_Camera.SetParameter( FGP_TRIGGER , TriggerValue );
//     m_Camera.WriteRegister( uiAddrOn , uiControlIO ) ;
//     m_Camera.WriteRegister( uiAddrOff , 0 ) ;
//     m_Camera.WriteRegister( 0xf0f00830 , 0x02000000 | (iPol ? 0x01000000 : 0 ) ) ;
    // Enable ext. trigger, edge mode (0), falling edge
  }
}

void AVTF::GetROI(CRect& rc)
{
  m_Camera.GetParameter( FGP_XSIZE , (UINT32*)&m_CurrentROI.right ) ;
  m_Camera.GetParameter( FGP_YSIZE , (UINT32*)&m_CurrentROI.bottom ) ;
  m_Camera.GetParameter( FGP_XPOSITION , (UINT32*)&m_CurrentROI.left ) ;
  m_Camera.GetParameter( FGP_YPOSITION , (UINT32*)&m_CurrentROI.top ) ;

  rc = m_CurrentROI;
}

bool AVTF::SetROI(CRect& rc)
{
  if (m_Resolution != RES_SCALABLE)
  {
    DEVICESENDERR_0("Can't crop image. Reason: The camera format isn't scalable");
    return false;
  }
  int uiMaxWidth = GetMaxWidth(m_Camera);
  int uiMaxHeight = GetMaxHeight(m_Camera);
  if (rc.left!=-1) //must be divisible by 4
  {
    rc.left&=~3;
    rc.right&=~3;
    rc.bottom&=~3;
    rc.top&=~3;
  }
  else
  {
    rc.left = rc.top = 0 ;
    rc.right = uiMaxWidth ;
    rc.bottom = uiMaxHeight ;
  }
  if ( rc.left < 0 )
    rc.left = 0 ;
  if ( rc.left >= uiMaxWidth )
    rc.left = 0 ;
  if ( rc.top < 0 )
    rc.top = 0 ;
  if ( rc.top > uiMaxHeight )
    rc.top = 0 ;
  if ( rc.left + rc.right > uiMaxWidth )
    rc.right = uiMaxWidth - rc.left ;
  if ( rc.left + rc.right < 0 )
  {
    rc.left = 0 ;
    rc.right = uiMaxWidth ;
  }
  if ( rc.top + rc.bottom > uiMaxHeight )
    rc.bottom = uiMaxHeight - rc.top ;
  if ( rc.top + rc.bottom < 0 )
  {
    rc.top = 0 ;
    rc.bottom = uiMaxHeight ;
  }
  if (   (rc.Width() == m_CurrentROI.Width()) 
    && (rc.Height() == m_CurrentROI.Height()) )
  {     // if format 7 and size the same simply do origin shift
    // by writing into IMAGE_ORIGIN register
    // without grab stop
    m_Camera.WriteRegister( 0xf0f08008 , (rc.left << 16) | rc.top ) ;
    m_CurrentROI = rc;
    return true;
  }
  int iOldSize = m_CurrentROI.Width() * m_CurrentROI.Height() ;
  int iNewSize = rc.right * rc.bottom ;

  //   m_LastError = m_Camera.SetParameter( FGP_RESIZE , 1 ) ;
  //   if (m_LastError!=FCE_NOERROR)
  //   {
  //     DEVICESENDERR_1("SetROI: Can't enter into resize mode: %s", GetErrorMessage(m_LastError));
  // //     
  // //     DWORD ImageFormat ;
  // //     m_LastError = m_Camera.GetParameter( FGP_IMAGEFORMAT , &ImageFormat ) ;
  // //     if ( ISDCAMFORMAT(ImageFormat) )
  // //     {
  // //       UINT32 Format = DCAMFORMAT(ImageFormat);
  // //       UINT32 Mode = DCAMMODE(ImageFormat);
  // //       m_ColorMode = (FG_COLORMODE)DCAMRATE(ImageFormat); 
  // //       if ( Format != 7 || Mode != 0 )
  // //       {
  // // //         UINT32 DCAM_Image_Format = MAKEDCAMFORMAT( 7 , 0 , m_ColorMode ) ;
  // // //         m_LastError = m_Camera.SetParameter( FGP_IMAGEFORMAT , DCAM_Image_Format ) ;
  // //         UINT32 ImgFormat = MAKEIMAGEFORMAT( RES_SCALABLE , m_ColorMode , 0 ) ; // last zero is mode0
  // //         m_LastError = m_Camera.SetParameter( FGP_IMAGEFORMAT , ImgFormat ) ;
  // //         if ( m_LastError != FCE_NOERROR )
  // //         {
  // //           DEVICESENDERR_1("DeviceInit error - Can't reprogram format 7 for image %s",GetErrorMessage( m_LastError) );
  // //         }
  // //       }
  // //     }
  // //     else
  // //     {
  // //       FG_RESOLUTION Resolution = (FG_RESOLUTION)IMGRES( ImageFormat ) ;
  // //       m_ColorMode = (FG_COLORMODE) IMGCOL(ImageFormat) ; // color info
  // //       FG_FRAMERATE FrameRate = (FG_FRAMERATE)IMGRATE(ImageFormat) ;
  // // //       UINT32 DCAM_Image_Format = MAKEDCAMFORMAT( 7 , 0 , m_ColorMode ) ;
  // // //       m_LastError = m_Camera.SetParameter( FGP_IMAGEFORMAT , DCAM_Image_Format ) ;
  // //       UINT32 ImgFormat = MAKEIMAGEFORMAT( RES_SCALABLE , m_ColorMode , 0 ) ; // last zero is mode0
  // //       m_LastError = m_Camera.SetParameter( FGP_IMAGEFORMAT , ImgFormat ) ;
  // //       if ( m_LastError != FCE_NOERROR )
  // //       {
  // //         DEVICESENDERR_1("DeviceInit error - Can't set format 7 for image %s",GetErrorMessage( m_LastError) );
  // //       }
  // //     }
  //   }

  FXAutolock al(m_Lock);
  bool wasRunning=IsRunning();
  if ( wasRunning )
  {
    //     if ( iNewSize > iOldSize )
    //     {
    DeviceStop();
    Sleep( 80 ) ;
    m_bWasStopped = true ;
    //     }
    //     else 
    //     {
    //       m_LastError = m_Camera.SetParameter( FGP_RESIZE , 1 ) ;
    //       if (m_LastError!=FCE_NOERROR)
    //         DEVICESENDERR_1("SetROI: Can't enter into resize mode: %s", GetErrorMessage(m_LastError));
    //     }
  }

  int iNReprogrammed = 0 ;
  m_LastError=m_Camera.SetParameter(FGP_XPOSITION,rc.left) ;
  if (m_LastError!=FCE_NOERROR)
    DEVICESENDERR_1("SetROI error on XPos setting: %s", GetErrorMessage(m_LastError));
  else
  {
    m_CurrentROI.left = rc.left ;
    iNReprogrammed++ ;
    m_LastError=m_Camera.SetParameter(FGP_XSIZE,rc.right) ;
    if (m_LastError!=FCE_NOERROR)
      DEVICESENDERR_1("SetROI error on XSize setting: %s", GetErrorMessage(m_LastError));
    else
    {
      m_CurrentROI.right = rc.right ;
      iNReprogrammed++ ;
      m_LastError=m_Camera.SetParameter(FGP_YPOSITION,rc.top) ;
      if (m_LastError!=FCE_NOERROR)
        DEVICESENDERR_1("SetROI error on YPos setting: %s", GetErrorMessage(m_LastError));
      else
      {
        m_CurrentROI.top = rc.top ;
        iNReprogrammed++ ;
        m_LastError=m_Camera.SetParameter(FGP_YSIZE , rc.bottom) ; 
        if (m_LastError!=FCE_NOERROR)
          DEVICESENDERR_1("SetROI error on YSize setting: %s", GetErrorMessage(m_LastError));
        else
        {
          m_CurrentROI.bottom = rc.bottom ;
          iNReprogrammed++ ;
          double dPacketSizeRatio = (double)iNewSize / (DOUBLE)iOldSize ;
          m_Camera.GetParameter( FGP_PACKETSIZE , &m_PacketSize.IsValue ) ;
          int iNewPacketSize = ROUND( (m_PacketSize.IsValue * dPacketSizeRatio *m_PacketSize.Unit) 
            / (double)m_PacketSize.Unit ) ;
          m_Camera.SetParameter( FGP_PACKETSIZE , m_PacketSize.IsValue = iNewPacketSize ) ;
        }
      }
    }
  }
  if ( wasRunning && !m_bInScanProperties )
  {
    //     if ( iNewSize > iOldSize )
    //     {
      DeviceStart();
    //     }
    //     else 
    //     {
    //       m_LastError = m_Camera.SetParameter( FGP_RESIZE , 0 ) ;
    //       if (m_LastError!=FCE_NOERROR)
    //         DEVICESENDERR_1("SetROI: Can't exit from resize mode: %s", GetErrorMessage(m_LastError));
    //     }
  }
  if ( m_LastError != FCE_NOERROR )
  {
    DEVICESENDERR_1("Error in SetROI(): %s", GetErrorMessage(m_LastError));
    return false;        
  }
  return true ;
}
int AVTF::GetLongExposure()
{
  UINT32 iExp_usec ;
  m_Camera.ReadRegister(0xF100020c,&iExp_usec);
  return (iExp_usec & 0x03ffffff);
}

void AVTF::SetLongExposure( int iExp_usec)
{
  m_Camera.WriteRegister(0xF100020c,iExp_usec);
}

DWORD AVTF::IsFrameInfoAvailable()
{
  DWORD iAvail ;
  m_Camera.ReadRegister(0xF1000630,&iAvail);
  if ( iAvail & 0x80000000 )
    return (iAvail & 0x0200ffff ) ;
  else
    return 0 ;
}

void AVTF::SetSendFrameInfo( int iSend)
{
  m_Camera.WriteRegister(0xF1000630 , (iSend) ? 0x02000000 : 0 );
}

int AVTF::GetTriggerDelay()
{
  UINT32 iDelay_usec ;
  m_Camera.ReadRegister(0xF1000400,&iDelay_usec);
  return iDelay_usec ;
}

void AVTF::SetTriggerDelay( int iDelay_uS)
{
  if ( iDelay_uS )
    iDelay_uS = 0x02000000 | (iDelay_uS & 0x001fffff) ;
  m_Camera.WriteRegister(0xF1000400,iDelay_uS);
}

bool AVTF::SetGrab( int iNFrames )
{
  if (  (iNFrames < -1)  
    ||  (iNFrames > 0xffff) )
    return false ;
  if ( iNFrames == -1 )
    m_Camera.WriteRegister(0xF0f00614,0x80000000);
  else 
  {
    m_Camera.WriteRegister(0xF0f00614,0x00000000);
    if ( iNFrames == 1 )
      m_Camera.WriteRegister(0xF0f0061c,0x80000000);
    else if ( iNFrames > 0 )
      m_Camera.WriteRegister(0xF0f0061c,
      0x40000000 + (iNFrames & 0x0000ffff));
    else
      m_Camera.WriteRegister(0xF0f0061c,0x00000000);
  }
  m_iNGrabSet = iNFrames;
  return true ;
}

bool AVTF::GetGrabConditions( 
  bool& bContinuous , INT_PTR& iNRestFrames ) 
{
  UINT32 uiVal = 0 ;
  m_Camera.ReadRegister( 0xF0f00614 , &uiVal );
  if ( uiVal & 0x80000000 )
  {
    bContinuous = true ;
    iNRestFrames = 0 ;
    return true ;
  }
  else
  {
    bContinuous = false ;
    m_Camera.ReadRegister( 0xF0f0061c , &uiVal );
    if ( uiVal & 0x80000000 )
      iNRestFrames = 1 ;
    else if ( uiVal & 0x40000000 )
      iNRestFrames = uiVal & 0xffff ;
    else
      iNRestFrames = 0 ;
  }
  return true ;
}


bool AVTF::UpdateBMIH(void)
{
  m_Camera.GetParameter( FGP_COLORFORMAT , (UINT32*)&m_ColorMode ) ;
  m_BMIH.biSize=sizeof(BITMAPINFOHEADER);
  m_BMIH.biWidth=GetXSize(m_Camera);
  m_BMIH.biHeight=GetYSize(m_Camera);
  m_BMIH.biSizeImage = m_BMIH.biWidth * m_BMIH.biHeight ;
  unsigned XOff=GetXOffset(m_Camera);
  unsigned YOff=GetYOffset(m_Camera);
  m_CurrentROI=CRect(XOff,YOff,XOff+m_BMIH.biWidth,YOff+m_BMIH.biHeight);
  m_BMIH.biPlanes = 1 ;
  double dSizeCoeff ; 
  bool bRes = GetBitCountAndSizeCoeff( m_ColorMode , m_BMIH.biBitCount , dSizeCoeff ) ;
  switch(m_ColorMode)
  {
  case CM_RAW8:
  case CM_Y8    : 
    m_BMIH.biCompression=BI_Y8;
    m_BMIH.biBitCount=8;
    m_BMIH.biSizeImage=m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case CM_YUV411:
    m_BMIH.biCompression=BI_YUV411;
    m_BMIH.biBitCount = 12 ;
    m_BMIH.biSizeImage = 3*m_BMIH.biWidth*m_BMIH.biHeight/2 ;
    break;
  case CM_YUV422:
    m_BMIH.biCompression = BI_YUV422;
    m_BMIH.biBitCount = 16 ;
    m_BMIH.biSizeImage = 2*m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case CM_RAW16:
  case CM_Y16   : 
    m_BMIH.biCompression=BI_Y16;
    m_BMIH.biBitCount=16;
    m_BMIH.biSizeImage=2*m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case CM_YUV444:
  case CM_RGB8  :
  case CM_RGB16 :

  default: 
    m_BMIH.biSize=0;
    TRACE("!!! Unsupported color mode #%d\n", m_ColorMode);
    DEVICESENDERR_1("!!! Unsupported color mode #%d",m_ColorMode);
    return false;
  }
  return true ;
}


bool AVTF::GetBitCountAndSizeCoeff( FG_COLORMODE ColorMOde , WORD& iBitCount, double& dSizeCoeff)
{
  switch( ColorMOde )
  {
  case CM_RAW8:
  case CM_Y8    : 
    iBitCount = 8 ;
    dSizeCoeff = 1. ;
    break;
  case CM_YUV411:
    iBitCount = 12 ;
    dSizeCoeff = 1.5 ;
    break;
  case CM_RAW16:
  case CM_Y16   : 
  case CM_YUV422:
    iBitCount = 16 ;
    dSizeCoeff = 2.0 ;
    break;
  case CM_YUV444:
  case CM_RGB8  :
    iBitCount = 24 ;
    dSizeCoeff = 3.0 ;
    break ;
  case CM_RGB16 :
    iBitCount = 48 ;
    dSizeCoeff = 6.0 ;
    break ;

  default: 
    return false ;
  }
  return true ;
}


bool AVTF::RestartDevice(void)
{
  bool bWasRunning = IsRunning() ;
  if ( bWasRunning )
    DeviceStop() ;
  Sleep( 50 ) ;
  DeviceClose() ;
  DeviceInit() ;
  if ( IsDeviceConnected() )
  {
    UINT32 Data  , Result = FCE_NOERROR ;
    if ( m_Camera.ReadRegister( 0xf1000550 , &Data ) == FCE_NOERROR )
    {
      Result = m_Camera.WriteRegister( 0xf1000550 , 0x80400001 ) ; // restore from profile 1
      if ( Result != FCE_NOERROR )
      {
        DEVICESENDERR_1("Error on restore from profile 1: ", GetErrorMessage( Result ) );
      }
    }
    if ( bWasRunning )
      return DeviceStart() ;
    else
      return ( Result == FCE_NOERROR ) ;
  }
  DEVICESENDERR_0( "Can't restart device: no connection" ) ;
  return false;
}
