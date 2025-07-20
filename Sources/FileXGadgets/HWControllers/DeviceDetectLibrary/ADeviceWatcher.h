#ifndef _A_DEVICE_WATCHER_H_
#define _A_DEVICE_WATCHER_H_


#include "Win_CriticalSection.h"
#include "IDeviceChanged.h"
#include "ICollector.h"
#include "DeviceInfo.h"
#include "NotifyWindow.h"

namespace DeviceDetectLibrary
{
    struct IDeviceWatcherObserver;

	//Abstract class
    class ADeviceWatcher //non copyable
		: public ICollector
		, public IDeviceChanged
    {
	private:
		IDeviceWatcherObserver     *m_pObserver;
//     std::auto_ptr<NotifyWindow> m_pWindow;
    std::unique_ptr<NotifyWindow> m_pWindow;

		DevicesMap                  m_devicesActual;
        DevicesMap                  m_devicesTemp;        

        CriticalSection             m_criticalSection;

		DISALLOW_COPY_AND_ASSIGN(ADeviceWatcher);

    public:
		ADeviceWatcher(IDeviceWatcherObserver *pObserver);
        ~ADeviceWatcher();

	protected:
		virtual void StartEnumerators() = 0;
		virtual void StopEnumerators() = 0;
		
		NotifyWindow& CreateNotifyWindow();
		void RemoveNotifyWindow();

	public:
        void Start();
        void Stop();

		// ICollector implementation
        void Found(const DeviceInfo& deviceInfo);
        void Lost(const DeviceInfo::DeviceId& devId);
    };

    typedef shared_ptr<ADeviceWatcher> DeviceWatcher_Ptr;
}
#endif _A_DEVICE_WATCHER_H_
