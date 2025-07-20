#include "stdafx.h"
#include "C_UsbEnumeratorHID.h"

using namespace DeviceDetectLibrary::Connection;
using namespace DeviceDetectLibrary;

UsbEnumeratorHID::UsbEnumeratorHID(const NotifyWindow& window, ICollector& collector)
	: Base(collector)
{
	// Human Interface Device (HID) {4d1e55b2-f16f-11cf-88cb-001111000030} 
	GUID guid = {0x4d1e55b2,0xf16f, 0x11cf, { 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};
	AddCollector(guid, IEnumerator_Ptr(new ResultEnumerator(collector, L"Human Interface Device", L"USB Human Interface Device")), window); 
}
