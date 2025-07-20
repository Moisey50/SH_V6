// FRegression.h: interface for the CFRegression class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__FStraightLineRegression_H__)
#define __FStraightLineRegression_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//#include <math/PlaneGeometry.h>
#include <math/Intf_sup.h>
#include <math/Line2d.h>

class StraightLineRegression: public CFilterRegression
{
public:
  cmplx m_cCenter ;
  int m_iCentIndex ;
  int m_iUpperIndex ;
  int m_iLowerIndex ;
  int m_iNFiltered ;
  double m_dStd ;
  double m_dStdFiltered ;
  double m_dFa  ;
  double m_dFb  ;
  double m_dFaY ;
  double m_dFbY ;
  CLine2d m_FilteredLine ;

  StraightLineRegression() {} ;

  // function extracts from figure segment with 
  // central point near cCentPt with all hearest points in range 
  // dRangeFromCenter and calculate regression 
  double GetAndCalc( const CFigure * pFig ,
    cmplx cCentPt , double dAngleFromCenter ,
    double dRangeFromCenter , double dLineDirection ,
    cmplx* pFirst = NULL , cmplx * pSecond = NULL , 
    bool bYCW = false ) ; // false if counter clock wise, true otherwise
  double DoFiltering( double dThresInStdUnits ) ;
  double DoOneSideFiltering( bool bSelectFromCenter ) ;
  double DoOneSideFiltering( bool bSelectFromCenter , 
    int iNIntervals , CFilterRegression& RegrFiltered , 
    CmplxVector * pEdgePts = NULL ) ;
  double DoFilteringByDeviation( double dMaxDeviation ,
    CFilterRegression& RegrFiltered , int& iNGood ) ;

  CLine2d GetFilteredLine() { return m_FilteredLine; } ;
  double GetStdFromFilteredLine( CFilterRegression& RegrFiltered ) ;
  double DoFilterFarestFromLine(
    CLine2d& Line , double dTakePercent ,
    CFilterRegression& RegrFiltered , CmplxVector * pEdgePts ) ;

};

#endif // !defined(__FStraightLineRegression_H__)
