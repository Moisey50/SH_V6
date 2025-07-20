// SerIO.cpp: implementation of the CCSerIO class.
//
//////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include <afxmt.h>
#include "SerIO.h"
#include "get_time.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define NCHANNELS 16
CCSerIO * Channels[ NCHANNELS ];
//CEvent SerIOEvents[ NCHANNELS ];

CCSerIO::CCSerIO( int Port , int Baud , int Mode , int Flow )
{
  m_Port = -1;
  m_OutBuf = NULL ;
  Reprogram( Port , Baud , Mode , Flow );
  
  if (m_Port > 0)
  {
    m_AutoFlush = 0;
    m_OutBuf = new char[ 100 ];
    m_OutBufLen = 100;
    m_OutBufCnt = 0;
    m_NHandShaked = 0;

    m_SleepTime = 10.;

    m_NewSource = 0;
    m_BetweenActivityTime = 2000.;  // in ms
    m_LastInterruptMoment = 0. ;

    m_Close = 0;
    m_Event = CreateEvent( FALSE , FALSE , 0 , NULL );

    m_ControlWindow = NULL ;
  }
}

CCSerIO::~CCSerIO()
{
   if ( m_Port >= 0 )
   {
     sio_close( m_Port );
     Sleep( 10 );
   }

   if ( m_OutBuf )
     delete[  ] m_OutBuf;

   m_OutBuf = NULL ;

   CloseHandle( m_Event ) ;
   m_Close = 1;
}

int CCSerIO::GetBaud()
{
  return sio_getbaud( m_Port );
}

int CCSerIO::GetMode()
{
  return sio_getmode( m_Port );
}


int CCSerIO::GetInputLen()
{
  return sio_iqueue( m_Port );
}

int CCSerIO::GetInputData(BYTE * buf, int BufLen)
{
  int InBuffer = 0;
  while ( (GetInputLen() > 0) && ( InBuffer < BufLen ) ) 
  {
    int InChar = sio_getch( m_Port );
    if ( InChar >= 0 )
      buf[ InBuffer++ ] = ( BYTE )InChar;
    else
      return InChar;
  };
  
  return InBuffer;
}

int 
CCSerIO::SendData( 
  const char * buf, int BufLen , int delay )
{
//  m_SendStartMoment = get_current_time();

  if ( BufLen )
    m_IsAck = 0 ;

  if ( delay >= 0 )
    m_InOutBuf = m_OutBufCnt = 0;

  if ( !delay )
    return sio_write( m_Port , ( char* )buf , BufLen );
  else
  {
    if ( delay > 0 )
    {
      CString tmp = m_AutoEcho;
      m_AutoEcho.Empty();
      for ( int OutCnt = 0 ; OutCnt < BufLen ; OutCnt++ )
      {
        int Oq = sio_oqueue( m_Port );
        int Iq = sio_iqueue( m_Port );
        int Err = sio_write( m_Port , 
                    ( char * )&buf[ OutCnt ] , 1 );
        if ( Err < 0 )
          return Err;
        else if ( delay ) 
          Sleep( delay );
      }
      m_AutoEcho = tmp;
      return BufLen;
    }
    else  // Transmission with HandShake for each char
    {
      int Attempt = 0;
      int OldCnt = m_OutBufCnt;
      while ( m_OutBufCnt > 0 )
      {
        if ( ++Attempt < 30 )
        {
          Sleep( 5 );
          if ( OldCnt == m_OutBufCnt )
            m_OutBufCnt = 0;
          else
            OldCnt = m_OutBufCnt;
        }
        else
          return -15; // Channel Busy
      }

      sio_flush( m_Port , 2 );  // Flush all buffers

      if ( OldCnt )
      {
        Sleep( ( int )m_SleepTime );
      }
      if ( m_OutBufLen < BufLen )
      {
        delete[  ] m_OutBuf;
        m_OutBuf = new char[ BufLen ];
        m_OutBufLen = BufLen;
      }
      memcpy( m_OutBuf , buf , BufLen );
      m_InOutBuf = BufLen ;
      int Err = sio_write( m_Port , &m_OutBuf[ 0 ] , 1 );
      if ( Err < 0 )
        return Err;
      else
      {
        m_OutBufCnt = 1;
        m_NHandShaked = 0;
      }
      return m_OutBufCnt;
    }
  }
}


int CCSerIO::GetPort()
{
  return m_Port;
}

int CCSerIO::Reprogram(int Port, int Baud, int Mode , int Flow )
{
  if ( Port < 0 )
    return -10;

  int OldPort = m_Port;
  
  if ( m_Port >= 0 )
  {
    sio_close( m_Port );
    Sleep( 10 );
    Channels[ m_Port ] = NULL;
  }

  int res = sio_open( Port );
  if ( res )
  {
    if ( res == -8 )
    {
      char Msg[ 200 ];
      FormatMessage( 
          FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL,
          m_LastError = GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
          Msg,
          199,
          NULL 
      );
      MessageBox( NULL, Msg , "Serial IO Error on Reprogramming", MB_OK | MB_ICONINFORMATION );
    }
    m_Port = res;
    return res;
  }

  m_Port = Port;
  Channels[ m_Port ] = this;

  switch ( Baud )
  {
    case 50: Baud = 0; break;
    case 75: Baud = 1; break;
    case 110: Baud = 2; break;
    case 150: Baud = 4; break;
    case 300: Baud = 5; break;
    case 600: Baud = 6; break;
    case 1200: Baud = 7; break;
    case 1800: Baud = 8; break;
    case 2400: Baud = 9; break;
    case 4800: Baud = 10; break;
    case 7200: Baud = 11; break;
    case 9600: Baud = 12; break;
    case 19200: Baud = 13; break;
    case 38400: Baud = 14; break;
    case 57600: Baud = 15; break;
    case 115200: Baud = 16; break;
    case 230400: Baud = 17; break;
    //case 460800: Baud = 18; break;
    //case 921600: Baud = 19; break;
    default: Baud = 12; break;
  }
  sio_ioctl( m_Port , Baud , Mode );
  sio_flowctrl( m_Port , Flow ); // no flow control

  m_IsAck = 0;
  m_InAckSequence = 0;
  //m_In = "T";
  //m_Out = "T";
  //m_Ack = "!A";
  m_TimeOut = 300;
  m_AutoAnswered = 0;
  //m_SendAfter = "!EON\r";
  return OldPort * 1000;
}


int CCSerIO::SendChar(int Char, int delay_ms)
{
  int res = sio_write( m_Port , ( char* )&Char , 1 );

  m_IsAck = 0 ;
  if ( delay_ms )
    Sleep( delay_ms );
  return res;
}



//*************************************************************
//  Next two functions - for callbacks

void /*CALLBACK*/
ReceiveIrqFunc(int Port)
{
  CCSerIO * Obj = Channels[ Port ];
  char buf[ 100 ];   // !!!!! dangerous
  int InLen = sio_iqueue( Port );
  int Len = ( InLen > 99 ) ? 99 : InLen ;

  int HandShake = 0;
  if ( Len )
  {

    double t = get_current_time(  );

    if ( t - Obj->m_LastInterruptMoment < Obj->m_BetweenActivityTime )
      Obj->m_NewSource = 0;

    if ( Obj->m_OutBufCnt > 0 )  // Exchange with HandShake
    {
      if ( Len > 1 )  // Something Strange
      {
        if ( Obj->m_OutBufCnt == Obj->m_InOutBuf )
        {
          sio_getch( Port );
          Len--;
          Obj->m_OutBufCnt = 0; // Finished
          Obj->m_SendTime = ( Obj->m_SendFinishMoment = t ) 
            - Obj->m_SendStartMoment;
          HandShake = sio_getch( Port ) ;
          Len-- ;
          if ( HandShake == 13   ||   HandShake == 10 )
            HandShake = 0;
        }
        else
        {
          for ( int i = 0 ; i < Len ; i++ )
            buf[ i ] = sio_getch( Port );

          sio_flush( Port , 0 );  // flush in buffer
          Obj->m_OutBufCnt = -1;
        }
      }
      else
      {
        sio_getch( Port );
        Len--;
        if ( Obj->m_OutBufCnt < Obj->m_InOutBuf )
          sio_write( Port , &Obj->m_OutBuf[ Obj->m_OutBufCnt++ ] , 1 );
        else
        {
          Obj->m_OutBufCnt = 0 ; // We did finish send operation
          Obj->m_SendTime = ( Obj->m_SendFinishMoment = t ) 
            - Obj->m_SendStartMoment;
        }
      }
    }
    if ( Obj->m_OutBufCnt <= 0   &&   Len )
    {
      if ( HandShake )
        buf[ 0 ] = HandShake;
      int i = ( HandShake )? 1 : 0 ;
      for (  ; i < Len ; i++ )  
      {
        int Char = sio_getch( Port );
        if ( Char >= 0 )
          buf[ i ] = ( char )toupper( Char ) ;
        else
          break;
      }
      buf[ i ] = 0;
      if ( InLen != Len  &&  Obj->GetAutoFlush() < InLen )
        sio_flush( Port , 0 );
      if ( Obj->m_In == buf )
      {
        sio_write( Port , ( char * )( LPCTSTR )( Obj->m_Out ) ,
          Obj->m_Out.GetLength() );
        sio_flush( Port , 0 ); // flush in buffer

        SetEvent( Obj->m_Event );
        Obj->m_AutoAnswered = 0;
        Obj->m_NewSource = 1;
        Obj->m_InAckSequence = Obj->m_IsAck = 0;
        Obj->m_LastInterruptMoment = t;
        return;
      }
      else
      {
        if ( t - Obj->m_LastInterruptMoment > Obj->m_BetweenActivityTime 
             && ! Obj->m_AutoEcho.IsEmpty() )
        {
          //Obj->SendData( ( LPCTSTR )Obj->m_Out , Obj->m_Out.GetLength() , 0 );
          SetEvent( Obj->m_Event );
          Obj->m_AutoAnswered = 0;
          Obj->m_NewSource = 1;
          Obj->m_InAckSequence = Obj->m_IsAck = 0;
          Obj->m_LastInterruptMoment = t;
          return;
        }
      }
      if ( Obj->m_InAckSequence )
      {
        if ( toupper( buf[ 0 ] ) == toupper( Obj->m_Ack[ 1 ] ) )
        {
          Obj->m_AckTime = ( Obj->m_AckMoment = t ) 
            - Obj->m_SendStartMoment;
          Obj->m_IsAck = 1;
        }
        Obj->m_InAckSequence = 0 ;
      }
      if ( ! Obj->m_IsAck )
      {
        int i = 0 ;
        for (  ; i < Len ; i++ )
        {
          if ( toupper( buf[ i ] ) == toupper( Obj->m_Ack[ 0 ] ) )
            break;
        }
        if ( i < Len - 1 )
        {
          if ( toupper( buf[ i + 1 ] ) == toupper( Obj->m_Ack[ 1 ] ) )
          {
            Obj->m_AckTime = ( Obj->m_AckMoment = t ) 
              - Obj->m_SendStartMoment;
            Obj->m_IsAck = 1;
          }
        }
        else if ( i < Len )
            Obj->m_InAckSequence = 1;
      }
    }
    Obj->m_LastInterruptMoment = t;
  }
}

//**************************************************************

int CCSerIO::SetReceiveIrqReaction(
  int Install, int TimeOut , 
  CString * In, CString * Out , CString * Ack )
{
  if ( m_SetReceiveReaction = Install )
  {
    m_In = ( In )? *In : "T" ;
    m_Out = ( Out )? *Out : "T" ;
    m_Ack = ( Ack )? *Ack : "!A" ;
    if ( TimeOut )
      m_TimeOut = TimeOut;
    return ! sio_cnt_irq( m_Port , ReceiveIrqFunc , m_In.GetLength() );
  }
  else
    return ! sio_cnt_irq( m_Port , NULL , 1 );
}


int CCSerIO::IsAck()
{
  //if ( !m_IsAck )
  //  ReceiveIrqFunc( m_Port );

  return m_IsAck;
}

void CCSerIO::SetAutoFlush( int MaxLen )
{
  m_AutoFlush = MaxLen ; 
}

int CCSerIO::GetAutoFlush()
{
  return m_AutoFlush ;
}

int CCSerIO::GetNSentWithHandShake()
{
  return m_OutBufCnt;
}

double CCSerIO::GetAcknowledgeTime()
{
  return m_AckTime;
}

int CCSerIO::WaitAcknowledge(double TimeOut_ms)
{
  double Start = get_current_time(  );
  while ( ! IsAck() )
  {
    if ( get_current_time() - Start > TimeOut_ms )
      return 0;
    Sleep( 20 );
  }
  return 1;
}

int 
CCSerIO::WaitForSendFinish(double SleepAfter)
{
  double Start = get_current_time(  );
  while ( m_OutBufCnt )
  {
    if ( get_current_time() - Start > 2000. )
      return 0;
    Sleep( 10 );
  }
  Sleep( ( int )SleepAfter );
  return 1;
}

void 
CCSerIO::SetSendAfter( CString & Msg , int TimeOut )
{
  m_SendAfter = Msg;
  m_TimeOut = TimeOut;
}

int 
CCSerIO::GetTimeForSend(int BufLen)
{
  int Baud = sio_getbaud( m_Port );
  return ( ( ( BufLen + 2 ) * 12 * 1000 ) / Baud );
}

void /*CALLBACK*/
TermIrqFunction( int Port )
{
  CCSerIO * Obj = Channels[ Port ];
  int InLen = sio_iqueue( Port );
  
  int Len = ( InLen > 99 ) ? 99 : InLen ;

  if ( Len )
  {

    double TimeNow = get_current_time() ;
//     Obj->m_CriticalSection.Lock() ;
    Obj->m_InBufCnt = sio_read( Port , Obj->m_InBuf , Len );
    Obj->m_InBuf[ Obj->m_InBufCnt ] = 0 ;
    sio_flush( Port , 0 ) ;
    if ( Obj->m_ControlWindow  &&  strlen( Obj->m_InBuf ) > 5 )
    {
      sprintf( &( Obj->m_InBuf[ Obj->m_InBufCnt - 2 ] ) , "    T=%12.3f ms" , TimeNow ) ;
      Obj->m_InBufCnt = strlen( Obj->m_InBuf ) ;
//       Obj->m_CriticalSection.Unlock() ;
      char * pOut = new char[Obj->m_InBufCnt] ;
      memcpy( pOut , Obj->m_InBuf , Obj->m_InBufCnt + 1 ) ;
      
      Obj->m_ControlWindow->SendMessage( 
        SERIO_FLAG_CHAR_MSG , Obj->m_InBufCnt , ( DWORD )pOut  ) ;
    }
//     else
//       Obj->m_CriticalSection.Unlock() ;
    SetEvent( Obj->m_Event ) ;
  }
}

int 
CCSerIO::SetFlagChar(char FlagChar, int Set)
{
  if ( Set )
    return sio_term_irq( m_Port , TermIrqFunction , FlagChar ) ;
  else
    return sio_term_irq( m_Port , NULL , FlagChar ) ;
}

int 
CCSerIO::SendData(CString &Data)
{
  return SendData( ( LPCTSTR )Data , Data.GetLength() ) ;
}

void 
CCSerIO::SetControlWindow(CWnd *pWnd)
{
  m_ControlWindow = pWnd ;
}
