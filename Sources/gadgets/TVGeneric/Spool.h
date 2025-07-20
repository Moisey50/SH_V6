// Spool.h : Declaration of the Spool class

#pragma once
#include <gadgets\gadbase.h>

class CBufferedInput;

class Spool : public CGadget
{
private:
	CBufferedInput* m_pInput;
	COutputConnector* m_pOutput;
    int             m_Timeout;
private:
            Spool(void);
    void    ShutDown();
public:
	int     GetInputsCount();
	int     GetOutputsCount();
	CInputConnector* GetInputConnector(int n);
	COutputConnector* GetOutputConnector(int n);

	bool    ScanProperties(LPCTSTR text, bool& Invalidate);
	bool    PrintProperties(FXString& text);
	bool    ScanSettings(FXString& text);
private:
    virtual int DoJob();
    virtual void OnEOS();
	DECLARE_RUNTIME_GADGET(Spool);
};
