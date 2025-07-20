#ifndef FXEXT_INCLUDE
#define FXEXT_INCLUDE
// fxext.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <fxfc/fxtime.h>
#include <fxfc/fxarrays.h>

FXString    FXFC_EXPORT FxGetAppName();
FXString    FXFC_EXPORT FxGetAppPath();
FXString    FXFC_EXPORT FxGetModuleName(HINSTANCE hInstance);
BOOL        FXFC_EXPORT FxGetAppPath(FXString& path);
BOOL        FXFC_EXPORT FxIsDirectory(LPCTSTR path);

int         FXFC_EXPORT FxMessageBox(LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0);
__forceinline int FxMessageBox(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = 0)
{
    FXString mes;
    mes.LoadString(nIDPrompt);
    return FxMessageBox(mes, nType, nIDHelp);
}
BOOL        FXFC_EXPORT  FxIsFullPath(LPCTSTR path);
BOOL        FXFC_EXPORT  FxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
FXString    FXFC_EXPORT  FxFullPath(LPCTSTR lpszFileIn);
FXString    FXFC_EXPORT  FxGetFileExtension(LPCTSTR fName);
FXString    FXFC_EXPORT  FxExtractPath( const FXString& fName ); // famous GetPathName
FXString    FXFC_EXPORT  FxExtractRootName( LPCTSTR fName ); // famous GetPathName
BOOL        FXFC_EXPORT  FxVerifyCreateDirectory(FXString& path);
FXString    FXFC_EXPORT  FxGetProductVersion(LPCTSTR module_name);
BOOL        FXFC_EXPORT  FxVerifyCreateDirectory(LPCTSTR path);
FXString    FXFC_EXPORT  FxGetFileName(LPCTSTR pathName);
FXString    FXFC_EXPORT  FxGetNotExpandedFileName(LPCTSTR pathName);
FXString    FXFC_EXPORT  FxGetFileTitle(LPCTSTR path);
FXSIZE      FXFC_EXPORT  FxGetFileLength( int hFile ) ;
FXSIZE      FXFC_EXPORT  FxGetFileLength( FILE * f ) ;
BOOL        FXFC_EXPORT  FxGetTempFileName(LPCTSTR prefix, LPCTSTR suffix, FXString& name);
FXString    FXFC_EXPORT  FxRegularize(LPCTSTR src);
FXString    FXFC_EXPORT  FxUnregularize(LPCTSTR src);
void        FXFC_EXPORT  FxReleaseHandle(HANDLE& h);
DWORD       FXFC_EXPORT  FxAffinityGetProcessorMask();
FXString    FXFC_EXPORT  FxGenerateStringUID(LPCTSTR prefix);
FXString    FXFC_EXPORT  ConvertTimeToString( time_t _tTime ) ;
FXString    FXFC_EXPORT  GetDateAsString( LPCTSTR prefix = _T( "" ) ) ;
FXSIZE      FXFC_EXPORT  FxGetListOfDirs( LPCTSTR DirectoryForAnalysis ,
  FXStringArray& Subdirectories , FXUIntArray& NFilesInFoundDir ,
  LPCTSTR pSubDirDirWildCard = NULL ) ;
UINT        FXFC_EXPORT  FxGetNFilesInDir( LPCTSTR DirectoryForAnalysis ) ;
FXString    FXFC_EXPORT  GetTimeAsString( LPCTSTR prefix = _T( "" ) ) ;
FXString    FXFC_EXPORT  FormatFILETIMEasString_ms( FILETIME& FiTime , 
                            LPCTSTR pPrefix = _T( "" ) , LPCTSTR pSuffix = _T( "" ) ) ;
FXString    FXFC_EXPORT  GetTimeAsString_ms( LPCTSTR prefix = _T( "" ) , int Indexer = -1 ) ;
FXString    FXFC_EXPORT  GetTimeStamp( LPCTSTR prefix = _T( "" ), LPCTSTR suffix = _T( "" )) ;
FXString    FXFC_EXPORT  FxGetStringWithTime( LPCTSTR prefix , int iIndexer = 0 ) ;
SYSTEMTIME  FXFC_EXPORT  GetSystemTimeFromString( LPCTSTR pszWithoutPrefix ) ;
int         FXFC_EXPORT  FXstrcpy_s( LPTSTR Dest, int DestLen, LPCTSTR Src );
FXString    FXFC_EXPORT  Wide2FXStr(BSTR bstr);
FXString    FXFC_EXPORT  GetStringVarValue( const FXString& Src , LPCTSTR Variable ,
                              char cOpen , char cClose ) ;
// Split string Src to substrings into array Splitted with Separators
int         FXFC_EXPORT  SplitFXString( FXString& Src , LPCTSTR Separators , FXStringArray& Splitted ) ;
// If pString is inside Items, returns index, otherwise returns -1
int         FXFC_EXPORT  IsStringInArray( LPCTSTR pString , FXStringArray& Items ) ;
BOOL        FXFC_EXPORT FXGetFileNameFromHandle( HANDLE hFile , 
  TCHAR* pszFileName , const unsigned int uiMaxLen ) ;
BOOL        FXFC_EXPORT FXGetSizesOfSectorClusterAndDisk( HANDLE hFile ,
  DWORD& dwClusterSize_bytes , DWORD& dwBytesPerSector , __int64& i64DiskSize_bytes ) ;

#include <filesystem>
#include <fstream>
#include <string>

std::ofstream FxCreateOrReplaceFileWithPrefix( const std::string& directory ,
  const std::string& prefix , const std::string& separator , const std::string& suffix ) ;
#endif // #ifndef FXEXT_INCLUDE