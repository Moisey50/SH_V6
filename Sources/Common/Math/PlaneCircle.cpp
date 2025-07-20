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
  double& diameter ,			// Out radius
  double dMeasuredArea ) // if zero - no filtering by Area/Perimeter
                 // ideal circle this ratio is R/2.
                 // All others are lower
{
  center = cmplx();
  diameter = 0.0;
  if (iNPts < 3)
    return false;

  // Intermediate variables
  double sumX1 = 0.0; // Sums
  double sumY1 = 0.0;
  double sumX2 = 0.0;	// Sums of squares
  double sumY2 = 0.0;
  double sumX3 = 0.0; // Sums third degrees
  double sumY3 = 0.0;
  double sumX1Y1 = 0.0;
  double sumX1Y2 = 0.0;
  double sumX2Y1 = 0.0;
  const double dN = (double)iNPts;
  double dPerimeter = 0.;
  cmplx cPrev = pts[iNPts-1];
  double dXMin = DBL_MAX, dXMax = -DBL_MAX;
  for (int i = 0; i < iNPts; ++i)
  {
    double x = pts[i].real();
    double y = pts[i].imag();
    double x2 = x * x;
    double y2 = y * y;
    double x3 = x2 * x;
    double y3 = y2 * y;
    double xy = x * y;
    double x1y2 = x * y2;
    double x2y1 = x2 * y;

    sumX1 += x;
    sumY1 += y;
    sumX2 += x2;
    sumY2 += y2;
    sumX3 += x3;
    sumY3 += y3;
    sumX1Y1 += xy;
    sumX1Y2 += x1y2;
    sumX2Y1 += x2y1;
    if ( dMeasuredArea > 0.)
    {
      dPerimeter += abs( pts[ i ] - cPrev );
      cPrev = pts[ i ] ;
    }
  }
  double C = dN * sumX2 - sumX1 * sumX1;
  double D = dN * sumX1Y1 - sumX1 * sumY1;
  double E = dN * (sumX3 + sumX1Y2) - (sumX2 + sumY2) * sumX1;
  double G = dN * sumY2 - sumY1 * sumY1;
  double H = dN * (sumX2Y1 + sumY3) - (sumX2 + sumY2) * sumY1;

  double denominator = C * G - D * D;
  if (std::abs(denominator) < DBL_EPSILON)
    return false;
  double a = (H * D - E * G) / (denominator);
  //denominator = D * D - G * C;
  //if (std::abs(denominator) < DBL_EPSILON) return false;
  double b = (H * C - E * D) / (-denominator);
  double c = -(a * sumX1 + b * sumY1 + sumX2 + sumY2) / dN;

  center = cmplx(a / (-2.), b / (-2.));
  diameter = std::sqrt(a * a + b * b - 4. * c) ;
  if (dMeasuredArea > 0.)
    return   ( dMeasuredArea/dPerimeter ) > sqrt( dMeasuredArea/M_PI )/4. ;
  return true;
}


bool CircleFitting(
  CmplxVector &pts ,		// In Coordinates for points on circle
  cmplx& center ,				// Out Center coordinates
  double& diameter ,  		// Out diameter
  double dFilterValue = 0.) // if zero - no filtering by S/R
                             // ideal circle this ratio is 0.5
                             // All others are lower
{
  return CircleFitting( pts.data() , 
    (int)pts.size() , center , diameter , dFilterValue ) ;
}

