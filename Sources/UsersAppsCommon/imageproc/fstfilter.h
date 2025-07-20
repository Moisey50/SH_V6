//  $File : fstfilter.h - low- and high- pass 2D filters
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef _FSTFILTER_INC
#define _FSTFILTER_INC

#include <math.h>
#include <imageproc\settings.h>
#include <video\tvframe.h>

__forceinline void _lpass(int width, int height, LPBYTE src, LPBYTE dst,int __fr_nmb=CONOVOLUTION_PARAM)
{
    int size=height*width;
    int i=0; double accum=128; BYTE temp;
    double rate=1000.0/(1000 - __fr_nmb)-1.0;
    double fading =(double)__fr_nmb/1000.0;
    while (i<size)
    {
        accum=fading*accum;
        *(dst+i)=(BYTE)(accum/rate+0.5);
        accum+=*(src+i);
        i++;
    }
    i--;
    LPBYTE scn=dst+i;

    while (i>=0)
    {
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*scn;
        *scn=temp;
        i--; scn--;
    }
    DWORD off; i++;
    while (i<size)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i++;
    }
    i--;
    while (i>=0)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i--;
    }; 
    *dst=*(dst+width+1);
    *(dst+1)=*(dst+width+2);
    *(dst+width)=*(dst+width+1);
    *(dst+2*width)=*(dst+2*width+1);
}

__forceinline void _lpass16(int width, int height, LPWORD src, LPWORD dst,int __fr_nmb=CONOVOLUTION_PARAM)
{
    int size=height*width;
    int i=0; double accum=128; WORD temp;
    double rate=1000.0/(1000 - __fr_nmb)-1.0;
    double fading =(double)__fr_nmb/1000.0;
    while (i<size)
    {
        accum=fading*accum;
        *(dst+i)=(WORD)(accum/rate+0.5);
        accum+=*(src+i);
        i++;
    }
    i--;
    LPWORD scn=dst+i;

    while (i>=0)
    {
        accum=fading*accum;
        temp=(WORD)(accum/rate+0.5);
        accum+=*scn;
        *scn=temp;
        i--; scn--;
    }
    DWORD off; i++;
    while (i<size)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(WORD)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i++;
    }
    i--;
    while (i>=0)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(WORD)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i--;
    }; 
    *dst=*(dst+width+1);
    *(dst+1)=*(dst+width+2);
    *(dst+width)=*(dst+width+1);
    *(dst+2*width)=*(dst+2*width+1);
}


__forceinline void _hpass(int width, int height, LPBYTE src, LPBYTE dst,int __fr_nmb=CONOVOLUTION_PARAM)
{
    _lpass(width, height, src, dst,__fr_nmb);
    LPBYTE sA=src;
    LPBYTE sB=dst;
    LPBYTE eod=dst+width*height;
    while(sB<eod)
    {
        int r=128+(*sA-*sB);
        *sB=(r<0)?0:(r>255)?255:r;
        sA++; sB++;
    }
}

__forceinline pTVFrame _lpass(pTVFrame frame, int __fr_nmb=CONOVOLUTION_PARAM, bool freesrc=false)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    if (__fr_nmb>=1000)
    {
        TRACE("!!! Warrning: \"_lpass\" - requested too high __fr_nmb==%d, truncated to 999\n",__fr_nmb);
        __fr_nmb=999;
    }
    if (__fr_nmb<=0)
    {
        TRACE("!!! Warrning: \"_lpass\" - requested too low __fr_nmb==%d, truncated to 1\n",__fr_nmb);
        __fr_nmb=1;
    }
    pTVFrame dstframe=makecopyTVFrame(frame);
    
    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;

    LPBYTE src=GetData(frame);
    LPBYTE dst=GetData(dstframe);

    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
        _lpass( width, depth, src, dst,__fr_nmb);
        break;
    case BI_Y8:
        _lpass( width, depth, src, dst,__fr_nmb);
        break;
    case BI_Y16:
        _lpass16( width, depth, (LPWORD)src, (LPWORD)dst,__fr_nmb);
        break;
    }
    if (freesrc) freeTVFrame(frame);
    return dstframe;
}

__forceinline pTVFrame _hpass(pTVFrame frame, int __fr_nmb=CONOVOLUTION_PARAM, bool freesrc=false)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    if (__fr_nmb>=1000)
    {
        TRACE("!!! Warrning: \"_hpass\" - requested too high __fr_nmb==%d, truncated to 999\n",__fr_nmb);
        __fr_nmb=999;
    }
    if (__fr_nmb<=0)
    {
        TRACE("!!! Warrning: \"_hpass\" - requested too low __fr_nmb==%d, truncated to 1\n",__fr_nmb);
        __fr_nmb=1;
    }

    pTVFrame dstframe =_lpass(frame,__fr_nmb);
    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;

    int size=depth*width;
    int i=0;
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
        {
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstframe);

            while (i<size)
            {
                int s=128+(*(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>255)?255:s));
                i++;
            }
            break;
        }
    case BI_Y8:
        {
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstframe);

            while (i<size)
            {
                int s=128+(*(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>255)?255:s));
                i++;
            }
            break;
        }
    case BI_Y16:
        {
            LPWORD src=(LPWORD)GetData(frame);
            LPWORD dst=(LPWORD)GetData(dstframe);
            while (i<size)
            {
                int s=32768+(*(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>65535)?65535:s));
                i++;
            }
        }
        break;
    }
    if (freesrc) freeTVFrame(frame);
    return dstframe;
}

__forceinline pTVFrame _8_1_passfilter(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    pTVFrame dstframe=makecopyTVFrame(frame);
    
    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;

    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
        {
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstframe);
    
            memset(dst,0,width*depth);
            memset(dst+width*depth,128,(width*depth)/8);

            for (int y=1; y<depth-1; y++)
            {
                LPBYTE srcoff=src+width*y+1;
                LPBYTE dstoff=dst+width*y+1;
                for (int x=1; x<width-1; x++)
                {
                    int dd=(*(srcoff-1)+*(srcoff+1)+*(srcoff-width)+*(srcoff+width)+*(srcoff-width-1)+*(srcoff+width-1)+*(srcoff-width+1)+*(srcoff+width+1))/8;
                    *dstoff=abs(dd-*srcoff);
                    srcoff++; dstoff++;
                }
            }   
            break;
        }
    case BI_Y8:
        {
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstframe);
            memset(dst,0,width*depth);
            for (int y=1; y<depth-1; y++)
            {
                LPBYTE srcoff=src+width*y+1;
                LPBYTE dstoff=dst+width*y+1;
                for (int x=1; x<width-1; x++)
                {
                    int dd=(*(srcoff-1)+*(srcoff+1)+*(srcoff-width)+*(srcoff+width)+*(srcoff-width-1)+*(srcoff+width-1)+*(srcoff-width+1)+*(srcoff+width+1))/8;
                    *dstoff=abs(dd-*srcoff);
                    srcoff++; dstoff++;
                }
            }   
            break;
        }
    case BI_Y16:
        {
            LPWORD src=(LPWORD)GetData(frame);
            LPWORD dst=(LPWORD)GetData(dstframe);
            memset(dst,0,width*depth*sizeof(WORD));
            for (int y=1; y<depth-1; y++)
            {
                LPWORD srcoff=src+width*y+1;
                LPWORD dstoff=dst+width*y+1;
                for (int x=1; x<width-1; x++)
                {
                    int dd=(*(srcoff-1)+*(srcoff+1)+*(srcoff-width)+*(srcoff+width)+*(srcoff-width-1)+*(srcoff+width-1)+*(srcoff-width+1)+*(srcoff+width+1))/8;
                    *dstoff=abs(dd-*srcoff);
                    srcoff++; dstoff++;
                }
            }   
            break;
        }
    }
    return dstframe;
}

__forceinline pTVFrame _hpass_1D(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    pTVFrame dstframe=makecopyTVFrame(frame);
    
    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
    case BI_Y8:
        {
            for (int i=0; i<depth; i++)
            {
                LPBYTE src=GetData(frame)+width*i;
                LPBYTE eol=src+width-1;
                LPBYTE dst=GetData(dstframe)+width*i;
                *dst=128;
                dst++; src++;
                while(src<eol)
                {
                    int res=128 + *src-(*(src-1)+*(src+1))/2;
                    *dst=(res<0)?0:(res>255)?255:res;
                    src++; dst++;
                }
                *dst=128;
            }
            break;
        }
    case BI_Y16:
        {
            for (int i=0; i<depth; i++)
            {
                LPWORD src=(LPWORD)GetData(frame)+width*i;
                LPWORD eol=src+width-1;
                LPWORD dst=(LPWORD)GetData(dstframe)+width*i;
                *dst=128;
                dst++; src++;
                while(src<eol)
                {
                    int res=32768 + *src-(*(src-1)+*(src+1))/2;
                    *dst=(res<0)?0:(res>65535)?65535:res;
                    src++; dst++;
                }
                *dst=128;
            }
            break;
        }
    }
    return dstframe;
}

__forceinline void _lpass_1D(LPBYTE _src, LPBYTE _dst, int width, int height, int kernel=10)
{
    for (int i=0; i<height; i++)
    {
        LPBYTE srcs=_src+width*i;
        LPBYTE dsts=_dst+width*i;
        int x=0;
        while(x<width)
        {
            int res=0;
            for (int j=-kernel; j<kernel; j++)
            {
                int offx=(x+j)%width;
                res+=*(srcs+offx);
            }
            *dsts=res/(2*kernel+1);
            dsts++; x++;
        }
        srcs+=width;
    }
}

__forceinline void _lpass_1D16(LPWORD _src, LPWORD _dst, int width, int height, int kernel=10)
{
    for (int i=0; i<height; i++)
    {
        LPWORD srcs=_src+width*i;
        LPWORD dsts=_dst+width*i;
        int x=0;
        while(x<width)
        {
            int res=0;
            for (int j=-kernel; j<kernel; j++)
            {
                int offx=(x+j)%width;
                res+=*(srcs+offx);
            }
            *dsts=res/(2*kernel+1);
            dsts++; x++;
        }
        srcs+=width;
    }
}

__forceinline void _lpass_1DV(int width, int height, LPBYTE dst,int __fr_nmb=CONOVOLUTION_PARAM)
{
    int size=height*width;
    int i=0; double accum=128; BYTE temp;
    double rate=1000.0/(1000 - __fr_nmb)-1.0;
    double fading =(double)__fr_nmb/1000.0;

    DWORD off=0; //i++;
    while (i<size)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i++;
    }
    accum=0; i--;
    while (i>=0)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i--;
    } 
}

__forceinline void _lpass_1DV16(int width, int height, LPWORD dst,int __fr_nmb=CONOVOLUTION_PARAM)
{
    int size=height*width;
    int i=0; double accum=128; WORD temp;
    double rate=1000.0/(1000 - __fr_nmb)-1.0;
    double fading =(double)__fr_nmb/1000.0;

    DWORD off=0; //i++;
    while (i<size)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(WORD)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i++;
    }
    accum=0; i--;
    while (i>=0)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(WORD)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i--;
    } 
}


__forceinline void _lpass_1DH(int width, int height, LPBYTE dst,int __fr_nmb=CONOVOLUTION_PARAM)
{
    int size=height*width;
    int i=0; double accum=128; BYTE temp;
    double rate=1000.0/(1000 - __fr_nmb)-1.0;
    double fading =(double)__fr_nmb/1000.0;

    while (i<size)
    {
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*(dst+i);
        *(dst+i)=temp;
        i++;
    }
    accum=0; i--;
    while (i>=0)
    {
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*(dst+i);
        *(dst+i)=temp;
        i--;
    } 
}

__forceinline void _lpass_1DH16(int width, int height, LPWORD dst,int __fr_nmb=CONOVOLUTION_PARAM)
{
    int size=height*width;
    int i=0; double accum=128; WORD temp;
    double rate=1000.0/(1000 - __fr_nmb)-1.0;
    double fading =(double)__fr_nmb/1000.0;

    while (i<size)
    {
        accum=fading*accum;
        temp=(WORD)(accum/rate+0.5);
        accum+=*(dst+i);
        *(dst+i)=temp;
        i++;
    }
    accum=0; i--;
    while (i>=0)
    {
        accum=fading*accum;
        temp=(WORD)(accum/rate+0.5);
        accum+=*(dst+i);
        *(dst+i)=temp;
        i--;
    } 
}

__forceinline void _lpass_1DHMask(pTVFrame frame, pTVFrame mask, int __fr_nmb=CONOVOLUTION_PARAM)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    int height=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;
    
    LPBYTE dst=GetData(frame);
    LPBYTE msk=GetData(mask);

    int size=9*(height*width/8);
    int i=0; double accum=0; BYTE temp;
    double rate=1000.0/(1000 - __fr_nmb)-1.0;
    double fading =(double)__fr_nmb/1000.0;

    DWORD off=0; //i++;
    while (i<size)
    {
        off=width*(i%height)+i/height;
        if (*(msk+off)!=0)
        {
            accum=fading*accum;
            temp=(BYTE)(accum/rate+0.5);
            accum+=*(dst+off);
            *(dst+off)=temp;
        }
        i++;
    }
    accum=0; i--;
    while (i>=0)
    {
        off=width*(i%height)+i/height;
        if (*(msk+off)!=0)
        {
            accum=fading*accum;
            temp=(BYTE)(accum/rate+0.5);
            accum+=*(dst+off);
            *(dst+off)=temp;
        }
        i--;
    } 
}


__forceinline pTVFrame _lpass_1DH(pTVFrame frame, int __fr_nmb=CONOVOLUTION_PARAM, bool freesrc=false)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    if (__fr_nmb>=1000)
    {
        TRACE("!!! Warrning: \"_lpass\" - requested too high __fr_nmb==%d, truncated to 999\n",__fr_nmb);
        __fr_nmb=999;
    }
    if (__fr_nmb<=0)
    {
        TRACE("!!! Warrning: \"_lpass\" - requested too low __fr_nmb==%d, truncated to 1\n",__fr_nmb);
        __fr_nmb=1;
    }

    pTVFrame dstframe=makecopyTVFrame(frame);
    
    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;

    LPBYTE src=GetData(frame);
    LPBYTE dst=GetData(dstframe);
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
        _lpass_1DH( width, depth,  dst,__fr_nmb);
        break;
    case BI_Y8:
        _lpass_1DH( width, depth, dst,__fr_nmb);
        break;
    case BI_Y16:
        _lpass_1DH16( width, depth, (LPWORD)dst,__fr_nmb);
        break;
    }
    
    if (freesrc) freeTVFrame(frame);
    return dstframe;
}

__forceinline pTVFrame _lpass_1DV(pTVFrame frame, int __fr_nmb=CONOVOLUTION_PARAM, bool freesrc=false)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    if (__fr_nmb>=1000)
    {
        TRACE("!!! Warrning: \"_lpass\" - requested too high __fr_nmb==%d, truncated to 999\n",__fr_nmb);
        __fr_nmb=999;
    }
    if (__fr_nmb<=0)
    {
        TRACE("!!! Warrning: \"_lpass\" - requested too low __fr_nmb==%d, truncated to 1\n",__fr_nmb);
        __fr_nmb=1;
    }
    
    pTVFrame dstframe=makecopyTVFrame(frame);

    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;

        LPBYTE src=GetData(frame);
    LPBYTE dst=GetData(dstframe);

    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
        _lpass_1DV( width, depth, dst,__fr_nmb);
        break;
    case BI_Y8:
        _lpass_1DV( width, depth, dst,__fr_nmb);
        break;
    case BI_Y16:
        _lpass_1DV16( width, depth, (LPWORD)dst,__fr_nmb);
        break;
    }
    if (freesrc) freeTVFrame(frame);
    return dstframe;
}


__forceinline pTVFrame _hpass_1DH(pTVFrame frame, int __fr_nmb=CONOVOLUTION_PARAM, bool freesrc=false)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    if (__fr_nmb>=1000)
    {
        TRACE("!!! Warrning: \"_hpass\" - requested too high __fr_nmb==%d, truncated to 999\n",__fr_nmb);
        __fr_nmb=999;
    }
    if (__fr_nmb<=0)
    {
        TRACE("!!! Warrning: \"_hpass\" - requested too low __fr_nmb==%d, truncated to 1\n",__fr_nmb);
        __fr_nmb=1;
    }

    pTVFrame dstframe =_lpass_1DH(frame,__fr_nmb);
    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;

    int size=depth*width;
    int i=0;
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
        {
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstframe);

            while (i<size)
            {
                int s=128+(*(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>255)?255:s));
                i++;
            }
            break;
        }
    case BI_Y8:
        {
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstframe);

            while (i<size)
            {
                int s=128+(*(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>255)?255:s));
                i++;
            }
            break;
        }
    case BI_Y16:
        {
            LPWORD src=(LPWORD)GetData(frame);
            LPWORD dst=(LPWORD)GetData(dstframe);
            while (i<size)
            {
                int s=32768+(*(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>65535)?65535:s));
                i++;
            }
        }
        break;
    }
    if (freesrc) freeTVFrame(frame);
    return dstframe;
}

__forceinline pTVFrame _hpass_1DV(pTVFrame frame, int __fr_nmb=CONOVOLUTION_PARAM, bool freesrc=false)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    if (__fr_nmb>=1000)
    {
        TRACE("!!! Warrning: \"_hpass\" - requested too high __fr_nmb==%d, truncated to 999\n",__fr_nmb);
        __fr_nmb=999;
    }
    if (__fr_nmb<=0)
    {
        TRACE("!!! Warrning: \"_hpass\" - requested too low __fr_nmb==%d, truncated to 1\n",__fr_nmb);
        __fr_nmb=1;
    }

    pTVFrame dstframe =_lpass_1DV(frame,__fr_nmb);
    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;

    int size;
    int i=0;
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
        {
            size=9*(depth*width/8);
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstframe);
            while (i<size)
            {
                int s=128+(*(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>255)?255:s));
                i++;
            }
            break;
        }
    case BI_Y8:
        {
            size=depth*width;
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstframe);
            while (i<size)
            {
                int s=128+(*(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>255)?255:s));
                i++;
            }
            break;
        }
    case BI_Y16:
        {
            size=depth*width;
            LPWORD src=(LPWORD)GetData(frame);
            LPWORD dst=(LPWORD)GetData(dstframe);
            while (i<size)
            {
                int s=32768+(*(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>65535)?65535:s));
                i++;
            } 
        }
        break;
    }
    if (freesrc) freeTVFrame(frame);
    return dstframe;
}


__forceinline void _lpass_1D(pTVFrame frame, int kernel = 10)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;

    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
    case BI_Y8:
        {
            LPBYTE tmpB=(LPBYTE)malloc(depth*width);
            memcpy(tmpB,GetData(frame),depth*width);
            _lpass_1D(tmpB,GetData(frame),width,depth,kernel);
            free(tmpB);
            break;
        }
    case BI_Y16:
        {
            LPWORD tmpB=(LPWORD)malloc(depth*width*sizeof(WORD));
            memcpy(tmpB,GetData(frame),depth*width*sizeof(WORD));
            _lpass_1D16(tmpB,(LPWORD)GetData(frame),width,depth,kernel);
            free(tmpB);
            break;
        }
    }
}

// simple vertical filter: just remove point==0 filling them with last color
__forceinline void _v_fill0(pTVFrame frame)
{
    int width=frame->lpBMIH->biWidth;
    int height=frame->lpBMIH->biHeight;

        switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
    case BI_Y8:
        {
            LPBYTE data=GetData(frame)+width; //start from second line
            LPBYTE eod =GetData(frame)+width*(height);
            while (data<eod)
            {
                //if ((*data==0) && (*(data+width)!=0)) 
                if (*data==0) 
                    *data=*(data-width);
                data++;
            }
            break;
        }
    case BI_Y16:
        {
            LPWORD data=((LPWORD)GetData(frame))+width; //start from second line
            LPWORD eod =((LPWORD)GetData(frame))+width*(height);
            while (data<eod)
            {
                //if ((*data==0) && (*(data+width)!=0)) 
                if (*data==0) 
                    *data=*(data-width);
                data++;
            }
            break;
        }
    }
}

__forceinline pTVFrame _sharpen(pTVFrame frame, int __fr_nmb=CONOVOLUTION_PARAM, bool freesrc=false)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    if (__fr_nmb>=1000)
    {
        TRACE("!!! Warrning: \"_hpass\" - requested too high __fr_nmb==%d, truncated to 999\n",__fr_nmb);
        __fr_nmb=999;
    }
    if (__fr_nmb<=0)
    {
        TRACE("!!! Warrning: \"_hpass\" - requested too low __fr_nmb==%d, truncated to 1\n",__fr_nmb);
        __fr_nmb=1;
    }

    pTVFrame dstframe =_lpass(frame,__fr_nmb);
    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
    case BI_Y8:
        {
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstframe);

            int size=depth*width;
            int i=0;
            while (i<size)
            {
                int s=(2* *(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>255)?255:s));
                i++;
            }
            break;
        }
    case BI_Y16:
        {
            LPWORD src=(LPWORD)(GetData(frame));
            LPWORD dst=(LPWORD)(GetData(dstframe));

            int size=depth*width;
            int i=0;
            while (i<size)
            {
                int s=(2* *(src+i)-*(dst+i));
                *(dst+i)=((s<0)?0:((s>0xFFFF)?0xFFFF:s));
                i++;
            }
            break;
        }
        break;
    }

    if (freesrc) freeTVFrame(frame);
    return dstframe;
}

__forceinline LPBITMAPINFOHEADER  _sharpen(LPBITMAPINFOHEADER bmih, int __fr_nmb=CONOVOLUTION_PARAM)
{
    TVFrame src={bmih,NULL};
    pTVFrame dst=_sharpen(&src, __fr_nmb, false);
    ASSERT(dst->lpData==NULL);
    bmih=dst->lpBMIH;
    free(dst);
    return bmih;
}

typedef enum boxsize{ box3x3=0, box5x5=1, box7x7=2 };

__forceinline pTVFrame _sqrfilter8(pTVFrame frame, boxsize box)
{
    pTVFrame retV=(pTVFrame)malloc(sizeof(TVFrame)); memset(retV,0,sizeof(TVFrame));
    retV->lpBMIH=(LPBITMAPINFOHEADER)malloc(getsize4BMIH(frame));
    memcpy(retV->lpBMIH,frame->lpBMIH,frame->lpBMIH->biSize);

    int width=Width(frame);
    int height=Height(frame);

    if (frame->lpBMIH->biCompression==BI_YUV9) // copy color info
    {
        memcpy(GetData(retV)+width*height,GetData(frame)+width*height, (width*height)/8);
    }
    // prepare table of horizontal sums
    double* ht=(double*)malloc(sizeof(double)*width*height);
    memset(ht,0,sizeof(double)*width*height);
    for(int y=0; y<height; y++)
    {
        LPBYTE src=GetData(frame)+y*width;
        LPBYTE eod=src+width;
        double*dst=ht+y*width;
        double acc=0;
        while(src<eod)
        {
            acc+=*src;
            *dst=acc;
            src++; dst++;
        }
    }
    int delta=0;
    switch (box)
    {
    case box3x3:
        delta=1;
        break;
    case box5x5:
        delta=2;
        break;
    case box7x7:
        delta=3;
        break;
    }
    for (int y=0; y<height; y++)
    {
        for (int x=0; x<width; x++)
        {
            CRect rc(x-delta,y-delta,x+delta,y+delta);
            if (rc.left<0) rc.left=0;
            if (rc.top<0) rc.top=0;
            if (rc.right>=width) rc.right=width-1;
            if (rc.bottom>=height) rc.bottom=height-1;
            double sum=0;
            for (int i=rc.top; i<rc.bottom; i++)
            {
                sum+=ht[rc.right+i*width]-ht[rc.left+i*width];
            }
            sum/=rc.Width()*rc.Height();
            GetData(retV)[x+y*width]=(BYTE)(sum+0.5);
        }
    }
    free(ht);
    return retV;
}

__forceinline pTVFrame _sqrfilter16(pTVFrame frame, boxsize box)
{
    pTVFrame retV=(pTVFrame)malloc(sizeof(TVFrame)); memset(retV,0,sizeof(TVFrame));
    retV->lpBMIH=(LPBITMAPINFOHEADER)malloc(getsize4BMIH(frame));
    memcpy(retV->lpBMIH,frame->lpBMIH,frame->lpBMIH->biSize);
    //memset(GetData(retV),0,GetImageSize(retV));
    int width=Width(frame);
    int height=Height(frame);
    // prepare table of horizontal sums
    double* ht=(double*)malloc(sizeof(double)*width*height);
    memset(ht,0,sizeof(double)*width*height);
    for(int y=0; y<height; y++)
    {
        LPWORD src=((LPWORD)GetData(frame))+y*width;
        LPWORD eod=src+width;
        double*dst=ht+y*width;
        double acc=0;
        while(src<eod)
        {
            acc+=*src;
            *dst=acc;
            src++; dst++;
        }
    }
    int delta=0;
    switch (box)
    {
    case box3x3:
        delta=1;
        break;
    case box5x5:
        delta=2;
        break;
    case box7x7:
        delta=3;
        break;
    }
    for (int y=0; y<height; y++)
    {
        for (int x=0; x<width; x++)
        {
            CRect rc(x-delta,y-delta,x+delta,y+delta);
            if (rc.left<0) rc.left=0;
            if (rc.top<0) rc.top=0;
            if (rc.right>=width) rc.right=width-1;
            if (rc.bottom>=height) rc.bottom=height-1;
            double sum=0;
            for (int i=rc.top; i<rc.bottom; i++)
            {
                sum+=ht[rc.right+i*width]-ht[rc.left+i*width];
            }
            sum/=rc.Width()*rc.Height();
            ((LPWORD)GetData(retV))[x+y*width]=(WORD)(sum+0.5);
        }
    }
    free(ht);
    return retV;
}

__forceinline pTVFrame _sqrfilter(pTVFrame frame, boxsize box)
{
    if ((!frame) || (!frame->lpBMIH)) return NULL;
    switch (frame->lpBMIH->biCompression)
    {
        case BI_YUV9:
        case BI_Y8:
            return _sqrfilter8(frame, box);
        case BI_Y16:
            return _sqrfilter16(frame, box);
    }
    return NULL;
}

#endif