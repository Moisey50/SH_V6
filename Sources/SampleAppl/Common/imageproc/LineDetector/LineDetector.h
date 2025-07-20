// LineDetector.h: interface for the CLineDetector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(LINEDETECTOR_INC_)
#define LINEDETECTOR_INC_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <imageproc\clusters\clusters.h>
#include <imageproc\integer_imagefunc.h>

#define BoundaryDirNoDir 0
#define BoundaryDir01_30 1
#define BoundaryDir03_00 2
#define BoundaryDir04_30 4
#define BoundaryDir00_00 8

#define StrokeDirNoDir 0
#define StrokeDir01_30 1
#define StrokeDir03_00 2
#define StrokeDir04_30 4
#define StrokeDir00_00 8

typedef struct tagBoundaryEdge 
{
   bool            isEdge;
   int             magnitude;  
   int             edgeDir;
   int             scanDir;   
   int             x,y;
   double          xFrac;
   double          yFrac;
   tagBoundaryEdge()
   {
        isEdge=false;
        magnitude=0;
        edgeDir=scanDir=100;
        x=y=-1;
        xFrac=yFrac=0.0;
   }
   tagBoundaryEdge(int _x, int _y)
   {
        isEdge=false;
        magnitude=0;
        edgeDir=scanDir=100;
        x=_x; y=_y;
        xFrac=yFrac=0.0;
   }
   void SetXY(int _x, int _y, int _edgeDir)
   {
       x=_x; y=_y; edgeDir=scanDir=_edgeDir;
   }
}BoundaryEdge;

typedef struct tagEdgeDesc
{
    PPOINT      pCurData;
    int         pntsNumber;
    int         minX,maxX,minY,maxY;
    int         aintensity;
}EdgeDesc;

typedef struct tag_lineparam
{
    bool   isline;
    bool   vertical;
    double meanX, meanY ; //, meanXY, meanX2, meanY2;
    double meandx, meandy;
    double meanddx, meanddy;
    double a,b;
    double da,db;
}lineparam,*plineparam;

typedef struct tag_stroke
{
    int     type;
    int     length;
    CPoint  start;
}stroke,*pstroke;

class CLineDetector : public CClusters
{
private:
// settings
        int         m_ScanStep;
        long        m_maxLength;
        int         m_MagnitudeThreshold;
        int			m_BorderOffset;
// temporary data
        idata      *m_pMagnitude;
        idata      *m_pDirections;
        idata      *m_pMaxMap;
        EdgeDesc    m_EdgeL, m_EdgeR;
private:
        void    Add();
        void    EstimateBoundaries();
        bool    SeekNextStep(BoundaryEdge& be, int dx, int dy);
        void    TrailEdge(BoundaryEdge& be, int forward);
        void    SeekClusterAt(int x, int y);
        void    DrawPixel(int x, int y, LPBYTE Data, int width);
        bool    GetMagnitude(BoundaryEdge& be)
        {
            if ((be.x>0) && (be.x<m_Width) && (be.y>0) && (be.y<m_Height) && (m_pMaxMap[be.y*m_Width+be.x]!=0))
            {
                be.magnitude =*(m_pMagnitude+be.y*m_Width+be.x);
                be.edgeDir   =*(m_pDirections+be.y*m_Width+be.x);
                return true;
            }
            be.magnitude = 0;
            return false;
        }
public:
                CLineDetector();
               ~CLineDetector(); 
        bool    SeekClusters();
};

void LineParams(pClustersInfo pCI);
void SeekDuplicateLines(pClustersInfo pCI);

#endif // !defined(LINEDETECTOR_INC_)