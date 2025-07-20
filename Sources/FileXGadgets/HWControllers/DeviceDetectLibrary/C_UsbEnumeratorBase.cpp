#include "StdAfx.h"
#include "C_UsbEnumeratorBase.h"

using namespace DeviceDetectLibrary;
using namespace DeviceDetectLibrary::Connection;
using namespace DeviceDetectLibrary;

UsbEnumeratorBase::UsbEnumeratorBase(ICollector& collector)
	:  m_collector(collector)
{
}

UsbEnumeratorBase::~UsbEnumeratorBase()
{
}

void UsbEnumeratorBase::TryThis(const GUID& guid)
{
    struct Predicate_GuidEqual
    {
	private:
		const GUID& m_guid;

	public:
        Predicate_GuidEqual(const GUID& guid)
			: m_guid(guid)
		{}

        bool operator()(const GTPair_Ptr& item) const
        {
            return memcmp(&(item->Guid), &m_guid, sizeof(m_guid)) == 0;
        }
    };

    try
    {
        GuidTable::const_iterator ci_guidTbl = std::find_if(m_table.begin(), m_table.end(), Predicate_GuidEqual(guid));
        if (ci_guidTbl != m_table.end())
        {
            ConnectionInfo_vt result = GetDevicesByGuid(guid);
            for (ConnectionInfo_vt::const_iterator ci_connInfo = result.begin(); ci_connInfo != result.end(); ++ci_connInfo)
            {
                (*ci_guidTbl)->Enumerator->Collect(DeviceInfo(ci_connInfo->DevicePath, ci_connInfo->FriendlyName, L"Unknown device", *ci_connInfo));
            }
        }
    }
    catch (const std::exception&)
    {
        // ERROR: in detection
    }
}

void UsbEnumeratorBase::AddCollector(const GUID& guid, IEnumerator_Ptr enumerator, const NotifyWindow& window)
{
	m_table.push_back(GTPair_Ptr(new GuidTblPair(guid, enumerator, window)));
}


void UsbEnumeratorBase::Collect(const DeviceInfo& deviceInfo) 
{
    struct PredicateCaller 
    {
	private:
        ICollector&     m_collector;
        UsbEnumeratorBase& m_enumerator;

	public:
        PredicateCaller(ICollector& collector, UsbEnumeratorBase& enumerator) 
            : m_collector(collector) 
            , m_enumerator(enumerator)
        {}

        void operator() (const GuidTable::const_iterator::value_type& pair)
        {
            m_enumerator.TryThis(pair->Guid);
        }
    };

    for_each(m_table.begin(), m_table.end(), PredicateCaller(m_collector, *this));
}


UsbEnumeratorBase::GuidTblPair::GuidTblPair(const GUID& guid, IEnumerator_Ptr enumerator, const NotifyWindow& window)
	: Guid(guid)
	, Enumerator(enumerator)
	, m_devNotify(RegisterUSBNotify(guid, window.GetHWND()))
{
}

UsbEnumeratorBase::GuidTblPair::~GuidTblPair()
{
    try
    {
        UnRegisterUSBNotify(m_devNotify);
    }
    catch (const std::exception& /*e*/)
    {
        // ERROR: GTPair::~GTPair fails (%s)", e.what());
    }
}

void DeviceDetectLibrary::Connection::UsbEnumeratorBase::CopyCollector( const UsbEnumeratorBase& other )
{
	m_table.insert(m_table.end(), other.m_table.begin(), other.m_table.end());
}
