#include "StdAfx.h"
#include <fxfc\fxfc.h>

FXPlex* PASCAL FXPlex::Create(FXPlex*& pHead, UINT_PTR nMax, UINT_PTR cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	if (nMax == 0 || cbElement == 0)
		return NULL;
	FXPlex* p = (FXPlex*) new BYTE[sizeof(FXPlex) + nMax * cbElement];
	p->pNext = pHead;
	pHead = p;  
	return p;
}

void FXPlex::FreeDataChain()     
{
	FXPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		FXPlex* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}
