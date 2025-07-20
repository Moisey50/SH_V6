#pragma once

#include <intsafe.h>

#include "helpers/IP_Defs.h"
#include "helpers/IPInterface_filex.h"

// //enum defining the return codes for the image processing module
// //add enums as needed
// typedef enum
// {
//   IP_SUCCESS ,			// no error
//   IP_TRAY_EMPTY ,		// no flower found on tray
//   IP_INVALID_PARAM ,	// invalid parameter(s)
//   IP_INVALID_MODELS ,
//   IP_HEAD_IMAGE_CLIPPED , // Head image clipped by tray edge
//   IP_NO_WIDTH_SAMPLES , // not enough samples for width measurement 
//   IP_FILE_OPEN_FAILURE ,
//   IP_GENERAL_ERROR ,
// } IP_RETURN_CODE;

inline LPCTSTR ipGetErrorString( IP_RETURN_CODE ErrCode )
{
  switch( ErrCode )
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

// ipInitialize - initialize module
// [in] loggerCallback - pointer to function to be called for logging
// [return] - return code enum
IP_RETURN_CODE ipInitialize( loggerCallback pLoggerFunc );


// ipTerminate - terminate module
// [return] - return code enum
IP_RETURN_CODE ipTerminate();


// ipAnalyzeCalibration - validate that calibration images are okay - used during initial calibration process
// [in] imageCount - number of cameras/files (if "1" - check validity of single image, if more - also check correlation validity)
// [in] calibFiles - list of calibration image files (BMP)
// [in] camsLayout - list of enums indicating the layout (position) of each camera
// [return] - return code enum
IP_RETURN_CODE ipAnalyzeCalibration( int imageCount , char* calibFiles[] , IP_CAM_POSITION camsLayout[] );


// ipSetFlower - sets flower type
// [in] flowerName - flower name/type
// [in] camCount - count of calibration file
// [in] calibFile - array of calibration image file names (BMP)
// [in] camsLayout - list of enums indicating the layout (position) of each camera
// [in] roiPoints - array of points for defining ROI
// [in] configFile - path to file name containing a flower specific configuration parameters (NULL if not specified)
// [return] - return code enum


IP_RETURN_CODE ipSetFlower( char* flowerName , int camCount , char* calibFiles[] , IP_CAM_POSITION camsLayout[] ,
  ip_RoiPoints roiPoints[] , char* configFile = NULL );


// ipGetParametersList - returns the list of parameters names for current flower
// [out] count - number of parameters
// [out] parametersList - list of parameters names
// [return] - return code enum
IP_RETURN_CODE ipGetParametersList( char* flowerName , int* count , char parametersList[][ 20 ] );


// ipAnalyzeFlower - analyze a set of images and calculate required parameters
// [in] ticket - tag for set
// [in] count - count of images
// [in] images - array of imageData structures. caller will free buffers.
// [out] params - array of floats to be filled. order is same as list returned in ipGetParametersList.
// [in] showPictures - optional parameter to control image display. default is -1 to disable. positive value will cause a wait for  keystroke up to 'value' milisec (0 wait "forever").
// [return] - return code enum (if tray is empty, return IP_TRAY_EMPTY)
IP_RETURN_CODE ipAnalyzeFlower( int ticket , char *flowerName , int count , 
  ip_ImageData images[] , float params[ IP_MAX_PARAMS ] , int showPictures = -1 );


#define IP_MAX_CAMS 4 // iwilf added to support ipTest: 26.3.14

#define DECFACTOR 4

// Interface for simulation - get image in ip_ImageData format from external source

IP_RETURN_CODE ipGetSimulationImages( char * pFlowerName ,
  int& iNImages , void * pBufForImages , DWORD& iBufSize ) ;

DWORD GetImageBufferSize( ip_ImageData * pImage ) ;;

DWORD GetSHImageBufferSize( ip_ImageData * pImage ) ;
