#include "stdafx.h"
#include "C_UsbEnumeratorRawDevice.h"

using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary::Connection;

UsbEnumeratorRawDevice::UsbEnumeratorRawDevice(const NotifyWindow& window, ICollector& collector)
	: Base(collector)
{
	//USB Raw Device {a5dcbf10-6530-11d2-901f-00c04fb951ed} 
	GUID guid = { 0xa5dcbf10, 6530, 0x11d2, { 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed } };
	IEnumerator_Ptr enumerator(new ResultEnumerator(collector, L"RawDevice", L"Raw USB Device"));
	AddCollector(guid, enumerator, window);
}
