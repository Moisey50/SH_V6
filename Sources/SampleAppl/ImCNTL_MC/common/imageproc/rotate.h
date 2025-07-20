#ifndef ROTATE_INC
#define ROTATE_INC

#include <math.h>
#include <video\TVFrame.h>
#include <imageproc\simpleip.h>

#ifndef PI
    #define PI			(3.1415926535897932384626433832795)
#endif

#define RAD2DEG(x)	((x*180.)/PI)

__forceinline void _rotate(double& x, double& y, double cosA, double sinA)
{
	double X = x * cosA - y * sinA;
	y = x * sinA + y * cosA;
	x = X;
}

__forceinline void _min_max(double& min, double& max, double val)
{
	if (min > val) min = val;
	if (max < val) max = val;
}


__forceinline void _rotate_frame(pTVFrame framesrc,double angle)
{
    TVFrame dstframe     = {NULL,NULL};
    LPBYTE datasrc  = GetData(framesrc);
    int widthsrc    = framesrc->lpBMIH->biWidth;
    int heightsrc   = framesrc->lpBMIH->biHeight;
    double cosA = cos(angle), sinA = sin(angle);
	double xMin = 0, xMax = xMin, yMin = 0, yMax = yMin;

    double x = (double)widthsrc, y = 0;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);
	
    x = 0; y = (double)heightsrc;
	_rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

    x = (double)widthsrc; y = (double)heightsrc;
	_rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

	int up = (int)yMin, dn = (int)yMax;
	int lt = (int)xMin, rt = (int)xMax;
	int width = (rt - lt + 4) / 4 * 4;
	int height = (dn - up + 4) / 4 * 4;
	int size = width * height;
    dstframe.lpBMIH=(LPBITMAPINFOHEADER)malloc(framesrc->lpBMIH->biSize+(9*size)/8);
    memcpy(dstframe.lpBMIH,framesrc->lpBMIH,framesrc->lpBMIH->biSize);
    LPBYTE dstdata = ((LPBYTE)dstframe.lpBMIH)+framesrc->lpBMIH->biSize;
    dstframe.lpBMIH->biWidth     = width;
    dstframe.lpBMIH->biHeight    = height;
    dstframe.lpBMIH->biSizeImage =(9*size)/8;
    memset(dstdata,0,size);
    memset(dstdata+size,128,size/8);

    int i, j, dst = -1;
    dn = up + height; rt = lt + width;

	for (j = up; j < dn; j++)
	{
		for (i = lt; i < rt; i++)
		{
			dst++;
			x = (double)i; y = (double)j;
			_rotate(x, y, cosA, -sinA);
			int xsrc = (int)x, ysrc = (int)y;
			if (xsrc < 0 || xsrc >= framesrc->lpBMIH->biWidth|| ysrc < 0 || ysrc >= framesrc->lpBMIH->biHeight)
				continue;
			int src = xsrc + ysrc * framesrc->lpBMIH->biWidth;
			*(dstdata+dst) = *(datasrc+src);
		}
	}

    if (framesrc->lpData) free(framesrc->lpData); framesrc->lpData=NULL;
    free(framesrc->lpBMIH); framesrc->lpBMIH=dstframe.lpBMIH;
}

__forceinline void _fliph(pTVFrame frame)
{
    switch(frame->lpBMIH->biCompression)
    {
	case BI_RGB:
		switch( frame->lpBMIH->biBitCount)
		{
		case 24:
			{
				LPBYTE datasrc  = GetData(frame);
				int width    = (frame->lpBMIH->biWidth * (frame->lpBMIH->biBitCount / 8) + 3) & ~3;
				int height   = frame->lpBMIH->biHeight;
				int fsize=width*height;
				LPBYTE datacpy=(LPBYTE)malloc(fsize);
				LPBYTE eod=datacpy+fsize;
				memcpy(datacpy,datasrc,fsize);
				LPBYTE dstI=datasrc+fsize-width;
				LPBYTE srcI=datacpy;
				memcpy(datasrc,datacpy,fsize); 
				while(srcI<eod)
				{
					memcpy(dstI,srcI,width);
					dstI-=width;
					srcI+=width;
				}
				free(datacpy);
				return;
			}
		default:
			 TRACE("Error: Non supported videoformat in _fliph function\n");
		}
        case BI_YUV9:
        {
            LPBYTE datasrc  = GetData(frame);
            int width    = frame->lpBMIH->biWidth;
            int height   = frame->lpBMIH->biHeight;
            int isize=width*height;
            int fsize=(9*isize)/8;
            int cwidth=width/4;
            int csize=isize/16;
            LPBYTE datacpy=(LPBYTE)malloc(fsize);
            LPBYTE eod=datacpy+isize;
            memcpy(datacpy,datasrc,fsize);
            LPBYTE dstI=datasrc+isize-width;
            LPBYTE srcI=datacpy;
            while(srcI<eod)
            {
                memcpy(dstI,srcI,width);
                dstI-=width;
                srcI+=width;
            }
            srcI= datacpy+isize;
            dstI= datasrc+isize+csize-cwidth;
            eod = datacpy+isize+csize;
            while(srcI<eod)
            {
                memcpy(dstI,srcI,cwidth);
                dstI-=cwidth;
                srcI+=cwidth;
            }

            srcI=datacpy+isize+csize;
            dstI=datasrc+isize+2*csize-cwidth;
            eod=datacpy+isize+2*csize;
            while(srcI<eod)
            {
                memcpy(dstI,srcI,cwidth);
                dstI-=cwidth;
                srcI+=cwidth;
            }
            free(datacpy);
            return;
        }
        case BI_Y8:
        {
            LPBYTE datasrc  = GetData(frame);
            int width    = frame->lpBMIH->biWidth;
            int height   = frame->lpBMIH->biHeight;
            int isize=width*height;
            LPBYTE datacpy=(LPBYTE)malloc(isize);
            LPBYTE eod=datacpy+isize;
            memcpy(datacpy,datasrc,isize);
            LPBYTE dstI=datasrc+isize-width;
            LPBYTE srcI=datacpy;
            while(srcI<eod)
            {
                memcpy(dstI,srcI,width);
                dstI-=width;
                srcI+=width;
            }
            free(datacpy);
            return;
        }
        case BI_Y16:
        {
            LPWORD datasrc  = (LPWORD)GetData(frame);
            int width    = frame->lpBMIH->biWidth;
            int height   = frame->lpBMIH->biHeight;
            int isize=width*height;
            int cwidth=width/4;
            LPWORD datacpy=(LPWORD)malloc(isize*sizeof(WORD));
            LPWORD eod=datacpy+isize;
            memcpy(datacpy,datasrc,isize*sizeof(WORD));
            LPWORD dstI=datasrc+isize-width;
            LPWORD srcI=datacpy;
            while(srcI<eod)
            {
                memcpy(dstI,srcI,width*sizeof(WORD));
                dstI-=width;
                srcI+=width;
            }
            free(datacpy);
            return;
        }
        default:
            TRACE("Error: Non supported videoformat in _fliph function\n");
    }
}

__forceinline void _fliph(LPBITMAPINFOHEADER frame)
{
    TVFrame fr={frame,NULL};
    _fliph(&fr);
}

__forceinline void memicpy(LPBYTE dest, const LPBYTE src, size_t count)
{
    LPBYTE eod=dest+count;
    LPBYTE sc=src+count;
    while (dest<eod)
    {
        *dest=*sc; dest++; sc--;
    }
}

__forceinline void _flipv(pTVFrame frame)
{
    switch(frame->lpBMIH->biCompression)
    {
        case BI_YUV9:
        {
            LPBYTE datasrc  = GetData(frame);
            int width    = frame->lpBMIH->biWidth;
            int height   = frame->lpBMIH->biHeight;
            int isize=width*height;
            int fsize=(9*isize)/8;
            int cwidth=width/4;
            int csize=isize/16;
            LPBYTE datacpy=(LPBYTE)malloc(fsize);
            LPBYTE eod=datacpy+isize;
            memcpy(datacpy,datasrc,fsize);
            LPBYTE dstI=datasrc;
            LPBYTE srcI=datacpy;
            while(srcI<eod)
            {
                memicpy(dstI,srcI,width);
                dstI+=width;
                srcI+=width;
            }

            srcI= datacpy+isize;
            dstI= datasrc+isize;
            eod = datacpy+isize+csize;
            while(srcI<eod)
            {
                memicpy(dstI,srcI,cwidth);
                dstI+=cwidth;
                srcI+=cwidth;
            }

            srcI=datacpy+isize+csize;
            dstI=datasrc+isize+csize;
            eod=datacpy+isize+2*csize;
            while(srcI<eod)
            {
                memicpy(dstI,srcI,cwidth);
                dstI+=cwidth;
                srcI+=cwidth;
            }
            free(datacpy);
            break;
        }
        case BI_Y8:
        {
            LPBYTE datasrc  = GetData(frame);
            int width    = frame->lpBMIH->biWidth;
            int height   = frame->lpBMIH->biHeight;
            int isize=width*height;
            LPBYTE datacpy=(LPBYTE)malloc(isize);
            LPBYTE eod=datacpy+isize;
            memcpy(datacpy,datasrc,isize);
            LPBYTE dstI=datasrc;
            LPBYTE srcI=datacpy;
            while(srcI<eod)
            {
                memicpy(dstI,srcI,width);
                dstI+=width;
                srcI+=width;
            }
            free(datacpy);
            break;
        }
        case BI_Y16:
        {
            LPWORD datasrc  = (LPWORD)GetData(frame);
            int width    = frame->lpBMIH->biWidth;
            int height   = frame->lpBMIH->biHeight;
            int isize=width*height;
            LPWORD datacpy=(LPWORD)malloc(isize*sizeof(WORD));
            LPWORD eod=datacpy+isize;
            memcpy(datacpy,datasrc,isize*sizeof(WORD));
            LPWORD dstI=datasrc;
            LPWORD srcI=datacpy;
            while(srcI<eod)
            {
                memicpy16(dstI,srcI,width);
                dstI+=width;
                srcI+=width;
            }
            free(datacpy);
            break;
        }
        default:
            TRACE("Error: Non supported videoformat in _fliph function\n");
    }
}

__forceinline void _flipv(LPBITMAPINFOHEADER frame)
{
    TVFrame fr={frame,NULL};
    _flipv(&fr);
}


__forceinline void _rotate90(pTVFrame frame)
{
    int x,y;
    LPBYTE datasrc  = GetData(frame);
    int width    = frame->lpBMIH->biWidth;
    int height   = frame->lpBMIH->biHeight;
    frame->lpBMIH->biWidth =height;
    frame->lpBMIH->biHeight=width;

    int isize=width*height;
    int fsize=(9*isize)/8;
    //int cwidth=width/4;
    LPBYTE datacpy=(LPBYTE)malloc(fsize);
    LPBYTE eod=datacpy+isize;
    memcpy(datacpy,datasrc,fsize);
    for (y=0; y<height; y++)
    {
        for (x=0; x<width; x++)
        {
            datasrc[y+x*height]=datacpy[x+(height-y-1)*width];
        }
    }
    height/=4; width/=4;
    LPBYTE udatas=datasrc+isize,vdatas=udatas+height*width;
    LPBYTE udatac=eod,vdatac=eod+height*width;
    for (y=0; y<height; y++)
    {
        for (x=0; x<width; x++)
        {
            udatas[y+x*height]=udatac[x+(height-y-1)*width];
            vdatas[y+x*height]=vdatac[x+(height-y-1)*width];
        }
    }
    free(datacpy);
}

__forceinline void _rotate90(LPBITMAPINFOHEADER frame)
{
    TVFrame fr={frame,NULL};
    _rotate90(&fr);
}


#endif