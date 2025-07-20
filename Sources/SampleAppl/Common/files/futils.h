//  $File : futils.h - file system helpers
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef _FUTILS_INC
#define _FUTILS_INC

#include <Lmcons.h>
#include <fxfc\fxfc.h>

#ifndef USE_CSTRINGS
__forceinline FXString GetFileExtension(const FXString& fName)
{
  FXString Ext;
  int pntPos=fName.ReverseFind('.');
  if ((pntPos>0) && (pntPos<fName.GetLength()))
  {
    Ext=fName.Mid(pntPos+1);
  }
  return Ext;
}

__forceinline FXString GetPathName(const FXString& fName)
{
  CFileFind cff;
  FXString _fname;

  if (cff.FindFile(fName))
  {
    cff.FindNextFile();
    _fname=(LPCTSTR)cff.GetFilePath();
  }
  else _fname=fName;
  FXString result;
  int slashpos=_fname.ReverseFind('\\');
  if (slashpos!=-1)
  {
    result=_fname.Left(slashpos+1);
  }
  else
  {
    result=".\\";
  }
  return(result);
}

__forceinline bool VerifyCreateDirectory(FXString& path)
{
  if (path.GetLength()==0) return false;

  FXString path2verify=path;

  if (path2verify[path2verify.GetLength()-1]=='\\') 
    path2verify=path2verify.Left(path2verify.GetLength()-1);

  CFileStatus fs;
  int result=CFile::GetStatus(path2verify,fs);
  if ((result) && (fs.m_attribute & CFile::directory))  return true;

  return (CreateDirectory(path2verify,NULL)!=0);
}

#endif

#ifdef __AFX_H__

__forceinline CString GetFileExtension(const CString& fName)
{
  CString Ext;
  int pntPos=fName.ReverseFind('.');
  if ((pntPos>0) && (pntPos<fName.GetLength()))
  {
    Ext=fName.Mid(pntPos+1);
  }
  return Ext;
}

__forceinline bool SearchFile(const _TCHAR* fName, CString& res)
{
  _TCHAR *fpntr;
  _TCHAR *pntr=res.GetBuffer(MAX_PATH+1);
  DWORD len= SearchPath(NULL,fName,NULL,MAX_PATH,pntr,&fpntr);
  res.ReleaseBuffer();
  return (len!=0);
}

__forceinline CString GetPathName(const CString& fName)
{
  CFileFind cff;
  CString _fname;

  if (cff.FindFile(fName))
  {
    cff.FindNextFile();
    _fname=cff.GetFilePath();
  }
  else _fname=fName;
  CString result;
  int slashpos=_fname.ReverseFind('\\');
  if (slashpos!=-1)
  {
    result=_fname.Left(slashpos+1);
  }
  else
  {
    result=".\\";
  }
  return(result);
}

__forceinline CString GetFileName(const CString& pathName)
{
  CFileFind cff;
  CString _fname(_T(""));

  if (cff.FindFile(pathName))
  {
    cff.FindNextFile();
    _fname=cff.GetFileName();
  }
  else
  {
    int slashpos=pathName.ReverseFind('\\');
    int rslashpos=pathName.ReverseFind('/');
    slashpos=(slashpos>rslashpos)?slashpos:rslashpos;
    if (slashpos!=-1)
    {
      _fname=pathName.Mid(slashpos+1);
    }
    else
    {
      _fname=pathName;
    }
  }
  return(_fname);
}

__forceinline CString GetFileTitle(const CString& pathName)
{
  CFileFind cff;
  CString _fname(_T(""));

  if (cff.FindFile(pathName))
  {
    cff.FindNextFile();
    _fname=cff.GetFileTitle();
  }
  else
  {
    int slashpos=pathName.ReverseFind('\\');
    if (slashpos!=-1)
    {
      _fname=pathName.Mid(slashpos+1);
    }
    else
    {
      _fname=pathName;
    }
    int pointpos=_fname.ReverseFind('.');
    if (pointpos!=-1)
    {
      _fname=_fname.Left(pointpos);
    }
  }
  return(_fname);
}


__forceinline CString GetStartDir(void)
{
  _TCHAR tmpS[_MAX_PATH];
  CString RetVal(_T(""));
  memset(tmpS,0,_MAX_PATH*sizeof(_TCHAR));

  HMODULE hm=GetModuleHandle(NULL); // must return handle of the calling process (.exe file)
  GetModuleFileName(hm,tmpS,_MAX_PATH);

  _TCHAR *eod = _tcsrchr(tmpS,_T('\\'));
  if (eod)
  {
    *(eod+1)=NULL;
    RetVal=tmpS;
  }
  return(RetVal);
}

__forceinline CString GetAppStartDir(void)
{
  _TCHAR tmpS[_MAX_PATH];
  CString RetVal(_T(""));
  memset(tmpS,0,_MAX_PATH*sizeof(_TCHAR));
  CWinApp* wa=AfxGetApp();

  GetModuleFileName(NULL,tmpS,_MAX_PATH);

  CString AppName=AfxGetAppName();
  _TCHAR *eod = _tcsstr(tmpS,AppName);
  if (eod)
  {
    *eod=NULL;
    RetVal=tmpS;
  }
  else
  {
    eod=_tcsrchr(tmpS,'\\');
    if (eod)
    {
      *(eod+1)=0;
      RetVal=tmpS;
    }
  }
  ASSERT(RetVal.GetLength()!=0);
  return(RetVal);
}

__forceinline BOOL GetStartingDirectory(CString& dir) // result is ending with '\'
{
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  GetStartupInfo(&si);
  if (!si.lpTitle)
    return FALSE;
  dir = si.lpTitle;
  dir = dir.Left(dir.ReverseFind('\\') + 1);
  return (!dir.IsEmpty());
}

__forceinline CString GetStartingDirectory() // result is ending with '\'
{
  TCHAR DirName[MAX_PATH] ;
  GetCurrentDirectory( MAX_PATH , DirName ) ;
  return CString( DirName ) ;
}

__forceinline CString GetAppFName(void)
{
  _TCHAR tmpS[_MAX_PATH],*pntr=NULL;

  CString AppName=AfxGetAppName();

  ::SearchPath(NULL,AppName,_T(".exe"),_MAX_PATH,tmpS,&pntr);
  if (pntr) 
    *pntr=0;
  CString RetVal=tmpS;
  RetVal+=AppName;
  RetVal+=_T(".exe");
  return(RetVal);
}


__forceinline void GetCurDir(CString& s)
{
  DWORD l=GetCurrentDirectory(0,NULL);
  _TCHAR* sz=s.GetBuffer(l+1);
  GetCurrentDirectory(l+1,sz);
  s.ReleaseBuffer();
}

__forceinline bool VerifyCreateDirectory(CString& path)
{
  if (path.GetLength()==0) return false;

  CString path2verify=path;

  if (path2verify[path2verify.GetLength()-1]=='\\') 
    path2verify=path2verify.Left(path2verify.GetLength()-1);

  CFileStatus fs;
  int result=CFile::GetStatus(path2verify,fs);
  if ((result) && (fs.m_attribute & CFile::directory))  
    return true;

  return (CreateDirectory(path2verify,NULL)!=0);
}

__forceinline bool getTempFileName(LPCTSTR prefix, LPCTSTR suffix, CString& name)
{
  TCHAR temppath[_MAX_PATH];
  TCHAR tempname[_MAX_PATH];
  GetTempPath(_MAX_PATH,temppath);
  GetTempFileName(temppath,prefix,0,tempname);
  DeleteFile(tempname); // delete this file,  we make it with aniother ext
  int pos=_tcslen(tempname)-4;
  ASSERT(_tcscmp(&tempname[pos],_T(".tmp"))==0);
  _tcscpy_s(&tempname[pos+1],_MAX_PATH - pos - 1,suffix);
  name=tempname;
  return true;
}
inline int DeleteFileBySH( LPCTSTR FileName , bool bShowErrors = false )
{
  SHFILEOPSTRUCT Operation ;
  TCHAR From[MAX_PATH] ;
  _tcscpy_s( From , FileName ) ;
  int iLen = _tcslen( From ) ;
  From[iLen + 1] = 0 ;
  memset( &Operation , 0 , sizeof(Operation) ) ;
  Operation.wFunc = FO_DELETE ;
  Operation.pFrom = From ;
  Operation.fFlags = FOF_NO_UI ; 
  int ires = SHFileOperation( &Operation ) ;
  if ( ires == ERROR_SUCCESS )
    return ERROR_SUCCESS ;
  if ( bShowErrors )
  {
    CString msg ;
    msg.Format( _T("DeleteFileBySH: ERROR 0x%x") , ires ) ;
    AfxMessageBox( msg ) ;
  }
  return ires ;
}

inline int CopyFileAndWait( LPCTSTR SrcPath , LPCTSTR DestPath )
{
  CopyFile( SrcPath , DestPath , false ) ;
  FILE * fr = NULL ;
  DWORD dwBegin = GetTickCount() ;
  while ( _tfopen_s( &fr , DestPath , _T("r") ) )  
  {
    if ( GetTickCount() - dwBegin > 1000)
      return false ; // wait up to one second
    Sleep(10) ;
  }
  if ( fr )
  {
    fclose( fr ) ;
    return true ;
  }
  return false ;
}

__forceinline CString getAppName(HINSTANCE hInstance)
{
  TCHAR  strDLLPath[_MAX_PATH];
  ::GetModuleFileName(hInstance, strDLLPath, _MAX_PATH);
  CString dir=strDLLPath;
  int fNameLen=dir.GetLength()-dir.ReverseFind('\\')-1;
  ASSERT(fNameLen>0);
  if (fNameLen>0)
  {
    dir = dir.Right(fNameLen);
    if (dir.ReverseFind('.')>0)
      dir=dir.Left(dir.ReverseFind('.'));
  }
  else
    dir.Empty();
  return dir;
}
#endif //#ifdef __AFX_H__

#endif