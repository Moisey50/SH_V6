// VFilter.cpp: implementation of the CVFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VFilter.h"
#include "DShowCapture.h"
#include <Dvdmedia.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const AMOVIESETUP_MEDIATYPE sudIpPinTypes[] =	{
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
        }
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

__forceinline bool IsFrameDividabe4(const CMediaType *pMediaType, bool JustWidth=true)
{
    if ((pMediaType) && (pMediaType->formattype==FORMAT_VideoInfo))
    {
        VIDEOINFOHEADER* pvhr=(VIDEOINFOHEADER*)pMediaType->pbFormat;
        TRACE("+++ Width = %d\n",pvhr->bmiHeader.biWidth);
        if (JustWidth)
            return (pvhr->bmiHeader.biWidth%4==0);
        return ((pvhr->bmiHeader.biWidth%4==0) && (pvhr->bmiHeader.biHeight%4==0));
    }
    else if ((pMediaType) && (pMediaType->formattype==FORMAT_VideoInfo2)) 
    {
        VIDEOINFOHEADER2* pvhr=(VIDEOINFOHEADER2*)pMediaType->pbFormat;
        TRACE("+++ Width = %d\n",pvhr->bmiHeader.biWidth);
        if (JustWidth)
            return (pvhr->bmiHeader.biWidth%4==0);
        return ((pvhr->bmiHeader.biWidth%4==0) && (pvhr->bmiHeader.biHeight%4==0));
    }
    return false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4355)
int instanceCntr=0;

CVFilter::CVFilter(LPUNKNOWN pUnk,HRESULT *phr, void *callback):
CBaseVideoRenderer(CLSID_VideoRenderer, NAME("Virtual render filter"), pUnk, phr)
{
    TRACE("+++ new CVFilter object created, %d\n",instanceCntr);
    instanceCntr++;
    m_CallBack=callback;
}

#pragma warning(default:4355)

CVFilter::~CVFilter()
{
    instanceCntr--;
    TRACE("+++ CVFilter object deleted, %d\n",instanceCntr);
}

HRESULT CVFilter::DoRenderSample(IMediaSample *pMediaSample)
{
    HRESULT hr=E_FAIL;
    ASSERT(pMediaSample);
    LONGLONG llTstart , llTEnd ;
    hr = pMediaSample->GetTime( &llTstart , &llTEnd ) ;
//    TRACE( "+++ CVFilter::DoRenderSample T=%I64d\n" , llTstart );
    if ( m_CallBack )
    {
        HRESULT hr=((DShowCapture*)m_CallBack)->DoRenderSample(pMediaSample);
        if (hr!=S_OK)
        {
            EndOfStream();
        }
    }
    return hr;
}

// {30303859-0000-0010-8000-00AA00389B71} 'Y800' == MEDIASUBTYPE_Y800
GUID MEDIASUBTYPE_Y800={0x30303859, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};

FXString GUIDToString(GUID guid)
{
    FXString retV;
    retV.Format("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], 
        guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    return retV;
}

bool IsGUIDVFWCodec(GUID guid)
{
    return (
        (guid.Data2==MEDIASUBTYPE_Y800.Data2) &&
        (guid.Data3==MEDIASUBTYPE_Y800.Data3) &&
        (guid.Data4[0]==MEDIASUBTYPE_Y800.Data4[0]) &&
        (guid.Data4[1]==MEDIASUBTYPE_Y800.Data4[1]) &&
        (guid.Data4[2]==MEDIASUBTYPE_Y800.Data4[2]) &&
        (guid.Data4[3]==MEDIASUBTYPE_Y800.Data4[3]) &&
        (guid.Data4[4]==MEDIASUBTYPE_Y800.Data4[4]) &&
        (guid.Data4[5]==MEDIASUBTYPE_Y800.Data4[5]) &&
        (guid.Data4[6]==MEDIASUBTYPE_Y800.Data4[6]) &&
        (guid.Data4[7]==MEDIASUBTYPE_Y800.Data4[7]) 
        );
}


FXString MediaTypeToString(const CMediaType *pmt)
{
    FXString retV = GUIDToString(pmt->subtype)+" - ";
    if (IsGUIDVFWCodec(pmt->subtype))
    {
        CString a;
        if (pmt->subtype.Data1==0)
            a="WMMEDIASUBTYPE_Base";
        else
        {
            memcpy(a.GetBufferSetLength(5),&pmt->subtype.Data1,sizeof(pmt->subtype.Data1));
            a.SetAt(4,0);
            a.ReleaseBuffer();
        }
        retV+=a;
    }
    else
        retV+="Unknown";
    return retV;
}

HRESULT CVFilter::CheckMediaType(const CMediaType *pmt)
{
    TRACE("+++ CVFilter::CheckMediaType(%s)\n",MediaTypeToString(pmt));
    
    if (pmt->majortype != MEDIATYPE_Video) return E_INVALIDARG;

    //if (!IsFrameDividabe4(pmt)) return E_INVALIDARG;
    
    if (pmt->subtype == MEDIASUBTYPE_YVU9) return NOERROR;
    if (pmt->subtype == MEDIASUBTYPE_UYVY) return NOERROR;
    if (pmt->subtype == MEDIASUBTYPE_Y800) return NOERROR;
//    if (pmt->subtype == MEDIASUBTYPE_RGB8) return NOERROR;
    if (pmt->subtype == MEDIASUBTYPE_Y411) return NOERROR;
    if (pmt->subtype == MEDIASUBTYPE_YUY2) return NOERROR;
    if (pmt->subtype == MEDIASUBTYPE_MJPG) return NOERROR;
    return E_INVALIDARG;
}

void CVFilter::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
  LONGLONG llTstart , llTEnd ;
  HRESULT hr = pMediaSample->GetTime( &llTstart , &llTEnd ) ;
    TRACE("+++ CVFilter::OnReceiveFirstSample T=%I64d\n" , llTstart );
    DoRenderSample(pMediaSample);
    //CBaseVideoRenderer::OnReceiveFirstSample(pMediaSample);
}

HRESULT CVFilter::SetMediaType(const CMediaType *pmt)
{
    TRACE("+++ CVFilter::SetMediaType(%s)\n",MediaTypeToString(pmt));
    //if (!IsFrameDividabe4(pmt)) return E_INVALIDARG;
    m_mtIn=*pmt;
    VIDEOINFOHEADER* vhr=(VIDEOINFOHEADER*)(pmt->pbFormat);
    if (m_CallBack) ((DShowCapture*)m_CallBack)->NotifyMediaType(&m_mtIn);
    return NOERROR;
}


int CVFilter::StateCapturing()
{
    FILTER_STATE fs;
    if (GetState(1000,&fs)==NOERROR) return ((fs==State_Running) && (!IsEndOfStream()))?1:0;
    return -1;
}
