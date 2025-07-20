// MxReader.h: interface for the CMxReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MXREADER_H__75B9E266_C35B_4C0D_A49A_48D9706BC145__INCLUDED_)
#define AFX_MXREADER_H__75B9E266_C35B_4C0D_A49A_48D9706BC145__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\tvframe.h>

typedef double DmtxColor3;
typedef unsigned char DmtxPixel;

#define DMTX_FAILURE           0
#define DMTX_SUCCESS           1

#define DMTX_EDGE_STEP_TOO_WEAK       1
#define DMTX_EDGE_STEP_PERPENDICULAR  2
#define DMTX_EDGE_STEP_NOT_QUITE      3
#define DMTX_EDGE_STEP_TOO_FAR        4
#define DMTX_EDGE_STEP_EXACT          5

#ifndef M_PI
#define M_PI 3.14159265358979323846 	// 
#define M_E 2.7182818284590452354 	//
#define M_LOG2E 1.4426950408889634074 	//log_2 e 
#define M_LOG10E 0.43429448190325182765 //lg e 
#define M_LN2 0.69314718055994530942 	//ln 2 
#define M_LN10 2.30258509299404568402 	//ln 10 
#define M_PI_2 1.57079632679489661923 	// 
#define M_PI_4 0.78539816339744830962 	// 
#define M_1_PI 0.31830988618379067154 	//1
#define M_2_PI 0.63661977236758134308 	// 
#define M_SQRTPI 1.77245385090551602729 //
#define M_2_SQRTPI 1.12837916709551257390 	//2/sqrt) 
#define M_SQRT2 1.41421356237309504880 	//sqrt(2) 
#define M_SQRT3 1.73205080756887729352 	//sqrt(3) [4.0.2] 
#define M_SQRT1_2 0.70710678118654752440 	//1/sqrt(2) 
#define M_LNPI 1.14472988584940017414 	//ln 
#define M_EULER 0.57721566490153286061 	//
#endif

#define DMTX_SYMBOL_SQUARE_AUTO  -1
#define DMTX_SYMBOL_SQUARE_COUNT 24
#define DMTX_SYMBOL_RECT_AUTO    -2
#define DMTX_SYMBOL_RECT_COUNT    6

#define DMTX_MODULE_ON_RGB     0x07  /* ON_RED | ON_GREEN | ON_BLUE */
#define DMTX_MODULE_ON_RED     0x01
#define DMTX_MODULE_ON_GREEN   0x02
#define DMTX_MODULE_ON_BLUE    0x04
#define DMTX_MODULE_ASSIGNED   0x10
#define DMTX_MODULE_VISITED    0x20
#define DMTX_MODULE_DATA       0x40

typedef enum {
   DmtxEdgeTop    = 0x01 << 0,
   DmtxEdgeBottom = 0x01 << 1,
   DmtxEdgeLeft   = 0x01 << 2,
   DmtxEdgeRight  = 0x01 << 3
} DmtxEdgeLoc;

typedef enum {
   DmtxDirNone       = 0x00,
   DmtxDirUp         = 0x01 << 0,
   DmtxDirLeft       = 0x01 << 1,
   DmtxDirDown       = 0x01 << 2,
   DmtxDirRight      = 0x01 << 3,
   DmtxDirHorizontal = DmtxDirLeft  | DmtxDirRight,
   DmtxDirVertical   = DmtxDirUp    | DmtxDirDown,
   DmtxDirRightUp    = DmtxDirRight | DmtxDirUp,
   DmtxDirLeftDown   = DmtxDirLeft  | DmtxDirDown
} DmtxDirection;

typedef struct DmtxVector2_struct {
   double X;
   double Y;
} DmtxVector2;

typedef struct DmtxRay2_struct {
   char        isDefined;
   double      tMin, tMax;
   DmtxVector2 p;
   DmtxVector2 v;
} DmtxRay2;

typedef struct DmtxRay3_struct {
   DmtxColor3 p;
   DmtxColor3 c;
} DmtxRay3;

typedef enum {
   DmtxCompassDirNoDir = -1,
   DmtxCompassDirNeg45 = 0x01,
   DmtxCompassDir0     = 0x02,
   DmtxCompassDir45    = 0x04,
   DmtxCompassDir90    = 0x08
} DmtxCompassDir;

#define DMTX_ALL_COMPASS_DIRS 0x0f

typedef struct DmtxCompassEdge_struct {
   double          magnitude;  /* sqrt(R^2 + G^2 + B^2) */
   DmtxColor3      intensity;
   DmtxCompassDir  edgeDir;
   DmtxCompassDir  scanDir;    /* DmtxCompassDir0 | DmtxCompassDir90 */
} DmtxCompassEdge;

typedef struct DmtxEdgeSubPixel_struct {
   int             isEdge;
   int             xInt;
   int             yInt;
   double          xFrac;
   double          yFrac;
   DmtxCompassEdge compass;
} DmtxEdgeSubPixel;

typedef struct DmtxGradient_struct {
   char        isDefined;
   double      tMin, tMax, tMid;
   DmtxRay3    ray;
   DmtxColor3 color, colorPrev; /* XXX maybe these aren't appropriate variables for a gradient? */
} DmtxGradient;

typedef struct DmtxChain_struct {
   float tx, ty;
   float phi, shx;
   float scx, scy;
   float bx0, bx1;
   float by0, by1;
   float sz;
} DmtxChain;

typedef enum {
   DmtxCorner00 = 0x01 << 0,
   DmtxCorner10 = 0x01 << 1,
   DmtxCorner11 = 0x01 << 2,
   DmtxCorner01 = 0x01 << 3
} DmtxCornerLoc;

typedef double DmtxMatrix3[3][3];

typedef struct DmtxMatrix3Struct_struct {
   DmtxMatrix3 m;
} DmtxMatrix3Struct;

typedef struct DmtxCorners_struct {
   DmtxCornerLoc known; /* combination of (DmtxCorner00 | DmtxCorner10 | DmtxCorner11 | DmtxCorner01) */
   DmtxVector2 c00;
   DmtxVector2 c10;
   DmtxVector2 c11;
   DmtxVector2 c01;
} DmtxCorners;

class DmtxMatrixRegion
{
public:
   DmtxGradient    gradient;      /* Linear blend of colors between background and symbol color */
   DmtxChain       chain;         /* List of values that are used to build a transformation matrix */
   DmtxCorners     corners;       /* Corners of barcode region */
   DmtxMatrix3     raw2fit;       /* 3x3 transformation from raw image to fitted barcode grid */
   DmtxMatrix3     fit2raw;       /* 3x3 transformation from fitted barcode grid to raw image */
   int             sizeIdx;       /* Index of arrays that store Data Matrix constants */
   int             symbolRows;    /* Number of total rows in symbol including alignment patterns */
   int             symbolCols;    /* Number of total columns in symbol including alignment patterns */
   int             mappingRows;   /* Number of data rows in symbol */
   int             mappingCols;   /* Number of data columns in symbol */
   int             arraySize;     /* mappingRows * mappingCols */
   int             codeSize;      /* Size of encoded data (data words + error words) */
   int             outputSize;    /* Size of buffer used to hold decoded data */
   int             outputIdx;     /* Internal index used to store output progress */
   unsigned char   *array;        /* Pointer to internal representation of scanned Data Matrix modules */
   unsigned char   *code;         /* Pointer to internal storage of code words (data and error) */
   unsigned char   *output;       /* Pointer to internal storage of decoded output */
   DmtxMatrixRegion()
   {
       array    =NULL;
       code     =NULL;
       output   =NULL;
   }
   ~DmtxMatrixRegion()
   {
       if (array)  free(array);
       if (code)   free(code);
       if (output) free(output);
   }
};

class CMxReader  
{
protected:
    int              m_matrixCount;
    pTVFrame         m_Frame;
    //DmtxMatrixRegion m_matrix[16];
    CString          m_ResultStr[16];
public:
	        CMxReader();
	virtual ~CMxReader();
////
    void    SetImage(pTVFrame tvf);
    bool    ScanImage();
    const char* GetResult() //{ if (m_matrixCount) return (char*)(m_matrix[0].output); return "";}
                { if (m_matrixCount) return (m_ResultStr[0]); return "";}
private:
    int                 ScanLine(DmtxDirection dir, int lineNbr);
    DmtxCompassEdge     GetCompassEdge(int x, int y, int edgeScanDirs);
    DmtxEdgeSubPixel    FindZeroCrossing(int x, int y, DmtxCompassEdge compassStart);
    DmtxRay2            FollowEdge(int x, int y, DmtxEdgeSubPixel edgeStart, int forward);
    int                 MatrixRegion2AlignFirstEdge(DmtxMatrixRegion *region, DmtxEdgeSubPixel *edgeStart, DmtxRay2 ray0, DmtxRay2 ray1);
    int                 MatrixRegion2UpdateXfrms(DmtxMatrixRegion *region);
    int                 MatrixRegion2AlignEdge(DmtxMatrixRegion *region, DmtxMatrix3 postRaw2Fit, DmtxMatrix3 preFit2Raw, DmtxVector2 *p0, DmtxVector2 *p1, DmtxVector2 *pCorner, int *weakCount);
    int                 MatrixRegion2AlignSecondEdge(DmtxMatrixRegion *region);
    int                 MatrixRegion2AlignRightEdge(DmtxMatrixRegion *region);
    int                 MatrixRegion2AlignTopEdge(DmtxMatrixRegion *region);
    int                 MatrixRegion2FindSize(DmtxMatrixRegion *region);
    int                 AllocateStorage(DmtxMatrixRegion *region);
    int                 StepAlongEdge(DmtxMatrixRegion *region, DmtxVector2 *pProgress, DmtxVector2 *pExact, DmtxVector2 forward, DmtxVector2 lateral);
    int                 MatrixRegion2AlignCalibEdge(DmtxMatrixRegion *region, DmtxEdgeLoc edgeLoc, DmtxMatrix3 preFit2Raw, DmtxMatrix3 postRaw2Fit);
    DmtxColor3          ReadModuleColor(int symbolRow, int symbolCol, int sizeIdx, DmtxMatrix3 fit2raw);
    int                 PopulateArrayFromImage2(DmtxMatrixRegion *region);
};

#endif // !defined(AFX_MXREADER_H__75B9E266_C35B_4C0D_A49A_48D9706BC145__INCLUDED_)
