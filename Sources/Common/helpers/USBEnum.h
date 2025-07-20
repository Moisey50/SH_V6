#pragma once

#include <helpers/uvcview.h>
#include <fxfc/fxfc.h>

typedef PVOID HDEVINFO;

extern DEVICE_GUID_LIST gHubList;
extern DEVICE_GUID_LIST gDeviceList;

FORCEINLINE VOID InitializeListHead( _Out_ PLIST_ENTRY ListHead )
{
  ListHead->Flink = ListHead->Blink = ListHead;
}

VOID EnumerateHostControllers(
  HTREEITEM  hTreeParent ,
  ULONG     *DevicesConnected
);

const char * GetDevices() ;

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))


#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}


#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

FORCEINLINE void Oops(char * pFile, ULONG String)
{
  TRACE("\nOops!!!! File=%s, Line=%u", pFile, String);
}

#define IFENUMDEV_RETURN(hr,mes) if (hr!=NOERROR) { \
    SENDERR("--- CWDMDriver::EnumDevices(): %s",mes); return hr; }


//*****************************************************************************
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//*****************************************************************************

VOID
EnumerateHostControllers(
  HTREEITEM  hTreeParent,
  ULONG     *DevicesConnected
);

VOID
EnumerateHostController(
  HTREEITEM                hTreeParent,
  HANDLE                   hHCDev,
  _Inout_ PCHAR            leafName,
  _In_    HANDLE           deviceInfo,
  _In_    PSP_DEVINFO_DATA deviceInfoData
);

VOID
EnumerateHub(
  HTREEITEM                                       hTreeParent,
  _In_reads_(cbHubName) PCHAR                     HubName,
  _In_ size_t                                     cbHubName,
  _In_opt_ PUSB_NODE_CONNECTION_INFORMATION_EX    ConnectionInfo,
  _In_opt_ PUSB_NODE_CONNECTION_INFORMATION_EX_V2 ConnectionInfoV2,
  _In_opt_ PUSB_PORT_CONNECTOR_PROPERTIES         PortConnectorProps,
  _In_opt_ PUSB_DESCRIPTOR_REQUEST                ConfigDesc,
  _In_opt_ PUSB_DESCRIPTOR_REQUEST                BosDesc,
  _In_opt_ PSTRING_DESCRIPTOR_NODE                StringDescs,
  _In_opt_ PUSB_DEVICE_PNP_STRINGS                DevProps
);

VOID
EnumerateHubPorts(
  HTREEITEM   hTreeParent,
  HANDLE      hHubDevice,
  ULONG       NumPorts
);

PCHAR GetRootHubName(
  HANDLE HostController
);

PCHAR GetExternalHubName(
  HANDLE  Hub,
  ULONG   ConnectionIndex
);

PCHAR GetHCDDriverKeyName(
  HANDLE  HCD
);

PCHAR GetDriverKeyName(
  HANDLE  Hub,
  ULONG   ConnectionIndex
);

PUSB_DESCRIPTOR_REQUEST
GetConfigDescriptor(
  HANDLE  hHubDevice,
  ULONG   ConnectionIndex,
  UCHAR   DescriptorIndex
);

PUSB_DESCRIPTOR_REQUEST
GetBOSDescriptor(
  HANDLE  hHubDevice,
  ULONG   ConnectionIndex
);

DWORD
GetHostControllerPowerMap(
  HANDLE hHCDev,
  PUSBHOSTCONTROLLERINFO hcInfo);

DWORD
GetHostControllerInfo(
  HANDLE hHCDev,
  PUSBHOSTCONTROLLERINFO hcInfo);

PCHAR WideStrToMultiStr(
  _In_reads_bytes_(cbWideStr) PWCHAR WideStr,
  _In_ size_t                   cbWideStr
);

BOOL
AreThereStringDescriptors(
  PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
  PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
);

PSTRING_DESCRIPTOR_NODE
GetAllStringDescriptors(
  HANDLE                          hHubDevice,
  ULONG                           ConnectionIndex,
  PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
  PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
);

PSTRING_DESCRIPTOR_NODE
GetStringDescriptor(
  HANDLE  hHubDevice,
  ULONG   ConnectionIndex,
  UCHAR   DescriptorIndex,
  USHORT  LanguageID
);

HRESULT
GetStringDescriptors(
  _In_ HANDLE                         hHubDevice,
  _In_ ULONG                          ConnectionIndex,
  _In_ UCHAR                          DescriptorIndex,
  _In_ ULONG                          NumLanguageIDs,
  _In_reads_(NumLanguageIDs) USHORT  *LanguageIDs,
  _In_ PSTRING_DESCRIPTOR_NODE        StringDescNodeHead
);

void
EnumerateAllDevices();

void
EnumerateAllDevicesWithGuid(
  PDEVICE_GUID_LIST DeviceList,
  LPGUID Guid
);

void
FreeDeviceInfoNode(
  _In_ PDEVICE_INFO_NODE *ppNode
);

PDEVICE_INFO_NODE
FindMatchingDeviceNodeForDriverName(
  _In_ PSTR    DriverKeyName,
  _In_ BOOLEAN IsHub
);

int EnumerateChildren(DEVINST DevInst,
  HTREEITEM hTreeParent, LPARAM    lParam, TREEICON  TreeIcon);

FXString GetAllUSBDevices();
PCHAR WideStrToMultiStr(
  _In_reads_bytes_(cbWideStr) PWCHAR WideStr,
  _In_ size_t                   cbWideStr
);




