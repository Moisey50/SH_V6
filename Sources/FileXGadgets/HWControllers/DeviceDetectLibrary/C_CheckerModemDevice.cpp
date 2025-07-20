#include "StdAfx.h"

#include "C_CheckerModemDevice.h"

using namespace DeviceDetectLibrary;

CheckerModemDevice::CheckerModemDevice(ICollector& collector, const std::vector<std::wstring>& patternNames )
	: m_collector(collector)
	, m_patternNames(patternNames)
{
}

CheckerModemDevice::~CheckerModemDevice()
{
   
}


bool CheckerModemDevice::Check( const DeviceInfo& deviceInfo )
{
    int searchCount = 0;
    std::wstring name = deviceInfo.GetName();
    for (std::vector<std::wstring>::const_iterator ci = m_patternNames.begin(); ci != m_patternNames.end(); ++ci)
    {
        if (name.end() != std::search(
            name.begin(), 
            name.end(), 
            (*ci).begin(),
            (*ci).end()))
        {
            searchCount++;
        }
    }
	
	return searchCount == m_patternNames.size();
}


void CheckerModemDevice::Collect( const DeviceInfo& deviceInfo, const std::wstring& pluginID)
{
    if (Check(deviceInfo))
    {
        DeviceInfo info(deviceInfo);
        info.SetDeviceDisplayName(pluginID);
        m_collector.Found(info);
    }
}
