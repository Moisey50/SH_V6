#include "stdafx.h"
#include "CMouseHook.h"

char CMouseHook::m_ResultBuffer[ 100 ] ;
MouseHookProc CMouseHook::m_CallBack ;
HHOOK CMouseHook::m_hHook ;
void *CMouseHook::m_pHost ;
