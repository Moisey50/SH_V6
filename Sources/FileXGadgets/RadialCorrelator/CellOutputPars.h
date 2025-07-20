#ifndef CONTUR_DATA_H__
#define CONTUR_DATA_H__

#define MAX_N_CONTUR_SLICES 50

typedef struct  
{
  double dLeftSide_um ;
  double dRightSide_um ;
  cmplx  LeftPoint ;
  cmplx  RightPoint ;
} CONTUR_SLICE;

// dLength is distance  between two contour points on main axis
// dWidth is sum of distances of outermost points 
// to main axis
// distance between slices is dLength/(N_CONUR_CLICEs + 1)

typedef struct  
{
  int    iCellNumber ;
  int    iMeasurementNumber ;
  double dTime ;  // it could be relative to program start or absolute by computer timer
  double dLength_um ;
  double dWidth_um  ;
  double dAngle_deg ;
  double dSquare_um2 ;
  cmplx  Center ;
  int    iNConturSlices ; // N_CONTUR_SLICES
  CONTUR_SLICE Slices[MAX_N_CONTUR_SLICES] ;
  void  * Presentation[MAX_N_CONTUR_SLICES] ;
} CONTUR_DATA;

// In Excel file data for one cell measurement will be placed in one row as 
// comma separated in next order:
// Cell Number, Measurement Number , Time Stamp , Length , Width, Angle , Square, N Slices, 
//      Left1, Right1, Left2, RIght2,... Left10, Right10
//
// Numbers will be provided with 2 decimal digits after point


#endif //CONTUR_DATA_H__
