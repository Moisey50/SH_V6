// avt2_11Camera.h: interface for the avt2_11 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVT2_11CAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)
#define AFX_AVT2_11CAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <FGCamera.h>
#include <gadgets\1394Camera.h>

class avt2_11 : public C1394Camera  
{
protected:
    CFGCamera   m_Camera;
    UINT        m_LastError;
    FG_COLORMODE    m_Format;
    FG_FRAMERATE    m_FrameRate;
    FG_RESOLUTION   m_Resolution;
    BITMAPINFOHEADER m_BMIH;
    BITMAPINFOHEADER m_RealBMIH;
    CRect           m_CurrentROI;
    CLockObject     m_Lock;
    CString         m_CameraID;
    bool            FrameInfoOn;
public:
	avt2_11();
    // HW stuff
    virtual bool DriverInit();
    virtual bool CameraInit();
    virtual void CameraClose();

    virtual bool CameraStart();
    virtual void CameraStop();

    bool GetCameraProperty(unsigned i, int &value, bool& bauto);
    bool SetCameraProperty(unsigned i, int &value, bool& bauto, bool& Invalidate);

    virtual CVideoFrame* CameraDoGrab();
    // Capture gadget stuff
    virtual void ShutDown();

    DECLARE_RUNTIME_GADGET(avt2_11);
private: // helpers
        bool            BuildPropertyList();
        bool            CameraConnected() { return (m_Camera.GetPtrDCam()!=NULL); };
        bool            IsTriggerMode();
        int             GetTriggerMode();
        void            SetTriggerMode(int iMode);
        void            GetROI(CRect& rc);
        bool            SetROI(CRect& rc);
        int             GetLongExposure() ;
        void            SetLongExposure( int iExp ) ;
        void            SetSendFrameInfo( int iSend ) ;
        DWORD           IsFrameInfoAvailable() ; // also availability check
        int             GetTriggerDelay() ;
        void            SetTriggerDelay( int iDelay_uS ) ;
        bool            SetGrab( int iNFrames ) ;
        bool            GetGrabConditions( bool& bContinuous , int& iNRestFrames ) ;
};

#endif // !defined(AFX_AVT2_11CAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)
