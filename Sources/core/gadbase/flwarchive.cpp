// FLWArchive.cpp: implementation of the CFLWArchive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\FLWArchive.h>
#include <math/Intf_sup.h>
#include <gadgets/videoframe.h>
#include <fxfc/FXAsyncFile.h>
#include <fstream>

#define THIS_MODULENAME "CFLWArchive"

__forceinline void _init_flwheader( FLWHeader& hdr )
{
  hdr.uLabel = ('f' | ('l' << 8) | ('w' << 16));
  hdr.uStructSize = sizeof( FLWHeader );
  hdr.uStreamsCount = 0 ;
  hdr.uFramesCnt = 0;
  hdr.uIndexOffset = 0; // No index
  hdr.reserved0 = 0;
  hdr.fMaxTime = 0;
}

__forceinline void _init_flwindex( FLWIndex*& index )
{
  DWORD size = sizeof( FLWIndex ) + DELTA_INDEX * sizeof( FLWIndexEntry );
  index = (FLWIndex*) realloc( index , size );
  ::ZeroMemory( index , size );
  index->uMaxFrames = DELTA_INDEX;
}

inline BOOL AnalyzeTiming( double& dValueNow , double& dMin , double& dMax ,
  double dTimeNow , double& dTimeOfMax )
{
  SetMinMax( dValueNow , dMin , dMax );
  if ( dValueNow == dMax )
  {
    dTimeOfMax = dTimeNow ;
    return TRUE ;
  }
  else if ( dTimeNow - dTimeOfMax > 3000. )
  {
    dTimeOfMax = dTimeNow ;
    dMax = dValueNow ;
  }
  return FALSE ;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFLWArchive::CFLWArchive( CGadget* host ) :
  CDataChain( host ) ,
  m_OpenWrite( false ) ,
  m_OpenRead( false ) ,
  m_AutoRewind( true ) ,
  m_CrntFrame( 0 ) ,
  m_dwNTakenFrames( 0 ) ,
  m_Timer( _T( "FLWArchive" ) ) ,
  m_pIndex( NULL ) ,
  m_bSkippedEOS( false ) ,
  m_bFixedDelay( false ) ,
  m_FixedDelay( 40 ) ,
  m_bForceNextFrame( false ) ,
  m_bAutoRewindToStart( true ) ,
  m_LastOperationTime( 0.0 ) ,
  m_pAsyncFile( NULL )
{
  _init_flwheader( m_FLWHeader );
  _init_flwindex( m_pIndex );
}

CFLWArchive::~CFLWArchive()
{
  SetAsyncWriteMode( 0 , 0 ) ;
  if ( m_pAsyncFile )
  {
    delete m_pAsyncFile ;
    m_pAsyncFile = NULL ;
  }
  Close();
  free( m_pIndex );
}

void CFLWArchive::SerializationThreadFunc( CFLWArchive * pArchive )
{
#ifndef SHBASE_CLI
  pArchive->SerThreadFunc() ;
#endif
}
int CFLWArchive::SendBufferToDiskAndSaveTheRest( LPBYTE pBuffer , int iLen , int iBufferIndex ,
  FXBufferManager& Manager )
{
  int iNClusters = iLen / m_iClasterSize ;
  int iWriteLen = iNClusters * m_iClasterSize ;
  int iTheRest = iLen - iWriteLen ;
  LPBYTE pNewBuffer = NULL ;
  int iNewBufferIndex = -1 ;
  if ( iTheRest )
  {
    iNewBufferIndex = m_ShortBufferManager.GetBuffer() ;
    if ( iNewBufferIndex >= 0 )
    {
      pNewBuffer = m_ShortBufferManager.GetBufferPtr( iNewBufferIndex ) ;
      memcpy( pNewBuffer , pBuffer + iWriteLen , iTheRest ) ;
    }
  }
  if ( iWriteLen )
  {
    bool bLongBuffer = ( &Manager == &m_LongBufferManager ) ;
    AsyncIOParam BufManagerData = 
      { bLongBuffer ?  SOL_Long : SOL_Short , iBufferIndex } ;
    if ( !m_pAsyncFile->Write( pBuffer , iWriteLen ,
      0xffffffff , 0xffffffff , *( ( LARGE_INTEGER* ) &BufManagerData ) ) )
    {
      SENDERR( "Can't write long buffer to file. L=%d" , iWriteLen ) ;
      Manager.ReleaseBuffer( iBufferIndex ) ;
      if ( bLongBuffer )
      {
        m_pLongBuffer = NULL ;
        m_iLongBufferFilledLen = 0 ;
      }
      else
      {
        m_pShortBuffer = NULL ;
        m_iShortBufferFilledLen = 0 ;
      }
      return 0 ;
    }
    else
    {
      m_iNRequestedWrites++ ;
      m_pShortBuffer = pNewBuffer ; // The Rest always goes to short buffer
      m_iShortBufferFilledLen = iTheRest ;
      m_iLastShortBufferIndex = ( iTheRest ) ? iNewBufferIndex : -1 ;
      m_pLongBuffer = NULL ;
    }
  }
  return iNClusters ;
}

BOOL CFLWArchive::GetNewShortBuffer()
{
  int iShortBufferIndex = m_ShortBufferManager.GetBuffer() ;
  if ( iShortBufferIndex >= 0 )
  {
    m_pShortBuffer = m_ShortBufferManager.GetBufferPtr( iShortBufferIndex ) ;
    m_iLastShortBufferIndex = iShortBufferIndex ;
    m_iShortBufferFilledLen = 0 ;
    return TRUE ;
  }
  else
    SENDERR( "No free short buffers when no old data" ) ;
  return FALSE ;
}

void CFLWArchive::SerThreadFunc()
{
#ifndef SHBASE_CLI
  m_bCloseThread = false ;
  SetCurrentThreadName( "CFLWArchive::SerializationThreadFunc" ) ;
  //SetPriorityClass( GetCurrentProcess() , HIGH_PRIORITY_CLASS ) ;
  //SetPriorityClass( GetCurrentProcess() , REALTIME_PRIORITY_CLASS ) ;
  //SetThreadPriority( GetCurrentThread() , THREAD_PRIORITY_ABOVE_NORMAL /*THREAD_PRIORITY_TIME_CRITICAL*/ ) ;
  double dTimeStamp = GetHRTickCount() ;
  while ( !m_bCloseThread )
  {
    while ( m_OpenWrite && !m_DataForWriting.empty() && !m_bCloseThread )
    {
      double dStart = GetHRTickCount() ;
      m_DataAccessMutex.lock() ;
      const CDataFrame * df = m_DataForWriting.front() ;
      m_DataForWriting.pop();
      m_DataAccessMutex.unlock() ;

      if ( !df )
        continue ;
      double tm = df->GetTime() ;
      FSIZE pos = m_i64CurrentPositionInFile ;//GetPosition();

      FLWDataFrame* dt = NULL ;
      LPBYTE pBuffer = NULL ;
      bool bWritten = false ;
      if ( m_pAsyncFile )
      {
        int iBufferIndex = -1 ;
        FXSIZE LabelLen , AttribLen ;
        int iSerializeLen = GetNecessaryBufferSize( df , LabelLen , &AttribLen ) ;
        if ( m_pLongBuffer ) // should not be
        {
          ASSERT( 0 ) ;
        }
        bool bShortBufferAllocated = ( m_pShortBuffer != NULL ) ;
        if ( !bShortBufferAllocated )
        {
          if ( iSerializeLen < m_ShortBufferManager.GetBufferSize() )
          {
            int iShortBufferIndex = m_ShortBufferManager.GetBuffer() ;
            if ( iShortBufferIndex >= 0 )
            {
              m_pShortBuffer = m_ShortBufferManager.GetBufferPtr( iShortBufferIndex ) ;
              m_iLastShortBufferIndex = iShortBufferIndex ;
              m_iShortBufferFilledLen = 0 ;
            }
            else
            {
              SENDERR( "No free short buffers when no old data" ) ;
            }
          }
          else // Short buffer is not enough
          {
            int iLongBufferIndex = m_LongBufferManager.GetBuffer() ;
            if ( iLongBufferIndex >= 0 )
            {
              m_pLongBuffer = m_LongBufferManager.GetBufferPtr( iLongBufferIndex ) ;
              m_iLastLongBufferIndex = iLongBufferIndex ;
              m_iLongBufferFilledLen = 0 ;
            }
            else
            {
              SENDERR( "No free long buffers when no old data" ) ;
            }
          }
        }
        if ( m_pShortBuffer )
        {
          if ( iSerializeLen < ( m_ShortBufferManager.GetBufferSize() - m_iShortBufferFilledLen ) )
          {
            __int64 i64InBufIndex = m_iShortBufferFilledLen ;
            dt = Serialize( df , m_pShortBuffer , i64InBufIndex , 
              m_ShortBufferManager.GetBufferSize() - m_iShortBufferFilledLen ) ;
            if ( dt ) // serializing is OK
            {
              m_iShortBufferFilledLen = ( int ) i64InBufIndex ;
              if ( m_iShortBufferFilledLen >= m_iClasterSize )
              {
                int iNClusters = SendBufferToDiskAndSaveTheRest( m_pShortBuffer ,
                  m_iShortBufferFilledLen , m_iLastShortBufferIndex , m_ShortBufferManager ) ;
                if ( !iNClusters )
                {
                  SENDERR( "No written clasters from short buffer. L=%d" , m_iShortBufferFilledLen ) ;
                }
                else
                {
                  m_i64NWrittenClusters += iNClusters ;
                  bWritten = true;
                }
              }
              else
                bWritten = true;
              m_i64CurrentPositionInFile = ( m_i64NWrittenClusters * m_iClasterSize ) + m_iShortBufferFilledLen ;
            }
            else // too small length, simply recalculate in file position
            {
              m_i64CurrentPositionInFile = ( m_i64NWrittenClusters * m_iClasterSize ) + m_iShortBufferFilledLen ;
            }
          }
          else // not enough short buffer length
          {
            ASSERT( m_pLongBuffer == NULL ) ;
            if ( iSerializeLen <= m_LongBufferManager.GetBufferSize() - m_iShortBufferFilledLen )
            {
              int iNewLongBufferIndex = m_LongBufferManager.GetBuffer() ;
              if ( iNewLongBufferIndex >= 0 )
              {
                m_pLongBuffer = m_LongBufferManager.GetBufferPtr( iNewLongBufferIndex ) ;
                if ( m_iShortBufferFilledLen ) // copy old data
                  memcpy( m_pLongBuffer , m_pShortBuffer , m_iShortBufferFilledLen ) ;
                m_iLongBufferFilledLen = m_iShortBufferFilledLen ;
                m_iLastLongBufferIndex = iNewLongBufferIndex ;
                m_ShortBufferManager.ReleaseBuffer( m_iLastShortBufferIndex ) ;
                m_pShortBuffer = NULL ;
              }
              else
              {
                SENDERR( "No free long buffers" ) ;
              }
            }
            else
            {
              SENDERR( "Not enough long buffer length" ) ;
            }
          }
        }
        if ( m_pLongBuffer ) // long buffer was allocated for this df
        {
          __int64 i64InBufIndex = m_iLongBufferFilledLen ;
          dt = Serialize( df , m_pLongBuffer , i64InBufIndex ,
            m_LongBufferManager.GetBufferSize() ) ;
          if ( dt )
          {
            m_iLongBufferFilledLen = ( int ) i64InBufIndex ;
            if ( m_iLongBufferFilledLen >= m_iClasterSize )
            {
              int iNClusters = SendBufferToDiskAndSaveTheRest( m_pLongBuffer ,
                m_iLongBufferFilledLen , m_iLastLongBufferIndex , m_LongBufferManager ) ;
              if ( !iNClusters )
              {
                SENDERR( "No written clasters from long buffer. L=%d" , m_iLongBufferFilledLen ) ;
              }
              else
              {
                m_i64NWrittenClusters += iNClusters ;
                bWritten = true;
              }
            }
            else
              bWritten = true;
            m_i64CurrentPositionInFile = ( m_i64NWrittenClusters * m_iClasterSize ) + m_iShortBufferFilledLen ;
          }
          else // should not be, but... too small length, simply recalculate in file position
          {
            m_i64CurrentPositionInFile = ( m_i64NWrittenClusters * m_iClasterSize ) + m_iShortBufferFilledLen ;
          }
        }
      }
      else
      {
        dt = Serialize( df );
        if ( dt && m_File.m_pFile )
        {
          m_File.Write( dt , ( size_t ) dt->uSize );
          free( dt );
        }
        else 
        {
          SENDERR( "Can't serialize %s frame" ,
            df->IsContainer() ? _T("Container") : (LPCTSTR)Tvdb400_TypeToStr( df->GetDataType() ) ) ;
        }
      }

      ( ( CDataFrame* ) df )->Release() ;

      if ( m_FLWHeader.fMaxTime < tm )
        m_FLWHeader.fMaxTime = tm;
      if ( bWritten )
      {
        if ( !( m_FLWHeader.uFramesCnt % INDEX_SAVE_PERIOD ) )
        {
          UpdateIndex( pos , tm );
        }
        m_FLWHeader.uFramesCnt++ ;
        m_NWrittenFrames++ ;
      }

      m_dSerializationTime_ms = GetHRTickCount() - dStart ;
      AnalyzeTiming( m_dSerializationTime_ms ,
        m_dSerializationTimeMin_ms , m_dSerializationTimeMax_ms ,
        dStart , m_dTimeOfSerializationTimeMax ) ;
    }
    while ( m_OpenRead && !m_bCloseThread )
    {
      if ( m_bRecalculate )
      {
        ResetInputBuffers() ;
        int iInIndexTable = GetFrameIndexInTable() ;
        m_dwNRestoredFrames = iInIndexTable * INDEX_SAVE_PERIOD ;
      }

      bool bContinueRead = ( m_dwNRestoredFrames < (DWORD)m_iPlayNumberMax ) ;
      double dFirstTime = 0. , dLastTime = 0. ;
      if ( bContinueRead )
      {
        m_DataAccessMutex.lock() ;
        if ( m_DataForSending.size() >= 2 )
        {
          dFirstTime = m_DataForSending.front()->GetTime() ;
          dLastTime = m_DataForSending.back()->GetTime() ;
          if ( dLastTime - dFirstTime > 100. ) // saved frames for 100ms of simulation
            bContinueRead = false ;  // <== there is why we have twice if ( bContinueRead )
        }
        m_DataAccessMutex.unlock() ;
      }
      if ( bContinueRead )
      {
        CDataFrame * pNewFrame = NULL ;
        if ( m_i64CurrentPositionInFile >= m_i64FLWDataLen )
          m_bRecalculate = true ;
        while ( bContinueRead )
        {
          m_ReadingMutex.lock() ;
          bool bRecalculate = m_bRecalculate ;
          if ( CheckAndReadToBuffer() < 0 )
          {
            Sleep( 3 ) ;
            m_ReadingMutex.unlock() ;
            continue ;
          }

          FLWDataFrame* dt = ( FLWDataFrame* ) ( m_pReadBuffer + m_dwInBufferIndex ) ;
          if ( dt->uLabel != HUNK )
          {
            SENDERR_1( "Bad Frame label in archive file %s" , m_FileName );
            m_ReadingMutex.unlock() ;
            continue ;
          }
          bool firstframe = ( m_dwNRestoredFrames == m_iPlayNumberMin );
          m_CrntFrame = dt->dwID;
          if ( !m_bFirstIsReached )
          {
            if ( m_bIterateByFrames )
            {
              if ( m_iFirstFrame == m_iPlayNumberMin ) // OK, calculate first frame offset from file beginning
              {
                m_i64FirstFrameOrigin = m_i64BufferOriginPositionInFile + m_dwInBufferIndex ;
                m_bFirstIsReached = TRUE ;
              }
              else 
              {
                m_iFirstFrame++ ;
                m_dwNRestoredFrames++ ;
                CorrectBuffer() ;
                m_ReadingMutex.unlock() ;
                continue ;  // read next frame
              }
            }
            else if ( m_bIterateByTime )
            {
              if ( dt->dWriteTime >= m_dPlayTimeMin_ms )
              {
                m_i64FirstFrameOrigin = m_i64BufferOriginPositionInFile + m_dwInBufferIndex ;
                m_bFirstIsReached = TRUE ;
              }
              else
              {
                m_iFirstFrame++ ;
                m_dwNRestoredFrames++ ;
                CorrectBuffer() ;
                m_ReadingMutex.unlock() ;
                continue ;  // read next frame
              }
            }
          }
          pNewFrame = Restore( dt );
          m_LastOperationTime = GetHRTickCount() - dTimeStamp ;
          if ( pNewFrame )
          {
            if ( bRecalculate )
            {
              dFirstTime = pNewFrame->GetTime() ;
              m_InFilePositions.clear() ;
            }


            if ( !m_bLastIsReached )
            {
              if ( m_bIterateByFrames )
              {
                if ( m_dwNRestoredFrames == m_iPlayNumberMax )
                {
                  m_i64LastFrameOrigin = m_i64BufferOriginPositionInFile + m_dwInBufferIndex ;
                  m_bLastIsReached = TRUE ;
                }
              }
              else if ( m_bIterateByTime )
              {
                if ( dt->dWriteTime >= m_dPlayTimeMin_ms )
                {
                  m_i64LastFrameOrigin = m_i64BufferOriginPositionInFile + m_dwInBufferIndex ;
                  m_bLastIsReached = TRUE ;
                }
              }
            }
            m_DataAccessMutex.lock() ;
            m_DataForSending.push( pNewFrame ) ;
            if ( !bRecalculate )
              m_dwNRestoredFrames++ ;
            m_DataAccessMutex.unlock() ;
            dLastTime = pNewFrame->GetTime() ;
            if ( ((dLastTime - dFirstTime) > 100.)  // saved frames for 100ms of simulation
              || m_dwNRestoredFrames >= (UINT)m_iPlayNumberMax )
              bContinueRead = false ;  // <== there is why we have twice if ( bContinueRead )
            //m_InFilePositions.push_back( m_i64CurrentPositionInFile ) ;
          }

          int iMoveLen = CorrectBuffer() ;
  //         if ( m_bAutoRewindToStart && firstframe )
  //           m_StartTime -= dt->dTime /** 1000.*/ ;

          m_ReadingMutex.unlock() ;
        }
      }
      else
      {
        if ( m_bAutoRewindToStart && ( m_DataForSending.size() == 0 )
          && ( m_dwNRestoredFrames >= (DWORD)m_iPlayNumberMax ) )
        {
          Rewind() ;
        }
        else
          break ;
      }
    }
    if ( m_hDataAvailable )
    {
      WaitForSingleObjectEx( m_hDataAvailable , 100 , TRUE ) ;
      if ( m_bWaitForFileOperationFinished && m_hAllertEvent )
        SetEvent( m_hAllertEvent ) ;
    }
    else
      Sleep( 3 ) ;
  }
#endif
}
bool    CFLWArchive::OpenRead( LPCTSTR fName )
{
  FXAutolock al( m_Lock );
  if ( m_OpenRead || m_OpenWrite )
  {
    Close();
  }

  FXFileException e;

  m_FileName = fName;
  if ( !m_File.Open( m_FileName , FXFile::modeRead | FXFile::shareDenyNone , &e ) )
  {
    m_FileName.Empty();
    return false;
  }
  if ( m_File.Read( &m_FLWHeader , sizeof( m_FLWHeader ) ) != sizeof( m_FLWHeader ) )
  {
    m_File.Close();
    m_FileName.Empty();
    return false;
  }
  LoadIndex();

  if ( m_bAsyncFileRead )
  {
    m_File.Close() ;
    m_pAsyncFile = new FXAsyncFile( m_FileName , FALSE ,
      GENERIC_READ , FILE_SHARE_READ , NULL , // no notifier
      FALSE , 0 , FALSE ) ;
    if ( !m_pAsyncFile->IsOpen() )
    {
      delete m_pAsyncFile ;
      return false ;
    }
    DWORD dwHighPart = 0 ;
    DWORD dwFileLen = m_pAsyncFile->GetFileLength( &dwHighPart ) ;
    m_i64FileLength = (__int64) dwFileLen + (( __int64 ) ( dwHighPart ) << 32) ;
    m_i64FLWDataLen = m_i64FileLength - ( m_pIndex->uFramesCount * sizeof( FLWIndexEntry ) ) ;
    m_i64BufferOriginPositionInFile = -1 ; // no read
    m_bRecalculate = TRUE ;
    m_dwNRestoredFrames = 0 ;
    SetAsyncReadMode() ;
  }
  m_OpenRead = true;
  m_StartTime = (m_HostGadget->GetGraphTime() * 1.e-3) ;
  return true;
}

int CFLWArchive::ReadToBuffer( int iLength )
{
  LPBYTE pReadArea = m_pReadBuffer ;
  int iReadLength = m_dwReadBufferLen ;
  if ( m_dwTheRestInBuffer ) // some old data are in buffer
  {   // the first block with cluster size will be filled with last cluster
    DWORD dwLastClusterBeginning = ( m_dwInBufferIndex / CLUSTER_SIZE )
      * CLUSTER_SIZE ;
      // we do copy the last cluster to the buffer beginning
    memcpy( m_pReadBuffer , m_pReadBuffer + dwLastClusterBeginning , CLUSTER_SIZE ) ;
      // and reading will be done from next cluster size block
    pReadArea += CLUSTER_SIZE ;
    iReadLength -= CLUSTER_SIZE ;
  }
  DWORD dwNTransferedBytes = 0 ;
  if ( !m_pAsyncFile->ReadSync( pReadArea , iReadLength , dwNTransferedBytes ) )
  {
    SENDERR( "SerThreadFunc READ: can't read %u bytes from pos=%li file %s" ,
      iReadLength , m_i64CurrentPositionInFile , ( LPCTSTR ) m_pAsyncFile->GetFilePath() ) ;
    ASSERT( 0 ) ;
    return -1 ; // can't read
  };
  if ( iReadLength != dwNTransferedBytes )
  {
    SENDINFO( "SerThreadFunc READ: read %u bytes instead of %u from pos=%li file %s" ,
      iReadLength , dwNTransferedBytes , m_i64CurrentPositionInFile , ( LPCTSTR ) m_pAsyncFile->GetFilePath() ) ;
  }
  m_dwTheRestInBuffer += dwNTransferedBytes ;
  m_dwInBufferFLWDataLen = iReadLength + (int)( pReadArea - m_pReadBuffer ) ;
  m_dwInBufferIndex = m_dwInBufferFLWDataLen - m_dwTheRestInBuffer ;
  m_i64BufferOriginPositionInFile += iReadLength ;
  return iReadLength ;
}

int CFLWArchive::ReadToBuffer( LPBYTE pBuf , DWORD dwLen , DWORD& dwNTransferedBytes , LPCTSTR pMsg )
{
  if ( !m_pAsyncFile->ReadSync( pBuf , dwLen , dwNTransferedBytes ) )
  {
    SENDERR( "ReadToBuffer : %s Read ERROR of %d bytes from file %s" ,
      pMsg , dwLen , ( LPCTSTR ) m_pAsyncFile->GetFilePath() ) ;
    return -1 ; // can't read
  };
//   if ( dwLen != dwNTransferedBytes )
//   {
//     SENDINFO( "ReadToBuffer : %s , read %u bytes instead of %u from file %s" ,
//       pMsg , dwLen , dwNTransferedBytes , ( LPCTSTR ) m_pAsyncFile->GetFilePath() ) ;
//     ASSERT( 0 ) ;
//     return -2 ;
//   }
  return (int)dwNTransferedBytes ;
}

int CFLWArchive::GetFrameIndexInTable( unsigned uPos )
{
  if ( m_OpenWrite || !m_OpenRead || !m_pIndex->uFramesCount )
    return -1 ;

  int iIndex = uPos / INDEX_SAVE_PERIOD ;
  if ( iIndex >= (int)m_pIndex->uFramesCount )
    return -1 ;

  return iIndex ;
}

int CFLWArchive::GetFrameIndexInTable( double time )
{
  if ( m_OpenWrite || !m_OpenRead || !m_pIndex->uFramesCount )
    return -1 ;

  DWORD i;
  for ( i = 0; i < m_pIndex->uFramesCount; i++ )
  {
    if ( m_pIndex->iEntry[ i ].fFrameTime > time )
    {
      int iIndex = ( i == 0 ) ? 0 : i - 1 ;
      return iIndex ;
    }
  }
  return -1 ;
}

int CFLWArchive::GetFrameIndexInTable()
{
  int iInIndexTable = m_bIterateByFrames ? GetFrameIndexInTable( ( unsigned ) m_iPlayNumberMin )
    : m_bIterateByTime ? GetFrameIndexInTable( m_dPlayTimeMin_ms ) : -1 ;
  
  return iInIndexTable ;
}

// returns first frame offset in file, -1 if fault
// m_iFirstFrame will be set to maximal frame number multiple of INDEX_SAVE_PERIOD which is less or equal to ordered
// After this function on the restore process may be necessary to pass several frames until ordered
// frame number or time
__int64 CFLWArchive::GetFirstFrameOffset() 
{
  int iInIndexTable = GetFrameIndexInTable() ;

  if ( iInIndexTable >= 0 )
  {
    m_iFirstFrame = iInIndexTable * INDEX_SAVE_PERIOD ;
    return (__int64)(m_pIndex->iEntry[ iInIndexTable ].uFrameOffset) ;
  }
  return -1 ;
}

// !!!!!!! Should be called under m_ReadingMutex.lock() ;

int CFLWArchive::CheckAndReadToBuffer()
{
  int iResultLen = 0 ; // no read operation performed
  if ( (m_i64BufferOriginPositionInFile == -1) || m_bRecalculate )
  {
    if ( !m_pReadBuffer )
    {
      m_pReadBuffer = new BYTE[ 8 * 1024 * 1024 ] ;
      m_dwReadBufferLen = 8 * 1024 * 1024 ;
    }
    if ( m_bRecalculate )
    {
      m_i64FirstFrameOrigin = GetFirstFrameOffset() ; // frame number for this origin will be
                                                      // hold in m_iFirstFrame
      m_bRecalculate = m_bFirstIsReached = m_bLastIsReached = FALSE ;
    }
    __int64 i64ClusterBegin = ClusterBegin( m_i64FirstFrameOrigin ) ;
    __int64 iRestLen = m_i64FLWDataLen - i64ClusterBegin ;

    DWORD dwReadDataLen = ( iRestLen < m_dwReadBufferLen ) ?
      (DWORD) NecessarySizeInClusters( iRestLen ) : m_dwReadBufferLen ;
    DWORD dwNTransferedBytes = 0 ;

    __int64 i64OffSet = 0 ;
    m_pAsyncFile->SeekFile( i64ClusterBegin , i64OffSet , FILE_BEGIN ) ;
    if ( ReadToBuffer( m_pReadBuffer , dwReadDataLen , 
      dwNTransferedBytes , "Initial read" ) < 0 ) // fill the buffer
    {
      ASSERT( 0 ) ;
      return -6 ; // can't read
    };
    m_dwInBufferIndex = (DWORD)(m_i64FirstFrameOrigin - i64ClusterBegin) ;
    m_dwInBufferFLWDataLen = dwNTransferedBytes ;
    m_dwTheRestInBuffer = m_dwInBufferFLWDataLen - m_dwInBufferIndex ;
    m_i64CurrentPositionInFile = m_dwInBufferIndex + i64ClusterBegin ;
    m_i64BufferOriginPositionInFile = i64ClusterBegin ;
    m_dwNRestoredFrames = m_iFirstFrame ;
    iResultLen = dwNTransferedBytes ;
    FLWDataFrame * pFLWData = ( FLWDataFrame * ) ( m_pReadBuffer + m_dwInBufferIndex ) ;
    m_dPlayTimeMin_ms = m_dArchiveMinTime_ms = pFLWData->dWriteTime ;
    //m_dPlayTimeMax_ms = m_FLWHeader.fMaxTime ;
//     m_iPlayNumberMin = 0 ;
//     m_iPlayNumberMax = m_FLWHeader.uFramesCnt - 1 ;
//     m_dTimeZoom = 1.0 ;
  }
  else // not first reading
  {
    m_dwInBufferIndex = ( DWORD ) ( m_i64CurrentPositionInFile - m_i64BufferOriginPositionInFile ) ;
    m_dwTheRestInBuffer = m_dwInBufferFLWDataLen - m_dwInBufferIndex ;
    bool bRead = ( m_dwTheRestInBuffer <= sizeof( FLWDataFrame ) ) ;
    DWORD dwNTransferedBytes = 0 ;
    if ( m_dwTheRestInBuffer < FLWFrameSizeVisible ) // we don't know necessary buffer size
    {
      if ( m_dwTheRestInBuffer > 0 )
        memcpy( m_ShortBuf + CLUSTER_SIZE - m_dwTheRestInBuffer , m_pReadBuffer + m_dwInBufferIndex , m_dwTheRestInBuffer ) ;
      if ( ReadToBuffer( m_ShortBuf + CLUSTER_SIZE ,
        CLUSTER_SIZE , dwNTransferedBytes , "Read to SHORT BUF" ) < 0 )
      {
        ASSERT( 0 ) ;
        return -1 ; // can't read
      };

      LPBYTE pFLW = m_ShortBuf + CLUSTER_SIZE - m_dwTheRestInBuffer ;
      FLWDataFrame * pFLWData = ( FLWDataFrame * ) pFLW ;

      DWORD dwNecessarySize = NecessarySizeInClusters( pFLWData->uSize ) + CLUSTER_SIZE;
      if ( m_dwReadBufferLen < dwNecessarySize )
      {
        delete[] m_pReadBuffer ;
        m_pReadBuffer = new BYTE[ dwNecessarySize ] ;
        if ( !m_pReadBuffer )
        {
          SENDINFO( "CheckAndReadToBuffer : Can't reallocate read buffer for %u bytes" ,
            dwNecessarySize ) ;
          ASSERT( 0 ) ;
          return -2 ;
        }
        m_dwReadBufferLen = dwNecessarySize ;
      }
      memcpy( m_pReadBuffer , m_ShortBuf , CLUSTER_SIZE * 2 ) ; // NOTE:
                                                                // if ( m_dwTheRestInBuffer == 0 ) the first cluster is not used
      dwNecessarySize -= CLUSTER_SIZE * 2 ;
      if ( ReadToBuffer( m_pReadBuffer + CLUSTER_SIZE * 2 ,
        dwNecessarySize , dwNTransferedBytes , "Read additional bytes" ) < 0 ) // fill the rest of buffer
      {
        ASSERT( 0 ) ;
        return -3 ; // can't read
      };
      m_dwInBufferFLWDataLen = m_dwReadBufferLen ;
      m_dwInBufferIndex = CLUSTER_SIZE - m_dwTheRestInBuffer ;
      m_dwTheRestInBuffer = m_dwInBufferFLWDataLen - m_dwInBufferIndex ;
      iResultLen = dwNecessarySize + CLUSTER_SIZE ; // one cluster read to short buffer
    }
    FLWDataFrame * pFLWData = ( FLWDataFrame * ) ( m_pReadBuffer + m_dwInBufferIndex ) ;
    int iIndexInIndexes = m_dwNRestoredFrames / INDEX_SAVE_PERIOD ;
//     if ( m_dwNRestoredFrames % INDEX_SAVE_PERIOD == 0 )
//     {
//       ASSERT( m_i64CurrentPositionInFile == m_pIndex->iEntry[ iIndexInIndexes ].uFrameOffset ) ;
//     }
    if ( pFLWData->uLabel != HUNK ) // not proper format
    {
      ASSERT( 0 ) ;
      return -8 ;
    }
    if ( pFLWData->uSize > m_dwTheRestInBuffer ) // we hope, that it will be rare situation when buffer should be reallocated
    {
      DWORD dwBufferShortage = pFLWData->uSize - m_dwTheRestInBuffer ;
      DWORD dw1stClusterIndex = ClusterBegin( m_dwInBufferIndex ) ;
      DWORD dwMoveSize = m_dwReadBufferLen - dw1stClusterIndex ;
      if ( m_dwInBufferIndex > dwBufferShortage ) // possible to shift existing data forward 
      {                                           // and read additional data to the same buffer
        memcpy( m_pReadBuffer , m_pReadBuffer + dw1stClusterIndex , dwMoveSize ) ; // move the rest to the beginning

        m_dwInBufferIndex -= dw1stClusterIndex ;
        m_dwInBufferFLWDataLen -= dw1stClusterIndex ;
        DWORD dwEmptyLen = dw1stClusterIndex ;
        if ( ReadToBuffer( m_pReadBuffer + dwMoveSize ,
          dwEmptyLen , dwNTransferedBytes , "Read after move" ) < 0 ) // fill the rest of buffer
        {
          ASSERT( 0 ) ;
          return -4 ; // can't read
        };
        m_dwInBufferFLWDataLen += dwNTransferedBytes ;
        m_dwTheRestInBuffer = m_dwInBufferFLWDataLen - m_dwInBufferIndex ;
        m_i64BufferOriginPositionInFile += dw1stClusterIndex ;
        iResultLen = dwNTransferedBytes ;
      }
      else // Necessary to increase buffer
      {
        DWORD dwNewLength = NecessarySizeInClusters( pFLWData->uSize ) ;
        LPBYTE pNewBuffer = new BYTE[ dwNewLength ] ;
        if ( !pNewBuffer )
        {
          ASSERT( 0 ) ;
          return -5 ;
        }
        memcpy( pNewBuffer , m_pReadBuffer + dw1stClusterIndex , dwMoveSize ) ; // move the rest to the beginning
        if ( ReadToBuffer( m_pReadBuffer + dwMoveSize ,
          dwNewLength - dwMoveSize , dwNTransferedBytes , "Read to new buffer after move" ) < 0 ) // fill the rest of buffer
        {
          ASSERT( 0 ) ;
          return -4 ; // can't read
        };
        delete m_pReadBuffer ;
        m_pReadBuffer = pNewBuffer ;

        m_dwInBufferFLWDataLen = dwNewLength ;
        m_dwInBufferIndex -= dwMoveSize ;
        m_dwTheRestInBuffer = m_dwInBufferFLWDataLen - m_dwInBufferIndex ;
        m_i64BufferOriginPositionInFile += dwMoveSize ;
        iResultLen = dwNewLength - dwMoveSize ;
      }
    }
  }
  return iResultLen ;
}

bool CFLWArchive::Seek( __int64 i64Offset , UINT From , __int64 * pi64AbsOffset )
{
  if ( m_OpenWrite || !m_OpenRead || !m_pIndex->uFramesCount )
    return false;

  __int64 i64AbsOffset = 0 ;
  if ( m_pAsyncFile )
  {
    if ( m_pAsyncFile->IsOpen() )
    {
      // enumerators for async file are the same with enumerators for FXFile
      if ( m_pAsyncFile->SeekFile( i64Offset , i64AbsOffset , From ) )
      {
        if ( pi64AbsOffset )
          *pi64AbsOffset = i64AbsOffset ;
        return true ;
      }

    }
  }
  else if ( ( FILE* ) m_File ) // file is open
  {
    i64AbsOffset = m_File.Seek( i64Offset , From );
    if ( pi64AbsOffset )
      *pi64AbsOffset = i64AbsOffset ;
    return true ;
  }
  return false ;
}


bool CFLWArchive::Seek( unsigned int pos )
{
  if ( m_OpenWrite ) 
    return false;
  if ( !m_OpenRead ) 
    return false;
  if ( pos > GetFramesNmb() ) 
    return false;

  if ( !m_pIndex->uFramesCount || ( pos < INDEX_SAVE_PERIOD ) )
    ResetInputBuffers() ;
  else
  {
    unsigned keyindex = pos / INDEX_SAVE_PERIOD ;
    if ( keyindex > m_pIndex->uFramesCount )
      keyindex = m_pIndex->uFramesCount;
    FSIZE offset = m_pIndex->iEntry[ keyindex ].uFrameOffset;

    if ( m_pAsyncFile )
    {
      m_ReadingMutex.lock() ;
      ResetInputBuffers() ;
      __int64 i64AbsOffset ;
      FSIZE ClusterBeginOffset = ClusterBegin( offset ) ;

      if ( !Seek( ClusterBeginOffset , FXFile::begin , &i64AbsOffset ) )
      {
        m_ReadingMutex.unlock() ;
        return false ;
      }

      DWORD dwNTransferedBytes ;
      if ( ReadToBuffer( m_pReadBuffer , m_dwReadBufferLen ,
        dwNTransferedBytes , "Seek for frame number " ) < 0 )
      {
        m_ReadingMutex.unlock() ;
        return false ;
      }

      m_i64BufferOriginPositionInFile = i64AbsOffset ;
      m_dwInBufferIndex = ( DWORD ) ( offset - m_i64BufferOriginPositionInFile ) ;
      m_i64CurrentPositionInFile = m_i64BufferOriginPositionInFile
        + m_dwInBufferIndex ;
      m_dwTheRestInBuffer = dwNTransferedBytes - m_dwInBufferIndex ;

      FLWDataFrame * pdt = ( FLWDataFrame * ) ( m_pReadBuffer + m_dwInBufferIndex );

      DWORD dwFrameCnt = keyindex * INDEX_SAVE_PERIOD ;
      while ( (dwFrameCnt < pos) && (dwFrameCnt < GetFramesNmb()) )
      {
        CorrectBuffer() ; // MOve to the next frame
        if ( !CheckAndReadToBuffer() )
        {
          m_ReadingMutex.unlock() ;
          return false ; // end of file
        }
        pdt = ( FLWDataFrame * ) ( m_pReadBuffer + m_dwInBufferIndex );
        dwFrameCnt++ ;
      }
      m_dwNRestoredFrames = dwFrameCnt ;
      m_ReadingMutex.unlock() ;
    }
    else
    {
      m_File.Seek( offset , FXFile::begin );
      FLWDataFrame tmp;
      if ( m_File.Read( &tmp , sizeof( FLWDataFrame ) ) != sizeof( FLWDataFrame ) )
      {
        TRACE( "Error reading file #1\n" );
        return false;
      }
      else if ( tmp.uLabel != HUNK )
      {
        SENDERR_1( "Error reading file %s" , m_FileName );
        return false;
      }
      else
      {
        m_CrntFrame = tmp.dwID;
        m_File.Seek( tmp.uSize - sizeof( FLWDataFrame ) , FXFile::current );
        m_StartTime = ( m_HostGadget->GetGraphTime() * 1.e-3 ) - tmp.dTime;
      }
      
      FLWDataFrame* dt = ( FLWDataFrame* ) malloc( sizeof( FLWDataFrame ) );
      while ( m_CrntFrame + 1 < pos )
      {
        if ( m_File.Read( dt , sizeof( FLWDataFrame ) ) != sizeof( FLWDataFrame ) )
        {
          // error
          TRACE( "Error reading file #1\n" );
        }
        else
        {
          if ( dt->uLabel != HUNK )
          {
            free( dt );
            SENDERR_1( "Error reading file %s" , m_FileName );
            return false;
          }
          m_CrntFrame = dt->dwID;
          m_File.Seek( dt->uSize - sizeof( FLWDataFrame ) , FXFile::current );
          m_StartTime = ( m_HostGadget->GetGraphTime() * 1.e-3 ) - dt->dTime;
        }
      }
      free( dt );
    }
  }

  return true;
}

bool CFLWArchive::Seek( double time )
{
  if ( m_OpenWrite || !m_OpenRead || !m_pIndex->uFramesCount )
    return false;

  __int64 i64AbsOffset = 0 ;
  DWORD i;
  for ( i = 0; i < m_pIndex->uFramesCount; i++ )
  {
    if ( m_pIndex->iEntry[ i ].fFrameTime > time )
      break;
  }

  if ( !i )
    ResetInputBuffers() ;
  else
  {
    FSIZE offset = m_pIndex->iEntry[ i - 1 ].uFrameOffset;

    if ( m_pAsyncFile )
    {
      m_dwNRestoredFrames = ( i - 1 ) * INDEX_SAVE_PERIOD ;

      m_ReadingMutex.lock() ; // stop main reading thread
      ResetInputBuffers() ;
      FSIZE ClusterBeginOffset = ClusterBegin( offset ) ;
      if ( !Seek( offset , FXFile::begin , &i64AbsOffset ) )
      {
        m_ReadingMutex.unlock() ;
        return false ;
      }
      
      DWORD dwNTransferedBytes ;
      if ( ReadToBuffer( m_pReadBuffer , m_dwReadBufferLen ,
        dwNTransferedBytes , "Seek for time " ) < 0 )
      {
        m_ReadingMutex.unlock() ;
        return false ;
      }

      m_i64BufferOriginPositionInFile = i64AbsOffset ;
      m_dwInBufferIndex = ( DWORD ) ( offset - m_i64BufferOriginPositionInFile ) ;
      m_i64CurrentPositionInFile = m_i64BufferOriginPositionInFile
        + m_dwInBufferIndex ;
      m_dwTheRestInBuffer = dwNTransferedBytes - m_dwInBufferIndex ;

      FLWDataFrame * pdt = (FLWDataFrame *)(m_pReadBuffer + m_dwInBufferIndex);

      while ( pdt->dTime < time )
      {
        m_i64BufferOriginPositionInFile += pdt->uSize ;
        m_dwInBufferIndex += pdt->uSize ;
        m_i64CurrentPositionInFile += pdt->uSize;
        m_dwTheRestInBuffer = m_dwReadBufferLen - m_dwInBufferIndex ;
        if ( !CheckAndReadToBuffer() )
        {
          m_ReadingMutex.unlock() ;
          return false ; // end of file
        }
        pdt = ( FLWDataFrame * ) ( m_pReadBuffer + m_dwInBufferIndex );
        m_dwNRestoredFrames++ ;
      }
      m_ReadingMutex.unlock() ; // resume main reading thread: all data are prepared
    }
    else if ( !Seek( offset , FXFile::begin , &i64AbsOffset ) ) // synchronous mode with FXFile
      return false ;
    else // Seeking in file OK
    {
      FLWDataFrame df;
      do
      {
        if ( m_File.Read( &df , sizeof( df ) ) != sizeof( df ) )
        {
          TRACE( "Error reading file #1\n" );
          return false;
        }
        else if ( df.uLabel != HUNK )
        {
          SENDERR_1( "Error reading file %s" , m_FileName );
          return false;
        }
        else if ( df.dTime > time )
          break;
        m_File.Seek( df.uSize - sizeof( FLWDataFrame ) , FXFile::current );
      } while ( true );
      m_StartTime = ( m_HostGadget->GetGraphTime() * 1.e-3 ) - df.dTime;
      m_File.Seek( -( LONG )sizeof( FLWDataFrame ) , FXFile::current );
      m_CrntFrame = df.dwID;
    }
  }
  return true;
}

bool CFLWArchive::OpenWrite( LPCTSTR fName )
{
  FXAutolock al( m_Lock );
  if ( m_OpenRead || m_OpenWrite )
  {
    Close();
  }
  FXFileException e;

  m_FileName = fName;
  int iNBuffers = 0 , iBufferSize_KB = 0 ;
  if ( m_HostGadget )
  {
    FXPropertyKit pk ;
    m_HostGadget->PrintProperties( pk ) ;
    pk.GetInt( "NBuffers" , iNBuffers ) ;
    pk.GetInt( "BufferSize_KB" , iBufferSize_KB ) ;
    SetAsyncWriteMode( iBufferSize_KB , iNBuffers ) ;
  }
  if ( !m_bAsyncFileWrite )
  {
    if ( !m_File.Open( m_FileName , FXFile::modeCreate | FXFile::modeWrite , &e ) )
    {
      m_FileName.Empty();
      return false;
    }
    m_File.SetAsyncWriteMode( true ) ;
    m_File.Write( &m_FLWHeader , sizeof( m_FLWHeader ) );
  }
  else
  {
    if ( m_pAsyncFile )
    {
      delete m_pAsyncFile ;
      m_pAsyncFile = NULL ;
    }
    m_pAsyncFile = new FXAsyncFile( m_FileName , TRUE , 
      GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE , 
      this , TRUE , 0 , FALSE ) ;

    m_iClasterSize = CLUSTER_SIZE ;
    
    if ( !m_pShortBuffer )
    {
      if ( GetNewShortBuffer() )
      {
        memcpy( m_pShortBuffer , &m_FLWHeader , sizeof( m_FLWHeader ) ) ;
        m_i64CurrentPositionInFile = m_iShortBufferFilledLen = sizeof( m_FLWHeader ) ;
        m_i64NWrittenClusters = 0 ;
        m_OpenWrite = true;
        m_StartTime = ( m_HostGadget->GetGraphTime() * 1.e-3 );
        m_dLastDiagnosticMsgTime = GetHRTickCount() ;
        m_dVFrameSaveTimeMin_ms = m_dSerializationTimeMin_ms = 10000. ;
        m_dVFrameSaveTimeMax_ms = m_dSerializationTimeMax_ms = 0. ;
        m_iNCompletedWrights = m_iNRequestedWrites = 0 ;
        m_i64NWritten = 0 ;
        m_bDisable = false ;
        return true;
      }
    }
    else
      ASSERT( 0 ) ;
  }
  return false ;
}

// should be called when m_ReadingMutex is locked
void CFLWArchive::ResetInputBuffers()
{
  m_DataAccessMutex.lock() ;
  while ( m_DataForSending.size() )
  {
    CDataFrame * pFrame = m_DataForSending.front() ;
    m_DataForSending.pop() ;
    pFrame->Release() ;
  }
  m_DataAccessMutex.unlock() ;
}

void CFLWArchive::Close()
{
  if ( IsOpen() )
  {
    if ( m_OpenWrite )
    {
      m_Timer.ResetPause();
      int iNCycles = 0 ;
      m_bDisable = true ;

      while ( m_DataForWriting.size() )
      {
        m_DataAccessMutex.lock() ;
        const CDataFrame * df = m_DataForWriting.front() ;
        m_DataForWriting.pop();
        m_DataAccessMutex.unlock() ;
        ( ( CDataFrame* ) df )->Release() ;
      }
      SetAsyncWriteMode( false ) ;
      FXString FileName = m_pAsyncFile->GetFilePath() ;
      if ( m_pAsyncFile && m_pAsyncFile->IsOpen() )
      {
        FlushFileBuffers( ( HANDLE ) m_pAsyncFile ) ;
        Sleep( 100 ) ;
        delete m_pAsyncFile ;
        m_pAsyncFile = NULL ;
      }
      FXFileException e;


      if ( !m_File.Open( m_FileName , FXFile::modeReadWrite | FXFile::modeNoTruncate , &e ) )
      {
        SENDERR( "CFLWArchive::Close(): FLW file %s is invalid: Can't open for final modification" , ( LPCTSTR ) FileName ) ;
        return ;
      }
      m_File.SetAsyncWriteMode( false ) ;
        // if something is in buffers then necessary to write to disk
      if (m_pLongBuffer && m_iLongBufferFilledLen ) // SHould not be, but...
      {
        m_File.SeekToEnd() ;
        m_File.Write( m_pLongBuffer , m_iLongBufferFilledLen ) ;
        m_i64CurrentPositionInFile = m_File.GetPosition() ;
        m_LongBufferManager.ReleaseBuffer( m_pLongBuffer ) ;
        m_pLongBuffer = NULL ;
      }
      if ( m_pShortBuffer && m_iShortBufferFilledLen )
      {
        m_File.SeekToEnd() ;
        m_File.Write( m_pShortBuffer , m_iShortBufferFilledLen ) ;
        m_i64CurrentPositionInFile = m_File.GetPosition() ;
        m_ShortBufferManager.ReleaseBuffer( m_pShortBuffer ) ;
        m_pShortBuffer = NULL ;
      }
      m_FLWHeader.uIndexOffset = m_File.GetPosition();
      SaveIndex();
      m_File.SeekToBegin();
      FSIZE Pos = m_File.GetPosition() ;
      m_File.Write( &m_FLWHeader , sizeof( m_FLWHeader ) );
      FSIZE PosA = m_File.GetPosition() ;
      m_File.SeekToBegin();
      FLWHeader FLWReadHeader ;
      m_File.Read( &FLWReadHeader , sizeof( FLWReadHeader ) ) ;
      m_LongBufferManager.SetEnable( false ) ;
      m_OpenWrite = false;
    }

    if ( m_OpenRead )
    {
      if ( m_pSerializationThread )
      {
        m_bCloseThread = true ;
        m_pSerializationThread->join() ;

        delete m_pAsyncFile ;
        m_pAsyncFile = NULL ;
      }
      m_OpenRead = false ;
      ResetInputBuffers() ;
      if ( m_pReadBuffer )
      {
        delete[] m_pReadBuffer ;
        m_pReadBuffer = NULL ;
      }
      m_OpenRead = false ;
    }

    if ( m_File.m_pFile )
      m_File.Close();

  }
  _init_flwheader( m_FLWHeader );
  //_init_flwindex( m_pIndex );
}

bool CFLWArchive::IsOpen()
{
  return ( m_OpenRead || m_OpenWrite 
    || (m_File.m_pFile != FXFile::hFileNull) 
    || ( m_pAsyncFile && m_pAsyncFile->IsOpen() ) );
}

int CFLWArchive::GetNBusyBuffers()
{
  return (m_LongBufferManager.GetNBusyBuffers() + m_ShortBufferManager.GetNBusyBuffers()) ;
}
bool CFLWArchive::WriteFrame( const CDataFrame* df )
{
  if ( !m_pSerializationThread && !m_bDisable )
  {
    m_File.SetAsyncWriteMode( false ) ;
    if ( m_File.m_pFile )
    {
      FXAutolock al( m_Lock );
      FLWDataFrame* dt = Serialize( df );
      if ( dt )
      {
        FSIZE pos = m_File.GetPosition();
        double tm = dt->dTime;
        m_File.Write( dt , ( size_t ) dt->uSize );
        if ( m_FLWHeader.fMaxTime < tm )
          m_FLWHeader.fMaxTime = tm;
        free( dt );
        ++m_FLWHeader.uFramesCnt ;
        // #ifdef _DEBUG
        //     if ( m_FLWHeader.uFramesCnt % 500 == 0 )
        //     {
        //       iNFrames = m_FLWHeader.uFramesCnt ;
        //     }
        // #endif
        if ( !( m_FLWHeader.uFramesCnt % INDEX_SAVE_PERIOD ) || m_bSkippedEOS )
        {
          m_bSkippedEOS = false;
          if ( Tvdb400_IsEOS( df ) )
            m_bSkippedEOS = true;
          else
            UpdateIndex( pos , tm );
        }
      }
      return true;
    }
  }
  else
  {
    if ( Tvdb400_IsEOS( df ) )
    {
      Close();
      SetAsyncWriteMode( 0 , 0 ) ;
    }
    else if ( !m_bDisable )
    {
      ( ( CDataFrame* ) df )->AddRef() ;
      m_DataAccessMutex.lock() ;
      m_DataForWriting.push( df ) ;
      m_DataAccessMutex.unlock() ;
      SetEvent( m_hDataAvailable ) ;
    }
    return true ;
  }
  return false;
}
void CFLWArchive::OnAsyncOperationComplete( BOOL bRead , DWORD dwErrorCode , LPVOID pParams )
{
  LPOVERLAPPEDEX pOverData = ( LPOVERLAPPEDEX ) pParams ;

  AsyncIOParam BufLocation = { (ShortOrLong)pOverData->Param.LowPart , pOverData->Param.HighPart } ;
  if ( TRUE == bRead )
  {
    if ( dwErrorCode )
      SENDERR( _T( "OnAsyncOperationComplete-->Read, error code:%d" ) , dwErrorCode );
  }
  else
  {
    if ( dwErrorCode )
    {
     // _stprintf( tcszMsg , _T( "OnAsyncOperationComplete-->Write, error code:%d" ) , dwErrorCode );
      SENDERR( "FLW write error %s buf manager Index=%d Error=%u" ,
        ( BufLocation.m_UsedBufManager == SOL_Short ) ? "Short" : "Long" ,
        BufLocation.m_iBufferIndex , dwErrorCode ) ;
    }
    else
      m_iNCompletedWrights++ ;
    switch ( BufLocation.m_UsedBufManager )
    {
      case SOL_Short:    // release one of short buffers
        m_ShortBufferManager.ReleaseBuffer( BufLocation.m_iBufferIndex );
        m_pShortBuffer = NULL ;
        break ;
      case SOL_Long:    // release one of long buffers
        m_LongBufferManager.ReleaseBuffer( BufLocation.m_iBufferIndex ) ;
        m_pLongBuffer = NULL ;
        break ;
      default: // Nothing to release (SOL_Unknown)
        break ;
    } ;
  }
  if ( m_bWaitForFileOperationFinished )
    m_bWaitForFileOperationFinished = false ;

  DWORD dwTransferred = 0 ;
  BOOL bRet = GetOverlappedResult( 
    (HANDLE)m_pAsyncFile , pOverData , &dwTransferred , TRUE );
  if ( FALSE == bRet )
  {
    m_dwLastError = GetLastError();
    if ( ERROR_IO_INCOMPLETE == m_dwLastError )
    {
      SENDINFO( _T( "IsAsyncIOComplete --> IO pending..." ) );
    }
  }
  else
    m_i64NWritten += dwTransferred;

  double dNow = GetHRTickCount() ;
  m_dVFrameSaveTime_ms = dNow - pOverData->m_dStartTime_ms ;

  SetMinMax( m_dVFrameSaveTime_ms ,
    m_dVFrameSaveTimeMin_ms , m_dVFrameSaveTimeMax_ms ) ;
  if ( m_dVFrameSaveTime_ms == m_dVFrameSaveTimeMax_ms )
    m_dTimeOfAquireTimeMax = dNow ;
  else if ( dNow - m_dTimeOfAquireTimeMax > 3000. )
    m_dVFrameSaveTimeMax_ms = m_dVFrameSaveTime_ms ;
}

void    CFLWArchive::Rewind()
{
  if ( !m_OpenRead ) 
    return;
  m_Timer.ResetPause();
  if ( m_bAsyncFileRead )
  {
    m_ReadingMutex.lock() ;
    __int64 i64Pos = 0 ;
    if ( m_bFirstIsReached )
      i64Pos = m_i64FirstFrameOrigin ;
    else
      i64Pos = GetFirstFrameOffset() ;               

    __int64 i64BeginCluster = ClusterBegin( i64Pos ) , i64CalculatedOffset ;
    DWORD dwNTransferedBytes = 0 ;
    m_pAsyncFile->SeekFile( i64BeginCluster , i64CalculatedOffset , FILE_BEGIN ) ;
    ASSERT( i64CalculatedOffset == i64BeginCluster ) ;
    DWORD dwHighPart = 0 ;
    __int64 i64ReadLen = m_pAsyncFile->GetFileLength( &dwHighPart ) ;
    i64ReadLen += (( __int64 ) dwHighPart) << 32 ;
    if ( ( ( __int64 ) m_dwReadBufferLen ) < i64ReadLen )
      i64ReadLen = m_dwReadBufferLen ;

    if ( ReadToBuffer( m_pReadBuffer , (DWORD)i64ReadLen ,
      dwNTransferedBytes , "Rewind " ) < 0 )
    {
    }
    else
    {
      m_i64BufferOriginPositionInFile = i64BeginCluster ;
      m_dwInBufferIndex = (DWORD)(i64Pos - i64BeginCluster) ;
      m_i64CurrentPositionInFile = m_i64BufferOriginPositionInFile
        + m_dwInBufferIndex ;
      m_dwTheRestInBuffer = dwNTransferedBytes - m_dwInBufferIndex ;
      m_dwInBufferFLWDataLen = dwNTransferedBytes ;
      if ( i64ReadLen < m_dwReadBufferLen )
        m_dwInBufferFLWDataLen -= sizeof( m_pIndex->uFramesCount ) + m_pIndex->uFramesCount * sizeof( FLWIndexEntry );
      m_dwNRestoredFrames = m_iFirstFrame ;

      ResetInputBuffers() ;
    }
    m_ReadingMutex.unlock() ;
  }
  else
  {
    m_File.Seek( sizeof( m_FLWHeader ) , FXFile::begin );
  }
  m_StartTime = m_HostGadget->GetGraphTime() * 1.e-3 ;
  m_dwNTakenFrames = m_CrntFrame = 0;
}

CDataFrame* CFLWArchive::ReadFrame( unsigned int nmb , double time )
{
  FXAutolock al( m_Lock );
  double ts = m_HostGadget->GetGraphTime() * 1.e-3; // convert to milli seconds
  m_Timer.ResetPause();

  if ( !m_OpenRead ) 
    return NULL;
  m_dwNextFrame = nmb ;
  m_dNextFrameTime = time ;
  if ( nmb != NEXT_FRAME )
  {
    if ( nmb == FRAME_BY_TIME )
      Seek( time );
    else
      Seek( nmb );
  }
//   if ( IsEOF() )
//   {
//     if ( m_AutoRewind )
//     {
//       Rewind();
//     }
//     else 
//       return NULL;
//   }

  CDataFrame * pNewFrame = NULL ;
  m_DataAccessMutex.lock() ;
  if ( m_DataForSending.size() )
  {
    pNewFrame = m_DataForSending.front() ;
    m_DataForSending.pop() ;
  }
  m_DataAccessMutex.unlock() ;
  if ( pNewFrame )
  {
    bool bFirstAfterRewind = (m_dwNTakenFrames++ == m_iFirstFrame ) ;
    m_CrntFrame = pNewFrame->GetId() ;
    if ( m_bAutoRewindToStart && bFirstAfterRewind )
      m_StartTime -= pNewFrame->GetTime() ;

    m_LastOperationTime = (m_HostGadget->GetGraphTime() * 1.e-3) - ts ; // GetHRTickCount() - ts;
    if ( !m_bForceNextFrame )	// no external trigger
    {
      if ( m_bFixedDelay )	// fixed framerate
      {
        if ( !m_Timer.Pause( m_FixedDelay ) )
        {
          pNewFrame->Release();
          pNewFrame = NULL;
        }
      }
      else	// calculate actual delay
      {

        if ( bFirstAfterRewind )
        {
          m_dLastSentArchiveFrameTime_ms = pNewFrame->GetTime() ;
          m_dLastSentGraphFrameTime_ms = m_HostGadget->GetGraphTime() * 1.e-3 ;
        }
        else
        {
          double dArchiveTimeAfterLast = pNewFrame->GetTime() - m_dLastSentArchiveFrameTime_ms ;
          m_dLastSentArchiveFrameTime_ms = pNewFrame->GetTime() ;
          double dGraphTime = m_HostGadget->GetGraphTime() * 1.e-3 ;
          double dPassedAfterLastTime_ms = dGraphTime - m_dLastSentGraphFrameTime_ms ;
          int iPassedTime_ms = ROUND(dPassedAfterLastTime_ms - dArchiveTimeAfterLast) ;
          if ( Tvdb400_IsEOS( pNewFrame ) )
            iPassedTime_ms = 0;
          TRACE( "ReadFrame ID=%u dt->dTime: %f; delay: %d.......   " , 
            pNewFrame->GetId() , pNewFrame->GetTime() , iPassedTime_ms );

          if ( iPassedTime_ms < -2 )
          {
            if ( ( iPassedTime_ms < -1 ) && !m_Timer.Pause( -iPassedTime_ms - 2 ) )
            {
              pNewFrame->Release();
              pNewFrame = NULL;
            }
            TRACE( "End delay %d ms. Lqueue=%d\n" , -iPassedTime_ms , (int)m_DataForSending.size() );
          }
          m_dLastSentGraphFrameTime_ms = m_HostGadget->GetGraphTime() * 1.e-3 ;
        }
//         double et = dGraphTime - m_StartTime;
//         //TRACE("+++ %g-%g=%g\n", dt->dTime,et, dt->dTime-et);
//         double dTime = pNewFrame->GetTime() - et ;
//         if ( dTime > 1. )
//         {
//           DWORD delay = ( DWORD ) dTime ;
//           if ( Tvdb400_IsEOS( pNewFrame ) )
//             delay = 0;
//           TRACE( "ReadFrame ID=%u dt->dTime: %f; delay: %d.......   " , 
//             pNewFrame->GetId() , pNewFrame->GetTime() , delay );
//           if ( delay && !m_Timer.Pause( delay ) )
//           {
//             pNewFrame->Release();
//             pNewFrame = NULL;
//           }
//           TRACE( "End delay\n" );
//         }
      }
    }
    else do
    {
    } while ( m_Timer.Pause( 10000 ) ); // waiting for external trigger
  }
  return pNewFrame;
}

bool CFLWArchive::IsEOF()
{
  if ( m_FLWHeader.uIndexOffset )
    return (m_File.GetPosition() == m_FLWHeader.uIndexOffset);
  return (m_File.GetLength() == m_File.GetPosition());
}

void CFLWArchive::LoadIndex()
{
  if ( m_FLWHeader.uIndexOffset )
  {
    _init_flwindex( m_pIndex );
    m_File.Seek( m_FLWHeader.uIndexOffset , FXFile::begin );
    if ( m_File.Read( &m_pIndex->uFramesCount , sizeof( unsigned ) ) == sizeof( unsigned ) 
      && m_pIndex->uFramesCount )
    {
      if ( m_pIndex->uFramesCount > m_pIndex->uMaxFrames )
        m_pIndex->uMaxFrames = m_pIndex->uFramesCount;

      DWORD size = sizeof( FLWIndex ) + m_pIndex->uFramesCount * sizeof( FLWIndexEntry );
      m_pIndex = (FLWIndex*) realloc( m_pIndex , size );
      size -= sizeof( FLWIndex );
      if ( m_File.Read( m_pIndex->iEntry , size ) != size )
        _init_flwindex( m_pIndex );
        /*				TRACE("INDEX TABLE:\n");
                for (int i = 0; i < (int)m_pIndex->uFramesCount; i++)
                {
                  TRACE("%d ==> %f\n", m_pIndex->iEntry[i].uFrameOffset, m_pIndex->iEntry[i].fFrameTime);
                }*/
    }
    Rewind();
  }
}

void CFLWArchive::SaveIndex()
{
  DWORD size = sizeof( m_pIndex->uFramesCount ) + m_pIndex->uFramesCount * sizeof( FLWIndexEntry );
  if ( m_File.m_pFile != m_File.hFileNull )
    m_File.Write( &m_pIndex->uFramesCount , size );
  else if ( m_pAsyncFile && m_pAsyncFile->IsOpen() )
  {
    m_bWaitForFileOperationFinished = true ;
    m_pAsyncFile->Write( &m_pIndex->uFramesCount , size , 0xffffffff , 0xffffffff ) ;
    double dStart = GetHRTickCount() ;
    while ( m_bWaitForFileOperationFinished )
    {
      if ( GetHRTickCount() - dStart > 500. ) //timeout
      {
        SENDERR( "Timeout 500ms on FLW index saving" ) ;
        break ;
      }
      WaitForSingleObjectEx( m_hAllertEvent , 5 , TRUE ) ;
    }
  }
}

void CFLWArchive::UpdateIndex( FSIZE pos , double tm )
{
  if ( m_pIndex )
  {
  if ( m_pIndex->uMaxFrames == m_pIndex->uFramesCount )
  {
    DWORD size = sizeof( FLWIndex ) + (m_pIndex->uMaxFrames + DELTA_INDEX) * sizeof( FLWIndexEntry );
    m_pIndex = (FLWIndex*) realloc( m_pIndex , size );
      ASSERT( m_pIndex ) ;
    m_pIndex->uMaxFrames += DELTA_INDEX;
  }
  m_pIndex->iEntry[ m_pIndex->uFramesCount ].uFrameOffset = pos;
  m_pIndex->iEntry[ m_pIndex->uFramesCount ].fFrameTime = tm;
  m_pIndex->uFramesCount++;
}
  else
  {
    ASSERT( 0 ) ;
  }
}

LONGLONG CFLWArchive::GetPosition()
{
  if ( m_File.m_pFile != m_File.hFileNull )
    return m_File.GetPosition() ;
  else if ( m_pAsyncFile && m_pAsyncFile->IsOpen() )
    return m_pAsyncFile->GetPosition() ;
  
  return -1 ;
}

int CFLWArchive::CorrectBuffer()
{
  FLWDataFrame * pFLW = (FLWDataFrame *)(m_pReadBuffer + m_dwInBufferIndex) ;
  m_dwInBufferIndex += pFLW->uSize ;
  m_i64CurrentPositionInFile += pFLW->uSize ;
  m_dwTheRestInBuffer -= pFLW->uSize ;
  return pFLW->uSize ;
}

bool CFLWArchive::SetAsyncWriteMode( FXSIZE BufferLen_KB , int iNBuffers )
{
#ifndef SHBASE_CLI
  bool bSet = ( BufferLen_KB != 0 ) && ( iNBuffers != 0 ) ;
  if ( m_pSerializationThread )
  {
      m_bCloseThread = true ;
      if ( m_hDataAvailable )
      {
        SetEvent( m_hDataAvailable ) ;
        Sleep( 2 ) ;
        CloseHandle( m_hDataAvailable ) ;
        m_hDataAvailable = NULL ;
      }
      if ( m_hAllertEvent )
      {
        CloseHandle( m_hAllertEvent ) ;
        m_hAllertEvent = NULL ;
      }
      Sleep( 5 ) ;
      if ( m_pSerializationThread->joinable() )
        m_pSerializationThread->join() ;
      delete m_pSerializationThread ;
      m_pSerializationThread = NULL ;
      m_iSerializationByThreadMode = 0 ;
  }
  if ( bSet )
  {
    if ( !m_pSerializationThread )
    {
      if ( !m_hDataAvailable )
        m_hDataAvailable = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
      if ( !m_hAllertEvent )
        m_hAllertEvent = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
      if ( BufferLen_KB && iNBuffers )
      {
        m_DataAccessMutex.lock() ;
        if ( !m_LongBufferManager.Allocate( ( int ) BufferLen_KB * 1024 , iNBuffers , ( ( BufferLen_KB % 4 ) == 0 ) ) )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "CFLWArchive::SetAsyncWriteMode" , 0 ,
            "Can't allocate %d buffers L=%uKB, continue to work in synchronous mode" , 
            iNBuffers , BufferLen_KB ) ;
          m_DataAccessMutex.unlock() ;
            return false ;
        }
      }
      if ( !m_ShortBufferManager.Allocate(  64 * 1024 , 100 , true ) )
      {
        FxSendLogMsg( MSG_ERROR_LEVEL , "CFLWArchive::SetAsyncWriteMode" , 0 ,
          "Can't allocate short %d buffers L=%uKB, continue to work in synchronous mode" ,
          100 , 4 ) ;
        m_DataAccessMutex.unlock() ;
        return false ;
      }
      m_NWrittenFrames = 0 ;
      m_pSerializationThread = new std::thread( SerializationThreadFunc , this ) ;
      m_iSerializationByThreadMode = 1 ;

      m_DataAccessMutex.unlock() ;
      m_LongBufferManager.SetEnable( true ) ;
      m_ShortBufferManager.SetEnable( true ) ;
    }
  }
  return true ;
#elif
  return false ;
#endif 

}

bool CFLWArchive::SetAsyncReadMode()
{
#ifndef SHBASE_CLI
  if ( m_pSerializationThread )
  {
    m_bCloseThread = true ;
    if ( m_hDataAvailable )
    {
      SetEvent( m_hDataAvailable ) ;
      Sleep( 2 ) ;
      CloseHandle( m_hDataAvailable ) ;
      m_hDataAvailable = NULL ;
    }
    if ( m_hAllertEvent )
    {
      CloseHandle( m_hAllertEvent ) ;
      m_hAllertEvent = NULL ;
    }
    Sleep( 5 ) ;
    if ( m_pSerializationThread->joinable() )
      m_pSerializationThread->join() ;
    delete m_pSerializationThread ;
    m_pSerializationThread = NULL ;
    m_iSerializationByThreadMode = 0 ;
  }
  if ( !m_pSerializationThread )
  {
    m_NWrittenFrames = 0 ;
    //m_ReadingMutex.lock() ; // thread suspending
    m_pSerializationThread = new std::thread( SerializationThreadFunc , this ) ;
    m_iSerializationByThreadMode = 1 ;
  }
  return true ;
#elif
  return false ;
#endif 

}

