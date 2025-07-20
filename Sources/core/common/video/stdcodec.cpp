//  $File : stdcodec.cpp - interface functions to VFW video codec interface
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#include "stdafx.h"
#include <video\shvideo.h>
#include <helpers\16bit.h>
//#ifndef NOTSBYUV_KILL
//    #include <helpers\tsbyuvkill.h>
//#endif

#pragma comment( lib, "Vfw32.lib" )


class CHic
{
public:
  HIC m_HIC_Compress;
  HIC m_HIC_Decompress;
public:
  CHic(DWORD Handler);
  ~CHic();
};

CHic::CHic(DWORD Handler)
{
  //#ifndef NOTSBYUV_KILL
  //    ClearTSBYUV();
  //#endif
  m_HIC_Compress = ICOpen(mmioFOURCC('v','i','d','c'), Handler, ICMODE_COMPRESS);
  m_HIC_Decompress = ICOpen(mmioFOURCC('v','i','d','c'), Handler, ICMODE_DECOMPRESS);
}

CHic::~CHic()
{
  ICClose(m_HIC_Compress);
  ICClose(m_HIC_Decompress);
}

CHic HICYUV9(0x39555659);

bool _decompress_any(pTVFrame frame)
{
  HANDLE h = ICImageDecompress(NULL,0,(LPBITMAPINFO)frame->lpBMIH,frame->lpData,NULL);
  if (h)
  {
    LPBITMAPINFOHEADER  lpbi;
    lpbi=(LPBITMAPINFOHEADER)GlobalLock(h);
    if (frame->lpData)
    {
      free(frame->lpData); 
      frame->lpData=NULL;
    }
    else 
      free(frame->lpBMIH);
    frame->lpBMIH = (LPBITMAPINFOHEADER)malloc(lpbi->biSize + lpbi->biSizeImage);
    memcpy( frame->lpBMIH , lpbi , lpbi->biSize+lpbi->biSizeImage );
    GlobalUnlock(h);
    GlobalFree(h);
    return true;
  }
  return false;
}

bool    makeYUV9(pTVFrame frame)
{
  if ((frame->lpBMIH->biCompression==BI_YUV9) || (frame->lpBMIH->biCompression==BI_Y8))
  { 
#if (defined _DEBUG) && (defined _DETAIL_LOG)
    TRACE("+++ stdcodec.cpp: Load - YUV9 or Y8 formats\n"); 
#endif
    return true;
  }
#if (defined _DEBUG) && (defined _DETAIL_LOG)
  if (frame->lpBMIH->biCompression!=BI_RGB)
  {
#define frm ((char*)(&(frame->lpBMIH->biCompression)))
    TRACE("+++ stdcodec.cpp: LOAD %c%c%c%c\n",frm[0],frm[1],frm[2],frm[3]);
  } else TRACE("+++ stdcodec.cpp: Load - BI_RGB\n");
#endif
  if (frame->lpBMIH->biCompression!=BI_RGB)
  {
    if (!_decompress_any(frame)) 
      return false;
  }   
  if (frame->lpBMIH->biCompression==BI_RGB)
  {
    if (compress2yuv9(frame)) 
      return true;
  }
  return false;
}

bool    makeYUV9(LPBITMAPINFOHEADER* frame)
{
  TVFrame tvframe;
  tvframe.lpBMIH=*frame;
  tvframe.lpData=NULL;
  if (makeYUV9(&tvframe))
  {
    *frame=tvframe.lpBMIH;
    return true;
  }
  return false;
}

LPBITMAPINFOHEADER _decompress_raw(pTVFrame frame)
{
  ASSERT(frame->lpBMIH->biSizeImage);
  BITMAPINFOHEADER bmih;
  memcpy(&bmih,frame->lpBMIH,frame->lpBMIH->biSize);
  bmih.biBitCount=24;
  HIC hic=(frame->lpBMIH->biCompression==BI_YUV9)?HICYUV9.m_HIC_Decompress:NULL;
  LPBYTE data=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);
  HANDLE h = ICImageDecompress(NULL,0,(LPBITMAPINFO)frame->lpBMIH,data,(LPBITMAPINFO)&bmih);
  if (h)
  {
    LPBITMAPINFOHEADER  lpbi;
    lpbi=(LPBITMAPINFOHEADER)GlobalLock(h);
    data=(LPBYTE)malloc(lpbi->biSizeImage+lpbi->biSize);
    memcpy(data,lpbi,lpbi->biSizeImage+lpbi->biSize);
    GlobalUnlock(h);
    GlobalFree(h);
    return((LPBITMAPINFOHEADER)data);
  }
  else
  {
    return yuv9rgb24(frame->lpBMIH, frame->lpData);
  }
  return(NULL);
}

LPBITMAPINFOHEADER _compress_yuv9(LPBITMAPINFOHEADER in)
{
  LPBITMAPINFOHEADER out;
  DWORD width  = in->biWidth;
  DWORD height = in->biHeight;
  DWORD size   = (9*width*height)/8;
  out=(LPBITMAPINFOHEADER)malloc(size+in->biSize);
  memcpy(out,in,in->biSize);
  out->biClrUsed=0;
  out->biSizeImage=size;
  out->biCompression=BI_YUV9;
  LPBYTE src=((LPBYTE)in) + in->biSize;
  LPBYTE dst=((LPBYTE)out)+ out->biSize;

  HANDLE h = ICImageCompress(HICYUV9.m_HIC_Compress,0,(LPBITMAPINFO)in,src,(LPBITMAPINFO)out,10000,(long*)&size);
  if (h)
  {
    LPBITMAPINFOHEADER  lpbi;
    lpbi=(LPBITMAPINFOHEADER)GlobalLock(h);
    ASSERT(lpbi->biSizeImage==out->biSizeImage);
    ASSERT(lpbi->biSize==out->biSize);
    memcpy(out,lpbi,lpbi->biSizeImage+lpbi->biSize);
    GlobalUnlock(h);
    GlobalFree(h);
    return(out);
  }
  return(NULL);
}

LPBITMAPINFOHEADER _compress(LPBITMAPINFOHEADER in, HIC compressor)
{
  LPBYTE src=((LPBYTE)in) + in->biSize;
  long size=10000;
  HANDLE h=NULL;
  if (in->biCompression!=BI_RGB)
  {
    HIC hic=(in->biCompression==BI_YUV9)?HICYUV9.m_HIC_Decompress:NULL;
    LPBYTE data=(((LPBYTE)in)+in->biSize);
    HANDLE htemp = ICImageDecompress(NULL,0,(LPBITMAPINFO)in,data,NULL);
    if (htemp)
    {
      LPBITMAPINFOHEADER  lpbi=(LPBITMAPINFOHEADER)GlobalLock(htemp);
      h = ICImageCompress(compressor,0,(LPBITMAPINFO)lpbi,NULL,NULL,5000,(long*)&size);
      GlobalUnlock(htemp);
      GlobalFree(htemp);
    }
  }
  else
  {
    h = ICImageCompress(compressor,0,(LPBITMAPINFO)in,src,NULL,5000,(long*)&size);
  }
  if (h)
  {
    LPBITMAPINFOHEADER  lpbi,out;
    lpbi=(LPBITMAPINFOHEADER)GlobalLock(h);
    out =(LPBITMAPINFOHEADER) malloc(lpbi->biSizeImage+lpbi->biSize);
    memcpy(out,lpbi,lpbi->biSizeImage+lpbi->biSize);
    GlobalUnlock(h);
    GlobalFree(h);
    return(out);
  }
  return(NULL);
}

LPBITMAPINFOHEADER _convertYUY2YUV9(pTVFrame frame)
{
  LPBITMAPINFOHEADER retV=NULL;
  ASSERT(frame->lpBMIH->biCompression==BI_YUY2);
  LPBYTE org=GetData(frame);
  int    orgSize  = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight= frame->lpBMIH->biHeight;

  int dstWidth = (orgWidth/4)*4;
  int dstHeight= (orgHeight/4)*4;

  int retSize=9*dstWidth*dstHeight/8;
  retV=(LPBITMAPINFOHEADER)malloc(frame->lpBMIH->biSize+retSize);
  memcpy(retV,frame->lpBMIH,frame->lpBMIH->biSize);

  retV->biWidth=dstWidth;
  retV->biHeight=dstHeight;
  retV->biSizeImage=retSize;
  retV->biCompression=BI_YUV9;
  retV->biBitCount=9;
  LPBYTE dst=GetData(retV);

  int eod=(dstWidth*dstHeight)/16;
  int width1=dstWidth;
  int width2=width1<<1;
  int width3=width1+width2;
  int width4=width1<<2;
  int width5=width4+width2;
  LPBYTE offU=dst+(dstWidth*dstHeight);
  LPBYTE offV=offU+eod;
  for (int i=0; i<eod; )
  {
    int U=0,V=0;
    V=*(org+1)+*(org+5);                  U=*(org+3)+*(org+7);
    V+=*(org+width2+1)+*(org+width2+5);   U+=*(org+width2+3)+*(org+width2+7);
    V+=*(org+width4+1)+*(org+width4+5);   U+=*(org+width4+3)+*(org+width4+7);
    V+=*(org+width5+1)+*(org+width5+5);   U+=*(org+width5+3)+*(org+width5+7);

    *offU=(BYTE)(U/8); offU++;
    *offV=(BYTE)(V/8); offV++;

    *dst = *(org);    *(dst+width1) =*(org+width2); *(dst+width2) =*(org+width4); *(dst+width3) =*(org+width5); dst++; org+=2;
    *dst = *(org);    *(dst+width1) =*(org+width2); *(dst+width2) =*(org+width4); *(dst+width3) =*(org+width5); dst++; org+=2;
    *dst = *(org);    *(dst+width1) =*(org+width2); *(dst+width2) =*(org+width4); *(dst+width3) =*(org+width5); dst++; org+=2;
    *dst = *(org);    *(dst+width1) =*(org+width2); *(dst+width2) =*(org+width4); *(dst+width3) =*(org+width5); dst++; org+=2;

    i++;
    if ((i%(width1/4))==0)
    {
      dst+=3*width1;
      org+=6*width1;
    }
  } 
  return retV;
}

LPBITMAPINFOHEADER  _convertYUY2YUV12(pTVFrame frame)
{
  LPBITMAPINFOHEADER retV=NULL;

  ASSERT(frame->lpBMIH->biCompression==BI_YUY2);
  LPBYTE org=GetData(frame);
  int    orgSize  = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight= frame->lpBMIH->biHeight;

  ASSERT((orgWidth%4)==0);
  ASSERT((orgHeight%4)==0);

  int retSize=12*frame->lpBMIH->biWidth*frame->lpBMIH->biHeight/8;
  retV=(LPBITMAPINFOHEADER)malloc(frame->lpBMIH->biSize+retSize);
  memcpy(retV,frame->lpBMIH,frame->lpBMIH->biSize);

  retV->biSizeImage=retSize;
  retV->biCompression=BI_YUV12;
  retV->biBitCount = 12;
  retV->biPlanes = 1;
  LPBYTE dst=GetData(retV);

  int eod=(retV->biWidth*retV->biHeight)/4;
  int width1=retV->biWidth;
  int width2=width1<<1;
  int width3=width1+width2;
  int width4=width1<<2;
  int width5=width4+width2;
  LPBYTE offU=dst+(retV->biWidth*retV->biHeight);
  LPBYTE offV=offU+eod;
  for (int i=0; i<eod-1; )
  {
    int U=0,V=0;
    V=*(org+1)+*(org+5);                  U=*(org+3)+*(org+7);
    V+=*(org+width2+1)+*(org+width2+5);   U+=*(org+width2+3)+*(org+width2+7);

    *offV=(BYTE)(U/4); offU++;
    *offU=(BYTE)(V/4); offV++;

    *dst = *(org);    *(dst+width1) =*(org+width2); dst++; org+=2;
    *dst = *(org);    *(dst+width1) =*(org+width2); dst++; org+=2;

    i++;
    if ((i%(width1/2))==0)
    {
      dst+=width1;
      org+=2*width1;
    }
  }
  return retV;
}

LPBITMAPINFOHEADER _convertUYVY2YUV12(pTVFrame frame)
{
  LPBITMAPINFOHEADER retV=NULL;

  ASSERT(frame->lpBMIH->biCompression==BI_UYVY);
  LPBYTE org=GetData(frame);
  int    orgSize  = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight= frame->lpBMIH->biHeight;

  ASSERT((orgWidth%4)==0);
  ASSERT((orgHeight%4)==0);

  int retSize=3*frame->lpBMIH->biWidth*frame->lpBMIH->biHeight/2;
  retV=(LPBITMAPINFOHEADER)malloc(frame->lpBMIH->biSize+retSize);
  memcpy(retV,frame->lpBMIH,frame->lpBMIH->biSize);

  retV->biSizeImage=retSize;
  retV->biCompression=BI_YUV12;
  retV->biBitCount=0;
  LPBYTE dst=GetData(retV);

  int eod=(retV->biWidth*retV->biHeight)/4;
  int width1=retV->biWidth;
  int width2=width1<<1;
  int width3=width1+width2;
  int width4=width1<<2;
  int width5=width4+width2;
  LPBYTE offU=dst+(retV->biWidth*retV->biHeight);
  LPBYTE offV=offU+eod;
  for (int i=0; i<eod; )
  {
    int U=0,V=0;
    V=*org+*(org+4);                    U=*(org+2)+*(org+6);
    V+=*(org+width2)+*(org+width2+4);   U+=*(org+width2+2)+*(org+width2+6);

    *offV=(BYTE)(U/4); offU++;
    *offU=(BYTE)(V/4); offV++;

    *dst = *(org+1);    *(dst+width1) =*(org+width2+1);  dst++; org+=2;
    *dst = *(org+1);    *(dst+width1) =*(org+width2+1);  dst++; org+=2;

    i++;
    if ((i%(width1/2))==0)
    {
      dst+=width1;
      org+=2*width1;
    }
  } 
  return retV;
}

LPBITMAPINFOHEADER _convertUYVY2YUV9(pTVFrame frame)
{
  LPBITMAPINFOHEADER retV=NULL;

  ASSERT(frame->lpBMIH->biCompression==BI_UYVY);
  LPBYTE org=GetData(frame);
  int    orgSize  = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight= frame->lpBMIH->biHeight;

  ASSERT((orgWidth%4)==0);
  ASSERT((orgHeight%4)==0);

  int retSize=9*frame->lpBMIH->biWidth*frame->lpBMIH->biHeight/8;
  retV=(LPBITMAPINFOHEADER)malloc(frame->lpBMIH->biSize+retSize);
  memcpy(retV,frame->lpBMIH,frame->lpBMIH->biSize);

  retV->biSizeImage=retSize;
  retV->biCompression=BI_YUV9;
  retV->biBitCount=0;
  LPBYTE dst=GetData(retV);

  int eod=(retV->biWidth*retV->biHeight)/16;
  int width1=retV->biWidth;
  int width2=width1<<1;
  int width3=width1+width2;
  int width4=width1<<2;
  int width5=width4+width2;
  LPBYTE offU=dst+(retV->biWidth*retV->biHeight);
  LPBYTE offV=offU+eod;
  for (int i=0; i<eod; )
  {
    int U=0,V=0;
    V=*org+*(org+4);                    U=*(org+2)+*(org+6);
    V+=*(org+width2)+*(org+width2+4);   U+=*(org+width2+2)+*(org+width2+6);
    V+=*(org+width4)+*(org+width4+4);   U+=*(org+width4+2)+*(org+width4+6);
    V+=*(org+width5)+*(org+width5+4);   U+=*(org+width5+2)+*(org+width5+6);

    *offU=(BYTE)(U/8); offU++;
    *offV=(BYTE)(V/8); offV++;

    *dst = *(org+1);    *(dst+width1) =*(org+width2+1); *(dst+width2) =*(org+width4+1); *(dst+width3) =*(org+width5+1); dst++; org+=2;
    *dst = *(org+1);    *(dst+width1) =*(org+width2+1); *(dst+width2) =*(org+width4+1); *(dst+width3) =*(org+width5+1); dst++; org+=2;
    *dst = *(org+1);    *(dst+width1) =*(org+width2+1); *(dst+width2) =*(org+width4+1); *(dst+width3) =*(org+width5+1); dst++; org+=2;
    *dst = *(org+1);    *(dst+width1) =*(org+width2+1); *(dst+width2) =*(org+width4+1); *(dst+width3) =*(org+width5+1); dst++; org+=2;

    i++;
    if ((i%(width1/4))==0)
    {
      dst+=3*width1;
      org+=6*width1;
    }
  } 
  return retV;
}

LPBITMAPINFOHEADER _convertY411YUV9(pTVFrame frame)
{
  LPBITMAPINFOHEADER retV=NULL;

  ASSERT(frame->lpBMIH->biCompression==BI_Y411);
  LPBYTE org=GetData(frame);
  int    orgSize  = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight= frame->lpBMIH->biHeight;

  ASSERT((orgWidth%4)==0);
  ASSERT((orgHeight%4)==0);

  int retSize=9*frame->lpBMIH->biWidth*frame->lpBMIH->biHeight/8;
  retV=(LPBITMAPINFOHEADER)malloc(frame->lpBMIH->biSize+retSize);
  memcpy(retV,frame->lpBMIH,frame->lpBMIH->biSize);

  retV->biSizeImage=retSize;
  retV->biCompression=BI_YUV9;
  retV->biBitCount=0;
  LPBYTE dst=GetData(retV);

  int eod=(retV->biWidth*retV->biHeight)/16;
  int width=retV->biWidth;
  int widthY=3*width/2;

  LPBYTE offU=dst+(retV->biWidth*retV->biHeight);
  LPBYTE offV=offU+eod;
  ASSERT((width%4)==0);
  for (int i=0; i<eod; )
  {
    int U=0,V=0;
    V=*org;             U=*(org+3);
    V+=*(org+widthY);   U+=*(org+widthY+3);
    V+=*(org+2*widthY);   U+=*(org+2*widthY+3);
    V+=*(org+3*widthY);   U+=*(org+3*widthY+3);

    *offU=(BYTE)(U/4); offU++;
    *offV=(BYTE)(V/4); offV++;

    *dst = *(org+1);    *(dst+width) =*(org+widthY+1); *(dst+2*width) =*(org+2*widthY+1); *(dst+3*width) =*(org+3*widthY+1); dst++; org++;
    *dst = *(org+1);    *(dst+width) =*(org+widthY+1); *(dst+2*width) =*(org+2*widthY+1); *(dst+3*width) =*(org+3*widthY+1); dst++; org+=2;
    *dst = *(org+1);    *(dst+width) =*(org+widthY+1); *(dst+2*width) =*(org+2*widthY+1); *(dst+3*width) =*(org+3*widthY+1); dst++; org++;
    *dst = *(org+1);    *(dst+width) =*(org+widthY+1); *(dst+2*width) =*(org+2*widthY+1); *(dst+3*width) =*(org+3*widthY+1); dst++; org+=2;

    i++;
    if ((i%(width/4))==0)
    {
      dst+=3*width;
      org+=9*(width/2);
    }
  } 
  return retV;
}

LPBITMAPINFOHEADER _convertRGB8Gray2YUV9(pTVFrame frame)
{
  LPBITMAPINFOHEADER retV=NULL;

  LPBYTE org=GetData(frame);
  int    orgSize  = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight= frame->lpBMIH->biHeight;

  int dstWidth = (orgWidth/4)*4;
  int dstHeight= (orgHeight/4)*4;

  int retSize=9*dstWidth*dstHeight/8;
  retV=(LPBITMAPINFOHEADER)malloc(frame->lpBMIH->biSize+retSize);
  memcpy(retV,frame->lpBMIH,frame->lpBMIH->biSize);

  retV->biWidth=dstWidth;
  retV->biHeight=dstHeight;
  retV->biSizeImage=retSize;
  retV->biCompression=BI_YUV9;
  retV->biBitCount=0;
  retV->biClrUsed=0;
  retV->biClrImportant=0;
  LPBYTE dst=GetData(retV);

  int rorgWidth=orgWidth;
  if (frame->lpBMIH->biSizeImage)
    rorgWidth=frame->lpBMIH->biSizeImage/frame->lpBMIH->biHeight;
  LPBYTE src=org+rorgWidth*(orgHeight-1);
  for (int i=0; i<dstHeight; i++)
  {
    memcpy(dst,src,dstWidth);
    dst+=dstWidth;
    src-=rorgWidth;
  }
  memset(GetData(retV)+dstWidth*dstHeight, 
    128,
    dstWidth*dstHeight/8);
  return retV;
}

LPBITMAPINFOHEADER _convertY82YUV9(pTVFrame frame)
{
  LPBITMAPINFOHEADER retV=NULL;

  LPBYTE org=GetData(frame);
  int    orgSize  = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight= frame->lpBMIH->biHeight;

  ASSERT((orgWidth%4)==0);
  ASSERT((orgHeight%4)==0);

  int retSize=9*frame->lpBMIH->biWidth*frame->lpBMIH->biHeight/8;
  retV=(LPBITMAPINFOHEADER)malloc(frame->lpBMIH->biSize+retSize);
  memcpy(retV,frame->lpBMIH,frame->lpBMIH->biSize);

  retV->biSizeImage=retSize;
  retV->biCompression=BI_YUV9;
  retV->biBitCount=0;
  retV->biClrUsed=0;
  retV->biClrImportant=0;
  LPBYTE dst=GetData(retV);

  LPBYTE src=org; 
  memcpy(dst,src,frame->lpBMIH->biWidth*frame->lpBMIH->biHeight);

  memset(GetData(retV)+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight, 
    128,
    frame->lpBMIH->biWidth*frame->lpBMIH->biHeight/8);

  return retV;
}

LPBITMAPINFOHEADER _convertYUV122YUV9(pTVFrame frame , bool bAsI420 )
{
  LPBITMAPINFOHEADER retV=NULL;

  LPBYTE org=GetData(frame);
  int    orgSize  = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight= frame->lpBMIH->biHeight;

  ASSERT((orgWidth%4)==0);
  ASSERT((orgHeight%4)==0);

  int retSize=9*frame->lpBMIH->biWidth*frame->lpBMIH->biHeight/8;
  retV=(LPBITMAPINFOHEADER)malloc(frame->lpBMIH->biSize+retSize);
  memcpy(retV,frame->lpBMIH,frame->lpBMIH->biSize);

  retV->biSizeImage=retSize;
  retV->biCompression=BI_YUV9;
  retV->biBitCount=0;
  retV->biClrUsed=0;
  retV->biClrImportant=0;
  LPBYTE dst=GetData(retV);

  LPBYTE src=org; 
  memcpy(dst,src,retV->biWidth*retV->biHeight);
  int uv12width=retV->biWidth/2;
  int uv9width=retV->biWidth/4;
  LPBYTE dstV=dst+retV->biWidth*retV->biHeight;
  LPBYTE dstU=dstV+uv9width;
  LPBYTE srcU=org+retV->biWidth*retV->biHeight;
  LPBYTE srcV=srcU+uv12width;
  if ( bAsI420 )
    swap( srcU , srcV ) ;
  int eod=retV->biWidth*retV->biHeight/16;
  int i=0;
  while (i<eod)
  {
    int u=(int)*srcU;
    u+=*(srcU+1);
    u+=*(srcU+uv12width);
    u+=*(srcU+uv12width+1);
    u/=4;
    *dstU=u;
    int v=(int)*srcV;
    v+=*(srcV+1);
    v+=*(srcV+uv12width);
    v+=*(srcV+uv12width+1);
    v/=4;
    *dstV=v;
    srcV+=2; srcU+=2;
    dstV++;  dstU++;
    if ((i!=0)&& ((i%uv9width)==0))
    {
      srcU+=uv12width;
      srcV+=uv12width;
    }
    i++;
  }
  return retV;
}

__forceinline void yuvrgb(int y, int u, int v, int R, int G, int B, LPBYTE r, LPBYTE g, LPBYTE b)
{
  R+=y; *r=(R<0)?0:((R>255)?255:R);
  G+=y; *g=(G<0)?0:((G>255)?255:G);
  B+=y; *b=(B<0)?0:((B>255)?255:B);
}

LPBITMAPINFOHEADER yuv9rgb24(LPBITMAPINFOHEADER src, LPBYTE lpData, LPVOID lpDst)
{
  if(src->biCompression!=BI_YUV9) return NULL;

  ASSERT(src->biWidth%4==0);
  ASSERT(src->biHeight%4==0);

  DWORD newSize=3*src->biWidth*src->biHeight;
  DWORD rgbWidth=3*src->biWidth;
  LPBITMAPINFOHEADER res;
  if (lpDst)
    res=(LPBITMAPINFOHEADER)lpDst;
  else
    res=(LPBITMAPINFOHEADER)malloc(src->biSize+newSize);
  memset(res,0,src->biSize+newSize);
  memcpy(res,src,src->biSize);
  res->biCompression = 0;
  res->biSizeImage   = newSize;
  res->biBitCount    = 24;
  res->biClrUsed     = 0;
  res->biPlanes = 1;
  LPBYTE srcb=(lpData)?lpData:((LPBYTE)src)+src->biSize;
  LPBYTE dstb=((LPBYTE)res)+res->biSize+newSize-rgbWidth;
  LPBYTE eod=srcb+src->biWidth*src->biHeight;
  LPBYTE usrc=eod;
  LPBYTE vsrc=eod+(src->biWidth*src->biHeight)/16;
  int    u=(*usrc-128)<<13;
  int    v=(*vsrc-128)<<13;
  int   _r=v/7185;
  int   _g=-(u/20687+v/14100);
  int   _b=u/4037;
  int    cWidth=src->biWidth/4;
  DWORD cntr=0;
  DWORD cntr4=0;
  int clrInfoOff=0;
  while (srcb<eod)
  {
    yuvrgb(*srcb,u,v,_r,_g,_b,dstb,dstb+1,dstb+2);
    srcb++;
    dstb+=3;
    cntr++;

    yuvrgb(*srcb,u,v,_r,_g,_b,dstb,dstb+1,dstb+2);
    srcb++;
    dstb+=3;
    cntr++;

    yuvrgb(*srcb,u,v,_r,_g,_b,dstb,dstb+1,dstb+2);
    srcb++;
    dstb+=3;
    cntr++;

    yuvrgb(*srcb,u,v,_r,_g,_b,dstb,dstb+1,dstb+2);
    srcb++;
    dstb+=3;
    cntr++;

    cntr4++;
    clrInfoOff=(cntr4%cWidth)+(cntr4/src->biWidth)*cWidth;
    u=(*(usrc+clrInfoOff)-128)<<13;
    v=(*(vsrc+clrInfoOff)-128)<<13;
    _r=v/7185;
    _g=-(u/20687+v/14100);
    _b=u/4037;

    if ((cntr%src->biWidth)==0) 
    {
      dstb-=2*rgbWidth;
    }
  }
  return res;
}


LPBITMAPINFOHEADER yuv12rgb24(LPBITMAPINFOHEADER src, LPBYTE lpData, LPVOID lpDst)
{
  if(src->biCompression!=BI_YUV12) return NULL;

  ASSERT(src->biWidth%4==0);
  ASSERT(src->biHeight%4==0);

  DWORD newSize=3*src->biWidth*src->biHeight;
  DWORD rgbWidth=3*src->biWidth;
  LPBITMAPINFOHEADER res=NULL;
  if (lpDst)
    res=(LPBITMAPINFOHEADER)lpDst;
  else
    res=(LPBITMAPINFOHEADER)malloc(src->biSize+newSize);
  memset(res,0,src->biSize+newSize);
  memcpy(res,src,src->biSize);
  res->biCompression = BI_RGB ;
  res->biSizeImage   = newSize;
  res->biBitCount    = 24;
  res->biClrUsed     = 0;
  LPBYTE srcb=(lpData)?lpData:((LPBYTE)src)+src->biSize;
  LPBYTE dstb=((LPBYTE)res)+res->biSize+newSize-rgbWidth;
  LPBYTE eod=srcb+src->biWidth*src->biHeight;
  LPBYTE usrc=eod;
  LPBYTE vsrc=eod+(src->biWidth*src->biHeight)/4;
  int    v=(*usrc-128)<<13;
  int    u=(*vsrc-128)<<13;
  int   _r=v/7185;
  int   _g=-(u/20687+v/14100);
  int   _b=u/4037;
  int    cWidth=src->biWidth/2;
  DWORD cntr=0;
  DWORD cntr4=0;
  int clrInfoOff=0;
  while (srcb<eod)
  {
    //yuvrgb(*srcb,u,v,dstb,dstb+1,dstb+2);
    yuvrgb(*srcb,u,v,_r,_g,_b,dstb,dstb+1,dstb+2);
    srcb++;
    dstb+=3;
    cntr++;

    //yuvrgb(*srcb,u,v,dstb,dstb+1,dstb+2);
    yuvrgb(*srcb,u,v,_r,_g,_b,dstb,dstb+1,dstb+2);
    srcb++;
    dstb+=3;
    cntr++;

    cntr4++;
    clrInfoOff=(cntr4%cWidth)+(cntr4/src->biWidth)*cWidth;
    v=(*(usrc+clrInfoOff)-128)<<13;
    u=(*(vsrc+clrInfoOff)-128)<<13;
    _r=v/7185;
    _g=-(u/20687+v/14100);
    _b=u/4037;

    if ((cntr%src->biWidth)==0) 
    {
      dstb-=2*rgbWidth;
    }
  }
  return res;
}

_forceinline int rgb_to_y8( BYTE * pr )
{

  int r = (*(pr++) << 13); 
  int g = (*(pr++) << 13); 
  int b = (*(pr++) << 13);

  int iY = r / 27397 + g / 13955 + b / 71859;
  return iY ;
 }


__forceinline void rgbyuv(int r, int g, int b, LPBYTE y, LPBYTE u, LPBYTE v)
{

  r=(r<<13); g=(g<<13); b=(b<<13);

  int Y= r/27397+ g/13955+ b/71859;
  int U=-r/55728- g/28346+ b/18788 + 128;
  int V= r/13320- g/15907- b/81920  + 128;

  *y=Y; *u=U; *v=V;
}

__forceinline void rgbyuv_ex( int r , int g , int b , LPBYTE y , LPDWORD u , LPDWORD v )
{

  r = (r << 13); g = (g << 13); b = (b << 13);

  int Y = r / 27397 + g / 13955 + b / 71859;
  int U = -r / 55728 - g / 28346 + b / 18788 + 128;
  int V = r / 13320 - g / 15907 - b / 81920 + 128;

  *y = Y; *u += U; *v += V;
}

LPBITMAPINFOHEADER rgb24_to_y8( LPBITMAPINFOHEADER src , LPBYTE lpData )
{
  if ( (src->biCompression != 0) || (src->biBitCount != 24) ) return NULL;

  DWORD newSize = src->biWidth*src->biHeight ;

  LPBITMAPINFOHEADER res = (LPBITMAPINFOHEADER) malloc( src->biSize + newSize );
  memcpy( res , src , src->biSize );
  res->biCompression = BI_Y8;
  res->biSizeImage = newSize;
  res->biBitCount = 8;

  LPBYTE srcb = (lpData) ? lpData : ((LPBYTE) src) + src->biSize;
  LPBYTE dst_begin = ((LPBYTE) res) + res->biSize ;
  LPBYTE dstb = dst_begin + newSize - res->biWidth;

  int cntr = 0;
  while ( dst_begin <= dstb )
  {
    *dstb = (BYTE) rgb_to_y8( srcb ) ;   
    dstb++;
    srcb += 3;
    if ( (++cntr % res->biWidth) == 0 )
      dstb -= 2 * res->biWidth ;
  }
  return res;
}


LPBITMAPINFOHEADER rgb24yuv9(LPBITMAPINFOHEADER src, LPBYTE lpData)
{
  if ((src->biCompression!=0) || (src->biBitCount!=24)) return NULL;
  if (src->biWidth%4) return NULL;
  if (src->biHeight%4) return NULL;

  DWORD ySize=src->biWidth*src->biHeight;
  DWORD newSize=9*(ySize/8);
  DWORD uSize=ySize/16;
  DWORD uLineLen=4*uSize/src->biHeight;

  LPBITMAPINFOHEADER res=(LPBITMAPINFOHEADER)malloc(src->biSize+newSize);
  memset(res,128,src->biSize+newSize);
  memcpy(res,src,src->biSize);
  res->biCompression = BI_YUV9;
  res->biSizeImage   = newSize;
  res->biBitCount    = 0;

  LPBYTE srcb= (lpData)?lpData:((LPBYTE)src)+src->biSize;
  LPBYTE srco=srcb;
  LPBYTE dstb= ((LPBYTE)res)+res->biSize+ySize-src->biWidth;
  LPBYTE eod = ((LPBYTE)res)+res->biSize;
  LPBYTE udst=eod+ySize+uSize-uLineLen;
  LPBYTE vdst=udst+uSize;

  int cntr=0;
  DWORD  offYUV1=src->biWidth,offRGB1=src->biWidth*3;
  DWORD  offYUV2=2*offYUV1,   offRGB2=2*offRGB1;
  DWORD  offYUV3=3*offYUV1,   offRGB3=3*offRGB1;
  DWORD  U=0, V=0;
  while (eod<=dstb)
  {
    BYTE u,v;

    rgbyuv(*srcb, *(srcb+1), *(srcb+2), dstb,&u,&v);                                    U+=u; V+=v;
    rgbyuv(*(srcb+offRGB1),  *(srcb+offRGB1+1), *(srcb+offRGB1+2), dstb-offYUV1,&u,&v);  U+=u; V+=v;
    rgbyuv(*(srcb+offRGB2),  *(srcb+offRGB2+1), *(srcb+offRGB2+2), dstb-offYUV2,&u,&v);  U+=u; V+=v;
    rgbyuv(*(srcb+offRGB3),  *(srcb+offRGB3+1), *(srcb+offRGB2+2), dstb-offYUV3,&u,&v);  U+=u; V+=v;
    dstb++;
    srcb+=3;
    cntr++;

    rgbyuv(*srcb, *(srcb+1), *(srcb+2), dstb,&u,&v);                                    U+=u; V+=v;
    rgbyuv(*(srcb+offRGB1),  *(srcb+offRGB1+1), *(srcb+offRGB1+2), dstb-offYUV1,&u,&v);  U+=u; V+=v;
    rgbyuv(*(srcb+offRGB2),  *(srcb+offRGB2+1), *(srcb+offRGB2+2), dstb-offYUV2,&u,&v);  U+=u; V+=v;
    rgbyuv(*(srcb+offRGB3),  *(srcb+offRGB3+1), *(srcb+offRGB2+2), dstb-offYUV3,&u,&v);  U+=u; V+=v;
    dstb++;
    srcb+=3;
    cntr++;

    rgbyuv(*srcb, *(srcb+1), *(srcb+2), dstb,&u,&v);                                    U+=u; V+=v;
    rgbyuv(*(srcb+offRGB1),  *(srcb+offRGB1+1), *(srcb+offRGB1+2), dstb-offYUV1,&u,&v);  U+=u; V+=v;
    rgbyuv(*(srcb+offRGB2),  *(srcb+offRGB2+1), *(srcb+offRGB2+2), dstb-offYUV2,&u,&v);  U+=u; V+=v;
    rgbyuv(*(srcb+offRGB3),  *(srcb+offRGB3+1), *(srcb+offRGB2+2), dstb-offYUV3,&u,&v);  U+=u; V+=v;
    dstb++;
    srcb+=3;
    cntr++;

    rgbyuv(*srcb, *(srcb+1), *(srcb+2), dstb,&u,&v);                                    U+=u; V+=v;
    rgbyuv(*(srcb+offRGB1),  *(srcb+offRGB1+1), *(srcb+offRGB1+2), dstb-offYUV1,&u,&v);  U+=u; V+=v;
    rgbyuv(*(srcb+offRGB2),  *(srcb+offRGB2+1), *(srcb+offRGB2+2), dstb-offYUV2,&u,&v);  U+=u; V+=v;
    rgbyuv(*(srcb+offRGB3),  *(srcb+offRGB3+1), *(srcb+offRGB2+2), dstb-offYUV3,&u,&v);  U+=u; V+=v;
    dstb++;
    srcb+=3;
    cntr++;

    *(udst++)=(BYTE)(U>>4); 
    *(vdst++)=(BYTE)(V>>4);
    U=0; V=0;        
    if ((cntr%src->biWidth)==0) 
    {
      udst-=2*uLineLen;
      vdst-=2*uLineLen;
      dstb-=5*src->biWidth;
      srcb+=9*src->biWidth;
    }
  }
  return res;
}

LPBITMAPINFOHEADER rgb24yuv12( LPBITMAPINFOHEADER src , LPBYTE lpData )
{
  if ( (src->biCompression != BI_RGB ) || (src->biBitCount != 24) ) 
    return NULL;
  if ( src->biWidth % 2 ) 
    return NULL;
  if ( src->biHeight % 2 ) 
    return NULL;

  DWORD ySize =  src->biWidth * src->biHeight ;
  DWORD newSize = 3 * (ySize / 2);
  DWORD uSize = ySize / 4;
  DWORD uLineLen = src->biWidth / 2 ;

  LPBITMAPINFOHEADER res = (LPBITMAPINFOHEADER) malloc( src->biSize + newSize );
  memset( res , 128 , src->biSize + newSize );
  memcpy( res , src , src->biSize );
  res->biCompression = BI_YUV12;
  res->biSizeImage = newSize;
  res->biBitCount = 12 ;

  LPBYTE srcb = (lpData) ? lpData : ((LPBYTE) src) + src->biSize;
  LPBYTE srco = srcb;
  LPBYTE eod = ((LPBYTE) res) + res->biSize;
  LPBYTE dstb = eod + ySize - src->biWidth;
  LPBYTE udst = dstb + uSize + src->biWidth - uLineLen;
  LPBYTE vdst = udst + uSize;

  int cntr = 0;
  DWORD  offYUV1 = src->biWidth , offRGB1 = src->biWidth * 3;
  while ( eod <= dstb )
  {
    DWORD  U = 0 , V = 0;

    rgbyuv_ex( *srcb , *(srcb + 1) , *(srcb + 2) , dstb , &V , &U );                              
    rgbyuv_ex( *(srcb + offRGB1) , *(srcb + offRGB1 + 1) , *(srcb + offRGB1 + 2) , dstb - offYUV1 , &V , &U );
    dstb++;
    srcb += 3;
    cntr++;

    rgbyuv_ex( *srcb , *(srcb + 1) , *(srcb + 2) , dstb , &V , &U );
    rgbyuv_ex( *(srcb + offRGB1) , *(srcb + offRGB1 + 1) , *(srcb + offRGB1 + 2) , dstb - offYUV1 , &V , &U );
    dstb++;
    srcb += 3;
    cntr++;

    *( udst++ ) = ( BYTE ) ( U >> 2 );
    *( vdst++ ) = ( BYTE ) ( V >> 2 );
    if ( (cntr%src->biWidth) == 0 )
    {
      udst -= 2 * uLineLen;
      vdst -= 2 * uLineLen;
      dstb -= 3 * src->biWidth;
      srcb += 3 * src->biWidth;
    }
  }
  return res;
}

#define NEW_CVRT_PROC
LPBITMAPINFOHEADER yuv411yuv9(LPBITMAPINFOHEADER src, LPBYTE lpData)
{
  //    DWORD ts= GetHRTickCount();
  if (src->biCompression!=BI_YUV411) 
    return NULL;
  int w=src->biWidth;
  int h=src->biHeight;
  LPBITMAPINFOHEADER res=(LPBITMAPINFOHEADER)malloc(src->biSize+(9*w*h)/8);
  if (!res) 
    return NULL;
  LPBYTE in=(lpData)?lpData:((LPBYTE)src)+src->biSize;
  LPBYTE out=((LPBYTE)res)+src->biSize; 
  try
  {
#ifndef NEW_CVRT_PROC
    DWORD Size;
    DWORD newXSize;
    DWORD newSize;
    DWORD UOffset;
    DWORD VOffset;
    DWORD ResOffset, BufOffset;
    DWORD i, j, k;
    DWORD U, V;
    DWORD XSize=3*w/2;

    Size=src->biSizeImage;
    newXSize=XSize*2/3;
    newSize=Size*3/4;

    UOffset=newSize*16/18;
    VOffset=newSize*17/18;

    for(j=0; j*4<h; j++)
    {
      for(i=1; i<XSize; i+=6)
      {
        U=0; V=0;
        for(k=0; k<4; k++)
        {
          ResOffset=(4*j+k)*newXSize+4*i/6;
          BufOffset=(4*j+k)*XSize   +i;
          out[ResOffset  ]=in[BufOffset  ];
          out[ResOffset+1]=in[BufOffset+1];
          out[ResOffset+2]=in[BufOffset+3];
          out[ResOffset+3]=in[BufOffset+4];
          U+=in[BufOffset-1];
          V+=in[BufOffset+2];
        }
        out[UOffset+(i/6)+(XSize/6)*j]=(UCHAR)(V/4);
        out[VOffset+(i/6)+(XSize/6)*j]=(UCHAR)(U/4);
      }
    }
#else
    int Xsize411=3*w/2;
    int nmb=w*h/16;


    LPBYTE src=in;
    LPBYTE dst=out;
    LPBYTE dstU=out+w*h;
    LPBYTE dstV=dstU+nmb;

    unsigned U=0,V=0;

    for (int i=0; i<nmb;i++)
    {
      V+=*src; V+=*(src+Xsize411); V+=*(src+2*Xsize411); V+=*(src+3*Xsize411); src++;  

      *(WORD *)(dst)=*(WORD *)(src); *(WORD *)(dst+w)=*(WORD *)(src+Xsize411);
      *(WORD *)(dst+2*w)=*(WORD *)(src+2*Xsize411);
      *(WORD *)(dst+3*w)=*(WORD *)(src+3*Xsize411);
      dst+=2; src+=2;  

      U+=*src; U+=*(src+Xsize411); U+=*(src+2*Xsize411); U+=*(src+3*Xsize411); src++; 
      *(WORD *)(dst)=*(WORD *)(src); *(WORD *)(dst+w)=*(WORD *)(src+Xsize411);
      *(WORD *)(dst+2*w)=*(WORD *)(src+2*Xsize411);
      *(WORD *)(dst+3*w)=*(WORD *)(src+3*Xsize411);
      dst+=2; src+=2;  

      *dstU=(unsigned char)(U/4); *dstV=(unsigned char)(V/4);

      U=0; V=0; dstU++; dstV++;

      if ((i!=0) && ((i%(w/4))==0))
      {
        dst+=3*w;
        src+=3*Xsize411;
      }
    }
#endif
  }
  catch(...)
  {
    TRACE("!!! Error in conversion utility yuv411yuv9!\n");
    free(res);
    return NULL;
  }
  memcpy(res,src,src->biSize);
  res->biCompression=BI_YUV9;
  res->biBitCount=9;
  res->biSizeImage=(9*w*h)/8;
  //    TRACE("+++ %d\n",  GetHRTickCount()-ts);
  return res;
}

static RGBQUAD graypalette[256]=
{
  {0,0,0,0},    {1,1,1,0},    {2,2,2,0},    {3,3,3,0},    {4,4,4,0},    {5,5,5,0},
  {6,6,6,0},    {7,7,7,0},    {8,8,8,0},    {9,9,9,0},
  {10,10,10,0},    {11,11,11,0},    {12,12,12,0},    {13,13,13,0},    {14,14,14,0},    {15,15,15,0},
  {16,16,16,0},    {17,17,17,0},    {18,18,18,0},    {19,19,19,0},
  {20,20,20,0},    {21,21,21,0},    {22,22,22,0},    {23,23,23,0},    {24,24,24,0},    {25,25,25,0},
  {26,26,26,0},    {27,27,27,0},    {28,28,28,0},    {29,29,29,0},
  {30,30,30,0},    {31,31,31,0},    {32,32,32,0},    {33,33,33,0},    {34,34,34,0},    {35,35,35,0},
  {36,36,36,0},    {37,37,37,0},    {38,38,38,0},    {39,39,39,0},
  {40,40,40,0},    {41,41,41,0},    {42,42,42,0},    {43,43,43,0},    {44,44,44,0},    {45,45,45,0},
  {46,46,46,0},    {47,47,47,0},    {48,48,48,0},    {49,49,49,0},
  {50,50,50,0},    {51,51,51,0},    {52,52,52,0},    {53,53,53,0},    {54,54,54,0},    {55,55,55,0},
  {56,56,56,0},    {57,57,57,0},    {58,58,58,0},    {59,59,59,0},
  {60,60,60,0},    {61,61,61,0},    {62,62,62,0},    {63,63,63,0},    {64,64,64,0},    {65,65,65,0},
  {66,66,66,0},    {67,67,67,0},    {68,68,68,0},    {69,69,69,0},
  {70,70,70,0},    {71,71,71,0},    {72,72,72,0},    {73,73,73,0},    {74,74,74,0},    {75,75,75,0},
  {76,76,76,0},    {77,77,77,0},    {78,78,78,0},    {79,79,79,0},
  {80,80,80,0},    {81,81,81,0},    {82,82,82,0},    {83,83,83,0},    {84,84,84,0},    {85,85,85,0},
  {86,86,86,0},    {87,87,87,0},    {88,88,88,0},    {89,89,89,0},
  {90,90,90,0},    {91,91,91,0},    {92,92,92,0},    {93,93,93,0},    {94,94,94,0},    {95,95,95,0},
  {96,96,96,0},    {97,97,97,0},    {98,98,98,0},    {99,99,99,0},
  {100,100,100,0},    {101,101,101,0},    {102,102,102,0},    {103,103,103,0},    {104,104,104,0},    {105,105,105,0},
  {106,106,106,0},    {107,107,107,0},    {108,108,108,0},    {109,109,109,0},
  {110,110,110,0},    {111,111,111,0},    {112,112,112,0},    {113,113,113,0},    {114,114,114,0},    {115,115,115,0},
  {116,116,116,0},    {117,117,117,0},    {118,118,118,0},    {119,119,119,0},
  {120,120,120,0},    {121,121,121,0},    {122,122,122,0},    {123,123,123,0},    {124,124,124,0},    {125,125,125,0},
  {126,126,126,0},    {127,127,127,0},    {128,128,128,0},    {129,129,129,0},
  {130,130,130,0},    {131,131,131,0},    {132,132,132,0},    {133,133,133,0},    {134,134,134,0},    {135,135,135,0},
  {136,136,136,0},    {137,137,137,0},    {138,138,138,0},    {139,139,139,0},
  {140,140,140,0},    {141,141,141,0},    {142,142,142,0},    {143,143,143,0},    {144,144,144,0},    {145,145,145,0},
  {146,146,146,0},    {147,147,147,0},    {148,148,148,0},    {149,149,149,0},
  {150,150,150,0},    {151,151,151,0},    {152,152,152,0},    {153,153,153,0},    {154,154,154,0},    {155,155,155,0},
  {156,156,156,0},    {157,157,157,0},    {158,158,158,0},    {159,159,159,0},
  {160,160,160,0},    {161,161,161,0},    {162,162,162,0},    {163,163,163,0},    {164,164,164,0},    {165,165,165,0},
  {166,166,166,0},    {167,167,167,0},    {168,168,168,0},    {169,169,169,0},
  {170,170,170,0},    {171,171,171,0},    {172,172,172,0},    {173,173,173,0},    {174,174,174,0},    {175,175,175,0},
  {176,176,176,0},    {177,177,177,0},    {178,178,178,0},    {179,179,179,0},
  {180,180,180,0},    {181,181,181,0},    {182,182,182,0},    {183,183,183,0},    {184,184,184,0},    {185,185,185,0},
  {186,186,186,0},    {187,187,187,0},    {188,188,188,0},    {189,189,189,0},
  {190,190,190,0},    {191,191,191,0},    {192,192,192,0},    {193,193,193,0},    {194,194,194,0},    {195,195,195,0},
  {196,196,196,0},    {197,197,197,0},    {198,198,198,0},    {199,199,199,0},
  {200,200,200,0},    {201,201,201,0},    {202,202,202,0},    {203,203,203,0},    {204,204,204,0},    {205,205,205,0},
  {206,206,206,0},    {207,207,207,0},    {208,208,208,0},    {209,209,209,0},
  {210,210,210,0},    {211,211,211,0},    {212,212,212,0},    {213,213,213,0},    {214,214,214,0},    {215,215,215,0},
  {216,216,216,0},    {217,217,217,0},    {218,218,218,0},    {219,219,219,0},
  {220,220,220,0},    {221,221,221,0},    {222,222,222,0},    {223,223,223,0},    {224,224,224,0},    {225,225,225,0},
  {226,226,226,0},    {227,227,227,0},    {228,228,228,0},    {229,229,229,0},
  {230,230,230,0},    {231,231,231,0},    {232,232,232,0},    {233,233,233,0},    {234,234,234,0},    {235,235,235,0},
  {236,236,236,0},    {237,237,237,0},    {238,238,238,0},    {239,239,239,0},
  {240,240,240,0},    {241,241,241,0},    {242,242,242,0},    {243,243,243,0},    {244,244,244,0},    {245,245,245,0},
  {246,246,246,0},    {247,247,247,0},    {248,248,248,0},    {249,249,249,0},
  {250,250,250,0},    {251,251,251,0},    {252,252,252,0},    {253,253,253,0},    {254,254,254,0},    {255,255,255,0}
};

LPBITMAPINFOHEADER yuv9rgb8(LPBITMAPINFOHEADER src, LPBYTE lpData, LPVOID lpDst)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;

  DWORD dwSize = src->biWidth*src->biHeight ;
  if (!lpData)
    lpData=GetData(src);
  if (lpDst)
    lpBMIH=(LPBITMAPINFOHEADER)lpDst;
  else
    lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize + dwSize + sizeof(graypalette));
  memset(lpBMIH,0,src->biSize + dwSize + sizeof(graypalette));
  memcpy(lpBMIH,src,src->biSize);
  lpBMIH->biBitCount=8;
  lpBMIH->biSizeImage = dwSize;
  lpBMIH->biCompression=0;
  lpBMIH->biClrUsed=BI_RGB;
  lpBMIH->biPlanes = 1 ;
//   lpBMIH->biClrImportant = 0 ;
//   lpBMIH->biClrUsed = 0 ;
  lpBMIH->biClrImportant = 256 ;
  lpBMIH->biClrUsed = 256 ;

  memcpy((LPBYTE)lpBMIH+src->biSize,graypalette,sizeof(graypalette));

  LPBYTE dst=((LPBYTE)lpBMIH)+src->biSize+sizeof(graypalette);
  LPBYTE dsc=dst;
  LPBYTE eod=dsc +  dwSize;
  LPBYTE ssc=lpData + ( dwSize - src->biWidth );

  while(dsc<eod)
  {
    memcpy(dsc,ssc,src->biWidth);
    dsc+=src->biWidth;
    ssc-=src->biWidth;
  }
  return lpBMIH;
}

LPBITMAPINFOHEADER yuv12rgb8(LPBITMAPINFOHEADER src, LPBYTE lpData, LPVOID lpDst)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;

  if (!lpData)
    lpData=GetData(src);
  if (lpDst)
    lpBMIH=(LPBITMAPINFOHEADER)lpDst;
  else
    lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize+src->biWidth*src->biHeight+sizeof(graypalette));
  memset(lpBMIH,0,src->biSize+src->biWidth*src->biHeight+sizeof(graypalette));
  memcpy(lpBMIH,src,src->biSize);
  lpBMIH->biBitCount=8;
  lpBMIH->biSizeImage=0;
  lpBMIH->biCompression=0;
  lpBMIH->biClrUsed=BI_RGB;
  lpBMIH->biPlanes = 1 ;
  lpBMIH->biClrImportant = 256 ;
  lpBMIH->biClrUsed = 256 ;

  memcpy((LPBYTE)lpBMIH+src->biSize,graypalette,sizeof(graypalette));

  LPBYTE dst=((LPBYTE)lpBMIH)+src->biSize+sizeof(graypalette);
  LPBYTE dsc=dst;
  LPBYTE eod=dsc + src->biWidth*src->biHeight;
  LPBYTE ssc=lpData + (src->biWidth*(src->biHeight-1));

  while(dsc<eod)
  {
    memcpy(dsc,ssc,src->biWidth);
    dsc+=src->biWidth;
    ssc-=src->biWidth;
  }
  return lpBMIH;
}

LPBITMAPINFOHEADER y8rgb8(LPBITMAPINFOHEADER src, LPBYTE lpData)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;


  if (!lpData)
    lpData=GetData(src);
  lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize+src->biWidth*src->biHeight+sizeof(graypalette));
  memset(lpBMIH,0,src->biSize+src->biWidth*src->biHeight+sizeof(graypalette));
  memcpy(lpBMIH,src,src->biSize);
  lpBMIH->biPlanes = 1 ;
  lpBMIH->biBitCount=8;
  lpBMIH->biCompression= BI_RGB;
  lpBMIH->biSizeImage=0;
  lpBMIH->biClrImportant = 256 ;
  lpBMIH->biClrUsed = 256 ;

  memcpy((LPBYTE)lpBMIH+src->biSize,graypalette,sizeof(graypalette));

  LPBYTE dst=((LPBYTE)lpBMIH)+src->biSize+sizeof(graypalette);
  LPBYTE dsc=dst;
  LPBYTE eod=dsc + src->biWidth*src->biHeight;
  LPBYTE ssc=lpData + (src->biWidth*(src->biHeight-1));

  while(dsc<eod)
  {
    memcpy(dsc,ssc,src->biWidth);
    dsc+=src->biWidth;
    ssc-=src->biWidth;
  }
  return lpBMIH;
}

LPBITMAPINFOHEADER y16rgb8(LPBITMAPINFOHEADER src, LPBYTE lpD, LPVOID lpDst)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;
  if (!lpD)
    lpD=GetData(src);

  unsigned short* lpData=(unsigned short*)lpD; 
  if (lpDst)
    lpBMIH=(LPBITMAPINFOHEADER)lpDst;
  else
    lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize+src->biWidth*src->biHeight+sizeof(graypalette));
  memset(lpBMIH,0,src->biSize+src->biWidth*src->biHeight+sizeof(graypalette));
  memcpy(lpBMIH,src,src->biSize);
  lpBMIH->biBitCount=8;
  lpBMIH->biSizeImage=0;
  lpBMIH->biCompression=0;
  lpBMIH->biClrUsed=BI_RGB;
  lpBMIH->biPlanes = 1 ;
  lpBMIH->biClrImportant = 256 ;
  lpBMIH->biClrUsed = 256 ;

  memcpy((LPBYTE)lpBMIH+src->biSize,graypalette,sizeof(graypalette));

  LPBYTE dst=((LPBYTE)lpBMIH)+src->biSize+sizeof(graypalette);
  LPBYTE dsc=dst;
  LPBYTE eod=dsc + src->biWidth*src->biHeight;
  unsigned short* ssc=lpData + (src->biWidth*(src->biHeight-1));

  while(dsc<eod)
  {
    w2bcpy(dsc,ssc,src->biWidth);
    dsc+=src->biWidth;
    ssc-=src->biWidth;
  }
  return lpBMIH;
}

LPBITMAPINFOHEADER y16yuv8(LPBITMAPINFOHEADER src, LPBYTE lpD , int iShift)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;
  if (!lpD)
    lpD=GetData(src);

  unsigned short* lpData=(unsigned short*)lpD; 
  int isize=9*((src->biWidth*src->biHeight)/8);
  lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize+isize);
  memcpy(lpBMIH,src,src->biSize);

  lpBMIH->biBitCount=9;
  lpBMIH->biSizeImage=isize;
  lpBMIH->biCompression=BI_YUV9;
  lpBMIH->biClrUsed=0;
  lpBMIH->biPlanes = 1 ;
  lpBMIH->biClrImportant = 0;
  lpBMIH->biClrUsed = 0;

  LPBYTE dst=((LPBYTE)lpBMIH)+lpBMIH->biSize;
  LPBYTE dsc=dst;
  LPBYTE eod=dsc + src->biWidth*src->biHeight;

  unsigned short* ssc=lpData;
  memset(eod,128,(src->biWidth*src->biHeight)/8);
  if ( iShift > 0 )
  {
    while ( dsc < eod )
      *( dsc++ ) = ( BYTE )( ( *( ssc++ ) ) >> iShift );
  }
  else if ( iShift < 0 )
  {
    iShift = -iShift ;
    while ( dsc < eod )
      *( dsc++ ) = ( BYTE )( ( *( ssc++ ) ) << iShift );
  }
  else
  {
    while ( dsc < eod )
      *( dsc++ ) = ( BYTE ) ( *( ssc++ ) ) ;
  }
  return lpBMIH;
}
LPBITMAPINFOHEADER y8y16( LPBITMAPINFOHEADER src , LPBYTE lpD , int iShift )
{
  if ( !src )
    return NULL ;
  if ( !lpD )
    lpD = GetData( src );

  int isize = 2 * ( src->biWidth*src->biHeight ) ;
  if ( !lpD )
    return NULL ;
  LPBITMAPINFOHEADER lpBMIH = ( LPBITMAPINFOHEADER )malloc( src->biSize + isize );
  memcpy( lpBMIH , src , src->biSize );

  lpBMIH->biBitCount = 16;
  lpBMIH->biSizeImage = isize;
  lpBMIH->biCompression = BI_Y16 ;
  lpBMIH->biClrUsed = 0;
  lpBMIH->biPlanes = 1 ;
  lpBMIH->biClrImportant = 0;
  lpBMIH->biClrUsed = 0;

  LPWORD dst = ( LPWORD )( ( LPBYTE )lpBMIH + lpBMIH->biSize );
  LPWORD eod = dst + src->biWidth*src->biHeight;

  LPBYTE ssc = lpD ;
  if ( iShift > 0 )
  {
    while ( dst < eod )
      *( dst++ ) = (( WORD )( *( ssc++ ) )) << iShift ;
  }
  else if ( iShift < 0 )
  {
    iShift = -iShift ;
    while ( dst < eod )
      *( dst++ ) = ( ( WORD )( *( ssc++ ) ) ) << iShift ;
  }
  else
  {
    while ( dst < eod )
      *( dst++ ) = ( WORD )( *( ssc++ ) ) ;
  }

  return lpBMIH;
}

LPBITMAPINFOHEADER yuv12yuv9(LPBITMAPINFOHEADER BMIH, LPBYTE lpData)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;
  LPBYTE org;
  if (lpData)
    org=lpData;
  else
    org=GetData(BMIH);
  int    orgSize  = BMIH->biSizeImage;
  int    orgWidth = BMIH->biWidth;
  int    orgHeight= BMIH->biHeight;
  int    newSize=9*BMIH->biWidth*BMIH->biHeight/8;
  lpBMIH=(LPBITMAPINFOHEADER)malloc(BMIH->biSize+newSize);
  memcpy(lpBMIH,BMIH,BMIH->biSize);
  lpBMIH->biSizeImage=newSize;
  lpBMIH->biCompression=BI_YUV9;
  lpBMIH->biBitCount=0;
  lpBMIH->biClrUsed=0;
  lpBMIH->biClrImportant=0;

  LPBYTE dst=GetData(lpBMIH);
  LPBYTE src=org; 
  memcpy(dst,src,lpBMIH->biWidth*lpBMIH->biHeight);
  int uv12width=lpBMIH->biWidth/2;
  int uv9width=lpBMIH->biWidth/4;
  LPBYTE dstV=dst+lpBMIH->biWidth*lpBMIH->biHeight;
  LPBYTE dstU=dstV+lpBMIH->biWidth*lpBMIH->biHeight/16;
  LPBYTE srcU=org+lpBMIH->biWidth*lpBMIH->biHeight;
  LPBYTE srcV=srcU+lpBMIH->biWidth*lpBMIH->biHeight/4;
  int eod=lpBMIH->biWidth*lpBMIH->biHeight/16;
  int i=0;
  while (i<eod)
  {
    int u=(int)*srcU;
    u+=*(srcU+1);
    u+=*(srcU+uv12width);
    u+=*(srcU+uv12width+1);
    u/=4;
    int v=(int)*srcV;
    v+=*(srcV+1);
    v+=*(srcV+uv12width);
    v+=*(srcV+uv12width+1);
    v/=4;
    *dstV=v;
    *dstU=u;
    srcV+=2; srcU+=2;
    dstV++;  dstU++;
    if ((i!=0)&& ((i%uv9width)==0))
    {
      srcU+=uv12width;
      srcV+=uv12width;
    }
    i++;
  }
  return lpBMIH;
}

LPBITMAPINFOHEADER y8yuv9(LPBITMAPINFOHEADER src, LPBYTE lpD)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;
  if (!lpD)
    lpD=GetData(src);

  LPBYTE lpData=lpD; 
  int isize=9*((src->biWidth*src->biHeight)/8);
  lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize+isize);
  memcpy(lpBMIH,src,src->biSize);

  lpBMIH->biBitCount=9;
  lpBMIH->biSizeImage=isize;
  lpBMIH->biCompression=BI_YUV9;
  lpBMIH->biClrUsed=0;
  lpBMIH->biPlanes = 1 ;
  lpBMIH->biClrImportant = 0;
  lpBMIH->biClrUsed = 0;

  LPBYTE dst=((LPBYTE)lpBMIH)+lpBMIH->biSize;
  LPBYTE dsc=dst;
  LPBYTE eod=dsc + src->biWidth*src->biHeight;

  LPBYTE ssc=lpData;
  memset(eod,128,(src->biWidth*src->biHeight)/8);
  memcpy(dst,ssc,src->biWidth*src->biHeight);
  return lpBMIH;
}

LPBITMAPINFOHEADER y8rgb24(LPBITMAPINFOHEADER src, LPBYTE lpD)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;
  if (!lpD)
    lpD=GetData(src);

  LPBYTE lpData=lpD; 
  int isize=3*(src->biWidth*src->biHeight);
  lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize+isize);
  memcpy(lpBMIH,src,src->biSize);

  lpBMIH->biBitCount=24;
  lpBMIH->biSizeImage=0;
  lpBMIH->biCompression=0;
  lpBMIH->biClrUsed=0;
  lpBMIH->biPlanes = 1 ;
  lpBMIH->biClrImportant = 0;
  lpBMIH->biClrUsed = 0;
  LPBYTE dst=((LPBYTE)lpBMIH)+lpBMIH->biSize+3*src->biWidth*(lpBMIH->biHeight-1);

  for (int y=0; y<lpBMIH->biHeight; y++)
  {
    LPBYTE ssc=lpData+src->biWidth*y;
    LPBYTE eod=ssc + src->biWidth;
    LPBYTE dsc=dst-3*y*src->biWidth;
    while(ssc<eod)
    {
      *dsc=*ssc;
      dsc++;
      *dsc=*ssc;
      dsc++;
      *dsc=*ssc;
      dsc++;
      ssc++;
    } 
  }
  return lpBMIH;
}

LPBITMAPINFOHEADER rgb24rdb32(LPBITMAPINFOHEADER pBMH, LPBYTE lpData)
{
  int newsize=4*pBMH->biWidth*pBMH->biHeight;
  LPBITMAPINFOHEADER res=(LPBITMAPINFOHEADER)malloc(pBMH->biSize+newsize);
  memcpy(res,pBMH,pBMH->biSize);
  LPBYTE dst=((LPBYTE)res)+res->biSize;
  LPBYTE src=(lpData)?lpData:GetData(pBMH);

  for (int i=0; i<newsize; )
  {
    *dst=*src; dst++; src++; i++;
    *dst=*src; dst++; src++; i++;
    *dst=*src; dst++; src++; i++;
    *dst=0;	   dst++; i++;
  }
  res->biBitCount    = 32;
  return res;
}

__forceinline LPBITMAPINFOHEADER rgb32rgb24(LPBITMAPINFOHEADER src, LPBYTE lpData, LPVOID lpDst)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;
  if (!lpData)
    lpData=GetData(src);

  int isize=src->biWidth*src->biHeight*3;
  if (lpDst)
    lpBMIH=(LPBITMAPINFOHEADER)lpDst;
  else
    lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize+isize);
  memset(lpBMIH,0,isize);
  memcpy(lpBMIH,src,src->biSize);
  lpBMIH->biBitCount=24;

  LPBYTE dsts=((LPBYTE)lpBMIH)+lpBMIH->biSize;
  LPBYTE eod=dsts + isize;
  LPBYTE srcs=lpData;

  while(dsts<eod)
  {
    *dsts=*srcs; dsts++; srcs++;
    *dsts=*srcs; dsts++; srcs++;
    *dsts=*srcs; dsts++; srcs++;
    srcs++;
  }
  return lpBMIH;
}

__forceinline LPBITMAPINFOHEADER rgb8rgb24(LPBITMAPINFOHEADER src, LPBYTE lpData, LPVOID lpDst)
{
  ASSERT(FALSE); // Not implemeted yet
  return NULL;
}

__forceinline LPBITMAPINFOHEADER rgb24rgb24(LPBITMAPINFOHEADER src, LPBYTE lpData, LPVOID lpDst)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;
  if (!lpData)
    lpData=GetData(src);

  int isize=src->biWidth*src->biHeight*3;
  ASSERT( lpData && isize ) ;
  if (lpDst)
    lpBMIH=(LPBITMAPINFOHEADER)lpDst;
  else
    lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize+isize);
  memcpy( lpBMIH , src , src->biSize );
  memcpy( lpBMIH + 1 , lpData , isize ) ;
  return lpBMIH;
}

LPBITMAPINFOHEADER rgbrgb24(LPBITMAPINFOHEADER src, LPBYTE lpData, LPVOID lpDst)
{
  switch (src->biBitCount)
  {
  case 8:
    return rgb8rgb24(src, lpData, lpDst);
  case 24:
    return rgb24rgb24(src, lpData, lpDst);
  case 32:
    return rgb32rgb24(src, lpData, lpDst);
  }
  ASSERT(FALSE); // Not implemeted yet
  return NULL;
}

LPBITMAPINFOHEADER y16rgb24(LPBITMAPINFOHEADER src, LPBYTE lpD)
{
  LPBITMAPINFOHEADER lpBMIH=NULL;
  if (!lpD)
    lpD=GetData(src);

  LPBYTE lpData=lpD; 
  int isize=3*(src->biWidth*src->biHeight);
  lpBMIH=(LPBITMAPINFOHEADER)malloc(src->biSize+isize);
  memcpy(lpBMIH,src,src->biSize);

  lpBMIH->biBitCount=24;
  lpBMIH->biSizeImage=0;
  lpBMIH->biCompression=0;
  lpBMIH->biClrUsed=0;
  lpBMIH->biPlanes = 1 ;
  lpBMIH->biClrImportant = 0;
  lpBMIH->biClrUsed = 0;
  LPBYTE dst=((LPBYTE)lpBMIH)+lpBMIH->biSize+3*src->biWidth*(lpBMIH->biHeight-1);

  for (int y=0; y<lpBMIH->biHeight; y++)
  {
    LPBYTE ssc=lpData+2*src->biWidth*y+1;
    LPBYTE eod=ssc + 2*src->biWidth;
    LPBYTE dsc=dst-3*y*src->biWidth;
    while(ssc<eod)
    {
      *dsc=*ssc;            dsc++;
      *dsc=*ssc;            dsc++;
      *dsc=*ssc;            dsc++;
      ssc+=2;
    } 
  }
  return lpBMIH;
}

LPBITMAPINFOHEADER _decompress2rgb(const pTVFrame frame, bool mono)
{
  LPBITMAPINFOHEADER lpRet=NULL;
  if ((frame) && (frame->lpBMIH))
  {
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
      if (mono)
        lpRet=yuv9rgb8(frame->lpBMIH,frame->lpData);
      else
        lpRet=yuv9rgb24(frame->lpBMIH,frame->lpData);
      break;
    case BI_YUV12:
      if (mono)
        lpRet=yuv12rgb8(frame->lpBMIH,frame->lpData);
      else
        lpRet=yuv12rgb24(frame->lpBMIH,frame->lpData);
      break;
    case BI_Y8:
    case BI_Y800:
      lpRet=y8rgb8(frame->lpBMIH,frame->lpData);
      break;
    case BI_Y16:
      lpRet=y16rgb8(frame->lpBMIH,frame->lpData);
      break;
    }
  }
  return lpRet;
}

LPBITMAPINFOHEADER _convertNV12toYUV9( pTVFrame frame )
{
  if ( frame->lpBMIH->biCompression != BI_NV12 )
  {
    ASSERT( 0 );
    return NULL ;
  }
  LPBYTE org = GetData( frame );
  int    orgSize = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight = frame->lpBMIH->biHeight;
  int    orgBWSize = orgWidth * orgHeight ;

  ASSERT( (orgWidth % 4) == 0 );
  ASSERT( (orgHeight % 4) == 0 );
  
  int iYUV9Size = (9 * orgBWSize) / 8;
  
  LPBITMAPINFOHEADER retV = (LPBITMAPINFOHEADER) malloc( frame->lpBMIH->biSize + iYUV9Size );
  memcpy( retV , frame->lpBMIH , frame->lpBMIH->biSize );

  retV->biSizeImage = iYUV9Size;
  retV->biCompression = BI_YUV9;
  retV->biBitCount = 9;
  LPBYTE dst = (LPBYTE)(retV + 1) ;

  memcpy( dst , GetData( frame ) , orgBWSize ) ; // copy luminance

  LPBYTE srcVU = org + orgBWSize ; // evens are V and odds are U
  LPBYTE dstV = dst + orgBWSize ;
  LPBYTE dstU = dstV + orgBWSize / 16 ;

  int iSrcColorStringSize = orgWidth ;
  int iDstColorStringSize = orgWidth / 4 ;
  int iNDstColorStrigns = orgHeight / 4 ;
  for ( int iY = 0 ; iY < iNDstColorStrigns ; iY++ )
  {
    LPBYTE pSrcStr = srcVU + iY * iSrcColorStringSize * 2 ;
    LPBYTE pDstV = dstV + iY * iDstColorStringSize ;
    LPBYTE pDstU = dstU + iY * iDstColorStringSize ;
    for ( int iX = 0 ; iX < iDstColorStringSize ; iX++ )
    {
      int iU = *(pSrcStr) + *(pSrcStr + iSrcColorStringSize)
        + *(pSrcStr + 2) + *(pSrcStr + iSrcColorStringSize) ;
      pSrcStr++ ;
      int iV = *(pSrcStr) +*(pSrcStr + iSrcColorStringSize)
        + *(pSrcStr + 2) + *(pSrcStr + iSrcColorStringSize) ;
      pSrcStr += 3 ;
      *(pDstV++) = (BYTE)(iV / 4) ;
      *(pDstU++) = (BYTE) (iU / 4) ;
    }
  }
  return retV;
}

LPBITMAPINFOHEADER _convertNV12toYUV12( pTVFrame frame )
{
  if ( frame->lpBMIH->biCompression != BI_NV12 )
  {
    ASSERT( 0 );
    return NULL ;
  }
  LPBYTE org = GetData( frame );
  int    orgSize = frame->lpBMIH->biSizeImage;
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight = frame->lpBMIH->biHeight;
  int    orgBWSize = orgWidth * orgHeight ;

  ASSERT( (orgWidth & 1) == 0 );
  ASSERT( (orgHeight &1) == 0 );

  int iYUV12Size = (orgBWSize * 3)/ 2 ;

  LPBITMAPINFOHEADER retV = (LPBITMAPINFOHEADER) malloc( frame->lpBMIH->biSize + iYUV12Size );
  memcpy( retV , frame->lpBMIH , frame->lpBMIH->biSize );

  retV->biSizeImage = iYUV12Size;
  retV->biCompression = BI_YUV12;
  retV->biBitCount = 12;
  LPBYTE dst = (LPBYTE) (retV + 1) ;

  memcpy( dst , GetData( frame ) , orgBWSize ) ; // copy luminance

  LPBYTE srcVU = org + orgBWSize ; // evens are V and odds are U
  LPBYTE dstV = dst + orgBWSize ;
  LPBYTE dstU = dstV + orgBWSize / 4 ;

  int iSrcColorStringSize = orgWidth ;
  int iDstColorStringSize = orgWidth / 2 ;
  int iNDstColorStrigns = orgHeight / 2 ;
  for ( int iY = 0 ; iY < iNDstColorStrigns ; iY++ )
  {
    LPBYTE pSrcStr = srcVU + iY * iSrcColorStringSize ;
    LPBYTE pDstV = dstV + iY * iDstColorStringSize ;
    LPBYTE pDstU = dstU + iY * iDstColorStringSize ;
    for ( int iX = 0 ; iX < iDstColorStringSize ; iX++ )
    {
      *(pDstV++) = *(pSrcStr++)  ;
      *(pDstU++) = *(pSrcStr++)  ;
    }
  }
  return retV;
}

void FX_EXT_SHVIDEO _swapUVforI420orYUV12( pTVFrame frame )
{
  if ( frame->lpBMIH->biCompression != BI_YUV12 
    && frame->lpBMIH->biCompression != BI_I420 )
  {
    ASSERT( 0 );
    return ;
  }
  LPBYTE org = GetData( frame );
  int    orgWidth = frame->lpBMIH->biWidth;
  int    orgHeight = frame->lpBMIH->biHeight;
  int    orgBWSize = orgWidth * orgHeight ;

  int iUSize = orgBWSize / 4 ;
  LPBYTE pTmp = new BYTE[ iUSize ] ;
  LPBYTE srcVU = org + orgBWSize ; // evens are V and odds are U
  memcpy( pTmp , srcVU , iUSize ) ;
  memcpy( srcVU , srcVU + iUSize , iUSize ) ;
  memcpy( srcVU + iUSize , pTmp , iUSize ) ;
  delete[] pTmp ;

  if ( frame->lpBMIH->biCompression == BI_YUV12 )
    frame->lpBMIH->biCompression = BI_I420 ;
  else
    frame->lpBMIH->biCompression = BI_YUV12 ;
}
