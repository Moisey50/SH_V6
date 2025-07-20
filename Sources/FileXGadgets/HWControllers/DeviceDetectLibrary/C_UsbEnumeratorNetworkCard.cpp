#include "stdafx.h"
#include "C_UsbEnumeratorNetworkCard.h"

using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary::Connection;

UsbEnumeratorNetworkCard::UsbEnumeratorNetworkCard(const NotifyWindow& window, ICollector& collector)
	: Base(collector)
{
	// Network Card {ad498944-762f-11d0-8dcb-00c04fc3358c}                          
	GUID guidBlackBerry = { 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } };
	AddCollector(guidBlackBerry, IEnumerator_Ptr(new ResultEnumerator(collector, L"NetworkCard", L"USB Network Card Device")), window);    
}

