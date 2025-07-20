#pragma once
#include "gadgets\gadbase.h"
class CIPImageFrame :
  public CUserDataFrame
{
public:
  CIPImageFrame(void);
  CIPImageFrame(CIPImageFrame * pOrigin );
  CIPImageFrame( unsigned long ulLen , LPVOID pData );
  virtual ~CIPImageFrame(void);
  CIPImageFrame* Copy() { return new CIPImageFrame(this); }

  static CIPImageFrame * Create(CIPImageFrame * pFrame)
  {
    return new CIPImageFrame(pFrame);
  }
  static CIPImageFrame * Create(unsigned long DataLen, LPVOID pData)
  {
    return new CIPImageFrame( DataLen , pData );
  }
  FXSIZE GetDataLen() { return m_dwDataLen ; }
  const LPVOID GetData() { return m_pData ; }
  virtual CIPImageFrame* GetUserDataFrame(LPCTSTR label)
  {
    if ( !label || m_Label == label )
      return this;
    return NULL;
  }

  virtual const CIPImageFrame* GetUserDataFrame(LPCTSTR label) const
  {
    if (!label || m_Label == label)
      return this;
    return NULL;
  }
  virtual CUserDataFrame* CreateEmpty() const 
  { 
    return new CIPImageFrame ; 
  }

  bool IsNullData() { return (m_pData != NULL) ; }
  virtual BOOL Serialize(LPBYTE* ppData, FXSIZE* cbData) const;
  virtual BOOL Restore(LPBYTE lpData, FXSIZE cbData);
protected: 
  FXSIZE   m_dwDataLen ;
  LPBYTE  m_pData ;
};

