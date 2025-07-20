#ifndef MEANDER_INC
#define MEANDER_INC

#include <imageproc\basedef.h>
#include <imageproc\simpleip.h>

__forceinline void _hmeander8(LPBYTE Data, int imgsize, int qsize)
{
	if (imgsize < 4 * qsize)
		return;
	int* val = (int*)calloc(imgsize, sizeof(int));
	int* dst = val + 2 * qsize;
	int i;
	for (i = 0; i < qsize; i++)
		*dst += (int)Data[i];
	for (; i < 3 * qsize; i++)
		*dst -= (int)Data[i];
	for (; i < 4 * qsize; i++)
		*dst += (int)Data[i];
	int min = *dst, max = *dst;
	int* pMin = dst;
	int* pMax = dst;
	dst++;
	LPBYTE src = Data, end = Data + imgsize - 4 * qsize;
	while (src < end)
	{
		*dst = *(dst - 1);
		(*dst) -= (int)(*src);
		(*dst) += (int)(*(src + 4 * qsize));
		(*dst) += 2 * (int)(*(src + qsize));
		(*dst) -= 2 * (int)(*(src + 3 * qsize));
		if (min > *dst)
		{
			min = *dst;
			pMin = dst;
		}
		else if (max < *dst)
		{
			max = *dst;
			pMax = dst;
		}
		src++;
		dst++;
	}
	TRACE("-__--__--__--__--__--__--__--__- min = %d; max = %d\n", min, max);
	double rate = (max > min) ? 255. / (double)(max - min) : 0;
	src = Data;
	end = Data + imgsize;
	dst = val;
	while (src < end)
	{
		if (*dst < min)
			*src = 0;
		else if (*dst > max)
			*src = 255;
		else
			*src = (BYTE)((double)(*dst - min) * rate);
		src++;
		dst++;
	}
	free(val);
}

__forceinline void _hmeander8slow(LPBYTE Data, int imgsize, int qsize)
{
	if (imgsize < 4 * qsize)
		return;
	int* val = (int*)calloc(imgsize, sizeof(int));
	int* dst = val + 2 * qsize;
	int i, min, max;
	LPBYTE src = Data, end = Data + imgsize - 4 * qsize;
	while (src < end)
	{
		for (i = 0; i < qsize; i++)
			*dst += (int)src[i];
		for (; i < 3 * qsize; i++)
			*dst -= (int)src[i];
		for (; i < 4 * qsize; i++)
			*dst += (int)src[i];
		if (src == Data)
		{
			min = *dst;
			max = *dst;
		}
		else if (min > *dst)
			min = *dst;
		else if (max < *dst)
			max = *dst;
		src++;
		dst++;
	}
	double rate = (max > min) ? 255. / (double)(max - min) : 0;
	src = Data;
	end = Data + imgsize;
	dst = val;
	while (src < end)
	{
		if (*dst < min)
			*src = 0;
		else if (*dst > max)
			*src = 255;
		else
			*src = (BYTE)((double)(*dst - min) * rate);
		src++;
		dst++;
	}
	free(val);
}

__forceinline pTVFrame _hmeander(const pTVFrame Frame, int qsize)
{
	pTVFrame Copy = makecopyTVFrame(Frame);
	int width = Copy->lpBMIH->biWidth;
	int height = Copy->lpBMIH->biHeight;
	LPBYTE Data = GetData(Copy);
	if (Copy->lpBMIH->biCompression == BI_YUV9 || Copy->lpBMIH->biCompression == BI_Y8)
	{
		_hmeander8(Data, width * height, qsize);
	}
	return Copy;
}


#endif