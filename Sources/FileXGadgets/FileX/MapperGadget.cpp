// MotionEstimate.cpp: implementation of the MotionEstimate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapperGadget.h"
#include <math\intf_sup.h>
#include <limits.h>
#include <algorithm>
#include "gadgets\containerframe.h"
#include <gadgets\vftempl.h>
#include "classes\drect.h"
#include <float.h>

#define THIS_MODULENAME "MapperGadget"

//in calibration regime, all points (only point figures!) of each incoming figure frame are accumulated into a masiive and then sorted into a net
#define REGIME_CALIBRATE 1 
//in work regime, all points of all incoming figures are transformed in-place according to the calibration net
#define REGIME_WORK 2

//switch gadget into calibration regime
#define COMMAND_CALIBRATE "calibrate"
//switch gadget into work regime
#define COMMAND_WORK "work"
//set origin: screen coords of geopoint (0,0)
#define COMMAND_ORIGIN "origin"
//set size: geosize of a calibration cell
#define COMMAND_SIZE "size"
//read calibr. net and prepared matrices from a settings file
#define COMMAND_READ "read"
//write calibr. net and prepared matrices into a settings file
#define COMMAND_WRITE "write"
//prepare pixel net for common visible area
#define COMMAND_MAKENET "makenet"

#define COMMAND_IMAGE "image"
#define COMMAND_WORLD "world"
//human oriented directions. not screen!
#define DIRECTION_CENTER	 0
#define DIRECTION_UP_LEFT	 1
#define DIRECTION_UP		 2
#define DIRECTION_UP_RIGHT	 3
#define DIRECTION_RIGHT		 4
#define DIRECTION_DOWN_RIGHT 5
#define DIRECTION_DOWN		 6
#define DIRECTION_DOWN_LEFT	 7
#define DIRECTION_LEFT		 8

//minimum quantity of calibration nods required for calibraion algorhytm to work
//otherwise, it is possible to use alternative creation of calibration net, consisting of 1-2 cells only.
//this task currently has no priority
#define MINIMUM_NODS 9

//minimum distance in pixels from a point to a line, necessary to link the point to that line
#define DISTANCE_EPSILON 6.

//service cheat. used for hashing
#define MULTIPLIKATOR 100000

//default origin
#define DEFAULT_ORIGIN 0
//default size
#define DEFAULT_SIZE 1

//we need to define procedure of hashing pair of screen coords to a key value
//as fabs of screen coords will NEVER be more than 50000, the key is unique. otherwise, increase multiplikator :)
template<> 
UINT AFXAPI HashKey<CPoint> (CPoint key)
{
	return key.y * MULTIPLIKATOR + key.x;
}

template<>
BOOL AFXAPI CompareElements<CPoint, CPoint> 
	(const CPoint* pElement1, const CPoint* pElement2)
{
	if ( *pElement1 == *pElement2 ) {
		// true even if pE1==pE2==NULL
		return true;
	} else {
		return false;
	}
}

//calculate A, B and C and determine m_Vert
CNodLine::CNodLine(NOD_INFO nod0, CDPoint p1, CDPoint vert, CDPoint horz)
{
	double sqrtA2B2;
	nods.RemoveAll();
	A = nod0.coords.y - p1.y;
	B = p1.x - nod0.coords.x;
	C = nod0.coords.x * p1.y - p1.x * nod0.coords.y;
	sqrtA2B2 = sqrt(A*A+B*B);
	A /= sqrtA2B2;
	B /= sqrtA2B2;
	C /= sqrtA2B2;
	nods.AddHead(nod0);
	//	nods.SetAt(0, nod0);
	//ORIENTATION
	//line is closer to horizontal
	if(fabs(B * horz.x  - A * horz.y) > fabs(B * vert.x  - A * vert.y))
	{
		m_Vert = FALSE;
		//ORIENTATION
		//check orientation
		if((A * horz.y - B * horz.x) < 0)
		{
			A = - A;
			B = -B;
			C = -C;
		}
	}
	else
	{
		m_Vert= TRUE;
		//ORIENTATION
		//check orientation
		if((A * vert.y - B * vert.x) < 0)
		{
			A = - A;
			B = -B;
			C = -C;
		}
	}
}

CNodLine::~CNodLine()
{
}

//ads a nod into line only if it is close enough
//inserts keeping sorted order
void CNodLine::Insert(NOD_INFO nod)
{
	double distance = fabs(A * nod.coords.x + B * nod.coords.y + C);
	if(distance > DISTANCE_EPSILON)
		return;
	POSITION pos = nods.GetHeadPosition();
	NOD_INFO temp;
	int i = 0;
	int count = (int)nods.GetCount();
	for(i = 0; i < count; i++)
	{
		temp = nods.GetAt(pos);
		if(temp.index == nod.index)
			return;
		if((nod.coords.y - temp.coords.y) * A - (nod.coords.x - temp.coords.x) * B < 0)
		{
			nods.InsertBefore(pos, nod);
			break;
		}
		nods.GetNext(pos);
	}
	if(i == count)
		nods.AddTail(nod);
}

//check if three points are on the same line
BOOL CNodLine::isOnLine(CDPoint p0, CDPoint p1, CDPoint p)
{
	double A = p0.y - p1.y;
	double B = p1.x - p0.x;
	double C = p0.x * p1.y - p1.x * p0.y;
	double d = fabs(A * p.x + B * p.y + C) / sqrt(A * A + B * B);
	return d <= DISTANCE_EPSILON ? TRUE : FALSE;
}

//top and left MUST be less than zero, right and bottom MUST be bigger than zero
CCalibrNet::CCalibrNet(int left, int top, int right, int bottom)
{
	m_BoundRect.SetRect(left, top, right, bottom);
	m_iWidth = right - left + 1;
	m_iHeight = bottom - top + 1;
	m_pData = new DPOINT[4 * m_iWidth * m_iHeight];	
	memset(m_pData, 0 , 4 * m_iWidth * m_iHeight * sizeof(DPOINT)); 
	m_pValid = new bool[m_iWidth * m_iHeight];
	memset(m_pValid, 0 , m_iWidth * m_iHeight * sizeof(bool));
}

CCalibrNet::CCalibrNet(int x, int y)
{
	m_BoundRect.SetRect(0, 0, x+1, y+1);
	m_iWidth = x;
	m_iHeight = y;
	m_pData = new DPOINT[4 * m_iWidth * m_iHeight];	
	memset(m_pData, 0 , 4 * m_iWidth * m_iHeight * sizeof(DPOINT)); 
	m_pValid = new bool[m_iWidth * m_iHeight];
	memset(m_pValid, 0 , m_iWidth * m_iHeight * sizeof(bool));
}

CCalibrNet::~CCalibrNet()
{
	if(m_pData)
		delete []m_pData;
	if(m_pValid)
		delete []m_pValid;
}

DPOINT* CCalibrNet::GetAt(int x, int y)
{
	if(x > m_BoundRect.right || x < m_BoundRect.left || y < m_BoundRect.top || y > m_BoundRect.bottom)
		return NULL;
	return m_pData + 4 * ((y - m_BoundRect.top) * m_iWidth + (x - m_BoundRect.left));
}

bool CCalibrNet::GetValidAt(int x, int y)
{
	if(x > m_BoundRect.right || x < m_BoundRect.left || y < m_BoundRect.top || y > m_BoundRect.bottom)
		return FALSE;
	return m_pValid[(y - m_BoundRect.top) * m_iWidth + (x - m_BoundRect.left)];
}

void CCalibrNet::SetAt(int x, int y, DPOINT* coords)
{
	if(x > m_BoundRect.right || x < m_BoundRect.left || y < m_BoundRect.top || y > m_BoundRect.bottom)
		return;
	memcpy(m_pData + 4 * ((y - m_BoundRect.top) * m_iWidth + (x - m_BoundRect.left)), coords, 4 * sizeof(DPOINT));
	m_pValid[(y - m_BoundRect.top) * m_iWidth + (x - m_BoundRect.left)] = TRUE;
}

//get direcion from (x,y)-cell towards point. prev_direction is used to reduce measurements
int CCalibrNet::GetNextDirection(int x, int y, CDPoint point, int prev_direction)
{
	DPOINT* cell = GetAt(x, y);
	//point is lower than (x,y)-(x+1,y)
	bool bDownToFirst;
	//point is upper than (x,y+1)-(x+1,y+1)
	bool bUpToThird;
	//point is righter than (x,y)-(x,y+1)
	bool bRightToSecond;
	//point is lefter than (x+1,y)-(x+1,y+1)
	bool bLeftToFourth;
	bool bOrient = (cell[2].y - cell[0].y) * (cell[1].x - cell[0].x) + (cell[2].x - cell[0].x) * (cell[0].y - cell[1].y) > 0;

	if(bOrient)
	{
	//point is lower than (x,y)-(x+1,y)
	bDownToFirst = (cell[1].y - cell[0].y) * (point.x - cell[0].x) + (cell[0].x - cell[1].x) *  (point.y - cell[0].y) > 0;
	//point is upper than (x,y+1)-(x+1,y+1)
	bUpToThird = (cell[3].y - cell[2].y) * (point.x - cell[2].x) + (cell[2].x - cell[3].x) *  (point.y - cell[2].y) < 0;
	//point is righter than (x,y)-(x,y+1)
	bRightToSecond = (cell[2].y - cell[0].y) * (point.x - cell[0].x) + (cell[0].x - cell[2].x) *  (point.y - cell[0].y) >= 0;
	//point is lefter than (x+1,y)-(x+1,y+1)
	bLeftToFourth  = (cell[3].y - cell[1].y) * (point.x - cell[1].x) + (cell[1].x - cell[3].x) *  (point.y - cell[1].y) <= 0;
	}
	else
		//if we know previous direction, several mesurements are not necessary
	{
		//point is lower than (x,y)-(x+1,y)
	bDownToFirst = (cell[1].y - cell[0].y) * (point.x - cell[0].x) + (cell[0].x - cell[1].x) *  (point.y - cell[0].y) < 0;
	//point is upper than (x,y+1)-(x+1,y+1)
	bUpToThird = (cell[3].y - cell[2].y) * (point.x - cell[2].x) + (cell[2].x - cell[3].x) *  (point.y - cell[2].y) > 0;
	//point is righter than (x,y)-(x,y+1)
	bRightToSecond = (cell[2].y - cell[0].y) * (point.x - cell[0].x) + (cell[0].x - cell[2].x) *  (point.y - cell[0].y) <= 0;
	//point is lefter than (x+1,y)-(x+1,y+1)
	bLeftToFourth  = (cell[3].y - cell[1].y) * (point.x - cell[1].x) + (cell[1].x - cell[3].x) *  (point.y - cell[1].y) >= 0;
	}
	int iDirection = 0;

	//several combinations are inconsistent - but there is no check for it here
	//upper row
	if(!bDownToFirst && bUpToThird)
	{
		if(!bRightToSecond)
			iDirection =  DIRECTION_UP_LEFT;
		else
			if(bLeftToFourth)
				iDirection = DIRECTION_UP;
			else 
				iDirection = DIRECTION_UP_RIGHT;
	}
	//middle row
	else 
		if (!bDownToFirst && !bUpToThird)
		{
			if(!bRightToSecond)
				iDirection = DIRECTION_LEFT;
			else
				if(bLeftToFourth)
					iDirection = DIRECTION_CENTER;
				else 
					iDirection = DIRECTION_RIGHT;
		}
		//lower row
		else
		{
			if(!bRightToSecond)
				iDirection = DIRECTION_DOWN_LEFT;
			else
				if(bLeftToFourth)
					iDirection = DIRECTION_DOWN;
				else 
					iDirection = DIRECTION_DOWN_RIGHT;
		}
	////we need to caltucalate fair
	//if(prev_direction == DIRECTION_CENTER)
	//{
	//	bDownToFirst = ((cell[0].y - cell[1].y) * point.x + (cell[1].x - cell[0].x) * point.y + (cell[0].x * cell[1].y - cell[1].x * cell[0].y)) >= 0 ? TRUE : FALSE;
	//	bUpToThird = ((cell[2].y - cell[3].y) * point.x + (cell[3].x - cell[2].x) * point.y + (cell[2].x * cell[3].y - cell[3].x * cell[2].y)) <= 0 ? TRUE : FALSE;
	//	bRightToSecond = ((cell[0].y - cell[2].y) * point.x + (cell[2].x - cell[0].x) * point.y + (cell[0].x * cell[2].y - cell[2].x * cell[0].y)) <= 0 ? TRUE : FALSE;
	//	bLeftToFourth = ((cell[1].y - cell[3].y) * point.x + (cell[3].x - cell[1].x) * point.y + (cell[1].x * cell[3].y - cell[3].x * cell[1].y)) >= 0 ? TRUE : FALSE;
	//}
	//else
	//	//if we know previous direction, several mesurements are not necessary
	//{
	//	//0-1
	//	if(prev_direction != DIRECTION_UP_LEFT && prev_direction != DIRECTION_UP && prev_direction != DIRECTION_UP_RIGHT)
	//		bDownToFirst = TRUE;
	//	else
	//		bDownToFirst = ((cell[0].y - cell[1].y) * point.x + (cell[1].x - cell[0].x) * point.y + (cell[0].x * cell[1].y - cell[1].x * cell[0].y)) >= 0 ? TRUE : FALSE;
	//	//2-3
	//	if(prev_direction != DIRECTION_DOWN_LEFT && prev_direction != DIRECTION_DOWN && prev_direction != DIRECTION_DOWN_RIGHT)
	//		bUpToThird = TRUE;
	//	else 
	//		bUpToThird = ((cell[2].y - cell[3].y) * point.x + (cell[3].x - cell[2].x) * point.y + (cell[2].x * cell[3].y - cell[3].x * cell[2].y)) <= 0 ? TRUE : FALSE;
	//	//0-2
	//	if(prev_direction != DIRECTION_UP_LEFT && prev_direction != DIRECTION_LEFT && prev_direction != DIRECTION_DOWN_LEFT)
	//		bRightToSecond = TRUE;
	//	else
	//		bRightToSecond = ((cell[0].y - cell[2].y) * point.x + (cell[2].x - cell[0].x) * point.y + (cell[0].x * cell[2].y - cell[2].x * cell[0].y)) <= 0 ? TRUE : FALSE;
	//	//1-3
	//	if(prev_direction != DIRECTION_UP_RIGHT && prev_direction != DIRECTION_RIGHT && prev_direction != DIRECTION_DOWN_RIGHT)
	//		bLeftToFourth = TRUE;
	//	else
	//		bLeftToFourth = ((cell[1].y - cell[3].y) * point.x + (cell[3].x - cell[1].x) * point.y + (cell[1].x * cell[3].y - cell[3].x * cell[1].y)) >= 0 ? TRUE : FALSE;
	//}
	////several combinations are inconsistent - but there is no check for it here
	////upper row
	//if(!bDownToFirst && bUpToThird)
	//{
	//	if(!bRightToSecond)
	//		iDirection =  DIRECTION_UP_LEFT;
	//	else
	//		if(bLeftToFourth)
	//			iDirection = DIRECTION_UP;
	//		else 
	//			iDirection = DIRECTION_UP_RIGHT;
	//}
	////middle row
	//else 
	//	if (bDownToFirst && bUpToThird)
	//	{
	//		if(!bRightToSecond)
	//			iDirection = DIRECTION_LEFT;
	//		else
	//			if(bLeftToFourth)
	//				iDirection = DIRECTION_CENTER;
	//			else 
	//				iDirection = DIRECTION_RIGHT;
	//	}
	//	//lower row
	//	else
	//	{
	//		if(!bRightToSecond)
	//			iDirection = DIRECTION_DOWN_LEFT;
	//		else
	//			if(bLeftToFourth)
	//				iDirection = DIRECTION_DOWN;
	//			else 
	//				iDirection = DIRECTION_DOWN_RIGHT;
	//	}
		return iDirection;
}

//returns true (and sets pX and pY) if next cell towards dir is valid
//if returns false - means you are on the border
BOOL CCalibrNet::GetNextCell_Safe(int *pX, int *pY, int dir)
{
	int x = *pX;
	int y = *pY;
	int temp = 0;
	//calculate next cell geo coords
	switch(dir)
	{
	case DIRECTION_UP_LEFT:
		x -= 1;
		y += 1;
		break;
	case DIRECTION_UP:
		y += 1;
		break;
	case DIRECTION_UP_RIGHT:
		x += 1;
		y += 1;
		break;
	case DIRECTION_RIGHT:
		x += 1;
		break;
	case DIRECTION_DOWN_RIGHT:
		x += 1;
		y -= 1;
		break;
	case DIRECTION_DOWN:
		y -= 1;
		break;
	case DIRECTION_DOWN_LEFT:
		x -= 1;
		y -= 1;
		break;
	case DIRECTION_LEFT:
		x -= 1;
		break;
	}
	if(!GetValidAt(x,y))
	{
		//if we mode diagonally, we need check for other suitable directions
		temp = y;
		y = *pY;
		if(x == (*pX) || !GetValidAt(x,y))
		{
			y = temp;
			x = *pX;
			if(!GetValidAt(x,y) || (*pY) == y)
				return FALSE;
		}
	}
	*pX = x;
	*pY = y;
	return TRUE;
}

//returns coords of a bounding cell, if point lies within tne net, or a somewhat-close-cell on the border of net, if not
void CCalibrNet::GetNearestCell(CDPoint point, CPoint *pCoords)
{
	int x = 0;
	int y = 0;
	int dir = 0;
	while(true)
	{
		dir = GetNextDirection(x, y, point, dir);
		if(dir == DIRECTION_CENTER)
			break;
		if(!GetNextCell_Safe(&x, &y, dir))
			break;
	}
	pCoords->x = x;
	pCoords->y = y;
}

// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MapperWrapper::MapperWrapper()
{
	m_bCalibrated = false;

	m_Net = NULL;
	m_pMatrices = NULL;
	m_pInvMatrices = NULL;
	m_aNods.RemoveAll();
	m_SortedNods.RemoveAll();
	m_IOrigin.x = m_IOrigin.y = DEFAULT_ORIGIN;
	m_Size.x = m_Size.y = DEFAULT_SIZE;
	m_iRegime = REGIME_CALIBRATE;
	m_bImage = TRUE;
	m_pLocalImageNetFrame = NULL;

	m_Label.Empty();
	m_ImageSize = CPoint(0,0);
}

MapperWrapper::MapperWrapper(FXString label)
{
	m_bCalibrated = false;

	m_Net = NULL;
	m_pMatrices = NULL;
	m_pInvMatrices = NULL;
	m_aNods.RemoveAll();
	m_SortedNods.RemoveAll();
	m_IOrigin.x = m_IOrigin.y = DEFAULT_ORIGIN;
	m_Size.x = m_Size.y = DEFAULT_SIZE;
	m_iRegime = REGIME_CALIBRATE;
	m_bImage = TRUE;
	m_pLocalImageNetFrame = NULL;

	m_Label = label;
	m_ImageSize = CPoint(0,0);
}

MapperWrapper::~MapperWrapper()
{
	if(m_Net)
		delete m_Net;
	if(m_pMatrices)
		delete []m_pMatrices;
	if(m_pInvMatrices)
		delete []m_pInvMatrices;

	m_Lock.Lock();
	if (m_pLocalImageNetFrame)
	{
		m_pLocalImageNetFrame->RELEASE(m_pLocalImageNetFrame);
	}
	m_Lock.Unlock();

	m_aNods.RemoveAll();
	m_SortedNods.RemoveAll();
}

//main calibration function
bool MapperWrapper::CreateCalibrNet()
{
	if(m_aNods.GetCount() <= MINIMUM_NODS)
		return FALSE;

	int i = 0, x = 0, y = 0;
	CDPoint temp0(0,0), temp1(0,0), temp2(0,0), temp3(0,0), center(0,0), temp(0,0);
	//cheat TODO
	CRect boundRect(50,50,50,50);
	DPOINT coords[4];
	memset(coords, 0, 4 * sizeof(DPOINT));
	AXIS_INFO axInfo,ayInfo, axInfo1, ayInfo1;
	NOD_INFO centerNod, nearestNod;
	//main net - of nods - CPoint - geo coords, CDPoint - screen coords
	CMap<CPoint, CPoint, CDPoint, CDPoint> nodsnet;

	//init
	m_SortedNods.RemoveAll();
	for(i = 0; i < m_aNods.GetCount();i++)
		m_SortedNods.Add(FALSE);

	//calculate boundrect of nods
	for(i = 0; i < m_aNods.GetCount(); i++)
	{
		center = m_aNods.GetAt(i);
		boundRect.left = (LONG)min(center.x, boundRect.left);
		boundRect.right = (LONG)max(center.x, boundRect.right);
		boundRect.top = (LONG)min(center.y, boundRect.top);
		boundRect.bottom = (LONG)max(center.y, boundRect.bottom);
	}
	//centerpoint
	center.x = boundRect.CenterPoint().x;
	center.y = boundRect.CenterPoint().y;

	/////////////
	//ORIENTATION
	/////////////
	//most central node
	centerNod = getNearestNod(center, NULL);
	nodsnet.SetAt(CPoint(0,0), centerNod.coords);
	m_SortedNods.SetAt(centerNod.index, TRUE);
	//get nearest to central in horz line
	CArray<NOD_INFO> skipList;
	for(i = 0; i < m_aNods.GetCount(); i++)
	{
		nearestNod = getNearestNod(centerNod.coords, &skipList);
		CDPoint vect = centerNod.coords - nearestNod.coords;
		if((vect.x * m_Horz.x + vect.y * m_Horz.y) / sqrt(vect.x * vect.x + vect.y * vect.y) < 0.85)
			skipList.Add(nearestNod);
		else 
			break;
	}
	if(i == m_aNods.GetCount())
		return FALSE;
	//sort first axis
	axInfo = createAxis(centerNod, nearestNod.coords, CPoint(0,0), &nodsnet);
	if(axInfo.firstpoint.index == INT_MAX)
		return FALSE;

	//get nearest to central in vert line
	skipList.RemoveAll();
	for(i = 0; i < m_aNods.GetCount(); i++)
	{
		nearestNod = getNearestNod(centerNod.coords, &skipList);
		CDPoint vect = centerNod.coords - nearestNod.coords;
		if((vect.x * m_Vert.x + vect.y * m_Vert.y) / sqrt(vect.x * vect.x + vect.y * vect.y) < 0.75)
			skipList.Add(nearestNod);
		else 
			break;
	}
	if(i == m_aNods.GetCount())
		return FALSE;


	ayInfo = createAxis(centerNod, nearestNod.coords, CPoint(0,0), &nodsnet);
	if(ayInfo.firstpoint.index == INT_MAX)
		return FALSE;
	/////////////
	//ORIENTAION
	/////////////

	//create vertical (1,0) axis
	centerNod = axInfo.firstpoint;
	nearestNod = getNearestNod(centerNod.coords, NULL);
	ayInfo1 = createAxis(centerNod, nearestNod.coords, CPoint(1,0), &nodsnet);
	if(ayInfo1.firstpoint.index == INT_MAX)
		return FALSE;
	//create horz (0,1) axis
	centerNod = ayInfo.firstpoint;
	nearestNod = ayInfo1.firstpoint;
	axInfo1 = createAxis(centerNod, nearestNod.coords, CPoint(0,1), &nodsnet);
	if(axInfo1.firstpoint.index == INT_MAX)
		return FALSE;

	//calculate cross bounds
	boundRect.left = max(axInfo.min, axInfo1.min);
	boundRect.right = min(axInfo.max, axInfo1.max);
	boundRect.top =  max(ayInfo.min, ayInfo1.min);
	boundRect.bottom =  min(ayInfo.max, ayInfo1.max);

	//bugfix:
	if(boundRect.left >= 0 || boundRect.right <= 0 || boundRect.top >= 0 || boundRect.bottom <= 0)
		return FALSE;

	//sort ALL other nods using our mega cross
	for(i = 0; i < m_aNods.GetCount(); i++)
	{
		if(m_SortedNods.GetAt(i))
			continue;
		temp = m_aNods.GetAt(i);
		//determine y-coord
		for(y = boundRect.top; y <= boundRect.bottom; y++)
			if(CNodLine::isOnLine(nodsnet[CPoint(0,y)],nodsnet[CPoint(1,y)], temp))
				break;
		//determine x-coord
		for(x = boundRect.left; x <= boundRect.right; x++)
			if(CNodLine::isOnLine(nodsnet[CPoint(x,0)],nodsnet[CPoint(x,1)], temp))
				break;
		//TODO: no points that lie beyond the "cross" borders are included into the net
		//that is not necessary now, but it may be added in later patches
		if(y > boundRect.bottom || x > boundRect.right)
			continue;
		nodsnet.SetAt(CPoint(x,y),temp);
		m_SortedNods.SetAt(i, TRUE);
	}
	//fill real net of cells
	if(m_Net)
		delete m_Net;
	//quantity of cells is less than quantity of nodes
	m_Net = new CCalibrNet(boundRect.left, boundRect.top, boundRect.right - 1, boundRect.bottom - 1);
	for(x = boundRect.left; x < boundRect.right; x++)
		for(y = boundRect.top; y < boundRect.bottom; y++)
		{
			//check for validity: cell is valid if ALL it's corners are
			if(!nodsnet.Lookup(CPoint(x, y), temp0) || !nodsnet.Lookup(CPoint(x + 1, y + 1), temp3))
				continue;
			if(!nodsnet.Lookup(CPoint(x + 1, y), temp1) || !nodsnet.Lookup(CPoint(x, y + 1), temp2))
				continue;
			coords[0] = temp0;
			coords[1] = temp1;
			coords[2] = temp2;
			coords[3] = temp3;
			m_Net->SetAt(x, y, coords);
		}
		return TRUE;
}

//returns a nod fron UNSORTED ONLY m_aNods, nearest to the point
NOD_INFO MapperWrapper::getNearestNod(CDPoint point, CArray<NOD_INFO> *pSkipList)
{
	//little cheat
	double distance = 10000000;
	CDPoint temp;
	NOD_INFO nearestNod;
	int j = 0;
	for(int i = 0; i < m_aNods.GetCount(); i++)
	{
		if(m_SortedNods.GetAt(i))
			continue;
		temp = m_aNods.GetAt(i);

		//ORIENTATION
		if(pSkipList)
		{
			for(j = 0; j < pSkipList->GetCount(); j++)
			{
				if(i == pSkipList->GetAt(j).index)
					break;
			}
			if(j != pSkipList->GetCount())
				continue;
		}

		if(distance > fabs(temp.x - point.x) + fabs(temp.y - point.y))
		{
			nearestNod.index = i;
			nearestNod.coords = temp;
			distance = fabs(temp.x - point.x) + fabs(temp.y - point.y);
		}
	}
	return nearestNod;
}

//create an axis - insert into pNodsnet all points, that are on the same line that nod0 and p1. start - geoocoords of the first point on the line
AXIS_INFO MapperWrapper::createAxis(NOD_INFO nod0, CDPoint p1, CPoint start, CMap<CPoint, CPoint, CDPoint, CDPoint> *pNodsnet)
{
	AXIS_INFO aInfo;
	aInfo.firstpoint.index = INT_MAX;
	CNodLine axis(nod0, p1, m_Vert, m_Horz);
	NOD_INFO tempNod;
	POSITION pos;
	int i = 0;
	aInfo.vert = axis.m_Vert;
	//fill axis with points
	for(i = 0; i < m_aNods.GetCount(); i++)
	{
		tempNod.index = i;
		tempNod.coords =  m_aNods.GetAt(i);
		axis.Insert(tempNod);
	}

	//calculate min and max (as we know the zero point - nod0)
	aInfo.min = 0;
	pos = axis.nods.GetHeadPosition();
	for(i = 0; i < axis.nods.GetCount(); i++)
	{
		tempNod = axis.nods.GetNext(pos);
		if(((tempNod.coords.y - nod0.coords.y) * axis.A - (tempNod.coords.x - nod0.coords.x) * axis.B) < 0)
			aInfo.min--;

	}
	aInfo.max = aInfo.min + (int)axis.nods.GetCount() - 1;

	//insert all nods of the line into pNodsNet
	pos = axis.nods.GetHeadPosition();
	if(axis.m_Vert)
		for(i = 0; i < axis.nods.GetCount(); i++)
		{
			tempNod = axis.nods.GetNext(pos);
			if(aInfo.min + i == start.y + 1)
				aInfo.firstpoint = tempNod;
			if(m_SortedNods.GetAt(tempNod.index))
				continue;
			pNodsnet->SetAt(CPoint(start.x, aInfo.min + i), tempNod.coords);
			//(0,1)
			m_SortedNods.SetAt(tempNod.index, TRUE);
		}
	else
		for(i = 0; i < axis.nods.GetCount(); i++)
		{
			tempNod = axis.nods.GetNext(pos);
			if(aInfo.min + i == start.x + 1)
				//(1,0)
				aInfo.firstpoint = tempNod;

			if(m_SortedNods.GetAt(tempNod.index))
				continue;
			pNodsnet->SetAt(CPoint(aInfo.min + i, start.y), tempNod.coords);
			m_SortedNods.SetAt(tempNod.index, TRUE);
		}
		return aInfo;
}

//using calibrtion met, fill m_pMatrices
bool MapperWrapper::PrepareMatrices()
{
	if(!m_Net)
		return FALSE;

	int x = 0, y = 0, iW = 0, iH = 0;
	CRect BRect;
	double det, w0, w1;
	DPOINT* pCell;
	double *pMatrix = NULL;
	double *pInvMatrix = NULL;
	double P[6], Q[6], P4[2], Q4[2], Obr[9], V[3], R[3];

	iW = m_Net->GetWidth();
	iH = m_Net->GetHeight();
	if(iW == 0 || iH == 0)
		return FALSE;
	if(m_pMatrices)
		delete []m_pMatrices;
	if(m_pInvMatrices)
		delete []m_pInvMatrices;
	m_pMatrices = new double[9 * iW * iH];
	m_pInvMatrices = new double[9 * iW * iH];
	memset(m_pMatrices, 0, 9 * iW * iH * sizeof(double));
	memset(m_pInvMatrices, 0, 9 * iW * iH * sizeof(double));
	BRect = m_Net->GetBoundRect();
	for(y = BRect.top; y <= BRect.bottom; y++)
		for(x = BRect.left; x <= BRect.right; x++)
		{
			if(!m_Net->GetValidAt(x, y))
				continue;

			pMatrix = m_pMatrices + 9 * ((y - BRect.top) * iW + (x - BRect.left));
			pInvMatrix = m_pInvMatrices + 9 * ((y - BRect.top) * iW + (x - BRect.left));
			pCell = m_Net->GetAt(x, y);

			P[0] = pCell[0].x; P[1] = pCell[1].x; P[2] = pCell[2].x; P[3] = pCell[0].y; P[4] = pCell[1].y; P[5] = pCell[2].y;
			P4[0] = pCell[3].x; P4[1] = pCell[3].y; 
			Q[0] = x; Q[1] = x + 1; Q[2] = x; Q[3] = y; Q[4] = y; Q[5] = y + 1; 
			Q4[0] = x + 1; Q4[1] = y + 1;

			//P - reverse matrix
			det = P[0]*P[4] + P[2]*P[3] + P[1]*P[5] - P[2]*P[4] - P[0]*P[5] - P[1]*P[3];

			if(fabs(det) <= 0.0001)
				continue;

			Obr[0] = (P[4] - P[5])/det;
			Obr[3] = (P[5] - P[3])/det;
			Obr[6] = (P[3] - P[4])/det;
			Obr[1] = (P[2] - P[1])/det;
			Obr[4] = (P[0] - P[2])/det;
			Obr[7] = (P[1] - P[0])/det;
			Obr[2] = (P[1]*P[5] - P[2]*P[4])/det;
			Obr[5] = (P[2]*P[3] - P[0]*P[5])/det;
			Obr[8] = (P[0]*P[4] - P[1]*P[3])/det;

			V[0] = Obr[0]*P4[0] + Obr[1]*P4[1] + Obr[2];
			V[1] = Obr[3]*P4[0] + Obr[4]*P4[1] + Obr[5];
			V[2] = Obr[6]*P4[0] + Obr[7]*P4[1] + Obr[8];

			//Q - reverse matrix
			det = Q[0]*Q[4] + Q[2]*Q[3] + Q[1]*Q[5] - Q[2]*Q[4] - Q[0]*Q[5] - Q[1]*Q[3];

			Obr[0] = (Q[4] - Q[5])/det;
			Obr[3] = (Q[5] - Q[3])/det;
			Obr[6] = (Q[3] - Q[4])/det;
			Obr[1] = (Q[2] - Q[1])/det;
			Obr[4] = (Q[0] - Q[2])/det;
			Obr[7] = (Q[1] - Q[0])/det;
			Obr[2] = (Q[1]*Q[5] - Q[2]*Q[4])/det;
			Obr[5] = (Q[2]*Q[3] - Q[0]*Q[5])/det;
			Obr[8] = (Q[0]*Q[4] - Q[1]*Q[3])/det;

			R[0] = Obr[0]*Q4[0] + Obr[1]*Q4[1] + Obr[2];
			R[1] = Obr[3]*Q4[0] + Obr[4]*Q4[1] + Obr[5];
			R[2] = Obr[6]*Q4[0] + Obr[7]*Q4[1] + Obr[8];

			w0 = (V[0]*R[2]) / (R[0]*V[2]);
			w1 = (V[1]*R[2]) / (R[1]*V[2]);

			//Q1-1 reverse matrix
			Obr[0] *= w0; Obr[1] *= w0; Obr[2] *= w0;
			Obr[3] *= w1; Obr[4] *= w1; Obr[5] *= w1;

			pInvMatrix[0] = P[0] * Obr[0] + P[1] * Obr[3] + P[2] * Obr[6];
			pInvMatrix[1] = P[0] * Obr[1] + P[1] * Obr[4] + P[2] * Obr[7];
			pInvMatrix[2] = P[0] * Obr[2] + P[1] * Obr[5] + P[2] * Obr[8];
			pInvMatrix[3] = P[3] * Obr[0] + P[4] * Obr[3] + P[5] * Obr[6];
			pInvMatrix[4] = P[3] * Obr[1] + P[4] * Obr[4] + P[5] * Obr[7];
			pInvMatrix[5] = P[3] * Obr[2] + P[4] * Obr[5] + P[5] * Obr[8];
			pInvMatrix[6] = 1 * Obr[0] + 1 * Obr[3] + 1 * Obr[6];
			pInvMatrix[7] = 1 * Obr[1] + 1 * Obr[4] + 1 * Obr[7];
			pInvMatrix[8] = 1 * Obr[2] + 1 * Obr[5] + 1 * Obr[8];

			det = pInvMatrix[0]*pInvMatrix[4]*pInvMatrix[8] + pInvMatrix[2]*pInvMatrix[3]*pInvMatrix[7] + pInvMatrix[1]*pInvMatrix[5]*pInvMatrix[6] - pInvMatrix[2]*pInvMatrix[4]*pInvMatrix[6] - pInvMatrix[0]*pInvMatrix[5]*pInvMatrix[7] - pInvMatrix[1]*pInvMatrix[3]*pInvMatrix[8];

			pMatrix[0] = (pInvMatrix[4]*pInvMatrix[8] - pInvMatrix[5]*pInvMatrix[7])/det;
			pMatrix[3] = (pInvMatrix[5]*pInvMatrix[6] - pInvMatrix[3]*pInvMatrix[8])/det;
			pMatrix[6] = (pInvMatrix[3]*pInvMatrix[7] - pInvMatrix[4]*pInvMatrix[6])/det;
			pMatrix[1] = (pInvMatrix[2]*pInvMatrix[7] - pInvMatrix[1]*pInvMatrix[8])/det;
			pMatrix[4] = (pInvMatrix[0]*pInvMatrix[8] - pInvMatrix[2]*pInvMatrix[6])/det;
			pMatrix[7] = (pInvMatrix[1]*pInvMatrix[6] - pInvMatrix[0]*pInvMatrix[7])/det;
			pMatrix[2] = (pInvMatrix[1]*pInvMatrix[5] - pInvMatrix[2]*pInvMatrix[4])/det;
			pMatrix[5] = (pInvMatrix[2]*pInvMatrix[3] - pInvMatrix[0]*pInvMatrix[5])/det;
			pMatrix[8] = (pInvMatrix[0]*pInvMatrix[4] - pInvMatrix[1]*pInvMatrix[3])/det;

		}
		return TRUE;
}

//pure math
inline bool MapperWrapper::Transform(double *pMatrix, CDPoint *pPoint)
{
	double z = pMatrix[6] * pPoint->x + pMatrix[7] * pPoint->y + pMatrix[8];
	if (z==0)
		return false;
	double x = (pMatrix[0] * pPoint->x + pMatrix[1] * pPoint->y + pMatrix[2]) / z;
	pPoint->y = (pMatrix[3] * pPoint->x + pMatrix[4] * pPoint->y + pMatrix[5]) / z;
	pPoint->x = x;
	return true;
}


//read all, except LABEL. The label is only for MapperGadget class
bool MapperWrapper::ReadMeFromFile( FILE * fr )
{
	FXString str("");
	FXString str2("");
	FXString token("");
	CRect BRect(0,0,0,0);
	double dTemp = 0;
	DPOINT pD[4];
	int iTemp = 0;
	FXSIZE curPos = 0;
	FXSIZE curPos2 = 0;
	int i = 0, j = 0, k = 0;
	int width = 0;
	int height = 0;

	//setting regime to calibrate, if no matrices is written
	m_iRegime = REGIME_CALIBRATE;

	//read image size
	if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	if(str.Compare("IMAGESIZE:"))
	{
		return FALSE;
	}

  if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	curPos = 0;
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	m_ImageSize.x = atoi(token);
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	m_ImageSize.y = atoi(token);


	//read origin
  if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	if(str.Compare("ORIGIN:"))
	{
		return FALSE;
	}
  if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	curPos = 0;
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	m_IOrigin.x = atoi(token);
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	m_IOrigin.y = atoi(token);

	//read size
  if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	if(str.Compare("SIZE:"))
	{
		return FALSE;
	}
  if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	curPos = 0;
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	m_Size.x = atof(token);
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	m_Size.y = atof(token);

  if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	if(str.Compare("NET:"))
	{
		return FALSE;
	}


	//read boundRect
  if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	curPos = 0;
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	BRect.left = atoi(token);
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	BRect.top = atoi(token);
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	BRect.right = atoi(token);
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		return FALSE;
	}
	BRect.bottom = atoi(token);

	if(BRect.left == 0 && BRect.right == 0)
	{
		return FALSE;
	}
	if(BRect.top == 0 && BRect.bottom == 0)
	{
		return FALSE;
	}

	if(m_Net)
		delete m_Net;

	m_Net = new CCalibrNet(BRect.left, BRect.top, BRect.right, BRect.bottom);
	for(i = 0; i < m_Net->GetHeight(); i++)
	{
		str.Empty();
    if (!ReadStringFromFile( fr , str ) )
		curPos = 0;
		for(j = 0; j < m_Net->GetWidth(); j++)
		{
			str2 = str.Tokenize(";", curPos);
			curPos2 = 0;
			token = str2.Tokenize((","),curPos2);
			if(token == "")
			{
				return FALSE;
			}
			if(token == "0")
			{
				continue;
			}
			for(k = 0; k < 4; k++)
			{
				token = str2.Tokenize((","),curPos2);
				if(token == "")
				{
					return FALSE;
				}
				pD[k].x = atof(token);
				token = str2.Tokenize((","),curPos2);
				if(token == "")
				{
					return FALSE;
				}
				pD[k].y = atof(token);

			}
			m_Net->SetAt(j + BRect.left, i + BRect.top, pD);
		} 
	}

  if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	if(str.Compare("MATRICES:"))
	{
		return FALSE;
	}

	if(m_pMatrices)
		delete []m_pMatrices;
	m_pMatrices = new double[9 * m_Net->GetWidth() * m_Net->GetHeight()];
	memset(m_pMatrices, 0, 9 * m_Net->GetWidth() * m_Net->GetHeight() * sizeof(double));

	for(i = 0; i < m_Net->GetHeight(); i++)
	{
		str.Empty();
    if (!ReadStringFromFile( fr , str ) )
		curPos = 0;
		for(j = 0; j < m_Net->GetWidth(); j++)
		{
			curPos2 = 0;
			str2 = str.Tokenize(";", curPos);
			for(k = 0; k < 9; k++)
			{
				token = str2.Tokenize((","),curPos2);
				if(token == "")
				{
					return FALSE;
				}
				m_pMatrices[9 * (i * m_Net->GetWidth() + j) + k] = atof(token);
			}
		}
	}

  if (!ReadStringFromFile( fr , str ) )
	{
		return FALSE;
	}
	if(str.Compare("INVMATRICES:"))
	{
		return FALSE;
	}

	if(m_pInvMatrices)
		delete []m_pInvMatrices;
	m_pInvMatrices = new double[9 * m_Net->GetWidth() * m_Net->GetHeight()];
	memset(m_pInvMatrices, 0, 9 * m_Net->GetWidth() * m_Net->GetHeight() * sizeof(double));

	for(i = 0; i < m_Net->GetHeight(); i++)
	{
		str.Empty();
    if (!ReadStringFromFile( fr , str ) )
		curPos = 0;
		for(j = 0; j < m_Net->GetWidth(); j++)
		{
			curPos2 = 0;
			str2 = str.Tokenize(";", curPos);
			for(k = 0; k < 9; k++)
			{
				token = str2.Tokenize((","),curPos2);
				if(token == "")
				{
					return FALSE;
				}
				m_pInvMatrices[9 * (i * m_Net->GetWidth() + j) + k] = atof(token);
			}
		}
	}

	//calculate geocoords of (0,0) nod
	CPoint coords(0,0);
	m_GOrigin.x = m_IOrigin.x;
	m_GOrigin.y = m_IOrigin.y;
	m_Net->GetNearestCell(m_GOrigin, &coords);
	Transform(m_pMatrices + 9 
    * ((coords.y - (m_Net->GetBoundRect().top) * m_Net->GetWidth()) 
      + coords.x  - m_Net->GetBoundRect().left ), &(m_GOrigin) );

	m_bCalibrated = TRUE;

	//Mapper is calibrated, so we can work!
	m_iRegime = REGIME_WORK;

	return true;
}

bool MapperWrapper::WriteMeToFile( FILE * fw )
{
	FXString str;
	CRect BRect(0,0,0,0);
	DPOINT *pD = NULL;
	double *pd = NULL;
	int i,j;

	str.Format("IMAGESIZE:\n");
  _fputts( (LPCTSTR)str , fw ) ;
	str.Format("%d,%d\n",m_ImageSize.x,m_ImageSize.y);
  _fputts( (LPCTSTR)str , fw ) ;

	str.Format("ORIGIN:\n");
  _fputts( (LPCTSTR)str , fw ) ;
	str.Format("%d,%d\n",m_IOrigin.x,m_IOrigin.y);
  _fputts( (LPCTSTR)str , fw ) ;

	str.Format("SIZE:\n");
  _fputts( (LPCTSTR)str , fw ) ;
	str.Format("%lf,%lf\n",m_Size.x,m_Size.y);
  _fputts( (LPCTSTR)str , fw ) ;

	str.Format("NET:\n");
  _fputts( (LPCTSTR)str , fw ) ;
	if(m_Net)
	{
		BRect = m_Net->GetBoundRect();
		str.Format("%d,%d,%d,%d\n", BRect.left, BRect.top, BRect.right, BRect.bottom);
    _fputts( (LPCTSTR)str , fw ) ;
		for(i = 0; i < m_Net->GetHeight(); i++)
		{
			str.Empty();
			for(j = 0; j < m_Net->GetWidth(); j++)
			{
				pD = m_Net->GetAt(j + BRect.left, i + BRect.top);
				str.Format("%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf;", 
          m_Net->GetValidAt(j + BRect.left, i + BRect.top), 
          (*pD).x, (*pD).y, (*(pD+1)).x, (*(pD+1)).y, 
          (*(pD+2)).x, (*(pD+2)).y, (*(pD+3)).x, (*(pD+3)).y);
        _fputts( (LPCTSTR)str , fw ) ;
			}
			str.Format("\n");
      _fputts( (LPCTSTR)str , fw ) ;
		}

		if(m_pMatrices && m_pInvMatrices)
		{
			str.Format("MATRICES:\n");
      _fputts( (LPCTSTR)str , fw ) ;

			for(i = 0; i < m_Net->GetHeight(); i++)
			{
				str.Empty();
				for(j = 0; j < m_Net->GetWidth(); j++) 
				{
					pd = m_pMatrices + 9 * (m_Net->GetWidth() * i + j);
					str.Format("%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf;", 
            (*pd), (*(pd+1)), (*(pd+2)), (*(pd+3)), (*(pd+4)), 
            (*(pd+5)), (*(pd+6)), (*(pd+7)), (*(pd+8)));
          _fputts( (LPCTSTR)str , fw ) ;
				}
				str.Format("\n");
        _fputts( (LPCTSTR)str , fw ) ;
			}

			str.Format("INVMATRICES:\n");
      _fputts( (LPCTSTR)str , fw ) ;

			for(i = 0; i < m_Net->GetHeight(); i++)
			{
				str.Empty();
				for(j = 0; j < m_Net->GetWidth(); j++) 
				{
					pd = m_pInvMatrices + 9 * (m_Net->GetWidth() * i + j);
					str.Format("%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf;",
            (*pd), (*(pd+1)), (*(pd+2)), (*(pd+3)), (*(pd+4)), 
            (*(pd+5)), (*(pd+6)), (*(pd+7)), (*(pd+8)));
          _fputts( (LPCTSTR)str , fw ) ;
				}
				str.Format("\n");
        _fputts( (LPCTSTR)str , fw ) ;
			}
		}
	}
	else
	{
		str.Format("%d,%d,%d,%d\n",0,0,0,0);
    _fputts( (LPCTSTR)str , fw ) ;
	}
	return true;
}

bool MapperWrapper::TransformForward(CDPoint *cdPoint)
{
	if (!m_Net || !m_pMatrices)
		return false;

	CPoint coords;	
	m_Net->GetNearestCell(*cdPoint, &coords);
	if (!Transform(m_pMatrices + 9 * ((coords.y - m_Net->GetBoundRect().top) * m_Net->GetWidth() + coords.x  - m_Net->GetBoundRect().left), cdPoint))
		return false;

	if (_isnan(cdPoint->x) || _isnan(cdPoint->y))
		return false;

	*cdPoint = *cdPoint - m_GOrigin;
	*cdPoint = *cdPoint * m_Size;
	return true;
}

bool MapperWrapper::TransformInverse(CDPoint *cdPoint)
{
	if (!m_Net || !m_pInvMatrices)
		return false;

	CPoint coords;
	CDPoint tempPoint = *cdPoint;
	tempPoint = tempPoint / m_Size;
	tempPoint = tempPoint + m_GOrigin;
	coords.x = (int)tempPoint.x;
	coords.y = (int)tempPoint.y;

	if (coords.x<m_Net->GetBoundRect().left)
		coords.x = m_Net->GetBoundRect().left;
	else if (coords.x>m_Net->GetBoundRect().right)
		coords.x = m_Net->GetBoundRect().right;

	if (coords.y<m_Net->GetBoundRect().top)
		coords.y = m_Net->GetBoundRect().top;
	else if (coords.y>m_Net->GetBoundRect().bottom)
		coords.y = m_Net->GetBoundRect().bottom;

	if (!Transform(m_pInvMatrices + 9 * ((coords.y - m_Net->GetBoundRect().top) * m_Net->GetWidth() + coords.x  - m_Net->GetBoundRect().left), &tempPoint))
		return false;

	if (_isnan(tempPoint.x) || _isnan(tempPoint.y))
		return false;

	*cdPoint = tempPoint;
	return true;
}

bool MapperWrapper::SetOrigin(double X, double Y)
{
	m_IOrigin.x = (LONG)X;
	m_IOrigin.y = (LONG)Y;
	if (m_bCalibrated)
	{
		CPoint coords;
		m_GOrigin.x = m_IOrigin.x;
		m_GOrigin.y = m_IOrigin.y;
		m_Net->GetNearestCell(m_GOrigin, &coords);
		Transform(m_pMatrices + 9 * ((coords.y - m_Net->GetBoundRect().top) * m_Net->GetWidth() + coords.x  - m_Net->GetBoundRect().left), &(m_GOrigin));
	}
	return false;
}

//ORIENTATION
BOOL MapperWrapper::EstimateMajorDirections(CDPointArray *pBigSpots)
{
	//TODO: finding truely big spots
	if(pBigSpots->GetCount() < 5)
		return FALSE;
	CDPoint p0 = pBigSpots->GetAt(0);
	pBigSpots->RemoveAt(0);

	CPoint pare = MapperWrapper::GetLine(p0, pBigSpots);
	if(pare.x == 0 && pare.y == 0)
		return FALSE;
	CDPoint p1 = pBigSpots->GetAt(pare.x);
	CDPoint p2 = pBigSpots->GetAt(pare.y);
	pBigSpots->RemoveAt(max(pare.x, pare.y));
	pBigSpots->RemoveAt(min(pare.x, pare.y));
	pare = MapperWrapper::GetLine(p0, pBigSpots);
	if(pare.x == 0 && pare.y == 0)
	{
		pare = MapperWrapper::GetLine(p1, pBigSpots);
		if(pare.x == 0 && pare.y == 0)
		{
			pare = MapperWrapper::GetLine(p2, pBigSpots);
			if(pare.x == 0 && pare.y == 0)
				return FALSE;
			else
				swap(&p0, &p2);
		}
		else
			swap(&p0, &p1);
	}
	CDPoint p3 = pBigSpots->GetAt(pare.x);
	CDPoint p4 = pBigSpots->GetAt(pare.y);
	//now we know, that points p0,p1,p2 are on a line, and p0,p3,p4 are on a line 
	m_Horz.y = p1.y - p0.y;
	m_Horz.x = p1.x - p0.x;

	m_Vert.y = p3.y - p0.y;
	m_Vert.x = p3.x - p0.x;

	//check for clockwise situation
	if(m_Vert.y * m_Horz.x - m_Vert.x * m_Horz.y > 0)
		swap(&m_Horz, & m_Vert);

	//normalize
	double norm = sqrt(m_Vert.x * m_Vert.x + m_Vert.y * m_Vert.y);
	m_Vert.x /= norm;
	m_Vert.y /= norm;
	norm = sqrt(m_Horz.x * m_Horz.x + m_Horz.y * m_Horz.y);
	m_Horz.x /= norm;
	m_Horz.y /= norm;

	//set origin
	m_IOrigin.x = (LONG)p0.x;
	m_IOrigin.y = (LONG)p0.y;

	return TRUE;
}

CPoint MapperWrapper::GetLine(CDPoint p0, CDPointArray *pList)
{
	CDPoint p1 = CDPoint(0,0);
	for(int i = 0; i < pList->GetCount() - 1; i++)
	{
		p1 = pList->GetAt(i);
		for(int j = i + 1; j < pList->GetCount(); j++)
			if(CNodLine::isOnLine(p0, p1, pList->GetAt(j)))
				return CPoint(i, j);
	}
	return CPoint(0,0);
}

void MapperWrapper::swap(CDPoint *p1, CDPoint *p2)
{
	CDPoint temp = *p1;
	*p1 = *p2;
	*p2 = temp;
}

//ORIENTATION
double MapperWrapper::getSqDistance(CDPoint p0, CDPoint p1)
{
	return (p0.x - p1.x) * (p0.x - p1.x) + (p0.y - p1.y) * (p0.y - p1.y);
}

IMPLEMENT_RUNTIME_GADGET_EX(Mapper, CFilterGadget, LINEAGE_FILEX, TVDB400_PLUGIN_NAME);

void CALLBACK onText(CDataFrame* lpData, void* lpParam, CConnector* lpInput)
{
	((Mapper*)lpParam)->onText(lpData);
}

Mapper::Mapper()
{
	m_Mappers.RemoveAll();

	m_pInputs[0] = new CInputConnector(transparent);
	m_pInputs[1] = new CInputConnector(text, ::onText, this);
	//m_pOutput = new COutputConnector(transparent);
	m_pOutputs[0] = new COutputConnector(transparent);
	m_pOutputs[1] = new COutputConnector(transparent);
  m_OutputMode = modeReplace ;
	Resume();
}

bool Mapper::PrintProperties(FXString& text)
{
	FXPropertyKit pc;
	pc.WriteString("NetFile",m_sNetFile);
	pc.WriteString("SmallLabel",m_sSmallLabel);
	pc.WriteString("BigLabel",m_sBigLabel);
	text=pc;
	return true;
}

bool Mapper::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	FXPropertyKit pc(text);
	pc.GetString("NetFile",m_sNetFile);
	pc.GetString("SmallLabel",m_sSmallLabel);
	pc.GetString("BigLabel",m_sBigLabel);
	return true;
}


bool Mapper::ScanSettings(FXString& text)
{
	text.Format("template(EditBox(NetFile),EditBox(SmallLabel),EditBox(BigLabel))",TRUE,FALSE);
	return true;
}


bool Mapper::LoadSettingsFromFile()
{
	m_Lock.Lock();
	while (m_Mappers.GetSize()>0)
	{
		MapperWrapper* mapper = m_Mappers.GetAt(0);
		delete mapper;
		m_Mappers.RemoveAt(0);
	}	

	FILE * fr ;
	FXString str;
	FXString token;
	FXSIZE curPos;
	
	if(m_sNetFile.IsEmpty())
	{
		m_Lock.Unlock();
		return FALSE;
	}
  errno_t Err = _tfopen_s ( &fr , m_sNetFile , "r" ) ; 
  //  if(!file.Open(m_sNetFile, CFile::modeWrite | CFile::modeCreate, NULL))
  if( Err != 0 )
  {
    m_Lock.Unlock();
    SENDERR_2( "Error %s on File %s opening for reading" , _tcserror(Err) , (LPCTSTR)m_sNetFile ) ;
    return FALSE;
  }

	while (ReadStringFromFile( fr , str) && (str.Compare("LABEL:")==0))
	{
		if (!ReadStringFromFile( fr , str))
		{
			m_Lock.Unlock();
			return FALSE;
		}

		MapperWrapper *mapper = new MapperWrapper(str);

		if (!mapper->ReadMeFromFile(fr))
			delete mapper;
		else
			m_Mappers.Add(mapper);
	}

	if(str.Compare("WORLDBOUNDRECT:"))
	{
		m_Lock.Unlock();
		return FALSE;
	}

	//read m_WorldBoundRect
	if(!ReadStringFromFile( fr , str))
	{
		m_Lock.Unlock();
		return FALSE;
	}
	curPos = 0;
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		m_Lock.Unlock();
		return FALSE;
	}
	m_WorldBoundRect.left = atof(token);
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		m_Lock.Unlock();
		return FALSE;
	}
	m_WorldBoundRect.top = atof(token);
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		m_Lock.Unlock();
		return FALSE;
	}
	m_WorldBoundRect.right = atof(token);
	token = str.Tokenize((","),curPos);
	if(token == "")
	{
		m_Lock.Unlock();
		return FALSE;
	}
	m_WorldBoundRect.bottom = atof(token);

	m_Lock.Unlock();
	return TRUE;
}


bool Mapper::SaveSettingsToFile()
{
	//CStdioFile file;
  FILE * fw = NULL ;
	FXString str;
	
	int i = 0, j = 0;
	if(m_sNetFile.IsEmpty())
		return FALSE;
	
	
	m_Lock.Lock();
  errno_t Err = _tfopen_s ( &fw , m_sNetFile , "w" ) ; 
//  if(!file.Open(m_sNetFile, CFile::modeWrite | CFile::modeCreate, NULL))
  if( Err != 0 )
	{
		m_Lock.Unlock();
    SENDERR_2( "%s on File %s opening for writing" , _tcserror(Err) , (LPCTSTR)m_sNetFile ) ;
		return FALSE;
	}

	for (int counter=0; counter<m_Mappers.GetSize(); counter++)
	{
		MapperWrapper *mapper = m_Mappers.GetAt(counter);

		str.Format("LABEL:\n%s\n", (LPCTSTR)mapper->m_Label);
    _fputts( (LPCTSTR)str , fw ) ;

		mapper->WriteMeToFile( fw );
	}
	str.Format("WORLDBOUNDRECT:\n%lf,%lf,%lf,%lf\n", m_WorldBoundRect.left, 
    m_WorldBoundRect.top, m_WorldBoundRect.right, m_WorldBoundRect.bottom);
  _fputts( (LPCTSTR)str , fw ) ;
	fclose( fw ) ;
	m_Lock.Unlock();
	return TRUE;
}

void Mapper::ShutDown()
{
	CGadget::ShutDown();

	CDataFrame * pFr ;
	for (int i=0; i<2; i++)
	{
		while ( m_pInputs[i]->Get( pFr ) )
			pFr->Release( pFr )  ;
	}
	delete m_pInputs[0];
	m_pInputs[0] = NULL;
	delete m_pInputs[1];
	m_pInputs[1] = NULL;
	delete m_pOutputs[0];
	m_pOutputs[0] = NULL;
	delete m_pOutputs[1];
	m_pOutputs[1] = NULL;


	m_Lock.Lock();
	while (m_Mappers.GetSize()>0)
	{
		MapperWrapper* mapper = m_Mappers.GetAt(0);
		delete mapper;
		m_Mappers.RemoveAt(0);
	}	
	m_Lock.Unlock();
}

void Mapper::ParseCommand(MapperWrapper* mapper, FXString command)
{
	double tempX, tempY;
	FXString templ;

	if(!command.Compare(COMMAND_CALIBRATE))
		mapper->m_iRegime = REGIME_CALIBRATE;
	if(!command.Compare(COMMAND_WORK))
		mapper->m_iRegime = REGIME_WORK;
	if(!command.Left(FXString(COMMAND_ORIGIN).GetLength()).Compare(COMMAND_ORIGIN))
	{
		templ = COMMAND_ORIGIN + FXString("(%lf,%lf)");
		if ( sscanf( (LPCTSTR)command , templ , &tempX, &tempY) > 0)
		{
			//ORIGIN MUST BE ON THE IMAGE!!!
			if (tempX<0 || tempY<0 || tempX>mapper->m_ImageSize.x || tempY>mapper->m_ImageSize.y)
				return;
			mapper->SetOrigin(tempX, tempY);
			CalculateCommonNet();
		}
	}
	if(!command.Left(FXString(COMMAND_SIZE).GetLength()).Compare(COMMAND_SIZE))
	{
		templ = COMMAND_SIZE + FXString("(%lf,%lf)");
		if ( sscanf( (LPCTSTR)command , templ , &tempX, &tempY) > 0)
		{
			mapper->m_Size.x = tempX;
			mapper->m_Size.y = tempY;
			CalculateCommonNet();
		}
	}
	if(!command.Compare(COMMAND_WORLD))
		mapper->m_bImage = FALSE;
	if(!command.Compare(COMMAND_IMAGE))
		mapper->m_bImage = TRUE;
	return;
}

void Mapper::onText(CDataFrame* lpData)
{
	CTextFrame *pCf = NULL;
	if (!lpData)
		return;

	pCf = lpData->GetTextFrame();
	if(!pCf)
	{
		lpData->RELEASE(lpData);
		return;
	}
	FXString str = pCf->GetString();

	FXSIZE currPos = 0;
	FXString command = str.Tokenize(" ", currPos);
	FXString mapper_name = str.Tokenize(" ", currPos);

	//Global commands
	if(!command.Compare(COMMAND_WRITE))
	{
		SaveSettingsToFile();
		lpData->RELEASE(lpData);
		return;
	}
	if(!command.Compare(COMMAND_READ))
	{
		LoadSettingsFromFile();
		lpData->RELEASE(lpData);
		return;
	}
	if (!command.Compare(COMMAND_MAKENET))
	{
		//We have X and Y parameters in mapper_name parameter =)
		int dX=0, dY=0;
		if (sscanf(mapper_name, "X%dY%d", &dX, &dY)!=2)
		{
			lpData->RELEASE(lpData);
			return;
		}
		MakeCommonNetForImages(dX, dY);
	}

	//Local mapper command
	if (mapper_name.Compare("All")==0)
	{
		for (int i=0; i<m_Mappers.GetSize(); i++)
		{
			MapperWrapper *mapper = m_Mappers.GetAt(i);
			ParseCommand(mapper, command);
		}
	}
	else
	{
		int index = GetMapperNumberByString(mapper_name);
		if (index<0)
		{
			lpData->RELEASE(lpData);
			return;
		}
		MapperWrapper *mapper = m_Mappers.GetAt(index);
		ParseCommand(mapper, command);
	}

	lpData->RELEASE(lpData);
}

int Mapper::DoJob()
{
	CDataFrame* pDataFrame = NULL;
	while (m_pInputs[0]->Get(pDataFrame))
	{
		ASSERT(pDataFrame);
		if ( Tvdb400_IsEOS (pDataFrame ) )
		{
			if (!m_pOutputs[0]->Put(pDataFrame))
				pDataFrame->RELEASE(pDataFrame);
			return WR_CONTINUE;
		}
        switch (m_Mode)
		{
			case mode_reject:
				pDataFrame->RELEASE(pDataFrame);
				break;
			case mode_transmit:
				if (!m_pOutput->Put(pDataFrame))
					pDataFrame->RELEASE(pDataFrame);
				break;
			case mode_process:
                {
		            CDataFrame* Container = pDataFrame->CopyContainer();
		            if (Container)
		            {
			            pDataFrame->RELEASE(pDataFrame);
			            pDataFrame = Container;
		            }
		            CDataFrame* pResultFrame = DoProcessing(pDataFrame);

		            if ((pResultFrame) && (!m_pOutputs[0]->Put(pResultFrame)))
			            pResultFrame->RELEASE(pResultFrame);
                }
        }
	}
	return WR_CONTINUE;
}

CDataFrame* Mapper::DoProcessing(const CDataFrame* pDataFrame) 
{ 
	int test = 0;
	int counter = 0;
	int count = 0;
	CPoint coords;
	CFigureFrame *pFf = NULL;
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);

	FXString VideoLabel;
	if (VideoFrame)
		VideoLabel = VideoFrame->GetLabel();
	else
		VideoLabel = "";

	int index = GetMapperNumberByString(VideoLabel);

	m_Lock.Lock();
	MapperWrapper *mapper;
	if (index<0)
	{
		mapper = new MapperWrapper(VideoLabel);
		mapper->m_ImageSize = CPoint(VideoFrame->lpBMIH->biWidth, VideoFrame->lpBMIH->biHeight);
		m_Mappers.Add(mapper);
	}
	else
	{
		mapper = m_Mappers.GetAt(index);
	}

	CContainerFrame* allVal = CContainerFrame::Create();
	allVal->ChangeId(pDataFrame->GetId());
	allVal->SetTime(pDataFrame->GetTime());
	allVal->SetLabel("Mapper_Results");

	CContainerFrame* resVal = CContainerFrame::Create();
	resVal->ChangeId(pDataFrame->GetId());
	resVal->SetTime(pDataFrame->GetTime());
	resVal->SetLabel("Mapper_FiguresOutput");
	if(pDataFrame)
	{
		if(mapper->m_iRegime==REGIME_WORK && mapper->m_bCalibrated)
		{
			if(!mapper->m_Net)
				return (CDataFrame*)pDataFrame;
			mapper->m_Lock.Lock();
			CRect bRect = mapper->m_Net->GetBoundRect();
			//let's transform
			CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(figure);
			if (Iterator!=NULL)
			{
				pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
				while(pFf)
				{
					count = (int) pFf->GetCount();
					CFigureFrame *pNewFf = CFigureFrame::Create(pFf);
					pNewFf->ChangeId(pFf->GetId());
					pNewFf->SetTime(pFf->GetTime());
					pNewFf->SetLabel(pFf->GetLabel()); 
					pNewFf->Attributes()->SetString(pFf->Attributes()->GetBuffer());
					for(counter = 0; counter < count; counter++)
					{
						CDPoint cdPoint = pFf->GetAt(counter);
						if(mapper->m_bImage)
						{
							mapper->TransformForward(&cdPoint);
						}
						else
						{
							mapper->TransformInverse(&cdPoint);
						}

						//in-place
						pNewFf->SetAt(counter, cdPoint);
					}
					pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
					resVal->AddFrame(pNewFf);
				}
				delete Iterator; Iterator = NULL;
			}
			mapper->m_Lock.Unlock();
		}

		if(mapper->m_iRegime==REGIME_CALIBRATE)
		{
			//filling calibration "net"
			mapper->m_Lock.Lock();
			CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(figure);
			CDPointArray aBigSpots;
			if (Iterator!=NULL)
			{
				pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
				mapper->m_aNods.RemoveAll();
				while(pFf)
				{
					CFigureFrame *pNewFf = CFigureFrame::Create(pFf);
					pNewFf->ChangeId(pFf->GetId());
					pNewFf->SetTime(pFf->GetTime());
					pNewFf->SetLabel(pFf->GetLabel());
					pNewFf->Attributes()->SetString(pFf->Attributes()->GetBuffer());
					//only points are included into the array
					if(pFf->GetNumberVertex() == 1)
					{
						for(counter = 0; counter < pFf->GetCount(); counter++)
						{
							CDPoint cdPoint = pFf->GetAt(counter);
							//ORIENTATION
							//check for big spot label
							if(m_sBigLabel.Compare(pFf->GetLabel()) == 0)
							{
								aBigSpots.Add(cdPoint);
								mapper->m_aNods.Add(cdPoint);
							}
							else
							{
								//if not - check if such big spot has been already added
								for(test = 0; test < aBigSpots.GetCount(); test++)
									if(fabs(aBigSpots.GetAt(test).x - cdPoint.x) + fabs(aBigSpots.GetAt(test).y - cdPoint.y) < 8)
										break;
								if(test == aBigSpots.GetCount())
									mapper->m_aNods.Add(cdPoint);
							}
						}
					}
					resVal->AddFrame(pNewFf);
					pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
				}
				delete Iterator; Iterator=NULL;

				//ORIENTATION
				if(!mapper->EstimateMajorDirections(&aBigSpots))
				{
					mapper->m_Horz = CDPoint(1,0);
					mapper->m_Vert = CDPoint(0,1);
				}
				//calculate CalibrNet and matrices 
				if(mapper->CreateCalibrNet() && mapper->PrepareMatrices())
				{
					//calculate geocoords of (0,0) nod
					mapper->m_bCalibrated = TRUE;
					mapper->SetOrigin(mapper->m_IOrigin.x, mapper->m_IOrigin.y);
					CalculateCommonNet();
				}
			}
			mapper->m_Lock.Unlock();
		}
	}

	mapper->m_Lock.Lock();
	if (mapper->m_pLocalImageNetFrame)
	{
		CContainerFrame *NetFrame = CContainerFrame::Create();
		CFramesIterator* Iterator = mapper->m_pLocalImageNetFrame->CreateFramesIterator(figure);
		if (Iterator!=NULL)
		{
			pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
			while(pFf)
			{	
				CFigureFrame *pNewFf = CFigureFrame::Create(pFf);
				pNewFf->ChangeId(pDataFrame->GetId());
				pNewFf->SetTime(pDataFrame->GetTime());
				pNewFf->SetLabel(pFf->GetLabel());
				pNewFf->Attributes()->SetString(pFf->Attributes()->GetBuffer());
				NetFrame->AddFrame(pNewFf);
				pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
			}
			delete Iterator; Iterator=NULL;
		}
		NetFrame->SetTime(allVal->GetTime());
		NetFrame->ChangeId(allVal->GetId());
		NetFrame->SetLabel(mapper->m_pLocalImageNetFrame->GetLabel());
		resVal->AddFrame(NetFrame);
	}
	mapper->m_Lock.Unlock();
	m_Lock.Unlock();

	allVal->AddFrame(resVal);
	allVal->AddFrame(VideoFrame);

  CDataFrame *pdf = (CDataFrame*)VideoFrame ;
  pdf->AddRef();

	return allVal;
}

int Mapper::GetMapperNumberByString(FXString name)
{
	for (int i=0; i<m_Mappers.GetCount(); i++)
	{
		if ((m_Mappers.GetAt(i))->m_Label.Compare(name)==0)
			return i;
	}
	return -1;
}

void Mapper::CalculateCommonNet()
{
	//On the assumption that we have Origin (0,0) point in the view...
	//First, we go left to find the first image, when there is no points
	//The same for the right
	//Up, left
	// As a result, we have a rectangle, which can get off some part of images, but main part
	// would be inside.
	// 
	// Another assumption - they have common m_Size
	
	CDPoint CurrPos(0,0);
	CDPoint size = m_Mappers.GetAt(0)->m_Size;
	while (PointIsInAllImages(CurrPos))
	{
		CurrPos.x-=size.x;
	}
	m_WorldBoundRect.left = CurrPos.x+size.x;
	CurrPos = CDPoint(0,0);
	//Using assumption that 0,0 is in image, so the cycle will be done at least one time
	while (PointIsInAllImages(CurrPos))
	{
		CurrPos.x+=size.x;
	}
	m_WorldBoundRect.right = CurrPos.x-size.x;
	CurrPos = CDPoint(0,0);
	while (PointIsInAllImages(CurrPos))
	{
		CurrPos.y-=size.y;
	}
	m_WorldBoundRect.top = CurrPos.y+size.y;
	CurrPos = CDPoint(0,0);
	while (PointIsInAllImages(CurrPos))
	{
		CurrPos.y+=size.y;
	}
	m_WorldBoundRect.bottom = CurrPos.y-size.y;

	if (m_WorldBoundRect.left>m_WorldBoundRect.right || m_WorldBoundRect.top>m_WorldBoundRect.bottom)
		return;

	//Now it's time to detect, whether corners of the rectangle are fine to calculate
	CurrPos = CDPoint(m_WorldBoundRect.left, m_WorldBoundRect.top);
	while (!PointIsInAllImages(CurrPos))
	{
		CurrPos.x+=size.x;
		CurrPos.y+=size.y;
	}
	m_WorldBoundRect.left = CurrPos.x;
	m_WorldBoundRect.top = CurrPos.y;

	CurrPos = CDPoint(m_WorldBoundRect.right, m_WorldBoundRect.top);
	while (!PointIsInAllImages(CurrPos))
	{
		CurrPos.x-=size.x;
		CurrPos.y+=size.y;
	}
	m_WorldBoundRect.right = CurrPos.x;
	m_WorldBoundRect.top = CurrPos.y;

	CurrPos = CDPoint(m_WorldBoundRect.left, m_WorldBoundRect.bottom);
	while (!PointIsInAllImages(CurrPos))
	{
		CurrPos.x+=size.x;
		CurrPos.y-=size.y;
	}
	m_WorldBoundRect.left = CurrPos.x;
	m_WorldBoundRect.bottom = CurrPos.y;

	CurrPos = CDPoint(m_WorldBoundRect.right, m_WorldBoundRect.bottom);
	while (!PointIsInAllImages(CurrPos))
	{
		CurrPos.x-=size.x;
		CurrPos.y-=size.y;
	}
	m_WorldBoundRect.right = CurrPos.x;
	m_WorldBoundRect.bottom = CurrPos.y;

	return;
}

bool Mapper::PointIsInAllImages(CDPoint point)
{
	for (int i=0; i<m_Mappers.GetSize(); i++)
	{
		CPoint coords;
		CDPoint tempPoint = point;
		MapperWrapper *mapper = m_Mappers.GetAt(i);

		CRect bRect = mapper->m_Net->GetBoundRect();

		if (!mapper->TransformInverse(&tempPoint))
			return false;
		
		if (tempPoint.x<0 || tempPoint.x>mapper->m_ImageSize.x || tempPoint.y<0 || tempPoint.y>mapper->m_ImageSize.y)
			return false;
	}
	return true;
}

bool Mapper::MakeCommonNetForImages(int dX, int dY)
{
	//dX, dY - number of cells (there will be dX+1, dY+1 points)
	if (dX<=0 || dY<=0)
		return FALSE;

	m_Lock.Lock();
	CContainerFrame *pNetFrame = CContainerFrame::Create();
	for (int k=0; k<m_Mappers.GetSize(); k++)
	{
		MapperWrapper *mapper = m_Mappers.GetAt(k);
		mapper->m_Lock.Lock();
		CFigureFrame *forNetFrame = CFigureFrame::Create();
		forNetFrame->SetLabel(mapper->m_Label);
		forNetFrame->Attributes()->WriteInt("totalCol", dX);
		forNetFrame->Attributes()->WriteInt("totalRow", dY);

		CDPoint m_StepSize = CDPoint((m_WorldBoundRect.right-m_WorldBoundRect.left)/dX, (m_WorldBoundRect.top-m_WorldBoundRect.bottom)/dY);
		CDPoint currPoint = CDPoint(m_WorldBoundRect.left, m_WorldBoundRect.bottom);

		int ntime=0;
		LARGE_INTEGER ntime1,ntime2;
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&ntime1);
		for (int i=0; i<dX+1; i++)
		{
			for (int j=0; j<dY+1; j++)
			{
				CDPoint cdPointTL = currPoint; //top-left
				mapper->TransformInverse(&cdPointTL);
				forNetFrame->AddPoint(cdPointTL);
				currPoint.y += m_StepSize.y;
			}
			currPoint.y = m_WorldBoundRect.bottom;
			currPoint.x += m_StepSize.x;
		}
		QueryPerformanceCounter(&ntime2);
		ntime = (int)((ntime2.QuadPart-ntime1.QuadPart)/(freq.QuadPart/1000));
		TRACE("MAKING NET: %d %d: %ld milliseconds\n", dX, dY, ntime);
		pNetFrame->AddFrame(forNetFrame);
		mapper->m_Lock.Unlock();
	}

	if ((pNetFrame) && (!m_pOutputs[1]->Put(pNetFrame)))
		pNetFrame->RELEASE(pNetFrame);
	m_Lock.Unlock();
	return TRUE;
}