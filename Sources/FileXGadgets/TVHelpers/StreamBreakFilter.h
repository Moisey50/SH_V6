// StreamBreakFilter.h: interface for the SwitchFilter class.
//
//////////////////////////////////////////////////////////////////////


#pragma once
#pragma comment( lib, "Winmm" ) // adds  win multi-media timer from the winmm.lib (other way is to add this lib file to Additional Dependences)

#include <gadgets\gadbase.h>


class StreamBreak : public CFilterGadget
{
	bool               m_bDoBreakStream;
    bool               m_bAutoReleaseBreak;
    bool               m_bAutoSetBreak;
    int                m_iAutoSetBreakDelay;
	CDuplexConnector  *m_pControl;
    FXLockObject	   m_Lock;
    UINT               m_uResolution;
    UINT               m_idEvent;

    void LaunchAutoBreak( int iAutoBreakDelay );
    void BreakStream( const CDataFrame *cpEOS = 0 );
    void StopAutoBreak();
    static void CALLBACK OnTimerTick(UINT wTimerID, UINT msg,DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
    
    FXString GetCmboboxString(const FXString& cBxName);
    bool ScanSettings(FXString& txt);
    bool ScanProperties(LPCTSTR txt, bool& Invalidate);
    bool PrintProperties(FXString& txt);

    DECLARE_RUNTIME_GADGET(StreamBreak);

public:
	virtual void ShutDown();
	virtual int GetDuplexCount()
	{
		return 1;
	}

	virtual CDuplexConnector* GetDuplexConnector(int n)
	{
		return ((!n) ? m_pControl : NULL);
	}
    void MMTimerHandler(UINT nTimerID);
protected:
	StreamBreak();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
    
};