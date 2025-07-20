#include "stdafx.h"
#include "UsbRelayDriversCollection.h"




const UsbRelayDriver* UsbRelayDriversCollection::GetDeviceBySerialNum( const string& deviceSN )
{
	UsbRelayDriver* pDevice = NULL;
	m_Locker.Lock();
	map<string, UsbRelayDriver*>::const_iterator ci = m_BusBySerialNum.find(deviceSN);
	if(ci != m_BusBySerialNum.end())
		pDevice = ci->second;
	else
	{
		ostringstream oss;
		oss << "Wrong Device SN '" << deviceSN << "'.";
		//RaisePhidgetErrorEvent(-1, 0, oss.str());
	}
	m_Locker.UnLock();
	return pDevice;
}
