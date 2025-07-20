// SoliosDalsaCamera.cpp: implementation of the SoliosDalsa class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoliosDalsa.h"
#include "SoliosDalsaCamera.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "SoliosDalsa"
#define MARLINCAMERAMAXNAMELENGTH 128

// #pragma comment( lib, "FGCamera.lib" )

IMPLEMENT_RUNTIME_GADGET_EX(SoliosDalsa, C1394Camera, "Video.capture", TVDB400_PLUGIN_NAME);


static LPCTSTR GetErrorMessage(UINT32 errCode)
{
// 	switch (errCode)
// 	{
// 	case FCE_NOERROR:       return "No Error";
// 	case FCE_ALREADYOPENED: return "Something already opened";
// 	case FCE_NOTOPENED:     return "Need open before";
// 	case FCE_NODETAILS:     return "No details";
// 	case FCE_DRVNOTINSTALLED: return "Driver not installed";
// 	case FCE_MISSINGBUFFERS: return "Don't have buffers";
// 	case FCE_INPARMS:       return "Parameter error";
// 	case FCE_CREATEDEVICE:  return "Error creating WinDevice";
// 	case FCE_WINERROR:      return "Internal Windows error";
// 	case FCE_IOCTL:         return "Error DevIoCtl";
// 	case FCE_DRVRETURNLENGTH: return "Wrong length return data";
// 	case FCE_INVALIDHANDLE: return "Wrong handle";
// 	case FCE_NOTIMPLEMENTED: return "Function not implemented";
// 	case FCE_DRVRUNNING:    return "Driver runs already";
// 	case FCE_STARTERROR:    return "Couldn't start";
// 	case FCE_INSTALLERROR:  return "Installation error";
// 	case FCE_DRVVERSION:    return "Driver has wrong version";
// 	case FCE_NODEADDRESS:   return "Wrong node address";
// 	case FCE_PARTIAL:       return "Partial info. copied";
// 	case FCE_NOMEM:         return "No memory";
// 	case FCE_NOTAVAILABLE:  return "Requested function not available";
// 	case FCE_NOTCONNECTED:  return "Not connected to target";
// 	case FCE_ADJUSTED:      return "A parameter had to be adjusted";
// 	case HALER_NOCARD:      return "Card is not present";
// 	case HALER_NONTDEVICE:  return "No logical Device";
// 	case HALER_NOMEM:       return "Not enough memory";
// 	case HALER_MODE:        return "Not allowed in this mode";
// 	case HALER_TIMEOUT:     return "Timeout";
// 	case HALER_ALREADYSTARTED: return "Something is started";
// 	case HALER_NOTSTARTED:  return "Not started";
// 	case HALER_BUSY:        return "Busy at the moment";
// 	case HALER_NORESOURCES: return "No resources available";
// 	case HALER_NODATA:      return "No data available";
// 	case HALER_NOACK:       return "Didn't get acknowledge";
// 	case HALER_NOIRQ:       return "Interrupt install error";
// 	case HALER_NOBUSRESET:  return "Error waiting for bus reset";
// 	case HALER_NOLICENSE:   return "No license";
// 	case HALER_RCODEOTHER:  return "RCode not RCODE_COMPLETE";
// 	case HALER_PENDING:     return "Something still pending";
// 	case HALER_INPARMS:     return "Input parameter range";
// 	case HALER_CHIPVERSION: return "Unrecognized chip version";
// 	case HALER_HARDWARE:    return "Hardware error";
// 	case HALER_NOTIMPLEMENTED: return "Not implemented";
// 	case HALER_CANCELLED:   return "Cancelled";
// 	case HALER_NOTLOCKED:   return "Memory is not locked";
// 	case HALER_GENERATIONCNT: return "Bus reset in between";
// 	case HALER_NOISOMANAGER: return "No IsoManager present";
// 	case HALER_NOBUSMANAGER: return "No BusManager present";
// 	case HALER_UNEXPECTED:  return "Unexpected value";
// 	case HALER_REMOVED:     return "Target was removed";
// 	case HALER_NOBUSRESOURCES: return "No ISO resources available";
// 	case HALER_DMAHALTED:   return "DMA halted";
// 	}
	return "Unknown error";
}


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
	{FGP_FRAMERATE, "FrameRate"},
	{FGP_GAIN,"Gain"},
	{FGP_TRIGGERONOFF, "Trigger"},
	{FGP_EXTSHUTTER, "Shutt_uS"},
	{FGP_GRAB , "Grab" }
};


__forceinline int GetFormatId(int format)
{
	return 1;
}

__forceinline int GetFramerateId(int framerate)
{
	return 1;
}

__forceinline int GetResolutionId(int resolution)
{
	return 1;
}


static FXLockObject avtInitLock;
static long avtInitCntr=0;
static bool avtDrvReady=false;
static FGNODEINFO  nodeinfo[MAX_CAMERASNMB];

__forceinline UINT32HL getFullGid(unsigned low, int camonbus)
{
  UINT32HL retV = { (DWORD)-1, (DWORD)-1 };
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

__forceinline unsigned GetXSize()
{
	return 1024 ;
}

__forceinline unsigned GetYSize()
{
	return 1024;
}

__forceinline unsigned GetXOffset()
{
	return 0;
}

__forceinline unsigned GetYOffset()
{
	return 0;
}

__forceinline unsigned GetMaxWidth()
{
	return 1024;
}

__forceinline unsigned GetMaxHeight()
{
	return 1024;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SoliosDalsa::SoliosDalsa()
{
  m_CurrentCamera = 0 ; // no camera by default 
  m_bTransferImages = FALSE ;
  m_pCamRequest = NULL ;
	memset(&m_BMIH,0,sizeof(BITMAPINFOHEADER));
  m_BMIH.biSize = sizeof( m_BMIH ) ;
  m_BMIH.biBitCount = 16 ;
  m_BMIH.biClrImportant = 0 ;
  m_BMIH.biClrUsed = 0 ;
  m_BMIH.biCompression = BI_Y16 ;
  m_BMIH.biHeight = 1024 ;
  m_BMIH.biWidth = 1024 ;
  m_BMIH.biPlanes = 1 ;
  m_BMIH.biSizeImage = m_BMIH.biHeight * m_BMIH.biWidth * sizeof(WORD) ;
  m_BMIH.biXPelsPerMeter = 0 ;
  m_BMIH.biYPelsPerMeter = 0 ;
  m_RealBMIH = m_BMIH ;
}

void SoliosDalsa::ShutDown()
{
	if (IsTriggerByInputPin())
		SetTriggerMode(0); // no trigger
  FXAutolock al(m_Lock);
  for ( int i = 0 ; i < m_ImCNTL.m_RQImaging.GetCount() ; i++ )
  {
    if ( m_ImCNTL.m_RQImaging[i] == m_pCamRequest )
    {
      m_ImCNTL.m_RQImaging.RemoveAt(i) ;
      delete m_pCamRequest ;
      m_pCamRequest = NULL ;
      m_CamerasOnBus-- ;
    }
  }
  C1394Camera::ShutDown();
}

bool SoliosDalsa::DriverInit()
{
  if ( !avtDrvReady || !m_pCamRequest )
  {
  FXString CamSuffix ;
    CamSuffix.Format( "Solios%d" , m_CurrentCamera ) ;
  FXString ShMemName("Camera") ; ShMemName += CamSuffix ;
  FXString InEventName("CamInEvent") ; InEventName += CamSuffix ;
  FXString OutEventName("CamOutEvent") ; OutEventName += CamSuffix ;
  m_pCamRequest = new CSMSoliosRequest(  
    (LPCTSTR)ShMemName , (LPCTSTR)InEventName , 
    (LPCTSTR)OutEventName , 1024 * 1024 * 2 ) ;
  FXString ImOutEventName( "ImOutEvt" ) ; ImOutEventName += CamSuffix ;
  m_pCamRequest->SetOutImageEventName( ImOutEventName ) ;
  m_bCamApplWasstarted = false ;
  if ( m_pCamRequest )
  {
    m_ImCNTL.m_RQImaging.Add( m_pCamRequest ) ;
      BuildPropertyList()  ;
      m_CamerasOnBus++ ;
    if ( ! m_pCamRequest->IsNotNewArea() )
    {
      FXString CmndStr("start ") ;
      m_ControlProgram.Trim( " \t\n\r.," ) ;
      CmndStr += m_ControlProgram ;
      system( (LPCTSTR)CmndStr ) ;
    }
    else
      m_bCamApplWasstarted = false ;
    avtDrvReady = true ;
  }
  }
	return (avtDrvReady && (m_CamerasOnBus!=0));
}

bool SoliosDalsa::CameraInit()
{
	return BuildPropertyList();;    
}

void SoliosDalsa::CameraClose()
{
}

bool SoliosDalsa::BuildPropertyList()
{
	m_Properties.RemoveAll();
	int propCount=sizeof(cProperties)/sizeof(CamProperties);
	CAMERA1394::Property P;
	for (int i=0; i<propCount; i++)
	{
		switch (cProperties[i].pr)
		{
		case FGP_FRAMERATE:
			{
				P.name=cProperties[i].name;
				P.id=(unsigned)cProperties[i].pr;
				P.property.Format("EditBox(%s)",P.name);
				m_Properties.Add(P);
				break;
			}
		case FGP_TRIGGERONOFF:
			{
				P.name=cProperties[i].name;
				P.id=(unsigned)cProperties[i].pr;
				P.property.Format("ComboBox(%s(Off(0),Tr1(1)))",
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
		case FGP_GRAB:
			P.name=cProperties[i].name;
			P.id=(unsigned)cProperties[i].pr;
			P.property.Format("Spin(%s,%d,%d)", P.name , -1 , 30000000 );
			m_Properties.Add(P);
			break ; // not shown property
    case FGP_GAIN:
      {
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        P.property.Format("ComboBox(%s(1(0),2(1),4(2)))",
          P.name);
        m_Properties.Add(P);
        break;
      }
		default:
			{
				SENDERR_1("Error: unsupported property %s", cProperties[i].name);
				break;
			}
		}
	}  
	m_BMIH.biSize=sizeof(BITMAPINFOHEADER);
	m_BMIH.biWidth = 1024 ; //GetXSize();
	m_BMIH.biHeight = 1024 /*GetYSize()*/;
	unsigned XOff = 0 /*GetXOffset()*/;
	unsigned YOff = 0 ; //GetYOffset();
	m_CurrentROI=CRect(XOff,YOff,XOff+m_BMIH.biWidth,YOff+m_BMIH.biHeight);
	m_BMIH.biPlanes=1;
	m_BMIH.biCompression=BI_Y16;
	m_BMIH.biBitCount=16;
	m_BMIH.biSizeImage=2*m_BMIH.biWidth*m_BMIH.biHeight;
	return true;
}

bool SoliosDalsa::GetCameraProperty(unsigned i, int &value, bool& bauto)
{
  switch (i)
  {
  case FGP_FRAMERATE:
	  {
		  value= m_iFramesPerSecond ;
		  return true;
	  }
  case FGP_TRIGGERONOFF:
	  {
		  value = m_bTriggerMode ;
		  return true;
	  }
  case FGP_EXTSHUTTER:
	  {
		  value = m_iShutter_us ;
		  return true ;
	  }
  case FGP_GRAB:
	  {
		  value = m_bLiveVideo ;
		  return true;
	  }
  default:
	  break;
  }
	return false;
}

bool SoliosDalsa::SetCameraProperty(
  unsigned i, int &value, bool& bauto, bool& Invalidate)
{
	switch (i)
	{
	case FGP_FRAMERATE:
		{
			return true ;
		}
	case FGP_TRIGGERONOFF:
		{
			SetTriggerMode( value );
			return true;
		}
	case FGP_EXTSHUTTER:
		{
			SetLongExposure( value ) ;
    m_iShutter_us = value;
			return true ;
		}
	case FGP_GRAB:
		{
			SetGrab( value ) ;
			return true;
		}
	default:
		break;
	}
	return false;
}

bool SoliosDalsa::CameraStart()
{
	m_bTransferImages = TRUE ;
  return true;
}

void SoliosDalsa::CameraStop()
{
  m_bTransferImages = FALSE ;
}

CVideoFrame* SoliosDalsa::CameraDoGrab(double* dStartTime)
{
	if (m_BMIH.biSize==0) 
	{
		Sleep(10);
		return NULL;
	}
	FXAutolock al(m_Lock);
  
//   if ( !m_bLiveVideo )
//   {
//     SetGrab( 1 ) ;
//   }
  if ( m_pCamRequest->WaitForImage( 2000 ) && m_bTransferImages )
  {
    pTVFrame frame = (pTVFrame)malloc(sizeof(TVFrame));
    if ( frame )
  {
    frame->lpBMIH =  (LPBITMAPINFOHEADER)
      malloc( sizeof(BITMAPINFOHEADER) + m_RealBMIH.biSizeImage );
      if ( frame->lpBMIH )
    {
      memcpy(frame->lpBMIH,&m_RealBMIH,m_RealBMIH.biSize);
      frame->lpData = NULL;
      LPBYTE pImage = m_pCamRequest->GetImage() ;
      memcpy( &frame->lpBMIH[1] , pImage , m_RealBMIH.biSizeImage ) ;
      CVideoFrame* vf=CVideoFrame::Create(frame);
      vf->SetLabel(m_CameraID);
      return vf;
    }
      free(frame);
  }
  }
	return NULL;
}

int SoliosDalsa::GetTriggerMode()
{
  return (int)m_bTriggerMode ;
}

void SoliosDalsa::SetTriggerMode(int iMode)
{
  m_ImCNTL.SetSyncMode( iMode ) ; // ==0 - external sync

}

void SoliosDalsa::GetROI(CRect& rc)
{
	rc=m_CurrentROI;
}

int SoliosDalsa::GetLongExposure()
{
	return m_ImCNTL.GetExposure() ;
}

void SoliosDalsa::SetLongExposure( int iExp_usec)
{
  m_ImCNTL.SetExposure( iExp_usec ) ;
}

bool SoliosDalsa::SetGrab( int iNFrames )
{
	if ( iNFrames < -1 )
		return false ;
  m_ImCNTL.Grab( iNFrames ) ;
  return true ;
}

bool SoliosDalsa::ScanSettings(FXString& text)
{
  text = "template(Spin(FrameRate,1,30,30),"
    "Spin(Shutt_uS,10,60000000,12000)"
    "Spin(Grab,-1,100000,0),"
    "EditBox(Program),"
    "ComboBox(Gain(1(0),2(1),4(2))),"
    "ComboBox(Trigger(Off(0),Trigger(1)))"
    ")";
  return true;
}

bool SoliosDalsa::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  //C1394Camera::PrintProperties(text);
  pk.WriteInt("FrameRate" , m_iFramesPerSecond ) ;
  pk.WriteInt( "Shutt_uS" , m_iShutter_us ) ;
  pk.WriteInt( "Grab" , (m_bLiveVideo)? -1 : 0 ) ;
  pk.WriteString( "Program" , m_ControlProgram ) ;
  pk.WriteInt( "Gain" , m_iGain ) ;
  pk.WriteInt("Trigger" , m_bTriggerMode ? 1 : 0 ) ;

  text = pk ;
  return true;
}

bool SoliosDalsa::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  pk.GetInt("FrameRate" , m_iFramesPerSecond ) ;
  pk.GetInt( "Shutt_uS" , m_iShutter_us ) ;
  int iLiveVideo ;
  pk.GetInt( "Grab" , iLiveVideo ) ;
  m_bLiveVideo = ( iLiveVideo == -1 ) ;
  pk.GetString( "Program" , m_ControlProgram ) ;
  pk.GetInt( "Gain" , m_iGain ) ;
  int iTriggerMode ;
  pk.GetInt("Trigger" , iTriggerMode ) ;
  m_bTriggerMode = ( iTriggerMode != 0 ) ;
  C1394Camera::ScanProperties( text , Invalidate ) ;
  return true;
}
