#ifndef _C_MS_CHECKER_H_
#define _C_MS_CHECKER_H_

#include <memory>

#include "Win_Thread.h"
#include "Win_ManualResetEvent.h"


namespace DeviceDetectLibrary
{
    namespace Connection
    {
        class CheckerMassStorage
        {
		private:
            DeviceInfo::DeviceId m_devicePath;
            ICollector&          m_collector; 

            Thread               m_workerThread;
            ManualResetEvent     m_eventStop;
        public:
			CheckerMassStorage(ICollector& collector, const DeviceInfo::DeviceId& devicePath);
            ~CheckerMassStorage();
 
		private:
            static void ThreadFunction(void *pState);
            void ThreadFunction();

		public:
            void Start();
            void Stop();
        };

        typedef shared_ptr<CheckerMassStorage> CheckerMassStorage_Ptr;
    }
}


#endif // _C_MS_CHECKER_H_
