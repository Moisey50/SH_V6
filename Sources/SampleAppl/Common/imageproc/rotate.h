#ifndef ROTATE_INC
#define ROTATE_INC

#include <math.h>
#include <video\TVFrame.h>
#include <imageproc\simpleip.h>
#include <math\hbmath.h>

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


__forceinline void _rotate_frame8(pTVFrame framesrc,double angle)
{
    TVFrame dstframe     = {NULL,NULL};
    LPBYTE datasrc  = GetData(framesrc);
    int widthsrc    = framesrc->lpBMIH->biWidth;
    int heightsrc   = framesrc->lpBMIH->biHeight;
    int bwsizesrc   = widthsrc*heightsrc;
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

	int up = _ROUND(yMin), dn = _ROUND(yMax);
	int lt = _ROUND(xMin), rt = _ROUND(xMax);
	int width = (rt - lt) / 4 * 4;
	int height = (dn - up) / 4 * 4;
    // correct values
    rt=lt+width;
    dn=up+height;

    int bwsize = width * height;
    int size   =(framesrc->lpBMIH->biCompression==BI_YUV9)?(9*bwsize)/8:bwsize;
    dstframe.lpBMIH=(LPBITMAPINFOHEADER)malloc(framesrc->lpBMIH->biSize+size);
    memcpy(dstframe.lpBMIH,framesrc->lpBMIH,framesrc->lpBMIH->biSize);
    LPBYTE dstdata = ((LPBYTE)dstframe.lpBMIH)+framesrc->lpBMIH->biSize;
    dstframe.lpBMIH->biWidth     = width;
    dstframe.lpBMIH->biHeight    = height;
    dstframe.lpBMIH->biSizeImage = size;
    memset(dstdata,0,bwsize);
    if (framesrc->lpBMIH->biCompression==BI_YUV9)
        memset(dstdata+bwsize,128,bwsize/8);
    int i, j, dst = -1;

	for (j = up; j < dn; j++)
	{
		for (i = lt; i < rt; i++)
		{
            dst++;
			x = (double)i; y = (double)j;
			_rotate(x, y, cosA, -sinA);
			int xsrc = _ROUND(x), ysrc = _ROUND(y);
			if (xsrc < 0 || xsrc >= framesrc->lpBMIH->biWidth|| ysrc < 0 || ysrc >= framesrc->lpBMIH->biHeight)
				continue;
			int src = xsrc + ysrc * framesrc->lpBMIH->biWidth;
			*(dstdata+dst) = *(datasrc+src);
		}
	}
    if (framesrc->lpBMIH->biCompression==BI_YUV9)
    {
        dst = -1;
        LPBYTE USrc=datasrc+bwsizesrc;
        LPBYTE UDst=dstdata+bwsize;
        LPBYTE VSrc=USrc+bwsizesrc/16;
        LPBYTE VDst=UDst+bwsize/16;

        up/=4; dn=up+height/4; lt/=4; rt=lt+width/4;
	    for (j = up; j < dn; j++)
	    {
		    for (i = lt; i < rt; i++)
		    {
                dst++;
			    x = (double)i; y = (double)j;
			    _rotate(x, y, cosA, -sinA);
			    int xsrc = _ROUND(x), ysrc = _ROUND(y);
			    if (xsrc < 0 || xsrc >= framesrc->lpBMIH->biWidth/4|| ysrc < 0 || ysrc >= framesrc->lpBMIH->biHeight/4)
				    continue;
			    int src = xsrc + ysrc * framesrc->lpBMIH->biWidth/4;
			    *(UDst+dst) = *(USrc+src);
                *(VDst+dst) = *(VSrc+src);
		    }
	    }
    } 
    if (framesrc->lpData) free(framesrc->lpData); framesrc->lpData=NULL;
    free(framesrc->lpBMIH); framesrc->lpBMIH=dstframe.lpBMIH;
}

__forceinline void _rotate_frame16(pTVFrame framesrc,double angle)
{
    TVFrame dstframe     = {NULL,NULL};
    LPWORD datasrc  = (LPWORD) GetData(framesrc);
    int widthsrc    = framesrc->lpBMIH->biWidth;
    int heightsrc   = framesrc->lpBMIH->biHeight;
    double cosA = cos(angle), sinA = sin(angle);
	double xMin = 0, xMax = xMin, yMin = 0, yMax = yMin;

    double x = widthsrc, y = 0;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);
	
    x = 0; y = heightsrc;
	_rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

    x = widthsrc; y = heightsrc;
	_rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

	int up = _ROUND(yMin), dn = _ROUND(yMax);
	int lt = _ROUND(xMin), rt = _ROUND(xMax);
	int width = (rt - lt) / 4 * 4;
	int height = (dn - up) / 4 * 4;
    // correct values
    rt=lt+width;
    dn=up+height;

	int size = width * height*sizeof(WORD);
    dstframe.lpBMIH=(LPBITMAPINFOHEADER)malloc(framesrc->lpBMIH->biSize+size);
    memcpy(dstframe.lpBMIH,framesrc->lpBMIH,framesrc->lpBMIH->biSize);
    LPWORD dstdata = (LPWORD)(((LPBYTE)dstframe.lpBMIH)+framesrc->lpBMIH->biSize);
    dstframe.lpBMIH->biWidth     = width;
    dstframe.lpBMIH->biHeight    = height;
    dstframe.lpBMIH->biSizeImage = size;
    memset(dstdata,0,size);

    int i, j, dst = -1;

	for (j = up; j < dn; j++)
	{
		for (i = lt; i < rt; i++)
		{
			dst++;
			x = (double)i; y = (double)j;
			_rotate(x, y, cosA, -sinA);
			int xsrc = _ROUND(x), ysrc = _ROUND(y);
			if (xsrc < 0 || xsrc >= framesrc->lpBMIH->biWidth|| ysrc < 0 || ysrc >= framesrc->lpBMIH->biHeight)
				continue;
			int src = xsrc + ysrc * framesrc->lpBMIH->biWidth;
			*(dstdata+dst) = *(datasrc+src);
		}
	}

    if (framesrc->lpData) free(framesrc->lpData); framesrc->lpData=NULL;
    free(framesrc->lpBMIH); framesrc->lpBMIH=dstframe.lpBMIH;
}

__forceinline void _rotate_frame(pTVFrame framesrc,double angle)
{
    switch (framesrc->lpBMIH->biCompression)
    {
    case BI_Y8:
    case BI_YUV9:
        _rotate_frame8(framesrc,angle);
        break;
    case BI_Y16:
        _rotate_frame16(framesrc,angle);
        break;
    default:
        ASSERT(false); // Format not supported
    }
}

__forceinline void _rotate_frame_rect8(pTVFrame framesrc, RECT rc, double angle)
{
    if (rc.left==rc.right) return;
    if (rc.top==rc.bottom) return;

    TVFrame dstframe     = {NULL,NULL};
    LPBYTE datasrc  = GetData(framesrc);
    int widthsrc    = framesrc->lpBMIH->biWidth;
    int heightsrc   = framesrc->lpBMIH->biHeight;
    int bwsizesrc   = widthsrc*heightsrc;
    double cosA = cos(angle), sinA = sin(angle);
    //correct rectangle, width and height must be odd
    if (rc.left<0) rc.left=0; if (rc.top<0) rc.top=0;
    if (rc.right>=widthsrc) rc.right=widthsrc-1; if (rc.top>=heightsrc) rc.top=heightsrc-1;

    if ((rc.right-rc.left)%2==0) rc.right--;
    if ((rc.top-rc.bottom)%2==0) rc.top--;
    POINT center; center.x=(rc.right+rc.left)/2; center.y=(rc.top+rc.bottom)/2;
    // shift rect to new coordinates
    rc.left  -= center.x; rc.right -= center.x;
    rc.bottom-= center.y; rc.top   -= center.y;

	double xMin = INT_MAX, xMax = INT_MIN, yMin = INT_MAX, yMax = INT_MIN;

    double x = rc.left, y = rc.top;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

    x = rc.right; y = rc.top;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

    x = rc.right; y = rc.bottom;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

    x = rc.left; y = rc.bottom;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);
    
    // return to real bmp coordinates
    xMin += center.x, xMax += center.x, yMin += center.y, yMax += center.y;
    rc.left  += center.x; rc.right += center.x;
    rc.bottom+= center.y; rc.top   += center.y;

	int up = _ROUND(yMin), dn = _ROUND(yMax);
	int lt = _ROUND(xMin), rt = _ROUND(xMax);
	int width = (rt - lt) / 4 * 4;
	int height = (dn - up) / 4 * 4;
    // correct values
    rt=lt+width;
    dn=up+height;

    dstframe.lpBMIH=(LPBITMAPINFOHEADER)malloc(framesrc->lpBMIH->biSize+GetImageSize(framesrc));
    memcpy(dstframe.lpBMIH,framesrc->lpBMIH,framesrc->lpBMIH->biSize);
    LPBYTE dstdata = ((LPBYTE)dstframe.lpBMIH)+framesrc->lpBMIH->biSize;
    memcpy(dstdata,datasrc,GetImageSize(framesrc));

    int i, j, dst;

	for (j = up; j < dn; j++)
	{
        dst=widthsrc*j-1+lt;
		for (i = lt; i < rt; i++)
		{
            dst++;
            if ((i<0) || (j<0) || (i>=widthsrc) || (j>=heightsrc))
                continue;
			x = (double)(i-center.x); y = (double)(j-center.y);
			_rotate(x, y, cosA, -sinA);
            x+=center.x; y+=center.y;
			int xsrc = _ROUND(x), ysrc = _ROUND(y);
            if (xsrc < rc.left || xsrc >= rc.right|| ysrc < rc.top || ysrc >= rc.bottom)
				continue;
            if ((xsrc<0) || (ysrc<0) || (xsrc>=widthsrc) || (ysrc>=heightsrc))
                continue;
			int src = xsrc + ysrc * framesrc->lpBMIH->biWidth;
			*(dstdata+dst) = *(datasrc+src);
		}
	} 
    if (framesrc->lpBMIH->biCompression==BI_YUV9)
    {
        LPBYTE USrc=datasrc+bwsizesrc;
        LPBYTE UDst=dstdata+bwsizesrc;
        LPBYTE VSrc=USrc+bwsizesrc/16;
        LPBYTE VDst=UDst+bwsizesrc/16;

        up/=4;  dn=up+height/4; lt/=4; rt=lt+width/4;
        ASSERT((rt-lt)==(width/4));
        ASSERT((dn-up)==(height/4));
	    for (j = up; j < dn; j++)
	    {
            dst=widthsrc*j/4-1+lt;
		    for (i = lt; i < rt; i++)
		    {
                dst++;
                if ((i<0) || (j<0) || (i>=widthsrc/4) || (j>=heightsrc/4))
                    continue;

			    x = (double)(i-center.x/4); y = (double)(j-center.y/4);
			    _rotate(x, y, cosA, -sinA);
                x+=center.x/4; y+=center.y/4;
			    int xsrc = _ROUND(x), ysrc = _ROUND(y);
                if (xsrc < rc.left/4 || xsrc >= rc.right/4 || ysrc < rc.top/4 || ysrc >= rc.bottom/4)
				    continue;
                if ((xsrc<0) || (ysrc<0) || (xsrc>=widthsrc/4) || (ysrc>=heightsrc/4))
                    continue;
			    int src = xsrc + ysrc * framesrc->lpBMIH->biWidth/4;
			    *(UDst+dst) = *(USrc+src);
                *(VDst+dst) = *(VSrc+src);
		    }
	    }
    } 
    if (framesrc->lpData) free(framesrc->lpData); framesrc->lpData=NULL;
    free(framesrc->lpBMIH); framesrc->lpBMIH=dstframe.lpBMIH;
}

__forceinline void _rotate_frame_rect16(pTVFrame framesrc, RECT rc, double angle)
{
    if (rc.left==rc.right) return;
    if (rc.top==rc.bottom) return;

    TVFrame dstframe     = {NULL,NULL};
    LPWORD datasrc  = (LPWORD)GetData(framesrc);
    int widthsrc    = framesrc->lpBMIH->biWidth;
    int heightsrc   = framesrc->lpBMIH->biHeight;
    int bwsizesrc   = widthsrc*heightsrc;
    double cosA = cos(angle), sinA = sin(angle);
    //correct rectangle, width and height must be odd
    if (rc.left<0) rc.left=0; if (rc.top<0) rc.top=0;
    if (rc.right>=widthsrc) rc.right=widthsrc-1; if (rc.top>=heightsrc) rc.top=heightsrc-1;

    if ((rc.right-rc.left)%2==0) rc.right--;
    if ((rc.top-rc.bottom)%2==0) rc.top--;
    POINT center; center.x=(rc.right+rc.left)/2; center.y=(rc.top+rc.bottom)/2;
    // shift rect to new coordinates
    rc.left  -= center.x; rc.right -= center.x;
    rc.bottom-= center.y; rc.top   -= center.y;

	double xMin = INT_MAX, xMax = INT_MIN, yMin = INT_MAX, yMax = INT_MIN;

    double x = rc.left, y = rc.top;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

    x = rc.right; y = rc.top;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

    x = rc.right; y = rc.bottom;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);

    x = rc.left; y = rc.bottom;
    _rotate(x, y, cosA, sinA);
	_min_max(xMin, xMax, x);
	_min_max(yMin, yMax, y);
    
    // return to real bmp coordinates
    xMin += center.x, xMax += center.x, yMin += center.y, yMax += center.y;
    rc.left  += center.x; rc.right += center.x;
    rc.bottom+= center.y; rc.top   += center.y;

	int up = _ROUND(yMin), dn = _ROUND(yMax);
	int lt = _ROUND(xMin), rt = _ROUND(xMax);
	int width = (rt - lt) / 4 * 4;
	int height = (dn - up) / 4 * 4;
    // correct values
    rt=lt+width;
    dn=up+height;

    dstframe.lpBMIH=(LPBITMAPINFOHEADER)malloc(framesrc->lpBMIH->biSize+GetImageSize(framesrc));
    memcpy(dstframe.lpBMIH,framesrc->lpBMIH,framesrc->lpBMIH->biSize);
    LPWORD dstdata = (LPWORD) (((LPBYTE)dstframe.lpBMIH)+framesrc->lpBMIH->biSize);
    memcpy(dstdata,datasrc,GetImageSize(framesrc));

    int i, j, dst;

	for (j = up; j < dn; j++)
	{
        dst=widthsrc*j-1+lt;
		for (i = lt; i < rt; i++)
		{
            dst++;
            if ((i<0) || (j<0) || (i>=widthsrc) || (j>=heightsrc))
                continue;
			x = (double)(i-center.x); y = (double)(j-center.y);
			_rotate(x, y, cosA, -sinA);
            x+=center.x; y+=center.y;
			int xsrc = _ROUND(x), ysrc = _ROUND(y);
            if (xsrc < rc.left || xsrc >= rc.right|| ysrc < rc.top || ysrc >= rc.bottom)
				continue;
            if ((xsrc<0) || (ysrc<0) || (xsrc>=widthsrc) || (ysrc>=heightsrc))
                continue;
			int src = xsrc + ysrc * framesrc->lpBMIH->biWidth;
			*(dstdata+dst) = *(datasrc+src);
		}
	} 
    if (framesrc->lpData) free(framesrc->lpData); framesrc->lpData=NULL;
    free(framesrc->lpBMIH); framesrc->lpBMIH=dstframe.lpBMIH;
}

__forceinline void _rotate_frame_rect(pTVFrame framesrc, RECT rc, double angle)
{
    switch (framesrc->lpBMIH->biCompression)
    {
    case BI_Y8:
    case BI_YUV9:
        _rotate_frame_rect8(framesrc, rc, angle);
        break;
    case BI_Y16:
        _rotate_frame_rect16(framesrc, rc, angle);
        break;
    default:
        ASSERT(false); // Format not supported
    }
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