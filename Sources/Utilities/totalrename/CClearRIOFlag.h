#pragma once
#include "filescanner.h"

class CClearRIOFlag :
    public CFileScanner
{
public:
    int Process(const FXFileFind& ff);
};
