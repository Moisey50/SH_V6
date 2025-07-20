#ifndef ROTATE_INC
#define ROTATE_INC

//#include <math.h>
#include <video\TVFrame.h>
#include <imageproc\simpleip.h>
#include <math\hbmath.h>
#include <math\Intf_sup.h>
#include <helpers\FramesHelper.h>

#ifndef PI
#define PI			(3.1415926535897932384626433832795)
#endif

class HorShiftZone  
{
public:
  int m_iXBeg   ;
  int m_iXEnd   ;
  int m_iDeltaY ;

  HorShiftZone( int iXBeg = 0 , int iXEnd = 0 , int iDeltaY = 0 )
  {
    m_iXBeg =  iXBeg  ;
    m_iXEnd =  iXEnd  ;
    m_iDeltaY =iDeltaY;
  }
  int GetWidth() { return m_iXEnd - m_iXBeg + 1 ; }
} ;

class VertShiftZone  
{
public:
  int m_iDeltaX ;
  int m_iYBeg   ;
  int m_iYEnd   ;

  VertShiftZone( int iDeltaX = 0 , int iYBeg = 0 , int iYEnd = 0 )
  {
    m_iDeltaX = iDeltaX;
    m_iYBeg   = iYBeg  ;
    m_iYEnd   = iYEnd  ;
  }
  int GetHeight() { return m_iYEnd - m_iYBeg + 1 ; }
} ;

typedef FXArray<HorShiftZone> HorShifts ;
typedef FXArray<VertShiftZone> VertShifts ;

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
  int size   =(framesrc->lpBMIH->biCompression==BI_YUV12)?(12*bwsize)/8:(framesrc->lpBMIH->biCompression==BI_YUV9)?(9*bwsize)/8:bwsize;
  dstframe.lpBMIH=(LPBITMAPINFOHEADER)malloc(framesrc->lpBMIH->biSize+size);
  memcpy(dstframe.lpBMIH,framesrc->lpBMIH,framesrc->lpBMIH->biSize);
  LPBYTE dstdata = ((LPBYTE)dstframe.lpBMIH)+framesrc->lpBMIH->biSize;
  dstframe.lpBMIH->biWidth     = width;
  dstframe.lpBMIH->biHeight    = height;
  dstframe.lpBMIH->biSizeImage = size;
  memset(dstdata,0,bwsize);
  if (framesrc->lpBMIH->biCompression==BI_YUV9)
    memset(dstdata+bwsize,128,bwsize/8);
  else     if (framesrc->lpBMIH->biCompression==BI_YUV12)
    memset(dstdata+bwsize,128,bwsize/2);
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
  else if (framesrc->lpBMIH->biCompression==BI_YUV12)
  {
    dst = -1;
    LPBYTE USrc=datasrc+bwsizesrc;
    LPBYTE UDst=dstdata+bwsize;
    LPBYTE VSrc=USrc+bwsizesrc/4;
    LPBYTE VDst=UDst+bwsize/4;

    up/=2; dn=up+height/2; lt/=2; rt=lt+width/2;
    for (j = up; j < dn; j++)
    {
      for (i = lt; i < rt; i++)
      {
        dst++;
        x = (double)i; y = (double)j;
        _rotate(x, y, cosA, -sinA);
        int xsrc = _ROUND(x), ysrc = _ROUND(y);
        if (xsrc < 0 || xsrc >= framesrc->lpBMIH->biWidth/2|| ysrc < 0 || ysrc >= framesrc->lpBMIH->biHeight/2)
          continue;
        int src = xsrc + ysrc * framesrc->lpBMIH->biWidth/2;
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
  case BI_Y800:
  case BI_YUV9:
  case BI_YUV12:
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
  else if (framesrc->lpBMIH->biCompression==BI_YUV12)
  {
    LPBYTE USrc=datasrc+bwsizesrc;
    LPBYTE UDst=dstdata+bwsizesrc;
    LPBYTE VSrc=USrc+bwsizesrc/4;
    LPBYTE VDst=UDst+bwsizesrc/4;

    up/=2;  dn=up+height/2; lt/=2; rt=lt+width/2;
    ASSERT((rt-lt)==(width/2));
    ASSERT((dn-up)==(height/2));
    for (j = up; j < dn; j++)
    {
      dst=widthsrc*j/2-1+lt;
      for (i = lt; i < rt; i++)
      {
        dst++;
        if ((i<0) || (j<0) || (i>=widthsrc/2) || (j>=heightsrc/2))
          continue;

        x = (double)(i-center.x/2); y = (double)(j-center.y/2);
        _rotate(x, y, cosA, -sinA);
        x+=center.x/2; y+=center.y/2;
        int xsrc = _ROUND(x), ysrc = _ROUND(y);
        if (xsrc < rc.left/2 || xsrc >= rc.right/2 || ysrc < rc.top/2 || ysrc >= rc.bottom/2)
          continue;
        if ((xsrc<0) || (ysrc<0) || (xsrc>=widthsrc/2) || (ysrc>=heightsrc/2))
          continue;
        int src = xsrc + ysrc * framesrc->lpBMIH->biWidth/2;
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
  case BI_Y800:
  case BI_YUV9:
  case BI_YUV12:
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
    case 32:
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
      ASSERT(FALSE);
      TRACE("Error: Non supported videoformat in _fliph function\n");
      break;
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
  case BI_YUV12:
    {
      LPBYTE datasrc  = GetData(frame);
      int width    = frame->lpBMIH->biWidth;
      int height   = frame->lpBMIH->biHeight;
      int isize=width*height;
      int fsize=(3*isize)/2;
      int cwidth=width/2;
      int csize=isize/4;
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
  case BI_Y800:
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
    ASSERT(FALSE);
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
  case BI_YUV12:
    {
      LPBYTE datasrc  = GetData(frame);
      int width    = frame->lpBMIH->biWidth;
      int height   = frame->lpBMIH->biHeight;
      int isize=width*height;
      int fsize=(3*isize)/2;
      int cwidth=width/2;
      int csize=isize/4;
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
  case BI_Y800:
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
    ASSERT(FALSE);
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
  ASSERT(frame->lpBMIH->biCompression==BI_YUV9);
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
// in place rotation, don't use for passing to another gadgets video frames
__forceinline void _rotate90_Y8_16(pTVFrame frame)
{
  int x,y;
  DWORD dwCompr = frame->lpBMIH->biCompression ;

  bool bY8 = ( dwCompr == BI_YUV9 ) || ( dwCompr == BI_Y8 ) 
    || ( dwCompr == BI_Y800 ) || ( dwCompr == BI_YUV12 ) ;
  if ( !bY8 &&  ( dwCompr == BI_Y16 ))
    return ;
  LPBYTE datasrc  = GetData(frame);
  int width    = frame->lpBMIH->biWidth;
  int height   = frame->lpBMIH->biHeight;
  frame->lpBMIH->biWidth =height;
  frame->lpBMIH->biHeight=width;

  int isize=width*height;
  int fsize=isize ;
  if ( dwCompr == BI_Y16 )
    fsize *= 2 ;
  //int cwidth=width/4;
  LPBYTE datacpy=(LPBYTE)malloc(fsize);
  if ( bY8 )
  {
    memcpy( datacpy , datasrc , isize );
    for ( y = 0; y < height; y++ )
    {
      LPBYTE pSrc = datacpy + y * width ;
      LPBYTE pEnd = pSrc + width ;
      LPBYTE pDst = datasrc + y + ( width - 1 ) * height ;
      while ( pSrc < pEnd )
      {
        *pDst = *( pSrc++ ) ;
        pDst -= height ;
      }
    }
  }
  else
  {
    LPWORD wData = (LPWORD)datasrc ;
    LPWORD eod=wData+isize;
    memcpy(datacpy,datasrc,fsize);
    LPWORD wCpy = (LPWORD)datacpy ;
    for (y=0; y<height; y++)
    {
      for (x=0; x<width; x++)
      {
        wData[y+x*height]=wCpy[x+(height-y-1)*width];
      }
    }
  }
  free(datacpy);
}

// in place rotation, don't use for passing to another gadgets video frames
__forceinline void _rotate_minus90_Y8_16( pTVFrame frame )
{
  int y;
  DWORD dwCompr = frame->lpBMIH->biCompression ;

  bool bY8 = (dwCompr == BI_YUV9) || (dwCompr == BI_Y8) 
    || (dwCompr == BI_Y800) || (dwCompr == BI_YUV12) ;
  if ( !bY8 && (dwCompr == BI_Y16) )
    return ;
  LPBYTE datasrc = GetData( frame );
  int width = frame->lpBMIH->biWidth;
  int height = frame->lpBMIH->biHeight;
  frame->lpBMIH->biWidth = height;
  frame->lpBMIH->biHeight = width;

  int isize = width * height;
  int fsize = isize ;
  if ( dwCompr == BI_Y16 )
    fsize *= 2 ;
  //int cwidth=width/4;
  if ( bY8 )
  {
    LPBYTE datacpy = (LPBYTE) malloc( isize );
    memcpy( datacpy , datasrc , isize );
    LPBYTE pUpperUp = datasrc + height - 1 ;
    for ( y = 0; y < height; y++ )
    {
      LPBYTE psrc = datacpy + y * width ;
      LPBYTE pend = psrc + width ;
      LPBYTE pdst = pUpperUp - y ;
      while ( psrc < pend )
      {
        *pdst = *(psrc++) ;
        pdst += height ;
      }
    }
    free( datacpy );
  }
  else
  {
    LPWORD datacpy = (LPWORD) malloc( fsize );
    memcpy( datacpy , datasrc , fsize );
    LPWORD pUpperUp = (LPWORD) datasrc + height - 1 ;
    for ( y = 0; y < height; y++ )
    {
      LPWORD psrc = datacpy + y * width ;
      LPWORD pend = psrc + width ;
      LPWORD pdst = pUpperUp - y ;
      while ( psrc < pend )
      {
        *pdst = *(psrc++) ;
        pdst += height ;
      }
    }
    free( datacpy );
  }
}


// in place rotation, don't use for passing to another gadgets video frames
__forceinline void _rotate_90_Y8_16( pTVFrame frame )
{
  int y;
  DWORD dwCompr = frame->lpBMIH->biCompression ;

  bool bY8 = (dwCompr == BI_YUV9) || (dwCompr == BI_Y8) 
    || (dwCompr == BI_Y800) || (dwCompr == BI_YUV12) ;
  if ( !bY8 && (dwCompr == BI_Y16) )
    return ;
  LPBYTE datasrc = GetData( frame );
  int width = frame->lpBMIH->biWidth;
  int height = frame->lpBMIH->biHeight;
  frame->lpBMIH->biWidth = height;
  frame->lpBMIH->biHeight = width;

  int isize = width * height;
  int fsize = isize ;
  if ( dwCompr == BI_Y16 )
    fsize *= 2 ;
  //int cwidth=width/4;
  if ( bY8 )
  {
    LPBYTE datacpy = (LPBYTE) malloc( isize );
    memcpy( datacpy , datasrc , isize );
    LPBYTE pLeftDownPt = datasrc + height * (width - 1) ;
    for ( y = 0; y < height; y++ )
    {
      LPBYTE psrc = datacpy + y * width ;
      LPBYTE pend = psrc + width ;
      LPBYTE pdst = pLeftDownPt + y ;
      while ( psrc < pend )
      {
        *pdst = *(psrc++) ;
        pdst -= height ;
      }
    }
    free( datacpy );
  }
  else
  {
    LPWORD datacpy = (LPWORD) malloc( fsize );
    memcpy( datacpy , datasrc , fsize );
    LPWORD pLeftDownPt = (LPWORD) datasrc + height * (width - 1) ;
    for ( y = 0; y < height; y++ )
    {
      LPWORD psrc = datacpy + y * width ;
      LPWORD pend = psrc + width ;
      LPWORD pdst = pLeftDownPt + y ;
      while ( psrc < pend )
      {
        *pdst = *(psrc++) ;
        pdst -= height ;
      }
    }
    free( datacpy );
  }
}


// in place rotation, don't use for passing to another gadgets video frames
__forceinline void _rotate180_Y8_16( pTVFrame frame )
{
  DWORD dwCompr = frame->lpBMIH->biCompression ;

  bool bY8 = (dwCompr == BI_YUV9) || (dwCompr == BI_Y8) 
    || (dwCompr == BI_Y800) || (dwCompr == BI_YUV12) ;
  if ( !bY8 && (dwCompr == BI_Y16) )
    return ;
  LPBYTE datasrc = GetData( frame );
  int width = frame->lpBMIH->biWidth;
  int height = frame->lpBMIH->biHeight;
  int isize = width * height;
  LPBYTE pLast = datasrc + isize - 1 ;

  int fsize = isize ;
  if ( dwCompr == BI_Y16 )
    fsize *= 2 ;
  if ( bY8 )
  {
    do 
    {
      BYTE tmp = *datasrc ;
      *datasrc = *pLast ;
      *pLast = tmp ;
    } while ( ++datasrc < --pLast );
  }
  else
  {
    LPWORD pwData = (LPWORD) datasrc ;
    LPWORD pLast = pwData + isize - 1 ;
    do
    {
      WORD tmp = *pwData ;
      *pwData = *pLast ;
      *pLast = tmp ;
    } while ( ++pwData < --pLast );
  }
}

__forceinline void _rotate90(LPBITMAPINFOHEADER frame)
{
  TVFrame fr={frame,NULL};
  _rotate90(&fr);
}
__forceinline pTVFrame _flipboth(const pTVFrame frame)
{
  switch(frame->lpBMIH->biCompression)
  {
  case BI_RGB:
    switch( frame->lpBMIH->biBitCount)
    {
    case 24:
    case 32:
      {
        int iBytesPerPixel = (frame->lpBMIH->biBitCount / 8) ;
        LPBYTE datasrc  = GetData(frame);
        int width    = frame->lpBMIH->biWidth *  iBytesPerPixel ;
        int height   = frame->lpBMIH->biHeight;
        int fsize = width * height ;
        pTVFrame pFlipped = (pTVFrame)malloc( sizeof(TVFrame) ) ;
        pFlipped->lpBMIH = (LPBITMAPINFOHEADER)malloc( fsize + sizeof(BITMAPINFOHEADER) ) ;
        pFlipped->lpData = NULL ;
        memcpy( pFlipped->lpBMIH , frame->lpBMIH , sizeof(BITMAPINFOHEADER) ) ;
        LPBYTE datacpy=(LPBYTE) (pFlipped->lpBMIH + 1) ;
        LPBYTE dstI=datacpy+fsize - iBytesPerPixel ;
        LPBYTE srcI=datasrc ;
        LPBYTE eod=datasrc + fsize;
        while(srcI<eod)
        {
          for ( int i = 0 ; i < iBytesPerPixel ; i++ )
            *(dstI++) = *(srcI++) ;
          dstI -= 2 * iBytesPerPixel ;
        }
        return pFlipped ;
      }
    default:
      ASSERT(FALSE);
      TRACE("Error: Non supported videoformat in _fliph function\n");
      return NULL ;
    }
  case BI_YUV9:
    {
      int width    = frame->lpBMIH->biWidth;
      int height   = frame->lpBMIH->biHeight;
      int isize=width*height;
      int fsize=(9*isize)/8;
      int cwidth=width/4;
      int csize=isize/16;
      pTVFrame pFlipped = (pTVFrame)malloc( sizeof(TVFrame) ) ;
      pFlipped->lpBMIH = (LPBITMAPINFOHEADER)malloc( fsize + sizeof(BITMAPINFOHEADER) ) ;
      pFlipped->lpData = NULL ;
      memcpy( pFlipped->lpBMIH , frame->lpBMIH , sizeof(BITMAPINFOHEADER) ) ;
      LPBYTE datacpy=(LPBYTE) (pFlipped->lpBMIH + 1) ;
      LPBYTE dstI = datacpy + isize - 1;
      LPBYTE datasrc  = GetData(frame);
      LPBYTE srcI = datasrc ;
      LPBYTE eod = datasrc + isize ;
      while(srcI<eod)
        *(dstI--) = *(srcI++) ;

      srcI= datasrc + isize;
      dstI= datacpy + isize + csize - 1;
      eod = datasrc + isize + csize;
      while(srcI<eod)
        *(dstI--) = *(srcI++) ;

      srcI = datasrc + isize + csize ;
      dstI = datacpy + isize + 2 * csize - 1 ;
      eod = datasrc + isize + 2 * csize ;
      while(srcI<eod)
        *(dstI--) = *(srcI++) ;
      return pFlipped ;
    }
  case BI_YUV12:
    {
      int width    = frame->lpBMIH->biWidth;
      int height   = frame->lpBMIH->biHeight;
      int isize=width*height;
      int fsize=(3*isize)/2;
      int cwidth=width/2;
      int csize=isize/4;
      pTVFrame pFlipped = (pTVFrame)malloc( sizeof(TVFrame) ) ;
      pFlipped->lpBMIH = (LPBITMAPINFOHEADER)malloc( fsize + sizeof(BITMAPINFOHEADER) ) ;
      pFlipped->lpData = NULL ;
      memcpy( pFlipped->lpBMIH , frame->lpBMIH , sizeof(BITMAPINFOHEADER) ) ;
      LPBYTE datacpy=(LPBYTE) (pFlipped->lpBMIH + 1) ;
      LPBYTE dstI = datacpy + isize - 1;
      LPBYTE datasrc  = GetData(frame);
      LPBYTE srcI = datasrc ;
      LPBYTE eod = datasrc + isize ;
      while(srcI<eod)
        *(dstI--) = *(srcI++) ;

      srcI= datasrc + isize;
      dstI= datacpy + isize + csize - 1;
      eod = datasrc + isize + csize;
      while(srcI<eod)
        *(dstI--) = *(srcI++) ;

      srcI = datasrc + isize + csize ;
      dstI = datacpy + isize + 2 * csize - 1 ;
      eod = datasrc + isize + 2 * csize ;
      while(srcI<eod)
        *(dstI--) = *(srcI++) ;
      return pFlipped ;
    }
  case BI_Y8:
  case BI_Y800:
  {
      LPBYTE datasrc  = GetData(frame);
      int width    = frame->lpBMIH->biWidth;
      int height   = frame->lpBMIH->biHeight;
      int isize=width*height;
      pTVFrame pFlipped = (pTVFrame)malloc( sizeof(TVFrame) ) ;
      pFlipped->lpBMIH = (LPBITMAPINFOHEADER)malloc( isize + sizeof(BITMAPINFOHEADER) ) ;
      pFlipped->lpData = NULL ;
      memcpy( pFlipped->lpBMIH , frame->lpBMIH , sizeof(BITMAPINFOHEADER) ) ;
      LPBYTE datacpy=(LPBYTE) (pFlipped->lpBMIH + 1) ;
      LPBYTE dstI = datacpy + isize - 1;
      LPBYTE srcI = datasrc ;
      LPBYTE eod = datasrc + isize ;
      while(srcI<eod)
        *(dstI--) = *(srcI++) ;
      return pFlipped ;
    }
  case BI_Y16:
    {
      int width    = frame->lpBMIH->biWidth;
      int height   = frame->lpBMIH->biHeight;
      int isize = width * height ;
      pTVFrame pFlipped = (pTVFrame)malloc( sizeof(TVFrame) ) ;
      pFlipped->lpBMIH = (LPBITMAPINFOHEADER)malloc( (isize* 2) + sizeof(BITMAPINFOHEADER) ) ;
      pFlipped->lpData = NULL ;
      memcpy( pFlipped->lpBMIH , frame->lpBMIH , sizeof(BITMAPINFOHEADER) ) ;
      LPWORD datacpy=(LPWORD) (pFlipped->lpBMIH + 1) ;
      LPWORD dstI = datacpy + isize - 1;
      LPWORD datasrc  = (LPWORD)GetData(frame);
      LPWORD srcI = datasrc ;
      LPWORD eod = datasrc + isize ;
      while(srcI<eod)
        *(dstI--) = *(srcI++) ;
      return pFlipped ;
    }
  default:
    ASSERT(FALSE);
    TRACE("Error: Non supported videoformat 0x%08X in _fliph function\n");
  }
  return NULL ;
}


__forceinline void _skew_frame_rect(pTVFrame framesrc, RECT rco, double angle)
{
  CRect rc( rco ) ;
  if (rc.left==rc.right) 
    return;
  if (rc.top==rc.bottom) 
    return;

  int widthsrc    = framesrc->lpBMIH->biWidth;
  int heightsrc   = framesrc->lpBMIH->biHeight;
  int bwsizesrc   = widthsrc*heightsrc;
  double cosA = cos(angle), sinA = sin(angle);
  //correct rectangle, width and height must be odd
  if (rc.left<0) 
    rc.left=0; 
  if (rc.top<0) 
    rc.top=0;
  if (rc.right>=widthsrc) 
    rc.right=widthsrc-1; 
  if (rc.top>=heightsrc) 
    rc.top=heightsrc-1;

  CPoint center( (rc.right+rc.left)/2 , (rc.top+rc.bottom)/2 ) ;
  double dRCW = rc.Width() ;
  double dRCH = rc.Height() ;
  double dY = dRCW * 0.5 * sin( angle ) ;
  double dYAbs = fabs(dY) ;
  int iNSteps = ROUND( dYAbs ) ;
  if (iNSteps < 1 )
    return ; // too small angle, no skew

  double dYLimit = dYAbs - 0.5 ;
  double dSlope = dYAbs / dRCW ;
  HorShifts Zones ;
  double dStep = 0.5/dSlope ; 
  int iDeltaY = ( dY > 0 ) ? -1 : 1 ;
  for ( double dYStep = 0.5 ; dYStep < dYAbs ; dYStep += 1.0 )
  {
    HorShiftZone ZoneMinus( 
      center.x - ROUND( dStep * (dYStep + 1.) ), 
      center.x - ROUND( dStep * dYStep ) , iDeltaY ) ;
    
    if ( ZoneMinus.m_iXBeg < rc.left )
      ZoneMinus.m_iXBeg = rc.left ;
    Zones.Add( ZoneMinus ) ;
        
    HorShiftZone ZonePlus( 
      center.x + ROUND( dStep * dYStep ), 
      center.x + ROUND( dStep * (dYStep + 1.) ) , -iDeltaY ) ;

    if ( ZonePlus.m_iXEnd > rc.right )
      ZonePlus.m_iXEnd = rc.right ;
    Zones.Add( ZonePlus ) ;
    iDeltaY += ( dY > 0 ) ? -1 : 1 ;
  }
  
  // skew rect to new coordinates
 

  //skew intensity
  switch ( GetCompression(framesrc) )
  {
  case BI_Y8:
  case BI_Y800:
  case BI_YUV9:
  case BI_YUV12:
    {
      LPBYTE pImage = GetData(framesrc) ;
      LPBYTE pImageEnd = pImage + widthsrc * heightsrc ;
      for ( int iZonesCnt = 0 ; iZonesCnt < Zones.GetCount() ; iZonesCnt++ )
      {
        HorShiftZone& Zone = Zones.GetAt(iZonesCnt) ;
        if ( Zone.m_iDeltaY > 0 )
        {
          for ( int y = rc.bottom ; y >= rc.top ; y-- )
          {
            if ( y + Zone.m_iDeltaY >= heightsrc )
              continue ;
            LPBYTE pSrc = pImage + y * widthsrc + Zone.m_iXBeg ;
      
            LPBYTE pDst = pSrc + Zone.m_iDeltaY * widthsrc ;
            if ( pDst <= pImage )
              continue ;
            if ( pDst + Zone.GetWidth() >= pImageEnd )
              continue ;
            if ( pSrc <= pImage )
              continue ;
            if ( pSrc + Zone.GetWidth() >= pImageEnd )
              continue ;
            memcpy( pDst , pSrc , Zone.GetWidth() ) ;
    }
  }
        else
  {
          for ( int y = rc.top ; y < rc.bottom ; y++ )
    {
            if ( y + Zone.m_iDeltaY <= 0 )
          continue;
            LPBYTE pSrc = pImage + y * widthsrc + Zone.m_iXBeg ;
            LPBYTE pDst = pSrc + Zone.m_iDeltaY * widthsrc ;
            if ( pDst <= pImage )
              continue ;
            if ( pDst + Zone.GetWidth() >= pImageEnd )
          continue;
            if ( pSrc <= pImage )
          continue;
            if ( pSrc + Zone.GetWidth() >= pImageEnd )
              continue ;
            
            memcpy( pDst , pSrc , Zone.GetWidth() ) ;
          }
      }
    }
  } 
    break ;
  case BI_Y16:
  {
      LPWORD pImage = GetData16(framesrc) ;
      for ( int iZonesCnt = 0 ; iZonesCnt < Zones.GetCount() ; iZonesCnt++ )
    {
        HorShiftZone& Zone = Zones.GetAt(iZonesCnt) ;
        int iWidthBytes = Zone.GetWidth() * sizeof( WORD ) ;
        if ( Zone.m_iDeltaY > 0 )
      {
          for ( int y = rc.bottom ; y >= rc.top ; y-- )
          {
            if ( y + Zone.m_iDeltaY > heightsrc )
          continue;
            LPWORD pSrc = pImage + y * widthsrc + Zone.m_iXBeg ;
            LPWORD pDst = pSrc + Zone.m_iDeltaY * widthsrc ;
            memcpy( pDst , pSrc , iWidthBytes ) ;
          }
        }
        else
        {
          for ( int y = rc.top ; y < rc.bottom ; y++ )
          {
            if ( y + Zone.m_iDeltaY < 0 )
              continue ;
            LPWORD pSrc = pImage + y * widthsrc + Zone.m_iXBeg ;
            LPWORD pDst = pSrc + Zone.m_iDeltaY * widthsrc ;
            memcpy( pDst , pSrc , iWidthBytes ) ;
      }
    }
  } 
    }
    break ;
  }

//   if (framesrc->lpBMIH->biCompression==BI_YUV9)
//   {
//     LPBYTE USrc=datasrc+bwsizesrc;
//     LPBYTE UDst=dstdata+bwsizesrc;
//     LPBYTE VSrc=USrc+bwsizesrc/16;
//     LPBYTE VDst=UDst+bwsizesrc/16;
// 
//     up/=4;  dn=up+height/4; lt/=4; rt=lt+width/4;
//     ASSERT((rt-lt)==(width/4));
//     ASSERT((dn-up)==(height/4));
//     for (j = up; j < dn; j++)
//     {
//       dst=widthsrc*j/4-1+lt;
//       for (i = lt; i < rt; i++)
//       {
//         dst++;
//         if ((i<0) || (j<0) || (i>=widthsrc/4) || (j>=heightsrc/4))
//           continue;
// 
//         x = (double)(i-center.x/4); y = (double)(j-center.y/4);
//         _rotate(x, y, cosA, -sinA);
//         x+=center.x/4; y+=center.y/4;
//         int xsrc = _ROUND(x), ysrc = _ROUND(y);
//         if (xsrc < rc.left/4 || xsrc >= rc.right/4 || ysrc < rc.top/4 || ysrc >= rc.bottom/4)
//           continue;
//         if ((xsrc<0) || (ysrc<0) || (xsrc>=widthsrc/4) || (ysrc>=heightsrc/4))
//           continue;
//         int src = xsrc + ysrc * framesrc->lpBMIH->biWidth/4;
//         *(UDst+dst) = *(USrc+src);
//         *(VDst+dst) = *(VSrc+src);
//       }
//     }
//   } 
//   else if (framesrc->lpBMIH->biCompression==BI_YUV12)
//   {
//     LPBYTE USrc=datasrc+bwsizesrc;
//     LPBYTE UDst=dstdata+bwsizesrc;
//     LPBYTE VSrc=USrc+bwsizesrc/4;
//     LPBYTE VDst=UDst+bwsizesrc/4;
// 
//     up/=2;  dn=up+height/2; lt/=2; rt=lt+width/2;
//     ASSERT((rt-lt)==(width/2));
//     ASSERT((dn-up)==(height/2));
//     for (j = up; j < dn; j++)
//     {
//       dst=widthsrc*j/2-1+lt;
//       for (i = lt; i < rt; i++)
//       {
//         dst++;
//         if ((i<0) || (j<0) || (i>=widthsrc/2) || (j>=heightsrc/2))
//           continue;
// 
//         x = (double)(i-center.x/2); y = (double)(j-center.y/2);
//         _rotate(x, y, cosA, -sinA);
//         x+=center.x/2; y+=center.y/2;
//         int xsrc = _ROUND(x), ysrc = _ROUND(y);
//         if (xsrc < rc.left/2 || xsrc >= rc.right/2 || ysrc < rc.top/2 || ysrc >= rc.bottom/2)
//           continue;
//         if ((xsrc<0) || (ysrc<0) || (xsrc>=widthsrc/2) || (ysrc>=heightsrc/2))
//           continue;
//         int src = xsrc + ysrc * framesrc->lpBMIH->biWidth/2;
//         *(UDst+dst) = *(USrc+src);
//         *(VDst+dst) = *(VSrc+src);
//       }
//     }
//   } 
// 
//   if (framesrc->lpData) free(framesrc->lpData); framesrc->lpData=NULL;
//   free(framesrc->lpBMIH); framesrc->lpBMIH=dstframe.lpBMIH;
}

#endif