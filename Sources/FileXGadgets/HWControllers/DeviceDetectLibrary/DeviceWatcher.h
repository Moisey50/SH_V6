#ifndef _DEVICE_WATCHER_H_
#define _DEVICE_WATCHER_H_

#include "ADeviceWatcher.h"
#include "IDeviceChanged.h"
#include "DeviceInfo.h"

#include "C_UsbEnumeratorCollection.h"
#include "C_UsbEnumeratorMassStorage.h"

namespace DeviceDetectLibrary
{
    struct IDeviceWatcherObserver;

    class DeviceWatcher
		: public ADeviceWatcher
    {
    private:
		typedef ADeviceWatcher Base;
        
		Connection::UsbEnumeratorBase_Ptr m_pUsbEnumerator;
        Connection::UsbEnumeratorMassStorage_Ptr  m_pMsEnumerator;

    public:
		DeviceWatcher(IDeviceWatcherObserver *pObserver);
        ~DeviceWatcher();

    private: // IDeviceChanged 
        void InterfaceArrival(const GUID& guid);
        void InterfaceRemoved(const DeviceInfo::DeviceId& devId);
        void VolumeArrival(const DeviceInfo::DeviceId& devId);
		void VolumeRemoved(const DeviceInfo::DeviceId& devId);
	
	protected:
		void StartEnumerators();
		void StopEnumerators();

	public:
		void Start(Connection::UsbEnumeratorBase *pUsbEnumerator);
		NotifyWindow& CreateDummyWindow();
    };
}

#endif // _DEVICE_WATCHER_H_