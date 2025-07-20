//  $File : Yuv9.cpp - base yuv9 convbersion utility
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 First release version, all followed changes must be listed below  (Andrey Chernushich)


#include "stdafx.h"
#include <video\shvideo.h>
#include <imageproc\dibutils.h>

#pragma comment(lib,"winmm.lib")

//#define VR_RATIO(a) (-5669*(int)a) 
//#define VG_RATIO(a) (-11141*(int)a)
//#define VB_RATIO(a) (16777*(int)a)
#define VR_RATIO(a) (-5536*(int)a) 
#define VG_RATIO(a) (-10847*(int)a)
#define VB_RATIO(a) (16383*(int)a)

#define UR_RATIO(a) (16383*(int)a) 
#define UG_RATIO(a) (-13663*(int)a)
#define UB_RATIO(a) (-2720*(int)a)

__forceinline void _makeRGB15(LPBYTE pntr, DWORD& R, DWORD& G, DWORD& B)
{
    unsigned short dw=*((unsigned short*)pntr);
    B=((dw&0x1F)<<3); dw=(dw>>5);
    G=((dw&0x1F)<<3); dw=(dw>>5);
    R=((dw&0x1F)<<3);
}

__forceinline unsigned char _makeY(DWORD R, DWORD G, DWORD B)
{
    return((unsigned char)((9765*R+19169*G+3801*B)>>15));
}

__forceinline bool _compress2yuv9_15(pTVFrame in)
{
    int oldWidth=in->lpBMIH->biWidth;
    int oldWidthB=oldWidth*2;
    int oldWidthB2=oldWidth*4;
    int oldWidthB3=oldWidth*6;
    int oldDepth=in->lpBMIH->biHeight;
    int newWidth=(oldWidth>>2)<<2;
    int newDepth=(oldDepth>>2)<<2;
    int newImageSize=newWidth*newDepth;
    int oldImageSize=oldWidth*newDepth*2;
    newImageSize+=(newImageSize>>3);
    LPBYTE newData=(LPBYTE)malloc(newImageSize);
    LPBYTE lpDataSrc=(in->lpData)?in->lpData:((LPBYTE)in->lpBMIH)+in->lpBMIH->biSize;
    LPBYTE newDataPntr=newData;
    LPBYTE colUDataOffset=newData+newWidth*newDepth, colVDataOffset=colUDataOffset+(newWidth*newDepth>>4);
    LPBYTE oldDataPntr=lpDataSrc+oldImageSize-oldWidth*2;

    int boxDepth=newDepth>>2;
    int boxWidth=newWidth>>2;
    DWORD R,G,B, rS, gS, bS;
    for (int y=0; y<boxDepth; y++)
    {
        for (int x=0; x<boxWidth; x++)
        {
            rS=0; gS=0; bS=0;

            _makeRGB15(oldDataPntr,R,G,B);   rS+=R; gS+=G; bS+=B;
            *newDataPntr=_makeY(R,G,B);
            _makeRGB15(oldDataPntr+2,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+1)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr+4,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr+6,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3)=_makeY(R,G,B);

            _makeRGB15(oldDataPntr-oldWidthB,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr-oldWidthB+2,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+1)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr-oldWidthB+4,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+2)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr-oldWidthB+6,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+3)=_makeY(R,G,B);

            _makeRGB15(oldDataPntr-oldWidthB2,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr-oldWidthB2+2,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+1)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr-oldWidthB2+4,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+2)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr-oldWidthB2+6,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+3)=_makeY(R,G,B);

            _makeRGB15(oldDataPntr-oldWidthB3,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr-oldWidthB3+2,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+1)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr-oldWidthB3+4,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+2)=_makeY(R,G,B);
            _makeRGB15(oldDataPntr-oldWidthB3+6,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+3)=_makeY(R,G,B);
            *colVDataOffset=(unsigned char)(((VR_RATIO(rS)+VG_RATIO(gS)+VB_RATIO(bS))>>19)+128);
            *colUDataOffset=(unsigned char)(((UR_RATIO(rS)+UG_RATIO(gS)+UB_RATIO(bS))>>19)+128);

            colVDataOffset++;
            colUDataOffset++;

            newDataPntr+=4;
            oldDataPntr+=8;
        }
        newDataPntr+=3*newWidth;
        oldDataPntr-=oldWidth*10;
    }
    in->lpBMIH->biHeight=newDepth;
    in->lpBMIH->biWidth=newWidth;
    in->lpBMIH->biSizeImage=newImageSize;

    if (in->lpData) 
    {
        free(in->lpData);
        in->lpData=newData;
    }
    else
    {
        LPBYTE newBMIH=(LPBYTE)malloc(newImageSize+in->lpBMIH->biSize);
        memcpy(newBMIH,in->lpBMIH,in->lpBMIH->biSize);
        memcpy(newBMIH+in->lpBMIH->biSize,newData,newImageSize);
        free(in->lpBMIH);
        free(newData);
        in->lpBMIH=(LPBITMAPINFOHEADER)newBMIH;
    }
    
    return(true);
}

//BITMAPINFO
__forceinline void _makeRGB8(LPBYTE pntr, RGBQUAD * Palette, DWORD& R, DWORD& G, DWORD& B)
{
    R=Palette[*(pntr)].rgbRed;
    G=Palette[*(pntr)].rgbGreen;
    B=Palette[*(pntr)].rgbBlue; 
}

__forceinline bool _compress2yuv9_8(pTVFrame in)
{

    ASSERT(in->lpData==NULL); //Just that format supported...

    LPBITMAPINFOHEADER oldBMIH=in->lpBMIH, newBMIH=NULL;

    int rnd=(oldBMIH->biWidth%4)?(4-(oldBMIH->biWidth%4)):0;
    int oldWidth=oldBMIH->biWidth+rnd;
    int a=oldWidth%4;
    int oldWidthB=oldWidth;
    int oldWidthB2=oldWidth*2;
    int oldWidthB3=oldWidth*3;
    int oldDepth=oldBMIH->biHeight;
    int newWidth=(oldWidth>>2)<<2;
    int newDepth=(oldDepth>>2)<<2;
    int newImageSize=newWidth*newDepth;
    int oldImageSize=(oldWidth*oldDepth);
    newImageSize+=(newImageSize>>3);
    newBMIH=(LPBITMAPINFOHEADER)malloc(oldBMIH->biSize+newImageSize);
    memcpy(newBMIH,oldBMIH,oldBMIH->biSize);
    LPBYTE newData=(LPBYTE)newBMIH+oldBMIH->biSize;
    LPBYTE newDataPntr=newData;
    LPBYTE colUDataOffset=newData+newWidth*newDepth, colVDataOffset=colUDataOffset+(newWidth*newDepth>>4);
    LPBYTE oldDataPntr=(LPBYTE)oldBMIH+oldBMIH->biSize+PaletteSize(oldBMIH)+oldImageSize-oldWidth;
    RGBQUAD* Palette=(RGBQUAD*)((LPBYTE)(oldBMIH)+oldBMIH->biSize);

    int boxDepth=newDepth>>2;
    int boxWidth=newWidth>>2;
    DWORD R,G,B, rS, gS, bS;
    for (int y=0; y<boxDepth; y++)
    {
        for (int x=0; x<boxWidth; x++)
        {
            rS=0; gS=0; bS=0;

            _makeRGB8(oldDataPntr,Palette,R,G,B);   rS+=R; gS+=G; bS+=B;
            *newDataPntr=_makeY(R,G,B);
            _makeRGB8(oldDataPntr+1,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+1)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr+2,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr+3,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3)=_makeY(R,G,B);

            _makeRGB8(oldDataPntr-oldWidthB,Palette,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr-oldWidthB+1,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+1)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr-oldWidthB+2,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+2)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr-oldWidthB+3,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+3)=_makeY(R,G,B);

            _makeRGB8(oldDataPntr-oldWidthB2,Palette,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr-oldWidthB2+1,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+1)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr-oldWidthB2+2,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+2)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr-oldWidthB2+3,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+3)=_makeY(R,G,B);

            _makeRGB8(oldDataPntr-oldWidthB3,Palette,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr-oldWidthB3+1,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+1)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr-oldWidthB3+2,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+2)=_makeY(R,G,B);
            _makeRGB8(oldDataPntr-oldWidthB3+3,Palette,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+3)=_makeY(R,G,B);
            *colVDataOffset=(unsigned char)((VR_RATIO(rS)+VG_RATIO(gS)+VB_RATIO(bS))>>19)+128;
            *colUDataOffset=(unsigned char)((UR_RATIO(rS)+UG_RATIO(gS)+UB_RATIO(bS))>>19)+128;

            colVDataOffset++;
            colUDataOffset++;

            newDataPntr+=4;
            oldDataPntr+=4;
        }
        newDataPntr+=3*newWidth;
        oldDataPntr-=oldWidth*5;
    }
    free(oldBMIH);
    in->lpBMIH=newBMIH;
    in->lpBMIH->biHeight=newDepth;
    in->lpBMIH->biWidth=newWidth;
    in->lpBMIH->biSizeImage=newImageSize;
    return(true);
}

__forceinline void _makeRGB(LPBYTE pntr, DWORD& R, DWORD& G, DWORD& B)
{
    B=*(pntr);
    G=*(pntr+1);
    R=*(pntr+2);
}

__forceinline bool _compress2yuv9_24(pTVFrame in)
{
    ASSERT(in->lpData==NULL); //Just that format supported...

    LPBITMAPINFOHEADER oldBMIH=in->lpBMIH, newBMIH=NULL;

    int oldWidth=oldBMIH->biWidth;
    int oldDepth=oldBMIH->biHeight;
    int newWidth=(oldWidth>>2)<<2;
    int newDepth=(oldDepth>>2)<<2;
    int newImageSize=newWidth*newDepth;
    int oldWidthB=(((oldWidth*oldBMIH->biBitCount) + 31) / 32 * 4);
    int oldWidthB2=oldWidthB*2;
    int oldWidthB3=oldWidthB*3;
    int oldImageSize=oldWidthB*newDepth;
    newImageSize+=(newImageSize>>3);
    newBMIH=(LPBITMAPINFOHEADER)malloc(oldBMIH->biSize+newImageSize);
    memcpy(newBMIH,oldBMIH,oldBMIH->biSize);
    LPBYTE newData=(LPBYTE)newBMIH+oldBMIH->biSize;
    LPBYTE newDataPntr=newData;
    LPBYTE colUDataOffset=newData+newWidth*newDepth, colVDataOffset=colUDataOffset+(newWidth*newDepth>>4);
    LPBYTE oldDataPntr=(LPBYTE)oldBMIH+oldBMIH->biSize+PaletteSize(oldBMIH)+oldImageSize-oldWidthB;

    int boxDepth=newDepth>>2;
    int boxWidth=newWidth>>2;
    DWORD R,G,B, rS, gS, bS;
    for (int y=0; y<boxDepth; y++)
    {
        for (int x=0; x<boxWidth; x++)
        {
            rS=0; gS=0; bS=0;

            _makeRGB(oldDataPntr,R,G,B);   rS+=R; gS+=G; bS+=B;
            *newDataPntr=_makeY(R,G,B);
            _makeRGB(oldDataPntr+3,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+1)=_makeY(R,G,B);
            _makeRGB(oldDataPntr+6,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2)=_makeY(R,G,B);
            _makeRGB(oldDataPntr+9,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3)=_makeY(R,G,B);

            _makeRGB(oldDataPntr-oldWidthB,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB+3,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+1)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB+6,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+2)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB+9,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+3)=_makeY(R,G,B);

            _makeRGB(oldDataPntr-oldWidthB2,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB2+3,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+1)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB2+6,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+2)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB2+9,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+3)=_makeY(R,G,B);

            _makeRGB(oldDataPntr-oldWidthB3,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB3+3,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+1)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB3+6,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+2)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB3+9,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+3)=_makeY(R,G,B);

	        *colVDataOffset=(unsigned char)(((VR_RATIO(rS)+VG_RATIO(gS)+VB_RATIO(bS))>>19)+128);
            *colUDataOffset=(unsigned char)(((UR_RATIO(rS)+UG_RATIO(gS)+UB_RATIO(bS))>>19)+128);

            colVDataOffset++;
            colUDataOffset++;

            newDataPntr+=4;
            oldDataPntr+=12;
        }
        newDataPntr+=3*newWidth;
        oldDataPntr-=oldWidthB*4+newWidth*3;
    }
    free(oldBMIH);
    in->lpBMIH=newBMIH;
    in->lpBMIH->biHeight=newDepth;
    in->lpBMIH->biWidth=newWidth;
    in->lpBMIH->biSizeImage=newImageSize;
    return(true);
}

__forceinline bool _compress2yuv9_32(pTVFrame in)
{
    ASSERT(in->lpData==NULL); //Just that format supported...

    LPBITMAPINFOHEADER oldBMIH=in->lpBMIH, newBMIH=NULL;

    int oldWidth=oldBMIH->biWidth;
    int oldDepth=oldBMIH->biHeight;
    int newWidth=(oldWidth>>2)<<2;
    int newDepth=(oldDepth>>2)<<2;
    int newImageSize=newWidth*newDepth;
    int oldWidthB=(((oldWidth*oldBMIH->biBitCount) + 31) / 32 * 4);
    int oldWidthB2=oldWidthB*2;
    int oldWidthB3=oldWidthB*3;
    int oldImageSize=oldWidthB*newDepth;
    newImageSize+=(newImageSize>>3);
    newBMIH=(LPBITMAPINFOHEADER)malloc(oldBMIH->biSize+newImageSize);
    memcpy(newBMIH,oldBMIH,oldBMIH->biSize);
    LPBYTE newData=(LPBYTE)newBMIH+oldBMIH->biSize;
    LPBYTE newDataPntr=newData;
    LPBYTE colUDataOffset=newData+newWidth*newDepth, colVDataOffset=colUDataOffset+(newWidth*newDepth>>4);
    LPBYTE oldDataPntr=(LPBYTE)oldBMIH+oldBMIH->biSize+PaletteSize(oldBMIH)+oldImageSize-oldWidthB;

    int boxDepth=newDepth>>2;
    int boxWidth=newWidth>>2;
    DWORD R,G,B, rS, gS, bS;
    for (int y=0; y<boxDepth; y++)
    {
        for (int x=0; x<boxWidth; x++)
        {
            rS=0; gS=0; bS=0;

            _makeRGB(oldDataPntr,R,G,B);   rS+=R; gS+=G; bS+=B;
            *newDataPntr=_makeY(R,G,B);
            _makeRGB(oldDataPntr+4,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+1)=_makeY(R,G,B);
            _makeRGB(oldDataPntr+8,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2)=_makeY(R,G,B);
            _makeRGB(oldDataPntr+12,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3)=_makeY(R,G,B);

            _makeRGB(oldDataPntr-oldWidthB,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB+4,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+1)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB+8,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+2)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB+12,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+newWidth+3)=_makeY(R,G,B);

            _makeRGB(oldDataPntr-oldWidthB2,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB2+4,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+1)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB2+8,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+2)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB2+12,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+2*newWidth+3)=_makeY(R,G,B);

            _makeRGB(oldDataPntr-oldWidthB3,R,G,B);   rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB3+4,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+1)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB3+8,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+2)=_makeY(R,G,B);
            _makeRGB(oldDataPntr-oldWidthB3+12,R,G,B); rS+=R; gS+=G; bS+=B;
            *(newDataPntr+3*newWidth+3)=_makeY(R,G,B);
            *colVDataOffset=(unsigned char)((VR_RATIO(rS)+VG_RATIO(gS)+VB_RATIO(bS))>>19)+128;
            *colUDataOffset=(unsigned char)((UR_RATIO(rS)+UG_RATIO(gS)+UB_RATIO(bS))>>19)+128;

            colVDataOffset++;
            colUDataOffset++;

            newDataPntr+=4;
            oldDataPntr+=16;
        }
        newDataPntr+=3*newWidth;
        oldDataPntr-=oldWidthB*4+newWidth*4;
    }
    free(oldBMIH);
    in->lpBMIH=newBMIH;
    in->lpBMIH->biHeight=newDepth;
    in->lpBMIH->biWidth=newWidth;
    in->lpBMIH->biSizeImage=newImageSize;
    return(true);
}

bool compress2yuv9(pTVFrame in)
{
  if ((in->lpBMIH->biCompression!=BI_RGB)) return(false);
  if (in->lpData) // Make "bitmap in one memblock"
  {
      BITMAPINFOHEADER* tmpBMIH=in->lpBMIH;
      DWORD imagesize=in->lpBMIH->biSizeImage;
      if (!imagesize)
      {
          imagesize=RowSize(in->lpBMIH)*in->lpBMIH->biHeight;
      }
      in->lpBMIH=(BITMAPINFOHEADER*)malloc(tmpBMIH->biSize+PaletteSize(tmpBMIH)+imagesize);
      memcpy(in->lpBMIH,tmpBMIH,tmpBMIH->biSize+PaletteSize(tmpBMIH));
      memcpy(((char*)in->lpBMIH)+tmpBMIH->biSize+PaletteSize(tmpBMIH),
             in->lpData,imagesize);
      free(in->lpData);
  }
  bool result=false;
  if (in->lpBMIH->biBitCount==16) result=_compress2yuv9_15(in);
  else if (in->lpBMIH->biBitCount==24) result=_compress2yuv9_24(in);
  else if (in->lpBMIH->biBitCount==8) result=_compress2yuv9_8(in);
  else if (in->lpBMIH->biBitCount==32) result=_compress2yuv9_32(in);

  if (result)
  {
      in->lpBMIH->biCompression=BI_YUV9;
      in->lpBMIH->biBitCount=9;
      in->lpBMIH->biClrUsed=0;
  }
  return (result);
}
