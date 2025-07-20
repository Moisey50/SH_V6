//  $File : motiondetectors.h - primitives for motion detectors
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef MOTIONDETECTORS_INC
#define MOTIONDETECTORS_INC

#include <imageproc\simpleip.h>

#define ABS(a) (((a)<0)?-(a):(a))

inline double compareframes_abs(pTVFrame a, pTVFrame b)
{
    if(memcmp(a->lpBMIH,b->lpBMIH,a->lpBMIH->biSize)!=0) return 0;

    double i=0.0;
    int datasize=a->lpBMIH->biWidth*a->lpBMIH->biHeight;
    LPBYTE pA=GetData(a);
    LPBYTE pB=GetData(b);
    LPBYTE endpntr=pA+datasize;
    while (pA<endpntr)
    {
        i+=ABS(*pA - *pB); pA++; pB++;
    }
    return i/datasize;
}

inline double compareframes_val(pTVFrame a, pTVFrame b)
{
    if(memcmp(a->lpBMIH,b->lpBMIH,a->lpBMIH->biSize)!=0) return 0;

    double i=0.0;
    int datasize=a->lpBMIH->biWidth*a->lpBMIH->biHeight;
    LPBYTE pA=(a->lpData)?a->lpData:((LPBYTE)a->lpBMIH)+a->lpBMIH->biSize;
    LPBYTE pB=(b->lpData)?b->lpData:((LPBYTE)b->lpBMIH)+b->lpBMIH->biSize;
    LPBYTE endpntr=pA+datasize;
    while (pA<endpntr)
    {
        i+=(*pA - *pB); pA++; pB++;
    }
    return 10*ABS(i)/datasize;
}


inline double compareframes_max(pTVFrame a, pTVFrame b)
{
    if(memcmp(a->lpBMIH,b->lpBMIH,a->lpBMIH->biSize)!=0) return 0;
    int hist[256];
    memset(hist,0,sizeof(int)*256);
    double i=0.0;
    int datasize=a->lpBMIH->biWidth*a->lpBMIH->biHeight;
    LPBYTE pA=(a->lpData)?a->lpData:((LPBYTE)a->lpBMIH)+a->lpBMIH->biSize;
    LPBYTE pB=(b->lpData)?b->lpData:((LPBYTE)b->lpBMIH)+b->lpBMIH->biSize;
    LPBYTE endpntr=pA+datasize-a->lpBMIH->biWidth;
    pA+=a->lpBMIH->biWidth;
    while (pA<endpntr)
    {
//        if (i<ABS(*pA - *pB)) i=ABS(*pA - *pB); pA++; pB++;
          hist[ABS(*pA - *pB)]++; pA++; pB++;
    }
    int pcnt=0; int l=255;
    double val=0;
    while (pcnt<datasize/5)
    {
        pcnt+=hist[l];
        val+=((double)l)*hist[l];
        l--;
    }
    return 0.33*val/pcnt;
}

inline pTVFrame compareframes(pTVFrame a, pTVFrame b)
{
    if(memcmp(a->lpBMIH,b->lpBMIH,a->lpBMIH->biSize)!=0) return NULL;

    int oldwidth=a->lpBMIH->biWidth,oldheight=a->lpBMIH->biHeight;
  	int width=(oldwidth>>1),height=(oldheight>>1); 
    width=((width>>2)<<2);
    height=((height>>2)<<2); 
    int new_size=width*height*9/8;

    LPBITMAPINFOHEADER bmih=(LPBITMAPINFOHEADER)malloc(a->lpBMIH->biSize+new_size);
    memcpy(bmih,a->lpBMIH,a->lpBMIH->biSize);
    bmih->biHeight=height;
    bmih->biWidth=width;
    bmih->biSizeImage=new_size;
            
    LPBYTE dst  = ((LPBYTE)bmih)+bmih->biSize;
    LPBYTE src1 = ((LPBYTE)(a->lpBMIH))   +bmih->biSize;
    LPBYTE src2 = ((LPBYTE)(b->lpBMIH))   +bmih->biSize;
    memset(&dst[width*height],128,width*height/8);
    for (int y=0; y<height; y++)
    {
         for(int x=0; x<width; x++)
         {
             int z=((src2[2*x+4*y*width]+src2[1+2*x+4*y*width]+src2[2*x+2*(2*y+1)*width]+src2[1+2*x+2*(2*y+1)*width])-
                                 (src1[2*x+4*y*width]+src1[1+2*x+4*y*width]+src1[2*x+2*(2*y+1)*width]+src1[1+2*x+2*(2*y+1)*width]));
             dst[x+y*width]=2*ABS((z<-128)?-128:(z>128)?128:z);
                 
/*                 abs(src2[2*x+4*y*width]-src1[2*x+4*y*width])+
                            abs(src2[1+2*x+4*y*width]-src1[1+2*x+4*y*width])+
                            abs(src2[2*x+2*(2*y+1)*width]-src1[2*x+2*(2*y+1)*width])+
                            abs(src2[1+2*x+2*(2*y+1)*width]-src1[1+2*x+2*(2*y+1)*width]); */
         }
    }
    
    pTVFrame Frame=newTVFrame(bmih,NULL);
    Frame=_lpass(Frame,400);
    _normalize(Frame);
    return Frame;
}

inline pTVFrame compareframes2(pTVFrame a, pTVFrame b)
{
    if(memcmp(a->lpBMIH,b->lpBMIH,a->lpBMIH->biSize)!=0) return NULL;

    int oldwidth=a->lpBMIH->biWidth,oldheight=a->lpBMIH->biHeight;
  	int width=(oldwidth>>1),height=(oldheight>>1); 
    width=((width>>2)<<2);
    height=((height>>2)<<2); 
    int new_size=width*height*9/8;

    LPBITMAPINFOHEADER bmih=(LPBITMAPINFOHEADER)malloc(a->lpBMIH->biSize+new_size);
    memcpy(bmih,a->lpBMIH,a->lpBMIH->biSize);
    bmih->biHeight=height;
    bmih->biWidth=width;
    bmih->biSizeImage=new_size;
            
    LPBYTE dst  = ((LPBYTE)bmih)+bmih->biSize;
    LPBYTE src1 = ((LPBYTE)(a->lpBMIH))   +bmih->biSize;
    LPBYTE src2 = ((LPBYTE)(b->lpBMIH))   +bmih->biSize;
    memset(&dst[width*height],128,width*height/8);
    for (int y=0; y<height; y++)
    {
         for(int x=0; x<width; x++)
         {
             int z=((src2[2*x+4*y*width]+src2[1+2*x+4*y*width]+src2[2*x+2*(2*y+1)*width]+src2[1+2*x+2*(2*y+1)*width])-
                                 (src1[2*x+4*y*width]+src1[1+2*x+4*y*width]+src1[2*x+2*(2*y+1)*width]+src1[1+2*x+2*(2*y+1)*width]));
             dst[x+y*width]=2*ABS((z<-128)?-128:(z>128)?128:z);
                 
/*                 abs(src2[2*x+4*y*width]-src1[2*x+4*y*width])+
                            abs(src2[1+2*x+4*y*width]-src1[1+2*x+4*y*width])+
                            abs(src2[2*x+2*(2*y+1)*width]-src1[2*x+2*(2*y+1)*width])+
                            abs(src2[1+2*x+2*(2*y+1)*width]-src1[1+2*x+2*(2*y+1)*width]); */
         }
    }
    
    pTVFrame Frame=newTVFrame(bmih,NULL);
    return Frame;
}

inline pTVFrame frames_dif(pTVFrame a, pTVFrame b)
{
    if(memcmp(a->lpBMIH,b->lpBMIH,a->lpBMIH->biSize)!=0) return NULL;

    int width=a->lpBMIH->biWidth,height=a->lpBMIH->biHeight;

    int size=a->lpBMIH->biSizeImage;

    LPBITMAPINFOHEADER bmih=(LPBITMAPINFOHEADER)malloc(a->lpBMIH->biSize+size);
    memcpy(bmih,a->lpBMIH,a->lpBMIH->biSize);
    bmih->biHeight=height;
    bmih->biWidth=width;
    bmih->biSizeImage=size;
            
    LPBYTE dst  = ((LPBYTE)bmih)+bmih->biSize;
    LPBYTE src1 = GetData(a); 
    LPBYTE src2 = GetData(b); 
    memset(&dst[width*height],128,width*height/8);
    for (int y=0; y<height; y++)
    {
         for(int x=0; x<width; x++)
         {
             int z=(int)(*src1) - *src2;
             //*dst=2*abs((z<-128)?-128:(z>128)?128:z);
             *dst=((z<-128)?-128:(z>128)?128:z)+128;
             src1++; src2++; dst++;
         }
    }
    pTVFrame Frame=newTVFrame(bmih,NULL);
    Frame=_lpass(Frame,400);
    _normalize(Frame);
    return Frame;
}

inline pTVFrame frames_difpos(pTVFrame a, pTVFrame b)
{
    if(memcmp(a->lpBMIH,b->lpBMIH,a->lpBMIH->biSize)!=0) 
    {
        return NULL;
    }

    int width=a->lpBMIH->biWidth,height=a->lpBMIH->biHeight;

    int size=a->lpBMIH->biSizeImage;

    LPBITMAPINFOHEADER bmih=(LPBITMAPINFOHEADER)malloc(a->lpBMIH->biSize+size);
    memcpy(bmih,a->lpBMIH,a->lpBMIH->biSize);
    bmih->biHeight=height;
    bmih->biWidth=width;
    bmih->biSizeImage=size;
            
    LPBYTE dst  = ((LPBYTE)bmih)+bmih->biSize;
    LPBYTE src1 = GetData(a); 
    LPBYTE src2 = GetData(b); 
    memset(&dst[width*height],128,width*height/8);
    LPBYTE eof=src1+width*height;
    while (src1<eof)
    {
         int z=(int)(*src1) - *src2;
         ASSERT(z<256);
         *dst=((z<0)?0:z);
         src1++; src2++; dst++;
    }
    pTVFrame Frame=newTVFrame(bmih,NULL);
    return Frame;
}

inline pTVFrame frames_difabs(pTVFrame a, pTVFrame b)
{
    if(memcmp(a->lpBMIH,b->lpBMIH,a->lpBMIH->biSize)!=0) return NULL;

    int width=a->lpBMIH->biWidth,height=a->lpBMIH->biHeight;

    int size=a->lpBMIH->biSizeImage;

    LPBITMAPINFOHEADER bmih=(LPBITMAPINFOHEADER)malloc(a->lpBMIH->biSize+size);
    memcpy(bmih,a->lpBMIH,a->lpBMIH->biSize);
    bmih->biHeight=height;
    bmih->biWidth=width;
    bmih->biSizeImage=size;
            
    LPBYTE dst  = ((LPBYTE)bmih)+bmih->biSize;
    LPBYTE src1 = GetData(a); 
    LPBYTE src2 = GetData(b); 
    memset(&dst[width*height],128,width*height/8);
    for (int y=0; y<height; y++)
    {
         for(int x=0; x<width; x++)
         {
             int z=(int)(*src1) - *src2;
             *dst=2*abs((z<-128)?-128:(z>128)?128:z);
             src1++; src2++; dst++;
         }
    }
    pTVFrame Frame=newTVFrame(bmih,NULL);
    Frame=_lpass(Frame,400);
    _normalize(Frame);
    return Frame;
}

#endif