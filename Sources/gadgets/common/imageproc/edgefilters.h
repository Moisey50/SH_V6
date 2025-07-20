#ifndef EDGE_FILETRS
#define EDGE_FILETRS

#include <imageproc\simpleip.h>

__forceinline unsigned char absdif(unsigned char a, unsigned char b)
{
    return ((a<b)?(b-a):(a-b));
}

__forceinline int imax(int a, int b)
{
    return ((a>b)?a:b);
}

__forceinline WORD imax16(WORD a, WORD b)
{
    return ((a>b)?a:b);
}


__forceinline int imin(int a, int b)
{
    return ((a<b)?a:b);
}

__forceinline unsigned* makeSumMatrix(pTVFrame frame)
{
    int size=frame->lpBMIH->biHeight*frame->lpBMIH->biWidth;
    unsigned * retVal = (unsigned *) malloc(size*sizeof(int));

    LPBYTE s=GetData(frame);
    LPBYTE e=s+size;
    unsigned *d=retVal;
    unsigned sum=0;
    while (s<e)
    {
        sum+=*s;
        *d=sum;
        s++; d++;
    }
    return retVal;
}

__forceinline unsigned getSum(int* SumMatrix, POINT lu, POINT br)
{
    ASSERT(SumMatrix!=NULL);
    return 0;
}

__forceinline unsigned char maxdif(unsigned char s, 
                  unsigned char a00, unsigned char a01, unsigned char a02, 
                  unsigned char a10,                    unsigned char a12, 
                  unsigned char a20, unsigned char a21, unsigned char a22)
{
    //return imax(imax(2*absdif(a00,a22)/3,absdif(a01,a21)),2*imax(absdif(a02,a20)/3,absdif(a10,a12)));
    {
                BYTE res0=abs(a00+2*(a01-a21)+a02-a20-a22)/4;
                BYTE res1=abs(a01+2*(a02-a20)+a12-a10-a21)/4;
                BYTE res2=abs(a02+2*(a12-a10)+a22-a00-a20)/4;
                BYTE res3=abs(a12+2*(a22-a00)+a21-a10-a01)/4;
                return imax(imax(res0,res1),imax(res2,res3));
    }
}

__forceinline WORD maxdif16(WORD s, 
                  WORD a00, WORD a01, WORD a02, 
                  WORD a10,           WORD a12, 
                  WORD a20, WORD a21, WORD a22)
{
    //return imax(imax(2*absdif(a00,a22)/3,absdif(a01,a21)),2*imax(absdif(a02,a20)/3,absdif(a10,a12)));
    {
                WORD res0=abs(a00+2*(a01-a21)+a02-a20-a22)/4;
                WORD res1=abs(a01+2*(a02-a20)+a12-a10-a21)/4;
                WORD res2=abs(a02+2*(a12-a10)+a22-a00-a20)/4;
                WORD res3=abs(a12+2*(a22-a00)+a21-a10-a01)/4;
                return imax16(imax16(res0,res1),imax16(res2,res3));
    }
}


__forceinline unsigned char gso_v( 
                  unsigned char a00, unsigned char a01, unsigned char a02, 
                  unsigned char a10, unsigned char a11, unsigned char a12, 
                  unsigned char a20, unsigned char a21, unsigned char a22)
{
        return  abs(-2*(a10+a11+a12)+a00+a01+a02+a20+a21+a22);
}

__forceinline unsigned char gso_h( 
                  unsigned char a00, unsigned char a01, unsigned char a02, 
                  unsigned char a10, unsigned char a11, unsigned char a12, 
                  unsigned char a20, unsigned char a21, unsigned char a22)
{
        return  abs(-2*(a01+a11+a21)+a00+a10+a20+a02+a12+a22);
}

__forceinline unsigned char gso_d( 
                  unsigned char a00, unsigned char a01, unsigned char a02, 
                  unsigned char a10, unsigned char a11, unsigned char a12, 
                  unsigned char a20, unsigned char a21, unsigned char a22)
{
        return  abs(a00+a22-a20-a02);
}



__forceinline void _gso_diagonal(pTVFrame frame)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
        *out=gso_d( *(in-width-1),
                    *(in-width),
                    *(in-width+1),
                    *(in-1),
                    *in,
                    *(in+1),
                    *(in+width-1),
                    *(in+width),
                    *(in+width+1));
        in++;
		out++;
    }
    _clear_frames(frame,0,2);
    free(Copy);
}

__forceinline void _gso_And(pTVFrame frame)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
        *out=imin(gso_h( *(in-width-1),
                    *(in-width),
                    *(in-width+1),
                    *(in-1),
                    *in,
                    *(in+1),
                    *(in+width-1),
                    *(in+width),
                    *(in+width+1)),
                    imin(gso_v( *(in-width-1),
                        *(in-width),
                        *(in-width+1),
                        *(in-1),
                        *in,
                        *(in+1),
                        *(in+width-1),
                        *(in+width),
                        *(in+width+1)),
                        gso_d( *(in-width-1),
                        *(in-width),
                        *(in-width+1),
                        *(in-1),
                        *in,
                        *(in+1),
                        *(in+width-1),
                        *(in+width),
                        *(in+width+1))
                       )
                  );
        in++;
		out++;
    }
    _clear_frames(frame,0,2);
    free(Copy);
}

__forceinline void _gso_Or(pTVFrame frame)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
        *out=imax(gso_h( *(in-width-1),
                    *(in-width),
                    *(in-width+1),
                    *(in-1),
                    *in,
                    *(in+1),
                    *(in+width-1),
                    *(in+width),
                    *(in+width+1)),
                    imax(gso_v( *(in-width-1),
                        *(in-width),
                        *(in-width+1),
                        *(in-1),
                        *in,
                        *(in+1),
                        *(in+width-1),
                        *(in+width),
                        *(in+width+1)),
                        gso_d( *(in-width-1),
                        *(in-width),
                        *(in-width+1),
                        *(in-1),
                        *in,
                        *(in+1),
                        *(in+width-1),
                        *(in+width),
                        *(in+width+1))
                       )
                  );
        in++;
		out++;
    }
    _clear_frames(frame,0,2);
    free(Copy);
}

__forceinline void _gso_horizontal(pTVFrame frame)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
        *out=gso_h( *(in-width-1),
                    *(in-width),
                    *(in-width+1),
                    *(in-1),
                    *in,
                    *(in+1),
                    *(in+width-1),
                    *(in+width),
                    *(in+width+1));
        in++;
		out++;
    }
    _clear_frames(frame,0,2);
    free(Copy);
}


__forceinline void _gso_vertical(pTVFrame frame)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
        *out=gso_v( *(in-width-1),
                    *(in-width),
                    *(in-width+1),
                    *(in-1),
                    *in,
                    *(in+1),
                    *(in+width-1),
                    *(in+width),
                    *(in+width+1));
        in++;
		out++;
    }
    _clear_frames(frame,0,2);
    free(Copy);
}

__forceinline void _edge_detector(LPBYTE Data, int width, int height)
{
    DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
    int max=0;
	while (in < end)
	{
        *out=maxdif(*in,
                    *(in-width-1),
                    *(in-width),
                    *(in-width+1),
                    *(in-1),
                    *(in+1),
                    *(in+width-1),
                    *(in+width),
                    *(in+width+1));
        if (*out>max) max=*out;
        in++;
		out++;
    }
    out = Data + width + 1;
    end = Data + width * (height - 1) - 1;
/*    while (out<end)
    {
        *out=(BYTE)(255*(*out)/max); out++;
    } */
    free(Copy);
}

__forceinline void _edge_detector(pTVFrame frame,double sigma=10)
{
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;
	DWORD size =width * height;
    DWORD csize=0;
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
        csize=size/2;
        break;
    case BI_YUV9:
        csize=size/8;
        break;
    }
    switch (frame->lpBMIH->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
        {
            LPBYTE Data=GetData(frame);
	        LPBYTE Copy = (LPBYTE)malloc(size);
	        memcpy(Copy, Data, size);
            memset(Data,0,size);
            memset(Data+size,128,csize);
	        LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	        LPBYTE out = Data + width + 1;
	        while (in < end)
	        {
                *out=maxdif(*in,
                            *(in-width-1),
                            *(in-width),
                            *(in-width+1),
                            *(in-1),
                            *(in+1),
                            *(in+width-1),
                            *(in+width),
                            *(in+width+1));
                in++;
		        out++;
            }
            free(Copy);
            break;
        }
    case BI_Y16:
        {
            LPWORD Data =(LPWORD)GetData(frame);
	        LPWORD Copy = (LPWORD)malloc(size*sizeof(WORD));
	        memcpy(Copy, Data, size*sizeof(WORD));
            memset(Data,0,size*sizeof(WORD));
	        LPWORD in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	        LPWORD out = Data + width + 1;
	        while (in < end)
	        {
                *out=maxdif16(*in,
                            *(in-width-1),
                            *(in-width),
                            *(in-width+1),
                            *(in-1),
                            *(in+1),
                            *(in+width-1),
                            *(in+width),
                            *(in+width+1));
                in++;
		        out++;
            }
            free(Copy);
            break;
        }
    }
    _clear_frames(frame,0,2);
}

__forceinline void _get_edges_fast(LPBYTE Data, int width, int height)
{
    int size=width*height;
    LPBYTE iErode=(LPBYTE)malloc(size);
    memcpy(iErode,Data,size);
    LPBYTE iDilate=(LPBYTE)malloc(size);
    memcpy(iDilate,Data,size);
    _erode(width,height,iErode,Data);
    _dilate(width,height,iDilate,Data);
    memcpy(Data,iDilate,size);
    _videoXOR(Data,iErode,size);
    free(iErode);
    free(iDilate);
}

__forceinline void _get_edges_fast(pTVFrame frame)
{
    _get_edges_fast(GetData(frame),frame->lpBMIH->biWidth,frame->lpBMIH->biHeight);
/*    pTVFrame fE=makecopyTVFrame(frame);
    _dilate(frame);
    _erode(fE);
    _videoXOR(GetData(frame), GetData(fE), frame->lpBMIH->biWidth*frame->lpBMIH->biHeight);
    freeTVFrame(fE); */
}

__forceinline void _get_edges(pTVFrame frame)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
/*		double v00 = (double)*(in - width - 1), v01 = (double)*(in - width), v02 = (double)*(in - width + 1);
		double v10 = (double)*(in - 1), v11 = (double)*in, v12 = (double)*(in + 1);
		double v20 = (double)*(in + width - 1), v21 = (double)*(in + width), v22 = (double)*(in + width + 1);
*/
		int v00 = *(in - width - 1), v01 = *(in - width), v02 = *(in - width + 1);
		int v10 = *(in - 1), v11 = *in, v12 = *(in + 1);
		int v20 = *(in + width - 1), v21 = *(in + width), v22 = *(in + width + 1);

		double m1 = (v00 + v01 + v02 + v10 + v11 + v12 + v20 + v21 + v22) / 9.;
		double m2 = (v00 * v00 + v01 * v01 + v02 * v02 +
					 v10 * v10 + v11 * v11 + v12 * v12 +
					 v20 * v20 + v21 * v21 + v22 * v22) / 9.;

/*        double m1 = (v00 + v01 + v10 + v11 ) / 4.;
		double m2 = (v00 * v00 + v01 * v01 + 
					 v10 * v10 + v11 * v11 ) / 4.;
*/

		m2 -= m1 * m1;
		if (m2 < 1) m2 = 1;
        int a =16 * ((BYTE)(3.796 * log10(m2)))/* + (BYTE)(m1 / 16.)*/; 
        //*out = a; 
        *out = (a>255)?255:(a<0)?0:a;
		in++;
		out++;
	}
	free(Copy);
}

#define EDGE_DIRECTION_000 0
#define EDGE_DIRECTION_130 1
#define EDGE_DIRECTION_300 2
#define EDGE_DIRECTION_500 3

__forceinline void _get_directed_edges(pTVFrame frame, int direction)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
		int v00 = *(in - width - 1), v01 = *(in - width), v02 = *(in - width + 1);
		int v10 = *(in - 1),         v11 = *in,           v12 = *(in + 1);
		int v20 = *(in + width - 1), v21 = *(in + width), v22 = *(in + width + 1);
        int res=0;
        switch (direction)
        {
        case EDGE_DIRECTION_000:
            {
                res=abs(v00+2*(v01-v21)+v02-v20-v22)/4;
                break;
            }
        case EDGE_DIRECTION_130:
            {
                res=abs(v01+2*v02+v12-v10-2*v20-v21)/4;
                break;
            }
        case EDGE_DIRECTION_300:
            {
                res=abs(v02+2*v12+v22-v00-2*v10-v20)/4;
                break;
            }
        case EDGE_DIRECTION_500:
            {
                res=abs(v12+2*v22+v21-v10-2*v00-v01)/4;
                break;
            }

        }
        *out = res;
		in++;
		out++;
	}
	free(Copy);
}

__forceinline void _get_edges(pTVFrame frame, pTVFrame mask)
{
    _enlarge(mask);
    LPBYTE Data=GetData(frame);
    LPBYTE Mask=GetData(mask);
    
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width +1, end = Copy + width * (mask->lpBMIH->biHeight - 1);
	LPBYTE out = Data + width +1;
	while (in < end)
	{
        if (*Mask)
        {
		    double v00 = *(in - width - 1), v01 = *(in - width), v02 = *(in - width + 1);
		    double v10 = *(in - 1), v11 = *in, v12 = *(in + 1);
		    double v20 = *(in + width - 1), v21 = *(in + width), v22 = *(in + width + 1);

		    double m1 = (v00 + v01 + v02 + v10 + v11 + v12 + v20 + v21 + v22) / 9.;
		    double m2 = (v00 * v00 + v01 * v01 + v02 * v02 +
					     v10 * v10 + v11 * v11 + v12 * v12 +
					     v20 * v20 + v21 * v21 + v22 * v22) / 9.;

		    m2 -= m1 * m1;
		    if (m2 < 1) m2 = 1;
            int a =16 * ((BYTE)(3.796 * log10(m2))); // + (BYTE)(m1 / 16.); 
            *out = (a>255)?255:(a<0)?0:a; 
            //*out=*in;
        }
        else *out =0;
		in++;
        Mask++;
		out++;
	}
    out--;
	free(Copy);
}

__forceinline void _get_object(pTVFrame frame, pTVFrame mask)
{
    _enlarge(mask);
    LPBYTE Data=GetData(frame);
    LPBYTE Mask=GetData(mask);
    
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width +1, end = Copy + width * (mask->lpBMIH->biHeight - 1);
	LPBYTE out = Data + width +1;
	while (in < end)
	{
        if (*Mask)
        {
            *out = 255; 
            //*out=*in;
        }
        else *out =0;
		in++;
        Mask++;
		out++;
	}
    out--;
	free(Copy);
}


__forceinline void _edge_detector(pTVFrame frame, pTVFrame mask, double sigma=10)
{
    _enlarge(mask);
    LPBYTE Data=GetData(frame);
    LPBYTE Mask=GetData(mask);

    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
        if (*Mask)
        {
            *out=maxdif(*in,
                        *(in-width-1),
                        *(in-width),
                        *(in-width+1),
                        *(in-1),
                        *(in+1),
                        *(in+width-1),
                        *(in+width),
                        *(in+width+1));
        }
        else
            *out=0;
        in++;
		out++;
        Mask++;
    }
    _clear_frames(frame,0,2);
    free(Copy);
}


__forceinline void _HEdges(pTVFrame frame, pTVFrame mask)
{
    _enlarge(mask);
    LPBYTE Data=GetData(frame);
    LPBYTE Mask=GetData(mask);
    
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1, mend=Mask+size;;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
        if (*Mask)
        {
            LPBYTE sIn=in;
            LPBYTE sOut=out;
            int max=0;
            int len=0;
            while (*(Mask+len))
            {
                if ((Mask+len)>mend) break;
                len++;
            }
            len--;
            int base=(
                        *in+*(in+len) +
                        *(in+width)+*(in+width+len) +
                        *(in-width)+*(in-width+len)
                     )/6;
            int i=len;
            while (i)
            {
                int d=base-*in;
                d=(d>0)?d:-d;
                //d=(d>255)?255:d;
                *out = (BYTE)(d);
                if (max<*out) max=*out;
                i--;
		        in++;
                Mask++;
		        out++;
            }
            while(len)
            {
                if (max)
                    *sOut = (int)((255*(*sOut))/max);
                else
                    *sOut=0;
                len--;
		        sOut++;
            } 
        }
        else *out =0;
		in++;
        Mask++;
		out++;
	}
	free(Copy);
}


__forceinline int difp(unsigned char* a, unsigned char* b)
{
    int i=(int)*a-*b;
    return (i);
}

__forceinline int iabs(int i) { return ((i<0)?-i:i);}

__forceinline unsigned char __max_hill_0(unsigned char* pntr, int width, int kernel)
{
    int maxV = imax(
                   iabs(difp(pntr,pntr-kernel)-difp(pntr,pntr+kernel)),
                   iabs(difp(pntr,pntr-kernel*width)-difp(pntr,pntr+kernel*width))
                   );
    double maxV2 = imax(
                   iabs(difp(pntr,pntr-kernel*(width+1))-difp(pntr,pntr+kernel*(width+1))),
                   iabs(difp(pntr,pntr-kernel*(width-1))-difp(pntr,pntr+kernel*(width-1)))
                   );

    maxV2=(int)(maxV2/1.414+0.5);
    return ((maxV>maxV2)?maxV:(int)maxV2);
}


__forceinline int difp3(unsigned char* a, unsigned char* b, unsigned char* c)
{
    int r=(*b+*(b+1)+*(b-1))/3;
    int l=(*a+*(a+1)+*(a-1))/3;
    int m=(*c+*(c+1)+*(c-1))/3;
    int i=r-(l+m)/2;
    return (i);
}

__forceinline unsigned char __max_hill(unsigned char* pntr, int width, int kernel)
{
    int maxV = iabs(
                   iabs(difp3(pntr-kernel,pntr, pntr+kernel))-
                   iabs(difp3(pntr-kernel*width,pntr,pntr+kernel*width))
                   );
    int maxV2 = iabs(
                   iabs(difp3(pntr-kernel*(width+1),pntr,pntr+kernel*(width-1)))-
                   iabs(difp3(pntr-kernel*(width-1),pntr,pntr+kernel*(width+1)))
                   );
    return ((maxV>maxV2)?maxV:maxV2);
}

__forceinline unsigned char __max_hill_2(unsigned char* pntr, int width, int kernel)
{
    LPBYTE sc;
    int acc=0;
    for (int y=-kernel; y<=kernel; y++)
    {
        sc=pntr+y*width;
        for (int x=-kernel; x<=kernel; x++)
        {
            if ((x==0) && (y==0)) continue;
            acc+=*(sc+x);
        }

    }
    acc/=(2*kernel+1)*(2*kernel+1)-1;
    return (iabs(*pntr-acc));
}

__forceinline void _hills_detector(pTVFrame frame,int kernel=3)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width*kernel + kernel+1, 
           end = Copy + width * (height - kernel-1) - kernel-1;
	LPBYTE out = Data + width*kernel + kernel+1;
	while (in < end)
	{
        int val=__max_hill_2(in,width,kernel);
        ASSERT(val<256);
        *out=val;
        in++;
		out++;
    }
    _clear_frames(frame,0,kernel);
    free(Copy);
}

__forceinline void _v_hills_detector(pTVFrame frame,int kernel=3)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;
	DWORD size =width * height;

    LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
    
    for (DWORD x=0; x<width; x++)
    {
        for (DWORD y=kernel; y<height-3*kernel; y++)
        {
            int a=0, b=0,am=0,bm=0, d0, d1;
            int r=Copy[x+y*width];
            for (int d=1; d<=kernel; d++)
            {
                int a0=Copy[x+(y-d)*width];
                if (am<iabs(a0-r)) { am=iabs(a0-r); a=a0; d0=d;}
                int b0=Copy[x+(y+d)*width];
                if (bm<iabs(b0-r)) { bm=iabs(b0-r); b=b0; d1=d;}
            }
            //int res=(a-r)*(r-b);
            // Data[x+y*width]=(res<0)?0: (BYTE)sqrt(res);
            //int res=((r-a)*(r-b)/*(d0+d1)*/);
            int res=((a-r)*(r-b));
            Data[x+y*width]=(res<0)?0:res/256; 
//            ASSERT(Data[x+y*width]<100);
/*            int a0=Copy[x+(y-kernel)*width];
            int b0=Copy[x+(y+kernel)*width];
            int res=(a-r)*(r-b);
            Data[x+y*width]=(res<0)?0:sqrt(res); */
        }
    }
    free(Copy);
}

__forceinline void _hills_detector_q(pTVFrame frame, int kernel = 3)
{
	LPBYTE Data = GetData(frame);
	DWORD width = frame->lpBMIH->biWidth;
	DWORD height = frame->lpBMIH->biHeight;
	DWORD size = width * height;

	int* Cols = (int*)calloc(size, sizeof(int));
	int acc, ksize = 2 * kernel + 1;
	int x,y;
	for (x = 0; x < (int)width; x++)
	{
		acc = 0;
		int* ptr = Cols + x;
		for (y = 0; y < ksize; y++)
			acc += (int)Data[y * width + x];
		*ptr = acc;
		ptr+=width;
		while (y < (int)height)
		{
			acc -= (int)Data[(y - ksize) * width + x];
			acc += (int)Data[y * width + x];
			*ptr = acc;
			ptr+=width;
			y++;
		}
	}

	LPBYTE Dst = Data + width * kernel + kernel;
	LPBYTE End = Data + size - width * kernel - kernel - 1;
	int* Src = Cols;

	int kerSize = (2 * kernel + 1) * (2 * kernel + 1) - 1;
	acc = 0;
	for (x = 0; x < ksize; x++)
		acc += Src[x];

	*Dst = (BYTE)iabs(*Src - (acc - *Src) / kerSize);

	while (Dst < End)
	{
		Dst++;
		acc -= *Src;
		acc += *(Src + ksize);
		Src++;
		*Dst = (BYTE)iabs(*Dst - (acc - *Dst) / kerSize);
	}

	free(Cols);
	_clear_frames(frame, 0, kernel);
}


__forceinline void _borders_detector(pTVFrame frame)
{
	LPBYTE Data = GetData(frame);
	DWORD width = frame->lpBMIH->biWidth;
	DWORD height = frame->lpBMIH->biHeight;
	DWORD size = width * height;

	LPBYTE Copy1 = (LPBYTE)malloc(size);
	memset(Copy1, 0, size);
	LPBYTE Copy2 = (LPBYTE)malloc(size);
	memset(Copy2, 0, size);
	DWORD x,y;
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			DWORD index = y * width + x;
			int valh = (int)Data[index] - ((int)Data[index+1] + (int)Data[index-1]) / 2;
			valh /= 4;
			Copy1[index] += (BYTE)(valh);
			if (valh > 0)
			{
				Copy1[index-width-1] += (BYTE)(valh / 8);
				Copy1[index-width+1] += (BYTE)(valh / 8);
				Copy1[index+width-1] += (BYTE)(valh / 8);
				Copy1[index+width+1] += (BYTE)(valh / 8);
				Copy1[index-width] += (BYTE)(valh / 4);
				Copy1[index+width] += (BYTE)(valh / 4);
			}
			int valv = (int)Data[index] - ((int)Data[index+width] + (int)Data[index-width]) / 2;
			valv /= 4;
			Copy2[index] += (BYTE)(valv);
			if (valv > 0)
			{
				Copy2[index-width-1] += (BYTE)(valv / 8);
				Copy2[index-width+1] += (BYTE)(valv / 8);
				Copy2[index+width-1] += (BYTE)(valv / 8);
				Copy2[index+width+1] += (BYTE)(valv / 8);
				Copy2[index-1] += (BYTE)(valv / 4);
				Copy2[index+1] += (BYTE)(valv / 4);
			}
		}
	}

	for (y = 0; y < size; y++)
		Data[y] = (BYTE)iabs((int)Copy1[y] - (int)Copy2[y]);

	free(Copy1);
	free(Copy2);
}

__forceinline void _borders_detector1(pTVFrame frame)
{
	LPBYTE Data = GetData(frame);
	DWORD width = frame->lpBMIH->biWidth;
	DWORD height = frame->lpBMIH->biHeight;
	DWORD size = width * height;

	LPBYTE Copy = (LPBYTE)malloc(size);
	memset(Copy, 0, size);

	for (DWORD y = 1; y < height - 1; y++)
	{
		for (DWORD x = 1; x < width - 1; x++)
		{
			DWORD index = y * width + x;
			int val = (Data[index-1] + Data[index+1] - Data[index-width] - Data[index+width]);
			Copy[index] = (BYTE)((val + 510) / 4);
		}
	}

	memcpy(Data, Copy, size);
	free(Copy);
}


#endif // EDGE_FILETRS