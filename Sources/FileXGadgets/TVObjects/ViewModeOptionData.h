#pragma once

//  Defines for graphics viewing      //  Bit Mask          View
#define OBJ_VIEW_POS    0x00000001     //   0  0x0001    Cross on measured position
#define OBJ_VIEW_DET    0x00000002     //   1  0x02      Measurement details
#define OBJ_VIEW_ANGLE  0x00000010     //   4  0x0010    Angle show
#define OBJ_VIEW_SCALED 0x00000100     //   8  0x0100    SHow coordinates as scaled
#define OBJ_VIEW_DIFFR  0x00000200     //   9  0x0200    Diffraction areas
#define OBJ_VIEW_CONT   0x00000400     //  10  0x0400    Contour show
#define OBJ_VIEW_PROFX  0x00001000     //  12  0x1000      Profile X
#define OBJ_VIEW_PROFY  0x00002000     //  13  0x2000      Profile Y
#define OBJ_VIEW_DIA    0x00004000     //  14  0x4000     dia show
#define OBJ_WEIGHTED    0x00020000
#define OBJ_VIEW_TEXT   0x00040000
#define OBJ_VIEW_CSV    0x00080000     // form data text with comma
#define OBJ_OUT_MOMENTS 0x00100000     // Do output of image moments in separate text frame
#define OBJ_VIEW_MRECTS 0x20000000
#define OBJ_FIND_OBJ    0x10000000
#define OBJ_VIEW_COORD  0x40000000 //  30  0x40000000    Measured coordinates
#define OBJ_VIEW_ROI    0x80000000 //  31  0x80000000    ROI (search area)
                        
#define OBJ_VIEW_ALL_CHECKBOXES 0x077F  // ALL CONTROLLABLE BY CHECKBOXES

class ViewModeOptionData
{
public: 
  ViewModeOptionData()
  {
    m_bFindObject = m_bFindObjectInArea = m_bCatchObject = FALSE ;
    SetViewMode( 0x80000003 ) ;
  }
  void setbDispROI(BOOL value);
  BOOL getbDispROI();
  void setbDispPos(BOOL value);
  BOOL getbDispPos();
  void setbDispCoor(BOOL value);
  BOOL getbDispCoor();
  void setbDispDetails(BOOL value);
  BOOL getbDispDetails();
  void setbDispProfX(BOOL value);
  BOOL getbDispProfX();
  void setbDispProfY(BOOL value);
  BOOL getbDispProfY();
  void setbDispMGraphics(BOOL value);
  BOOL getbDispMGraphics();
  void setbFindObject(BOOL value);
  BOOL getbFindObject();
  void setbCatchObject(BOOL value);
  BOOL getbCatchObject();
  void setbFindObjectInArea(BOOL value);
  BOOL getbFindObjectInArea() ; 
  int  GetViewMode() ;
  void SetViewMode( int iViewMode ) ;
  void setbViewObjectContur(BOOL value);
  BOOL getbViewObjectContur() ; 
  void setbViewCoordScaled(BOOL value);
  BOOL getbViewCoordScaled() ; 
  void setbViewAngle(BOOL value) { m_bViewAngle = (value != 0) ;};
  BOOL getbViewAngle() { return m_bViewAngle ; } ; 
  void setbViewDia(BOOL value) { m_bViewDia = (value != 0) ;};
  BOOL getbViewDia() { return m_bViewDia ; }; 
  void setbWeighted( BOOL value ) { m_bWeighted = ( value != 0 ) ; };
  BOOL getbWeighted() { return m_bWeighted ; };
  void setbCSV( BOOL value ) { m_bCSV = ( value != 0 ) ; };
  BOOL getbCSV() { return m_bCSV ; };
  void setbMomentsOut( BOOL value ) { m_bImageMomentsOut = ( value != 0 ) ; };
  BOOL getbImageMomentsOut() { return m_bImageMomentsOut ; };

    //Default values:
    BOOL m_bDispROI ;
    BOOL m_bDispPos ;
    BOOL m_bDispCoor ;
    BOOL m_bDispDetails ;
    BOOL m_bDispProfX ;
    BOOL m_bDispProfY ;
    BOOL m_bDispMGraphics ;
    BOOL m_bFindObjectInArea ;
    BOOL m_bViewObjectContur ;
    BOOL m_bViewCoordsScaled ;
    BOOL m_bFindObject ;
    BOOL m_bCatchObject ;
    BOOL m_bViewAngle ;
    BOOL m_bViewDia ;
    BOOL m_bWeighted ;
    BOOL m_bCSV ; // for text form with comma
    BOOL m_bImageMomentsOut ; //Do text frame with image moments
};

