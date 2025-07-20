#ifndef _RANDDISTORT_INC
#define _RANDDISTORT_INC

//#define _CRT_RAND_S
#include <stdlib.h>

__forceinline void _shuffle(int ImageSize, int PixelSize, LPBYTE Src, int nTimes)
{
#ifndef _CRT_RAND_S
	srand((unsigned)time(NULL));
#endif
	LPBYTE Copy = (LPBYTE)calloc(ImageSize, PixelSize);
	while (nTimes-- > 0)
	{
		memcpy(Copy, Src, ImageSize * PixelSize);
#ifdef _CRT_RAND_S
		unsigned int rnd;
		rand_s(&rnd);
		int offset = (int)(rnd % (ImageSize - 2)) + 1;
#else
		int offset = (rand() * rand()) % (ImageSize - 2) + 1;
#endif
		memcpy(Src, Copy + offset * PixelSize, (ImageSize - offset) * PixelSize);
		memcpy(Src + (ImageSize - offset) * PixelSize, Copy, offset * PixelSize);
	}
	free(Copy);
}

__forceinline pTVFrame _shuffle(const pTVFrame frame, int nTimes)
{
	ASSERT(frame);
	ASSERT(frame->lpBMIH);
	pTVFrame dstframe = makecopyTVFrame(frame);

	int ImageSize = frame->lpBMIH->biWidth * frame->lpBMIH->biHeight;
	int PixelSize = (frame->lpBMIH->biCompression == BI_Y16) ? 2 : 1;

	_shuffle(ImageSize, PixelSize, GetData(dstframe), nTimes);
	return dstframe;
}


#endif