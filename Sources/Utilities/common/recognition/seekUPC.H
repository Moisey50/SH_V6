//  $File : seekUPC.h - Image covolution. Seek regions with high intensity of space high frequency
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#ifndef _SEEKUPC_INC
#define _SEEKUPC_INC

#include <imageproc\videologic.h>
#include <imageproc\FeatureDetector.h>
#include <imageproc\clusters\Clusters.h>
#include <imageproc\clusters\ClusterOp.h>

inline void _marklineuniformity(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    for (int y=0; y<h; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE le=ls+w;
        LPBYTE sc=ls;
        int avg=0;
        while(sc<le)
        {
            avg+=*sc;
            sc++;
        }
        avg/=w;
        int acc=0;
        sc=ls;
        while(sc<le)
        {
            acc+=abs(*sc-avg);
            sc++;
        }
        acc/=w;
        memset(ls,acc,w);
    }
}

inline void _zOutErode(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    int nw=(w/2);
    int nh=(h/2);
    nw=((nw/4)*4);//+((nw/4)?4:0);
    nh=((nh/4)*4);//+((nh/4)?4:0);
    int newSize=(9*nw*nh)/8;
    LPBITMAPINFOHEADER newBM=(LPBITMAPINFOHEADER)malloc(newSize+sizeof(BITMAPINFOHEADER));
    LPBYTE newData=((LPBYTE)newBM)+sizeof(BITMAPINFOHEADER);
    memcpy(newBM,frame->lpBMIH,sizeof(BITMAPINFOHEADER));
    newBM->biHeight=nh;
    newBM->biWidth=nw;
    newBM->biSizeImage=newSize;
    memset(newData,128,newSize);
    for (int y=0; y<h/2; y++)
    {
        for (int x=0; x<w/2; x++)
        {
            LPBYTE off=data+2*(x+w*y);
            int r=*off;
            if (r>(*(off+1)))   r=*(off+1);
            if (r>(*(off+w)))   r=*(off+w);
            if (r>(*(off+w+1))) r=*(off+w+1);
            *(newData+x+nw*y)=r;
        }
    }
    if (frame->lpData)
    {
        free(frame->lpData); frame->lpData=NULL;
    }
    frame->lpBMIH=newBM;
}

inline void __mrkLcMin(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    memset(data+w*h,128,w*h/8);
    for (int y=0; y<h; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE le=ls+w-1;
        LPBYTE sc=ls;
        int dif;
        while(sc<le)
        {
            dif=abs(*(sc+1)-*sc);
            *sc=dif;
            sc++;
        }
        *sc=0;
    }
}

inline void _mrkLMax(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    memset(data+w*h,128,w*h/8);
    for (int y=0; y<h; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE le=ls+w-1;
        LPBYTE sc=ls;
        int dif=0;
        while(sc<le)
        {
            dif+=abs(*(sc+1)-*sc);
            sc++;
        }
        dif/=(w-1);
        memset(ls,dif,w);
    }
}


inline void _mrkLcMin(pTVFrame frame)
{
    pTVFrame f=_lpass_1DH(frame,800);//_lpass(frame,800);
    LPBYTE data=GetData(frame);
    LPBYTE fdata=GetData(f);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    memset(data+w*h,128,w*h/8);
    for (int y=0; y<h; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE le=ls+w;
        LPBYTE sc=ls;
        int dif;
        while(sc<le)
        {
            dif=(*fdata-*sc);
            *sc=(dif<0)?255:(255-dif);
            sc++;
            fdata++;
        }
    }
    freeTVFrame(f);
}

inline void _normalize_s(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    for (int y=0; y<h; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE le=ls+w-1;
        LPBYTE sc=ls;
        int min=255, max=0;
        while(sc<le)
        {
            if (min>*sc) min=*sc;
            if (max<*sc) max=*sc;
            sc++;
        }
        sc=ls;
        min=128-min;
        max=max-128;
        min=(min<max)?max:min; 
        if (min==0)
        {
            memset(sc,128,w);
        }
        else
        {
            while(sc<le)
            {
                int res=(*sc-128);
                res=128+res*128/min;
                *sc=(res<0)?0:(res>255)?255:res;
                sc++;
            }
        }
    }

}

inline void _lnDelta(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    for (int y=0; y<h; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE le=ls+w-1;
        LPBYTE sc=ls;
        int dif;
        while(sc<le)
        {
            dif=(*(sc+1)-*sc)+128;
            *sc=(dif<0)?0:(dif>255)?255:dif;
            sc++;
        }
        *sc=128;
    }
}

inline void _extr(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    LPBYTE org=(LPBYTE)malloc(w*h);
    memcpy(org,data,w*h);
    for (int y=0; y<h; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE os=org +w*y;
        LPBYTE le=ls+w-2;
        LPBYTE sc=ls;
        //int dif;
        while(sc<le)
        {
            sc++; os++;
        }
        *sc=128;
    }
    free(org);
}

inline void _lnSharpen(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    LPBYTE org=(LPBYTE)malloc(w*h);
    memcpy(org,data,w*h);
    for (int y=0; y<h; y++)
    {
        LPBYTE sc=data+w*y;
        LPBYTE le=sc+w-1;
        LPBYTE os=org+w*y+1;
        int dif;
        while(sc<le)
        {
            dif=(*(os+1)-*(os-1))+*sc;
            *sc=(dif<0)?0:(dif>255)?255:dif;
            sc++; os++;
        }
    }
    free(org);
}


inline void _lnADelta(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    for (int y=0; y<h; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE le=ls+w-1;
        LPBYTE sc=ls;
        int dif;
        while(sc<le)
        {
            dif=2*abs(*(sc+1)-*sc);
            *sc=(dif<0)?0:(dif>255)?255:dif;
            sc++;
        }
        *sc=128;
    }
}

inline void _killHLines(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    for (int y=1; y<h-1; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE le=ls+w-1;
        LPBYTE sc=ls;
        //int dif;
        while(sc<le)
        {
            if ((*sc==0) && ((*(sc+w)!=0) && (*(sc-w)!=0)))
            {
                *sc=255;
            }
            sc++;
        }
        *sc=128;
    }
}

inline void _InvDelta(pTVFrame frame)
{
    #define fieldcolor 255
    LPBYTE data=GetData(frame);
    int w=frame->lpBMIH->biWidth;
    int h=frame->lpBMIH->biHeight;
    LPBYTE cpy=(LPBYTE)malloc(w*h);
    memcpy(cpy,data,w*h);
//    memset(data, 0,w*h);
    memset(data,fieldcolor,w);
    memset(data+w*(h-1),fieldcolor,w);
    for (int y=1; y<h-1; y++)
    {
        LPBYTE ls=data+w*y;
        LPBYTE le=ls+w-1;
        LPBYTE sc=ls;
        LPBYTE cc=cpy+w*y;
        unsigned dif=255;
        *ls=fieldcolor;
        while(sc<le)
        {
            if ((*cc<88) && (*(cc+1)>*cc))
            {
                dif=0;
                //dif=(dif<0)?0:(dif>255)?255:0;
                *sc=dif;
            }
            else if ((*cc>168) && (*(cc+1)<*cc))
            {
                dif=fieldcolor;
                *sc=fieldcolor;
            }
            else *sc=dif;
            sc++; cc++;
        } 
        sc=le-2;
        cc=cpy+w*y+w-2;
        while (sc>ls)
        {
            if ((*cc<88) && (*(cc-1)>*cc))
            {
                dif=255;
                //dif=(dif<0)?0:(dif>255)?255:0;
                *sc|=dif;
            }
            else if ((*cc>168) && (*(cc-1)<*cc))
            {
                dif=0;
                *sc|=dif;
            }
            else *sc|=dif;
            sc--;
            cc--;
        }  
    } 
    free(cpy);
}


#endif