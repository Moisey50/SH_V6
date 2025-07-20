// usb_relayKit.h: interface for the UsbRelayGadget class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_usb_relayKit_H__INCLUDED_)
#define AFX_usb_relayKit_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <assert.h>

#include "helpers\UserBaseGadget.h"

#include "usb_relay_device.h"

#include "usb_relay.h"
#include "IRelaysManagerListener.h"
#include "IRelayDriverListener.h"
#include "UsbRelayDriver.h"
#include "UsbRelaysManager.h"
#include "helpers/SerialPort.h"

enum USB_Relay_Mode
{
  URM_Unknown = 0 ,
  URM_HID ,
  URM_ComPort
};


class UsbRelayGadget
	: public UserBaseGadget
	, public IRelaysManagerListener
	, public IRelayDriverListener
  , public CSerialPort
{
#pragma region | Fields |

protected:
  USB_Relay_Mode  m_CommunicationMode;
	UsbRelayDriver *m_pSelectedUsbRelayDrvr;
	bool            m_bIsStateAsMask;
	bool            m_bIsCmdAsMask;
	FXString        m_szStoredSN;
  int             m_iNChannels = 0 ;
  DWORD           m_dwStates = 0 ; // bit 0 - chan 1, bit1 - chan 2...
  FXString        m_GadgetInfo ;
	
#pragma endregion | Fields |

#pragma region | Constructors |
private:
	UsbRelayGadget(const UsbRelayGadget&);
	UsbRelayGadget& operator=(const UsbRelayGadget&);
protected:
	UsbRelayGadget();
public:
	DECLARE_RUNTIME_GADGET(UsbRelayGadget);	

	virtual ~UsbRelayGadget(void)
	{
		if(m_pSelectedUsbRelayDrvr)
		{
			m_pSelectedUsbRelayDrvr->DetachRelayLisener(this);
			
			m_pSelectedUsbRelayDrvr = NULL;
		}

		if(g_pUsbRelaysDrvr)
		{
			g_pUsbRelaysDrvr->DetachRelayLisener(this);
			if(!g_pUsbRelaysDrvr->IsInUse())
			{
				delete g_pUsbRelaysDrvr;
				g_pUsbRelaysDrvr = NULL;
			}
		}
	}
#pragma endregion | Constructors |

#pragma region | Methods Private |
private:
	void SendOut(  COutputConnector* pOutConnector, const FXString &label, const FXString& data );
	void SendOut(unsigned channelIndx, BYTE bitVal);
	int GetNewValue(const FXString& command);
	void ChangeDigitalChannel(BYTE bitIndx, const FXString& command);
	void ChangeDigitalChannels(int bitsIndxMask, const FXString& command);
	void HandleFrameText(const CTextFrame& data);
	void HandleFrameQuantity( const CQuantityFrame& data );
	FXString OnGetCommand(const FXString &getCmd, int val, FXString *pLabel = NULL );
	int GetCommandVal( FXParser &pk, FXSIZE pos );
	FXString GetCmdParamValDescriptor( const FXString &cmd, const FXString &paramName, const FXString &descr, const FXString &valName = "");

#pragma endregion | Methods Private |

#pragma region | Methods Protected |
protected:
	virtual void /*IRelayListenerBase::*/OnRelayError( const string& deviceSN, int errCode, const string& errMsg ){}
	virtual void /*IRelaysManagerListener::*/OnRelaysCollectionChanged()
	{
		SelectDriver();
	}

	virtual void /*IRelayDriverListener::*/OnRelayAttachmentChanged( const string& deviceSN, bool isAttached ){ }
	virtual void /*IRelayDriverListener::*/OnRelayChanged( const string& deviceSN, unsigned uStateMask, BYTE channelID )
	{
		SendOut(channelID, (uStateMask & (1<<channelID))>>channelID);
		//switch (eventId)
		//{
		//case PHIDGET_EVENT_CHANGED_INPUT:
		//	SendOut(true, (BYTE)dwFeet, (dwStateMask & (1<<dwFeet))>>dwFeet);
		//	break;
		//case PHIDGET_EVENT_CHANGED_OUTPUT:
		//	SendOut(false, (BYTE)dwFeet, (dwStateMask & (1<<dwFeet))>>dwFeet);
		//	break;
		//case PHIDGET_EVENT_REQUESTED_CHANGE_OUTPUT:
		//	PhidgetKit_OutputChangeRequested(dwStateMask, dwFeet);
		//	break;
		//default:
		//	assert(eventId);
		//}
	}

  virtual void ShutDown()	;

	bool ScanProperties(LPCTSTR text, bool& Invalidate)
	{
    UserGadgetBase::ScanProperties( text , Invalidate );

    FXPropertyKit pc( text );

    if ( pc.GetInt( "Mode" , (int&)m_CommunicationMode ) )
      Invalidate = true ;

    switch (m_CommunicationMode)
    {
      case URM_HID:
      {
        if (g_pUsbRelaysDrvr)
          g_pUsbRelaysDrvr->Start( );

        if (pc.GetString( PROP_NAME_RELAY_DVC , m_szStoredSN ))
          SelectDriver( );

        bool boolValue = false;

        if (pc.GetBool( PROP_NAME_RELAY_STATE_AS_MASK , boolValue ) && boolValue != m_bIsStateAsMask)
          m_bIsStateAsMask = boolValue;
        boolValue = false;

        if (pc.GetBool( PROP_NAME_RELAY_CMMND_AS_MASK , boolValue ) && boolValue != m_bIsCmdAsMask)
          m_bIsCmdAsMask = boolValue;
      }
      break ;
      case URM_ComPort:
      {
        if ( m_dInitTime_ms == 0. )
        {
          CSerialPort::Create( RxProc , this );
          m_dInitTime_ms = GetHRTickCount() ;
        }
        CSerialPort::ScanProperties( text , Invalidate ) ;
        if ( _tcsstr( text , "Port" ) )
        {
          OpenConnection() ;
          if ( m_hComDev && (m_hComDev != (HANDLE)0xffffffff) )
          {
            m_iNChannels = 0 ;
            Sleep( 20 ) ;
            BYTE ff = 0xff ;
            SendBlock( &ff , 1 ) ; // request status for N channels set
          }
        }
      }
      break ;
    }

		return true;
	}

	void SelectDriver();

	bool PrintProperties(FXString& text)
	{
		UserGadgetBase::PrintProperties(text);

		FXPropertyKit pc;
		pc.WriteInt( "Mode" , (int)m_CommunicationMode ) ;
    switch( m_CommunicationMode )
    {
    case URM_HID:
      pc.WriteString( PROP_NAME_RELAY_DVC , m_szStoredSN );
      pc.WriteBool( PROP_NAME_RELAY_STATE_AS_MASK , m_bIsStateAsMask );
      pc.WriteBool( PROP_NAME_RELAY_CMMND_AS_MASK , m_bIsCmdAsMask );
      break ;
    case URM_ComPort:
      {
        FXString ComPortPars ;
        CSerialPort::PrintProperties( ComPortPars ) ;
        pc += ComPortPars ;
        break ;
      }
      break ;
    }

		text+=pc;

		return true;
	}

	bool ScanSettings(FXString& text)
	{
    FXString HeaderAndMode( "template(ComboBox(Mode(HID(1),ComPort(2)))," ) ;
    FXString devicesInCombo;
		FXString devices;

    switch( m_CommunicationMode )
    {
      case URM_HID:
      {
        devicesInCombo.Format( "%s(%d)" , "NOT_SELECTED" , PROP_INDEX_NOT_SELECTED );
        if (g_pUsbRelaysDrvr)
        {
          devices = g_pUsbRelaysDrvr->ToString( ).c_str( );
          if (m_szStoredSN.GetLength( ) > 0
            && m_szStoredSN.CompareNoCase( PROP_INDEX_NOT_SELECTED ) != 0
            && !g_pUsbRelaysDrvr->GetDeviceBySerialNum( m_szStoredSN.GetString( ) ))
          {
            devicesInCombo += LIST_DELIMITER;
            devicesInCombo.Format( "%sMISSING_DEVICE_SN#%s(%s)" , devicesInCombo , m_szStoredSN , m_szStoredSN );
          }
        }
        if (!devices.IsEmpty( ))
          devicesInCombo += LIST_DELIMITER + devices;

        text.Format( "%s(%s(%s)),"
          //"%s(%s(False(false),True(true))),"
          "%s(%s(False(false),True(true)))"
          ")"
          , SETUP_COMBOBOX , PROP_NAME_RELAY_DVC , devicesInCombo
          //, SETUP_COMBOBOX, PROP_NAME_RELAY_STATE_AS_MASK
          , SETUP_COMBOBOX , PROP_NAME_RELAY_CMMND_AS_MASK );
      }
      break ;
      case URM_ComPort:
      {
        CSerialPort::ScanSettings( text ) ;
        text += ')' ;
      }
      break ;
    }
    text.Insert( 0 , HeaderAndMode ) ;
		return true;
	}
#pragma endregion | Methods Protected |

#pragma region | Methods Public |
public:
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);	
  bool IsRelayModuleConnected()
  {
    if ( !m_pSelectedUsbRelayDrvr )
    {
      SENDERR( "Relay Module is not connected" ) ;
      return false ;
    }
    return true ;
  }

  // Receive from COM port
  static BOOL UsbRelayGadget::RxProc( LPVOID pParam , char *Data , int iLen ) ;
  // Retrieve number of relays on com port based board
  int GetChannelNumber( FXString& AsText ) ;
  #pragma endregion | Methods Public |
};
#endif // !defined(AFX_usb_relayKit_H__INCLUDED_)