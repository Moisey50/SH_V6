// SH_ShMemStaticLibInterface.cpp : Defines the functions for the static library.
//
#include "stdafx.h"
#include <tchar.h>
#include "ShMemControl.h"
#include "IPInterface_filex.h"
#include "TectoMsgs.h"
#include "registry.h"


#define BI_YUV411   0x31313459
#define BI_YUV422   0x32323459
#define BI_Y411     0x31313459
#define BI_YUV9     0x39555659
#define BI_YUV12    0x32315659
#define BI_I420     0x30323449
#define BI_UYVY			0x59565955
#define BI_YUY2			0x32595559
#define BI_Y8   	  0x20203859
#define BI_Y800     0x30303859
#define BI_Y16      0x20363159
#define BI_Y12      0x20323159
#define BI_YP12     0x32315059
#define BI_MJPG 	  0x47504a4d
#define BI_H264     0x34363248
#define BI_RGB48    0x38344752
#define BI_NV12     0x3231564E
#define I50_FORMAT  0x30355649
#define I32_FORMAT  0x32335649
//#define I420_FORMAT 0x30323449
#define NV12_FORMAT 0x3231564E

CShMemControl * g_pTectoShMem = NULL ;

static char ShMemAreaName[100] , InEventName[100] , OutEventName[100] ;
static int iShMemInSize_KB = 4096 , iShMemOutSize_KB = 4096 ;

static DWORD g_dwTimeout_ms = 0 ; // for debugging

DWORD GetImageBufferSize( ip_ImageData * pImage )
{
  return ( ( pImage->width * pImage->height ) * ( ( pImage->format == IP_FORMAT_YUV422_PACKED ) ? 2 : 1 ) ) ;
}

DWORD GetSHImageBufferSize( ip_ImageData * pImage )
{
  DWORD dwImageSize =  ( pImage->width * pImage->height ) ;
  switch ( pImage->format )
  {
    case BI_YUV422:
    case BI_UYVY:
    case BI_I420:
    case BI_YUY2:
    case IP_FORMAT_YUV422_PACKED: dwImageSize *= 2 ; break ;
    case BI_YUV12:
    case BI_NV12:
    case BI_YUV411: dwImageSize = ( dwImageSize * 3 ) / 2 ; break ;
    case BI_YUV9: dwImageSize = ( dwImageSize * 9 ) / 8 ; break ;
  }
  return dwImageSize ;
}

LPCTSTR ipGetErrorString_filex( IP_RETURN_CODE ErrCode )
{
  switch ( ErrCode )
  {
    case IP_SUCCESS:	         return "Success" ;
    case IP_TRAY_EMPTY:        return "Tray Empty" ;
    case IP_INVALID_PARAM:     return "Invalid Params" ;
    case IP_INVALID_MODELS:    return "Invalid Models" ;
    case IP_HEAD_IMAGE_CLIPPED:return "Head Image Clipped" ;
    case IP_NO_WIDTH_SAMPLES:  return "No Width Samples" ;
    case IP_FILE_OPEN_FAILURE: return "File Open Error" ;
    case IP_GENERAL_ERROR:     return "General Error" ;
  }
  return "UNKNOWN ERROR" ;
}


DWORD GetImageHeaderSize()
{
  return sizeof( ip_ImageData ) - sizeof( BYTE * ) ;
}

loggerCallback g_Logger = NULL ;

// ipInitialize - initialize module
// [in] loggerCallback - pointer to function to be called for logging
// [return] - return code enum
static IP_RETURN_CODE ipInitialize( loggerCallback pLoggerFunc )
{
  // Get shared memory parameters
  CRegistry Reg( "TheFileX\\UI_NViews" );
  LPCTSTR pTmp = Reg.GetRegiString( "Tecto" , "ShMemAreaName" , "TectoSharedArea" ) ;
  strcpy_s( ShMemAreaName , pTmp ) ;
  pTmp = Reg.GetRegiString( "Tecto" , "InEventName" , "TectoInEvent" ) ;
  strcpy_s( InEventName , pTmp ) ;
  pTmp = Reg.GetRegiString( "Tecto" , "OutEventName" , "TectoOutEvent" ) ;
  strcpy_s( OutEventName , pTmp ) ;
  iShMemInSize_KB = Reg.GetRegiInt( "Tecto" , "ShMemInSize_KB" , iShMemInSize_KB ) ;
  iShMemOutSize_KB = Reg.GetRegiInt( "Tecto" , "ShMemOutSize_KB" , iShMemOutSize_KB ) ;

  // Communication with graph init
  if ( g_pTectoShMem )
  {
    delete g_pTectoShMem ;
    g_pTectoShMem = NULL ;
  }
  g_pTectoShMem = new CShMemControl( 1024 , 1024 + ( iShMemInSize_KB * 1024 ) , 
    1024 + ( iShMemInSize_KB + iShMemOutSize_KB) * 1024 , ShMemAreaName,
    InEventName , OutEventName ) ;
  if ( !g_pTectoShMem )
    return IP_GENERAL_ERROR ;

  int iMsgBufLen = 4096 ;
  TectoMsg * pInitMsg = TectoMsg::CreateMsg( TM_Init , iMsgBufLen ) ;
  int iRes = g_pTectoShMem->SendRqWaitAndReceiveAnswer( pInitMsg , pInitMsg->GetMsgLen() , 1000 , iMsgBufLen ) ;
  if ( ( iRes < 0 ) || ( pInitMsg->m_Id != TM_OK ) )
  {
    delete g_pTectoShMem ;
    g_pTectoShMem = NULL ;
    iRes = IP_GENERAL_ERROR ;
  }
  else
  {
  // Init logging
    g_Logger = pLoggerFunc ;
    iRes = IP_SUCCESS ;
    if ( g_Logger )
      g_Logger( IPLOG_INFO , (char*)("Initialization OK\n") ) ;
  }
  delete pInitMsg ;

  return (IP_RETURN_CODE) iRes ;
}

IP_RETURN_CODE ipInitialize_filex( loggerCallback pLoggerFunc )
{
  return ipInitialize( pLoggerFunc ) ;
}

// ipTerminate - terminate module
// [return] - return code enum
static IP_RETURN_CODE ipTerminate()
{
  if ( g_pTectoShMem )
  {
    int iMsgBufLen = 4096 ;
    TectoMsg * pTerminateMsg = TectoMsg::CreateMsg( TM_Terminate , 4096 ) ;
    int iRes = g_pTectoShMem->SendRqWaitAndReceiveAnswer( 
      pTerminateMsg , pTerminateMsg->GetMsgLen() , 1000 , iMsgBufLen ) ;
    delete pTerminateMsg ;
    delete g_pTectoShMem ;
    g_pTectoShMem = NULL ;
    if ( g_Logger )
    {
      g_Logger( IPLOG_INFO , ( char* ) ( "Server is Disconnected\n" ) ) ;
      g_Logger = NULL ;
    }
  }
  return IP_SUCCESS ;
}

IP_RETURN_CODE ipTerminate_filex()
{
  return ipTerminate() ;
}

// ipAnalyzeCalibration - validate that calibration images are okay - used during initial calibration process
// [in] imageCount - number of cameras/files (if "1" - check validity of single image, 
//                   if more - also check correlation validity)
// [in] calibFiles - list of calibration image files (BMP)
// [in] camsLayout - list of enums indicating the layout (position) of each camera
// [return] - return code enum
static IP_RETURN_CODE ipAnalyzeCalibration( int imageCount ,
  char* calibFiles[] , IP_CAM_POSITION camsLayout[] )
{
  if ( imageCount )
  {
    int iMsgBufLen = 4096 ;
    TectoMsg * pCalibMsg = TectoMsg::CreateMsg( TM_Calibration , sizeof( CalibContent ) ) ;

    CalibContent * pContent = (CalibContent *) pCalibMsg->GetData() ;
    pContent->m_iNFiles = imageCount ;
    memcpy( pContent->m_CamPositions , camsLayout , imageCount * sizeof( IP_CAM_POSITION ) ) ;
    int iCopiedLen = 0 ;
    int iFileIndex = 0 ;
    do 
    {
      strcpy_s( pContent->m_CalibFileNames[iFileIndex] , 
        sizeof( pContent->m_CalibFileNames[ iFileIndex ] ) , calibFiles[ iFileIndex ] ) ;
    } while ( ++iFileIndex < imageCount );
    
   
    int iRes = g_pTectoShMem->SendRqWaitAndReceiveAnswer( pCalibMsg ,
      pCalibMsg->GetMsgLen() , g_dwTimeout_ms ? g_dwTimeout_ms : 2000 , iMsgBufLen ) ;
    if ( iRes < 0 )
      iRes = IP_GENERAL_ERROR ;
    else if ( pCalibMsg->m_Id != TM_OK )
    {
      if ( pCalibMsg->m_Id == TM_FileOpenError )
        iRes = IP_FILE_OPEN_FAILURE ;
      else
        iRes = IP_GENERAL_ERROR ;
    }
    else
      iRes = IP_SUCCESS ;

    if ( g_Logger )
    {
      g_Logger( iRes == IP_SUCCESS ? IPLOG_INFO : IPLOG_ERROR , ( char* ) pCalibMsg->GetData() ) ;
    }
    delete pCalibMsg ;
    return ( IP_RETURN_CODE ) iRes ;
  }

  return IP_INVALID_PARAM ;
}

IP_RETURN_CODE ipAnalyzeCalibration_filex(
  int imageCount , char* calibFiles[] , IP_CAM_POSITION camsLayout[] )
{
  return ipAnalyzeCalibration( imageCount , calibFiles , camsLayout ) ;
}

// ipSetFlower - sets flower type
// [in] flowerName - flower name/type
// [in] camCount - count of calibration file
// [in] calibFile - array of calibration image file names (BMP)
// [in] camsLayout - list of enums indicating the layout (position) of each camera
// [in] roiPoints - array of points for defining ROI
// [in] configFile - path to file name containing a flower specific configuration parameters (NULL if not specified)
// [return] - return code enum


static IP_RETURN_CODE ipSetFlower( char* flowerName ,
  int camCount , char* calibFiles[] , IP_CAM_POSITION camsLayout[] ,
  ip_RoiPoints roiPoints[] , char* configFile )
{
  int iMsgBufLen = 4096 ;
  TectoMsg * pSetFlowerMsg = TectoMsg::CreateMsg( TM_SetFLower , sizeof( FlowerSetContent ) ) ;

  FlowerSetContent * pContent = ( FlowerSetContent * ) pSetFlowerMsg->GetData() ;
  strcpy_s( pContent->m_FlowerName , flowerName ) ;
  pContent->m_iCamCount= camCount ;
  memcpy( pContent->m_CamPositions , camsLayout , camCount * sizeof( IP_CAM_POSITION ) ) ;
  memcpy( pContent->m_ROI , roiPoints , camCount * sizeof( RoiPoints ) ) ;
  int iCopiedLen = 0 ;
  int iFileIndex = 0 ;
  do
  {
    strcpy_s( pContent->m_CalibFileNames[ iFileIndex ] , calibFiles[ iFileIndex ] ) ;
  } while ( ++iFileIndex < camCount );
  if ( configFile && *configFile )
    strcpy_s( pContent->m_ConfigFileName , sizeof( pContent->m_ConfigFileName ) , configFile ) ;
  else
    pContent->m_ConfigFileName[ 0 ] = 0 ;

  int iRes = g_pTectoShMem->SendRqWaitAndReceiveAnswer( pSetFlowerMsg , 
    pSetFlowerMsg->GetMsgLen() , g_dwTimeout_ms? g_dwTimeout_ms : 2000 , iMsgBufLen ) ;
  if ( iRes < 0 )
    iRes = IP_GENERAL_ERROR ;
  else if ( pSetFlowerMsg->m_Id != TM_OK )
  {
    if ( pSetFlowerMsg->m_Id == TM_FileOpenError )
      iRes = IP_FILE_OPEN_FAILURE ;
    else
      iRes = IP_GENERAL_ERROR ;
  }
  else
    iRes = IP_SUCCESS ;

  return ( IP_RETURN_CODE ) iRes ;
}

IP_RETURN_CODE ipSetFlower_filex(
  char* flowerName , int camCount , char* calibFiles[] , IP_CAM_POSITION camsLayout[] ,
  ip_RoiPoints roiPoints[] , char* configFile )
{
  return ipSetFlower( flowerName , camCount , calibFiles ,
    camsLayout , roiPoints , configFile ) ;
}

// ipGetParametersList - returns the list of parameters names for current flower
// [out] count - number of parameters
// [out] parametersList - list of parameters names
// [return] - return code enum
IP_RETURN_CODE ipGetParametersList_filex( char* flowerName ,
  int* count , char parametersList[][ 100 ] )
{
  int iMsgBufLen = 4096 ;
  TectoMsg * pParamListMsg = TectoMsg::CreateMsg( TM_GetParameterList , sizeof( ParameterListContent ) ) ;
  
  ParameterListContent * pContent = ( ParameterListContent * ) pParamListMsg->GetData() ;
  memset( pContent , 0 , sizeof( ParameterListContent ) ) ;
  strcpy_s( pContent->m_FlowerName , flowerName ) ;

  int iRes = g_pTectoShMem->SendRqWaitAndReceiveAnswer( pParamListMsg ,
    pParamListMsg->GetMsgLen() , g_dwTimeout_ms ? g_dwTimeout_ms : 2000 , iMsgBufLen ) ;
  if ( pParamListMsg->m_Id == TM_OK )
  {
    *count = pContent->m_iParamCount ;
    for ( int i = 0 ; i < *count  ; i++ )
      strcpy_s( parametersList[i] , pContent->m_Parameters[ i ] ) ;
    iRes = IP_SUCCESS ;
  }
  else
  {
    strcpy_s( flowerName , 128 , pContent->m_Diagnostics ) ;
    iRes = IP_GENERAL_ERROR ;
  }

  return ( IP_RETURN_CODE ) iRes ;
}

// ipAnalyzeFlower - analyze a set of images and calculate required parameters
// [in] ticket - tag for set
// [in] count - count of images
// [in] images - array of imageData structures. caller will free buffers.
// [out] params - array of floats to be filled. order is same as list returned in ipGetParametersList.
// [in] showPictures - optional parameter to control image display. default is -1 to disable. 
//              positive value will cause a wait for  keystroke up to 'value' milisec (0 wait "forever").
// [return] - return code enum (if tray is empty, return IP_TRAY_EMPTY)
static IP_RETURN_CODE ipAnalyzeFlower( int ticket ,
  char *flowerName , int count , ip_ImageData images[] ,
  float params[ IP_MAX_PARAMS ] , int showPictures )
{
  int iSizeOfImages = 0 ;
  DWORD ImageHeaderSize = sizeof( ip_ImageData ) - sizeof( BYTE * ) ;
  for ( int i = 0 ; i < count ; i++ )
  {
    int iImageSize = GetImageHeaderSize() + GetImageBufferSize( &images[ i ] ) ;
    iSizeOfImages += iImageSize ;
  }
  int iMsgDataSize = sizeof( AnalyzeFlowerContent) - 1 + iSizeOfImages ;
  TectoMsg * pAnalyzeFlowerMsg = TectoMsg::CreateMsg( TM_AnalyzeFlower , iMsgDataSize ) ;
  if ( pAnalyzeFlowerMsg ) // allocation and size OK
  {
    AnalyzeFlowerContent * pContent = ( AnalyzeFlowerContent * ) pAnalyzeFlowerMsg->GetData() ;
    pContent->m_iTicket = ticket ;
    pContent->m_iImageCount = count ;
    strcpy_s( pContent->m_FlowerName , flowerName ) ;
    BYTE * pImageData = pContent->m_Images ;
    for ( int i = 0 ; i < count ; i++ )
    {
      memcpy( pImageData , &images[ i ] , ImageHeaderSize ) ;
      pImageData += ImageHeaderSize ;
      DWORD dwImageSize = /*GetImageHeaderSize() + */GetImageBufferSize( &images[ i ] ) ;
      memcpy( pImageData , images[ i ].data , dwImageSize ) ;
      pImageData += dwImageSize ;
//       delete[] images[ i ].data ;
    }


    int iRes = g_pTectoShMem->SendRqWaitAndReceiveAnswer( pAnalyzeFlowerMsg ,
      pAnalyzeFlowerMsg->GetMsgLen() , g_dwTimeout_ms ? g_dwTimeout_ms : 500 , iMsgDataSize ) ;

    if ( pAnalyzeFlowerMsg->m_Id == TM_OK )
      iRes = IP_SUCCESS ;
    else if ( pAnalyzeFlowerMsg->m_Id == TM_TrayEmpty )
      iRes = IP_TRAY_EMPTY ;
    else if ( pAnalyzeFlowerMsg->m_Id == TM_NotInRange )
      iRes = IP_INVALID_PARAM ;
    else if ( pAnalyzeFlowerMsg->m_Id == TM_ClippedByTray )
      iRes = IP_HEAD_IMAGE_CLIPPED ;
    else
      iRes = IP_GENERAL_ERROR ;

    if ( (pAnalyzeFlowerMsg->m_Id == TM_OK)
      || (pAnalyzeFlowerMsg->m_Id == TM_ERROR ) )
    {
      LPCTSTR pAnswerAsText = (LPCTSTR) pAnalyzeFlowerMsg->GetData() ;
      LPCTSTR pNumericalAsText = _tcsstr( pAnswerAsText , "Numerical" ) ;
      if ( pNumericalAsText )
      {
        pNumericalAsText = _tcschr( pNumericalAsText , '=' ) ;
        if ( pNumericalAsText )
        {
          memset( params , 0 , sizeof( float ) * IP_MAX_PARAMS ) ;
          int iParNum = 0 ;
          do 
          {
            pNumericalAsText++ ;
            params[ iParNum ] = (float)atof( pNumericalAsText ) ;
          } while ( (++iParNum < IP_MAX_PARAMS) &&
            ( pNumericalAsText = _tcschr( pNumericalAsText , ',' )) );
        }
      }
    }

    if ( g_Logger )
      g_Logger( iRes == IP_SUCCESS ? IPLOG_INFO : IPLOG_ERROR , ( char* ) pAnalyzeFlowerMsg->GetData() ) ;

    delete pAnalyzeFlowerMsg ;

    return ( IP_RETURN_CODE ) iRes ;
  }

  return IP_GENERAL_ERROR ;
}

IP_RETURN_CODE ipAnalyzeFlower_filex(
  int ticket , char *flowerName , int count , ip_ImageData images[] ,
  float params[ IP_MAX_PARAMS ] , int showPictures )
{
  return ipAnalyzeFlower( ticket , flowerName , count , images ,
    params , showPictures ) ;
}


// Interface for simulation - get image in ip_ImageData format from external source
// iNImages - number of read images delivering (output parameter)
// pBufForImages - all images will be saved into this buffer provided by caller
// iBufSize - size of pBufForImages on entry and real necessary space on exit
//            if this size on exit is bigger than allocated buffer (value on entry)
//            then function is failed because buffer size
//    Keep in the mind, that on next call new images will be provided (new files will be read
//    for simulation. Size will be the same with very high probability.

IP_RETURN_CODE ipGetSimulationImages( char * pFlowerName ,  
  int& iNImages , void * pBufForImages , DWORD& dwBufSize )
{
  TectoMsg * pGetImageMsg = TectoMsg::CreateMsg( TM_GetSimulationImage , 4080 * 1024 ) ;
  SimulationImageContent * pSimu = ( SimulationImageContent* ) pGetImageMsg->GetData() ;
  pSimu->m_iNImages = 1 ;
  strcpy_s( pSimu->m_FlowerName , pFlowerName ) ;
  pSimu->m_Images[ 0 ] = 0 ;
  int iMsgLen = pGetImageMsg->GetMsgLen() ;
  int iRes = g_pTectoShMem->SendRqWaitAndReceiveAnswer( pGetImageMsg ,
    pGetImageMsg->GetMsgLen() , g_dwTimeout_ms ? g_dwTimeout_ms : 2000 , iMsgLen ) ;
  if ( iRes < 0 )
    iRes = IP_GENERAL_ERROR ;
  else if ( pGetImageMsg->m_Id == TM_SimuImageArrived )
  {
    DWORD dwFullCopySize = 0 ;
    iNImages = pSimu->m_iNImages ;
    ip_ImageData * pDestImage = ( ip_ImageData * )pBufForImages ;
    ip_ImageData * pSrcImage = (ip_ImageData * )pSimu->m_Images ;
    while ( iNImages )
    {
      DWORD dwImageSize = GetSHImageBufferSize( pSrcImage ) ;
      DWORD dwCopySize = sizeof( ip_ImageData ) + dwImageSize ;
      // Last string will be with file name for ticket extraction
      DWORD dwCopySizeWithFileName = dwCopySize + pSrcImage->width ;
      if ( dwFullCopySize + dwCopySizeWithFileName <= dwBufSize )
      {
        memcpy( pDestImage , pSrcImage , dwCopySize ) ;
        // Copy file name into string after image
        strcpy_s( ((char*)(pDestImage)) + dwCopySize , pDestImage->width , pSimu->m_FlowerName ) ;
        dwFullCopySize += dwCopySizeWithFileName ;
        pDestImage->data = ( LPBYTE ) ( pDestImage + 1 ) ;
        if ( --iNImages )
        {
          pSrcImage = ( ip_ImageData * ) ( ( LPBYTE ) pSrcImage + dwCopySize ) ;
          pDestImage = ( ip_ImageData * ) ( ( LPBYTE ) pDestImage + dwCopySizeWithFileName ) ;
        }
      }
      else
      {
        dwBufSize = dwFullCopySize + dwCopySizeWithFileName ; // it's not correct
        iNImages = pSimu->m_iNImages ;
        delete pGetImageMsg ;
        return IP_GENERAL_ERROR ; // not enough buffer size
      }
    }
    iNImages = pSimu->m_iNImages ;
    iRes = IP_SUCCESS ;
  }
  else
    iRes = IP_GENERAL_ERROR ;

  delete pGetImageMsg ;
  return IP_RETURN_CODE(iRes) ;
}

