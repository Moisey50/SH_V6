#include "stdafx.h"
#include <io.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <fxfc\fxfc.h>
#include <direct.h>
#include <errno.h>
#include <filesystem>

namespace fs = std::filesystem ;

int FxMessageBox(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
{
    FXString appName=FxGetAppName();
    if ((nType&MB_ICONMASK)==0)
        nType|=MB_ICONEXCLAMATION;
    return ::MessageBox(NULL,lpszText,appName,nType);
}

FXString FxGetAppName()
{
    FXString retV;
    GetModuleFileName(NULL,retV.GetBuffer(MAX_PATH),MAX_PATH);
    retV.ReleaseBuffer();
    FXSIZE pos=retV.ReverseFind(_T('\\'));
    if (pos>=0)
    {
        retV=retV.Mid(pos+1);
    }
    pos=retV.ReverseFind(_T('.'));
    if (pos>=0)
    {
        retV=retV.Left(pos);
    }
    return retV;
}

FXString FxGetModuleName(HINSTANCE hInstance)
{
    FXString retV;
    GetModuleFileName(hInstance,retV.GetBuffer(MAX_PATH),MAX_PATH);
    retV.ReleaseBuffer();
    FXSIZE pos=retV.ReverseFind(_T('\\'));
    if (pos>=0)
    {
        retV=retV.Mid(pos+1);
    }
    pos=retV.ReverseFind(_T('.'));
    if (pos>=0)
    {
        retV=retV.Left(pos);
    }
    return retV;
}

BOOL FxIsDirectory(LPCTSTR path)
{
    FXFileStatus rStatus;
    if (FXFile::GetStatus(path,rStatus))
    {
        return ((rStatus.m_attribute & FXFile::directory)!=0);
    }
    return FALSE;
}

FXString FxGetAppPath()
{
    FXString retV;
    GetModuleFileName(NULL,retV.GetBuffer(MAX_PATH),MAX_PATH);
    retV.ReleaseBuffer();
    FXSIZE pos=retV.ReverseFind(_T('\\'));
    if (pos>=0)
    {
        retV=retV.Left(pos);
    }
    return retV;
}

BOOL FxGetAppPath(FXString& path)
{
    GetModuleFileName(NULL,path.GetBuffer(MAX_PATH),MAX_PATH);
    path.ReleaseBuffer();
    FXSIZE pos=path.ReverseFind(_T('\\'));
    if (pos>=0)
    {
        path=path.Left(pos);
    }
    return FxIsDirectory(path);
}

BOOL FxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn)
{
    FXFileFind ff;
    FXString fPath;

    BOOL fxRes = ff.FindFile(lpszFileIn);
    if (fxRes)
    {
        ff.FindNextFile();
        fPath=ff.GetFilePath();
        if (fPath.GetLength()>=MAX_PATH)
            fxRes=FALSE;
        else
        {
            _tcscpy_s(lpszPathOut,MAX_PATH,fPath);
        }
    }
    ff.Close();
    return fxRes;
}

BOOL FxIsFullPath(LPCTSTR path)
{
    return (_tcschr(path,_T(':'))!=NULL);
}

FXString FxFullPath(LPCTSTR lpszFileIn)
{
    FXFileFind ff;
    FXString fPath,retV;
    FXFile f;

    BOOL fxRes = ff.FindFile(lpszFileIn);
    if (!fxRes)
    {
        f.Open(lpszFileIn,FXFile::modeCreate|FXFile::modeWrite);
        fxRes = ff.FindFile(lpszFileIn);
    }
    if (fxRes)
    {
        ff.FindNextFile();
        fPath=ff.GetFilePath();
        if (fPath.GetLength()<MAX_PATH)
        {
            retV=fPath;
        }
    }
    if (f.m_pFile!=FXFile::hFileNull)
    {
        f.Close(); FXFile::Remove(lpszFileIn);
    }
    ff.Close();
    return retV;
}

FXSIZE FxGetFileLength( int hFile )
{
  FXSIZE sz = -1L ;
#ifdef _WIN64    
  struct _stat64 status ;
  int iRes = _fstat64( (int) _get_osfhandle( hFile ) ,
    &status ) ;
  if ( iRes == 0 )
    sz = status.st_size ;
#else
  struct _stat32 status ;
  int iRes = _fstat32( (int) _get_osfhandle( hFile ) ,
    &status ) ;
  if ( iRes == 0 )
    sz = status.st_size ;
#endif
  return sz ;
}

FXSIZE FxGetFileLength( FILE * f )
{
#ifdef _WIN64
  fpos_t CurrPos = _ftelli64(f);

  fpos_t EndPos = 0L ;
  _fseeki64(f, EndPos , SEEK_END);
  fgetpos(f, &EndPos);

  _fseeki64(f, CurrPos, SEEK_SET);
  return (FXSIZE)EndPos;
#else
  fpos_t CurrPos;
  fgetpos(f, &CurrPos);

  fpos_t EndPos = 0L;
  fseek(f, (LONG)EndPos, SEEK_END);
  fgetpos(f, &EndPos);

  fseek(f, (LONG)CurrPos, SEEK_SET);
  return (FXSIZE)EndPos;

#endif
}

FXString FxGetFileName(LPCTSTR pathName)
{
  FXFileFind cff;
  FXString _fname(pathName);

  if (cff.FindFile(pathName))
  {
    cff.FindNextFile();
    _fname = cff.GetFileName();
  }
  else
  {
    FXSIZE slashpos = _fname.ReverseFind(_T('\\'));
    FXSIZE rslashpos = _fname.ReverseFind(_T('/'));
    slashpos = (slashpos > rslashpos) ? slashpos : rslashpos;
    if (slashpos != -1)
    {
      _fname = _fname.Mid(slashpos + 1);
    }
  }
  return(_fname);
}
FXString FxGetNotExpandedFileName(LPCTSTR pathName)
{
  FXFileFind cff;
  FXString _fname(pathName);

  FXSIZE slashpos = _fname.ReverseFind(_T('\\'));
  FXSIZE rslashpos = _fname.ReverseFind(_T('/'));
  slashpos = (slashpos > rslashpos) ? slashpos : rslashpos;
  if (slashpos != -1)
  {
    _fname = _fname.Mid(slashpos + 1);
  }
  return(_fname);
}

FXString  FxGetFileTitle(LPCTSTR path)
{
    FXString retV(path);
    FXSIZE pos=retV.ReverseFind(_T('\\'));
    if (pos>=0)
    {
        retV=retV.Mid(pos+1);
    }
    pos=retV.ReverseFind(_T('.'));
    if (pos>=0)
    {
        retV=retV.Left(pos);
    }
    return retV;
}

FXString FxGetFileExtension(LPCTSTR fName)
{
    FXString ext, name(fName);
    FXSIZE pntpos=name.ReverseFind('.');
    if ((pntpos>0) && (pntpos<name.GetLength()))
    {
        ext=name.Mid(pntpos+1);
    }
    return ext;
}

FXString FxExtractPath(LPCTSTR fName)
{
  fs::path FullPath( fName ) ;

  FXString result = FullPath.remove_filename().c_str();
//     FXFileFind cff;
//     FXString _fname=fName;
// 
//     //_fname.TrimRight(_T('\\'));
//     if (cff.FindFile(_fname))
//     {
//         cff.FindNextFile();
//         _fname=(LPCTSTR)cff.GetFilePath();
//         if (cff.IsDirectory())
//             return _fname;
//     }
// 	
//     FXSIZE slashpos=_fname.ReverseFind('\\');
//     if (slashpos!=-1)
// 	{
//         result=_fname.Left(slashpos+1);
//     }
//     else
//     {
//         result=".\\";
//     }
    return(result);
}

FXString FxExtractPath(const FXString& fName)
{
  return FxExtractPath((LPCTSTR)fName);
}

FXString FxExtractRootName( LPCTSTR fName )
{
  fs::path FullPath( fName ) ;

  FXString result = FullPath.root_name().c_str();
  return result ;
}

BOOL CreateDirectoryAnyDepth(const char *path)
{
  if ( path[0] != '.' )
  {
    FXString Path( path ) ;
    Path.Replace( '/' , '\\' ) ;
    int res = SHCreateDirectoryEx( NULL , Path , NULL ) ; 
    switch ( res )
    {
    case ERROR_SUCCESS: return TRUE ;
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
      return TRUE ;
    default:
      {
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA( 
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS ,
          NULL , res , MAKELANGID( LANG_NEUTRAL , SUBLANG_DEFAULT ) , (LPSTR) &messageBuffer , 0 , NULL );

        FxSendLogMsg( MSG_ERROR_LEVEL , "CreateDirectoryAnyDepth" , 0 , "%s" , messageBuffer ) ;
        //Free the buffer.
        LocalFree( messageBuffer );

      }
      return FALSE ;
    }
  }

  int res ;
  char opath[MAX_PATH];
  char *p;
  size_t len;
  strcpy_s(opath, sizeof(opath), path);
  len = strlen(opath);
  if (opath[len - 1] == L'/' || opath[len - 1] == L'\\')
    opath[len - 1] = L'\0';

  for (p = opath; *p; p++)
  {
    if (*p == L'/' || *p == L'\\')
    {
      *p = L'\0';
      FXSIZE mkDirRes = _mkdir(opath);
      res = (mkDirRes == 0 || errno == EEXIST);
      if (!res)
        break;
      *p = L'/';
    }
  }
  int mkDirRes = _mkdir(opath);
  res = (mkDirRes == 0 || errno == EEXIST);
  return res;
}

BOOL FxVerifyCreateDirectory(LPCTSTR path)
{
  if ((path == NULL) || (_tcslen(path) == 0)) return FALSE;

  return CreateDirectoryAnyDepth(path);
}

BOOL FxVerifyCreateDirectory(FXString& path)
{
  return FxVerifyCreateDirectory(path.GetString());
}

#pragma comment( lib, "Version.lib" )

typedef struct _TAG_VS_VERSIONINFO
{
    WORD                wLength;
    WORD                wValueLength;
    WORD                wType;
    WCHAR               szKey[1];
    WORD                wPadding1[1];
    VS_FIXEDFILEINFO    Value;
    WORD                wPadding2[1];
    WORD                wChildren[1];
}VS_VERSIONINFO;

#define roundpos(a,b,r) (((BYTE *) (a)) + roundoffs(a,b,r))
#define roundoffs(a,b,r) (((BYTE *) (b) - (BYTE *) (a) + ((r) - 1)) & ~((r) - 1))

FXString FxGetProductVersion(LPCTSTR module)
{
    LPBYTE              pOffsetBytes;
    VS_FIXEDFILEINFO    *pFixedInfo;
    DWORD dwHandle;
    FXString retVal, tmpS;

    DWORD visize=GetFileVersionInfoSize(module,&dwHandle);
    if (visize==0) 
    {
      DWORD dwErr = GetLastError() ;
      return retVal;
    }

    GetFileVersionInfo(module,dwHandle,visize,tmpS.GetBuffer(visize+1));

    VS_VERSIONINFO *pVerInfo = (VS_VERSIONINFO *) (LPCTSTR)tmpS;

    pOffsetBytes = (BYTE *) (&(pVerInfo->szKey[wcslen(pVerInfo->szKey) + 1]));

    pFixedInfo = (VS_FIXEDFILEINFO *) roundpos(pVerInfo, pOffsetBytes, 4);

    //pFixedInfo->dwFileVersionMS    = pFixedInfo->dwFileVersionMS + 0x00010001;
    //pFixedInfo->dwFileVersionLS    = pFixedInfo->dwFileVersionLS + 0x00010001;
    pFixedInfo->dwProductVersionMS = pFixedInfo->dwProductVersionMS + 0x00010001;
    pFixedInfo->dwProductVersionLS = pFixedInfo->dwProductVersionLS + 0x00010001;
    retVal.Format("%d.%d.%d.%d",(pFixedInfo->dwFileVersionMS>>16),(pFixedInfo->dwFileVersionMS&0xFFFF),
                                (pFixedInfo->dwFileVersionLS>>16),(pFixedInfo->dwFileVersionLS&0xFFFF));
    return retVal;
}



BOOL FxGetTempFileName(LPCTSTR prefix, LPCTSTR suffix, FXString& name)
{
    TCHAR temppath[_MAX_PATH];
    TCHAR tempname[_MAX_PATH];
    GetTempPath(_MAX_PATH,temppath);
    GetTempFileName(temppath,prefix,0,tempname);
    DeleteFile(tempname); // delete this file,  we make it with aniother ext
    size_t pos=strlen(tempname)-4;
    ASSERT(_tcscmp(&tempname[pos],_T(".tmp"))==0);
    strcpy_s(&tempname[pos+1],_MAX_PATH-pos-1,suffix);
    name=tempname;
    return TRUE;
}

const _TCHAR replaced_chars[][2]={_T("\\"),_T(" "),_T(";"),_T("="),_T("\n"),_T("\r"),_T("("),_T(")")};
const _TCHAR replacement[][3]={_T("\\\\"),_T("\\b"),_T("\\s"),_T("\\e"),_T("\\n"),_T("\\r"),_T("\\o"),_T("\\c")};
const int  replacemen_nmb=8;

FXString    FxRegularize(LPCTSTR src)
{
    FXString retV(src);
    for (int i=0; i<replacemen_nmb; i++)
    {
        retV.Replace(replaced_chars[i],replacement[i]);
    }
    return retV;
}

FXString    FxUnregularize(LPCTSTR src)
{
    FXString retV;
    for (int i=0; i<(int)_tcslen(src); i++)
    {
        next:
        for (int j=0; j<replacemen_nmb; j++)
        {
            if ((src[i]==replacement[j][0]) && (src[i+1]==replacement[j][1]))
            {
                retV+=replaced_chars[j];
                i+=2;
                goto next;
            }
            
        }
        retV+=src[i];
    }
    return retV;
}

void FxReleaseHandle(HANDLE& h)
{
	if (h)
	{
		::CloseHandle(h);
		h = NULL;
	}
}

DWORD FxAffinityGetProcessorMask()
{
	DWORD retV=0;
	_SYSTEM_INFO si;
	GetSystemInfo(&si);
	for (unsigned i=0; i<si.dwNumberOfProcessors; i++)
	{
		retV=(retV<<1);
		retV|=1;
	}
	return retV;
}

FXString    FxGenerateStringUID(LPCTSTR prefix)
{
    FXString retV;
    FXTime fxTime=FXTime::GetCurrentTime();
    retV.Format("%s-%s-%04x",prefix,fxTime.Format(_T("%Y-%m%w-%d%H-%M%S")),(rand()&0xFFFF));
    return retV;
}

FXString ConvertTimeToString( time_t _tTime )
{
  FXString fxTime;
  struct tm tTime;
  errno_t err = localtime_s( &tTime , &_tTime );
  fxTime.Format( "%d-%d-%d_%d-%d-%d" , 
    tTime.tm_year + 1900 ,
    tTime.tm_mon ,
    tTime.tm_mday ,
    tTime.tm_hour ,
    tTime.tm_min ,
    tTime.tm_sec 
    );
  return fxTime;
}

FXString   GetDateAsString( LPCTSTR prefix )
{
  FXString retV;
  FXTime fxTime = FXTime::GetCurrentTime();
  retV.Format( "%s%s" , prefix , fxTime.Format( _T( "%Y-%m-%d" ) ) );
  return retV;
}


FXString   GetTimeAsString( LPCTSTR prefix )
{
  FXString retV;
  FXTime fxTime = FXTime::GetCurrentTime();
  retV.Format( "%s%s" , prefix , fxTime.Format( _T( "%Y-%m-%d_%H-%M-%S" ) ) );
  return retV;
}


FXString FormatFILETIMEasString_ms( FILETIME& FiTime , 
  LPCTSTR pPrefix , LPCTSTR pSuffix )
{
  FXString retV;
  SYSTEMTIME stime;
  //structure to store system time (in usual time format)
  FILETIME ltime;

  FileTimeToLocalFileTime( &FiTime , &ltime );//convert to local time and store in ltime
  FileTimeToSystemTime( &ltime , &stime );//convert in system time and store in stime

  retV.Format( "%s%d-%02d-%02d_%02d-%02d-%02d.%02d%s" , pPrefix ,
    stime.wYear , stime.wMonth , stime.wDay ,
    stime.wHour , stime.wMinute , stime.wSecond , stime.wMilliseconds ,
    pSuffix );

  return retV;
}

FXString   GetTimeAsString_ms( LPCTSTR prefix , int iIndexer )
{
  FXString retV;
  SYSTEMTIME stime;
  //structure to store system time (in usual time format)
  FILETIME ltime;
  //structure to store local time (local time in 64 bits)
  FILETIME ftTimeStamp;
  GetSystemTimeAsFileTime( &ftTimeStamp ); //Gets the current system time

  FileTimeToLocalFileTime( &ftTimeStamp , &ltime );//convert to local time and store in ltime
  FileTimeToSystemTime( &ltime , &stime );//convert in system time and store in stime

  if (iIndexer == -1)
  {
    retV.Format( "%s%d-%02d-%02d_%02d-%02d-%02d.%03d" , prefix , 
      stime.wYear , stime.wMonth , stime.wDay ,  
      stime.wHour , stime.wMinute , stime.wSecond , stime.wMilliseconds );
  }
  else
  {
    retV.Format("%s%d-%02d-%02d_%02d-%02d-%02d.%03d_%d", prefix,
      stime.wYear, stime.wMonth, stime.wDay,
      stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds ,
      iIndexer );

  }
  return retV;
}

FXString   GetTimeStamp( LPCTSTR prefix , LPCTSTR suffix )
{
  FXString retV;
  SYSTEMTIME stime;
  //structure to store system time (in usual time format)
  FILETIME ltime;
  //structure to store local time (local time in 64 bits)
  FILETIME ftTimeStamp;
  GetSystemTimeAsFileTime( &ftTimeStamp ); //Gets the current system time

  FileTimeToLocalFileTime( &ftTimeStamp , &ltime );//convert to local time and store in ltime
  FileTimeToSystemTime( &ltime , &stime );//convert in system time and store in stime

  retV.Format( "%s%04d%02d%02d%02d%02d%02d%03d%s" , prefix ,
    stime.wYear , stime.wMonth , stime.wDay ,
    stime.wHour , stime.wMinute , stime.wSecond , stime.wMilliseconds ,
    suffix );

  return retV;
}

SYSTEMTIME GetSystemTimeFromString( LPCTSTR pszWithoutPrefix )
{
  SYSTEMTIME stime ;

  if (6 > sscanf_s( pszWithoutPrefix , "%4hd%2hd%2hd%2hd%2hd%2hd%3hd" ,
    &stime.wYear , &stime.wMonth , &stime.wDay , &stime.wHour ,
    &stime.wMinute , &stime.wSecond , &stime.wMilliseconds ) )
  {
    memset( &stime , 0 , sizeof( stime ) ) ;
  }
  stime.wDayOfWeek = 1000 ; // not relevant

  return stime ;
}
FXString   FxGetStringWithTime( LPCTSTR prefix , int iIndexer )
{
  FXString retV;
  FXTime fxTime = FXTime::GetCurrentTime();
  retV.Format( (iIndexer) ? "%s-%s_%d" : "%s-%s%c" , prefix ,
    fxTime.Format( _T( "%X_%x" ) ) , iIndexer );
  return retV;
}

int  FXstrcpy_s(LPTSTR Dest, int DestLen, LPCTSTR Src)
{
  LPCTSTR pDest = Dest;
  DestLen /= sizeof(TCHAR);
  while ( *Src && (DestLen > 0) )
  {
    *(Dest++) = *(Src++);
    --DestLen ;
  }
  if (DestLen > 0)
    *Dest = 0;
  else
    *(Dest - 1) = 0;
  return (int)(Dest - pDest);
}

FXString Wide2FXStr(BSTR bstr)
{
  FXString retVal;
  if (!bstr)
    return (retVal);
  int size = 0;
  size = WideCharToMultiByte(CP_ACP, 0, bstr, -1, NULL, size, 0, 0);
  if (size == 0) return retVal;
  char* pntr = retVal.GetBuffer(size);
  WideCharToMultiByte(CP_ACP, 0, bstr, -1, pntr, size, 0, 0);
  retVal.ReleaseBuffer();
  return (retVal);
}

FXString GetStringVarValue( const FXString& Src , LPCTSTR Variable , 
  char cOpen , char cClose )
{
  FXString Result ;
  FXString Var( Variable ) ;
  Var += '=' ;
  FXSIZE iPos = Src.Find( Var ) ;
  if ( iPos < 0 )
    return Result ;
  iPos += Var.GetLength() ;

  int iOpenPos = (int)Src.Find( cOpen , iPos ) ;
  if ( ( iOpenPos < 0 ) || ( ( iOpenPos - iPos ) > 10 ) ) // Open char should be near 
    return Result ; // not found                             Variable name

  int iClosePos = ( int ) Src.Find( cClose , iOpenPos ) ;
  if ( iClosePos < 0 )
    return Result ;
  Result = Src.Mid( iOpenPos + 1 , iClosePos - iOpenPos - 2 ) ;
  return Result ;
} ;

int SplitFXString( FXString& Src , LPCTSTR Separators , FXStringArray& Splitted )
{
  FXSIZE iPos = 0 ;
  FXSIZE uInitialSize = Splitted.Size() ;
  FXString Token = Src.Tokenize( Separators , iPos ) ;
  while ( iPos >= 0 )
  {
    if ( !Token.Trim().IsEmpty() )
      Splitted.Add( Token ) ;
    Token = Src.Tokenize( Separators , iPos ) ;
  }
  return (int)(Splitted.Size() - uInitialSize) ;
}

int IsStringInArray( LPCTSTR pString , FXStringArray& Items )
{
  for ( FXSIZE i = 0 ; i < Items.Size() ; i++ )
  {
    if ( Items[ i ].CompareNoCase( pString ) == 0 )
      return ( int ) i ;
  }
  return -1 ;
}

FXSIZE FxGetListOfDirs( LPCTSTR pDirectoryForAnalysis ,
  FXStringArray& Subdirectories , FXUIntArray& NFilesInFoundDirs ,
  LPCTSTR pSubDirDirWildCard )
{
  FXFileFind finder;

// build a string with wildcards
  FXString strWildcard( pSubDirDirWildCard ? pSubDirDirWildCard : pDirectoryForAnalysis );
  strWildcard += _T( "\\*.*" );

  // start working for files
  BOOL bWorking = finder.FindFile( strWildcard );

  while ( bWorking )
  {
    bWorking = finder.FindNextFile();

    // skip . and .. files; otherwise, we'd
    // recur infinitely!

    if ( finder.IsDots() )
      continue;

   // if it's a directory, recursively search it

    if ( finder.IsDirectory() )
    {
      FXString SubDirectory = finder.GetFilePath();
      Subdirectories.Add( SubDirectory ) ;
      
      UINT NFiles = FxGetNFilesInDir( SubDirectory ) ;
      NFilesInFoundDirs.Add( NFiles ) ;
    }
  }

  finder.Close();

  return Subdirectories.GetCount() ;
}

UINT FxGetNFilesInDir( LPCTSTR pDirectoryForAnalysis )
{
  FXFileFind finder;
  UINT uiCounter = 0 ;

  FXString strWildcard( pDirectoryForAnalysis );
  strWildcard += _T( "\\*.*" );

  // start working for files
  BOOL bWorking = finder.FindFile( strWildcard );

  while ( bWorking )
  {
    bWorking = finder.FindNextFile();

    // skip . and .. files; otherwise, we'd
    // recur infinitely!

    if ( finder.IsDots() )
      continue;
    uiCounter++ ;
  }

  finder.Close();

  return uiCounter ;
}

BOOL FXGetFileNameFromHandle( HANDLE hFile , TCHAR* pszFileName , const unsigned int uiMaxLen )
{
  pszFileName[ 0 ] = 0;

  std::unique_ptr<BYTE[]> ptrcFni( new BYTE[ _MAX_PATH * sizeof( TCHAR ) + sizeof( FILE_NAME_INFO ) ] );
  FILE_NAME_INFO* pFni = reinterpret_cast< FILE_NAME_INFO* >( ptrcFni.get() );
  BOOL b = GetFileInformationByHandleEx( hFile ,
    FileNameInfo ,
    pFni ,
    sizeof( FILE_NAME_INFO ) + ( _MAX_PATH * sizeof( TCHAR ) ) );
  if ( b )
  {
#ifdef  _UNICODE
    wcsncpy_s( pszFileName ,
      min( uiMaxLen , ( pFni->FileNameLength / sizeof( pFni->FileName[ 0 ] ) ) + 1 ) ,
      pFni->FileName ,
      _TRUNCATE );
#else
    strncpy_s( pszFileName ,
      min( uiMaxLen , ( pFni->FileNameLength / sizeof( pFni->FileName[ 0 ] ) ) + 1 ) ,
      CW2A( pFni->FileName ) ,
      _TRUNCATE );
#endif
  }
  return b;
}

BOOL FXGetSizesOfSectorClusterAndDisk( HANDLE hFile , 
  DWORD& dwClusterSize_bytes , DWORD& dwBytesPerSector , __int64& i64DiskSize_bytes )
{
  FXString FilePath;
  LPTSTR pAsCString = FilePath.GetBuffer( 1000 ) ;
  if ( FXGetFileNameFromHandle( hFile , pAsCString , 999 ) )
  {
    FilePath.ReleaseBuffer() ;
    FXString Root = FxExtractRootName( FilePath ) ;
    if ( Root.IsEmpty() )
    {
      TCHAR DirName[ 1000 ] ;
      if ( GetCurrentDirectory( 999 , DirName ) )
        Root = FxExtractRootName( DirName ) ;
    }
    if ( !Root.IsEmpty() )
    {
      DWORD numberOfFreeClusters , dwNSectorsPerCluster , dwTotalNumberOfClasters ;
      if ( GetDiskFreeSpace( Root , &dwNSectorsPerCluster , &dwBytesPerSector ,
        &numberOfFreeClusters , &dwTotalNumberOfClasters ) )
      {
        dwClusterSize_bytes = dwBytesPerSector * dwNSectorsPerCluster ;
        i64DiskSize_bytes = ( __int64 ) dwClusterSize_bytes * ( __int64 ) dwTotalNumberOfClasters ;
        return TRUE ;
      }
    }
  }
  dwClusterSize_bytes = 4096 ; // defaulting
  return FALSE ;
}

std::ofstream FxCreateOrReplaceFileWithPrefix(
  const std::string& directory ,
  const std::string& prefix ,
  const std::string& separator ,
  const std::string& suffix
)
{
  namespace fs = std::filesystem;

  try
  {
    if ( !fs::exists( directory ) )
    {
      return std::ofstream(); // empty stream for error case
    }

    // remove the files that starts with the prefix
    for ( const auto& entry : fs::directory_iterator( directory ) )
    {
      if ( entry.is_regular_file() )
      {
        std::string filename = entry.path().filename().string();
        std::string prefix_s( prefix + separator ) ;
        if ( filename.rfind( prefix_s , 0 ) == 0 )
        { // begin with prefix
          fs::remove( entry.path() );
        }
      }
    }

    // form new file path
    fs::path filePath = fs::path( directory ) / ( prefix + separator + suffix );

    // Open file for writing
    std::ofstream ofs( filePath , std::ios::out | std::ios::trunc | std::ios::binary );
    return ofs;
  }
  catch ( ... )
  {
    return std::ofstream(); // empty stream for error case
  }
}