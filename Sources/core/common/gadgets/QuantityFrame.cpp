// QuantityFrame.cpp: implementation of the CQuantityFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\QuantityFrame.h>
#include <math.h>
#include <math/Intf_sup.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGenericQuantity::CGenericQuantity(int i)
{
	_type = GQ_INTEGER;
	_i = i;
}

CGenericQuantity::CGenericQuantity(double d)
{
	_type = GQ_FLOATING;
	_d = d;
}

CGenericQuantity::CGenericQuantity(double r, double i)
{
	_type = GQ_COMPLEX;
	_c.x = r;
	_c.y = i;
}

CGenericQuantity::CGenericQuantity( DPOINT c )
{
  _type = GQ_COMPLEX;
  _c.x = c.x;
  _c.y = c.y;
}
// CGenericQuantity::CGenericQuantity( cmplx c )
// {
//   _type = GQ_COMPLEX;
//   _c.x = c.real();
//   _c.y = c.imag();
// }

CGenericQuantity::CGenericQuantity(LPGENERICQUANTITY gq)
{
	memcpy(LPGENERICQUANTITY(this), gq, sizeof(GENERICQUANTITY));
}

CGenericQuantity::operator int() const
{
	switch (_type)
	{
	case GQ_INTEGER:
		return _i;
	case GQ_FLOATING:
		return ROUNDPM(_d);
	case GQ_COMPLEX:
		return ROUNDPM( _c.x );
	}
	ASSERT(FALSE);
	return 0;
}

CGenericQuantity::operator long() const
{
	switch (_type)
	{
	case GQ_INTEGER:
		return (long)_i;
	case GQ_FLOATING:
    return ROUNDPM( _d );
  case GQ_COMPLEX:
    return ROUNDPM( _c.x );
  }
	ASSERT(FALSE);
	return 0;
}

CGenericQuantity::operator double() const
{
	switch (_type)
	{
	case GQ_INTEGER:
		return (double)_i;
	case GQ_FLOATING:
		return _d;
	case GQ_COMPLEX:
		return _c.x ;
	}
	ASSERT(FALSE);
	return 0;
}

CGenericQuantity::operator DPOINT() const
{
  DPOINT c;
  c.y = 0;
  switch ( _type )
  {
    case GQ_INTEGER:
      c.x = (double) _i;
      return c;
    case GQ_FLOATING:
      c.x = _d;
      return c;
    case GQ_COMPLEX:
      return _c;
  }
  ASSERT( FALSE );
  return c;
}
// CGenericQuantity::operator cmplx() const
// {
//   cmplx c;
//   switch ( _type )
//   {
//     case GQ_INTEGER:
//       c._Val[_RE] = (double) _i;
//       return c;
//     case GQ_FLOATING:
//       c._Val[ _RE ] = _d;
//       return c;
//     case GQ_COMPLEX:
//       return cmplx(_c.x,_c.y)  ;
//   }
//   ASSERT( FALSE );
//   return c;
// }

CGenericQuantity::CGenericQuantity()
{
	memset(LPGENERICQUANTITY(this), 0, sizeof(GENERICQUANTITY));
}

CGenericQuantity& CGenericQuantity::operator = (int i)
{
	_type = GQ_INTEGER;
	_i = i;
	return *this;
}

CGenericQuantity& CGenericQuantity::operator = (double d)
{
	_type = GQ_FLOATING;
	_d = d;
	return *this;
}

CGenericQuantity& CGenericQuantity::operator = ( DPOINT c )
{
  _type = GQ_COMPLEX;
  _c.x = c.x;
  _c.y = c.y;
  return *this;
}
// CGenericQuantity& CGenericQuantity::operator = ( cmplx c )
// {
//   _type = GQ_COMPLEX;
//   _c.x = c.real();
//   _c.y = c.imag();
//   return *this;
// }


bool CGenericQuantity::operator == (CGenericQuantity& gq) const
{
	if (_type == GQ_INTEGER && LPGENERICQUANTITY(gq)->_type == GQ_INTEGER)
		return (_i == LPGENERICQUANTITY(gq)->_i);
	DPOINT c1 = DPOINT(*this);
	DPOINT c2 = DPOINT(gq);
	return ((c1.x == c2.x) && (c1.y == c2.y));
}

bool CGenericQuantity::operator != (CGenericQuantity& gq) const
{
	if (_type == GQ_INTEGER && LPGENERICQUANTITY(gq)->_type == GQ_INTEGER)
		return (_i != LPGENERICQUANTITY(gq)->_i);
	DPOINT c1 = DPOINT(*this);
	DPOINT c2 = DPOINT(gq);
	return ((c1.x != c2.x) || (c1.y != c2.y));
}

bool CGenericQuantity::operator <= (CGenericQuantity& gq) const
{
	if (_type == GQ_COMPLEX || LPGENERICQUANTITY(gq)->_type == GQ_COMPLEX)
		return false;
	if (_type == GQ_INTEGER && LPGENERICQUANTITY(gq)->_type == GQ_INTEGER)
		return (_i <= LPGENERICQUANTITY(gq)->_i);
	return (double(*this) <= double(gq));
}

bool CGenericQuantity::operator >= (CGenericQuantity& gq) const
{
	if (_type == GQ_COMPLEX || LPGENERICQUANTITY(gq)->_type == GQ_COMPLEX)
		return false;
	if (_type == GQ_INTEGER && LPGENERICQUANTITY(gq)->_type == GQ_INTEGER)
		return (_i >= LPGENERICQUANTITY(gq)->_i);
	return (double(*this) >= double(gq));
}

bool CGenericQuantity::operator <  (CGenericQuantity& gq) const
{
	if (_type == GQ_COMPLEX || LPGENERICQUANTITY(gq)->_type == GQ_COMPLEX)
		return false;
	if (_type == GQ_INTEGER && LPGENERICQUANTITY(gq)->_type == GQ_INTEGER)
		return (_i < LPGENERICQUANTITY(gq)->_i);
	return (double(*this) < double(gq));
}

bool CGenericQuantity::operator >  (CGenericQuantity& gq) const
{
	if (_type == GQ_COMPLEX || LPGENERICQUANTITY(gq)->_type == GQ_COMPLEX)
		return false;
	if (_type == GQ_INTEGER && LPGENERICQUANTITY(gq)->_type == GQ_INTEGER)
		return (_i > LPGENERICQUANTITY(gq)->_i);
	return (double(*this) > double(gq));
}

void CGenericQuantity::FromString(LPCTSTR str)
{
	FXString s(str);
	s.TrimLeft("-+1234567890");
	if (s.IsEmpty())
	{
		_type = GQ_INTEGER;
		_i = atoi(str);
	}
	else
	{
		_type = GQ_FLOATING;
		_d = atof(str);
	}
}


CQuantityFrame::CQuantityFrame(LPCGENERICQUANTITY pQuantity)
{
	m_DataType = quantity;
	memcpy(LPGENERICQUANTITY(this), pQuantity, sizeof(GENERICQUANTITY));
}

CQuantityFrame::CQuantityFrame(const CQuantityFrame* QuantityFrame)
{
	m_DataType = quantity;
	memcpy(LPGENERICQUANTITY(this), LPGENERICQUANTITY(QuantityFrame), sizeof(GENERICQUANTITY));
    CopyAttributes(QuantityFrame);
}

CQuantityFrame::~CQuantityFrame()
{
}

CQuantityFrame* CQuantityFrame::GetQuantityFrame(LPCTSTR label)
{
	if (!label || !strcmp(m_Label, label))
		return this;
	return NULL;
}

const CQuantityFrame* CQuantityFrame::GetQuantityFrame(LPCTSTR label) const
{
	if (!label || !strcmp(m_Label, label))
		return this;
	return NULL;
}

CQuantityFrame* CQuantityFrame::Create(int i)
{
	return new CQuantityFrame(CGenericQuantity(i));
}

CQuantityFrame* CQuantityFrame::Create(double d)
{
	return new CQuantityFrame(CGenericQuantity(d));
}

CQuantityFrame* CQuantityFrame::Create(double r, double i)
{
	return new CQuantityFrame(CGenericQuantity(r, i));
}

CQuantityFrame* CQuantityFrame::Create( DPOINT c )
{
  return new CQuantityFrame( CGenericQuantity( c ) );
}
// CQuantityFrame* CQuantityFrame::Create( cmplx c )
// {
//   return new CQuantityFrame( CGenericQuantity( c ) );
// }

CDataFrame* CQuantityFrame::CreateFrom(void* pData, UINT cData)
{
	if (cData == sizeof(GENERICQUANTITY))
		return new CQuantityFrame((LPGENERICQUANTITY)pData);
	if (cData == sizeof(DPOINT))
		return CQuantityFrame::Create(*(DPOINT*)pData);
	if (cData == sizeof(double))
		return CQuantityFrame::Create(*(double*)pData);
	if (cData == sizeof(int))
		return CQuantityFrame::Create(*(int*)pData);
	return NULL;
}

BOOL CQuantityFrame::Serialize(LPBYTE* ppData, FXSIZE* cbData) const
{
	ASSERT(ppData);
	FXSIZE cb;
	if (!CDataFrame::Serialize(ppData, &cb))
		return FALSE;
	*cbData = cb + sizeof(GENERICQUANTITY);
	*ppData = (LPBYTE)realloc(*ppData, *cbData);
	LPBYTE ptr = *ppData + cb;
	memcpy(ptr, LPGENERICQUANTITY(this), sizeof(GENERICQUANTITY));
	return TRUE;
}

BOOL CQuantityFrame::Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
{
  FXSIZE uiLabelLen , uiAttribLen ;
  FXSIZE AdditionLen = GetSerializeLength( uiLabelLen , &uiAttribLen ) ;
  if ( ( CurrentWriteIndex + AdditionLen ) >= BufLen )
    return FALSE ;

  CDataFrame::Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) ;

  LPBYTE ptr = pBufferOrigin + CurrentWriteIndex ;
  memcpy( ptr , LPGENERICQUANTITY( this ) , sizeof( GENERICQUANTITY ) );

  CurrentWriteIndex += sizeof( GENERICQUANTITY ) ;
  return TRUE;
}

FXSIZE CQuantityFrame::GetSerializeLength( FXSIZE& uiLabelLen ,
  FXSIZE * pAttribLen ) const
{
  FXSIZE Len = CDataFrame::GetSerializeLength( uiLabelLen , pAttribLen ) ;
  Len += sizeof( GENERICQUANTITY ) ; // trailing zero is included
  return Len ;
};

BOOL CQuantityFrame::Restore(LPBYTE lpData, FXSIZE cbData)
{
	FXSIZE cb;
	if (!CDataFrame::Serialize(NULL, &cb) || !CDataFrame::Restore(lpData, cbData))
		return FALSE;
	memcpy(LPGENERICQUANTITY(this), lpData + cb, sizeof(GENERICQUANTITY));
	return TRUE;
}

void CQuantityFrame::ToLogString(FXString& Output)
{
  CDataFrame::ToLogString(Output);
  Output += ToString();
}


CBooleanFrame::CBooleanFrame(bool b) :
_value(b)
{
	m_DataType = logical;
}

CBooleanFrame::CBooleanFrame(const CBooleanFrame* BooleanFrame)
{
	m_DataType = logical;
	_value = *BooleanFrame;
    CopyAttributes(BooleanFrame);
}

CBooleanFrame::~CBooleanFrame()
{
}

CBooleanFrame* CBooleanFrame::GetBooleanFrame(LPCTSTR label)
{
	if (!label || m_Label == label)
		return this;
	return NULL;
}

const CBooleanFrame* CBooleanFrame::GetBooleanFrame(LPCTSTR label) const
{
	if (!label || m_Label == label)
		return this;
	return NULL;
}


BOOL CBooleanFrame::Serialize(LPBYTE* ppData, FXSIZE* cbData) const
{
	ASSERT(ppData);
	FXSIZE cb;
	if (!CDataFrame::Serialize(ppData, &cb))
		return FALSE;
	*cbData = cb + sizeof(_value);
	*ppData = (LPBYTE)realloc(*ppData, *cbData);
	LPBYTE ptr = *ppData + cb;
	memcpy(ptr, &_value, sizeof(_value));
	return TRUE;
}

BOOL CBooleanFrame::Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
{
  FXSIZE uiLabelLen , uiAttribLen ;
  FXSIZE AdditionLen = GetSerializeLength( uiLabelLen , &uiAttribLen ) ;
  if ( ( CurrentWriteIndex + AdditionLen ) >= BufLen )
    return FALSE ;

  CDataFrame::Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) ;

  LPBYTE ptr = pBufferOrigin + CurrentWriteIndex ;
  memcpy( ptr , &_value , sizeof( _value ) );
  CurrentWriteIndex += sizeof( _value ) ;

  return TRUE;
}
FXSIZE CBooleanFrame::GetSerializeLength( FXSIZE& uiLabelLen ,
  FXSIZE * pAttribLen ) const
{
  FXSIZE Len = CDataFrame::GetSerializeLength( uiLabelLen , pAttribLen ) ;
  Len += sizeof( _value ) ; // trailing zero is included
  return Len ;
};

BOOL CBooleanFrame::Restore(LPBYTE lpData, FXSIZE cbData)
{
	FXSIZE cb;
	if (!CDataFrame::Serialize(NULL, &cb) || !CDataFrame::Restore(lpData, cbData))
		return FALSE;
	memcpy(&_value, lpData + cb, sizeof(_value));
	return TRUE;
}

void CBooleanFrame::ToLogString(FXString& Output)
{
  CDataFrame::ToLogString(Output);
  Output += _value ? _T("True") : _T("False") ;
}
