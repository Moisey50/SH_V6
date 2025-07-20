#ifndef ENCHANCE1_INC
#define ENCHANCE1_INC

#include <video\TVFrame.h>
#include <imageproc\resample.h>
#include <imageproc\fstfilter.h>
#include <imageproc\gammacorrection.h>

inline void _enchance1(pTVFrame frame, int conv_param=CONOVOLUTION_PARAM)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    pTVFrame LoFrame=_lpass(frame,conv_param); 
    pTVFrame LLoFrame=makecopyTVFrame(LoFrame);
    pTVFrame HiFrame=_hpass(frame,conv_param); 
    
    _gamma(LLoFrame,0.3);
    //_log(LLoFrame);

    int depth=frame->lpBMIH->biHeight;
    int width=frame->lpBMIH->biWidth;
    LPBYTE pData=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);
    LPBYTE pLoData=(LoFrame->lpData)?LoFrame->lpData:(((LPBYTE)LoFrame->lpBMIH)+LoFrame->lpBMIH->biSize);
    LPBYTE pLLoData=(LLoFrame->lpData)?LLoFrame->lpData:(((LPBYTE)LLoFrame->lpBMIH)+LLoFrame->lpBMIH->biSize);
    LPBYTE pHiData=(HiFrame->lpData)?HiFrame->lpData:(((LPBYTE)HiFrame->lpBMIH)+HiFrame->lpBMIH->biSize);
    for (int i=0; i<width*depth; i++)
    {
        double k= (pLoData[i])?((double)pLLoData[i])/pLoData[i]:0;
        int I=(int)( 0.8* ((int)(pLLoData[i])+k*(((int)(pHiData[i]))-128)));
        pData[i]=(I<0)?0:((I>255)?255:I);
    }
    freeTVFrame(LoFrame);
    freeTVFrame(LLoFrame);
    freeTVFrame(HiFrame);
}

inline void _flattering0(long width, long depth, unsigned char *src, unsigned char *dst)
{
#define AREA 3

  int i,j,x,y;
  int max;
  int min;
  int datasize=width*depth;
  LPBYTE srcoff,dstoff,dstoff_e;

  // memcpy(dst+datasize,src+datasize,datasize>>3); /* copy color block */
  for (i=0; i<depth; i++)
  {
	  y=(i<AREA)?0:((i>depth-AREA)?depth-2*AREA:i-AREA);
      dstoff=dst+width*i; dstoff_e=dstoff+width;
      srcoff=src+width*i;
	  for (j=0; j<width; j++)		    
	  {  
		  x=(j<AREA)?0:((j>width-AREA)?width-2*AREA:j-AREA);
		  int k;
		  max=0; min=255;
		  for (k=y;k<y+2*AREA;k++)
		  {
              LPBYTE lsrcoff=src+x+width*k;
              LPBYTE lsrcoff_e=lsrcoff+2*AREA;
              while (lsrcoff<lsrcoff_e)
              {
				  if (max<*lsrcoff) max=*lsrcoff;
                  if (min>*lsrcoff) min=*lsrcoff;
                  lsrcoff++;
			  }
		  }
          int ampl=max-min;
		  if (ampl)
			  *dstoff=(255*(*srcoff-min))/ampl;
		  else
		      *dstoff=0;
          dstoff++; srcoff++;
	  }
  }
#undef AREA
}

inline void _flattering1(long width, long height, unsigned char *src, unsigned char *dst,int conv_param=CONOVOLUTION_PARAM)
{
    int datasize=width*height;
    LPBYTE tmpD=(LPBYTE)malloc(datasize);

    _lpass(width,height,src,tmpD,conv_param);
    LPBYTE sc=tmpD; 
    LPBYTE eod=sc+datasize;
   
    while (sc<eod)
    {
        if (*sc)
        {
            int res=(128* *src)/ *sc;
            *dst= (res>255)?255:res;
        }
        else
            *dst=0;
        sc++; src++; dst++;
    }
    
    free(tmpD);
}

inline void _flattering1_16(long width, long height, LPWORD src, LPWORD dst,int conv_param=CONOVOLUTION_PARAM)
{
    int datasize=width*height;
    LPWORD tmpD=(LPWORD)malloc(datasize*sizeof(WORD));

    _lpass16(width,height,src,tmpD,conv_param);
    LPWORD sc=tmpD; 
    LPWORD eod=sc+datasize;
   
    while (sc<eod)
    {
        if (*sc)
        {
            int res=(32768* *src)/ *sc;
            *dst= (res>65535)?65535:res;
        }
        else
            *dst=0;
        sc++; src++; dst++;
    }
    
    free(tmpD);
}

inline void _enchance2(pTVFrame frame, int conv_param=CONOVOLUTION_PARAM)
{
    int datasize=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
        {
            LPBYTE tmpD=(LPBYTE)malloc(datasize);
            LPBYTE dst=GetData(frame);
            memcpy(tmpD,dst,datasize);
    
            _flattering1(frame->lpBMIH->biWidth, frame->lpBMIH->biHeight, tmpD, dst,conv_param);
    
            free(tmpD);
            break;
        }
    case BI_Y16:
        {
            LPWORD tmpD=(LPWORD)malloc(datasize*sizeof(WORD));
            LPWORD dst=(LPWORD)GetData(frame);
            memcpy(tmpD,dst,datasize*sizeof(WORD));
    
            _flattering1_16(frame->lpBMIH->biWidth, frame->lpBMIH->biHeight, tmpD, dst,conv_param);
    
            free(tmpD);
            break;
        }
    default:
        ASSERT(FALSE);
    }
}

// build resampled down twice image with mins from source
__forceinline BYTE _min4(BYTE a, BYTE b, BYTE c, BYTE d)
{
    BYTE e=(a<b)?a:b;
    BYTE f=(c<d)?c:d;
    return (e<f)?e:f;
}

__forceinline WORD _min4(WORD a, WORD b,WORD c, WORD d)
{
    WORD e=(a<b)?a:b;
    WORD f=(c<d)?c:d;
    return (e<f)?e:f;
}

__forceinline LPBITMAPINFOHEADER _minmap_base(LPBITMAPINFOHEADER srcbmp, LPVOID lpData=NULL)
{
    int oldW=srcbmp->biWidth;
    int oldH=srcbmp->biHeight;
    int newW=srcbmp->biWidth/2;
    int newH=srcbmp->biHeight/2;
    int datasize=newW*newH;
    if (lpData==NULL) lpData=GetData(srcbmp);
    ASSERT(datasize!=0);
    LPBITMAPINFOHEADER dstbmp=NULL;
    switch (srcbmp->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
        {
            dstbmp=(LPBITMAPINFOHEADER)malloc(datasize+srcbmp->biSize);
            ASSERT(dstbmp);
            memcpy(dstbmp,srcbmp,srcbmp->biSize);
            dstbmp->biWidth=newW;
            dstbmp->biHeight=newH;
            dstbmp->biSizeImage=newW*newH;
            dstbmp->biCompression=BI_Y8;
            LPBYTE src=(LPBYTE)lpData;
            LPBYTE dst=GetData(dstbmp);
            int nx,noff=0,ooff=0;
            for(int y=0; y<newH; y++)
            {
                for (int x=0; x<newW; x++)
                {
                    nx=x<<1;
                    dst[x+noff]=_min4(src[ooff+nx], src[ooff+nx+1], src[ooff+nx+oldW], src[ooff+nx+oldW+1]);
                }
                noff+=newW;
                ooff+=2*oldW;
            }
            break;
        }
    case BI_Y16:
        {
            dstbmp=(LPBITMAPINFOHEADER)malloc(datasize+srcbmp->biSize*sizeof(WORD));
            ASSERT(dstbmp);
            memcpy(dstbmp,srcbmp,srcbmp->biSize);
            dstbmp->biWidth=newW;
            dstbmp->biHeight=newH;
            dstbmp->biSizeImage=newW*newH;
            dstbmp->biCompression=BI_Y8;
            LPWORD src=(LPWORD)lpData;
            LPWORD dst=(LPWORD)GetData(dstbmp);
            int nx,noff=0,ooff=0;
            for(int y=0; y<newH; y++)
            {
                for (int x=0; x<newW; x++)
                {
                    nx=x<<1;
                    dst[x+noff]=_min4(src[ooff+nx], src[ooff+nx+1], src[ooff+nx+oldW], src[ooff+nx+oldW+1]);
                }
                noff+=newW;
                ooff+=2*oldW;
            }
            break;
            break;
        }
    default:
        ASSERT(FALSE);
    }
    return dstbmp;
}

// build resampled down twice image with mins from source
__forceinline BYTE _max4(BYTE a, BYTE b, BYTE c, BYTE d)
{
    BYTE e=(a>b)?a:b;
    BYTE f=(c>d)?c:d;
    return (e>f)?e:f;
}

__forceinline WORD _max4(WORD a, WORD b,WORD c, WORD d)
{
    WORD e=(a>b)?a:b;
    WORD f=(c>d)?c:d;
    return (e>f)?e:f;
}

__forceinline LPBITMAPINFOHEADER _maxmap_base(LPBITMAPINFOHEADER srcbmp, LPVOID lpData=NULL)
{
    int oldW=srcbmp->biWidth;
    int oldH=srcbmp->biHeight;
    int newW=srcbmp->biWidth/2;
    int newH=srcbmp->biHeight/2;
    int datasize=newW*newH;
    if (lpData==NULL) lpData=GetData(srcbmp);
    ASSERT(datasize!=0);
    LPBITMAPINFOHEADER dstbmp=NULL;
    switch (srcbmp->biCompression)
    {
    case BI_YUV9:
    case BI_YUV12:
    case BI_Y8:
        {
            dstbmp=(LPBITMAPINFOHEADER)malloc(datasize+srcbmp->biSize);
            ASSERT(dstbmp);
            memcpy(dstbmp,srcbmp,srcbmp->biSize);
            dstbmp->biWidth=newW;
            dstbmp->biHeight=newH;
            dstbmp->biSizeImage=newW*newH;
            dstbmp->biCompression=BI_Y8;
            LPBYTE src=(LPBYTE)lpData;
            LPBYTE dst=GetData(dstbmp);
            int nx,noff=0,ooff=0;
            for(int y=0; y<newH; y++)
            {
                for (int x=0; x<newW; x++)
                {
                    nx=x<<1;
                    dst[x+noff]=_max4(src[ooff+nx], src[ooff+nx+1], src[ooff+nx+oldW], src[ooff+nx+oldW+1]);
                }
                noff+=newW;
                ooff+=2*oldW;
            }
            break;
        }
    case BI_Y16:
        {
            dstbmp=(LPBITMAPINFOHEADER)malloc(datasize+srcbmp->biSize*sizeof(WORD));
            ASSERT(dstbmp);
            memcpy(dstbmp,srcbmp,srcbmp->biSize);
            dstbmp->biWidth=newW;
            dstbmp->biHeight=newH;
            dstbmp->biSizeImage=newW*newH;
            dstbmp->biCompression=BI_Y8;
            LPWORD src=(LPWORD)lpData;
            LPWORD dst=(LPWORD)GetData(dstbmp);
            int nx,noff=0,ooff=0;
            for(int y=0; y<newH; y++)
            {
                for (int x=0; x<newW; x++)
                {
                    nx=x<<1;
                    dst[x+noff]=_max4(src[ooff+nx], src[ooff+nx+1], src[ooff+nx+oldW], src[ooff+nx+oldW+1]);
                }
                noff+=newW;
                ooff+=2*oldW;
            }
            break;
            break;
        }
    default:
        ASSERT(FALSE);
    }
    return dstbmp;
}

__forceinline LPBITMAPINFOHEADER _maxmap(int times, LPBITMAPINFOHEADER srcbmp, LPVOID lpData=NULL)
{
    if ((times<1) || (times>10)) return NULL;
    LPBITMAPINFOHEADER tmpMAP=_maxmap_base(srcbmp,lpData);
    for (int i=1; i<times; i++)
    {
        LPBITMAPINFOHEADER tmpMAP2=_maxmap_base(tmpMAP);
        free(tmpMAP); tmpMAP=tmpMAP2;
        if ((tmpMAP->biWidth<3) || (tmpMAP->biHeight<3))
        {
            times=i;
            break;
        }
    }
    TVFrame retF; retF.lpBMIH=tmpMAP; retF.lpData=NULL;
    for (int i=0; i<times; i++)
    {
        _enlarge(&retF);
    }
    ASSERT(retF.lpData==NULL);
    return retF.lpBMIH;
}

__forceinline LPBITMAPINFOHEADER _minmap(int times, LPBITMAPINFOHEADER srcbmp, LPVOID lpData=NULL)
{
    if ((times<1) || (times>10)) return NULL;
    LPBITMAPINFOHEADER tmpMAP=_maxmap_base(srcbmp,lpData);
    for (int i=1; i<times; i++)
    {
        LPBITMAPINFOHEADER tmpMAP2=_minmap_base(tmpMAP);
        free(tmpMAP); tmpMAP=tmpMAP2;
        if ((tmpMAP->biWidth<3) || (tmpMAP->biHeight<3))
        {
            times=i;
            break;
        }
    }
    TVFrame retF; retF.lpBMIH=tmpMAP; retF.lpData=NULL;
    for (int i=0; i<times; i++)
    {
        _enlarge(&retF);
    }
    ASSERT(retF.lpData==NULL);
    return retF.lpBMIH;
}

inline void _enchance3(pTVFrame frame, int times=CONOVOLUTION_PARAM/100)
{
    LPBITMAPINFOHEADER dstBMP=NULL;
    if (times<1) times=1;
    if (times>10) times=10;
    LPBITMAPINFOHEADER minBMP=_minmap(times,frame->lpBMIH,frame->lpData);
    LPBITMAPINFOHEADER maxBMP=_maxmap(times,frame->lpBMIH,frame->lpData);
    
    ASSERT(minBMP!=NULL);
    ASSERT(maxBMP!=NULL);

    int mapW=minBMP->biWidth;            
    int mapH=minBMP->biHeight;
    ASSERT(minBMP->biWidth==mapW);
    ASSERT(minBMP->biHeight==mapH);
    int orgW=frame->lpBMIH->biWidth;
    int orgH=frame->lpBMIH->biHeight;
    int datasize=orgW*orgH;
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
        {
            dstBMP=(LPBITMAPINFOHEADER)malloc(getBMIHsize(frame->lpBMIH));
            memcpy(dstBMP,frame->lpBMIH,frame->lpBMIH->biSize);
            if (frame->lpBMIH->biCompression==BI_YUV9)
                memcpy(GetData(dstBMP)+datasize,GetData(frame)+datasize,datasize/8);
            if (frame->lpBMIH->biCompression==BI_YUV12)
                memcpy(GetData(dstBMP)+datasize,GetData(frame)+datasize,datasize/2);

            LPBYTE minsrc=(LPBYTE)GetData(minBMP);
            LPBYTE maxsrc=(LPBYTE)GetData(maxBMP);
            LPBYTE src=GetData(frame);
            LPBYTE dst=GetData(dstBMP);
            for (int y=0; y<orgH; y++)
            {
                for (int x=0; x<orgW; x++)
                {
                    int mapX=(x>=mapW)?mapW-1:x;
                    int mapY=(y>=mapH)?mapH-1:y;
                    int min=minsrc[mapX+mapY*mapW];
                    int max=maxsrc[mapX+mapY*mapW];
                    int res;
                    if (max-min)
                        res=(200*(src[x+y*orgW]-min))/(max-min);
                    else
                        res=src[x+y*orgW];
                    dst[x+y*orgW]=(res<0)?0:(res>255)?255:res;
                }
            }
            break;
        }
    case BI_Y16:
        {
            dstBMP=(LPBITMAPINFOHEADER)malloc(getBMIHsize(frame->lpBMIH));
            memcpy(dstBMP,frame->lpBMIH,frame->lpBMIH->biSize);
            if (frame->lpBMIH->biCompression==BI_YUV9)
                memcpy(GetData(dstBMP)+datasize,GetData(frame)+datasize,datasize/8);
            LPWORD minsrc=(LPWORD)GetData(minBMP);
            LPWORD maxsrc=(LPWORD)GetData(maxBMP);
            LPWORD src=(LPWORD)GetData(frame);
            LPWORD dst=(LPWORD)GetData(dstBMP);
            for (int y=0; y<orgH; y++)
            {
                for (int x=0; x<orgW; x++)
                {
                    int mapX=(x>=mapW)?mapW-1:x;
                    int mapY=(y>=mapH)?mapH-1:y;
                    int min=minsrc[mapX+mapY*mapW];
                    int max=maxsrc[mapX+mapY*mapW];
                    int res;
                    if (max-min)
                        res=(65535*(src[x+y*orgW]-min))/(max-min);
                    else
                        res=src[x+y*orgW];
                    dst[x+y*orgW]=(res<0)?0:(res>65535)?65535:res;
                }
            }
            break;
        }
    default:
        ASSERT(FALSE);
    }
    if (dstBMP)
    {
        if (frame->lpData==NULL)
        {
            free(frame->lpBMIH); frame->lpBMIH=dstBMP;
        }
        else
        {
            free(frame->lpData); frame->lpData=NULL; frame->lpBMIH=dstBMP;
        }
    }
    free(minBMP);
    free(maxBMP);
}

#endif