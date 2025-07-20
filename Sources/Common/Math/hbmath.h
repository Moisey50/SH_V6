#ifndef HB_MATH
#define HB_MATH

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846 	//Pi
#define M_E 2.7182818284590452354 	//Euler number
#define M_LOG2E 1.4426950408889634074 	//log_2 e 
#define M_LOG10E 0.43429448190325182765 //lg e
#define M_LN2 0.69314718055994530942 	//ln 2 
#define M_LN10 2.30258509299404568402 	//ln 10 
#define M_PI_2 1.57079632679489661923 	//Pi/2 
#define M_PI_4 0.78539816339744830962 	//Pi/4 
#define M_1_PI 0.31830988618379067154 	//1/Pi 
#define M_2_PI 0.63661977236758134308 	//2/Pi 
#define M_SQRTPI 1.77245385090551602729 //sqrt(Pi) [4.0.2] 
#define M_2_SQRTPI 1.12837916709551257390 	//2/sqrt(Pi) 
#define M_SQRT2 1.41421356237309504880 	//sqrt(2) 
#define M_SQRT3 1.73205080756887729352 	//sqrt(3) [4.0.2] 
#define M_SQRT1_2 0.70710678118654752440 	//1/sqrt(2) 
#endif
#define M_LNPI 1.14472988584940017414 	//ln Pi [4.0.2] 
#define M_EULER 0.57721566490153286061 	//Euler  constant [4.0.2] 

__forceinline double sqrt(int i)   { return sqrt((double)i);}
__forceinline double sqrt(DWORD i) { return sqrt((double)i);}
__forceinline double log10(int i)  { return log10((double)i);}
__forceinline double pow(int i,double p) { return pow((double)i,p);}

#ifndef USE_FXFC_Strings
    #pragma message("!!! Warning! Old version of " __FILE__ " included (without FXFC)")
    #define FXString CString
#endif

__forceinline bool  IsDigit(LPCTSTR s)
{	
    bool retV=true;
    for(size_t i=0; i<_tcslen(s); i++)
    {
         retV&=(isdigit(s[i])!=0);
    }
    return retV;
}

#define _ROUND(a) ((int)(a+0.5))

#endif
