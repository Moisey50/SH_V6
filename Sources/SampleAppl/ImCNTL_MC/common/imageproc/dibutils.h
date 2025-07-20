//  $File : dibutils.h - utilites for calculation some sizes of data in bitmaps
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef _DIBUTILS_INC
#define _DIBUTILS_INC

#include <video\stdcodec.h>

#define BMAP_HEADER_MARKER    ((WORD) ('M' << 8) | 'B')
#define JPEG_HEADER_MARKER    ((WORD) (0xD8 << 8) | 0xFF)
#define TIFF_HEADER_MARKER    ((WORD) (0x49 << 8) | 0x49)
#define ISWIN30DIB(BMSTRUCT)  ((*(LPDWORD)(BMSTRUCT)) == sizeof(BITMAPINFOHEADER))

__forceinline void	_rectvalid(RECT* rc)
{
	if (rc->left>rc->right)
	{
		int temp=rc->left;
		rc->left=rc->right;
		rc->right=temp;
	}
	if (rc->top>rc->bottom)
	{
		int temp=rc->top;
		rc->top=rc->bottom;
		rc->bottom=temp;
	}
}

__forceinline int NumColors(BITMAPINFOHEADER *lpBMIH)
{
	ASSERT(lpBMIH);

    if (lpBMIH->biCompression!=BI_RGB) return(0);

	WORD BitCount;  // DIB bit count
	if (ISWIN30DIB(lpBMIH))
	{
		DWORD ClrUsed;

		ClrUsed = lpBMIH->biClrUsed;
		if (ClrUsed != 0) return (WORD)ClrUsed;
	}
	if (ISWIN30DIB(lpBMIH))
		BitCount = lpBMIH->biBitCount;
	else
		BitCount = ((BITMAPCOREHEADER*)lpBMIH)->bcBitCount;
	switch (BitCount)
	{
		case 1: return 2;
		case 4: return 16;
        case 8: return 256;
		default:
			return 0;
	}
}

__forceinline int PaletteSize(BITMAPINFOHEADER *lpBMIH) // Used just for file size calc...
{
   if (!lpBMIH) return(0);
   if (lpBMIH->biCompression!=BI_RGB) return(0); // No palette
   if (ISWIN30DIB (lpBMIH))
	  return (WORD)(NumColors(lpBMIH) * sizeof(RGBQUAD));
   else
	  return (WORD)(NumColors(lpBMIH) * sizeof(RGBTRIPLE));

}

__forceinline int RowSize(BITMAPINFOHEADER *lpBMIH) // Used just for file size calc...
{
    int row_width;
    if (lpBMIH->biBitCount== 24)
        row_width = lpBMIH->biWidth * 3;
    else if (lpBMIH->biBitCount== 16)
        row_width = lpBMIH->biWidth * 2;
    else if (lpBMIH->biBitCount== 8)
        row_width = lpBMIH->biWidth ;
    else
        row_width = (lpBMIH->biWidth*lpBMIH->biBitCount) /8 +(((lpBMIH->biWidth*lpBMIH->biBitCount)  %8)?1:0);
    while ((row_width & 3) != 0) row_width++;
    return(row_width);
}

__forceinline DWORD ImageSize(BITMAPINFOHEADER *lpBMIH)
{
    ASSERT((lpBMIH->biCompression==BI_YUV9) || (lpBMIH->biCompression==BI_Y8));
    return (lpBMIH->biSizeImage);
}


#endif //_DIBUTILS_INC
