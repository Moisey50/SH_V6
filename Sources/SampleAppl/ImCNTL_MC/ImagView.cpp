// ImagView.cpp : implementation of the CImageView class
//

#include "stdafx.h"
#include <math.h>
#include "math\intf_sup.h"
#include "ImCNTL.h"
#include "ImagView.h"
#include "helpers\get_time.h"
#include "helpers\Registry.h"
#include "cpp_util\synchronizations.h"
#include <helpers/propertykitEx.h>
#include <gadgets\QuantityFrame.h>
#include "FImParamDialog.h"
#include <gadgets\tvinspect.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int iCaptureGadgetMessage = RegisterWindowMessage("CaptureGadgetMessage") ;
int iImProcResultMessage = RegisterWindowMessage("ImProcResultMessage") ;
int iMsgFromSMThreads = RegisterWindowMessage("SharedMemoryMessage") ;
int iMsgForLog = RegisterWindowMessage("LogMessage") ;

CImageView * thisDlg=NULL;

#define REGISTRY_SANDBOX HKEY_CURRENT_USER

#define BLOCKS_5 5
#define BLOCKS_9_FULL 9
#define BLOCKS_9_HALF 10


LPCTSTR g_wszAppID = _T("FileX.Imaging");
LPCTSTR g_wszProgID = _T("FileX.ImagingID");

typedef struct tagRegEntry
{
  LPCTSTR sKey; 
  LPCTSTR szValue;
  LPCTSTR szData;
}RegEntry;



bool RegisterAsHandler()
{
  //   CString sIconPath, sCommandLine, sProgIDKey;
  //   TCHAR szModulePath[MAX_PATH] = {0};
  //   GetModuleFileName ( NULL, szModulePath, MAX_PATH );
  //   sIconPath.Format ( _T("\"%s\",-%d"), szModulePath, IDR_MAINFRAME);
  //   sCommandLine.Format ( _T("\"%s\" \"%%1\""), szModulePath );
  //   sProgIDKey.Format ( _T("software\\classes\\%s"), g_wszProgID );
  //   CString DefaultIcon=sProgIDKey + _T("\\DefaultIcon");
  //   CString CurVer=sProgIDKey + _T("\\CurVer");
  //   CString SheelCommand=sProgIDKey + _T("\\shell\\open\\command");
  //   RegEntry aEntries[] =
  //   {
  //     { _T("software\\classes\\.flw"), NULL, g_wszProgID },
  //     { _T("software\\classes\\.flw\\OpenWithProgIDs"), g_wszProgID, _T("") },
  //     { sProgIDKey, _T("FriendlyTypeName"), _T("FLW video player") },
  //     { sProgIDKey, _T("AppUserModelID"), g_wszAppID },
  //     { DefaultIcon, NULL, sIconPath },
  //     { CurVer, NULL, g_wszProgID },
  //     { SheelCommand, NULL, sCommandLine }
  //   }; 
  //   for ( int i = 0; i <sizeof(aEntries)/sizeof(RegEntry); i++ )
  //   {
  //     if ( !WriteRegString ( REGISTRY_SANDBOX, aEntries[i].sKey, aEntries[i].szValue, aEntries[i].szData) )
  //       return false;
  //   }
  return true;
}


void __stdcall PrintMsg(int msgLevel, LPCTSTR src, int msgId, LPCTSTR msgText)
{
  TRACE("+++ %d %s %d %s\n",msgLevel, src, msgId, msgText);
}


//#define  TOOLBAR_SIZE     24

//#define  STATUSBAR_SIZE   42
#define  PI 3.14159265359
#define  FACTOR          0.5
#define  Y_SCAN_OFFSET    16

#define GRAB_EVENT              10
#define BAD_PIXELS_FIND_EVENT   11
#define STOP_GRAB_EVENT         12
#define GRAB_EVENT_OUT          13
#define MEASURE_BLOB            14
#define GET_BLOB_PARAM          15
#define VIEW_PARAM_WINDOW       18
#define GRAB_BACK_EVENT         19
#define RBUTTON_EVENT           20
#define SAVE_FILE_LIMIT_EVENT   21

CImageView * pView;
char MyMsg[1000];
/////////////////////////////////////////////////////////////////////////////
// CImageView

IMPLEMENT_DYNCREATE(CImageView, CMainFrame)

  BEGIN_MESSAGE_MAP(CImageView, CMainFrame)
    //{{AFX_MSG_MAP(CImageView)
    ON_WM_TIMER()
    ON_COMMAND(ID_GRAB_BUTTON, OnGrabButton)
    ON_WM_MOUSEMOVE()
    ON_COMMAND(ID_LIVE_VIDEO, OnLiveVideo)
    ON_WM_DESTROY()
    ON_COMMAND(ID_FILE_SAVE, OnFileSave)
    ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
    ON_COMMAND(ID_EDIT_GRABBER, OnEditGrabber)
    ON_COMMAND(IDM_FFT, OnFft)
    ON_COMMAND(IDM_EDIT_FFT_FILTER, OnEditFftFilter)
    ON_COMMAND(IDM_SAVE_BINARY, OnSaveBinary)
    ON_COMMAND(ID_EXPOSURE_MINUS, (AFX_PMSG)OnExposureMinus)
    ON_COMMAND(ID_EXPOSURE_PLUS, (AFX_PMSG)OnExposurePlus)
    ON_COMMAND(ID_GAIN_MINUS, (AFX_PMSG)OnGainMinus)
    ON_COMMAND(ID_GAIN_PLUS, (AFX_PMSG)OnGainPlus)
    ON_COMMAND(ID_MINUS_BACKGROUND, OnMinusBackground)
    ON_COMMAND(ID_GRAY_BINARY_DISP, OnGrayBinaryDisp)
    ON_COMMAND(ID_EDIT_PARAMETERS, OnEditParameters)
    //ON_COMMAND(ID_DRAW_REDLINE,OnDrawredLine)
    ON_WM_LBUTTONDOWN()
    ON_UPDATE_COMMAND_UI(ID_MINUS_BACKGROUND, OnUpdateMinusBackground)
    ON_WM_MOVING()
    ON_WM_PAINT()
    ON_UPDATE_COMMAND_UI(ID_ASYNCH_MODE, OnUpdateAsynchMode)
    ON_WM_RBUTTONDOWN()
    ON_COMMAND(ID_CONTRASTING, OnContrasting)
    ON_WM_CREATE()
    ON_UPDATE_COMMAND_UI(ID_LIVE_VIDEO, OnUpdateLiveVideo)
    ON_UPDATE_COMMAND_UI(ID_GRAB_BUTTON, OnUpdateGrabButton)
    ON_WM_KEYDOWN()
    ON_COMMAND(ID_FILE_LoadCamCfg, OnFILELoadCamCfg)
    ON_COMMAND(ID_BUTTON1024x1024, OnBUTTON1024x1024)
    ON_COMMAND(ID_BUTTON512x512, OnBUTTON512x512)
    ON_COMMAND(IDM_FILTRATION, OnFiltration)
    ON_COMMAND(ID_MEASURE_BLOB, OnMeasureBlob)
    ON_COMMAND(ID_MEASURE_LINE, OnMeasureLine)
    ON_COMMAND(ID_ASYNCHRONE, OnAsynchroneMode )
    ON_COMMAND(ID_MEASURE_POWER, OnMeasurePower)
    ON_COMMAND(ID_MEASURE_Y_DIST, OnMeasureYDist)
    ON_COMMAND(ID_User_Dlg,RButton)
    ON_COMMAND(ID_Draw_Line,IniExpoandGain)
    ON_COMMAND(ID_SHOW_RADIAL_DISTRIBUTION, OnViewRadialDistr)
    ON_COMMAND(IDM_NOT_USING_EXT_IMAGING, &CImageView::OnDontUseExternalImaging)
    ON_COMMAND(IDM_SEND_IMAGES_TO_LAN, &CImageView::OnSendImagesToLAN)
    ON_COMMAND(IDM_TAKE_IMAGES_FROM_LAN, &CImageView::OnTakeImagesFromLAN)
    ON_COMMAND(ID_HELP,OnViewHelp) 
    //   ON_COMMAND(ID_BAD_PIXELS, OnFindBadPixels)
    ON_WM_INITMENU()
    ON_COMMAND(ID_APP_EXIT, OnAppExit)
    ON_REGISTERED_MESSAGE( iCaptureGadgetMessage , OnCaptureGadgetMsg )
    ON_REGISTERED_MESSAGE( iImProcResultMessage , OnImProcResultMsg )
    ON_REGISTERED_MESSAGE( iMsgFromSMThreads , OnSharedMemoryMsg )
    ON_REGISTERED_MESSAGE( iMsgForLog , OnLogMsg )
    //}}AFX_MSG_MAP
    // Standard printing commands
    //ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
    //ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
    //ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
    ON_WM_CLOSE()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_SIZE()
    ON_COMMAND(IDM_GRAPH_SETUP, &CImageView::OnGraphSetup)
    ON_COMMAND(IDM_VIEW_GRAPH, &CImageView::OnViewGraph)
    ON_BN_CLICKED(IDCANCEL, &CImageView::OnCancel)
    ON_WM_PARENTNOTIFY()
  END_MESSAGE_MAP()

  BOOL CALLBACK fnCaptureSettingsCallBack( CDataFrame*& pDataFrame , FXString& PinName , void * lParam )
  {
    CMeasurementSet * pSet = (CMeasurementSet *) lParam ;
    CTextFrame * pText = pDataFrame->GetTextFrame( DEFAULT_LABEL ) ;
    if ( pText && pSet && pSet->m_pView )
    {
      pSet->m_ReceivedFromCap = pText->GetString() ;
      pSet->m_pView->PostMessage( iCaptureGadgetMessage , 0 , (LPARAM) pSet ) ;
    }
    pDataFrame->Release( pDataFrame ) ;
    return TRUE ; // data frame is released
  }

  BOOL CALLBACK fnResultCallBack( CDataFrame*& pDataFrame , FXString& PinName , void * lParam )
  {
    CMeasurementSet * pSet = (CMeasurementSet *) lParam ;
    if ( pSet && pSet->m_pView )
    {
      pSet->m_pView->ProcessReceivedDataFrame( pDataFrame , pSet) ;
      SetEvent (pSet->m_hEVDataImg);
    }
    pDataFrame->Release( pDataFrame ) ;

    double dNow = get_current_time();
    pSet->m_dLastFrameInterval = dNow - pSet->m_dCaptureFinishedAt;
    pSet->m_dCaptureFinishedAt = dNow;
    HWND hWnd = pSet->m_pView->GetSafeHwnd() ;
    if ( hWnd )
    {
      char Msg[30];
      sprintf_s( Msg , "dT=%.3f", pSet->m_dLastFrameInterval);
      char * pMsg = new char[30];
      strcpy_s(pMsg, 30, Msg);
      BOOL bRes = (::PostMessage( hWnd , iMsgForLog ,
        3, (LPARAM)pMsg ) != FALSE) ;
    }
    return TRUE ; // data frame is released
  }


  BOOL CALLBACK fnBMPSaverCallback(CDataFrame*& pDataFrame , FXString& PinName , void * lParam)
  {
    CMeasurementSet * pSet = (CMeasurementSet *) lParam ;
    pSet->m_pView->m_iSavedImg++;
    return TRUE ; // data frame is released  
  } 

  double time_1=0,time_2=0;
  double prevX=0,prevY=0;

  BOOL CALLBACK fnRendererCallBack( CDataFrame*& pDataFrame , FXString& PinName , void * lParam )
  {
    bool bDblClick = FALSE;


    CMeasurementSet * pSet = (CMeasurementSet *) lParam ;
    CTextFrame * pText = pDataFrame->GetTextFrame() ;
    CString sCoor = pText->GetString();
    if (sCoor.MakeUpper().Find("RECT")>-1)
    {
      sCoor.Trim("RECT=");
      int itok = 0;
      CString Xtop,YTop,XBot,YBot;

      CString s1=sCoor.Tokenize(",",itok);
      Xtop = (s1);
      s1=sCoor.Tokenize(",",itok);
      YTop = (s1);
      s1=sCoor.Tokenize(",",itok);
      XBot = (s1);
      s1=sCoor.Tokenize(",",itok);
      YBot = (s1); 

      CString sRect = _T("Rect=")+Xtop+(",")+YTop+(",")+XBot+(",")+YBot;

      CString stemp = PinName;
      if (stemp.Find("Render1")>-1) 
        pSet->m_pView->m_sSaveRectCam1=sRect;
      if (stemp.Find("Render2")>-1)
        pSet->m_pView->m_sSaveRectCam0=sRect; 

      return TRUE ; 
    }

    int sep = 0;

    // if ( sCoor.Find( "selected" ) == -1 )
    // {
    //  return FALSE ;
    // }

    if ( sCoor.Find("LINE") > -1)
    {
      return FALSE ;
    }




    sCoor.Tokenize(";",sep);
    CString sX =  sCoor.Tokenize(";",sep); 
    CString sY = sCoor.Tokenize(";",sep);
    sY.Trim("Y=");
    sX.Trim("X=");

    pSet->m_pView->cSpotX = atof(sX);
    pSet->m_pView->cSpotY = atof(sY);

    pDataFrame->Release( pDataFrame ) ;

    time_2 = time_1;
    time_1 = GetCurrentTime();

    //////////
    //   CString sFileName = "C:\\Clicks.dat";
    //   CString sOut;
    //   FILE * fr = fopen( ( LPCTSTR )sFileName , "a+" ) ;
    //   if (fr)
    //   {
    //     sOut.Format("%11.2f,%11.2f,%6.2f,%6.2f",time_1, time_2,pSet->m_pView->cSpotX, pSet->m_pView->cSpotY);
    //     sOut+=_T("\r\n");
    //     fputs(sOut,fr);
    //     fclose(fr) ;
    //   } 
    //////////

    //   if (fabs(time_2 - time_1)>800 && fabs(time_2 - time_1)<1200/*time_1 - time_2 < 78 && time_1 - time_2 > 60*/)
    //     if (fabs(prevX-pSet->m_pView->cSpotX)<1 && fabs(prevY-pSet->m_pView->cSpotY)<1)
    //          ;//bDblClick = TRUE;

    prevX = pSet->m_pView->cSpotX;
    prevY = pSet->m_pView->cSpotY;

    if (PinName.Find("2")>-1)
      pSet->m_pView->m_iCam = 2;
    else
      pSet->m_pView->m_iCam = 1;

    if(pSet->m_pView->m_WindisAlive == 0 && bDblClick)
    {
      Sleep(100);
      SetTimer(pSet->m_pView->m_hWnd,RBUTTON_EVENT,300,NULL) ;
      pSet->m_pView->m_WindisAlive = 1;
    }

    if(pSet->m_pView->m_WindisAlive == 1 && bDblClick)
    {
      Sleep(100);
      SetTimer(pSet->m_pView->m_hWnd,RBUTTON_EVENT,300,NULL) ;
    }
    return TRUE ; // data frame is released
  }

  bool __stdcall DataCallbackFromExternalImaging(
    CallBackDataA& Data , LPVOID lParam)
  {
    CMeasurementSet * pSet = (CMeasurementSet*)lParam ;

    switch ( Data.m_ResultType )
    {
    case DATA_TEXT :
      {
        //       if ( pSet && pSet->m_pView && (pSet->m_GraphMode == 2) ) // send images and get results
        //       {
        //         CString StringData( Data.m_Par1.pString ) ;
        //         CString Label( Data.m_Label ) ;
        //         pSet->m_pView->ProcessReceivedDataFrame( StringData , Label , pSet) ;
        //         //send text to window
        //         
        //         CString AddPart ;
        // 
        //         double dTime = get_current_time() ;
        //         CString Msg ;
        //         Msg = _T("Data from Ex = ");
        // 
        //         pSet->m_SpotResults.Lock() ;
        //         if ( (pSet->m_iMaxBlobNumber >= 0) )
        //         {
        //           AddPart.Format( ": %d Spots - C=[%6.1f,%6.1f] Sz=[%6.1f,%6.1f]" ,
        //             pSet->m_SpotResults.GetCount() ,
        //             pSet->m_SpotResults[pSet->m_iMaxBlobNumber].m_SimpleCenter.x , 
        //             pSet->m_SpotResults[pSet->m_iMaxBlobNumber].m_SimpleCenter.y ,
        //             pSet->m_SpotResults[pSet->m_iMaxBlobNumber].m_dBlobWidth ,
        //             pSet->m_SpotResults[pSet->m_iMaxBlobNumber].m_dBlobHeigth ) ;
        //           Msg += AddPart ;
        //         }
        //           pSet->m_SpotResults.Unlock() ;
        //           strcpy( pSet->m_pView->m_LogMessageData , (LPCTSTR)Msg ) ;
        //           PostMessage( pSet->m_pView->m_hWnd, iImProcResultMessage , 0 , (LPARAM) pSet->m_pView->m_LogMessageData ) ;
        // 
        // 
        //       }
        break ;
      }
      // the next is commented, because graph does image processing and result sending automatically
    case DATA_IMAGE :
      {
        if ( pSet->m_GraphMode == 1 ) // get images and send results
        { 

           double dNow = get_current_time() ;
           pSet->m_dLastFrameInterval = dNow - pSet->m_dCaptureFinishedAt;
           pSet->m_dCaptureFinishedAt = dNow ;
//            CString Msg;
//            Msg.Format("dT=%.3f", pSet->m_dLastFrameInterval);
//            pSet->m_pView->LogMessage3(Msg);
           //         HWND hWnd = pView->GetSafeHwnd() ;
          //         if ( hWnd )
          //         {
          //           BITMAPINFOHEADER * lpBMIH = new BITMAPINFOHEADER ;
          //           memcpy_s( lpBMIH , sizeof(BITMAPINFOHEADER) , 
          //             Data.m_Par1.BmpInfo , sizeof(BITMAPINFOHEADER) ) ;
          //           DWORD dwSize = lpBMIH->biHeight * lpBMIH->biWidth ;
          //           if ( lpBMIH->biBitCount == 16 )
          //             dwSize *= 2 ;
          //           BYTE * pImage = new BYTE[ dwSize ] ;
          //           memcpy_s( pImage , dwSize , Data.m_Par2 , dwSize ) ;
          //           BOOL bRes = (::PostMessage( hWnd , EVT_IMAGE_CALLBACK , 
          //             (WPARAM)lpBMIH , (LPARAM)pImage ) != FALSE) ;
          //           if ( bRes )
          //             return true ;
          //           delete lpBMIH ;
          //           delete[] pImage ;
          //         }
        }
        break ;
      }
    default : break ;
    }

    return false ;
  };

  bool __stdcall CallbackFromProfileGadget(CallBackDataA& Data , LPVOID lParam)
  { 
    CMeasurementSet * pSet = (CMeasurementSet*)lParam ;
    if(pSet->m_GraphMode  != 0)
    {
      if(Data.m_Par1.pFigure->GetSize() == 1024 || Data.m_Par1.pFigure->GetSize() > 2000)
        return false;
    }
    else
    {
      if(Data.m_Par1.pFigure->GetSize() == 1036 || Data.m_Par1.pFigure->GetSize() > 2000)
        return false;
    }

    pSet->m_pProfilePtArray.Lock();
    pSet->m_pProfilePtArray.RemoveAll();
    pSet->m_pProfilePtArray.Copy(*Data.m_Par1.pFigure);
    pSet->m_pProfilePtArray.Unlock();
    return false;
  };

  /////////////////////////////////////////////////////////////////////////////
  // CImageView construction/destruction

  CImageView::CImageView()
    : m_iLastMissedFrame(0)
    , m_iFirstChangeBuf(0)
    , m_iBackToLiveVideo(0)
    , m_dUpRelation(0)
    , m_dDownRelation(0)
    // , m_hPrThread(NULL)
    , m_GrabberConfigFileName(_T(""))
    , m_dMaxBackExposureRatio(0)
    , m_bAsyncGrab(0)
    , m_pBuilder(NULL)
    , m_dLastDataArrivingTime(0.0)
    , m_dScanTime(0.0)
    , m_dwSHViewMode(0)
  {
    m_pUserBuff = NULL ;
    m_pSaveBuff = NULL ;
    //   m_pDibview = NULL ;

    m_iGrabberType = GRABBER_SOLIOS ;

    m_imode = 1;
    m_dPrevTime = 0;
    m_iNotBlob = 0;
    m_dTimeInterval = 0;
    m_bBigGrab = 0;
    m_iOnBlobProc = 0;
    m_bOnBinaryDisp = FALSE;
    m_bEndLine = FALSE;
    m_iCheckExp = 0;
    m_i8Bit = 0;
    m_iImgMode = 0; // changed on 17.12.08 to open the window with smaller image
    m_iFindBadPix = 0;
    m_NBlobs = 0;
    m_iDraw = 0;
    m_iBackToLiveVideo = 0;
    m_bRButtonDlgModal = FALSE ;
    pView = this;
    m_bShow = FALSE;
    m_bDisp = FALSE;
    m_bOnGrab = FALSE;
    m_pCommon  = NULL;
    m_bContrasting = FALSE;
    m_piBlobIndexArr = NULL;
    m_bDrawLine = FALSE;
    // Allocation and initialization of shared memory
    DWORD dwAreaSize = ((sizeof(PowerImagingExch) / 4096) + 2) * 4096 ;
    m_pSharedMemory = new CSharedMemLib( 256 , 512 ,dwAreaSize,
      "IMAGE_CNTL_AREA" , "IMAGE_CNTL_IN_EVENT", "IMAGE_CNTL_OUT_EVENT" ) ;
    m_pCommon = ( PowerImagingExch*) m_pSharedMemory->GetAreaPtr(1024) ;
    m_pSharedMemory->ResetOutEvent();
    m_pCommon->iCounter = 0;
    m_pCommon->iDoSingleGrab  = 0;
    m_pCommon->iDoContinueGrab= 0 ;
    m_pCommon->iIsGrab  = 0;
    m_pCommon->iDoQuit  = 0;
    m_pCommon->iDoStopGrab = 0;
    m_pCommon->iDoMeasLine = 0;
    m_pCommon->iDoMeasBlob = 0;
    m_pCommon->iDoSubExp = 0;
    m_pCommon->iDoAddExp = 0;
    m_pCommon->iping    = 0;
    m_pCommon->iMeasMode = 0;
    m_pCommon->iBigSpots = 0;
    m_pCommon->dFindSpotTime = 0.;
    m_pCommon->dFrameInterval = 0.;

    m_dNormThreshold = 0;

    m_LiveVideo = 0 ;
    m_SingleGrab = 0;
    m_SingleCont = 0;
    m_dFindSpotETime = 0;
    m_iBigSpots = 0;
    m_dGrabTime = 0;
    m_dResizeTime = 0;
    m_dFindSpotETime = 0;
    m_iBigSpots	= 0;
    m_iLock = 0;
    m_iGrabFromProxy = 0;
    m_iFirstChangeBuf = 0;
    // Meteor Values. Should start with Solios values
    //  m_pCommon->iExpMax = 700;//500000;
    //  m_pCommon->iExpMin = 1;
    m_pCommon->iExpMax = 63000;
    m_pCommon->iExpMin = 50;

    m_LogMessageData[0] = 0 ;
    m_bSubBackground = 0 ;
    m_MarkerPos = CPoint( 20 , 400 ) ;
    m_MarkerType = 1 ;
    m_MarkerColor = 0x000000ff ;

    m_iCam = 2;
    m_WindisAlive = 0;
    cSpotX = -1;
    cSpotX = -1;

    m_bsetAutoSave = 0;
    m_bsetManSave = 0;
    m_bSaveOnePic = 0;
    m_iSavedImg = 0;

    dAngles[0]=dAngles[1]=0;

    m_bVideoSavingEnabled = 0; 
    CRegistry Reg( "File Company\\OpticJig" ) ;

    CRegistry ImcntlReg( "File Company\\ImCNTL" ) ;
    m_RButtonDlg.m_dScaleXCam1 = 
      Reg.GetRegiDouble( "Scales" , "ScaleX_pix_per_mm_Cam1" , -1622. ) ;

    m_RButtonDlg.m_dScaleXCam2 = 
      Reg.GetRegiDouble( "Scales" , "ScaleX_pix_per_mm_Cam0" , -1622. ) ;

    m_RButtonDlg.m_dScaleYCam1 =  
      Reg.GetRegiDouble( "Scales" , "ScaleY_pix_per_mm_Cam1" , 1622. ) ;

    m_RButtonDlg.m_dScaleYCam2 =  
      Reg.GetRegiDouble( "Scales" , "ScaleY_pix_per_mm_Cam0" , 1622. ) ;

    m_RButtonDlg.m_StringScales.Format( "c1 X=%6.1f Y=%6.1f C2 X=%6.1f Y=%6.1f" , 
      m_RButtonDlg.m_dScaleXCam1 , m_RButtonDlg.m_dScaleYCam1,  
      m_RButtonDlg.m_dScaleXCam2 , m_RButtonDlg.m_dScaleYCam2) ;
    m_ImParam.m_iMinAmplitude = 
      Reg.GetRegiInt( "ImageMeasPar" , "Min Amplitude" , 1000 ) ;

    CRegistry RegIm( "File Company\\ImCNTL" ) ;
    m_iGraphMode = RegIm.GetRegiInt( "CameraComm" , "GraphMode" , 1 ) ;

    //m_bFlippedImg = Reg.GetRegiInt( "Positions" , "UnifiedWH" , 0 ) ;

    m_ImParam.m_iDiffractionMeasurementMethod = 
      RegIm.GetRegiInt( "ImProcParam" , "DiffractionMeasurementMethod" , 0 ) ;
    if(m_iGraphMode == 0)
    {
      m_ImParam.m_iDiffractionRadius = 
        RegIm.GetRegiInt( "ImProcParam" , "DiffractionRadius_pix_AVT" , 40 ) ;
      m_ImParam.m_iDiffractionRadius_Y = 
        RegIm.GetRegiInt( "ImProcParam" , "DiffractionRadius_Y_pix_AVT" , 40 ) ;
    }
    else
    {
      m_ImParam.m_iDiffractionRadius = 
        RegIm.GetRegiInt( "ImProcParam" , "DiffractionRadius_pix" , 40 ) ;
      m_ImParam.m_iDiffractionRadius_Y = 
        RegIm.GetRegiInt( "ImProcParam" , "DiffractionRadius_Y_pix" , 40 ) ;
    }


    m_ImParam.m_iMaxSpotDia = 
      RegIm.GetRegiInt( "ImProcParam" , "MaxSpotSizeForDiffraction_pix" , 140 ) ;
    m_ImParam.m_iBackgroundDist = 
      RegIm.GetRegiInt( "ImProcParam" , "BackgroundDist_pix" , 150 ) ;
    m_ImParam.m_ImageSaveDir =
      RegIm.GetRegiString( "ImProcParam" , "ImageSaveDirectory" , "c:\\temp" ) ;
    m_ImParam.m_bSave16Bits = 
      RegIm.GetRegiInt( "ImProcParam" , "ImageSave16Bits" , 0 ) ;
    m_ImParam.m_bImageZip = 
      RegIm. GetRegiInt( "ImProcParam" , "ImageSaveZip" , 0 ) ;
    m_ImParam.m_iDiffrMaxSearchDist =
      RegIm.GetRegiInt( "ImProcParam" , "DiffractionMaxSearchDist" , 150 ) ;
    m_ImParam.m_BinThreshold =
      RegIm.GetRegiDouble( "ImProcParam" , "Binary Threshold" , 0.3 ) ;
    m_ImParam.m_MeasBeamRotBySectors =
      RegIm.GetRegiInt( "ImProcParam" , "BeamRotationBySectors" , 0 ) ;
    m_ImParam.m_bBackSubstract =
      RegIm.GetRegiInt( "ImProcParam" , "Substract Background" , 1 ) ;
    m_ImParam.m_bUseExposureForBackground = ( 0 != 
      RegIm.GetRegiInt( "ImProcParam" , "Use Exposure for Background" , 1 ) )  ;

    m_iMaxExposure_us = 
      RegIm.GetRegiInt( "ExposureParam" , "MaxExposure_us" , 1000000 ) ;
    m_iMinExposure_us = 
      RegIm.GetRegiInt( "ExposureParam" , "MinExposure_us" , 20 ) ;
    m_iMaxGain = 
      RegIm.GetRegiInt( "ExposureParam" , "MaxGain" , 540 ) ;
    m_iMinGain = 
      RegIm.GetRegiInt( "ExposureParam" , "MinGain" , 0 ) ;

    int bHDLA = Reg.GetRegiInt( "HeadControl" , "HDLA" , 0 ) ;
    m_dScanTime  = RegIm.GetRegiDouble(
      "ExposureParam", "Scan Time_usec", 260.75 ) ; // defaulting to Carmel
    if(bHDLA == 1)
      m_dScanTime  = RegIm.GetRegiDouble("ExposureParam", "Scan ScanTimeHDLA_usec", 267. ) ; 

    m_iTriggerSrc  = RegIm.GetRegiInt(
      "ExposureParam", "TriggerSource", 1 ) ; // channel 2 by default
    m_iTriggerSrc  = RegIm.GetRegiInt(
      "ExposureParam", "TriggerPolarity", 0 ) ; // no inverse by default
    m_TriggerModePropertyName = Reg.GetRegiString( "CameraComm" , "TriggerModePropertyName" , "TriggerMode" ) ;

    m_iSearchArea = RegIm.GetRegiInt( "ImProcParam" , 
      "Diffraction area(5-5cubes,9-9cubes,10-9cubes(diagonal with factor 0.5))" , 5 );

    LPCTSTR pClass = AfxRegisterWndClass( NULL ) ;
    CRect rPos( 520 , 50 , 1280 , 380 ) ;

    if ( !m_BigViewWnd.CreateEx( WS_EX_TOPMOST , pClass , "Result View Window" , 
      WS_MINIMIZEBOX | WS_OVERLAPPED | WS_CAPTION | WS_VISIBLE ,
      rPos , this , 0 ) )
    {
      DWORD ErrCode = GetLastError() ;
      ASSERT(0) ;
    };

    m_BigViewWnd.ShowWindow( SW_HIDE ) ;
    m_NProfilePoints = 0 ;
    m_NVProfilePoints = 0 ;
    m_iProxyGrab = 0;
    m_iExist = 1;

    FirstFileReady = false;
    SecondfileReady = false;


    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      m_MeasSets[i].m_dLastBinThreshold = -1;
      m_MeasSets[i].m_dLastRotThreshold = -1;
    }


    //   CString  FileName = RegIm.GetRegiString("ImProcParam","BadPixelsDirectory",
    //     "S:\\DotNetOpticJigDev\\BadPixels.dat");
    //   // FileName = "S:\\DotNetOpticJigDev\\BadPixels3.dat";
    //   int iX,iY;
    //   FILE * fr = fopen( ( LPCTSTR )FileName , "r" ) ;
    //   if (fr)
    //   {
    //     char cBadPix[200] ;
    //     while(fgets(cBadPix,100,fr) && !strstr(cBadPix, "eof"))
    //     {
    //       sscanf(cBadPix,"%d",&iX);
    //       char *cY = strstr(cBadPix,",") + 1;
    //       sscanf(cY,"%d",&iY);
    //       CPoint NewPoint(iX,iY) ;
    //       m_BadPoints.Add(NewPoint) ;
    //     }
    //     fclose(fr) ;
    //   } 
    InitializeCriticalSection( &m_BackProtect ) ;
  }

  CImageView::~CImageView()
  {
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      m_pBuilder->SetOutputCallback( m_MeasSets[i].m_CaptureControlName , NULL , this ) ;
      m_pBuilder->SetOutputCallback( m_MeasSets[i].m_DataOutName , NULL , this ) ;
      Sleep(100);
      m_MeasSets[i].ReleaseShMem() ;
      Sleep(100);
    }
    if (m_iGraphMode > 0 )
    {  
      HWND hExistingAppWindow = ::FindWindow( NULL , "Image Control Server Solios" ) ;
      if ( hExistingAppWindow )
      {
        ::SendMessage(hExistingAppWindow,WM_CLOSE,0,0);
        Sleep(500);
      }
    }

    DeleteCriticalSection( &m_BackProtect ) ;
  }

  BOOL CImageView::PreCreateWindow(CREATESTRUCT& cs)
  {
    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.lpszClass = AfxRegisterWndClass(0);
    //  cs.cy += 40 ;
    // next two lines edited on 17.12.08 by Alex Bernstein
    // To fit the size of the window to the smaller 512*512 size
    cs.cx = 550;
    cs.cy = 620;

    return CMainFrame::PreCreateWindow(cs);
  }

  void CImageView::OnDraw(CDC* pDC)
  {
    // TODO: add draw code for native data here
  }

  /////////////////////////////////////////////////////////////////////////////
  // CImageView printing
  // BOOL CImageView::OnPreparePrinting(CPrintInfo* pInfo)
  // {
  //   // default preparation
  //   //return DoPreparePrinting(pInfo);
  // 
  //   return 0 ;
  // }
  // 
  // void CImageView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
  // {
  //   // TODO: add extra initialization before printing
  // }
  // 
  // void CImageView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
  // {
  //   // TODO: add cleanup after printing
  // }

  /////////////////////////////////////////////////////////////////////////////
  // CImageView diagnostics

#ifdef _DEBUG
  void CImageView::AssertValid() const
  {
    CMainFrame::AssertValid();
  }

  void CImageView::Dump(CDumpContext& dc) const
  {
    CMainFrame::Dump(dc);
  }

#endif //_DEBUG

  /////////////////////////////////////////////////////////////////////////////
  // CImageView message handlers
  void 
    CImageView::Clean()
  {
    if ( m_LiveVideo )
      OnLiveVideo() ;
    Sleep( 1000 ) ;

    if(!m_bAsyncGrab) //if sync -> disable sync  : crash problem
    {
      for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
        SetGrabTrigger( 0 , i ) ;
    }
    Sleep(800);

    if (m_pBuilder)
    {
      m_Graph.Disconnect() ;
      m_pBuilder->Stop();
      Sleep( 600 ) ;
      m_pBuilder->Release();
      m_pBuilder=NULL;
      FxExitMsgQueues();
    }
    //   if (m_hPrThread)
    //   {
    //     m_hPrThread->Close();
    //     delete m_hPrThread;
    //     m_hPrThread = NULL ;
    //   }
    Sleep(2000);
    if ( m_pSharedMemory )
    {
      m_pSharedMemory->SetInEvent();
      m_pCommon->iIsGrab = 0;
    }
    Sleep(500);
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      m_MeasSets[i].m_pChildWindow->CloseWindow() ;
      Sleep(200);
      m_MeasSets[i].m_pChildWindow->DestroyWindow() ;
      Sleep(200);
      delete m_MeasSets[i].m_pChildWindow ;
      if ( m_MeasSets[i].m_pShMemControl )
      {
//         m_MeasSets[i].m_pShMemControl->m_bExit = true ;
//         m_MeasSets[i].m_pShMemControl->SetInEvent() ;
        delete m_MeasSets[i].m_pShMemControl ;
        m_MeasSets[i].m_pShMemControl = NULL ;
      }
    }
    Sleep(200);
    m_MeasSets.RemoveAll() ;
    if ( m_pSharedMemory )
    {

      m_pCommon = NULL ;
      delete m_pSharedMemory;
      m_pSharedMemory = NULL ;
    }

    if (m_piBlobIndexArr)
    {
      delete [] m_piBlobIndexArr;
      m_piBlobIndexArr = NULL ;
    }
    if ( m_pSaveBuff )
    {
      delete [] m_pSaveBuff  ;
      m_pSaveBuff = NULL ;
    }

    m_BigViewWnd.DestroyWindow() ;
    m_iExist = 0;
  }

  BOOL 
    CImageView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, 
    DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, 
    UINT nID, CCreateContext* pContext) 
  {
    int Res = CWnd::Create(lpszClassName, lpszWindowName, 
      dwStyle, rect, pParentWnd, nID, pContext);

    return Res ;
  }

  int 
    CImageView::SingleGrab( bool bSync )
  {
    if ( m_MeasSets.GetCount() > 1 )
    {
      CMeasurementSet * pSet = &m_MeasSets[1] ;
      if (pSet->m_GraphMode==1)
      {
        pSet->SetTextThroughLAN("GRAB");
        return 1;
      }
      if (pSet->m_GraphMode==2)
      {
        if( !m_LiveVideo )
          m_MeasSets[1].Grab() ;
        else
          OnLiveVideo();

        return 1;
      }
    }
    //   if( !m_LiveVideo )
    //   {
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
      m_MeasSets[i].Grab() ;
    //SwitchCamera(1) ;
    m_bOnGrab = true ;

    if ( bSync )
    {
      double dBegin = get_current_time() ;
      while ( m_bOnGrab
        && (get_current_time() - dBegin < 600 ) )
      {
        Sleep(5) ;
      }
      return (m_bOnGrab == false) ;
    }
    //   }
    //   else
    //     OnLiveVideo() ;

    return 1 ;
  }
  int 
    CImageView::SwitchCamera( int iNFrames , int iCamNumber )
  {
    if ( m_MeasSets.GetCount() < iCamNumber || iCamNumber < -1 )
    {
      CString Msg ;
      Msg.Format( "There is no camera #%d" , iCamNumber ) ;
      LogMessage( Msg) ;
      return 0 ;
    }
    int iFirst = ( iCamNumber == -1 ) ? 0 : iCamNumber  ;
    int iLast = ( iCamNumber == -1 ) ? m_MeasSets.GetCount() - 1 : iCamNumber ;
    for (int iCam = iFirst ; iCam <= iLast ; iCam++)
    {
      CTextFrame * pCommand = CTextFrame::Create() ;
      pCommand->GetString().Format( "set grab(%d)" , iNFrames );
      if ( !m_pBuilder->SendDataFrame( pCommand , m_MeasSets[iCam].m_CaptureControlName ) )
      {
        CString Msg ;
        Msg.Format( "Can't switch camera #%d" , iCam ) ;
        LogMessage( Msg) ;
        pCommand->Release( pCommand ) ;        
        return 0;
      }
    }
    return 1 ;
  }

  void 
    CImageView::OnGrabButton() 
  {
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      if ( m_MeasSets[i].m_iActiveInput != 0 )
      {
        CQuantityFrame * pQuan = CQuantityFrame::Create( 0. ) ;
        pQuan->ChangeId( 0 ) ;
        if ( !m_pBuilder->SendDataFrame( pQuan , m_MeasSets[i].m_ImageOrBackName ) )
        {
          CString Msg ;
          Msg.Format( "Can't switch image path #%d" , i ) ;
          LogMessage( Msg) ;
          pQuan->Release( pQuan ) ;
        }
        m_MeasSets[i].m_iActiveInput = 0 ;
      }
    }
    SingleGrab() ;
  }


  void CImageView::OnLiveVideo() 
  {
    m_dStartGrabTime = get_current_time();
    m_NProfilePoints = 0;
    m_NVProfilePoints = 0;

    if(!m_LiveVideo)
    {
      bool bAreActive = false ;
      for ( int iCam = 0 ; iCam < m_MeasSets.GetCount() ; iCam++ )
      {
        SwitchCamera( -1 , iCam ) ; // infinite grab
        bAreActive = true ;
      }
      if ( bAreActive )
      {
        m_wndToolBar.SetButtonInfo(4, ID_LIVE_VIDEO, TBBS_BUTTON, 23/*21*/);//to on
        m_LiveVideo = 1;
        m_bOnGrab = TRUE;
        m_pCommon->iIsGrab = 1;
      }
    }
    else
    {	
      for ( int iCam = 0 ; iCam < m_MeasSets.GetCount() ; iCam++ )
      {
        m_MeasSets[iCam].Grab() ; // do one single grab, which will stop continuous grabbing
      }
      m_wndToolBar.SetButtonInfo(4, ID_LIVE_VIDEO, TBBS_BUTTON, 3); //to off     
      m_LiveVideo = 0;
    }
  }

  void 
    CImageView::OnTimer(UINT nIDEvent) 
  {
    switch( nIDEvent )
    {
    case GRAB_BACK_EVENT:
      GrabBack();
      KillTimer(GRAB_BACK_EVENT);
      break;
    case RBUTTON_EVENT:
      RButton();
      KillTimer(RBUTTON_EVENT);
      break;
    case GRAB_EVENT : 
      KillTimer( nIDEvent ) ;

      break ;
    case BAD_PIXELS_FIND_EVENT:
      KillTimer( nIDEvent ) ;
      FindBadPixels() ;
      break ;
    case SAVE_FILE_LIMIT_EVENT:
      if (CheckIfStopAutoSaving() == TRUE)
      { 
        m_iSavedImg = 0;
        m_bSaveOnePic = 0;
        KillTimer( nIDEvent ) ;
        m_ImParam.m_bSaveImage = FALSE;
        SavePictures(FALSE);
      }
      break;
    case STOP_GRAB_EVENT:
      if(!m_iProxyGrab && m_LiveVideo && m_iGrabFromProxy)
      {
        OnLiveVideo();
        m_iGrabFromProxy = 0;
      }
      else
        m_iProxyGrab = 0;
      break;
    case GRAB_EVENT_OUT:
      {
        m_pCommon->iMeasMode = 3;
        for(int i = 0; i < 7; i++)
        {
          if(!m_NBlobs)
          {
            m_pSharedMemory->ResetOutEvent();
            if(WaitForSingleObject(m_pSharedMemory->GetOutEventHandle(),500) == WAIT_OBJECT_0 )
              continue;
          }
          else
            break;
        }
        m_pCommon->iMeasMode = 0;

        if(m_NBlobs && m_LiveVideo)
        {

          CColorSpot mySpot= m_ColSpots.GetAt( m_iMaxBlobNumber);

          CString strResult;

          if ( mySpot.m_Area > 100. )
          {
            if ( m_ImParam.m_iDiffractionMeasurementMethod == 0 )
            {
              strResult.Format( "%d %d %6.2f %6.2f  %d %d %6.2f %6.2f %6.2f %6.2f 100 %6.2f" ,
                mySpot.m_SimpleCenter.x , mySpot.m_SimpleCenter.y,
                mySpot.m_dAngle  ,
                mySpot.m_dBlobWidth , mySpot.m_dBlobHeigth ,
                ( int )mySpot.m_iMaxPixel,
                (( int )mySpot.m_iMaxPixel > 0 ) ? 
                ( 100. * mySpot.m_dRDiffraction / mySpot.m_iMaxPixel) 
                :
              0.,
                (( int )mySpot.m_iMaxPixel > 0 ) ? 
                ( 100. * mySpot.m_dLDiffraction / mySpot.m_iMaxPixel) 
                :
              0.,
                (( int )mySpot.m_iMaxPixel > 0 ) ? 
                ( 100. * mySpot.m_dDDiffraction / mySpot.m_iMaxPixel) 
                :
              0.,
                (( int )mySpot.m_iMaxPixel > 0 ) ? 
                ( 100. * mySpot.m_dUDiffraction / mySpot.m_iMaxPixel) 
                :
              0.,
                mySpot.m_dAngle      //for two angle measure methods results presentation 

                ) ;
              if ( m_ImParam.m_UseFFTFilter  
                && (  abs( mySpot.m_SimpleCenter.x - m_ImParam.m_iFFTXc ) > 220
                || abs( mySpot.m_SimpleCenter.y - m_ImParam.m_iFFTYc ) > 220 ) )
              {
                strResult += "ERROR: Out of FFT area" ;
              }
            }
            else if ( m_ImParam.m_iDiffractionMeasurementMethod == 1 )
            {
              strResult.Format( "%d %d %6.2f %6.2f %6.2f %d  %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f" ,
                mySpot.m_SimpleCenter.x ,
                mySpot.m_SimpleCenter.y ,
                mySpot.m_dAngle   ,
                mySpot.m_dBlobWidth ,
                mySpot.m_dBlobHeigth ,
                mySpot.m_iMaxPixel,
                100. * mySpot.m_dRDiffraction ,
                100. * mySpot.m_dLDiffraction ,
                100. * mySpot.m_dDDiffraction ,
                100. * mySpot.m_dUDiffraction ,
                100. * mySpot.m_dCentralIntegral ,
                mySpot.m_dAngle
                ) ;
            }
          }
          strcpy(MyMsg, (LPCSTR)strResult) ;
          IApp()->LogMessageGuest( MyMsg,1 ) ;

        }
      }
      SetTimer(GRAB_EVENT_OUT,200,NULL) ;
      break;
    case VIEW_PARAM_WINDOW:
      {
        RButDlgReaction();
        if ( m_RButtonDlg.m_iViewMode==0)
          KillTimer(VIEW_PARAM_WINDOW);
        else
          SetTimer(VIEW_PARAM_WINDOW,1000,NULL) ;
      }
      break;
    default:
      break ;
    }	  

    CMainFrame::OnTimer(nIDEvent);
  }


  void CImageView::OnDestroy() 
  {
    m_MeasSets.RemoveAll() ;
    PostQuitMessage( 0 ) ;
    CMainFrame::OnDestroy();
  }

  void CImageView::OnFileSave() 
  {

    if ( m_bSaveOnePic == 1)
      m_bSaveOnePic = 0;
    else if ( m_bSaveOnePic == 0)
      m_bSaveOnePic = 1;


    // commented - using SOLIOS code instead
    // 29.10.09 Alex
    //   CFileDialog fd( FALSE , ".tif" , NULL ,
    //     OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"Tif Files (*.tif)|*.tif" ) ;
    // 
    //   BYTE * pShowBuff = (BYTE *)m_pDibview->GetDIBdataBufferPtr();
    //   int iHeight = m_pDibview->GetDIBdataHeight();
    //   int iWidth = m_pDibview->GetDIBdataWidth();	
    //   int iSize = m_pDibview->GetDIBdataBufferSize();
    //   BYTE * pTemp = new BYTE[iSize] ;
    //   BYTE * pDst = pTemp ;	   
    //   for(int iY = 0;iY < iHeight; iY ++)
    //   {
    //     BYTE * pSrc;
    //     pSrc = &pShowBuff[ (iHeight - iY - 1) * iWidth ] ; 
    //     BYTE * pStop = pSrc + iWidth ;
    //     for( ; pSrc < pStop ; pSrc ++ , pDst++ )
    //       *pDst = *pSrc ;
    //   }
    //   delete[] pTemp;

    // SOlios code:


    //   CString FileName ;
    //   CTime t = CTime::GetCurrentTime() ;
    // 
    //   FileName.Format( "%02d%02d%02d-%02d%02d%02d_%s.tif" ,
    //     t.GetDay() , t.GetMonth() , t.GetYear() % 100 ,
    //     t.GetHour() , t.GetMinute() , t.GetSecond() ,
    //     m_ImParam.m_bSave16Bits ? "16" : "8" ) ;
    // 
    //   CString Temp = m_ImParam.m_ImageSaveDir ;
    //   Temp.TrimRight("\\") ;
    //   Temp += "\\" ;
    //   FileName = Temp + FileName ;

    // Commented Out for Eilat ATJ
    //MbufExport( (MIL_TEXT_PTR)((LPCTSTR)FileName) , M_TIFF , m_MGrabBuf ) ;
  }

  void 
    CImageView::OnFileOpen() 
  {
    //21/2/2010 edited for Eilat ATJ
    //MIL functionality disabled to enable use of new graphs

    CFileDialog fd( TRUE , ".tif" , NULL , NULL ,
      "TIF Files (*.tif)|*.tif|Zip Files (*.zip)|*zip||" ) ;
    if ( fd.DoModal() == IDOK )
    {
      CString Ext = fd.GetFileExt() ;
      Ext.MakeUpper() ;
      if ( Ext == "TIF" )
      {
        if ( m_LiveVideo ) // turn off live video to show selected image.
        {
          OnLiveVideo() ;
          m_LiveVideo = FALSE ;
        }
        CString sFileName = fd.GetPathName() ;

        // Import data to MIL
        //       MbufImport( (MIL_TEXT_PTR)(LPCTSTR)sFileName, M_TIFF, M_LOAD, M_NULL,  &m_MGrabBuf ) ;
        Invalidate() ;
      }
    }
  }

  void 
    CImageView::OnEditGrabber() 
  {
    if ( GetAsyncKeyState( VK_CONTROL ) )
    {
      if ((m_pBuilder) && (m_pDebugWnd==NULL))
      {
        m_pDebugWnd = new CDebugWnd(m_pBuilder);
        m_pDebugWnd->Create(this);
        m_pDebugWnd->ShowWindow(SW_SHOW);
      }
    }
    else
    {
      if ( !m_MeasSets[0].m_pCaptureGadget->IsSetupOn() )
      {
        CSetupObject * SObj = m_MeasSets[0].m_pCaptureGadget->GetSetupObject() ;
        if ( SObj )
          SObj->Show( CPoint(100,100) , m_MeasSets[0].m_CaptureGadgetName ) ;
        else if ( m_MeasSets.GetCount() > 1 )
        {
          if ( !m_MeasSets[1].m_pCaptureGadget->IsSetupOn() )
          {
            SObj = m_MeasSets[0].m_pCaptureGadget->GetSetupObject() ;
            if ( SObj )
              SObj->Show( CPoint(100,100) , m_MeasSets[1].m_CaptureGadgetName ) ;
          }
        }
      }
    }
  }

  int 
    CImageView::SetGrabTrigger(int Setting , int iCamNum )
  {
    if ( m_MeasSets.GetCount() <= iCamNum || iCamNum < -1 )
    {
      CString Msg ;
      Msg.Format( "There is no camera #%d" , iCamNum ) ;
      LogMessage( Msg) ;
      return 0 ;
    }

    int iFirst = ( iCamNum < 0 ) ? 0 : iCamNum ;
    int iLast = ( iCamNum < 0 ) ? m_MeasSets.GetUpperBound() : iCamNum ;
    int iTriggerValue = 0 ;
    if ( Setting != 0 )
    {
      if ( m_iGraphMode <= 1 )
      {
        iTriggerValue |= (m_iTriggerSrc==0)? 1 : 3 ;
        iTriggerValue += (m_iTriggerPolarity != 0) ;
      }
      else  // Dalsa camera
      {
        iTriggerValue = ( Setting != 0 ) ;
      }
    }
    for ( int i = iFirst ; i <= iLast ; i++)
    {
      CTextFrame * pCommand = CTextFrame::Create() ;
      pCommand->GetString().Format( "set %s(%d)" , (LPCTSTR)m_TriggerModePropertyName , iTriggerValue );  
      if ( !m_pBuilder->SendDataFrame( pCommand , m_MeasSets[iCamNum].m_CaptureControlName ) )
      {
        CString Msg ;
        Msg.Format( "Can't set trigger #%d Val=%d" , iCamNum , iTriggerValue ) ;
        LogMessage( Msg) ;
        pCommand->Release( pCommand ) ;
        return 0 ;
      }
      if ( Setting != 0 )
        m_MeasSets[iCamNum].SetTriggerDelay( m_MeasSets[iCamNum].m_iTrigDelay_uS ) ;

    }

    return 1 ;
  }


  int CImageView::SetGammaCorrection(int iValue, int iCamNum)
  {
    if ( m_MeasSets.GetCount() <= iCamNum || iCamNum < -1 )
    {
      CString Msg ;
      Msg.Format( "There is no camera #%d" , iCamNum ) ;
      LogMessage( Msg) ;
      return 0 ;
    }
    if ( m_iGraphMode > 1 )  // Dalsa camera
      return 1 ;
    int iFirst = ( iCamNum < 0 ) ? 0 : iCamNum ;
    int iLast = ( iCamNum < 0 ) ? m_MeasSets.GetUpperBound() : iCamNum ;
    for ( int i = iFirst ; i <= iLast ; i++)
    {
      CTextFrame * pCommand = CTextFrame::Create() ;
      pCommand->GetString().Format( "set Gamma(%d)" , iValue);
      if ( !m_pBuilder->SendDataFrame( pCommand , m_MeasSets[iCamNum].m_CaptureControlName ) )
      {
        CString Msg ;
        Msg.Format( "Can't set Gamma Correction #%d Val=%d" , iCamNum , iValue ) ;
        LogMessage( Msg) ;
        pCommand->Release( pCommand ) ;
        return 0 ;
      }
    }
    return 1 ;
  }

  int 
    CImageView::SetGrabTriggerDelay(int Delay_us , int iCamNum )
  {
    if ( m_MeasSets.GetCount() <= iCamNum || iCamNum < -1 )
    {
      CString Msg ;
      Msg.Format( "There is no camera #%d" , iCamNum ) ;
      LogMessage( Msg) ;
      return 0 ;
    }
    if ( m_iGraphMode > 1 )  // Dalsa camera
      return 1 ;
    int iFirst = ( iCamNum < 0 ) ? 0 : iCamNum ;
    int iLast = ( iCamNum < 0 ) ? m_MeasSets.GetUpperBound() : iCamNum ;
    for ( int i = iFirst ; i <= iLast ; i++)
    {
      CTextFrame * pCommand = CTextFrame::Create() ;
      pCommand->GetString().Format( "set TriggerDelay(%d)" , Delay_us);
      if ( !m_pBuilder->SendDataFrame( pCommand , m_MeasSets[iCamNum].m_CaptureControlName ) )
      {
        CString Msg ;
        Msg.Format( "Can't set trigger delay #%d Val=%d" , 
          iCamNum , Delay_us ) ;
        LogMessage( Msg) ;
        pCommand->Release( pCommand ) ;
        return 0 ;
      }
    }
    return 1 ;
  }

  int 
    CImageView::SetTriggerSource(int Setting , int iCamNum)
  {
    if ( m_MeasSets.GetCount() <= iCamNum || iCamNum < -1 )
    {
      CString Msg ;
      Msg.Format( "There is no camera #%d" , iCamNum ) ;
      LogMessage( Msg) ;
      return 0 ;
    }
    if ( m_iGraphMode > 1 )  // Dalsa camera
      return 1 ;
    m_iTriggerSrc = Setting ; // 0 - channel 1, 1 -  channel 2
    if ( m_bAsyncGrab == 0 )
    {
      int iFirst = ( iCamNum < 0 ) ? 0 : iCamNum ;
      int iLast = ( iCamNum < 0 ) ? m_MeasSets.GetUpperBound() : iCamNum ;
      int iTriggerValue = (m_iTriggerSrc)? 1 : 3 ;
      iTriggerValue += (m_iTriggerPolarity != 0) ;
      for ( int i = iFirst ; i <= iLast ; i++)
      {
        CTextFrame * pCommand = CTextFrame::Create() ;
        pCommand->GetString().Format( "set Trigger(%d)" , iTriggerValue);
        if ( !m_pBuilder->SendDataFrame( pCommand , m_MeasSets[iCamNum].m_CaptureControlName ) )
        {
          CString Msg ;
          Msg.Format( "Can't set source #%d" , iCamNum , Setting ) ;
          LogMessage( Msg) ;
          pCommand->Release( pCommand ) ;
          return 0 ;
        }
      }
    }
    return 1 ;
  }


  int 
    CImageView::SetTriggerPolarity(int Setting , int iCamNum )
  {
    if ( m_MeasSets.GetCount() <= iCamNum || iCamNum < -1 )
    {
      CString Msg ;
      Msg.Format( "There is no camera #%d" , iCamNum ) ;
      LogMessage( Msg) ;
      return 0 ;
    }
    if ( m_iGraphMode > 1 )  // Dalsa camera
      return 1 ;

    m_iTriggerPolarity = Setting ; // 0 - positive, 1 - negative

    if ( m_bAsyncGrab == 0 )
    {
      int iFirst = ( iCamNum < 0 ) ? 0 : iCamNum ;
      int iLast = ( iCamNum < 0 ) ? m_MeasSets.GetUpperBound() : iCamNum ;
      int iTriggerValue = (m_iTriggerSrc)? 1 : 3 ;
      iTriggerValue += (m_iTriggerPolarity != 0) ;
      for ( int i = iFirst ; i <= iLast ; i++)
      {
        CTextFrame * pCommand = CTextFrame::Create() ;
        pCommand->GetString().Format( "set Trigger(%d)" , iTriggerValue);
        if ( !m_pBuilder->SendDataFrame( pCommand , m_MeasSets[iCamNum].m_CaptureControlName ) )
        {
          CString Msg ;
          Msg.Format( "Can't set source #%d" , iCamNum , Setting ) ;
          LogMessage( Msg) ;
          pCommand->Release( pCommand ) ;
          return 0 ;
        }
      }
    }
    return 1 ;
  }
  void CImageView::OnFft() 
  {

  }

  void CImageView::OnEditFftFilter() 
  {
    m_FilterDial.DoModal() ;
  }

  void CImageView::OnFiltration() 
  {
  }

  void CImageView::OnViewRadialDistr() 
  {
    if ( m_MeasSets.GetCount() )
    {
      bool bEnabled ;
      if ( m_MeasSets[0].GetRadialDistributionShow( bEnabled) )
      {
        for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
          m_MeasSets[i].EnableRadialDistributionShow( !bEnabled ) ;
      }
    }
  }

  void CImageView::OnSaveBinary() 
  {

  }

  int 
    CImageView::GrabBack()
  {
    bool bClearBack = (GetAsyncKeyState( VK_CONTROL ) & 0x8000) != 0 ;

    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      if ( bClearBack )
        m_MeasSets[i].ClearBack() ;
      else if ( !m_bPlayBackMode )
        m_MeasSets[i].GrabBack() ;
    }
    //   SingleGrab() ;
    return 1 ;
  }

  void CImageView::SavePictures(BOOL bSave)
  { 
    SetBMPPath("");
    if (m_ImParam.m_iImageSaveMode==0) //Crop
    {
      for ( int iSetCnt = 0 ; iSetCnt < m_MeasSets.GetCount(); iSetCnt++)
      {
        CString sCrect;
        CMeasurementSet * pSet = &m_MeasSets[iSetCnt] ;
        if (pSet->m_RenderGadgetName.Find("Render1")>-1) 
          sCrect = m_sSaveRectCam1;

        if (pSet->m_RenderGadgetName.Find("Render2")>-1)
          sCrect = m_sSaveRectCam0;

        CTextFrame *pText = CTextFrame::Create(sCrect);
        if (!m_pBuilder->SendDataFrame(pText,pSet->m_BMPCutRectPinName))
          pText->Release( pText );
      }

    }
    else  if (m_ImParam.m_iImageSaveMode==1)  //full Image
    {
      for ( int iSetCnt = 0 ; iSetCnt < m_MeasSets.GetCount(); iSetCnt++)
      {
        CMeasurementSet * pSet = &m_MeasSets[iSetCnt] ;
        CString sCrect = _T("Rect=8,8,1200,1008;");
        if (pSet->m_GraphMode>0)
          sCrect = _T("Rect=4,4,984,984;");

        CTextFrame *pText = CTextFrame::Create(sCrect);
        if (!m_pBuilder->SendDataFrame(pText,pSet->m_BMPCutRectPinName))
          pText->Release( pText );
      }
    }

    m_iSavedImg = 0;
    SetTimer(SAVE_FILE_LIMIT_EVENT,1000,NULL) ;
    bool bsavePic = (bSave == TRUE)   ? true : false;

    for ( int iSetCnt = 0 ; iSetCnt < m_MeasSets.GetCount(); iSetCnt++)
    {
      CMeasurementSet * pSet = &m_MeasSets[iSetCnt] ;
      CBooleanFrame * pBul = CBooleanFrame::Create( bsavePic );
      pBul->ChangeId( 0 ) ;
      if ( !m_pBuilder->SendDataFrame( pBul , pSet->m_BMPSwitchName  ) ) // send images to image input
        pBul->Release( pBul ) ;                                          // of anchor, not to background
      pSet->m_iActiveInput = 0 ;
    }
  }

  void CImageView::EnableLPFilter(bool bEnable)
  {
    for ( int iSetCnt = 0 ; iSetCnt < m_MeasSets.GetCount(); iSetCnt++)
    {
      CMeasurementSet * pSet = &m_MeasSets[iSetCnt] ;
      CQuantityFrame * pQuan = CQuantityFrame::Create( 0. ) ;
      pQuan->ChangeId( 0 ) ;
      if ( !m_pBuilder->SendDataFrame( pQuan , pSet->m_ImageOrBackName ) ) // send images to image input
        pQuan->Release( pQuan ) ;                                          // of anchor, not to background
      pQuan = CQuantityFrame::Create( bEnable ? 0 : 1 ) ; 
      if ( !m_pBuilder->SendDataFrame( pQuan , pSet->m_LowPassOnName ) )   // Send images directly, without LPF
        pQuan->Release( pQuan ) ;  
    }
  }
  void 
    CImageView::OnMinusBackground() 
  {
    if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
      GrabBack() ;

    m_bSubBackground ^= 1 ;
  }


  void CImageView::OnMeasureBlob() 
  {
    SHORT shState = GetAsyncKeyState(VK_SHIFT) ;
    CString Control = ( shState & 0x8000 ) ? "Task(6)" : "Task(0)" ;

    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      CTextFrame * p = CTextFrame::Create( Control ) ;
      if ( !m_pBuilder->SendDataFrame( p , m_MeasSets[i].m_ProcessControlName ) )
        p->Release( p ) ;
    }
  }


  void 
    CImageView::OnGrayBinaryDisp() 
  {
    /********************************************************************
    created:	2005/11/09
    class: CImageView
    author:	Michael Son
    purpose: Converting of image to image with 2 levels of intensity
    *********************************************************************/

    // Moisey 2023.05.15


    CQuantityFrame * pQuan = CQuantityFrame::Create(1);
    pQuan->ChangeId(0);
    if (!m_pBuilder->SendDataFrame(pQuan, m_MeasSets[0].m_BMPBufferControlPinName))
      pQuan->Release(pQuan);


    if(!m_bOnBinaryDisp)
    {
//       m_bOnBinaryDisp = TRUE;
//       m_wndToolBar.SetButtonInfo(6, ID_GRAY_BINARY_DISP, TBBS_BUTTON, 25/*23*/);
      // 		if(!m_LiveVideo)
      // 		{
      // 		  MakeBinary();
      //       memcpy(m_pSaveBuff,m_pUserBuff,m_BuffSize);
      //       Invalidate();
      // 		}
      //     else
      //     {
      //       m_bOnGrab = TRUE;
      //       StopGrab();
      //       m_bOnGrab = TRUE;
      //       Sleep(300);
      //       m_bOnBinaryDisp = TRUE;
      //       m_iBackToLiveVideo = 0;
      //       m_iFirstChangeBuf = 0;
      //       SingleGrab();
      //     }
    }
    else 
    {
//       m_bOnBinaryDisp = FALSE;
//       m_wndToolBar.SetButtonInfo(6, ID_GRAY_BINARY_DISP, TBBS_BUTTON, 4);
      //     if(m_LiveVideo)
      //     {
      //       Sleep(600);
      //       if(WaitForSingleObject(m_pSharedMemory->GetOutEventHandle(),500) == WAIT_OBJECT_0 )
      //       {
      //         m_bOnGrab = TRUE;
      //         MdigGrabContinuous( m_MDig , m_MGrabBufOnGrabber ) ;
      //       }
      //     }
    }
  }

  int 
    CImageView::SetThreshold( double dNormThreshold ) 
  {
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      m_MeasSets[i].SetThresholdForBinarization( dNormThreshold ) ;
    }
    m_ImParam.m_BinThreshold = dNormThreshold ;

    return 1 ;
  }

  void 
    CImageView::OnEditParameters() 
  {
    m_ImParam.DoModal() ;
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      m_MeasSets[i].SetThresholdForBinarization( m_ImParam.m_BinThreshold ) ;
      m_MeasSets[i].SetDiffractionMeasPar( 
        m_ImParam.m_iDiffractionMeasurementMethod ,
        m_ImParam.m_iDiffractionRadius , m_ImParam.m_iDiffractionRadius_Y , 
        m_ImParam.m_iBackgroundDist ) ;
    }
    EnableLPFilter( m_ImParam.m_bLowPassOn != 0  ) ;
  }

  void 
    CImageView::OnAsynchroneMode() 
  {
    m_bAsyncGrab ^= 1 ;
    if ( m_MeasSets.GetCount() )
    {
      CMeasurementSet * pSet = &m_MeasSets[0] ;
      if (pSet->m_GraphMode==1)
      {
        if (m_bAsyncGrab==1)
          pSet->SetTextThroughLAN("SETSYNCMODE__FALSE");    
        else
          pSet->SetTextThroughLAN("SETSYNCMODE__TRUE");
      }
      else
      {
        for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
        {
          SetGrabTrigger( (m_bAsyncGrab == 0) , i ) ;
        }
      }
    }
    m_wndToolBar.SetButtonInfo(
      11, ID_ASYNCHRONE, TBBS_BUTTON, (m_bAsyncGrab==0)? 8 : 22/*20*/) ;
  }


  void 
    CImageView::SelectView()
  {
    //        MdispSelectWindow( m_MDisp , m_DispBuf , m_hWnd ) ;
  }

  void 
    CImageView::OnMeasureLine()
  {
    CString Control( "Task(1);") ;
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      CTextFrame * p = CTextFrame::Create( Control ) ;
      if ( !m_pBuilder->SendDataFrame( p , m_MeasSets[i].m_ProcessControlName ) )
        p->Release( p ) ;
    }
  }

  int 
    CImageView::SaveRestoreImage(bool bSave)
  {
    if ( bSave )
      memcpy( m_pUserBuff , m_pSaveBuff,m_BuffSize ) ;    // save
    else
      memcpy( m_pSaveBuff , m_pUserBuff,m_BuffSize ) ;     // restore

    return 1 ;
  }


  void
    CImageView::CheckAndStopGrab( int bStop )
  {
    if ( m_LiveVideo )
    {
      KillTimer( m_Timer ) ;
      m_LiveVideo = 0 ;
      m_wndToolBar.SetButtonInfo(4, ID_LIVE_VIDEO, TBBS_BUTTON, 3);
      m_Timer = 0 ;
    }
  }

  void CImageView::OnLButtonDown(UINT nFlags, CPoint point) 
  {
    //   CString DirName = m_ImParam.m_ImageSaveDir ;
    //   DirName.TrimRight("\\") ;
    //   DirName += "\\" ;
    //   CString FileName ;
    //   if ( nFlags & MK_CONTROL )
    //   {
    // //     point.y -= TOOLBAR_SIZE ;
    //     if ( m_iImgMode == 0 ) //512x512
    //     {
    //       point.x *= 2 ;
    //       point.y *= 2 ;
    //     }
    //     //point.y -= Y_SCAN_OFFSET ;
    // 
    //     FileName.Format( "Prof%d_%d.dat" , point.x , point.y ) ;
    //     FileName = DirName + FileName ;
    // 
    //     FILE * fw = fopen( (LPCTSTR)FileName , "wt" ) ;
    // 
    //     if ( fw )
    //     {
    //       fprintf( fw , "Y Profile for X=%d\n  y   Br    i\n" , point.x ) ;
    // 
    //       for ( int i = -300 ; i <= 300 ; i++ )
    //       {
    //         int y = point.y + i ;
    //         int iResY = -1 ;
    //         if ( y >= 0   &&   y < 1024 )
    //         {
    //           iResY = *( GetGrabRow( y ) + point.x ) ;
    //           fprintf( fw , "%4d %4d %4d\n" , y , iResY , i  ) ;
    //         }
    //       }
    // 
    //       fprintf( fw , "X Profile for Y=%d\n  x   Br    i\n" , point.y ) ;
    // 
    //       for ( int i = -300 ; i <= 300 ; i++ )
    //       {
    //         int x = point.x + i ;
    //         int iResX = -1 ;
    //         if ( x >= 0   &&   x < 1024 )
    //         {
    //           iResX = *( GetGrabRow( point.y ) + x ) ;
    //           fprintf( fw, "%4d %4d %4d\n" , x , iResX , i  ) ;
    //         }
    //       }
    // 
    //       fclose( fw ) ;
    //     }
    //   }
    //   else
    //   {
    //     OnMeasureBlob() ;
    // 
    //     if(!m_NBlobs)
    //       return;
    // 
    //     CColorSpot mySpot = m_ColSpots.GetAt(m_piBlobIndexArr[0]);
    //     if ( m_NBlobs )
    //     {
    //       double dMinDist = GetDist( point.x , point.y , 
    //         mySpot.m_SimpleCenter.x,mySpot.m_SimpleCenter.y	);
    //       int iNearest = 0 ;
    //       for ( int i = 0 ; i < m_pCommon->iNLastFoundBlobs ; i++ )
    //       {
    //         mySpot.m_Runs.RemoveAll();
    //         mySpot = m_ColSpots.GetAt(m_iBigWhiteBlobsArrInd[i]);
    //         double dDist = GetDist( point.x , point.y , 
    //           mySpot.m_SimpleCenter.x,mySpot.m_SimpleCenter.y	);	
    // 
    //         if ( dDist < dMinDist  &&  mySpot.m_Area > 30 )
    //         {
    //           dMinDist = dDist ;
    //           iNearest = i ;
    //         }
    //       }
    // 
    //       if ( dMinDist < 20. )
    //       {
    //         mySpot = m_ColSpots.GetAt(m_piBlobIndexArr[iNearest]);
    //         CString msg ;
    //         msg.Format( 
    //           "C(%d,%d) Sz(%6.2f,%6.2f) Dist=%6.2f(%6.2f,%6.2f)" ,
    //           mySpot.m_SimpleCenter.x ,mySpot.m_SimpleCenter.y ,
    //           mySpot.m_dBlobWidth , mySpot.m_dBlobHeigth , 
    //           GetDist( m_dLastCx , m_dLastCy , 
    //           mySpot.m_SimpleCenter.x,mySpot.m_SimpleCenter.y ), 
    //           fabs(mySpot.m_SimpleCenter.x  - m_dLastCx ),
    //           fabs( mySpot.m_SimpleCenter.y	- m_dLastCy ) ) ;
    // 
    //         IApp()->LogMessage( msg ) ;
    // 
    //         FileName = DirName + "SpotPos.dat" ;
    //         FILE * fa = fopen( (LPCTSTR)FileName , "ab" ) ;
    //         if ( fa )
    //         {
    //           msg.Format( 
    //             "C= %6.1f %6.1f Size= %6.2f %6.2f Dist= %6.2f Dist_X= %6.2f Dist_Y= %6.2f" ,
    //             mySpot.m_SimpleCenter.x ,	mySpot.m_SimpleCenter.y	,
    //             mySpot.m_dBlobWidth, mySpot.m_dBlobHeigth,
    //             GetDist( m_dLastCx , m_dLastCy , mySpot.m_SimpleCenter.x , mySpot.m_SimpleCenter.y ) ,
    //             fabs( mySpot.m_SimpleCenter.x - m_dLastCx ) ,	
    //             fabs( mySpot.m_SimpleCenter.y - m_dLastCy ) ) ;
    // 
    //           fprintf( fa , "\r\n%s" , (LPCTSTR)msg ) ;
    //           fclose( fa ) ;
    //         }
    //         m_dLastCx = mySpot.m_SimpleCenter.x ;
    //         m_dLastCy = mySpot.m_SimpleCenter.y ;
    // 
    //       }
    //     }
    //   }
    //   CMainFrame::OnLButtonDown(nFlags, point);
    // 

  }

  void CImageView::OnUpdateLiveVideo(CCmdUI* pCmdUI) 
  {
    pCmdUI->SetCheck( m_LiveVideo ) ; //change picture i/o checked
  }    

  void CImageView::OnUpdateMinusBackground(CCmdUI* pCmdUI) 
  {
    pCmdUI->SetCheck( m_bSubBackground );
  }

  void CImageView::OnMeasurePower() 
  {
    //   double start = get_current_time() ;
    // 
    //   CGuard guard( &m_lock );
    //   guard.Lock();	//Blocks execution untill thread-safe lock is acquired.
    // 
    //   CRect  Img (0,0,m_CaptureSize.cx,m_CaptureSize.cy); 
    //   double Threshold = FindMinMaxIntens(Img,1);
    // 
    //   CColorSpot mySpot;
    // 
    //   if ( m_iLastMaxIntens - m_iLastMinIntens < (int)m_ImParam.m_iMinAmplitude )  
    //     return ;
    // 
    //   m_NBlobs = m_ColSpots.GetSize();
    //   if ( m_NBlobs )
    //   {
    // 
    //     int NMax = 0;
    //     mySpot = m_ColSpots.GetAt( NMax );	        
    // 
    //     //find first white spot with area:
    //     while (
    //           (mySpot.m_Area < 0  ||  mySpot.m_iColor <= 0)
    //       && ++NMax < m_NBlobs)
    //     {
    //       mySpot = m_ColSpots.GetAt( NMax );	        
    //     }
    // 
    //     if( NMax == m_NBlobs)
    //     {
    //       CString msg ;
    //       msg.Format( "No blobs" );
    //       IApp()->LogMessage( msg ) ;
    //       return;
    //     }
    // 
    //     int iMaxArea = mySpot.m_Area ;
    //     m_iMaxBlobNumber = NMax ;// suggesting NMax instead of 0
    // 
    // 
    //     // look for more white blobs:
    //     for ( ++NMax ; NMax < m_NBlobs ; NMax++ )
    //     {
    //       mySpot = m_ColSpots.GetAt( NMax );	        
    // 
    //       //find first white spot with area:
    //       while (
    //         (mySpot.m_Area < 0  ||  mySpot.m_iColor <= 0)
    //         && ++NMax < m_NBlobs)
    //     {
    //         mySpot = m_ColSpots.GetAt( NMax );	        
    //       }
    // 
    //       if(NMax == m_NBlobs)
    //         break; // no more white blobs
    // 
    //       if ( mySpot.m_Area > iMaxArea )
    //       {
    //         if ( ! m_ImParam.m_UseFFTFilter  
    //           || ( abs( mySpot.m_SimpleCenter.x - m_ImParam.m_iFFTXc ) < 200
    //           && abs( mySpot.m_SimpleCenter.y - m_ImParam.m_iFFTYc ) < 200 ) )
    //         {
    //           m_iMaxBlobNumber = NMax ;
    //           iMaxArea = mySpot.m_Area ;
    //         }
    //       }
    //     }
    // 
    //     cmplx LastPoint( m_ImParam.m_iViewCX , m_ImParam.m_iViewCY ) ;
    //     cmplx MinDist( 2000 , 2000 ) ;
    //     double dMinDist = 2000 ;
    // 
    //     double dAverageBack = GetAverageBackground() ;
    // 
    //     // MimArith( m_DefGrabBuf , ( int )( AverageBack + 0.5 ) , 
    //     //           m_DefGrabBuf , M_SUB_CONST ) ;
    //     for ( int i = 0 ; i < m_NBlobs ; i++ )
    //     {
    //       do {
    //         mySpot = m_ColSpots.GetAt(i);
    //       } while((mySpot.m_Area < 0 || !mySpot.m_iColor) && 
    //         ++i < m_NBlobs );
    // 
    //       if(i == m_NBlobs)
    //         break;
    // 
    //       double dSumLinBacks = 0 ;
    //       int iXc = mySpot.m_SimpleCenter.x ;
    //       int iYc = mySpot.m_SimpleCenter.y ;
    //       cmplx cPos( (double ) mySpot.m_SimpleCenter.x , (double )mySpot.m_SimpleCenter.y ) ;
    //       mySpot.m_dRDiffraction = 0. ;
    //       mySpot.m_dLDiffraction = 0. ;
    //       mySpot.m_dUDiffraction = 0. ;
    //       mySpot.m_dDDiffraction = 0. ;
    // 
    // #define BACK_LENGTH 20
    //       if ( mySpot.m_Area > m_ImParam.m_iMinArea 
    //         && (UINT)iXc > m_ImParam.m_iPowerRadius + 1 + BACK_LENGTH
    //         && (UINT)iXc < 1023 - m_ImParam.m_iPowerRadius - 1 - BACK_LENGTH
    //         && (UINT)iYc > m_ImParam.m_iPowerRadius + 1 + BACK_LENGTH 
    //         && (UINT)iYc < 1023 - m_ImParam.m_iPowerRadius - 1 - BACK_LENGTH
    //         )
    //       {
    //         int iSum = 0;
    //         int iBackSum = 0 ;
    // 
    //         int * Sums = new int[ m_ImParam.m_iPowerRadius + 1 ] ;
    //         int iRad = 0 ;
    //         for (  ; iRad <= (int)(m_ImParam.m_iPowerRadius) ; iRad++ )
    //         {
    //           int iRadSum = 0 ;
    //           int iY = iYc - iRad ;
    //           int iX = iXc - iRad ;
    //           WORD * p = GetGrabRow( iY ) + iX ; // upper row
    // 
    //           int iCurrLen = 2 * iRad + 1 ;
    //           int iCnt = 0 ;
    //           for (  ; iCnt < iCurrLen ; iCnt++ )
    //             iRadSum += (int)(*(p++)) /*- (int)(*(pBack++))*/ ;
    // 
    //           int iBackSum = 0 ;
    //           WORD * pBack = GetGrabRow( iY ) + iXc - m_ImParam.m_iPowerRadius - BACK_LENGTH ;
    //           for ( iCnt = 0 ; iCnt < BACK_LENGTH ; iCnt++ ) // sum left back
    //             iBackSum += (int)(*(pBack++)) ;
    //           pBack = GetGrabRow( iY ) + iXc + m_ImParam.m_iPowerRadius + 1 ;
    //           for ( iCnt = 0 ; iCnt < BACK_LENGTH ; iCnt++ ) // sum left back
    //             iBackSum += (int)(*(pBack++)) ;
    //           dSumLinBacks += iBackSum ;
    // 
    //           if ( iRad > 0 )
    //           {
    //             iY = iYc + iRad ;
    //             p = GetGrabRow( iY ) + iX ; // lower row
    // 
    //             for ( iCnt = 0 ; iCnt < iCurrLen ; iCnt++ )
    //               iRadSum += (int)(*(p++)) /*- (int)(*(pBack++))*/ ;
    // 
    //             int iBackSum = 0 ;
    //             WORD * pBack = GetGrabRow( iY ) + iXc - m_ImParam.m_iPowerRadius - BACK_LENGTH ;
    //             for ( iCnt = 0 ; iCnt < BACK_LENGTH ; iCnt++ ) // sum left back
    //               iBackSum += (int)(*(pBack++)) ;
    //             pBack = GetGrabRow( iY ) + iXc + m_ImParam.m_iPowerRadius + 1 ;
    //             for ( iCnt = 0 ; iCnt < BACK_LENGTH ; iCnt++ ) // sum left back
    //               iBackSum += (int)(*(pBack++)) ;
    //             dSumLinBacks += iBackSum ;
    // 
    //             iY = iYc - iRad + 1 ;       // beginning of left column
    //             p = GetGrabRow( iY ) + iX ; // left column
    // 
    //             iCurrLen -= 2 ; // remove corner points
    // 
    //             for ( iCnt = 0 ; iCnt < iCurrLen ; iCnt++ )
    //             {
    //               iRadSum += (int)(*p) /*- (int)(*pBack)*/ ;
    //               p += m_GrabWidth ;
    //               /*              pBack += m_GrabPitch ;
    //               */            }
    // 
    //             iX = iXc + iRad ;           // beginning of right column
    //             p = GetGrabRow( iY ) + iX ; // right column
    //             /*            pBack = GetBackGrabRow( iY ) + iX ;
    //             */
    //             for ( iCnt = 0 ; iCnt < iCurrLen ; iCnt++ )
    //             {
    //               iRadSum += (int)(*p) /*- (int)(*pBack)*/ ;
    //               p += m_GrabWidth ;
    //               /*              pBack += m_GrabPitch ;
    //               */            }
    //           }
    //           iSum += iRadSum ;
    //           if ( Sums )
    //             Sums[ iRad ] = iRadSum /*
    //                                    - (int)( ( ( iRad == 0 )? 1 : 8 * iRad ) * dAverageBack )*/ ;
    //         }
    // 
    //         int iNPix = 1 + (iRad - 1) * 2 ;
    //         iNPix *= iNPix ;
    // 
    //         double dAverageLast = (double)Sums[ iRad - 1 ] / (8. * iRad);
    //         mySpot.m_dSumPower = (double)iSum - dAverageLast * iNPix ;
    // 
    //         int dX = mySpot.m_OuterFrame.right - mySpot.m_OuterFrame.left ;
    //         int dY = mySpot.m_OuterFrame.bottom - mySpot.m_OuterFrame.top ;
    // 
    //         int OffX = mySpot.m_OuterFrame.left - m_ImParam.m_iMeasExpansion ;
    //         if ( OffX < 0 )
    //           OffX = 0 ;
    //         int OffY = mySpot.m_OuterFrame.top - m_ImParam.m_iMeasExpansion ;
    //         if ( OffY < 0 )
    //           OffY = 0 ;
    //         int LenX = mySpot.m_OuterFrame.right - mySpot.m_OuterFrame.left + m_ImParam.m_iMeasExpansion * 2 ;
    //         if ( OffX + LenX >= m_GrabWidth )
    //           LenX = m_GrabWidth - OffX ;
    //         int LenY = mySpot.m_OuterFrame.bottom - mySpot.m_OuterFrame.top + m_ImParam.m_iMeasExpansion * 2 ;
    //         if ( OffY + LenY >= m_GrabHeight )
    //           LenY = m_GrabHeight - OffY ;
    //         int iHorizValues[1024] ;
    //         int iVertValues[1024]	;
    // 
    // 
    // 
    //         if ( LenX < 400  &&  LenY < 400 )
    //         {
    //           CRect  pChildImg (OffX,OffY,OffX + LenX,OffY + LenY );
    // //           FindSumOfRows(iHorizValues,pChildImg);
    // //           FindSumOfCols(iVertValues,pChildImg);
    //           // BUG! a mix up between the horizontal and vertical values.
    //           // Fixed by Alex at 26.10.09
    //           FindSumOfRows( iVertValues, pChildImg ) ;
    //           FindSumOfCols( iHorizValues, pChildImg ) ;
    //           mySpot.m_dBlobWidth = FindWidthInArray( iHorizValues , LenX ) ;	
    //           mySpot.m_dBlobHeigth = FindWidthInArray( iVertValues , LenY ) ;
    //           if ( Sums  &&  m_ImParam.m_bSavePowerData )
    //           {
    //             iRad = (int)m_ImParam.m_iPowerRadius ;
    // 
    // //             FILE * fa = fopen( (LPCTSTR)m_ImParam.m_PowerSaveName , "ab" ) ;
    // // 
    // //             if ( fa )
    // //             {
    // //               fseek( fa , 0 , SEEK_END ) ;
    // //               fprintf( fa , "\r\nHProfile (%d points):" , LenX ) ;
    // //               for (  int i = 0 ; i < LenX ; i++ )
    // //               {
    // //                 if ( (i % 10) == 0 )
    // //                 {
    // //                   fprintf( fa , "\r\n" ) ;
    // //                 }
    // //                 fprintf( fa , "%7d " , iHorizValues[i] ) ;
    // //               }
    // //               fprintf( fa , "\r\nVProfile (%d points):" , LenY ) ;
    // //               for (  int i = 0 ; i < LenX ; i++ )
    // //               {
    // //                 if ( (i % 10) == 0 )
    // //                 {
    // //                   fprintf( fa , "\r\n" ) ;
    // //                 }
    // //                 fprintf( fa , "%7d " , iVertValues[i] ) ;
    // //               }
    // //               
    // //               fclose (fa) ;
    // //             }
    //           }
    //         }
    //         else
    //         {
    //           mySpot.m_dBlobWidth = 0.0 ;
    //           mySpot.m_dBlobHeigth = 0.0 ;
    //         }
    //         double dAverLinBack = 
    //           dSumLinBacks / (2. * BACK_LENGTH * ( 2. * m_ImParam.m_iPowerRadius + 1 ) ) ;
    //         double dSumBacksOnArea = iNPix * dAverLinBack ;
    // 
    // 
    //         cmplx Dist = cPos - LastPoint ;
    //         double dDist = abs( Dist ) ;
    // 
    //         if ( dDist < dMinDist )// &&  dDist < 100 )
    //         {
    //           dMinDist = dDist ;
    //           MinDist = Dist ;
    //           CString msg ;
    //           msg.Format( "pL=%10.1f pG=%10.1f L=%4.1f G=%4.1f (%d,%d) %8.1f" ,
    //             mySpot.m_dSumPower , 
    //             mySpot.m_dSumPower + dAverageLast * iNPix - dSumBacksOnArea ,
    //             dAverageLast , dAverLinBack , 
    //             mySpot.m_SimpleCenter.x , mySpot.m_SimpleCenter.y ,
    //             get_current_time() - start ) ;
    // 
    //           IApp()->LogMessage( msg ) ;
    // 
    //           if ( Sums  &&  m_ImParam.m_bSavePowerData )
    //           {
    //             iRad = (int)m_ImParam.m_iPowerRadius ;
    // 
    //             FILE * fa = fopen( (LPCTSTR)m_ImParam.m_PowerSaveName , "ab" ) ;
    // 
    //             if ( fa )
    //             {
    //               fseek( fa , 0 , SEEK_END ) ;
    //               if ( ftell( fa ) < 10 )
    //               {
    //                 CTime t = CTime::GetCurrentTime();
    //                 CString s = t.Format( "%A, %B %d, %Y   %H:%M:%S" );
    //                 fprintf( fa , "\r\n%s" , (LPCTSTR)s ) ;
    //               }
    // 
    //               double dSumMinusNeightbours =
    //                 mySpot.m_dSumPower + dAverageLast * iNPix - dSumBacksOnArea ;
    //               fprintf( fa , "\r\npL=%10.1f pG=%10.1f L=%4.1f G=%4.1f C=(%4d,%4d) Sz=(%5.1f,%5.1f) Exp=%d [%d,%d,%d,%d]" ,
    //                 mySpot.m_dSumPower , 
    //                 dSumMinusNeightbours ,
    //                 dAverageLast , dAverLinBack , 
    //                 mySpot.m_SimpleCenter.x , mySpot.m_SimpleCenter.y ,
    //                 mySpot.m_dBlobWidth , mySpot.m_dBlobHeigth ,
    //                 m_ExposureControl.m_iExposure , OffX , OffY ,
    //                 OffX+LenX , OffY+LenY ) ;
    // 
    //               m_dAverSum += dSumMinusNeightbours ;
    //               m_dAverSum2 += dSumMinusNeightbours * dSumMinusNeightbours ;
    //               m_dAverX += (double)mySpot.m_SimpleCenter.x; //m_dCenterX[ i ] ;
    //               m_dAverY += (double)mySpot.m_SimpleCenter.y; //m_dCenterY[ i ] ;
    //               m_dAverSzX += mySpot.m_dBlobHeigth; //m_dBlobWidth[ i ] ;
    //               m_dAverSzY += mySpot.m_dBlobWidth; //m_dBlobHeigth[ i ] ;
    //               m_iNAver++ ;
    // 
    //               if ( m_ImParam.m_SaveExtendedInformation )
    //               {
    //                 CString s("\r\n     ");
    // 
    //                 int iSum10 = 0 ;
    //                 for ( int j = 0 ; j <= (int)(m_ImParam.m_iPowerRadius) ; j++ )
    //                 {
    //                   CString tmp ;
    //                   int iPure = Sums[ j ] - ROUND( dAverageLast * 8 * j ) ; 
    //                   tmp.Format( "%7d " , iPure /*Sums[ j ]*/ ) ;
    //                   s += tmp ;
    //                   iSum10 += iPure ;
    //                   if ( j &&  ((j % 10) == 9) )
    //                   {
    //                     fprintf( fa , (LPCTSTR) s ) ;
    //                     s.Format( "  %7.5f\r\n%4d " , 
    //                       mySpot.m_dSumPower > 0 ? 
    //                       (double)iSum10/mySpot.m_dSumPower : 0. , j + 1 ) ;
    //                     iSum10 = 0 ;
    //                   }
    //                 }
    // 
    //                 CString tmp ;
    //                 tmp.Format( "  %7.5f\r\n " , 
    //                   mySpot.m_dSumPower > 0 ? 
    //                   (double)iSum10/mySpot.m_dSumPower : 0. ) ;
    //                 s += tmp ;
    // 
    //                 fprintf( fa , (LPCTSTR) s ) ;
    //               }
    // 
    //               fclose( fa ) ;
    //             }
    // 
    //           }
    //           m_ImParam.m_iViewCX = ROUND( cPos.real() ) ;
    //           m_ImParam.m_iViewCY = ROUND( cPos.imag() ) ;
    //         }
    //         delete[] Sums ;
    //       }
    // 
    //       mySpot.m_Runs.RemoveAll();
    //       m_ColSpots.SetAt(i,mySpot);
    //     }
    // 
    //   }
    // 
    // 
  }



  void CImageView::OnMoving(UINT fwSide, LPRECT pRect) 
  {
    CMainFrame::OnMoving(fwSide, pRect);

    // TODO: Add your message handler code here

  }

  // void CImageView::OnPaint() 
  // {
  //   /********************************************************************
  //   created:	23.02.10
  //   class: CImageView
  // 
  //   purpose:Output of image and time intervals, Creating compressed image 
  //   for output.
  //   *********************************************************************/
  // }

  int CImageView::SaveCurrentImage(
    int iXc , int iYc , int iRadiusX , int iRadiusY)
  {
    // Edited on 28.10.09 by Alex to fit the Solios code
    // the Meteor code is here and commented out
    // even though, as it is, it really makes no sence at all

    // Some contents commented out at 21/2/2010 by Alex
    // as a part of Eilat ATJ development 

    CString FileName ;
    CTime t = CTime::GetCurrentTime() ;

    //   FileName.Format( "%02d%02d%02d-%02d%02d%02d_%s.tif" ,
    //     t.GetDay() , t.GetMonth() , t.GetYear() % 100 ,
    //     t.GetHour() , t.GetMinute() , t.GetSecond() ,
    //     m_ImParam.m_bSave16Bits ? "16" : "8" ) ;

    FileName.Format( "%02d%02d%02d_%02d%02d%02d_X%dY%d.tif" ,
      t.GetDay() , t.GetMonth() , t.GetYear() % 100 ,
      t.GetHour() , t.GetMinute() , t.GetSecond() ,
      iXc , iYc ) ;

    CString Temp = m_ImParam.m_ImageSaveDir ;
    Temp.TrimRight("\\") ;
    Temp += "\\" ;
    FileName = Temp + FileName ;

    if ( iXc < iRadiusX )
      iRadiusX = iXc ;
    if ( iYc < iRadiusY )
      iRadiusY = iYc ;


    //   BYTE * pShowBuff = (BYTE *)m_pDibview->GetDIBdataBufferPtr();
    //   int iHeight = m_pDibview->GetDIBdataHeight();
    //   int iWidth = m_pDibview->GetDIBdataWidth();	
    //   int iSize = m_pDibview->GetDIBdataBufferSize();
    //   BYTE * pTemp = new BYTE[iSize] ;
    //   BYTE * pDst = pTemp ;	   
    //   for(int iY = 0;iY < iHeight; iY ++)
    //   {
    //     BYTE * pSrc;
    //     pSrc = &pShowBuff[ (iHeight - iY - 1) * iWidth ] ; 
    //     BYTE * pStop = pSrc + iWidth ;
    //     for( ; pSrc < pStop ; pSrc ++ , pDst++ )
    //       *pDst = *pSrc ;
    //   }
    // 
    //   int iNewYc = m_CaptureSize.cy - iYc; 
    // 
    //   delete [] pTemp;

    //   MIL_ID idChild = MbufChild2d( m_MGrabBuf , 
    //     iXc - (2*iRadiusX) , iYc - (2*iRadiusY) , 
    //     (4 * iRadiusX) + 1 , (4 * iRadiusY) + 1 ,
    //     M_NULL ) ;
    // 
    //   if (idChild)
    //   {
    //     MbufExport( (MIL_TEXT_PTR)(LPCTSTR)FileName , M_MIL , idChild ) ;
    //     MbufFree(idChild) ;
    //   }
    //   else
    //     ASSERT(0) ;


    return 1 ;
  }

  void CImageView::OnMeasureYDist() 
  {
    //   /********************************************************************
    //   created:	2005/11/09
    //   class: CImageView
    //   author:	Moisey Bernstein
    //   modified by Michael Son	 
    // 
    //   purpose: finding distance between 2 blobs on the y-coordinate 
    //   *********************************************************************/
    // 
    //   OnMeasureBlob() ;
    //   CString msg ;
    //   if ( !m_NBlobs )
    //   {
    //     IApp()->LogMessage( "ERROR: Don't see blobs" ) ;
    //     return ;
    //   }
    //   if ( m_NBlobs < 2 )
    //   {
    //     IApp()->LogMessage( "ERROR: See only one blob" ) ;
    //     return ;
    //   }
    //   CColorSpot mySpot = m_ColSpots.GetAt(m_iMaxBlobNumber) ;
    //   msg.Format( "N=%d C=(%d,%d) S=%d" ,
    //     m_NBlobs , 
    //     mySpot.m_SimpleCenter.x, mySpot.m_SimpleCenter.y ,
    //     mySpot.m_Area);
    //   IApp()->LogMessage( msg ) ;
    // 
    //   double dBlobWidth = mySpot.m_OuterFrame.right - mySpot.m_OuterFrame.left ;
    //   double dBlobHeight = mySpot.m_OuterFrame.bottom - mySpot.m_OuterFrame.top ;
    //   if ( abs(mySpot.m_SimpleCenter.x - 512) < 100 
    //     && dBlobWidth > 250 
    //     && dBlobHeight < 120 )
    //   {  // OK, we did find blob
    //     // There is Necessary to find second big blob
    //     CColorSpot 	mySpot2;
    //     int iSecondBlob = -1 ;
    //     for ( int i = 0 ; i < m_pCommon->iNLastFoundBlobs ; i++ )
    //     {
    //       mySpot2.m_Runs.RemoveAll();
    //       mySpot2 = m_ColSpots.GetAt(m_iBigWhiteBlobsArrInd[i]);
    //       if ( m_iBigWhiteBlobsArrInd[i] == m_iMaxBlobNumber )
    //         continue ;
    //       double dBlobWidth2 = mySpot2.m_OuterFrame.right -
    //         mySpot2.m_OuterFrame.left ;
    //       double dBlobHeight2 = mySpot2.m_OuterFrame.bottom - 
    //         mySpot2.m_OuterFrame.top ;
    //       if ( abs(mySpot.m_SimpleCenter.x - mySpot2.m_SimpleCenter.x ) < 70
    //         && dBlobWidth2 > 250
    //         && dBlobHeight2 < 120 )
    //       {  // OK. There is second  blob
    //         iSecondBlob = i ;
    // 
    //         if ( m_iMaxLines < 2 )
    //           AllocLineResults( 2 ) ;
    // 
    // 
    //         int iMinX = 30 + max(mySpot2.m_OuterFrame.left , mySpot.m_OuterFrame.left ) ;
    //         int iMaxX = -30 + min(mySpot2.m_OuterFrame.right ,mySpot2.m_OuterFrame.right ) ;
    // 
    //         CPoint LeftTopFirst( iMinX , mySpot.m_SimpleCenter.y - 50 ) ;
    //         CPoint RightDownFirst( iMaxX , mySpot.m_SimpleCenter.y + 50 ) ;
    // 
    //         double dCx1 , dCy1 , dCx2 , dCy2 ;
    //         int iNPix1 = GetWeightCenter( LeftTopFirst , RightDownFirst , dCx1 , dCy1 ) ;
    // 
    //         CPoint LeftTopSecond( iMinX , mySpot2.m_SimpleCenter.y - 50 ) ;
    //         CPoint RightDownSecond( iMaxX , mySpot2.m_SimpleCenter.y + 50 ) ;
    // 
    //         int iNPix2 = GetWeightCenter( LeftTopSecond , RightDownSecond , dCx2 , dCy2 ) ;
    // 
    //         double dDistY = fabs( dCy1 - dCy2 ) ;
    // 
    //         msg.Format( "dY=%7.2f C1(%d,%d) C2(%d,%d) S1=%d S2=%d" ,
    //           dDistY , 
    //           mySpot.m_SimpleCenter.x,mySpot.m_SimpleCenter.y ,
    //           mySpot2.m_SimpleCenter.x,mySpot2.m_SimpleCenter.y ,
    //           mySpot.m_Area ,mySpot2.m_Area) ;
    //         IApp()->LogMessage( msg ) ;
    // 
    //         m_dLinePos[ 0 ] = mySpot.m_SimpleCenter.y ;
    //         m_dLinePos[ 1 ] = mySpot2.m_SimpleCenter.y ;
    //         m_NLines = 2 ;
    //         m_dLineLeftPos[ 0 ] = m_dLinePos[ 0 ] ;
    //         m_dLineLeftPos[ 1 ] = m_dLinePos[ 1 ] ;
    //         m_dLineRightPos[ 0 ] = m_dLinePos[ 0 ] ;
    //         m_dLineRightPos[ 1 ] = m_dLinePos[ 1 ] ;
    //         m_dLineLeftWidth[ 0 ] = 20 ;
    //         m_dLineLeftWidth[ 1 ] = 20 ;
    //         m_dLineRightWidth[ 0 ] = 20 ;
    //         m_dLineRightWidth[ 1 ] = 20 ;
    //         m_dLineWidth[ 0 ] = 20 ;
    //         m_dLineWidth[ 1 ] = 20 ;
    //         m_dLineAngle[ 0 ] = mySpot.m_dAngle ;
    //         m_dLineAngle[ 1 ] = mySpot2.m_dAngle ;
    // 
    //         if ( m_ImParam.m_SaveExtendedInformation )
    //         {
    //           int iXc = mySpot.m_SimpleCenter.x ;
    //           int iYc = (int)( (dCy1 + dCy2) * 0.5 ) ;
    //           int iRadX = (int)(dBlobWidth * 0.5 + 85.5) ;
    //           int iRadY = (int)(dDistY * 0.5 + 75.5) ;
    //           SaveCurrentImage(  iXc , iYc , iRadX , iRadY );
    // 
    //           CString FileName ;
    //           CTime t = CTime::GetCurrentTime() ;
    // 
    //           FileName.Format( "%02d%02d%02d-%02d%02d%02d.dat" ,
    //             t.GetDay() , t.GetMonth() , t.GetYear() % 100 ,
    //             t.GetHour() , t.GetMinute() , t.GetSecond() ) ;
    // 
    //           CString Temp = m_ImParam.m_ImageSaveDir ;
    //           Temp.TrimRight("\\") ;
    //           Temp += "\\" ;
    // 
    //           FileName = Temp + FileName ;
    //           FILE * fw = fopen( (LPCTSTR)FileName , "wb" ) ;
    //           if ( fw )
    //           {
    //             int ForBackGround[ 1024 ] ;
    // 
    //             CPoint Center = CPoint( iXc , iYc ) ;
    // 
    //             int iTopBorder = Center.y - iRadY ;
    //             if ( iTopBorder < 1 )
    //               iTopBorder = 1 ;
    //             int iBottomBorder = Center.y + iRadY ;
    //             if ( iBottomBorder > 1022 )
    //               iBottomBorder = 1022 ;
    // 
    //             int iRightBackGroundBorder = Center.x + iRadX ;
    //             if ( iRightBackGroundBorder > 1022 )
    //               iRightBackGroundBorder = 1022 ;
    // 
    //             int iLeftBackGroundBorder = Center.x - iRadX ;
    //             if ( iLeftBackGroundBorder < 1 )
    //               iLeftBackGroundBorder = 1 ;
    //             int iy = iTopBorder ;
    //             for (  ; iy <= iBottomBorder ; iy++ )
    //             {
    //               int iSum = 0 ;
    //               WORD * pLeft = GetGrabRow( iy ) + Center.x - iRadX ;
    //               WORD * pRight = GetGrabRow( iy ) + Center.x + iRadX - 10 ;
    //               for ( int i = 0 ; i < 10 ; i++ )
    //               {
    //                 iSum += *(pLeft++) + *(pRight++) ;
    //               }
    //               ForBackGround[ iy ] = iSum / 20 ;
    //             }
    // 
    //             fprintf( fw , "LT(%d,%d) RB(%d,%d)\r\n  #   Integral  Background\r\n" ,
    //               iLeftBackGroundBorder + 10 , iTopBorder , 
    //               iRightBackGroundBorder - 10 , iBottomBorder ) ;
    // 
    //             for ( iy = iTopBorder ; iy <= iBottomBorder ; iy++ )
    //             {
    //               WORD * p = GetGrabRow( iy ) + Center.x - iRadX + 10 ;
    //               int iSum = 0 ;
    //               for ( int ix = 0 ; ix <= 2 * iRadX - 20 ; ix++ )
    //                 iSum += *(p++) ;
    // 
    //               iSum -= ForBackGround[ iy ] * ( 2 * (iRadX - 20) + 1 ) ;
    //               fprintf( fw , "%3d    %7d    %4d\r\n" , iy , iSum , ForBackGround[ iy ] ) ;  
    //             }
    //             fclose( fw ) ;
    //           }
    // 
    //         }
    // 
    // 
    //         return ; // second blob searching and all measurements
    //       }
    //     }
    //   }
    // 
    //   if ( dBlobWidth > 150  &&  dBlobHeight < 120 )
    //     m_NLines = -(mySpot.m_SimpleCenter.x) ;
    //   else
    //     m_NLines = 0 ;
    // 
    //   CString Additional ;
    //   Additional.Format(" %d returned" , m_NLines ) ;
    //   msg += Additional ;
    //   IApp()->LogMessage( msg ) ;
  }


  void CImageView::OnUpdateAsynchMode(CCmdUI* pCmdUI) 
  {
    // TODO: Add your command update UI handler code here

  }

  void CImageView::OnRButtonDown(UINT nFlags, CPoint point) 
  {
    if ( (nFlags & MK_CONTROL)  && (nFlags & MK_SHIFT) )
    {
      FindBadPixels() ;
    }
    else if ( !m_bRButtonDlgModal )
    {
      m_bRButtonDlgModal = TRUE ;
      m_RButtonDlg.DoModal() ;
      m_bRButtonDlgModal = FALSE ;

      m_BigViewWnd.ShowWindow( ( m_RButtonDlg.m_iViewMode ) ? SW_SHOW : SW_HIDE ) ;          	
      if ( m_RButtonDlg.m_iViewMode == 0 )
      {
        m_RButtonDlg.m_SpotSearchCoordinates.Empty() ;
        m_RButtonDlg.m_SpotSearch = CPoint(0,0) ;
      }
    }
    else
    {
      m_RButtonDlg.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
      m_RButtonDlg.ShowWindow(SW_SHOW);	  
    }
    CMainFrame::OnRButtonDown(nFlags, point);
  }

  int CImageView::FindBadPixels()
  {
    //   m_BadPoints.RemoveAll() ;
    //   CRegistry Reg("File Company\\ImCNTL");
    //   CString  FileName = Reg.GetRegiString("ImProcParam","BadPixelsDirectory",
    //     "S:\\DotNetOpticJigDev\\BadPixels.dat"); 
    // 
    //   //FileName = "S:\\DotNetOpticJigDev\\BadPixels3.dat";                                                       
    // 
    //   FILE * fw = fopen( ( LPCTSTR )FileName , "wt" ) ;
    // 
    //   m_GrabWidth	= m_CaptureSize.cx;
    //   m_GrabHeight = m_CaptureSize.cy;
    // 
    //   CRect Img	(5,5,m_CaptureSize.cx - 5,m_CaptureSize.cy - 5);
    //   m_byThres = 300 ; //FindMinMaxIntens(Img,1)	;
    //   m_iFindBadPix = 1;
    //   int index = 0;
    //   CString msg;
    //   WORD * pImage = m_pUserBuff ;
    //   for ( int iY = 0 ; iY < m_GrabHeight ; iY++ )
    //   {
    //     WORD * pRow = pImage + iY * m_GrabWidth ;
    //     for ( int iX = 0 ; iX < m_GrabWidth	; iX++ )
    //     {
    //       int iRightPt = (iX < m_GrabWidth - 1) ? abs(*pRow - *(pRow+1)) : 0;
    //       int iLeftPt = (iY > 0) ? abs(*pRow - *(pRow-1)) : 0;
    //       int iUpPt = (iX > 0) ? abs(*pRow - *(pRow-m_GrabWidth)): 0;
    //       int iDown = (iY < m_GrabHeight-1) ? abs(*pRow - *(pRow+m_GrabWidth)) : 0;
    // 
    //       if (    ( iRightPt > m_byThres )  // check right
    //         &&  ( iUpPt > m_byThres ) // check up
    //         &&  ( iLeftPt > m_byThres )  // check left
    //         &&  ( iDown > m_byThres)  // check down
    //         )
    //       {   // we have separate point with high contrast
    //         m_BadPoints.Add( CPoint(iX,iY) ) ;
    //         if(index < 50)
    //         {
    //           m_BadPix[index].iX = iX;
    //           m_BadPix[index].iY = iY;
    //           m_BadPix[index++].iB = *pRow;
    // 
    //           msg.Format( "%d,%d %d \n\r",iX,iY,min(min(iLeftPt,iRightPt),min(iUpPt,iDown))) ;
    //           fputs ( (LPCTSTR)msg, fw );
    //         }
    //       }
    //       pRow++ ;
    //     }
    //   }
    //   fputs("eof",fw);
    //   fclose (fw);
    //   m_iYOffset = (m_iImgMode) ? Y_SCAN_OFFSET : Y_SCAN_OFFSET / 2;      
    //   CString Msg ;
    //   if (m_BadPoints.GetSize() )
    //     Msg.Format( "%d bad CCD pix, 1[%d,%d] " , 
    //     m_BadPoints.GetSize() , m_BadPoints[0].x , m_BadPoints[0].y /*- m_iYOffset*/) ;
    //   else
    //     Msg = "No bad CCD pixels" ;
    // 
    // 
    //   IApp()->LogMessage2(Msg) ;
    //   return m_BadPoints.GetSize() ;

    return 0 ;
  }

  // This is correspondent to SH version 3.0.11
  //  Defines for graphics viewing      //  Bit Mask          View
#define OBJ_VIEW_POS    0x00000001     //   0  0x0001    Cross on measured position
#define OBJ_VIEW_DET    0x00000002     //   1  0x02      Measurement details
#define OBJ_VIEW_ANGLE  0x00000010     //   4  0x0010    Angle show
#define OBJ_VIEW_SCALED 0x00000100     //   8  0x0100    SHow coordinates as scaled
#define OBJ_VIEW_DIFFR  0x00000200     //   9  0x0200    Diffraction areas
#define OBJ_VIEW_CONT   0x00000400     //  10  0x0400    Contour show
#define OBJ_VIEW_PROFX  0x00001000     //  12  0x1000      Profile X
#define OBJ_VIEW_PROFY  0x00002000     //  13  0x2000      Profile Y
#define OBJ_VIEW_DIA    0x00004000     //  14  0x4000    dia show
#define OBJ_WEIGHTED    0x00020000
#define OBJ_VIEW_MRECTS 0x20000000
#define OBJ_FIND_OBJ    0x10000000
#define OBJ_VIEW_COORD  0x40000000 //  30  0x40000000    Measured coordinates
#define OBJ_VIEW_ROI    0x80000000 //  31  0x80000000    ROI (search area)


  void CImageView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
  {
    if ( GetAsyncKeyState( VK_CONTROL) 
      &&  !GetAsyncKeyState( VK_SHIFT) )
    {
      DWORD dwViewMode = 0 ;
      switch ( nChar )
      {
      case '1': dwViewMode |= OBJ_VIEW_ROI ; break ; // ROI view
      case '2': dwViewMode |= OBJ_VIEW_POS ; break ; // Show cross on position
      case '3': dwViewMode |= OBJ_VIEW_COORD ; break ; // View position coordinates
      case '4': dwViewMode |= OBJ_VIEW_DET ; break ; // View details
      case '5': dwViewMode |= OBJ_VIEW_PROFX ; break ;// View Profile X
      case '6': dwViewMode |= OBJ_VIEW_PROFY ; break ;// View profile Y
      case '7': dwViewMode |= OBJ_VIEW_MRECTS ; break ;// View diffraction measurement areas
      case '8': dwViewMode |= OBJ_VIEW_CONT ; break ;// View contour
      case '9': dwViewMode |= OBJ_VIEW_DIA ; break ;// View diameters
      case '0': dwViewMode |= OBJ_VIEW_DIFFR ; break ;// toggle on/off scale
      default: break ;
      }
      if ( dwViewMode )
      {
        m_dwSHViewMode ^= dwViewMode ;
        CString Control ;
        Control.Format( "ViewMode(0x%08x);" , m_dwSHViewMode ) ;
        {
          for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
          {
            CTextFrame * p = CTextFrame::Create( Control ) ;
            if ( !m_pBuilder->SendDataFrame( p , m_MeasSets[i].m_ProcessControlName ) )
              p->Release( p ) ;
          }
        }
      }
    }
    if ( GetAsyncKeyState( VK_CONTROL) 
      &&  GetAsyncKeyState( VK_SHIFT) )
    {
      if ( nChar != VK_CONTROL  &&  nChar != VK_SHIFT )
      {
        DWORD dwTask = -2 ;
        switch ( nChar )
        {
        case 'S':
        case 's':
        case '0': 
        case ')': dwTask = 0 ; break ; // spot

        case 'L':
        case 'l':
        case '!':
        case '1': dwTask = 1 ; break ; // lines
        case '@':
        case '2': dwTask = 2 ; break ; // many spots without coords
        case '#':
        case '3': dwTask = 3 ; break ; // horizontal scaling
        case '$':
        case '4': dwTask = 4 ; break ; // vertical scaling
        case '%':
        case '5': dwTask = 5 ; break ;// both scalings
        case '^':
        case '6': dwTask = 6 ; break ;// many spots with coords
        case 'N':
        case 'n': dwTask = -1 ; break ;// View profile Y
        default: break ;
        }
        if ( dwTask != -2 )
        {
          CString Control ;
          Control.Format( "Task(%d);" , dwTask ) ;
          {
            for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
            {
              CTextFrame * p = CTextFrame::Create( Control ) ;
              if ( !m_pBuilder->SendDataFrame( p , m_MeasSets[i].m_ProcessControlName ) )
                p->Release( p ) ;
            }
          }
        }
      //     else
      //     {
      //       int bOurCommand = 1 ;
      //       CRect r ;
      //       GetClientRect( &r ) ;
      // 
      //       switch (nChar)
      // 
      //       {
      //       case VK_UP: 
      //         m_LastCursorPoint.y--;
      //         if ( !r.PtInRect( m_LastCursorPoint ) )
      //           m_LastCursorPoint.y++ ;
      //         break ;
      //       case VK_DOWN:
      //         m_LastCursorPoint.y++;
      //         if ( !r.PtInRect( m_LastCursorPoint ) )
      //           m_LastCursorPoint.y-- ;
      //         break ;
      //       case VK_LEFT:
      //         m_LastCursorPoint.x--;
      //         if ( !r.PtInRect( m_LastCursorPoint ) )
      //           m_LastCursorPoint.x++ ;
      //         break ;
      //       case VK_RIGHT:
      //         m_LastCursorPoint.x++;
      //         if ( !r.PtInRect( m_LastCursorPoint ) )
      //           m_LastCursorPoint.x-- ;
      //         break ;
      //       default:
      //         bOurCommand = 0 ;
      //         break ;
      //       }
      //       if (bOurCommand)
      //       {
      //         DrawProfiles(m_LastCursorPoint) ;
      //         CPoint point (m_LastCursorPoint) ;
      //         ClientToScreen( &point ) ;
      //         SetCursorPos( point.x , point.y ) ;
      //       }
      //     }
      //   }
      // 
      }
    }

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
  }


  void CImageView::OnContrasting() 
  {
    if ( !m_bContrasting ) //to off
    {
      m_wndToolBar.SetButtonInfo(12, ID_CONTRASTING, TBBS_BUTTON, 24/*22*/);
      m_bContrasting = TRUE;
      //     if(!m_LiveVideo)
      //     {
      //      m_bContrasting = TRUE;
      //      MakeContrast();
      //      memcpy(m_pSaveBuff,m_pUserBuff,m_BuffSize);
      // 		 Invalidate();
      //     }
      //     else
      //     {
      //       m_bOnGrab = TRUE;
      //       StopGrab();
      //       m_bOnGrab = TRUE;
      //       Sleep(300);
      //       m_bContrasting = TRUE;
      //       m_iBackToLiveVideo = 0;
      //       m_iFirstChangeBuf = 0;
      //       SingleGrab();
      // 
      //     }
    }
    else   //to on
    {
      m_wndToolBar.SetButtonInfo(12, ID_CONTRASTING, TBBS_BUTTON, 9);
      m_bContrasting = FALSE;
      //     if(m_LiveVideo)
      //     {
      //       Sleep(600);
      //       if(WaitForSingleObject(m_pSharedMemory->GetOutEventHandle(),500) == WAIT_OBJECT_0 )
      //         m_bOnGrab = TRUE;
      //     }
    }


  }


  int CImageView::PreSetting()
  {
    /********************************************************************
    created:	2005/11/09
    class:	CImageView
    author:	Michael Son	

    purpose:	Camera initializations
    *********************************************************************/
    return -1 ;
  }

  int CImageView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
  {
    /************************************************************************
    Edited by Alexandra Bernstein to allow both Solios and Meteor Image controls using the same application
    Attempt initialization of Solios, and if it does not work, attempt initializing Meteor
    8.10.2009
    Edited by Alexandra Bernstein - clear all MIL functionality from ImCNTL to allow usage of graphs architecture.
    Part of the new ATJ development (Eilat) 
    21.2.2010
    ************************************************************************/ 

    CMainFrame::OnCreate(lpCreateStruct);

    m_bPaintOK = TRUE;
    //	m_lpDibview = NULL;
    m_pUserBuff = NULL;
    m_bOnGrab = FALSE;
    CRegistry Reg( "File Company\\ImCNTL_MV" ) ;
    m_bPlayBackMode = Reg.GetRegiInt( "CameraComm" , "PlaybackMode" , 0 ) ;
    m_iGraphMode = Reg.GetRegiInt( "CameraComm" , "GraphMode" , 0 ) ;
    CString GraphName;
    switch ( m_iGraphMode )
    {
    case 0:
    case 1:
      GraphName = Reg.GetRegiString( "CameraComm" , "GraphName" , "c:\\ATJ\\ATJ_WRadDistr.tvg" ) ;
      break ;
    case 2:
      GraphName = Reg.GetRegiString( "CameraComm" , "GraphNameSolios" , 
        "c:\\ATJ\\ATJ_WithSolios.tvg" ) ;
      break ;
    case 3:
      {
        //if (!m_bFlippedImg)  
        GraphName = Reg.GetRegiString( "CameraComm" , "GraphSolios2Chan" , 
          "c:\\ATJ\\ATJ_WRadDistrAndSolios2.tvg" ) ;
      }
      break ;
    case 4:
      {
        //if (!m_bFlippedImg)  
        GraphName = Reg.GetRegiString( "CameraComm" , "GraphMatrixVision" , 
          "c:\\ATJ\\TVG\\ATJ_MatVisCamera.tvg" ) ;
      }
      break ;
    }
    m_wndToolBar.SetButtonInfo(26, ID_SHOW_RADIAL_DISTRIBUTION, TBBS_CHECKBOX , 27);

    if (LoadGraph(GraphName))
    {
      SetExposure( 12000 , true);
    }
    return 0;
  }


  void CImageView::StopGrab()
  {
    if ( m_pBuilder )
      m_pBuilder->Stop() ;
    if (m_bOnGrab)
      m_bOnGrab = FALSE;
  }

  UINT CImageView::CommandFunc(LPVOID P)
  {
    /********************************************************************
    created:	2005/11/09
    class: CImageView
    author:	Michael Son

    purpose: Thread function for management of the application via shared memory
    *********************************************************************/

    CImageView * pHost = (CImageView*)P ;
    PowerImagingExch * pExch = pHost->m_pCommon ;
    if( WaitForSingleObject(pHost->m_pSharedMemory->
      GetInEventHandle(),INFINITE) == WAIT_OBJECT_0  )
    {
      pHost->m_pSharedMemory->ResetInEvent();
      if(pExch->iDoSingleGrab )//&& pHost->m_iFindBadPix)
      {
        pHost->SingleGrab();
      }
      if(pExch->iDoContinueGrab && !pHost->m_LiveVideo )//&& pHost->m_iFindBadPix) 
      {	
        // 					  pHost->m_pSharedMemory->SetOutEvent();
        pHost->OnLiveVideo();
      }
      if(pExch->iDoStopGrab)// && pHost->m_iFindBadPix)
      {
        if(pHost->m_LiveVideo )
          pHost->OnLiveVideo();

        pHost->m_SingleCont = 0;
      }
      if(pExch->iDoMeasBlob)
      {
        //       pHost->OnMeasureBlob();
        pExch->iDoMeasBlob = 0;
      }
      if(pExch->iDoMeasLine)
      {
        //       pHost->OnMeasureLine();
        pExch->iDoMeasLine = 0;
      }
      if(pExch->iDoAddExp)
      {
        if(!pHost->OnExposurePlus())
          pExch->iDoAddExp = 0 ;
        CRect r(3,0,pHost->m_CaptureSize.cx - 3,pHost->m_CaptureSize.cy);
        //       pHost->FindMinMaxIntens(r,1);
        pExch->iLastMaxIntens = pHost->m_iLastMaxIntens;
        pExch->iLastMinIntens = pHost->m_iLastMinIntens;
        IApp()->LogMessageGuest(MyMsg,3);
        pExch->iDoAddExp= 0;
      }
      if(pExch->iDoSubExp )
      {
        if(pHost->OnExposureMinus())
          pExch->iDoSubExp = 0;
        CRect r(3,0,pHost->m_CaptureSize.cx - 3,pHost->m_CaptureSize.cy)	;
        //       pHost->FindMinMaxIntens(r,1) ;
        pExch->iLastMaxIntens = pHost->m_iLastMaxIntens;
        pExch->iLastMinIntens = pHost->m_iLastMinIntens;
        IApp()->LogMessageGuest(MyMsg,3);
        pExch->iDoSubExp = 0;
      }
      if(pExch->iDoAddGain)
      {
        pHost->OnGainPlus();
        IApp()->LogMessageGuest(MyMsg,3);
        pExch->iDoAddGain= 0;
      }
      if(pExch->iDoSubGain )
      {
        pHost->OnGainMinus();
        IApp()->LogMessageGuest(MyMsg,3);
        pExch->iDoSubExp = 0;
      }
      if(pExch->iping)

        pExch->iping = 0;
      if(pExch->iDoQuit)
      {
        IApp()->m_iViewExist = 0;
        AfxGetApp()->GetMainWnd()->PostMessage( WM_QUIT ) ;
        //       pHost->m_hPrThread->Close();
      }	
    }

    return 0;
  }


  void CImageView::OnUpdateGrabButton(CCmdUI* pCmdUI) 
  {
    // TODO: Add your command update UI handler code here
    pCmdUI->Enable( (m_LiveVideo) ? m_bOnGrab : ! m_bOnGrab );
  }



  void CImageView::OnFILELoadCamCfg() 
  {
    static char szFilter[] = "Graph file(*.tvg)|*.tvg||";

    CFileDialog fileDialog(
      TRUE,                   // TRUE for open, FALSE for save as
      NULL,                   // def ext
      NULL,                   // file name
      OFN_PATHMUSTEXIST,
      szFilter,
      NULL                    // parent window
      );

    if (fileDialog.DoModal() == IDOK)
    {
      // extract path without the file name
      CString fileName = fileDialog.GetPathName();
      LoadGraph( fileName );
    }	

  }



  void CImageView::OnBUTTON1024x1024() 
  {
    CString Control( "Task(2);") ;
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      CTextFrame * p = CTextFrame::Create( Control ) ;
      if ( !m_pBuilder->SendDataFrame( p , m_MeasSets[i].m_ProcessControlName ) )
        p->Release( p ) ;
    }
    //   if(!m_iImgMode)
    //   {
    //     m_iImgMode = 1;
    //     m_NProfilePoints = 0;
    //     m_wndToolBar.SetButtonInfo(19, ID_BUTTON1024x1024, TBBS_BUTTON, 15);
    //     Invalidate();
    //   }
    //   else
    //   {
    //     m_iImgMode = 0;
    //     m_NProfilePoints = 0;
    //     m_wndToolBar.SetButtonInfo(19, ID_BUTTON512x512, TBBS_BUTTON, 24);
    //     Invalidate();
    //   }
  }

  void CImageView::OnBUTTON512x512() 
  {

    //   if(m_iImgMode)
    //   {
    //     m_iImgMode = 0;
    //     m_NProfilePoints = 0;
    //     m_wndToolBar.SetButtonInfo(19, ID_BUTTON512x512, TBBS_BUTTON, 24);
    //     Invalidate();
    //   }
    //   else
    //   {
    //     m_iImgMode = 1;
    //     m_NProfilePoints = 0;
    //     m_wndToolBar.SetButtonInfo(19, ID_BUTTON1024x1024, TBBS_BUTTON,15);
    //     Invalidate();
    //   }
  }


  void CImageView::OnAppExit() 
  {
    IApp()->m_iViewExist = 0;
    OnDestroy();
    CWnd::OnClose();
  }


  void CImageView::RButDlgReaction()
  { 
    /********************************************************************
    created:	2005/11/09
    class: CImageView
    author:	Moisey Bernstein
    modified by Michael Son	 

    purpose: showing measured parameters  after grabbing of every frame 
    by chose from Right button dialog
    *********************************************************************/

    if(m_RButtonDlg.m_iViewMode && m_pCommon->iMeasMode && 
      (m_RButtonDlg.m_iViewMode != m_pCommon->iMeasMode))
    {
      m_RButtonDlg.m_iViewMode = m_pCommon->iMeasMode ;
    }

    int index = m_pCommon->iNLastFoundBlobs - 1;


    double dScaleY,dScaleX;
    if(m_iCam == 2)
    {
      dScaleY = m_RButtonDlg.m_dScaleYCam2;
      dScaleX = m_RButtonDlg.m_dScaleXCam2;
    }
    else
    {
      dScaleY = m_RButtonDlg.m_dScaleYCam1;
      dScaleX = m_RButtonDlg.m_dScaleXCam1;
    }

    dScaleX = abs(dScaleX);
    dScaleY = abs(dScaleY);

    switch ( m_RButtonDlg.m_iViewMode )
    {
    case 1: // one line measurement
      {
        OnMeasureLine() ;
        CString Msg ;

        //       Msg.Format( "Y=%6.2f %s" , m_dLinePos[ 0 ] 
        //       * ((m_RButtonDlg.m_bShowMicrons) ? 
        //         (1000./m_RButtonDlg.m_dScaleY) : 1.) ,
        //         ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ) ;
        //       m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
        if (m_MeasuredLines.GetSize()==0)
        {
          Msg = "No Line" ;
          m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
          break;
        }
        else if (m_MeasuredLines.GetSize()>3)
        {
          Msg = "Multiple Lines" ;
          m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
          break;
        }

        Msg.Format( "Y=%6.2f %s" , m_MeasuredLines[0].m_Center.y 
          * ((m_RButtonDlg.m_bShowMicrons) ? 
          (1000./dScaleY) : 1.) ,
          ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ) ;
        //       Msg.Format( "Y=%6.2f %s" , m_MeasuredLines[0].m_Center.y,"pix" ); 
        m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;


      }  
      break;
    case 2: // distance between lines measurement
      {
        OnMeasureLine() ;
        CString Msg ;
        if ( m_MeasuredLines.GetSize() < 6 )
        {
          if (m_MeasuredLines.GetSize() == 3)
            Msg = "Single Line" ;
          else
            Msg = "No Lines" ;
        }    
        else
        {
          Msg.Format( "dY=%6.2f %s" , fabs( m_MeasuredLines[ 2 ].m_Center.y - m_MeasuredLines[ 3 ].m_Center.y )
            * ((m_RButtonDlg.m_bShowMicrons) ? 
            (1000./dScaleY) : 1.) ,
            ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ) ;


          double dYBetween = m_MeasuredLines[1].m_Center.y -m_MeasuredLines[0].m_Center.y;
          int y=0;
        }
        m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
      }  
      break;
    case 3: // blob measurement
      {
        OnMeasureBlob() ;
        CString Msg ;
        if ( !m_NBlobs )   
          Msg = "NO BLOBS" ;
        else
        {
          //         if ( m_RButtonDlg.m_SpotSearch.x ) // we need blob near
          //           // some coordinates
          //         {
          //           cmplx Center( m_RButtonDlg.m_SpotSearch.x , m_RButtonDlg.m_SpotSearch.y ) ;
          //           for ( int i = 0 ; i < m_NBlobs ; i++ )
          //           {
          //             if ( abs( Center - m_pCommon->Blobs[i].CenterOfGravity ) < 30 
          //               &&    m_pCommon->Blobs[i].dArea > 100 )
          //             {
          //               index = i ;
          //               break ;
          //             }
          //           }
          //         }
          //         m_iLastFrame = 0;

          //         Msg.Format( 
          //         "C( %5.1f , %5.1f ) pix;Sz(%5.1f , %5.1f) %s;R=%5.1f" ,
          //        m_LastSpot.m_SimpleCenter.x,
          //        m_LastSpot.m_SimpleCenter.y,
          //         m_LastSpot.m_dBlobWidth * ((m_RButtonDlg.m_bShowMicrons) ? 
          //         (1000./m_RButtonDlg.m_dScaleY) : 1.), 
          //         m_LastSpot.m_dBlobHeigth * ((m_RButtonDlg.m_bShowMicrons) ? 
          //         (1000./m_RButtonDlg.m_dScaleY) : 1.), 
          //         ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ,
          //         m_LastSpot.m_dAngle) ;

          if (fabs (m_MeasuredBlobs[0].m_SimpleCenter.x - cSpotX ) < 100 || m_MeasuredBlobs.GetSize() == 1)
          {
            Msg.Format( 
              "C( %5.1f , %5.1f ) pix;Sz(%5.1f , %5.1f) %s;R1=%5.1f   |   R0=%5.1f " ,
              m_MeasuredBlobs[0].m_SimpleCenter.x,
              m_MeasuredBlobs[0].m_SimpleCenter.y,
              m_MeasuredBlobs[0].m_dBlobWidth * ((m_RButtonDlg.m_bShowMicrons) ? 
              (1000./dScaleX) : 1.), 
              m_MeasuredBlobs[0].m_dBlobHeigth * ((m_RButtonDlg.m_bShowMicrons) ? 
              (1000./dScaleY) : 1.), 
              ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ,
              /* m_MeasuredBlobs[0].m_dAngle*/dAngles[1],dAngles[0]) ;

            dAngles[0] = dAngles[1]=0;

          }
          else if (m_MeasuredBlobs.GetSize() > 1) 
          {
            Msg.Format( 
              "C( %5.1f , %5.1f ) pix;Sz(%5.1f , %5.1f) %s;R=%5.1f" ,
              m_MeasuredBlobs[1].m_SimpleCenter.x,
              m_MeasuredBlobs[1].m_SimpleCenter.y,
              m_MeasuredBlobs[1].m_dBlobWidth * ((m_RButtonDlg.m_bShowMicrons) ? 
              (1000./dScaleY) : 1.), 
              m_MeasuredBlobs[1].m_dBlobHeigth * ((m_RButtonDlg.m_bShowMicrons) ? 
              (1000./dScaleY) : 1.), 
              ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ,
              m_MeasuredBlobs[1].m_dAngle) ;
          }

        }
        m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
        //       if (m_RButtonDlg.m_IsAutoSave)
        //       {
        // 
        //         FILE * fa = fopen( (LPCTSTR)m_RButtonDlg.m_AutoSaveFileName, "ab" ) ;
        // 
        //         if ( fa )
        //         {
        //           fseek( fa , 0 , SEEK_END ) ;
        //           if ( ftell( fa ) < 10 )
        //           {
        //             CTime t = CTime::GetCurrentTime();
        //             CString s = t.Format( "%A, %B %d, %Y   %H:%M:%S" );
        //             fprintf( fa , "\r\n%s" , (LPCTSTR)s ) ;
        //             fprintf( fa , "\r\nSizes in %s; rotation in degrees; order is X,Y,Sx,Sy,Rot" ,
        //               (m_RButtonDlg.m_bShowMicrons)? "microns" : "pixels" ) ;
        //           }
        //           Msg.Format( "%5.1f %5.1f %5.1f %5.1f %5.1f",
        //             m_pCommon->Blobs[index].CenterOfGravity.real() , 
        //             m_pCommon->Blobs[index].CenterOfGravity.imag() ,
        //             m_pCommon->Blobs[index].dBlobWidth * ((m_RButtonDlg.m_bShowMicrons) ? 
        //             (1000./m_RButtonDlg.m_dScaleY) : 1.), 
        //             m_pCommon->Blobs[index].dBlobHeigth * ((m_RButtonDlg.m_bShowMicrons) ? 
        //             (1000./m_RButtonDlg.m_dScaleY) : 1.), 
        //             m_pCommon->Blobs[index].dAxisPrincipalAngle ) ;
        // 
        //           fprintf( fa , "\r\n%s" , (LPCTSTR)Msg ) ;
        //           fclose( fa ) ;
        //         }
        //       }

      }  
      break;
    case 4:     // Extended measurement results presentation
      {
        CPoint MaxPos ;
        //       int iMaxVal = FindPositionOfMax( MaxPos ) ;
        //       int iIntegral = 0 ;
        int iMaxVal = 4095 ;
        int iIntegral = 0 ;
        //       if (   MaxPos.x > 50 
        //         &&   MaxPos.x < IMAGE_WIDTH - 50 
        //         &&   MaxPos.y > 50 
        //         &&   MaxPos.y < IMAGE_HEIGHT - 50 )
        //       {
        //         iIntegral = ROUND( GetSumOfArea( 
        //           MaxPos.x - 5 , MaxPos.x + 5 , MaxPos.y - 5 , MaxPos.y + 5 )) ;
        //       }
        OnMeasureBlob() ;
        CString Msg ;
        if ( m_NBlobs < 1 )
          Msg = "ERROR" ;
        else
        {
          Msg.Format( 
            "C( %5.1f , %5.1f ) pix;Sz(%5.1f , %5.1f) %s;R=%5.1f deg;Max=%4d(%4d,%4d);Sum=%8d" ,
            m_pCommon->Blobs[index].CenterOfGravity.real() , 
            m_pCommon->Blobs[index].CenterOfGravity.imag(),
            m_pCommon->Blobs[index].dBlobWidth * ((m_RButtonDlg.m_bShowMicrons) ? 
            (1000./dScaleY) : 1.), 
            m_pCommon->Blobs[index].dBlobHeigth * ((m_RButtonDlg.m_bShowMicrons) ? 
            (1000./dScaleY) : 1.), 
            ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ,
            m_pCommon->Blobs[index].dAxisPrincipalAngle ,
            iMaxVal , MaxPos.x , MaxPos.y , 
            iIntegral ) ;


          //Msg.Format( 
          //					"C( %.2f , %.2f ) pix;Sz(%5.1f , %5.1f) ;R=%5.1f" ,
          //        			m_pCommon->Blobs[index].CenterOfGravity.real(), 
          //       			m_pCommon->Blobs[index].CenterOfGravity.imag(),
          //                   m_pCommon->Blobs[index].dBlobWidth* ((m_RButtonDlg.m_bShowMicrons) ? 
          //                   (1000./m_RButtonDlg.m_dScaleY) : 1.), 
          //                   m_pCommon->Blobs[index].dBlobHeigth * ((m_RButtonDlg.m_bShowMicrons) ? 
          //                   (1000./m_RButtonDlg.m_dScaleY) : 1.), 
          //                   ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ,
          //					m_pCommon->Blobs[index].dAxisPrincipalAngle) ;

        }
        m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
        if (m_RButtonDlg.m_IsAutoSave)
        {   // auto save in this mode is full image saving
          CString FileName ;
          CTime t = CTime::GetCurrentTime();
          FileName = t.Format("%d%b%y%H%M%S.raw") ;
          FileName = m_RButtonDlg.m_AutoSaveFileName + FileName ;
          FILE * fa = fopen( (LPCTSTR)FileName , "wb" ) ;
          if ( fa )
          {
            for ( int iy = 0 ; iy < IMAGE_HEIGHT ; iy++ )
            {
              //             short int * pRow = (short int *)GetGrabRow( iy ) ;
              //             fwrite( pRow , IMAGE_WIDTH * sizeof(short int) , 1 , fa ) ;
            }
            fclose( fa ) ;
          }
        }
      }
      break ;
    case 5:     // Two Spots distance measurement (for deskew estimation)
      {
        CString Msg ;
        OnMeasureBlob() ;
        if ( m_NBlobs >= 2 )  
        {
          int index1 = -1 , index2 = -1 ;
          int i = 0 ;
          for (  ; i < m_pCommon->iNLastFoundBlobs ; i++ )
          {

            if ( m_pCommon->Blobs[i].dBlobWidth >= 10 
              &&  m_pCommon->Blobs[i].dBlobHeigth >= 10 )
            {
              index1 = i ;
              break ;
            } ;
          }
          for ( i++ ; i <  m_pCommon->iNLastFoundBlobs; i++ )
          {

            if (    m_pCommon->Blobs[i].dBlobWidth >= 10 
              &&  m_pCommon->Blobs[i].dBlobHeigth >= 10 )
            {
              index2 = i ;
              break ;
            } ;
          }

          if ( m_MeasuredBlobs.GetSize() < 2 )
          {
            m_BigViewWnd.SetOutputText( "Don't see two big enough blobs." ) ;
            m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
          }
          else
          {

            double dYBetween = m_MeasuredBlobs[1].m_SimpleCenter.y -m_MeasuredBlobs[0].m_SimpleCenter.y;
            double dXBetween = m_MeasuredBlobs[1].m_SimpleCenter.x - m_MeasuredBlobs[0].m_SimpleCenter.x ;

            
            Msg.Format( "dX=%7.2f %s;dY=%7.2f %s" ,
              dXBetween * ((m_RButtonDlg.m_bShowMicrons) ? 
              (1000./dScaleX) : 1.), 
              ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ,
              dYBetween * ((m_RButtonDlg.m_bShowMicrons) ? 
              (1000./dScaleY) : 1.), 
              ((m_RButtonDlg.m_bShowMicrons) ? "um" : "pix" ) ) ;
            
          }
        }
        else
          m_BigViewWnd.SetOutputText( 
          "Don't see two blobs." ) ;
        m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
      }
      break ;
    case 6: // full frame statistics
      {
        CString Msg ;
        //       double dAve , dStd ;
        //       int iMin , iMax ;
        CRect rc( 2 , 2 , m_GrabWidth - 2 , m_GrabHeight - 2 );
        //       FindFrameStatistics( rc , dAve , dStd , iMin , iMax ) ;
        //       Msg.Format( "Ave=%6.1f Std=%6.1f; Min=%4d Max=%4d" ,
        //         dAve , dStd , iMin , iMax ) ;
        m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
      }
      break;
    case 7:     // Two Spots distance measurement (for linear deskew live)
      {
        CString Msg ;
        OnMeasureBlob() ;
        if ( m_NBlobs >= 2 )  
        {
          int index1 = -1 , index2 = -1 ;
          int i = 0 ;
          for (  ; i < m_pCommon->iNLastFoundBlobs ; i++ )
          {

            if ( m_pCommon->Blobs[i].dBlobWidth >= 10 
              &&  m_pCommon->Blobs[i].dBlobHeigth >= 10 )
            {
              index1 = i ;
              break ;
            } ;
          }
          for ( i++ ; i <  m_pCommon->iNLastFoundBlobs; i++ )
          {

            if (    m_pCommon->Blobs[i].dBlobWidth >= 10 
              &&  m_pCommon->Blobs[i].dBlobHeigth >= 10 )
            {
              index2 = i ;
              break ;
            } ;
          }

          if ( m_MeasuredBlobs.GetSize() < 2 )
          {
            m_BigViewWnd.SetOutputText( "Don't see two big enough blobs." ) ;
            m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
          }
          else
          {

            double dYBetween = m_MeasuredBlobs[1].m_SimpleCenter.y -m_MeasuredBlobs[0].m_SimpleCenter.y;
            double dXBetween = m_MeasuredBlobs[1].m_SimpleCenter.x - m_MeasuredBlobs[0].m_SimpleCenter.x ;


            CRegistry Reg( "File Company\\OpticJig" ) ;
            double dXForLiveDeskew = dXBetween * ((m_RButtonDlg.m_bShowMicrons) ? 
              (1000./dScaleX) : 1.);
            double dYForLiveDeskew = dYBetween * ((m_RButtonDlg.m_bShowMicrons) ? 
              (1000./dScaleY) : 1.);

            int bHDLA = Reg.GetRegiInt( "HeadControl" , "HDLA" , 0 ) ;
            int nLasersForCalculation = bHDLA ? 39 : 27;
            int iFirstLaser =  Reg.GetRegiInt( "HeadControl" , "FirstLaserNumForDeskewLive" , 21 ) ;
            int iLastLaser =  Reg.GetRegiInt( "HeadControl" , "LastLaserNumForDeskewLive" , 31 ) ;           
            double dNominalLasersDist = atof(Reg.GetRegiString("DeskewPar", "LaserDist_um", "31.25" ));
            int iLinearDeskew = ROUND((dXForLiveDeskew)/(iLastLaser - iFirstLaser)*nLasersForCalculation );         

            Msg.Format( "LinDeskew = %d " ,  iLinearDeskew );
                    
          }
        }
        else
          m_BigViewWnd.SetOutputText( 
          "Don't see two blobs." ) ;
         m_BigViewWnd.SetOutputText( (LPCTSTR)Msg ) ;
      }
      break ;
    }

  }


  void CImageView::Measure()
  {
    // This func was left as it is since it only shows result of external measurements
    int index = m_pCommon->iNLastFoundBlobs - 1;
    //			 if( m_RButtonDlg.m_iViewMode)
    //				 m_RButtonDlg.m_iViewMode = m_pCommon->iMeasMode;
    switch ( m_pCommon->iMeasMode )
    {
    case 1: // one line measurement
      {
        OnMeasureLine() ;

        CString Msg ;
        Msg.Format( "Y=%6.2f um" , m_dLinePos[ 0 ]	);

        strcpy(MyMsg, (LPCSTR)Msg) ;
        IApp()->LogMessageGuest((LPCSTR)MyMsg,1 ) ;

      }  
      break;
    case 2: // distance between lines measurement
      {
        OnMeasureLine() ;
        CString Msg ;
        if ( m_NLines < 2 )
        {
          Msg = "ERROR" ;
          //m_pCommon->iMeasMode  = 0;
        }
        else
        {
          Msg.Format( "dY=%6.2f um" , fabs( m_dLinePos[ 0 ] - m_dLinePos[ 1 ] )	);
          strcpy(MyMsg, (LPCSTR)Msg) ;
          IApp()->LogMessageGuest((LPCSTR)MyMsg,1 ) ;

        }
      }  
      break;
    case 3: // blob measurement
      {
        m_iLastFrame = 0;
        OnMeasureBlob() ;
        if(m_NBlobs || !m_iNotBlob)
        {
          CString Msg ;

          if ( !m_NBlobs )
          {
            Msg = "ERROR" ;
          }
          else
          {
            Msg.Format( 
              "C(%.2f,%.2f)pix;Sz(%5.1f,%5.1f)um;R=%5.1f;Max=%d dS=%g" ,
              m_pCommon->Blobs[index].CenterOfGravity.real(), 
              m_pCommon->Blobs[index].CenterOfGravity.imag(),
              m_pCommon->Blobs[index].dBlobWidth, 
              m_pCommon->Blobs[index].dBlobHeigth, 
              m_pCommon->Blobs[index].dAxisPrincipalAngle ,
              (int) m_pCommon->Blobs[index].dMaxIntens ,
              m_pCommon->Blobs[index].dSumPixels ) ;
            if ( m_ImParam.m_MeasBeamRotBySectors )
            {
              CString Addition ;
              Addition.Format( "(%6.1f)" , m_dMaxDirBySectors ) ;
              Msg += Addition ;
            }
            Msg += " deg" ;
          }
          strcpy(MyMsg, (LPCSTR)Msg) ;
          IApp()->LogMessageGuest((LPCSTR)MyMsg,1 ) ;
        }

      }  
      break;
    case 4:     // Extended measurement results presentation
      {
        CPoint MaxPos ;
        //       int iMaxVal = FindPositionOfMax( MaxPos ) ;
        //       int iIntegral = 0 ;
        int iMaxVal = 4095 ;
        int iIntegral = 0 ;
        //       if (   MaxPos.x > 50 
        //         &&   MaxPos.x < IMAGE_WIDTH - 50 
        //         &&   MaxPos.y > 50 
        //         &&   MaxPos.y < IMAGE_HEIGHT - 50 )
        //       {
        //         iIntegral = ROUND( GetSumOfArea( 
        //           MaxPos.x - 5 , MaxPos.x + 5 , MaxPos.y - 5 , MaxPos.y + 5 )) ;
        //       }
        OnMeasureBlob() ;
        CString Msg ;
        m_pCommon->iMaxVal = iMaxVal;
        m_pCommon->MaxPos = MaxPos;
        m_pCommon->iIntegral = iIntegral;
        if ( m_NBlobs < 1 )
        {
          Msg = "ERROR" ;
          //	m_pCommon->iMeasMode  = 0;
        }

        else
        {
          Msg.Format("C(%.2f,%.2f)pix;Sz(%5.1f,%5.1f)pix;R=%5.1f" ,
            m_pCommon->Blobs[index].CenterOfGravity.real(), 
            m_pCommon->Blobs[index].CenterOfGravity.imag(),
            m_pCommon->Blobs[index].dBlobWidth, 
            m_pCommon->Blobs[index].dBlobHeigth, 
            m_pCommon->Blobs[index].dAxisPrincipalAngle) ;

        }
        strcpy(MyMsg, (LPCSTR)Msg) ;
        IApp()->LogMessageGuest((LPCSTR)MyMsg,1 ) ;

      }
      break ;
    case 5:     // Two Spots distance measurement (for deskew estimation)
      {
        CString Msg ;
        OnMeasureBlob() ;
        if ( m_NBlobs >= 2 )  
        {
          int index1 = -1 , index2 = -1 ;
          int i = 0 ;
          for (  ; i < m_pCommon->iNLastFoundBlobs ; i++ )
          {

            if ( m_pCommon->Blobs[i].dBlobWidth >= 10 
              &&  m_pCommon->Blobs[i].dBlobHeigth >= 10 )
            {
              index1 = i ;
              break ;
            } ;
          }
          for ( i++ ; i <  m_pCommon->iNLastFoundBlobs; i++ )
          {

            if (    m_pCommon->Blobs[i].dBlobWidth >= 10 
              &&  m_pCommon->Blobs[i].dBlobHeigth>= 10 )
            {
              index2 = i ;
              break ;
            } ;
          }

          if ( index1 < 0   ||  index2 < 0 )
          {
            m_BigViewWnd.SetOutputText( "Don't see two big enough blobs." ) ;
          }
          else
          {
            double dYBetween = m_pCommon->Blobs[index1].CenterOfGravity.imag()
              - m_pCommon->Blobs[index2].CenterOfGravity.imag();
            double dXBetween = m_pCommon->Blobs[index1].CenterOfGravity.real()
              - m_pCommon->Blobs[index2]. CenterOfGravity.real() ;
            Msg.Format( "dX=%7.2f um ;dY=%7.2f um" ,dXBetween ,dYBetween );

          }

        }
        else
        {
          Msg = "Don't see two blobs."  ;
          //m_pCommon->iMeasMode  = 0;
        }

        strcpy(MyMsg, (LPCSTR)Msg) ;
        IApp()->LogMessageGuest((LPCSTR)MyMsg,1 ) ;

      }
      break ;
    }
  }

  void CImageView::MakeBinary()
  {
    //   CRect r(5,5,m_CaptureSize.cx - 5 ,m_CaptureSize.cy - 5)	;
    //   m_byThres = FindMinMaxIntens(r,1);
    //   for (int i_UsBuff = 0; i_UsBuff < (int)m_BuffSize/2 ; i_UsBuff ++)
    //   {
    //     if(m_pUserBuff[i_UsBuff] < m_byThres)
    //       m_pUserBuff[i_UsBuff]	= 0;
    //     else
    //       m_pUserBuff[i_UsBuff]	= 4095;
    //   }
    //Invalidate();
  }

  void CImageView::MakeContrast()
  {
    //   CRect r(5,5,m_CaptureSize.cx - 5 ,m_CaptureSize.cy - 5)	;
    //   FindMinMaxIntens(r,1);
    //   if(!m_iLastMaxIntens)
    //     return;
    //   int k = (int)(4095 / m_iLastMaxIntens );
    //   int iTable[4096];
    //   // Creating Look Up Table
    //   for(int i = 0; i < 4096; i++)
    //   {
    //     iTable[i] = i * k;
    //   }
    // 
    //   //Making contrast buffer
    //   for (int i_UsBuff = 0; i_UsBuff < (int)m_BuffSize/2 ; i_UsBuff ++)
    //   {
    //     if(m_pUserBuff[i_UsBuff] > 4095)
    //       m_pUserBuff[i_UsBuff] = 4095;
    //     m_pUserBuff[i_UsBuff] = iTable[m_pUserBuff[i_UsBuff]];
    //   }
    // Invalidate();

  }

  void CImageView::OnClose()
  {
    if ( m_pBuilder )
      m_pBuilder->Stop() ;
    Sleep(100) ;
    WINDOWPLACEMENT PLacement;;
    GetWindowPlacement( &PLacement );
    CRegistry Reg("File Company\\OpticJig");
    CString PosOut;
    PosOut.Format("%d,%d,%d,%d",
      PLacement.rcNormalPosition.left,
      PLacement.rcNormalPosition.top,
      PLacement.rcNormalPosition.right - PLacement.rcNormalPosition.left,
      PLacement.rcNormalPosition.bottom - PLacement.rcNormalPosition.top);
    Reg.WriteRegiString("Positions", "ImageDlgPos" , PosOut);

    m_RButtonDlg.m_iViewMode = 0 ;
    m_BigViewWnd.ShowWindow( SW_HIDE ) ;

    Clean() ;
    Sleep(100);
    //  MdigHalt( m_MDig ) ;
    //  Sleep(300) ;
    CMainFrame::OnClose();
  }

  LRESULT CImageView::OnCaptureGadgetMsg(WPARAM w_dummy, LPARAM l_param)
  {
    CMeasurementSet * pSet = (CMeasurementSet*) l_param ;
    if ( pSet->m_ReceivedFromCap == "OK" )
    {
      pSet->m_CaptureProperties.Empty() ;
      FXString Tmp ;
      pSet->m_pCaptureGadget->PrintProperties( Tmp ) ;
      pSet->m_CaptureProperties = Tmp ;
    }
    return LRESULT();
  }

  LRESULT CImageView::OnImProcResultMsg(WPARAM w_dummy, LPARAM l_param)
  {
    LogMessage( (const char *) l_param ) ;
    *((char*)l_param) = 0 ;
    return LRESULT(1);
  }

  LRESULT CImageView::OnSharedMemoryMsg(WPARAM iOperCode, LPARAM l_param)
  {
    switch ( iOperCode )
    {
    case SETSYNCMODE: 
      {
        m_bAsyncGrab = (l_param != 0) ;
        m_wndToolBar.SetButtonInfo(
          11, ID_ASYNCHRONE, TBBS_BUTTON, (m_bAsyncGrab==0)? 8 : 20) ;
        //OnAsynchroneMode() ;
      }
      break ;
    case GRAB:
      {
        if ( m_LiveVideo )
          OnLiveVideo() ;
      }
    }
    return LRESULT(1);
  }

  LRESULT CImageView::OnLogMsg(WPARAM iPanIndex, LPARAM pStringPointer)
  {
    if ( pStringPointer )
    {
      char * pMsg = (char*) pStringPointer ;
      switch ( iPanIndex )
      {
      case 0: SetWindowText(pMsg);  break;
      case 1: LogMessage( pMsg ) ; break ;
      case 2: LogMessage2( pMsg ) ; break ;
      case 3: LogMessage3( pMsg ) ; break ;
      default: break ;
      }
      delete pMsg;
    }
    return LRESULT(1);
  }

  void CImageView::OnLButtonDblClk(UINT nFlags, CPoint point)
  {
    if ( nFlags & MK_CONTROL )
    {
      if ( m_RButtonDlg.m_iViewMode )
      {
        m_RButtonDlg.m_iViewMode = 0 ;
        m_BigViewWnd.ShowWindow( SW_HIDE ) ;
        m_RButtonDlg.m_SpotSearchCoordinates.Empty() ;
        m_RButtonDlg.m_SpotSearch = CPoint(0,0) ;
      }
      else
      {
        m_RButtonDlg.m_iViewMode = 3 ;
        point.y -= 26 ;
        if ( m_iImgMode == 0 ) // 512x512
        {
          point.x *= 2 ;
          point.y *= 2 ;
        }
        m_RButtonDlg.m_SpotSearch = point ;
        m_RButtonDlg.m_SpotSearchCoordinates.Format( 
          "%d %d" , point.x , point.y ) ;
        m_BigViewWnd.ShowWindow( SW_SHOW ) ;
      }
    }

    CMainFrame::OnLButtonDblClk(nFlags, point);
  }

  void CImageView::OnSize(UINT nType, int cx, int cy)
  {
    CMainFrame::OnSize(nType, cx, cy);
    CRect Client ;
    GetClientRect( &Client ) ;
    Client.top += 24 ;
    Client.bottom -= 18 ;
    int iNx = 1 +  (m_MeasSets.GetCount() > 1) ;
    int iNy = 1 +  (m_MeasSets.GetCount() > 2) ;
    int iLx = Client.Width()/iNx ;
    int iLy = Client.Height()/iNy ;
    for ( int i = 0 ; i < m_MeasSets.GetCount(); i++)
    {
      CPoint LT( ((i) & 1) * iLx , (((i) & 2) >> 1) * iLy + 24 ) ;
      CSize Sz( iLx , iLy ) ;
      m_MeasSets[i].m_pChildWindow->SetWindowPos( 0 , 
        LT.x , LT.y , iLx  , iLy , SWP_NOZORDER ) ;
      m_MeasSets[i].m_pRenderWnd->SetWindowPos( 0 , 
        0 , 0 , iLx  , iLy , SWP_NOZORDER ) ;
    }
  }

  int CImageView::OnExposureMinus() 
  {
    /********************************************************************
    created:	26.2.2010
    class: CImageView (moved from CMainFrame)
    author:	Moisey Bernstein

    purpose: exposure time decreasing by changing values in camera
    // Old content wiped out for Eilat ATJ 26.2.2010 by Moisey

    *********************************************************************/
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      FXPropertyKit pk ;
      m_MeasSets[i].m_pCaptureGadget->PrintProperties( pk ) ;
      int iExp ;
      if ( pk.GetInt( "Shutt_uS" , iExp ) )
      {
        DWORD dwState = GetAsyncKeyState( VK_SHIFT ) ;
        double dFactor = ( dwState & 0x8000 ) ? 0.1 : 0.01 ;

        int iNewExposure = ROUND( iExp * (1. - dFactor) ) ;
        if ( iNewExposure == iExp )
          iNewExposure = iExp - 1 ;
        SetExposure( iNewExposure , true ) ;

        //     if (m_MeasSets[i].m_GraphMode==2)
        //     {
        //        CString GadgetName;
        //      
        //        CCaptureGadget * pCaptureGadget = NULL ;
        //        GadgetName.Format("SoliosDalsa%d",i+1);
        //        pCaptureGadget = (CCaptureGadget *) m_pBuilder->GetGadget( (LPCTSTR) GadgetName ) ;
        //        if (pCaptureGadget)
        //        {
        //          pCaptureGadget->PrintProperties( pk ) ;
        //          CString sOut;
        //          sOut.Format("%d",iNewExposure);
        //          pk.WriteString("Shutt_uS",sOut);
        //          bool Invalidate = true ;
        //          pCaptureGadget->ScanProperties( pk,Invalidate ) ;
        //        }
        //     }

        if (m_MeasSets[i].m_GraphMode==1)
        {
          CString sExpo;
          iNewExposure = int (iNewExposure / m_dScanTime);
          sExpo.Format("%d",iNewExposure);
          sExpo = _T("SETEXPOSURE__")+ sExpo;
          m_MeasSets[i].SetTextThroughLAN(sExpo);
        }
      }
    }
    return 1 ;
  }

  int CImageView::OnExposurePlus() 
  {
    /********************************************************************
    created:	26.2.2010
    class: CImageView (moved from CMainFrame)
    author:	Moisey Bernstein

    purpose: exposure time increasing by changing values in camera   

    // Old content wiped out for Eilat ATJ 26.2.2010 by Moisey
    *********************************************************************/
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      FXPropertyKit pk ;
      m_MeasSets[i].m_pCaptureGadget->PrintProperties( pk ) ;
      int iExp ;
      if ( pk.GetInt( "Shutt_uS" , iExp ) )
      {
        DWORD dwState = GetAsyncKeyState( VK_SHIFT ) ;
        double dFactor = ( dwState & 0x8000 ) ? 0.1 : 0.01 ;

        int iNewExposure = ROUND( iExp * (1. + dFactor) ) ;
        if ( iNewExposure == iExp )
          iNewExposure = iExp + 1 ;
        SetExposure( iNewExposure , true ) ;

        //     if (m_MeasSets[i].m_GraphMode==2)
        //     {
        //       CString GadgetName;
        // 
        //       CCaptureGadget * pCaptureGadget = NULL ;
        //       GadgetName.Format("SoliosDalsa%d",i+1);
        //       pCaptureGadget = (CCaptureGadget *) m_pBuilder->GetGadget( (LPCTSTR) GadgetName ) ;
        //       if (pCaptureGadget)
        //       {
        //         pCaptureGadget->PrintProperties( pk ) ;
        //         CString sOut;
        //         sOut.Format("%d",iNewExposure);
        //         pk.WriteString("Shutt_uS",sOut);
        //         bool Invalidate = true ;
        //         pCaptureGadget->ScanProperties( pk,Invalidate ) ;
        //       }
        //     }

        if (m_MeasSets[i].m_GraphMode==1)
        {
          CString sExpo;
          iNewExposure=int(iNewExposure / m_dScanTime);
          sExpo.Format("%d",iNewExposure);
          sExpo = _T("SETEXPOSURE__")+ sExpo;
          m_MeasSets[i].SetTextThroughLAN(sExpo);
        }
      }
    }
    return 1 ;
  }
  int CImageView::OnGainMinus() 
  {
    /********************************************************************
    created:	28.06.2011
    class: CImageView 
    author:	Moisey Bernstein

    purpose: Gain decreasing by changing values in camera

    *********************************************************************/
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      FXPropertyKit pk ;
      m_MeasSets[i].m_pCaptureGadget->PrintProperties( pk ) ;
      int iGain = 0 ;
      pk.GetInt( "Gain_dBx10" , iGain ) ;
      DWORD dwState = GetAsyncKeyState( VK_SHIFT ) ;
      int iChange = ( dwState & 0x8000 ) ? 10 : 1 ;

      iGain -= iChange ;
      if ( iGain < m_iMinGain )
        iGain = m_iMinGain ;
      SetGain( iGain , i ) ;
    }
    return 1 ;
  }

  int CImageView::OnGainPlus() 
  {
    /********************************************************************
    created:	28.06.2011
    class: CImageView 
    author:	Moisey Bernstein

    purpose: Gain increasing by changing values in camera

    *********************************************************************/
    for ( int i = 0 ; i <= m_MeasSets.GetUpperBound() ; i++ )
    {
      FXPropertyKit pk ;
      m_MeasSets[i].m_pCaptureGadget->PrintProperties( pk ) ;
      int iGain = 0 ;
      pk.GetInt( "Gain_dBx10" , iGain ) ;
      DWORD dwState = GetAsyncKeyState( VK_SHIFT ) ;
      int iChange = ( dwState & 0x8000 ) ? 10 : 1 ;

      iGain += iChange ;
      if ( iGain > m_iMaxGain )
        iGain = m_iMaxGain ;
      SetGain( iGain , i ) ;
    }
    return 1 ;
  }
  int CImageView::SetGain( int iGain , int iCamNum )
  {
    CString Control ;
    CString Status ;
    int iFirst = (iCamNum == -1) ? 0 : iCamNum ;
    int iLast  = (iCamNum == -1) ? m_MeasSets.GetUpperBound() : iCamNum ;
    int iRes = 0 ; 
    for ( int i = iFirst ; i <= iLast ; i++ )
      iRes += ( m_MeasSets[i].SetGain( iGain ) == OPERATION_ERROR ) ;
    return (iRes == 0) ;
  }

  int CImageView::SetExposure( 
    int iNewExposure , bool bInMicroSeconds , int iCamNumber ) 
  {
    /********************************************************************
    created:	26.2.2010
    class: CImageView (moved from CMainFrame)
    author:	Moisey Bernstein

    purpose: exposure time increasing by changing values in camera   

    // Old content wiped out for Eilat ATJ 26.2.2010 by Moisey
    *********************************************************************/
    if ( ! bInMicroSeconds )
    {
      double dNewExposure = iNewExposure * m_dScanTime ;
      iNewExposure = ROUND(dNewExposure) ;
    }
    int iSetExposure = iNewExposure ;
    if ( iNewExposure > m_iMaxExposure_us )
      iNewExposure = m_iMaxExposure_us ;
    if ( iNewExposure < m_iMinExposure_us )
      iNewExposure = m_iMinExposure_us ;
    if ( iSetExposure != iNewExposure )
    {
      CString Msg ;
      Msg.Format( "Cam%d Exposure uS out of range [%d,%d]: requested %d Settled %d" , 
        iCamNumber , m_iMinExposure_us , m_iMaxExposure_us , iSetExposure , iNewExposure ) ;
      LogMessage( Msg ) ;
    }
    int iRes = 0 ;
    FXString Tmp ;
    FXPropertyKit pk( Tmp ) ;
    m_MeasSets[0].m_CaptureProperties = Tmp ;
    CString Control ;
    Control.Format( "set Shutt_uS(%d)" , iNewExposure ) ;
    int iFirst = ( iCamNumber < 0 ) ? 0 : iCamNumber ;
    int iLast = ( iCamNumber < 0 ) ? m_MeasSets.GetUpperBound() : iCamNumber ;
    for ( int i = iFirst ; i <= iLast ; i++)
      iRes += ( m_MeasSets[i].SetExposure( iNewExposure , 1 ) == OPERATION_ERROR ) ;
    return iNewExposure ;
  }

  int CImageView::ProcessReceivedDataFrame( 
    CString Data , CString Label , CMeasurementSet * pSet , bool bFinish )
  {

    if ( !bFinish )
    {
      if ( Label.Left(5) != "Data_" )
        return 0 ;
      if ( Label.Mid(5,4) == "Spot" )
      {
        m_iNSpotPackets++ ;
        int iNItems = 0 ;
        pSet->m_SpotResults.Lock() ;
        int iPos = Data.Find( '\n' ) ;
        if ( iPos > 0   )
        {
          CString Statistics = Data.Left( iPos ) ;
          int iNSpotPos = Statistics.Find( "Spots=" ) ;
          int iMaxPos = Statistics.Find( "Max=" ) ;
          int iMinPos = Statistics.Find( "Min=" ) ;
          CHAR * p = Statistics.GetBuffer() ;
          pSet->m_iLastNBlobs = ( iNSpotPos >= 0 ) ? 
            atoi( p + iNSpotPos + 6 ) : 0 ;
          pSet->m_iLastMaxIntensity = (iMaxPos >= 0) ?
            ROUND(atof( p + iMaxPos + 4 )) : 0 ;
          pSet->m_iLastMinIntensity = ( iMinPos >= 0 ) ?
            ROUND( atof( p + iMinPos + 4 ) ) : 0  ;

          if ( (iNSpotPos >= 0)  || (iMaxPos >= 0)  || (iMinPos >= 0) )
            Data = Data.Mid( iPos + 1 ) ; // remove statistics string
        }
        do 
        {
          if ( pSet->m_iLastNBlobs == 0 )
            break ;
          CColorSpot Spot ;
          int iIndex ;
          int iWidthFOV = 0 , iHeightFOV = 0 ;
          iNItems = sscanf( (LPCTSTR)Data ,
            "%d %lf %lf %lf %lf %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf \
            %d %d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf" ,
            &iIndex ,
            &Spot.m_SimpleCenter.x ,
            &Spot.m_SimpleCenter.y ,
            &Spot.m_dBlobWidth ,
            &Spot.m_dBlobHeigth ,
            &Spot.m_Area ,
            &Spot.m_iMaxPixel,
            &Spot.m_dCentral5x5Sum ,
            &Spot.m_dSumOverThreshold ,
            &Spot.m_dAngle   ,
            &Spot.m_dLongDiametr,
            &Spot.m_dShortDiametr ,
            &Spot.m_dRDiffraction ,
            &Spot.m_dLDiffraction ,
            &Spot.m_dDDiffraction ,
            &Spot.m_dUDiffraction ,
            &Spot.m_dCentralIntegral ,
            &Spot.m_OuterFrame.left ,
            &Spot.m_OuterFrame.top ,
            &Spot.m_OuterFrame.right ,
            &Spot.m_OuterFrame.bottom ,
            &iWidthFOV , &iHeightFOV,
            &Spot.m_dIntegrals[0],&Spot.m_dIntegrals[1],&Spot.m_dIntegrals[2],
            &Spot.m_dIntegrals[3],&Spot.m_dIntegrals[4],&Spot.m_dIntegrals[5],
            &Spot.m_dIntegrals[6],&Spot.m_dIntegrals[7],&Spot.m_dIntegrals[8],
            &Spot.m_ImgMoments.m_dM00, &Spot.m_ImgMoments.m_dM01, &Spot.m_ImgMoments.m_dM10,
            &Spot.m_ImgMoments.m_dM11, &Spot.m_ImgMoments.m_dM02, &Spot.m_ImgMoments.m_dM20) ;
          Spot.m_iGain = pSet->m_iGain * m_dGainStep_dB;
          Spot.m_iExposure = pSet->m_iExposure /  pSet->m_dScanTime_us;
          if ( iNItems >= 8 )
          {
            if ( !m_bIsSpotData )
            {
              pSet->m_SpotResults.RemoveAll() ;
              m_bIsSpotData = true ;
            }
            Spot.m_SimpleCenter = Spot.m_SimpleCenter /*+ CDPoint( 160. , 8 )*/ ;
            if ( (iNItems > 20)  &&  (Spot.m_dCentralIntegral != 0.) 
              && (Spot.m_dCentralIntegral > Spot.m_dLDiffraction) 
              && (Spot.m_dCentralIntegral > Spot.m_dRDiffraction)
              && (Spot.m_dCentralIntegral > Spot.m_dUDiffraction)
              && (Spot.m_dCentralIntegral > Spot.m_dDDiffraction) )
              Spot.m_bDetailed = true ;
            pSet->m_SpotResults.Add( Spot ) ;
            if( m_RButtonDlg.m_IsAutoSave /*m_ImParam.m_bSavePowerData*/ )
              AddspotToFile(Spot , pSet->m_CaptureGadgetName );

            if ( iWidthFOV && iHeightFOV )
              pSet->m_FOV = CRect( 0 , 0 , iWidthFOV - 1 , iHeightFOV - 1 ) ;
            if ( Spot.m_iMaxPixel > pSet->m_iLastMaxIntensity )
              pSet->m_iLastMaxIntensity = Spot.m_iMaxPixel ;
            //               if (pSet->m_CaptureGadgetName.Find("Capture2")>-1)//right
            //               {
            //                 m_LastSpot = Spot;
            //                 m_MeasuredBlobs.Copy(pSet->m_SpotResults);
            //                 m_NBlobs = pSet->m_SpotResults.GetSize();
            //               }
            if (m_iCam == 2 && pSet->m_CaptureGadgetName.Find("2")>-1)//right
            {
              m_LastSpot = Spot;
              m_MeasuredBlobs.Copy(pSet->m_SpotResults);
              // m_NBlobs = pSet->m_SpotResults.GetSize();
            }
            if (m_iCam == 1 && pSet->m_CaptureGadgetName.Find("1")>-1)//left
            {
              m_LastSpot = Spot;
              m_MeasuredBlobs.Copy(pSet->m_SpotResults);
              //m_NBlobs = pSet->m_SpotResults.GetSize();
            }

            if(pSet->m_CaptureGadgetName.Find("2")>-1)
              dAngles[0] = Spot.m_dAngle;

            if(pSet->m_CaptureGadgetName.Find("1")>-1)
              dAngles[1] = Spot.m_dAngle;

            m_NBlobs = pSet->m_SpotResults.GetSize();
          }
          int iPos = Data.Find( '\n' ) ;
          if ( iPos > 0  &&  (Data.GetLength() > (iPos + 10)) )
            Data = Data.Mid( iPos + 1 ) ;
          else
            break ;
        } while( iNItems >= 7 ) ;

        if ( !pSet->m_SpotResults.GetCount() )
        {	 
          //             CString msg ;
          //             msg.Format( "ERROR: All %d blobs are too small" , m_NBlobs ) ;
          //             strcpy(MyMsg, (LPCSTR)msg) ;
          //             IApp()->LogMessageGuest( MyMsg,1 ) ;
        }
        else
        {
          pSet->m_iMaxBlobNumber = -1 ;
          double dDistMax =  100000. ; 
          double dAreaRatio = 1.0 ;
          double dMaxArea ;
          CRect FOMeas = pSet->m_FOV ;
          double dCentX = FOMeas.CenterPoint().x ;
          double dCentY = FOMeas.CenterPoint().y ;
          FOMeas.DeflateRect( 30 , 20 ) ;
          for ( int NMax = 0 ; NMax < pSet->m_SpotResults.GetCount() ; NMax++ )
          {
            CColorSpot mySpot = pSet->m_SpotResults.GetAt(NMax);
            CRect rectInter ;
            rectInter.IntersectRect( FOMeas , mySpot.m_OuterFrame ) ;
            if ( rectInter != mySpot.m_OuterFrame )
            {
              pSet->m_SpotResults.RemoveAt( NMax ) ;
              NMax-- ;
              continue ;
            }
            double dDistNext =  GetDist( 
              mySpot.m_SimpleCenter.x , mySpot.m_SimpleCenter.y  ,
              dCentX , dCentY ) ;
            if ( NMax == 0 )
            {
              dMaxArea = mySpot.m_Area ;
              pSet->m_iMaxBlobNumber = NMax ;
              dMaxArea = (double)mySpot.m_Area;
              dDistMax = dDistNext ;
            }
            else
            {
              dAreaRatio  = (double)mySpot.m_Area / dMaxArea ;
              if ( dAreaRatio > 0.4  &&  dDistNext < dDistMax )
              {
                pSet->m_iMaxBlobNumber = NMax ;
                dMaxArea = (double)mySpot.m_Area;
                dDistMax = dDistNext ;
              }
            }
          }
          if ( pSet->m_iMaxBlobNumber >= 0
            && pSet->m_SpotResults.GetAt(pSet->m_iMaxBlobNumber).m_SimpleCenter.x > 1100 )
          {
            // ASSERT(0) ;
          }
        }
        pSet->m_SpotResults.Unlock() ;
      }
      if ( pSet->m_iLastNBlobs == 0 )
        m_NBlobs=0;
      if ( Label.Mid(5,4) == "Line" )
      {
        m_iNLinePackets++ ;
        pSet->m_LastResultAsString = Data ;
        int iNItems = 0 ;
        pSet->m_LineResults.Lock() ;
        int iPos = Data.Find( '\n' ) ;
        if ( iPos > 0   )
        {
          CString Statistics = Data.Left( iPos ) ;
          int iNSpotPos = Statistics.Find( "Lines=" ) ;
          int iMaxPos = Statistics.Find( "Max=" ) ;
          int iMinPos = Statistics.Find( "Min=" ) ;
          CHAR * p = Statistics.GetBuffer() ;
          if ( (iNSpotPos >= 0)  && (iMaxPos > 0)  && (iMinPos > 0) )
          {
            pSet->m_iLastNLines = atoi( p + iNSpotPos + 6 ) ;
            pSet->m_iLastMaxIntensity = (int)atof( p + iMaxPos + 4 ) ;
            pSet->m_iLastMinIntensity = (int)atof( p + iMinPos + 4 ) ;
          }
          if ( (iNSpotPos >= 0)  || (iMaxPos > 0)  || (iMinPos > 0) )
            Data = Data.Mid( iPos + 1 ) ; // remove statistics string
        }
        do 
        {
          if ( pSet->m_iLastNLines == 0 )
            break ;

          CLineResult Line ;
          int iIndex ;
          iNItems = sscanf( (LPCTSTR)Data ,
            "%d %lf %lf %lf %lf %lf %lf %lf %lf %d %d" ,
            &iIndex ,
            &Line.m_Center.x ,&Line.m_Center.y ,
            &Line.m_dExtremalAmpl , &Line.m_dAverCent5x5 , 
            &Line.m_DRect.left , &Line.m_DRect.right , 
            &Line.m_DRect.top , &Line.m_DRect.bottom ,
            &Line.m_ImageMinBrightness , &Line.m_ImageMaxBrightness
            ) ;         

          if ( iNItems >= 5 )
          {
            if ( !m_bIsLineData )
            {
              pSet->m_LineResults.RemoveAll() ;
              m_bIsLineData = true ;
            }
            pSet->m_LineResults.Add( Line ) ;
            if ( Line.m_dExtremalAmpl > m_iMaxIntensity )
              m_iMaxIntensity = (int)Line.m_dExtremalAmpl ;
          }
          int iPos = Data.Find( '\n' ) ;
          if ( iPos > 0  &&  (Data.GetLength() > (iPos + 10)) )
            Data = Data.Mid( iPos + 1 ) ;
          else
            break ;
        } while( iNItems >= 5 ) ;
        pSet->m_LineResults.Unlock() ;
      }

      if ( m_iMaxIntensity )
      {
        m_iLastMaxIntens = m_iMaxIntensity ;
        pSet->m_iLastMaxIntensity = m_iMaxIntensity ;
      }
      return 1 ;
    }
    if ( m_bIsLineData )
    {
      CArray <CLineResults *> LineGroups ;
      pSet->m_LineResults.Lock() ;
      for ( int iLine = 0 ; iLine < pSet->m_LineResults.GetCount() ; iLine++ )
      {
        CLineResult Line = pSet->m_LineResults[iLine] ;
        //
        if (m_iCam == 2 && pSet->m_CaptureGadgetName.Find("2")>-1)//right
          m_MeasuredLines.Copy(pSet->m_LineResults);
        if (m_iCam == 1 && pSet->m_CaptureGadgetName.Find("1")>-1)//left
          m_MeasuredLines.Copy(pSet->m_LineResults);
        //
        double dXLine = Line.m_Center.x ;
        double dYLine = Line.m_Center.y ;
        bool bFound = false ;
        for ( int iGroup = 0 ; iGroup < LineGroups.GetCount() ; iGroup++ )
        {
          CLineResults * pRes = LineGroups[iGroup] ;
          double dXGroup = pRes->GetAt(0).m_Center.x ;
          if ( fabs(dXGroup - dXLine) <= 10. )
          {
            for ( int iInGroup = 0 ; iInGroup < pRes->GetCount() ; iInGroup++)
            {
              double dYLineInGroup = pRes->GetAt(iInGroup).m_Center.y ;
              if ( dYLine < dYLineInGroup )
              {
                pRes->InsertAt( iInGroup , Line ) ;
                bFound = true ;
                break ;
              }
            }
            if ( !bFound )
            {
              pRes->Add( Line ) ;
              bFound = true ;
            }
            break ;
          }
        }
        if ( bFound )
          continue ;
        CLineResults * pNewGroup = new CLineResults ;        
        pNewGroup->Add( Line ) ;
        bool bInserted = false ;
        for ( int i = 0 ; i < LineGroups.GetCount() ; i++ )
        {
          if ( Line.m_Center.x < (LineGroups[i]->GetAt(0).m_Center.x - 5) ) 
          {
            LineGroups.InsertAt( i , pNewGroup ) ;
            bInserted = true ;
            break ;
          }
        }
        if ( !bInserted )
          LineGroups.Add( pNewGroup ) ;
      }
      if ( LineGroups.GetCount() )
      {
        int iNLines = LineGroups[0]->GetCount() ;
        bool bDifferentNLines = false ;
        for ( int iGroup = 1 ; iGroup < LineGroups.GetCount() ; iGroup++ )
        {
          if ( iNLines != LineGroups[iGroup]->GetCount() )
          {
            bDifferentNLines = true ;
            break ;
          }
        }
        if ( !bDifferentNLines  &&  (LineGroups.GetCount() == 3) )
        {
          pSet->m_ProcLResults.Lock() ;
          pSet->m_ProcLResults.RemoveAll() ;
          for ( int iLine = 0 ; iLine < LineGroups[0]->GetCount() ; iLine++ )
          {
            CLineResult Line0 = LineGroups[0]->GetAt(iLine) ;
            CLineResult Line1 = LineGroups[1]->GetAt(iLine) ;
            CLineResult Line2 = LineGroups[2]->GetAt(iLine) ;
            double dThres = (Line0.m_ImageMaxBrightness + Line0.m_ImageMinBrightness)/2. ;
            // All brighter, then average
            if ( (     Line0.m_dExtremalAmpl > dThres
              && Line1.m_dExtremalAmpl > dThres 
              && Line2.m_dExtremalAmpl > dThres )
              //                || (  Line0.m_dExtremalAmpl < dThres
              //                   && Line1.m_dExtremalAmpl < dThres 
              //                   && Line2.m_dExtremalAmpl < dThres )
              )
            {
              double dDiffX = Line2.m_Center.x - Line0.m_Center.x ;
              double dDiffY = Line2.m_Center.y - Line0.m_Center.y ;
              CLineResult NewLine(Line1) ;
              NewLine.m_dAngle = atan2( dDiffY , dDiffX ) ;
              pSet->m_ProcLResults.Add( NewLine ) ;
            }
            else
            {
              pSet->m_ProcLResults.RemoveAll() ;
              break ;
            }
          }
        }
        pSet->m_ProcLResults.Unlock() ;
      }
      pSet->m_LineResults.Unlock() ;
      for ( int iGroup = 0 ; iGroup < LineGroups.GetCount() ; iGroup++ )
      {
        delete LineGroups[iGroup] ;
      }
      LineGroups.RemoveAll() ;
      pSet->m_iLastNLines = pSet->m_ProcLResults.GetCount() ;
    }

    double dTime = get_current_time() ;
    CString Msg ;
    Msg.Format( "%s T=%6.1f" , (LPCTSTR)pSet->m_SetName ,
      dTime - m_dLastDataArrivingTime ) ;
    m_dLastDataArrivingTime = dTime ;
    if ( m_LogMessageData[0] == 0 && (pSet->m_GraphMode != 3) )
    {
      CString AddPart ;

      pSet->m_SpotResults.Lock() ;
      if ( m_bIsSpotData && (pSet->m_iMaxBlobNumber >= 0) )
      {
        AddPart.Format( ": %d Spots - C=[%6.1f,%6.1f] Sz=[%6.1f,%6.1f]" ,
          pSet->m_SpotResults.GetCount() ,
          pSet->m_SpotResults[pSet->m_iMaxBlobNumber].m_SimpleCenter.x , 
          pSet->m_SpotResults[pSet->m_iMaxBlobNumber].m_SimpleCenter.y ,
          pSet->m_SpotResults[pSet->m_iMaxBlobNumber].m_dBlobWidth ,
          pSet->m_SpotResults[pSet->m_iMaxBlobNumber].m_dBlobHeigth ) ;
        Msg += AddPart ;
      }
      pSet->m_SpotResults.Unlock() ;

      pSet->m_ProcLResults.Lock() ;
      if ( pSet->m_ProcLResults.GetCount() )
      {
        AddPart.Format( ": %d Lines - C=[%6.1f,%6.1f] Ang=%8.3f" ,
          pSet->m_ProcLResults.GetCount() ,
          pSet->m_ProcLResults[0].m_Center.x , 
          pSet->m_ProcLResults[0].m_Center.y ,
          pSet->m_ProcLResults[0].m_dAngle * 180./M_PI) ;
        Msg += AddPart ;
      }
      pSet->m_ProcLResults.Unlock() ;

      strcpy( m_LogMessageData , (LPCTSTR)Msg ) ;
      PostMessage( iImProcResultMessage , 0 , (LPARAM) m_LogMessageData ) ;
    }
    pSet->m_dCaptureFinishedAt = get_current_time() ;
    m_bOnGrab = 0 ;
    m_iMaxIntensity = 0 ;
    m_bIsSpotData = false ;
    m_bIsLineData = false ;
    m_iNSpotPackets = 0 ;
    m_iNLinePackets = 0 ;
    m_iNFrames = 0 ;

    return 0;
  }

  int CImageView::ProcessReceivedDataFrame(CDataFrame * pDataFrame , CMeasurementSet * pSet )
  {
    //   int iMaxIntensity = 0 ;
    //   bool bIsSpotData = false ;
    //   bool bIsLineData = false ;
    //   int iNSpotPackets = 0 ;
    //   int iNLinePackets = 0 ;
    int iNFrames = 0 ;
    int iNTextFrames = 0 ;
    m_iMaxIntensity = 0 ;
    m_bIsSpotData = false ;
    m_bIsLineData = false ;
    m_iNSpotPackets = 0 ;
    m_iNLinePackets = 0 ;
    m_iNFrames = 0 ;

    if ((m_ImParam.m_bSaveVideo && !m_bVideoSavingEnabled) )
    {
      m_bVideoSavingEnabled = TRUE;
      if (m_ImParam.m_bCam0)
        EnableVideoWriting(true,0);
      if (m_ImParam.m_bCam1)
        EnableVideoWriting(true,1);

    }
    else if((!m_ImParam.m_bSaveVideo && m_bVideoSavingEnabled) )
    {
      m_bVideoSavingEnabled = FALSE;
      if (m_ImParam.m_bCam0)
      {
        EnableVideoWriting(false,0);
        CloseVideoFile(0);
      }
      if (m_ImParam.m_bCam1)
      {
        EnableVideoWriting(false,1);
        CloseVideoFile(1);
      }
    }

    if ( m_bPlayBackMode && !m_LiveVideo )
    {
      pSet->Grab(0) ;  // close file read switch 
    }
    CFramesIterator* pIterator = pDataFrame->CreateFramesIterator( text );
    if ( pIterator )
    {
      CDataFrame * pF = NULL ;
      int iCnt = 0 ;

      //save Picture of Spots
      if(pSet->m_CaptureGadgetName.Find("2")>-1) 
      {
        if (( m_ImParam.m_bSaveImage != m_bsetAutoSave) && (!m_bSaveOnePic))
        {         
          m_bsetAutoSave =  m_ImParam.m_bSaveImage;
          SavePictures( m_bsetAutoSave );
        }

        if (!m_ImParam.m_bSaveImage && m_bsetManSave!=m_bSaveOnePic)
        {
          m_bsetManSave = m_bSaveOnePic;
          SavePictures( m_bsetManSave );
        }
      }


      while ( pF =  pIterator->Next( NULL ) )
      {
        iNFrames++ ;
        CTextFrame * pTF = pF->GetTextFrame( DEFAULT_LABEL ) ;
        if ( pTF )
        {
          //         //save Picture of Spots
          //         if (( m_ImParam.m_bSaveImage != m_bsetAutoSave) && (!m_bSaveOnePic))
          //         {         
          //           m_bsetAutoSave =  m_ImParam.m_bSaveImage;
          //           SavePictures( m_bsetAutoSave );
          //         }
          // 
          //         if (!m_ImParam.m_bSaveImage && m_bsetManSave!=m_bSaveOnePic)
          //         {
          //           m_bsetManSave = m_bSaveOnePic;
          //           SavePictures( m_bsetManSave );
          //         }

          iNTextFrames++ ;
          CString Label = pF->GetLabel() ;
          if ( Label.Left(5) != "Data_" )
            continue ;
          CString Data = pSet->m_LastResultAsString = pTF->GetString() ;

          ProcessReceivedDataFrame( Data , Label , pSet ) ;
          iCnt++ ;
        }
      }
      if ( iCnt )
        ProcessReceivedDataFrame( CString("FINISHED") , CString("FINISHED") , pSet , true ) ;
      delete pIterator ;
    }
    return 0;
  }

  bool CImageView::WaitEndOfGrabAndProcess( int iTimeOut_ms) 
  {
    double dWaitBegin = get_current_time() ;
    while ( m_bOnGrab )
    {
      if ( (get_current_time() - dWaitBegin) >= iTimeOut_ms )
        return false ;
      Sleep(5) ;
    }
    return true ;
  }

  void CImageView::OnGraphSetup()
  {
    if ( m_Graph.IsInitialized() )
      m_Graph.RunSetupDialog() ;
  }

  int CImageView::LoadGraph(CString& GraphName)
  {
    CRegistry Reg( "File Company\\ImCNTL_MV" ) ;
    CString CaptureGadgetName = Reg.GetRegiString( "CameraComm" , "CaptureGadgetName" , "MarlinCapture%d" ) ;
    CString CaptureControlName = Reg.GetRegiString( "CameraComm" , "CaptureControlName" , "MarlinCapture%d<>0" ) ;
    CString SoliosCaptureGadgetName = Reg.GetRegiString( "CameraComm" , "SoliosCaptureGadgetName" , "SoliosDalsa%d" ) ;  
    CString SoliosCaptureControlName = Reg.GetRegiString( "CameraComm" , "SoliosCaptureControlName" , "SoliosDalsa%d<>0" ) ; 
    CString AVT_SoliosSwitchControl = Reg.GetRegiString( "CameraComm" , "AVT_SoliosSwitchControl" , "Multiplex%d<<2" ) ;
    CString Image_vs_BackControlName = Reg.GetRegiString( "CameraComm" , "Image_vs_BackControlName" , "Mux2OutCh%d_2<<0" ) ;
    CString ClearBackName = Reg.GetRegiString( "CameraComm" , "ClearBackName" , "SubBack%d<<2" ) ;
    CString LowPassUseControlName = Reg.GetRegiString( "CameraComm" , "LowPassUseControlName" , "Mux2OutCh%d_1<<0" ) ;
    CString ProcessControlPinName = Reg.GetRegiString( "CameraComm" , "ProcessControlName" , "TVObjects%d<<1" ) ;
    CString ProcessScriptPinName = Reg.GetRegiString( "CameraComm" , "ProcessScriptName" , "TVObjects%d<<2" ) ;
    CString ProcessGadgetNameFormat = Reg.GetRegiString( "CameraComm" , "ProcessGadgetName" , "TVObjects%d" ) ;
    CString DataOutName = Reg.GetRegiString( "CameraComm" , "DataOutName" , "TVObjects%d>>0" ) ;
    CString RenderGadgetName = Reg.GetRegiString( "CameraComm" , "RenderGadgetName" , "VideoRender%d" ) ;
    CString BMPSwitchGadgetName = Reg.GetRegiString("CameraComm","BMPSwitchGadgetName" ,"Switch%d<>0" );
    CString BMPRenderGadgetName = Reg.GetRegiString("CameraComm","BMPREnderGadgetNmae", "BMPRender%d<<0" );
    CString VideoOutGadgetName = Reg.GetRegiString( "CameraComm" , "VideoOutGadgetName" , "VideoRender%d>>0" ) ;
    CString EnableFileReadName = Reg.GetRegiString( "CameraComm" , "EnableFileReadName" , "ReadEnable<>0" ) ;
    CString EnableFileWriteName = Reg.GetRegiString( "CameraComm" , "EnableFileWriteName" , "WriteEnable%d<>0" ) ;
    CString CamOrLanSwitchPinName = Reg.GetRegiString( "CameraComm" , "CamOrLanSwitchPinName" , "SwCamOrLAN%d<<2" ) ; 
    CString ExtractResultsGadgetName = Reg.GetRegiString( "CameraComm" , "ExtractResultsGadgetName" , "ExtractRes%d" ) ; 
    CString ImageToLANExtractorGadgetName = Reg.GetRegiString( "CameraComm" , "ImageToLANExtractorGadgetName" , "Disassemble%d" ) ; 
    CString LANOutputPinName = Reg.GetRegiString( "CameraComm" , "LANOutputPinName" , "LANserver%d>>0" ) ;
    CString LANInputPinName  = Reg.GetRegiString("CameraComm" , "LANInputPinName" , "LANserver%d<<0" );
    CString BMPCutRect = Reg.GetRegiString("CameraComm" , "VideoCutRect" ,"VideoCutRect%d<>0" );
    CString RadialDistrCalcGadgetName = Reg.GetRegiString("CameraComm" , "RadialDistrCalcGadgetName" ,"RadialDistr%d" );
    CString BMPBufferGadgetControlName = Reg.GetRegiString("CameraComm" , "BMPBuffer" ,"BMPBuffer%d<>0");
    CString ProfileGadgetControlName = Reg.GetRegiString("CameraComm","ProfileGadgetOutputPinName","CAreaProfile%d<>0");
    CString ProfileGadgetOutputPinName = Reg.GetRegiString("CameraComm","ProfileGadgetName","CAreaProfile%d>>0");
    CString VideoSwitchGadgetName = Reg.GetRegiString("CameraComm","VideoSwitchGadgetName" ,"VideoWriteSwitch%d<>0" );
    CString AviRenderGadgetName = Reg.GetRegiString("CameraComm","AviRenderGadgetName" ,"AviRender%d<<0" );
    int iTriggerDelay_us = Reg.GetRegiInt( "ImageMeasPar" , "TriggerDelayBase_us" , 399 ) ;
    int iTriggerDelayStep_us = Reg.GetRegiInt( "ImageMeasPar" , "TriggerDelayStep_us" , 0 ) ;
    int iNCameras = Reg.GetRegiInt( "CameraComm" , "NCameras" , 2 ) ;
    m_dGainStep_dB = Reg.GetRegiDouble( "ExposureParam" , "GainStep_dB" , 0.0359 ) ;

    bool bWasWorking = (m_pBuilder != NULL) ;
    if ( m_pBuilder )
    {
     
      for ( int i = 0 ; (i < m_MeasSets.GetCount())  &&  !m_bPlayBackMode ; i++ )
        SwitchCamera( 0 , i ) ;
      Sleep(100) ;
      m_pBuilder->Stop() ;
      m_MeasSets.RemoveAll() ;
      m_pBuilder->Release();
      m_pBuilder=NULL;
      FxExitMsgQueues();
      Sleep(200);
    }


    m_pBuilder = Tvdb400_CreateBuilder();
    FxInitMsgQueues( PrintMsg );
    m_pPluginLoader = m_pBuilder->GetPluginLoader();
    m_pPluginLoader->RegisterPlugins( m_pBuilder) ;

    if ( m_iGraphMode > 0 && m_iGraphMode < 4 )
    {  
      HWND hExistingAppWindow = ::FindWindow( NULL , "Image Control Server Solios" ) ;
      while ( !hExistingAppWindow )
      {
        CRegistry FileReg( "File Company\\" ) ;
        CString CmndStr("start ") ;
        CString ControlProgram = FileReg.GetRegiString( "ImCNTL", "SoliosControlProgram" , 
          "D:\\Dev\\OpticJigDev_Moisey_DB\\Debug\\ImCNTL_MIL9_MC.exe") ;
        // clear leading spaces etc.
        ControlProgram.Trim( " \t\n\r.," ) ;
        CmndStr += ControlProgram ;
        system( (LPCTSTR)CmndStr ) ;
        ULONGLONG ullStartTime = GetTickCount64();
        while (!hExistingAppWindow && ((GetTickCount64() - ullStartTime) < 50000) )
        {
          Sleep(1000);
          hExistingAppWindow = ::FindWindow(NULL, "Image Control Server Solios");
        }

        if ((GetTickCount64() - ullStartTime) >= 50000)
        {
          int iRes = AfxMessageBox( (LPCTSTR)CmndStr.Insert(0, "Can't start x64 vision:\n") , 
            MB_OKCANCEL);
          if ( iRes != IDOK )
            return -1;
        }
      }
    }

    if ( m_pBuilder->Load( (LPCTSTR) GraphName ) == MSG_ERROR_LEVEL )
    {
      CString Msg("Failed to load graph: ") ;
      Msg += GraphName ;
      AfxMessageBox( Msg );
    }
    else
    {
      m_Graph.Init( m_pBuilder , m_pPluginLoader ) ;

      CRect Client ;
      GetClientRect( &Client ) ;
      Client.top += 24 ;
      Client.bottom -= 24 ;

      int iSetCnt = 0 ;
      int iClientId = 10000 ;
      CCaptureGadget * pCaptureGadget = NULL ;
      LPCTSTR pClass = AfxRegisterWndClass( NULL ) ;

      do
      {
        CString GadgetName ;
        switch ( m_iGraphMode )
        {
        case 0 :
        case 1 :
          GadgetName.Format( (LPCTSTR) CaptureGadgetName , iSetCnt + 1 ) ;
          break;
        case 2:
//           if( iSetCnt == 1 )
//           {
            GadgetName.Format( (LPCTSTR) SoliosCaptureGadgetName , iSetCnt + 1 ) ;
            CaptureControlName = SoliosCaptureControlName;
//           }
          break ;
        case 3:
          {
            GadgetName.Format( (LPCTSTR) SoliosCaptureGadgetName , iSetCnt + 1 ) ;
            CaptureControlName = SoliosCaptureControlName;
            CString AVT_SoliosSwPinName ;
            AVT_SoliosSwPinName.Format( AVT_SoliosSwitchControl , iSetCnt + 1 ) ;
            m_Graph.SendQuantity( AVT_SoliosSwPinName , 1. ) ;
          }
          break ;
        case 4:
          {
            GadgetName.Format( (LPCTSTR) CaptureGadgetName , iSetCnt + 1 ) ;
            CString AVT_SoliosSwPinName ;
            AVT_SoliosSwPinName.Format( AVT_SoliosSwitchControl , iSetCnt + 1 ) ;
            m_Graph.SendQuantity( AVT_SoliosSwPinName , 0. ) ;
          }
          break ;
        }

        pCaptureGadget = (CCaptureGadget *) m_pBuilder->GetGadget( (LPCTSTR) GadgetName ) ;
        if ( pCaptureGadget )
        {
          CString RendName;
          RendName.Format("Renderer%d" , iSetCnt + 1  ) ;
          CMeasurementSet NewSet(this) ;
          NewSet.m_pCaptureGadget = pCaptureGadget ;
          NewSet.m_CaptureGadgetName = GadgetName ;
          NewSet.m_CaptureControlName.Format( CaptureControlName , iSetCnt + 1 ) ;
          NewSet.m_ImageOrBackName.Format( Image_vs_BackControlName , iSetCnt + 1 ) ;
          NewSet.m_ClearBackName.Format( ClearBackName , iSetCnt + 1 ) ;
          NewSet.m_LowPassOnName.Format( LowPassUseControlName , iSetCnt + 1 ) ;
          NewSet.m_BMPSwitchName.Format( BMPSwitchGadgetName , iSetCnt + 1 );
          NewSet.m_BMPRenderName.Format(BMPRenderGadgetName , iSetCnt + 1 );
          NewSet.m_BMPCutRectPinName.Format(BMPCutRect , iSetCnt + 1);
          NewSet.m_CamOrLANSwitchPinName.Format( CamOrLanSwitchPinName , iSetCnt + 1 ) ;
          NewSet.m_ResultsExtractorGadgetName.Format( ExtractResultsGadgetName , iSetCnt + 1 ) ;
          NewSet.m_GraphMode = Reg.GetRegiInt( "CameraComm" , "GraphMode" , 0 ) ; 
          NewSet.m_ImageToLANExtractorGadgetName.Format( ImageToLANExtractorGadgetName , iSetCnt + 1 ) ;
          NewSet.m_LANOutputPinName.Format( LANOutputPinName , iSetCnt + 1 ) ;
          NewSet.m_ProcessControlName.Format( ProcessControlPinName , iSetCnt + 1 ) ;
          NewSet.m_ProcessScriptName.Format( ProcessScriptPinName , iSetCnt + 1 ) ;
          NewSet.m_OpenReadFileSwitchName.Format( EnableFileReadName , iSetCnt + 1 ) ; // really, no set 
          NewSet.m_RadialDistrCalcGadgetName.Format( RadialDistrCalcGadgetName , iSetCnt + 1 ) ; // really, no set 
          NewSet.m_ProfileGadgetOutputPinName.Format(ProfileGadgetOutputPinName,iSetCnt + 1);
          NewSet.m_ProfileGadgetControlName.Format(ProfileGadgetControlName,iSetCnt + 1);
          // number will be inserted, all channels will be opened
          NewSet.m_OpenWriteFileSwitchName.Format( EnableFileWriteName , iSetCnt + 1 ) ;
          CString ProcessGadgetName ;
          ProcessGadgetName.Format( ProcessGadgetNameFormat , iSetCnt + 1 ) ;
          NewSet.m_pProcessGadget = (CFilterGadget *)m_pBuilder->GetGadget((LPCTSTR)ProcessGadgetName) ;
          if ( NewSet.m_pProcessGadget )
          {
            FXPropertyKit Properties ;
            if ( NewSet.m_pProcessGadget->PrintProperties( Properties ) )
            {
              DWORD dwViewMode ;
              if ( Properties.GetInt( "ViewMode" , (int&)dwViewMode ) )
                m_dwSHViewMode = dwViewMode ;
            }

          }

          NewSet.m_DataOutName.Format( DataOutName , iSetCnt + 1 ) ;
          NewSet.m_RenderGadgetName.Format( RenderGadgetName , iSetCnt + 1 ) ;
          NewSet.m_LANOutputPinName.Format( LANOutputPinName , iSetCnt + 1 ) ;
          NewSet.m_LANInputPinName.Format( LANInputPinName, iSetCnt + 1 );
          NewSet.m_BMPBufferControlPinName.Format(BMPBufferGadgetControlName , iSetCnt + 1);
          NewSet.m_VideoSwitchName.Format(VideoSwitchGadgetName,iSetCnt + 1);
          NewSet.m_AviRenderInputName.Format(AviRenderGadgetName, iSetCnt + 1);

          FXString Tmp ;
          NewSet.m_pCaptureGadget->PrintProperties( Tmp ) ;
          NewSet.m_CaptureProperties = Tmp ;
          NewSet.m_iTrigDelay_uS = iTriggerDelay_us ;
          m_MeasSets.Add( NewSet ) ;
          iSetCnt++ ;

        }
      } while ( pCaptureGadget && (iSetCnt < iNCameras) ) ;


      if ( m_MeasSets.GetCount() )
      {
        m_pBuilder->Start() ;
        Sleep(250) ;
        
        for ( int i = 0 ;  (i < m_MeasSets.GetCount())  &&  !m_bPlayBackMode ; i++ )
          SwitchCamera( 0 , i ) ;
        m_bAsyncGrab = 0 ;
        Sleep(100) ;      

        int iNx = 1 +  (m_MeasSets.GetCount() > 1) ;
        int iNy = 1 +  (m_MeasSets.GetCount() > 2) ;
        int iLx = Client.Width()/iNx ;
        int iLy = Client.Height()/iNy ;
        for ( int iSetCnt = 0 ; iSetCnt < m_MeasSets.GetCount(); iSetCnt++)
        {
          CMeasurementSet * pSet = &m_MeasSets[iSetCnt] ;
          CQuantityFrame * pQuan = CQuantityFrame::Create( 0. ) ;
          pQuan->ChangeId( 0 ) ;
          if ( !m_pBuilder->SendDataFrame( pQuan , pSet->m_ImageOrBackName ) ) // send images to image input
            pQuan->Release( pQuan ) ;                                          // of anchor, not to background
          pQuan = CQuantityFrame::Create( 1. ) ; 
          pQuan->ChangeId( 0 ) ;
          if ( !m_pBuilder->SendDataFrame( pQuan , pSet->m_LowPassOnName ) )   // Send images directly, without
            pQuan->Release( pQuan ) ;                                          // low pass filter

          CBooleanFrame * pBul = CBooleanFrame::Create(FALSE);
          if (!m_pBuilder->SendDataFrame(pBul , pSet->m_BMPSwitchName))
            pBul->Release( pBul );

          CString sCrect = _T("Rect=8,8,1200,1008;");
          if( pSet->m_GraphMode > 0 )
            sCrect = _T("Rect=4,4,984,984;");
          CTextFrame *pText = CTextFrame::Create(sCrect);
          if (!m_pBuilder->SendDataFrame(pText,pSet->m_BMPCutRectPinName))
            pText->Release( pText );


          pSet->m_iActiveInput = 0 ;
          if ( !m_bPlayBackMode )
          {
            SetTriggerPolarity( 0 , iSetCnt ) ;
            SetTriggerSource( 1 , iSetCnt ) ;
            SetGrabTrigger( 0 , iSetCnt ) ;
            SetGrabTriggerDelay( 
              iTriggerDelay_us + iSetCnt * iTriggerDelayStep_us , iSetCnt ) ;
            SetGammaCorrection( 0 , iSetCnt ) ;
          }
          else
          {  // Playback mode
          }



          Sleep(20) ;
          //pSet->ClearBack() ; 
          m_pBuilder->SetOutputCallback( pSet->m_CaptureControlName ,
            fnCaptureSettingsCallBack , pSet ) ;
          m_pBuilder->SetOutputCallback( pSet->m_DataOutName ,
            fnResultCallBack , pSet ) ;
          m_Graph.SetDataCallBack( pSet->m_LANOutputPinName , DataCallbackFromExternalImaging , pSet , true ) ;
          m_Graph.SetDataCallBack( pSet->m_ProfileGadgetOutputPinName,CallbackFromProfileGadget , pSet , true );

          CString sPin = pSet->m_RenderGadgetName + ">>0"; 
          m_pBuilder->SetOutputCallback(sPin , fnRendererCallBack,pSet);

          sPin = pSet->m_BMPSwitchName;
          sPin = sPin.Trim("<>0");
          sPin += ">>0"; 
          m_pBuilder->SetOutputCallback(sPin , fnBMPSaverCallback,pSet);

          CPoint LT( ((iSetCnt) & 1) * iLx , (((iSetCnt) & 2) >> 1) * iLy + 24) ;
          CSize Sz( iLx , iLy ) ;
          CRect NewClient( LT , Sz ) ;
          CWnd * pW = new CWnd() ;
          pSet->m_pChildWindow = pW ;
          CString RenderName ;
          RenderName.Format( _T("Camera%d") , m_MeasSets.GetUpperBound() - iSetCnt ) ;
          //RenderName.Format( _T("Camera%d") , iSetCnt + 1 ) ;
          CString CaptionWindowName = Reg.GetRegiString( "CameraComm" , (LPCTSTR)RenderName , (LPCTSTR)RenderName ) ;

          BOOL bRes = pW->Create( NULL , (LPCTSTR)CaptionWindowName , 
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CAPTION,
            NewClient, this, 0 ) ;
          //         pSet->m_pRenderWnd = pW ;  // for debugging without connection to graph
          pW->ShowWindow( SW_SHOW ) ;
          pW->SetWindowPos( 0 , 
            LT.x , LT.y , Sz.cx  , Sz.cy , SWP_NOZORDER ) ;
          m_pBuilder->ConnectRendererAndMonitor( 
            pSet->m_RenderGadgetName , 
            pW , (LPCTSTR) RenderName , pSet->m_pRenderGadget ) ;
          if (pSet->m_pRenderGadget)
          {
            pSet->m_pRenderWnd = 
              pSet->m_pRenderGadget->GetRenderWnd() ;
          }
          else
          {
            CString Msg ;
            Msg.Format( "Can't attach %s to Display Window " , 
              (LPCTSTR) pSet->m_RenderGadgetName ) ;
            AfxMessageBox( Msg ) ;
            return S_FALSE ;
          }
          FXString Tmp ;
          pSet->m_pRenderGadget->PrintProperties( Tmp ) ;
          pSet->m_RenderProperties = Tmp ;
          if ( !bWasWorking )
          {
            pSet->m_pRenderWnd->SetWindowPos( 0 , 
              0 , 0 , Sz.cx  , Sz.cy , SWP_NOZORDER ) ;
          }


          CString CamSuffix ;
          CamSuffix.Format( "ATJ%d" , iSetCnt ) ;
          CString ShMemName("Camera") ; ShMemName += CamSuffix ;
          CString InEventName("CamInEvent") ; InEventName += CamSuffix ;
          CString OutEventName("CamOutEvent") ; OutEventName += CamSuffix ;
          pSet->m_pShMemControl = new CSMCamControl( 
            (LPCTSTR)ShMemName , (LPCTSTR)InEventName , (LPCTSTR)OutEventName ,
            this , pSet ) ;
          CString GainControl ;
          GainControl.Format( "Gain%d" , iSetCnt ) ;
          int iGain = Reg.GetRegiInt( "ImageMeasPar" , (LPCTSTR)GainControl , 100 ) ;
          pSet->m_dScanTime_us = m_dScanTime ;
          pSet->SetGain( iGain ) ;
          pSet->SetExposure( 4 ) ;
          CRegistry Reg("File Company\\OpticJig") ;
          CString ScaleName ;
          //ScaleName.Format( "ScaleX_pix_per_mm_Cam%d" , iSetCnt ) ;
          ScaleName.Format( "ScaleX_pix_per_mm_Cam%d" , m_MeasSets.GetCount() - iSetCnt -1) ;
          double dScaleX = Reg.GetRegiDouble( "Scales" , ScaleName , 1.0 ) ;
          //ScaleName.Format( "ScaleY_pix_per_mm_Cam%d" , iSetCnt ) ;
          ScaleName.Format( "ScaleY_pix_per_mm_Cam%d" , m_MeasSets.GetCount() - iSetCnt -1) ;
          double dScaleY = Reg.GetRegiDouble( "Scales" , ScaleName , 1.0 ) ;
          double dScale = 0.5 * ( fabs(dScaleX) + fabs(dScaleY) ) ;

          /*
          if (pSet->m_GraphMode > 0)
          {   
            dScaleX = Reg.GetRegiDouble( "Scales" , "Scale CarmelCamera_X_per_mm_cam0" , 1575.5 ) ;
            dScaleY = Reg.GetRegiDouble( "Scales" , "Scale CarmelCamera_Y_per_mm_cam0" , 1575.5 ) ;
            dScale = 0.5 * ( fabs(dScaleX) + fabs(dScaleY) ) ;
          }
          */

          if ( dScale != 0.0 )
            pSet->SetScale( 1000./dScale , _T("um") ) ;
          pSet->SetGamma( 0 ) ;
          pSet->EnableRadialDistributionShow( false ) ;
          FXPropertyKit pk ;
          pSet->m_pProcessGadget->PrintProperties( pk ) ;
          FXString Template ;
          pk.GetString( "Template" , Template ) ;
          CString FileName ;
          FileName.Format( "Template%d.txt" , iSetCnt ) ;
          FILE * fw = fopen( FileName , "wb" ) ;
          if ( fw )
          {
            fwrite( (LPCTSTR)Template , 1 , Template.GetLength() , fw ) ;
            fwrite( "\r\n\0" , 1 , 3 , fw ) ;
            fclose(fw) ;
          }
          pSet->ClearBack() ;
        }
        
        OnAsynchroneMode() ;
        OnMeasureLine() ;
        SetWindowPos( &wndTop , 5 , 5 , (m_MeasSets.GetCount() == 1) ? 600 : 1200 ,
          (m_MeasSets.GetCount() <= 2) ? 560 : 1040 , 0 ) ; 
        switch ( m_MeasSets[0].m_GraphMode )
        {
        case 0 : OnDontUseExternalImaging() ; break ;
        case 1 : OnTakeImagesFromLAN() ; break ;  // get images and send results
        case 2 : UseSoliosCamera(); break;
        case 3 : UseSoliosCamera() ; break ;
        }
      }

      for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
      {
        m_MeasSets[i].SetThresholdForBinarization( m_ImParam.m_BinThreshold ) ;
        m_MeasSets[i].SetDiffractionMeasPar( 
          m_ImParam.m_iDiffractionMeasurementMethod ,
          m_ImParam.m_iDiffractionRadius , m_ImParam.m_iDiffractionRadius_Y , 
          m_ImParam.m_iBackgroundDist ) ;
      }
    }

    return 0;
  }

  void CImageView::OnViewGraph()
  {
    if (m_pBuilder)
    {
      m_bRunBeforeInspect = (m_pBuilder->IsRuning() != FALSE) ;
      StartInspect(m_pBuilder, this);
      FinishInspect();
    }
  }

  void CImageView::OnParentNotify(UINT message, LPARAM lParam)
  {
    //   CMainFrame::OnParentNotify(message, lParam);
    // 
    if ((message == WM_DESTROY) && ((HWND)lParam == GetSafeHwnd())) // this must be a message from debug window
    {
      if (m_pBuilder)
      {
        CRegistry Reg( "File Company\\ImCNTL" ) ;
        CRect Client ;
        GetClientRect( &Client ) ;
        Client.top += 24 ;
        Client.bottom -= 24 ;
        int iNx = 1 +  (m_MeasSets.GetCount() > 1) ;
        int iNy = 1 +  (m_MeasSets.GetCount() > 2) ;
        int iLx = Client.Width()/iNx ;
        int iLy = Client.Height()/iNy ;
        for ( int iSetCnt = 0 ; iSetCnt < m_MeasSets.GetCount(); iSetCnt++)
        {
          CPoint LT( ((iSetCnt) & 1) * iLx , (((iSetCnt) & 2) >> 1) * iLy + 24) ;
          CSize Sz( iLx , iLy ) ;
          CRect NewClient( LT , Sz ) ;

          CWnd * pW = new CWnd() ;
          CMeasurementSet * pSet = &m_MeasSets[iSetCnt] ;
          if ( pSet->m_pChildWindow )
            delete pSet->m_pChildWindow ;

          pSet->m_pChildWindow = pW ;
          CString RenderName ;
          RenderName.Format( _T("Camera%d") , m_MeasSets.GetUpperBound() - iSetCnt ) ;
          //RenderName.Format( _T("Camera%d") , iSetCnt + 1 ) ;
          CString CaptionWindowName = Reg.GetRegiString( "CameraComm" , 
            (LPCTSTR)RenderName , (LPCTSTR)RenderName      ) ;

          BOOL bRes = pW->Create( NULL , (LPCTSTR)CaptionWindowName , 
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CAPTION,
            NewClient, this, 0 ) ;
          //         pSet->m_pRenderWnd = pW ;  // for debugging without connection to graph
          pW->ShowWindow( SW_SHOW ) ;
          pW->SetWindowPos( 0 , 
            LT.x , LT.y , Sz.cx  , Sz.cy , SWP_NOZORDER ) ;

          m_pBuilder->ConnectRendererAndMonitor( 
            pSet->m_RenderGadgetName , 
            pW , (LPCTSTR) RenderName , pSet->m_pRenderGadget ) ;
          if (pSet->m_pRenderGadget)
          {
            pSet->m_pRenderWnd = 
              pSet->m_pRenderGadget->GetRenderWnd() ;
          }
          else
          {
            CString Msg ;
            Msg.Format( "Can't attach %s to Display Window " , 
              (LPCTSTR) pSet->m_RenderGadgetName ) ;
            AfxMessageBox( Msg ) ;
          }
          FXString Tmp ;
          pSet->m_pRenderGadget->PrintProperties( Tmp ) ; 
          pSet->m_RenderProperties = Tmp ;
        };
        if (m_bRunBeforeInspect)
          m_pBuilder->Start();
        if ( !m_bPlayBackMode )
        {
          for ( int iSetCnt = 0 ; iSetCnt < m_MeasSets.GetCount(); iSetCnt++)
          {
            CMeasurementSet * pSet = &m_MeasSets[iSetCnt] ;

            CQuantityFrame * pQuan = CQuantityFrame::Create( 0. ) ;
            pQuan->ChangeId( 0 ) ;
            if ( !m_pBuilder->SendDataFrame( pQuan , pSet->m_ImageOrBackName ) ) // send images to image input
              pQuan->Release( pQuan ) ;                                          // of anchor, not to background
            pQuan = CQuantityFrame::Create( 1. ) ; 
            pQuan->ChangeId( 0 ) ;
            if ( !m_pBuilder->SendDataFrame( pQuan , pSet->m_LowPassOnName ) )   // Send images directly, without
              pQuan->Release( pQuan ) ;                                          // low pass filter
            pSet->m_iActiveInput = 0 ;
            pSet->ClearBack() ;
            OnMeasureBlob() ;
            if (m_bRunBeforeInspect && m_bPlayBackMode)
            {
              CBooleanFrame * pCommand = CBooleanFrame::Create( true ) ;
              if ( !m_pBuilder->SendDataFrame( pCommand , pSet->m_OpenReadFileSwitchName ) )
              {
                pCommand->Release( pCommand ) ;
              }
            }
          }
        }
      }
    }
    else
      CMainFrame::OnParentNotify( message , lParam ) ;
  }
  int CImageView::AddspotToFile(CColorSpot cSpot , CString sCamNum)
  {
    if (sCamNum.Find("1")>-1)
      sCamNum = _T("Cam1");
    if (sCamNum.Find("2")>-1)  
      sCamNum = _T("Cam0");


    CString FileNameDebug;
    CString spath = m_RButtonDlg.m_AutoSaveFileName;//m_ImParam.m_PowerSaveName; 
    FileNameDebug.Format("\\BlobResultFrom_%s.dat",sCamNum);
    spath+=FileNameDebug;
    FILE * fw ;

    if (cSpot.m_bDetailed)
    {
      fw = fopen( ( LPCTSTR )spath , "a+" ) ;
      if (fw)
      {

        if (m_RButtonDlg.m_bWriteHeadlines)
        {
          if (sCamNum.Find("0")>-1 && !FirstFileReady )
          {
            CString sHeadString (_T("#SpotNum,X_Center,Y_Center,BlobWidth,BlobHeight,Area,MaxPixel,Central5x5,SumOverThreshold,Angle,LongDiameter,ShortDiameter,RDiffraction,LDiffraction,DDiffraction,UDiffraction,CentralIntegral,left,top,right,bottom,SumOverTh,Area,c1,c2,c3,c4,c5,c6,c7,c8,c9\r\n"));
            fputs( ( LPCTSTR )sHeadString , fw ) ;
            FirstFileReady = true;
          }
          if (sCamNum.Find("1")>-1 && !SecondfileReady )
          {
            CString sHeadString (_T("#SpotNum,X_Center,Y_Center,BlobWidth,BlobHeight,Area,MaxPixel,Central5x5,SumOverThreshold,Angle,LongDiameter,ShortDiameter,RDiffraction,LDiffraction,DDiffraction,UDiffraction,CentralIntegral,left,top,right,bottom,SumOverTh,Area,c1,c2,c3,c4,c5,c6,c7,c8,c9\r\n"));
            fputs( ( LPCTSTR )sHeadString , fw ) ;
            SecondfileReady = true;
          }
          if (FirstFileReady && SecondfileReady)
          {
            m_RButtonDlg.m_bWriteHeadlines = FALSE;
            SecondfileReady = FirstFileReady = FALSE;
          }
        }



        double dScaleX,dScaleY;
        if (sCamNum.Find("0")>-1)
        {
          dScaleX = m_RButtonDlg.m_dScaleXCam2; 
          dScaleY = m_RButtonDlg.m_dScaleYCam2;
        }
        else if(sCamNum.Find("1")>-1)
        {
          dScaleX = m_RButtonDlg.m_dScaleXCam1; 
          dScaleY = m_RButtonDlg.m_dScaleYCam1;
        }




        int iSpotNum=-1;
        if (cSpot.m_SimpleCenter.x < 650)
          iSpotNum = 1;
        else 
          iSpotNum = 2;

        CString sBlob;
        sBlob.Format( 
          "%d %6.2lf,%6.2f,%6.2f,%6.2f,%d,%d,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%d,%d,%d,%d,%6.2f,%d,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f ",
          iSpotNum,
          cSpot.m_SimpleCenter.x ,
          cSpot.m_SimpleCenter.y ,
          fabs(cSpot.m_dBlobWidth * 1000 / dScaleX ),
          (cSpot.m_dBlobHeigth * 1000 /dScaleY),
          cSpot.m_Area ,
          cSpot.m_iMaxPixel,
          cSpot.m_dCentral5x5Sum ,
          cSpot.m_dSumOverThreshold ,
          cSpot.m_dAngle   ,
          cSpot.m_dLongDiametr,
          cSpot.m_dShortDiametr ,
          cSpot.m_dRDiffraction ,
          cSpot.m_dLDiffraction ,
          cSpot.m_dDDiffraction ,
          cSpot.m_dUDiffraction ,
          cSpot.m_dCentralIntegral ,
          cSpot.m_OuterFrame.left ,
          cSpot.m_OuterFrame.top ,
          cSpot.m_OuterFrame.right ,
          cSpot.m_OuterFrame.bottom ,
          cSpot.m_dSumOverThreshold,
          cSpot.m_Area,
          cSpot.m_dIntegrals[0],
          cSpot.m_dIntegrals[1],
          cSpot.m_dIntegrals[2],
          cSpot.m_dIntegrals[3],
          cSpot.m_dIntegrals[4],
          cSpot.m_dIntegrals[5],
          cSpot.m_dIntegrals[6],
          cSpot.m_dIntegrals[7],
          cSpot.m_dIntegrals[8]) ;
        sBlob+=_T("\r\n");
        fputs( ( LPCTSTR )sBlob , fw ) ;
        fclose ( fw );
        return 1;
      }
    }
    return 0;
  }

  void CImageView::RButton()
  {
    if ( !m_bRButtonDlgModal )
    {
      m_bRButtonDlgModal = TRUE ;
      m_RButtonDlg.DoModal() ;
      m_bRButtonDlgModal = FALSE ;

      m_BigViewWnd.ShowWindow( ( m_RButtonDlg.m_iViewMode ) ? SW_SHOW : SW_HIDE ) ;          	
      if ( m_RButtonDlg.m_iViewMode == 0 )
      {
        m_WindisAlive = 0;
        m_RButtonDlg.m_SpotSearchCoordinates.Empty() ;
        m_RButtonDlg.m_SpotSearch = CPoint(0,0) ;
      }
    }
    else
    {
      m_RButtonDlg.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
      m_RButtonDlg.ShowWindow(SW_SHOW);	  
    }
    //RButDlgReaction();
    SetTimer(VIEW_PARAM_WINDOW,1000,NULL) ;
  }
  void CImageView::IniExpoandGain()
  {
    for (int iCam = 0; iCam < 2 ; iCam++)
    {
      SetGain(0,iCam);
      Sleep(50);
      SetExposure(ROUND(m_dScanTime*6),TRUE,iCam);
    }


    //SavePictures(TRUE);
    //    if (!m_bDrawLine)
    //      m_bDrawLine=TRUE;
    //    else
    //      m_bDrawLine=FALSE;
    // 
    //    HPEN hPen,hLinePen;
    //    hLinePen = CreatePen(PS_SOLID,5,RGB(255,0,0));
    //    CDC* cdc = GetDC(/*m_hWnd*/);
    //    hPen  = (HPEN)SelectObject(cdc->m_hDC,hLinePen);  
    //    CRect rc;
    //    GetWindowRect(rc);
    // 
    //    rc.bottom-=rc.top;
    //    rc.top = 0;
    //    rc.right-=rc.left;
    //    rc.left = 0;
    // 
    //    MoveToEx(cdc->m_hDC,rc.right-150,rc.bottom - rc.bottom/2,NULL);
    //    LineTo(cdc->m_hDC,rc.right+50,(rc.bottom + rc.top)/2);


  }

  void CImageView::OnCancel()
  {
    int bquit;
    bquit=1;
  }
  void CImageView::OnDontUseExternalImaging()
  {
    CRegistry Reg("File Company\\OpticJig") ;
    CString ScaleName ;
    bool Invalidate ;
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      m_MeasSets[i].m_GraphMode = 0 ;
      m_MeasSets[i].EnableGetImagesFromLAN( false ) ;
      m_MeasSets[i].EnableSendResultsToLAN( false ) ;
      m_MeasSets[i].EnableSendImagesToLAN( false ) ;

      ScaleName.Format( "ScaleX_pix_per_mm_Cam%d" , m_MeasSets.GetCount() - i -1) ;
      double dScaleX = Reg.GetRegiDouble( "Scales" , ScaleName , 1.0 ) ;
      ScaleName.Format( "ScaleY_pix_per_mm_Cam%d" , m_MeasSets.GetCount() - i -1) ;
      double dScaleY = Reg.GetRegiDouble( "Scales" , ScaleName , 1.0 ) ;
      double dScale = 0.5 * ( fabs(dScaleX) + fabs(dScaleY) ) ;
      if ( dScale != 0.0 )
        m_MeasSets[i].SetScale( 1000./dScale , _T("um") ) ;




      CRegistry ImReg("File Company\\ImCNTL") ;
      CString sTemplateFile;
      if (m_iSearchArea == BLOCKS_5)
      {
        CString sPattern = _T("TemplateAVT_5Cubes.txt");
        sTemplateFile = ImReg.GetRegiString( "CameraComm" , 
          "TemplatePath" , _T("C:\\ATJ\\TVObjectsTemplates\\") ) ;
        sTemplateFile+=sPattern;
      }
      else if (m_iSearchArea == BLOCKS_9_FULL)
      {
        CString sPattern = _T("TemplateAVT_9CubesFull.txt");
        sTemplateFile = ImReg.GetRegiString( "CameraComm" , 
          "TemplatePath" , _T("C:\\ATJ\\TVObjectsTemplates\\") ) ;
        sTemplateFile+=sPattern;
      }
      else if  (m_iSearchArea == BLOCKS_9_HALF)
      {
        CString sPattern = _T("TemplateAVT_9CubesHalf.txt");
        sTemplateFile = ImReg.GetRegiString( "CameraComm" , 
          "TemplatePath" , _T("C:\\ATJ\\TVObjectsTemplates\\") ) ;
        sTemplateFile+=sPattern;
      }
      else                                                                  
      {
        CString sPattern = _T("TemplateAVT_5Cubes.txt");
        sTemplateFile = ImReg.GetRegiString( "CameraComm" , 
          "TemplatePath" , _T("C:\\ATJ\\TVObjectsTemplates\\") ) ;
        sTemplateFile+=sPattern;
      }



      CString Template ;
      //CString FileName ;
      //FileName.Format( "Template%d.txt" , i ) ;
      FILE * fr = fopen( sTemplateFile , "r" ) ;
      if ( fr )
      {
        char * pBuf = Template.GetBuffer( 10000 ) ;
        int iLen = fread( pBuf , 1 , 10000 , fr ) ;
        fclose(fr) ;
        Template.ReleaseBuffer( iLen ) ;
        FXPropertyKit pk ;
        m_MeasSets[i].m_pProcessGadget->PrintProperties( pk ) ;
        pk.WriteString( "Template" , Template ) ;
        m_MeasSets[i].m_pProcessGadget->ScanProperties( pk , Invalidate ) ;
      }
    }

  }
  void CImageView::OnSendImagesToLAN()
  {
    CRegistry Reg("File Company\\OpticJig") ;
    CString ScaleName ;
    bool Invalidate ;
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      m_MeasSets[i].m_GraphMode = 2 ;
      m_MeasSets[i].EnableGetImagesFromLAN( false ) ;
      m_MeasSets[i].EnableSendResultsToLAN( false ) ;
      m_MeasSets[i].EnableSendImagesToLAN( true ) ;

      ScaleName.Format( "ScaleX_pix_per_mm_Cam%d" , m_MeasSets.GetCount() - i -1) ;
      double dScaleX = Reg.GetRegiDouble( "Scales" , ScaleName , 1.0 ) ;
      ScaleName.Format( "ScaleY_pix_per_mm_Cam%d" , m_MeasSets.GetCount() - i -1) ;
      double dScaleY = Reg.GetRegiDouble( "Scales" , ScaleName , 1.0 ) ;
      double dScale = 0.5 * ( fabs(dScaleX) + fabs(dScaleY) ) ;
      if ( dScale != 0.0 )
        m_MeasSets[i].SetScale( 1000./dScale , _T("um") ) ;
      CString Template ;
      CString FileName ;
      FileName.Format( "Template%d.txt" , i ) ;
      FILE * fr = fopen( FileName , "r" ) ;
      if ( fr )
      {
        char * pBuf = Template.GetBuffer( 10000 ) ;
        int iLen = fread( pBuf , 1 , 10000 , fr ) ;
        fclose(fr) ;
        Template.ReleaseBuffer( iLen ) ;
        FXPropertyKit pk ;
        m_MeasSets[i].m_pProcessGadget->PrintProperties( pk ) ;
        pk.WriteString( "Template" , Template ) ;
        m_MeasSets[i].m_pProcessGadget->ScanProperties( pk , Invalidate ) ;
      }
    }
  }
  void CImageView::OnTakeImagesFromLAN()
  {
    CRegistry ImReg("File Company\\ImCNTL") ;
    CRegistry OpticJigReg("File Company\\OpticJig");
    CString ScaleNameY,ScaleNameX ;
    CString FileName = ImReg.GetRegiString( "CameraComm" , "TemplatePath" , 
      "TemplateForWhel24.txt" ) ;
    CString Template ;
    FILE * fr = fopen( FileName , "r" ) ;
    if ( fr )
    {
      char * pBuf = Template.GetBuffer( 10000 ) ;
      int iLen = fread( pBuf , 1 , 10000 , fr ) ;
      fclose(fr) ;
      Template.ReleaseBuffer( iLen ) ;
    }
    bool Invalidate ;
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      m_MeasSets[i].m_GraphMode = 1 ;
      m_MeasSets[i].EnableGetImagesFromLAN( true ) ;
      //m_MeasSets[i].EnableSendResultsToLAN( true ) ; - don't send results 
      m_MeasSets[i].EnableSendImagesToLAN( false ) ;
      //ScaleName.Format( "Scale_ExtCam_pix_per_mm_Cam%d" , m_MeasSets.GetCount() - i -1) ;
      ScaleNameX.Format("Scale CarmelCamera_X_per_mm_cam%d",m_MeasSets.GetCount() - i - 1);
      ScaleNameY.Format("Scale CarmelCamera_Y_per_mm_cam%d",m_MeasSets.GetCount() - i - 1);
      double dScaleX = OpticJigReg.GetRegiDouble( "Scales" , ScaleNameX , 1575.5 ) ;
      double dScaleY = OpticJigReg.GetRegiDouble( "Scales" , ScaleNameY , 1575.5 ) ;
      double dScale = 0.5 * ( fabs(dScaleX) + fabs(dScaleY) ) ;
      if ( dScale != 0.0 )
        m_MeasSets[i].SetScale( 1000./dScale , _T("um") ) ;
      if ( !Template.IsEmpty() )
      {
        FXPropertyKit pk ;
        if ( m_MeasSets[i].m_pProcessGadget->PrintProperties( pk ) )
        {
          pk.WriteString( "Template" , Template ) ;
          m_MeasSets[i].m_pProcessGadget->ScanProperties( pk , Invalidate ) ;
        }
      }
    }
  }

  void CImageView::UseSoliosCamera()
  {
    return;
    CRegistry ImReg("File Company\\ImCNTL") ;
    CString sTemplateFile;
    if (m_iSearchArea == BLOCKS_5)
    {
      CString sPattern = ImReg.GetRegiString("CameraComm",
        "TemplateForDiffr", _T("TemplateForLPHDalsa_5Cubes.txt"));
      sTemplateFile = ImReg.GetRegiString( "CameraComm" , 
        "TemplatePath" , _T("C:\\ATJ\\TVObjectsTemplates\\") ) ;
      sTemplateFile+=sPattern;
    }
    else if (m_iSearchArea == BLOCKS_9_FULL)
    {
      CString sPattern = _T("TemplateForDalsa_9CubesFull.txt");
      sTemplateFile = ImReg.GetRegiString( "CameraComm" , 
        "TemplatePath" , _T("C:\\ATJ\\TVObjectsTemplates\\") ) ;
      sTemplateFile+=sPattern;
    }
    else if  (m_iSearchArea == BLOCKS_9_HALF)
    {
      CString sPattern = _T("TemplateForDalsa_9CubesHalf.txt");
      sTemplateFile = ImReg.GetRegiString( "CameraComm" , 
        "TemplatePath" , _T("C:\\ATJ\\TVObjectsTemplates\\") ) ;
      sTemplateFile+=sPattern;
    }
    else                                                                  
    {
      CString sPattern = _T("TemplateForDalsa_5Cubes.txt");
      sTemplateFile = ImReg.GetRegiString( "CameraComm" , 
        "TemplatePath" , _T("C:\\ATJ\\TVObjectsTemplates\\") ) ;
      sTemplateFile+=sPattern;
    }

    //Robot Calibration mode
    int bUseBigSpot = ImReg.GetRegiInt( "CameraComm" , "UseBigSpotMode(RobotCalibration)" , 0 ) ;
    if (bUseBigSpot)
    {
      CString sPattern = _T("TemplateForDalsa_5Cubes_BigSpot.txt");
      sTemplateFile = ImReg.GetRegiString( "CameraComm" , 
        "TemplatePath" , _T("C:\\ATJ\\TVObjectsTemplates\\") ) ;
      sTemplateFile+=sPattern;

      m_ImParam.m_iDiffractionRadius = m_ImParam.m_iDiffractionRadius_Y = 100;
      m_ImParam.m_bShowRadDisable = TRUE;
    }


    CRegistry OpticJigReg("File Company\\OpticJig");
    CString ScaleNameY,ScaleNameX ;
    //CString FileName = ImReg.GetRegiString( "CameraComm" , "TemplateForWhel24" , "TemplateForWhel24.txt" ) ;
    CString Template ;
    FILE * fr = fopen( sTemplateFile/*FileName*/ , "r" ) ;
    if ( fr )
    {
      char * pBuf = Template.GetBuffer( 10000 ) ;
      int iLen = fread( pBuf , 1 , 10000 , fr ) ;
      fclose(fr) ;
      Template.ReleaseBuffer( iLen ) ;
    }
    bool Invalidate ;
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      //m_MeasSets[i].m_GraphMode = 1 ;
      //     m_MeasSets[i].EnableGetImagesFromLAN( false ) ;
      //     m_MeasSets[i].EnableSendResultsToLAN( false ) ; //- don't send results 
      //     m_MeasSets[i].EnableSendImagesToLAN( false ) ;

      ScaleNameX.Format("Scale CarmelCamera_X_per_mm_cam%d",m_MeasSets.GetCount() - i - 1);
      ScaleNameY.Format("Scale CarmelCamera_Y_per_mm_cam%d",m_MeasSets.GetCount() - i - 1);
      double dScaleX = OpticJigReg.GetRegiDouble( "Scales" , ScaleNameX , 1575.5 ) ;
      double dScaleY = OpticJigReg.GetRegiDouble( "Scales" , ScaleNameY , 1575.5 ) ;
      double dScale = 0.5 * ( fabs(dScaleX) + fabs(dScaleY) ) ;
      if ( dScale != 0.0 )
        m_MeasSets[i].SetScale( 1000./dScale , _T("um") ) ;
      if ( !Template.IsEmpty() )
      {
        FXPropertyKit pk ;
        if ( m_MeasSets[i].m_pProcessGadget->PrintProperties( pk ) )
        {
          pk.WriteString( "Template" , Template ) ;
          m_MeasSets[i].m_pProcessGadget->ScanProperties( pk , Invalidate ) ;
        }
      }
    }
  }

  void CImageView::SetBMPPath(CString sBMPPath)
  { 
    FXPropertyKit pk ;
    for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    {
      CString sBMPRender;
      sBMPRender.Format("BMPRender%d",i+1);
      CRenderGadget  * m_pBMPRenderGadget  = (CRenderGadget *) m_pBuilder->GetGadget( sBMPRender );
      m_pBMPRenderGadget->PrintProperties( pk ) ;
      CString sOut;

      if (sBMPPath.GetLength()==0)
        sOut = m_ImParam.m_ImageSaveDir;
      else
        sOut = sBMPPath;

      pk.WriteString("FileName",sOut);
      bool Invalidate = true ;
      m_pBMPRenderGadget->ScanProperties( pk,Invalidate ) ;
    }
  }

  BOOL CImageView::CheckIfStopAutoSaving()
  {
    if(m_iSavedImg>100)
      return TRUE;
    else
      return FALSE;
  } 

  void CImageView::OnViewHelp()
  {
    CHTMLDlg HtmlHelpMe;
    char buff[300];
    GetCurrentDirectory(300,buff);
    CString path(buff);
    path.Append("\\ImCNTLHelp.htm");
    HtmlHelpMe.Init(path);
    HtmlHelpMe.SetHeight(600);
    HtmlHelpMe.SetWidth(900);
    HtmlHelpMe.Show();
  }

  void CImageView::SaveLastPicture( int iCam )
  {
    CMeasurementSet * pSet = &m_MeasSets[iCam] ;

    CQuantityFrame * pQuan = CQuantityFrame::Create( -1 ) ;
    pQuan->ChangeId( 0 ) ;
    if ( !m_pBuilder->SendDataFrame( pQuan , pSet->m_BMPBufferControlPinName ) ) 
      pQuan->Release( pQuan ) ;                                         

  }
  void CImageView::EnableVideoWriting(bool  bEnable , int iCam)
  {
    //for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    //{
    CMeasurementSet * pSet = &m_MeasSets[iCam] ;
    CBooleanFrame * pQuan = CBooleanFrame::Create( bEnable ) ;
    pQuan->ChangeId( 0 ) ;
    if ( !m_pBuilder->SendDataFrame( pQuan , pSet->m_VideoSwitchName ) )
      pQuan->Release( pQuan ) ; 
    //}
  }

  void CImageView::CloseVideoFile(int iCam)
  {
    //for ( int i = 0 ; i < m_MeasSets.GetCount() ; i++ )
    //{
    CMeasurementSet * pSet = &m_MeasSets[iCam] ;
    CDataFrame * pQuan = CDataFrame::Create( TRUE ) ;
    pQuan->ChangeId( -1 ) ;
    if ( !m_pBuilder->SendDataFrame( pQuan , pSet->m_AviRenderInputName ) )
      pQuan->Release( pQuan ) ;    
    //}
  }

  void CImageView::LogMsg3(LPCTSTR Msg)
  {

  }
