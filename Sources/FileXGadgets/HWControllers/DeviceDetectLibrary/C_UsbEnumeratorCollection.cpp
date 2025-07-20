#include "StdAfx.h"
#include "C_UsbEnumeratorCollection.h"
#include "C_UsbEnumeratorModemDevice.h"
#include "C_UsbEnumeratorRawDevice.h"
#include "C_UsbEnumeratorHID.h"

using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary::Connection;
using namespace DeviceDetectLibrary;

UsbEnumeratorCollection::UsbEnumeratorCollection(const NotifyWindow& window, ICollector& collector)
    :  Base(collector)
{
	UsbEnumeratorRawDevice rd(window, collector);
	CopyCollector(rd);
	UsbEnumeratorHID hid(window, collector);
	CopyCollector(hid);
	
	/*
	GUID guidHID = {0x4d1e55b2,0xf16f, 0x11cf, { 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};
    AddCollector(guidHID, IEnumerator_Ptr(new ResultEnumerator(collector, L"Human Interface Device", L"USB Human Interface Device")), window); 
    
	GUID guidBlackBerry = { 0x80375827, 0x83b8, 0x4a51, { 0xb3, 0x9b, 0x90, 0x5f, 0xed, 0xd4, 0xf1, 0x18 } };
    AddCollector(guidBlackBerry, IEnumerator_Ptr(new ResultEnumerator(collector, L"Blackberry", L"BlackBerry USB Device")), window);    
	
	GUID guidGarmin = {0x2c9c45c2, 0x8e7d, 0x4c08, {0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7, 0x22, 0xc0}}; 
    AddCollector(guidGarmin, IEnumerator_Ptr(new ResultEnumerator(collector, L"Garmin GPS device", L"Garmin USB Device")), window);

    GUID guidModem = {0x2c7089aa, 0x2e0e, 0x11d1, {0xb1, 0x14, 0x00, 0xc0, 0x4f, 0xc2, 0xaa, 0xe4}}; 
    AddCollector(guidModem, IEnumerator_Ptr(new UsbEnumeratorModemDevice(collector)), window);


	*A list of common device interface class GUIDs is given below:

	Device Interface Name GUID 

	--USB Raw Device {a5dcbf10-6530-11d2-901f-00c04fb951ed} 
	Disk Device {53f56307-b6bf-11d0-94f2-00a0c91efb8b} 
	--Network Card {ad498944-762f-11d0-8dcb-00c04fc3358c} 
	--Human Interface Device (HID) {4d1e55b2-f16f-11cf-88cb-001111000030} 
	Palm {784126bf-4190-11d4-b5c2-00c04f687a67} 

	 */
}
