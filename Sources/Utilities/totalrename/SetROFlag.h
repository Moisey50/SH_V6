#pragma once
#include "filescanner.h"

class CSetROFlag :
    public CFileScanner
{
public:
    int Process(const FXFileFind& ff);
};
