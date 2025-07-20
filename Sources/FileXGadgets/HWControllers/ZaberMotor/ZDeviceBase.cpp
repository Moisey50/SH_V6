#include "stdafx.h"
#include "ZDeviceBase.h"


ZDeviceBase::ZDeviceBase(int dvcId /*= DEVICE_ID_UNDEFINED*/, int stepsPerRevol /*= UNDEFINED_VALUE*/, double motionPerRevol_um /*= UNDEFINED_VALUE*/)
	: m_deviceId(dvcId)
	, m_settings(dvcId<=0 ? ZDeviceSettings::NULL_DEVICE_SETTINGS : ZDeviceSettings())
{
	if (dvcId > 0)
	{
		if (stepsPerRevol > 0)
			m_settings.SetProductStepsPerRevolution(stepsPerRevol);

		if (motionPerRevol_um > 0)
			m_settings.SetProductLinearMotionPerRevolution_um(motionPerRevol_um);
	}
}


ZDeviceBase::~ZDeviceBase(void)
{
}
