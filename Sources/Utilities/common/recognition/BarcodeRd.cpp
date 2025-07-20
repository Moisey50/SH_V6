#include "stdafx.h"
#include "BarcodeRd.h"
#include <imageproc\simpleIP.h>
//#include <helpers\ctimer.h>
#include <fxfc\fxtimer.h>
#include <recognition\seekUPC.H>
#include <imageproc\recognition\decodeUPC.h>
#include <imageproc\recognition\htriggerbin.h>

CBarcodeRd::CBarcodeRd(void)
{
}

CBarcodeRd::~CBarcodeRd(void)
{
    CleanUp();
}

void CBarcodeRd::CleanUp(void)
{
}

bool CBarcodeRd::Parse(CSData& d, FXString& res)
{
    parse(d,res);
    return verify(res);
}


bool CBarcodeRd::Parse(pTVFrame frame, bool freedata)
{
    double s=GetHRTickCount();
    CleanUp();

    VERIFY(frame->lpData==NULL);
	
	_normalize(frame);
	pTVFrame bin=htriggerbin(frame);
	
    bool retV=false;
	
	retV=parse(bin, m_Result);
#ifdef	_DEBUG
	memcpy(GetData(frame),GetData(bin),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight);
#endif
	freeTVFrame(bin);
    m_TS=GetHRTickCount()-s;
    if (freedata)
    {
        free(frame->lpBMIH); frame->lpBMIH=NULL;
    }
    return retV;
}


/*
bool CBarcodeRd::Parse(pTVFrame frame, bool freedata)
{
    DWORD s=GetHRTickCount();
    CleanUp();

    int dSize=frame->lpBMIH->biSize+GetImageSize(frame);
    int iSize=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;

    VERIFY(frame->lpData==NULL);

    pTVFrame tmpFr=_hpass_1DH(frame,900); if (frame->lpBMIH) free(frame->lpBMIH);
    frame->lpBMIH=tmpFr->lpBMIH; free(tmpFr);
    _equalize_hist(frame);
    _lnDelta(frame);

    _InvDelta(frame);
    bool retV=false;

    retV=parse(frame, m_Result);

    m_TS=GetHRTickCount()-s;
    if (freedata)
    {
        free(frame->lpBMIH); frame->lpBMIH=NULL;
    }
    return retV;
}
*/