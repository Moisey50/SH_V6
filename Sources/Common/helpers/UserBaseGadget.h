#ifndef __INCLUDE__UserBaseGadget_H__
#define __INCLUDE__UserBaseGadget_H__

#pragma once
#include <gadgets\gadbase.h>

#include "gadgets\VideoFrame.h"
#include "gadgets\TextFrame.h"
#include "gadgets\WaveFrame.h"
#include "gadgets\QuantityFrame.h"
#include "gadgets\RectFrame.h"
#include "gadgets\FigureFrame.h"

#include "UserPropertiesStruct.h"


#ifndef USER_FILTER_RUNTIME_GADGET
#define USER_FILTER_RUNTIME_GADGET(gadget_class, folder) \
  IMPLEMENT_RUNTIME_GADGET_EX(gadget_class,CFilterGadget,folder,TVDB400_PLUGIN_NAME)
#endif

#ifndef USER_PORT_RUNTIME_GADGET
#define USER_PORT_RUNTIME_GADGET(gadget_class, folder) \
  IMPLEMENT_RUNTIME_GADGET_EX(gadget_class,CPortGadget,folder,TVDB400_PLUGIN_NAME)
#endif


#ifndef USER_CAPTURE_RUNTIME_GADGET
#define USER_CAPTURE_RUNTIME_GADGET(gadget_class, folder) \
  IMPLEMENT_RUNTIME_GADGET_EX(gadget_class,CCaptureGadget,folder,TVDB400_PLUGIN_NAME)
#endif


#define INPUTS_LOCKING    1
#define OUTPUTS_LOCKING   2
#define DUPLEX_LOCKING    4

class UserGadgetBase /*: virtual public CGadget*/
{
private:	


protected:

	FXArray<CInputConnector*>	  m_inputConnectors;
	FXArray<COutputConnector*>	m_outputConnectors;
	FXArray<CDuplexConnector*>	m_duplexConnectors;

	PropertiesArray m_propertiesArray ;
  UINT            m_PinAccessLocking ;
  FXLockObject    m_PropertiesReworkProtection ;

// 	virtual CDataFrame* DoProcessing(CDataFrame* pDataFrame) = 0;
	virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pDataFrame) {};
  virtual void PrintAdditionalData( FXString& Text ) {} ;
  virtual void ShutDownBase() {} ;
  virtual bool ScanPropertiesBase( LPCTSTR text , bool& Invalidate ) { return false;  } ;
  virtual bool PrintPropertiesBase( FXString& text ) { return false; } ;
  virtual void UserResume() {} ;
  virtual void PinsLockUnlock( bool bLock , bool bDoThis ) {} ;
  virtual CGadget * GetThisGadget() { return NULL; }
  virtual void ShutDown()
	{
    LockProperties() ;
    //for ( int i = 0; i < m_propertiesArray.GetCount() ; i++ )
    //{
    //  m_propertiesArray[ i ]->SetNotification( NULL , NULL ) ;
    //}
    ClearProperties();
    UnlockProperties() ;
    ShutDownBase();
		clearInputConnectors();
		clearOutputConnectors();
		clearDuplexConnectors();
    Clear() ;
	}
  virtual void RemoveInputConnector( int uiConnNumber )
  {
    if ( uiConnNumber >= m_inputConnectors.size() )
      return ;
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    delete m_inputConnectors[ uiConnNumber ] ;
    m_inputConnectors.RemoveAt( uiConnNumber ) ;
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
  }
  virtual void RemoveOutputConnector( int uiConnNumber )
  {
    if ( uiConnNumber >= m_outputConnectors.size() )
      return ;
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    delete m_outputConnectors[ uiConnNumber ] ;
    m_outputConnectors.RemoveAt( uiConnNumber ) ;
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
  }
public:

  UserGadgetBase() :
    m_PinAccessLocking( 0 )
  {} ;

	inline void init()
	{
    FXAutolock al( m_PropertiesReworkProtection ) ;
		PropertiesRegistration();
		ConnectorsRegistration();
		handleMandatoryConnectors();
    UserResume();
	}
  inline void Clear()
  {
  }

  inline void LockProperties()
  {
    m_PropertiesReworkProtection.Lock() ;
  }
  inline void UnlockProperties()
  {
    m_PropertiesReworkProtection.Unlock() ;
  }

  virtual LPCTSTR GetGadgetInfo() { return _T( "UnknownGadget" ) ; } ;

  void SetLocking( UINT uiMask ) { m_PinAccessLocking = uiMask ; }

  void RemoveOutputConnectorsAfter( int iIndex )
  {
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    int iLastIndex = (int) m_outputConnectors.GetUpperBound() ;
    while ( iLastIndex > iIndex + 1 )
    {
      delete m_outputConnectors[ iLastIndex ] ;
      m_outputConnectors.RemoveAt( iLastIndex ) ;
      --iLastIndex ;
    }
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
  }

	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate)
	{
    bool bRes = false ;
    m_PropertiesReworkProtection.Lock( INFINITE , "UserBaseGadget::ScanProperties" ) ;
    ScanPropertiesBase( text , Invalidate ) ;

    FXPropertyKit pk( text );
    for ( int iCnt = 0; iCnt < m_propertiesArray.GetCount(); iCnt++ )
    {
      bool bLastOK = false ;
      if ( m_propertiesArray.GetAt( iCnt )->IsCombo() )
      {
        bLastOK = ( ( SComboProperty* ) m_propertiesArray.GetAt( iCnt ) )->getCombo( pk , Invalidate ) ;
        if ( m_propertiesArray.GetAt( iCnt )->m_ePropertyBox == SProperty::INDEXED_COMBO )
        {
          int iAbsIndex = *( ( int* ) ( ( SComboProperty* ) m_propertiesArray.GetAt( iCnt ) )->m_ptrVar ) ;
          *( ( __int64* ) ( ( SComboProperty* ) m_propertiesArray.GetAt( iCnt ) )->m_ptrVar ) = 
            ( ( SComboProperty* ) m_propertiesArray.GetAt( iCnt ) )->m_iIndexes[ iAbsIndex ] ;
        }
      }
      else
        bLastOK |= ( m_propertiesArray.GetAt(iCnt) )->ScanProperties(pk , Invalidate);
      bRes |= bLastOK ;
      if ( bLastOK && m_propertiesArray.GetAt(iCnt)->m_bInitRescan )
      {
        m_PropertiesReworkProtection.Unlock();
        PropertiesReregistration();
        m_PropertiesReworkProtection.Lock( INFINITE , "UserBaseGadget::ScanProperties.AfterReregist" );
        ( m_propertiesArray.GetAt( iCnt ) )->m_bInitRescan = false ;
        iCnt = -1; // restart scan, something very important changed
      }
    }
    m_PropertiesReworkProtection.Unlock();

    if ( bRes && Invalidate )
    {
      PropertiesReregistration() ;
//       FXPropertyKit pk ;
//       PrintProperties( pk ) ;
      FXAutolock al( m_PropertiesReworkProtection ) ;
      if ( m_propertiesArray.GetCount() > 0 )
      {
        for ( int iCnt = 0; iCnt < m_propertiesArray.GetCount(); iCnt++ )
          bRes |= (m_propertiesArray.GetAt( iCnt ))->ScanProperties( pk , Invalidate );
      }
    }

    return true ;
	}

  inline bool SetChangeNotification( LPCTSTR pPropertyName ,
    PropertyUpdateCB pCallBack , void * pObject )
  {
    for ( int i = 0 ; i < m_propertiesArray.GetCount() ; i++ )
    {
      if ( m_propertiesArray[ i ]->m_sName == pPropertyName )
      {
        m_propertiesArray[ i ]->SetNotification( pCallBack , pObject ) ;
        m_propertiesArray[ i ]->m_bInvalidate = true ;
        return true ;
      }
    }
    return false ;
  }
  inline bool SetChangeNotificationForLast( PropertyUpdateCB pCallBack , void * pObject )
  {
    if ( !m_propertiesArray.GetCount() )
      return false ;
    m_propertiesArray[ m_propertiesArray.GetUpperBound() ]
      ->SetNotification( pCallBack , pObject ) ;
    return true ;
  }

  inline bool SetChangeNotificationWithId( LPCTSTR pPropertyName , 
    PropertyUpdateCB_WithId pCallBack , void * pObject , int iId )
  {
    for ( int i = 0 ; i < m_propertiesArray.GetCount() ; i++ )
    {
      if ( m_propertiesArray[ i ]->m_sName == pPropertyName )
      {
        m_propertiesArray[ i ]->SetNotificationWithId( pCallBack , pObject , iId ) ;
        return true ;
      }
    }
    return false ;
  }
  inline bool SetInvalidateOnChange( LPCTSTR pPropertyName , bool bInvalidate )
  {
    for (int i = 0 ; i < m_propertiesArray.GetCount() ; i++)
    {
      if (m_propertiesArray[ i ]->m_sName == pPropertyName)
      {
        m_propertiesArray[ i ]->m_bInvalidate = bInvalidate ;
        return true ;
      }
    }
    return false ;
  }

  virtual bool PrintProperties( FXString& text )
	{
    FXAutolock al( m_PropertiesReworkProtection ) ;
    bool bRes = true;

		FXPropertyKit pk;

		for(int iCnt = 0; iCnt<m_propertiesArray.GetCount(); iCnt++)
		{
			bRes &= (m_propertiesArray.GetAt(iCnt))->PrintProperties(pk);
		}
		PrintPropertiesBase(pk);

		text+=pk;
    PrintAdditionalData( text ) ;

		return bRes;
	}

  inline bool PrintOwnProperties( FXString& text )
  {
    FXAutolock al( m_PropertiesReworkProtection ) ;
    bool bRes = true;

    FXPropertyKit pk;

    for ( int iCnt = 0; iCnt<m_propertiesArray.GetCount(); iCnt++ )
    {
      bRes &= (m_propertiesArray.GetAt( iCnt ))->PrintProperties( pk );
    }

    text += pk;
    PrintAdditionalData( text ) ;

    return bRes;
  }

  virtual void OnScanSettings() { return ; }

	virtual bool ScanSettings(FXString& text)
	{
    OnScanSettings() ;
    FXAutolock al( m_PropertiesReworkProtection ) ;
    bool bRes = (m_propertiesArray.GetCount()>0) ? true : false;

		if(bRes)
		{
			text=_T("template(");

			int iCnt;

			for(iCnt = 0; iCnt<( m_propertiesArray.GetCount()-1 ); iCnt++)
			{
				(m_propertiesArray.GetAt(iCnt))->ScanSettings(text);
				text.Append(_T(","));
			}

			(m_propertiesArray.GetAt(iCnt))->ScanSettings(text);

			text += _T(")");
		}		

		return bRes;
	}

  virtual void addInputConnector( int dataType = transparent , LPCTSTR sName = "" , 
    FN_SENDINPUTDATA fnSendInputData = NULL , void* lpHostGadget = NULL )
  {
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    m_inputConnectors.Add( new CInputConnector( dataType , fnSendInputData , lpHostGadget ) ) ;

    setConnectorDefinitions( m_inputConnectors[ m_inputConnectors.GetUpperBound() ] , sName );
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
  }
  virtual void addOutputConnector( int dataType = transparent , LPCTSTR sName = "" )
  {
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    m_outputConnectors.Add( new COutputConnector( dataType ) );
    setConnectorDefinitions( m_outputConnectors[ m_outputConnectors.GetUpperBound() ] , sName );
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
  }
  virtual void addDuplexConnector( int outDataType = transparent , int inDataType = transparent , LPCTSTR sName = "" )
  {
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    m_duplexConnectors.Add( new CDuplexConnector( GetThisGadget() , outDataType , inDataType ) );
    setConnectorDefinitions( m_duplexConnectors[ m_duplexConnectors.GetUpperBound() ] , sName );
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
  }

  virtual int GetInputsCount()
	{
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    int iCount = (int) m_inputConnectors.GetCount();
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
    return iCount ;
	}
  virtual int GetOutputsCount()
	{
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    int iCount = (int) m_outputConnectors.GetCount();
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
    return iCount ;
	}
  virtual int GetDuplexCount()
	{
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    int iCount = (int) m_duplexConnectors.GetCount();
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
    return iCount ;
	}

  virtual CInputConnector*		GetInputConnector(int n)
	{
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    CInputConnector * Connector = ((UINT) n < (UINT) m_inputConnectors.GetCount()) ?
      m_inputConnectors[n] : NULL;
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
    return Connector ;
	}
  virtual COutputConnector*	GetOutputConnector(int n)
	{
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    COutputConnector * Connector = ( (UINT)n < (UINT)m_outputConnectors.GetCount()) ?
      m_outputConnectors[n] : NULL;
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
    return Connector ;
	}
  virtual CDuplexConnector*	GetDuplexConnector(int n)
	{
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    CDuplexConnector * Connector = ( (UINT)n < (UINT)m_duplexConnectors.GetCount()) ?
      m_duplexConnectors[n] : NULL;
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
    return Connector ;
	}   

  
	CDuplexConnector * m_pDuplexConnector;

  virtual void handleMandatoryConnectors()
	{
// 		m_pInput = GetInputConnector(0);
// 		m_pOutput = GetOutputConnector(0);
		m_pDuplexConnector = GetDuplexConnector(0);
	}

private:


	inline void clearInputConnectors()
	{
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
		for(int iCnt=0; iCnt<m_inputConnectors.GetCount(); iCnt++)
			delete m_inputConnectors[iCnt];

		m_inputConnectors.RemoveAll();
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
  }
	inline void clearOutputConnectors()
	{
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    for ( int iCnt = 0; iCnt < m_outputConnectors.GetCount(); iCnt++ )
			delete m_outputConnectors[iCnt];

		m_outputConnectors.RemoveAll();
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
  }
	inline void clearDuplexConnectors()
	{
    PinsLockUnlock( true , m_PinAccessLocking & INPUTS_LOCKING ) ;
    for ( int iCnt = 0; iCnt < m_duplexConnectors.GetCount(); iCnt++ )
			delete m_duplexConnectors[iCnt];

    m_duplexConnectors.RemoveAll();
    PinsLockUnlock( false , m_PinAccessLocking & INPUTS_LOCKING ) ;
  }


protected:

	//	TO DO: Add Properties and Connectors into PropertiesRegistration and ConnectorsRegistration accordingly
	virtual void PropertiesRegistration() = 0;
	virtual void ConnectorsRegistration() = 0;
  virtual void ClearProperties()
  {
    for (int i = 0; i < m_propertiesArray.GetCount(); i++)
    {
      m_propertiesArray[i]->SetNotification(NULL, NULL);

      //SComboProperty* pComboProp = (SComboProperty*)m_propertiesArray[i];
      //
      //if (pComboProp)
      //  pComboProp->DestroyList();
      
      delete m_propertiesArray[i];
      m_propertiesArray[i] = NULL;
    }
    m_propertiesArray.Clear() ;
  }
  virtual void PropertiesReregistration() 
  {
    FXAutolock al( m_PropertiesReworkProtection ) ;
    ClearProperties() ;
    PropertiesRegistration() ;
  }
// For end user use for ConnectorsRegistration
	inline int createComplexDataType(int numberOfDataTypes, basicdatatype dataType=transparent, ...)
	{
		int type = dataType;
		basicdatatype additionalType;

		va_list argsList;
		va_start(argsList,dataType);		

		for (int iCnt=1; iCnt<numberOfDataTypes; iCnt++)
		{
			additionalType = va_arg(argsList,basicdatatype);
			type = type*additionalType;
		}
		va_end(argsList);

		return type;
	}

  inline void setConnectorDefinitions( CConnector* pConnector , LPCTSTR sName = "" )
  {
    if ( strcmp( sName , "" ) )
    {
      pConnector->SetName( sName );
    }
  }


// For end user use for PropertiesRegistration
	/*
EVariableType:

		Int				
		Long			
		Double			
		Bool			
		Binary			
		String			

		EDITBOX: 
			addProperty(EDITBOX,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType)
		SPIN:
			addProperty(SPIN,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax)
		SPIN_BOOL:
			addProperty(SPIN_BOOL,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax, bool *ptrBool)
		COMBO:
			addProperty(COMBO,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, const char *pList)
		BINARY_BOX: 
			addProperty(BINARY_BOX,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, UINT uiBinarySize)

	*/
	inline bool addProperty(SProperty::PropertyBox ePropertyBox, 
    LPCTSTR sName, void* ptrVar, SProperty::EVariableType eVariableType, ...)
	{
		SProperty * pNewProperty;

		va_list argsList;
		va_start(argsList,eVariableType);

		switch (ePropertyBox)
		{

		case (SProperty::EDITBOX):

			pNewProperty = new SEditProperty( sName , ptrVar , eVariableType ) ;

			break;

    case ( SProperty::CHECK_BOX ):

      pNewProperty = new SCheckBoxProperty( sName , ptrVar , eVariableType ) ;

      break;
    case ( SProperty::BINARY_BOX ):

			{
				UINT uiBinarySize = va_arg( argsList , UINT );
				pNewProperty = new SBinaryProperty( sName , ptrVar , eVariableType , uiBinarySize ) ;
			}

			break;

		case (SProperty::SPIN):

			{
				int iSpinMin	= va_arg( argsList , int );
				int iSpinMax	= va_arg( argsList , int );
				pNewProperty = new SSpinProperty( sName , ptrVar , 
          eVariableType , iSpinMin , iSpinMax ) ;
			}
			
			break;

		case (SProperty::SPIN_BOOL):

			{
				int iSpinMin	= va_arg( argsList , int );
				int iSpinMax	= va_arg( argsList , int );
				bool* ptrBool	= va_arg( argsList , bool* );
				pNewProperty = new SSpinAndBoolProperty( sName , ptrVar , 
          eVariableType , iSpinMin , iSpinMax , ptrBool );
 //       pNewProperty->m_ptrVar = ptrVar ;
      }
			
			break;

		case (SProperty::COMBO):
    case (SProperty::INDEXED_COMBO):
			{
				const char* pList = va_arg( argsList , const char* );
        __int64 * pIndexes = (ePropertyBox == SProperty::INDEXED_COMBO) ?
          va_arg( argsList , __int64* ) : NULL ;
				pNewProperty = new SComboProperty( sName , ptrVar , eVariableType , pList , pIndexes );
			}
			
			break;

		default:

			{
				pNewProperty = NULL;
			}

			break;
		}

		va_end( argsList );

		bool bIsOK = ( pNewProperty != NULL );

		if(bIsOK)
		{
			m_propertiesArray.Add( pNewProperty );
		}	

		return bIsOK;
	}	
};

class UserBaseGadget : public CFilterGadget , public UserGadgetBase
{
private:

protected:

  virtual void ShutDownBase()
  {
    CFilterGadget::ShutDown();
//    CGadget::ShutDown();
  }
  virtual void UserResume() { Resume();  } ;

  virtual void PinsLockUnlock( bool bLock , bool bDoThis )
  {
    if ( bDoThis )
    {
      if ( bLock )
        m_PinsLock.Lock() ;
      else 
        m_PinsLock.Unlock() ;
    }
  } ;
  virtual CGadget * GetThisGadget() { return this; }

  virtual void handleMandatoryConnectors()
  {
    UserGadgetBase::handleMandatoryConnectors() ;
    m_pInput = UserGadgetBase::GetInputConnector( 0 );
    m_pOutput = UserGadgetBase::GetOutputConnector( 0 );
  }
public:

  UserBaseGadget() : CFilterGadget(), UserGadgetBase()
  {} ;

  virtual LPCTSTR GetGadgetInfo()
  {
    static FXString GadgetName ;
    if ( GetGadgetName( GadgetName ) )
      return GadgetName ;
    return _T( "UnknownGadget" ) ;
  }

  virtual bool ScanPropertiesBase( LPCTSTR text , bool& Invalidate )
  {
    return CFilterGadget::ScanProperties( text , Invalidate ) ;
  }
  virtual bool PrintPropertiesBase( FXString& text )
  {
    FXPropertyKit pk;

    bool bRes = CFilterGadget::PrintProperties( pk );

    text += pk;

    return bRes;
  }

  virtual CInputConnector * GetInputConnector( int iInpNumber )
  {
    return UserGadgetBase::GetInputConnector( iInpNumber );
  }
  virtual COutputConnector * GetOutputConnector( int iOutNumber )
  {
    return UserGadgetBase::GetOutputConnector( iOutNumber );
  }
  virtual CDuplexConnector * GetDuplexConnector( int iDupNumber )
  {
    return UserGadgetBase::GetDuplexConnector( iDupNumber );
  }

  virtual int  GetInputsCount()
  {
    return UserGadgetBase::GetInputsCount();
  }
  virtual int GetOutputsCount()
  {
    return UserGadgetBase::GetOutputsCount();
  }
  virtual int GetDuplexCount()
  {
    return UserGadgetBase::GetDuplexCount();
  }

  virtual bool ScanSettings( FXString& text )
  {
    return UserGadgetBase::ScanSettings( text ) ;
  }
  virtual bool PrintProperties( FXString& text )
  {
    return UserGadgetBase::PrintProperties( text ) ;
  }
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate )
  {
    return UserGadgetBase::ScanProperties( text , Invalidate ) ;
  }


private:


protected:
 // DECLARE_RUNTIME_GADGET( UserBaseGadget );

};

class Intermediate : public UserBaseGadget
{
public:
  Intermediate() {} ;
};
class UserCaptureBaseGadget : public CCaptureGadget , public UserGadgetBase
{
private:

protected:

  virtual void ShutDownBase()
  {
    CCaptureGadget::ShutDown();
  }
  virtual void UserResume() { Resume(); } ;

  virtual void PinsLockUnlock( bool bLock , bool bDoThis )
  {
    if ( bDoThis )
    {
      if ( bLock )
        m_PinsLock.Lock() ;
      else
        m_PinsLock.Unlock() ;
    }
  } ;
  virtual CGadget * GetThisGadget() { return this; }

  virtual void handleMandatoryConnectors()
  {
    UserGadgetBase::handleMandatoryConnectors() ;
    //m_pInput = UserGadgetBase::GetInputConnector( 0 );
    m_pOutput = UserGadgetBase::GetOutputConnector( 0 );
  }

public:

  UserCaptureBaseGadget() : CCaptureGadget() , UserGadgetBase()
  {} ;

  virtual bool BuildPropertyList() { return false; }
  virtual LPCTSTR GetGadgetInfo()
  {
    FXString GadgetName ;
    if ( GetGadgetName( GadgetName ) )
      return GadgetName ;
    return _T( "UnknownGadget" ) ;
  }

  virtual bool ScanPropertiesBase( LPCTSTR text , bool& Invalidate )
  {
    return CCaptureGadget::ScanProperties( text , Invalidate ) ;
  }
  virtual bool PrintPropertiesBase( FXString& text )
  {
    FXPropertyKit pk;

    bool bRes = CCaptureGadget::PrintProperties( pk );

    text += pk;

    return bRes;
  }
  virtual bool ScanSettings( FXString& text )
  {
    return UserGadgetBase::ScanSettings( text ) ;
  }
  virtual bool PrintProperties( FXString& text )
  {
    return UserGadgetBase::PrintProperties( text ) ;
  }
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate )
  {
    return UserGadgetBase::ScanProperties( text , Invalidate ) ;
  }
  inline void init()
  {
    FXAutolock al( m_PropertiesReworkProtection , "") ;
    PropertiesRegistration();
    ConnectorsRegistration();
    handleMandatoryConnectors();
//     while ( !m_pStatus )
//       Sleep( 10 ) ;
//     UserResume();
  }

  // 
  //   virtual void OnStart()
  //   {
  //     return ;
  //   }
  //   virtual void OnStop()
  //   {
  //     return ;
  //   }
  // 
  DECLARE_RUNTIME_GADGET( UserCaptureBaseGadget );

private:


protected:
  virtual void PropertiesRegistration() {};
  virtual void ConnectorsRegistration() {};

};

class UserPortBaseGadget : public CPortGadget , public UserGadgetBase
{
private:

protected:

  virtual void ShutDownBase()
  {
    CFilterGadget::ShutDown();
//    CGadget::ShutDown();
  }
  virtual void UserResume() { Resume(); } ;

  virtual void PinsLockUnlock( bool bLock , bool bDoThis )
  {
    if ( bDoThis )
    {
      if ( bLock )
        m_PinsLock.Lock() ;
      else
        m_PinsLock.Unlock() ;
    }
  } ;
  virtual CGadget * GetThisGadget() { return this; }

  virtual void handleMandatoryConnectors()
  {
    UserGadgetBase::handleMandatoryConnectors() ;
    m_pInput = UserGadgetBase::GetInputConnector( 0 );
    m_pOutput = UserGadgetBase::GetOutputConnector( 0 );
  }
public:

  UserPortBaseGadget() : CPortGadget() , UserGadgetBase()
  {} ;

  virtual LPCTSTR GetGadgetInfo()
  {
    static FXString GadgetName ;
    if ( GetGadgetName( GadgetName ) )
      return GadgetName ;
    return _T( "UnknownGadget" ) ;
  }

  virtual bool ScanPropertiesBase( LPCTSTR text , bool& Invalidate )
  {
    return CPortGadget::ScanProperties( text , Invalidate ) ;
  }
  virtual bool PrintPropertiesBase( FXString& text )
  {
    FXPropertyKit pk;

    bool bRes = CPortGadget::PrintProperties( pk );

    text += pk;

    return bRes;
  }

  virtual CInputConnector * GetInputConnector( int iInpNumber )
  {
    return UserGadgetBase::GetInputConnector( iInpNumber );
  }
  virtual COutputConnector * GetOutputConnector( int iOutNumber )
  {
    return UserGadgetBase::GetOutputConnector( iOutNumber );
  }
  virtual CDuplexConnector * GetDuplexConnector( int iDupNumber )
  {
    return UserGadgetBase::GetDuplexConnector( iDupNumber );
  }

  virtual int  GetInputsCount()
  {
    return UserGadgetBase::GetInputsCount();
  }
  virtual int GetOutputsCount()
  {
    return UserGadgetBase::GetOutputsCount();
  }
  virtual int GetDuplexCount()
  {
    return UserGadgetBase::GetDuplexCount();
  }

  virtual bool ScanSettings( FXString& text )
  {
    return UserGadgetBase::ScanSettings( text ) ;
  }
  virtual bool PrintProperties( FXString& text )
  {
    return UserGadgetBase::PrintProperties( text ) ;
  }
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate )
  {
    return UserGadgetBase::ScanProperties( text , Invalidate ) ;
  }


private:


protected:
 // DECLARE_RUNTIME_GADGET( UserBaseGadget );

};


#endif	// #ifndef __INCLUDE__UserBaseGadget_H__