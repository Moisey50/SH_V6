#ifndef CINPUTPINWRRAPPER_INC
#define CINPUTPINWRRAPPER_INC

class CInputPinWrrapper :
	public CGadget
{
protected:
	COutputConnector *m_pOutput;
private:
	CInputPinWrrapper(void);
public:
	void ShutDown();
	virtual int GetInputsCount() { return 0; }
	virtual int GetOutputsCount() { return 1; }
	virtual CInputConnector*    GetInputConnector(int n) { return NULL; }
	virtual COutputConnector*   GetOutputConnector(int n) { return ((n==0)?m_pOutput:NULL); }
	BOOL    Send(CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(CInputPinWrrapper);
};

#endif //CINPUTPINWRRAPPER_INC