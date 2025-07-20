#pragma once

#include <helpers/CameraData.h>

#define MAX_NUMBER_OF_USB_CAMERAS_IN_SYSTEM 32

class USBLocation
{
  FXString m_AsString ;
public:
  USBLocation() { Reset() ;  } ;
  USBLocation( __int64 iHub , int iPort , int VID , int PID )
  {
    m_iHub = iHub ; m_iPort = iPort ;
    m_Index = m_iHub * 100 + m_iPort;
    m_VID = VID ; m_PID = PID ; 
    GetLocationAsString();
  }
  USBLocation( __int64 i64Index , int VID = 0 , int PID = 0 )
  {
    m_iPort = ( int ) ( i64Index % 100L ) ;
    m_iHub = i64Index / 100 ;
    m_Index = i64Index ;
    m_VID = VID ; m_PID = PID ;
    GetLocationAsString() ;
  }
  void Reset() { m_Index = m_iHub = m_iPort = 0; m_VID = m_PID = 0; m_AsString.Empty(); }
  bool IsEmpty() { return ( !m_iHub || !m_iPort) ; }
  USBLocation( LPCTSTR pAsString ) { FromString( pAsString ) ; }
  // next function produces string "R<root>.H<hub>.P<port>"
  FXString& GetLocationAsString()
  {
    int iNumbers[8];
    __int64 iHub = m_iHub;
    int i = 0;
    for ( ; i < 8 && iHub != 0 ; i++)
    {
      iNumbers[i] = iHub % 100L;
      iHub /= 100L;
    }
    m_AsString = "R";
    FXString Add;

    for ( int j = i - 1 ; j >= 0 ; j-- )
    {
      Add.Format("%d.", iNumbers[j]);
      m_AsString += Add;
    }
    Add.Format("%d", m_iPort);
    m_AsString += Add ;
    return m_AsString ;
  }
  FXString& GetFullDataAsString()
  {
    FXString USBLocation = GetLocationAsString();
    m_AsString.Format("%s/VID_%04X&PID_%04X&MI_%02X",
      (LPCTSTR)USBLocation , m_VID, m_PID, m_MI);
    return m_AsString;
  }
  // next function parses string "R<root>.H<hub>.P<port>" and sets parameters
  // returns true is parsing is OK, false if not; 
  // not parsed parameters are settled to -1
  bool FromString( const char * AsString )
  {
    FXString ForParse( AsString ) ;
    ForParse = ForParse.Trim( "/\\ :,.[]{}()" ).MakeUpper() ;
    m_iHub = m_iPort = -1 ;
    size_t iPos = 0 ;
    if ( ForParse[ 0 ] != _T( 'R' ) )
    {
      iPos = ForParse.Find( "/R" , iPos ) ;
      if ( iPos < 0 )
        return false ;
      iPos += 2 ;
    }
    else
      iPos++ ;

    int iRoot = atoi( (LPCTSTR) ForParse + iPos ) ;
    __int64 i64Hub = iRoot ;
    while ( iPos = ForParse.Find( '.' , iPos ) )
    {
      if (ForParse[++iPos] == 'P')
      {
        m_iPort = atoi((LPCTSTR)ForParse + iPos + 1 );
        break; //hub address decoding is finished
      }
      int iHubPort = atoi((LPCTSTR)ForParse + iPos);
      if (!iHubPort )
      {
        m_iHub = 0;
        return false;
      }
      i64Hub *= 100;
      if ( i64Hub > 20000000000000L )
      {
        ASSERT(i64Hub < 200000000000000000L); // more than 7 hubs in chain, first not zero is root number
        FxSendLogMsg(MSG_ERROR_LEVEL, "USB Enumeration", 0, "More than 7 USB hubs in chain");
        return false;
      }
      i64Hub += iHubPort;
    }
    m_iHub = i64Hub;
    iPos = ForParse.Find( "VID_" , iPos + 1 ) ;
    if ( iPos > 0 )
      m_VID = strtol( (LPCTSTR) ForParse + iPos + 4 , NULL , 16 ) ;
    else
      m_VID = 0 ;
    iPos = ForParse.Find( "PID_" , iPos + 3 ) ;
    if ( iPos > 0 )
      m_PID = strtol( (LPCTSTR) ForParse + iPos + 4 , NULL , 16 ) ;
    else
      m_PID = 0 ;
    iPos = ForParse.Find( "MI_" , iPos + 3 ) ;
    if ( iPos > 0 )
      m_MI = strtol( (LPCTSTR) ForParse + iPos + 3 , NULL , 16 ) ;
    else
      m_MI = 0 ;
    m_Index = m_iHub * 100 + m_iPort ;
    return true ;
  }
  bool IsMatched( USBLocation& Other )
  {
    return (memcmp( this , &Other , sizeof( *this ) ) == 0) ;
  }
  bool IsMatched( CameraData& Other )
  {
    USBLocation Location( Other.m_ViewName.c_str() ) ;
    return IsMatched( Location ) ;
  }
  bool IsMatched(__int64 iOtherIndex )
  {
    if (!m_Index || !iOtherIndex)
      return false ;
    return (m_Index == iOtherIndex) ;
  }
  __int64 m_iHub ;
  int m_iPort ;
  int m_VID ;
  int m_PID ;
  int m_MI ;
  
  // Index is camera location in USB tree as one __int64 number
  // Text presentation of this number: number should be divided to two digits 
  // groups. Right group is port number in hub where camera connected, 
  // next two digits - port number where is hub input connected and so on
  // till last left one or two digits: it's USB root bus number
  __int64 m_Index ;
};

class CameraInSystem
{
public:
  CameraInSystem()
  {
    memset( this , 0 , sizeof( *this ) );
  }

  CameraInSystem( USBLocation Location ,
    void * pLocalPtr , DWORD ProcessId = NULL )
  {
    m_Location = Location ;
    m_pLocalPtr = pLocalPtr ;
    m_ProcessId = ProcessId ;
  }
  bool SetVID_PID_MI( LPCTSTR AsString )
  {
    FXString ForParse( AsString ) ;
    ForParse = ForParse.Trim( " ([{}]);" ).MakeUpper() ;
    size_t iPos = ForParse.Find( "VID_" ) ;
    if ( iPos >= 0 )
    {
      m_Location.m_VID = strtol( (LPCTSTR) ForParse + 4 , NULL , 16 ) ;
      if ( m_Location.m_VID && (iPos = ForParse.Find( "PID_" , iPos + 5 )) )
      {
        m_Location.m_PID = strtol( (LPCTSTR) ForParse + 4 , NULL , 16 ) ;
        if ( m_Location.m_PID )
        {
          if ( iPos = ForParse.Find( "MI_" , iPos + 5 ) )
            m_Location.m_MI = strtol( (LPCTSTR) ForParse + 3 , NULL , 16 ) ;
          else
            m_Location.m_MI = 0 ;
        }
      }
    }
  }
  bool IsMatched( LPCTSTR pOtherAsString )
  {
    FXString ForCompare(pOtherAsString);
    FXString ThisAsString = m_Location.GetFullDataAsString() ;
    ForCompare = ForCompare.MakeUpper() ;
    return ( ForCompare.Find( ThisAsString ) >= 0 ) ;
  }
  bool IsMatched( CameraData& Other )
  {
    return this->m_Location.IsMatched( Other ) ;
  }
  USBLocation m_Location ;
  char        m_FriendlyName[100];
  char        m_FullInfo[256];
  void *    m_pLocalPtr ;
  DWORD     m_ProcessId ;
};

extern int GetNAllocatedUSBCAmeras();
// Following program returns camera index in global cameras array
// If not found returns - 1
extern int GetCameraForLocation( __int64 Location ) ;


extern CameraInSystem g_USBCameras[MAX_NUMBER_OF_USB_CAMERAS_IN_SYSTEM];
// Index is camera location in USB tree as one __int64 number
// (look comment to m_Index member in USBLocation class
// Only available indexes will be in following array, number of indexes is
// in g_dwNumOfConnectedCameras
extern __int64 g_ActiveIndexes[MAX_NUMBER_OF_USB_CAMERAS_IN_SYSTEM];
// Mutex is global for all applications
extern HANDLE  g_hGlobalMutex ;
extern DWORD   g_dwNumOfConnectedCameras ;
