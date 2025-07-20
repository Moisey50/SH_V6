#pragma once

#include "Interfaces.h"

//#using <System.dll> 
using namespace System;

namespace MfcCoupler
{
	public interface class IWndInspector
	{
		// get main window handle
		System::IntPtr GetMainWnd();
	};

    public ref class MfcDllCoupler
    {
    private:
		HWND		            m_hWnd;					
		IWndInspector^		    m_pWndInspector;
        static MfcDllCoupler^	Instance;
    private:
        bool        AttachApplication(IWndInspector^ WndInspector); 
        !MfcDllCoupler();
    internal:
        ~MfcDllCoupler();
    public:
        MfcDllCoupler(void);
        static MfcDllCoupler^ CreateInstance(IWndInspector^ WndInspector);
    };

}