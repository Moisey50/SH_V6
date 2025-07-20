#ifndef _CLSER____H_
#define _CLSER____H_

#ifndef CLSER___EXPORT
#define CLSER___EXPORT __declspec(dllimport)
#endif

#ifndef CLSER___CC
#define CLSER___CC __cdecl
#endif

#ifdef __cplusplus
extern "C"{
#endif

CLSER___EXPORT int CLSER___CC
	clSerialInit(unsigned long serialIndex, void** serialRefPtr);

typedef int(*ptclSerInitProc)(unsigned long, void**);

CLSER___EXPORT int CLSER___CC
	clSerialRead(void* serialRef, char* buffer,
				 unsigned long* bufferSize,
				unsigned long serialTimeout);

typedef int(*ptclSerReadProc)(void*, char*, unsigned long*,	unsigned long);

CLSER___EXPORT int CLSER___CC
	clSerialWrite(void* serialRef, char* buffer, unsigned long* bufferSize,
		unsigned long serialTimeout);
	
typedef int(*ptclSerWriteProc)(void*, char*, unsigned long*, unsigned long);

CLSER___EXPORT void CLSER___CC
	clSerialClose(void* serialRef);

typedef int(*ptclSerCloseProc)(void*);

#ifdef __cplusplus
}
#endif

#endif