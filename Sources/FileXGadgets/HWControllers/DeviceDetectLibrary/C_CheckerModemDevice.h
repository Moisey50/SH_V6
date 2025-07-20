#ifndef _C_MODEMDEVICECHECKER_H_
#define _C_MODEMDEVICECHECKER_H_


namespace DeviceDetectLibrary
{

    class CheckerModemDevice
	{
		ICollector&               m_collector;
		std::vector<std::wstring> m_patternNames;

    public:
        CheckerModemDevice(ICollector& collector, const std::vector<std::wstring>& patternNames);
        virtual ~CheckerModemDevice();

    protected:
        bool virtual Check( const DeviceInfo& deviceInfo );

	public:
        void virtual Collect(const DeviceInfo& deviceInfo, const std::wstring& pluginID); 
    };
    typedef shared_ptr<CheckerModemDevice> CheckerModemDeviceName_Ptr;
    typedef vector<CheckerModemDeviceName_Ptr> ModemDeviceNameCheckers;
}
#endif // _C_MODEMDEVICECHECKER_H_