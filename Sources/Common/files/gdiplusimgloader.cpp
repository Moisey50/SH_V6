#include "stdafx.h"
#include "gdiplusimgloader.h"
#include <comdef.h>
#include <gdiplus.h>

#pragma comment( lib, "GdiPlus.lib" )

using namespace Gdiplus;

inline LPBITMAPINFOHEADER HBITMAP2LPBMIH(HBITMAP hBmp, DWORD &dSize)
{
    BITMAP bmp;
    WORD    cClrBits;
    DWORD   ClrUsed=0, PaletteSize=0;
    dSize=0;
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
        return NULL;
    
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 
    int iSize=(((bmp.bmWidth * cClrBits +31)&~31)/8)* bmp.bmHeight;
    if (cClrBits < 24) 
    {
        ClrUsed = (1<<cClrBits); 
        PaletteSize = ClrUsed * sizeof (RGBQUAD); 
    }
    LPBITMAPINFOHEADER lpBMIH= (LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+PaletteSize+iSize);
    if (lpBMIH)
    {
        memset(lpBMIH,0,sizeof(BITMAPINFOHEADER));
        dSize=sizeof(BITMAPINFOHEADER)+PaletteSize+iSize;
        LPBYTE data=(LPBYTE)lpBMIH+sizeof(BITMAPINFOHEADER);
        lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
        lpBMIH->biWidth = bmp.bmWidth; 
        lpBMIH->biHeight = bmp.bmHeight; 
        lpBMIH->biPlanes = bmp.bmPlanes; 
        lpBMIH->biBitCount = bmp.bmBitsPixel; 
        lpBMIH->biClrUsed = ClrUsed;
        lpBMIH->biCompression = BI_RGB;
        lpBMIH->biSizeImage=0;
        lpBMIH->biClrImportant = 0;
        memcpy(data,bmp.bmBits,PaletteSize+iSize);
    }
    return lpBMIH;
}


LPBITMAPINFOHEADER LoadImg(LPCTSTR fName)
{
    HBITMAP hbmReturn=NULL;
    LPBITMAPINFOHEADER retV=NULL;
    DWORD dSize=0;
    ULONG_PTR                    gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    {    
        _bstr_t fnName=_variant_t(fName);
        Bitmap gbm(fnName);
        Status sts=gbm.GetHBITMAP(Gdiplus::Color(), &hbmReturn);
        if (sts==Ok)
        {
            retV=HBITMAP2LPBMIH(hbmReturn, dSize);
        }
        DeleteObject(hbmReturn);
    }
    GdiplusShutdown(gdiplusToken);
	return retV;
}