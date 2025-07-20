#include "StdAfx.h"
#include "SetROFlag.h"

int CSetROFlag::Process(const FXFileFind& ff)
{
    int changed=0;
    bool success=true;

    FXString fpath=ff.GetFilePath();
    if ((!ff.IsReadOnly()) && (!ff.IsDirectory()))
    {
        try
        {
            FXFileStatus rStatus;
            if (FXFile::GetStatus(fpath,rStatus))
            {
                rStatus.m_attribute|=FXFile::readOnly;
                FXFile::SetStatus(fpath,rStatus);
            }
            else
                success=false;
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