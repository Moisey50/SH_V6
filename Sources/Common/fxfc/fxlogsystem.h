#ifndef FXLOGSYSTEM_INCLUDE
#define FXLOGSYSTEM_INCLUDE
// fxlogsystem.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum
{
  MSG_INFO_LEVEL = 1 ,
  MSG_DEBUG_LEVEL = 3 ,
  MSG_WARNING_LEVEL = 5 ,
  MSG_ERROR_LEVEL = 7 ,
  MSG_CRITICAL_LEVEL = 9 ,
  MSG_SYSTEM_LEVEL = 10 ,
  MSG_NOT_REGISTERED = 101
};

void FXFC_EXPORT FxSendLogMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR lpszFormat , ... );

#define SENDLOGMSG FxSendLogMsg
/*
#define SENDINFO_0(sz)          FxSendLogMsg(MSG_INFO_LEVEL,THIS_MODULENAME,0,sz)
#define SENDINFO_1(sz,a)        FxSendLogMsg(MSG_INFO_LEVEL,THIS_MODULENAME,0,sz,a)
#define SENDINFO_2(sz,a,b)      FxSendLogMsg(MSG_INFO_LEVEL,THIS_MODULENAME,0,sz,a,b)
#define SENDINFO_3(sz,a,b,c)    FxSendLogMsg(MSG_INFO_LEVEL,THIS_MODULENAME,0,sz,a,b,c)

#define SENDTRACE_0(sz)         FxSendLogMsg(MSG_DEBUG_LEVEL,THIS_MODULENAME,0,sz)
#define SENDTRACE_1(sz,a)       FxSendLogMsg(MSG_DEBUG_LEVEL,THIS_MODULENAME,0,sz,a)
#define SENDTRACE_2(sz,a,b)     FxSendLogMsg(MSG_DEBUG_LEVEL,THIS_MODULENAME,0,sz,a,b)
#define SENDTRACE_3(sz,a,b,c)   FxSendLogMsg(MSG_DEBUG_LEVEL,THIS_MODULENAME,0,sz,a,b,c)

#define SENDWARN_0(sz)          FxSendLogMsg(MSG_WARNING_LEVEL,THIS_MODULENAME,0,sz)
#define SENDWARN_1(sz,a)        FxSendLogMsg(MSG_WARNING_LEVEL,THIS_MODULENAME,0,sz,a)
#define SENDWARN_2(sz,a,b)      FxSendLogMsg(MSG_WARNING_LEVEL,THIS_MODULENAME,0,sz,a,b)
#define SENDWARN_3(sz,a,b,c)    FxSendLogMsg(MSG_WARNING_LEVEL,THIS_MODULENAME,0,sz,a,b,c)

#define SENDERR_0(sz)           FxSendLogMsg(MSG_ERROR_LEVEL,THIS_MODULENAME,0,sz)
#define SENDERR_1(sz,a)         FxSendLogMsg(MSG_ERROR_LEVEL,THIS_MODULENAME,0,sz,a)
#define SENDERR_2(sz,a,b)       FxSendLogMsg(MSG_ERROR_LEVEL,THIS_MODULENAME,0,sz,a,b)
#define SENDERR_3(sz,a,b,c)     FxSendLogMsg(MSG_ERROR_LEVEL,THIS_MODULENAME,0,sz,a,b,c)

#define SENDFAIL_0(sz)          FxSendLogMsg(MSG_CRITICAL_LEVEL,THIS_MODULENAME,0,sz)
#define SENDFAIL_1(sz,a)        FxSendLogMsg(MSG_CRITICAL_LEVEL,THIS_MODULENAME,0,sz,a)
#define SENDFAIL_2(sz,a,b)      FxSendLogMsg(MSG_CRITICAL_LEVEL,THIS_MODULENAME,0,sz,a,b)
#define SENDFAIL_3(sz,a,b,c)    FxSendLogMsg(MSG_CRITICAL_LEVEL,THIS_MODULENAME,0,sz,a,b,c)
*/

#define SENDINFO(sz,...) 		FxSendLogMsg(MSG_INFO_LEVEL,THIS_MODULENAME,0,sz,##__VA_ARGS__)
#define SENDTRACE(sz,...) 	FxSendLogMsg(MSG_DEBUG_LEVEL,THIS_MODULENAME,0,sz,##__VA_ARGS__)
#define SENDWARN(sz,...) 		FxSendLogMsg(MSG_WARNING_LEVEL,THIS_MODULENAME,0,sz,##__VA_ARGS__)
#define SENDERR(sz,...) 		FxSendLogMsg(MSG_ERROR_LEVEL,THIS_MODULENAME,0,sz,##__VA_ARGS__)
#define SENDFAIL(sz,...) 		FxSendLogMsg(MSG_CRITICAL_LEVEL,THIS_MODULENAME,0,sz,##__VA_ARGS__)

#define SENDINFO_0(sz)          SENDINFO(sz)
#define SENDINFO_1(sz,a)        SENDINFO(sz,a)
#define SENDINFO_2(sz,a,b)      SENDINFO(sz,a,b)
#define SENDINFO_3(sz,a,b,c)    SENDINFO(sz,a,b,c)

#define SENDTRACE_0(sz)         SENDTRACE(sz)
#define SENDTRACE_1(sz,a)       SENDTRACE(sz,a)
#define SENDTRACE_2(sz,a,b)     SENDTRACE(sz,a,b)
#define SENDTRACE_3(sz,a,b,c)   SENDTRACE(sz,a,b,c)

#define SENDWARN_0(sz)          SENDWARN(sz)
#define SENDWARN_1(sz,a)        SENDWARN(sz,a)
#define SENDWARN_2(sz,a,b)      SENDWARN(sz,a,b)
#define SENDWARN_3(sz,a,b,c)    SENDWARN(sz,a,b,c)

#define SENDERR_0(sz)           SENDERR(sz)
#define SENDERR_1(sz,a)         SENDERR(sz,a)
#define SENDERR_2(sz,a,b)       SENDERR(sz,a,b)
#define SENDERR_3(sz,a,b,c)     SENDERR(sz,a,b,c)

#define SENDFAIL_0(sz)          SENDFAIL(sz)
#define SENDFAIL_1(sz,a)        SENDFAIL(sz,a)
#define SENDFAIL_2(sz,a,b)      SENDFAIL(sz,a,b)
#define SENDFAIL_3(sz,a,b,c)    SENDFAIL(sz,a,b,c)

#define SEND_GADGET_INFO(sz,...)        FxSendLogMsg(MSG_INFO_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_GADGET_TRACE(sz,...)        FxSendLogMsg(MSG_DEBUG_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_GADGET_WARN(sz,...)        FxSendLogMsg(MSG_WARNING_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_GADGET_ERR(sz,...)        FxSendLogMsg(MSG_ERROR_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_GADGET_FAIL(sz,...)        FxSendLogMsg(MSG_CRITICAL_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)

#endif //#ifndef FXLOGSYSTEM_INCLUDE