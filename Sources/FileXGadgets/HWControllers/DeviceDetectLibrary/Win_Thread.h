#ifndef _WIN_THREAD_H_
#define _WIN_THREAD_H_

#include "Win_WaitObject.h"
#include "Utilities.h"

namespace DeviceDetectLibrary
{
    class Thread //non copyable
		: public WaitObject
	{
	public:
		typedef void (*LPThreadWorker)(void *pState);	

    private:
        LPThreadWorker  m_pWorkerRoutine;
        void           *m_pCurrentState;

		DISALLOW_COPY_AND_ASSIGN(Thread);

	public:
        Thread(LPThreadWorker pWorkerRoutine);
		~Thread();

    private:
        static unsigned int __stdcall Routine(void *pState);

    public:
        void Start(void *pState);
        bool Wait(unsigned int milliseconds);
    };
}

#endif // _WIN_THREAD_H_