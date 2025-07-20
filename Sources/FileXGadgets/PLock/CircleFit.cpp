/* Copyright (c) Mark J. Kilgard, 1997. */

/* This program is freely distributable without licensing fees
and is provided without guarantee or warrantee expressed or
implied. This program is -not- in the public domain. */

/* This is a small interactive demo of Dave Eberly's algorithm
that fits a circle boundary to a set of 2D points. */
#include "stdafx.h"
#include <math\Intf_sup.h>

/* Some <math.h> files do not define M_PI... */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/****************************************************************************
Least squares fit of circle to set of points.
by Dave Eberly (eberly@cs.unc.edu or eberly@ndl.com)
ftp://ftp.cs.unc.edu/pub/users/eberly/magic/circfit.c
---------------------------------------------------------------------------
Input:  (x_i,y_i), 1 <= i <= N, where N >= 3 and not all points
are collinear
Output:  circle center (a,b) and radius r

Energy function to be minimized is

E(a,b,r) = sum_{i=1}^N (L_i-r)^2

where L_i = |(x_i-a,y_i-b)|, the length of the specified vector.
Taking partial derivatives and setting equal to zero yield the
three nonlinear equations

E_r = 0:  r = Average(L_i)
E_a = 0:  a = Average(x_i) + r * Average(dL_i/da)
E_b = 0:  b = Average(y_i) + r * Average(dL_i/db)

Replacing r in the last two equations yields

a = Average(x_i) + Average(L_i) * Average(dL_i/da) = F(a,b)
b = Average(y_i) + Average(L_i) * Average(dL_i/db) = G(a,b)

which can possibly be solved by fixed point iteration as

a_{n+1} = F(a_n,b_n),  b_{n+a} = G(a_n,b_n)

with initial guess a_0 = Average(x_i) and b_0 = Average(y_i).
Derivative calculations show that

dL_i/da = (a-x_i)/L_i,  dL_i/db = (b-y_i)/L_i.

---------------------------------------------------------------------------
WARNING.  I have not analyzed the convergence properties of the fixed
point iteration scheme.  In a few experiments it seems to converge
just fine, but I do not guarantee convergence in all cases.
****************************************************************************/

int
CircleFit( CmplxArray& Points , cmplx& Center , double& dRadius )
{
  /* user-selected parameters */
  const int maxIterations = 256;
  const double tolerance = 0.01;

  double r;

  /* compute the average of the data points */
  int i , j;
  cmplx Aver ;

  for ( i = 0; i < Points.GetSize(); i++ )
    Aver += Points[ i ] ;


  /* initial guess */
  cmplx Cent0 = Aver / (double)Points.GetSize() ;
  cmplx Cent = Cent0 ; // temporary values

  for ( j = 0; j < maxIterations; j++ ) 
  {
    /* update the iterates */
    cmplx Cent_tmp = Cent ;
    
    /* compute average L, dL/da, dL/db */
    double dLAvr = 0.0;
    cmplx LAvr ;

    for ( i = 0; i < Points.GetSize() ; i++ ) 
    {
      cmplx diff = Points[ i ] - Cent_tmp ;
      double dL = abs(diff) ;
      if ( dL > tolerance ) 
      {
        dLAvr += dL;
        LAvr -= diff / dL;
      }
    }
    dLAvr /= Points.GetSize() ;
    LAvr /= (double)Points.GetSize() ;

    Cent = Cent0 + dLAvr * LAvr ;
    r = dLAvr;
    cmplx CentShift = Cent_tmp - Cent ;
    if ( abs( CentShift ) <= tolerance )
      break;
  }

  Center = Cent ;
  dRadius = r ;

  return ( j < maxIterations ? j : -1 );
}
