#ifndef DRAW_OVER_INC
#define DRAW_OVER_INC
#include <video\yuv411.h>
#include <imageproc\rectangles.h>
#include <helpers\16bit.h>

__forceinline pTVFrame _draw_over(pTVFrame bgFrame, pTVFrame fgFrame, int x, int y)
{
	pTVFrame Copy=NULL;
    if ((bgFrame->lpBMIH->biCompression==BI_YUV9) || (bgFrame->lpBMIH->biCompression==BI_Y8))
        Copy = makecopyTVFrame(bgFrame);
    else if (bgFrame->lpBMIH->biCompression==BI_Y16)
    {
        Copy = (pTVFrame)malloc(sizeof(TVFrame));
        Copy->lpBMIH=y16yuv8(bgFrame->lpBMIH,bgFrame->lpData);
        Copy->lpData=NULL;
    }
	RECT rc = { x, y, x + fgFrame->lpBMIH->biWidth, y + fgFrame->lpBMIH->biHeight };
	_correct_rect4(rc);
	x = rc.left;
	y = rc.top;
	if (x >= Copy->lpBMIH->biWidth || y >= Copy->lpBMIH->biHeight)
		return Copy;
	int width = rc.right - rc.left;//fgFrame->lpBMIH->biWidth;
	if (width > Copy->lpBMIH->biWidth - x)
		width = Copy->lpBMIH->biWidth - x;
	int height = rc.bottom - rc.top;//fgFrame->lpBMIH->biHeight;
	if (height > Copy->lpBMIH->biHeight - y)
		height = Copy->lpBMIH->biHeight - y;
    switch (fgFrame->lpBMIH->biCompression)
    {
    case BI_YUV9:
        {
	        LPBYTE Src = GetData(fgFrame);
	        LPBYTE Dst = GetData(Copy) + y * Copy->lpBMIH->biWidth + x;
	        LPBYTE End = Dst + height * Copy->lpBMIH->biWidth;
	        while (Dst < End)
	        {
		        memcpy(Dst, Src, width);
		        Dst += Copy->lpBMIH->biWidth;
		        Src += fgFrame->lpBMIH->biWidth;
	        }
	        width /= 4;
	        height /= 4;
	        Src = GetData(fgFrame) + fgFrame->lpBMIH->biWidth * fgFrame->lpBMIH->biHeight;
	        Dst = GetData(Copy) + Copy->lpBMIH->biWidth * Copy->lpBMIH->biHeight + (y / 4) * (Copy->lpBMIH->biWidth / 4) + x / 4;
	        End = Dst + height * (Copy->lpBMIH->biWidth / 4);
	        while (Dst < End)
	        {
		        memcpy(Dst, Src, width);
		        Dst += Copy->lpBMIH->biWidth / 4;
		        Src += fgFrame->lpBMIH->biWidth / 4;
	        }
            break;
        }
    case BI_Y8:
        break;
    case BI_Y16:
        {
	        LPWORD Src = (LPWORD)GetData(fgFrame);
	        LPBYTE Dst = GetData(Copy) + y * Copy->lpBMIH->biWidth + x;
	        LPBYTE End = Dst + height * Copy->lpBMIH->biWidth;
	        while (Dst < End)
	        {
		        w2bcpy(Dst, Src, width);
		        Dst += Copy->lpBMIH->biWidth;
		        Src += fgFrame->lpBMIH->biWidth;
	        }
            break;
        }
    }
	return Copy;
}

#endif