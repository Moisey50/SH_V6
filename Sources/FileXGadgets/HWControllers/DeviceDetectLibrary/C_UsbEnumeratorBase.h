#ifndef _C_AUSBENUMERATOR_H_
#define _C_AUSBENUMERATOR_H_

#include <memory>
#include <windows.h>
#include <string>
#include <map>

#include "IEnumerator.h"
#include "ICollector.h"



namespace DeviceDetectLibrary
{
    class NotifyWindow;

    namespace Connection
    {
        class UsbEnumeratorBase
			: public IEnumerator
        {
		private:
			struct GuidTblPair //non copyable
			{
			private:
				HDEVNOTIFY m_devNotify;
				DISALLOW_COPY_AND_ASSIGN(GuidTblPair);

			public:
				GUID Guid;
				IEnumerator_Ptr Enumerator;

			public:
				GuidTblPair(const GUID& guid, IEnumerator_Ptr enumerator, const NotifyWindow& window ); 
				~GuidTblPair();
			};

			typedef shared_ptr<GuidTblPair> GTPair_Ptr;
			typedef vector<GTPair_Ptr> GuidTable;

			GuidTable   m_table;
			ICollector& m_collector;

		protected:
			void CopyCollector(const UsbEnumeratorBase& other);

        public:
            UsbEnumeratorBase(ICollector& collector); 
            virtual ~UsbEnumeratorBase();

            virtual void TryThis(const GUID& guid); 
            virtual void AddCollector(const GUID& guid, IEnumerator_Ptr enumerator, const NotifyWindow& window);

        public: // IEnumerator implementation
            virtual void Collect(const DeviceInfo& deviceInfo);
        };

        typedef shared_ptr<UsbEnumeratorBase> UsbEnumeratorBase_Ptr;
    }
}

#endif _C_AUSBENUMERATOR_H_