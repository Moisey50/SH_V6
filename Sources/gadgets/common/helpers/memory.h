#ifndef memory_inc
#define memory_inc

__forceinline void  memcpy_i( LPBYTE dest, LPBYTE src, size_t count)
{
	void* eod=dest+count;
	src=src+count-1;
	while(dest<eod)
	{
		*dest=*src;
		dest++;
		src--;
	}
}

#endif