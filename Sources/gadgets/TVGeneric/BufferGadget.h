// BufferGadget.h: interface for the Buffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFERGADGET_H__B83A6B04_88AC_4B94_977A_EC534A1E336B__INCLUDED_)
#define AFX_BUFFERGADGET_H__B83A6B04_88AC_4B94_977A_EC534A1E336B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets/gadbase.h>
#include <queue>

using namespace std ;

class Buffer : public CFilterGadget  
{
protected:
    BOOL                m_Transparent;
    CDuplexConnector*	m_pControl;
    FXStaticQueue<CDataFrame*> *m_pStack;
    int                 m_BufferSize;
    BOOL                m_bPureFIFO ;
    int                 m_CrntOffset;
    FXLockObject         m_Lock;
    std::queue<CDataFrame*> m_FIFO ;

private:
	CDataFrame* GetFrame(int nmb);
	void ClearStack();
  void CreateBuffer(int size);
  void ClearQueue()
  {
    while ( !m_FIFO.empty() )
    {
      CDataFrame * pFr = m_FIFO.front() ;
      pFr->Release() ;
      m_FIFO.pop() ;
    }
  }
public:
	Buffer();
	virtual void ShutDown();
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	virtual int GetDuplexCount();
	virtual CDuplexConnector* GetDuplexConnector(int n);
    virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

    bool    ScanSettings(FXString& text);
    bool    ScanProperties(LPCTSTR text, bool& Invalidate);
    bool    PrintProperties(FXString& text);

    DECLARE_RUNTIME_GADGET(Buffer);
};

#endif // !defined(AFX_BUFFERGADGET_H__B83A6B04_88AC_4B94_977A_EC534A1E336B__INCLUDED_)
