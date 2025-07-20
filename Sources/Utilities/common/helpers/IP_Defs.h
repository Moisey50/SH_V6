#pragma once 

// enum defining the return codes for the image processing module
// add enums as needed
typedef enum
{
	IP_SUCCESS,			// no error
	IP_TRAY_EMPTY,		// no flower found on tray
	IP_INVALID_PARAM,	// invalid parameter(s)
	IP_INVALID_MODELS,
	IP_HEAD_IMAGE_CLIPPED, // Head image clipped by tray edge
	IP_NO_WIDTH_SAMPLES, // not enough samples for width measurement 
	IP_FILE_OPEN_FAILURE,
	IP_GENERAL_ERROR,
} IP_RETURN_CODE;

// enum defining the various log levels
enum IP_LOG_LEVEL
{
	IPLOG_DEBUG,
	IPLOG_INFO,
	IPLOG_WARNING,
	IPLOG_ERROR,
	IPLOG_FATAL,

	IPLOG_END		// used for value validation
};

// enum used to define the layout of the different cameras
enum IP_CAM_POSITION
{
	FULL_IR,			// complete tray - IR
	FULL_COLOR,			// complete tray - color
	HEAD_CENTER_COLOR,	// tray/flower head - centered
	HEAD_RIGHT_COLOR,	// tray/flower head - right side (from bottom of image)
	HEAD_LEFT_COLOR,	// tray/flower head - left side (from top of image)
};

// enum describing the supported images formats
enum IP_IMAGE_FORMAT
{
	IP_FORMAT_MONO8,
	IP_FORMAT_YUV422_PACKED,
	IP_FORMAT_UNKNOWN,
};

// structure used for sending image data
struct ip_ImageData {
	int width;				// image width
	int height;				// image height
	IP_IMAGE_FORMAT format;	// format of image buffer
	BYTE* data;				// image data buffer
};

// structure used to define a specific pixel location on the image buffer
// bottom-left pixel is (0,0)
struct ip_PixelLoc
{
	int x;
	int y;
};

// structure used to indicate the ROI for a specific image/camera
struct ip_RoiPoints {
	ip_PixelLoc topleft;
	ip_PixelLoc topright;
	ip_PixelLoc bottomleft;
	ip_PixelLoc bottomright;
};

// typedef declaration for the logger callback function
extern "C" typedef void(__stdcall * loggerCallback)(IP_LOG_LEVEL, char*);

#define IP_MAX_PARAMS	7	// maximum number of measured parameters

#define IP_MAX_CAMS 4 // iwilf added to support ipTest: 26.3.14

#define DECFACTOR 4

