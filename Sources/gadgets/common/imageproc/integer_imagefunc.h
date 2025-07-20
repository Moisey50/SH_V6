#ifndef INTEGERIMAGEFUNC_INC
#define INTEGERIMAGEFUNC_INC

typedef int idata;
typedef idata* LPIDATA;

inline void _lpass(int width, int height, LPIDATA src, LPIDATA dst, int __fr_nmb=CONOVOLUTION_PARAM)
{
    int size=height*width;
    int i=0; double accum=128; BYTE temp;
    double rate=1000.0/(1000 - __fr_nmb)-1.0;
    double fading =(double)__fr_nmb/1000.0;
    while (i<size)
    {
        accum=fading*accum;
        *(dst+i)=(BYTE)(accum/rate+0.5);
        accum+=*(src+i);
        i++;
    }
    i--;
    LPIDATA scn=dst+i;

    while (i>=0)
    {
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*scn;
        *scn=temp;
        i--; scn--;
    }
    DWORD off; i++;
    while (i<size)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i++;
    }
    i--;
    while (i>=0)
    {
        off=width*(i%height)+i/height;
        accum=fading*accum;
        temp=(BYTE)(accum/rate+0.5);
        accum+=*(dst+off);
        *(dst+off)=temp;
        i--;
    }; 
    *dst=*(dst+width+1);
    *(dst+1)=*(dst+width+2);
    *(dst+width)=*(dst+width+1);
    *(dst+2*width)=*(dst+2*width+1);
}

inline void _and(LPIDATA U, LPBYTE V, int Size)
{
    int* pU=U;
    LPBYTE pV=V;
    int* eod=pU+Size;
    while (pU<eod)
    {
        *pU=(abs(*pU)<abs(*pV *4))?*pU:*pV*4;
        pU++; pV++;
    }

}

#endif