#ifndef FXFILE_INCLUDE
#define FXFILE_INCLUDE
// fxfile.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <fxfc\fxexception.h>
#include "fxfc/fxtimer.h"

#ifndef SHBASE_CLI
#include <thread>
#include <queue>
#include <mutex>
#include "fxfc/FXBufferManager.h"

using namespace std ;
#endif 

class FXFC_EXPORT FXFileException : public FXException
{
public:
  enum
  {
    none ,
    genericException ,
    fileNotFound ,
    badPath ,
    tooManyOpenFiles ,
    accessDenied ,
    invalidFile ,
    removeCurrentDir ,
    directoryFull ,
    badSeek ,
    hardIO ,
    sharingViolation ,
    lockViolation ,
    diskFull ,
    endOfFile
  };
public:
  int      m_cause;
  int      m_errno;
  FXString m_strFileName;
public:
  FXFileException( int cause = FXFileException::none , int error = 0 , LPCTSTR lpszArchiveName = NULL )
  {
    m_cause = cause; m_errno = error; m_strFileName = lpszArchiveName;
  }
  virtual ~FXFileException()
  {};
  // Operations
  static  int     OsErrorToException( int error );
  static  void    ThrowOsError( int error , LPCTSTR lpszFileName = NULL );
  virtual BOOL    GetErrorMessage( LPTSTR lpszError , UINT nMaxError , PUINT pnHelpContext = NULL );
};

typedef struct tagFXFileStatus
{
  FXTime m_ctime;          // creation date/time of file
  FXTime m_mtime;          // last modification date/time of file
  FXTime m_atime;          // last access date/time of file
  ULONGLONG m_size;        // logical size of file in bytes
  BYTE m_attribute;        // logical OR of CFile::Attribute enum values
  BYTE _m_padding;         // pad the structure to a WORD
  TCHAR m_szFullName[ _MAX_PATH ]; // absolute path name
}FXFileStatus , *pFXFileStatus;

class WriteItem
{
public:
  void * m_pData ;
  size_t m_DataLen ;

  WriteItem( void * pData , size_t DataLen )
  {
    m_pData = pData ;
    m_DataLen = DataLen ;
  }
};

class FXBufferManager ;

class ManagedWriteItem
{
public:
  FXBufferManager * m_pManager ;
  int               m_iBufferIndex ;
  size_t            m_DataLen ;
  double            m_dGetTime ;

  ManagedWriteItem( FXBufferManager * pManager , int iBufIndex , size_t DataLen )
  {
    m_pManager = pManager ;
    m_iBufferIndex = iBufIndex ;
    m_DataLen = DataLen ;
    m_dGetTime = GetHRTickCount() ;
  }
};

//void __declspec(noreturn) FXThrowFileException(int cause, int error = 0,LPCTSTR lpszFileName = NULL);

class FXFC_EXPORT FXFile
{
public:
  // Flag values
  enum OpenFlags
  {
    modeRead = (int) 0x00000 ,
    modeWrite = (int) 0x00001 ,
    modeReadWrite = (int) 0x00002 ,
    shareCompat = (int) 0x00000 ,
    shareExclusive = (int) 0x00010 ,
    shareDenyWrite = (int) 0x00020 ,
    shareDenyRead = (int) 0x00030 ,
    shareDenyNone = (int) 0x00040 ,
    modeNoInherit = (int) 0x00080 ,
    modeCreate = (int) 0x01000 ,
    modeNoTruncate = (int) 0x02000 ,
    typeText = (int) 0x04000 , // typeText and typeBinary are
    typeBinary = (int) 0x08000 , // used in derived classes only
    osNoBuffer = (int) 0x10000 ,
    osWriteThrough = (int) 0x20000 ,
    osRandomAccess = (int) 0x40000 ,
    osSequentialScan = (int) 0x80000 ,
  };
  enum Attribute
  {
    normal = (int) 0x00 ,
    readOnly = (int) 0x01 ,
    hidden = (int) 0x02 ,
    system = (int) 0x04 ,
    volume = (int) 0x08 ,
    directory = (int) 0x10 ,
    archive = (int) 0x20
  };
  enum SeekPosition
  {
    begin = (int) 0x0 ,
    current = (int) 0x1 ,
    end = (int) 0x2
  };
  static const FILE* hFileNull;
protected:
  BOOL        m_CloseOnDelete;
  FXString    m_FileName;
#ifndef SHBASE_CLI
  std::thread * m_pAsyncThread = NULL ;
  queue<WriteItem> m_DataForWriting ;
  queue<ManagedWriteItem> m_ManagedDataForWriting ;
  std::mutex  m_DataAccessMutex ;
  HANDLE      m_hDataAvailable = NULL ;

  bool        m_bCloseThread ;
#endif 

public:
  FILE*       m_pFile;
  size_t      m_NAllocatedBytes ;
  size_t      m_nReleasedBytes ;
  size_t      m_NWriteStorageBytes ;
  size_t      m_NWrittenBlocks ;
  double      m_dAverLifeTime ;

  DWORD m_BytesPerSector = 0 ;
  DWORD m_TotalNumberOfClasters = 0;
  DWORD m_ClusterSize_bytes = 0;
  __int64 m_i64DiskSize_bytes = 0 ;


public:
  FXFile();
  FXFile( LPCTSTR lpszFileName , UINT nOpenFlags );
  virtual BOOL Open( LPCTSTR lpszFileName , UINT nOpenFlags , FXFileException* pError = NULL );

  virtual void Close();
  operator FILE*() const;
  virtual void Abort();
  virtual FXFile* Duplicate() const;
  virtual void Flush();
  virtual FXSIZE Read( void* lpBuf , FXSIZE nCount );
  virtual void Write( const void* lpBuf , FXSIZE nCount );

  virtual ULONGLONG GetLength() const;
  virtual ULONGLONG Seek( LONGLONG lOff , UINT nFrom );
  void      SeekToBegin();
  ULONGLONG SeekToEnd();
  virtual ULONGLONG GetPosition() const;

  virtual void      SetLength( ULONGLONG dwNewLen );

  virtual FXString  GetFileName() const;
  virtual FXString  GetFilePath() const;
  virtual FXString  GetFileTitle() const;
  DWORD GetClasterSize() { return m_ClusterSize_bytes ;  }

  BOOL      GetStatus( FXFileStatus& rStatus ) const;
  static BOOL       GetStatus( LPCTSTR lpszFileName , FXFileStatus& rStatus );
  static void       Remove( LPCTSTR lpszFileName );
  static void       Rename( LPCTSTR lpszOldName , LPCTSTR lpszNewName );
  static void       SetStatus( LPCTSTR lpszFileName , const FXFileStatus& status );

#ifndef SHBASE_CLI
  virtual void Write( FXBufferManager * pManager , int iBufIndex , FXSIZE nCount ) ;
  static void  AsyncWriteThreadFunc( FXFile * pFile ) ;
  virtual bool SetAsyncWriteMode( bool bSet ) ;
  size_t       GetWritingQueueLength() { return m_DataForWriting.size() ; }
#endif

  /// Not exists in CFile
  static BOOL IsExist( LPCTSTR fName );
};

#endif //#ifndef FXFILE_INCLUDE