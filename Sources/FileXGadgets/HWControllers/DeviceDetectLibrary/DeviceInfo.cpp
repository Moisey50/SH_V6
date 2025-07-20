#include "StdAfx.h"
#include "DeviceInfo.h"

using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary;

DeviceInfo::DeviceInfo()
    : m_id()
    , m_name()
    , m_deviceDisplayName(L"Unknown device")
{
}

DeviceInfo::DeviceInfo(const DeviceId& id, const std::wstring& name, std::wstring deviceDisplayName, const ConnectionInfo& connInfo) 
    : m_id(Utilities::StringUpper(id))
    , m_name(name)
    , m_deviceDisplayName(deviceDisplayName)
    , m_connInfo(connInfo)
{
}

DeviceInfo::~DeviceInfo()
{
}

