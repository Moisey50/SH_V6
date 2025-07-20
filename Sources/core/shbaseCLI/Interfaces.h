#pragma once

class IApplication
{
	public:
		virtual BOOL AttachWnd(HWND hwnd)=0;
		virtual BOOL DetachWnd()=0;
};
