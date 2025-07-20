#include "stdafx.h"
#include "ImagView.h"
#include "MeasurementSet.h"
#include "SMCamControl.h"

#define RECEIVE_MSG_SIZE 1024

CSMCamControl::CSMCamControl( LPCTSTR pShMemName , 
                             LPCTSTR pInEventName , LPCTSTR pOutEventName , 
                             CImageView * pView , CMeasurementSet * pSet )
                             : CShMemControl( 1024 , 2048 , 8192 , 
                             pShMemName , pInEventName , pOutEventName )
{ 
  SetHostView( pView ) ; 
  SetHostSet( pSet ) ; 
  m_ShMemName = pShMemName ; m_InEventName = pInEventName ;
  m_OutEventName = pOutEventName ; 
  m_bExit = false ;

  m_hProcessingThread = CreateThread( NULL , 0 , 
    ControlFunction , this , CREATE_SUSPENDED , 
    &m_dwThreadId ) ;
  char ThreadName[80] ;
  int iAreaNameLen = strlen( pShMemName ) ;
  int i = 0 ;
  for ( ; i < iAreaNameLen ; i++ )
    ThreadName[i] = (char)pShMemName[i] ;
  ThreadName[i] = 0 ;
  strcat_s( ThreadName , "_Thread") ;
  SetThreadName( ThreadName , m_dwThreadId ) ;
  ResumeThread( m_hProcessingThread ) ;
} ;


CSMCamControl::~CSMCamControl(void)
{
  m_bExit = true ;
  SetInEvent() ;
  WaitForSingleObject( m_hProcessingThread , INFINITE ) ;
//   m_bExit = true ;
}

DWORD WINAPI CSMCamControl::ControlFunction( LPVOID pParam )
{
  CSMCamControl * pControl = (CSMCamControl*)pParam ;
  BYTE ReceiveBuf[RECEIVE_MSG_SIZE] ;
  while ( !pControl->m_bExit )
  {
    WaitForSingleObject( pControl->m_hInEvent , INFINITE ) ;
    if ( !pControl->m_bExit )
    {
      int iMsgLen = pControl->ReceiveRequest( ReceiveBuf , sizeof(ReceiveBuf) ) ;
      if ( iMsgLen > 0 )
        pControl->Process( ReceiveBuf , iMsgLen ) ;
    }
  }
  return 0 ;
}


int CSMCamControl::Process( BYTE * pMsg , int iMsgLen )
{
  CameraControlMessage * p = (CameraControlMessage *)pMsg ;
  CameraControlMessage Answer ;
  CString Status ;
  switch( p->m_iId)
  {
  case GRAB :
    {
      if (m_pMyMeasSet->m_GraphMode==1)
      {
        m_pMyMeasSet->m_SpotResults.Lock() ;
        m_pMyMeasSet->m_dCaptureFinishedAt = 0.0 ;
        m_pMyMeasSet->m_iMaxBlobNumber = -1 ;
        m_pMyMeasSet->m_SpotResults.RemoveAll() ;
        m_pMyMeasSet->m_LineResults.RemoveAll() ;
        m_pMyMeasSet->m_ProcLResults.RemoveAll() ;
        m_pMyMeasSet->m_SpotResults.Unlock() ;
        Answer.m_iId = m_pMyMeasSet->SetTextThroughLAN("GRAB");
      }
      else
        Answer.m_iId = m_pMyMeasSet->Grab() ;
      // if ( Answer.m_iId != OPERATION_ERROR )
      //   ResetEvent(m_pMyMeasSet->m_hEVDataImg);
    }
    break ;
  case GRABBACK :
    if (m_pMyMeasSet->m_GraphMode==1)
      m_pMyMeasSet->SetTextThroughLAN("GRABBACK");
    else
      Answer.m_iId = m_pMyMeasSet->GrabBack() ;
    break ;
  case CLEARBACK :
    if (m_pMyMeasSet->m_GraphMode==1)
      m_pMyMeasSet->SetTextThroughLAN("CLEARBACK");
    else
      Answer.m_iId = m_pMyMeasSet->ClearBack() ;
    break ;
  case MEASUREBLOBS :
    {
      double dNormThreshold = p->m_dDoubleParams[0] ;
      //m_pView->m_ImParam.m_BinThreshold = dNormThreshold ;
      Answer.m_iId = m_pMyMeasSet->MeasureBlobs( dNormThreshold ) ;
      if ( Answer.m_iId >= 0 && Answer.m_iId <= MEASUREBLOBS )
      {
        Answer.m_longIntParams[0] = (int)Answer.m_iId ;
        Answer.m_iId = MEASUREBLOBS ;
      }
    }
    break ;
  case GETBLOBPARAM :
    {
      Answer.m_iId = m_pMyMeasSet->GetBlobParam( p->m_longIntParams[0]) ;
    }   
    break ;
  case GETEXPOSURE :
    {
      int iNewExp ;
      Answer.m_iId = m_pMyMeasSet->GetExposure( iNewExp ) ;
      Answer.m_longIntParams[0] = iNewExp ;
    }
    break ;
  case SETEXPOSURE :
    {
      CString sExpo;
      if (p->m_longIntParams[1]==TRUE)//convert to scans
      {
        int iExpo = int(p->m_longIntParams[0]/m_pMyMeasSet->m_dScanTime_us);
        sExpo.Format("%d",iExpo);
      }
      else
        sExpo.Format("%d",p->m_longIntParams[0]);

      sExpo = ("SETEXPOSURE__") + sExpo;
      m_pMyMeasSet->SetTextThroughLAN(sExpo);

      Answer.m_iId = m_pMyMeasSet->SetExposure( 
        p->m_longIntParams[0] , p->m_longIntParams[1] != 0 ) ;
    }
    break ;
  case GETGAIN :
    {
      if (m_pMyMeasSet->m_GraphMode>0)     
      {
        Answer.m_longIntParams[0] = 0 ;
      }
      else
      {
        int iNewExp ;
        Answer.m_iId = m_pMyMeasSet->GetGain( iNewExp ) ;
        Answer.m_longIntParams[0] = iNewExp ;
      }
    }
    break ;
  case SETGAIN :
    Answer.m_iId = m_pMyMeasSet->SetGain( 
      p->m_longIntParams[0] ) ;
    break ;
  case SETTRIGGERDELAY:
    Answer.m_iId = m_pMyMeasSet->SetTriggerDelay(p->m_longIntParams[0] , p->m_longIntParams[1] ) ;
    break ;
  case GETTRIGGERDELAY:

    break ;
  case SETDEBOUNCING:

    break ;
  case GETDEBOUNCING:

    break ;
  case SET_THRESHOLD_FOR_BINARIZATION :
    Answer.m_iId = m_pMyMeasSet->SetThresholdForBinarization( 
      p->m_dDoubleParams[0] ) ;
    break ;
  case SET_THRESHOLD_FOR_ROTATION :
    Answer.m_iId = m_pMyMeasSet->SetThresholdForRotation( 
      p->m_dDoubleParams[0] ) ;
    break ;

  case SAVEPICTURE :
    Answer.m_iId = m_pMyMeasSet->SaveLastPicture(p->m_longIntParams[0]);
    break;

  case SETBMPPATH:
    //Answer.m_iId = m_pMyMeasSet->SetBmpPath(p->m_longIntParams[0]);
    break;

  case ENABLELPFILTER :
    Answer.m_iId = m_pMyMeasSet->EnableLPFilter(p->m_longIntParams[0]);
    break;

  case MEASURELINES :
    {
      double dNormThreshold = p->m_dDoubleParams[0] ;
      //m_pView->m_ImParam.m_BinThreshold = dNormThreshold ;
      Answer.m_iId = m_pMyMeasSet->MeasureLines( dNormThreshold ) ;
      if ( Answer.m_iId >= 0 && Answer.m_iId < MEASURELINES )
      {
        Answer.m_longIntParams[0] = (int)Answer.m_iId ;
        Answer.m_iId = MEASURELINES ;
      }
    }
    break ;
  case GETLINEPARAM :
    {
      Answer.m_iId = m_pMyMeasSet->GetLineParam( p->m_longIntParams[0]) ;
    }   
    break ;
  case GETMAXINTENSITY :
  case GETLASTMAXINTENSITY :
    {
      int iMaxIntens = 0 ;
      Answer.m_iId = m_pMyMeasSet->GetLastMaxIntensity( iMaxIntens) ;
      if ( Answer.m_iId > 0 )
        Answer.m_longIntParams[0] = iMaxIntens ;
    }
    break ;
  case GETLASTMININTENSITY :   
    {
      int iMinIntens = 0 ;
      Answer.m_iId = m_pMyMeasSet->GetLastMinIntensity( iMinIntens) ;
      if ( Answer.m_iId > 0 )
        Answer.m_longIntParams[0] = iMinIntens ;
    }
    break ;
  case SETWINPOS :
    Answer.m_iId = m_pMyMeasSet->SetWinPos(
      p->m_longIntParams[0] , p->m_longIntParams[1] ) ;
    break ;
  case SETMINAREA :
    Answer.m_iId = m_pMyMeasSet->SetMinArea(
      p->m_longIntParams[0] ) ;
    break ;
  case ABSTRACTCALL :
    Answer.m_iId = m_pMyMeasSet->AbstractCall(
      p->m_longIntParams[0] , p->m_szStringData ,
      p->m_longIntParams[1] , p->m_dDoubleParams[0]) ;
    break ;
  case SETSYNCMODE : 
    if (m_pMyMeasSet->m_GraphMode==1)
      m_pMyMeasSet->SetTextThroughLAN("SETSYNCMODE__TRUE");
    else
      Answer.m_iId = m_pMyMeasSet->SetSyncMode(
      p->m_longIntParams[0] ) ;
    break ;
  case GETDIFFRACTIONMEASPAR :
    Answer.m_iId = m_pMyMeasSet
      ->GetDiffractionMeasPar( p->m_szStringData) ;
    break ;
  case SETDIFFRACTIONMEASPAR :
    Answer.m_iId = m_pMyMeasSet->SetDiffractionMeasPar(
      p->m_longIntParams[0] , p->m_longIntParams[1] ,
      p->m_longIntParams[2] , p->m_longIntParams[3]) ;
    break ;
  case INITOPERATION :
    {
      if ( p->m_longIntParams[0] == -1 )
      {
        m_pView->m_RButtonDlg.m_iViewMode = 0 ;
        ::ShowWindow( m_pView->m_BigViewWnd.m_hWnd , SW_HIDE ) ;
        m_pMyMeasSet->m_pView->m_pDialog->PostMessage( WM_QUIT , 0 , 0 ) ;
      }
      Answer.m_iId = INITOPERATION ;
      p->m_longIntParams[0] = 0 ;
    }
    break ;
  case SETMARKERPOSISITON :
  case MEASLINEPOWER :
  case GETPOWERDATA :
  case MEASUREPOWER :
  case SETINTEGRATIONRADIUS :
  case GETINTEGRATIONRADIUS :
  case SETEXPOSURESTARTPOSITION :
  case GETEXPOSURESTARTPOSITION :
  case SAVERESTOREIMAGE :
  case SETGETMEASAREAEXPANSION :
  case SETFFTFLAG :
  case SETCAMERASCALE :
  case GETCAMERASCALE :
  case SETFFTZONE :
  case DOFFTFILTRATION :
  case SETPROFILEROI:
    {
      Answer.m_iId = m_pMyMeasSet->SetROIforProfile( 
        p->m_longIntParams[0], p->m_longIntParams[1],
        p->m_longIntParams[2], p->m_longIntParams[3]) ;
    }
    break;
  case GETPROFILE:
    {
      Answer.m_iId = m_pMyMeasSet->GetProfile( p->m_longIntParams[0]) ;
    }
    break;
  default :
    Answer.m_iId = UNKNOWN_REQUEST ;
    m_pMyMeasSet->m_Status = "Unknown Request" ;
    break ;
  }
  int iLenLimit = sizeof(Answer.m_szStringData)/sizeof(Answer.m_szStringData[0]) ;
  if ( m_pMyMeasSet->m_Status.GetLength() >= iLenLimit )
  {
    ASSERT(0) ;
    m_pMyMeasSet->m_Status = m_pMyMeasSet->m_Status.Left(iLenLimit - 1) ;
  }
  strcpy_s( Answer.m_szStringData , (LPCTSTR)m_pMyMeasSet->m_Status ) ;
  SendAnswer( (BYTE*)&Answer , sizeof(Answer) ) ;
  return 0 ;
}