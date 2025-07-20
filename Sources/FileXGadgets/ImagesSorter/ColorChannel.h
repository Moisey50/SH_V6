#pragma once
#include <string>
#include "Interfaces.h"

using namespace std;

#define UNDEFINED                        (-1)
#define PERIOD_TOLERANCE_us              (120.)
#define EQUALITY_PERIOD_TOLERANCE_us     (0.01)
#define DELIMITER_CLR_NAME_TO_DELTA_TIME (':')



class ColorChannel
  : public IEquatable<ColorChannel>
{
private:
  string m_name;
  int    m_deltaTime_us;
  double m_dPeriodExpected_us;
public:
  ColorChannel(const string& name, int deltaTime_us)
    : m_name(name)
    , m_deltaTime_us(deltaTime_us)
    , m_dPeriodExpected_us(UNDEFINED)
  {}
  ~ColorChannel()
  {}

  const string& GetName()const
  {
    return m_name;
  }
  ColorChannel& SetPeriodExpected_us(uint32_t ui32PeriodNominal)
  {
    m_dPeriodExpected_us = ui32PeriodNominal + m_deltaTime_us;
    return *this;
  }
  double GetPeriodExpected_us() const
  {
    return m_dPeriodExpected_us;
  }
  bool IsColorChannel(double dPeriodActual_us) const
  {
    return fabs(m_dPeriodExpected_us - dPeriodActual_us) <= PERIOD_TOLERANCE_us;
  }
  bool Equals(const ColorChannel& other) const override
  {
    return m_name.compare(other.m_name) == 0
      && m_deltaTime_us == other.m_deltaTime_us
      && fabs(m_dPeriodExpected_us - other.m_dPeriodExpected_us) < EQUALITY_PERIOD_TOLERANCE_us;
  }

  static ColorChannel* Deserialize(const string& src)
  {
    ColorChannel* pRes = NULL;

    int dlmtrIndx = (int)src.find_first_of(DELIMITER_CLR_NAME_TO_DELTA_TIME);
    if (dlmtrIndx >= 0)
    {
      string name = src.substr(0, dlmtrIndx);
      string dltTime = src.substr(dlmtrIndx + 1);

      int nDltTime = atoi(dltTime.c_str());

      pRes = new ColorChannel(name, nDltTime);
    }
    return pRes;
  }
};