#ifndef __INCLUDE__UserPropertiesStruct_H__
#define __INCLUDE__UserPropertiesStruct_H__


#pragma once

//#include "stdafx.h"
#include <exception>
#include <gadgets\stdsetup.h>
#include <helpers\PropertyKitEx.h>

typedef void( *PropertyUpdateCB )( LPCTSTR pPropertyName , 
  void * pObject , bool& bInvalidate , bool& bInitRescan ) ;

typedef void( *PropertyUpdateCB_WithId )(
  LPCTSTR pPropertyName , void * pObject , int iId ) ;

struct SpinBoolStuct
{
private:

	bool*	m_ptrBool;
	int*	m_ptrInt;

	inline void setInt(int iVal)
	{
		setBool(false);
		*m_ptrInt = iVal;		
	}

	inline void setBool(bool bVal)
	{
		*m_ptrBool = bVal;
	}


public:

	inline void setPtr(int* ptrInt, bool* ptrBool)
	{
		m_ptrInt = ptrInt;
		m_ptrBool= ptrBool;		
	}

	inline void operator=(int iVal)
	{
		setInt(iVal);
	}

	inline void operator=(bool bVal)
	{
		setBool(bVal);
	}

	inline void operator=(SpinBoolStuct obj)
	{
		setPtr(obj.m_ptrInt,obj.m_ptrBool);
	}

	inline bool getBool()
	{
		return *m_ptrBool;
	}

	inline int getInt()
	{
		return *m_ptrInt;
	}

};


struct SProperty
{
	static inline bool writeInt(FXPropertyKit &pk, LPCTSTR name, void* pValue, UINT m_uiBinarySize=0)
	{
		return pk.WriteInt( name, *(int*)pValue );
	}
  static inline bool writeLong( FXPropertyKit &pk , LPCTSTR name , void* pValue , UINT m_uiBinarySize = 0 )
  {
    return pk.WriteLong( name , *( long* ) pValue );
  }
  static inline bool writeInt64( FXPropertyKit &pk , LPCTSTR name , void* pValue , UINT m_uiBinarySize = 0 )
  {
    return pk.WriteInt64( name , *( __int64* ) pValue );
  }
  static inline bool writeDouble( FXPropertyKit &pk , LPCTSTR name , void* pValue , UINT m_uiBinarySize = 0 )
  {
    return pk.WriteDouble( name , *(double*) pValue );
  }
  static inline bool writeCmplx( FXPropertyKit &pk , LPCTSTR name , void* pValue , UINT m_uiBinarySize = 0 )
  {
    return ((FXPropKit2 &) pk).WriteCmplx( name , *((cmplx*) pValue ));
  }
  static inline bool writeBool( FXPropertyKit &pk , LPCTSTR name , void* pValue , UINT m_uiBinarySize = 0 )
	{
		return pk.WriteBool( name, *(bool*)pValue );
	}
	static inline bool writeBinary(FXPropertyKit &pk, LPCTSTR name, void* pValue, UINT m_uiBinarySize)
	{
		return pk.WriteBinary( name, *(LPBYTE*)pValue, m_uiBinarySize );
	}
	static inline bool writeString(FXPropertyKit &pk, LPCTSTR name, void* pValue, UINT m_uiBinarySize=0)
	{
		return pk.WriteString( name, *(FXString*)pValue );
	}
	static inline bool writeSpinBool(FXPropertyKit &pk, LPCTSTR name, void* pValue, UINT m_uiBinarySize=0)
	{
		bool bIsOK = false;

//		FxSendLogMsg(1,"DEBUG",0,"Line %d Function %s: Bool=%d, Int=%d", __LINE__, __FUNCTION__,((SpinBoolStuct*)pValue)->getBool(), ((SpinBoolStuct*)pValue)->getInt());

		if ( ((SpinBoolStuct*)pValue)->getBool() )
		{
			bIsOK = pk.WriteString( name, "auto");
		}
		else
		{
			bIsOK= pk.WriteInt( name, ((SpinBoolStuct*)pValue)->getInt() );
		}

//		FxSendLogMsg(1,"DEBUG",0,"Line %d Function %s: Bool=%d, Int=%d", __LINE__, __FUNCTION__,((SpinBoolStuct*)pValue)->getBool(), ((SpinBoolStuct*)pValue)->getInt());

		return bIsOK;
	}

	static inline bool writeUndefined(FXPropertyKit &pk, LPCTSTR name, void* pValue, UINT m_uiBinarySize=0)
	{
		return false;
	}

  static inline bool getInt( FXPropertyKit &pk , LPCTSTR name , void* bValue , UINT& m_uiBinarySize )
  {
    int iTmp ;
    bool bRes = pk.GetInt( name , iTmp );
    if ( bRes && ( *( ( int* ) bValue ) != iTmp ) )
      *( ( int* ) bValue ) = iTmp ;
    else
      return false ;
    return bRes ;
  }
  static inline bool getInt64( FXPropertyKit &pk , LPCTSTR name , void* bValue , UINT& m_uiBinarySize )
  {
    __int64 iTmp ;
    bool bRes = pk.GetInt64( name , iTmp );
    if ( bRes && ( *( ( __int64* ) bValue ) != iTmp ) )
      *( ( __int64* ) bValue ) = iTmp ;
    else
      return false ;
    return bRes ;
  }
  static inline bool getLong( FXPropertyKit &pk , LPCTSTR name , void* bValue , UINT& m_uiBinarySize )
	{
		long lTmp ;
		bool bRes = pk.GetLong( name , lTmp );
		if (bRes && ( *( ( long* ) bValue ) != lTmp ) )
			*((long*)bValue) = lTmp ;
    else
      return false ;
    return bRes ;
	}
  static inline bool getCmplx( FXPropertyKit &pk , LPCTSTR name , void* bValue , UINT& m_uiBinarySize )
  {
    cmplx cTmp ;
    bool bRes = ((FXPropKit2 &)pk).GetCmplx( name , cTmp );
    if ( bRes && ( *( ( cmplx* ) bValue ) != cTmp ) )
      *((cmplx*) bValue) = cTmp ;
    else
      return false ;
    return bRes ;
  }
  static inline bool getDouble( FXPropertyKit &pk , LPCTSTR name , void* bValue , UINT& m_uiBinarySize )
  {
    double dTmp ;
    bool bRes = pk.GetDouble( name , dTmp );
    if ( bRes && ( *( ( double* ) bValue ) != dTmp ) )
      *((double*) bValue) = dTmp ;
    else
      return false ;
    return bRes ;
  }
  static inline bool getBool( FXPropertyKit &pk , LPCTSTR name , void* bValue , UINT& m_uiBinarySize )
	{	
		bool bTmp ;
		bool bRes = pk.GetBool( name , bTmp );
		if (bRes && ( *( ( bool* ) bValue ) != bTmp ) )
			*((bool*)bValue) = bTmp ;
    else
      return false ;
    return bRes ;
	}
	static inline bool getBinary(FXPropertyKit &pk, LPCTSTR name, void* bValue, UINT& m_uiBinarySize)
	{
#ifndef FXSIZE
		return pk.GetBinary( name , reinterpret_cast<LPBYTE&>(bValue) , m_uiBinarySize );
#else
    FXSIZE Val ;
    if ( pk.GetBinary( name , reinterpret_cast<LPBYTE&>( bValue ) , Val ) )
    {
      m_uiBinarySize = ( UINT ) Val ;
      return true ;
    }
    return false ;
#endif
	}
	static inline bool getString(FXPropertyKit &pk, LPCTSTR name, void* bValue, UINT& m_uiBinarySize)
	{
		FXString sTmp ;
		bool bRes = pk.GetString( name , sTmp );
		if (bRes)
			*((FXString*)bValue) = sTmp ;
		return bRes ;
	}
	static inline bool getSpinBool(FXPropertyKit &pk, LPCTSTR name, void* bValue, UINT& m_uiBinarySize)
	{
		FXString sTmp ;
		bool bRes = pk.GetString( name , sTmp );

//		FxSendLogMsg(1,"DEBUG",0,"Line %d Function %s: Bool=%d, Int=%d", __LINE__, __FUNCTION__,((SpinBoolStuct*)bValue)->getBool(), ((SpinBoolStuct*)bValue)->getInt());

		if (bRes)
		{
			if (sTmp==_T("auto"))
			{
				*((SpinBoolStuct*)bValue) = true ;
			}
			else
			{
				int iTmp ;
				bRes = pk.GetInt( name , iTmp );
				if (bRes)
				{
					*((SpinBoolStuct*)bValue) = iTmp ;
				}
			}
		}

//		FxSendLogMsg(1,"DEBUG",0,"Line %d Function %s: Bool=%d, Int=%d", __LINE__, __FUNCTION__,((SpinBoolStuct*)bValue)->getBool(), ((SpinBoolStuct*)bValue)->getInt());

		return bRes ;
	}

	static inline bool getUndefined(FXPropertyKit &pk, LPCTSTR name, void* bValue, UINT& m_uiBinarySize)
	{
		return false;
	}

	bool (*write_)	(FXPropertyKit &pk, LPCTSTR lpszEntry, void* bValue,	UINT m_uiBinarySize);
	bool (*get_)	(FXPropertyKit &pk, LPCTSTR lpszEntry, void* bValue, UINT& m_uiBinarySize);

public:

	enum EVariableType
	{
		NotDefined = -1,
		Int				,
    Int64     ,
		Long			,
		Double		,
    Cmplx     ,
		Bool			,
		Binary			,	//	Actually not in use
		String			,
		SpinBool		,
		Reserved		,
		UserDefined	
	};
	enum PropertyBox
	{
		NOT_DEFINED = -1,
		EDITBOX			,
		SPIN			,
		SPIN_BOOL		,			
		COMBO			,
    INDEXED_COMBO ,
    CHECK_BOX ,
		BINARY_BOX		,	//	Actually not in use
		RESERVED		,
		USER_DEFINED
	};
  bool m_bInitRescan ;
  bool m_bInvalidate ;

protected:

	FXString    	m_sName;
	void*			    m_ptrVar;
	EVariableType	m_eVariableType;		
	PropertyBox		m_ePropertyBox;
	UINT			m_uiBinarySize;
  PropertyUpdateCB m_pNotification ;
  PropertyUpdateCB_WithId m_pNotificationWithId ;
  void*         m_pObject ;
  int           m_iPropId ;

	SProperty(LPCTSTR sName, void* ptrVar, EVariableType eVariableType, PropertyBox ePropertyBox):
		m_bInitRescan( false ) , m_bInvalidate ( false ) ,
    m_sName(sName), m_eVariableType(eVariableType), m_ptrVar(ptrVar), 
    m_ePropertyBox(ePropertyBox) , m_uiBinarySize(0) , 
    m_pNotification(NULL) , m_pNotificationWithId (NULL ) , 
    m_pObject( NULL ) , m_iPropId( INT_MAX )
	{
	 	switch (m_eVariableType)
	 	{
    case Int:
    {
      write_ = &SProperty::writeInt;
      get_ = &SProperty::getInt;
    }
    break;

    case Int64:
    {
      write_ = &SProperty::writeInt64;
      get_ = &SProperty::getInt64;
    }
    break;

		case Long:
			{
				write_	= &SProperty::writeLong;
				get_	= &SProperty::getLong;
			}
			break;

		case Double:
			{
				write_	= &SProperty::writeDouble;
				get_	= &SProperty::getDouble;
			}
			break;
    case Cmplx:
      {
        write_ = &SProperty::writeCmplx;
        get_ = &SProperty::getCmplx;
      }
      break;
		case Bool:
			{
				write_	= &SProperty::writeBool;
				get_	= &SProperty::getBool;
			}
			break;

		case Binary:
			{
				write_	= &SProperty::writeBinary;
				get_	= &SProperty::getBinary;
			}
			break;

		case String:
			{
				write_	= &SProperty::writeString;
				get_	= &SProperty::getString;
			}
			break;

		case SpinBool:
			{
				write_	= &SProperty::writeSpinBool;
				get_	= &SProperty::getSpinBool;
			}
			break;

		default:
			{
				write_	= &SProperty::writeUndefined;
				get_	= &SProperty::getUndefined;
			}
			break;
		}
	};

	SProperty(const SProperty &obj)
	{	
    m_bInitRescan = obj.m_bInitRescan ;
		m_sName			= obj.m_sName;
		m_ptrVar		= obj.m_ptrVar;
		m_eVariableType = obj.m_eVariableType;
		m_ePropertyBox	= obj.m_ePropertyBox;
    m_uiBinarySize = obj.m_uiBinarySize ;
    m_pNotification = obj.m_pNotification ;
    m_pNotificationWithId = obj.m_pNotificationWithId ;
    m_pObject = obj.m_pObject ;
    m_iPropId = obj.m_iPropId ;
	};

public:

	~SProperty() 
	{

	};

  void SetNotification( PropertyUpdateCB pNewCallBack , void * pObject )
  {
    m_pNotification = pNewCallBack ;
    m_pObject = pObject ;
    m_iPropId = INT_MAX ;
  }
  void SetNotificationWithId( PropertyUpdateCB_WithId pNewCallBack , void * pObject , int iId  )
  {
    m_pNotificationWithId = pNewCallBack ;
    m_pObject = pObject ;
    m_iPropId = iId ;
  }

  bool IsCombo() { return ( ( m_ePropertyBox == COMBO ) 
    || ( m_ePropertyBox == INDEXED_COMBO ) ) ; }
protected:

	inline bool PrintProperties(FXPropertyKit &pk)
	{
		bool bRes = (*write_)(pk, m_sName, m_ptrVar, m_uiBinarySize);
		return bRes;
	};

	inline bool ScanProperties(FXPropertyKit &pk , bool& bInvalidate )
	{
		/*
		UINT uiSize;
		bool bRes = (*get_)(pk, m_sName, m_ptrVar, uiSize);
		*/

		UINT uiSize;
    m_bInitRescan = false;
    if ( m_ePropertyBox == COMBO || m_ePropertyBox == INDEXED_COMBO )
    {

    }
	 	bool bRes = (*get_)(pk, m_sName, m_ptrVar, uiSize);
    if ( bRes  )
    {
      if ( m_pNotification )
        m_pNotification( m_sName , m_pObject , bInvalidate , m_bInitRescan ) ;
      else if ( m_pNotificationWithId )
        m_pNotificationWithId(m_sName , m_pObject , m_iPropId) ;
      if (m_bInvalidate)
        bInvalidate = true ;
    }
    
	 	return bRes;
	};

	virtual void ScanSettings(FXString& text) = 0;

	friend class UserGadgetBase;
};

struct SEditProperty : SProperty
{
protected:

	SEditProperty(LPCTSTR sName, void* ptrVar, EVariableType eVariableType) :
		 SProperty(sName, ptrVar, eVariableType, EDITBOX)
	{

	};
	SEditProperty(const SEditProperty &obj) : SProperty(obj)
	{

	}
	inline void ScanSettings(FXString& text)					
	{
		 text.Append(SETUP_EDITBOX);
		 text.Append(_T("("));
		 text.Append(m_sName);
		 text.Append(_T(")"));
	};

  friend class UserGadgetBase;
};

struct SCheckBoxProperty : SProperty
{
protected:
  SCheckBoxProperty( LPCTSTR sName , void* ptrVar , EVariableType eVariableType )
    : SProperty( sName , ptrVar , eVariableType , CHECK_BOX )
  {
  };
  SCheckBoxProperty( const SCheckBoxProperty &obj ) : SProperty( obj )
  {
  }

  inline void ScanSettings( FXString& text )
  {
    text.Append( SETUP_CHECKBOX );
    text.Append( _T( "(" ) );
    text.Append( m_sName );
    text.Append( _T( ")" ) );
  };
  friend class UserGadgetBase;
};

struct SBinaryProperty : SProperty
{
protected:

	SBinaryProperty(LPCTSTR sName, void* ptrVar, EVariableType eVariableType, UINT uiBinarySize) :
		 SProperty(sName, ptrVar, eVariableType, EDITBOX)
	 {
		m_uiBinarySize = uiBinarySize;
	 };
	 SBinaryProperty(const SBinaryProperty &obj) : SProperty(obj)
	 {
		m_uiBinarySize = obj.m_uiBinarySize;
	 }
	 inline void ScanSettings(FXString& text)					
	 {
		 text.Append(SETUP_CHECKBOX);
		 text.Append(_T("("));
		 text.Append(m_sName);
		 text.Append(_T(")"));
	 };

   friend class UserGadgetBase;
};

struct SSpinProperty : SProperty
{
protected:

	int	m_iSpinMin, m_iSpinMax;

	SSpinProperty(LPCTSTR sName, void* ptrVar, EVariableType eVariableType, int iSpinMin, int iSpinMax ) :
		SProperty(sName, ptrVar, eVariableType, SPIN), m_iSpinMin(iSpinMin), m_iSpinMax(iSpinMax)
		{

		};
	SSpinProperty(const SSpinProperty &obj) : SProperty(obj)
	{
		m_iSpinMin	= obj.m_iSpinMin;
		m_iSpinMax	= obj.m_iSpinMax;
	}
	inline void ScanSettings(FXString& text)				
	{		
		FXString sMin, sMax;
		sMin.Format("%d",m_iSpinMin);
		sMax.Format("%d",m_iSpinMax);

		text.Append(SETUP_SPIN);
		text.Append(_T("("));
		text.Append(m_sName);
		text.Append(_T(",")+sMin+_T(",")+sMax+_T(")"));
	};

  friend class UserGadgetBase;
};

struct SSpinAndBoolProperty : SProperty
{
protected:

	int	m_iSpinMin, m_iSpinMax;
	SpinBoolStuct m_SSpinBool;

	SSpinAndBoolProperty(LPCTSTR sName, void* ptrVar, 
    EVariableType eVariableType, int	iSpinMin, int iSpinMax, bool *ptrBool)
    :	SProperty(sName, &m_SSpinBool , eVariableType, SPIN_BOOL)
    , m_iSpinMin(iSpinMin), m_iSpinMax(iSpinMax)
	{
		m_SSpinBool.setPtr((int*)ptrVar, ptrBool);
	};
	SSpinAndBoolProperty(const SSpinAndBoolProperty &obj) : SProperty(obj)
		{
			m_iSpinMin	= obj.m_iSpinMin;
			m_iSpinMax	= obj.m_iSpinMax;
			m_SSpinBool = obj.m_SSpinBool;
		}
	inline void ScanSettings(FXString& text)				
	{		
		FXString sMin, sMax;
		sMin.Format("%d",m_iSpinMin);
		sMax.Format("%d",m_iSpinMax);

		text.Append(SETUP_SPINABOOL);
		text.Append(_T("("));
		text.Append(m_sName);
		text.Append(_T(",")+sMin+_T(",")+sMax+_T(")"));
	};

  friend class UserGadgetBase;
};

struct SComboProperty : SProperty
{
private:
  SComboProperty(const SComboProperty &obj); //: SProperty(obj)
  //{
  //  //if (m_pList)
  //  //  delete m_pList;
  //  m_pList = NULL;
  //  m_pList = obj.m_pList;
  //}

  SComboProperty& operator=(const SComboProperty &obj);

protected:

	const char *m_pList;
  Int64Vector m_iIndexes ;

	SComboProperty(LPCTSTR sName, void* ptrVar, EVariableType eVariableType, 
    const char *pList , __int64 * piIndexes = NULL ) :
		SProperty(sName, ptrVar, eVariableType, COMBO), m_pList(pList) 
	{
    if ( piIndexes )
    {
      const char * pFound = strchr( m_pList , ';' ) ;
      while ( pFound )
      {
        m_iIndexes.push_back( *(piIndexes++) ) ;
        pFound = strchr( pFound + 1 , ';' ) ;
      }
    }
	};

	inline void ScanSettings(FXString& text)
	{
		int iCnt = 0;
		char sList[2048];
		char * pch;

		text.Append(SETUP_COMBOBOX);
		text.Append(_T("("));
		text.Append(m_sName);
		text.Append(_T("("));

		strncpy_s(sList,m_pList, 2048 );
    TCHAR * pContext = NULL;
		pch = strtok_s (sList,  ";" , &pContext );

		FXString sCnt;
		while (pch != NULL)
		{
			if(iCnt>0) 
				text.Append(_T(","));

			sCnt.Format("%d", m_iIndexes.size() ? m_iIndexes[iCnt++] : iCnt++);

			text.Append(_T(pch));
			text.Append(_T("(")+sCnt+_T(")"));
			pch = strtok_s(NULL,";" , &pContext);
		}				

		text.Append(_T("))"));
	};

  friend class UserGadgetBase;

public:
  inline bool getCombo( FXPropertyKit &pk , bool& bInvalidate )
  {
    int iTmp = -1 ;
    bool bRes = pk.GetInt( m_sName , iTmp );
    if ( bRes )
    {
      if ( iTmp != 0 )
      {
        if ( *( ( int* ) m_ptrVar ) != iTmp )
          *( ( int* ) m_ptrVar ) = iTmp ;
        else
          bRes = false ;
      }
      else
      {
        FXString AsString ;
        if ( pk.GetString( m_sName , AsString ) )
        {
          AsString.Trim() ;
          if ( (iTmp == 0) && (AsString[ 0 ] == '0') )
          {
            if ( *( ( int* ) m_ptrVar ) != iTmp )
              *( ( int* ) m_ptrVar ) = iTmp ;
            else
              bRes = false ;
          }
          else
          {
            FXString Chooses( m_pList ) ;
            FXSIZE iPos = 0 ;
            int iIndex = 0 ;
            while ( iPos >= 0 )
            {
              FXString Choice = Chooses.Tokenize( ";" , iPos ) ;
              if ( Choice == AsString )
              {
                if ( *( ( int* ) m_ptrVar ) != iIndex )
                {
                  *( ( int* ) m_ptrVar ) = iIndex ;
                  break ;
                }
                else
                  iPos = -1 ; // for exit from loop with false result
              }
              iIndex++ ;
            }
            if ( iPos < 0 )
              bRes = false ;
          }
        }
      }
    }
    if ( bRes )
    {
      if ( m_pNotification )
        m_pNotification( m_sName , m_pObject , bInvalidate , m_bInitRescan ) ;
      else if ( m_pNotificationWithId )
        m_pNotificationWithId( m_sName , m_pObject , m_iPropId ) ;
      if ( m_bInvalidate )
        bInvalidate = true ;
    }
    return bRes ;
  }

  //SComboProperty& DestroyList()
  //{
  //  try
  //  {
  //    if (m_pList)
  //      delete m_pList;
  //  }
  //  catch(const std::exception&)
  //  {

  //  }
  //  m_pList = NULL;

  //  return *this;
  //}

};


class PropertiesArray : public FXArray<SProperty*>
{
public:

	PropertiesArray() {}

	PropertiesArray(const PropertiesArray &obj)
	{	
    Clear();
		for(int iCnt = 0; iCnt<obj.GetCount(); iCnt++)
		{
			Add(obj.GetAt(iCnt));
		}
	}
  void Clear()
  {
    for ( int iCnt = 0; iCnt < GetCount(); iCnt++ )
    {
      delete GetAt( iCnt );
    }
    RemoveAll() ;
  }
  
	~PropertiesArray()
	{
    Clear() ;
	}
};





#endif	//	#ifndef __INCLUDE__UserPropertiesStruct_H__



