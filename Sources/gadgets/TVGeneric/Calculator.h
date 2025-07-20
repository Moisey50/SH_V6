#pragma once
#include <gadgets\gadbase.h>

#ifdef _DEBUG
#define _DUMP_STACKS
#endif

class Calculator : public CCollectorGadget
{
	FXLockObject m_Lock;
	FXString m_Expression;
	int m_nVars;
	typedef double (Calculator::*CallFn)();
	FXStaticStack<CallFn> m_CallStack;
	FXStaticStack<double> m_DataStack;
	FXStaticStack<CallFn> m_InitCode;
	FXStaticStack<double> m_InitData;
	CDataFrame const*const* m_ppFrames;
	int m_nFrames;
	int m_nError;
public:
	Calculator();
	virtual void ShutDown();
	virtual CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);
	bool ScanSettings(FXString& text);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	void Restore();
	void Init();
private:
	BOOL ParseExpr(char*& expr, bool bSubExpr = false);
	BOOL ParseConst(char*& expr);
	BOOL ParseId(char*& expr, FXStaticStack<CallFn>* localStack);
	int Priority(CallFn fn);
	void UpdateLocalCallStack(FXStaticStack<CallFn>* localStack, CallFn fn);
private:
	double Calculate();
	// operations/functions
	double GetStackVal();
	double GetArgVal();
	double Plus();
	double Minus();
	double Multiply();
	double Devide();
	double Power();
	double Exp();
	double Log();

#ifdef _DUMP_STACKS
	void CallStack2Str(FXStaticStack<CallFn>* Stack, FXString& str);
	void DataStack2Str(FXStaticStack<double>* Stack, FXString& str);
#endif

	DECLARE_RUNTIME_GADGET(Calculator);
};
