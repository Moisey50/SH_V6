#ifndef _C_MODEM_ENUMERATOR_H_
#define _C_MODEM_ENUMERATOR_H_

#include "C_CheckerModemDevice.h"

namespace DeviceDetectLibrary
{
    namespace Connection
    {
        class UsbEnumeratorModemDevice : public IEnumerator
        {
		private:
			typedef std::map<std::wstring, DeviceDetectLibrary::ModemDeviceNameCheckers> ModemCheckersTable;

			ModemCheckersTable m_modemParsers;            
			ICollector&        m_collector;            
            
        public:
            UsbEnumeratorModemDevice(ICollector& collector);
            ~UsbEnumeratorModemDevice();

        public: // IEnumerator

            void Collect(const DeviceInfo& deviceInfo);            

        private:


        };
    }
}



#endif // _C_MODEM_ENUMERATOR_H_