#include "StdAfx.h"
#include "C_UsbEnumeratorMassStorage.h"

using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary::Connection;
using namespace Disk::Types;
using namespace Disk::Functions;

UsbEnumeratorMassStorage::UsbEnumeratorMassStorage( ICollector& collector )
    : m_collector(collector)
{
}

UsbEnumeratorMassStorage::~UsbEnumeratorMassStorage()
{
}

void UsbEnumeratorMassStorage::TryThis( const DeviceInfo::DeviceId& devicePath )
{
    AutoCriticalSection lock(m_criticalSec);
    if (m_checkers.count(devicePath) == 0)
    {
        m_checkers[devicePath] = CheckerMassStorage_Ptr(new CheckerMassStorage(m_collector, devicePath));
    }
    // Cancel previous process
    m_checkers[devicePath]->Stop();
    // Begin new process
    m_checkers[devicePath]->Start();

}

void UsbEnumeratorMassStorage::RemoveThis( const DeviceInfo::DeviceId& devicePath )
{
    AutoCriticalSection lock(m_criticalSec);
    if (m_checkers.count(devicePath) != 0)
    {
        m_checkers.erase(devicePath);
    }
}

void UsbEnumeratorMassStorage::Collect(const DeviceInfo& deviceInfo)
{
    try
    {
        RemovableDeviceInfo_vt disks = SearchRemovalDisks();
        for(RemovableDeviceInfo_vt_cit iter = disks.begin(); iter != disks.end(); ++iter )
        {
                ConnectionInfo info = {
                    TypeUnknown,
                    iter->wsFriendlyName,
                    iter->wsPath,
                    L"",
                    iter->wsPath};

                    m_collector.Found(DeviceInfo(iter->wsPath, iter->wsFriendlyName, L"Mass Storage device", info));
        }
    }
    catch (const std::exception& /*ex*/)
    {
        // EXCEPTION: Cannot do SearchRemovalDisks(). Error = %s", ex.what());        
    }    

}
