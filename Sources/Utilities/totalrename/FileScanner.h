#pragma once

class CFileScanner
{
public:
    CFileScanner(void);
    ~CFileScanner(void);
    int  Scan(LPCTSTR rootpath, LPCTSTR ext=NULL);
    virtual int Process(const FXFileFind& ff);
};
