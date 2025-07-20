#ifndef _AUTO_CRITICAL_SECTION_H_
#define _AUTO_CRITICAL_SECTION_H_

#include "Win_CriticalSection.h"

namespace DeviceDetectLibrary
{
	class AutoCriticalSection
	{
	private:
    	CriticalSection& m_criticalSection;

		DISALLOW_COPY_AND_ASSIGN(AutoCriticalSection)

	public:
		AutoCriticalSection(CriticalSection& section);
		virtual ~AutoCriticalSection();
	};
} 

#endif // __AUTO_CRITICAL_SECTION_H_INCLUDED__

