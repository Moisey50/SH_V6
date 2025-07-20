#ifndef _RESULT_ENUMERATOR_H_
#define _RESULT_ENUMERATOR_H_

#include "IEnumerator.h"
#include "ICollector.h"

namespace DeviceDetectLibrary
{
    class ResultEnumerator
		: public IEnumerator
	{
		ICollector   &m_collector;
		std::wstring  m_deviceID;
		std::wstring  m_friendlyName;
		

    public:
        ResultEnumerator(ICollector& collector, std::wstring deviceDisplayName, const std::wstring& friendlyName);
        virtual ~ResultEnumerator();

        void Collect(const DeviceInfo& deviceInfo);
    };
}

#endif // _RESULT_ENUMERATOR_H_