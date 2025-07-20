// FRegression.h: interface for the CFRegression class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FREGRESSION_H__DA52B4B3_CAC0_11D3_8D73_000000000000__INCLUDED_)
#define AFX_FREGRESSION_H__DA52B4B3_CAC0_11D3_8D73_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFRegression  
{
public:
	double Calculate();
	double GetY( double X );
	int Add( double X , double Y );
	void Reset();
  int Remove( double X, double Y ) ;
  double GetRSquared();
	CFRegression();
	virtual ~CFRegression();

  double m_dSumX ;
  double m_dSumY ;
  double m_dSumXX ;
  double m_dSumXY ;
  double m_dSumYY ;

  int    m_iNSamples ;

  double m_a ;
  double m_b ;
};

#endif // !defined(AFX_FREGRESSION_H__DA52B4B3_CAC0_11D3_8D73_000000000000__INCLUDED_)
