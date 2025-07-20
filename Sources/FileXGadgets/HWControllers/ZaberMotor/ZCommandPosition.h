#pragma once
#include "zcommandbase.h"

#define ZBR_KEY_POSITION                   ("Position(um)")

class ZCommandPosition :
	public ZCommandBase
{
	double m_position;

private:
protected:
public:
	double GetPosition() const
	{
		return m_position;
	}
	void SetPosition( const ZDeviceSettings& settings)
	{
		m_position = GetMicrons(GetData(), settings);
	}
	void SetPosition( double posInMicrons, const ZDeviceSettings* pSettings = NULL);
	
private:
protected:
public:
	ZCommandPosition(void);
	virtual ~ZCommandPosition(void);

private:
	int GetMicroSteps( double posInMicrons, const ZDeviceSettings& settings )
	{
		int iRes = settings.GetPositionMicrosteps(posInMicrons);
		return iRes;
	}
	double GetMicrons( int posInMicroSteps, const ZDeviceSettings& settings )
	{
		double dRes = settings.GetPositionMicrons(posInMicroSteps);

		return dRes;
	}
protected:

public:
	static ZCommandBase* __stdcall DeserializeByFactory(const ZCommandHeader* pHeader, int rawData, const ZDeviceSettings& settings)
	{
		ZCommandBase* pRes = new ZCommandPosition();
		pRes->SetHeader(pHeader);
		pRes->SetData(rawData);

		return pRes;
	}

	virtual string Serialize() const
	{
		return ZCommandBase::Serialize();
	}

	virtual string ToString() const
	{
		ostringstream oss;

		oss << ZCommandBase::ToString() << LIST_DELIMITER << ZBR_KEY_POSITION << KEY_VAL_DELIMITER;
		oss.setf( std::ios::fixed, std:: ios::floatfield ); ;
		oss << m_position;

		return oss.str();
	}
	
};

