#include "StdAfx.h"
#include "DeviceWatcher.h"
#include "C_UsbEnumeratorHID.h"

using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary::Connection;

DeviceWatcher::DeviceWatcher(IDeviceWatcherObserver* observer)
	: Base(observer)
	//, m_pUsbEnumerator()
	//, m_pMsEnumerator()
{    
}

DeviceWatcher::~DeviceWatcher()
{
    Stop();
}

void DeviceWatcher::InterfaceArrival( const GUID& guid )
{
    m_pUsbEnumerator->TryThis(guid);
}

void DeviceWatcher::InterfaceRemoved( const DeviceInfo::DeviceId& devId )
{
    Lost(devId);
}

void DeviceWatcher::VolumeArrival( const DeviceInfo::DeviceId& devId )
{
    if (NULL != m_pMsEnumerator)
    {
        m_pMsEnumerator->TryThis(devId);
    }
}

void DeviceWatcher::VolumeRemoved( const DeviceInfo::DeviceId& devId )
{
    if (NULL != m_pMsEnumerator)
    {
        m_pMsEnumerator->RemoveThis(devId);
        Lost(devId);
    }
}

void DeviceWatcher::StartEnumerators()
{
    NotifyWindow& window = CreateNotifyWindow();

    // Create Enumerators
    m_pUsbEnumerator.reset(new UsbEnumeratorHID(window, *this));
    //if(m_pUsbEnumerator.get() != NULL)
		m_pUsbEnumerator->Collect(DeviceInfo());

    m_pMsEnumerator.reset(new UsbEnumeratorMassStorage(*this));
    m_pMsEnumerator->Collect(DeviceInfo()); 
}

void DeviceWatcher::StopEnumerators()
{    
    RemoveNotifyWindow();

    // Remove Enumerators
    m_pUsbEnumerator.reset();
    m_pMsEnumerator.reset();
}

void DeviceWatcher::Start( Connection::UsbEnumeratorBase *pUsbEnumerator )
{
	m_pUsbEnumerator.reset(pUsbEnumerator);
	Base::Start();
}

NotifyWindow& DeviceWatcher::CreateDummyWindow()
{
	return Base::CreateNotifyWindow();
}
