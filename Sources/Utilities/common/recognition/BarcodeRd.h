#pragma once

#include <video\TVFrame.h>
#include <imageproc\statistics.h>

class CBarcodeRd
{
private:
    FXString  m_Result;
    double   m_TS;
public:
    CBarcodeRd(void);
    ~CBarcodeRd(void);
    void CleanUp(void);
    bool Parse(pTVFrame frame, bool freedata=true);
    bool Parse(CSData& d, FXString& res);
    LPCTSTR GetResult() { return m_Result; };
    double GetTimeSpent() { return m_TS; }
};
