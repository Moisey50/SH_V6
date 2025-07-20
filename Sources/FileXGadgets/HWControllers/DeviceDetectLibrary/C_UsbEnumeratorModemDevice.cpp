#include "StdAfx.h"
#include "C_UsbEnumeratorModemDevice.h"

using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary::Connection;

UsbEnumeratorModemDevice::UsbEnumeratorModemDevice(ICollector& collector)
    : m_collector(collector)
{           
    std::vector<std::wstring> motorolaPattern;
    // for example connected mobile phone Motorola
    motorolaPattern.push_back(L"Motorola USB");
    ModemDeviceNameCheckers motorolaParsers;
    motorolaParsers.push_back(CheckerModemDeviceName_Ptr(new CheckerModemDevice(collector, motorolaPattern)));
    m_modemParsers[L"Motorola Modem device"] = motorolaParsers;
   
}

UsbEnumeratorModemDevice::~UsbEnumeratorModemDevice()
{
}

void UsbEnumeratorModemDevice::Collect( const DeviceInfo& deviceInfo )
{    
    for (ModemCheckersTable::const_iterator iter = m_modemParsers.begin(); iter != m_modemParsers.end(); ++iter)
    {
        for (ModemDeviceNameCheckers::const_iterator parserIterator = iter->second.begin(); parserIterator != iter->second.end(); ++parserIterator )
        {
            (*parserIterator)->Collect(deviceInfo, iter->first);            
        }       
    }
}
