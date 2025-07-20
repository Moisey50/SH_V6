// MxReader.cpp: implementation of the CMxReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math\intf_sup.h>
#include <imageproc\recognition\MxReader.h>
#include <imageproc\recognition\mtrxDecode.h>

#include "DataMarixReader.h"

#define TRESHOLDMAGNITUDE 200					//starting magniture threshold
static int startRow=120;						//The row from which starting the scan (adaptive)
static int startSizeIdx=1;						//The index of the size of the matrix (adaptive)
static int tresholdMagnitude=TRESHOLDMAGNITUDE;	//the minimum magnitude of the edges to be accepted	(adaptive)


#define SUBPIXEL_MAGNITUDE_THRESHOLD 60	
#define DMTX_ALMOST_ZERO       0.0000001
#define SCANGAP				   6					//how many pixel rows to jump for each row scan

const int symbolRows[] = { 10, 12, 14, 16, 18, 20,  22,  24,  26,
32, 36, 40,  44,  48,  52,
64, 72, 80,  88,  96, 104,
120, 132, 144,
8,  8, 12,  12,  16,  16 };
const int symbolCols[] = { 10, 12, 14, 16, 18, 20,  22,  24,  26,
32, 36, 40,  44,  48,  52,
64, 72, 80,  88,  96, 104,
120, 132, 144,
18, 32, 26,  36,  36,  48 };

__forceinline int DmtxSymAttribSymbolRows(int sizeIdx)
{
	return symbolRows[sizeIdx];
}

__forceinline int DmtxSymAttribSymbolCols(int sizeIdx)
{
	return symbolCols[sizeIdx];
}

__forceinline int DmtxSymAttribDataRegionRows(int sizeIdx)
{
	return dataRegionRows[sizeIdx];
}

__forceinline int DmtxSymAttribDataRegionCols(int sizeIdx)
{
	return dataRegionCols[sizeIdx];
}

__forceinline void ClampIntRange(int *value, int min, int max) //value = adjusted coordinate
{
	//making sure that the coordinate is in the image,if not - correcting
	ASSERT(*value >= min - 1); 
	ASSERT(*value <= max + 1);

	if(*value < min)
		*value = min;
	else if(*value > max)
		*value = max;
}

// Colors and pixels

__forceinline DmtxPixel dmtxPixelFromImage(pTVFrame image, int x, int y)
{
	DmtxPixel bad = 0;

	if(x >= 0 && x < image->lpBMIH->biWidth && y >= 0 && y < image->lpBMIH->biHeight) //if we still in the image
	{
		return GetData(image)[(image->lpBMIH->biHeight-y-1) * image->lpBMIH->biWidth + x];
	}
	return bad;
}


__forceinline DmtxColor3 * dmtxColor3FromPixel(DmtxColor3 *color, DmtxPixel *pxl)
{
	*color = *pxl;
	return color;
}

__forceinline  void dmtxPixelFromColor3(DmtxPixel *pxl, DmtxColor3 *color)
{
	*pxl = (int)(*color + 0.5);
}

__forceinline  DmtxColor3 *dmtxColor3AddTo(DmtxColor3 *c1, DmtxColor3 *c2)
{
	*c1 += *c2;
	return c1;
}

__forceinline  DmtxColor3 * dmtxColor3ScaleBy(DmtxColor3 *c, double s)
{
	*c *= s;
	return c;
}

__forceinline  double dmtxColor3Mag(DmtxColor3 *c)
{
	double a=1.7320508075688772935274463415059* *c;
	return (a<0)?-a:a;
}

__forceinline  double dmtxColor3Dot(DmtxColor3 *c1, DmtxColor3 *c2)
{
	return (*c1 * *c2);
}

__forceinline  DmtxColor3 *dmtxColor3SubFrom(DmtxColor3 *c1, DmtxColor3 *c2)
{
	*c1-= *c2;
	return c1;
}

__forceinline  DmtxColor3 *dmtxColor3Sub(DmtxColor3 *cOut, DmtxColor3 *c1, DmtxColor3 *c2)
{
	*cOut = *c1;
	return dmtxColor3SubFrom(cOut, c2);
}

__forceinline  double dmtxColor3Norm(DmtxColor3 *c)
{
	double mag;

	mag = dmtxColor3Mag(c);

	ASSERT(mag!=0);
	//if(mag == 0)
	; /* Div/0 Error */

	dmtxColor3ScaleBy(c, 1/mag);

	return mag;
}

/// Vectors

__forceinline  DmtxVector2 *dmtxVector2SubFrom(DmtxVector2 *v1, DmtxVector2 *v2)
{
	v1->X -= v2->X;
	v1->Y -= v2->Y;

	return v1;
}

__forceinline  DmtxVector2 *dmtxVector2Sub(DmtxVector2 *vOut, DmtxVector2 *v1, DmtxVector2 *v2)
{
	*vOut = *v1;
	return dmtxVector2SubFrom(vOut, v2);
}

__forceinline  double dmtxVector2Mag(DmtxVector2 *v)
{
	return sqrt(v->X * v->X + v->Y * v->Y);
}

__forceinline  DmtxVector2 *dmtxVector2ScaleBy(DmtxVector2 *v, double s)
{
	v->X *= s;
	v->Y *= s;
	return v;
}

__forceinline  double dmtxVector2Norm(DmtxVector2 *v)
{
	double mag;

	mag = dmtxVector2Mag(v);

	ASSERT(mag > DMTX_ALMOST_ZERO);

	dmtxVector2ScaleBy(v, 1/mag);

	return mag;
}

__forceinline  double dmtxVector2Dot(DmtxVector2 *v1, DmtxVector2 *v2)
{
	return (v1->X * v2->X) + (v1->Y * v2->Y);
}

__forceinline  double dmtxDistanceAlongRay2(DmtxRay2 *r, DmtxVector2 *q)
{
	DmtxVector2 vSubTmp;

	/* Assumes that v is a unit vector */
	//#ifdef DEBUG
	//   if(fabs(1.0 - dmtxVector2Mag(v)) > DMTX_ALMOST_ZERO) {
	//      ; /* XXX big error goes here */
	//   }
	//#endif

	return dmtxVector2Dot(dmtxVector2Sub(&vSubTmp, q, &(r->p)), &(r->v));
}

__forceinline  double dmtxDistanceAlongRay3(DmtxRay3 *r, DmtxColor3 *q)
{
	DmtxColor3 cSubTmp;

	/* Assume that ray has a unit length of 1 */
	ASSERT(fabs(1.0 - dmtxColor3Mag(&(r->c))) < DMTX_ALMOST_ZERO);

	return dmtxColor3Dot(dmtxColor3Sub(&cSubTmp, q, &(r->p)), &(r->c));
}

__forceinline  DmtxVector2 * dmtxVector2Scale(DmtxVector2 *vOut, DmtxVector2 *v, double s)
{
	*vOut = *v;

	return dmtxVector2ScaleBy(vOut, s);
}

__forceinline  DmtxVector2 * dmtxVector2AddTo(DmtxVector2 *v1, DmtxVector2 *v2)
{
	v1->X += v2->X;
	v1->Y += v2->Y;

	return v1;
}

__forceinline  DmtxVector2 *dmtxVector2Add(DmtxVector2 *vOut, DmtxVector2 *v1, DmtxVector2 *v2)
{
	*vOut = *v1;

	return dmtxVector2AddTo(vOut, v2);
}

__forceinline  int dmtxPointAlongRay2(DmtxVector2 *point, DmtxRay2 *r, double t)
{
	DmtxVector2 vTmp;

	/* Ray should always have unit length of 1 */
	ASSERT(fabs(1.0 - dmtxVector2Mag(&(r->v))) < DMTX_ALMOST_ZERO);

	dmtxVector2Scale(&vTmp, &(r->v), t);
	dmtxVector2Add(point, &(r->p), &vTmp);

	return DMTX_SUCCESS;
}

__forceinline  void SetCornerLoc(DmtxMatrixRegion *region, DmtxCornerLoc cornerLoc, DmtxVector2 point)
{
	switch(cornerLoc) 
	{
	case DmtxCorner00:
		region->corners.c00 = point;
		break;
	case DmtxCorner10:
		region->corners.c10 = point;
		break;
	case DmtxCorner11:
		region->corners.c11 = point;
		break;
	case DmtxCorner01:
		region->corners.c01 = point;
		break;
	}
	region->corners.known = (DmtxCornerLoc)(((int)region->corners.known) | ((int)cornerLoc));
}

__forceinline  double dmtxVector2Cross(DmtxVector2 *v1, DmtxVector2 *v2)
{
	return (v1->X * v2->Y) - (v1->Y * v2->X);
}

__forceinline  int dmtxRay2Intersect(DmtxVector2 *point, DmtxRay2 *p0, DmtxRay2 *p1)
{
	double numer, denom;
	DmtxVector2 w;

	denom = dmtxVector2Cross(&(p1->v), &(p0->v));
	if(fabs(denom) < DMTX_ALMOST_ZERO)
		return DMTX_FAILURE;

	dmtxVector2Sub(&w, &(p1->p), &(p0->p));
	numer = dmtxVector2Cross(&(p1->v), &w);

	return dmtxPointAlongRay2(point, p0, numer/denom);
}


__forceinline void dmtxMatrix3Copy(DmtxMatrix3 m0, DmtxMatrix3 m1)
{
	*(DmtxMatrix3Struct *)m0 = *(DmtxMatrix3Struct *)m1;
}

/**
* Create Identity Transformation
*
*      | 1  0  0 |
*  m = | 0  1  0 |
*      | 0  0  1 |
*
*                  Transform "m"
*            (doesn't change anything)
*                       |\
*  (0,1)  x----o     +--+ \    (0,1)  x----o
*         |    |     |     \          |    |
*         |    |     |     /          |    |
*         +----*     +--+ /           +----*
*  (0,0)     (1,0)      |/     (0,0)     (1,0)
*
*/
__forceinline void dmtxMatrix3Identity(DmtxMatrix3 m)
{
	static DmtxMatrix3 tmp = { {1, 0, 0},
	{0, 1, 0},
	{0, 0, 1} };
	dmtxMatrix3Copy(m, tmp);
}
/**
* Translate Transformation
*
*      | 1  0  0 |
*  m = | 0  1  0 |
*      | tx ty 1 |
*
*                  Transform "m"
*                      _____    (tx,1+ty)  x----o  (1+tx,1+ty)
*                      \   |               |    |
*  (0,1)  x----o       /   |      (0,1)  +-|--+ |
*         |    |      /  /\|             | +----*  (1+tx,ty)
*         |    |      \ /                |    |
*         +----*       `                 +----+
*  (0,0)     (1,0)                (0,0)     (1,0)
*
*/
__forceinline  void dmtxMatrix3Translate(DmtxMatrix3 m, double tx, double ty)
{
	dmtxMatrix3Identity(m);
	m[2][0] = tx;
	m[2][1] = ty;
}

/**
* Rotate Transformation
*
*     |  cos(a)  sin(a)  0 |
* m = | -sin(a)  cos(a)  0 |
*     |  0       0       1 |
*                                       o
*                  Transform "m"      /   `
*                       ___         /       `
*  (0,1)  x----o      |/   \       x          *  (cos(a),sin(a))
*         |    |      '--   |       `        /
*         |    |        ___/          `    /  a
*         +----*                        `+  - - - - - -
*  (0,0)     (1,0)                     (0,0)
*
*/
__forceinline  void dmtxMatrix3Rotate(DmtxMatrix3 m, double angle)
{
	double sinAngle, cosAngle;

	sinAngle = sin(angle);
	cosAngle = cos(angle);

	dmtxMatrix3Identity(m);
	m[0][0] = cosAngle;
	m[0][1] = sinAngle;
	m[1][0] = -sinAngle;
	m[1][1] = cosAngle;
}

__forceinline  void dmtxMatrix3Multiply(DmtxMatrix3 mOut, DmtxMatrix3 m0, DmtxMatrix3 m1)
{
	int i, j, k;
	double val;

	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			val = 0.0;
			for(k = 0; k < 3; k++) {
				val += m0[i][k] * m1[k][j];
			}
			mOut[i][j] = val;
		}
	}
}

__forceinline  DmtxVector2 *dmtxMatrix3VMultiply(DmtxVector2 *vOut, DmtxVector2 *vIn, DmtxMatrix3 m)
{
	double w;

	vOut->X = vIn->X*m[0][0] + vIn->Y*m[1][0] + m[2][0];
	vOut->Y = vIn->X*m[0][1] + vIn->Y*m[1][1] + m[2][1];
	w = vIn->X*m[0][2] + vIn->Y*m[1][2] + m[2][2];

	ASSERT(fabs(w) > DMTX_ALMOST_ZERO);

	dmtxVector2ScaleBy(vOut, 1/w);

	return vOut;
}

__forceinline  DmtxVector2 *dmtxMatrix3VMultiplyBy(DmtxVector2 *v, DmtxMatrix3 m)
{
	DmtxVector2 vOut;

	dmtxMatrix3VMultiply(&vOut, v, m);
	*v = vOut;

	return v;
}

__forceinline  void dmtxMatrix3Shear(DmtxMatrix3 m, double shx, double shy)
{
	dmtxMatrix3Identity(m);
	m[1][0] = shx;
	m[0][1] = shy;
}

__forceinline  void dmtxMatrix3Scale(DmtxMatrix3 m, double  sx, double sy)
{
	dmtxMatrix3Identity(m);
	m[0][0] = sx;
	m[1][1] = sy;
}

__forceinline  void dmtxMatrix3MultiplyBy(DmtxMatrix3 m0, DmtxMatrix3 m1)
{
	DmtxMatrix3 mTmp;

	dmtxMatrix3Copy(mTmp, m0);
	dmtxMatrix3Multiply(m0, mTmp, m1);
}

__forceinline  void dmtxMatrix3LineSkewSide(DmtxMatrix3 m, double b0, double b1, double sz)
{
	ASSERT(b0 > DMTX_ALMOST_ZERO);

	dmtxMatrix3Identity(m);
	m[0][0] = sz/b0;
	m[1][1] = b1/b0;
	m[1][2] = (b1 - b0)/(sz*b0);
}

/**
* Line skew transformation for removing perspective
*
*     | b1/b0    0    (b1-b0)/(sz*b0) |
* m = |   0    sz/b0         0        |
*     |   0      0           1        |
*
*     (sz,b1)  o
*             /|    Transform "m"
*            / |
*           /  |        +--+
*          /   |        |  |
* (0,b0)  x    |        |  |
*         |    |      +-+  +-+
* (0,sz)  +----+       \    /    (0,sz)  x----o
*         |    |        \  /             |    |
*         |    |         \/              |    |
*         +----+                         +----+
*  (0,0)    (sz,0)                (0,0)    (sz,0)
*
*/
__forceinline  void dmtxMatrix3LineSkewTop(DmtxMatrix3 m, double b0, double b1, double sz)
{
	ASSERT(b0 > DMTX_ALMOST_ZERO);

	dmtxMatrix3Identity(m);
	m[0][0] = b1/b0;
	m[1][1] = sz/b0;
	m[0][2] = (b1 - b0)/(sz*b0);
}

__forceinline  void dmtxMatrix3LineSkewTopInv(DmtxMatrix3 m, double b0, double b1, double sz)
{
	ASSERT(b1 > DMTX_ALMOST_ZERO);

	dmtxMatrix3Identity(m);
	m[0][0] = b0/b1;
	m[1][1] = b0/sz;
	m[0][2] = (b0 - b1)/(sz*b1);
}

__forceinline  void dmtxMatrix3LineSkewSideInv(DmtxMatrix3 m, double b0, double b1, double sz)
{
	ASSERT(b1 > DMTX_ALMOST_ZERO);

	dmtxMatrix3Identity(m);
	m[0][0] = b0/sz;
	m[1][1] = b0/b1;
	m[1][2] = (b0 - b1)/(sz*b1);
}

/// helpers

__forceinline void dmtxColor3FromImage2(DmtxColor3 *color, pTVFrame image, DmtxVector2 p)
{
	int xInt, yInt;
	double xFrac, yFrac;
	DmtxColor3 clrLL, clrLR, clrUL, clrUR;
	DmtxPixel pxlLL, pxlLR, pxlUL, pxlUR;

	/* p = dmtxRemoveLensDistortion(p, image, -0.000003, 0.0); */

	xInt = (int)p.X;
	yInt = (int)p.Y;
	xFrac = p.X - xInt;
	yFrac = p.Y - yInt;

	pxlLL = dmtxPixelFromImage(image, xInt,   yInt);
	pxlLR = dmtxPixelFromImage(image, xInt+1, yInt);
	pxlUL = dmtxPixelFromImage(image, xInt,   yInt+1);
	pxlUR = dmtxPixelFromImage(image, xInt+1, yInt+1);

	dmtxColor3ScaleBy(dmtxColor3FromPixel(&clrLL, &pxlLL), (1 - xFrac) * (1 - yFrac));	//convertiong to integer and scaling
	dmtxColor3ScaleBy(dmtxColor3FromPixel(&clrLR, &pxlLR), xFrac * (1 - yFrac));
	dmtxColor3ScaleBy(dmtxColor3FromPixel(&clrUL, &pxlUL), (1 - xFrac) * yFrac);
	dmtxColor3ScaleBy(dmtxColor3FromPixel(&clrUR, &pxlUR), xFrac * yFrac);

	*color = clrLL;						//summing all the 4 surrounding pixels
	dmtxColor3AddTo(color, &clrLR);
	dmtxColor3AddTo(color, &clrUL);
	dmtxColor3AddTo(color, &clrUR);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMxReader::CMxReader():
m_Frame(NULL)
{
	m_matrixCount=0;
}

CMxReader::~CMxReader()
{

}

void CMxReader::SetImage(pTVFrame tvf)
{
	m_Frame=tvf;
}

bool    CMxReader::ScanImage()				// for scanning an image
{
	if (!m_Frame) return false;				//if there is no frame
	if (!m_Frame->lpBMIH) return false;		//if there is no frame properties
	int row;		//the number of the row that is scanned


	//parameters for scanning in both diractions
	int rowScanDirFlag=1; //1 - increase. (-1) - decrease
	int rowScanInd=0;		
	//scan every SCANGAP row up and down alternately.
	for(row = startRow ; ((row < startRow + m_Frame->lpBMIH->biHeight/3) && (row > 0) && (row < m_Frame->lpBMIH->biHeight)); rowScanInd++ , row += SCANGAP*rowScanDirFlag*rowScanInd)
	{
		rowScanDirFlag=-1*rowScanDirFlag;					//change scan diraction
		// TRACE("+++ Scan row %d %x\n",row,m_Frame);
		if (ScanLine(DmtxDirRight, row)) break;				//scan till one matrix found - sending the row to scan.
	}
	if (m_matrixCount)	//if found a matrix return true
	{
		startRow=row;
		return true;
	}
	else				//incase didn't found a matrix - return false.
	{

		if (tresholdMagnitude > TRESHOLDMAGNITUDE)	//incase the magnitude threshold is bigger than the starting threshold magnitude
			tresholdMagnitude--;					//lower the thresold
		if ( startRow > m_Frame->lpBMIH->biHeight/2-40 )	//incase the start row is higher than (image height)/2-40
			startRow--;		//lower the starting row
		else
			startRow++;		//raise the starting row

		return false;

	}
}

DmtxCompassEdge CMxReader::GetCompassEdge(int x, int y, int edgeScanDirs)
{
	int dirIdx;
	DmtxCompassDir dirVal[] = { DmtxCompassDirNeg45, DmtxCompassDir0, DmtxCompassDir45, DmtxCompassDir90 };
	int patternIdx, coefficientIdx;
	int xAdjust, yAdjust;
	double mag0, mag90;
	//static const double coefficient[] = {  0,  1,  2,  1,  0, -1, -2, -1 };		// edge detection filter
	static const double coefficient[] = {  0,  1,  1,  1,  0, -1, -1, -1 };		// edge detection filter
	static const int patternX[] =       { -1,  0,  1,  1,  1,  0, -1, -1 };		//the shift of the pixels around the scanned pixel in x
	static const int patternY[] =       { -1, -1, -1,  0,  1,  1,  1,  0 };		//the shift of the pixels around the scanned pixel in y
	DmtxPixel pixel;
	DmtxCompassEdge edge, maxEdge;
	DmtxColor3 color, black = 0.0; /* XXX move black to a global scope later */

	ASSERT(edgeScanDirs);

	/* Set maxEdge to invalid state */
	maxEdge.edgeDir = DmtxCompassDirNoDir;
	maxEdge.scanDir = DmtxCompassDirNoDir;
	maxEdge.intensity = black;
	maxEdge.magnitude = 0.0;

	mag0 = mag90 = -1.0;

	if(x <= 0 || x >= m_Frame->lpBMIH->biWidth - 1 || y <= 0 || y >= m_Frame->lpBMIH->biHeight - 1) //incase not in the image - return
		return maxEdge; /* XXX should really communicate failure with a dedicated value instead */

	/* Calculate this pixel's edge intensity for each direction (-45, 0, 45, 90) */
	for(dirIdx = 0; dirIdx <= 3; dirIdx++) {		//going over all the direction options

		/* Only scan for edge if this direction was requested */
		if(!(dirVal[dirIdx] & edgeScanDirs))	//edgeScanDirs - defines in which directions to search	
			continue;

		edge.edgeDir = dirVal[dirIdx];
		edge.scanDir = DmtxCompassDirNoDir;
		edge.intensity = black;
		edge.magnitude = 0.0;

		/* Set maxEdge on the first iteration */
		if(maxEdge.edgeDir == -1)
			maxEdge = edge;

		/* Add portion from each position in the convolution matrix pattern */
		for(patternIdx = 0; patternIdx < 8; patternIdx++) {
			xAdjust = x + patternX[patternIdx];				//calculating the edge according to the pixels around.
			yAdjust = y + patternY[patternIdx];

			/* Accommodate 1 pixel beyond edge of image with nearest neighbor value */
			ClampIntRange(&xAdjust, 0, m_Frame->lpBMIH->biWidth - 1);		//incase its outside of the image - adjusting it
			ClampIntRange(&yAdjust, 0, m_Frame->lpBMIH->biHeight - 1);

			/* Weight pixel value by appropriate coefficient in convolution matrix */
			coefficientIdx = (patternIdx - dirIdx + 8) % 8;			//spinning the edge detector filter.
			pixel = dmtxPixelFromImage(m_Frame, xAdjust, yAdjust);		//getting the pixel from the coordinates (xAdjust,yAdjust)
			dmtxColor3FromPixel(&color, &pixel);						//translating it from char to double
			dmtxColor3AddTo(&edge.intensity, dmtxColor3ScaleBy(&color, coefficient[coefficientIdx]));	//adding diraction  into the edge intensity
		}
		edge.magnitude = dmtxColor3Mag(&edge.intensity);			//magnitue = absolute (intensity * sqrt(3) )
		if(edge.edgeDir == DmtxCompassDir0)
			mag0 = edge.magnitude;				//saves the magnitude in 0 degrees
		else if(edge.edgeDir == DmtxCompassDir90)
			mag90 = edge.magnitude;			//saves the magnitude in 90 degrees

		/* Capture the strongest edge direction and its intensity */
		if(edge.magnitude > maxEdge.magnitude)
			maxEdge = edge;
	}
	if(mag0 > -1.0 && mag90 > -1.0)
	{
		maxEdge.scanDir = (mag0 > mag90) ? DmtxCompassDir0 : DmtxCompassDir90;
	}
	return maxEdge;
}

DmtxEdgeSubPixel CMxReader::FindZeroCrossing(int x, int y, DmtxCompassEdge compassStart)
{
	double accelPrev, accelNext, frac;
	DmtxCompassEdge compassPrev, compassNext;
	DmtxEdgeSubPixel subPixel;

	ASSERT(compassStart.scanDir == DmtxCompassDir0 || compassStart.scanDir == DmtxCompassDir90);

	subPixel.isEdge = 0;
	subPixel.xInt = x;
	subPixel.yInt = y;
	subPixel.xFrac = 0.0;
	subPixel.yFrac = 0.0;
	subPixel.compass = GetCompassEdge(x, y, compassStart.edgeDir);

	if(subPixel.compass.magnitude < SUBPIXEL_MAGNITUDE_THRESHOLD)			//TTT
		return subPixel;

	if(dmtxColor3Dot(&subPixel.compass.intensity, &compassStart.intensity) < 0)
		return subPixel;

	if(compassStart.scanDir == DmtxCompassDir0) 
	{
		compassPrev = GetCompassEdge(x-1, y, compassStart.edgeDir);		//get the edge of the pixel on the left
		compassNext = GetCompassEdge(x+1, y, compassStart.edgeDir);		//get the edge of the pixel on the right
	}
	else 
	{ // DmtxCompassDir90
		compassPrev = GetCompassEdge(x, y-1, compassStart.edgeDir);		//get the edge of the pixel below 
		compassNext = GetCompassEdge(x, y+1, compassStart.edgeDir);		//get the edge of the pixel above
	}

	// Calculate 2nd derivatives left and right / up and down of center
	accelPrev = subPixel.compass.magnitude - compassPrev.magnitude;
	accelNext = compassNext.magnitude - subPixel.compass.magnitude;

	/* If it looks like an edge then interpolate subpixel loc based on 0 crossing */
	if(accelPrev * accelNext < DMTX_ALMOST_ZERO) 
	{
		frac = (fabs(accelNext - accelPrev) > DMTX_ALMOST_ZERO) ?(accelPrev / (accelPrev - accelNext)) - 0.5 : 0.0;

		subPixel.isEdge = 1;
		subPixel.xFrac = (compassStart.scanDir == DmtxCompassDir0) ? frac : 0.0;
		subPixel.yFrac = (compassStart.scanDir == DmtxCompassDir90) ? frac : 0.0;
	}

	return subPixel;
}

DmtxRay2 CMxReader::FollowEdge(int x, int y, DmtxEdgeSubPixel edgeStart, int forward)
{
	int xFollow, yFollow;
	int xIncrement, yIncrement;
	DmtxEdgeSubPixel edge, edge0, edge1;
	DmtxVector2 p, pStart, pSoft, pHard, pStep;
	double strong0, strong1;
	DmtxCompassEdge compass;
	DmtxRay2 ray;

	memset(&ray, 0x00, sizeof(DmtxRay2));

	/*		//TTT - no need
	// No edge here, thanks for playing 
	if(!edgeStart.isEdge)
	return ray;
	*/
	pStart.X = edgeStart.xInt + edgeStart.xFrac;		//subpixel x location
	pStart.Y = edgeStart.yInt + edgeStart.yFrac;		//subpixel y location
	pSoft = pHard = pStart;

	edge = edgeStart;
	compass = edgeStart.compass;

	/* If we have a true edge then continue to follow it forward */
	if(compass.scanDir == DmtxCompassDir0) {
		xIncrement = 0;
		yIncrement = forward;
	}
	else {
		xIncrement = forward;
		yIncrement = 0;
	}

	xFollow = x + xIncrement;
	yFollow = y + yIncrement;

	while(edge.isEdge &&													//while still edge and still in the image
		xFollow >= 0 && xFollow < m_Frame->lpBMIH->biWidth &&
		yFollow >= 0 && yFollow < m_Frame->lpBMIH->biHeight) {

			edge = FindZeroCrossing(xFollow, yFollow, compass);				//checking if the current pixel is an edge

			if(!edge.isEdge) {
				edge0 = FindZeroCrossing(xFollow + yIncrement, yFollow + xIncrement, compass);		//try going 45 degrees
				edge1 = FindZeroCrossing(xFollow - yIncrement, yFollow - xIncrement, compass);		//try going -45 degrees
				if(edge0.isEdge && edge1.isEdge) {		//incase there are edges in both 45 *and* -45 diraction pixels - look for the stronger
					strong0 = dmtxColor3Dot(&edge0.compass.intensity, &compass.intensity);
					strong1 = dmtxColor3Dot(&edge1.compass.intensity, &compass.intensity);
					edge = (strong0 > strong1) ? edge0 : edge1;
				}
				else {
					edge = (edge0.isEdge) ? edge0 : edge1;	//incase the pixel in 45 *or* -45 diraction is an edges
				}

				if(!edge.isEdge) {		//incase no edges in all the pixels in all the diractions
					edge0 = FindZeroCrossing(xFollow + 2*yIncrement, yFollow + 2*xIncrement, compass);	//look for edges in pixel in ~26.5 degrees
					edge1 = FindZeroCrossing(xFollow - 2*yIncrement, yFollow - 2*xIncrement, compass);	//look for edges in pixel in ~(-26.5) degrees
					if(edge0.isEdge && edge1.isEdge) {	//incase both are edges look for the stronger one
						strong0 = dmtxColor3Dot(&edge0.compass.intensity, &compass.intensity);
						strong1 = dmtxColor3Dot(&edge1.compass.intensity, &compass.intensity);
						edge = (strong0 > strong1) ? edge0 : edge1;
					}
					else {
						edge = (edge0.isEdge) ? edge0 : edge1;	//incase the pixel in ~26.5 *or* ~(-26.5) diraction is an edges
					}
				}
			}

			if(edge.isEdge) {
				p.X = edge.xInt + edge.xFrac;		//save the position of the current end of the edge pixels
				p.Y = edge.yInt + edge.yFrac;

				/* Outline of follower in 2nd pane */
				/*       CALLBACK_DECODE_FUNC4(plotPointCallback, decode, p, 1, 1, DMTX_DISPLAY_POINT); */

				if(edge.compass.magnitude > 0.50 * compass.magnitude)
					pSoft = p;
				if(edge.compass.magnitude > 0.90 * compass.magnitude)
					pHard = p;
			}

			xFollow = (int)(edge.xInt + edge.xFrac + 0.5) + xIncrement;		//go to the next pixel in the ray
			yFollow = (int)(edge.yInt + edge.yFrac + 0.5) + yIncrement;
	}

	/* CALLBACK_DECODE_FUNC4(plotPointCallback, decode, pHard, 1, 1, DMTX_DISPLAY_SQUARE); */

	dmtxVector2Sub(&pStep, &pHard, &pStart);
	if(dmtxVector2Mag(&pStep) < 4)		//incase the ray is shorter than 4 (hard edge) (calculating the distance)
		return ray;

	ray.isDefined = 1;
	ray.tMin = 0;
	ray.p = pStart;				//the starting point of the ray
	dmtxVector2Norm(&pStep);	//normalizing the point to create the diraction vector
	ray.v = pStep;
	ray.tMax = dmtxDistanceAlongRay2(&ray, &pSoft);		//calculating the distance between the begining the the end of the edge pixels in subpixel distance.

	return ray;
}

int CMxReader::MatrixRegion2UpdateXfrms(DmtxMatrixRegion *region)
{
	DmtxVector2 v01, vTmp, vCenter, pCenter;			//v01 - is the difference bitween corner00 and corner01
	double tx, ty, phi, shx, scx, scy, skx, sky;
	double dimOT, dimOX, dimOR, dimRT, dimTX, dimRX;
	DmtxMatrix3 m, mtxy, mphi, mshx, mscxy, msky, mskx;
	DmtxCorners corners;

	ASSERT((region->corners.known & DmtxCorner00) && (region->corners.known & DmtxCorner01));

	/* Make copy of known corners to update with temporary values */
	corners = region->corners;

	if(corners.c00.X < 0.0 || corners.c00.Y < 0.0 ||			// incase one of the corners isn't in the image - fail
		corners.c01.X < 0.0 || corners.c01.Y < 0.0)
		return DMTX_FAILURE;

	if(corners.c00.X > m_Frame->lpBMIH->biWidth - 1 || corners.c00.Y > m_Frame->lpBMIH->biHeight - 1 ||
		corners.c01.X > m_Frame->lpBMIH->biWidth - 1 || corners.c01.Y > m_Frame->lpBMIH->biHeight - 1)
		return DMTX_FAILURE;

	dimOT = dmtxVector2Mag(dmtxVector2Sub(&v01, &corners.c01, &corners.c00)); /* XXX could use MagSquared() */ //updates v01 = corners.c01 - corners.c00
	if(dimOT < 8)					//making sure the the distance between the corners is bigger than 8
		return DMTX_FAILURE;

	/* Bottom-right corner -- validate if known or create temporary value */
	if(corners.known & DmtxCorner10) {
		if(corners.c10.X < 0.0 || corners.c10.Y < 0.0 ||
			corners.c10.X > m_Frame->lpBMIH->biWidth - 1 ||
			corners.c10.Y > m_Frame->lpBMIH->biHeight - 1)
			return DMTX_FAILURE;
	}
	else {
		vTmp.X = v01.Y;			//give temporary difference values to corner01 using corner10
		vTmp.Y = -v01.X;
		dmtxVector2Add(&corners.c10, &corners.c00, &vTmp);

		/* Choose direction that points toward center of image */
		pCenter.X = m_Frame->lpBMIH->biWidth/2.0;
		pCenter.Y = m_Frame->lpBMIH->biHeight/2.0;
		dmtxVector2Sub(&vCenter, &pCenter, &corners.c00);		//vCenter = pCenter - corners.c00

		if(dmtxVector2Dot(&vTmp, &vCenter) < 0.0)
			dmtxVector2ScaleBy(&vTmp, -1.0);
	}

	/* Top-right corner -- validate if known or create temporary value */
	if(corners.known & DmtxCorner11) {
		if(corners.c11.X < 0.0 || corners.c11.Y < 0.0 ||
			corners.c11.X > m_Frame->lpBMIH->biWidth - 1 ||
			corners.c11.Y > m_Frame->lpBMIH->biHeight - 1)
			return DMTX_FAILURE;
	}
	else {
		dmtxVector2Add(&corners.c11, &corners.c10, &v01);
	}

	/* Verify that the 4 corners define a reasonably fat quadrilateral */
	dimOX = dmtxVector2Mag(dmtxVector2Sub(&vTmp, &corners.c11, &corners.c00)); /* XXX could use MagSquared() */
	dimOR = dmtxVector2Mag(dmtxVector2Sub(&vTmp, &corners.c10, &corners.c00)); /* XXX could use MagSquared() */
	dimRT = dmtxVector2Mag(dmtxVector2Sub(&vTmp, &corners.c01, &corners.c10)); /* XXX could use MagSquared() */
	dimTX = dmtxVector2Mag(dmtxVector2Sub(&vTmp, &corners.c11, &corners.c01)); /* XXX could use MagSquared() */
	dimRX = dmtxVector2Mag(dmtxVector2Sub(&vTmp, &corners.c11, &corners.c10)); /* XXX could use MagSquared() */

	if(dimOR < 8 || dimTX < 8 || dimRX < 8 || dimOX < 11 || dimRT < 11)		//incase the distances between the corners are too small
		return DMTX_FAILURE;

	if(min(dimOX,dimRT)/max(dimOX,dimRT) < 0.5 ||			//incase the shape of the datamatrix isn't logical
		min(dimOT,dimRX)/max(dimOT,dimRX) < 0.5 ||
		min(dimOR,dimTX)/max(dimOR,dimTX) < 0.5)
		return DMTX_FAILURE;

	/* Calculate values needed for transformations */
	tx = -1 * corners.c00.X;
	ty = -1 * corners.c00.Y;
	dmtxMatrix3Translate(mtxy, tx, ty);		//creating the translation matrix

	phi = atan2(v01.X, v01.Y);
	dmtxMatrix3Rotate(mphi, phi);			//mphi - rotation matrix

	/* Update transformation with values known so far */
	dmtxMatrix3Multiply(m, mtxy, mphi);

	dmtxMatrix3VMultiply(&vTmp, &corners.c10, m);
	//ASSERT(vTmp.X > DMTX_ALMOST_ZERO);				
	if (vTmp.X < DMTX_ALMOST_ZERO)				//preventing devision by 0	
		return DMTX_FAILURE;							
	shx = -vTmp.Y / vTmp.X;
	dmtxMatrix3Shear(mshx, 0.0, shx);			//mshx - shear matrix

	scx = 1.0/vTmp.X;
	scy = 1.0/dmtxVector2Mag(&v01);
	dmtxMatrix3Scale(mscxy, scx, scy);			//mscxy - scaling matrix

	/* Update transformation with values known so far */
	dmtxMatrix3MultiplyBy(m, mshx);				//shearing
	dmtxMatrix3MultiplyBy(m, mscxy);			//scaling

	dmtxMatrix3VMultiply(&vTmp, &corners.c11, m);
	skx = vTmp.X;
	dmtxMatrix3LineSkewSide(mskx, 1.0, skx, 1.0);

	/* Update transformation with values known so far */
	dmtxMatrix3MultiplyBy(m, mskx);			//applying skew

	/* Update transformation with values known so far */
	dmtxMatrix3VMultiply(&vTmp, &corners.c11, m);
	sky = vTmp.Y;
	dmtxMatrix3LineSkewTop(msky, 1.0, sky, 1.0);

	/* Update region with final update */
	dmtxMatrix3Multiply(region->raw2fit, m, msky);

	/* Create inverse tranformation fit2raw (true matrix inverse is unnecessary) */
	dmtxMatrix3LineSkewTopInv(msky, 1.0, sky, 1.0);
	dmtxMatrix3LineSkewSideInv(mskx, 1.0, skx, 1.0);
	dmtxMatrix3Scale(mscxy, 1.0/scx, 1.0/scy);
	dmtxMatrix3Shear(mshx, 0.0, -shx);
	dmtxMatrix3Rotate(mphi, -phi);
	dmtxMatrix3Translate(mtxy, -tx, -ty);

	dmtxMatrix3Multiply(m, msky, mskx);
	dmtxMatrix3MultiplyBy(m, mscxy);
	dmtxMatrix3MultiplyBy(m, mshx);
	dmtxMatrix3MultiplyBy(m, mphi);
	dmtxMatrix3Multiply(region->fit2raw, m, mtxy);

	return DMTX_SUCCESS;
}

int CMxReader::MatrixRegion2AlignFirstEdge(DmtxMatrixRegion *region, DmtxEdgeSubPixel *edgeStart, DmtxRay2 ray0, DmtxRay2 ray1)
{
	int success;
	DmtxRay2 rayFull;
	DmtxVector2 p0={0,0}, p1, pTmp;

	/*fprintf(stdout, "MatrixRegion2AlignFirstEdge()\n"); */
	if(!ray0.isDefined && !ray1.isDefined)		//incase no rays at all
		return DMTX_FAILURE;
	else if(ray0.isDefined && ray1.isDefined) 
	{
		/* XXX test if reasonably colinear? */
		dmtxPointAlongRay2(&p0, &ray0, ray0.tMax);		//find the one end of the ray (p0)
		dmtxPointAlongRay2(&p1, &ray1, ray1.tMax);		//find the second end of the same ray (p1)
		rayFull.isDefined = 1;
		rayFull.p = p1;									//save the end pixel of the ray
		dmtxVector2Sub(&pTmp, &p0, &p1);				//calculate the diraction vector of the differnce between the rays
		dmtxVector2Norm(&pTmp);
		rayFull.v = pTmp;								//normalized vector using its length
		rayFull.tMin = 0;
		rayFull.tMax = dmtxDistanceAlongRay2(&rayFull, &p0);
		/* XXX else choose longer of the two */
	}
	else 
		rayFull = (ray0.isDefined) ? ray0 : ray1;

	/* Reject edges shorter than 10 pixels */
	if(rayFull.tMax < 10) 
		return DMTX_FAILURE;

	SetCornerLoc(region, DmtxCorner00, rayFull.p);		//c00 = rayFull.p (p1) - the beginning of the ray
	SetCornerLoc(region, DmtxCorner01, p0);				//c01 = p0			   - the end of the ray

	success = MatrixRegion2UpdateXfrms(region);
	if(!success)
		return DMTX_FAILURE;

	/* Top-right pane showing first edge fit */
	//CALLBACK_DECODE_FUNC1(buildMatrixCallback2, decode, region);

	return DMTX_SUCCESS;
}

/**
* XXX returns 3 points in raw coordinates
*
* @param
* @return XXX
*/
int CMxReader::MatrixRegion2AlignEdge(DmtxMatrixRegion *region, DmtxMatrix3 postRaw2Fit, DmtxMatrix3 preFit2Raw, DmtxVector2 *p0, DmtxVector2 *p1, DmtxVector2 *pCorner, int *weakCount)
{
	int hitCount, edgeHit, prevEdgeHit;
	DmtxVector2 c00, c10, c01;
	DmtxMatrix3 sRaw2Fit, sFit2Raw;
	DmtxVector2 forward, lateral;
	DmtxVector2 pFitExact, pFitProgress, pRawProgress, pRawExact, pLast;
	double interceptTest, intercept[8];
	DmtxVector2 adjust[8];
	double slope[8];
	int i;
	int stepsSinceStarAdjust;
	DmtxVector2 pTmp;

	/*fprintf(stdout, "MatrixRegion2AlignEdge()\n"); */
	dmtxMatrix3Multiply(sRaw2Fit, region->raw2fit, postRaw2Fit);
	dmtxMatrix3Multiply(sFit2Raw, preFit2Raw, region->fit2raw);

	/* Draw skewed image in bottom left pane */
	/* CALLBACK_DECODE_FUNC1(buildMatrixCallback3, decode, sFit2Raw); */

	/* Set starting point */
	pFitProgress.X = -0.003;
	pFitProgress.Y = 0.9;
	*pCorner = pFitExact = pFitProgress;
	dmtxMatrix3VMultiply(&pRawProgress, &pFitProgress, sFit2Raw);

	/* Initialize star lines */
	for(i = 0; i < 8; i++) {
		slope[i] = tan((M_PI_2/8.0) * i);		//tan(( (Pi/2)/8  )*i)
		intercept[i] = 0.9;
		adjust[i].X = 0.0;
		adjust[i].Y = 0.9;
	}

	hitCount = 0;
	stepsSinceStarAdjust = 0;
	*weakCount = 0;

	prevEdgeHit = edgeHit = DMTX_EDGE_STEP_EXACT;

	for(;;) {
		/* XXX technically we don't need to recalculate lateral & forward once we have left the finder bar */
		dmtxMatrix3VMultiply(&pTmp, &pRawProgress, sRaw2Fit);

		/* XXX still not happy with this */
		c00 = pTmp;
		c10.X = c00.X + 0.1;
		c10.Y = c00.Y;
		c01.X = c00.X - 0.0087155743; /* sin(5deg) */
		c01.Y = c00.Y + 0.0996194698; /* cos(5deg) */

		dmtxMatrix3VMultiplyBy(&c00, sFit2Raw);
		dmtxMatrix3VMultiplyBy(&c10, sFit2Raw);
		dmtxMatrix3VMultiplyBy(&c01, sFit2Raw);

		/* Calculate forward and lateral directions in raw coordinates */
		dmtxVector2Sub(&forward, &c10, &c00);
		if(dmtxVector2Mag(&forward) < DMTX_ALMOST_ZERO) /* XXX modify dmtxVector2Norm() to return failure without ASSERT */
			return 0;
		dmtxVector2Norm(&forward);

		dmtxVector2Sub(&lateral, &c01, &c00);
		if(dmtxVector2Mag(&lateral) < DMTX_ALMOST_ZERO)
			return 0;
		dmtxVector2Norm(&lateral);

		prevEdgeHit = edgeHit;
		edgeHit = StepAlongEdge(region, &pRawProgress, &pRawExact, forward, lateral);	//checks if the edge line is good
		dmtxMatrix3VMultiply(&pFitProgress, &pRawProgress, sRaw2Fit);

		if(edgeHit == DMTX_EDGE_STEP_EXACT) {
			/**
			if(prevEdgeHit == DMTX_EDGE_STEP_TOO_WEAK) <-- XXX REVIST LATER ... doesn't work
			hitCount++;
			*/
			hitCount++;

			dmtxMatrix3VMultiply(&pFitExact, &pRawExact, sRaw2Fit);

			/* Adjust star lines upward (non-vertical) */
			for(i = 0; i < 8; i++) {
				interceptTest = pFitExact.Y - slope[i] * pFitExact.X;
				if(interceptTest > intercept[i]) {
					intercept[i] = interceptTest;
					adjust[i] = pFitExact;
					stepsSinceStarAdjust = 0;

					/* XXX still "turning corner" but not as bad anymore */
					if(i == 7) {
						*pCorner = pFitExact;
						/*                CALLBACK_DECODE_FUNC4(plotPointCallback, decode, pRawExact, 1, 1, DMTX_DISPLAY_POINT); */
					}

					if(i == 0) {
						pLast = pFitExact;
						//CALLBACK_DECODE_FUNC4(plotPointCallback, decode, pRawExact, 1, 1, DMTX_DISPLAY_POINT);
					}
				}
			}

			/* Draw edge hits along skewed edge in bottom left pane */
			//CALLBACK_DECODE_FUNC4(xfrmPlotPointCallback, decode, pFitExact, NULL, 4, DMTX_DISPLAY_POINT);
		}
		else if(edgeHit == DMTX_EDGE_STEP_TOO_WEAK) {
			stepsSinceStarAdjust++;
			if(prevEdgeHit == DMTX_EDGE_STEP_TOO_WEAK)
				(*weakCount)++;
		}

		/* XXX also change stepsSinceNear and use this in the break condition */
		if(hitCount >= 20 && stepsSinceStarAdjust > hitCount)		//stop incase there are enough hits and haven't found any new for more the "hitCount" steps.
			break;

		if(pFitProgress.X > 1.0) {	//incase the steps are too big
			/*       CALLBACK_DECODE_FUNC4(plotPointCallback, decode, pRawProgress, 1, 1, DMTX_DISPLAY_SQUARE); */
			break;
		}

		if(pRawProgress.X < 1 || pRawProgress.X > m_Frame->lpBMIH->biWidth - 1 ||		//incase out of image - stop
			pRawProgress.Y < 1 || pRawProgress.Y > m_Frame->lpBMIH->biHeight - 1)
			break;
	}

	/* Find lowest available horizontal starline adjustment */
	for(i = 0; i < 8; i++) {
		if(adjust[i].X < 0.1)
			break;
	}
	if(i == 8)			//incase wasn't found
		return 0;

	*p0 = adjust[i];	//lowest available horizontal starline adjustment

	/* Find highest available non-horizontal starline adjustment */
	for(i = 7; i > -1; i--) 
	{		
		if(adjust[i].X > 0.8)
			break;
	}
	if(i == -1)
		return 0;

	*p1 = adjust[i];	//highest available non-horizontal starline adjustment

	if(fabs(p0->X - p1->X) < 0.1 || p0->Y < 0.2 || p1->Y < 0.2) {	//incase the adjustments are small (?)
		return 0;
	}

	dmtxMatrix3VMultiplyBy(pCorner, sFit2Raw);	//turning the matrix back to Raw
	dmtxMatrix3VMultiplyBy(p0, sFit2Raw);
	dmtxMatrix3VMultiplyBy(p1, sFit2Raw);

	return hitCount;
}

/**
*    T |
*      |
*      |
*      +-----
*    O      R
*
* @param
* @return XXX
*/
int CMxReader::MatrixRegion2AlignSecondEdge(DmtxMatrixRegion *region)
{
	int success;
	DmtxVector2 p0[4], p1[4], pCorner[4];
	int hitCount[4] = { 0, 0, 0, 0 };
	int weakCount[4] = { 0, 0, 0, 0 };
	DmtxMatrix3 xlate, flip, shear;
	DmtxMatrix3 preFit2Raw, postRaw2Fit;
	DmtxVector2 O, T, R;
	DmtxVector2 fitO, fitT;
	DmtxVector2 oldRawO, oldRawT;
	int i, bestFit;
	DmtxRay2 rayOT, rayNew;
	double ratio, maxRatio;

	/*fprintf(stdout, "MatrixRegion2AlignSecondEdge()\n"); */

	/* Scan top edge left-to-right (shear only)
	.---
	|

	If used, alter chain as follows:
	O = intersection of known edges
	T = farthest known point along scanned edge
	R = old O
	update tx, ty, rotate, scx, scy, shear based on this info
	*/
	dmtxMatrix3Shear(postRaw2Fit, 0.0, 1.0);
	dmtxMatrix3Shear(preFit2Raw, 0.0, -1.0);

	hitCount[0] = MatrixRegion2AlignEdge(region, postRaw2Fit, preFit2Raw, &p0[0], &p1[0], &pCorner[0], &weakCount[0]);

	/* Scan top edge right-to-left (horizontal flip and shear)
	---.
	|

	If used, alter chain as follows:
	O = intersection of known edges
	T = old O
	R = farthest known point along scanned edge
	update tx, ty, rotate, scx, scy, shear based on this info
	*/
	dmtxMatrix3Scale(flip, -1.0, 1.0);
	dmtxMatrix3Shear(shear, 0.0, 1.0);
	dmtxMatrix3Multiply(postRaw2Fit, flip, shear);

	dmtxMatrix3Shear(shear, 0.0, -1.0);
	dmtxMatrix3Scale(flip, -1.0, 1.0);
	dmtxMatrix3Multiply(preFit2Raw, shear, flip);

	hitCount[1] = MatrixRegion2AlignEdge(region, postRaw2Fit, preFit2Raw, &p0[1], &p1[1], &pCorner[1], &weakCount[1]);

	/* Scan bottom edge left-to-right (vertical flip and shear)
	|
	'---

	If used, alter chain as follows:
	O = intersection of known edges
	T = old T
	R = farthest known point along scanned edge
	update tx, ty, rotate, scx, scy, shear based on this info
	*/
	dmtxMatrix3Scale(flip, 1.0, -1.0);
	dmtxMatrix3Translate(xlate, 0.0, 1.0);
	dmtxMatrix3Shear(shear, 0.0, 1.0);
	dmtxMatrix3Multiply(postRaw2Fit, flip, xlate);
	dmtxMatrix3MultiplyBy(postRaw2Fit, shear);

	dmtxMatrix3Shear(shear, 0.0, -1.0);
	dmtxMatrix3Translate(xlate, 0.0, -1.0);
	dmtxMatrix3Scale(flip, 1.0, -1.0);
	dmtxMatrix3Multiply(preFit2Raw, shear, xlate);
	dmtxMatrix3MultiplyBy(preFit2Raw, flip);

	hitCount[2] = MatrixRegion2AlignEdge(region, postRaw2Fit,preFit2Raw, &p0[2], &p1[2], &pCorner[2], &weakCount[2]);

	/* Scan bottom edge right-to-left (flip flip shear)
	|
	---'

	If used, alter chain as follows:
	O = intersection of known edges
	T = farthest known point along scanned edge
	R = old T
	update tx, ty, rotate, scx, scy, shear based on this info
	*/
	dmtxMatrix3Scale(flip, -1.0, -1.0);
	dmtxMatrix3Translate(xlate, 0.0, 1.0);
	dmtxMatrix3Shear(shear, 0.0, 1.0);
	dmtxMatrix3Multiply(postRaw2Fit, flip, xlate);
	dmtxMatrix3MultiplyBy(postRaw2Fit, shear);

	dmtxMatrix3Shear(shear, 0.0, -1.0);
	dmtxMatrix3Translate(xlate, 0.0, -1.0);
	dmtxMatrix3Scale(flip, -1.0, -1.0);
	dmtxMatrix3Multiply(preFit2Raw, shear, xlate);
	dmtxMatrix3MultiplyBy(preFit2Raw, flip);

	hitCount[3] = MatrixRegion2AlignEdge(region, postRaw2Fit, preFit2Raw, &p0[3], &p1[3], &pCorner[3], &weakCount[3]);

	/* choose orientation with highest hitCount/(weakCount + 1) ratio */
	for(i = 0; i < 4; i++) {
		ratio = (double)hitCount[i]/(weakCount[i] + 1);

		if(i == 0 || ratio > maxRatio) {
			bestFit = i;
			maxRatio = ratio;
		}
	}

	if(hitCount[bestFit] < 5)		//if there are not enough hits - fail
		return DMTX_FAILURE;

	fitT.X = 0.0;
	fitT.Y = 1.0;
	dmtxMatrix3VMultiply(&oldRawT, &fitT, region->fit2raw);

	fitO.X = 0.0;
	fitO.Y = 0.0;
	dmtxMatrix3VMultiply(&oldRawO, &fitO, region->fit2raw);

	if(bestFit == 0 || bestFit == 1)
		oldRawT = pCorner[bestFit];
	else
		oldRawO = pCorner[bestFit];

	rayOT.p = oldRawO;
	dmtxVector2Sub(&rayOT.v, &oldRawT, &oldRawO);
	dmtxVector2Norm(&rayOT.v);

	rayNew.p = p0[bestFit];
	dmtxVector2Sub(&rayNew.v, &p1[bestFit], &p0[bestFit]);
	dmtxVector2Norm(&rayNew.v);

	/* New origin is always origin of both known edges */
	dmtxRay2Intersect(&O, &rayOT, &rayNew);

	if(bestFit == 0) {
		T = p1[bestFit];
		R = oldRawO;
	}
	else if(bestFit == 1) {
		T = oldRawO;
		R = p1[bestFit];
	}
	else if(bestFit == 2) {
		T = oldRawT;
		R = p1[bestFit];
	}
	else {
		ASSERT(bestFit == 3);
		T = p1[bestFit];
		R = oldRawT;
	}

	SetCornerLoc(region, DmtxCorner00, O);
	SetCornerLoc(region, DmtxCorner01, T);
	SetCornerLoc(region, DmtxCorner10, R);

	success = MatrixRegion2UpdateXfrms(region);

	/* Skewed barcode in the bottom middle pane */
	/* CALLBACK_DECODE_FUNC1(buildMatrixCallback4, decode, region->fit2raw); */

	return success;
}

int CMxReader::MatrixRegion2AlignRightEdge(DmtxMatrixRegion *region)
{
	int success;
	DmtxMatrix3 rotate, flip, skew, scale;
	DmtxMatrix3 preFit2Raw, postRaw2Fit;

	dmtxMatrix3Rotate(rotate, -M_PI_2);
	dmtxMatrix3Scale(flip, 1.0, -1.0);
	dmtxMatrix3LineSkewTop(skew, 1.0, 0.5, 1.0);
	dmtxMatrix3Scale(scale, 1.25, 1.0);

	dmtxMatrix3Multiply(postRaw2Fit, rotate, flip);
	dmtxMatrix3MultiplyBy(postRaw2Fit, skew);
	dmtxMatrix3MultiplyBy(postRaw2Fit, scale);

	dmtxMatrix3Scale(scale, 0.8, 1.0);
	dmtxMatrix3LineSkewTopInv(skew, 1.0, 0.5, 1.0);
	dmtxMatrix3Scale(flip, 1.0, -1.0);
	dmtxMatrix3Rotate(rotate, M_PI_2);
	dmtxMatrix3Multiply(preFit2Raw, scale, skew);
	dmtxMatrix3MultiplyBy(preFit2Raw, flip);
	dmtxMatrix3MultiplyBy(preFit2Raw, rotate);

	success = MatrixRegion2AlignCalibEdge(region, DmtxEdgeRight, preFit2Raw, postRaw2Fit);

	return success;
}

int CMxReader::MatrixRegion2AlignTopEdge(DmtxMatrixRegion *region)
{
	int success;
	DmtxMatrix3 preFit2Raw, postRaw2Fit;

	dmtxMatrix3LineSkewTop(postRaw2Fit, 1.0, 0.5, 1.0);
	dmtxMatrix3LineSkewTopInv(preFit2Raw, 1.0, 0.5, 1.0);

	success = MatrixRegion2AlignCalibEdge(region, DmtxEdgeTop, preFit2Raw, postRaw2Fit);

	return success;
}

int CMxReader::MatrixRegion2FindSize(DmtxMatrixRegion *region)
{
	int sizeIdx;
	int errors[30] = { 0 };
	int minErrorsSizeIdx;
	int row, col, symbolRows, symbolCols;
	double tOff, tOn, jumpThreshold;
	DmtxColor3 colorOn, colorOff;
	DmtxColor3 colorOnAvg, colorOffAvg;
	DmtxColor3 black = 0.0;
	DmtxGradient gradient, testGradient;
	int sizeIdxAttempts[] = { startSizeIdx , 23 , 22, 21, 20, 19, 18, 17, 16, 15, 14,	//the first is the last found matrix size
		13, 29, 12, 11, 10, 28, 27,  9, 25,  8,
		26,  7,  6,  5,  4, 24,  3,  2,  1,  0 };
	int *ptr;

	/* First try all sizes to determine which sizeIdx with best contrast */
	ptr = sizeIdxAttempts;
	gradient.tMin = gradient.tMid = gradient.tMax = 0;
	do {																	
		sizeIdx = *ptr;
		symbolRows = DmtxSymAttribSymbolRows(sizeIdx);
		symbolCols = DmtxSymAttribSymbolCols(sizeIdx);

		colorOnAvg = colorOffAvg = black;

		for(row = 0, col = 0; col < symbolCols; col++) {
			colorOn = ReadModuleColor(row, col, sizeIdx, region->fit2raw);
			colorOff = ReadModuleColor(row-1, col, sizeIdx, region->fit2raw);
			dmtxColor3AddTo(&colorOnAvg, &colorOn);
			dmtxColor3AddTo(&colorOffAvg, &colorOff);
		}

		for(row = 0, col = 0; row < symbolRows; row++) {
			colorOn = ReadModuleColor( row, col, sizeIdx, region->fit2raw);
			colorOff = ReadModuleColor( row, col-1, sizeIdx, region->fit2raw);
			dmtxColor3AddTo(&colorOnAvg, &colorOn);
			dmtxColor3AddTo(&colorOffAvg, &colorOff);
		}

		dmtxColor3ScaleBy(&colorOnAvg, 1.0/(symbolRows + symbolCols));
		dmtxColor3ScaleBy(&colorOffAvg, 1.0/(symbolRows + symbolCols));

		testGradient.ray.p = colorOffAvg;
		dmtxColor3Sub(&testGradient.ray.c, &colorOnAvg, &colorOffAvg);
		dmtxColor3Norm(&testGradient.ray.c);
		testGradient.tMin = 0;
		testGradient.tMax = dmtxDistanceAlongRay3(&testGradient.ray, &colorOnAvg);
		testGradient.tMid = (testGradient.tMin + testGradient.tMax) / 2.0;

		if(testGradient.tMax > gradient.tMax)
		{
			gradient = testGradient;
			jumpThreshold = 0.4 * (gradient.tMax - gradient.tMin);
			if(jumpThreshold > 22)
				break;
		}
	} while(*(ptr++) != 0);

	if(jumpThreshold < 20)
		return DMTX_FAILURE;

	//Start with the last found and then with largest possible pattern size and work downward.  If done in other direction then false positive is possible.

	ptr = sizeIdxAttempts;
	minErrorsSizeIdx = *ptr;
	do {
		sizeIdx = *ptr;
		symbolRows = DmtxSymAttribSymbolRows(sizeIdx);
		symbolCols = DmtxSymAttribSymbolCols(sizeIdx);

		/* Test each pair of ON/OFF modules in the calibration bars */

		/* Top calibration row */
		row = symbolRows - 1;
		for(col = 0; col < symbolCols; col += 2) {
			colorOff = ReadModuleColor(row, col + 1, sizeIdx, region->fit2raw);
			tOff = dmtxDistanceAlongRay3(&gradient.ray, &colorOff);
			colorOn = ReadModuleColor(row, col, sizeIdx, region->fit2raw);
			tOn = dmtxDistanceAlongRay3(&gradient.ray, &colorOn);

			if(tOn - tOff < jumpThreshold)	//how many errors are on this ray
				errors[sizeIdx]++;

			if(errors[sizeIdx] > errors[minErrorsSizeIdx])
				break;
		}

		/* Right calibration column */
		col = symbolCols - 1;
		for(row = 0; row < symbolRows; row += 2) {
			colorOff = ReadModuleColor(row + 1, col, sizeIdx, region->fit2raw);
			tOff = dmtxDistanceAlongRay3(&gradient.ray, &colorOff);
			colorOn = ReadModuleColor( row, col, sizeIdx, region->fit2raw);
			tOn = dmtxDistanceAlongRay3(&gradient.ray, &colorOn);

			if(tOn - tOff < jumpThreshold)
				errors[sizeIdx]++;

			if(errors[sizeIdx] > errors[minErrorsSizeIdx])
				break;
		}

		if (!errors[sizeIdx])	//incase there are no errors at all - choose it
		{
			minErrorsSizeIdx = sizeIdx;
			break;					
		}
		/* Track of which sizeIdx has the fewest errors */
		if(errors[sizeIdx] < errors[minErrorsSizeIdx])
			minErrorsSizeIdx = sizeIdx;

	} while(*(ptr++) != 0);

	region->gradient = gradient;
	region->sizeIdx = minErrorsSizeIdx;

	if(errors[minErrorsSizeIdx] >= 4)		//we allow 4 or less errors
		return DMTX_FAILURE;

	return DMTX_SUCCESS;
}

int CMxReader::AllocateStorage(DmtxMatrixRegion *region)
{
	if (region->array)  free(region->array); region->array=NULL;
	if (region->code)   free(region->code);  region->code=NULL;
	if (region->output) free(region->output);region->output=NULL;

	region->symbolRows = DmtxSymAttribSymbolRows(region->sizeIdx);
	region->symbolCols = DmtxSymAttribSymbolCols(region->sizeIdx);
	region->mappingRows = DmtxSymAttribMappingMatrixRows(region->sizeIdx);
	region->mappingCols = DmtxSymAttribMappingMatrixCols(region->sizeIdx);

	region->arraySize = sizeof(unsigned char) * region->mappingRows * region->mappingCols;

	region->array = (unsigned char *)malloc(region->arraySize);
	if(region->array == NULL) {
		perror("Malloc failed");
		exit(2); /* XXX find better error handling here */
	}
	memset(region->array, 0x00, region->arraySize);

	region->codeSize = sizeof(unsigned char) *
		DmtxSymAttribDataWordLength(region->sizeIdx) +
		DmtxSymAttribErrorWordLength(region->sizeIdx);

	region->code = (unsigned char *)malloc(region->codeSize);
	if(region->code == NULL) {
		perror("Malloc failed");
		exit(2); /* XXX find better error handling here */
	}
	memset(region->code, 0x00, region->codeSize);

	/* XXX not sure if this is the right place or even the right approach.
	Trying to allocate memory for the decoded data stream and will
	initially assume that decoded data will not be larger than 2x encoded data */
	region->outputSize = sizeof(unsigned char) * region->codeSize * 10;
	region->output = (unsigned char *)malloc(region->outputSize);
	if(region->output == NULL) {
		perror("Malloc failed");
		exit(2); /* XXX find better error handling here */
	}
	memset(region->output, 0x00, region->outputSize);

	return DMTX_SUCCESS; /* XXX good read */
}


int CMxReader::PopulateArrayFromImage2(DmtxMatrixRegion *region)
{
	int col, row, rowTmp;
	int symbolRow, symbolCol;
	int dataRegionRows, dataRegionCols;
	int statusPrev, statusModule;
	double tPrev, tModule;
	double jumpThreshold;
	DmtxColor3 color;

	dataRegionRows = DmtxSymAttribDataRegionRows(region->sizeIdx);
	dataRegionCols = DmtxSymAttribDataRegionCols(region->sizeIdx);

	memset(region->array, 0x00, region->arraySize);

	jumpThreshold = 0.4 * (region->gradient.tMax - region->gradient.tMin);

	/* XXX later add additional pass in second (3rd, 4th?) with confidence value */
	for(row = 0; row < region->mappingRows; row++) {

		/* Transform mapping row to symbol row (Swap because the array's
		origin is top-left and everything else is bottom-left) */
		rowTmp = region->mappingRows - row - 1;
		symbolRow = rowTmp + 2 * (rowTmp / dataRegionRows) + 1;

		/* Capture initial tModule location from symbolCol == 0 (known finder bar) */
		color = ReadModuleColor(symbolRow, 0, region->sizeIdx, region->fit2raw);
		tModule = dmtxDistanceAlongRay3(&(region->gradient.ray), &color);
		statusModule = DMTX_MODULE_ON_RGB;

		for(col = 0; col < region->mappingCols; col++) {

			/* Transform mapping col to symbol col */
			symbolCol = col + 2 * (col / dataRegionCols) + 1;

			tPrev = tModule;
			statusPrev = statusModule;

			color = ReadModuleColor(symbolRow, symbolCol, region->sizeIdx, region->fit2raw);
			tModule = dmtxDistanceAlongRay3(&(region->gradient.ray), &color);

			if(tModule > tPrev + jumpThreshold)
				statusModule = DMTX_MODULE_ON_RGB;
			else if(tModule < tPrev - jumpThreshold)
				statusModule = 0;
			else
				statusModule = statusPrev;

			/* Value has been assigned, but not visited */
			region->array[row*region->mappingCols+col] |= statusModule;
			region->array[row*region->mappingCols+col] |= DMTX_MODULE_ASSIGNED;
		}
	}

	/* Ideal barcode drawn in lower-right (final) window pane */
	//CALLBACK_DECODE_FUNC2(finalCallback, decode, decode, region);

	return DMTX_SUCCESS;
}


int CMxReader::ScanLine(DmtxDirection dir, int lineNbr)		//scanning one line in the image - starting from the center and going to the sides.
{
	int lineOffset, travelLimit;		//line offset - x coordinate in the image
	int *x, *y;
	int success;
	DmtxEdgeSubPixel edgeStart;			//edge information
	DmtxRay2 ray0, ray1;
	DmtxCompassEdge compassEdge;
	DmtxMatrixRegion region;			//all the info on the scanned matrix

	// XXX rethink about which direction we should scan in, and how this can be controlled by user 
	ASSERT(dir == DmtxDirRight || dir == DmtxDirUp);	//stop incase the diraction isn't right or up
	ASSERT(m_Frame);			//stop incase there is no image
	ASSERT(m_Frame->lpBMIH);	//stop incase there is no image properties


	travelLimit = (dir == DmtxDirRight) ? m_Frame->lpBMIH->biWidth: m_Frame->lpBMIH->biHeight; //if right travelLimit=Width, else travelLimit=Height

	if(dir == DmtxDirRight) //if the diraction is right
	{
		x = &lineOffset;	//x is the adress of the offset in the spesific row
		y = &lineNbr;		//y is the adress of the row number
	}
	else
	{
		x = &lineNbr;		//x is the adress of the row number
		y = &lineOffset;	//y is the adress of the offset in the spesific row
	}

	int offsetScanDirflag=1; //1 - increase. (-1) - decrease
	int offsetInd=0;
	// Loop through each pixel in the scanline starting from the center and going to both sides
	for(lineOffset = (travelLimit/2) ; ((lineOffset < travelLimit) && (lineOffset > 0)); offsetInd++ , lineOffset+=offsetScanDirflag*offsetInd)
	{
		offsetScanDirflag=-1*offsetScanDirflag;		//toggle the offset scan direction

		// Test whether this pixel is sitting on an edge of any direction 
		compassEdge = GetCompassEdge(*x, *y, DMTX_ALL_COMPASS_DIRS);				//DMTX_ALL_COMPASS_DIRS - which directions to scan.
		if(compassEdge.magnitude < tresholdMagnitude)
			continue;
		//TRACE("Something found at line %d, offset %d!\n",lineNbr,lineOffset);

		// If the edge is strong then find its subpixel location 
		edgeStart = FindZeroCrossing(*x, *y, compassEdge);
		edgeStart.compass = compassEdge; // XXX for now ... need to preserve scanDir 
		if(!edgeStart.isEdge)
			continue;

		// Next follow it to the end in both directions 
		ray0 = FollowEdge( *x, *y, edgeStart, 1);
		ray1 = FollowEdge( *x, *y, edgeStart, -1);

		if (region.array)  free(region.array); region.array=NULL;
		if (region.code)   free(region.code);  region.code=NULL;
		if (region.output) free(region.output);region.output=NULL;

		memset(&region, 0x00, sizeof(DmtxMatrixRegion));

		success = MatrixRegion2AlignFirstEdge(&region, &edgeStart, ray0, ray1);	//finds two corners and builds transformation matrix for going from row to fit and backwards
		if(!success)
			continue;

		success = MatrixRegion2AlignSecondEdge(&region);	//finds another corner and updates transformation matrix
		if(!success)
			continue;

		success = MatrixRegion2AlignRightEdge(&region);	//checks if the right edge exists and good, updates transformation matrix
		if(!success)
			continue;

		success = MatrixRegion2AlignTopEdge(&region);
		if(!success)
			continue;

		// XXX When adding clipping functionality against previously
		// scanned barcodes, this is a good place to add a test for
		// collisions.  Although we tested for this early on in the jump
		// region scan, the subsequent follower and alignment steps may
		// have moved us into a collision with another matrix region.  A
		// collision at this point is grounds for immediate failure. 

		success = MatrixRegion2FindSize(&region);			//updating region.gradient
		if(!success)
			continue;

		AllocateStorage(&region);		//allocates memory for region.array	according to the size of the matrix
		PopulateArrayFromImage2(&region);	//filling region.array with the scanned matrix

		success = DecodeRegion(&region);
		if(!success)
			continue;

		// We are now holding a valid, fully decoded Data Matrix barcode.
		// Add this to the list of valid barcodes and continue searching
		// for more. 

		ASSERT(m_matrixCount<16);
		m_ResultStr[m_matrixCount++] = region.output;

		if(m_matrixCount > 0)
		{
			//adapting - remember the size of the matrix and the magnitude threshold for the next time.
			startSizeIdx=region.sizeIdx;											
			if (compassEdge.magnitude-tresholdMagnitude < 30)					
				tresholdMagnitude=int(compassEdge.magnitude)-35;		
			else if (compassEdge.magnitude-tresholdMagnitude > 65)	
				tresholdMagnitude=tresholdMagnitude+40;
			//limiting the threshold to 500
			if (tresholdMagnitude > 500)		
				tresholdMagnitude = 500;
			if (region.array)  free(region.array); region.array=NULL;
			if (region.code)   free(region.code);  region.code=NULL;
			if (region.output) free(region.output);region.output=NULL;
			break;
		}
	}
	if (region.array)  free(region.array); region.array=NULL;
	if (region.code)   free(region.code);  region.code=NULL;
	if (region.output) free(region.output);region.output=NULL;
	return m_matrixCount;
}

int CMxReader::StepAlongEdge(DmtxMatrixRegion *region, DmtxVector2 *pProgress, DmtxVector2 *pExact, DmtxVector2 forward, DmtxVector2 lateral)
{
	int x, y;
	int xToward, yToward;
	double frac, accelPrev, accelNext;
	DmtxCompassEdge compass, compassPrev, compassNext;
	DmtxVector2 vTmp;

	x = (int)(pProgress->X + 0.5);
	y = (int)(pProgress->Y + 0.5);

	*pExact = *pProgress;
	compass = GetCompassEdge(x, y, DMTX_ALL_COMPASS_DIRS);	//finding the strongest edge

	/* If pixel shows a weak edge in any direction then advance forward */
	if(compass.magnitude < 60) {
		dmtxVector2AddTo(pProgress, &forward);
		/*    CALLBACK_DECODE_FUNC4(plotPointCallback, decode, *pProgress, 1, 1, DMTX_DISPLAY_POINT); */
		return DMTX_EDGE_STEP_TOO_WEAK;
	}

	/* forward is toward edge */
	/* lateral is away from edge */

	/* Determine orthagonal step directions */
	if(compass.scanDir == DmtxCompassDir0) {
		yToward = 0;

		if(fabs(forward.X) > fabs(lateral.X))
			xToward = (forward.X > 0) ? 1 : -1;
		else
			xToward = (lateral.X > 0) ? -1 : 1;
	}
	else {
		xToward = 0;

		if(fabs(forward.Y) > fabs(lateral.Y))
			yToward = (forward.Y > 0) ? 1 : -1;
		else
			yToward = (lateral.Y > 0) ? -1 : 1;
	}

	/* Pixel shows edge in perpendicular direction */
	compassPrev = GetCompassEdge(x-xToward, y-yToward, compass.edgeDir);
	compassNext = GetCompassEdge(x+xToward, y+yToward, compass.edgeDir);

	accelPrev = compass.magnitude - compassPrev.magnitude;
	accelNext = compassNext.magnitude - compass.magnitude;

	/* If we found a strong edge then calculate the zero crossing */
	/* XXX explore expanding this test to allow a little more fudge (without screwing up edge placement later) */
	if(accelPrev * accelNext < DMTX_ALMOST_ZERO) {		//incase there is zero-crossing (there is an edge)

		dmtxVector2AddTo(pProgress, &lateral);

		frac = (fabs(accelNext - accelPrev) > DMTX_ALMOST_ZERO) ?
			(accelPrev / (accelPrev - accelNext)) - 0.5 : 0.0;

		vTmp.X = xToward;
		vTmp.Y = yToward;
		dmtxVector2ScaleBy(&vTmp, frac);
		dmtxVector2AddTo(pExact, &vTmp);

		/*    CALLBACK_DECODE_FUNC4(plotPointCallback, decode, *pExact, 2, 1, DMTX_DISPLAY_POINT); */
		return DMTX_EDGE_STEP_EXACT;
	}

	/* Passed edge */
	if(compassPrev.magnitude > compass.magnitude) {
		/*    CALLBACK_DECODE_FUNC4(plotPointCallback, decode, *pProgress, 3, 1, DMTX_DISPLAY_POINT); */
		dmtxVector2AddTo(pProgress, &lateral);	//go to the next subpixel
		return DMTX_EDGE_STEP_TOO_FAR;
	}

	/* Approaching edge but not there yet */
	/* CALLBACK_DECODE_FUNC4(plotPointCallback, decode, *pProgress, 4, 1, DMTX_DISPLAY_POINT); */
	dmtxVector2AddTo(pProgress, &forward);
	return DMTX_EDGE_STEP_NOT_QUITE;
}

int CMxReader::MatrixRegion2AlignCalibEdge(DmtxMatrixRegion *region, DmtxEdgeLoc edgeLoc, DmtxMatrix3 preFit2Raw, DmtxMatrix3 postRaw2Fit)
{
	DmtxVector2 p0, p1, pCorner;
	DmtxVector2 cFit, cBefore, cAfter, cTmp;
	int success;
	double slope;
	int hitCount;
	int weakCount;

	ASSERT(edgeLoc == DmtxEdgeTop || edgeLoc == DmtxEdgeRight);

	hitCount = MatrixRegion2AlignEdge(region, postRaw2Fit, preFit2Raw, &p0, &p1, &pCorner, &weakCount);
	if(hitCount < 2)
		return DMTX_FAILURE;

	/* Update pCorner first, tracking value before and after update */
	if(edgeLoc == DmtxEdgeRight) {
		cFit.X = 1.0;
		cFit.Y = 0.0;
		dmtxMatrix3VMultiply(&cBefore, &cFit, region->fit2raw);
		/* XXX we are probably doing a few extra ops here */
		SetCornerLoc(region, DmtxCorner10, pCorner);
		success = MatrixRegion2UpdateXfrms(region);
		if(!success)
			return DMTX_FAILURE;

		dmtxMatrix3VMultiply(&cAfter, &cFit, region->fit2raw);
	}
	else {
		cFit.X = 0.0;
		cFit.Y = 1.0;
		dmtxMatrix3VMultiply(&cBefore, &cFit, region->fit2raw);

		SetCornerLoc(region, DmtxCorner01, pCorner);
		success = MatrixRegion2UpdateXfrms(region);
		if(!success)
			return DMTX_FAILURE;

		dmtxMatrix3VMultiply(&cBefore, &cFit, region->fit2raw);
	}

	/* If pCorner's change was significant then it probably affected edge
	fit quality.  Since pCorner is now correct, a second edge alignment
	should give accurate results. */
	if(dmtxVector2Mag(dmtxVector2Sub(&cTmp, &cAfter, &cBefore)) > 20.0) {
		hitCount = MatrixRegion2AlignEdge(region, postRaw2Fit, preFit2Raw, &p0, &p1, &pCorner, &weakCount);
		if(hitCount < 2)
			return DMTX_FAILURE;
	}

	/* With reliable edge fit results now update remaining corners */
	if(edgeLoc == DmtxEdgeRight) 
	{
		dmtxMatrix3VMultiplyBy(&p0, region->raw2fit);
		dmtxMatrix3VMultiplyBy(&p1, region->raw2fit);

		ASSERT(fabs(p1.Y - p0.Y) > DMTX_ALMOST_ZERO);
		slope = (p1.X - p0.X) / (p1.Y - p0.Y);

		p0.X = p0.X - slope * p0.Y;
		p0.Y = 0.0;
		p1.X = p0.X + slope;
		p1.Y = 1.0;

		dmtxMatrix3VMultiplyBy(&p0, region->fit2raw);
		dmtxMatrix3VMultiplyBy(&p1, region->fit2raw);

		SetCornerLoc(region, DmtxCorner10, p0);
		SetCornerLoc(region, DmtxCorner11, p1);
	}
	else 
	{
		dmtxMatrix3VMultiplyBy(&p0, region->raw2fit);
		dmtxMatrix3VMultiplyBy(&p1, region->raw2fit);

		ASSERT(fabs(p1.X - p0.X) > DMTX_ALMOST_ZERO);
		slope = (p1.Y - p0.Y) / (p1.X - p0.X);

		p0.Y = p0.Y - slope * p0.X;
		p0.X = 0.0;
		p1.Y = p0.Y + slope;
		p1.X = 1.0;

		dmtxMatrix3VMultiplyBy(&p0, region->fit2raw);
		dmtxMatrix3VMultiplyBy(&p1, region->fit2raw);

		SetCornerLoc(region, DmtxCorner01, p0);
		SetCornerLoc(region, DmtxCorner11, p1);
	}
	success = MatrixRegion2UpdateXfrms(region);
	if(!success)
		return DMTX_FAILURE;

	return DMTX_SUCCESS;
}

DmtxColor3 CMxReader::ReadModuleColor(int symbolRow, int symbolCol, int sizeIdx, DmtxMatrix3 fit2raw)
{
	int i;
	int symbolRows, symbolCols;
	double sampleX[] = { 0.5, 0.3, 0.5, 0.7, 0.5 };
	double sampleY[] = { 0.5, 0.5, 0.3, 0.5, 0.7 };
	DmtxVector2 p, p0;
	DmtxColor3 cPoint, cAverage;

	cAverage = 0.0;

	symbolRows = DmtxSymAttribSymbolRows(sizeIdx);
	symbolCols = DmtxSymAttribSymbolCols(sizeIdx);

	for(i = 0; i < 5; i++) {

		p.X = (1.0/symbolCols) * (symbolCol + sampleX[i]);
		p.Y = (1.0/symbolRows) * (symbolRow + sampleY[i]);

		dmtxMatrix3VMultiply(&p0, &p, fit2raw);
		dmtxColor3FromImage2(&cPoint, m_Frame, p0);
		/*    dmtxColor3FromImage(&cPoint, &(decode->image), p0.X, p0.Y); */

		dmtxColor3AddTo(&cAverage, &cPoint);

		/*    CALLBACK_DECODE_FUNC4(plotPointCallback, decode, p0, 1, 1, DMTX_DISPLAY_POINT); */
	}
	dmtxColor3ScaleBy(&cAverage, 0.2);

	return cAverage;
}
