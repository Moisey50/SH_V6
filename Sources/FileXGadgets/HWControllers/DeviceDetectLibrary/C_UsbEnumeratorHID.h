#pragma once
#include "c_usbenumeratorbase.h"

namespace DeviceDetectLibrary
{
	namespace Connection
	{
		class UsbEnumeratorHID :
			public UsbEnumeratorBase
		{
			typedef UsbEnumeratorBase Base;

		public:
			UsbEnumeratorHID(const NotifyWindow& window, ICollector& collector);
		};
	}
}

