// Circle processing routines
// 

#include "stdafx.h"

#include <math/Intf_sup.h>

// Explanation in https://russianblogs.com/article/21941238926/
// And in https://blog.csdn.net/Jacky_Ponder/article/details/70314919 (Chinese)

bool CircleFitting(
  const cmplx * pts,	// In Coordinates for points on circle
  int     iNPts,
  cmplx& center,			// Out Center coordinates
  double& diameter ,  		// Out diameter
  double dFilterValue = 0.); // if zero - no filtering by S/R
                             // ideal circle this ratio is 0.5
                             // All others are lower


bool CircleFitting(
  CmplxVector &pts,		// In Coordinates for points on circle
  cmplx& center,				// Out Center coordinates
  double& diameter ,  		// Out diameter
  double dFilterValue = 0.); // if zero - no filtering by S/R
                             // ideal circle this ratio is 0.5
                             // All others are lower

