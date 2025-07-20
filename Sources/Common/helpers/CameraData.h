#pragma once

#include <string>

using namespace std ;

enum class DeviceType { Audio , Video };

typedef enum // flags as is could be used for camera and amplifier control
{
  Control_Flags_Auto = 0x1 ,
  Control_Flags_Manual = 0x2
}	ControlFlags;

typedef enum 
{
  CN_Camera = 1 ,
  CN_Amplifier = 2
} ControlNode ;

class CaptureControl
{
public:
  long m_Prop ;

  ControlNode m_Node ;

  long m_lMin ;
  long m_lMax ;
  long m_lStep ;
  long m_lDefault ;
  long m_lFlags;
  long m_lCurrentValue ;
  long m_lCurrentFlags ;
  bool m_bAuto ;

  char m_Name[ 50 ] ;

  CaptureControl() { memset( this , 0 , sizeof( *this ) ) ; }
};

typedef vector<CaptureControl> ControlVector ;

class CaptureMode
{
public:
  UINT32 m_Fourcc ;
  UINT32 m_uiWidth ;
  UINT32 m_uiHeight ;
  int    m_iFPS ;
  int    m_iModeIndexInCamera ;
  char   m_AsFOURCC[30] ;
  UINT32 m_uiFPSnum ;
  UINT32 m_uiFPSden ;

  GUID   m_Guid ;

  CaptureMode() { memset( this , 0 , sizeof( *this ) ) ; }
} ;

typedef vector<CaptureMode> CaptureModes ;

class CameraData
{
public:
  string m_CameraFriendlyName ;
  string m_FullSymbolicLink ;
  string m_ViewName ;
  string m_Location ;
  string m_VID_PID_MI ;
  string m_Instance ;
  string m_SelectedMode ;
  CaptureModes m_CaptureModes ;
  ControlVector m_ControlVector ;
  __int64 m_Index ;

  vector<string> m_CaptureModesAsStrings ;
  string m_CaptureModesForList ;

  CameraData( LPCTSTR pFriendlyName = NULL ) 
  { 
    if ( pFriendlyName )
      m_CameraFriendlyName = pFriendlyName ; 
  }
  bool operator==( CameraData& Other ) { return (m_ViewName == Other.m_ViewName) ; }
  bool AreIdAndLocationTheSame( CameraData& Other ) 
  { 
    return (m_Location == Other.m_Location)  ; 
  }

  CameraData& operator=( CameraData& Other )
  {
    m_CameraFriendlyName = Other.m_CameraFriendlyName;
    m_FullSymbolicLink = Other.m_FullSymbolicLink;
    m_ViewName = Other.m_ViewName;
    m_VID_PID_MI = Other.m_VID_PID_MI;
    m_Location = Other.m_Location;
    m_Instance = Other.m_Instance;
    m_SelectedMode = Other.m_SelectedMode ;
    m_CaptureModes = Other.m_CaptureModes ;
    m_ControlVector = Other.m_ControlVector ;
    m_CaptureModesAsStrings = Other.m_CaptureModesAsStrings ;
    m_CaptureModesForList = Other.m_CaptureModesForList ;
    m_Index = Other.m_Index ;
    return *this ;
  }
};

typedef vector<CameraData> CamerasVector ;
