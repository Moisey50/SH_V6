//  $File : simpleip.h - simple image processing utilites
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)
#pragma once

#ifndef _SIMPLE_IMGPROC_INC
#define _SIMPLE_IMGPROC_INC

#include <math\intf_sup.h>
#include <video\tvframe.h>
#include <imageproc\basedef.h>
#include <imageproc\rectangles.h>
#include <helpers\16bit.h>

__forceinline bool _negative(pTVFrame frame)
{
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
        {
          LPBYTE lpData=GetData(frame);
          if (!lpData) return false;
          LPBYTE eod=lpData+frame->lpBMIH->biSizeImage;
          while (lpData<eod) { *lpData=~(*lpData); lpData++;}
          return true;
        }
    case BI_Y16:
        {
          WORD* lpData=(WORD*)GetData(frame);
          if (!lpData) return false;
          WORD* eod=lpData+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
          while (lpData<eod) { *lpData=~(*lpData); lpData++;}
          return true;
        }
    }
    return false;
}

__forceinline bool _contrast(pTVFrame frame, double c)
{
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
    case BI_Y8:
        {
          LPBYTE lpData=GetData(frame);
          if (!lpData) return false;
          LPBYTE eod=lpData+frame->lpBMIH->biSizeImage;
          while (lpData<eod) 
	  { 
        int d=(int)(128.0+(*lpData-128.0)*c+0.5); 
        *lpData=(d>255)?255:(d<0)?0:d;
		lpData++;
	  }
          return true;
        }
    case BI_Y16:
        {
          WORD* lpData=(WORD*)GetData(frame);
          if (!lpData) return false;
          WORD* eod=lpData+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
          while (lpData<eod) 
          { 
              int d=(int)(128.0+(*lpData-128.0)*c+0.5); 
              *lpData=(d>255)?255:(d<0)?0:d;
              lpData++;
    	  }
          return true;
        }
    }
    return false;
}

__forceinline bool _clearcolor(pTVFrame frame)
{    
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9: //the only color format
        {
            // This old implementation just set U and V values to 128...
            /* LPBYTE lpData=GetData(frame);
            if (!lpData) return false;
            LPBYTE eod=lpData+frame->lpBMIH->biSizeImage;
            lpData+=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
            while (lpData<eod) { *lpData=128; lpData++;}
            return true;*/

            // The second implementation converts it to Y8 format (just change BM header
            frame->lpBMIH->biCompression = BI_Y8;
            frame->lpBMIH->biSizeImage    = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
			frame->lpBMIH->biBitCount=0;
            return true;

        }
    }
    return false;
}

__forceinline bool _clearDID(pTVFrame frame)
{    
  LPBYTE lpData=GetData(frame);
  if (!lpData) return false;
  memcpy(lpData,lpData+Width(frame), Width(frame));
  return true;
}

__forceinline void _mass_binarize8(LPBYTE data,long datasize, double ratio=1.0)
{
    DWORD cm=0;
	LPBYTE i;
    for (i=data; i<data+datasize; i++)
    {
        cm+=*i;
    }
    cm/=datasize;
    cm=(int)(ratio*cm);
    for (i=data; i<data+datasize; i++)
    {
        *i=((*i)>cm)?255:0;
    }
}

__forceinline void _mass_binarize16(LPWORD data,long datasize, double ratio=1.0)
{
    __int64 cm=0;
	LPWORD i;
    for (i=data; i<data+datasize; i++)
    {
        cm+=*i;
    }
    cm/=datasize;
    cm=(int)(ratio*cm);
    for (i=data; i<data+datasize; i++)
    {
        *i=((*i)>cm)?65535:0;
    }
}

__forceinline void _percent_binarize8(LPBYTE data,long datasize, int percent)
{
    DWORD cm=0;
    int hist[256];
    memset(hist,0,sizeof(hist));
	LPBYTE i;
    for (i=data; i<data+datasize; i++)
    {
        hist[*i]++;
    }
    
    int level=255;
    int vol=0;
    int tvol=(percent*datasize)/100;
    
    while((level) && (vol<tvol))
    {
        vol+=hist[level]; level--;
    }
    
    level--;

    for (i=data; i<data+datasize; i++)
    {
        *i=((*i)>level)?255:0;
    }
}

__forceinline void _percent_binarize16(LPWORD data,long datasize, int percent)
{
    DWORD cm=0;
    DWORD hist[256];
    memset(hist,0,sizeof(hist));
	LPWORD i;
    for (i=data; i<data+datasize; i++)
    {
        hist[((*i)>>8)]++;
    }
    
    int level=255;
    int vol=0;
    int tvol=(percent*datasize)/100;
    
    while((level) && (vol<tvol))
    {
        vol+=hist[level]; level--;
    }
    
    level--;
    level=(level<<8);
    for (i=data; i<data+datasize; i++)
    {
        *i=((*i)>level)?65535:0;
    }
}


__forceinline void _simplebinarize8(LPBYTE data,long datasize,int level)
{
    for (LPBYTE i=data; i<data+datasize; i++)
    {
        *i=((*i)>level)?255:0;
    }
}

__forceinline void _simplebinarize16(LPWORD data,long datasize,int level)
{
    for (LPWORD i=data; i<data+datasize; i++)
    {
        *i=((*i)>level)?65535:0;
    }
}

__forceinline void _simplebinarize(pTVFrame frame,int level=128)
{

    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
        {
            _simplebinarize8(GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,level);
            LPBYTE yu=GetData(frame)+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
            memset(yu,128,(frame->lpBMIH->biWidth*frame->lpBMIH->biHeight)>>1);
            break;
        }
    case BI_YUV9:
        {
            _simplebinarize8(GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,level);
            LPBYTE yu=GetData(frame)+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
            memset(yu,128,(frame->lpBMIH->biWidth*frame->lpBMIH->biHeight)>>3);
            break;
        }
    case BI_Y8:
        {
            _simplebinarize8(GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,level);
            break;
        }
    case BI_Y16:
        {
            _simplebinarize16((LPWORD)GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,level);
            break;
        }
    }
}

__forceinline void _percentbinarize(pTVFrame frame, int percent)
{
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
        {
            _percent_binarize8(GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,percent);
            if (frame->lpBMIH->biCompression==BI_YUV12)
            {
                LPBYTE yu=GetData(frame)+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
                memset(yu,128,(frame->lpBMIH->biWidth*frame->lpBMIH->biHeight)>>1);
            }
            break;
        }
    case BI_YUV9:
        {
            _percent_binarize8(GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,percent);
            if (frame->lpBMIH->biCompression==BI_YUV9)
            {
                LPBYTE yu=GetData(frame)+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
                memset(yu,128,(frame->lpBMIH->biWidth*frame->lpBMIH->biHeight)>>3);
            }
            break;
        }
    case BI_Y8:
        {
            _percent_binarize8(GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,percent);
            break;
        }
    case BI_Y16:
        {
            _percent_binarize16((LPWORD)GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,percent);
            break;
        }
    }
}

__forceinline void _massbinarize(pTVFrame frame, double ratio=1.0)
{
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
        {
            _mass_binarize8(GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,ratio);
            if (frame->lpBMIH->biCompression==BI_YUV12)
            {
                LPBYTE yu=GetData(frame)+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
                memset(yu,128,(frame->lpBMIH->biWidth*frame->lpBMIH->biHeight)>>1);
            }
            break;
        }
    case BI_YUV9:
        {
            _mass_binarize8(GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,ratio);
            if (frame->lpBMIH->biCompression==BI_YUV9)
            {
                LPBYTE yu=GetData(frame)+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
                memset(yu,128,(frame->lpBMIH->biWidth*frame->lpBMIH->biHeight)>>3);
            }
            break;
        }
    case BI_Y8:
        {
            _mass_binarize8(GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,ratio);
            break;
        }
    case BI_Y16:
        {
            _mass_binarize16((LPWORD)GetData(frame),frame->lpBMIH->biWidth*frame->lpBMIH->biHeight,ratio);
            break;
        }
    }
}

__forceinline void _decrease_colordepth(pTVFrame frame)
{
    pTVFrame retV=frame;
    LPBYTE lpD=GetData(frame);
    LPBYTE eoD=lpD+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
    while(lpD<eoD)
    {
        *lpD=(((*lpD)>>4)<<4);
        lpD++;
    }
}

__forceinline void _clear_ROI(pTVFrame frame, RECT* rc, DWORD color=0)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;
    
    int offset=rc->left+rc->top*width;
    memset(Data,color,offset);
    Data+=rc->right+rc->top*width;
    for (int i=0; i<rc->bottom-rc->top; i++)
    {
        memset(Data,0,width-rc->right+rc->left);
        Data+=width;
    }
    offset=width*(height-rc->bottom)-rc->right;
    memset(Data,color,offset);
}


__forceinline void _clear_frames(pTVFrame frame, DWORD color=0, DWORD marge = 5)
{
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
        {
            LPBYTE Data=GetData(frame);
            DWORD width=frame->lpBMIH->biWidth;
            DWORD height=frame->lpBMIH->biHeight;

	        DWORD field = marge * width;
	        LPBYTE end = Data + width * height - field;
	        memset(Data, color, field);
	        memset(end, color, field);
	        Data += field;
	        while (Data < end)
	        {
		        memset(Data, color, marge);
		        memset(Data + width - marge, color, marge);
		        Data += width;
	        }
            break;
        }
    case BI_Y16:
        {
            LPWORD Data=(LPWORD)GetData(frame);
            DWORD width=frame->lpBMIH->biWidth;
            DWORD height=frame->lpBMIH->biHeight;

	        DWORD field = marge * width;
	        LPWORD end = Data + width * height - field;
	        memsetW(Data, color, field);
	        memsetW(end, color, field);
	        Data += field;
	        while (Data < end)
	        {
		        memsetW(Data, color, marge);
		        memsetW(Data + width - marge, color, marge);
		        Data += width;
	        }
            break;
        }
    }
}

__forceinline void _rotate_upside_down(LPBYTE Data, int width, int height)
{
	int i, size = width * height;
	for (i = 0; i < size/2; i++)
	{
		BYTE tmp = Data[i]; Data[i] = Data[size-i-1]; Data[size-i-1] = tmp;
	}
	Data += size;
	size /= 16;
	for (i = 0; i < size/2; i++)
	{
		BYTE tmp = Data[i]; Data[i] = Data[size-i-1]; Data[size-i-1] = tmp;
	}
	Data += size;
	for (i = 0; i < size/2; i++)
	{
		BYTE tmp = Data[i]; Data[i] = Data[size-i-1]; Data[size-i-1] = tmp;
	}
}

__forceinline pTVFrame _align_size(pTVFrame SrcFrame, pTVFrame Pattern)
{
	if ((SrcFrame->lpBMIH->biWidth > Pattern->lpBMIH->biWidth) || (SrcFrame->lpBMIH->biHeight > Pattern->lpBMIH->biHeight))
		return NULL;
	pTVFrame DstFrame = makecopyTVFrame(Pattern);
	LPBYTE Dst = GetData(DstFrame);
	LPBYTE Src = GetData(SrcFrame);
	memset(Dst, 0, DstFrame->lpBMIH->biWidth * DstFrame->lpBMIH->biHeight);
	LPBYTE SrcEnd = Src + SrcFrame->lpBMIH->biWidth * SrcFrame->lpBMIH->biHeight;
	while (Src < SrcEnd)
	{
		memcpy(Dst, Src, SrcFrame->lpBMIH->biWidth);
		Src += SrcFrame->lpBMIH->biWidth;
		Dst += DstFrame->lpBMIH->biWidth;
	}
	SrcEnd += SrcFrame->lpBMIH->biWidth * SrcFrame->lpBMIH->biHeight / 16;
	Dst = GetData(DstFrame) + DstFrame->lpBMIH->biWidth * DstFrame->lpBMIH->biHeight;
	memset(Dst, 0x7f, DstFrame->lpBMIH->biWidth * DstFrame->lpBMIH->biHeight / 8);
	while (Src < SrcEnd)
	{
		memcpy(Dst, Src, SrcFrame->lpBMIH->biWidth / 4);
		Src += SrcFrame->lpBMIH->biWidth / 4;
		Dst += DstFrame->lpBMIH->biWidth / 4;
	}
	SrcEnd += SrcFrame->lpBMIH->biWidth * SrcFrame->lpBMIH->biHeight / 16;
	Dst = GetData(DstFrame) + DstFrame->lpBMIH->biWidth * DstFrame->lpBMIH->biHeight * 17 / 16;
	while (Src < SrcEnd)
	{
		memcpy(Dst, Src, SrcFrame->lpBMIH->biWidth / 4);
		Src += SrcFrame->lpBMIH->biWidth / 4;
		Dst += DstFrame->lpBMIH->biWidth / 4;
	}
	return DstFrame;
}

__forceinline pTVFrame _align1_size(pTVFrame SrcFrame, pTVFrame Pattern)
{
	if ((SrcFrame->lpBMIH->biWidth > Pattern->lpBMIH->biWidth) || (SrcFrame->lpBMIH->biHeight > Pattern->lpBMIH->biHeight))
		return NULL;
	pTVFrame DstFrame = makecopyTVFrame(Pattern);
	LPBYTE Dst1 = GetData(DstFrame);
	LPBYTE Src1 = GetData(SrcFrame);
	memset(Dst1, 0, DstFrame->lpBMIH->biWidth * DstFrame->lpBMIH->biHeight);
	LPBYTE SrcEnd = Src1 + SrcFrame->lpBMIH->biWidth * SrcFrame->lpBMIH->biHeight / 2;
	LPBYTE Dst2 = Dst1 + DstFrame->lpBMIH->biWidth - SrcFrame->lpBMIH->biWidth / 2;
	LPBYTE Src2 = Src1 + SrcFrame->lpBMIH->biWidth / 2;
	LPBYTE Dst3 = Dst1 + (DstFrame->lpBMIH->biHeight - SrcFrame->lpBMIH->biHeight / 2) * DstFrame->lpBMIH->biWidth;
	LPBYTE Src3 = Src1 + SrcFrame->lpBMIH->biHeight / 2 * SrcFrame->lpBMIH->biWidth;
	LPBYTE Dst4 = Dst2 + (DstFrame->lpBMIH->biHeight - SrcFrame->lpBMIH->biHeight / 2) * DstFrame->lpBMIH->biWidth;
	LPBYTE Src4 = Src2 + SrcFrame->lpBMIH->biHeight / 2 * SrcFrame->lpBMIH->biWidth;
	while (Src1 < SrcEnd)
	{
		memcpy(Dst1, Src4, SrcFrame->lpBMIH->biWidth / 2);
		memcpy(Dst2, Src3, SrcFrame->lpBMIH->biWidth / 2);
		memcpy(Dst3, Src2, SrcFrame->lpBMIH->biWidth / 2);
		memcpy(Dst4, Src1, SrcFrame->lpBMIH->biWidth / 2);
		Src1 += SrcFrame->lpBMIH->biWidth;
		Dst1 += DstFrame->lpBMIH->biWidth;
		Src2 += SrcFrame->lpBMIH->biWidth;
		Dst2 += DstFrame->lpBMIH->biWidth;
		Src3 += SrcFrame->lpBMIH->biWidth;
		Dst3 += DstFrame->lpBMIH->biWidth;
		Src4 += SrcFrame->lpBMIH->biWidth;
		Dst4 += DstFrame->lpBMIH->biWidth;
	}
	Src1 = GetData(SrcFrame) + SrcFrame->lpBMIH->biWidth * SrcFrame->lpBMIH->biHeight;
	SrcEnd = Src1 + SrcFrame->lpBMIH->biWidth * SrcFrame->lpBMIH->biHeight / 32;
	Dst1 = GetData(DstFrame) + DstFrame->lpBMIH->biWidth * DstFrame->lpBMIH->biHeight;
	memset(Dst1, 0x7f, DstFrame->lpBMIH->biWidth * DstFrame->lpBMIH->biHeight / 8);
/*	while (Src < SrcEnd)
	{
		memcpy(Dst, Src, SrcFrame->lpBMIH->biWidth / 4);
		Src += SrcFrame->lpBMIH->biWidth / 4;
		Dst += DstFrame->lpBMIH->biWidth / 4;
	}
	SrcEnd += SrcFrame->lpBMIH->biWidth * SrcFrame->lpBMIH->biHeight / 16;
	Dst = GetData(DstFrame) + DstFrame->lpBMIH->biWidth * DstFrame->lpBMIH->biHeight * 17 / 16;
	while (Src < SrcEnd)
	{
		memcpy(Dst, Src, SrcFrame->lpBMIH->biWidth / 4);
		Src += SrcFrame->lpBMIH->biWidth / 4;
		Dst += DstFrame->lpBMIH->biWidth / 4;
	}*/
	return DstFrame;
}

__forceinline void _draw_rect(pTVFrame Frame, LPRECT rc)
{
	BYTE color = 255;
	if (rc->top >= 0 && rc->top < Frame->lpBMIH->biHeight && rc->right >= 0)
	{
		LPBYTE Dst = GetData(Frame) + rc->top * Frame->lpBMIH->biWidth;
		LPBYTE End = (rc->right < Frame->lpBMIH->biWidth) ? Dst + rc->right : Dst + Frame->lpBMIH->biWidth;
		if (rc->left > 0)
			Dst += rc->left;
		while (Dst < End)
		{
			*Dst = color;
			Dst++;
			color = 255 ^ color;
		}
	}
	if (rc->bottom > rc->top && rc->bottom < Frame->lpBMIH->biHeight && rc->right >= 0)
	{
		LPBYTE Dst = GetData(Frame) + rc->bottom * Frame->lpBMIH->biWidth;
		LPBYTE End = (rc->right < Frame->lpBMIH->biWidth) ? Dst + rc->right : Dst + Frame->lpBMIH->biWidth;
		if (rc->left > 0)
			Dst += rc->left;
		while (Dst < End)
		{
			*Dst = color;
			Dst++;
			color = 255 ^ color;
		}
	}
	if (rc->left >= 0 && rc->left < Frame->lpBMIH->biWidth && rc->bottom >= 0)
	{
		LPBYTE Dst = (rc->top > 0) ? GetData(Frame) + rc->top * Frame->lpBMIH->biWidth : GetData(Frame);
		LPBYTE End = (rc->bottom < Frame->lpBMIH->biHeight) ? GetData(Frame) + rc->bottom * Frame->lpBMIH->biWidth : GetData(Frame) + Frame->lpBMIH->biWidth * Frame->lpBMIH->biHeight;
		if (rc->left > 0)
		{
			Dst += rc->left;
			End += rc->left;
		}
		while (Dst < End)
		{
			*Dst = color;
			Dst += Frame->lpBMIH->biWidth;
			color = 255 ^ color;
		}
	}
	if (rc->right >= rc->left && rc->right < Frame->lpBMIH->biWidth && rc->right >= 0 && rc->bottom >= 0)
	{
		LPBYTE Dst = (rc->top > 0) ? GetData(Frame) + rc->top * Frame->lpBMIH->biWidth : GetData(Frame);
		LPBYTE End = (rc->bottom < Frame->lpBMIH->biHeight) ? GetData(Frame) + rc->bottom * Frame->lpBMIH->biWidth : GetData(Frame) + Frame->lpBMIH->biWidth * Frame->lpBMIH->biHeight;
		if (rc->right < Frame->lpBMIH->biWidth)
		{
			Dst += rc->right;
			End += rc->right;
		}
		else
		{
			Dst += Frame->lpBMIH->biWidth - 1;
			End += Frame->lpBMIH->biWidth - 1;
		}
		while (Dst < End)
		{
			*Dst = color;
			Dst += Frame->lpBMIH->biWidth;
			color = 255 ^ color;
		}
	}
}

__forceinline void _draw_cross(pTVFrame Frame, POINT* pt)
{
	if (pt->x < 2 || pt->y < 2 || pt->x > Frame->lpBMIH->biWidth - 3 || pt->y > Frame->lpBMIH->biHeight - 3)
		return;
	LPBYTE Dst = GetData(Frame) + (pt->y - 2) * Frame->lpBMIH->biWidth + pt->x - 2;
	LPBYTE End = Dst + 5 * Frame->lpBMIH->biWidth;
	while (Dst < End)
	{
		memset(Dst, 0, 5);
		Dst += Frame->lpBMIH->biWidth;
	}
	Dst = GetData(Frame) + pt->y * Frame->lpBMIH->biWidth + pt->x;
	*(Dst - 2 * Frame->lpBMIH->biWidth) = 255;
	*(Dst - 1 * Frame->lpBMIH->biWidth) = 255;
	*(Dst + 1 * Frame->lpBMIH->biWidth) = 255;
	*(Dst + 2 * Frame->lpBMIH->biWidth) = 255;
	*(Dst - 2) = 255;
	*(Dst - 1) = 255;
	*(Dst + 1) = 255;
	*(Dst + 2) = 255;
}

#include <imageproc\videologic.h>
#include <imageproc\cut.h>
#include <imageproc\resample.h>
#include <imageproc\normalize.h>
#include <imageproc\fstfilter.h>
#include <imageproc\featuredetector.h>
#endif //_SIMPLE_IMGPROC_INC
