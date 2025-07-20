#ifndef DEINTERLACE_INC
#define DEINTERLACE_INC

#include <imageproc\simpleip.h>
#include <video\iv.h>

__forceinline void _deinterlace_TVData(pTVData data)
{
    pTVFrame newframe=(pTVFrame)malloc(sizeof(TVFrame));
    newframe->lpData=NULL;
    DWORD size=data->frame->lpBMIH->biSize+data->frame->lpBMIH->biSizeImage/2;
    newframe->lpBMIH=(LPBITMAPINFOHEADER)malloc(size);
    memcpy(newframe->lpBMIH,data->frame->lpBMIH,data->frame->lpBMIH->biSize);
    newframe->lpBMIH->biSizeImage/=2;
    newframe->lpBMIH->biHeight/=2;
    LPBYTE vdata=((LPBYTE)newframe->lpBMIH)+newframe->lpBMIH->biSize;
    memset(vdata,128,newframe->lpBMIH->biSizeImage);
    LPBYTE idata=GetData(data->frame);
    int i;
    for (i=0; i<newframe->lpBMIH->biHeight; i++)
    {
        memcpy(vdata,idata,newframe->lpBMIH->biWidth);
        vdata+=newframe->lpBMIH->biWidth;
        idata+=2*newframe->lpBMIH->biWidth;
    }

    vdata=((LPBYTE)newframe->lpBMIH)+newframe->lpBMIH->biSize+newframe->lpBMIH->biHeight*newframe->lpBMIH->biWidth;
    idata=GetData(data->frame)+newframe->lpBMIH->biHeight*newframe->lpBMIH->biWidth*2;
    for (i=0; i<newframe->lpBMIH->biHeight/2; i++)
    {
        memcpy(vdata,idata,newframe->lpBMIH->biWidth/4);
        vdata+=newframe->lpBMIH->biWidth/4;
        idata+=newframe->lpBMIH->biWidth/2;
    }
    data->frame=newframe;
    data->frame->lpBMIH->biYPelsPerMeter*=2;
}

__forceinline void	_deinterlace(pTVFrame frame, bool even=TRUE)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

	int width=frame->lpBMIH->biWidth;
	int height=frame->lpBMIH->biHeight;
	LPBYTE offset=GetData(frame)+((even)?0:width);
	for (int y=(even)?0:1; y<height-(even)?1:0; y+=2)
	{
		if (even)
			memcpy(offset+width,offset,width);
		else
			memcpy(offset-width,offset,width);
		offset+=width*2;
	}
}

#endif
