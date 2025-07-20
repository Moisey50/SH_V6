#include "stdafx.h"
#include "ZCommandPosition.h"


ZCommandPosition::ZCommandPosition(void)
	: m_position(0)
{
	SetSimplifiedDataType(CDT_POSITION);
}


ZCommandPosition::~ZCommandPosition(void)
{
}

void ZCommandPosition::SetPosition( double posInMicrons, const ZDeviceSettings* pSettings /* = NULL*/)
{
	if(abs(m_position-posInMicrons)>=0.00001)
	{
		m_position = posInMicrons;

		if(pSettings)
			ZCommandBase::SetData(GetMicroSteps(m_position, *pSettings));
	}
}