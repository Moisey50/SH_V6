#ifndef GAMMACORRECTION_INC
#define GAMMACORRECTION_INC

#include <math\hbmath.h>

__forceinline void __log(int width, int depth, unsigned char *pData)
{
	if (!pData) return;
	int offset=0,i;
	for (i=0; i<depth; i++)
	{
		for (int j=0; j<width; j++)
		{
			pData[offset+j]=(pData[offset+j])?(int)(105.0*log10(pData[offset+j])):0;
		}
		offset+=width;
	}    
}

__forceinline void __alog(int width, int depth, LPBYTE  pData)
{
	if (!pData) return;
	int offset=0,i;
	for (i=0; i<depth; i++)
	{
		for (int j=0; j<width; j++)
		{
			pData[offset+j]=(int)(pow(10.0, 0.0094*pData[offset+j]));
		}
		offset+=width;
	}    
}

__forceinline void __gamma8(double gamma, int width, int height, LPBYTE  pData)
{
    double norma=pow(255.0,1.0-gamma);
    LPBYTE eod=pData+width*height;
    while (pData<eod)
    {
        *pData=(unsigned char)(norma*pow(*pData,gamma));
        pData++;
    }
}

__forceinline void __gamma16(double gamma, int width, int height, LPWORD pData)
{
    double norma=pow(65535.0,1.0-gamma);
    LPWORD eod=pData+width*height;
    while (pData<eod)
    {
        *pData=(WORD)(norma*pow(*pData,gamma));
        pData++;
    }
}

__forceinline void _log(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    LPBYTE pData;
    pData=GetData(frame);

    int width=frame->lpBMIH->biWidth, height=frame->lpBMIH->biHeight;
    __log(width,height,pData);
}

__forceinline void _alog(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    LPBYTE pData;
    pData=GetData(frame);

    int width=frame->lpBMIH->biWidth, height=frame->lpBMIH->biHeight;
    __alog(width,height,pData);
}

__forceinline void _gamma(pTVFrame frame,double gamma)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
        {
            LPBYTE pData;
            pData=GetData(frame);

            int width=frame->lpBMIH->biWidth, height=frame->lpBMIH->biHeight;
            __gamma8(gamma,width,height,pData);
            break;
        }
    case BI_Y16:
        {
            LPWORD pData;
            pData=(LPWORD)GetData(frame);

            int width=frame->lpBMIH->biWidth, height=frame->lpBMIH->biHeight;
            __gamma16(gamma,width,height,pData);
            break;
        }
    }
}


#endif