// COMCapture.cpp: implementation of the COMCapture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextGadgetsImpl.h"
#include "COMCapture.h"
#include "TxtGadgets.h"

BOOL RxProc( LPVOID pParam , char *Data , int iLen )
{
  return ( ( COMCapture* )pParam )->RxProc( pParam , Data , iLen );
}

static void CALLBACK fn_ToComPort( CDataFrame* pDataFrame ,
  void* pGadget , CConnector* lpInput )
{
  double dStart = GetHRTickCount( );
  COMCapture * pComCapture = ( COMCapture* ) pGadget;
  CTextFrame* txtFrame = pDataFrame->GetTextFrame( DEFAULT_LABEL );
  if ( !Tvdb400_IsEOS( pDataFrame ) )
  {
    if ( _tcsicmp( pDataFrame->GetLabel() , _T( "ComCapControl" ) ) == 0 )
    {
      if (txtFrame && pComCapture->m_IsRunning
        && ( txtFrame->GetString().CompareNoCase( _T( "Reconnect" ) ) == 0) )
      {
        pComCapture->CloseConnection() ;
        Sleep( 50 ) ;
        switch (pComCapture->m_InOutMode)
        {
          case IOM_Normal: pComCapture->m_FlagChar = '\n'; break;
          case IOM_HEX:
          case IOM_CSV:
          case IOM_NoFlag:
          case IOM_SpecialFlag: pComCapture->m_FlagChar = 0; break; // ?
        }
        pComCapture->OpenConnection( "Reconnect Command" ) ;
        pComCapture->m_Buffer.Empty() ;
      }
    }
    else
    {
      //Added check for null CTextFrame
      if (txtFrame)
      {
        const FXString& Content = txtFrame->GetString() ;
        int iLen = (int)Content.GetLength() ;
        FXString ForTx( Content );
        if (iLen)
        {
          if (pComCapture->m_InOutMode != IOM_Normal)
          {
            BYTE * buf = new BYTE[ iLen + 10 ]; // in reality the necessary length will be less
            int iIndex = 0;
            LPCSTR pNext = (LPCSTR)(LPCTSTR) ForTx ;
            LPCSTR pLast = pNext + iLen;
            char * pStop;
            if (pComCapture->m_InOutMode == IOM_HEX) // hex
            {
              char NextHex[ 3 ];
              NextHex[ 2 ] = 0;
              while (( pNext < pLast ) && isxdigit( *pNext ))
              {
                NextHex[ 0 ] = *( pNext++ );
                NextHex[ 1 ] = *( pNext++ );
                BYTE NextByte = ( BYTE ) strtol( NextHex , &pStop , 16 );
                if (pStop != NextHex)
                  buf[ iIndex++ ] = NextByte;
                else
                  break;
              }
            }
            else if (pComCapture->m_InOutMode == IOM_CSV) // i.e. CSV decimal bytes
            {
              int iPos = 0;
              while (pNext < pLast)
              {
                while (pNext < pLast && !isdigit( *pNext ))
                  pNext++;
                if (pNext < pLast)
                {
                  int iVal = atoi( pNext++ );
                  buf[ iIndex++ ] = ( BYTE ) iVal;
                  while (pNext < pLast &&  isdigit( *pNext ))
                    pNext++;
                }
                else
                  break;
              }
            }
            if (iIndex)
            {
              int charsPerByteInHex = 2;
              int i = ( pComCapture->m_InOutMode == IOM_HEX ) && iLen / iIndex != charsPerByteInHex ? iIndex : 0;

              for (; i < iIndex; i++)
              {
                if (buf[ i ] != 0xff)
                {
                  pComCapture->TransmitText( ( LPCTSTR ) buf , iIndex );
                  break;
                }
              }

              if (i >= iIndex) // all 0xff, reset input buffers 
              {
                EnterCriticalSection( &pComCapture->m_BufferCritSect );
                pComCapture->m_iInByteBuffer = 0;
                pComCapture->m_iNSamples = 0;
                LeaveCriticalSection( &pComCapture->m_BufferCritSect );
              }
            }
            delete[ ] buf;
          }
        }
        if (pComCapture->m_InOutMode == IOM_Normal)
        {
          if (pComCapture->m_WantsReturn)
          {
            txtFrame->Attributes( )->GetInt( "hex" , ( int& ) ( pComCapture->m_InOutMode ) );
            if (pComCapture->m_InOutMode != IOM_Normal)
            {
              if ( !iLen )
                ForTx = _T( "\r" ) ;
              else
              {
                TCHAR LastSymb = ForTx[ iLen - 1 ];
                if (( LastSymb != _T( '\r' ) ) && ( LastSymb != _T( '\n' ) ))
                  ForTx += "\r\n" ;
              }
              if (pComCapture->m_dLineDelay_ms > 0)
              {
                FXSIZE iPos = 0;
                FXString Next = Content.Tokenize( "\r\n" , iPos );
                if (iPos)
                {
                  Next += Content[ iPos - 1 ];
                  while (( iPos < iLen ) && ( Content[ iPos ] == _T( '\n' ) ) || ( Content[ iPos ] == _T( '\r' ) ))
                  {
                    Next += Content[ iPos ];
                    iPos++;
                  }
                }
                int iNTokes = 0;
                int TxLength = iLen ;
                while (iPos >= 0)
                {
                  int iTransmitLen = ( int ) Next.GetLength() ;
                  pComCapture->TransmitText( Next.GetBufferSetLength( iTransmitLen + 2 ) , iTransmitLen );
                  Next.ReleaseBuffer( ) ;
                  Next = Content.Tokenize( "\r\n" , iPos );
                  if (iPos < 0)
                    break;
                  Next += _T( '\r' );

                  while (( iPos < TxLength - 1 )
                    && ( Content[ iPos ] == _T( '\n' ) ) || ( Content[ iPos ] == _T( '\r' ) ))
                  {
                    Next += Content[ iPos ];
                    iPos++;
                  }
                }
              }
              else
                pComCapture->TransmitText( ( LPCTSTR ) Content , ( int ) iLen );
            }
            else
            {
            }
          }
          else
          {
            pComCapture->TransmitText( ForTx.GetBufferSetLength( iLen + 5 ) , iLen );
            ForTx.ReleaseBuffer() ;
          }
        }
      }
      else
      {
        SENDERR_0( "No Text Frame" );
      }
      pComCapture->m_dLastSendingSpent_ms = GetHRTickCount( ) - dStart;
    }
  }

  pDataFrame->Release() ;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_RUNTIME_GADGET_EX( COMCapture , CCaptureGadget , "Communication" , TVDB400_PLUGIN_NAME );

COMCapture::COMCapture( int iFlagChar ) :
m_IsRunning( false ) ,
m_pInputForTx( NULL ) ,
m_bClose( false ) ,
m_iCRPosInBuffer( -1 ) ,
CSerialPort( iFlagChar ) ,
m_dLastSent( 0. ) ,
m_iInByteBuffer( 0 ) ,
m_iNSamples( 0 ) ,
m_SelectedPortAsString( "Not Selected" )
{
  m_DataReadyEvent = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  InitializeCriticalSection( &m_BufferCritSect ) ;
  m_pOutput = new COutputConnector( transparent ) ;
  CSerialPort::Create( ::RxProc , this );
  //   OpenConnection();

  m_pInputForTx = new CInputConnector( text , fn_ToComPort , this ) ;
  //m_SetupObject  = new CComPortSettings(this, NULL);
}

void COMCapture::ShutDown()
{
  OnStop();
  PortAbort();
//   m_pInputListen->ShutDown() ;
  SetEvent( FXWorker::m_evResume ) ;
  m_bClose = true ;
  m_IsRunning = false;
  SetEvent( m_DataReadyEvent ) ; // ?????
  Sleep( 40 ) ;
  CloseHandle( m_DataReadyEvent ) ;
  CCaptureGadget::ShutDown();
  m_DataReadyEvent = NULL ;
  delete m_pInputForTx ;
//   delete m_pInputListen ;
  DeleteCriticalSection( &m_BufferCritSect ) ;
  delete m_pOutput;
  m_pOutput = NULL;
}

void COMCapture::OnStart()
{
  CCaptureGadget::OnStart();
  CloseConnection() ;
  switch ( m_InOutMode )
  {
    case IOM_Normal: m_FlagChar = '\n'; break;
    case IOM_HEX: m_FlagChar = 0 ; break;
    case IOM_CSV: m_FlagChar = 0; break;
    case IOM_NoFlag: m_FlagChar = 0; break;
    case IOM_SpecialFlag:; break; // Flag was set before

  }
  if ( m_iSelectedPort >= 0 )
    OpenConnection( "On Start" ) ;
  m_Buffer.Empty();
  m_IsRunning = true;
  m_bClose = false ;
//   m_pInputListen->Resume() ;
}

void COMCapture::OnStop()
{
  CCaptureGadget::OnStop();
  m_IsRunning = false;
  SetEvent( m_DataReadyEvent ) ;
  Sleep( 10 );
}

BOOL COMCapture::RxProc( LPVOID pParam , char *Data , int iLen )
{
  if ( !m_IsRunning || m_bClose )
    return true;
  EnterCriticalSection( &m_BufferCritSect ) ;
  m_dLastReceivedTime = GetHRTickCount() ;
  if ( m_InOutMode != IOM_Normal )
  {
    memcpy_s( &m_ByteBuffer[ m_iInByteBuffer ] ,
      sizeof( m_ByteBuffer ) - m_iInByteBuffer , Data , iLen ) ;
    m_iInByteBuffer += iLen ;
  }
  else
  {
    Data[ iLen ] = 0 ;
    m_Buffer += Data;
  }
  m_Samples[ m_iNSamples ].m_dSampleTime = m_dLastReceivedTime ;
  m_Samples[ m_iNSamples++ ].m_iSampleLength = iLen ;
  if ( m_iNSamples >= ARRSZ( m_Samples ) )    // last will be last
    m_iNSamples = ARRSZ( m_Samples ) - 1 ;
  LeaveCriticalSection( &m_BufferCritSect ) ;
  SetEvent( m_DataReadyEvent );
  m_dLastReceivingSpent_ms += GetHRTickCount() - m_dLastReceivedTime ;
  return true;
}

__forceinline CDataFrame* COMCapture::GetNextFrame( double* StartTime )
{
  if ( !StartTime )
    return NULL ;  // stop state

  double dAccumulated = 0 ;
  do
  {
    if ( m_WantsReturn )
    {
      if ( m_IsRunning && !m_bClose
        && ( WaitForSingleObject( m_DataReadyEvent , INFINITE ) == WAIT_OBJECT_0 ) )
      {
        double dStart = GetHRTickCount() ;
        EnterCriticalSection( &m_BufferCritSect ) ;
        m_RestData += m_Buffer ;
        m_Buffer.Empty() ;
        int iNewLen = (int) m_RestData.GetLength() ;
        int iLen = iNewLen ;
        LeaveCriticalSection( &m_BufferCritSect ) ;
        bool bIsLF = false ;
        do
        {
          m_iCRPosInBuffer = (int) m_RestData.ReverseFind( '\r' ) ;
          if ( m_iCRPosInBuffer >= 0 && m_iCRPosInBuffer < iLen )
          {
            if ( m_RestData[ m_iCRPosInBuffer + 1 ] == _T( '\n' ) )
            {
              bIsLF = true ;
              m_iCRPosInBuffer++ ;
            }
          }
          else
          {
            m_iCRPosInBuffer = (int)m_RestData.ReverseFind( '\n' ) ;
            bIsLF = ( m_iCRPosInBuffer >= 0 ) ;
          }
          if ( m_iCRPosInBuffer >= 0 )
          {
            FXString output = m_RestData.Left( m_iCRPosInBuffer + 1 );
            if ( !bIsLF )
              output.Replace( "\r" , "\n" ) ;
            int iOutLen = (int)output.GetLength() ;
            if ( iOutLen > 1 && output[ iOutLen - 1 ] == '\n'  
              && output[ iOutLen - 2 ] == '\n' )
              output.Delete( iOutLen - 1 ) ;
            
            CTextFrame* pDataFrame = CTextFrame::Create( output );
            pDataFrame->SetTime( GetGraphTime() * 1.e-3 ) ;
            pDataFrame->ChangeId( m_FrameCounter++ ) ;
            if ( !m_pOutput || !m_pOutput->Put( pDataFrame ) )
              pDataFrame->Release() ;
            m_RestData = m_RestData.Mid( m_iCRPosInBuffer + 1 );
          }
          dAccumulated += GetHRTickCount() - dStart ;
        } while ( m_iCRPosInBuffer >= 0 );
        *StartTime = dStart - m_dLastSendingSpent_ms ;
      }
      if ( ! m_IsRunning )
        m_dLastReceivedTime = 0. ;
      return NULL ;
    }
    else if ( m_InOutMode == IOM_Normal )
    {
      if ( m_IsRunning && !m_bClose )
      {
        DWORD dwWaitTime = ( m_dBlockWaitTime_ms > 0. ) ? ROUND( m_dBlockWaitTime_ms ) : INFINITE ;
        FXString output ;
        double dTakenTime = 0 ;
        do
        {
          if ( WaitForSingleObject( m_DataReadyEvent , dwWaitTime ) == WAIT_OBJECT_0 )
          {
            double dStart = GetHRTickCount() ;
            EnterCriticalSection( &m_BufferCritSect ) ;
            output += m_Buffer ;
            m_Buffer.Empty();
            m_iNSamples = 0 ;
            LeaveCriticalSection( &m_BufferCritSect ) ;
            dTakenTime += GetHRTickCount() - dStart ;
          }
          else if ( output.GetLength() && ( GetHRTickCount() > m_dLastReceivedTime + dwWaitTime ) )
            break ;
        } while ( output.IsEmpty() && m_IsRunning );
        if ( output.GetLength() )
        {
          CTextFrame* pDataFrame = CTextFrame::Create();
          pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
          pDataFrame->GetString() = output;
          *StartTime = GetHRTickCount() - dTakenTime ;
          return pDataFrame;
        }
        else
        {
          *StartTime = GetHRTickCount() - dTakenTime ;
          return NULL;
        }
      }
      else
      {
        if ( !m_IsRunning )
          m_dLastReceivedTime = 0. ;
        return NULL ;
      }
    }
    else  // m_InOutMode != IOM_Normal, i.e. we do receive binary which will be converted to text
    {
      DWORD dwWaitTime = ( m_dBlockWaitTime_ms > 0. ) ? ROUND( m_dBlockWaitTime_ms ) : INFINITE ;
      if ( (m_iInByteBuffer < m_iBlockSize)  && m_IsRunning && !m_bClose )
        BOOL bWaitResult = ( WaitForSingleObject( m_DataReadyEvent , dwWaitTime ) == WAIT_OBJECT_0 ) ;
      double dStart = GetHRTickCount() ;
      if ( m_IsRunning && !m_bClose )
      {
        if ( m_iInByteBuffer >= m_iBlockSize )
        {
          char TmpBuf[ sizeof( m_ByteBuffer ) ] ;
          int iLen = 0 ;
          FXString Result ;
          EnterCriticalSection( &m_BufferCritSect ) ;
          int iCopySize = ( m_iBlockSize ) ? m_iBlockSize : m_iInByteBuffer ;
          memcpy_s( TmpBuf , sizeof( TmpBuf ) , m_ByteBuffer , iCopySize ) ;
          iLen = iCopySize ;
          if ( iCopySize < m_iInByteBuffer )
          {
            m_iInByteBuffer -= iCopySize ;
            memcpy_s( m_ByteBuffer , sizeof( m_ByteBuffer ) , m_ByteBuffer + iCopySize , m_iInByteBuffer ) ;
          }
          else
          {
            m_iInByteBuffer = 0 ;
            m_iNSamples = 0 ;
          }
          LeaveCriticalSection( &m_BufferCritSect ) ;
          if ( iLen )
          {
            if ( m_InOutMode == IOM_HEX ) // hex mode without separators        
            {
//              TRACE( "\nReceived %d=" , iLen ) ;
//               for ( int i = 0 ; i < iLen ; i++ )
//               {
//                 TRACE( "%02x" , ( ( ( int )TmpBuf[ i ] ) & 0xff ) ) ;
//               }
              for ( int i = 0 ; i < iLen ; i++ )
              {
                Result += ( char )ToHex( ( ( ( int )TmpBuf[ i ] ) & 0xff ) >> 4 ) ;
                Result += ( char )ToHex( ( ( ( int )TmpBuf[ i ] ) & 0x0f ) ) ;
              }
            }
            if ( m_InOutMode == IOM_CSV) // csv mode, decimal bytes separated by comma        
            {
//               TRACE( "\nReceived %d= %d" , iLen , ( ( int )TmpBuf[ 0 ] ) & 0xff ) ;
//               for ( int i = 1 ; i < iLen ; i++ )
//               {
//                 TRACE( ",%d" , ( ( ( int )TmpBuf[ i ] ) & 0xff ) ) ;
//               }
              char buf[ 20 ] ;
              for ( int i = 0 ; i < iLen ; i++ )
              {
                sprintf_s( buf , "%d%c" , ( ( ( int )TmpBuf[ i ] ) & 0xff ) , ( i < iLen - 1 ) ? ',' : '\0' ) ;
                Result += buf ;
              }
            }
            if ( Result.GetLength() )
            {
              CTextFrame* pDataFrame = CTextFrame::Create();
              pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
              pDataFrame->GetString() = Result ;
//               *StartTime = dStart - m_pInputListen->GetAccumulatedSendTime() ;
              return pDataFrame;
            }
          }
        }
      }
//       *StartTime = m_pInputListen->GetAccumulatedSendTime() ;
      if ( !m_IsRunning || m_bClose )
        m_dLastReceivedTime = 0. ;

      return NULL;
    }
  } while ( m_IsRunning && !m_bClose );
  TRACE( "+++ COMCapture wrk thread is finished\n" );
  return NULL;
}

bool COMCapture::ScanSettings( FXString& text )
{
  FXString ComPortParamList ;
  ComPortParamList += "ComboBox(IOM_Hex1_CSV2_NoFlag3(Normal(0),HEX(1),CSV(2),NoEOL(3),SpecialEOL(4))),";

  if ( CSerialPort::ScanSettings( ComPortParamList ) )
  {
    text.Format( "template(%s)" , ComPortParamList );
    return true;
  }
  return false ;
}


bool COMCapture::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  pk.WriteInt( "IOM_Hex1_CSV2_NoFlag3" , m_InOutMode );
  text += pk ;
  return CSerialPort::PrintProperties( text ) ;
}

bool COMCapture::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  bool bPortChanged = false ;
  m_dwLastChanged = 0 ;

  InOutMode iNew ;
  if ( ( pk.GetInt( "IOM_Hex1_CSV2_NoFlag3" , (int&)iNew )
    || pk.GetInt( "HexMode" , ( int& ) iNew )
    ) && (iNew != m_InOutMode ) )
  {
    m_InOutMode = iNew ;
    m_dwLastChanged |= CPCHANGE_HexMod ;
    Invalidate = true ;
  }

  CSerialPort::ScanProperties( text , Invalidate ) ;
  return true;
}

void COMCapture::TransmitText( LPCTSTR Text , int iLen )
{
  // Line delay realized in CSerialPort
  TxString( ( char * )Text , iLen ) ;
  m_dLastSent = GetHRTickCount() ;
}
