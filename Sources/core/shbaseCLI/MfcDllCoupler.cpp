#include "StdAfx.h"
#include "MfcDllCoupler.h"

using namespace MfcCoupler;

///// MfcDllCoupler
////////////////////////////////////////////////////////////////////////////////////////////////

MfcDllCoupler::MfcDllCoupler(void)
{
}

MfcDllCoupler::~MfcDllCoupler()
{
	this->!MfcDllCoupler();
}

MfcDllCoupler::!MfcDllCoupler()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	try 
	{
		IApplication* pApp = dynamic_cast<IApplication*>(AfxGetApp());
		if(pApp!=NULL)
			pApp->DetachWnd();
	} 
	finally
	{
		Instance=nullptr;
	}
}

MfcDllCoupler^ MfcDllCoupler::CreateInstance(IWndInspector^ WndInspector)
{
	ASSERT(WndInspector);
	
	//we use only one instance
	if(Instance==nullptr)
	{
		Instance = gcnew MfcDllCoupler();
		if(!Instance->AttachApplication(WndInspector))
		{
			//delete Instance;
			//Instance = nullptr;
		}
	}
	ASSERT(Instance!=nullptr);
	return Instance;
}

bool MfcDllCoupler::AttachApplication(IWndInspector^ WndInspector)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	bool retval = false;

    CWinApp* wa=AfxGetApp();
    ASSERT(wa!=NULL);
	IApplication* pApp = dynamic_cast<IApplication*>(wa);
	if(pApp!=NULL)
	{
		m_pWndInspector = WndInspector;
		m_hWnd = (HWND)(WndInspector->GetMainWnd()).ToInt64();
		if (::IsWindow(m_hWnd))
		{
			try 
			{
				if(pApp->AttachWnd(m_hWnd)==TRUE)
					retval = true;
			}
			catch(...){}
		}
	}
	return retval;
}
