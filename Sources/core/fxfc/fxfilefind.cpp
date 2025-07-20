#include "stdafx.h"
#include <fxfc\fxfc.h>

/// class FXFileDataInfo ///////////////////

FXFileDataInfo::FXFileDataInfo() :
  m_IsNew( false )
  , m_iNumberPos(0)
{}

FXFileDataInfo::FXFileDataInfo( WIN32_FIND_DATA& fd ) :
  m_IsNew( false )
  , m_iNumberPos(0)
{
  memcpy(&m_dwFileAttributes, &fd.dwFileAttributes, 
    (LPBYTE)&m_cFileName - (LPBYTE)&m_dwFileAttributes);
//   m_dwFileAttributes = fd.dwFileAttributes;
//   m_ftCreationTime = fd.ftCreationTime;
//   m_ftLastAccessTime = fd.ftLastAccessTime;
//   m_ftLastWriteTime = fd.ftLastWriteTime;
//   m_nFileSizeHigh = fd.nFileSizeHigh;
//   m_nFileSizeLow = fd.nFileSizeLow;
  _tcscpy_s(m_cFileName, fd.cFileName);
  _tcscpy_s(m_cAlternateFileName, fd.cAlternateFileName);
}
const FXFileDataInfo& FXFileDataInfo::operator=( const FXFileDataInfo& fdiSrc )
{
  if ( &fdiSrc != this )
  {
    m_IsNew = fdiSrc.m_IsNew;
    m_iNumberPos = fdiSrc.m_iNumberPos;
    memcpy(&m_dwFileAttributes, &fdiSrc.m_dwFileAttributes,
      (LPBYTE)&m_cFileName - (LPBYTE)&m_dwFileAttributes);
//     m_dwFileAttributes = fdiSrc.m_dwFileAttributes;
//     m_ftCreationTime = fdiSrc.m_ftCreationTime;
//     m_ftLastAccessTime = fdiSrc.m_ftLastAccessTime;
//     m_ftLastWriteTime = fdiSrc.m_ftLastWriteTime;
//     m_nFileSizeHigh = fdiSrc.m_nFileSizeHigh;
//     m_nFileSizeLow = fdiSrc.m_nFileSizeLow;
    _tcscpy_s(m_cFileName, fdiSrc.m_cFileName);
    _tcscpy_s(m_cAlternateFileName, fdiSrc.m_cAlternateFileName);
  }
  return *this;
}
const FXFileDataInfo& FXFileDataInfo::operator=( WIN32_FIND_DATA& fd )
{
  memcpy(&m_dwFileAttributes, &fd.dwFileAttributes,
    (LPBYTE)&m_cFileName - (LPBYTE)&m_dwFileAttributes);
//   m_dwFileAttributes = fd.dwFileAttributes;
//   m_ftCreationTime = fd.ftCreationTime;
//   m_ftLastAccessTime = fd.ftLastAccessTime;
//   m_ftLastWriteTime = fd.ftLastWriteTime;
//   m_nFileSizeHigh = fd.nFileSizeHigh;
//   m_nFileSizeLow = fd.nFileSizeLow;
  _tcscpy_s( m_cFileName , fd.cFileName) ;
  _tcscpy_s( m_cAlternateFileName , fd.cAlternateFileName ) ;
  return *this;
}
bool operator==( const FXFileDataInfo& fdi1 , const FXFileDataInfo& fdi2 )
{
  bool resV = !memcmp( &fdi1 , &fdi2, 
    (LPBYTE)&fdi1.m_cFileName - (LPBYTE)&fdi1.m_dwFileAttributes);
  resV &= ( !_tcscmp(fdi1.m_cFileName , fdi2.m_cFileName) );
  resV &= ( !_tcscmp(fdi1.m_cAlternateFileName , fdi2.m_cAlternateFileName) );
  return resV;
}

int CompareTimes(const FXFileDataInfo * fdi1, const FXFileDataInfo * fdi2)
{
  return (int)CompareFileTime(&(fdi1->m_ftCreationTime), &(fdi2->m_ftCreationTime));
}

int CompareASCIINumbers(const FXFileDataInfo * fdi1, const FXFileDataInfo * fdi2)
{
  LPCTSTR pFirst = fdi1->m_cFileName + fdi1->m_iNumberPos;
  LPCTSTR pSecond = fdi2->m_cFileName + fdi2->m_iNumberPos;
  int iNum1 = _tstoi(pFirst);
  int iNum2 = _tstoi(pSecond);
  if (iNum1 > iNum2)
    return 1;
  if (iNum1 < iNum2)
    return -1;
  return 0;
}
/// class FXFileFind ///////////////////

FXFileFind::FXFileFind() :
  m_NextPos( -1 ) ,
  m_FindNew( false )
{
  m_Files = new FXArray<FXFileDataInfo>;
}

FXFileFind::~FXFileFind()
{
  Close();
  delete m_Files;
  m_Files = NULL;
}

BOOL FXFileFind::FindFile( LPCTSTR pstrName , DWORD dwUnused )
{
  BOOL res = TRUE;
  WIN32_FIND_DATA FindFileData;

  m_Files->RemoveAll(); m_NextPos = -1;
  m_SearchRequest.Empty();
  m_Drive.Empty();
  m_Path.Empty();

  LPTSTR pstrRoot = m_SearchRequest.GetBufferSetLength( _MAX_PATH );
  LPCTSTR pstr = _tfullpath( pstrRoot , pstrName , _MAX_PATH );
  ASSERT( pstr != NULL );
  m_SearchRequest.ReleaseBuffer();
  LPTSTR pstrDrive = m_Drive.GetBufferSetLength( _MAX_PATH );
  LPTSTR pstrPath = m_Path.GetBufferSetLength( _MAX_PATH );
  _tsplitpath_s( m_SearchRequest , pstrDrive , _MAX_PATH , pstrPath , _MAX_PATH , NULL , 0 , NULL , 0 );
  m_Drive.ReleaseBuffer();
  m_Path.ReleaseBuffer();

//   char * Names[500];
//   int TimesCreate[500];
//   int TimesWrite[500];
//   int iIndex = 0;
//   SYSTEMTIME stime;

  HANDLE hFind = ::FindFirstFile( pstrName , &FindFileData );
  if ( hFind != INVALID_HANDLE_VALUE )
  {
    FXFileDataInfo fdi( FindFileData );
    m_Files->Add( fdi );

//     Names[iIndex] = fdi.m_cFileName;
//     FileTimeToSystemTime(&(fdi.m_ftCreationTime), &stime);
//     TimesCreate[iIndex] = stime.wMinute * 1000000 + stime.wSecond * 1000 + stime.wMilliseconds;
//     FileTimeToSystemTime(&(fdi.m_ftLastWriteTime), &stime);
//     TimesWrite[iIndex++] = stime.wMinute * 1000000 + stime.wSecond * 1000 + stime.wMilliseconds;

    while ( ::FindNextFile( hFind , &FindFileData ) )
    {
      fdi = FindFileData;
      fdi.m_IsNew = true;
      m_Files->Add( fdi );
//       Names[iIndex] = fdi.m_cFileName;
//       FileTimeToSystemTime(&(fdi.m_ftCreationTime), &stime);
//       TimesCreate[iIndex] = stime.wMinute * 1000000 + stime.wSecond * 1000 + stime.wMilliseconds;
//       FileTimeToSystemTime(&(fdi.m_ftLastWriteTime), &stime);
//       TimesWrite[iIndex++] = stime.wMinute * 1000000 + stime.wSecond * 1000 + stime.wMilliseconds;
    }
    FindClose( hFind );
    ::SetLastError( ERROR_SUCCESS );
  }
  else
    res = FALSE;
  m_FindNew = false;
  return res;
}

BOOL FXFileFind::FindNextFile()
{
  if ( m_NextPos < (int) m_Files->GetSize() )
  {
    if ( m_FindNew )
    {
      do
      {
        m_NextPos++;
      } while ( (m_NextPos < (int) m_Files->GetSize()) 
               && ((*m_Files)[ m_NextPos ].m_IsNew != true) );
      INT NextNext = m_NextPos + 1;
      while ( (NextNext < (int) m_Files->GetSize()) 
          && ((*m_Files)[ NextNext ].m_IsNew != true) ) 
      {
        NextNext++;
      }
      return (NextNext < (int) m_Files->GetSize());
    }
    else
    {
      m_NextPos++;
      return (m_NextPos + 1 < (int) m_Files->GetSize());
    }
  }
  else
  {
    ::SetLastError( ERROR_NO_MORE_FILES );
    return FALSE;
  }
}

void FXFileFind::Close()
{
  m_Files->RemoveAll();
  m_Files->FreeExtra();
  m_NextPos = -1;
}

FXString FXFileFind::GetFileName() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) 
    return FXString( _T( "" ) );
  return (*m_Files)[ m_NextPos ].m_cFileName;
}

FXString FXFileFind::GetFilePath() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) 
    return FXString( _T( "" ) );

  FXString retV = m_Drive + m_Path + (*m_Files)[ m_NextPos ].m_cFileName;
  return retV;
}

FXString FXFileFind::GetFileTitle() const
{
  if ( m_NextPos >= ( int ) m_Files->GetSize() )
    return FXString( _T( "" ) );
  FXString FileName( ( *m_Files )[ m_NextPos ].m_cFileName );
  FXSIZE dot = FileName.ReverseFind( _T( '.' ) );
  if ( dot < 0 )
    return FileName;
  else
    return FileName.Left( dot );
}

FXString FXFileFind::GetFileExt() const
{
  FXString Ext ;
  if ( m_NextPos >= ( int ) m_Files->GetSize() )
    return Ext ;
  FXString FileName( ( *m_Files )[ m_NextPos ].m_cFileName );
  FXSIZE dot = FileName.ReverseFind( _T( '.' ) );
  if ( dot > 0 )
    return FileName.Mid( dot + 1 );
  return Ext ;
}

FXString FXFileFind::GetFileURL() const
{
  FXString strResult( _T( "file://" ) );
  strResult += GetFilePath();
  return strResult;
}

BOOL FXFileFind::GetCreationTime( FILETIME* pTimeStamp ) const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  *pTimeStamp = (*m_Files)[ m_NextPos ].m_ftCreationTime;
  return TRUE;
}

BOOL FXFileFind::GetLastAccessTime( FILETIME* pTimeStamp ) const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  *pTimeStamp = (*m_Files)[ m_NextPos ].m_ftLastAccessTime;
  return TRUE;
}

BOOL FXFileFind::GetLastWriteTime( FILETIME* pTimeStamp ) const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  *pTimeStamp = (*m_Files)[ m_NextPos ].m_ftLastWriteTime;
  return TRUE;
}

ULONGLONG FXFileFind::GetLength() const
{
  ULARGE_INTEGER nFileSize;
  nFileSize.LowPart = (*m_Files)[ m_NextPos ].m_nFileSizeLow;
  nFileSize.HighPart = (*m_Files)[ m_NextPos ].m_nFileSizeHigh;
  return nFileSize.QuadPart;
}

FXString FXFileFind::GetRoot() const
{
  return (m_Drive + m_Path);
}

BOOL FXFileFind::IsArchived() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  return (((*m_Files)[ m_NextPos ].m_dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE) != 0);
}

BOOL FXFileFind::IsCompressed() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  return (((*m_Files)[ m_NextPos ].m_dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED) != 0);
}

BOOL FXFileFind::IsDirectory() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  return (((*m_Files)[ m_NextPos ].m_dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0);
}

BOOL FXFileFind::IsDots() const
{
  BOOL bResult = FALSE;
  if ( (m_NextPos < (int) m_Files->GetSize()) && (IsDirectory()) )
  {

    if ( (*m_Files)[ m_NextPos ].m_cFileName[ 0 ] == _T( '.' ) )
    {
      if ( (*m_Files)[ m_NextPos ].m_cFileName[ 1 ] == _T( '\0' ) 
        || (   (*m_Files)[ m_NextPos ].m_cFileName[ 1 ] == _T( '.' ) 
            && (*m_Files)[ m_NextPos ].m_cFileName[ 2 ] == _T( '\0' )) )
        bResult = TRUE;
    }
  }
  return bResult;
}

BOOL FXFileFind::IsHidden() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  return (((*m_Files)[ m_NextPos ].m_dwFileAttributes&FILE_ATTRIBUTE_HIDDEN) != 0);
}

BOOL FXFileFind::IsNormal() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  return (((*m_Files)[ m_NextPos ].m_dwFileAttributes&FILE_ATTRIBUTE_NORMAL) != 0);
}

BOOL FXFileFind::IsReadOnly() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  return (((*m_Files)[ m_NextPos ].m_dwFileAttributes&FILE_ATTRIBUTE_READONLY) != 0);
}

BOOL FXFileFind::IsSystem() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  return (((*m_Files)[ m_NextPos ].m_dwFileAttributes&FILE_ATTRIBUTE_SYSTEM) != 0);
}

BOOL FXFileFind::IsTemporary() const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  return (((*m_Files)[ m_NextPos ].m_dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY) != 0);
}

BOOL FXFileFind::MatchesMask( DWORD dwMask ) const
{
  if ( m_NextPos >= (int) m_Files->GetSize() ) return FALSE;
  return (!!((*m_Files)[ m_NextPos ].m_dwFileAttributes & dwMask));
}

/// Extra functions. They have no prototipes in MFC
BOOL FXFileFind::FindNew() // use it instead of FindFirst to get list of only new files
{
  FXString sPath = m_SearchRequest;
  FXArray<FXFileDataInfo> OldFiles;
  for ( int i = 0; i < (int) m_Files->GetSize(); i++ ) //Copy
  {
    OldFiles.Add( (*m_Files)[ i ] );
  }
  int countnew = 0;
  if ( FindFile( sPath ) )
  {
    for ( int i = 0; i < (int) m_Files->GetSize(); i++ )
    {
      if ( OldFiles.Find( (*m_Files)[ i ] ) < 0 )
      {
        (*m_Files)[ i ].m_IsNew = true;
        countnew++;
      }
      else
        (*m_Files)[ i ].m_IsNew = false;
    }
  }
  m_FindNew = true;
  return (countnew != 0);
}

FXFileFindEx::FXFileFindEx()
  : m_IterationMode(FFIM_Native)
{

}

BOOL FXFileFindEx::FindFile(LPCTSTR pstrName, FFIterationMode IterMode )
{
//   char * NamesBefore[500];
//   char * NamesAfter[500];
//   int TimesBefore[500];
//   int TimesAfter[500];

  if ( FXFileFind::FindFile(pstrName) && (m_Files->GetCount() >= 2) )
  {
//     SYSTEMTIME stime;
//     for (int i = 0; i < m_Files->GetCount(); i++)
//     {
//       NamesBefore[i] = m_Files->GetAt(i).m_cFileName;
//       FileTimeToSystemTime(&(m_Files->GetAt(i).m_ftCreationTime), &stime);
//       TimesBefore[i] = stime.wMinute * 1000000 + stime.wSecond * 1000 + stime.wMilliseconds;
//     }
    m_IterationMode = IterMode;
    switch( IterMode )
    {
      case FFIM_Time:
      {

        qsort( m_Files->GetData(), m_Files->GetCount(), sizeof(FXFileDataInfo),
          (_CoreCrtNonSecureSearchSortCompareFunction)CompareTimes);
      }
      break;
      case FFIM_Number:
      {
        FXString FileName = FxGetNotExpandedFileName(pstrName);
        int iStarPos = (int)FileName.Find(_T('*'));
        if ( iStarPos > 0 ) // star can't be first symbol in pattern
        {
          for ( int i = 0 ; i < (int)m_Files->GetCount() ; i++ )
          {
            m_Files->GetAt(i).m_iNumberPos = iStarPos  ;
          }
          qsort(m_Files->GetData(), m_Files->GetCount(), sizeof(FXFileDataInfo),
            (_CoreCrtNonSecureSearchSortCompareFunction)CompareASCIINumbers);
        }
      }
      break;
    }
//     for (int i = 0; i < m_Files->GetCount(); i++)
//     {
//       NamesAfter[i] = m_Files->GetAt(i).m_cFileName;
//       FileTimeToSystemTime(&(m_Files->GetAt(i).m_ftCreationTime), &stime);
//       TimesAfter[i] = stime.wMinute * 1000000 + stime.wSecond * 1000 + stime.wMilliseconds;
//     }
  };
  int iNFiles = (int)m_Files->GetCount();
  return ( iNFiles > 0) ;
};
//BOOL FXFileFindEx::FindNextFile();
FILETIME FXFileFindEx::GetFileTime() const
{
  const FXFileDataInfo * pData = GetCurrentFileInfo();
  if ( pData )
    return pData->m_ftLastWriteTime ;
  FILETIME retval = { 0,0 };
  return retval ;
};

#include <filesystem>

FXString FXGetExeFilePath()
{
//#ifdef _WIN32
    // Windows specific
  TCHAR szPath[ MAX_PATH ];
  GetModuleFileName( NULL , szPath , MAX_PATH );
  return FXString( szPath ) ;
// #elif __APPLE__
//   char szPath[ PATH_MAX ];
//   uint32_t bufsize = PATH_MAX;
//   if ( !_NSGetExecutablePath( szPath , &bufsize ) )
//     return std::filesystem::path { szPath }.parent_path() / ""; // to finish the folder path with (back)slash
//   return {};  // some error
// #else
//     // Linux specific
//   char szPath[ PATH_MAX ];
//   ssize_t count = readlink( "/proc/self/exe" , szPath , PATH_MAX );
//   if ( count < 0 || count >= PATH_MAX )
//     return {}; // some error
//   szPath[ count ] = '\0';
// #endif
}

FXString FXGetExeDirectory()
{
#ifdef _WIN32
    // Windows specific
  TCHAR szPath[ MAX_PATH ];
  GetModuleFileName( NULL , szPath , MAX_PATH );
  std::filesystem::path ExeFilePath( szPath ) ;
  FXString Dir(ExeFilePath.parent_path().c_str()) ;
  return Dir ;
#elif __APPLE__
  char szPath[ PATH_MAX ];
  uint32_t bufsize = PATH_MAX;
  if ( !_NSGetExecutablePath( szPath , &bufsize ) )
    return std::filesystem::path { szPath }.parent_path() / ""; // to finish the folder path with (back)slash
  return {};  // some error
#else
    // Linux specific
  char szPath[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe" , szPath , PATH_MAX );
  if ( count < 0 || count >= PATH_MAX )
    return {}; // some error
  szPath[ count ] = '\0';
#endif
}
