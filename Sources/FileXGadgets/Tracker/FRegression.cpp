// FRegression.cpp: implementation of the CFRegression class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FRegression.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFRegression::CFRegression()
{
  Reset() ;
}

CFRegression::~CFRegression()
{

}

void 
CFRegression::Reset()
{
  m_dSumX = m_dSumY = m_dSumXX = m_dSumYY = m_dSumXY = 0. ;
  m_a = m_b = 0. ;
  m_iNSamples = 0 ;
}

int 
CFRegression::Add(double dX, double dY)
{
  m_dSumX += dX ;
  m_dSumY += dY ;
  m_dSumXX += dX * dX ;
  m_dSumXY += dX * dY ;
  m_dSumYY += dY * dY ;
  m_iNSamples++ ;

  return m_iNSamples ;
}

double 
CFRegression::GetY(double dX)
{
  return m_a * dX + m_b ;
}

double 
CFRegression::Calculate()
{
  double Divider = m_dSumX * m_dSumX - m_iNSamples * m_dSumXX ;
  if ( fabs( Divider ) > 1e-6 )
  {
    m_a = ( m_dSumX * m_dSumY - m_iNSamples * m_dSumXY ) / Divider ;
    m_b =  ( m_dSumX * m_dSumXY - m_dSumY * m_dSumXX ) / Divider ;
  }
  return Divider ;
}

int
CFRegression::Remove(double X, double Y)
{
  if ( !m_iNSamples )
    return -1 ;
  m_dSumX -= X ;
  m_dSumY -= Y ;
  m_dSumXX -= X * X ; 
  m_dSumXY -= X * Y ;
  m_dSumYY -= Y * Y ;
  m_iNSamples --;

  return m_iNSamples;
}

double CFRegression::GetRSquared()
{
 double dSumXSquared = m_dSumX * m_dSumX;
 double dSumYSquared = m_dSumY * m_dSumY;
 double r = (m_iNSamples*m_dSumXY-m_dSumX*m_dSumY)/sqrt((m_iNSamples*m_dSumXX-dSumXSquared)*(m_iNSamples*m_dSumYY-dSumYSquared));
 return r;
}
