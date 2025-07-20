#ifndef _16BITHELPERS_INC
#define _16BITHELPERS_INC

// 16 bit helpers

__forceinline void memsetW(LPWORD dst, DWORD w, int size) //16 bit analog of memset
{
    LPWORD eod=dst+size;
    while (dst<eod)
    {
        *dst=(WORD)w; dst++;
    }
}

__forceinline void w2bcpy(LPBYTE dst, LPWORD src, DWORD size) // analog memcpy, which copy 16 bit to byte
{
    LPBYTE eod=dst+size;
    while (dst<eod)
    {
        *dst=(BYTE)((*src)>>8);
        dst++; src++;
    }
}

__forceinline void memicpy16(LPWORD dest, const LPWORD src, size_t count)
{
    LPWORD eod=dest+count;
    LPWORD sc=src+count;
    while (dest<eod)
    {
        *dest=*sc; dest++; sc--;
    }
}

#endif