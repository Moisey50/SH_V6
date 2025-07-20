#ifndef _C_USBENUMERATOR_H_
#define _C_USBENUMERATOR_H_

#include "ICollector.h"
#include "C_UsbEnumeratorBase.h"

namespace DeviceDetectLibrary
{
    class NotifyWindow;

    namespace Connection
    {
        class UsbEnumeratorCollection : public UsbEnumeratorBase
        {
            typedef UsbEnumeratorBase Base;

        public:
            UsbEnumeratorCollection(const NotifyWindow& window, ICollector& collector);
        };
    }
}


#endif //  _C_TESTENUMERATOR_H_
