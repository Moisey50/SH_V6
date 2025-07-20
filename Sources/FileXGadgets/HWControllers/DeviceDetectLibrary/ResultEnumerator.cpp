#include "StdAfx.h"
#include "ResultEnumerator.h"

using namespace DeviceDetectLibrary;

ResultEnumerator::ResultEnumerator(
	ICollector& collector,
	std::wstring deviceDisplayName,
	const std::wstring& friendlyName) 
    : m_deviceID(deviceDisplayName)
    , m_friendlyName(friendlyName)
    , m_collector(collector)
{
}

ResultEnumerator::~ResultEnumerator()
{
}

void ResultEnumerator::Collect(const DeviceInfo& deviceInfo)
{
    DeviceInfo info(deviceInfo);
    info.SetName(m_friendlyName);
    info.SetDeviceDisplayName(m_deviceID);
    m_collector.Found(info);
}
