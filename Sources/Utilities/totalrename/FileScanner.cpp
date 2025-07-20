#include "StdAfx.h"
#include "FileScanner.h"

CFileScanner::CFileScanner(void)
{
}

CFileScanner::~CFileScanner(void)
{
}

int  CFileScanner::Scan(LPCTSTR rootpath, LPCTSTR ext)
{
   FXFileFind finder;
   int count=0;
   CString fp=rootpath;
   fp+="\\*.*";
   BOOL bWorking = finder.FindFile(fp);
   while (bWorking)
   {
      bWorking = finder.FindNextFile();
      if (finder.IsDots()) continue;
      if (finder.IsDirectory())
      {
          count+=Scan(finder.GetFilePath(), ext);
      }
      CString Extension=FxGetFileExtension(finder.GetFileName());
      if ((ext==NULL) || (Extension.CompareNoCase(ext)==0))
          count+=Process(finder);
   }
   finder.Close();
   return count;
}

int CFileScanner::Process(const FXFileFind& ff)
{
    TRACE("File \"%s\" processed\n",(LPCTSTR)ff.GetFilePath());
    return 1;
}
