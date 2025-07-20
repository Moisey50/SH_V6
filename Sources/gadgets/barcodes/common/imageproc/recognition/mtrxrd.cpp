#include "stdafx.h"
#include "mtrxrd.h"
//#include <settings.h>
//#include <charsdetector.h>
#include <imageproc\normalize.h>
#include <imageproc\recognition\MxReader.h>

#define MXELEM_MIN_AREA 27
#define MXELEM_MAX_AREA 5000
//#define MXELEM_MIN_XYRATIO 27
//#define MXELEM_MAX_XYRATIO 5000

__forceinline void _preprocess4Mtrx(pTVFrame part)
{
        //_enchance2(part,700);
        _normalize(part);
        // binarizing 
    
        //_simplebinarize(part,110);
}

pMtrxRdBlck MtrxSetFrame(const pTVFrame frame)
{
    pMtrxRdBlck mxBlck=new MtrxRdBlck;
    mxBlck->orgFrame=dublicateTVFrame(frame);
    
    CMxReader mxReader;
    mxReader.SetImage(mxBlck->orgFrame);
    if (mxReader.ScanImage())
    {
        mxBlck->result=mxReader.GetResult();
    }
    else
        mxBlck->result="";
    return mxBlck;
}

void MtrxBlckDone(pMtrxRdBlck mxBlck)
{
    if (mxBlck)
    {
        if (mxBlck->orgFrame) freeTVFrame(mxBlck->orgFrame); mxBlck->orgFrame=NULL;
        delete mxBlck;
    }
}
