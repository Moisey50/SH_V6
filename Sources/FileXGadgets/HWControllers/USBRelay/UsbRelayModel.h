#pragma once
#include <string>
#include <sstream>

using namespace std;


typedef struct UsbRelayInfo
{
#pragma region | Fields |
private:
	string m_serialNum;
	string m_path;
	int m_iNumChannels;
#pragma endregion | Fields |

#pragma region | Constructors |
public:
	struct UsbRelayInfo
		( const string& deviceSN
		, const string& devicePath
		, int channels)
		: m_serialNum(deviceSN)
		, m_path(devicePath)
		, m_iNumChannels(channels)
	{}
#pragma endregion | Constructors |

#pragma region | Methods Private |
private:

#pragma endregion | Methods Private |
	
#pragma region | Methods Public |
public:

	bool IsValid() const
	{
		return m_serialNum.length() > 0;
	}

	const string& GetSerialNum() const
	{
		return m_serialNum;
	}
	const string& GetDevicePath() const{return m_path;}
	int GetNumChannels() const{return m_iNumChannels;}

	bool operator<(const UsbRelayInfo& other) const
	{
		return GetSerialNum().compare(other.GetSerialNum()) < 0;
	}
	string ToString() const
	{
		ostringstream oss;
		oss << "USB-Relay-"
			<< GetNumChannels()
			<< "ch_SN#"
			<< GetSerialNum()
			<< "("
			<< GetSerialNum()
			<< ")";
		return oss.str();
	}
#pragma endregion | Methods Public |

}*LPUsbRelayInfo;

