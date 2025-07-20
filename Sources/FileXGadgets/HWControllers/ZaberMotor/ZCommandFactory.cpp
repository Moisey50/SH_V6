#include "stdafx.h"
#include "ZCommandFactory.h"

ZCommandFactory::~ZCommandFactory(void)
{
	m_commandsByDataType.clear();
}
