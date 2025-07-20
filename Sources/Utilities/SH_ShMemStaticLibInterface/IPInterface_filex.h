#pragma once

#include <windows.h>
#include "IP_Defs.h"

// ipInitialize_filex - initialize module
// [in] loggerCallback - pointer to function to be called for logging
// [return] - return code enum
IP_RETURN_CODE ipInitialize_filex( loggerCallback pLoggerFunc );


// ipTerminate_filex - terminate module
// [return] - return code enum
IP_RETURN_CODE ipTerminate_filex();


// ipAnalyzeCalibration_filex - validate that calibration images are okay - used during initial calibration process
// [in] imageCount - number of cameras/files (if "1" - check validity of single image, if more - also check correlation validity)
// [in] calibFiles - list of caibration image files (BMP)
// [in] camsLayout - list of enums indicating the layout (position) of each camera
// [return] - return code enum
IP_RETURN_CODE ipAnalyzeCalibration_filex(
  int imageCount, char* calibFiles[], IP_CAM_POSITION camsLayout[]);


// ipSetFlower_filex - sets flower type
// [in] flowerName - flower name/type
// [in] camCount - count of calibration file
// [in] calibFile - array of calibration image file names (BMP)
// [in] camsLayout - list of enums indicating the layout (position) of each camera
// [in] roiPoints - array of points for defining ROI
// [in] configFile - path to file name containing a flower specific configuration parameters (NULL if not specified)
// [return] - return code enum
IP_RETURN_CODE ipSetFlower_filex(
  char* flowerName, int camCount, char* calibFiles[], IP_CAM_POSITION camsLayout[], 
	ip_RoiPoints roiPoints[], char* configFile = NULL);


// ipGetParametersList_filex - returns the list of parameters names for current flower
// [in] flowerName - current flower's name
// [out] count - number of parameters
// [out] parametersList - list of parameters names
// [return] - return code enum
IP_RETURN_CODE ipGetParametersList_filex(
  char* flowerName, int* count, char parametersList[][100]);


// ipAnalyzeFlower_filex - analyze a set of images and calculate required parameters
// [in] ticket - tag for set
// [in] flowerName - current flower's name
// [in] count - count of images
// [in] images - array of imageData structures. caller will free buffers.
// [out] params - array of floats to be filled. order is same as list returned in ipGetParametersList.
// [in] showPictures - optional parameter to control image display. default is -1 to disable. positive value will cause a wait for  keystroke up to 'value' milisec (0 wait "forever").
// [return] - return code enum (if tray is empry, return IP_TRAY_EMPTY)
IP_RETURN_CODE ipAnalyzeFlower_filex(
  int ticket, char *flowerName, int count, ip_ImageData images[],
  float params[IP_MAX_PARAMS], int showPictures=-1);

// Interface for simulation - get image in ip_ImageData format from external source

IP_RETURN_CODE ipGetSimulationImages( char * pFlowerName ,
  int& iNImages , void * pBufForImages , DWORD& iBufSize ) ;

DWORD GetImageBufferSize( ip_ImageData * pImage ) ;

DWORD GetSHImageBufferSize( ip_ImageData * pImage ) ;

LPCTSTR ipGetErrorString_filex( IP_RETURN_CODE ErrCode ) ;
