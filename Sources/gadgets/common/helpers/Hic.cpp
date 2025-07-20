#include "stdafx.h"
#include <helpers\Hic.h>

#pragma comment( lib, "Vfw32.lib" )

CvidcHic::CvidcHic(FOURCC Handler)
{
    m_HIC_Compress = ICOpen(mmioFOURCC(_T('v'),_T('i'),_T('d'),_T('c')), Handler, ICMODE_COMPRESS);
    m_HIC_Decompress = ICOpen(mmioFOURCC(_T('v'),_T('i'),_T('d'),_T('c')), Handler, ICMODE_DECOMPRESS);
}

CvidcHic::~CvidcHic(void)
{
    ICClose(m_HIC_Compress);
    ICClose(m_HIC_Decompress);
}
