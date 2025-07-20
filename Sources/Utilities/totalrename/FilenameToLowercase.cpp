#include "StdAfx.h"
#include "FilenameToLowercase.h"

int CFilenameToLowercase::Process(const FXFileFind& ff)
{
    int changed=0;

    FXString fpath=ff.GetFilePath();
    FXString fname=ff.GetFileName();
    FXString locase=fname; locase.MakeLower();
    if (locase.Compare(fname)!=0)
    {
        bool success=true;
        locase=FxExtractPath(fpath)+locase;
        try
        {
            FXFile::Rename(fpath,locase);
        }
        catch(FXFileException* fe)
        {
            delete fe;
            success=false;
        }
        if (success)
            changed=1;
    }
    return changed;
}