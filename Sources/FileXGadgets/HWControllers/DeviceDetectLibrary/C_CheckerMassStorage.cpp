#include "StdAfx.h"
#include "C_CheckerMassStorage.h"

using namespace DeviceDetectLibrary;
using namespace Disk::Types;
using namespace Disk::Functions;
using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary::Connection;
using namespace DeviceDetectLibrary;


CheckerMassStorage::CheckerMassStorage( ICollector& collector, const DeviceInfo::DeviceId& devicePath )
    : m_collector(collector)
    , m_devicePath(devicePath)
    , m_workerThread(ThreadFunction)
{

}

CheckerMassStorage::~CheckerMassStorage()
{
    Stop();
}

void CheckerMassStorage::Start()
{
    m_eventStop.Reset();
    m_workerThread.Start(this);
}

void CheckerMassStorage::Stop()
{
    m_eventStop.Set();
    m_workerThread.Wait(INFINITE);
}

void CheckerMassStorage::ThreadFunction( void *pState )
{
    CheckerMassStorage* This = static_cast<CheckerMassStorage*>(pState);
    This->ThreadFunction();
}

void CheckerMassStorage::ThreadFunction()
{
    int tries = 60;
    const int sleepTimeout = 1000;
    RemovableDeviceInfo_vt disks; 
    while ((tries--) > 0)
    {
        try
        {
            disks = SearchRemovalDisks();
            break;
        }
        catch (const std::exception& /*ex*/)
        {
            // EXCEPTION: Cannot found drives for (%S)", devicePath_.c_str());
            if (m_eventStop.Wait(sleepTimeout))
            {
                return;
            }
        }
    }
    
    for(RemovableDeviceInfo_vt_cit ci = disks.begin(); ci != disks.end(); ++ci )
    {
        if (m_devicePath == ci->wsPath)
        {
			ConnectionInfo info = {
                    TypeUnknown,
                    ci->wsFriendlyName,
                    ci->wsPath,
                    L"",
                    ci->wsPath};
                    m_collector.Found(DeviceInfo(ci->wsPath, ci->wsFriendlyName, L"Mass Storage device", info));
        }
    }
}