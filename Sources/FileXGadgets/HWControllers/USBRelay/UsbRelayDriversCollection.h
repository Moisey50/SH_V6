#pragma once

#include <map>
#include <vector>
#include <string>

#include "usb_relay_device.h"

#include "ContolablesCollectionBase.h"
#include "UsbRelayDriver.h"

using namespace std;


class UsbRelayDriversCollection
	: public ContolablesCollectionBase<UsbRelayDriver>
{	
	map<string, UsbRelayDriver*>         m_BusBySerialNum;
#pragma region | Constructors |
private:
	UsbRelayDriversCollection(const UsbRelayDriversCollection &);
	const UsbRelayDriversCollection& operator= (const UsbRelayDriversCollection &);
public:
	UsbRelayDriversCollection(IResetListener &collectionResetedListener)
		: ContolablesCollectionBase(collectionResetedListener)
	{ }

	virtual ~UsbRelayDriversCollection()
	{

	}
#pragma endregion | Constructors |

#pragma region | Methods Private |
private:
	virtual void /*ContolablesCollectionBase::*/ResetAll()
	{
		struct usb_relay_device_info *pDevicesHead = NULL;
		struct usb_relay_device_info *pDevicesIterator = NULL;
		
		vector<map<string, UsbRelayDriver*>::const_iterator> toRemove;
		map<string, struct usb_relay_device_info *> toAdd;
		
		pDevicesHead = usb_relay_device_enumerate();
		
		if(pDevicesHead!=NULL)
		{
			pDevicesIterator = pDevicesHead;
			
			while(pDevicesIterator)
			{
				toAdd[(char*)pDevicesIterator->serial_number] = pDevicesIterator;
				pDevicesIterator = pDevicesIterator->next;
			}

			map<string, UsbRelayDriver*>::const_iterator ci = m_BusBySerialNum.begin();
			
			//if(toAdd.empty())
			//	DestroyCollection();
			//else
			//{
		
			//	for(; ci != m_BusBySerialNum.end(); ci++)
			//	{
			//		UsbRelayDriver* pDevice = ci->second;
			//		if(!pDevice)
			//			toRemove.push_back(ci);
			//		else
			//		{
			//			string currentSN = pDevice->GetInfo().GetSerialNum();
		
			//			map<string, struct usb_relay_device_info *>::const_iterator cii = toAdd.find(currentSN);
		
			//			if(cii!=toAdd.end())
			//				toAdd.erase(cii);
			//			else
			//				toRemove.push_back(ci);
			//		}
			//	}
		
			//	if(!toRemove.empty())
			//		Remove(toRemove);
		
			//	if(!toAdd.empty())
					Add(toAdd);
			//}
			
			usb_relay_device_free_enumerate(pDevicesHead);
			pDevicesHead = NULL;
		
			ContolablesCollectionBase::RemoveAll();

			ci = m_BusBySerialNum.begin();
			for(; ci != m_BusBySerialNum.end(); ci++)
			{
				ContolablesCollectionBase::Add(ci->second);
			}
		}
	}

	void Add( const map<string, struct usb_relay_device_info *>& toAdd )
	{
		map<string, struct usb_relay_device_info *>::const_iterator cii = toAdd.begin();
	
		for( ; cii != toAdd.end(); cii++)
		{
			if(m_BusBySerialNum.find(cii->first)==m_BusBySerialNum.end())
			{
				UsbRelayDriver* pDev = UsbRelayDriver::Create(cii->second);

				if(pDev)
					m_BusBySerialNum[cii->first] = pDev;
			}
		}
	}


#pragma endregion | Methods Private |

#pragma region | Methods Protected |
#pragma endregion | Methods Protected |

#pragma region | Methods Public |
public:
	void RemoveByPath( const string& path )
	{
		m_Locker.Lock();
		map<string, UsbRelayDriver*>::const_iterator ci = m_BusBySerialNum.begin();
		for (; ci != m_BusBySerialNum.end(); ci++)
		{
			UsbRelayDriver *pDrvr = (ci->second);
			if(pDrvr->GetInfo().GetDevicePath().compare(path)==0)
			{
				Remove(pDrvr);
				m_BusBySerialNum.erase(ci);
				break;
			}
		}

		m_Locker.UnLock();
	}

	void /*ContolablesCollectionBase::*/DestroyCollection()
	{
		ContolablesCollectionBase::DestroyCollection();
		m_BusBySerialNum.clear();
	}

	const UsbRelayDriver* GetDeviceBySerialNum(const string& deviceSN);
	
	virtual string ToString()
	{
		string asTxt;
		vector<UsbRelayDriver*> devRefs;
		GetCollection(devRefs);
		vector<UsbRelayDriver*>::const_iterator ci = devRefs.begin();
		for ( ; ci!=devRefs.end(); ci++)
		{
			if(!asTxt.empty())
				asTxt.append(";");
			if((*ci)->IsInUse())
				asTxt.append("[x] ");
			asTxt.append((*ci)->ToString());
		}
		return asTxt;
	}

#pragma endregion | Methods Public |
};