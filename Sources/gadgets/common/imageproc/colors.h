#ifndef COLORS_INC
#define COLORS_INC

#include <imageproc\simpleip.h>
#include <math\hbmath.h>
#include <imageproc\clusters\clusters.h>
#include <imageproc\imagebits.h>

#define DELTA 10

#define COLOR_UNKNOWN		0
#define COLOR_BLACK         1
#define COLOR_RED			2
#define COLOR_BLUE			3
#define COLOR_YELLOW		4
#define COLOR_GREEN         5
#define COLOR_WHITE         6
#define COLOR_MAX           7

__forceinline void _swapRB(LPBYTE ptr)
{
    BYTE t=ptr[2];
    ptr[2]=ptr[0];
    ptr[0]=t;
}

__forceinline BOOL colors_match_not_strictly(DWORD color1, DWORD color2)
{
	return (color1 == color2 || color1 == COLOR_UNKNOWN || color2 == COLOR_UNKNOWN);
}

__forceinline COLORREF color_id_to_rgb(DWORD color)
{
	switch (color)
	{
	case COLOR_BLACK:
		return RGB(0, 0, 0);
	case COLOR_RED:
		return RGB(255, 0, 0);
	case COLOR_BLUE:
		return RGB(0, 0, 255);
	case COLOR_YELLOW:
		return RGB(255, 255, 0);
	case COLOR_GREEN:
		return RGB(0, 255, 0);
	case COLOR_WHITE:
		return RGB(255, 255, 255);
	case COLOR_UNKNOWN:
		return RGB(128, 0, 128);
	default:
		ASSERT(FALSE); // unimplemented color
	}
	return 0;
}

typedef struct tagcolorpnt
{
    int i,u,v;
    tagcolorpnt(int I,int U, int V) { i=I; u=U; v=V; };
}colorpnt,*pcolorpnt;

#define black   colorpnt(-53,128,128)
#define green   colorpnt(38,38,0)
#define red     colorpnt(0,255,128)
#define blue    colorpnt(0,128,255)
#define blue2    colorpnt(128,64,300)
#define yellow  colorpnt(308,200,40)
#define white   colorpnt(308,128,128)

__forceinline int clrdist(colorpnt& a, colorpnt& b)
{
    return (int)sqrt(1.0*(a.i - b.i)*(a.i - b.i)
                    +1.1*(a.u - b.u)*(a.u - b.u)
                    +1.1*(a.v - b.v)*(a.v - b.v));
}

__forceinline int colordefnew(int I, int U, int V)
{
    int retVal=COLOR_UNKNOWN;
    int min=1000;
    colorpnt a(I,U,V);
    if (abs(I-128)<40) 
    {
        if  (((U-128)*(U-128)+(V-128)*(V-128))<9) return COLOR_UNKNOWN;
    }
    if (min>clrdist(a,black)) 
    {
        min=clrdist(a,black);
        //retVal=COLOR_BLACK;
        retVal=COLOR_UNKNOWN;
    }
    if (min>clrdist(a,blue)) 
    {
        min=clrdist(a,blue);
        retVal=COLOR_BLUE;
    }
    if (min>clrdist(a,blue2)) 
    {
        min=clrdist(a,blue2);
        retVal=COLOR_BLUE;
    }
    if (min>clrdist(a,red)) 
    {
        min=clrdist(a,red);
        retVal=COLOR_RED;
    }
    if (min>clrdist(a,yellow)) 
    {
        min=clrdist(a,yellow);
        retVal=COLOR_YELLOW;
    }
    if (min>clrdist(a,green)) 
    {
        min=clrdist(a,green);
        retVal=COLOR_GREEN;
    }
    if (min>clrdist(a,white)) 
    {
        min=clrdist(a,black);
        //retVal=COLOR_WHITE;
        return COLOR_UNKNOWN;
    }
    return retVal;
}


__forceinline int colordef(int I, int U, int V)
{
    if (sqrt((U-128)*(U-128)+(V-128)*(V-128))<DELTA)
    {
        if (I<60)
            return COLOR_BLACK;
        else if (I>180)
            return COLOR_WHITE;
    }
    if (I<50)
    {
        if ((((V*V)+U*U)/20000)<1)
        {
            return COLOR_GREEN;
        }
        else if ((U>128) && (U>V))
        {
            return COLOR_RED;
        }
        else if ((I<40) && (V<128))
        {
            return COLOR_BLACK;
        }
        else 
        {
            return COLOR_BLUE;
        }
        return COLOR_BLACK;
    }
    else if (I<90)
    {
        if (sqrt((U-128)*(U-128)+(V-128)*(V-128))<DELTA/2)
            return COLOR_BLACK;

        if ((U>128) && (V>128))
        {
            return COLOR_UNKNOWN;
        }
        else if ((U<128) && (V>138))
        {
            return COLOR_BLUE;
        }
        else if ((U>128) && (V<128))
        {
            return COLOR_RED;            
        }
        return COLOR_UNKNOWN;
    }
    else if (I<170)
    {
        if (sqrt((U-128)*(U-128)+(V-128)*(V-128))<DELTA/2)
            if (I<120)
                return COLOR_BLACK;
            else
                return COLOR_UNKNOWN;

        if ((U>136) && (V<60))
        {
            return COLOR_YELLOW;
        }
        else if ((V>60) && (U>130))
        {
            return COLOR_RED;
        }
        else if ((U<128) && (V<115))
        {
            return COLOR_GREEN;
        }
        else if ((V>128) && (U<128))
        {
            return COLOR_BLUE;
        }
        if (I<120)
            return COLOR_BLACK;
        else if (I<150)
            return COLOR_UNKNOWN;
        else
            return COLOR_WHITE;
    }
    else
    {
        if (sqrt((U-128)*(U-128)+(V-128)*(V-128))<6*DELTA)
            return COLOR_WHITE;

        if ((U>94) && (V<84))
        {
            return COLOR_YELLOW;
        }
        else if ((((255-V)*(255-V)+(255-U)*(255-U))/16384)<1)
        {
            return COLOR_RED;
        }
        else if ((V<84) && (U<94))
        {
            return COLOR_GREEN;
        }
        else if ((V>84) && (U<94))
        {
            return COLOR_BLUE;
        }
        return COLOR_WHITE;
    }
}


__forceinline void showcolors(pTVFrame Frame)
{
    LPBYTE bits=GetData(Frame);
    LPBITMAPINFOHEADER lpBMIH=Frame->lpBMIH;
    int sizeY=lpBMIH->biWidth*lpBMIH->biHeight;
    int w=lpBMIH->biWidth;
    LPBYTE U  = bits +sizeY;
    LPBYTE V  = U    +sizeY/16;


    LPBYTE end = U+sizeY/16;
    
    LPBYTE I0   = bits;
    LPBYTE I1   =I0+w;
    LPBYTE I2   =I1+w;
    LPBYTE I3   =I2+w;

    
    int i;

    while (U<end)
    {
        i=(*I0+*(I0+1)+*(I0+2)+*(I0+3)+
           *I1+*(I1+1)+*(I1+2)+*(I1+3)+
           *I2+*(I2+1)+*(I2+2)+*(I2+3)+
           *I3+*(I3+1)+*(I3+2)+*(I3+3))/16;

        switch (colordef(i,*U,*V))
        {
        case COLOR_UNKNOWN:
            *U=128;
            *V=128;
            break;
        case COLOR_BLACK:
            *U=128;
            *V=128;
            *I0=*(I0+1)=*(I0+2)=*(I0+3)=
                *I1=*(I1+1)=*(I1+2)=*(I1+3)=
                *I2=*(I2+1)=*(I2+2)=*(I2+3)=
                *I3=*(I3+1)=*(I3+2)=*(I3+3)=0;
            break;
        case COLOR_RED:
            *U=255;
            *V=128;
            *I0=*(I0+1)=*(I0+2)=*(I0+3)=
                *I1=*(I1+1)=*(I1+2)=*(I1+3)=
                *I2=*(I2+1)=*(I2+2)=*(I2+3)=
                *I3=*(I3+1)=*(I3+2)=*(I3+3)=0;
            break;
        case COLOR_BLUE:
            *U=128;
            *V=255;
            *I0=*(I0+1)=*(I0+2)=*(I0+3)=
                *I1=*(I1+1)=*(I1+2)=*(I1+3)=
                *I2=*(I2+1)=*(I2+2)=*(I2+3)=
                *I3=*(I3+1)=*(I3+2)=*(I3+3)=0;
            break;
        case COLOR_YELLOW:
            *U=128;
            *V=0;
            *I0=*(I0+1)=*(I0+2)=*(I0+3)=
                *I1=*(I1+1)=*(I1+2)=*(I1+3)=
                *I2=*(I2+1)=*(I2+2)=*(I2+3)=
                *I3=*(I3+1)=*(I3+2)=*(I3+3)=255;
            break;
        case COLOR_GREEN:
            *U=128;
            *V=128;
            *U=0;
            *V=0;
            *I0=*(I0+1)=*(I0+2)=*(I0+3)=
                *I1=*(I1+1)=*(I1+2)=*(I1+3)=
                *I2=*(I2+1)=*(I2+2)=*(I2+3)=
                *I3=*(I3+1)=*(I3+2)=*(I3+3)=0;
            break;
        case COLOR_WHITE:
            *U=128;
            *V=128;
            *I0=*(I0+1)=*(I0+2)=*(I0+3)=
                *I1=*(I1+1)=*(I1+2)=*(I1+3)=
                *I2=*(I2+1)=*(I2+2)=*(I2+3)=
                *I3=*(I3+1)=*(I3+2)=*(I3+3)=255;
            break;
        }

        *U++; *V++;
        I0+=4;  I1+=4; I2+=4; I3+=4;
        if (((I0-bits)%w)==0)
        {
            I0+=3*w;  I1+=3*w; I2+=3*w; I3+=3*w;
        }
    }
}

__forceinline int distC(int I1, int U1, int V1,int I2, int U2, int V2)
{
    return (int)(0.57735026918962576450914878050196*sqrt((I2-I1)*(I2-I1)+(8*(U2-U1)*(U2-U1)+(V2-V1)*(V2-V1))));
}

__forceinline void _FindRoughnessC(pTVFrame Frame)
{
    int w=Frame->lpBMIH->biWidth;
    int wU=w/4;
    int size=Frame->lpBMIH->biHeight*w;
    int sizeU=size/16;
    LPBYTE I=GetData(Frame);
    LPBYTE U=I+size;
    LPBYTE V=U+sizeU;

    LPBYTE pI=I;
    LPBYTE pU=U;
    LPBYTE pV=V;

    LPBYTE eod=V;
    int aI=0,aU=0, aV=0;
    while (pU<eod)
    {
        aI+=(*(pI)+*(pI+1)+*(pI+2)+*(pI+3)); pI+=w;
        aI+=(*(pI)+*(pI+1)+*(pI+2)+*(pI+3)); pI+=w;
        aI+=(*(pI)+*(pI+1)+*(pI+2)+*(pI+3)); pI+=w;
        aI+=(*(pI)+*(pI+1)+*(pI+2)+*(pI+3)); pI-=3*w;
        aU+=*pU;
        aV+=*pV;
        pU++;
        pV++;
        if (((pU-eod)%wU)==0)
        {
            pI+=3*w;
        }
        pI+=4;
    }
    aI/=size;
    aU/=sizeU;
    aV/=sizeU;
    pI=I;
    pU=U;
    pV=V;
    while (pU<eod)
    {
        *(pI)=distC(aI,aU,aV,*pI,*pU,*pV); *(pI+1)=distC(aI,aU,aV,*(pI+1),*pU,*pV); *(pI+2)=distC(aI,aU,aV,*(pI+2),*pU,*pV); *(pI+3)=distC(aI,aU,aV,*(pI+3),*pU,*pV); pI+=w;
        *(pI)=distC(aI,aU,aV,*pI,*pU,*pV); *(pI+1)=distC(aI,aU,aV,*(pI+1),*pU,*pV); *(pI+2)=distC(aI,aU,aV,*(pI+2),*pU,*pV); *(pI+3)=distC(aI,aU,aV,*(pI+3),*pU,*pV); pI+=w;
        *(pI)=distC(aI,aU,aV,*pI,*pU,*pV); *(pI+1)=distC(aI,aU,aV,*(pI+1),*pU,*pV); *(pI+2)=distC(aI,aU,aV,*(pI+2),*pU,*pV); *(pI+3)=distC(aI,aU,aV,*(pI+3),*pU,*pV); pI+=w;
        *(pI)=distC(aI,aU,aV,*pI,*pU,*pV); *(pI+1)=distC(aI,aU,aV,*(pI+1),*pU,*pV); *(pI+2)=distC(aI,aU,aV,*(pI+2),*pU,*pV); *(pI+3)=distC(aI,aU,aV,*(pI+3),*pU,*pV); pI-=3*w;
        *pU=128;
        *pV=128;
        pU++;
        pV++;
        if (((pU-eod)%wU)==0)
        {
            pI+=3*w;
        }
        pI+=4;
    }
}

__forceinline int signC(int d)
{
    return (d<=128)?0:(d-128);
}

__forceinline void grey(LPBYTE Data, int size, double scale=1.0)
{
    LPBYTE eod=Data+size;
    while  (Data<eod)
    {
        int d=(int)(scale*signC(*Data));
        *Data=(d>255)?255:d;
        Data++;
    }
}

__forceinline void seekC(LPBYTE U, LPBYTE V, int Size)
{
    LPBYTE pU=U,pV=V;
    LPBYTE eod=pU+Size;
    while (pU<eod)
    {
        if ((*pU<130) && (*pV<130))
        {
            *pU=128;
            *pV=128;
        }
        pU++; pV++;
    } 
    grey(U,Size,2);
    grey(V,Size,1);
    _videoADD(U,V,Size);
}

BYTE __forceinline colorfunc(BYTE in)
{
    return in;
}

BYTE __forceinline colorfunc16(WORD in)
{
    return (BYTE)(in>>8);
}

LPBYTE __forceinline  _makecolormatrix(LPBYTE data, int& dsize, int width, int height)
{
    dsize=(width*height)/16;
    LPBYTE retV=(LPBYTE)malloc(dsize);
    LPBYTE sc=retV;
    for (int y=0; y<height; y+=4)
    {
        LPBYTE off=data+y*width;
        for (int x=0; x<width; x+=4)
        {
            *sc=colorfunc(*(off+x));
            sc++;
        }
    }
    return retV;
}

LPBYTE __forceinline  _makecolormatrix16(LPWORD data, int& dsize, int width, int height)
{
    dsize=(width*height)/16;
    LPBYTE retV=(LPBYTE)malloc(dsize);
    LPBYTE sc=retV;
    for (int y=0; y<height; y+=4)
    {
        LPWORD off=data+y*width;
        for (int x=0; x<width; x+=4)
        {
            *sc=colorfunc16(*(off+x));
            sc++;
        }
    }
    return retV;
}


void __forceinline  _invert(LPBYTE data, int dsize)
{
    LPBYTE eod=data+dsize;
    while (data<eod)
    {
        *data=255-*data;
        data++;
    }
}

__forceinline bool _pseudocolor(pTVFrame frame)
{    
  switch (frame->lpBMIH->biCompression)
  {
  case BI_YUV9:
    {
      LPBYTE lpData=GetData(frame);
      if (!lpData) return false;
      int dsize;
      LPBYTE d=_makecolormatrix(lpData,dsize, frame->lpBMIH->biWidth, frame->lpBMIH->biHeight);
      LPBYTE eod=lpData+frame->lpBMIH->biSizeImage;
      lpData+=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      //while (lpData<eod) { *lpData=0; lpData++;}
      memcpy(lpData,d,dsize); lpData+=dsize;
      _invert(d, dsize); 
      memcpy(lpData,d,dsize); 
      free(d);
      return true;
    }
  case BI_Y8:
    {
      int size=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      int newsize=((9*size)/8);
      LPBITMAPINFOHEADER oBMIH=frame->lpBMIH;
      if (frame->lpData)
      {
        frame->lpBMIH=(LPBITMAPINFOHEADER)realloc(frame->lpBMIH,newsize+oBMIH->biSize);
        memcpy(((LPBYTE)frame->lpBMIH)+oBMIH->biSize,frame->lpData,size);
        free(frame->lpData); frame->lpData=NULL;
      }
      else
      {
        frame->lpBMIH=(LPBITMAPINFOHEADER)realloc(frame->lpBMIH,newsize+frame->lpBMIH->biSize);
      }
      frame->lpBMIH->biCompression=BI_YUV9;
      frame->lpBMIH->biSizeImage=newsize;
      LPBYTE lpData=GetData(frame);
      if (!lpData) return false;
      int dsize;
      LPBYTE d=_makecolormatrix(lpData,dsize, frame->lpBMIH->biWidth, frame->lpBMIH->biHeight);
      LPBYTE eod=lpData+frame->lpBMIH->biSizeImage;
      lpData+=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      //while (lpData<eod) { *lpData=0; lpData++;}
      memcpy(lpData,d,dsize); lpData+=dsize;
      _invert(d, dsize); 
      memcpy(lpData,d,dsize); 
      free(d);
      return true;
    }
  case BI_Y16:
    {
      int size=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      int newsize=((9*size)/8);
      LPWORD orgData=NULL;
      LPBITMAPINFOHEADER oBMIH=frame->lpBMIH;
      if (frame->lpData)
      {
        frame->lpBMIH=(LPBITMAPINFOHEADER)malloc(newsize+oBMIH->biSize);
        memcpy(frame->lpBMIH,oBMIH,oBMIH->biSize);
        orgData=(LPWORD)frame->lpData;
        frame->lpData=NULL;
      }
      else
      {
        orgData=(LPWORD)malloc(sizeof(WORD)*size);
        memcpy(orgData,GetData(frame),sizeof(WORD)*size);
        frame->lpBMIH=(LPBITMAPINFOHEADER)malloc(newsize+frame->lpBMIH->biSize);
      }
      LPWORD src=orgData;
      for (LPBYTE dst=GetData(frame); dst<GetData(frame)+size; dst++)
      {
        *dst=(BYTE)((*src)>>8);
        src++;
      }
      frame->lpBMIH->biCompression=BI_YUV9;
      frame->lpBMIH->biSizeImage=newsize;
      LPBYTE lpData=GetData(frame);
      if (!lpData) return false;
      int dsize;
      LPBYTE d=_makecolormatrix16(orgData,dsize, frame->lpBMIH->biWidth, frame->lpBMIH->biHeight);
      LPBYTE eod=lpData+frame->lpBMIH->biSizeImage;
      lpData+=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      //while (lpData<eod) { *lpData=0; lpData++;}
      memcpy(lpData,d,dsize); lpData+=dsize;
      _invert(d, dsize); 
      memcpy(lpData,d,dsize); 
      free(d);
      free(orgData);
      return true;
    }
  }
  return false;
}

typedef struct 
{
  BYTE U ;
  BYTE V ;
} UVComps;


__forceinline void FormColorTable( UVComps * Table , int iMin , int iMax )
{
  int i = iMin ;
  int iAmpl = iMax - iMin ;
  if ( iAmpl <= 0 )
    iAmpl = 1 ;
  int iStep_x1000 = (255 * 2 * 1000) / iAmpl ;
  for ( ; i < iMin + (iAmpl/2) ; i++ )
  {
    Table[i].U = ((i - iMin) * iStep_x1000) / 1000 ;
    Table[i].V = 255 ;
  }
  iMin = i ;
  for ( ; i < iMin + ( iAmpl/ 2 ) ; i++ )
  {
    Table[i].U = 255 ;
    Table[i].V = 255 -  ((i - iMin) * iStep_x1000) / 1000 ;
  }
}

__forceinline bool _pseudocolor( pTVFrame pDest , const pTVFrame pSrc ,
  UVComps * pTable , int iMin , int iMax )
{    
  LPBYTE lpData=GetData(pSrc);
  if (!lpData) 
    return false;
  LPBYTE lpDestData = GetData( pDest ) ;
  int iWidth = Width( pSrc ) ;
  int iWidthDest = Width( pDest ) ;
  int iHeight = Height( pSrc ) ;
  int iDestSizeI8 = GetI8Size( pDest ) ;
  char * pV = (char*)( lpDestData + iDestSizeI8 ) ;
  char * pU = pV + (iDestSizeI8/4) ;
  int iColorWidth = GetWidth(pDest) / 2 ; 

  switch (pSrc->lpBMIH->biCompression)
  {
  case BI_Y8:
  case BI_YUV9:
  case BI_YUV12:
    {
      for ( int iY = 0 ; iY < iHeight ; iY += 2 ) // step 2, because only because color
                            // color information has 2 times less resolution
      {
        int iColorOffset = (iColorWidth * iY) / 2 ;
        char * pVwork = pV + iColorOffset ;
        char * pUwork = pU + iColorOffset ;
        LPBYTE pRowSrc = lpData + iY * iWidth ;
        LPBYTE pSrcEnd = pRowSrc + iWidth ;
        LPBYTE pRowDest = lpDestData + iY * iWidthDest ; 
        for ( ; pRowSrc < pSrcEnd ; pRowSrc += 2 ) // we take every second pixel because color
          // color information has 2 times less resolution 
        {
          BYTE PixVal = *pRowSrc ;
          if ( PixVal < iMin  ||  PixVal > iMax )
          {
            *pVwork = *pUwork = 127 ;
            *pRowDest = PixVal ;
            *(pRowDest + 1) = *(pRowSrc + 1) ;
            *(pRowDest + iWidthDest) = *(pRowSrc + iWidth) ;
            *(pRowDest + iWidthDest+ 1) = *(pRowSrc + iWidth + 1) ;
          }
          else
          {
            *pVwork = pTable[*pRowSrc].V ;
            *pUwork = pTable[*pRowSrc].U ;
            *pRowDest = 128 ;
            *(pRowDest + 1) = 128 ;
            *(pRowDest + iWidthDest) = 128 ;
            *(pRowDest + iWidthDest+ 1) = 128 ;
          }
          pUwork++ ;
          pVwork++ ;
          pRowDest += 2 ;
        }

      }
      return true;
    }
  case BI_Y16:
    {
      for ( int iY = 0 ; iY < iHeight ; iY += 2 ) // step 2, because only because color
        // color information has 2 times less resolution
      {
        int iColorOffset = (iColorWidth * iY) / 2 ;
        char * pVwork = pV + iColorOffset ;
        char * pUwork = pU + iColorOffset ;
        LPWORD pRowSrc = (LPWORD)lpData + iY * iWidth ;
        LPWORD pSrcEnd = pRowSrc + iWidth ;
        LPBYTE pRowDest = lpDestData + iY * iWidthDest ; 
        for ( ; pRowSrc < pSrcEnd ; pRowSrc += 2 ) // we take every second pixel because color
          // color information has 2 times less resolution 
        {
          WORD PixVal = *pRowSrc ;
          if ( PixVal < iMin  ||  PixVal > iMax )
          {
            *pVwork = *pUwork = 0 ;
            *pRowDest = (BYTE)(PixVal >> 8) ;
            *(pRowDest + 1) = (BYTE) (*(pRowSrc + 1) >> 8) ;
            *(pRowDest + iWidthDest) = (BYTE) (*(pRowSrc + iWidth)>>8) ;
            *(pRowDest + iWidthDest+ 1) = (BYTE) (*(pRowSrc + iWidth + 1)>>8) ;
          }
          else
          {
            *pVwork = pTable[*pRowSrc].V ;
            *pUwork = pTable[*pRowSrc].U ;
            *pRowDest = 255 ;
            *(pRowDest + 1) =255 ;
            *(pRowDest + iWidthDest) = 255 ;
            *(pRowDest + iWidthDest+ 1) = 255 ;
          }
          pUwork++ ;
          pVwork++ ;
          pRowDest += 2 ;
        }

      }
      return true;
    }
  }
  return false;
}


#endif
