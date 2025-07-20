// UsbRelayGadget.cpp: implementation of the UsbRelayGadget class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <assert.h>

#include <helpers\FramesHelper.h>
#include "usb_relay.h"
#include "UsbRelayGadget.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

USER_FILTER_RUNTIME_GADGET( UsbRelayGadget , "HWControllers" );

char const hex_chars[ 16 ] = { '0' , '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , 'A' , 'B' , 'C' , 'D' , 'E' , 'F' };


BOOL RxProc( LPVOID pParam , char *Data , int iLen )
{
  return ( ( UsbRelayGadget* ) pParam )->RxProc( pParam , Data , iLen );
}

int UsbRelayGadget::GetChannelNumber( FXString& AsText )
{
  int iPos = AsText.Find( "CH" ) ;
  int iCRPos = AsText.Find( '\r' , iPos + 1 ) ;
  int iLen = AsText.GetLength() ;
  while ( ( iPos >= 0 ) && ( iCRPos > 0 ) )
  {
    int iChanNum = atoi( ( ( LPCTSTR ) AsText ) + 2 ) ;
    if ( iChanNum > m_iNChannels )
      m_iNChannels = iChanNum ;
    
    if ( AsText.Find( "ON" ) > 0 )
      m_dwStates |= ( 1 << (iChanNum - 1 ) ) ;
    else if ( AsText.Find( "OFF" ) > 0 )
      m_dwStates &= ~( 1 << ( iChanNum - 1 ) ) ;
    if ( m_GadgetInfo.IsEmpty() )
      GetGadgetName( m_GadgetInfo ) ;
    FXString View = AsText.Mid( iPos , iCRPos - iPos ) ;
    FxSendLogMsg( MSG_INFO_LEVEL , m_GadgetInfo , 0 ,  "%s" , ( LPCTSTR ) View ) ;

    iPos = AsText.Find( "CH" , iCRPos ) ;
    if ( iPos > 0 )
      iCRPos = AsText.Find( '\r' , iPos + 1 ) ;
  }
  return m_iNChannels ;
}

BOOL UsbRelayGadget::RxProc( LPVOID pParam , char *Data , int iLen )
{
  if ( !pParam )
    return FALSE ;

  UsbRelayGadget * pGadget = ( UsbRelayGadget * ) pParam ;

  CTextFrame * pOutText = CTextFrame::Create() ;
  if ( pOutText )
  {
    FXString& OutText = pOutText->GetString() ;
    LPTSTR  pOutString = OutText.GetBufferSetLength( iLen + 1 ) ;
    memcpy( pOutString , Data , iLen ) ;
    pOutString[ iLen ] = 0 ;
    OutText.ReleaseBuffer( iLen + 1 ) ;
    pGadget->GetChannelNumber( OutText ) ;
    pOutText->SetAbsTime( GetHRTickCount() ) ;
    pOutText->SetLabel( "Received From Relay Board" ) ;
    PutFrame( pGadget->m_pOutput , pOutText , 50 ) ;
  }
//   m_dLastReceivingSpent_ms += GetHRTickCount( ) - m_dLastReceivedTime;
  return true;
}

UsbRelayGadget::UsbRelayGadget()
  : m_CommunicationMode( URM_HID )
  , m_pSelectedUsbRelayDrvr( NULL )
  , m_bIsStateAsMask( false )
  , m_bIsCmdAsMask( false )
  , m_szStoredSN( PROP_INDEX_NOT_SELECTED )
  , CSerialPort( 0 )
{
  init();

  switch ( m_CommunicationMode )
  {
    case URM_Unknown: break;
    case URM_HID:
    {
      if ( !g_pUsbRelaysDrvr )
        g_pUsbRelaysDrvr = new UsbRelaysManager();
      g_pUsbRelaysDrvr->Init( ( LPIRelaysManagerListener ) this );
      //if(!g_pUsbRelaysDrvr->IsRunning())
      g_pUsbRelaysDrvr->Start();
    }
    break;
    case URM_ComPort:
    {
    }
    break;
  }
}

#pragma region | Methods Private |

void UsbRelayGadget::SendOut( COutputConnector* pOutConnector , const FXString &label , const FXString& data )
{
  if ( pOutConnector )
  {
    CTextFrame* pTxtFrm = CTextFrame::Create( data );
    pTxtFrm->ChangeId( NOSYNC_FRAME );
    pTxtFrm->SetLabel( label );
    pTxtFrm->SetTime(GetGraphTime() * 1.e-3);

    if ( !pOutConnector->Put( pTxtFrm ) )
      pTxtFrm->RELEASE( pTxtFrm );
  }
}
void UsbRelayGadget::SendOut( unsigned channelIndx , BYTE channelVal )
{
  FXString txtData;
  txtData.Format( "Value=%d;Chan=%d;" , channelVal , channelIndx );
  FXString label;
  label.Format( "Ch%d" , channelIndx );
  SendOut( m_pOutput , label , txtData );
}
int UsbRelayGadget::GetNewValue( const FXString& command )
{
  int chVal = DIGITAL_COMMAND_VAL_UNDEF;
  if ( command.CompareNoCase( DIGITAL_COMMAND_NAME_ON ) == 0 )
    chVal = DIGITAL_COMMAND_VAL_ON;
  else if ( command.CompareNoCase( DIGITAL_COMMAND_NAME_OFF ) == 0 )
    chVal = DIGITAL_COMMAND_VAL_OFF;
  return chVal;
}
void UsbRelayGadget::ChangeDigitalChannel( BYTE channelIndx , const FXString& command )
{
  int channelVal = GetNewValue( command );
  if ( m_pSelectedUsbRelayDrvr && channelIndx >= 0 && channelIndx <= m_pSelectedUsbRelayDrvr->GetInfo().GetNumChannels() )
  {
    if ( channelVal >= DIGITAL_COMMAND_VAL_OFF )
      m_pSelectedUsbRelayDrvr->SetChannel( channelIndx , channelVal );
    else if ( command.CompareNoCase( DIGITAL_COMMAND_NAME_TOGGLE ) == 0 )
      m_pSelectedUsbRelayDrvr->ToggelChannel( channelIndx );
  }
}
void UsbRelayGadget::ChangeDigitalChannels( int channelsIndxMask , const FXString& command )
{
  if ( m_pSelectedUsbRelayDrvr && TRUE == m_bIsCmdAsMask )
  {
    channelsIndxMask <<= 1; //align to channels indexes that started from 1;
    int channelsQty = m_pSelectedUsbRelayDrvr->GetInfo().GetNumChannels();
    for ( int channelIndx = 1; channelIndx <= channelsQty; channelIndx++ )
    {
      int chMask = 1 << channelIndx;
      if ( channelsIndxMask & chMask )
      {
        ChangeDigitalChannel( channelIndx , command );
      }
    }
  }
}
void UsbRelayGadget::HandleFrameText( const CTextFrame& data )
{
  FXString command = ( ( FXString ) data.GetLabel() ).Trim();

  if ( GetNewValue( command ) != DIGITAL_COMMAND_VAL_UNDEF )
  {       // Command is in label
    FXSIZE delimiterIndx = 0;
    FXString channelsIndxs = data.GetString();
    bool isHex = ( channelsIndxs.Find( "0x" ) == 0 || channelsIndxs.Find( "0X" ) == 0 );

    if ( TRUE == m_bIsCmdAsMask && isHex )
    {
      char *eptr;
      ChangeDigitalChannels( strtoul( channelsIndxs , &eptr , 16 ) , command );
    }
    else if ( FALSE == m_bIsCmdAsMask && !isHex )
    {
      while ( channelsIndxs.GetLength() > delimiterIndx )
      {
        FXString chIndxTxt = channelsIndxs.Tokenize( LIST_DELIMITER , delimiterIndx );
        ChangeDigitalChannel( atoi( chIndxTxt ) , command );
      }
    }
  }
  else // command and value are in text, command is not in label
  {
    FXPropertyKit pk( data.GetString() );
    int iChan , iVal;
    int iChanPos = ( int ) pk.Find( "Chan=" );
    int iValPos = ( int ) pk.Find( "Value=" );
    int iTogglePos = ( int ) pk.Find( "Toggle" );
    int iSendPos = ( int ) pk.Find( "Status" );
    if ( iSendPos >= 0 )
    {
      if ( m_CommunicationMode == URM_ComPort )
      {
        BYTE Request = 0xff ;
        SendBlock( &Request , 1 );
      }
    }
    if ( iChanPos >= 0 )
    {
      iChan = atoi( ( LPCTSTR ) pk + iChanPos + 5 );
      if ( iValPos >= 0 )
      {
        iVal = atoi( ( LPCTSTR ) pk + iValPos + 6 );
        switch ( m_CommunicationMode )
        {
          case URM_HID:
          {
            m_pSelectedUsbRelayDrvr->SetChannel( iChan , iVal );
          }
          break ;
          case URM_ComPort:
          {
            if ( !m_iNChannels || ( iChan <= m_iNChannels ) )
            {
              BYTE Buf[ 4 ] = { 0xa0 , ( BYTE ) iChan , ( BYTE ) iVal , ( BYTE ) ( 0xa0 + iChan + iVal ) } ;
              if ( !IsConnectionOpen() )
              {
                PortAbort() ;
                CSerialPort::Create( RxProc , this );
              }

              if ( IsConnectionOpen() )
              {
                SendBlock( Buf , 4 );
                m_dwStates &= ~( 1 << (iChan - 1) ) ;
                if ( iVal != 0 )
                  m_dwStates |= ( 1 << ( iChan - 1 ) ) ;
                else
                  m_dwStates &= ~( 1 << ( iChan - 1 ) ) ;
              }
              else
                SEND_GADGET_ERR( "Can't open port %s" , GetPortName( m_iSelectedPort ) ) ;
            }
//             if ( !m_iNChannels )
//             {
//               Sleep( 20 ) ;
//               BYTE ff = 0xff ;
//               SendBlock( &ff , 1 ) ; // request status for N channels set
//             }
           }
        }
      }
      else if ( iTogglePos >= 0 )
        m_pSelectedUsbRelayDrvr->ToggelChannel( iChan );
    }
  }
}
void UsbRelayGadget::HandleFrameQuantity( const CQuantityFrame& data )
{
  if ( m_pSelectedUsbRelayDrvr )
  {
    FXString command = ( ( FXString ) data.GetLabel() ).Trim();
    long channelsIndxs = data.operator long();

    if ( TRUE == m_bIsCmdAsMask )
      ChangeDigitalChannels( channelsIndxs , command );
    else if ( channelsIndxs >= 0 && channelsIndxs <= m_pSelectedUsbRelayDrvr->GetInfo().GetNumChannels() )
      ChangeDigitalChannel( ( BYTE ) channelsIndxs , command );
  }
}
FXString UsbRelayGadget::OnGetCommand( const FXString &getCmd , int val , FXString *pLabel /*= NULL */ )
{
  FXString channelsStatesTxt;
  unsigned channelsStates = 0;
  bool isSingle = false;
  if ( (m_CommunicationMode == URM_HID) &&  !m_pSelectedUsbRelayDrvr )
    return "";
  else
  {
    if ( val > 0 && getCmd.CompareNoCase( DUPLEX_COMMAND_PARAM_CHANNEL ) == 0 )
    {
      switch ( m_CommunicationMode )
      {
        case URM_HID: 
          channelsStates = m_pSelectedUsbRelayDrvr->GetChannelState( val );
          break ;
        case URM_ComPort:
          channelsStates = ((m_dwStates & ( 1 << (val - 1) )) != 0) ;
          break ;
      }
    }
    else if ( val == 0 || getCmd.CompareNoCase( DUPLEX_COMMAND_PARAM_CHANNELS ) == 0 )
    {
      switch ( m_CommunicationMode )
      {
        case URM_HID:
          channelsStates = m_pSelectedUsbRelayDrvr->GetStatesMask();
          break ;
        case URM_ComPort:
          channelsStates = m_dwStates ;
          break ;
      }
    }

    FXString channelsState;

    channelsState.Format( "ValAll=0x%x(%d);" , channelsStates , channelsStates );

    if ( val > 0 )
    {
      if ( ((m_CommunicationMode == URM_HID ) && (val > m_pSelectedUsbRelayDrvr->GetInfo().GetNumChannels()))
        || ((m_CommunicationMode == URM_ComPort) && val > m_iNChannels) )
        channelsState = "Wrong Channel Id;";
      else
        channelsState.Format( "Ch%d; ChVal=%d;" , val , channelsStates );
    }

    if ( !pLabel )
      channelsStatesTxt = channelsState;
    else
    {
      *pLabel = "AllChannels";
      if ( val > 0 )
      {
        pLabel->Format( "Ch%d" , val );

        if ( val > m_pSelectedUsbRelayDrvr->GetInfo().GetNumChannels() )
          channelsState = "Wrong Channel Id;";
        else
          channelsState.Format( "ChVal=%d;" , channelsStates );
      }
      channelsStatesTxt = channelsState;
    }

    return channelsStatesTxt.IsEmpty() ? "Error; Enter Any key for list of commands." : channelsStatesTxt;
  }
}

int UsbRelayGadget::GetCommandVal( FXParser &pk , FXSIZE pos )
{
  int bitVal = -1;
  FXString val;
  if ( pk.GetParamString( pos , val ) )
    bitVal = atoi( val );
  return bitVal;
}
FXString UsbRelayGadget::GetCmdParamValDescriptor( const FXString &cmd , const FXString &paramName , const FXString &descr , const FXString &valName /*= ""*/ )
{
  FXString cmdParamVal , val;
  if ( !valName.IsEmpty() )
    val.Format( "(<%s>)" , valName );
  cmdParamVal.Format( "%s %s%s - %s;\n" , cmd , paramName , valName.IsEmpty() ? "" : val , descr );//"get channels - returns back all channels states\r\n as mask;\n")
  return cmdParamVal;
}
#pragma endregion | Methods Private |

#pragma region | Methods Protected |

void UsbRelayGadget::ShutDown()
{
  PortAbort() ;
  UserBaseGadget::ShutDownBase() ;
}

void UsbRelayGadget::SelectDriver()
{
  if ( m_szStoredSN.GetLength() == 0 )
  {
    if ( m_pSelectedUsbRelayDrvr )
    {
      m_pSelectedUsbRelayDrvr->DetachRelayLisener( this );
      m_pSelectedUsbRelayDrvr = NULL;
    }
  }
  else if ( g_pUsbRelaysDrvr )
  {
    UsbRelayDriver *pCurrentDrvr = m_pSelectedUsbRelayDrvr;
    FXString currentSN = m_pSelectedUsbRelayDrvr ? m_pSelectedUsbRelayDrvr->GetInfo().GetSerialNum().c_str() : m_szStoredSN.GetString();
    UsbRelayDriver *pRequestedDrvr = g_pUsbRelaysDrvr->GetDeviceBySerialNum( m_szStoredSN.GetString() );
    if ( !pRequestedDrvr )
      m_pSelectedUsbRelayDrvr = NULL;

    if ( m_pSelectedUsbRelayDrvr && m_pSelectedUsbRelayDrvr->GetInfo().GetSerialNum().compare( m_szStoredSN ) != 0 )
    {
      m_pSelectedUsbRelayDrvr->DetachRelayLisener( this );
      m_pSelectedUsbRelayDrvr = NULL;
    }

    if ( !m_pSelectedUsbRelayDrvr && pRequestedDrvr )
    {
      m_pSelectedUsbRelayDrvr = pRequestedDrvr;
      if ( m_pSelectedUsbRelayDrvr->IsInUse() )
      {
        USB_RELAY_SENDWARN_1
        ( "The '%s' device is already in use by other gadget."
          , m_pSelectedUsbRelayDrvr->ToString().c_str() );

        //string newGdgtName("other");
        //Phidget_0_16_16 * gdgtHost = reinterpret_cast<Phidget_0_16_16*>(m_pSelectedIFKDrvr->GetHost());
        //FXString name;
        //if(gdgtHost && gdgtHost->GetGadgetName(name))
        //	newGdgtName = name;

        //FXString msg;
        //msg.Format("The '%s' device is already in use by %s gadget.\nClick 'OK' to re-attach it to this gadget, otherwise - 'Cancel'.
        //	, m_pSelectedIFKDrvr->ToString().c_str()
        //	, newGdgtName);

        //if(IDOK==MessageBox(NULL, msg, "Phidget Device Select", MB_APPLMODAL | MB_ICONWARNING | MB_OKCANCEL ))
        //{
        //	//RemoveFromHost(m_pSelectedIFKDrvr);
        //}
        //else
        //{
        //	m_iStoredSN = currentSN;
        //	m_pSelectedIFKDrvr = pCurrentDrvr;							
        //}
      }

      if ( m_pSelectedUsbRelayDrvr )
      {
        m_pSelectedUsbRelayDrvr->Init( ( LPIRelayDriverListener ) this );
        m_pSelectedUsbRelayDrvr->Start();
      }
    }

    if ( pCurrentDrvr && pCurrentDrvr != m_pSelectedUsbRelayDrvr )
    {
      pCurrentDrvr->DetachRelayLisener( ( LPIRelayDriverListener ) this ) , pCurrentDrvr = NULL;
    }
  }
}


#pragma endregion | Methods Protected |

#pragma region | Methods Public |
void UsbRelayGadget::PropertiesRegistration()
{
  //ATTENTION!
  //All properties created and registered manually
  //in Threads Safe mode
}

void UsbRelayGadget::ConnectorsRegistration()
{
  //addInputConnector( createComplexDataType(2, text, quantity), "DigChannelCmd");
  addInputConnector( transparent , "DigChannelCmd" );
  addDuplexConnector( transparent , transparent , "Control" );
  //addOutputConnector( createComplexDataType(2, text, quantity) , "DigitalState");
  addOutputConnector( transparent , "DigitalState" );
}

// Commands as text frame
// 1. If mode is not mask:
//   if Label=="On" or Label == "Off" or Label == "toggle"
//      the channel number will be extracted from text frame content 
//      and appointed channel will be set or reset or toggled
//   else
//     Text frame content will be used as property kit
//     Channel will be extracted as "Chan", value will be extracted as "Value"
//     If "Value" doesn't found and "Toggle
// 2. If mode is mask, all channels will be settled to proper values as bits
//      in text frame content
// 
//
// Commands as quantity frame are the same - label is operation, content is value

CDataFrame* UsbRelayGadget::DoProcessing( const CDataFrame* pDataFrame )
{
  if ( pDataFrame != NULL && !pDataFrame->IsContainer() )
  {
    const CTextFrame* pText = pDataFrame->GetTextFrame();

    if ( pText != NULL )
    {
      switch ( m_CommunicationMode )
      {
        case URM_HID:
          if ( IsRelayModuleConnected() )
            HandleFrameText( *pText );
          break ;
        case URM_ComPort:
          HandleFrameText( *pText );
          break ;
      }
    }
    else
    {
      const CQuantityFrame* pQuantity = pDataFrame->GetQuantityFrame();

      if ( pQuantity )
      {
        switch ( m_CommunicationMode )
        {
          case URM_HID:
            if ( IsRelayModuleConnected() )
              HandleFrameQuantity( *pQuantity );
            break;
          case URM_ComPort:
            HandleFrameQuantity( *pQuantity );
            break;
        }
      }
    }
  }
  return ( CDataFrame* ) NULL;
}

void UsbRelayGadget::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return;

  CTextFrame* tf = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( tf )
  {
    FXParser pk = ( LPCTSTR ) tf->GetString();
    FXString cmd;
    FXString param;
    FXSIZE pos = 0;
    pk.GetWord( pos , cmd );
    FXString label;
    switch ( m_CommunicationMode )
    {
      case URM_HID:
      {

        if ( ( cmd.MakeLower().CompareNoCase( "getDevices" ) == 0 ) )
        {
          FXString devs = g_pUsbRelaysDrvr->ToString().c_str();
          if ( devs.GetLength() == 0 )
            devs = "There are NO devices connected!";
          pk = devs;
          SendOut( pConnector , label , pk );
        }
        else if ( ( cmd.MakeLower().CompareNoCase( DUPLEX_COMMAND_NAME_GET ) == 0 ) && pk.GetWord( pos , param ) )
        {
          pk = OnGetCommand( param.MakeLower() , ( int ) GetCommandVal( pk , pos ) );
          SendOut( pConnector , label , pk );
        }
        else if ( ( cmd.CompareNoCase( DUPLEX_COMMAND_NAME_GET_OUT ) == 0 ) && pk.GetWord( pos , param ) )
        {
          if ( m_pOutput )
          {
            pk = OnGetCommand( param.MakeLower() , ( int ) GetCommandVal( pk , pos ) , &label );
            SendOut( m_pOutput , label , pk );
          }
        }
        else
        {
          pk.Empty();
          pk = "List of 5 available commands:\r\n";
          pk += GetCmdParamValDescriptor( "getDevices" , "" , "returns names of all available devices." , "" );
          pk += GetCmdParamValDescriptor( DUPLEX_COMMAND_NAME_GET , DUPLEX_COMMAND_PARAM_CHANNELS , "returns back all channels states\r\n as a mask." , "" );
          pk += GetCmdParamValDescriptor( DUPLEX_COMMAND_NAME_GET , DUPLEX_COMMAND_PARAM_CHANNEL , "returns back the state of\r\n the specified channel." , "channelIndex" );
          pk += GetCmdParamValDescriptor( DUPLEX_COMMAND_NAME_GET_OUT , DUPLEX_COMMAND_PARAM_CHANNELS , "returns all channels states as\r\n a mask to the out pin" , "" );
          pk += GetCmdParamValDescriptor( DUPLEX_COMMAND_NAME_GET_OUT , DUPLEX_COMMAND_PARAM_CHANNEL , "returns the specified channel\r\n state to the out pin" , "channelIndex" );
          SendOut( pConnector , label , pk );
        }
      }
      break ;
      case URM_ComPort:
      {
        if ( ( cmd.MakeLower().CompareNoCase( DUPLEX_COMMAND_NAME_GET ) == 0 ) && pk.GetWord( pos , param ) )
        {
          pk = OnGetCommand( param.MakeLower() , ( int ) GetCommandVal( pk , pos ) );
          SendOut( pConnector , label , pk );
        }

      }
    }
  }
  pParamFrame->Release( pParamFrame );
}
#pragma endregion | Methods Public |