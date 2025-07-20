#pragma once
#include <map>
#include "ZCommandBase.h"
#include "ZCommandPosition.h"
#include "ZDeviceSettings.h"

using namespace std;

class ZCommandFactory
{
	typedef map<eCommandDataType, FnDeserializeByFactory> CommandsMap;

	CommandsMap m_commandsByDataType;

public:
	ZCommandFactory()
		: m_commandsByDataType()
	{
		Register(CDT_POSITION,          &ZCommandPosition::DeserializeByFactory);
		Register(CDT_POSITION_NEW,      &ZCommandPosition::DeserializeByFactory);
		Register(CDT_POSITION_ABSOLUTE, &ZCommandPosition::DeserializeByFactory);
		Register(CDT_POSITION_RELATIVE, &ZCommandPosition::DeserializeByFactory);
		Register(CDT_POSITION_FINAL,    &ZCommandPosition::DeserializeByFactory);
		Register(CDT_POSITION_STORED,   &ZCommandPosition::DeserializeByFactory);
		Register(CDT_POSITION_TRAKING,  &ZCommandPosition::DeserializeByFactory);
		Register(CDT_POSITION_MIN,      &ZCommandPosition::DeserializeByFactory);
	}
	~ZCommandFactory(void);

private:
	void Register(eCommandDataType cmdDataType, FnDeserializeByFactory pDeserialByFactoryFn)
	{
		m_commandsByDataType[cmdDataType] = pDeserialByFactoryFn;
	}

	const ZCommandBase* Create(const ZCommandBase* pBase, const ZDeviceSettings& settings, eCommandDataType cmdDataType = CDT_NONE) const
	{
		const ZCommandBase* pRes = pBase;
		if(pRes && pRes->GetHeader())
		{
			const ZCommandHeader* pHeader = pRes->GetHeader();
			CommandsMap::const_iterator ci = m_commandsByDataType.find(cmdDataType); //);

			if(ci != m_commandsByDataType.end())
			{
				pRes = ci->second(pHeader, pRes->GetData(), settings);
				delete pBase;
			}
		}
		return pRes;
	}

protected:
public:
	const ZCommandBase* Deserialize(const string& cmdTxt, const ZCommandsHeadersRepository& headers, const ZDeviceSettings& settings ) const
	{
		const ZCommandBase* pCmd = ZCommandBase::Deserialize(cmdTxt, headers);
		const ZCommandBase* pRes = Create(pCmd, settings, 
      !pCmd || !pCmd->GetHeader() ? CDT_NONE : pCmd->GetHeader()->GetCommandDataType2Reply());
		if(pRes)
		{
			switch (pRes->GetSimplifiedDataType())
			{
			case CDT_POSITION:
				{
					((ZCommandPosition*)pRes)->SetPosition(settings);
				}
				break;
			default:
				break;
			}
		}
		return pRes;
	}

	const ZCommandBase* Deserialize(const string& cmdName, const string& dataTxt, const ZCommandsHeadersRepository& headers, const ZDeviceSettings& settings ) const
	{
		const ZCommandBase* pCmd = ZCommandBase::Deserialize(cmdName, dataTxt, headers);
		const ZCommandBase* pRes = Create(pCmd, settings, 
      !pCmd || !pCmd->GetHeader() ? CDT_NONE : pCmd->GetHeader()->GetCommandDataType2Send());
		if(pRes)
		{
			switch (pRes->GetSimplifiedDataType())
			{
			case CDT_POSITION:
				{
					double posInMicrons = atof(dataTxt.c_str());
					((ZCommandPosition*)pRes)->SetPosition(posInMicrons, &settings);
				}
				break;
			default:
				break;
			}
		}
		return pRes;
	}
};

