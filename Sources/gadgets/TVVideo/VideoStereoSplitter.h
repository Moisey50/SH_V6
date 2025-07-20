// VideoStereoSplitter.h: interface for the VideoStereoSplitter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEOSTEREOSPLITTER_H__A4AD1CB4_9A53_487E_8E8F_ABD5A0B5FE50__INCLUDED_)
#define AFX_VIDEOSTEREOSPLITTER_H__A4AD1CB4_9A53_487E_8E8F_ABD5A0B5FE50__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>

class VideoStereoSplitter : public CGadget  
{
private:
    CInputConnector* m_pInput;
    COutputConnector* m_pOutputs[2];
public:
	VideoStereoSplitter();
	virtual void ShutDown();
    virtual int GetInputsCount();
	virtual int GetOutputsCount();
	virtual CInputConnector* GetInputConnector(int n);
	virtual COutputConnector* GetOutputConnector(int n);
	virtual int DoJob();

    DECLARE_RUNTIME_GADGET(VideoStereoSplitter);
};

#endif // !defined(AFX_VIDEOSTEREOSPLITTER_H__A4AD1CB4_9A53_487E_8E8F_ABD5A0B5FE50__INCLUDED_)
