// Machine generated IDispatch wrapper class(es) created with ClassWizard

#include "stdafx.h"
#include "imcntl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

RequestArray IImCNTL::m_RQImaging ;
/////////////////////////////////////////////////////////////////////////////
// IImCNTL properties

/////////////////////////////////////////////////////////////////////////////
// IImCNTL operations

long IImCNTL::GrabBack( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GRABBACK) ;
    int iStat = m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 2000 )  ;
    return ( iStat > 0 ) ;
  }
  return 0 ;
}

long IImCNTL::Grab(long iNFrames , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GRAB , 16) ;
    Msg.m_longIntParams[0] = iNFrames ; // -1 -> Live Video, 0 - Stop Grab, >0 -> One Grab
    int iStat = m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 1000 )  ;
    return ( iStat > 0 ) ;
  }
  return 0 ;
}

long IImCNTL::MeasureBlobs(double dNormThreshold , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg( MEASUREBLOBS ) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_dDoubleParams[0] = dNormThreshold ;
    int iStat = m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 2000 )  ;
    if ( iStat > 0 ) 
      return Msg.m_longIntParams[0] ;
    else
      return iStat ;
  }
  return 0 ;
}

CString IImCNTL::GetBlobParam(long iBlobNumber , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg( GETBLOBPARAM ); 
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iBlobNumber ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return CString(Msg.m_szStringData) ;
    else
    {
      if ( Msg.m_szStringData[0] )
        return CString(Msg.m_szStringData) ;
    }
  }
  return CString("ERROR in communication") ;
}

void IImCNTL::SetFFTFlag(long bDoFFT , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg( SETFFTFLAG ) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = bDoFFT ;
    m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) ;
  }
}

void IImCNTL::SetCameraScale(double dScaleXmicronPerPixel, 
  double dScaleYmicronPerPixel , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETCAMERASCALE) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_dDoubleParams[0] = dScaleXmicronPerPixel ;
    Msg.m_dDoubleParams[1] = dScaleYmicronPerPixel ;
    m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) ;
  }
}

double IImCNTL::GetCameraScale(long iAxeNumber , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETCAMERASCALE) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iAxeNumber ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
          Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_dDoubleParams[0] ;
    else
      return 0. ;
  }
  return 0. ;
}

long IImCNTL::InitOperation(long InitValue , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(INITOPERATION) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = InitValue ;
    int iStat = m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) ;
    if ( iStat < 0 )
      return iStat ;
    return Msg.m_longIntParams[0] ;
  }
  return 0 ;
}
bool bIn_usecs = false ;
long IImCNTL::GetExposure( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETEXPOSURE) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
            Msg , Msg.m_iMessageLen , 200 ) >= 0 )
    {
      return (bIn_usecs) ? int(0.1 + (Msg.m_longIntParams[0]/m_dScanTime_usec)) 
        : Msg.m_longIntParams[0] ;
    }
  }
  return 0 ;
}

long IImCNTL::GetGain( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETGAIN) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return -1 ;
}
long IImCNTL::GetTriggerDelay( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETTRIGGERDELAY) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return -1 ;
}
long IImCNTL::GetDebouncing( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETDEBOUNCING) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return -1 ;
}

long IImCNTL::SetExposure(long iNewExposureInScans,  int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETEXPOSURE) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iNewExposureInScans ;
    Msg.m_longIntParams[1] = 0 ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 1200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
    else
      return 0 ;
  }
  return 0 ;
}
long IImCNTL::SetGain(long iNewGain,  int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETGAIN) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iNewGain ;
    Msg.m_longIntParams[1] = 0 ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 1200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return 0 ;
}


long IImCNTL::SavePicture (int iSave , int iCamNum )
{   
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SAVEPICTURE) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iSave ;
    Msg.m_longIntParams[1] = iCamNum ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 1200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }

  return 0;
}


long IImCNTL::EnableLPFilter (int iEnable , int iCamNum )
{   
//   CameraControlMessage Msg(ENABLELPFILTER) ;
//   Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
//   Msg.m_longIntParams[0] = iEnable ;
//   if (m_RQImaging[0]->ProcessRequest( 
//     Msg , Msg.m_iMessageLen , 1200 ) >= 0 )
//     return Msg.m_longIntParams[0] ;
//   return 0;

  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(ENABLELPFILTER) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iEnable ;
    Msg.m_longIntParams[1] = iCamNum ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 1200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return 0 ;
}


long IImCNTL::SetTriggerDelay(long iNewTriggerDelay_uS ,  int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETTRIGGERDELAY) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iNewTriggerDelay_uS ;
    Msg.m_longIntParams[1] = iCamNum ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 1200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return 0 ;
}

long IImCNTL::SetDebouncing(long iNewDebouncing_units ,  int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETDEBOUNCING) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iNewDebouncing_units ;
    Msg.m_longIntParams[1] = 0 ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 1200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return 0 ;
}

long IImCNTL::MeasureLines(double dNormThres,  int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(MEASURELINES , 100) ;
    Msg.m_dDoubleParams[0] = dNormThres ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 1200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
    else
      return 0 ;
  }
  return 0 ;
}

CString IImCNTL::GetLineParam(long LineNumber,  int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETLINEPARAM) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = LineNumber ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return CString(Msg.m_szStringData) ;
    else
    {
      if ( Msg.m_szStringData[0] )
        return CString(Msg.m_szStringData) ;
    }
  }
  return CString("ERROR in communication") ;
}

long IImCNTL::GetLastMinIntensity(  int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETLASTMININTENSITY) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
    else
      return 0 ;
  }
  return 0 ;
}

long IImCNTL::GetLastMaxIntensity( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETLASTMAXINTENSITY) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
    else
      return 0 ;
  }
  return 0 ;
}

void IImCNTL::SetFFTZone(long iXc, long iYc , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETFFTZONE) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iXc ;
    Msg.m_longIntParams[1] = iYc ;
    m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) ;
  }
}

long IImCNTL::DoFFTFiltration( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(DOFFTFILTRATION) ;
    return m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) ;
  }
  return 0 ;
}

long IImCNTL::SaveRestoreImage(long bSave , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
//     CameraControlMessage Msg(SAVERESTOREIMAGE) ;
//     return m_RQImaging[iCamNum]->ProcessRequest( 
//       Msg , Msg.m_iMessageLen , 200 ) ;
    return 0 ;
  }
  return 0 ;
}

double IImCNTL::SetGetMeasAreaExpansion(
  long bSet, double dExp_in_um , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
//     CameraControlMessage Msg(GRAB) ;
//     return m_RQImaging[iCamNum]->ProcessRequest( Msg , Msg.m_iMessageLen , 200 ) ;
    return 0 ;
  }
  return 0 ;
}

double IImCNTL::GetMaxIntensity( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETMAXINTENSITY) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return 0 ;
}

long IImCNTL::SetWinPos(long lx, long ly , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETWINPOS) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = lx ;
    Msg.m_longIntParams[1] = ly ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return 0 ;
}

void IImCNTL::SetExposureStartPosition(
  long iExpStartPos , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETEXPOSURESTARTPOSITION) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iExpStartPos ;
    m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) ;
  }
}

long IImCNTL::GetExposureStartPosition( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETEXPOSURESTARTPOSITION) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return 0 ;
}

long IImCNTL::MeasurePower( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(MEASUREPOWER) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
  }
  return 0 ;
}

CString IImCNTL::GetPowerData(long iBlobNumber , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETPOWERDATA) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iBlobNumber ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return CString(Msg.m_szStringData) ;
    else
    {
      if ( Msg.m_szStringData[0] )
        return CString(Msg.m_szStringData) ;
    }
  }
  return CString("ERROR in communication") ;
}

void IImCNTL::SetIntegrationRadius(long iRadius , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETINTEGRATIONRADIUS) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iRadius ;
    m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) ;
  }
}

long IImCNTL::GetIntegrationRadius( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETINTEGRATIONRADIUS) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return Msg.m_longIntParams[0] ;
    else
      return 0 ;
  }
  return 0 ;
}

double IImCNTL::MeasLinePower(long iLineDirection, 
  long iMeasureHalfWidth, long iCompensationWidth , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(MEASLINEPOWER) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iLineDirection ;
    Msg.m_longIntParams[0] = iMeasureHalfWidth ;
    Msg.m_longIntParams[0] = iCompensationWidth ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 800 ) >= 0 )
      return Msg.m_dDoubleParams[0] ;
    else
      return 0 ;
  }
  return 0 ;
}

void IImCNTL::SetMinArea(long iMinArea , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETMINAREA) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iMinArea ;
    m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) ;
  }
}

double IImCNTL::AbstractCall(long iCallID, LPCTSTR pszParameters, 
  long iPar, double dPar , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(ABSTRACTCALL) ;
    Msg.m_iMessageLen = sizeof(Msg) ;
    Msg.m_longIntParams[0] = iCallID ;
    if ( pszParameters )
      strcpy_s(Msg.m_szStringData , pszParameters ) ;
    Msg.m_longIntParams[1] = iPar ;
    Msg.m_dDoubleParams[0] = dPar ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 800 ) >= 0 )
      return Msg.m_dDoubleParams[0] ;
    else
      return 0 ;
  }
  return 0 ;
}

long IImCNTL::SetSyncMode(long bAsyncMode , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETSYNCMODE) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = bAsyncMode ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 800 ) > 0 )
      return 1 ;
  }
  return 0 ;
}

long IImCNTL::SetMarkerPosisiton(long iMarkerType, long iXPos, 
  long iYPos, long dwMarkerColor , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
//     CameraControlMessage Msg(GRAB) ;
//     return m_RQImaging[iCamNum]->ProcessRequest( Msg , Msg.m_iMessageLen , 200 ) ;
    return 0 ;
  }
  return 0 ;
}

CString IImCNTL::GetDiffractionMeasPar( int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(GETDIFFRACTIONMEASPAR) ;
    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return CString(Msg.m_szStringData) ;
    else
    {
      if ( Msg.m_szStringData[0] )
        return CString(Msg.m_szStringData) ;
      return CString("ERROR in communication") ;
    }
  }
  return CString("ERROR in communication") ;
}

CString IImCNTL::SetDiffractionMeasPar(long iMeasMode, long iXDist_pix, 
  long iYDist_pix, long iBackgoundDist_pix , int iCamNum )
{
  if ( iCamNum < m_RQImaging.GetCount() )
  {
    CameraControlMessage Msg(SETDIFFRACTIONMEASPAR) ;
    Msg.m_iMessageLen = 10 + (DWORD)&Msg.m_szStringData - (DWORD) &Msg ;
    Msg.m_longIntParams[0] = iMeasMode ;
    Msg.m_longIntParams[1] = iXDist_pix ;
    Msg.m_longIntParams[2] = iYDist_pix ;
    Msg.m_longIntParams[3] = iBackgoundDist_pix ;

    if (m_RQImaging[iCamNum]->ProcessRequest( 
      Msg , Msg.m_iMessageLen , 200 ) >= 0 )
      return CString(Msg.m_szStringData) ;
    else
    {
      if ( Msg.m_szStringData[0] )
        return CString(Msg.m_szStringData) ;
      return CString("ERROR in communication") ;
    }
  }
  return CString("ERROR in communication") ;
}

