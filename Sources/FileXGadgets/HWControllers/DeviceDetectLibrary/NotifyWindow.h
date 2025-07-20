#ifndef _NOTIFYWINDOW_H_
#define _NOTIFYWINDOW_H_


#include "Win_ManualResetEvent.h"
#include "Win_Thread.h"


namespace DeviceDetectLibrary
{
    struct IDeviceChanged;

    class NotifyWindow //non copyable
    {
	private:
		HWND              m_hWnd;
        bool              m_toDestroy;
        Thread            m_backgroundWorker;
        ManualResetEvent  m_eventStarted;
        IDeviceChanged   &m_deviceChanged;

		DISALLOW_COPY_AND_ASSIGN(NotifyWindow);
	
	public:
        NotifyWindow(IDeviceChanged& deviceChanged);
        ~NotifyWindow();



    private:

		bool RegisterNotifyWndwClass(HINSTANCE hInstance);
        bool InitInstance(HINSTANCE hInstance);
		int DestroyInstance();
		int UnregisterNotifyWndwClass( HINSTANCE hInstance );

		static void VolumeAddRemove(const std::vector<std::wstring>& drives, bool isArrived );
		static void DeviceInterfaceAddRemove(const GUID& guid, const DeviceInfo::DeviceId& devId, bool isArrived);

        static void ThreadProc(void* context);    
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	public:
		HWND GetHWND() const;
		
	};
}
#endif // _NOTIFYWINDOW_H_