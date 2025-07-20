#ifndef _MTRXRD_INC
#define _MTRXRD_INC

#include <video\tvframe.h>
#include <fxfc\fxfc.h>

typedef struct _tetragon
{
    CPoint tl,tr,bl,br;
}tetragon;

typedef struct tagMtrxRdBlck
{
    FXString  result;
    tetragon mtetragon;
    pTVFrame orgFrame;
public:
    tagMtrxRdBlck()
    {
        memset(&mtetragon,0,sizeof(tetragon));
        orgFrame=NULL;
    }
}MtrxRdBlck,*pMtrxRdBlck;

extern pMtrxRdBlck MtrxSetFrame(const pTVFrame frame);
extern void MtrxBlckDone(pMtrxRdBlck mxBlck);

#endif