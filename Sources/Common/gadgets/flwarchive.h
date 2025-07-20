// FLWArchive.h: interface for the CFLWArchive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLWARCHIVE_H__4577A9CA_2967_42FB_BC85_7F25F35F4C4C__INCLUDED_)
#define AFX_FLWARCHIVE_H__4577A9CA_2967_42FB_BC85_7F25F35F4C4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataChain.h"
#include "fxfc/FXBufferManager.h"
#include "fxfc/FXAsyncFile.h"

#define NEXT_FRAME		(-1)
#define FRAME_BY_TIME	(-2)

#define DELTA_INDEX	5          // step of index array size increase
#define INDEX_SAVE_PERIOD	(10) // per how many frames the frame offset will be saved in index table

#define INITIAL_READ_BUFFER_SIZE (8 * 1024 * 1024)

#define CLUSTER_SIZE (m_pAsyncFile->GetClusterSizeInBytes())

typedef struct _tagFLWHeader
{
  unsigned uLabel;
  unsigned uStructSize;
  unsigned uStreamsCount;
  unsigned uFramesCnt;
  FSIZE	 uIndexOffset;
  unsigned reserved0;
  double   fMaxTime;
} FLWHeader;

typedef struct _tagFLWIndexEntry
{
  FSIZE    uFrameOffset;
  double   fFrameTime;
}FLWIndexEntry;


#pragma warning (disable: 4200)

typedef struct _tagFLWIndex
{
  unsigned uMaxFrames;
  unsigned uFramesCount;
  FLWIndexEntry iEntry[ 1 ];
}FLWIndex;

enum ShortOrLong
{
  SOL_Unknown = 0 ,
  SOL_Short = 1 , // m_ShortBufferManager used
  SOL_Long = 2    // m_LongBufferManager used (long buffers 8-10MB)
};
typedef struct _tagAsyncIOParam
{
  ShortOrLong m_UsedBufManager ;
  int         m_iBufferIndex ;
} AsyncIOParam;

#pragma warning (default: 4200)


class FX_EXT_GADGET CFLWArchive : public CDataChain, IAsyncOperationCompletionNotifier
{
  FXFile    m_File;
  FXAsyncFile * m_pAsyncFile ;
  FXString  m_FileName;
  bool      m_OpenWrite;
  bool      m_OpenRead;
  FLWHeader m_FLWHeader;
  double    m_StartTime;
  double    m_dLastSentGraphFrameTime_ms ; // In graph time units
  double    m_dLastSentArchiveFrameTime_ms ; // In graph time units
  double    m_dArchiveMinTime_ms ;
  double    m_LastOperationTime;
  bool      m_AutoRewind;
  unsigned  m_CrntFrame;
  FXTimer   m_Timer;
  FLWIndex* m_pIndex;
  bool      m_bSkippedEOS;
  unsigned  m_FixedDelay;			// interval in ms for regular framerate
  bool      m_bFixedDelay;		// play with regular framerate
  bool      m_bForceNextFrame;	// true if external timer is used
  bool      m_bAutoRewindToStart;
  bool      m_bAsyncFileWrite = true ;
  bool      m_bAsyncFileRead = true ;
  bool      m_bWaitForFileOperationFinished = false ;
  FXLockObject m_Lock;

#ifndef SHBASE_CLI
  std::thread * m_pSerializationThread = NULL ;
  queue<const CDataFrame*> m_DataForWriting ; // frames should be serialized and written
  queue<CDataFrame*>       m_DataForSending ; // frames from archive, they should be sent
  std::mutex  m_DataAccessMutex ;
  std::mutex  m_ReadingMutex ;
  HANDLE      m_hDataAvailable = NULL ;
  HANDLE      m_hAllertEvent = NULL ;
  BOOL        m_bLogInSeparateThread = FALSE ;
  bool        m_bCloseThread ;
  bool        m_bDisable = false ; 
    // Common for read-write
  __int64     m_i64CurrentPositionInFile ;
    // For write archive
  int         m_iSerializationByThreadMode ;
  FXBufferManager m_LongBufferManager ;
  FXBufferManager m_ShortBufferManager ;
  LPBYTE      m_pLongBuffer = NULL ;
  int         m_iLongBufferFilledLen ;
  int         m_iLastLongBufferIndex = -1 ;
  LPBYTE      m_pShortBuffer = NULL ;
  int         m_iShortBufferFilledLen ;
  int         m_iLastShortBufferIndex = -1 ;
  __int64     m_i64NWrittenClusters ;
    // archive reading ;
  BYTE        m_ShortBuf[ 16 * 1024 ] ;
  LPBYTE      m_pReadBuffer = NULL ;
  DWORD       m_dwReadBufferLen = 0 ;
  LPBYTE      m_TmpBuffer = NULL ;
  DWORD       m_dwTmpBufferLen = 0 ;
  __int64     m_i64FileLength = 0 ;
  __int64     m_i64FLWDataLen = 0 ;
  __int64     m_i64BufferOriginPositionInFile = -1 ; // Position of first byte from file in read buffer
                                              // should be multiple of 4096
  __int64     m_i64FirstFrameOrigin ;
  __int64     m_i64LastFrameOrigin ;
  __int64     m_i64ActiveLength ; 

  DWORD       m_dwInBufferIndex ;
  DWORD       m_dwTheRestInBuffer ;
  DWORD       m_dwInBufferFLWDataLen ;
  DWORD       m_dwNRestoredFrames ;
  DWORD       m_dwNTakenFrames ;
  DWORD       m_dwNextFrame ; // NEXT_FRAME - next frame, FRAME_BY_TIME - find first frame after m_dNextFrameTime
  double      m_dNextFrameTime ; // Time for FRAME_BY_TIME

#endif 
public: // diagnostics info
  double      m_dVFrameSaveTime_ms , m_dVFrameSaveTimeMin_ms , m_dVFrameSaveTimeMax_ms ;
  double      m_dTimeOfAquireTimeMax ;
  double      m_dSerializationTime_ms , m_dSerializationTimeMin_ms , m_dSerializationTimeMax_ms ;
  double      m_dTimeOfSerializationTimeMax ;
  int         m_iNRequestedWrites , m_iNCompletedWrights ;
  DWORD       m_dwLastError ;
  __int64     m_i64NWritten ;

  int         m_iPlayNumberMin = 0 ;
  int         m_iPlayNumberMax = 100 ;
  double      m_dPlayTimeMin_ms = 0. ;
  double      m_dPlayTimeMax_ms = 1000. ;
  double      m_dTimeZoom = 1.0 ;
  BOOL        m_bIterateByFrames = TRUE ;
  BOOL        m_bIterateByTime = FALSE ;
  BOOL        m_bRecalculate = FALSE ;
  int         m_iFirstFrame = 0 ;
  int         m_iLastFrame = 0 ;
  BOOL        m_bFirstIsReached = FALSE ;
  BOOL        m_bLastIsReached = FALSE ;

  vector<__int64> m_InFilePositions ;

public:
  CFLWArchive( CGadget* host );
  ~CFLWArchive();
  bool    OpenRead( LPCTSTR fName ) ;
  __int64 GetFirstFrameOffset() ; // returns first frame offset in file, -1 if fault
  bool    OpenWrite( LPCTSTR fName );
  void    Close();
  bool    WriteFrame( const CDataFrame* df );
  CDataFrame* ReadFrame( unsigned nmb = NEXT_FRAME , double time = 0 );
  unsigned GetFramesNmb() { return m_FLWHeader.uFramesCnt; }
  double  GetMaxTime() { return m_FLWHeader.fMaxTime; }
  double  GetMinTime() { return m_dArchiveMinTime_ms ; }
  int     GetFrameIndexInTable() ;
  int     GetFrameIndexInTable( unsigned pos ) ;
  int     GetFrameIndexInTable( double time ) ;
  bool    Seek( unsigned pos );
  bool	  Seek( double time );
  bool    Seek( __int64 i64Offset , UINT From , __int64 * pi64AbsOffset = NULL ) ;
  void    Rewind();
  void    ResetInputBuffers() ;
  bool    IsEOF();
  bool    IsOpen();
  void    SkipDelay() { m_Timer.ResetPause(); }
  void  	LoadIndex();
  void  	SaveIndex();
  void  	UpdateIndex( FSIZE pos , double tm );
  bool  	IsAutoRewindOn() { return m_AutoRewind; }
  void  	SetAutoRewind( bool bOn ) { m_AutoRewind = bOn; };
  bool  	GetFixedDelay( unsigned& delay ) { delay = m_FixedDelay; return m_bFixedDelay; };
  void  	SetFixedDelay( bool bOn , unsigned delay ) { m_bFixedDelay = bOn; m_FixedDelay = delay; };
  bool    GetAutoRewindToStart() { return m_bAutoRewindToStart; }
  void    SetAutoRewindToStart( bool rewind ) { m_bAutoRewindToStart = rewind; }
  bool  	IsForceNextFrameOn() { return m_bForceNextFrame; };
  void  	SetForceNextFrame( bool bOn ) { m_bForceNextFrame = bOn; };
  LPCTSTR GetFileName() { return m_FileName; }
  double  GetLastOperationTime() { return m_LastOperationTime; }
  LONGLONG GetPosition() ;
  int     GetNBusyBuffers() ;
  BOOL    GetNewShortBuffer() ;

  virtual void OnAsyncOperationComplete( BOOL bRead , DWORD dwErrorCode , LPVOID pParams ) ;

#ifndef SHBASE_CLI
  static void SerializationThreadFunc( CFLWArchive * pFile ) ;
  void        SerThreadFunc() ;
  size_t      GetSerializationQueueLength() { return m_DataForWriting.size() ; }
  size_t      GetWritingQueueLength() { return m_File.GetWritingQueueLength() ; }
  size_t      GetAllocatedForWrite() { return m_File.m_NWriteStorageBytes ; }
  int         SendBufferToDiskAndSaveTheRest( 
                LPBYTE pBuffer , int iLen , int iBufferIndex , FXBufferManager& Manager ) ;
  int         CheckAndReadToBuffer() ;
  int         ReadToBuffer( LPBYTE pBuf , DWORD dwLen , DWORD& dwNTransferedBytes , LPCTSTR Msg = NULL ) ;
  int         ReadToBuffer( int iLength ) ;

     // if one of params is zero - will be sync mode
  virtual bool SetAsyncWriteMode( FXSIZE BufferLen = 0 , int iNBuffers = 0 ) ;
  virtual bool SetAsyncReadMode() ;
  size_t      m_NWrittenFrames ; // For write mode. Containers are accounted as one frame
  size_t      m_NReadFrames ;    // For read mode. Containers are accounted as one frame
  double      m_dLastDiagnosticMsgTime ;
  virtual int CorrectBuffer() ;

#endif

};

#endif // !defined(AFX_FLWARCHIVE_H__4577A9CA_2967_42FB_BC85_7F25F35F4C4C__INCLUDED_)
