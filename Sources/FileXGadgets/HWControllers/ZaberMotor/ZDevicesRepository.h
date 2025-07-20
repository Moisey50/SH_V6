#pragma once
#include "ZDeviceBase.h"
#include <set>
#include <map>

using namespace std;

class ZDevicesRepozitory
{
	typedef set<ZDeviceBase> DevicesSet;
	typedef map<unsigned short, const ZDeviceBase*> DevicesMapById;

	DevicesSet m_devices;
	DevicesMapById m_devicesById;

public:
	ZDevicesRepozitory(void)
		: m_devices()
		, m_devicesById()
	{

	}

	~ZDevicesRepozitory(void)
	{
		m_devices.clear();
		m_devicesById.clear();
	}

private:
protected:
public:
	void Init()
	{
		if(IsEmpty())
			Add(ZDeviceBase(0));
	}
	bool IsEmpty() const
	{
		return m_devices.empty();
	}
	void Add(const ZDeviceBase& zdevice)
	{
		pair<DevicesSet::iterator, bool> inserted = m_devices.insert(zdevice);
		if(inserted.second)
			m_devicesById[zdevice.GetID()] = &(*(inserted.first));
	}
	const ZDeviceBase* operator[](unsigned short id) const
	{
		const ZDeviceBase* pRes = NULL;
		DevicesMapById::const_iterator ci = m_devicesById.find(id);

		if(ci != m_devicesById.end())
			pRes = ci->second;

		return pRes;
	}

};

