#include "stdafx.h"
#include "ZGadgetBase.h"

const ZCommandsHeadersRepository ZGadgetBase::m_CommandsHeaders;
const ZCommandFactory            ZGadgetBase::m_CommandsFactory;
FXLockObject                     ZGadgetBase::m_Locker;
ZDevicesRepozitory               ZGadgetBase::m_Devices;

ZGadgetBase::ZGadgetBase(void)
{
}


ZGadgetBase::~ZGadgetBase(void)
{
}


