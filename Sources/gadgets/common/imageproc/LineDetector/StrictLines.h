// StrictLines.h: interface for the CStrictLines class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(STRICTLINES_INC_)
#define STRICTLINES_INC_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <classes\dpoint.h>
#include <imageproc\clusters\clusters.h>
#include <imageproc\integer_imagefunc.h>

#define TYPE_ARRAYOFPOINTS 0
#define TYPE_VSEGMENT	   1
#define TYPE_HSEGMENT	   2
#define TYPE_RECTANGLE     3

typedef struct taglinefactors
{
	int	   type;
	CDPoint coi; // center of mass;
	CDPoint averagesq;
	CDPoint a,b,c,d; // edge points of segment
	double sigma;
}linefactors,*plinefactors;

__forceinline double distance(CDPoint& a, CDPoint& b)
{
    return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
}

__forceinline double distance(plinefactors a, plinefactors b)
{
  double dMinDist = abs(CDP2Cmplx(a->a) - CDP2Cmplx(b->a));
  double dNextDist = abs(CDP2Cmplx(a->a) - CDP2Cmplx(b->b));
  if (dNextDist < dMinDist)
    dMinDist = dNextDist;
  dNextDist = abs(CDP2Cmplx(a->b) - CDP2Cmplx(b->a)); 
  if (dNextDist < dMinDist)
    dMinDist = dNextDist;
  dNextDist = abs(CDP2Cmplx(a->b) - CDP2Cmplx(b->b));
  if (dNextDist < dMinDist)
    dMinDist = dNextDist;

  return dMinDist ;
}

class soi;
class CStrictLines : public CClusters
{
private:
// settings
        long        m_maxLength;
        int         m_MagnitudeThreshold;
        CRect       m_ROI;
// temporary data
        idata      *m_pIntensity;
        idata      *m_pMagnitude;
#ifdef _DEBUG
public:
#endif
        pTVFrame    m_AOI;
public:
                CStrictLines();
               ~CStrictLines(); 
        bool    SeekClusters();
  plinefactors  GetFigure(int i);
private:
        void    Add(soi& _soi);
        void    EstimateBoundaries();
        void    DrawPixel(int x, int y, LPBYTE Data, int width);
	    void    Finish();
};

#endif // !defined(STRICTLINES_INC_)