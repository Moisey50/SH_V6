#ifndef FXPLEX_INCLUDE
#define FXPLEX_INCLUDE
// FXMapPtrToPtr.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct FXFC_EXPORT FXPlex
{
	FXPlex* pNext;
	// BYTE data[maxNum*elementSize];

	void* data() { return this+1; }
	static FXPlex* PASCAL Create(FXPlex*& head, UINT_PTR nMax, UINT_PTR cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions
	void FreeDataChain();       // free this one and links
};

#endif //#ifndef FXPLEX_INCLUDE