/******************************************************************************
* Filename: MFUtility.h
*
* Description:
* This header file contains common macros and functions that are used in the
* Media Foundation sample applications.
*
* Author:
* Aaron Clauson (aaron@sipsorcery.com)
*
* History:
* 07 Mar 2015	  Aaron Clauson	  Created, Hobart, Australia.
* 03 Jan 2019   Aaron Clauson   Removed managed C++ references.
*
* License: Public Domain (no warranty, use at own risk)
/******************************************************************************/

#include <stdio.h>
#include <tchar.h>
#include <strmif.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mmdeviceapi.h>
#include <wmcodecdsp.h>
#include <wmsdkidl.h>
#include <atlbase.h>

#include <codecvt>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#include <fxfc/fxfc.h>
#include <helpers/CameraData.h>
#include "Callback.h"

using namespace std ;
#define CHECK_HR(hr, msg) if (hr != S_OK) { TRACE(msg); TRACE(" Error: %.2X.\n", hr); goto done; }
#define CHECK_HR_SH(hr, msg) if (hr != S_OK) { TRACE(msg); TRACE(" Error: %.2X.\n", hr); goto done; }

#define CHECKHR_GOTO(x, y) if(FAILED(x)) goto y

#define INTERNAL_GUID_TO_STRING( _Attribute, _skip ) \
if (Attr == _Attribute) \
{ \
	pAttrStr = #_Attribute; \
	C_ASSERT((sizeof(#_Attribute) / sizeof(#_Attribute[0])) > _skip); \
	pAttrStr += _skip; \
	goto done; \
} \

inline std::string WSTR_tostring( LPWSTR pwStr )
{
  string out ;
  while ( *pwStr )
    out += (char) (*(pwStr++) & 0xff) ;
  return out ;
}

typedef struct 
{
  const char * pGroupName ;
  const char * pControlName ;
  int          id ;
} CamControlDef;

LPCSTR GetGUIDNameConst( const GUID& guid ) ;


/**
* Helper function to get a user friendly description for a media type.
* Note that there may be properties missing or incorrectly described.
* @param[in] pMediaType: pointer to the media type to get a description for.
* @@Returns A string describing the media type.
*
* Potential improvements https://docs.microsoft.com/en-us/windows/win32/medfound/media-type-debugging-code.
*/
std::string GetMediaTypeDescription( IMFMediaType* pMediaType );


/**
* Helper function to get a user friendly description for a media type.
* Note that there may be properties missing or incorrectly described.
* @param[in] pMediaType: pointer to the video media type to get a description for.
* @@Returns A string describing the media type.
*
* Potential improvements https://docs.microsoft.com/en-us/windows/win32/medfound/media-type-debugging-code.
*/
std::string GetVideoTypeDescriptionBrief( IMFMediaType* pMediaType , GUID& TypeGuid );

HRESULT FindMatchingVideoType( IMFMediaTypeHandler* pMediaTypeHandler ,
  const GUID& pixelFormat , uint32_t width , uint32_t height ,
  uint32_t fps , IMFMediaType* pOutMediaType );

/*
Lists all the available media types attached to a media type handler.
The media type handler can be acquired from a source reader or sink writer.
* @param[in] pMediaTypeHandler: pointer to the media type handler to list
*  the types for.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT ListMediaTypes( IMFMediaTypeHandler* pMediaTypeHandler ,
  vector<string> * pAsStrings = NULL );

/*
* List all the media modes available on a media source.
* @param[in] pReader: pointer to the media source reader to list the media types for.
*/
void ListModes( IMFSourceReader* pReader , CameraData& Camera , bool brief = false );

/**
* Fill a parameters of the video capture device.
* pMyDevicePath - device driver path
* Camera - structure for filling
** @@Returns S_OK if successful or an error code if not.
*
* Remarks:
* See https://docs.microsoft.com/en-us/windows/win32/coreaudio/device-properties.
*/
HRESULT GetCaptureDevice(const char * pMyDevicePath, CameraData& Camera);

/**
* Prints out a list of the audio or video capture devices available.
* @param[in] deviceType: whether to list audio or video capture devices.
* @@Returns S_OK if successful or an error code if not.
*
* Remarks:
* See https://docs.microsoft.com/en-us/windows/win32/coreaudio/device-properties.
*/
HRESULT ListCaptureDevices( DeviceType deviceType , CamerasVector& Cameras ) ;


/**
* Prints out a list of the audio or video capture devices available along with
* a brief description of the most important format properties.
* @param[in] deviceType: whether to list audio or video capture devices.
* @@Returns S_OK if successful or an error code if not.
*
* Remarks:
* See https://docs.microsoft.com/en-us/windows/win32/coreaudio/device-properties.
*/
HRESULT ListVideoDevicesWithBriefFormat( vector<string> * pAsStrings = NULL );

/**
* Attempts to print out a list of all the audio output devices
* available on the system.
* @@Returns S_OK if successful or an error code if not.
*
* Remarks:
* See https://docs.microsoft.com/en-us/windows/win32/medfound/streaming-audio-renderer.
*/
HRESULT ListAudioOutputDevices( vector<string> * pAsStrings = NULL ) ;


/*
* Attempts to get an audio output sink for the specified device index.
* @param[in] deviceIndex: the audio output device to get the sink for.
* @param[out] ppAudioSink: if successful this parameter will be set with
*  the output sink.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT GetAudioOutputDevice( UINT deviceIndex , IMFMediaSink** ppAudioSink ) ;


/**
* Gets a video source reader from a device such as a webcam.
* @param[in] nDevice: the video device index to attempt to get the source reader for.
* @param[out] ppVideoSource: will be set with the source for the reader if successful.
* @param[out] ppVideoReader: will be set with the reader if successful. Set this parameter
*  to nullptr if no reader is required and only the source is needed.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT GetVideoSourceFromDevice( UINT nDevice ,
  IMFMediaSource** ppVideoSource , IMFSourceReader** ppVideoReader ) ;


/**
* Gets an audio or video source reader from a capture device such as a webcam or microphone.
* @param[in] deviceType: the type of capture device to get a source reader for.
* @param[in] nDevice: the capture device index to attempt to get the source reader for.
* @param[out] ppMediaSource: will be set with the source for the reader if successful.
* @param[out] ppVMediaReader: will be set with the reader if successful. Set this parameter
*  to nullptr if no reader is required and only the source is needed.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT GetSourceFromCaptureDevice( DeviceType deviceType ,
  UINT nDevice , IMFMediaSource** ppMediaSource , 
  IMFSourceReader** ppMediaReader ) ;


/**
* Copies a media type attribute from an input media type to an output media type. Useful when setting
* up the video sink and where a number of the video sink input attributes need to be duplicated on the
* video writer attributes.
* @param[in] pSrc: the media attribute the copy of the key is being made from.
* @param[in] pDest: the media attribute the copy of the key is being made to.
* @param[in] key: the media attribute key to copy.
*/
HRESULT CopyAttribute( IMFAttributes* pSrc , IMFAttributes* pDest , const GUID& key ) ;


/**
* Creates a bitmap file and writes to disk.
* @param[in] fileName: the path to save the file at.
* @param[in] width: the width of the bitmap.
* @param[in] height: the height of the bitmap.
* @param[in] bitsPerPixel: colour depth of the bitmap pixels (typically 24 or 32).
* @param[in] bitmapData: a pointer to the bytes containing the bitmap data.
* @param[in] bitmapDataLength: the number of pixels in the bitmap.
*/
void CreateBitmapFile( LPCSTR fileName , 
  long width , long height , WORD bitsPerPixel , 
  BYTE* bitmapData , DWORD bitmapDataLength );

void CreateBitmapFromSample( LPCSTR fileName ,
  long width , long height , WORD bitsPerPixel , IMFSample* pSample ) ;


/**
* Calculate the minimum stride from the media type.
* From:
* https://docs.microsoft.com/en-us/windows/win32/medfound/uncompressed-video-buffers
*/
HRESULT GetDefaultStride( IMFMediaType* pType , LONG* plStride ) ;


/**
* Dumps the media buffer contents of an IMF sample to a file stream.
* @param[in] pSample: pointer to the media sample to dump the contents from.
* @param[in] pFileStream: pointer to the file stream to write to.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT WriteSampleToFile( IMFSample* pSample , std::ofstream* pFileStream ) ;

/**
* Creates a new single buffer media sample.
* @param[in] bufferSize: size of the media buffer to set on the create media sample.
* @param[out] pSample: pointer to the create single buffer media sample.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT CreateSingleBufferIMFSample( DWORD bufferSize , IMFSample** pSample );


/**
* Creates a new media sample and copies the first media buffer from the source to it.
* @param[in] pSrcSample: size of the media buffer to set on the create media sample.
* @param[out] pDstSample: pointer to the media sample created.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT CreateAndCopySingleBufferIMFSample( 
  IMFSample* pSrcSample , IMFSample** pDstSample ) ;


/**
* Attempts to get an output sample from an MFT transform.
* @param[in] pTransform: pointer to the media transform to apply.
* @param[out] pOutSample: pointer to the media sample output by the transform. Can be NULL
*  if the transform did not produce one.
* @param[out] transformFlushed: if set to true means the transform format changed and the
*  contents were flushed. Output format of sample most likely changed.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT GetTransformOutput( IMFTransform* pTransform ,
  IMFSample** pOutSample , BOOL* transformFlushed );


/**
* Gets the hex string representation of a byte array.
* @param[in] start: pointer to the start of the byte array.
* @param[in] length: length of the byte array.
* @@Returns a null terminated char array.
*/
unsigned char* HexStr( const uint8_t* start , size_t length );

/* returns all available Camera controls and Amplifier controls as text strings with values */
int GetCameraControls( IMFMediaSource* pMediaSource , ControlVector& CVector ) ;

FXString GetAllUSBDevices() ;

bool ExtractCameraData( FXString& DevicePathAsText ,
  CameraData& Result , FXString& AllUSBDevices ) ;


class MediaEventHandler : IMFAsyncCallback
{
public:

  HRESULT STDMETHODCALLTYPE Invoke(IMFAsyncResult* pAsyncResult)
  {
    HRESULT hr = S_OK;
    IMFMediaEvent* pEvent = NULL;
    MediaEventType meType = MEUnknown;
    BOOL fGetAnotherEvent = TRUE;
    HRESULT hrStatus = S_OK;
    IMFMediaEventGenerator* pEventGenerator = NULL;
    
    hr = pAsyncResult->GetState((IUnknown**)&pEventGenerator);
    if (!SUCCEEDED(hr))
    {
      printf("Failed to get media event generator from async state.\n");
    }

    // Get the event from the event queue.
    // Assume that m_pEventGenerator is a valid pointer to the
    // event generator's IMFMediaEventGenerator interface.
    hr = pEventGenerator->EndGetEvent(pAsyncResult, &pEvent);

    // Get the event type.
    if (SUCCEEDED(hr))
    {
      hr = pEvent->GetType(&meType);
      //printf("Media event type %d.\n", meType);
    }

    // Get the event status. If the operation that triggered the event 
    // did not succeed, the status is a failure code.
    if (SUCCEEDED(hr))
    {
      hr = pEvent->GetStatus(&hrStatus);
    }

    if (SUCCEEDED(hr))
    {
      // TODO: Handle the event.
    }

    // If not finished, request another event.
    // Pass in a pointer to this instance of the application's
    // CEventHandler class, which implements the callback.
    if (fGetAnotherEvent)
    {
      hr = pEventGenerator->BeginGetEvent(this, pEventGenerator);
    }

    SAFE_RELEASE(pEvent);
    return hr;
  }

  HRESULT STDMETHODCALLTYPE GetParameters(
    DWORD* pdwFlags,
    DWORD* pdwQueue
  )
  {
    pdwFlags = 0;
    pdwQueue = 0;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
  {
    return S_OK;
  }

  ULONG STDMETHODCALLTYPE AddRef(void)
  {
    return 0;
  }

  ULONG STDMETHODCALLTYPE Release(void)
  {
    return 0;
  }
};