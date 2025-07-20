#pragma once
class PID_Control
{

public:
  double m_dLastVal ;
  double m_dIntegral ;
  double m_dLastDiff ;
  double m_dKp ;
  double m_dKi ;
  double m_dKd ;
  double m_dDecrease ;
  PID_Control()
  {
    m_dLastVal = m_dLastDiff = m_dIntegral = 0. ;
    m_dKp = 0.3 ;
    m_dKi = 1.e-3 ;
    m_dKd = 1.e-1 ;
    m_dDecrease = 0.1 ;
  }

  ~PID_Control()
  {
  }

  void SetParameters( double dKp , double dKi , double dKd )
  {
    m_dKp = dKp ;
    m_dKi = dKi ;
    m_dKd = dKd ;
  }
  double PeocessNextValue( double dValue )
  {
    m_dLastDiff = dValue - m_dLastVal ;
    m_dLastVal = dValue ;
    m_dIntegral += dValue ;
    double dOutVal = m_dKp * dValue
      + m_dKi * m_dIntegral + m_dKd * m_dLastDiff ;
    m_dIntegral *= ( 1. - m_dDecrease ) ;
    return dOutVal ;
  }
  void Reset() { m_dLastVal = m_dLastDiff = m_dIntegral = 0. ; }
};

