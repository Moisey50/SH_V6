//  $File : cut.h - video processing primitives - cuting part of the image
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef _IPTOOLS_CUT_INC
#define _IPTOOLS_CUT_INC

#include <imageproc\rectangles.h>

__forceinline pTVFrame _cut_rect8(pTVFrame frame,RECT& rc)
{
    if (rc.left < 0) rc.left = 0; if (rc.top < 0) rc.top = 0;
    if ((rc.left>frame->lpBMIH->biWidth) || (rc.right>frame->lpBMIH->biWidth)) return NULL;
    if ((rc.top>frame->lpBMIH->biHeight) || (rc.bottom>frame->lpBMIH->biHeight)) return NULL;
    _correct_rect4(rc);

    if (rc.right > frame->lpBMIH->biWidth) rc.right = frame->lpBMIH->biWidth;
	if (rc.bottom > frame->lpBMIH->biHeight) rc.bottom = frame->lpBMIH->biHeight;
    if (rc.right < rc.left) return NULL;
    if (rc.bottom < rc.top) return NULL;

	DWORD width = rc.right - rc.left, height = rc.bottom - rc.top, size = width * height;

	if (size <= 0) return NULL;
	
    int dsize=0;

    if (frame->lpBMIH->biCompression==BI_YUV9)
        dsize=(9*size)/8;
    else if (frame->lpBMIH->biCompression==BI_Y8)
        dsize=size;
	LPBITMAPINFOHEADER lpbm=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER) + dsize);
    
    if (!lpbm) return NULL;

	memcpy(lpbm, frame->lpBMIH, sizeof(BITMAPINFOHEADER));
	lpbm->biWidth = width;
	lpbm->biHeight = height;
	lpbm->biSizeImage = dsize;
	
    LPBYTE offsrc = GetData(frame) + rc.top * frame->lpBMIH->biWidth + rc.left;
	LPBYTE offmax = offsrc + (height - 1) * frame->lpBMIH->biWidth + rc.right;
	LPBYTE offdst = (LPBYTE)lpbm + sizeof(BITMAPINFOHEADER);
	while (offsrc < offmax)
	{
		memcpy(offdst, offsrc, width);
		offsrc += frame->lpBMIH->biWidth;
		offdst += width;
	}
    if (frame->lpBMIH->biCompression==BI_YUV9)
    {
	    offsrc = GetData(frame) + frame->lpBMIH->biWidth * frame->lpBMIH->biHeight + (rc.top / 4) * (frame->lpBMIH->biWidth / 4) + rc.left / 4;
	    offmax = offsrc + ((height - 1) / 4) * (frame->lpBMIH->biWidth / 4) + rc.right / 4;
	    offdst = (LPBYTE)lpbm + sizeof(BITMAPINFOHEADER) + size;
	    DWORD offUVdst = size / 16, offUVsrc = frame->lpBMIH->biWidth * frame->lpBMIH->biHeight / 16;
	    width /= 4;
	    while (offsrc < offmax)
	    {
		    memcpy(offdst, offsrc, width);
		    memcpy(offdst + offUVdst, offsrc + offUVsrc, width);
		    offsrc += frame->lpBMIH->biWidth / 4;
		    offdst += width;
	    }
    }
	return newTVFrame(lpbm,NULL);
}

__forceinline pTVFrame _cut_rect16(pTVFrame frame,RECT& rc)
{
    if (rc.left < 0) rc.left = 0; if (rc.top < 0) rc.top = 0;
    
    if ((rc.left>frame->lpBMIH->biWidth) || (rc.right>frame->lpBMIH->biWidth)) return NULL;
    if ((rc.top>frame->lpBMIH->biHeight) || (rc.bottom>frame->lpBMIH->biHeight)) return NULL;
    
    _correct_rect4(rc);

    if (rc.right > frame->lpBMIH->biWidth) rc.right = frame->lpBMIH->biWidth;
	if (rc.bottom > frame->lpBMIH->biHeight) rc.bottom = frame->lpBMIH->biHeight;
    if (rc.right < rc.left) return NULL;
    if (rc.bottom < rc.top) return NULL;

	DWORD width = rc.right - rc.left, height = rc.bottom - rc.top, size = width * height;

	if (size <= 0) return NULL;
	LPBITMAPINFOHEADER lpbm = (LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER) + size * sizeof(WORD));
	
    if (!lpbm) return NULL;
	
    memcpy(lpbm, frame->lpBMIH, sizeof(BITMAPINFOHEADER));
	lpbm->biWidth = width;
	lpbm->biHeight = height;
	lpbm->biSizeImage = size * sizeof(WORD);
	LPWORD offsrc = ((LPWORD)GetData(frame)) + rc.top * frame->lpBMIH->biWidth + rc.left;
	LPWORD offmax = offsrc + (height - 1) * frame->lpBMIH->biWidth + rc.right;
	LPWORD offdst = (LPWORD)((LPBYTE)lpbm + sizeof(BITMAPINFOHEADER));
	while (offsrc < offmax)
	{
		memcpy(offdst, offsrc, width*sizeof(WORD));
		offsrc += frame->lpBMIH->biWidth;
		offdst += width;
	}

	return newTVFrame(lpbm,NULL);
}


__forceinline pTVFrame _cut_rect(pTVFrame frame,RECT& rc)
{
    switch(frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
    case BI_Y8:
        return _cut_rect8(frame,rc);
    case BI_Y16:
        return _cut_rect16(frame,rc);
    }
    return NULL;
}


#endif