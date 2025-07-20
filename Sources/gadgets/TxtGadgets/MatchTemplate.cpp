#include "stdafx.h"
#include "MatchTemplate.h"
#include "TxtGadgets.h"
#include <gadgets\textframe.h>
#include <gadgets\ContainerFrame.h>


BOOL CMatchFind::DoMatchPlainText(FXString& text, FXString& plainText)
{
	FXSIZE pos = text.Find(plainText);
	if (pos < 0)
		return FALSE;
	if (m_nStartExpected >= 0 && m_nStartExpected != pos)
		return FALSE;
	if (pos > 0 && !m_CurVar.IsEmpty())
	{
		FXString var = text.Left(pos - m_nQuestions);
		m_pkVars.WriteString(m_CurVar, var);
	}
	text.Delete(0, pos + plainText.GetLength());
	m_CurTemplate.Delete(0, plainText.GetLength() - m_FirstChar.GetLength());
	m_nStartExpected = 0;
	m_CurVar.Empty();
	m_nQuestions = 0;
	return TRUE;
}

BOOL CMatchFind::DoMatchQuestion(FXString& text)
{
	m_CurTemplate.Delete(0);
	if (m_nStartExpected >= 0)
		m_nStartExpected++;
	m_nQuestions++;
	return TRUE;
}

BOOL CMatchFind::DoMatchAsterisc(FXString& text)
{
	m_CurTemplate.Delete(0);
	m_nStartExpected = -1;
	return TRUE;
}

BOOL CMatchFind::DoMatchDollar(FXString& text)
{
	if (!m_CurVar.IsEmpty())
		return FALSE;
	m_CurTemplate.Delete(0);
	FXSIZE pos = m_CurTemplate.Find('$');
	if (pos < 0)
		return FALSE;
	m_CurVar = m_CurTemplate.Left(pos);
	m_CurTemplate.Delete(0, pos + 1);
	text.Delete(0, m_nQuestions);
	m_nQuestions = 0;
	m_nStartExpected = -1;
	return TRUE;
}

BOOL CMatchFind::DoMatchBackSlash(FXString& text)
{
	m_CurTemplate.Delete(0);
	if (m_CurTemplate.IsEmpty())
		return FALSE;
	m_FirstChar = m_CurTemplate.GetAt(0);
	m_CurTemplate.Delete(0);
	m_nStartExpected = 0;
	return TRUE;
}

BOOL CMatchFind::DoMatch(FXString& text, FXString& tmpl, FXString& vars)
{
	const char meta[] = "?*$\\";
	m_pkVars.Empty();
	m_nStartExpected = 0;
	m_CurTemplate = tmpl;
	m_nQuestions = 0;
	m_CurVar.Empty();
	while (!m_CurTemplate.IsEmpty())
	{
		BOOL bResult = TRUE;
		m_FirstChar.Empty();
		switch (m_CurTemplate[0])
		{
		case '?':
			bResult = DoMatchQuestion(text);
			break;
		case '*':
			bResult = DoMatchAsterisc(text);
			break;
		case '$':
			bResult = DoMatchDollar(text);
			break;
		case '\\':
			bResult = DoMatchBackSlash(text);
			break;
		default:
			break;
		}
		if (!bResult)
			return FALSE;
		FXString plainText = m_FirstChar + m_CurTemplate.SpanExcluding(meta);
		if (!plainText.IsEmpty())
		{
			if (!DoMatchPlainText(text, plainText))
				return FALSE;
		}
	}
	if (!m_CurVar.IsEmpty())
	{
		FXString var = text.Mid(0, text.GetLength() - m_nQuestions);
		text.Delete(0, var.GetLength());
		m_pkVars.WriteString(m_CurVar, var);
	}
	vars = m_pkVars;
	return TRUE;
}


IMPLEMENT_RUNTIME_GADGET_EX(MatchTemplate, CFilterGadget,  "Text.conversion", TVDB400_PLUGIN_NAME);

MatchTemplate::MatchTemplate()
{
	m_pOutput = new COutputConnector(text);
	m_pInput = new CInputConnector(text);
	Resume();
}

void MatchTemplate::ShutDown()
{
	CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

bool MatchTemplate::PrintProperties(FXString& text)
{
	FXPropertyKit pk(text);
	pk.WriteString("Template", m_Template);
	text = pk;
	return true;
}

bool MatchTemplate::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	FXPropertyKit pk(text);
	pk.GetString("Template", m_Template);
	return true;
}

bool MatchTemplate::ScanSettings(FXString& text)
{
	text.Format("template(EditBox(Template))");
	return true;
}

CDataFrame* MatchTemplate::DoProcessing(const CDataFrame* pDataFrame)
{
	const CTextFrame* Frame = pDataFrame->GetTextFrame();
	CContainerFrame* Result = NULL;
	if (Frame)
	{
		Result = CContainerFrame::Create();
        Result->CopyAttributes(Frame);

		FXString text = Frame->GetString(), vars;
		while (DoMatch(text, m_Template, vars))
		{
			CTextFrame* MatchFrame = CTextFrame::Create(vars);
            MatchFrame->CopyAttributes(Frame);
			Result->AddFrame(MatchFrame);
		}
	}
	return Result;
}
