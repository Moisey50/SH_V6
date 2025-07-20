//  $File : normalize.h - normalizing intensity distribution
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#ifndef _NORMALIZE_INC
#define _NORMALIZE_INC
#include <imageproc\dibutils.h>
#include <imageproc\simpleip.h>

template <typename T> T median(T a, T b, T c)
{
	if ((a < b && a > c) || (a > b && a < c))
		return a;
	if ((b < a && b > c) || (b > a && b < c))
		return b;
	return c;
}

template <typename T> T median(T a, T b, T c, T d, T e)
{
	T items[5] = { a, b, c, d, e }, tmp;
	int i;
	for (i = 0; i < 4; i++)
	{
		if (items[i] > items[i + 1])
		{
			tmp = items[i];
			items[i] = items[i + 1];
			items[i + 1] = tmp;
		}
	}
	for (i = 3; i > 0; i--)
	{
		if (items[i] < items[i - 1])
		{
			tmp = items[i];
			items[i] = items[i - 1];
			items[i - 1] = tmp;
		}
	}
	return median(items[1], items[2], items[3]);
}

__forceinline void _normalizeB(LPBYTE Data, int size, int MaxInit=0)
{
    ASSERT(Data);

    unsigned Max,Min;
    Max=MaxInit; Min=255;
    LPBYTE from=Data+300, to=Data+size;

    while (from<to)
	{
		Max=(Max>*from)?Max:*from;
		Min=((Min<*from)&&(*from!=0))?Min:*from;
        from++;
	}
	if ((Max-Min)<=0) return;
    from=Data;
    double k=255.0/(Max-Min);
	while (from<to)
	{
		*from=(unsigned char)(k*(*from-Min));
        from++;
	}    
}

__forceinline void _normalizeW(LPWORD Data, int size, int MaxInit=0)
{
    ASSERT(Data);

    unsigned Max,Min;
    Max=MaxInit; Min=255;
    LPWORD from=Data, to=Data+size;

    while (from<to)
	{
		Max=(Max>*from)?Max:*from;
		Min=((Min<*from)&&(*from!=0))?Min:*from;
        from++;
	}
	if ((Max-Min)<=0) return;
    from=Data;
    double k=65535.0/(Max-Min);
	while (from<to)
	{
		*from=(WORD)(k*(*from-Min));
        from++;
	}    
}

__forceinline void _normalize(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);
    switch (frame->lpBMIH->biCompression)
    {
        case BI_YUV9:
        case BI_Y8:
            _normalizeB(GetData(frame), frame->lpBMIH->biWidth*frame->lpBMIH->biHeight);
            break;
        case BI_Y16:
            _normalizeW((LPWORD)GetData(frame), frame->lpBMIH->biWidth*frame->lpBMIH->biHeight);
    }
   
}


__forceinline void _normalize_bw(pTVFrame frame,pTVFrame black, pTVFrame white)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);
    
    if ((GetImageSize(black)!=GetImageSize(white)) || (GetImageSize(frame)!=GetImageSize(white)))
        return;
    switch (frame->lpBMIH->biCompression)
    {
        case BI_YUV9:
        case BI_Y8:
            {
                LPBYTE fPntr=GetData(frame),wPntr=GetData(white), bPntr=GetData(black);
                LPBYTE eod=fPntr+GetI8Size(frame);
                while (fPntr<eod)
                {
                    int val=*fPntr;
                    int delta=*wPntr-*bPntr;
                    if (delta==0)
                        delta=1;
                    val=((val-*bPntr)*255)/delta;
                    *fPntr=(val>255)?255:val;
                    fPntr++; wPntr++; bPntr++;
                }
            }           
            break;
        case BI_Y16:
            {
                LPWORD fPntr=(LPWORD)GetData(frame),wPntr=(LPWORD)GetData(white), bPntr=(LPWORD)GetData(black);
                LPWORD eod=fPntr+GetI8Size(frame);
                while (fPntr<eod)
                {
                    int val=*fPntr;
                    int delta=*wPntr-*bPntr;
                    if (delta==0)
                        delta=1;
                    val=((val-*bPntr)*65535)/delta;
                    *fPntr=(val>65535)?65535:val;
                    fPntr++; wPntr++; bPntr++;
                }
            }           
    }
   
}

__forceinline void _normalize_b(pTVFrame frame,pTVFrame black)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);
    
    if (GetImageSize(black)!=GetImageSize(frame)) 
        return;
    switch (frame->lpBMIH->biCompression)
    {
        case BI_YUV9:
        case BI_Y8:
            {
                LPBYTE fPntr=GetData(frame), bPntr=GetData(black);
                LPBYTE eod=fPntr+GetI8Size(frame);
                while (fPntr<eod)
                {
                    int val=*fPntr;
                    int delta=255-*bPntr;
                    if (delta==0)
                        delta=1;
                    val=((val-*bPntr)*255)/delta;
                    *fPntr=(val>255)?255:val;
                    fPntr++; bPntr++;
                }
            }           
            break;
        case BI_Y16:
            {
                LPWORD fPntr=(LPWORD)GetData(frame), bPntr=(LPWORD)GetData(black);
                LPWORD eod=fPntr+GetI8Size(frame);
                while (fPntr<eod)
                {
                    int val=*fPntr;
                    int delta=255-*bPntr;
                    if (delta==0)
                        delta=1;
                    val=((val-*bPntr)*65535)/delta;
                    *fPntr=(val>65535)?65535:val;
                    fPntr++; bPntr++;
                }
            }           
    }
   
}

__forceinline void _normalize_w(pTVFrame frame, pTVFrame white)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);
    
    if (GetImageSize(frame)!=GetImageSize(white))
        return;
    switch (frame->lpBMIH->biCompression)
    {
        case BI_YUV9:
        case BI_Y8:
            {
                LPBYTE fPntr=GetData(frame), wPntr=GetData(white);
                LPBYTE eod=fPntr+GetI8Size(frame);
                while (fPntr<eod)
                {
                    int val=*fPntr;
                    int delta=*wPntr;
                    if (delta==0)
                        delta=1;
                    val=(val*255)/delta;
                    *fPntr=(val>255)?255:val;
                    fPntr++; wPntr++; 
                }
            }           
            break;
        case BI_Y16:
            {
                LPWORD fPntr=(LPWORD)GetData(frame),wPntr=(LPWORD)GetData(white);
                LPWORD eod=fPntr+GetI8Size(frame);
                while (fPntr<eod)
                {
                    int val=*fPntr;
                    int delta=*wPntr;
                    if (delta==0)
                        delta=1;
                    val=(val*65535)/delta;
                    *fPntr=(val>65535)?65535:val;
                    fPntr++; wPntr++; 
                }
            }           
    }
   
}


__forceinline void _normalizeUV(LPBYTE Data, int size)
{
    ASSERT(Data);

    int Max;
    Max=0; 
    LPBYTE from=Data, to=Data+size;

    while (from<to)
	{
		Max=(Max>abs(*from-128))?Max:abs(*from-128);
        from++;
	}
	if (Max==0) return;
    from=Data;
    double k=127.0/Max;
	while (from<to)
	{
		*from=(unsigned char)(k*(*from-128)+128);
        from++;
	}    
}

__forceinline void _normalizeUV(pTVFrame Frame)
{
    int datasize=Frame->lpBMIH->biWidth*Frame->lpBMIH->biHeight;
    int UVSize=datasize/16;
    LPBYTE Data=GetData(Frame)+datasize;
    _normalizeUV(Data, 2*UVSize);
}

__forceinline void _normalize(pTVFrame frame, int MaxInit)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    LPBYTE pData;
    pData=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);
    unsigned Max,Min;
    Min=*pData;
    Max=MaxInit;
    LPBYTE from=pData, to=pData+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;

    while (from<to)
	{
		Max=(Max>*from)?Max:*from;
		Min=(Min<*from)?Min:*from;
        from++;
	}
	if ((Max-Min)<=0) return;
    from=pData;
    double k=255.0/(Max-Min);
	while (from<to)
	{
		*from=(unsigned char)(k*(*from-Min));
        from++;
	}    
}


__forceinline void _normalize(pTVFrame frame, RECT* rc, int* MaxInit=NULL)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);
	ASSERT(rc);

	_rectvalid(rc);

    LPBYTE pData;
    pData=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);
    unsigned Max=0,Min=255;
    if (MaxInit)
        Max=*MaxInit;
	int width=frame->lpBMIH->biWidth;
	int height=frame->lpBMIH->biHeight;

	LPBYTE baseoffset=pData+rc->top*width+rc->left;
	LPBYTE endofscan=pData+rc->bottom*width+rc->left;
	LPBYTE from,to;
	while (baseoffset<endofscan)
	{
		from=baseoffset; to=baseoffset+rc->right-rc->left;	
		while (from<to)
		{
			Max=(Max>*from)?Max:*from;
			Min=(Min<*from)?Min:*from;
			from++;
		}
		baseoffset+=width;
	}
    
	if ((Max-Min)<=0) return;
    from=pData;
	to=pData+width*height;
    double k=255.0/(Max-Min);
	while (from<to)
	{
		int val=(int)(k*(*from-Min));
		val=(val<0)?0:((val>255)?255:val);
		*from=val;
        from++;
	}    
}

__forceinline void _normalize_ex(pTVFrame frame, unsigned& max, unsigned& min)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    LPBYTE pData;
    pData=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);
    unsigned Max=0,Min=255;

	int width=frame->lpBMIH->biWidth;
	int height=frame->lpBMIH->biHeight;

	LPBYTE from=pData;
	LPBYTE to=pData+height*width;
	while (from<to)
	{
			Max=(Max>*from)?Max:*from;
			Min=(Min<*from)?Min:*from;
			from++;
	}
    
	if ((Max-Min)<=0) return;
    if (Max>max)
        max=Max;
    //else
    //    max=(Max+3*max)/4;
    if (Min<min)
        min=Min;
    //else
    //    min=(Min+3*min)/4;
    from=pData;
	to=pData+width*height;
    double k=255.0/(max-min);
	while (from<to)
	{
		int val=(int)(k*(*from-min));
		val=(val<0)?0:((val>255)?255:val);
		*from=val;
        from++;
	}    
}

__forceinline void _normalize_ex2(pTVFrame frame, unsigned max, unsigned min)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV9:
    case BI_Y8:
        {
            LPBYTE pData;
            pData=GetData(frame);
            unsigned Max=0,Min=255;

	        int width=frame->lpBMIH->biWidth;
	        int height=frame->lpBMIH->biHeight;

	        LPBYTE from=pData;
	        LPBYTE to=pData+height*width;
	        while (from<to)
	        {
			        Max=(Max>*from)?Max:*from;
			        Min=(Min<*from)?Min:*from;
			        from++;
	        }
    
	        if ((Max-Min)<=0) return;
            if (Max>max)
                Max=max;
            if (Min<min)
                Min=min;
            from=pData;
	        to=pData+width*height;
            double k=255.0/(max-min);
	        while (from<to)
	        {
		        int val=(int)(k*((int)(*from)-(int)min));

		        val=(val<0)?0:((val>255)?255:val);
		        *from=val;
                from++;
	        }    
            break;
        }
    case BI_Y16:
        {
            LPWORD pData;
            pData=(LPWORD)GetData(frame);
            unsigned Max=0,Min=65535;

	        int width=frame->lpBMIH->biWidth;
	        int height=frame->lpBMIH->biHeight;

	        LPWORD from=pData;
	        LPWORD to=pData+height*width;
	        while (from<to)
	        {
			        Max=(Max>*from)?Max:*from;
			        Min=(Min<*from)?Min:*from;
			        from++;
	        }
    
	        if ((Max-Min)<=0) return;
            if (Max>max)
                Max=max;
            if (Min<min)
                Min=min;
            from=pData;
	        to=pData+width*height;
            double k=65535.0/(max-min);
	        while (from<to)
	        {
		        int val=(int)(k*((int)(*from)-(int)min));

		        val=(val<0)?0:((val>65535)?65535:val);
		        *from=val;
                from++;
	        }
            break;
        }
    }
}

__forceinline void _normalizeB_mass(LPBYTE data, unsigned size, int percent)
{
    int hist[256];
    memset(hist,0, sizeof(int)*256);
    LPBYTE sc=data, eod=data+size;
    while (sc<eod)
    {
        hist[*sc]++;
        sc++;
    }
    int up=255, down=0;
    int upV=0, downV=0;
    int v=percent*size/100;
    while (upV+downV<v)
    {
        if (upV>downV)
        {
            downV+=hist[down]; down++;
        }
        else
        {
            upV+=hist[up]; up--;
        }
    }
    sc=data;
    while (sc<eod)
    {
        if (*sc<down)
            *sc=0;
        else if (*sc>=up)
            *sc=255;
        else
            *sc=(*sc-down)*255/(up-down);
        sc++;
    }
}

__forceinline void _normalizeW_mass(LPWORD data, unsigned size, int percent)
{
    int hist[256];
    memset(hist,0, sizeof(int)*256);
    LPWORD sc=data, eod=data+size;
    while (sc<eod)
    {
        hist[(*sc)/256]++;
        sc++;
    }
    int up=255, down=0;
    int upV=0, downV=0;
    int v=percent*size/100;
    while (upV+downV<v)
    {
        if (upV>downV)
        {
            downV+=hist[down]; down++;
        }
        else
        {
            upV+=hist[up]; up--;
        }
    }
    sc=data;
    while (sc<eod)
    {
        if (*sc<down*256)
            *sc=0;
        else if (*sc>=up*256)
            *sc=255;
        else
            *sc=(*sc-down*256)*256/(up-down);
        sc++;
    }
}

__forceinline void _normalize_mass(pTVFrame frame, int percent)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);
    switch (frame->lpBMIH->biCompression)
    {
        case BI_YUV9:
        case BI_Y8:
            _normalizeB_mass(GetData(frame), frame->lpBMIH->biWidth*frame->lpBMIH->biHeight, percent);
            break;
        case BI_Y16:
            _normalizeW_mass((LPWORD)GetData(frame), frame->lpBMIH->biWidth*frame->lpBMIH->biHeight, percent);
    }
   
}

__forceinline void _normalize_exOld(pTVFrame frame)
{
    ASSERT(frame);
    ASSERT(frame->lpBMIH);

    LPBYTE pData;
    pData=(frame->lpData)?frame->lpData:(((LPBYTE)frame->lpBMIH)+frame->lpBMIH->biSize);
	int Max=0,Min=256,tmpJ,i,level;
    int hist[256];
    
    LPBYTE from=pData, to=pData+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
    memset(hist,0,sizeof(int)*256);

	while (from<to)
	{
        if (*from<Min) Min=*from;
        if (*from>Max) Max=*from;
        hist[*from]++;
        from++;
	}
    tmpJ=0; i=0; level=(int)(0.10*frame->lpBMIH->biWidth*frame->lpBMIH->biHeight);
    while ((tmpJ<level) && (i<256))
    {
        tmpJ+=hist[i];
        i++;
    }
    Min=i;
    tmpJ=0; i=0;
    while ((tmpJ<level) && (i<256))
    {
        tmpJ+=hist[255-i];
        i++;
    }
    Max=255-i;
	if ((Max-Min)<=0) Max=Min+1;
    from=pData;
    double k=255.0/(Max-Min);
	while (from<to)
	{
        if (*from<=Min)
            *from=0;
        else if (*from>=Max)
            *from=255;
        else
            *from=(unsigned char)(k*(*from-Min));
        from++;
	}    
}

__forceinline void _normalize_level(pTVFrame frame, DWORD level=0)
{
    LPBYTE Data=GetData(frame);
    DWORD length=frame->lpBMIH->biWidth * frame->lpBMIH->biHeight;
    if (!level)
        level=length/20;
    else
        level=length/level;
    DWORD hist[256];
	memset(hist,0,256*sizeof(DWORD));
    DWORD i=0;
    while(i<length) { hist[Data[i]]++; i++; }
	BYTE min=0,max=255;
	DWORD sum=0;
	while ((sum<level)&&(min<max)) { sum+=hist[min]; min++; }
	sum=0;
	while ((sum<level)&&(max>min)) { sum+=hist[max]; max--; }
	double rate=1;
	if (max>min)
		rate=255./(double)(max-min);
	i=0;
    while(i<length) { Data[i]=(Data[i]<min)?0:((Data[i]>max)?255:(BYTE)((double)(Data[i]-min)*rate)); i++;}
}

__forceinline void _normalize_val(pTVFrame frame, DWORD val=0)
{
    LPBYTE Data=GetData(frame);
    DWORD length=frame->lpBMIH->biWidth * frame->lpBMIH->biHeight;

    DWORD hist[256];
	memset(hist,0,256*sizeof(DWORD));
    DWORD i=0;
    while(i<length) { hist[Data[i]]++; i++; }
	BYTE min=0,max=255;
	while ((hist[min]<val)&&(min<max)) { min++; }
	while ((hist[max]<val)&&(max>min)) { max--; }
	double rate=1;
	if (max>min)
		rate=255./(double)(max-min);
	i=0;
    while(i<length) { Data[i]=(Data[i]<min)?0:((Data[i]>max)?255:(BYTE)((double)(Data[i]-min)*rate)); i++;}
}

__forceinline void _loc_normalize_max(pTVFrame frame, int r=10)
{
    LPBYTE Data=GetData(frame);
    DWORD length=frame->lpBMIH->biWidth * frame->lpBMIH->biHeight;
    int width  = frame->lpBMIH->biWidth;
    int height = frame->lpBMIH->biHeight;
    LPBYTE ref=(LPBYTE)malloc(length);
    memcpy(ref,Data,length);

    memset(Data,0,length);
    
    LPBYTE refsc=ref+(r*width+r);
    LPBYTE dstsc=Data+(r*width+r);
    LPBYTE end=ref+length-(r*width+r);

    while(refsc<end)
    {
        int min=255, max=0;
        for (int offsetY=-r*width; offsetY<=r*width; offsetY+=width)
        {
            for (int offsetX=-r; offsetX<=r; offsetX++)
            {
                if (*(refsc+offsetY+offsetX)<min) min=*(refsc+offsetY+offsetX);
                if (*(refsc+offsetY+offsetX)>max) max=*(refsc+offsetY+offsetX);
            }
        }
        if (max-min)
            *dstsc=((*refsc-min)*255)/(max-min);
        else
            *dstsc=0;
        refsc++;
        dstsc++;
    }
    free(ref);
}

__forceinline void _v_normalize(pTVFrame frame)
{
    LPBYTE data=GetData(frame);
    int width=frame->lpBMIH->biWidth;
    int height=frame->lpBMIH->biHeight;

    LPBYTE sc;
    LPBYTE eos;

    
    for (int x=0; x<width; x++)
    {
        sc=data+x;
        eos=data+height*width;
        int min=256, max=0;
        while (sc<eos)
        {
            if (*sc<min) min= *sc;
            if (*sc>max) max= *sc;
            sc+=width;
        }
        sc=data+x;
        while (sc<eos)
        {
            if (max-min)
                *sc=(255* (*sc-min))/(max-min);
            else
                *sc=0;
            sc+=width;
        }
    }

}

__forceinline int avgint(LPBYTE off, int w)
{
    LPBYTE o=off;
    int a;
    a=*(o) + *(o+1)+ + *(o+2)+ *(o+3);
    o+=w;
    a+=*(o) + *(o+1)+ + *(o+2)+ *(o+3);
    o+=w;
    a+=*(o) + *(o+1)+ + *(o+2)+ *(o+3);
    o+=w;
    a+=*(o) + *(o+1)+ + *(o+2)+ *(o+3);
    return (a/8);
}

__forceinline void colorbalance2(LPBYTE data, int width, int height)
{
    LPBYTE U,V,EOD;
    int umedian=0,vmedian=0;
    int size=width*height;
    ASSERT(size%16==0);
    int csize=size/16;
    LPBYTE d=(LPBYTE)malloc(csize);
    LPBYTE scd=d,scs=data;
    int cMax=0;
    while(scd<d+csize)
    {
        for (int i=0; i<width/4; i++)
        {
            *scd=avgint(scs,width);
            if (cMax<(*scd)) cMax=*scd;
            scd++;
            scs+=4;
        }
        scs+=3*width;
    }
    U=data+width*height;
    EOD=V=U+csize;
    scd=d;
    int cnt=0;
    while (U<EOD)
    {
        if ((*scd)>(2*cMax/3))
        {
            umedian+=*U;
            vmedian+=*V;
            cnt++;
        }
        U++; V++; scd++;
    }

    free(d); d=NULL;    

    if (cnt==0) cnt=1;
    umedian/=cnt;
    vmedian/=cnt;
    umedian-=128;
    vmedian-=128;
    U=data+width*height;
    EOD=V=U+csize;
    TRACE("+++ Color balance. Level=%d, Used %d points. U=%d, V=%d\n", cMax/2, cnt,umedian, vmedian);
    while (U<EOD)
    {
        *U-=umedian;
        *V-=vmedian;
        U++; V++;
    }
}

__forceinline void colorbalance(LPBYTE data, int width, int height)
{
    LPBYTE U,V,EOD;
    int umedian=0,vmedian=0;
    int size=width*height;
    ASSERT(size%16==0);
    int csize=size/16;
    U=data+width*height;
    EOD=V=U+csize;
    while (U<EOD)
    {
            umedian+=*U;
            vmedian+=*V;
        U++; V++; 
    }
    umedian/=csize;
    vmedian/=csize;
    umedian-=128;
    vmedian-=128;
    U=data+width*height;
    EOD=V=U+csize;
    TRACE("+++ Color balance. U=%d, V=%d\n", umedian, vmedian);
    while (U<EOD)
    {
        *U-=umedian;
        *V-=vmedian;
        U++; V++;
    }
}

__forceinline void colorbalance(pTVFrame frame)
{
    if (frame->lpBMIH->biCompression==BI_YUV9)
        colorbalance2(GetData(frame), frame->lpBMIH->biWidth, frame->lpBMIH->biHeight);
}

__forceinline void colorshift(LPBYTE data, int width, int height, int us, int vs)
{
    LPBYTE U,V,EOD;
    int size=width*height;
    ASSERT(size%16==0);
    int csize=size/16;
    U=data+width*height;
    EOD=V=U+csize;
    while (U<EOD)
    {
        *U+=us;
        *V+=vs;
        U++; V++;
    }
}

__forceinline void colorshift(pTVFrame frame, int U, int V)
{
    if (frame->lpBMIH->biCompression==BI_YUV9)
        colorshift(GetData(frame), frame->lpBMIH->biWidth, frame->lpBMIH->biHeight, U, V);
}


__forceinline void _smoothen_hist(int* I, int levels = 256)
{
	int* J = new int[levels];
	memcpy(J, I, sizeof(int) * levels);

	for (int i = 2; i < levels - 2; i++)
		I[i] = median(J[i - 2], J[i - 1], J[i], J[i + 1], J[i + 2]);
	I[1] = median(J[0], J[1], J[2]);
	I[levels - 2] = median(J[levels - 3], J[levels - 2], J[levels - 1]);
	if (I[0] > I[1])
		I[0] = I[1];
	if (I[levels - 1] > I[levels - 2])
		I[levels - 1] = I[levels - 2];

	delete[] J;
}

__forceinline void _equalize_hist(LPBYTE data, int size, bool bSmoothen = true)
{
	int I[256];
	memset(I, 0, sizeof(I));

	int i = 0;
	while (i < size)
		I[data[i++]]++;

	if (bSmoothen)
		_smoothen_hist(I);

	i = 1;
	while (i < 256)
	{
		I[i] += I[i - 1];
		i++;
	}

	i = 0;
	while (i < size)
	{
		data[i] = (BYTE)(255 * (double)I[data[i]] / (double)I[255]);
		i++;
	}
}

__forceinline void _equalize_hist16(LPWORD data, int size, bool bSmoothen = true)
{
	int* I = new int[256];
	memset(I, 0, sizeof(int)*256);

	int i = 0;
	while (i < size)
		I[(data[i++]>>8)]++;

	if (bSmoothen)
		_smoothen_hist(I);

	i = 1;
	while (i < 256)
	{
		I[i] += I[i - 1];
		i++;
	}

	i = 0;
	while (i < size)
	{
		data[i] = (WORD)(65535 * (double)I[(data[i]>>8)] / (double)I[255]);
		i++;
	}
    delete [] I;
}

__forceinline void _equalize_hist(pTVFrame frame, bool bSmoothen = true)
{
    switch (frame->lpBMIH->biCompression)
    {
        case BI_YUV9:
        case BI_Y8:
            _equalize_hist(GetData(frame),GetI8Size(frame), bSmoothen);
            break;
        case BI_Y16:
            _equalize_hist16((LPWORD)GetData(frame),GetI8Size(frame), bSmoothen);
            break;
    }
}

__forceinline void _equalize_color(pTVFrame frame, bool bSmoothen = true)
{
	int U[256];
	int V[256];
	memset(U, 0, sizeof(U));
	memset(V, 0, sizeof(V));

	int size = frame->lpBMIH->biWidth * frame->lpBMIH->biHeight, i = 0;
	LPBYTE srcU = GetData(frame) + size;
	LPBYTE srcV = srcU + size / 16;

	while (i < size / 16)
	{
		U[srcU[i]]++;
		V[srcV[i]]++;
		i++;
	}

	if (bSmoothen)
	{
		_smoothen_hist(U);
		_smoothen_hist(V);
	}

	i = 1;
	while (i < 256)
	{
		U[i] += U[i - 1];
		V[i] += V[i - 1];
		i++;
	}

	double rateU = (U[128] < U[255] / 2) ? 127.5 / (double)(U[255] - U[128]) : 127.5 / (double)U[128];
	double rateV = (V[128] < V[255] / 2) ? 127.5 / (double)(V[255] - V[128]) : 127.5 / (double)V[128];
	i = 0;
	while (i < size / 16)
	{
		srcU[i] = (BYTE)(127.5 + (double)(U[srcU[i]] - U[128]) * rateU);
		srcV[i] = (BYTE)(127.5 + (double)(V[srcV[i]] - V[128]) * rateV);
		i++;
	}
}

__forceinline void _minus_black(pTVFrame frame,pTVFrame black)
{
  ASSERT(frame);
  ASSERT(frame->lpBMIH);

  if (GetImageSize(black)!=GetImageSize(frame)) 
    return;
  switch (frame->lpBMIH->biCompression)
  {
  case BI_YUV9:
  case BI_Y8:
    {
      LPBYTE fPntr=GetData(frame), bPntr=GetData(black);
      LPBYTE eod=fPntr+GetI8Size(frame);
      while (fPntr<eod)
      {
        *fPntr = (*fPntr >= *bPntr) ? *fPntr - *bPntr : 0 ;
        fPntr++; bPntr++;
      }
    }           
    break;
  case BI_Y16:
    {
      LPWORD fPntr=(LPWORD)GetData(frame), bPntr=(LPWORD)GetData(black);
      LPWORD eod=fPntr+GetI8Size(frame);
      while (fPntr<eod)
      {
        *fPntr = (*fPntr >= *bPntr) ? *fPntr - *bPntr : 0 ;
        fPntr++; bPntr++;
      }
    }           
  }

}


#endif //_NORMALIZE_INC