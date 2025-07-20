// avt2_11Camera.cpp: implementation of the avt2_11 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "avt2_11.h"
#include "avt2_11Camera.h"
#include <video\yuv411.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "avt2_11Camera"
#define MARLINCAMERAMAXNAMELENGTH 128

#pragma comment( lib, "FGCamera.lib" )

IMPLEMENT_RUNTIME_GADGET_EX(avt2_11, C1394Camera, "Video.capture", TVDB400_PLUGIN_NAME);

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

typedef struct tagCamProperties
{
	FG_PARAMETER        pr;
	const char *        name; 
}CamProperties;

static LPCTSTR GetErrorMessage(UINT32 errCode)
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
	case FCE_NODEADDRESS:   return "Wrong nodeaddress";
	case FCE_PARTIAL:       return "Partial info. copied";
	case FCE_NOMEM:         return "No memory";
	case FCE_NOTAVAILABLE:  return "Requested function not available";
	case FCE_NOTCONNECTED:  return "Not connected to target";
	case FCE_ADJUSTED:      return "A pararmeter had to be adjusted";
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

Videoformats vFormats[]=
{
	{CM_Y8,"Y8",true},
	{CM_YUV411,"YUV411", true},
	{CM_YUV422, "YUV422", false},
	{CM_YUV444, "YUV444",false},
	{CM_RGB8, "RGB8",false},
	{CM_Y16,"Y16",true},
	{CM_RGB16,"RGB16",false},
	{CM_SY16, "SY16",false},
	{CM_SRGB16, "SRGB16",false},
	{CM_RAW8, "RAW8",true},
	{CM_RAW16, "RAW16",false},
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

CamProperties cProperties[] =
{
	{FGP_IMAGEFORMAT, "Format"},
	{FGP_FRAMERATE, "FrameRate"},
	{FGP_RESOLUTION, "Resolution"},
	{FGP_BRIGHTNESS,"Brightness"},
	{FGP_AUTOEXPOSURE,"Exposure"},
	{FGP_SHARPNESS,"Sharpness"},
	{FGP_WHITEBALCB,"HWWhitebalanceB"},
	{FGP_WHITEBALCR,"HWWhitebalanceR"},
	{FGP_HUE,"Hue"},
	{FGP_SATURATION,"Saturation"},
	{FGP_GAMMA,"Gamma"},
	{FGP_IRIS,"Iris"},
	{FGP_FOCUS,"Focus"},
	{FGP_ZOOM,"Zoom"},
	{FGP_PAN,"Pan"},
	{FGP_TILT,"Tilt"},
	{FGP_SHUTTER,"Shutter"},
	{FGP_GAIN,"Gain"},
	{FGP_ROI,"ROI"},
	{FGP_TRIGGERONOFF, "Trigger"},
	{FGP_EXTSHUTTER, "Shutt_uS"},
	{FGP_SEND_FINFO , "FrameInfo" } ,
	{FGP_TRIGGERDELAY , "TrigDelay_uS" } ,
	{FGP_GRAB , "Grab" }
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


static CLockObject avtInitLock;
static long avtInitCntr=0;
static bool avtDrvReady=false;
static FGNODEINFO  nodeinfo[MAX_CAMERASNMB];

__forceinline UINT32HL getFullGid(unsigned low, int camonbus)
{
	UINT32HL retV={-1,-1};
	for (int i=0; i<camonbus; i++)
	{
		if (nodeinfo[i].Guid.Low==low)
		{
			retV=nodeinfo[i].Guid;
			break;
		}
	}
	return retV;
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


bool avtInit()
{
	CAutolock al(avtInitLock);
	LONG res;
	InterlockedExchange(&res,avtInitCntr);
	if (res==0)
	{
		UINT32 LastResult=FGInitModule(NULL);
		if (LastResult!=FCE_NOERROR)
		{
			TRACE("!!! FGInit error\n");
			SENDERR_1("Error: MarlinCamera Initialization error %s", GetErrorMessage(LastResult));
			return false;
		}
		avtDrvReady=true;
	}
    InterlockedIncrement(&avtInitCntr);
	return avtDrvReady;
}

void avtDone()
{
	CAutolock al(avtInitLock);
	LONG res;
	InterlockedDecrement(&avtInitCntr);
	InterlockedExchange(&res,avtInitCntr);
	if (res==0)
	{
		FGExitModule();
		avtDrvReady=false;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

avt2_11::avt2_11():
    FrameInfoOn(false)
{
	avtInit();
	memset(&m_BMIH,0,sizeof(BITMAPINFOHEADER));
}

void avt2_11::ShutDown()
{
	if (IsTriggerMode())
		SetTriggerMode(0); // no trigger
    CAutolock al(m_Lock);
	C1394Camera::ShutDown();
	avtDone();
}

bool avt2_11::DriverInit()
{
	if (!avtDrvReady) return false;
	//UINT32      CamerasOnBus;
	CAutolock al(avtInitLock);

	m_LastError=FGGetNodeList(nodeinfo,sizeof(nodeinfo)/sizeof(FGNODEINFO),(unsigned long*)&m_CamerasOnBus);

	if (m_LastError!=FCE_NOERROR)
	{
		TRACE("!!! FGGetNodeList error\n");
		SENDERR_1("Error: MarlinCamera Initialization error (FGGetNodeList) %s", GetErrorMessage(m_LastError));
		return false;
	}
	char cName[MARLINCAMERAMAXNAMELENGTH];
	for (unsigned i=0; i<m_CamerasOnBus; i++)
	{
		if ( (i == m_CurrentCamera)  ||  (m_CurrentCamera == -1) )
		{
			CFGCamera cC;
			if (CameraConnected() && (i==m_CurrentCamera))
			{
				m_Camera.GetDeviceName(cName,MARLINCAMERAMAXNAMELENGTH);
			}
			else
			{
				m_LastError=cC.Connect(&nodeinfo[i].Guid); cName[0]=0;
				if (m_LastError!=FCE_NOERROR)
				{
					SENDERR_1("Error: m_Camera.Connect: %s", GetErrorMessage(m_LastError));
				}
				cC.GetDeviceName(cName,MARLINCAMERAMAXNAMELENGTH);
				cC.Disconnect();
			}
			m_CamerasInfo[i].name= cName;
			m_CamerasInfo[i].serialnmb=nodeinfo[i].Guid.Low;
		}
	}
	return (avtDrvReady && (m_CamerasOnBus!=0));
}

bool avt2_11::CameraInit()
{
	if (m_CamerasOnBus==0) 
	{
		SENDERR_0("Error: No Marlin Cameras found on a bus");
		return false;
	}
	ASSERT(m_CurrentCamera!=-1);
	if (CameraConnected())
		CameraClose();
	m_LastError=m_Camera.Connect(&nodeinfo[m_CurrentCamera].Guid);
	if (m_LastError!=FCE_NOERROR)
	{
		SENDERR_1("Error: MarlinCamera Initialization error (m_Camera.Connect) %s", GetErrorMessage(m_LastError));
		return false;
	}
	m_CameraID.Format("%s_%d",m_CamerasInfo[m_CurrentCamera].name,m_CamerasInfo[m_CurrentCamera].serialnmb);
	return BuildPropertyList();;    
}

void avt2_11::CameraClose()
{
	m_Camera.Disconnect();
}

bool avt2_11::BuildPropertyList()
{
	UINT32 ImageFormat;
	m_LastError=m_Camera.GetParameter(FGP_IMAGEFORMAT, &ImageFormat);

	m_Format    = (FG_COLORMODE)IMGCOL(ImageFormat);
	m_FrameRate = (FG_FRAMERATE)IMGRATE(ImageFormat);
	m_Resolution= (FG_RESOLUTION)IMGRES(ImageFormat);

	m_Properties.RemoveAll();
	int propCount=sizeof(cProperties)/sizeof(CamProperties);
	CAMERA1394::Property P;
	for (int i=0; i<propCount; i++)
	{
		if (cProperties[i].pr<=FGP_IMAGEFORMAT)
		{
			switch (cProperties[i].pr)
			{
			case FGP_IMAGEFORMAT:
				{
					CString items,tmpS;
					int formatCount=sizeof(vFormats)/sizeof(Videoformats);
					for (int j=0; j<formatCount; j++)
					{
						if ((vFormats[j].supported) && (IsFormatSupported(m_Camera,vFormats[j].vm)))
						{
							tmpS.Format("%s(%d),",vFormats[j].name,vFormats[j].vm);
							items+=tmpS;
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
					if (m_Resolution==RES_SCALABLE)
						break;
					CString items,tmpS;
					int frCount=sizeof(fRates)/sizeof(Framerates);
					for (int j=0; j<frCount; j++)
					{
						if (IsModeSupported(m_Camera,m_Format,fRates[j].fr, m_Resolution))
						{
							tmpS.Format("%s(%d),",fRates[j].name,fRates[j].fr);
							items+=tmpS;
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
			case FGP_RESOLUTION:
				{
					CString items,tmpS;
					int resCount=sizeof(cResolutions)/sizeof(Resolutions);
					for (int j=0; j<resCount; j++)
					{
						if (cResolutions[j].res!=RES_SCALABLE)
						{
							if (IsModeSupported(m_Camera,m_Format,FR_15, cResolutions[j].res))
							{
								tmpS.Format("%s(%d),",cResolutions[j].name,cResolutions[j].res);
								items+=tmpS;
							}
						}
						else
						{
							if (IsModeSupported(m_Camera,m_Format,(FG_FRAMERATE)0, cResolutions[j].res))
							{
								tmpS.Format("%s(%d),",cResolutions[j].name,cResolutions[j].res);
								items+=tmpS;
							}
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
					P.property.Format("Spin(%s,%d,%d)", P.name , 10 , 30000000 );
					m_Properties.Add(P);
					break;
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
					P.name=cProperties[i].name;
					P.id=(unsigned)cProperties[i].pr;
					P.property.Format("Spin(%s,%d,%d)", P.name , 0 , 30000000 );
					m_Properties.Add(P);
					break;
				}
			case FGP_GRAB:
				P.name=cProperties[i].name;
				P.id=(unsigned)cProperties[i].pr;
				P.property.Format("Spin(%s,%d,%d)", P.name , -1 , 30000000 );
				m_Properties.Add(P);
				break ; // not shown property
			default:
				{
					SENDERR_1("Error: unsupported property %s", cProperties[i].name);
					break;
				}
			}
		}
		else // std property
		{
			FGPINFO fInfo;
			if (m_Camera.GetParameterInfo(cProperties[i].pr,&fInfo)==FCE_NOERROR)
			{
				bool autocap=((fInfo.Specific.Type==1) && (fInfo.Specific.Data.FeatureInfo.AutoCap!=0));

				P.name=cProperties[i].name;
				P.id=(unsigned)cProperties[i].pr;
				P.property.Format("%s(%s,%d,%d)",(autocap)?SETUP_SPINABOOL:SETUP_SPIN,P.name,fInfo.MinValue,fInfo.MaxValue);
				m_Properties.Add(P);
			}
		}
	}  
	if (m_LastError!=FCE_NOERROR)
		m_BMIH.biSize=0;
	else
	{
		m_BMIH.biSize=sizeof(BITMAPINFOHEADER);
		m_BMIH.biWidth=GetXSize(m_Camera);
		m_BMIH.biHeight=GetYSize(m_Camera);
		unsigned XOff=GetXOffset(m_Camera);
		unsigned YOff=GetYOffset(m_Camera);
		m_CurrentROI=CRect(XOff,YOff,XOff+m_BMIH.biWidth,YOff+m_BMIH.biHeight);
		m_BMIH.biPlanes=1;
		switch(m_Format)
		{
		case CM_Y8    : 
			m_BMIH.biCompression=BI_Y8;
			m_BMIH.biBitCount=8;
			m_BMIH.biSizeImage=m_BMIH.biWidth*m_BMIH.biHeight;
			break;
		case CM_YUV411:
			m_BMIH.biCompression=BI_YUV411;
			m_BMIH.biBitCount=12;
			m_BMIH.biSizeImage=3*m_BMIH.biWidth*m_BMIH.biHeight/2;
			break;
		case CM_Y16   : 
			m_BMIH.biCompression=BI_Y16;
			m_BMIH.biBitCount=16;
			m_BMIH.biSizeImage=2*m_BMIH.biWidth*m_BMIH.biHeight;
			break;
		case CM_RAW8:
		case CM_RAW16:
		case CM_YUV422:
		case CM_YUV444:
		case CM_RGB8  :
		case CM_RGB16 :

		default: 
			m_BMIH.biSize=0;
			TRACE("!!! Unsupported format #%d\n", m_Format);
			SENDERR_1("!!! Unsupported format #%d",m_Format);
			return false;
		}
	}
	return true;
}

bool avt2_11::GetCameraProperty(unsigned i, int &value, bool& bauto)
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
				if (m_Resolution==RES_SCALABLE) return false;
				UINT32 res;
				m_Camera.GetParameter(FGP_IMAGEFORMAT,&res);
				value=IMGRATE(res);
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
				static CString sROI;
				if (m_Resolution != RES_SCALABLE) return false;
				CRect rc;
				GetROI(rc);
				sROI.Format("%d,%d,%d,%d",rc.left,rc.top,rc.right,rc.bottom);
				value=(int)(LPCTSTR)sROI;
				return true;
			}
		case FGP_TRIGGERONOFF:
			{
				value = GetTriggerMode() ;
				return true;
			}
		case FGP_EXTSHUTTER:
			{
				value = GetLongExposure() ;
				return true ;
			}
		case FGP_SEND_FINFO:
			{
                value=FrameInfoOn;
				//value = (IsFrameInfoAvailable() != 0) ;
				return true ;
			}
		case FGP_TRIGGERDELAY:
			{
				value = GetTriggerDelay() ;
				return true ;
			}
		case FGP_GRAB:
			{
				bool bContinuous ;
				bool bRes = GetGrabConditions( bContinuous , value ) ;
				return true;
			}
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
		else
			value=val;
		return true;
	}
	return false;
}

bool avt2_11::SetCameraProperty(unsigned i, int &value, bool& bauto, bool& Invalidate)
{
	if ((int)i<=FGP_IMAGEFORMAT)
	{

		switch (i)
		{
		case FGP_IMAGEFORMAT:
			{
				bool wasRunning=IsRunning();
				if (wasRunning)
					OnStop();
				m_Format=(FG_COLORMODE)value;
				bool res=
					(m_Camera.SetParameter(FGP_IMAGEFORMAT,
					MAKEIMAGEFORMAT(
					m_Resolution,
					m_Format,
					m_FrameRate
					)
					)==FCE_NOERROR);
				BuildPropertyList();
				Invalidate=true;
				if (wasRunning)
					OnStart();
				return res;
			}
		case FGP_FRAMERATE:
			{
				bool wasRunning=IsRunning();
				if (wasRunning)
					OnStop();
				m_FrameRate=(FG_FRAMERATE)value;
				bool res=
					(m_Camera.SetParameter(FGP_IMAGEFORMAT,
					MAKEIMAGEFORMAT(
					m_Resolution,
					m_Format,
					m_FrameRate
					)
					)==FCE_NOERROR);
				BuildPropertyList();
				Invalidate=true;
				if (wasRunning)
					OnStart();
				return res;

			}
		case FGP_RESOLUTION:
			{
				bool wasRunning=IsRunning();
				if (wasRunning)
					OnStop();
				m_Resolution=(FG_RESOLUTION)value;
				m_FrameRate=FR_15;
				bool res=
					(m_Camera.SetParameter(FGP_IMAGEFORMAT,
					MAKEIMAGEFORMAT(
					m_Resolution,
					m_Format,
					(m_Resolution==RES_SCALABLE)?0:m_FrameRate
					)
					)==FCE_NOERROR);
				BuildPropertyList();
				Invalidate=true;
				if (wasRunning)
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
				SetTriggerMode( value );
				return true;
			}
		case FGP_EXTSHUTTER:
			{
				SetLongExposure( value ) ;
				return true ;
			}
		case FGP_SEND_FINFO:
			{
                if (!IsFrameInfoAvailable())
                {
                    FrameInfoOn=false;
                    return false;
                }
				SetSendFrameInfo( value ) ;
                FrameInfoOn=(value!=0);
				return true ;
			}
		case FGP_TRIGGERDELAY:
			{
				SetTriggerDelay( value );
				return true;
			}
		case FGP_GRAB:
			{
				SetGrab( value ) ;
				return true;
			}
		default:
			break;
		}
	}
	else
	{
		UINT32  val=(bauto)?PVAL_AUTO:value;
		return (m_Camera.SetParameter(i,val)==FCE_NOERROR);
	}
	return false;
}

bool avt2_11::CameraStart()
{
	m_LastError=m_Camera.OpenCapture(); 
	if (m_LastError!=FCE_NOERROR)
	{
		SENDERR_1("Error in m_Camera.OpenCapture(): %s", GetErrorMessage(m_LastError));
		return false;
	}
	m_LastError=m_Camera.StartDevice();
	if (m_LastError!=FCE_NOERROR)
	{
		SENDERR_1("Error in m_Camera.StartDevice(): %s", GetErrorMessage(m_LastError));
		return false;
	}
/*	CString Prop ;
	bool Invalidate ;
	if ( PrintProperties( Prop ) )
		ScanProperties( Prop , Invalidate ); */
	//     for ( int i = 0 ; i < m_Properties.GetCount() ; i++ )
	//     {
	//       SetCameraProperty( i , m_Properties[i].)
	//     }
	return true;
}

void avt2_11::CameraStop()
{
	m_Camera.StopDevice(); 
	m_Camera.CloseCapture(); 
}

CVideoFrame* avt2_11::CameraDoGrab()
{
	if (m_BMIH.biSize==0) 
	{
		Sleep(10);
		return NULL;
	}
	CAutolock al(m_Lock);
	FGFRAME    CurFrame;
	m_LastError=m_Camera.GetFrame(&CurFrame,5000); 
	pTVFrame    frame = (pTVFrame)malloc(sizeof(TVFrame));
	frame->lpBMIH=NULL;
	frame->lpData=NULL;
    memcpy(&m_RealBMIH,&m_BMIH,m_BMIH.biSize);
	if(m_LastError==FCE_NOERROR) 
	{

		/*if (m_StartTime.QuadPart==-1)
		{
		m_StartTime.LowPart=CurFrame.RxTime.Low;
		m_StartTime.HighPart=CurFrame.RxTime.High;
		}*/
        bool rescaled=false;
        LPBYTE data=CurFrame.pData;
        if ((m_BMIH.biWidth%4!=0) || (m_BMIH.biHeight%4!=0))
        {
            int newWidth=(m_BMIH.biWidth/4)*4;
            int newHeight=(m_BMIH.biHeight/4)*4;
            rescaled=true;
            int size=(vFormats[m_Format].vm==CM_Y16)?newHeight*newWidth*2:newHeight*newWidth;
            data=(LPBYTE)malloc(size);
            memset(data,0,size);
            LPBYTE src=CurFrame.pData;
            LPBYTE dst=data;
            for (int i=0; i<newHeight; i++)
            {
                if (vFormats[m_Format].vm==CM_Y16)
                    memcpy(dst,src,2*newWidth);
                else
                    memcpy(dst,src,newWidth);
                if (vFormats[m_Format].vm==CM_Y16)
                {
                    dst+=2*newWidth;
                    src+=2*m_BMIH.biWidth;
                }
                else
                {
                    dst+=newWidth;
                    src+=m_BMIH.biWidth;
                }
            }
            if (vFormats[m_Format].vm==CM_Y16)
                m_RealBMIH.biSizeImage=size;
            m_RealBMIH.biWidth=newWidth;
            m_RealBMIH.biHeight=newHeight;
        }
		if (vFormats[m_Format].vm==CM_YUV411)
			frame->lpBMIH=yuv411yuv9(&m_RealBMIH,data);
		else if (vFormats[m_Format].vm==CM_Y8)
		{
			frame->lpBMIH = y8yuv9( &m_RealBMIH , data);
		}
		else if (vFormats[m_Format].vm==CM_Y16)
		{
			frame->lpBMIH=&m_RealBMIH;
            m_RealBMIH.biSizeImage=m_RealBMIH.biHeight*m_RealBMIH.biWidth*2;
			frame->lpData=(LPBYTE)malloc(m_RealBMIH.biSizeImage);
			_swab((char*)data,(char*)frame->lpData,m_RealBMIH.biSizeImage);
		}
        if (rescaled)
            free(data);
		m_LastError=m_Camera.PutFrame(&CurFrame);
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

bool avt2_11::IsTriggerMode()
{
	UINT32 TriggerValue;
	// Get current trigger mode
	m_Camera.GetParameter(FGP_TRIGGER,&TriggerValue);
	return (TRGON(TriggerValue)!=0);
}
int avt2_11::GetTriggerMode()
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

void avt2_11::SetTriggerMode(int iMode)
{

	// iMode : 0 - no trigger, 1 - channel 1 no inverse , 2 - channel 1 inverse
	//                         3 - channel 2 no inverse , 4 - channel 2 inverse
	UINT32 nOn=(iMode)?1:0; // 0=ext. trigger off, else ext. trigger on
	if ( !nOn )
	{
		UINT32 TriggerValue=MAKETRIGGER(0, 0, 0, 0, 0);
		m_Camera.SetParameter(FGP_TRIGGER,TriggerValue);
		return ;
	}
	UINT32 TrigReg , IO1 , IO2 ;
	m_Camera.ReadRegister( 0xf0f00830 , &TrigReg ) ;
	m_Camera.ReadRegister( 0xf1000300 , &IO1 ) ;
	m_Camera.ReadRegister( 0xf1000304 , &IO2 ) ;
	UINT32 nSrc=(iMode > 2) ? 1 : 0; // not currently applicable to AVT cameras, as of February 2008 - parameter inherited from IIDC/DCAM
	UINT32 uiAddrOn = 0xf1000300 + (nSrc * 4) ;
	UINT32 uiAddrOff = 0xf1000300 + ((nSrc ^ 1) * 4) ;
	UINT32 iPol = (iMode % 2) == 0 ;
	// Check and set polarity together with trigger mode
	UINT32 uiControlIO = 0x00020000 ;

	UINT32 TriggerValue=MAKETRIGGER( 1 , iPol , nSrc , 0 , 0 ) ;
	m_Camera.SetParameter(FGP_TRIGGER,TriggerValue);
	m_Camera.WriteRegister( uiAddrOn , uiControlIO ) ;
	m_Camera.WriteRegister( uiAddrOff , 0 ) ;
	m_Camera.WriteRegister( 0xf0f00830 , 0x02000000 | (iPol ? 0x01000000 : 0 ) ) ;
	//UINT32 nMode=0; // 0=edge mode, 1=level mode, 15=bulk mode
	//UINT32 nParm=0; // not currently applicable to AVT cameras, as of February 2008 - parameter inherited from IIDC/DCAM
	//UINT32 TriggerValue;
	// Enable ext. trigger, edge mode (0), falling edge
}

void avt2_11::GetROI(CRect& rc)
{
	rc=m_CurrentROI;
}

bool avt2_11::SetROI(CRect& rc)
{
	if (m_Resolution != RES_SCALABLE)
	{
		SENDERR_0("Can't crop image. Reason: The camera format isn't scalable");
		return false;
	}
	if (rc.left!=-1) //must be divisible by 4
	{
		rc.left&=~3;
		rc.right&=~3;
		rc.bottom&=~3;
		rc.top&=~3;
	}
	else
	{
		rc.left=rc.top=0;
		rc.right=GetMaxWidth(m_Camera);
		rc.bottom=GetMaxHeight(m_Camera);
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
	CAutolock al(m_Lock);
	bool wasRunning=IsRunning();
	if (wasRunning)
		CameraStop();

	if (
		((
		((m_LastError=m_Camera.SetParameter(FGP_XSIZE,rc.right-rc.left))==FCE_NOERROR) 
		&& ((m_LastError=m_Camera.SetParameter(FGP_XPOSITION,rc.left))==FCE_NOERROR)
		)
		||
		(
		((m_LastError=m_Camera.SetParameter(FGP_XPOSITION,rc.left))==FCE_NOERROR) 
		&& ((m_LastError=m_Camera.SetParameter(FGP_XSIZE,rc.right-rc.left))==FCE_NOERROR)
		))
		&& 
		((
		((m_LastError=m_Camera.SetParameter(FGP_YSIZE,rc.bottom-rc.top))==FCE_NOERROR) 
		&& ((m_LastError=m_Camera.SetParameter(FGP_YPOSITION,rc.top))==FCE_NOERROR)
		)
		||
		(
		((m_LastError=m_Camera.SetParameter(FGP_YPOSITION,rc.top))==FCE_NOERROR) 
		&& ((m_LastError=m_Camera.SetParameter(FGP_YSIZE,rc.bottom-rc.top))==FCE_NOERROR)
		))
		)
	{
		m_CurrentROI = rc;
		BuildPropertyList();
		if (wasRunning)
			CameraStart();
		return true;
	}
	SENDERR_1("Error in SetROI(): %s", GetErrorMessage(m_LastError));
	if (wasRunning)
		CameraStart();
	return false;        
}
int avt2_11::GetLongExposure()
{
	UINT32 iExp_usec ;
	m_Camera.ReadRegister(0xF100020c,&iExp_usec);
	return (iExp_usec & 0x03ffffff);
}

void avt2_11::SetLongExposure( int iExp_usec)
{
	m_Camera.WriteRegister(0xF100020c,iExp_usec);
}

DWORD avt2_11::IsFrameInfoAvailable()
{
	DWORD iAvail ;
	m_Camera.ReadRegister(0xF1000630,&iAvail);
	if ( iAvail & 0x80000000 )
		return (iAvail & 0x0200ffff ) ;
	else
		return 0 ;
}

void avt2_11::SetSendFrameInfo( int iSend)
{
	m_Camera.WriteRegister(0xF1000630 , (iSend) ? 0x02000000 : 0 );
}

int avt2_11::GetTriggerDelay()
{
	UINT32 iDelay_usec ;
	m_Camera.ReadRegister(0xF1000400,&iDelay_usec);
	return iDelay_usec ;
}

void avt2_11::SetTriggerDelay( int iDelay_uS)
{
	if ( iDelay_uS )
		iDelay_uS = 0x02000000 | (iDelay_uS & 0x001fffff) ;
	m_Camera.WriteRegister(0xF1000400,iDelay_uS);
}

bool avt2_11::SetGrab( int iNFrames )
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
	return true ;
}

bool avt2_11::GetGrabConditions( 
	bool& bContinuous , int& iNRestFrames ) 
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
