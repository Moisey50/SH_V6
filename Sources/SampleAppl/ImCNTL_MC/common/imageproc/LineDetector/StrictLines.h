// StrictLines.h: interface for the CStrictLines class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(STRICTLINES_INC_)
#define STRICTLINES_INC_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <helpers\DPoint.h>
#include <imageproc\clusters\clusters.h>
#include <imageproc\integer_imagefunc.h>

#define TYPE_ARRAYOFPOINTS 0
#define TYPE_VSEGMENT	   1
#define TYPE_HSEGMENT	   2
#define TYPE_RECTANGLE     3

typedef struct taglinefactors
{
	int	   type;
	DPOINT coi; // center of mass;
	DPOINT averagesq;
	DPOINT a,b,c,d; // edge points of segment
	double sigma;
}linefactors,*plinefactors;

__forceinline double distance(DPOINT a, DPOINT b)
{
    return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
}

__forceinline double distance(plinefactors a, plinefactors b)
{
    return min(min(distance(a->a,b->a),distance(a->a,b->b)),min(distance(a->b,b->a),distance(a->b,b->b)));
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