#ifndef _WIN_CRITICAL_SECTION_H_
#define _WIN_CRITICAL_SECTION_H_

#include "Win32Types.h"
#include "Utilities.h"

namespace DeviceDetectLibrary
{
	class CriticalSection //non copyable
	{
	private:
		CRITICAL_SECTION m_criticalSection;

		DISALLOW_COPY_AND_ASSIGN(CriticalSection);

	public:
		CriticalSection();
		virtual ~CriticalSection() ;


		void Enter();
		void Leave();
	};
} 

#endif // _WIN_CRITICAL_SECTION_H_