#include "StdAfx.h"
#include "ADeviceWatcher.h"

using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary;

ADeviceWatcher::ADeviceWatcher(IDeviceWatcherObserver* observer)
	: m_pObserver(observer)
{
}

ADeviceWatcher::~ADeviceWatcher()
{
	m_pObserver = NULL;
}

void ADeviceWatcher::Start()
{
    StartEnumerators();
    m_devicesTemp.clear();
}

void ADeviceWatcher::Stop()
{
    StopEnumerators();
    m_devicesActual.clear();
}

NotifyWindow& ADeviceWatcher::CreateNotifyWindow()
{
    if (0 == m_pWindow.get())
    {
        // Create NotifyWindow
        m_pWindow.reset(new NotifyWindow(*this));        
    }
    return *m_pWindow;
}

void ADeviceWatcher::RemoveNotifyWindow()
{
    // Remove NotifyWindow");
    m_pWindow.reset(0);
}

void ADeviceWatcher::Found(const DeviceInfo& deviceInfo)
{
    AutoCriticalSection lock(m_criticalSection);
    m_devicesTemp[deviceInfo.GetId()] = deviceInfo;
    if (m_devicesActual.count(deviceInfo.GetId()) == 0)
    {
        m_devicesActual[deviceInfo.GetId()] = deviceInfo;
        m_pObserver->AppearedDevice(deviceInfo);
    }
}

void ADeviceWatcher::Lost(const DeviceInfo::DeviceId& deviceId)
{
    AutoCriticalSection lock(m_criticalSection);
    if (m_devicesActual.count(deviceId) != 0)
    {
        m_pObserver->DisappearedDevice(m_devicesActual[deviceId]);
        m_devicesActual.erase(deviceId);
    }
}
