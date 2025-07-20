#ifndef __LINE_DATA_H_
#define __LINE_DATA_H_

#include <classes\drect.h>
#include <classes\dpoint.h>
class CLineResult
{
public:
  CLineResult() { m_dExtremalAmpl = 0. ; m_dAverCent5x5 = 0. ; } ;
  CLineResult( CLineResult& Orig ) 
  {
    m_DRect = Orig.m_DRect ;
    m_Center = Orig.m_Center ;
    m_dAngle = Orig.m_dAngle ;
    m_dExtremalAmpl = Orig.m_dExtremalAmpl ;
    m_dAverCent5x5 = Orig.m_dAverCent5x5 ;
    m_ImageMinBrightness = Orig.m_ImageMinBrightness ;
    m_ImageMaxBrightness = Orig.m_ImageMaxBrightness ;
  }
  ~CLineResult() {} ;
  CLineResult& operator =(const CLineResult& Orig)
  {
    if ( this != &Orig )
    {
      m_DRect = Orig.m_DRect ;
      m_Center = Orig.m_Center ;
      m_dAngle = Orig.m_dAngle ;
      m_dExtremalAmpl = Orig.m_dExtremalAmpl ;
      m_dAverCent5x5 = Orig.m_dAverCent5x5 ;
      m_ImageMinBrightness = Orig.m_ImageMinBrightness ;
      m_ImageMaxBrightness = Orig.m_ImageMaxBrightness ;
    }
    return *this ;
  }


  CDRect  m_DRect ;
  CDPoint m_Center ;
  double  m_dExtremalAmpl  ;
  double  m_dAverCent5x5 ;
  double  m_dAngle ;
  int     m_ImageMaxBrightness ;
  int     m_ImageMinBrightness ;
};

typedef CArray <CLineResult> CLineResults ;

#endif  //__LINE_DATA_H_

