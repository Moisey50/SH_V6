// PtGrey1_81.h: interface for the PtGrey1_8 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PTGREY1_81_H__D000205F_9C32_4B5F_8033_A3FE0057E8B2__INCLUDED_)
#define AFX_PTGREY1_81_H__D000205F_9C32_4B5F_8033_A3FE0057E8B2__INCLUDED_

#include "stdafx.h"
#include "ptgrey1_8.h"
#include "PtGrey1_8Camera.h"
#include <gadgets\stdsetup.h>
#include <video\shvideo.h>
#include <cameraerrors.h>
#include <PGRFlyCapturePlus.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\1394Camera.h>
#include <PGRFlyCapture.h>

class PtGrey1_8 : public C1394Camera  
{
protected:
    FlyCaptureError      m_Error;
    FlyCaptureContext    m_flyCaptureContext;
    FlyCaptureVideoMode  m_CurrentVideoMode;
    FlyCaptureFrameRate  m_CurrentFrameRate;
    FlyCaptureInfoEx     m_CameraInfo ;
    FXLockObject         m_Lock;
    CRect                m_CurrentROI;
    CString              m_CameraID;
	  bool				         m_ErrorCaptureShown;
public:
	PtGrey1_8();

    // HW stuff
    virtual bool DriverInit();
    virtual bool EnumCameras();
    virtual bool CameraInit();
    virtual void CameraClose();

    virtual bool CameraStart();
    virtual void CameraStop();

    bool GetCameraProperty(unsigned i, FXSIZE &value, bool& bauto);
    bool SetCameraProperty(unsigned i, FXSIZE &value, bool& bauto, bool& Invalidate);

    virtual CVideoFrame* CameraDoGrab(double* StartTime);
    // Capture gadget stuff
    virtual void ShutDown();

    DECLARE_RUNTIME_GADGET(PtGrey1_8);
private: // helpers
   	FlyCaptureError StartLockNext();
    bool            queryMaxSize(int &width, int &height);
    bool            IsTriggerMode();
    bool            BuildPropertyList();
    void            GetROI(CRect& rc);
    void            SetROI(CRect& rc);
};

#endif //!defined(AFX_PTGREY1_81_H__D000205F_9C32_4B5F_8033_A3FE0057E8B2__INCLUDED_)
