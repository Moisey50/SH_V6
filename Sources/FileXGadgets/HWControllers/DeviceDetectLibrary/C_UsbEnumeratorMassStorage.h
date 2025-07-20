#ifndef _C_MSENUMERATOR_H_
#define _C_MSENUMERATOR_H_

#include "IEnumerator.h"
#include "C_CheckerMassStorage.h"
#include "Win_CriticalSection.h"

namespace DeviceDetectLibrary
{
    namespace Connection
    {
        class UsbEnumeratorMassStorage
			: public IEnumerator
        {
			typedef std::map<DeviceInfo::DeviceId, CheckerMassStorage_Ptr> CheckersTable;
			
			ICollector& m_collector;            
            CheckersTable m_checkers;
            CriticalSection m_criticalSec;

        public:
            UsbEnumeratorMassStorage(ICollector& collector);
            ~UsbEnumeratorMassStorage();

        public: 
            void TryThis(const DeviceInfo::DeviceId& devicePath);
            void RemoveThis(const DeviceInfo::DeviceId& devicePath);
		
		public: // IEnumerator implementation
            void Collect(const DeviceInfo& deviceInfo);

        };

        typedef shared_ptr<UsbEnumeratorMassStorage> UsbEnumeratorMassStorage_Ptr;
    }
}


#endif //_C_MSENUMERATOR_H_
