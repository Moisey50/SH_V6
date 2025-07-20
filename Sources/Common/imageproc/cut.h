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

__forceinline pTVFrame _cut_rect8(const pTVFrame frame,RECT& rc)
{
    if (rc.left < 0) rc.left = 0; if (rc.top < 0) rc.top = 0;
    if ((rc.left>frame->lpBMIH->biWidth) || (rc.right>frame->lpBMIH->biWidth)) return NULL;
    if ((rc.top>frame->lpBMIH->biHeight) || (rc.bottom>frame->lpBMIH->biHeight)) return NULL;
    
    _correct_rect4(rc);

    if (rc.right > frame->lpBMIH->biWidth) rc.right = frame->lpBMIH->biWidth;
	if (rc.bottom > frame->lpBMIH->biHeight) rc.bottom = frame->lpBMIH->biHeight;
    if (rc.right < rc.left) return NULL;
    if (rc.bottom < rc.top) return NULL;

	DWORD width = rc.right - rc.left;
    DWORD height = rc.bottom - rc.top;
    DWORD size = width * height;

	if (size <= 0) return NULL;
	
    int dsize=0;

    if (frame->lpBMIH->biCompression==BI_YUV9)
        dsize=(9*size)/8;
    else     if (frame->lpBMIH->biCompression==BI_YUV12)
        dsize=(12*size)/8;
    else if (frame->lpBMIH->biCompression==BI_Y8 || frame->lpBMIH->biCompression==BI_Y800)
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
    else if (frame->lpBMIH->biCompression==BI_YUV12)
    {
	    offsrc = GetData(frame) + frame->lpBMIH->biWidth * frame->lpBMIH->biHeight + (rc.top / 2) * (frame->lpBMIH->biWidth / 2) + rc.left / 2;
	    offmax = offsrc + ((height - 1) / 2) * (frame->lpBMIH->biWidth / 2) + rc.right / 2;
	    offdst = (LPBYTE)lpbm + sizeof(BITMAPINFOHEADER) + size;
	    DWORD offUVdst = size / 4, offUVsrc = frame->lpBMIH->biWidth * frame->lpBMIH->biHeight / 4;
	    width /= 2;
	    while (offsrc < offmax)
	    {
		    memcpy(offdst, offsrc, width);
		    memcpy(offdst + offUVdst, offsrc + offUVsrc, width);
		    offsrc += frame->lpBMIH->biWidth / 2;
		    offdst += width;
	    }
    }
	return newTVFrame(lpbm,NULL);
}

__forceinline pTVFrame _cut_rect16(const pTVFrame frame,RECT& rc)
{
    if (rc.left < 0) rc.left = 0; if (rc.top < 0) rc.top = 0;
    
    if ((rc.left>frame->lpBMIH->biWidth) || (rc.right>frame->lpBMIH->biWidth)) return NULL;
    if ((rc.top>frame->lpBMIH->biHeight) || (rc.bottom>frame->lpBMIH->biHeight)) return NULL;
    
    _correct_rect4(rc);

    if (rc.right > frame->lpBMIH->biWidth) rc.right = frame->lpBMIH->biWidth;
	if (rc.bottom > frame->lpBMIH->biHeight) rc.bottom = frame->lpBMIH->biHeight;
    if (rc.right < rc.left) return NULL;
    if (rc.bottom < rc.top) return NULL;

	DWORD width = rc.right - rc.left;
    DWORD height = rc.bottom - rc.top;
    DWORD size = width * height;

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

__forceinline pTVFrame _cut_rectRGB( const pTVFrame frame , RECT& rc )
{
  if ( rc.left < 0 ) 
    rc.left = 0; 
  if ( rc.top < 0 ) 
    rc.top = 0;

  LPBITMAPINFOHEADER lpBMIH = frame->lpBMIH ;
  if ( (lpBMIH->biBitCount % 8) != 0 )
    return NULL ;
  int iNBytes = lpBMIH->biBitCount / 8 ;
  if ( (rc.left > lpBMIH->biWidth) || (rc.top > lpBMIH->biHeight)  ) 
    return NULL;

  _correct_rect4( rc );

  if ( rc.right > lpBMIH->biWidth ) 
    rc.right = lpBMIH->biWidth;
  if ( rc.bottom > lpBMIH->biHeight ) 
    rc.bottom = lpBMIH->biHeight;
  DWORD width = rc.right - rc.left;
  DWORD height = rc.bottom - rc.top;
  if ( width <= 0 )
    return NULL;
  if ( height <= 0 ) 
    return NULL;

  DWORD size = width * height;

  LPBITMAPINFOHEADER lpbm = (LPBITMAPINFOHEADER) malloc( sizeof( BITMAPINFOHEADER ) + size * iNBytes );

  if ( !lpbm ) 
    return NULL;

  memcpy( lpbm , frame->lpBMIH , sizeof( BITMAPINFOHEADER ) );
  lpbm->biCompression = BI_RGB ;
  lpbm->biWidth = width;
  lpbm->biHeight = height;
  lpbm->biSizeImage = size * iNBytes ;
  int iInterleaveSrc = lpBMIH->biWidth * iNBytes ;
  int iInterleaveDst = width * iNBytes ;
  LPBYTE offsrc = GetData( frame ) + rc.top * lpBMIH->biWidth + rc.left;
  LPBYTE offmax = offsrc + height * iInterleaveSrc + rc.right;
  LPBYTE offdst = (LPBYTE)(lpbm + 1) ;
  while ( offsrc < offmax )
  {
    memcpy( offdst , offsrc , iInterleaveDst );
    offsrc += iInterleaveSrc ;
    offdst += iInterleaveDst ;
  }

  return newTVFrame( lpbm , NULL );
}

__forceinline pTVFrame _cut_rect8_1(const pTVFrame frame,RECT& rc)
{
  if (rc.left < 0) 
    rc.left = 0; 
  if (rc.top < 0) 
    rc.top = 0;
  if ( (rc.left > frame->lpBMIH->biWidth) 
    || (rc.right > frame->lpBMIH->biWidth)
    || (rc.top > frame->lpBMIH->biHeight) 
    || (rc.bottom>frame->lpBMIH->biHeight) ) return NULL;

  if (rc.right > frame->lpBMIH->biWidth) rc.right = frame->lpBMIH->biWidth;
  if (rc.bottom > frame->lpBMIH->biHeight) rc.bottom = frame->lpBMIH->biHeight;
  if (rc.right < rc.left) return NULL;
  if (rc.bottom < rc.top) return NULL;

  LONG width = rc.right - rc.left; 
  //width=((width>>2)<<2)+((width%4)?4:0);
  //if (rc.left+width>frame->lpBMIH->biWidth) rc.left=frame->lpBMIH->biWidth-width;
  //if (rc.left<0) { width-=4; rc.left+=4; }
  DWORD height = rc.bottom - rc.top;
  DWORD size = width * height;
  //ASSERT((width%4)==0);

  if (size <= 0) 
    return NULL;

  int dsize = size ;

  LPBITMAPINFOHEADER lpbm=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER) + dsize);

  if (!lpbm) return NULL;

  memcpy(lpbm, frame->lpBMIH, sizeof(BITMAPINFOHEADER));
  lpbm->biWidth = width;
  lpbm->biHeight = height;
  lpbm->biSizeImage = dsize;
  lpbm->biCompression = BI_Y800 ;

  LPBYTE offsrc = GetData(frame) + rc.top * frame->lpBMIH->biWidth + rc.left;
  LPBYTE offmax = offsrc + (height - 1) * frame->lpBMIH->biWidth + rc.right;
  LPBYTE offdst = (LPBYTE)lpbm + sizeof(BITMAPINFOHEADER);
  while (offsrc < offmax)
  {
    memcpy(offdst, offsrc, width);
    offsrc += frame->lpBMIH->biWidth;
    offdst += width;
  }
  return newTVFrame(lpbm,NULL);
}

__forceinline pTVFrame _cut_rect16_1(const pTVFrame frame,RECT& rc)
{
  if (rc.left < 0) 
    rc.left = 0;
  if (rc.top < 0) 
    rc.top = 0;

  if ( (rc.left > frame->lpBMIH->biWidth) 
    || (rc.right > frame->lpBMIH->biWidth)
    || (rc.top > frame->lpBMIH->biHeight) 
    || (rc.bottom > frame->lpBMIH->biHeight) )
    return NULL;

  if (rc.right > frame->lpBMIH->biWidth) 
    rc.right = frame->lpBMIH->biWidth;
  if (rc.bottom > frame->lpBMIH->biHeight) 
    rc.bottom = frame->lpBMIH->biHeight;
  if (rc.right < rc.left) 
    return NULL;
  if (rc.bottom < rc.top) 
    return NULL;

  LONG width = rc.right - rc.left;
//  width=((width>>2)<<2)+((width%4)?4:0);
  if (rc.left+width>frame->lpBMIH->biWidth) rc.left=frame->lpBMIH->biWidth-width;
  if (rc.left<0) { width-=4; rc.left+=4; }
  DWORD height = rc.bottom - rc.top;
  DWORD size = width * height;

  if (size <= 0) 
    return NULL;
  LPBITMAPINFOHEADER lpbm = 
    (LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER) + size * sizeof(WORD));

  if (!lpbm) 
    return NULL;

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


__forceinline pTVFrame _cut_rect(const pTVFrame frame,RECT& rc , BOOL Step4 = TRUE )
{
    switch(frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
      return ( Step4 ) ? _cut_rect8(frame,rc) : _cut_rect8_1(frame,rc) ;
    case BI_Y16:
      return ( Step4 ) ? _cut_rect16(frame,rc) : _cut_rect16_1(frame,rc) ;
    case BI_RGB: return _cut_rectRGB( frame , rc ) ;
    }
    return NULL;
}


#endif