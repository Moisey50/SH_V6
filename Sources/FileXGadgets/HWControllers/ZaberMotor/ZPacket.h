#pragma once

#include "ZCommandFactory.h"
#include "ZDevicesRepository.h"






class ZPacket
{
private:
	
	const ZDeviceBase*   m_pDevice;
	const ZCommandBase*  m_pCommand;

public:
	const ZDeviceBase* GetDevice() const
	{
		return m_pDevice;
	}

	const ZCommandBase* GetCommand() const
	{
		return m_pCommand;
	}

private:
	ZPacket(const ZPacket&);
	const ZPacket& operator=(const ZPacket&);

	static bool GetDevice(const string& key, const string& value, __out int& iDvcID)
	{
		bool res = false;

		if(key.compare(ZBR_KEY_DEVICE_ID)==0)
		{
			iDvcID = atoi(value.c_str());
			res = (iDvcID > -1);
		}

		return res;
	}

protected:

public:
	ZPacket(unsigned short deviceId = 0, short cmdId = COMMAND_ID_NONE, int data = 0, const ZDevicesRepozitory* pDevices = NULL, const ZCommandsHeadersRepository* pHeaders = NULL)
		: m_pDevice(NULL)
		, m_pCommand(NULL)
	{
		if(pDevices)
			m_pDevice = (*pDevices)[deviceId];

		if(m_pDevice && cmdId != COMMAND_ID_NONE && pHeaders)
			m_pCommand = new ZCommandBase(cmdId, data, pHeaders);
	}
	~ZPacket(void)
	{
		if(m_pCommand)
			delete m_pCommand;
	}

	static const ZPacket* Deserialize(const string& keyValuePairs, const ZCommandFactory& factory, const ZCommandsHeadersRepository& headers, const ZDevicesRepozitory& devices, const ZDeviceBase& dvc)
	{
		ZPacket* pRes = NULL;
		int indx = -1;
		string token;
		string key;
		string value;
		int iDvcID = -1;

		do
		{
			token = ZUtils::Tokenize(keyValuePairs, LIST_DELIMITER, ++indx);

			if(!ZUtils::GetKeyValue(token, KEY_VAL_DELIMITER, key, value))
				break;
			else
			{
				if(GetDevice(key, value, iDvcID) && (iDvcID == 0 || iDvcID == dvc.GetID()))
				{
					if(!pRes)
						pRes = new ZPacket();

					pRes->m_pDevice = devices[iDvcID];
				}
				else if(iDvcID <= 0 || iDvcID == dvc.GetID())
				{
					const ZCommandBase* pCommand = factory.Deserialize(key, value, headers, dvc.GetSettings()); // ZCommandBase::Deserialize(key, value, headers);
					if(pCommand)
					{
						if(!pRes)
							pRes = new ZPacket();

						pRes->m_pCommand = pCommand;
					}
				}

				key.clear();
				value.clear();
			}
			
		}
		while (indx >= 0 && indx != std::basic_string<char>::npos);

		if(pRes)
		{
			if(!pRes->GetCommand())
				pRes = NULL;
			else if(iDvcID == -1)
				pRes->m_pDevice = &dvc;
		}

		return pRes;
	}

	static const ZPacket* Deserialize(const string& packetTxt, 
    const ZCommandFactory& factory, const ZCommandsHeadersRepository& headers, 
    ZDevicesRepozitory& devices)
	{
		ZPacket* pRes = NULL;
		int devId = 0;
		const int digitsInPacket = 12;
		const int digitsInPacket4Id = 2;
		if(packetTxt.length() == digitsInPacket && ZDeviceBase::Deserialize(
      packetTxt.substr(0, digitsInPacket4Id), devId))
		{			
			devices.Init();
			
			if(!devices[devId])
				devices.Add(ZDeviceBase(devId));

			pRes = new ZPacket();
			pRes->m_pDevice = devices[devId];
			if(!pRes->m_pDevice)
				pRes = NULL;
			else
				pRes->m_pCommand = factory.Deserialize(packetTxt.substr(
        digitsInPacket4Id, digitsInPacket - digitsInPacket4Id), 
        headers, pRes->GetDevice()->GetSettings());
		}
		return pRes;
	}

	string ToString() const
	{
		ostringstream oss;

		if(GetDevice())
		{
			oss << GetDevice()->ToString();
			if(GetCommand())
			{
				oss << LIST_DELIMITER << GetCommand()->ToString();
			}
		}
		return oss.str();
	}

	string Serialize() const
	{
		ostringstream oss;
		
		if(GetDevice() && GetCommand())
		{
			oss << GetDevice()->Serialize() << GetCommand()->Serialize();
		}
		return oss.str();
	}
};

