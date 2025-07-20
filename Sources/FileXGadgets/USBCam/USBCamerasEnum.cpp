#include "stdafx.h"
/*++

Copyright (c) 1997-2011 Microsoft Corporation

Module Name:

    USBEnum.cpp // Changed at Nov 29, 2021, by Moisey, was ENUM.C

Abstract:

    This source file contains the routines which enumerate the USB bus
    and populate the TreeView control.

    The enumeration process goes like this:

    (1) Enumerate Host Controllers and Root Hubs
    EnumerateHostControllers()
    EnumerateHostController()
    Host controllers currently have symbolic link names of the form HCDx,
    where x starts at 0.  Use CreateFile() to open each host controller
    symbolic link.  Create a node in the TreeView to represent each host
    controller.

    GetRootHubName()
    After a host controller has been opened, send the host controller an
    IOCTL_USB_GET_ROOT_HUB_NAME request to get the symbolic link name of
    the root hub that is part of the host controller.

    (2) Enumerate Hubs (Root Hubs and External Hubs)
    EnumerateHub()
    Given the name of a hub, use CreateFile() to map the hub.  Send the
    hub an IOCTL_USB_GET_NODE_INFORMATION request to get info about the
    hub, such as the number of downstream ports.  Create a node in the
    TreeView to represent each hub.

    (3) Enumerate Downstream Ports
    EnumerateHubPorts()
    Given an handle to an open hub and the number of downstream ports on
    the hub, send the hub an IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
    request for each downstream port of the hub to get info about the
    device (if any) attached to each port.  If there is a device attached
    to a port, send the hub an IOCTL_USB_GET_NODE_CONNECTION_NAME request
    to get the symbolic link name of the hub attached to the downstream
    port.  If there is a hub attached to the downstream port, recurse to
    step (2).

    GetAllStringDescriptors()
    GetConfigDescriptor()
    Create a node in the TreeView to represent each hub port
    and attached device.


Environment:

    user mode

Revision History:

    04-25-97 : created

--*/

//*****************************************************************************
// I N C L U D E S
//*****************************************************************************

#include <strmif.h>
#include <Dvdmedia.h>
#include <DShow.h>

#include <string>
#include <vector>
#include <fxfc/fxfc.h>
#include <helpers/CameraData.h>
#include "USBCamInSystem.h"
#include <helpers/USBEnum.h>


//*****************************************************************************
// D E F I N E S
//*****************************************************************************

#define THIS_MODULENAME ("USBCamera")

#define NUM_STRING_DESC_TO_GET 32

typedef struct _tag_capstuff
{
  // Interfaces
  ICaptureGraphBuilder2  *s_pBuilder;
  IMoniker               *s_rgpmVideoMenu[30];
  IMoniker               *s_pmVideo;
  IBaseFilter            *s_pVCap;
  IBaseFilter            *s_pRenderer;
  IGraphBuilder          *s_pFg;
  IAMDroppedFrames       *s_pDF;
  //    IFileSinkFilter        *s_pSink;
  IMediaControl          *s_pMediaControl;
  IConfigAviMux          *s_pConfigAviMux;
  IAMVideoCompression    *s_pVC;
  IAMStreamConfig        *s_pVSC;      // for video cap
  IAMVfwCaptureDialogs   *s_pDlg;
  IMediaEventEx          *s_pME;
  //CVFilter               *s_VideoRenderer;
  // State and settings
  LPBITMAPINFOHEADER      s_lpBMIH;
  int                     s_iNumVCapDevices;
  int                     s_iDevNumSelected;
  bool                    s_bUseFrameRate;
  bool                    s_fCapturing;
  bool                    s_fCCAvail;
  bool                    s_fCapCC;
  bool                    s_fDeviceMenuPopulated;
  bool                    s_fPreviewing;
  bool                    s_fCaptureGraphBuilt;
  bool                    s_fPreviewGraphBuilt;
  bool                    s_fWantPreview;
  bool                    s_fWantCapture;
  long                    s_lDroppedBase;
  long                    s_lNotBase;
  double                  s_FrameRate;
  LONG                    s_NumberOfVideoInputs;
  // data
  WCHAR                   s_wachFriendlyName[120];
  //CCrossbar              *s_pCrossbar;
  // notificators
  //    PUnregisterDeviceNotification s_gpUnregisterDeviceNotification;
  //    PRegisterDeviceNotification   s_gpRegisterDeviceNotification;
  //    HDEVNOTIFY                    s_ghDevNotify;
  CWnd*                         s_NotifyWnd;
}_capstuff;

//*****************************************************************************
// G L O B A L S
//*****************************************************************************

UCHAR   g_chMJPEGFrameDefault;
UCHAR   g_chUNCFrameDefault;
UCHAR   g_chVendorFrameDefault;
UCHAR   g_chFrameBasedFrameDefault;
// Spec version of UVC device
UINT g_chUVCversion;

//using namespace std;

bool ExtractCameraData(FXString& DevicePathAsText,
  CameraData& Result, FXString& AllUSBDevices)
{
  size_t i1stHashPos = DevicePathAsText.Find('#');
  size_t i2ndHashPos = DevicePathAsText.Find('#', i1stHashPos + 1);
  size_t i3rdHashPos = DevicePathAsText.Find('#', i2ndHashPos + 1);
  if (i2ndHashPos > 0 && i3rdHashPos > 0)
  {
    Result.m_VID_PID_MI = DevicePathAsText.Mid(
      i1stHashPos + 1, i2ndHashPos - i1stHashPos - 1).MakeUpper();
    FXString Inst = DevicePathAsText.Mid(
      i2ndHashPos + 1, i3rdHashPos - i2ndHashPos - 1).MakeUpper();
    size_t iPos = AllUSBDevices.Find(Inst);
    if (iPos >= 0)
    {
      size_t iNextCR = AllUSBDevices.Find('\n', iPos);
      FXString OurString = AllUSBDevices.Left(iNextCR);
      size_t iPrevCR = OurString.ReverseFind('\n');
      if (iPrevCR > 0)
        OurString = OurString.Mid(iPrevCR + 1);
      size_t i1stSlash = OurString.Find('/');
      size_t i2ndSlash = OurString.Find('/', i1stSlash + 1);
      Result.m_FullSymbolicLink = DevicePathAsText;
      Result.m_Location = OurString.Mid(i1stSlash + 1, i2ndSlash - i1stSlash - 1);
      Result.m_Instance = Inst;
      Result.m_ViewName = Result.m_CameraFriendlyName
        + '/' + Result.m_Location + '/' + Result.m_VID_PID_MI;
      USBLocation Loc(Result.m_ViewName.c_str());
      Result.m_Index = Loc.m_Index;
      return true;
    }
  }
  return false;
}

bool EnumCameras(CamerasVector& AllCameras, ULONG& ulLastEnumDevicesNumber)
{
  HRESULT hr;
  ULONG cFetched;
  IMoniker *pM;
  IEnumMoniker *pEm;
  ICreateDevEnum *pCreateDevEnum;
  UINT    uIndex = 0;

  // Clean up all
  AllCameras.clear();

  FXString AllUSBDevices = GetAllUSBDevices();

  hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
    CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
  IFENUMDEV_RETURN(hr, "Error Creating Device Enumerator");

  hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
  IFENUMDEV_RETURN(hr, "There are no video capture hardware");

  pEm->Reset();
  while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK)
  {
    IPropertyBag *pBag;
    hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
    if (SUCCEEDED(hr))
    {
      VARIANT var, var2;
      var.vt = VT_BSTR;
      var2.vt = VT_BSTR;
      hr = pBag->Read(L"FriendlyName", &var, NULL);
      if (hr == NOERROR)
      {
        FXString Name = Wide2FXStr(var.bstrVal);
        SysFreeString(var.bstrVal);
        CameraData NewCamera(Name);
        hr = pBag->Read(L"DevicePath", &var2, NULL);
        if (hr == NOERROR)
        {
          FXString Id = Wide2FXStr(var2.bstrVal);
          SysFreeString(var2.bstrVal);
          if (ExtractCameraData(Id, NewCamera, AllUSBDevices))
          {
            AllCameras.push_back(NewCamera);
          }
        }
      }
    }
  }
  return (AllCameras.size() != 0);
}

