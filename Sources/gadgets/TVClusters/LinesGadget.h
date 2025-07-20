// LinesGadget.h: interface for the Lines class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINESGADGET_H__255CD09A_8161_4A57_9AE9_59893235BDFC__INCLUDED_)
#define AFX_LINESGADGET_H__255CD09A_8161_4A57_9AE9_59893235BDFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <Gadgets\shkernel.h>
#include <Gadgets\VideoFrame.h>
#include <helpers\FramesHelper.h>
#include <imageproc\LineDetector\StrictLines.h>

class Lines : public CFilterGadget  
{
private:
    bool         m_FormatErrorProcessed;
    DWORD        m_LastFormat;
public:
	Lines();
	virtual void ShutDown();
protected:
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    DECLARE_RUNTIME_GADGET(Lines);
};

#endif // !defined(AFX_LINESGADGET_H__255CD09A_8161_4A57_9AE9_59893235BDFC__INCLUDED_)
