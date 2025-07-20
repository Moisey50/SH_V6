// QuantityFrameNew.h: interface for the CQuantityFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUANTITYFRAME_H__B7E4C936_DB5A_4FBC_BFAF_E9D3251E4143__INCLUDED_)
#define AFX_QUANTITYFRAME_H__B7E4C936_DB5A_4FBC_BFAF_E9D3251E4143__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <classes/dpoint.h>

#define TVBD400_MAXIMUM_QUANTITY_SIZE	256	// bytes

typedef struct tagGENERICQUANTITY
{
	int _type;
	union
	{
		int _i;
		double _d;
		DPOINT _c;
	};
}GENERICQUANTITY, *LPGENERICQUANTITY;

typedef const GENERICQUANTITY FAR* LPCGENERICQUANTITY;

class FX_EXT_GADGET CGenericQuantity : public GENERICQUANTITY
{
public:
	enum { GQ_INTEGER = 1 , GQ_FLOATING, GQ_COMPLEX };
	CGenericQuantity(int i);
	CGenericQuantity(double d);
	CGenericQuantity(double r, double i);
  CGenericQuantity( DPOINT c );
//   CGenericQuantity( cmplx c );
  CGenericQuantity( LPGENERICQUANTITY gq );
	operator int() const;
  operator long() const;
	operator double() const;
  operator DPOINT() const;
//   operator cmplx() const;
  operator LPGENERICQUANTITY()
  {
    return (LPGENERICQUANTITY) this;
  };
	bool operator == (CGenericQuantity& gq) const;
	bool operator != (CGenericQuantity& gq) const;
	bool operator <= (CGenericQuantity& gq) const;
	bool operator >= (CGenericQuantity& gq) const;
	bool operator <  (CGenericQuantity& gq) const;
	bool operator >  (CGenericQuantity& gq) const;
	CGenericQuantity& operator = (int i);
	CGenericQuantity& operator = (double d);
  CGenericQuantity& operator = ( DPOINT c );
//   CGenericQuantity& operator = ( cmplx c );
  const FXString ToString() const
	{
		FXString retVal;
		switch (_type)
		{
		case GQ_INTEGER:
			retVal.Format(_T("%d"), _i);
			break;
		case GQ_FLOATING:
			retVal.Format(_T("%8g"), _d);
			break;
		case GQ_COMPLEX:
			retVal.Format(_T("%8g + i%8g"), _c.x, _c.y);
			break;
		}
		return retVal;
	}
	void FromString(LPCTSTR str);
protected:
	CGenericQuantity();
};


class FX_EXT_GADGET CQuantityFrame : public CDataFrame, public CGenericQuantity
{
protected:
	CQuantityFrame(LPCGENERICQUANTITY pQuantity);
	CQuantityFrame(const CQuantityFrame* QuantityFrame);
	virtual ~CQuantityFrame();
public:
	CDataFrame* Copy() const { return new CQuantityFrame(this); };
	CQuantityFrame* GetQuantityFrame(LPCTSTR label = DEFAULT_LABEL);
    const CQuantityFrame* GetQuantityFrame(LPCTSTR label = DEFAULT_LABEL) const;
	static CQuantityFrame* Create(int i);
	static CQuantityFrame* Create(double d);
	static CQuantityFrame* Create(double r, double i);
  static CQuantityFrame* Create( DPOINT c );
//   static CQuantityFrame* Create( cmplx c );
  static CDataFrame* CreateFrom( void* pData , UINT cData );
  virtual BOOL Serialize( LPBYTE* ppData , FXSIZE* cbData ) const;
  virtual BOOL Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const;
  virtual FXSIZE GetSerializeLength( FXSIZE& uiLabelLen ,
    FXSIZE * pAttribLen = NULL ) const ;
  virtual BOOL Restore( LPBYTE lpData , FXSIZE cbData );
  virtual void ToLogString(FXString& Output);
};


class FX_EXT_GADGET CBooleanFrame : public CDataFrame
{
	bool _value;
protected:
	CBooleanFrame(bool b);
	CBooleanFrame(const CBooleanFrame* BooleanFrame);
	virtual ~CBooleanFrame();
public:
	operator bool() const { return _value; };
	CDataFrame* Copy() const { return new CBooleanFrame(this); };
	CBooleanFrame* GetBooleanFrame(LPCTSTR label = DEFAULT_LABEL);
    const CBooleanFrame* GetBooleanFrame(LPCTSTR label = DEFAULT_LABEL) const;
	static  CBooleanFrame* Create(bool b=false) { return new CBooleanFrame(b); };
  void    SetValue(bool b) { _value=b; }
  bool GetValue() { return _value;  }
	virtual BOOL Serialize(LPBYTE* ppData, FXSIZE* cbData) const;
  virtual BOOL Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const;
  virtual FXSIZE GetSerializeLength( FXSIZE& uiLabelLen ,
    FXSIZE * pAttribLen = NULL ) const ;
  virtual BOOL Restore( LPBYTE lpData , FXSIZE cbData );
  virtual void ToLogString(FXString& Output);
};


#endif // !defined(AFX_QUANTITYFRAME_H__B7E4C936_DB5A_4FBC_BFAF_E9D3251E4143__INCLUDED_)
