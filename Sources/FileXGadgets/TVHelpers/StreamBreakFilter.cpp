// SwitchFilter.cpp: implementation of the SwitchFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StreamBreakFilter.h"
#include <gadgets\QuantityFrame.h>
#include <mmsystem.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SETUP_FIELD_NAME_BREAK_ON "BreakOn"
#define SETUP_FIELD_NAME_AUTO_RELEASE "AutoReleaseBreak"
#define SETUP_FIELD_NAME_AUTO_SET "AutoSetBreak"
#define SETUP_FIELD_NAME_AUTO_SET_DELAY "AutoSetBreakDelay"

#define SETUP_FIELD_AUTO_SET_DELAY_MIN 2
#define SETUP_FIELD_AUTO_SET_DELAY_MAX 2000

#define PASSTHROUGH_NULLFRAME(vfr, fr)  \
{										\
	if (!(vfr)	)					    \
{	                                    \
	return NULL;                        \
}                                       \
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(StreamBreak, CFilterGadget, "Helpers", TVDB400_PLUGIN_NAME);

StreamBreak::StreamBreak():
m_bDoBreakStream(false), m_bAutoReleaseBreak(false), m_bAutoSetBreak(false), m_iAutoSetBreakDelay(1000), m_uResolution(0), m_idEvent(0)
{                                                                         
    m_pInput   = new CInputConnector(transparent);
    m_pOutput  = new COutputConnector(transparent);
	m_pControl = new CDuplexConnector(this, transparent, transparent);
	Resume();                                                         
}                                                                     



void StreamBreak::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
    CDataFrame* pEOS = NULL;

	if (pParamFrame)
	{
        if(Tvdb400_IsEOS(pParamFrame))
        {
            pEOS=pParamFrame;
            m_bDoBreakStream = true;
        }
        else
        {
            const CBooleanFrame* bFrame = pParamFrame->GetBooleanFrame();
		
            if (bFrame)
            {
                m_bDoBreakStream = *bFrame;
                ((CBooleanFrame*)bFrame)->RELEASE((CBooleanFrame*)bFrame);
            }
            else if (m_bAutoReleaseBreak)
            {
                m_bDoBreakStream = false;
            }
            
            if(m_bAutoSetBreak)
                LaunchAutoBreak(m_iAutoSetBreakDelay);
        }
        BreakStream(pEOS);
	}
  pParamFrame->Release( pParamFrame );
}

CDataFrame* StreamBreak::DoProcessing(const CDataFrame* pDataFrame)
{
	CDataFrame* retFrame=NULL;
	PASSTHROUGH_NULLFRAME(pDataFrame, pParamFrame);

	FXAutolock al(m_Lock);
	if(!m_bDoBreakStream)
	{
		retFrame=(CDataFrame*)pDataFrame;
		retFrame->AddRef(); // should be released once in parent class
	}

	return retFrame;
}

bool StreamBreak::ScanSettings(FXString& txt)
{
    FXString strDelay;
    strDelay.Format("Spin(%s,%d,%d)", SETUP_FIELD_NAME_AUTO_SET_DELAY, SETUP_FIELD_AUTO_SET_DELAY_MIN, SETUP_FIELD_AUTO_SET_DELAY_MAX);
    txt.Format("template(%s, %s, %s, %s)", GetCmboboxString(SETUP_FIELD_NAME_BREAK_ON), GetCmboboxString(SETUP_FIELD_NAME_AUTO_RELEASE), GetCmboboxString(SETUP_FIELD_NAME_AUTO_SET), strDelay);
    return true;
}
FXString StreamBreak::GetCmboboxString(const FXString& cBxName)
{
    FXString cBx_Str;
    cBx_Str.Format("ComboBox(%s(true(%d),false(%d)))", cBxName, (int)true, (int)false);
    return cBx_Str;
}

bool StreamBreak::ScanProperties(LPCTSTR txt, bool& Invalidate)
{
    FXAutolock al(m_Lock) ;
    bool res = false;
    CFilterGadget::ScanProperties(txt, Invalidate);
    FXPropertyKit pk(txt);

    res = pk.GetInt(SETUP_FIELD_NAME_BREAK_ON, (int&)m_bDoBreakStream);
    if(pk.GetInt(SETUP_FIELD_NAME_AUTO_RELEASE, (int&)m_bAutoReleaseBreak))
        res = true;
    if(pk.GetInt(SETUP_FIELD_NAME_AUTO_SET, (int&)m_bAutoSetBreak))
        res = true;
    if(pk.GetInt(SETUP_FIELD_NAME_AUTO_SET_DELAY, m_iAutoSetBreakDelay))
        res = true;
	
    return res;
}

bool StreamBreak::PrintProperties(FXString& txt)
{
    bool res = false;
    CFilterGadget::PrintProperties(txt);
	FXPropertyKit pk;
    FXString tmp;
    if(pk.WriteInt(SETUP_FIELD_NAME_BREAK_ON, m_bDoBreakStream) &&
        pk.WriteInt(SETUP_FIELD_NAME_AUTO_RELEASE, m_bAutoReleaseBreak) &&
        pk.WriteInt(SETUP_FIELD_NAME_AUTO_SET, m_bAutoSetBreak) &&
        pk.WriteInt(SETUP_FIELD_NAME_AUTO_SET_DELAY, m_iAutoSetBreakDelay))
    {
        tmp+=pk;
        res = true;
    }

    if(res)
        txt+=tmp;
	return res;
}

void StreamBreak::ShutDown()
{
	CFilterGadget::ShutDown();
	delete m_pInput, m_pInput=NULL;
	delete m_pOutput, m_pOutput=NULL;
	delete m_pControl, m_pControl=NULL;
}

void StreamBreak::LaunchAutoBreak( int iAutoBreakDelay )
{
    StopAutoBreak();
    
    // Set resolution to the minimum supported by the system

    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    m_uResolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
    timeBeginPeriod(m_uResolution);

    // create the timer

    m_idEvent = timeSetEvent(
        m_iAutoSetBreakDelay,
        m_uResolution,
        OnTimerTick,
        (DWORD_PTR)this,
        TIME_ONESHOT);
   
}

void CALLBACK StreamBreak::OnTimerTick(UINT wTimerID, UINT msg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    StreamBreak* sender = (StreamBreak*) dwUser;
    sender->MMTimerHandler(wTimerID);
}

void StreamBreak::MMTimerHandler( UINT nTimerID )
{
    if(m_idEvent == nTimerID)
    {
        m_bDoBreakStream=true;
        BreakStream();
    }
}

void StreamBreak::BreakStream( const CDataFrame *cpEOS /*= 0 */ )
{
    if(m_bDoBreakStream)
    {
        CDataFrame* pEOS = (CDataFrame *)cpEOS;
        
        if(!pEOS)
        {
            pEOS = CDataFrame::Create(transparent);
            Tvdb400_SetEOS(pEOS);
        }

        FXAutolock al(m_Lock);
        if (m_pOutput && pEOS && (!m_pOutput->Put(pEOS)))
            pEOS->RELEASE(pEOS);
    }
}

void StreamBreak::StopAutoBreak()
{
    // destroy the timer
    timeKillEvent(m_idEvent);

    // reset the timer
    timeEndPeriod (m_uResolution);
}