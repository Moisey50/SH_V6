#pragma once
#include "filescanner.h"

class CFilenameToLowercase :
    public CFileScanner
{
public:
    int Process(const FXFileFind& ff);
};
