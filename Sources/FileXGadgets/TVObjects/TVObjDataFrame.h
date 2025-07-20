#pragma once

#include <gadgets\gadbase.h>
#include <imageproc\clusters\segmentation.h>
#include "VideoObjects.h"
#define TVOBJECTS_DATA_NAME "TVObjectsData"

class CTVObjDataFrame : public CUserDataFrame
{
protected:
  LPBYTE      m_pTVObjData ;
  FXUintArray m_Indexes ;
  UINT        m_uiDataLen ;
  CTVObjDataFrame( const CTVObjDataFrame * pOrig = NULL );
  CTVObjDataFrame( VOArray& c );
  virtual ~CTVObjDataFrame();
public:
  virtual CUserDataFrame* CreateEmpty() const { return new CTVObjDataFrame(); }
  static  CTVObjDataFrame * Create() { return new CTVObjDataFrame(); };
  static  CTVObjDataFrame * Create( VOArray& c ) ;
  virtual CDataFrame* Copy() const ;
  virtual BOOL Serialize( LPBYTE* ppData , UINT* cbData ) const;
  virtual BOOL Restore( LPBYTE lpData , UINT cbData ) ;
  virtual BOOL SerializeUserData( LPBYTE* ppData , UINT* cbData ) const ;
  virtual BOOL RestoreUserData( LPBYTE lpData , UINT cbData ) ;
  virtual DWORD GetThisFrameDataLen() const { return m_uiDataLen ; } // len of data
  virtual DWORD GetThisFrameDataLenLen() const { return sizeof(m_uiDataLen) ; } //len of data len
  BOOL AddObject( const CVideoObject * pObject ) ;
  LPBYTE ExtractObject( LPBYTE pData ) ;
  UINT ExtractObjects( FXPtrArray& Results ) ;
};

