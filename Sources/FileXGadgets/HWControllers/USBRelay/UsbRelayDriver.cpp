#include "stdafx.h"
#include "UsbRelayDriver.h"




unsigned UsbRelayDriver::GetChannelState( BYTE channelId )
{
	unsigned channelVal = ~0;
	UINT allCannelsMask = 0;
	if(channelId > 0)
	{
		if(!GetHandle())
			RaiseUsbRelayErrorCallback(GetInfo().GetSerialNum(), 0,"There is no handle yet!");
		else if((allCannelsMask = GetStatesMask()) < ~0)
		{
			UINT channelMask = 1 << channelId;
			channelVal = (allCannelsMask & channelMask) >> channelId;
		}
	}
	return channelVal;
}
UINT UsbRelayDriver::GetStatesMask()
{
	UINT resultStatesMask = 0;
	
	if(!GetHandle())
		RaiseUsbRelayErrorCallback(GetInfo().GetSerialNum(), 0,"There is no handle yet!");
	else if(0 == usb_relay_device_get_status(GetHandle(), &resultStatesMask))
		resultStatesMask <<= 1; //align to the channels indexes that started from 1;
	else
	{
		RaiseUsbRelayErrorCallback(GetInfo().GetSerialNum(), 0,"Wrong relay status!");
		resultStatesMask = ~0;
	}

	return resultStatesMask;
}