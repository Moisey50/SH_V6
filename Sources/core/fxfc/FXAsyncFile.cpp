#include "stdafx.h"
#include "fxfc/FXAsyncFile.h"
#include <afxcoll.h>

#define TRACE_ENABLED

#ifdef TRACE_ENABLED
#define TRACE_STATUS(msg)\
        OutputDebugString(msg);
#define TRACE_ERROR(msg,errorcode)\
        OutputFormattedErrorString(msg,errorcode);
#else
#define TRACE_STATUS(msg)
#endif

const int MAX_TIMEOUT = 60*1000; // 1 minute, thinks it is sufficient


/*//////////////////////////////////////////////////////////////////////////////////////
The call back function registered with WriteFileEx. This will be invoked when a WriteFilEx
operation is completed.

dwErrorCode                 - The status of the operation
dwNumberOfBytesTransfered   - The number of bytes written
lpOverlapped                - The FXOVERLAPPEDEX pointer provided during WriteFileEx call
///////////////////////////////////////////////////////////////////////////////////////*/

VOID WINAPI FileWriteIOCompletionRoutine( DWORD dwErrorCode ,
  DWORD dwNumberOfBytesTransfered ,
  LPOVERLAPPED lpOverlapped )
{
  FXAsyncFile * pAsyncFile = ( FXAsyncFile *)(( ( LPOVERLAPPEDEX ) lpOverlapped )->pCustomData) ;

  pAsyncFile->OnFileIOComplete( dwErrorCode , dwNumberOfBytesTransfered , lpOverlapped , FALSE );
}


/*//////////////////////////////////////////////////////////////////////////////////////

The constructor which opens the specified file in overlapped mode. It creates
the file bCreate = TRUE, otherwise open existing. 
An I/O callback interface can be specified by the client. 
This shall be invoked when the IOCompletion routine is triggered by the system. 

The AyncFile object can be created in sequential mode. In this case the offset 
shall be calculated internally. But if the clients specifies the offset, this 
flag is reset and client specified offset will be used. 

The client can also specify whether it is in a UI thread or not. If UI thread, 
the message pumbing shall be done to make the UI responsive.

lpctszFileName    - The full path to file

bCreate           - Whether a new file needs to be created or not. 
                   If the file already exists, the contents will be lost.

dwDesiredAccess, dwShareMode - Same as that for CreateFile

pINotifier      - The callback interface specified by client. 
                  Its OnAsyncOperationComplete function shall be invoked 
                  from IOCompletion routine.

bSequentialMode - If this flag is TRUE the offset is incremented sequentially. Here
                  the client does not need to specify the offset in Read/Write calls. 
                  Otherwise client needs to specify the offset
                  during the Read/Write calls.

nStartOffset    - The offset from which Read/Write needs to be started. This is relevant
                  only in case bSequentialMode = TRUE

bInUIThread     - Whether asynchronous I/O is performed in the context of UI thread or not

///////////////////////////////////////////////////////////////////////////////////////*/

FXAsyncFile::FXAsyncFile(LPCTSTR lpctszFileName,BOOL bCreate,
                     DWORD dwDesiredAccess, DWORD dwShareMode,
                     IAsyncOperationCompletionNotifier* pINotifier,
                     BOOL bSequentialMode,
                     __int64 nStartOffset,
                     BOOL bInUIThread)
                     :m_pNotifier(pINotifier),
                      m_hAsyncFile(INVALID_HANDLE_VALUE),
                      m_hIOCompleteEvent(0),
                      m_dwAccessMode(dwDesiredAccess),
                      m_bSequentialMode(bSequentialMode),
                      m_nNumOfBytesRequested(0),
                      m_nOffset(nStartOffset),
                      m_dwReqestedOvpOprnCount(0),
                      m_dwErrorCode(0),
                      m_bInUIThread(bInUIThread),
                      m_pOvpBufArray(0),
                      m_bCleanupDone(FALSE),
                      m_bAborted(FALSE)
{
    DWORD dwCreateMode = OPEN_EXISTING;
    if( bCreate )
        dwCreateMode = CREATE_ALWAYS;

    if( 0 != lpctszFileName )
    {
      DWORD dwFlags = FILE_FLAG_NO_BUFFERING ;
      if ( dwDesiredAccess & GENERIC_WRITE )
        dwFlags |= FILE_FLAG_OVERLAPPED ;
        m_hAsyncFile= CreateFile(lpctszFileName,dwDesiredAccess,dwShareMode,0,dwCreateMode,
          dwFlags , 0 );
        m_FilePath = lpctszFileName ;
    }
    if( INVALID_HANDLE_VALUE == m_hAsyncFile )
    {        
        TCHAR tcszErrMsg[100] = {0};
        _stprintf_s(tcszErrMsg,_T("File open failed -->%s"),lpctszFileName);
        TRACE_ERROR(tcszErrMsg,GetLastError());
    }
    else
    {
      FXString Root = FxExtractRootName( m_FilePath ) ;
      DWORD numberOfFreeClusters ;
      if ( GetDiskFreeSpace( Root , &m_SectorsPerCluster , &m_BytesPerSector ,
        &numberOfFreeClusters , &m_TotalNumberOfClasters ) )
      {
        m_ClusterSize_bytes = m_BytesPerSector * m_SectorsPerCluster ;
        m_DiskSize_bytes = ( __int64 ) m_ClusterSize_bytes * ( __int64 ) m_TotalNumberOfClasters ;
      }
      else
        m_ClusterSize_bytes = 4096 ;
    }
    m_pOvpBufArray = new CPtrArray;
    m_hIOCompleteEvent = CreateEvent( 0 , TRUE , FALSE , 0 );

}

/*//////////////////////////////////////////////////////////////////////////////////////

Returns TRUE if the file is successfully opened.

///////////////////////////////////////////////////////////////////////////////////////*/

 BOOL FXAsyncFile::IsOpen()
{
    return (INVALID_HANDLE_VALUE != m_hAsyncFile);
}

/*//////////////////////////////////////////////////////////////////////////////////////
Performs the asynchronous Write operation.

pvBuffer    - The buffer to be written
dwBufLen    - The length of the buffer specified
dwOffsetLow - The lower part of the offset where input data is to be written
dwOffsetHigh- The higher part of the offset where input data is to be written. Normally
              this is specified for big files > 4 GB
///////////////////////////////////////////////////////////////////////////////////////*/

BOOL FXAsyncFile::Write(LPVOID pvBuffer,DWORD dwBufLen,DWORD dwOffsetLow,DWORD dwOffsetHigh, LARGE_INTEGER Param )
{
    if( 0 == pvBuffer )
        return FALSE;
    LPOVERLAPPEDEX pOverlapped = 0;
    if( 0 == ( pOverlapped = static_cast<LPOVERLAPPEDEX>(PreAsyncIO(dwOffsetLow,dwOffsetHigh))))
    {
        return FALSE;
    }
    pOverlapped->pBuffer = pvBuffer ;
    pOverlapped->Param = Param ;
    BOOL bRet = WriteFileEx(m_hAsyncFile,pvBuffer,dwBufLen,pOverlapped,
                            FileWriteIOCompletionRoutine);
    if( FALSE == bRet )
    {
      FXString ErrMsg = FxLastErr2Mes() ; 
      FxSendLogMsg( 7 , _T("WriteFileEx ERROR") , 0 , "Err=%s" , (LPCTSTR)ErrMsg );
    }
    else
    {
       TRACE_STATUS(_T("  WriteFileEx OK   "));
    }
    return PostAsyncIO(bRet,dwBufLen);    
}

/*//////////////////////////////////////////////////////////////////////////////////////
Performs the asynchronous Read operation.

pvBuffer    - The buffer to be read
dwBufLen    - The length of the buffer specified
dwOffsetLow - The lower part of the offset from where data is to be read
dwOffsetHigh- The higher part of the offset from where data is to be read. Normally
              this is specified for big files > 4 GB
///////////////////////////////////////////////////////////////////////////////////////*/

BOOL FXAsyncFile::Read(LPVOID pvBuffer,DWORD NumberOfRequestedBytes,DWORD dwOffsetLow,DWORD dwOffsetHigh, LARGE_INTEGER Param )
{
    if( !pvBuffer )
        return FALSE;
    LPOVERLAPPEDEX pOverlapped = 0;
    if( 0 == ( pOverlapped = static_cast< LPOVERLAPPEDEX >(PreAsyncIO(dwOffsetLow,dwOffsetHigh))))
    {
        return FALSE;
    }
    pOverlapped->pBuffer = pvBuffer ;
    pOverlapped->Param = Param ;
    BOOL bRet = ReadFileEx( m_hAsyncFile , pvBuffer , NumberOfRequestedBytes , pOverlapped ,
      FileReadIOCompletionRoutine );
    if( FALSE == bRet )
      FxLastErr2Mes( GetLastError() , "ERROR in FXAsyncFile::Read " ) ;
    else
    {
//        TRACE_STATUS(_T("ReadFileEx completed --> with success"));
    }
    return PostAsyncIO(bRet, NumberOfRequestedBytes );
}

BOOL FXAsyncFile::ReadSync( LPVOID pvBuffer , DWORD dwNumberOfRequestedBytes , DWORD& dwNumberOfTransferedBytes )
{
  if ( !pvBuffer )
    return FALSE;
//     LPOVERLAPPEDEX pOverlapped = 0;
//     if( 0 == ( pOverlapped = static_cast< LPOVERLAPPEDEX >(PreAsyncIO(dwOffsetLow,dwOffsetHigh))))
//     {
//         return FALSE;
//     }
//     pOverlapped->pBuffer = pvBuffer ;
//     pOverlapped->Param = Param ;
//     BOOL bRet = ReadFileEx( m_hAsyncFile , pvBuffer , dwBufLen , pOverlapped ,
//       FileReadIOCompletionRoutine );
  BOOL bRet = ReadFile( m_hAsyncFile , pvBuffer , dwNumberOfRequestedBytes , &dwNumberOfTransferedBytes , NULL );
  if ( FALSE == bRet )
    FxLastErr2Mes( GetLastError() , "ERROR in FXAsyncFile::ReadSync " ) ;
  else
  {
//     TRACE_STATUS( _T( "ReadFile completed --> with success" ) );
  }
  return bRet;
}


/*//////////////////////////////////////////////////////////////////////////////////////
The common routine called from Read/Write to prepare the FXOVERLAPPEDEX pointer with the help
of GetOverlappedPtr(). It keeps the FXOVERLAPPEDEX pointer in an array to be used during the 
GetOverlappedResult call.

dwOffsetLow - The lower part of the offset from where data is to be read/written
dwOffsetHigh- The higher part of the offset from where data is to be read/written. Normally
              this is specified for big files > 4 GB
///////////////////////////////////////////////////////////////////////////////////////*/

LPOVERLAPPED FXAsyncFile::PreAsyncIO(DWORD dwOffsetLow,DWORD dwOffsetHigh)
{
    if(!IsOpen())
    {
        return 0;
    }
    LPOVERLAPPED pOverlapped = GetOverlappedPtr(dwOffsetLow,dwOffsetHigh);
    if( 0 == pOverlapped )
    {
        return 0;
    }
//     if( m_pOvpBufArray )
//     {
//         m_pOvpBufArray->Add(pOverlapped);
//     }
    ++m_dwReqestedOvpOprnCount;
    m_bAborted = FALSE;
    return pOverlapped;
}

/*//////////////////////////////////////////////////////////////////////////////////////
The common routine called from Read/Write to prepare the FXOVERLAPPEDEX pointer. It also 
fills the custom pointer to make the context available during the OnFileIOComplete

dwOffsetLow - The lower part of the offset from where data is to be read/written
dwOffsetHigh- The higher part of the offset from where data is to be read/written. Normally
              this is specified for big files > 4 GB
///////////////////////////////////////////////////////////////////////////////////////*/

LPOVERLAPPED FXAsyncFile::GetOverlappedPtr(DWORD dwOffsetLow,DWORD dwOffsetHigh)
{
    LPOVERLAPPEDEX pOverlapped = new FXOVERLAPPEDEX;
    if( 0 == pOverlapped )
    {
        return 0;
    } 
    ZeroMemory(pOverlapped,sizeof(FXOVERLAPPEDEX));

    //if the client specified offset, override the m_bSequentialMode
    if( (0 != dwOffsetLow) || (0!=dwOffsetHigh) )
    {
        m_bSequentialMode = FALSE;
    }
    if( TRUE == m_bSequentialMode )
    {
        LARGE_INTEGER LargeOffset = {0};
        LargeOffset.QuadPart = m_nOffset;
        pOverlapped->Offset = LargeOffset.LowPart;
        pOverlapped->OffsetHigh = LargeOffset.HighPart;
    }
    else
    {
        pOverlapped->Offset = dwOffsetLow;
        pOverlapped->OffsetHigh = dwOffsetHigh;
    }
    pOverlapped->pCustomData = this;
    pOverlapped->m_dStartTime_ms = GetHRTickCount() ;
    return pOverlapped;
}

/*//////////////////////////////////////////////////////////////////////////////////////
The common routine called from Read/Write after the asynchronous I/O call. It calculates
the next offset if the m_bSequentialMode flag is TRUE. Also it keeps track of bytes requested
to Read/Write. This is to compare it after the whole I/O is completed. If all the bytes are
not Read/Write there is data loss. In the case of UI thread it dispatches the window messages
to make the UI responsive.

bRet     - The return of ReadFileEx/WriteFileEx
dwBufLen - The length of data buffer specified
///////////////////////////////////////////////////////////////////////////////////////*/

BOOL FXAsyncFile::PostAsyncIO(BOOL bRet,DWORD dwBufLen)
{
    if( m_bInUIThread )
    {
        PumpMessage();
    }
    DWORD dwLastError = GetLastError();
    if( FALSE == bRet )
    {
        TRACE_ERROR(_T("IO operation failed"),dwLastError);
        return FALSE;
    }
    
    if( ERROR_SUCCESS == dwLastError )
    {
        if( TRUE == m_bSequentialMode )
        {
            m_nOffset += dwBufLen;
        }
        m_nNumOfBytesRequested += dwBufLen;
        return TRUE;
    }
    return FALSE;
}

/*//////////////////////////////////////////////////////////////////////////////////////
The interface to cancel the asynchronous I/O operation. It signals the IOCompletion event
to stop waiting for the I/O to complete.
///////////////////////////////////////////////////////////////////////////////////////*/

BOOL FXAsyncFile::AbortIO()
{
    BOOL bRet = CancelIo(m_hAsyncFile);
    if( FALSE == bRet )
    {
       TRACE_ERROR(_T("CancelIo failed"),GetLastError());
    }
    else
    {
       TRACE_STATUS(_T("CancelIo completed --> with success"));
    }
    ::SetEvent (m_hIOCompleteEvent);
    m_bAborted = TRUE;
    return bRet;
}


/*//////////////////////////////////////////////////////////////////////////////////////
The call back function registered with WriteFileEx. This will be invoked when a ReadFileEx
oeration is completed.

dwErrorCode                 - The status of the operation
dwNumberOfBytesTransfered   - The number of bytes read
lpOverlapped                - The FXOVERLAPPEDEX pointer provided during ReadFileEx call
///////////////////////////////////////////////////////////////////////////////////////*/

VOID WINAPI FXAsyncFile::FileReadIOCompletionRoutine(DWORD dwErrorCode,
                                                   DWORD dwNumberOfBytesTransfered,
                                                   LPOVERLAPPED lpOverlapped)
{
  FXAsyncFile * pAsyncFile = ( FXAsyncFile * ) ( ( ( LPOVERLAPPEDEX ) lpOverlapped )->pCustomData ) ;

  pAsyncFile->OnFileIOComplete( dwErrorCode , dwNumberOfBytesTransfered , lpOverlapped , TRUE );
}

/*//////////////////////////////////////////////////////////////////////////////////////
The common routine which handles call back function registered with ReadFileEx/WriteFileEx.
This will be invoked when a ReadFileEx/WriteFileEx oeration is completed. It invokes the
callback interface registered by the client.

dwErrorCode                 - The status of the operation
dwNumberOfBytesTransfered   - The number of bytes read
lpOverlapped                - The FXOVERLAPPEDEX pointer provided during ReadFileEx/WriteFileEx
                              call
bRead                       - Identified Read/Write
///////////////////////////////////////////////////////////////////////////////////////*/

VOID FXAsyncFile::OnFileIOComplete(DWORD dwErrorCode,
                                 DWORD dwNumberOfBytesTransfered,
                                 LPOVERLAPPED lpOverlapped,BOOL bRead)
{
    if( 0 == lpOverlapped )
    {
        TRACE_STATUS(_T("FileIOCompletionRoutine completed --> with POVERLAPPED NULL"));
        return;
    }
    LPOVERLAPPEDEX pOverlappedEx = static_cast<LPOVERLAPPEDEX>(lpOverlapped);
    FXAsyncFile* pAsyncFile = reinterpret_cast<FXAsyncFile*>(pOverlappedEx->pCustomData);
    pAsyncFile->m_dwErrorCode = dwErrorCode;

    if( ERROR_SUCCESS == dwErrorCode )
    {
        TRACE_STATUS(_T("  FileIOComplete passed\n"));        
    }
    else
    {
        TRACE_ERROR(_T("FileIOComplete Error %d\n"),dwErrorCode);
    }
    pAsyncFile->CheckAndRaiseIOCompleteEvent();
    if( pAsyncFile->m_pNotifier )
    {
       pAsyncFile->m_pNotifier->OnAsyncOperationComplete( bRead,dwErrorCode, pOverlappedEx );
    }
}

/*//////////////////////////////////////////////////////////////////////////////////////
Checks whether all the requested overlapped calls completed by decrementing a counter.
If the counter is 0, all the requested asynchronous operations got callbacks. Then it
will set the m_hIOCompleteEvent to end the alertable wait state.
///////////////////////////////////////////////////////////////////////////////////////*/

VOID FXAsyncFile::CheckAndRaiseIOCompleteEvent()
{
    (m_dwReqestedOvpOprnCount > 0 )?--m_dwReqestedOvpOprnCount:m_dwReqestedOvpOprnCount;
    if( 0 == m_dwReqestedOvpOprnCount )
    {
        ::SetEvent(m_hIOCompleteEvent);
    }
}

/*//////////////////////////////////////////////////////////////////////////////////////
The multiple overlapped Read/Write requests can be done. Now, the thread is free to do
other operations. Finally, when the thread needs to know the status of the asynchronous
operation, it can place the thread in alretable wait state with the help of 
WaitForSingleObjectEx or MsgWaitForMultipleObjectsEx(for UI thread). Then the IOCompletion
routine registered will be called per asynchrnous I/O call requested. Once all the callbacks
are invoked the m_hIOCompleteEvent is signalled and the alertable wait state is completed.
Now, the GetOverlappedResult() API is invoked to find whether all the requested I/O operations
compeleted successfully.

bFlushBuffers - Whether to flush the file buffers or not. 
                This is required for preventing data loss as system performs a delayed write.
                So when an abnormal shutdown/restart of system happens, data is lost.
///////////////////////////////////////////////////////////////////////////////////////*/

BOOL FXAsyncFile::IsAsyncIOComplete(BOOL bFlushBuffers)
{
    if(!IsOpen())
    {
        return FALSE;
    }
    
    BOOL bWaitReturn = FALSE;
    if( m_bInUIThread )
    {
        bWaitReturn = WaitOnUIThread();
    }
    else
    {
        bWaitReturn = NormalWait();
    }

    int nTotalNumOfBytesTrans = 0;
    size_t nOvpBufferCount = m_pOvpBufArray->GetSize();

    for( size_t nIdx = 0; nIdx < nOvpBufferCount; ++nIdx )
    {
        LPOVERLAPPED lpOverlapped = reinterpret_cast<LPOVERLAPPED>(m_pOvpBufArray->GetAt(nIdx));
        DWORD dwNumberOfBytesTransferred = 0;
        BOOL bRet = GetOverlappedResult(m_hAsyncFile,lpOverlapped,&dwNumberOfBytesTransferred,TRUE);
        if( FALSE == bRet )
        {
            m_dwErrorCode = GetLastError();
            if( ERROR_IO_INCOMPLETE == m_dwErrorCode )
            {
                TRACE_STATUS(_T("IsAsyncIOComplete --> IO pending..."));
            }            
        }
        nTotalNumOfBytesTrans += dwNumberOfBytesTransferred;
        if( TRUE == m_bInUIThread )
        {
            PumpMessage();
        }
    }
    
    Cleanup(bFlushBuffers);

    if( m_nNumOfBytesRequested == nTotalNumOfBytesTrans )
    {
        m_nNumOfBytesRequested = 0;
        return TRUE;
    }

    if( FALSE == bWaitReturn )
    {
        return FALSE;
    }
    if( ERROR_SUCCESS != m_dwErrorCode )
    {
        TRACE_ERROR(_T("AsyncIO operation completed with error"),m_dwErrorCode);
        return FALSE;
    }
    return FALSE;
}

/*//////////////////////////////////////////////////////////////////////////////////////
Clears all the data structures. If bFlushBuffers = TRUE calls, FlushFileBuffers() API
to write all the pending data to disk.

bFlushBuffers - Whether to flush the file buffers or not. 
                This is required for preventing data loss as system performs a delayed write.
                So when an abnormal shutdown/restart of system happens, data is lost.
///////////////////////////////////////////////////////////////////////////////////////*/

VOID FXAsyncFile::Cleanup(BOOL bFlushBuffers)
{
    if( TRUE == m_bCleanupDone )
        return;
//     size_t nOvpBufCount = m_pOvpBufArray->GetSize();
//     for( size_t nIdx = 0; nIdx < nOvpBufCount; ++nIdx )
//     {
//         LPOVERLAPPEDEX pOverlapped = reinterpret_cast<LPOVERLAPPEDEX>(m_pOvpBufArray->GetAt(nIdx));
//         delete pOverlapped;
//     }
//     
//     m_pOvpBufArray->RemoveAll();

    if( ( 0 != m_hAsyncFile ) && ( GENERIC_READ != m_dwAccessMode ) 
      && ( TRUE == bFlushBuffers ) && ( FALSE == m_bAborted ) )
    {
        TRACE_STATUS(_T("Flushing file buffers..."));
        if( FALSE == FlushFileBuffers(m_hAsyncFile))
        {
            TRACE_ERROR(_T("FlushFileBuffers failed"),GetLastError());
        }
    }
    ::ResetEvent(m_hIOCompleteEvent);
    m_bCleanupDone = TRUE;
}

/*//////////////////////////////////////////////////////////////////////////////////////
The IOCompletion callbacks shall be invoked by the system, when the thread is palaced in
the alertable wait state. Actaully, from my observation, these callbacks are called in the
context of the wait API. In the case of UI thread, to make the UI responsive the window
messages should be dispatched. So the MsgWaitForMultipleEx() API is used to wait. Therefore
when a message is available in the queue, it will be dispatched. The function returns failure
in the case of timeout or any other failure. Otherwise it waits till the m_hIOCompleteEvent
is signalled.
///////////////////////////////////////////////////////////////////////////////////////*/

BOOL FXAsyncFile::WaitOnUIThread()
{
    for( ;; )
    {
        DWORD dwWaitOvpOprn = MsgWaitForMultipleObjectsEx( 1, &m_hIOCompleteEvent, 
                                                           MAX_TIMEOUT, QS_ALLEVENTS, MWMO_ALERTABLE|MWMO_INPUTAVAILABLE );
        switch( dwWaitOvpOprn )
        {
        case WAIT_FAILED:
            return FALSE;
        case WAIT_OBJECT_0:
            return TRUE;
        case WAIT_TIMEOUT:
            return FALSE;
        }
        
        if( m_bAborted )
        {
            return FALSE;
        }
        
        // Make the UI responsive, dispatch any message in the queue
        PumpMessage();
    }
    return FALSE;
}

/*//////////////////////////////////////////////////////////////////////////////////////
This is for non UI threads. The IOCompletion callbacks shall be invoked by the system, 
when the thread is palaced in the alertable wait state. Actaully, from my observation, 
these callbacks are called in the context of the wait API. The function returns failure
in the case of timeout or any other failure. Otherwise it waits till the m_hIOCompleteEvent
is signalled.
///////////////////////////////////////////////////////////////////////////////////////*/

BOOL FXAsyncFile::NormalWait()
{
    for( ; ; )
    {
        DWORD dwWaitOvpOprn = WaitForSingleObjectEx( m_hIOCompleteEvent,MAX_TIMEOUT, TRUE );
        switch( dwWaitOvpOprn )
        {
        case WAIT_FAILED:
            return FALSE;
        case WAIT_OBJECT_0:
            return TRUE;
        case WAIT_TIMEOUT:
            return FALSE;
        }
        
        if( m_bAborted )
        {
            return FALSE;
        }
    }
    return FALSE;
}

/*//////////////////////////////////////////////////////////////////////////////////////
To prepare the FXAsyncFile instance for next set of I/O operations. Resets all required
members.
///////////////////////////////////////////////////////////////////////////////////////*/

VOID FXAsyncFile::Reset(BOOL bSequentialMode,__int64 nStartOffset)
{
    m_bSequentialMode = bSequentialMode;
    m_nOffset = nStartOffset;
    m_nNumOfBytesRequested = 0;
    m_dwReqestedOvpOprnCount = 0;
    m_dwErrorCode = 0;
    m_bCleanupDone = FALSE;
    m_bAborted = FALSE;
}

/*//////////////////////////////////////////////////////////////////////////////////////
Returns the size of the file. This function can be used if the size is required as low
and high parts.
///////////////////////////////////////////////////////////////////////////////////////*/

DWORD FXAsyncFile::GetFileLength(DWORD* pdwOffsetHigh)
{
    return GetFileSize(m_hAsyncFile,pdwOffsetHigh);
}

/*//////////////////////////////////////////////////////////////////////////////////////
Returns the size of the file. This function can be used if the size as an int64 value.
///////////////////////////////////////////////////////////////////////////////////////*/

__int64 FXAsyncFile::GetLargeFileLength()
{
    LARGE_INTEGER liFileSize = {0};
    BOOL bRet = GetFileSizeEx(m_hAsyncFile,&liFileSize);
    if( FALSE != bRet )
    {
        return liFileSize.QuadPart;
    }
    return -1;
}

/*//////////////////////////////////////////////////////////////////////////////////////
Wrapper for the SetFilePointer() API. Moves the file pointer to the specified offset.
///////////////////////////////////////////////////////////////////////////////////////*/

BOOL FXAsyncFile::SeekFile(__int64 nBytesToSeek,__int64& nNewOffset,DWORD dwSeekOption)
{
    LARGE_INTEGER liBytesToSeek = {0};
    liBytesToSeek.QuadPart = nBytesToSeek;
    LARGE_INTEGER liNewOffset   = {0};
    BOOL bRet = SetFilePointerEx( m_hAsyncFile,liBytesToSeek,&liNewOffset,dwSeekOption);
    if( bRet )
    {
        nNewOffset = liNewOffset.QuadPart;
        return TRUE;
    }
    FXString ErrMsg = FxLastErr2Mes() ;
    ASSERT( 0 ) ;
    return FALSE;
}

/*//////////////////////////////////////////////////////////////////////////////////////
The destructor. Obiviously, clears all the members.
///////////////////////////////////////////////////////////////////////////////////////*/

FXAsyncFile::~FXAsyncFile(void)
{
    Cleanup(TRUE);
    delete m_pOvpBufArray;
    m_pOvpBufArray = 0;
    ::SetEvent (m_hIOCompleteEvent);
    CloseHandle(m_hIOCompleteEvent);
    m_hIOCompleteEvent = 0;
    if(IsOpen())
    {
        CloseHandle(m_hAsyncFile);
    }
    m_hAsyncFile = 0;
}

/*//////////////////////////////////////////////////////////////////////////////////////
Dispatches the window messages to make the UI responsive.
///////////////////////////////////////////////////////////////////////////////////////*/

VOID FXAsyncFile::PumpMessage()
{
    MSG stMsg = {0};
    while( ::PeekMessage(&stMsg,0,0,0,PM_REMOVE))
    {
        TranslateMessage(&stMsg);
        DispatchMessage(&stMsg);
    }
}

/*//////////////////////////////////////////////////////////////////////////////////////
Formats error message from system error code.
///////////////////////////////////////////////////////////////////////////////////////*/

VOID FXAsyncFile::OutputFormattedErrorString(const TCHAR* ptcMsg,DWORD dwErrorCode)
{
    LPTSTR lptszMsgBuf = 0;
    DWORD dwBufLen = 0;
    if( 0!= (dwBufLen=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|
                                    FORMAT_MESSAGE_FROM_SYSTEM|
                                    FORMAT_MESSAGE_IGNORE_INSERTS,
                                    0,dwErrorCode,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                    reinterpret_cast<LPTSTR>(&lptszMsgBuf),0,0)))
    {
        const TCHAR* INFIX = _T("-->");
        dwBufLen +=( DWORD )_tcslen(ptcMsg) + ( DWORD )_tcslen(INFIX)+1;
        TCHAR* ptcErrMsg = new TCHAR[dwBufLen];
        ZeroMemory(ptcErrMsg,sizeof(TCHAR)*dwBufLen);
        _stprintf_s( ptcErrMsg, dwBufLen ,_T("%s%s%s"),ptcMsg,INFIX,lptszMsgBuf);
        TRACE_STATUS(ptcErrMsg);
        LocalFree(lptszMsgBuf);
        delete[] ptcErrMsg;
    }
}

ULONGLONG FXAsyncFile::GetPosition() const
{
  ASSERT( m_hAsyncFile != INVALID_HANDLE_VALUE );
  LARGE_INTEGER liShift = { 0 } ;
  LARGE_INTEGER liPosition ;
  ULONGLONG retV = SetFilePointerEx( m_hAsyncFile , liShift , &liPosition , FILE_CURRENT ) ;
  if ( !retV )
    FxSendLogMsg( 7 , "FXAsyncFile::GetPosition" , 0 , "Can't get async file position, h=%p" , m_hAsyncFile ) ;
  return (retV) ? liPosition.QuadPart : -1  ;
}
