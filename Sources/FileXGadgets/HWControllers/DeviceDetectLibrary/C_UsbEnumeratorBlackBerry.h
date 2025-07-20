#ifndef _C_BLACKBERRYUSBENUMERATOR_H_
#define _C_BLACKBERRYUSBENUMERATOR_H_

#include "C_UsbEnumeratorBase.h"

namespace DeviceDetectLibrary
{
    namespace Connection
    {
        class UsbEnumeratorBlackBerry
			: public UsbEnumeratorBase
        {
            typedef UsbEnumeratorBase Base;
        public:
            UsbEnumeratorBlackBerry(const NotifyWindow& window, ICollector& collector);
        };
    }
}

#endif _C_BLACKBERRYUSBENUMERATOR_H_