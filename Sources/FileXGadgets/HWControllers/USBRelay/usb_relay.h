// usb_relay.h : main header file for the usb_relay DLL
//

#if !defined(AFX_usb_relay_H__INCLUDED_)
#define AFX_usb_relay_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"		// main symbols

#pragma region | Defines |
#define THIS_MODULENAME "usb_relay_gadget"

#pragma region | Logger Defines |
#define USB_RELAY_SENDLOG_3(vrbsty,sz,a,b,c) FxSendLogMsg(vrbsty,THIS_MODULENAME,0,sz,a,b,c)

#define USB_RELAY_SENDINFO_3(sz,a,b,c)  USB_RELAY_SENDLOG_3(MSG_INFO_LEVEL,sz,a,b,c)
#define USB_RELAY_SENDINFO_2(sz,a,b)    USB_RELAY_SENDINFO_3(sz,a,b,"")
#define USB_RELAY_SENDINFO_1(sz,a)      USB_RELAY_SENDINFO_2(sz,a,"")
#define USB_RELAY_SENDINFO_0(sz)        USB_RELAY_SENDINFO_1(sz,"")

#define USB_RELAY_SENDTRACE_3(sz,a,b,c)  USB_RELAY_SENDLOG_3(MSG_DEBUG_LEVEL,sz,a,b,c)
#define USB_RELAY_SENDTRACE_2(sz,a,b)    USB_RELAY_SENDTRACE_3(sz,a,b,"")
#define USB_RELAY_SENDTRACE_1(sz,a)      USB_RELAY_SENDTRACE_2(sz,a,"")
#define USB_RELAY_SENDTRACE_0(sz)        USB_RELAY_SENDTRACE_1(sz,"")

#define USB_RELAY_SENDWARN_3(sz,a,b,c)  USB_RELAY_SENDLOG_3(MSG_WARNING_LEVEL,sz,a,b,c)
#define USB_RELAY_SENDWARN_2(sz,a,b)    USB_RELAY_SENDWARN_3(sz,a,b,"")
#define USB_RELAY_SENDWARN_1(sz,a)      USB_RELAY_SENDWARN_2(sz,a,"")
#define USB_RELAY_SENDWARN_0(sz)        USB_RELAY_SENDWARN_1(sz,"")

#define USB_RELAY_SENDERR_3(sz,a,b,c)  USB_RELAY_SENDLOG_3(MSG_ERROR_LEVEL,sz,a,b,c)
#define USB_RELAY_SENDERR_2(sz,a,b)    USB_RELAY_SENDERR_3(sz,a,b,"")
#define USB_RELAY_SENDERR_1(sz,a)      USB_RELAY_SENDERR_2(sz,a,"")
#define USB_RELAY_SENDERR_0(sz)        USB_RELAY_SENDERR_1(sz,"")

#define USB_RELAY_SENDFAIL_3(sz,a,b,c)  USB_RELAY_SENDLOG_3(MSG_CRITICAL_LEVEL,sz,a,b,c)
#define USB_RELAY_SENDFAIL_2(sz,a,b)    USB_RELAY_SENDFAIL_3(sz,a,b,"")
#define USB_RELAY_SENDFAIL_1(sz,a)      USB_RELAY_SENDFAIL_2(sz,a,"")
#define USB_RELAY_SENDFAIL_0(sz)        USB_RELAY_SENDFAIL_1(sz,"")
#pragma endregion | Logger Defines |

#define DIGITAL_COMMAND_NAME_ON           ("ON")
#define DIGITAL_COMMAND_NAME_OFF          ("OFF")
#define DIGITAL_COMMAND_NAME_TOGGLE       ("TOGGLE")

#define DIGITAL_COMMAND_VAL_UNDEF        (-1)
#define DIGITAL_COMMAND_VAL_OFF           (0)
#define DIGITAL_COMMAND_VAL_ON            (1)


#define DUPLEX_COMMAND_NAME_GET       ("get")
#define DUPLEX_COMMAND_NAME_GET_OUT   ("getOut")

#define DUPLEX_COMMAND_PARAM_CHANNEL  ("channel")
#define DUPLEX_COMMAND_PARAM_CHANNELS ("channels")

#define LIST_DELIMITER                (";")

#define PROP_NAME_RELAY_DVC           ("USBRelay_Device")
#define PROP_NAME_RELAY_STATE_AS_MASK ("State_As_Mask")
#define PROP_NAME_RELAY_CMMND_AS_MASK ("Cmmnd_As_Mask")

#define PROP_INDEX_NOT_SELECTED       ("-----")
#pragma endregion | Defines |

extern char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];

#endif // !defined(AFX_usb_relay_H__INCLUDED_)
