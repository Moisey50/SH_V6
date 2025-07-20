#include "stdafx.h"
#include "Calculator.h"
#include <gadgets\QuantityFrame.h>
#include <gadgets\TextFrame.h>
#include <math\intf_sup.h>

IMPLEMENT_RUNTIME_GADGET_EX(Calculator, CCollectorGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

Calculator::Calculator():
m_Expression(""),
m_nVars(1),
m_CallStack(1000),
m_DataStack(1000),
m_InitCode(1000),
m_InitData(1000),
m_nError(0)
{
	m_pOutput = new COutputConnector(quantity);
	CreateInputs(m_nVars, transparent);
	Resume();
}

void Calculator::ShutDown()
{
	CCollectorGadget::ShutDown();
	delete m_pOutput;
	m_pOutput = NULL;
	CallFn fn;
	while (m_CallStack.ItemsInStack())
		m_CallStack.GetStackObject(fn);
	while (m_InitCode.ItemsInStack())
		m_InitCode.GetStackObject(fn);
	double d;
	while (m_InitData.ItemsInStack())
		m_InitData.GetStackObject(d);
	while (m_DataStack.ItemsInStack())
		m_DataStack.GetStackObject(d);
}

CDataFrame* Calculator::DoProcessing(CDataFrame const*const* frames, int nmb)
{
	FXAutolock al(m_Lock);
	Restore();
	m_ppFrames = frames;
	m_nFrames = nmb;
	double result = Calculate();
	if (m_nError == 0)
	{
		CQuantityFrame* pResult = CQuantityFrame::Create(result);
    for ( int i = 0 ; i < nmb ; i++ )
    {
      if ( frames[i]->GetId() != 0 )
      {
        pResult->ChangeId(frames[i]->GetId());
        pResult->SetTime(frames[i]->GetTime());
        return pResult ;
      }
    }
		pResult->ChangeId( 0 );
		return pResult;
	}
	return NULL;
}

bool Calculator::ScanSettings(FXString& text)
{
	text.Format("template(Spin(Vars,%d,%d),EditBox(Expression))", 1, 9);
	return true;
}

bool Calculator::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	CCollectorGadget::ScanProperties(text, Invalidate);
	FXAutolock al(m_Lock);
	FXPropertyKit pk(text);
	FXString expr;
	if (pk.GetString("Expression", expr) && m_Expression.Compare(expr))
	{
		m_Expression = expr;
		Init();
	}
	int vars = m_nVars;
	pk.GetInt("Vars", vars);
	if (vars != m_nVars)
	{
		m_nVars = vars;
		CreateInputs(m_nVars, transparent);
	}
	Status().WriteBool(STATUS_REDRAW, true);
	return true;
}

bool Calculator::PrintProperties(FXString& text)
{
	CCollectorGadget::PrintProperties(text);
	FXPropertyKit pk;
	pk.WriteString("Expression", m_Expression);
	pk.WriteInt("Vars", m_nVars);
	text += pk;
	return true;
}

void Calculator::Restore()
{
	m_nError = 0;
	CallFn fn;
	while (m_CallStack.ItemsInStack())
		m_CallStack.GetStackObject(fn);
	double d;
	while (m_DataStack.ItemsInStack())
		m_DataStack.GetStackObject(d);
	int i;
	for (i = m_InitCode.ItemsInStack(); i > 0; i--)
	{
		m_InitCode.Peep(i - 1, fn);
		m_CallStack.PutStackObject(fn);
	}
	for (i = m_InitData.ItemsInStack(); i > 0; i--)
	{
		m_InitData.Peep(i - 1, d);
		m_DataStack.PutStackObject(d);
	}
}

#define THIS_MODULENAME "TVGeneric.Calculator"
void Calculator::Init()
{
	m_nError = 0;
	CallFn fn;
	while (m_InitCode.ItemsInStack())
		m_InitCode.GetStackObject(fn);
	double d;
	while (m_InitData.ItemsInStack())
		m_InitData.GetStackObject(d);
	char* expr = (char*)(LPCTSTR)m_Expression;
	if (!ParseExpr(expr))
	{
#ifdef _DUMP_STACKS
		FXString str1, str2;
		CallStack2Str(&m_InitCode, str1);
		DataStack2Str(&m_InitData, str2);
		SENDERR_2("Stacks: %s  |   %s", str1, str2);
#endif
		CallFn fn;
		while (m_InitCode.ItemsInStack())
			m_InitCode.GetStackObject(fn);
		double d;
		while (m_InitData.ItemsInStack())
			m_InitData.GetStackObject(d);
		SENDERR_1("Syntax error %d", m_nError);
	}
#ifdef _DUMP_STACKS
	FXString str1, str2;
	CallStack2Str(&m_InitCode, str1);
	DataStack2Str(&m_InitData, str2);
	SENDERR_2("Stacks: %s  |   %s", str1, str2);
#endif
}

#undef THIS_MODULENAME

BOOL Calculator::ParseExpr(char*& expr, bool bSubExpr)
{
	FXStaticStack<CallFn> localCallStack(1000);
	bool bFirst = true;
	while (*expr != 0)
	{
		if (*expr == '-' && bFirst)
		{
			m_InitData.PutStackObject(-1);
			m_InitCode.PutStackObject(&Calculator::GetStackVal);
			localCallStack.PutStackObject(&Calculator::Multiply);
			expr++;
		}
		else if (*expr == '$')
		{
			UpdateLocalCallStack(&localCallStack, &Calculator::GetArgVal);
			expr++;
		}
		else if (*expr == '^')
		{
			UpdateLocalCallStack(&localCallStack, &Calculator::Power);
			expr++;
		}
		else if (*expr == '/')
		{
			UpdateLocalCallStack(&localCallStack, &Calculator::Devide);
			expr++;
		}
		else if (*expr == '*')
		{
			UpdateLocalCallStack(&localCallStack, &Calculator::Multiply);
			expr++;
		}
		else if (*expr == '-')
		{
			UpdateLocalCallStack(&localCallStack, &Calculator::Minus);
			expr++;
		}
		else if (*expr == '+')
		{
			UpdateLocalCallStack(&localCallStack, &Calculator::Plus);
			expr++;
		}
		else if (*expr == '(')
		{
			expr++;
			if (!ParseExpr(expr, true))
			{
				CallFn fn;
				while (localCallStack.ItemsInStack())
					localCallStack.GetStackObject(fn);
				return FALSE;
			}
		}
		else if (*expr == ')')
		{
			if (bSubExpr)
			{
				expr++;
				CallFn fn;
				while (localCallStack.ItemsInStack())
				{
					localCallStack.GetStackObject(fn);
					m_InitCode.PutStackObject(fn);
				}
				return TRUE;
			}
			m_nError = 6; // unexpected closing bracket
			CallFn fn;
			while (localCallStack.ItemsInStack())
				localCallStack.GetStackObject(fn);
			return FALSE;
		}
		else if (strchr("0123456789.", *expr))
		{
			if (!ParseConst(expr))
			{
				CallFn fn;
				while (localCallStack.ItemsInStack())
					localCallStack.GetStackObject(fn);
				return FALSE;
			}
		}
		else if (strchr("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", *expr))
		{
			if (!ParseId(expr, &localCallStack))
			{
				CallFn fn;
				while (localCallStack.ItemsInStack())
					localCallStack.GetStackObject(fn);
				return FALSE;
			}
		}
		else if (strchr(" \t\r\n", *expr))
		{
			expr++; // skip whitespaces
			continue;
		}
		else
		{
			m_nError = 7; // unknown symbol
			CallFn fn;
			while (localCallStack.ItemsInStack())
				localCallStack.GetStackObject(fn);		
			return FALSE;
		}
		bFirst = false;
	}
	if (bSubExpr)
	{
		m_nError = 8; // unexpected end of expression
		CallFn fn;
		while (localCallStack.ItemsInStack())
			localCallStack.GetStackObject(fn);
		return FALSE;
	}
	CallFn fn;
	while (localCallStack.ItemsInStack())
	{
		localCallStack.GetStackObject(fn);
		m_InitCode.PutStackObject(fn);
	}
	return TRUE;
}

BOOL Calculator::ParseConst(char*& expr)
{
	bool bDotFound = false;
	char* start = expr;
	while (*expr && strchr("1234567890.", *expr))
	{
		if (*expr == '.')
		{
			if (bDotFound)
			{
				m_nError = 9; // wrong const presentation
				return FALSE;
			}
			bDotFound = true;
		}
		expr++;
	}
	char ch = *expr;
	*expr = 0;
	double val = atof(start);
	*expr = ch;
	m_InitData.PutStackObject(val);
	m_InitCode.PutStackObject(&Calculator::GetStackVal);
	return TRUE;
}

BOOL Calculator::ParseId(char*& expr, FXStaticStack<CallFn>* localStack)
{
	bool bAlphaFound = false;
	char* start = expr;
	while (*expr && ((!bAlphaFound && strchr("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", *expr)) ||
			(bAlphaFound && strchr("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", *expr))))
	{
		if (!bAlphaFound && !strchr("_1234567890", *expr))
			bAlphaFound = true;
		expr++;
	}
	char ch = *expr;
	*expr = 0;
	if (!strcmp(start, "exp"))
	{
		UpdateLocalCallStack(localStack, &Calculator::Exp);
	}
	else if (!strcmp(start, "log"))
	{
		UpdateLocalCallStack(localStack, &Calculator::Log);
	}
	//else if ...
	else
	{
		m_nError = 10; // undefined identificator
		return FALSE;
	}
	*expr = ch;
	return TRUE;
}

int Calculator::Priority(CallFn fn)
{
	if (fn == &Calculator::GetStackVal)
		return 0; // immediate
	if (fn == &Calculator::GetArgVal)
		return 1;
	if (fn == &Calculator::Power)
		return 2;
	if (fn == &Calculator::Multiply || fn == &Calculator::Devide)
		return 3;
	if (fn == &Calculator::Plus || fn == &Calculator::Minus)
		return 4;
	// ...
	// boolean?
	// ...

	// default: id (function call)
	return 1;
}

void Calculator::UpdateLocalCallStack(FXStaticStack<CallFn>* localStack, CallFn fn)
{
	int priority = Priority(fn);
	CallFn fnStack;
	while (localStack->ItemsInStack())
	{
		localStack->GetStackObject(fnStack);
		if (Priority(fnStack) <= priority)
			m_InitCode.PutStackObject(fnStack);
		else
		{
			localStack->PutStackObject(fnStack);
			break;
		}
	}
	localStack->PutStackObject(fn);
}

double Calculator::Calculate()
{
	CallFn fn;
	if (!m_CallStack.GetStackObject(fn))
	{
		m_nError = 1; // end of code
		return 0;
	}
	return (this->*fn)();
}

double Calculator::GetStackVal()
{
	double val;
	if (!m_DataStack.GetStackObject(val))
	{
		m_nError = 2; // end of stack data (too little parameters)
		return 0;
	}
	return val;
}

double Calculator::GetArgVal()
{
	int narg = (int)Calculate() - 1;
	if (m_nError)
		return 0;
	if (narg < 0 || narg >= m_nFrames)
	{
		m_nError = 3; // index overflow;
		return 0;
	}
	CDataFrame* pFrame = (CDataFrame*)m_ppFrames[narg];
	CQuantityFrame* qFrame = pFrame->GetQuantityFrame();
	if (qFrame)
		return (double)(*qFrame);
	CBooleanFrame* bFrame = pFrame->GetBooleanFrame();
	if (bFrame)
		return ((bool)(*bFrame)) ? 1. : 0.;
	CTextFrame* tFrame = pFrame->GetTextFrame();
	if (tFrame)
		return (atof(tFrame->GetString()));
	m_nError = 4; // argument contains no data;
	return 0;
}

double Calculator::Plus()
{
	double arg2 = Calculate();
	if (m_nError)
		return 0;
	double arg1 = Calculate();
	if (m_nError)
		return 0;
	return (arg1 + arg2);
}

double Calculator::Minus()
{
	double arg2 = Calculate();
	if (m_nError)
		return 0;
	double arg1 = Calculate();
	if (m_nError)
		return 0;
	return (arg1 - arg2);
}

double Calculator::Multiply()
{
	double arg2 = Calculate();
	if (m_nError)
		return 0;
	double arg1 = Calculate();
	if (m_nError)
		return 0;
	return (arg1 * arg2);
}

double Calculator::Devide()
{
	double arg2 = Calculate();
	if (m_nError)
		return 0;
	double arg1 = Calculate();
	if (m_nError)
		return 0;
	if (arg2 == 0)
	{
		m_nError = 5; // deviding by zero
		return 0;
	}
	return (arg1 / arg2);
}

double Calculator::Power()
{
	double arg2 = Calculate();
	if (m_nError)
		return 0;
	double arg1 = Calculate();
	if (m_nError)
		return 0;
	return pow(arg1, arg2);
}

double Calculator::Exp()
{
	double arg = Calculate();
	if (m_nError)
		return 0;
	return exp(arg);
}

double Calculator::Log()
{
	double arg = Calculate();
	if (m_nError)
		return 0;
	return log(arg);
}

#ifdef _DUMP_STACKS
	void Calculator::CallStack2Str(FXStaticStack<CallFn>* Stack, FXString& str)
	{
		str.Empty();
		CallFn fn;
		int i;
		for (i = 0; i < Stack->ItemsInStack(); i++)
		{
			Stack->Peep(i, fn);
			if (fn == &Calculator::GetStackVal)
				str += "@ ";
			else if (fn == &Calculator::GetArgVal)
				str += "$ ";
			else if (fn == &Calculator::Power)
				str += "^ ";
			else if (fn == &Calculator::Devide)
				str += "/ ";
			else if (fn == &Calculator::Multiply)
				str += "* ";
			else if (fn == &Calculator::Minus)
				str += "- ";
			else if (fn == &Calculator::Plus)
				str += "+ ";
			else if (fn == &Calculator::Exp)
				str += "exp ";
			else if (fn == &Calculator::Log)
				str += "log ";
			else
				str += "ERROR ";
		}
	}

	void Calculator::DataStack2Str(FXStaticStack<double>* Stack, FXString& str)
	{
		str.Empty();
		FXString arg;
		double d;
		int i;
		for (i = 0; i < Stack->ItemsInStack(); i++)
		{
			Stack->Peep(i, d);
			if (d == (double)(int)d)
				arg.Format("%d ", (int)d);
			else
				arg.Format("%.3f ", d);
			str += arg;
		}
	}
#endif