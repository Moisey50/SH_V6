#include "stdafx.h"
#include <fxfc\fxfc.h>

double pf;

class _init_performance_cntr
{
public:
    _init_performance_cntr()
    {
        LARGE_INTEGER PF;
        QueryPerformanceFrequency(&PF);
        pf=(double)PF.QuadPart;
    }
};

_init_performance_cntr __ipc;
