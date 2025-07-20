//  $File : FeatureDetector.h - convolution primitives
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef _FEATUREDETECTOR_INC
#define _FEATUREDETECTOR_INC

#include <imageproc\videologic.h>

#define FREQ_APPERTUREX 2
#define FREQ_APPERTUREY 2

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// inline unsigned char* integrate(long width, long depth, int& x, int& y, int& maxV, unsigned char *nb1)
//// proc integrate whole picture with aperture defined in BlurApperture variable
////
#define BlurApperture 14

typedef struct tagSum
{
	DWORD Left,Right;
}vSum,*pSum;

typedef struct tagSum16
{
	__int64 Left,Right;
}vSum16,*pSum16;


const int AvgApperture = BlurApperture*BlurApperture*256;
const int AvgApperture16 = BlurApperture*BlurApperture*65536;

__forceinline int _firstsum(unsigned char *pBits, pSum Sum)
{
   Sum->Left=0; Sum->Right=0;
   int j=-BlurApperture;
   while(j<BlurApperture)
   {
        if (j<0) Sum->Left+=pBits[j]; else Sum->Right+=pBits[j];
		j++;
   }
   return(((Sum->Left)*(Sum->Right))/AvgApperture); 
}

__forceinline int _nextsum(unsigned char *pBits, pSum Sum)
{
   pBits--;
   Sum->Left-=pBits[-BlurApperture];
   Sum->Right-=pBits[0];
   Sum->Left+=pBits[0];
   Sum->Right+=pBits[BlurApperture];
   return(((Sum->Left)*(Sum->Right))/AvgApperture); 
}



__forceinline unsigned char* _integrate(long width, long depth, int& x, int& y, int& maxV, unsigned char *nb1)
{
	int tmpSum;

    unsigned char*nb2 = (unsigned char *) malloc(width*depth);
    memset(nb2,0,width*depth);
	vSum Sum;
	maxV=0;
	int i,j;
	int offset=width*(depth-1);
	for (i=depth-2; i>1; i--)
	{
		tmpSum=_firstsum(&nb1[offset+BlurApperture],&Sum);
		j=BlurApperture;
		while(j<width-BlurApperture-1)
		{
            nb2[offset+j]=tmpSum;
			if (maxV<tmpSum)
			{
				maxV=tmpSum;x=j;y=i;
			}
			tmpSum=_nextsum(&nb1[offset+j+1],&Sum);
			j++;
		}
		offset-=width;
	} 
    return(nb2);
}

__forceinline int _firstsum(LPWORD pBits, pSum Sum)
{
   Sum->Left=0; Sum->Right=0;
   int j=-BlurApperture;
   while(j<BlurApperture)
   {
        if (j<0) Sum->Left+=pBits[j]; else Sum->Right+=pBits[j];
		j++;
   }
   return(((Sum->Left)*(Sum->Right))/AvgApperture); 
}

__forceinline int _nextsum(LPWORD pBits, pSum Sum)
{
   pBits--;
   Sum->Left-=pBits[-BlurApperture];
   Sum->Right-=pBits[0];
   Sum->Left+=pBits[0];
   Sum->Right+=pBits[BlurApperture];
   return(((Sum->Left)*(Sum->Right))/AvgApperture); 
}

__forceinline int _zone(pTVFrame frame, int& x, int &y)
{
    int MaxV=0; 
    x=0, y=0;
    LPBYTE ptr=GetData(frame);
    LPBYTE ptrI=_integrate(frame->lpBMIH->biWidth,frame->lpBMIH->biHeight,x,y,MaxV,ptr);
    memcpy(ptr,ptrI,frame->lpBMIH->biWidth*frame->lpBMIH->biHeight);
    free(ptrI);
    return MaxV;
}

__forceinline __int64 _firstsum16(LPWORD pBits, pSum16 Sum)
{
   Sum->Left=0; Sum->Right=0;
   int j=-BlurApperture;
   while(j<BlurApperture)
   {
        if (j<0) Sum->Left+=pBits[j]; else Sum->Right+=pBits[j];
		j++;
   }
   return(((Sum->Left)*(Sum->Right))/AvgApperture16); 
}

__forceinline __int64 _nextsum16(LPWORD pBits, pSum16 Sum)
{
   pBits--;
   Sum->Left-=pBits[-BlurApperture];
   Sum->Right-=pBits[0];
   Sum->Left+=pBits[0];
   Sum->Right+=pBits[BlurApperture];
   return(((Sum->Left)*(Sum->Right))/AvgApperture16); 
}

__forceinline LPWORD _integrate16(long width, long depth, int& x, int& y, DWORD& maxV, LPWORD nb1)
{
	DWORD tmpSum;

    LPWORD nb2 = (LPWORD)malloc(width*depth*sizeof(WORD));
    memset(nb2,0,width*depth*sizeof(WORD));
	vSum16 Sum;
	maxV=0;
	int i,j;
	int offset=width*(depth-1);
	for (i=depth-2; i>1; i--)
	{
		tmpSum=(DWORD)_firstsum16(&nb1[offset+BlurApperture],&Sum);
		j=BlurApperture;
		while(j<width-BlurApperture-1)
		{
            nb2[offset+j]=(WORD)tmpSum;
			if (maxV<tmpSum)
			{
				maxV=tmpSum;x=j;y=i;
			}
			tmpSum=(DWORD)_nextsum16(&nb1[offset+j+1],&Sum);
			j++;
		}
		offset-=width;
	} 
    return(nb2);
}


__forceinline int _zone16(pTVFrame frame, int& x, int &y)
{
    DWORD MaxV=0; 
    x=0, y=0;
    LPWORD ptr=(LPWORD)GetData(frame);
    LPWORD  ptrI=_integrate16(frame->lpBMIH->biWidth,frame->lpBMIH->biHeight,x,y,MaxV,ptr);
    memcpy(ptr,ptrI,frame->lpBMIH->biWidth*frame->lpBMIH->biHeight*sizeof(WORD));
    free(ptrI);
    return MaxV;
}

__forceinline void _zone0(pTVFrame frame)
{
    LPBYTE ptr=GetData(frame);

    //LPBYTE ptrI=(LPBYTE)malloc(frame->lpBMIH->biWidth*frame->lpBMIH->biHeight);
    //_lpass_1DH(frame->lpBMIH->biWidth,frame->lpBMIH->biHeight,ptr,700);
    //memcpy(ptr,ptrI,frame->lpBMIH->biWidth*frame->lpBMIH->biHeight);
    //free(ptrI);
}

//Last was used
__forceinline int _abssubtraction(unsigned char *pBits,int width)
{
    int a=abs((pBits[-width-1]-pBits[-width+1])+(pBits[width-1]-pBits[width+1]));
    int b=abs((pBits[-width]-pBits[-width+2])+(pBits[-2*width]-pBits[-2*width+2]));
    int res=abs(a-b)/4;
    if (res>255) res=255;
    return(res);
}

__forceinline int _abssubtraction16(LPWORD pBits,int width)
{
    int a=abs((pBits[-width-1]-pBits[-width+1])+(pBits[width-1]-pBits[width+1]));
    int b=abs((pBits[-width]-pBits[-width+2])+(pBits[-2*width]-pBits[-2*width+2]));
    int res=abs(a-b)/4;
    if (res>65535) res=65535;
    return(res);
}


/*
__forceinline int _abssubtraction(unsigned char *pBits,int width)
{
    int a=abs((pBits[-1]-pBits[1])+(pBits[width-1]-pBits[width+1])+(pBits[-width-1]-pBits[-width+1]));
    int b=abs((pBits[-width]-pBits[width])+(pBits[-width-1]-pBits[width-1])+(pBits[-width+1]-pBits[width+1]));
    int res=abs(a-b); 
    //if (res<0) res=0;
    if (res>255) res=255;
	return(res);
}
*/

/*
#define FREQ_APPERTUREX 2
#define FREQ_APPERTUREY 2
__forceinline int _abssubtraction(unsigned char *pBits,int width)
{
    int a=abs( (pBits[-1]-pBits[1])+(pBits[width-1]-pBits[width+1])+(pBits[-width-1]-pBits[-width+1]) );
    int b=abs( (pBits[-width-1]-pBits[width-1]) + (pBits[-width]-pBits[width]) + (pBits[-width+1]-pBits[width+1]) );
	return(abs(a-b));
}
*/

__forceinline bool _frconv0(long width, long depth, LPBYTE src, long newwidth, long newdepth,  LPBYTE dst)
{
    int spectr[256]; 
    int i,j,k; 
    int newsize=width*depth/4;
	memset(dst,0,newsize); memset(spectr,0,256*sizeof(int));

	int offset=width<<1;
	i=FREQ_APPERTUREY;
	while (i<depth-FREQ_APPERTUREY)
	{
		j=FREQ_APPERTUREX;
		while(j<width-FREQ_APPERTUREX)
		{
			k=dst[((newwidth*i+j)>>1)]=_abssubtraction(src+offset+j,width);
			spectr[k]+=1;
			j+=2;
		}
		offset+=width<<1;
		i+=2;
	}        
	int _width=width>>1, _depth=depth>>1;
    return true;
}

__forceinline bool _frconv0_16(long width, long depth, LPWORD src, long newwidth, long newdepth,  LPBYTE dst)
{
    int spectr[256]; 
    int i,j,k; 
    int newsize=width*depth/4;
	memset(dst,0,newsize); 
	memset(spectr,0,256*sizeof(int));

	int offset=width<<1;
	i=FREQ_APPERTUREY;
	while (i<depth-FREQ_APPERTUREY)
	{
		j=FREQ_APPERTUREX;
		while(j<width-FREQ_APPERTUREX)
		{
			k=dst[((newwidth*i+j)>>1)]=_abssubtraction16(src+offset+j,width)/256;
			spectr[k]+=1;
			j+=2;
		}
		offset+=width<<1;
		i+=2;
	}        
	int _width=width>>1, _depth=depth>>1; 
    return true;
}


__forceinline bool _frconv(long width, long depth, LPBYTE src, long newwidth, long newdepth, LPBYTE dst)
{
    int spectr[256]; 
    int i,j,k; 
    int newsize=width*depth/4;
	memset(dst,0,newsize); memset(spectr,0,256*sizeof(int));

	int offset=width<<1;
	i=FREQ_APPERTUREY;
	while (i<depth-FREQ_APPERTUREY)
	{
		j=FREQ_APPERTUREX;
		while(j<width-FREQ_APPERTUREX)
		{
			k=dst[((newwidth*i+j)>>1)]=_abssubtraction(src+offset+j,width);
			spectr[k]+=1;
			j+=2;
		}
		offset+=width<<1;
		i+=2;
	}        
	int _width=width>>1, _depth=depth>>1;
#ifdef SIMPLE_ZONE_SEEKING
#pragma message("!!! Simple zone seeking defined!!!\n")
    return true;
#endif	
	int max;
    k=255; max=0; 
    int maxV=newsize/20;
	while (max<maxV) 
    { 
        max+=spectr[k]; 
        k--; 
        if (k<0) break;
    }
	max=k;
//   max=(k<15)?15:k;

    if (k<0) return false;

	offset=0;
	// Normalize intensity distribution
    LPBYTE dstoff=dst; 
	while(dstoff<dst+newsize)
    {  /// Strange but when higher unlinearity here, then better result
        if ((*dstoff)>=max) 
            (*dstoff)=255;
        else 
            (*dstoff)=0; //(255*(*dstoff))/max;
        dstoff++;
	}
//    _remove_points(_width, _depth, dst);
//	_erode(_width, _depth, dst);
//	_dilateh(_width, _depth, dst); 
	return true;
}

__forceinline bool _frconv16(long width, long depth, LPWORD src, long newwidth, long newdepth, LPWORD dst)
{
    int * spectr = new int[65536]; 
    int i,j,k; 
    int newsize=width*depth/4;
	memset(dst,0,newsize*sizeof(short)); memset(spectr,0,65536*sizeof(int));

	int offset=width<<1;
	i=FREQ_APPERTUREY;
	while (i<depth-FREQ_APPERTUREY)
	{
		j=FREQ_APPERTUREX;
		while(j<width-FREQ_APPERTUREX)
		{
			k=dst[((newwidth*i+j)>>1)]=_abssubtraction16(src+offset+j,width);
			spectr[k]+=1;
			j+=2;
		}
		offset+=width<<1;
		i+=2;
	}        
	int _width=width>>1, _depth=depth>>1;
#ifdef SIMPLE_ZONE_SEEKING
#pragma message("!!! Simple zone seeking defined!!!\n")
    delete []spectr;
    return true;
#endif	
	int max;
    k=65535; max=0; 
    int maxV=newsize/20;
	while (max<maxV) 
    { 
        max+=spectr[k]; 
        k--; 
        if (k<0) break;
    }
	max=k;
//   max=(k<15)?15:k;

    if (k<0) 
    {
        delete []spectr;
        return false;
    }

	offset=0;
	// Normalize intensity distribution
    LPWORD dstoff=dst; 
	while(dstoff<dst+newsize)
    {  /// Strange but when higher unlinearity here, then better result
        if ((*dstoff)>=max) 
            (*dstoff)=65535;
        else 
            (*dstoff)=0; //(255*(*dstoff))/max;
        dstoff++;
	}
//    _remove_points(_width, _depth, dst);
//	_erode(_width, _depth, dst);
//	_dilateh(_width, _depth, dst); 
    delete []spectr;
	return true;
}


__forceinline pTVFrame _FeatureDetector(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);


    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;
    int newwidth=(((width/2)>>2)<<2);
    int newheight=(((depth/2)>>2)<<2);
    int newsize=(newwidth*newheight*9)/8;

    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
    case BI_Y8:
        {
            LPBITMAPINFOHEADER lpbmih=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+newsize);
            memcpy(lpbmih,frame->lpBMIH,sizeof(BITMAPINFOHEADER));
            lpbmih->biWidth=newwidth;
            lpbmih->biHeight=newheight;
            lpbmih->biSizeImage=newsize;

            LPBYTE dst=((LPBYTE)lpbmih)+sizeof(BITMAPINFOHEADER);

            LPBYTE src=GetData(frame);
            if (_frconv0(width, depth, src,newwidth,newheight,dst)) 
            {
                //int x,y;
                pTVFrame dstframe = newTVFrame(lpbmih,NULL);
                memset(dst+newwidth*newheight,128,newwidth*newheight/8);
                //_remove_points(newwidth, newheight, dst);
                //_zone(dstframe, x, y);
                return dstframe;
            }
            free(lpbmih);
            break;
        }
    case BI_Y16:
        {
            LPBITMAPINFOHEADER lpbmih=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+newsize);
            memcpy(lpbmih,frame->lpBMIH,sizeof(BITMAPINFOHEADER));
			lpbmih->biCompression=BI_Y8;
			lpbmih->biSizeImage=0;
			lpbmih->biBitCount=8;
            lpbmih->biWidth=newwidth;
            lpbmih->biHeight=newheight;

            LPBYTE dst=((LPBYTE)lpbmih)+sizeof(BITMAPINFOHEADER);

            LPWORD src=(LPWORD)GetData(frame);
            if (_frconv0_16(width, depth, src,newwidth,newheight,dst)) 
            {
                //int x,y;
                pTVFrame dstframe = newTVFrame(lpbmih,NULL);
                //memset(dst+newwidth*newheight,128,newwidth*newheight/8);
                //_remove_points(newwidth, newheight, dst);
                //_zone16(dstframe, x, y);
                return dstframe;
            }
            free(lpbmih);
            break;
        }
    }
    return NULL;
}


__forceinline pTVFrame _FeatureDetector2(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);


    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;
    int newwidth=(((width/2)>>2)<<2);
    int newheight=(((depth/2)>>2)<<2);
    int newsize=(newwidth*newheight*9)/8;

    LPBITMAPINFOHEADER lpbmih=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+newsize);
    memcpy(lpbmih,frame->lpBMIH,sizeof(BITMAPINFOHEADER));
    lpbmih->biWidth=newwidth;
    lpbmih->biHeight=newheight;
    lpbmih->biSizeImage=newsize;

    LPBYTE dst=((LPBYTE)lpbmih)+sizeof(BITMAPINFOHEADER);

    LPBYTE src=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);
    if (_frconv0(width, depth, src, newwidth, newheight, dst)) 
    {
        pTVFrame dstframe = newTVFrame(lpbmih,NULL);
        memset(dst+newwidth*newheight,128,newwidth*newheight/8);
        //_zone0(dstframe);
        return dstframe;
    }
    free(lpbmih);
    return NULL;
}

#define less(a,b)  ((a<b)?1:0)
#define bigger(a,b)((a>b)?1:0)
#define equal(a,b) ((a==b)?1:0)

__forceinline int __min3x3(LPBYTE p,int w) 
{
    return 
        __min(
            __min(
                __min(__min(*(p),*(p+1)),__min(*(p-1),*(p+w))),
                __min(__min(*(p+w+1),*(p+w-1)),__min(*(p-w),*(p-w+1)))
                 ),*(p-w-1)
             );
}

__forceinline bool _maxa3x3(long width, long depth, LPBYTE src, LPBYTE dst)
{
    memset(dst,0,width*depth);
    LPBYTE ss=src+width+1;
    LPBYTE sd=dst+width+1;
    LPBYTE se=src+width*(depth-1)-1;
    while (ss<se)
    {
        bool max=(*ss>=*(ss-1)) && (*ss>=*(ss+1)) && (*ss>=*(ss-width)) && (*ss>=*(ss+width))
                  && (*ss>=*(ss-width-1)) && (*ss>=*(ss+width-1)) && (*ss>=*(ss-width+1)) && (*ss>=*(ss+width+1));
        if (max)
        {
            *sd=*ss;
        }
        else
        {
            *sd=__min3x3(ss,width);
        }
        sd++; ss++;
    }
    return true;
}

__forceinline bool _minmax3x3(long width, long depth, LPBYTE src, LPBYTE dst)
{
    //return _maxa3x3(width, depth, src, dst);

    memset(dst,0,width*depth);
    LPBYTE ss=src+width+1;
    LPBYTE sd=dst+width+1;
    LPBYTE se=src+width*(depth-1)-1;
    while (ss<se)
    {

        bool min=(*ss<*(ss-1)) && (*ss<*(ss+1)) && (*ss<*(ss-width)) && (*ss<*(ss+width))
              && (*ss<*(ss-width-1)) && (*ss<*(ss+width-1)) && (*ss<*(ss-width+1)) && (*ss<*(ss+width+1));
        bool max=(*ss>*(ss-1)) && (*ss>*(ss+1)) && (*ss>*(ss-width)) && (*ss>*(ss+width))
              && (*ss>*(ss-width-1)) && (*ss>*(ss+width-1)) && (*ss>*(ss-width+1)) && (*ss>*(ss+width+1));
        if (min||max)
        {
            //int dif=(*ss-*(ss-1)) + (*ss-*(ss+1)) + (*ss-*(ss-width)) + (*ss-*(ss+width))
                  //+ (*ss-*(ss-width-1)) + (*ss-*(ss+width-1)) + (*ss-*(ss-width+1)) + (*ss-*(ss+width+1));
            *sd=255; //abs(dif);
        }
        else
            *sd=0;
/*
        int min = less(*ss,*(ss-1)) + less(*ss,*(ss+1)) + less(*ss,*(ss-width)) + less(*ss,*(ss+width))
              +  less(*ss,*(ss-width-1)) + less(*ss,*(ss+width-1)) + less(*ss,*(ss-width+1)) + less(*ss,*(ss+width+1));
        int max = bigger(*ss,*(ss-1)) + bigger(*ss,*(ss+1)) + bigger(*ss,*(ss-width)) + bigger(*ss,*(ss+width))
              +   bigger(*ss,*(ss-width-1)) + bigger(*ss,*(ss+width-1)) + bigger(*ss,*(ss-width+1)) + bigger(*ss,*(ss+width+1));
        int eq = equal(*ss,*(ss-1)) + equal(*ss,*(ss+1)) + equal(*ss,*(ss-width)) + equal(*ss,*(ss+width))
              +  equal(*ss,*(ss-width-1)) + equal(*ss,*(ss+width-1)) + equal(*ss,*(ss-width+1)) + equal(*ss,*(ss+width+1));

        *sd=((min>eq) || (max>eq))?255:0; */
        sd++; ss++;
    }
	return true;
}


__forceinline pTVFrame _MinMax3x3(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);


    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;
    int newwidth=width;
    int newheight=depth;

    int newsize=(newwidth*newheight*9)/8;

    LPBITMAPINFOHEADER lpbmih=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+newsize);
    memcpy(lpbmih,frame->lpBMIH,sizeof(BITMAPINFOHEADER));
    lpbmih->biWidth=newwidth;
    lpbmih->biHeight=newheight;
    lpbmih->biSizeImage=newsize;

    LPBYTE dst=((LPBYTE)lpbmih)+sizeof(BITMAPINFOHEADER);

    LPBYTE src=GetData(frame);
    if (_minmax3x3(width, depth, src,dst)) 
    {
            pTVFrame dstframe = newTVFrame(lpbmih,NULL);
            memset(dst+newwidth*newheight,128,newwidth*newheight/8);
            return dstframe;
    }
    free(lpbmih);
    return NULL;
}

__forceinline bool _markMaximums3x3(long width, long depth, LPBYTE src, LPBYTE dst)
{
    memset(dst,0,width*depth);
    LPBYTE ss=src+width+1;
    LPBYTE sd=dst+width+1;
    LPBYTE se=src+width*(depth-1)-1;
    while (ss<se)
    {

        bool isnotmax=(*(ss-1)>*ss) ||
                      (*(ss+1)>*ss) ||
                      (*(ss-width)>*ss) ||
                      (*(ss-width-1)>*ss) ||
                      (*(ss-width+1)>*ss) ||
                      (*(ss+width)>*ss) ||
                      (*(ss+width-1)>*ss) ||
                      (*(ss+width+1)>*ss);
        *sd      = (isnotmax)? 1:*ss; 
        sd++; ss++;
    }
	return true;
}

__forceinline pTVFrame _MarkMaximums(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);


    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;
    int newwidth=width;
    int newheight=depth;

    int newsize=(newwidth*newheight*9)/8;

    LPBITMAPINFOHEADER lpbmih=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+newsize);
    memcpy(lpbmih,frame->lpBMIH,sizeof(BITMAPINFOHEADER));
    lpbmih->biWidth=newwidth;
    lpbmih->biHeight=newheight;
    lpbmih->biSizeImage=newsize;

    LPBYTE dst=((LPBYTE)lpbmih)+sizeof(BITMAPINFOHEADER);

    LPBYTE src=GetData(frame);
    if (_markMaximums3x3(width, depth, src,dst)) 
    {
            pTVFrame dstframe = newTVFrame(lpbmih,NULL);
            memset(dst+newwidth*newheight,128,newwidth*newheight/8);
            return dstframe;
    }
    free(lpbmih);
    return NULL;
}

#endif
