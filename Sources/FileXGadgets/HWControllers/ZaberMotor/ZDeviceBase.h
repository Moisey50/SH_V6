#pragma once
#include "ZDeviceSettings.h"
#include <string>
#include "ZUtils.h"

using namespace std;

#define UNDEFINED_VALUE   (-1)

#define ZBR_KEY_DEVICE_ID ("DeviceID")

class ZDeviceBase
{
	int             m_deviceId;
	ZDeviceSettings m_settings;

public:
	int GetID() const
	{
		return m_deviceId;
	}
	void SetID(unsigned short id)
	{
		m_deviceId = id;
	}

	const ZDeviceSettings& GetSettings() const
	{
		return m_settings;
	}
	void SetSettings(const ZDeviceSettings& settings)
	{
		m_settings = settings;
	}
protected:
private:

public:
	ZDeviceBase(int dvcId = UNDEFINED_VALUE, int stepsPerRevol = UNDEFINED_VALUE, double motionPerRevol_um = UNDEFINED_VALUE);
	virtual ~ZDeviceBase(void);

private:
protected:
public:
	static const bool Deserialize( const string& devIdInHex, __in int& devId )
	{
		if(!ZUtils::HexByte2DecByte(devIdInHex, devId))
			devId = -1;
		return devId >= 0;
	}

	friend bool operator< (const ZDeviceBase& lh, const ZDeviceBase& rh)
	{
		return lh.GetID() < rh.GetID();
	}

	string Serialize() const
	{
		string hexDvcID; 
		if(!ZUtils::DecByte2HexByte(m_deviceId, hexDvcID))
			hexDvcID = "00";
		return hexDvcID;
	}

	const string ToString() const
	{
		ostringstream oss;
		oss << ZBR_KEY_DEVICE_ID << KEY_VAL_DELIMITER << GetID();
		return oss.str();
	}

};

