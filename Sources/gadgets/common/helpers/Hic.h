#pragma once

#include <vfw.h>

class CvidcHic
{
private:
    HIC m_HIC_Compress;
    HIC m_HIC_Decompress;
public:
    CvidcHic(FOURCC Handler);
    ~CvidcHic(void);
    bool  CanCompress() { return m_HIC_Compress!=NULL; }
    bool  CanDeCompress() { return m_HIC_Decompress!=NULL; }
};
