//  $File : imagebits.h - access to points values in an image
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef IMAGEBITS_INC
#define IMAGEBITS_INC

#include <video\stdcodec.h>

__forceinline bool __veryfiyInFrame(pTVFrame frame, POINT pnt)
{
    bool res=true;
    if (pnt.x<0) res= false;
    if (pnt.y<0) res= false;
    if (pnt.x>=frame->lpBMIH->biWidth)  res= false;
    if (pnt.y>=frame->lpBMIH->biHeight) res= false;
    return res;
}

__forceinline bool __veryfiyInFrame(pTVFrame frame, RECT rc)
{
    bool res=true;
    if (rc.left<0) res= false;
    if (rc.bottom<0) res= false;
    if (rc.left>=frame->lpBMIH->biWidth)  res= false;
    if (rc.bottom>=frame->lpBMIH->biHeight) res= false;
    if (rc.right<0) res= false;
    if (rc.top<0) res= false;
    if (rc.right>=frame->lpBMIH->biWidth)  res= false;
    if (rc.top>=frame->lpBMIH->biHeight) res= false;
    return res;
}

__forceinline void __round2Bndr(pTVFrame frame, POINT pnt)
{
    if (pnt.x<0) pnt.x=0;
    if (pnt.y<0) pnt.y=0;
    if (pnt.x>=frame->lpBMIH->biWidth)  pnt.x=frame->lpBMIH->biWidth-1;
    if (pnt.y>=frame->lpBMIH->biHeight) pnt.y=frame->lpBMIH->biHeight-1;
}

__forceinline void __round2Bndr(pTVFrame frame, RECT rc)
{
    if (rc.left<0)   rc.left=0;
    if (rc.bottom<0) rc.bottom=0;
    if (rc.left>=frame->lpBMIH->biWidth)    rc.left=frame->lpBMIH->biWidth-1;
    if (rc.bottom>=frame->lpBMIH->biHeight) rc.bottom=frame->lpBMIH->biHeight-1;
    if (rc.right<0) rc.right=0;
    if (rc.top<0)   rc.top=0;
    if (rc.right>=frame->lpBMIH->biWidth)  rc.right = frame->lpBMIH->biWidth-1;
    if (rc.top>=frame->lpBMIH->biHeight)   rc.top   = frame->lpBMIH->biHeight;
}


__forceinline LPBYTE __getdata_I_XY(pTVFrame frame,int X, int Y)
{
    int offset=X+Y*(frame->lpBMIH->biWidth);
    LPBYTE src=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);
    if ((X>=0) && (Y>=0) && (X<frame->lpBMIH->biWidth) && (Y<frame->lpBMIH->biHeight)) return(&(src[offset]));
    return(NULL);
}

__forceinline unsigned getdata_I_XY(pTVFrame frame,int X, int Y)
{
    int offset=X+Y*(frame->lpBMIH->biWidth);
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
    case BI_Y8:
        {
            LPBYTE src=GetData(frame);
            if ((X>=0) && (Y>=0) && (X<frame->lpBMIH->biWidth) && (Y<frame->lpBMIH->biHeight)) 
                return (src[offset]);
        }
    case BI_Y16:
        {
            LPWORD src=(LPWORD)GetData(frame);
            if ((X>=0) && (Y>=0) && (X<frame->lpBMIH->biWidth) && (Y<frame->lpBMIH->biHeight)) 
                return (src[offset]);
        }
    }
    return (-1);
}

__forceinline bool __getdata_IUV(pTVFrame frame,int X, int Y, int& I, int& U, int& V)
{
    bool res=true;
    
    ASSERT(frame);

    LPBYTE src=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);   

    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
        {
            int offset=X+Y*(frame->lpBMIH->biWidth);
            int colWidth=(frame->lpBMIH->biWidth>>2);
            int ISize=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
            res=((X>=0) && (Y>=0) && (X<frame->lpBMIH->biWidth) && (Y<frame->lpBMIH->biHeight));
            if (res) 
            {
                I=(unsigned int)src[offset];
                offset=ISize+(X>>2)+(Y>>2)*colWidth;
                V=(unsigned int)src[offset];
                offset=ISize+(ISize>>4)+(X>>2)+(Y>>2)*colWidth;
                U=(unsigned int)src[offset];
            }
            else
            {
                TRACE("!!! Warrning! __getdata_IUV(%d,%d) outside of frame boundary!\n",X,Y);
                I=0;
                V=0;
                U=0;
            }
            break;
        }
    case BI_YUV12:
        {
            int offset=X+Y*(frame->lpBMIH->biWidth);
            int colWidth=(frame->lpBMIH->biWidth>>2);
            int ISize=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
            res=((X>=0) && (Y>=0) && (X<frame->lpBMIH->biWidth) && (Y<frame->lpBMIH->biHeight));
            if (res) 
            {
                I=(unsigned int)src[offset];
                offset=ISize+(X>>1)+(Y>>1)*colWidth;
                U=(unsigned int)src[offset];
                offset=ISize+(ISize>>2)+(X>>1)+(Y>>1)*colWidth;
                V=(unsigned int)src[offset];
            }
            else
            {
                TRACE("!!! Warrning! __getdata_IUV(%d,%d) outside of frame boundary!\n",X,Y);
                I=0;
                V=0;
                U=0;
            }
            break;
        }
    case BI_Y8:
        {
            int offset=X+Y*(frame->lpBMIH->biWidth);
            int colWidth=(frame->lpBMIH->biWidth>>2);
            int ISize=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
            res=((X>=0) && (Y>=0) && (X<frame->lpBMIH->biWidth) && (Y<frame->lpBMIH->biHeight));
            if (res) 
            {
                I=(unsigned int)src[offset];
                V=128;
                U=128;
            }
            else
            {
                TRACE("!!! Warrning! __getdata_IUV(%d,%d) outside of frame boundary!\n",X,Y);
                I=0;
                V=0;
                U=0;
            }
            break;
        }
    case BI_Y16:
        {
            int offset=X+Y*(frame->lpBMIH->biWidth);
            int ISize=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
            res=((X>=0) && (Y>=0) && (X<frame->lpBMIH->biWidth) && (Y<frame->lpBMIH->biHeight));
            if (res) 
            {
                I=((unsigned short*)src)[offset];
                V=128;
                U=128;
            }
            else
            {
                TRACE("!!! Warrning! __getdata_IUV(%d,%d) outside of frame boundary!\n",X,Y);
                I=0;
                V=0;
                U=0;
            }
            break;
        }
    default:
//        ASSERT(FALSE);
        ;
    }

    ASSERT(I>=0);
    ASSERT(U>=0);
    ASSERT(V>=0);
    return res;
}

__forceinline bool __setdata_IUV(pTVFrame frame,int X, int Y, int I, int U, int V)
{
    bool res=true;
    
    ASSERT(frame);

    LPBYTE src=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);   

    ASSERT((frame->lpBMIH->biCompression==BI_YUV9) || (frame->lpBMIH->biCompression==BI_Y8));

    int offset=X+Y*(frame->lpBMIH->biWidth);
    int colWidth=(frame->lpBMIH->biWidth>>2);
    int ISize=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
    if ((X>=0) && (Y>=0) && (X<frame->lpBMIH->biWidth) && (Y<frame->lpBMIH->biHeight)) 
    {
        src[offset]=I;
        offset=ISize+(X>>2)+(Y>>2)*colWidth;
        src[offset]=V;
        offset=ISize+(ISize>>4)+(X>>2)+(Y>>2)*colWidth;
        src[offset]=U;
    }
    return res;
}


#endif