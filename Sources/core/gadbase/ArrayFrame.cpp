#include "StdAfx.h"
#include <gadgets\arrayframe.h>

CArrayFrame::CArrayFrame(datatype dt, FN_CREATE_FROM fnCreateFrom, int cbElement):
CDataFrame(dt * arraytype)
{
	_dt = dt * arraytype;
	_fnCreateFrom = fnCreateFrom;
	_cbElement = cbElement;
	_count = 0;
	_lpData = NULL;
}

CArrayFrame::CArrayFrame(ARRAYFRAME* ArrayFrame):
CDataFrame(ArrayFrame->_dt * arraytype)
{
	_dt = ArrayFrame->_dt * arraytype;
	_fnCreateFrom = ArrayFrame->_fnCreateFrom;
	_cbElement = ArrayFrame->_cbElement;
	_count = 0;
	_lpData = NULL;
	SetAt(0, ArrayFrame->_lpData, ArrayFrame->_count);
}

CArrayFrame::~CArrayFrame()
{
	free(_lpData);
	_count = 0;
}

void CArrayFrame::SetAt(int nElement, void* pData, int count)
{
	if (nElement + count > _count)
	{
		_lpData = realloc(_lpData, _cbElement * (nElement + count));
		_count = nElement + count;
	}
	LPBYTE dst = (LPBYTE)_lpData + nElement * _cbElement;
	memcpy(dst, pData, _cbElement * count);
}

const CDataFrame* CArrayFrame::GetAt(int nElement) const
{
	if (nElement > _count)
		return NULL;
	void* pData = (void*)((LPBYTE)_lpData + nElement * _cbElement);
	CDataFrame* pDataFrame = _fnCreateFrom(pData, _cbElement);
	return pDataFrame;
}

CArrayFrame* CArrayFrame::GetArrayFrame(LPCTSTR label)
{
	if (!label || !strcmp(m_Label, label))
		return this;
	return NULL;
}

const CArrayFrame* CArrayFrame::GetArrayFrame(LPCTSTR label) const
{
	if (!label || !strcmp(m_Label, label))
		return this;
	return NULL;
}

CArrayFrame* CArrayFrame::Create(datatype dt, FN_CREATE_FROM fnCreateFrom, UINT cbElement)
{
	return new CArrayFrame(dt, fnCreateFrom, cbElement);
}

CDataFrame* CArrayFrame::CreateFrom(void* pData, UINT cData)
{
	if (cData == sizeof(ARRAYFRAME))
		return new CArrayFrame((LPARRAYFRAME)pData);
	return NULL;
}

void CArrayFrame::ToLogString(FXString& Output)
{
  CDataFrame::ToLogString(Output);
  FXString tmpS;
  tmpS.Format("Len=%u ", GetCount() );
  Output += tmpS;
}
