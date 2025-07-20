#include "stdafx.h"
#include <io.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <fxfc\fxfc.h>
#include "fxmodules.h"
#include "resource.h"
#include "fxfc/FXBufferManager.h"

#include <thread>

#pragma warning(disable : 4996)

///////////////////////////////////////////////////////////////////////
// Helpers
///////////////////////////////////////////////////////////////////////

void _FXFillExceptionInfo( FXFileException* pException , LPCTSTR lpszFileName )
{
  if ( pException != NULL )
  {
    pException->m_errno = errno;
    pException->m_cause = FXFileException::OsErrorToException( pException->m_errno );
    pException->m_strFileName = lpszFileName;
  }
}

void __declspec(noreturn) FXThrowFileException( int cause , int error , LPCTSTR lpszFileName )
{
  throw new FXFileException( cause , error , lpszFileName );
}

///////////////////////////////////////////////////////////////////////
// FXFileException class
///////////////////////////////////////////////////////////////////////

int FXFileException::OsErrorToException( int error )
{
  // NT Error codes
  switch ( error )
  {
    case 0: //
      return FXFileException::none;
    case EPERM: //Operation not permitted
      return FXFileException::accessDenied;
    case ENOENT: //No such file or directory
      return FXFileException::fileNotFound;
    case EIO: //I/O error
      return FXFileException::hardIO;
    case ENXIO: //No such device or address
      return FXFileException::badPath;
    case E2BIG: //Argument list too long
      return FXFileException::badPath;
    case EBADF: //Bad file number
      return FXFileException::hardIO;
    case ENOMEM: //Not enough memory
      return FXFileException::hardIO;
    case EACCES: //Permission denied
      return FXFileException::accessDenied;
    case EEXIST: //File exists
      return FXFileException::accessDenied;
    case ENODEV: //No such device
      return FXFileException::badPath;
    case ENOTDIR: //Not a directory
      return FXFileException::removeCurrentDir;
    case EISDIR: //Is a directory
      return FXFileException::accessDenied;
    case EINVAL: //Invalid argument
      return FXFileException::badPath;
    case ENFILE: //Too many files open in system
      return FXFileException::hardIO;
    case EMFILE: //Too many open files
      return FXFileException::hardIO;
    case ENOTTY: //Inappropriate I/O control operation
      return FXFileException::hardIO;
    case EFBIG: //File too large
      return FXFileException::endOfFile;
    case ENOSPC: //No space left on device
      return FXFileException::diskFull;
    case ESPIPE: //Invalid seek
      return FXFileException::badSeek;
    case EROFS: //Read-only file system
      return FXFileException::accessDenied;
    case ENAMETOOLONG: //Filename too long
      return FXFileException::badPath;
    case ENOTEMPTY: //Directory not empty
      return FXFileException::removeCurrentDir;
    default:
      ASSERT( TRUE );
      return FXFileException::genericException;
  }
}

void FXFileException::ThrowOsError( int error , LPCTSTR lpszFileName )
{
  if ( error != 0 )
    FXThrowFileException( FXFileException::OsErrorToException( error ) , error , lpszFileName );
}

BOOL FXFileException::GetErrorMessage( LPTSTR lpszError , UINT nMaxError , PUINT pnHelpContext )
{
  if ( lpszError == NULL || nMaxError == 0 )
  {
    return FALSE;
  }
  if ( pnHelpContext != NULL )
    *pnHelpContext = m_cause + FX_IDP_FILE_NONE;
  FXString frmt;
  frmt.LoadString( GetCurrentModule() , m_cause + FX_IDP_FILE_NONE );
  return (sprintf_s( lpszError , nMaxError , (LPCTSTR) frmt , (LPCTSTR) m_strFileName ) != -1);
}

///////////////////////////////////////////////////////////////////////
// Thread for asynchronous write
///////////////////////////////////////////////////////////////////////

void FXFile::AsyncWriteThreadFunc( FXFile * pFile )
{
#ifndef SHBASE_CLI
  pFile->m_bCloseThread = false ;
  SetCurrentThreadName( "FXFile::AsyncWriteThreadFunc" ) ;
  while ( !pFile->m_bCloseThread )
  {
    while ( !pFile->m_DataForWriting.empty() )
    {
      pFile->m_DataAccessMutex.lock() ;
      WriteItem FirstItem = pFile->m_DataForWriting.front() ;
      pFile->m_DataForWriting.pop();
      pFile->m_DataAccessMutex.unlock() ;
      size_t sz = fwrite( FirstItem.m_pData , 1 , FirstItem.m_DataLen , pFile->m_pFile );
      if ( sz != FirstItem.m_DataLen )
      {
        int iErrNo = 0 ;
        _get_errno( &iErrNo ) ;
        if ( iErrNo != 0 )
          FXFileException::ThrowOsError( iErrNo , pFile->m_FileName );
        else
          FXThrowFileException( FXFileException::diskFull , 0 , pFile->m_FileName );
      }
      free( FirstItem.m_pData ) ;
      pFile->m_nReleasedBytes += FirstItem.m_DataLen ;
      pFile->m_NWriteStorageBytes = pFile->m_NAllocatedBytes - pFile->m_nReleasedBytes ;
      pFile->m_NWrittenBlocks++ ;
    }
    while ( !pFile->m_ManagedDataForWriting.empty() )
    {
      pFile->m_DataAccessMutex.lock() ;
      ManagedWriteItem FirstItem = pFile->m_ManagedDataForWriting.front() ;
      pFile->m_ManagedDataForWriting.pop();
      pFile->m_DataAccessMutex.unlock() ;
      LPBYTE pData = FirstItem.m_pManager->GetBufferPtr( FirstItem.m_iBufferIndex ) ;
      if ( pData )
      {
        size_t sz = fwrite( pData , 1 , FirstItem.m_DataLen , pFile->m_pFile );
        if ( sz != FirstItem.m_DataLen )
        {
          int iErrNo = 0 ;
          _get_errno( &iErrNo ) ;
          if ( iErrNo != 0 )
            FXFileException::ThrowOsError( iErrNo , pFile->m_FileName );
          else
            FXThrowFileException( FXFileException::diskFull , 0 , pFile->m_FileName );
        }
      }
      FirstItem.m_pManager->ReleaseBuffer( FirstItem.m_iBufferIndex ) ;
      pFile->m_nReleasedBytes += FirstItem.m_DataLen ;
      pFile->m_NWriteStorageBytes = pFile->m_NAllocatedBytes - pFile->m_nReleasedBytes ;
      pFile->m_NWrittenBlocks++ ;
      double dLifeTime = GetHRTickCount() - FirstItem.m_dGetTime ;
      FirstItem.m_pManager->m_dAverageLifeTime += dLifeTime / pFile->m_NWrittenBlocks ;
    }
    if ( pFile->m_hDataAvailable )
      WaitForSingleObject( pFile->m_hDataAvailable , 100 ) ;
    else
      Sleep( 3 ) ;
  }
#endif
}

///////////////////////////////////////////////////////////////////////
// FXFile class
///////////////////////////////////////////////////////////////////////

const FILE* FXFile::hFileNull = NULL;

FXFile::FXFile()
{
  m_pFile = (FILE*) hFileNull;
  m_CloseOnDelete = FALSE;
}

FXFile::FXFile( LPCTSTR lpszFileName , UINT nOpenFlags )
{
  m_pFile = (FILE*) hFileNull;
  m_CloseOnDelete = FALSE;

  FXFileException e;
  if ( !Open( lpszFileName , nOpenFlags , &e ) )
    FXThrowFileException( e.m_cause , e.m_errno , e.m_strFileName );
}

BOOL FXFile::Open( LPCTSTR lpszFileName , UINT nOpenFlags , FXFileException* pError )
{
  BOOL retV = TRUE;
  FXString mode;
  ASSERT( (nOpenFlags & typeText) == 0 );   // text mode not supported
  ASSERT( m_pFile == hFileNull );
  ASSERT( m_CloseOnDelete == FALSE );
  ASSERT( m_FileName.IsEmpty() == TRUE );
  ASSERT( (modeRead | modeWrite | modeReadWrite) == 3 );

  nOpenFlags &= ~(UINT) typeBinary; // always binary

  switch ( nOpenFlags & 3 )
  {
    case modeRead:
      mode += "r";
      break;
    case modeWrite:
      if ( nOpenFlags&modeCreate )
      {
        if ( nOpenFlags&modeNoTruncate )
        {
          mode += "a";
        }
        else
        {
          mode += "w";
        }
      }
      else
      {
        if ( !IsExist( lpszFileName ) )
        {
          errno = ENOENT;
          retV = FALSE;
        }
        if ( nOpenFlags&modeNoTruncate )
        {
          mode += "a";
        }
        else
        {
          mode += "w";
        }
      }
      break;
    case modeReadWrite:
      if ( nOpenFlags&modeCreate )
      {
        if ( nOpenFlags&modeNoTruncate )
        {
          mode += "a+";
        }
        else
        {
          mode += "w+";
        }
      }
      else
      {
        if ( !IsExist( lpszFileName ) )
        {
          _set_errno( ENOENT );
          retV = FALSE;
        }
        if ( nOpenFlags&modeNoTruncate )
        {
          mode += "r+";
        }
        else
        {
          mode += "w+";
        }
      }
      break;
    default:
      ASSERT( FALSE );  // invalid share mode
  }
  if ( retV )
  {
    mode += 'b'; // FXFile always binnary
    m_pFile = ::_tfopen( lpszFileName , mode );
    if ( m_pFile == NULL )
    {
      retV = FALSE;
    }
    else
    {
      m_FileName = lpszFileName;
      m_CloseOnDelete = TRUE;
      HANDLE hFile = ( HANDLE ) _get_osfhandle( _fileno( m_pFile ) ) ;
      FXGetSizesOfSectorClusterAndDisk( hFile , m_ClusterSize_bytes , m_BytesPerSector , m_i64DiskSize_bytes ) ;
    }
  }
  if ( !retV )
  {
    _FXFillExceptionInfo( pError , lpszFileName );
  }
  return retV;
}

void FXFile::Close()
{
  BOOL bError = FALSE;
  if ( m_pFile )
    bError = (::fclose( m_pFile ) == EOF);

  m_pFile = (FILE*) hFileNull;
  m_CloseOnDelete = FALSE;
  m_FileName.Empty();
  if ( bError )
    FXFileException::ThrowOsError( (LONG)::GetLastError() , m_FileName );
  SetAsyncWriteMode( false ) ;
}


BOOL FXFile::IsExist( LPCTSTR fName )
{
  BOOL retV = true;
  FILE* fl = _tfopen( fName , "rb" );
  if ( fl == NULL )
  {
    ASSERT( errno == ENOENT );
    retV = FALSE;
  }
  else
    fclose( fl );
  return retV;
}

FXFile::operator FILE*() const
{
  return m_pFile;
}

void FXFile::Abort()
{
  Close();
}

FXFile* FXFile::Duplicate() const
{
  // Not implemented
  ASSERT( FALSE );
  return NULL;
}

void FXFile::Flush()
{
  if ( m_pFile == hFileNull )
    return;

  int ret = fflush( m_pFile );
  if ( ret == EOF )
    FXFileException::ThrowOsError( errno , m_FileName );

}

FXSIZE FXFile::Read( void* lpBuf , FXSIZE nCount )
{
  ASSERT( m_pFile );
  ASSERT( lpBuf != NULL );

  if ( nCount == 0 ) return 0;

  FXSIZE sz = fread( lpBuf , 1 , nCount , m_pFile );
  if ( sz != nCount )
  {
    if ( !feof( m_pFile ) )
      FXFileException::ThrowOsError( FXFileException::diskFull , m_FileName );
  }
  return sz;
}

void FXFile::Write( const void* lpBuf , FXSIZE nCount )
{
  ASSERT( m_pFile );
  ASSERT( lpBuf != NULL );

  if ( !m_pAsyncThread )
  {
    if ( nCount == 0 )
      return;

    size_t sz = fwrite( lpBuf , 1 , nCount , m_pFile );
    if ( sz != nCount )
    {
      if ( errno != 0 )
        FXFileException::ThrowOsError( errno , m_FileName );
      else
        FXThrowFileException( FXFileException::diskFull , 0 , m_FileName );
    }
  }
  else // Async write
  {
    void * pData = malloc( nCount ) ;
    m_NAllocatedBytes += nCount ;
    memcpy( pData , lpBuf , nCount ) ;
    WriteItem Item( pData , nCount ) ;
    m_DataAccessMutex.lock() ;
    m_DataForWriting.push( Item ) ;
    m_DataAccessMutex.unlock() ;
    SetEvent( m_hDataAvailable ) ;
  }
}

#ifndef SHBASE_CLI

void FXFile::Write( FXBufferManager * pManager , int iBufIndex , FXSIZE nCount )
{
  ASSERT( m_pFile );

  ManagedWriteItem Item( pManager , iBufIndex , nCount ) ;
  m_DataAccessMutex.lock() ;
  m_ManagedDataForWriting.push( Item ) ;
  m_DataAccessMutex.unlock() ;
  SetEvent( m_hDataAvailable ) ;
}

#endif
ULONGLONG FXFile::GetLength() const
{
  if ( m_pFile == hFileNull )
  {
    FXFileException::ThrowOsError( errno , m_FileName );
    return 0L ;
  }
  ULONGLONG sz = -1L ;
//#ifdef _WIN64    
  struct _stat64 status ;
  int iRes = _fstat64( (int)_get_osfhandle( _fileno( m_pFile ) ) ,
    &status ) ;
  if ( iRes == 0 )
    sz = status.st_size ;
//#else
//   struct _stat32 status ;
//   int iRes = _fstat32( (int)_get_osfhandle( _fileno( m_pFile ) ) ,
//     &status ) ;
//   if ( iRes == 0 )
//     sz = status.st_size ;
// #endif

  if ( sz == -1L )
    FXFileException::ThrowOsError( errno , m_FileName );
  
  return sz;
  /*    __int64 sz=0L;
      __int64 org=_ftelli64(m_pFile);
      if(org==-1L)                            goto error;
      if (_fseeki64(m_pFile,0L,SEEK_END)!=0)   goto error;
      sz=_ftelli64(m_pFile); if (sz==-1L)      goto error;
      if (_fseeki64(m_pFile,org,SEEK_CUR))     goto error;
      return sz;
  error:
      FXFileException::ThrowOsError(errno, m_FileName);
      return 0L; */
}

ULONGLONG FXFile::Seek( LONGLONG lOff , UINT nFrom )
{
  ASSERT( m_pFile );
  int origin = (nFrom == FXFile::begin) ? SEEK_SET : (nFrom == FXFile::current) ? SEEK_CUR : SEEK_END;
  if ( _fseeki64( m_pFile , lOff , origin ) != 0 )
    FXFileException::ThrowOsError( errno , m_FileName );
  return _ftelli64( m_pFile );
}

void FXFile::SeekToBegin()
{
  Seek( 0 , FXFile::begin );
}

ULONGLONG FXFile::SeekToEnd()
{
  return Seek( 0 , FXFile::end );
}

void FXFile::SetLength( ULONGLONG dwNewLen )
{
  ASSERT( m_pFile );
  if ( _chsize_s( (int)_get_osfhandle( _fileno( m_pFile ) ) , dwNewLen ) == -1 )
    FXFileException::ThrowOsError( errno , m_FileName );
}

FXString   FXFile::GetFileName() const
{
  ASSERT( m_pFile );
  FXFileFind ff;
  FXString retV;

  BOOL fxRes = ff.FindFile( m_FileName );
  if ( fxRes )
  {
    ff.FindNextFile();
    retV = ff.GetFileName();
  }
  ff.Close();
  return retV;
}

FXString FXFile::GetFilePath() const
{
  ASSERT( m_pFile );
  FXFileFind ff;
  FXString retV;

  BOOL fxRes = ff.FindFile( m_FileName );
  if ( fxRes )
  {
    ff.FindNextFile();
    retV = ff.GetFilePath();
  }
  ff.Close();
  return retV;
}

#include <Commdlg.h>
#undef GetFileTitle
#ifdef UNICODE
#define _GetFileTitle  GetFileTitleW
#else
#define _GetFileTitle  GetFileTitleA
#endif // !UNICODE

FXString FXFile::GetFileTitle() const
{
  FXString retV;
  ASSERT( m_pFile );

  ::_GetFileTitle( m_FileName , retV.GetBufferSetLength( MAX_PATH ) , MAX_PATH );
  return retV;
}

ULONGLONG FXFile::GetPosition() const
{
  ASSERT( m_pFile );
  ULONGLONG retV = _ftelli64( m_pFile );
  if ( retV == -1L )
    FXFileException::ThrowOsError( errno , m_FileName );
  return retV;
}

BOOL FXFile::GetStatus( FXFileStatus& rStatus ) const
{
  // TODO is not full compatible
  memset( &rStatus , 0 , sizeof( FXFileStatus ) );
  _tcscpy_s( rStatus.m_szFullName , _countof( rStatus.m_szFullName ) , m_FileName );

  if ( m_pFile )
  {
    struct _stat64 buf;
    int result = _fstat64( (int)_get_osfhandle( _fileno( m_pFile ) ) , &buf );
    if ( result != -1 )
    {
      rStatus.m_ctime = FXTime( buf.st_ctime );
      rStatus.m_atime = FXTime( buf.st_atime );
      rStatus.m_mtime = FXTime( buf.st_mtime );
      rStatus.m_size = buf.st_size;
      DWORD dwAttribute = ::GetFileAttributes( m_FileName );
      if ( dwAttribute == 0xFFFFFFFF )
        rStatus.m_attribute = 0;
      else
        rStatus.m_attribute = (BYTE) dwAttribute;
    }
    else
      return FALSE;
  }
  else
    return FALSE;
  return TRUE;
}

BOOL FXFile::GetStatus( LPCTSTR lpszFileName , FXFileStatus& rStatus )
{
  // TODO is not full compatible
  ASSERT( lpszFileName != NULL );

  if ( lpszFileName == NULL ) return FALSE;
  if ( _tcsclen( lpszFileName ) >= MAX_PATH ) return FALSE;
  if ( !FxFullPath( rStatus.m_szFullName , lpszFileName ) )
  {
    rStatus.m_szFullName[ 0 ] = '\0';
    return FALSE;
  }
  struct _stat64 buf;
  int result = _tstat64( lpszFileName , &buf );
  if ( result != -1 )
  {
    rStatus.m_ctime = FXTime( buf.st_ctime );
    rStatus.m_atime = FXTime( buf.st_atime );
    rStatus.m_mtime = FXTime( buf.st_mtime );
    rStatus.m_size = buf.st_size;
    DWORD dwAttribute = ::GetFileAttributes( lpszFileName );
    if ( dwAttribute == 0xFFFFFFFF )
      rStatus.m_attribute = 0;
    else
      rStatus.m_attribute = (BYTE) dwAttribute;
  }
  else
    return FALSE;
  return TRUE;
}

void FXFile::Remove( LPCTSTR lpszFileName )
{
  if ( _tremove( lpszFileName ) != 0 )
  {
    FXFileException::ThrowOsError( errno , lpszFileName );
  }
}

void FXFile::Rename( LPCTSTR lpszOldName , LPCTSTR lpszNewName )
{
  if ( _trename( lpszOldName , lpszNewName ) != 0 )
  {
    FXFileException::ThrowOsError( errno , lpszOldName );
  }
}

void FXFile::SetStatus( LPCTSTR lpszFileName , const FXFileStatus& status )
{
  // TODO is not full compatible
  if ( !SetFileAttributes( lpszFileName , status.m_attribute ) )
  {
    //FXFileException::ThrowOsError(errno, lpszOldName);
  }
}

bool FXFile::SetAsyncWriteMode( bool bSet )
{
#ifndef SHBASE_CLI
  if ( bSet )
  {
    if ( !m_pAsyncThread )
    {
      if ( !m_hDataAvailable )
        m_hDataAvailable = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
      m_NWriteStorageBytes = m_NWrittenBlocks = m_NAllocatedBytes = m_nReleasedBytes = 0 ;
      m_pAsyncThread = new std::thread( AsyncWriteThreadFunc , this ) ;
    }
  }
  else
  {
    if ( m_pAsyncThread )
    {
      m_bCloseThread = true ;
      if ( m_hDataAvailable )
      {
        SetEvent( m_hDataAvailable ) ;
        Sleep( 2 ) ;
        CloseHandle( m_hDataAvailable ) ;
        m_hDataAvailable = NULL ;
      }
      Sleep( 5 ) ;
      m_pAsyncThread->join() ;
      m_pAsyncThread = NULL ;
    }
  }
  return true ;
#elif
  return false ;
#endif 

}