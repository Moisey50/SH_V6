// VectorFieldFrame.cpp: implementation of the CVectorFieldFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVObjDataFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTVObjDataFrame::CTVObjDataFrame( const CTVObjDataFrame * pOrig ) :
  CUserDataFrame( TVOBJECTS_DATA_NAME )
{
  m_pTVObjData = NULL ;
  m_uiDataLen = 0 ;
  if ( pOrig )
  {
    m_pTVObjData = ( LPBYTE )malloc( pOrig->GetThisFrameDataLen() ) ;
    if ( m_pTVObjData )
    {
      m_uiDataLen = pOrig->GetThisFrameDataLen() ;
      memcpy( m_pTVObjData , pOrig->m_pTVObjData , m_uiDataLen ) ;
      m_Indexes.Append( pOrig->m_Indexes ) ;
    }
    else
      ASSERT( 0 ) ;
  }
}

CTVObjDataFrame::CTVObjDataFrame( VOArray& ObjArr ) :
CUserDataFrame( TVOBJECTS_DATA_NAME )
{
  m_pTVObjData = NULL ;
  m_uiDataLen = 0 ;
  for ( int i = 0 ; i < ObjArr.GetCount() ; i++ )
  {
    AddObject( &ObjArr.GetAt( i ) ) ;
  }
  
}

CTVObjDataFrame::~CTVObjDataFrame()
{
  if ( m_pTVObjData )
  {
    free( m_pTVObjData ) ;
    m_pTVObjData = NULL ;
    m_uiDataLen = 0 ;
    m_Indexes.RemoveAll() ;
  }
}

CTVObjDataFrame * CTVObjDataFrame::Create( VOArray& Objects )
{
  CTVObjDataFrame * pOut = new CTVObjDataFrame ;

  for ( int i = 0 ; i < Objects.GetCount() ; i++ )
  {
    CVideoObject& NewObject = (CVideoObject&)Objects.GetAt( i ) ;
    pOut->AddObject( &NewObject ) ;
  }
  
  return pOut ;
}
BOOL CTVObjDataFrame::AddObject( const CVideoObject * pObject )
{
  if ( !pObject->m_WhatIsMeasured )
    return FALSE ;

  LPCTSTR pName =  (LPCTSTR)(pObject->m_ObjectName)  ;
  int iNameLen = (int) pObject->m_ObjectName.GetLength() ;
  if ( iNameLen > MAX_OBJ_NAME_LEN )
    iNameLen = MAX_OBJ_NAME_LEN ;

  switch ( pObject->m_Type )
  {
    case SPOT :
    {
      CSegmentation * pSegm = pObject->m_pSegmentation ;
      if ( pSegm )
      {
        FXUintArray& ActiveSpots = pSegm->m_ActiveSpots ;
        SpotArray& Spots = pSegm->m_ColSpots ;
        for ( int i = 0 ; i < ActiveSpots.GetCount() ; i++ )
        {
          int iIndex = ActiveSpots[ i ] ;
          CColorSpot& Spot = Spots.GetAt( iIndex ) ;
          memcpy( Spot.m_szName , pName , iNameLen ) ;
          Spot.m_szName[ iNameLen ] = '\0' ;

          UINT iSavedDataLen = m_uiDataLen ;
          BOOL bResult = Spot.Serialize( &m_pTVObjData , &m_uiDataLen ) ;
          if ( bResult )
            ASSERT( m_uiDataLen > iSavedDataLen ) ;
          else            
            ASSERT( 0 ) ;
        }
        return TRUE ;
      }
      return FALSE ;
    }
    break ;
    case LINE_SEGMENT :
    case EDGE :
    {
      const CLineResults& LineResults = pObject->m_LineResults ;
      for ( int i = 0 ; i < LineResults.GetCount() ; i++ )
      {
        CLineResult& Line = ( CLineResult& )LineResults.GetAt( i ) ;
        memcpy( (void*)Line.m_szName , pName , iNameLen ) ;
        Line.m_szName[ iNameLen ] = '\0' ;
        UINT iSavedDataLen = m_uiDataLen ;
        BOOL bResult = Line.Serialize( &m_pTVObjData , &m_uiDataLen ) ;
        if ( bResult )
          ASSERT( m_uiDataLen > iSavedDataLen ) ;
        else
          ASSERT( 0 ) ;
      }
      return TRUE ;
    }
    break ;
    case OCR:
    {
      if ( pObject->m_WhatIsMeasured & MEASURE_TEXT )
      {
        FXString& Text = (FXString&)(pObject->m_TextResult) ;
        UINT uiTextLen = (UINT)Text.GetLength() ;
        UINT uiLen = sizeof( UINT ) + sizeof( VOBJ_TYPE ) + EMBEDDED_OBJ_NAME_LEN + uiTextLen + 1 ;
        UINT uiOLdLEn = m_uiDataLen ;
        UINT uiReallocLen = uiOLdLEn + uiLen ;
        m_pTVObjData = ( LPBYTE )realloc( m_pTVObjData , uiReallocLen ) ;
        if ( m_pTVObjData )
        {
          m_uiDataLen = uiReallocLen ;
          LPBYTE pNewBlock = m_pTVObjData + uiOLdLEn ;
          *( ( UINT* )pNewBlock ) = uiLen ;
          pNewBlock += sizeof( UINT ) ;
          *( ( VOBJ_TYPE* )pNewBlock ) = OCR ; // type save
          pNewBlock += sizeof( VOBJ_TYPE ) ;
          memset( pNewBlock , 0 , EMBEDDED_OBJ_NAME_LEN ) ;
          memcpy( pNewBlock , pName , iNameLen ) ;
          pNewBlock += EMBEDDED_OBJ_NAME_LEN ;
          memcpy( pNewBlock , ( LPCTSTR )Text , uiTextLen ) ;
          pNewBlock[ uiTextLen ] = '\0' ;
          return TRUE ;
        }
        ASSERT( 0 ) ; // no memory
      }
    }
    break ;
    default :
      return FALSE ;
  }
  ASSERT( 0 ) ;
  return FALSE ;
}

BOOL CTVObjDataFrame::SerializeUserData( LPBYTE* ppData , UINT* cbData ) const
{
  if ( !m_uiDataLen )
    return FALSE ; // no data
  UINT uiFrameLen = m_uiDataLen ;
  UINT uiReallocLen = uiFrameLen + *cbData ;
  LPBYTE pNewBuffer = ( LPBYTE )realloc( *ppData , uiReallocLen ) ;
  if ( pNewBuffer )
  {
    LPBYTE pNewData = pNewBuffer + *cbData ;
    memcpy( pNewData , m_pTVObjData , m_uiDataLen ) ;
    return TRUE ;
  }
  ASSERT( 0 ) ; // no realloc
  return FALSE;
}


BOOL CTVObjDataFrame::Serialize( LPBYTE* ppData , UINT* cbData ) const
{
  if ( !ppData || !cbData )
    return FALSE ;
  
  UINT uiNameLen = (UINT) m_UserDataName.GetLength() ;
	UINT uiFrameLen = uiNameLen + 1 + m_uiDataLen ;
//   UINT uiReallocLen = uiFrameLen + *cbData ;
//   *ppData = NULL ;
//   LPBYTE pNewBuffer = ( LPBYTE )realloc( *ppData , uiReallocLen ) ;
  LPBYTE pNewBuffer = *ppData = ( LPBYTE )malloc( uiFrameLen ) ;
  if ( pNewBuffer )
  {
//     LPBYTE pNewData = pNewBuffer + *cbData ;
//    memcpy( pNewData , (LPCTSTR)m_UserDataName , m_UserDataName.GetLength() ) ;
    memcpy( pNewBuffer , ( LPCTSTR )m_UserDataName , m_UserDataName.GetLength() ) ;
//    pNewData[ uiNameLen ] = '\0' ;
    pNewBuffer[ uiNameLen ] = '\0' ;
    if ( m_uiDataLen )
    {
      LPBYTE dst = pNewBuffer + uiNameLen + 1;
      memcpy( dst , m_pTVObjData , m_uiDataLen ) ;
    }
    return TRUE ;
  }
  ASSERT( 0 ) ; // no realloc or malloc
	return FALSE;
}

BOOL CTVObjDataFrame::RestoreUserData( LPBYTE lpData , UINT cbData )
{
  if ( m_pTVObjData )
    free( m_pTVObjData ) ;

  m_pTVObjData = ( LPBYTE )malloc( cbData ) ;
  if ( m_pTVObjData )
  {
    m_uiDataLen = cbData ;
    memcpy( m_pTVObjData , lpData , m_uiDataLen ) ;
    return TRUE;
  }
  ASSERT( 0 ) ;
  return FALSE ;
}

BOOL CTVObjDataFrame::Restore( LPBYTE lpData , UINT cbData )
{
	if (strcmp((char*)lpData, m_UserDataName))
		return FALSE;
  UINT uiUserNameLen = (UINT) strlen((char*)lpData) + 1;
  LPBYTE pObjectsData = lpData + uiUserNameLen ;
	if ( m_pTVObjData )
    free( m_pTVObjData ) ;

  m_pTVObjData = (LPBYTE)malloc( cbData - uiUserNameLen ) ;
  if ( m_pTVObjData )
  {
    m_uiDataLen = cbData - uiUserNameLen ;
    memcpy( m_pTVObjData , lpData + uiUserNameLen , m_uiDataLen ) ;
    return TRUE;
  }
  ASSERT( 0 ) ;
  return FALSE ;
}

LPBYTE CTVObjDataFrame::ExtractObject( LPBYTE pData )
{
  if ( !pData )
    return 0 ; // no data

  UINT uiObjSize = *( ( UINT* )pData ) ;
  VOBJ_TYPE ObjType = *( ( VOBJ_TYPE* )(pData + sizeof(UINT)) ) ;
  switch ( ObjType )
  {
    case SPOT:
    {
      CColorSpot * pNewSpot = new CColorSpot ;
      if ( pNewSpot->Restore( pData , uiObjSize ) )
        return (LPBYTE)pNewSpot ;
      delete pNewSpot ;
      return NULL ;
    }
    break ;
    case EDGE:
    {
      CLineResult * pNewEdge = new CLineResult ;
      if ( pNewEdge->Restore( pData , uiObjSize ) )
        return ( LPBYTE )pNewEdge ;
      delete pNewEdge ;
      return NULL ;
    }
    break ;
    case LINE_SEGMENT:
    {
      CLineResult * pNewLine = new CLineResult ;;
      if ( pNewLine->Restore( pData , uiObjSize ) )
        return ( LPBYTE )pNewLine ;
      delete pNewLine ;
      return NULL ;
    }
    break ;
    case OCR:
    {
      COCRResult * pNewOCR = new COCRResult ;
      if ( pNewOCR->Restore( pData , uiObjSize ) )
        return ( LPBYTE )pNewOCR ;
      delete pNewOCR ;
      return NULL ;
    }
    break ;

  }
  ASSERT( 0 ) ;
  return NULL ;
}


UINT CTVObjDataFrame::ExtractObjects( FXPtrArray& Results )
{
  if ( !m_pTVObjData || m_uiDataLen == 0 )
    return 0 ; // no data

  LPBYTE pNextData = m_pTVObjData ;
  LPBYTE pDataEnd = m_pTVObjData + m_uiDataLen ;

  while ( pNextData < pDataEnd )
  {
    LPBYTE pNextObjectData = pNextData ;
    UINT uiObjSize = *( ( UINT* )pNextData ) ;
    LPBYTE pNewObject = ExtractObject( pNextObjectData ) ;
    if ( pNewObject )
      Results.Add( pNewObject ) ;
    pNextData += uiObjSize ;
  }
  
  return (UINT) Results.GetCount() ;
}

CDataFrame* CTVObjDataFrame::Copy() const
{
  CTVObjDataFrame * pCopyFrame = new CTVObjDataFrame( this ) ;
  return pCopyFrame ;
}