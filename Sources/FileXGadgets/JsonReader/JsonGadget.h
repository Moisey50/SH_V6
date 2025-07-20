#pragma once
#include "helpers\UserBaseGadget.h"
#include "json\JFigures.h"

#define THIS_MODULENAME "JsonGadget"

#pragma region | Logger Defines |



#define JSN_SENDLOG_3(vrbsty,sz,a,b,c)  FxSendLogMsg(vrbsty,THIS_MODULENAME,0,sz,a,b,c)

#define JSN_SENDINFO_3(sz,a,b,c)   JSN_SENDLOG_3(MSG_INFO_LEVEL,sz,a,b,c)
#define JSN_SENDINFO_2(sz,a,b)     JSN_SENDINFO_3(sz,a,b,"")
#define JSN_SENDINFO_1(sz,a)       JSN_SENDINFO_2(sz,a,"")
#define JSN_SENDINFO_0(sz)         JSN_SENDINFO_1(sz,"")

#define JSN_SENDTRACE_3(sz,a,b,c)  JSN_SENDLOG_3(MSG_DEBUG_LEVEL,sz,a,b,c)
#define JSN_SENDTRACE_2(sz,a,b)    JSN_SENDTRACE_3(sz,a,b,"")
#define JSN_SENDTRACE_1(sz,a)      JSN_SENDTRACE_2(sz,a,"")
#define JSN_SENDTRACE_0(sz)        JSN_SENDTRACE_1(sz,"")

#define JSN_SENDWARN_3(sz,a,b,c)   JSN_SENDLOG_3(MSG_WARNING_LEVEL,sz,a,b,c)
#define JSN_SENDWARN_2(sz,a,b)     JSN_SENDWARN_3(sz,a,b,"")
#define JSN_SENDWARN_1(sz,a)       JSN_SENDWARN_2(sz,a,"")
#define JSN_SENDWARN_0(sz)         JSN_SENDWARN_1(sz,"")
								   
#define JSN_SENDERR_3(sz,a,b,c)    JSN_SENDLOG_3(MSG_ERROR_LEVEL,sz,a,b,c)
#define JSN_SENDERR_2(sz,a,b)      JSN_SENDERR_3(sz,a,b,"")
#define JSN_SENDERR_1(sz,a)        JSN_SENDERR_2(sz,a,"")
#define JSN_SENDERR_0(sz)          JSN_SENDERR_1(sz,"")
								   
#define JSN_SENDFAIL_3(sz,a,b,c)   JSN_SENDLOG_3(MSG_CRITICAL_LEVEL,sz,a,b,c)
#define JSN_SENDFAIL_2(sz,a,b)     JSN_SENDFAIL_3(sz,a,b,"")
#define JSN_SENDFAIL_1(sz,a)       JSN_SENDFAIL_2(sz,a,"")
#define JSN_SENDFAIL_0(sz)         JSN_SENDFAIL_1(sz,"")
#pragma endregion | Logger Defines |

#pragma region | Defines |

#define PROP_NAME_JSON_PATH      ("JSON_Path")
#define PROP_NAME_JSON_AUTO_SAVE ("JSON_AutoSave")

#pragma endregion | Defines |



class JsonGadget
	: public UserBaseGadget
{
#pragma region | Fields |
private:
protected:
  static     /*Locker*/ FXLockObject    m_Locker;

  bool       m_bIsJsonLoaded;
  FXString   m_jsonPath;

  JFigures   m_jsonFigures;
  bool       m_toAutoSaveJsonFigures;
public:
#pragma endregion | Fields |

#pragma region | Constructors |
private:
protected:
	JsonGadget(void);
public:
	DECLARE_RUNTIME_GADGET(JsonGadget);

	virtual ~JsonGadget(void);
#pragma endregion | Constructors |

#pragma region | Methods |
private:
	void HandleFrame();
	void HandleFrameText(const CTextFrame& data);
	void HandleFrameQuantity(const CQuantityFrame& data);

protected:
	void SendOut( COutputConnector* pOutConnector, const FXString &label, const FXString& data, long frameId = NOSYNC_FRAME )
	{
		if (pOutConnector)
		{
			CTextFrame* pTxtFrm=CTextFrame::Create(data);
			pTxtFrm->SetLabel(label);
			pTxtFrm->SetTime(GetGraphTime() * 1.e-3);
			pTxtFrm->ChangeId(frameId);

			if (!pOutConnector->Put(pTxtFrm))
				pTxtFrm->RELEASE(pTxtFrm);
		}
	}

	bool ScanProperties(LPCTSTR text, bool& Invalidate)
	{
		UserGadgetBase::ScanProperties(text, Invalidate);

		FXPropertyKit pc(text);

		FXString cacheJsonPath = m_jsonPath;

		pc.GetString(PROP_NAME_JSON_PATH, m_jsonPath);
		pc.GetBool(PROP_NAME_JSON_AUTO_SAVE, m_toAutoSaveJsonFigures);

		if (m_bIsJsonLoaded && !cacheJsonPath.CompareNoCase(m_jsonPath))
			m_bIsJsonLoaded = false;
		return true;
	}

	bool PrintProperties(FXString& text)
	{
		UserGadgetBase::PrintProperties(text);

		FXPropertyKit pc;

		pc.WriteString(PROP_NAME_JSON_PATH, m_jsonPath);
		pc.WriteBool(PROP_NAME_JSON_AUTO_SAVE, m_toAutoSaveJsonFigures);

		text += pc;

		return true;
	}

	bool ScanSettings(FXString& text)
	{		
		text.Format("template"
			"(%s(%s, %s),"
			"%s(%s(False(false),True(true))))"
			, SETUP_EDITBOX, PROP_NAME_JSON_PATH, m_jsonPath
			, SETUP_COMBOBOX, PROP_NAME_JSON_AUTO_SAVE
			);
		return true;
	}

public:
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
#pragma endregion | Methods Public |
};

