// MotionEstimate.h: interface for the MotionEstimate class.
//
//////////////////////////////////////////////////////////////////////

//#if !defined(AFX_MOTIONESTIMATE_H__0A832162_0950_4319_9CEC_C9C8BFAB0446__INCLUDED_)
//#define AFX_MOTIONESTIMATE_H__0A832162_0950_4319_9CEC_C9C8BFAB0446__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include "classes\dpoint.h"
#include "gadgets/FigureFrame.h"

#include "classes\drect.h"
#include "gadgets/ContainerFrame.h"


inline bool ReadStringFromFile( FILE * fr , FXString& Result )
{
  TCHAR buf[1000] ;
  if (_fgetts( buf , 1000 , fr ) )
  {
    Result = buf ;
    return true ;
  };
  return false ;
}
//a node of the net, before it sorted has only its screen coords, and an index in the primary massive
struct NOD_INFO
{
	CDPoint coords;
	int index;
};

//an axis is a vertical o horizontal line, consisting of points that have indices inside the line - from min to max
struct AXIS_INFO
{
	int min;
	int max;
	bool vert;
	//first point in the axis, that is bigger than zero (starting) one
	NOD_INFO firstpoint;
};

//line of nods
class CNodLine
{
public:
	//nod0 is a "zero" point for the line, p1 is necessary to calculate A B and C
	CNodLine(NOD_INFO nod0, CDPoint p1, CDPoint vert, CDPoint horz);
	~CNodLine();
	//line can be either vertical or horizontal
	bool m_Vert;
	//massive of nods, belonging to the line, sorted ascending
	CList<NOD_INFO> nods;
	//Ax+By+C=0
	double A, B, C;

public:
	//ads a nod into line only if it is close enough
	void Insert(NOD_INFO nod);
	//check if three points are on the same line
	static BOOL isOnLine(CDPoint p0, CDPoint p1, CDPoint p);
};

//class is needed to provide human-oriented access to data of calibration net nods and cells
//actually, we are no more interested in nods, but only in cells:
//(x,y)-cell consists of four nods: (x,y),(x+1,y),(x,y+1),(x+1,y+1)
class CCalibrNet
{ 
	//in fact, top and left MUST be less than zero, right and bottom MUST be bigger than zero
public: 
	CCalibrNet(int left, int top, int right, int bottom);
	//equals (0,0, width-1, heght-1)
	CCalibrNet(int width, int height);
	~CCalibrNet();
private:
	CRect m_BoundRect;
	DPOINT *m_pData;
	bool *m_pValid;
	int m_iHeight;
	int m_iWidth;

	//returns true (and sets pX and pY) if next cell towards dir is valid
	//if returns false - means you are on the border
	BOOL GetNextCell_Safe(int *pX, int *pY, int dir);
public:
	//get coords of (x,y) cell
	DPOINT* GetAt(int x, int y);
	//getters and setters
	int GetHeight(){return m_iHeight;}
	int GetWidth(){return m_iWidth;} 
	CRect GetBoundRect(){return m_BoundRect;}
	void SetAt(int x, int y, DPOINT* coords);
	//since several cells on the edges are not in the net actually, a cell is set to vaild only during SetAt method
	bool GetValidAt(int x, int y);

	//get direcion from (x,y)-cell towards point. prev_direction is used to reduce measurements
	int GetNextDirection(int x, int y, CDPoint point, int prev_direction = 0);
	//returns coords of a bounding cell, if point lies within tne net, or a somewhat-close-cell on the border of net, if not
	void GetNearestCell(CDPoint point, CPoint *pCoords);
};

class MapperWrapper
{
	// transformation matrices prepared during calibration
	double *m_pMatrices;
	double *m_pInvMatrices;

	//create an axis - insert into pNodsnet all points, that are on the same line that nod0 and p1. start - geoocoords of the first point on the line
	AXIS_INFO createAxis(NOD_INFO nod0, CDPoint p1, CPoint start, CMap<CPoint, CPoint, CDPoint, CDPoint> *pNodsnet);
	//returns a nod fron UNSORTED ONLY m_aNods, nearest to the point, with a "skiplist"
	NOD_INFO getNearestNod(CDPoint point, CArray<NOD_INFO> *pSkipList);

	//pure math
	inline bool Transform(double *pMatrix, CDPoint *pPoint);
public:
	MapperWrapper();
	MapperWrapper(FXString label);
	~MapperWrapper();

	//REGIME_CALIBRATE or REFIME_WORK
	int m_iRegime;
	//if not, incoming frames in work regime will be simply passed through
	bool m_bCalibrated; 

	//unsorted nods; futre calibr net
	CDPointArray m_aNods;


	bool m_bImage;
	//image coords of center of geoprojection: set from outer source - command or settings file
	CPoint m_IOrigin;
	//geocoords of my (0,0) node of net; calculated after calibration
	CDPoint m_GOrigin;
	//zoom: size of geonet cell, is set from outer source - command or settings file
	//e. g. my nod (1,1) has geocoords m_Size * (Transform(1,1) - m_GOrigin)
	CDPoint m_Size;
	
	//ORIENTATION
	CDPoint m_Vert;
	CDPoint m_Horz;
	BOOL EstimateMajorDirections(CDPointArray *pBigSpots);
	double getSqDistance(CDPoint p0, CDPoint p1);

	CContainerFrame* m_pLocalImageNetFrame;
	FXLockObject m_Lock;

	//service functions	
	//using calibrtion met, fill m_pMatrices
	bool PrepareMatrices();
	//using m_aNods, create m_Net
	bool CreateCalibrNet();
	static CPoint MapperWrapper::GetLine(CDPoint p0, CDPointArray *pList);
	static void swap(CDPoint *p1, CDPoint *p2);

	//shows whethe a nod is already inserted into net or not
	CArray<BOOL> m_SortedNods;
	
	CCalibrNet *m_Net;

	//Label of the camera used
	FXString m_Label;

	CPoint m_ImageSize;

	//interface  functions
	bool ReadMeFromFile(FILE * fr);
	bool WriteMeToFile(FILE * fw);
	bool TransformForward(CDPoint *cdPoint);
	bool TransformInverse(CDPoint *cdPoint);
	bool SetOrigin(double X, double Y);
};

class Mapper : public CFilterGadget
{
	CArray<MapperWrapper*, MapperWrapper*> m_Mappers;

	CInputConnector* m_pInputs[2];
	COutputConnector *m_pOutputs[2];

	//path to the calibration settings file 
	FXString m_sNetFile;
	//ORIENTATION
	FXString m_sSmallLabel;
	FXString m_sBigLabel;

	int GetMapperNumberByString(FXString name);
	void ParseCommand(MapperWrapper* mapper, FXString command);
	void CalculateCommonNet();
	bool MakeCommonNetForImages(int dX, int dY);
	bool PointIsInAllImages(CDPoint point);

	// Common rectangle area for all cameras in world coordinates
	CDRect m_WorldBoundRect;
	FXLockObject m_Lock;
public:
	Mapper();

	virtual int GetInputsCount() { return 2; }
	CInputConnector* GetInputConnector(int n) { return ((n<2)&&(n>=0))?m_pInputs[n]:NULL; }
	virtual int GetOutputsCount() { return 2; }
	COutputConnector* GetOutputConnector(int n) { return ((n<2)&&(n>=0))?m_pOutputs[n]:NULL; }
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
	virtual bool ScanSettings(FXString& text);

	virtual bool LoadSettingsFromFile();
	virtual bool SaveSettingsToFile();

	virtual void ShutDown();

	void onText(CDataFrame* lpData);

	virtual int DoJob();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame1);
	DECLARE_RUNTIME_GADGET(Mapper);
};
//#endif // !defined(AFX_MOTIONESTIMATE_H__0A832162_0950_4319_9CEC_C9C8BFAB0446__INCLUDED_)
