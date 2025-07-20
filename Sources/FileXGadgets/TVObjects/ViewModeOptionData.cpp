#include "stdafx.h"
#include "ViewModeOptionData.h"


void ViewModeOptionData::setbFindObject(BOOL value)
{
  m_bFindObject = (value != 0) ;
}

BOOL ViewModeOptionData::getbFindObject()
{
  return m_bFindObject;
}

void ViewModeOptionData::setbCatchObject(BOOL value)
{
  m_bCatchObject = (value != 0);
}

BOOL ViewModeOptionData::getbCatchObject()
{
  return m_bCatchObject;
}

void ViewModeOptionData::setbFindObjectInArea(BOOL value)
{
  m_bFindObjectInArea = (value != 0) ;
}

BOOL ViewModeOptionData::getbFindObjectInArea()
{
  return m_bFindObjectInArea;
}


void ViewModeOptionData::setbDispROI(BOOL value)
{
  m_bDispROI = (value != 0);
}

BOOL ViewModeOptionData::getbDispROI()
{
  return m_bDispROI;
}

void ViewModeOptionData::setbDispPos(BOOL value)
{
  m_bDispPos = (value != 0);
}

BOOL ViewModeOptionData::getbDispPos()
{
  return m_bDispPos;
}

void ViewModeOptionData::setbDispCoor(BOOL value)
{
  m_bDispCoor = (value != 0);
}

BOOL ViewModeOptionData::getbDispCoor()
{
	return m_bDispCoor;
}

void ViewModeOptionData::setbDispDetails(BOOL value)
{
  m_bDispDetails = (value != 0);
}

BOOL ViewModeOptionData::getbDispDetails()
{
	return m_bDispDetails;
}

void ViewModeOptionData::setbDispProfX(BOOL value)
{
  m_bDispProfX = (value != 0);
}

BOOL ViewModeOptionData::getbDispProfX()
{
	return m_bDispProfX;
}

void ViewModeOptionData::setbDispProfY(BOOL value)
{
  m_bDispProfY = (value != 0);
}

BOOL ViewModeOptionData::getbDispProfY()
{
	return m_bDispProfY;
}

void ViewModeOptionData::setbDispMGraphics(BOOL value)
{
  m_bDispMGraphics = (value != 0);
}

BOOL ViewModeOptionData::getbDispMGraphics()
{
  return m_bDispMGraphics;
}

void ViewModeOptionData::setbViewObjectContur(BOOL value)
{
  m_bViewObjectContur = (value != 0) ;
}

BOOL ViewModeOptionData::getbViewObjectContur()
{
  return m_bViewObjectContur;
}

void ViewModeOptionData::setbViewCoordScaled(BOOL value)
{
  m_bViewCoordsScaled = (value != 0) ;
}

BOOL ViewModeOptionData::getbViewCoordScaled()
{
  return m_bViewCoordsScaled ;
}

int ViewModeOptionData::GetViewMode()
{
  DWORD dwViewMode = (((DWORD)m_bDispROI) * OBJ_VIEW_ROI)
    | ( ((DWORD)m_bDispPos) * OBJ_VIEW_POS ) 
    | ( ((DWORD)m_bDispCoor) * OBJ_VIEW_COORD )
    | ( ((DWORD)m_bDispDetails) * OBJ_VIEW_DIFFR ) 
    | ( ((DWORD)m_bDispProfX) * OBJ_VIEW_PROFX ) 
    | ( ((DWORD)m_bDispProfY) * OBJ_VIEW_PROFY )  
    | ( ((DWORD)m_bDispMGraphics) * OBJ_VIEW_MRECTS ) 
    | ( ((DWORD)m_bFindObjectInArea) * OBJ_FIND_OBJ ) 
    | ( ((DWORD)m_bViewCoordsScaled)  * OBJ_VIEW_SCALED )
    | ( ((DWORD)m_bViewObjectContur) * OBJ_VIEW_CONT ) 
    | ( ((DWORD)m_bViewAngle) * OBJ_VIEW_ANGLE) 
    | ( ((DWORD)m_bViewDia) * OBJ_VIEW_DIA) 
    | ( ((DWORD)m_bWeighted) * OBJ_WEIGHTED) 
    | ( ((DWORD)m_bCSV) * OBJ_VIEW_CSV );
  return (int)dwViewMode;
}

void ViewModeOptionData::SetViewMode( int iViewMode )
{
  setbDispROI( iViewMode & OBJ_VIEW_ROI ) ;
  setbDispPos( iViewMode & OBJ_VIEW_POS ) ;
  setbDispCoor( iViewMode & OBJ_VIEW_COORD ) ;
  setbDispDetails( iViewMode & OBJ_VIEW_DIFFR ) ;
  setbDispProfX( iViewMode & OBJ_VIEW_PROFX ) ;
  setbDispProfY( iViewMode & OBJ_VIEW_PROFY ) ;
  setbDispMGraphics( iViewMode & OBJ_VIEW_MRECTS ) ; // display 3x3 network
  setbViewCoordScaled( iViewMode & OBJ_VIEW_SCALED ) ; 
  setbViewObjectContur( iViewMode & OBJ_VIEW_CONT ) ;
  setbViewAngle( iViewMode & OBJ_VIEW_ANGLE ) ;
  setbViewDia( iViewMode & OBJ_VIEW_DIA ) ;
  setbWeighted( iViewMode & OBJ_WEIGHTED ) ;
  setbCSV( iViewMode & OBJ_VIEW_CSV ) ;
}

