// VFilter.cpp: implementation of the CVFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "ivwdm.h"
#include "VFilter.h"
//#include "WDMDriver.h"
#include "WDMCaptureGadget.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// {30303859-0000-0010-8000-00AA00389B71} 'Y800' == MEDIASUBTYPE_Y800
GUID MEDIASUBTYPE_Y800 = {0x30303859, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};
// {30303859-0000-0010-8000-00AA00389B71} 'I420' == MEDIASUBTYPE_I420
GUID MEDIASUBTYPE_I420 = {0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};

const AMOVIESETUP_MEDIATYPE sudIpPinTypes[] =
{
  {
      &MEDIATYPE_Video,             // MajorType
      &MEDIASUBTYPE_YVU9            // MinorType
  },
  {
      &MEDIATYPE_Video,             // MajorType
      &MEDIASUBTYPE_UYVY            // MinorType
  },
  {
      &MEDIATYPE_Video,             // MajorType
      &MEDIASUBTYPE_RGB8            // MinorType
  },
  {
      &MEDIATYPE_Video,             // MajorType
      &MEDIASUBTYPE_RGB24            // MinorType
  } ,
  {

      &MEDIATYPE_Video,             // MajorType
      &MEDIASUBTYPE_I420            // MinorType
  }// ,
  //{
  //    &MEDIATYPE_Video,             // MajorType
  //    &MEDIASUBTYPE_YV12            // MinorType
  //} 
};


const AMOVIESETUP_PIN sudIpPin =
{
    L"Input",                     // The Pins name
    FALSE,                        // Is rendered
    FALSE,                        // Is an output pin
    FALSE,                        // Allowed none
    FALSE,                        // Allowed many
    &CLSID_NULL,                  // Connects to filter
    NULL,                         // Connects to pin
    3,                            // Number of types
    sudIpPinTypes                 // Pin details
};

const AMOVIESETUP_FILTER sudTextoutAx =
{
    &CLSID_VideoRenderer,         // Filter CLSID
    L"Virtual render filter",     // String name
    MERIT_NORMAL,                 // Filter merit
    1,                            // Number of pins
    &sudIpPin                     // Pin details
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4355)
int instanceCntr = 0;

CVFilter::CVFilter( LPUNKNOWN pUnk , HRESULT *phr , void *callback ) :
  CBaseVideoRenderer( CLSID_VideoRenderer , NAME( "Virtual render filter" ) , pUnk , phr )
{
  TRACE( "+++ new CVFilter object created, %d\n" , instanceCntr );
  instanceCntr++;
  m_CallBack = callback;
}

#pragma warning(default:4355)

CVFilter::~CVFilter()
{
  instanceCntr--;
  TRACE( "+++ CVFilter object deleted, %d\n" , instanceCntr );
}

HRESULT CVFilter::DoRenderSample( IMediaSample *pMediaSample )
{
  HRESULT hr = E_FAIL;
  ASSERT( pMediaSample );
  if ( m_CallBack )
  {
    HRESULT hr = ((WDMCapture*) m_CallBack)->DoRenderSample( pMediaSample );
    if ( hr != S_OK )
    {
      EndOfStream();
    }
  }
  return hr;
}


HRESULT CVFilter::CheckMediaType( const CMediaType *pmt )
{
  TRACE( "+++ CVFilter::CheckMediaType\n" );
  if ( pmt->majortype != MEDIATYPE_Video ) return E_INVALIDARG;
  if ( pmt->subtype == MEDIASUBTYPE_YVU9 ) return NOERROR;
  //if ( pmt->subtype == MEDIASUBTYPE_YV12 ) return NOERROR;
  if ( pmt->subtype == MEDIASUBTYPE_UYVY ) return NOERROR;
  if ( pmt->subtype == MEDIASUBTYPE_Y800 ) return NOERROR;
  if ( pmt->subtype == MEDIASUBTYPE_RGB8 ) return NOERROR;
  if ( pmt->subtype == MEDIASUBTYPE_RGB24 ) return NOERROR;
  if ( pmt->subtype == MEDIASUBTYPE_Y411 ) return NOERROR;
  if ( pmt->subtype == MEDIASUBTYPE_YUY2 ) return NOERROR;
  if ( pmt->subtype == MEDIASUBTYPE_I420 )  return NOERROR;
  //if ( pmt->subtype == MEDIASUBTYPE_MJPG )  return NOERROR;
  return E_INVALIDARG;
}

void CVFilter::OnReceiveFirstSample( IMediaSample *pMediaSample )
{
  //       TRACE("+++ CVFilter::OnReceiveFirstSample\n");
  DoRenderSample( pMediaSample );
  CBaseVideoRenderer::OnReceiveFirstSample( pMediaSample );
}

HRESULT CVFilter::SetMediaType( const CMediaType *pmt )
{
  TRACE( "+++ CVFilter::SetMediaType\n" );
  m_mtIn = *pmt;
  VIDEOINFOHEADER* vhr = (VIDEOINFOHEADER*) (pmt->pbFormat);
  if ( m_CallBack ) ((WDMCapture*) m_CallBack)->NotifyMediaType( &m_mtIn );
  return NOERROR;
}


int CVFilter::StateCapturing()
{
  FILTER_STATE fs;
  if ( GetState( 1000 , &fs ) == NOERROR ) return ((fs == State_Running) && (!IsEndOfStream())) ? 1 : 0;
  return -1;
}
