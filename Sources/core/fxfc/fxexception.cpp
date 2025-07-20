#include "stdafx.h"
#include "fxmodules.h"
#include <fxfc\fxfc.h>

FXException::FXException()
{
    m_bAutoDelete = TRUE;
}

FXException::FXException(BOOL bAutoDelete)
{
    m_bAutoDelete = bAutoDelete;
}

FXException::~FXException()
{
}

BOOL FXException::GetErrorMessage(LPTSTR lpszError,UINT nMaxError,PUINT pnHelpContext)
{
	if (pnHelpContext != NULL) *pnHelpContext = 0;
	if (nMaxError != 0 && lpszError != NULL) *lpszError = '\0';
	return FALSE;
}

int FXException::ReportError(UINT nType,UINT nMessageID)
{
    TCHAR   szErrorMessage[512];
	int     nDisposition;
	UINT    nHelpContext;

	if (GetErrorMessage(szErrorMessage, _countof(szErrorMessage), &nHelpContext))
		nDisposition = FxMessageBox(szErrorMessage, nType, nHelpContext);
	else
	{
		if (nMessageID != 0)
        {
            FXString mes;
            HMODULE hMod; GetParentModuleHandle(&hMod);
            mes.LoadString(hMod,nMessageID);
		    nDisposition = FxMessageBox(mes, nType, nHelpContext);
        }
        else
            nDisposition = FxMessageBox(_T("No error message is available"), nType, nHelpContext);
	}
	return nDisposition;
}

void FXException::Delete( )
{
	if (m_bAutoDelete > 0)
		delete this;
}
