#ifndef DEINTERLACE_INC
#define DEINTERLACE_INC

#include <video\tvframe.h>
#include <imageproc\simpleip.h>

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
