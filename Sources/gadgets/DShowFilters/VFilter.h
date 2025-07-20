// VFilter.h: interface for the CVFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VFILTER_H__AFCAA682_9D70_11D6_B0F2_000001360793__INCLUDED_)
#define AFX_VFILTER_H__AFCAA682_9D70_11D6_B0F2_000001360793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <DShow.h>

#define ALLOWED_RGB8 1
#define ALLOWED_UYVY 2
#define ALLOWED_YUV9 4
#define ALLOWED_Y411 8
#define ALLOWED_YUY2 16

class CVFilter: public CBaseVideoRenderer//CBaseRenderer
{
private:
    void*      m_CallBack;
    CMediaType m_mtIn;                  // Source connection media type
public:
	CVFilter(LPUNKNOWN pUnk,HRESULT *phr,void *callback);
	virtual ~CVFilter();
//  CBaseRenderer pure virtual methods overwritteables
    HRESULT DoRenderSample(IMediaSample *pMediaSample);
    HRESULT CheckMediaType(const CMediaType *pmt);
    virtual void OnReceiveFirstSample(IMediaSample *pMediaSample);
    HRESULT SetMediaType(const CMediaType *pmt);
    int     StateCapturing();
};

#endif // !defined(AFX_VFILTER_H__AFCAA682_9D70_11D6_B0F2_000001360793__INCLUDED_)
