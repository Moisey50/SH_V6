#pragma once
#include "c_usbenumeratorbase.h"

namespace DeviceDetectLibrary
{
	namespace Connection
	{
		class UsbEnumeratorNetworkCard :
			public UsbEnumeratorBase
		{
			typedef UsbEnumeratorBase Base;
		public:
			UsbEnumeratorNetworkCard(const NotifyWindow& window, ICollector& collector);
		};
	}
}

