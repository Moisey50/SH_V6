#pragma once
#include "c_usbenumeratorbase.h"

namespace DeviceDetectLibrary
{
	namespace Connection
	{
		class UsbEnumeratorRawDevice :
			public UsbEnumeratorBase
		{
			typedef UsbEnumeratorBase Base;
		public:
			UsbEnumeratorRawDevice(const NotifyWindow& window, ICollector& collector);
		};
	}
}
