#pragma once
#include <gadgets\gadbase.h>


class CMatchFind
{
	int m_nStartExpected;
	int m_nQuestions;
	FXPropertyKit m_pkVars;
	FXString m_CurTemplate;
	FXString m_CurVar;
	FXString m_FirstChar;
public:
	CMatchFind() {};
	~CMatchFind() {};
	BOOL DoMatch(FXString& text, FXString& tmpl, FXString& vars);

private:
	BOOL DoMatchPlainText(FXString& text, FXString& plainText);
	BOOL DoMatchQuestion(FXString& text);
	BOOL DoMatchAsterisc(FXString& text);
	BOOL DoMatchDollar(FXString& text);
	BOOL DoMatchBackSlash(FXString& text);
};


class MatchTemplate : public CFilterGadget, public CMatchFind
{
	FXString m_Template;
protected:
	MatchTemplate();
public:
    virtual void ShutDown();
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    virtual bool ScanSettings(FXString& text);
private:
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    DECLARE_RUNTIME_GADGET(MatchTemplate);
};
