// fxfile.h : header file
//

#ifndef FXASYNCFILE_INCLUDE
#define FXASYNCFILE_INCLUDE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <thread>
#include <queue>
#include <mutex>
#include "fxfc/FXBufferManager.h"

typedef struct FXOVERLAPPEDEX : public OVERLAPPED
{
  LPVOID pCustomData; // Append custom data to the end of the system defined structure
  LPVOID pBuffer ;
  LARGE_INTEGER Param ;
  double m_dStartTime_ms ;
}*LPOVERLAPPEDEX;



class IAsyncOperationCompletionNotifier
{
public:
  virtual void OnAsyncOperationComplete( 
    BOOL bRead , DWORD dwErrorCode , LPVOID pParams )
  {
    return ;
  }
};

VOID WINAPI FileWriteIOCompletionRoutine( DWORD dwErrorCode ,
  DWORD dwNumberOfBytesTransfered ,
  LPOVERLAPPED lpOverlapped );

class FXFC_EXPORT FXAsyncFile
{
public:
    FXAsyncFile(LPCTSTR lpctszFileName,
      BOOL bCreate,DWORD dwDesiredAccess, DWORD dwShareMode,
      IAsyncOperationCompletionNotifier* pINotifier,
      BOOL bSequentialMode=FALSE,
      __int64 nStartOffset=0,
      BOOL bInUIThread=FALSE);
    BOOL IsOpen();
    BOOL Write(LPVOID pvBuffer,DWORD dwBufLen,DWORD dwOffsetLow=0,DWORD dwOffsetHigh=0, LARGE_INTEGER Param = { 0 } );
    BOOL Read( LPVOID pvBuffer , DWORD dwBufLen , DWORD dwOffsetLow = 0 , DWORD dwOffsetHigh = 0 , LARGE_INTEGER Param = { 0 } );
    BOOL ReadSync( LPVOID pvBuffer , DWORD dwBufLen , DWORD& dwNumberOfTransferedBytes );
    BOOL IsAsyncIOComplete(BOOL bFlushBuffers=TRUE);
    BOOL AbortIO();
    DWORD GetFileLength(DWORD* pdwOffsetHigh);
    __int64 GetLargeFileLength();
    VOID Reset(BOOL bSequentialMode=FALSE,__int64 nStartOffset=0);
    FXString GetFilePath() { return m_FilePath ; } ;
    operator HANDLE()
    {
       return m_hAsyncFile;
    }
    BOOL SeekFile(__int64 nBytesToSeek,__int64& nNewOffset,DWORD dwSeekOption);
    ULONGLONG GetPosition() const ;

    ~FXAsyncFile(void);
    VOID OnFileIOComplete(DWORD dwErrorCode,
                          DWORD dwNumberOfBytesTransfered,
                          LPOVERLAPPED lpOverlapped,BOOL bRead);
    DWORD GetClusterSizeInBytes()
    {
      return m_ClusterSize_bytes ;
    }
private:
    LPOVERLAPPED PreAsyncIO(DWORD dwOffsetLow,DWORD dwOffsetHigh);
    LPOVERLAPPED GetOverlappedPtr(DWORD dwOffsetLow,DWORD dwOffsetHigh);
    BOOL PostAsyncIO(BOOL bRet,DWORD dwBufLen);
    BOOL WaitOnUIThread();
    BOOL NormalWait();
    VOID PumpMessage();
    static VOID WINAPI FileReadIOCompletionRoutine(DWORD dwErrorCode,
                                                   DWORD dwNumberOfBytesTransfered,
                                                   LPOVERLAPPED lpOverlapped);

    VOID CheckAndRaiseIOCompleteEvent();
    VOID Cleanup(BOOL bFlushBuffers);
    static VOID OutputFormattedErrorString(const TCHAR* ptcMsg,DWORD dwErrorCode);
    FXAsyncFile();
    FXAsyncFile(const FXAsyncFile&);
    FXAsyncFile& operator=(const FXAsyncFile&);
private:
    IAsyncOperationCompletionNotifier* m_pNotifier;
    HANDLE m_hAsyncFile;
    HANDLE m_hIOCompleteEvent;
    DWORD m_dwAccessMode;
    BOOL m_bSequentialMode;
    __int64 m_nNumOfBytesRequested;
    __int64 m_nOffset;
    DWORD m_dwReqestedOvpOprnCount;
    DWORD m_dwErrorCode;
    BOOL m_bInUIThread;
    CPtrArray* m_pOvpBufArray;
    BOOL m_bCleanupDone;
    BOOL m_bAborted;
    DWORD m_SectorsPerCluster = 0;
    DWORD m_BytesPerSector = 0 ;
    DWORD m_TotalNumberOfClasters = 0;
    DWORD m_ClusterSize_bytes = 0;
    __int64 m_DiskSize_bytes = 0 ;
    FXString  m_FilePath ;
};

#endif  // FXASYNCFILE_INCLUDE