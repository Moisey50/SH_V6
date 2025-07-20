#ifndef FXEXCEPTION_INCLUDE
#define FXEXCEPTION_INCLUDE
// fxexception.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FXFC_EXPORT FXException
{
protected:
    BOOL m_bAutoDelete;
public:
	FXException();   // sets m_bAutoDelete = TRUE
	explicit FXException(BOOL bAutoDelete);   // sets m_bAutoDelete = bAutoDelete
    virtual ~FXException();
    virtual BOOL GetErrorMessage(LPTSTR lpszError,UINT nMaxError,PUINT pnHelpContext = NULL);
    virtual int ReportError(UINT nType = MB_OK,UINT nMessageID = 0);
    void Delete( );
};

#endif // #ifndef FXEXCEPTION_INCLUDE