#ifndef ENCHANCE1_INC
#define ENCHANCE1_INC

#include <video\TVFrame.h>
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
    }
}

#endif