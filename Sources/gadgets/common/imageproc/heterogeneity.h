#ifndef HETEROGENEITY_INC
#define HETEROGENEITY_INC

#include <video\TVFrame.h>
#include <imageproc\Statistics.h>

__forceinline pTVFrame _FindRoughness(pTVFrame src)
{
    pTVFrame dst=makecopyTVFrame(src);
    int hist[256];
    int maxV, maxPos;
    GetHistogram(src, hist,maxV,maxPos);
    LPBYTE ss=GetData(src);
    LPBYTE es=ss+src->lpBMIH->biWidth*dst->lpBMIH->biHeight;
    LPBYTE sd=GetData(dst);
    LPBYTE ed=sd+dst->lpBMIH->biWidth*dst->lpBMIH->biHeight;
    while (ss<es)
    {
        double val=( ((double)maxV-hist[*ss]) /(double)maxV );
        *sd=(unsigned char)(255* val*val*val*val);
        ss++;
        sd++;
    }
    _clear_frames(dst,0,3);
    return dst;
}

__forceinline pTVFrame _markRoughness(pTVFrame src)
{
    pTVFrame dst=makecopyTVFrame(src);
    int hist[256];
    int maxV, maxPos;
    GetHistogram(src, hist,maxV,maxPos);
    LPBYTE ss=GetData(src);
    LPBYTE es=ss+src->lpBMIH->biWidth*dst->lpBMIH->biHeight;
    LPBYTE sd=GetData(dst);
    LPBYTE ed=sd+dst->lpBMIH->biWidth*dst->lpBMIH->biHeight;
    while (ss<es)
    {
        double val=abs(maxPos-*ss);
        *sd=(unsigned char)(val);
        ss++;
        sd++;
    }
    _normalize(dst);
    _clear_frames(dst,0,3);

    return dst;
}

__forceinline pTVFrame _markCircleCenters(pTVFrame src, int radius)
{
	int width = src->lpBMIH->biWidth;
	int size = width * src->lpBMIH->biHeight;

    pTVFrame dst=makecopyTVFrame(src);
	memset(GetData(dst), 0, size);

	int offset[8] = {	-radius*width, (int)(-radius/1.4142)*(width-1), radius,
						(int)(radius/1.4142)*(width+1), radius*width,
						(int)(radius/1.4142)*(width-1), -radius,
						(int)(-radius/1.4142)*(width+1) };
	int maxSum = 0;

	LPBYTE ss = GetData(src) - offset[0];
	LPBYTE es = GetData(src) + width * src->lpBMIH->biHeight - offset[4];
	LPBYTE sd = GetData(dst) - offset[0];
	while (ss < es)
	{
		int sum = (int)(*(ss + offset[0]) + *(ss + offset[1]) + *(ss + offset[2]) +
						*(ss + offset[3]) + *(ss + offset[4]) + *(ss + offset[5]) +
						*(ss + offset[6]) + *(ss + offset[7]));
		if (sum == maxSum)
		{
			*sd = 255;
		}
		else if (sum > maxSum)
		{
			memset(GetData(dst), 0, size);
			*sd = 255;
			maxSum = sum;
		}
		ss++;
		sd++;
	}

	return dst;
}



__forceinline pTVFrame _findCircleCentersDraft(pTVFrame src, int radius)
{
	int width = src->lpBMIH->biWidth;
	int size = width * src->lpBMIH->biHeight;

    pTVFrame dst=makecopyTVFrame(src);
	memset(GetData(dst), 0, size);

	int offset[8] = {	-radius*width, (int)(-radius/1.4142)*(width-1), radius,
						(int)(radius/1.4142)*(width+1), radius*width,
						(int)(radius/1.4142)*(width-1), -radius,
						(int)(-radius/1.4142)*(width+1) };

	LPBYTE ss = GetData(src) - offset[0];
	LPBYTE es = GetData(src) + width * src->lpBMIH->biHeight - offset[4];
	LPBYTE sd = GetData(dst) - offset[0];
	while (ss < es)
	{
		int sum = (int)(*(ss + offset[0]) + *(ss + offset[1]) + *(ss + offset[2]) +
						*(ss + offset[3]) + *(ss + offset[4]) + *(ss + offset[5]) +
						*(ss + offset[6]) + *(ss + offset[7]));
		*sd = (BYTE)(sum / 8);
		ss++;
		sd++;
	}

	return dst;
}

__forceinline pTVFrame _findCircleCenters(pTVFrame src, int radius)
{
    int nmbPnts8=(int)(radius/1.4142);
    int nmbPnts=8*nmbPnts8;
	int width = src->lpBMIH->biWidth;
	int size = width * src->lpBMIH->biHeight;
    int i,sum; 

    pTVFrame dst=makecopyTVFrame(src);
	memset(GetData(dst), 0, size);

    int *offsets=new int[nmbPnts8];
    for (i=0; i<nmbPnts8; i++)
    {
        offsets[i] = (int)(sqrt((double)radius*radius-i*i)+0.5);
    }

	LPBYTE ss = GetData(src) + radius*width;
	LPBYTE es = GetData(src) + width * src->lpBMIH->biHeight - radius*width;
	LPBYTE sd = GetData(dst) + radius*width;

	while (ss < es)
	{
        sum=0;
        for (i=0; i<nmbPnts8; i++)
        {
            sum += *(ss - width*i - offsets[i])+
                   *(ss + width*i - offsets[i])+
                   *(ss - width*i + offsets[i])+
                   *(ss + width*i + offsets[i])+
                   *(ss + width*offsets[i]-i)+
                   *(ss - width*offsets[i]-i)+
                   *(ss + width*offsets[i]+i)+
                   *(ss - width*offsets[i]+i);
        }
		*sd = (BYTE)(sum / nmbPnts);
		ss++;
		sd++;
    }

    delete [] offsets;
	return dst;
}

__forceinline void ringing(LPBYTE d, int w, int h)
{
    LPBYTE src=(LPBYTE)malloc(w*h);
    int x,y;

    memcpy(src,d,w*h);
    memset(d,0,w*h);
    int lev=0;

    for (y=0; y<h; y++)
    {
        src[y*w]=(src[y*w]+src[y*w+1])/2;
        lev+=abs(src[y*w]-src[y*w+1]);
        src[y*w+w-1]=(src[y*w+w-1]+src[y*w+w-2])/2;
        lev+=abs(src[y*w+w-1]-src[y*w+w-2]);
    }
    lev/=h;
    for (y=0; y<h; y++)
    {
        double a=((double)(src[y*w+w-1]-src[y*w]))/w;
        for (x=0; x<w; x++)
        {
            d[w*y+x]=(unsigned char)fabs(
                a*(double)x+
                (double)src[y*w]-
                (double)src[y*w+x]);
            if (d[w*y+x]<lev) d[w*y+x]=0;
        }
    }
    _lpass(w,h,d,src,500);
    memcpy(d,src,w*h);
    free(src);
    _normalizeB(d,w*h);
    _simplebinarize8(d,w*h,50);
}

__forceinline void ringingmsk(LPBYTE d, LPBYTE msk, int w, int h)
{
    LPBYTE src=(LPBYTE)malloc(w*h);
    int x,y;
    _normalizeB(d,w*h);
    memcpy(src,d,w*h);
    memset(d,0,w*h);
    int lev=0;

    for (y=0; y<h; y++)
    {
        x=0;
        while ((msk[y*w+x]==0) && (x<w-1)) x++;
        if (x<1) 
        {
            src[y*w]=(src[y*w]+src[y*w+1])/2;
            lev+=abs(src[y*w]-src[y*w+1]);
        }
        else
        {
            src[y*w]=(src[y*w+x-1]+src[y*w+x])/2;
            lev+=abs(src[y*w+x-1]-src[y*w+x]);
        }
        x=w-1;
        while ((msk[y*w+x]==0) && (x>0)) x--;
        if (x>=w-1)
        {
            src[y*w+w-1]=(src[y*w+w-1]+src[y*w+w-2])/2;
            lev+=abs(src[y*w+w-1]-src[y*w+w-2]);
        }
        else
        {
            src[y*w+w-1]=(src[y*w+x]+src[y*w+x+1])/2;
            lev+=abs(src[y*w+x]-src[y*w+x+1]);
        }
    }
    lev/=h;
    for (y=0; y<h; y++)
    {
        double a=((double)(src[y*w+w-1]-src[y*w]))/w;
        for (x=0; x<w; x++)
        {
            d[w*y+x]=(unsigned char)fabs(
                a*(double)x+
                (double)src[y*w]-
                (double)src[y*w+x]);
            if (d[w*y+x]<lev) d[w*y+x]=0;
        }
    } 
    _lpass(w,h,d,src,500);
    memcpy(d,src,w*h);
    free(src);
    _normalizeB(d,w*h);
    //_simplebinarize(d,w*h,10); 
}


__forceinline void ringing(pTVFrame f)
{
    LPBYTE d=GetData(f);
    int w=f->lpBMIH->biWidth;
    int h=f->lpBMIH->biHeight;
    ringing(d,w,h);
}

__forceinline void ringing0(LPBYTE d, int w, int h)
{
    LPBYTE src=(LPBYTE)malloc(w*h);
    int x,y;

    memcpy(src,d,w*h);
    memset(d,0,w*h);
    int lev=0;

    for (y=0; y<h; y++)
    {
        src[y*w]=(src[y*w]+src[y*w+1])/2;
        lev+=abs(src[y*w]-src[y*w+1]);
        src[y*w+w-1]=(src[y*w+w-1]+src[y*w+w-2])/2;
        lev+=abs(src[y*w+w-1]-src[y*w+w-2]);
    }
    lev/=h;
    for (y=0; y<h; y++)
    {
        double a=((double)(src[y*w+w-1]-src[y*w]))/w;
        for (x=0; x<w; x++)
        {
            d[w*y+x]=(unsigned char)fabs(
                a*(double)x+
                (double)src[y*w]-
                (double)src[y*w+x]);
            if (d[w*y+x]<lev) d[w*y+x]=0;
        }
    }
    _lpass(w,h,d,src,500);
    memcpy(d,src,w*h);
    free(src);
    _normalizeB(d,w*h,20);
    _simplebinarize8(d,w*h,50);
}

__forceinline void ringing0(pTVFrame f)
{
    LPBYTE d=GetData(f);
    int w=f->lpBMIH->biWidth;
    int h=f->lpBMIH->biHeight;
    ringing0(d,w,h);
}

#endif //#ifndef HETEROGENEITY_INC