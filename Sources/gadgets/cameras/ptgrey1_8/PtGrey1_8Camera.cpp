// PtGrey1_81.cpp: implementation of the PtGrey1_8 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ptgrey1_8.h"
#include "PtGrey1_8Camera.h"
#include <gadgets\stdsetup.h>
#include <video\shvideo.h>
#include <cameraerrors.h>
#include <PGRFlyCapturePlus.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment( lib, "PGRFlyCapture.lib" )



class CCamerasStartedArray: public CArray<unsigned,unsigned>
{
public:
  FXLockObject m_lo;
};

CCamerasStartedArray CamerasStarted;


__forceinline bool _isCameraStarted(unsigned ID)
{
  for (int i=0; i<CamerasStarted.GetSize(); i++)
  {   
    if (CamerasStarted[i]==ID) return true;
  }
  return false;
}

bool _AddCameraToList(unsigned ID)
{
  FXAutolock al(CamerasStarted.m_lo);
  if (!_isCameraStarted(ID))
  {
    CamerasStarted.Add(ID);
    return true;
  }
  return false;
}

void _RemoveCameraFromList(unsigned ID)
{
  FXAutolock al(CamerasStarted.m_lo);
  for (int i=0; i<CamerasStarted.GetSize(); i++)
  {   
    if (CamerasStarted[i]==ID) 
    {
      CamerasStarted.RemoveAt(i);
      return;
    }
  }
}

#define THIS_MODULENAME "PtGrey1_8.PtGrey1_8Camera.cpp"

typedef struct tagVideoformat
{
  FlyCaptureVideoMode vm;
  const char *        name; 
}Videoformat;

typedef struct tagFramerates
{
  FlyCaptureFrameRate fr;
  const char *        name; 
}Framerates;

typedef struct tagCamProperties
{
  FlyCaptureProperty  pr;
  const char *        name; 
}CamProperties;

Videoformat vFormats[]=
{
  {FLYCAPTURE_VIDEOMODE_320x240YUV422,"640 x 480 color"},
  {FLYCAPTURE_VIDEOMODE_640x480YUV422,"640 x 480 color"},
  {FLYCAPTURE_VIDEOMODE_640x480Y8, "640 x 480 mono"},
  {FLYCAPTURE_VIDEOMODE_800x600Y8, "800 x 600 mono"},
  {FLYCAPTURE_VIDEOMODE_CUSTOM, "Custom Video Mode"}
};

Framerates fRates[]= 
{
  {FLYCAPTURE_FRAMERATE_1_875,"1.875 fps."},
  {FLYCAPTURE_FRAMERATE_3_75,"3.75 fps."},
  {FLYCAPTURE_FRAMERATE_7_5,"7.5 fps."},
  {FLYCAPTURE_FRAMERATE_15,"15 fps."},
  {FLYCAPTURE_FRAMERATE_30,"30 fps."},
  {FLYCAPTURE_FRAMERATE_60,"60 fps."},
  {FLYCAPTURE_FRAMERATE_120,"120 fps."},
  {FLYCAPTURE_FRAMERATE_240,"240 fps."}
};

const FlyCaptureProperty FLYCAPTURE_VIDEOFORMAT     =((FlyCaptureProperty)(-1));
const FlyCaptureProperty FLYCAPTURE_FRAMERATE       =((FlyCaptureProperty)(-2));
const FlyCaptureProperty FLYCAPTURE_TRIGGER         =((FlyCaptureProperty)(-3));
const FlyCaptureProperty FLYCAPTURE_TRIGGERPOLARITY =((FlyCaptureProperty)(-4));
const FlyCaptureProperty FLYCAPTURE_FRAME_INFO      =((FlyCaptureProperty)(-5));
const FlyCaptureProperty FLYCAPTURE_ROI             =((FlyCaptureProperty)(-6));

CamProperties cProperties[] =
{
  {FLYCAPTURE_VIDEOFORMAT, "VideoFormat"},
  {FLYCAPTURE_FRAMERATE,   "FrameRate"},
  {FLYCAPTURE_BRIGHTNESS,  "Brightness"},
  {FLYCAPTURE_AUTO_EXPOSURE,"Exposure"},
  {FLYCAPTURE_SHARPNESS,   "Sharpness"},
  //{FLYCAPTURE_WHITE_BALANCE,"Whitebalance"},
  {FLYCAPTURE_SOFTWARE_WHITEBALANCE,"WhiteBalance"},
  {FLYCAPTURE_HUE,          "Hue"},
  {FLYCAPTURE_SATURATION,  "Saturation"},
  {FLYCAPTURE_GAMMA,       "Gamma"},
  {FLYCAPTURE_IRIS,        "Iris"},
  {FLYCAPTURE_FOCUS,       "Focus"},
  {FLYCAPTURE_ZOOM,        "Zoom"},
  {FLYCAPTURE_PAN,         "Pan"},
  {FLYCAPTURE_TILT,        "Tilt"},
  {FLYCAPTURE_SHUTTER,     "Shutter"},
  {FLYCAPTURE_GAIN,        "Gain"},
  {FLYCAPTURE_TRIGGER_DELAY,"Triggerdelay"},
  //    {FLYCAPTURE_FRAME_RATE,  "Framerate"},
  {FLYCAPTURE_SOFTWARE_WHITEBALANCE,"SWWhitebalance"},
  {FLYCAPTURE_ROI,         "ROI"},
  {FLYCAPTURE_TRIGGER,     "Trigger"},
  {FLYCAPTURE_TRIGGERPOLARITY,"TriggerPolarity"},
  {FLYCAPTURE_FRAME_INFO,"FrameInfo"}
};


IMPLEMENT_RUNTIME_GADGET_EX(PtGrey1_8, C1394Camera, "Video.capture", TVDB400_PLUGIN_NAME);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PtGrey1_8::PtGrey1_8():
  m_flyCaptureContext(NULL),
  m_CurrentVideoMode(FLYCAPTURE_VIDEOMODE_ANY),
  m_CurrentFrameRate(FLYCAPTURE_FRAMERATE_15),
  m_CurrentROI(-1,-1,-1,-1)
{
  m_GadgetInfo="PtGrey1_8 gadget";
}
void PtGrey1_8::ShutDown()
{
  CameraClose();
  C1394Camera::ShutDown();
  if (m_flyCaptureContext)
  {
    VERIFY(::flycaptureDestroyContext(m_flyCaptureContext)==FLYCAPTURE_OK);
    m_flyCaptureContext=NULL;
  }
}

bool PtGrey1_8::DriverInit()
{
  if (!m_flyCaptureContext)
  {
    int iCurrentversion = flycaptureGetLibraryVersion() ;
    if ( iCurrentversion < PGRFLYCAPTURE_VERSION)
    {
      m_Status.Format(CAMERA_ERROR_WRONGVERSION, 
        iCurrentversion ,PGRFLYCAPTURE_VERSION);
      C1394_SENDERR_1("Error: %s",m_Status);
      //return false;
    }

    m_Error = ::flycaptureCreateContext( &m_flyCaptureContext );
    if (m_Error!=FLYCAPTURE_OK)
    {
      m_Status.Format(CAMERA_ERROR_FATAL_ERROR,::flycaptureErrorToString( m_Error ));
      C1394_SENDERR_1("Fatal error: %s", m_Status);
      m_flyCaptureContext=NULL;
      return false;
    }
  }
  EnumCameras();
  m_Status.Format("ScanBus(): %s",::flycaptureErrorToString( m_Error ));
  C1394_SENDINFO_0(m_Status);
  return true;
}

bool PtGrey1_8::EnumCameras()
{
  m_CamerasOnBus=sizeof(m_CamerasInfo)/sizeof(m_CamerasInfo[0]);
  FlyCaptureInfoEx     CamerasInfo[MAX_CAMERASNMB];
  m_Error = ::flycaptureBusEnumerateCamerasEx( CamerasInfo, &m_CamerasOnBus);
  if (m_Error!=FLYCAPTURE_OK)
  {
    m_CamerasOnBus=0;
    m_Status.Format(CAMERA_ERROR_FATAL_ERROR,::flycaptureErrorToString( m_Error ));
    C1394_SENDERR_1("Fatal error: %s", m_Status);
    if (m_flyCaptureContext) VERIFY(::flycaptureDestroyContext(m_flyCaptureContext)==FLYCAPTURE_OK);
    m_flyCaptureContext=NULL;
    return false;
  }
  if (!m_CamerasOnBus) 
  {
    m_Status=CAMERA_ERROR_NOCAMERA;
    C1394_SENDERR_1("Fatal error: %s",m_Status);
    if (m_flyCaptureContext) VERIFY(::flycaptureDestroyContext(m_flyCaptureContext)==FLYCAPTURE_OK);
    m_flyCaptureContext=NULL;
    return false;
  }
  else
  {
    for (unsigned i=0; i<m_CamerasOnBus; i++)
    {
      m_CamerasInfo[i].name= CamerasInfo[i].pszModelName;
      m_CamerasInfo[i].serialnmb=CamerasInfo[i].SerialNumber;
    }
  }
  return true;
}

bool PtGrey1_8::BuildPropertyList()
{
  FlyCaptureError Err = flycaptureGetCameraInfo(
    m_flyCaptureContext , &m_CameraInfo ) ;

  m_Properties.RemoveAll();
  int propCount=sizeof(cProperties)/sizeof(CamProperties);
  CAMERA1394::Property P;
  for (int i=0; i<propCount; i++)
  {
    if ((int)cProperties[i].pr<0) // virtual property
    {
      switch (cProperties[i].pr)
      {
      case FLYCAPTURE_VIDEOFORMAT:
        {
          int iNmb=sizeof(vFormats)/sizeof(Videoformat);
          CString items,tmpS;
          for (int j=0; j<iNmb; j++)
          {
            bool supported;
            FlyCaptureFrameRate  frameRate=FLYCAPTURE_FRAMERATE_ANY;
            if ((vFormats[j].vm == FLYCAPTURE_VIDEOMODE_CUSTOM) 
              || ((flycaptureCheckVideoMode(m_flyCaptureContext,
              vFormats[j].vm,frameRate,&supported)==FLYCAPTURE_OK) && supported) )
            {
              tmpS.Format("%s(%d),",vFormats[j].name,vFormats[j].vm);
              items+=tmpS;
            }
          }
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("ComboBox(%s(%s))",P.name,items);
          m_Properties.Add(P);
          break;
        }
      case FLYCAPTURE_FRAMERATE:
        {
          if ((m_CurrentVideoMode!=FLYCAPTURE_VIDEOMODE_CUSTOM) 
            && (m_CurrentVideoMode!=FLYCAPTURE_VIDEOMODE_ANY))
          {
            int iNmb=sizeof(fRates)/sizeof(Framerates);
            CString items,tmpS;
            for (int j=0; j<iNmb; j++)
            {
              bool supported;
              if ((flycaptureCheckVideoMode(m_flyCaptureContext,
                m_CurrentVideoMode,fRates[j].fr,&supported)==FLYCAPTURE_OK) && supported)
              {
                tmpS.Format("%s(%d),",fRates[j].name,fRates[j].fr);
                items+=tmpS;
              }
            }
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("ComboBox(%s(%s))",P.name,items);
            m_Properties.Add(P);
          }
          break;
        }
      case FLYCAPTURE_TRIGGER:
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("ComboBox(%s(On(1),Off(0)))",P.name);
          m_Properties.Add(P);
          break;
        }
      case FLYCAPTURE_TRIGGERPOLARITY:
        {
          if (IsTriggerMode())
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("ComboBox(%s(Plus(1),Minus(0)))",P.name);
            m_Properties.Add(P);
          }
          break;
        }
      case FLYCAPTURE_FRAME_INFO:
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("ComboBox(%s(On(1),Off(0)))",P.name);
          m_Properties.Add(P);
        }
        break;
      case FLYCAPTURE_ROI:
        if (m_CurrentVideoMode==FLYCAPTURE_VIDEOMODE_CUSTOM)
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("EditBox(%s)",P.name);
          m_Properties.Add(P);
        }
        break;
      default:
        C1394_SENDERR_1("Undefined property:'%s'",cProperties[i].name);
      }
    }
    else
    {
      bool    bPresent;
      long    lMin;
      long    lMax;
      long    lDefault;
      bool    bAuto;
      bool    bManual;
      if ((cProperties[i].pr==FLYCAPTURE_PAN) && (m_CurrentVideoMode==FLYCAPTURE_VIDEOMODE_CUSTOM))
        continue;
      if ((flycaptureGetCameraPropertyRange(m_flyCaptureContext, cProperties[i].pr, 
        &bPresent,&lMin,&lMax,&lDefault,&bAuto,&bManual)==FLYCAPTURE_OK) 
        && bPresent
        && (lMin!=lMax))
      {
        //TRACE("Found property %s, min=%d, max=%d, auto - %s\n",cProperties[i].name,lMin,lMax,(bAuto)?"Supported":"Not supported");
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        P.property.Format("%s(%s,%d,%d)",(bAuto)?SETUP_SPINABOOL:SETUP_SPIN,P.name,lMin,lMax);
        m_Properties.Add(P);
      }
    }
  }
  return true;
}

bool PtGrey1_8::CameraInit()
{
  m_Error=FLYCAPTURE_OK;

  CameraClose();
  DriverInit();

  if (m_CurrentCamera!=-1)
  {
    m_Error = ::flycaptureInitializeFromSerialNumber(m_flyCaptureContext,m_CamerasInfo[m_CurrentCamera].serialnmb);
    if (m_Error != FLYCAPTURE_OK)
    {
      m_Status.Format("Fatal Error: %s",::flycaptureErrorToString( m_Error ));
      C1394_SENDERR_1("Fatal error %s", m_Status);
      if (m_flyCaptureContext) 
        VERIFY(::flycaptureDestroyContext(m_flyCaptureContext)==FLYCAPTURE_OK);
      m_flyCaptureContext=NULL;
      return false;
    }
    m_CurrentCamera=SerialToNmb(m_CamerasInfo[m_CurrentCamera].serialnmb);
  }
  else 
  {
    m_CurrentCamera=GetInstanceNmb()-1;
  }
  m_Error = ::flycaptureInitializePlus(m_flyCaptureContext,m_CurrentCamera,4,NULL );
  if (m_Error!=FLYCAPTURE_OK)
  {
    C1394_SENDERR_1("Fatal error: %s", ::flycaptureErrorToString( m_Error ));
    m_Status.Format(CAMERA_ERROR_FATAL_ERROR,::flycaptureErrorToString( m_Error ));
    if (m_flyCaptureContext) 
      VERIFY(::flycaptureDestroyContext(m_flyCaptureContext)==FLYCAPTURE_OK);
    m_flyCaptureContext=NULL;
    return false;
  } 
  m_Status.Format("CameraInit(): %s",::flycaptureErrorToString( m_Error ));
  // prepare list of properties for current camera
  m_CameraID.Format("%s_%d",m_CamerasInfo[m_CurrentCamera].name,m_CamerasInfo[m_CurrentCamera].serialnmb);
  m_GadgetInfo.Format("PtGrey1_8 gadget: %s Sn: %d",m_CamerasInfo[m_CurrentCamera].name,m_CamerasInfo[m_CurrentCamera].serialnmb);
  BuildPropertyList();

  C1394_SENDINFO_0(m_Status);
  return true;
}

void PtGrey1_8::CameraClose()
{
  if (IsTriggerMode())
  {
    bool   bOnOff; int iPolarity,iSource,iRawValue,iMode, iParameter;
    ::flycaptureGetTrigger(m_flyCaptureContext,&bOnOff,&iPolarity,&iSource,&iRawValue,&iMode,&iParameter);
    bOnOff=false;
    bool res = (::flycaptureSetTrigger(m_flyCaptureContext,bOnOff, iPolarity,iSource, iMode,iParameter)==FLYCAPTURE_OK);
  }
  if (IsRunning()) 
    CameraStop();
  if (m_flyCaptureContext) 
  {
    VERIFY(::flycaptureDestroyContext(m_flyCaptureContext)==FLYCAPTURE_OK);
    m_flyCaptureContext=NULL;
  }
  m_Status.Format("CameraClose(): %s",::flycaptureErrorToString( m_Error ));
  C1394_SENDINFO_0(m_Status);
  m_CameraID="PtGrey1_8 gadget";
}

CVideoFrame* PtGrey1_8::CameraDoGrab(double* StartTime)
{
  FlyCaptureImagePlus  imageP = { 0 };
  if ((m_flyCaptureContext==NULL) || (!IsRunning()))
  { // return empty frame 
    Sleep(100);
    return NULL;
  }
  m_Lock.Lock();
  try
  {
    m_Error = ::flycaptureLockNext( m_flyCaptureContext, &imageP );
  }
  catch(...)
  {
    m_Error=FLYCAPTURE_INVALID_CONTEXT;
  }
  *StartTime=::GetHRTickCount();

  if( m_Error == FLYCAPTURE_OK )
  {
    pTVFrame frame=(pTVFrame)malloc(sizeof(TVFrame));
    bool bBayer = (imageP.image.pixelFormat == FLYCAPTURE_RAW8 
                || imageP.image.pixelFormat == FLYCAPTURE_RAW16 
                || imageP.image.pixelFormat == FLYCAPTURE_MONO8 
                || imageP.image.pixelFormat == FLYCAPTURE_MONO16 
                  ) && imageP.image.bStippled ;
    if ( !bBayer )
//      if ( m_CameraInfo.CameraType != FLYCAPTURE_COLOR )
    {
      unsigned iSize=imageP.image.iRows*imageP.image.iCols;
      frame->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+iSize);
      frame->lpData=NULL;
      LPBYTE data=((LPBYTE)frame->lpBMIH)+sizeof(BITMAPINFOHEADER);

      memset(frame->lpBMIH,0,sizeof(BITMAPINFOHEADER));
      frame->lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
      frame->lpBMIH->biHeight=imageP.image.iRows;
      frame->lpBMIH->biWidth=imageP.image.iCols;
      frame->lpBMIH->biSizeImage=iSize;
      frame->lpBMIH->biCompression=BI_Y8;
      frame->lpBMIH->biBitCount=0;
      frame->lpBMIH->biPlanes=1;

      memcpy(data,imageP.image.pData,frame->lpBMIH->biHeight*frame->lpBMIH->biWidth);
    }
    else // color camera 
    {
      FlyCaptureImage RGB ;
      memset( &RGB , 0 , sizeof(RGB) ) ;
      RGB.iCols = imageP.image.iCols ;
      RGB.iRows = imageP.image.iRows ;
      RGB.iNumImages = 1 ;
      RGB.pixelFormat = FLYCAPTURE_BGR ;
      RGB.iRowInc = imageP.image.iCols * 3 ;
      DWORD dwImageSize = imageP.image.iCols * imageP.image.iRowInc * 3 ;
      RGB.pData = new BYTE[ dwImageSize ] ;

      m_Error = flycaptureConvertImage( m_flyCaptureContext , &imageP.image , &RGB ) ;
      if ( m_Error != FLYCAPTURE_OK )
      {
        int iErr = 5 ;
      }
      else
      {
//         m_Error = flycaptureInplaceWhiteBalance(
//           m_flyCaptureContext , RGB.pData , RGB.iRows , RGB.iCols ) ;
//         
        TVFrame AsFrame ;
        AsFrame.lpBMIH = (LPBITMAPINFOHEADER)malloc( sizeof(BITMAPINFOHEADER) ) ;

        memset( AsFrame.lpBMIH , 0 , sizeof(BITMAPINFOHEADER) ) ;
        
        AsFrame.lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
        AsFrame.lpBMIH->biHeight = imageP.image.iRows;
        AsFrame.lpBMIH->biWidth = imageP.image.iCols ;
        AsFrame.lpBMIH->biSizeImage = imageP.image.iCols * imageP.image.iRowInc * 3 ;
        AsFrame.lpBMIH->biCompression = BI_RGB ;
        AsFrame.lpBMIH->biBitCount = 24 ;
        AsFrame.lpBMIH->biPlanes = 1 ;

        frame->lpBMIH = rgb24yuv9( AsFrame.lpBMIH , RGB.pData ) ;
        frame->lpData = NULL ;
        delete AsFrame.lpBMIH ;
      }

      delete RGB.pData ;
    }

    CVideoFrame* vf=CVideoFrame::Create(frame);
    vf->SetLabel(m_CameraID);
    vf->SetTime(-1);

    m_Error = ::flycaptureUnlock( m_flyCaptureContext, imageP.uiBufferIndex );
    m_Lock.Unlock();
    return vf;
  }
  else if (m_Error!=FLYCAPTURE_TIMEOUT)
  {
    m_Lock.Unlock();
    if(!m_ErrorCaptureShown)
    {
      C1394_SENDERR_1("Fatal error: %s", ::flycaptureErrorToString( m_Error ));
      m_ErrorCaptureShown=true;
    }
    Sleep(1000);
    return NULL;
  }
  m_Lock.Unlock();
  return NULL;

}

bool PtGrey1_8::CameraStart()
{
  m_ErrorCaptureShown=false;
  if (!m_flyCaptureContext) 
  {
    m_Status="Fatal Error: Camera can't run due to lost of context";
    return false;
  }
  if (m_CurrentCamera==-1)
  {
    C1394_SENDERR_0("Can't start Camera: Camera is not selected");
    return false;
  }
  if (!_AddCameraToList(m_CurrentCamera))
  {
    C1394_SENDERR_0("Can't start Camera #d: Camera already started");
    return false;
  }
  m_Error=StartLockNext();
  m_bRun=(m_Error==FLYCAPTURE_OK);
  if (!m_bRun)
  {
    C1394_SENDERR_1("CameraStart: '%s'", ::flycaptureErrorToString( m_Error ));
  }
  return ((m_bRun) && C1394Camera::CameraStart());
}

void PtGrey1_8::CameraStop()
{
  m_Lock.Lock();
  m_Error=::flycaptureStop( m_flyCaptureContext );
  if (m_Error!=FLYCAPTURE_OK)
  {
    C1394_SENDERR_1("Error in ::flycaptureStop :'%s'",::flycaptureErrorToString(m_Error));
  }
  m_Status="Camera is stopped";
  C1394Camera::CameraStop();
  _RemoveCameraFromList(m_CurrentCamera);
  m_Lock.Unlock();
}

bool PtGrey1_8::GetCameraProperty(unsigned i, FXSIZE &value, bool& bauto)
{
  bauto=false;
  if ((int)i<0)
  {
    switch (i)
    {
    case FLYCAPTURE_VIDEOFORMAT:
      {
        bauto=false;
        value=m_CurrentVideoMode;
        return true;
      }
    case FLYCAPTURE_FRAMERATE:
      {
        bauto=false;
        value=m_CurrentFrameRate;
        return true;
      }
    case FLYCAPTURE_TRIGGER:
      {
        bool   bOnOff; 
        int iPolarity,iSource,iRawValue,iMode, iParameter;
        if (::flycaptureGetTrigger(m_flyCaptureContext,&bOnOff,&iPolarity,&iSource,&iRawValue,&iMode,&iParameter)!=FLYCAPTURE_OK)
          return false;
        value=(bOnOff)?1:0;
        return true;
      }
    case FLYCAPTURE_TRIGGERPOLARITY:
      {
        bool   bOnOff; 
        int iPolarity,iSource,iRawValue,iMode, iParameter;
        if (::flycaptureGetTrigger(m_flyCaptureContext,&bOnOff,&iPolarity,&iSource,&iRawValue,&iMode,&iParameter)!=FLYCAPTURE_OK)
          return false;
        value=(iPolarity==0)?0:1;
        return true;
      }
    case FLYCAPTURE_FRAME_INFO:
      {
        unsigned long    ulValue;
        if (::flycaptureGetCameraRegister( m_flyCaptureContext ,0x12f8 , &ulValue )==FLYCAPTURE_OK)
        {
          value=((ulValue&0xFFFF)==0x03c1);
          return true;
        }
      }
    case FLYCAPTURE_ROI:
      {
        static CString sROI;
        if (m_CurrentVideoMode!=FLYCAPTURE_VIDEOMODE_CUSTOM) return false;
        CRect rc;
        GetROI(rc);
        sROI.Format("%d,%d,%d,%d",rc.left,rc.top,rc.right,rc.bottom);
        value=(FXSIZE)(LPCTSTR)sROI;
        return true;
      }
    default:
      C1394_SENDERR_1("Undefined property:'%s'",cProperties[i].name);
    }
    return false;
  }
  else
  {
    long lB;
    return(::flycaptureGetCameraProperty(m_flyCaptureContext,(FlyCaptureProperty)i,(long*)&value,&lB,&bauto)==FLYCAPTURE_OK);
  }
}

bool PtGrey1_8::SetCameraProperty(unsigned i, FXSIZE &value, bool& bauto, bool& Invalidate)
{
  if ((FXSIZE)i<0)
  {
    switch (i)
    {
    case FLYCAPTURE_VIDEOFORMAT:
      {
        bool wasRunning=IsRunning();
        if (IsRunning()) 
          CameraStop();
        m_CurrentVideoMode=(FlyCaptureVideoMode)value;
        if (m_CurrentVideoMode==FLYCAPTURE_VIDEOMODE_CUSTOM)
          m_CurrentFrameRate=FLYCAPTURE_FRAMERATE_ANY;
        else
          m_CurrentFrameRate=FLYCAPTURE_FRAMERATE_15;
        Invalidate=true;
        BuildPropertyList();
        if (wasRunning)
          CameraStart();
        return true;
      }
    case FLYCAPTURE_FRAMERATE:
      {
        bool wasRunning=IsRunning();
        if (IsRunning()) 
          CameraStop();
        bauto=false;
        m_CurrentFrameRate=(FlyCaptureFrameRate)value;
        if (wasRunning)
          CameraStart();
        return true;
      }
    case FLYCAPTURE_TRIGGER:
      {
        bool   bOnOff; 
        int    iPolarity,iSource,iRawValue,iMode, iParameter;
        ::flycaptureGetTrigger(m_flyCaptureContext,&bOnOff,&iPolarity,&iSource,&iRawValue,&iMode,&iParameter);
        bOnOff=(value!=0);
        bool res = (::flycaptureSetTrigger(m_flyCaptureContext,bOnOff, iPolarity,iSource, iMode,iParameter)==FLYCAPTURE_OK);
        Invalidate=true;
        BuildPropertyList();
        return res;
      }
    case FLYCAPTURE_TRIGGERPOLARITY:
      {
        bool   bOnOff; 
        int    iPolarity,iSource,iRawValue,iMode, iParameter;
        ::flycaptureGetTrigger(m_flyCaptureContext,&bOnOff,&iPolarity,&iSource,&iRawValue,&iMode,&iParameter);
        iPolarity=(value==0)?0:1;
        bool res = (::flycaptureSetTrigger(m_flyCaptureContext,bOnOff, iPolarity,iSource, iMode,iParameter)==FLYCAPTURE_OK);
        return res;
      }
    case FLYCAPTURE_FRAME_INFO:
      {
        return (::flycaptureSetCameraRegister( m_flyCaptureContext ,0x12f8 , (value)?0x03c1:0x0 )==FLYCAPTURE_OK);
      }
    case FLYCAPTURE_ROI:
      {
        if (m_CurrentVideoMode!=FLYCAPTURE_VIDEOMODE_CUSTOM) return false;
        CRect rc;
        if (sscanf((LPCTSTR)value,"%d,%d,%d,%d",&rc.left,&rc.top,&rc.right,&rc.bottom)==4)
        {
          SetROI(rc);
          return true;
        }
        return false;
      }
    default:
      C1394_SENDERR_1("Undefined property:'%s'",cProperties[i].name);
    }
    return false;
  }
  else
  {
    long lB=0;
    return (::flycaptureSetCameraProperty(m_flyCaptureContext,(FlyCaptureProperty)i,(long)value,lB,bauto)==FLYCAPTURE_OK);
  }
}

FlyCaptureError PtGrey1_8::StartLockNext()
{
  if (m_CurrentVideoMode==FLYCAPTURE_VIDEOMODE_CUSTOM)
  {
    int width, height;
    if (queryMaxSize(width, height))
    {
      if (m_CurrentROI.bottom==-1)
      {
        m_CurrentROI=CRect(0,0,width,height);
      }
      m_Error=::flycaptureStartLockNextCustomImage(m_flyCaptureContext,0,m_CurrentROI.left,m_CurrentROI.top,
        m_CurrentROI.right-m_CurrentROI.left, m_CurrentROI.bottom-m_CurrentROI.top, 100.0,FLYCAPTURE_MONO8);
      if (m_Error==FLYCAPTURE_OK)
        C1394_SENDINFO_1("StartLockNext - %s", ::flycaptureErrorToString( m_Error ));
      else
        C1394_SENDERR_1("StartLockNext Error: %s", ::flycaptureErrorToString( m_Error ));
      return m_Error;
    }
    return  m_Error;
  }
  m_CurrentROI=CRect(-1,-1,-1,-1);
  m_Error=::flycaptureStartLockNext(m_flyCaptureContext, m_CurrentVideoMode, m_CurrentFrameRate);
  if (m_Error==FLYCAPTURE_OK)
    return m_Error;
  C1394_SENDERR_1("StartLockNext Error %s",::flycaptureErrorToString( m_Error ));
  return m_Error;
}

bool PtGrey1_8::queryMaxSize(int &width, int &height)
{
  bool    bAvailable;
  unsigned int    uiPixelUnitHorz;
  unsigned int    uiPixelUnitVert;
  unsigned int    uiPixelFormats;

  m_Error= flycaptureQueryCustomImage(
    m_flyCaptureContext,0,
    &bAvailable, (unsigned int*)(&width), (unsigned int*)(&height),
    &uiPixelUnitHorz, &uiPixelUnitVert, &uiPixelFormats
    );
  return (m_Error==FLYCAPTURE_OK);
}

bool PtGrey1_8::IsTriggerMode()
{
  bool   bOnOff; int iPolarity,iSource,iRawValue,iMode, iParameter;
  if (::flycaptureGetTrigger(m_flyCaptureContext,&bOnOff,&iPolarity,&iSource,&iRawValue,&iMode,&iParameter)!=FLYCAPTURE_OK)
    return false;
  return bOnOff;
}

void PtGrey1_8::GetROI(CRect& rc)
{
  rc=m_CurrentROI;
}

void PtGrey1_8::SetROI(CRect& rc)
{
  if ( 
    (m_CurrentROI.Width() == (rc.right - rc.left)) &&
    (m_CurrentROI.Height() == (rc.bottom - rc.top)) 
    )
  {
    // if format 7 and size the same simply do origin shift
    // by writing into IMAGE_ORIGIN register
    // without grab stop
    ::flycaptureSetCameraRegister(m_flyCaptureContext,0xa08, (rc.left << 16)|rc.top );
    m_CurrentROI = rc;
    return ;
  }
  bool wasRunning=IsRunning();
  if (IsRunning()) 
    CameraStop();

  m_CurrentROI=rc;
  if (m_CurrentROI.left!=-1)
  {
    m_CurrentROI.left&=~3;
    m_CurrentROI.right&=~3;
    m_CurrentROI.bottom&=~3;
    m_CurrentROI.top&=~3;
  }
  if (wasRunning)
    CameraStart();
}
