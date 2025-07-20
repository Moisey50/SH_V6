#include "stdafx.h"
#include "IPImageFrame.h"


CIPImageFrame::CIPImageFrame(void) : CUserDataFrame( _T("IPCamFrame") ) 
{ 
  m_dwDataLen = 0 ; 
  m_pData = NULL ; 
} 
CIPImageFrame::CIPImageFrame(CIPImageFrame * pOrigin ) 
  : CUserDataFrame( _T("IPCamFrame") )
{
  ASSERT(pOrigin->m_DataType==userdata);
  m_DataType = userdata;
  if ( pOrigin && pOrigin->GetDataLen() && pOrigin->GetData() )
  {
    CopyAttributes(pOrigin);
    m_dwDataLen = pOrigin->GetDataLen() ;
    m_pData = new BYTE[m_dwDataLen] ;
    memcpy( m_pData , pOrigin->GetData() , m_dwDataLen ) ;
  }
  else
  {
    m_dwDataLen = 0 ; 
    m_pData = NULL ; 
  }
}

CIPImageFrame::CIPImageFrame(unsigned long ulLen , LPVOID pData ) 
  : CUserDataFrame( _T("IPCamFrame") )
{
  m_DataType = userdata;
  if ( ulLen && pData )
  {
    m_dwDataLen = ulLen ;
    m_pData = new BYTE[m_dwDataLen] ;
    memcpy( m_pData , pData , m_dwDataLen ) ;
  }
  else
  {
    m_dwDataLen = 0 ; 
    m_pData = NULL ; 
  }
}

CIPImageFrame::~CIPImageFrame(void)
{
  if ( m_pData )
    delete[] m_pData ;
}

BOOL CIPImageFrame::Serialize(LPBYTE* ppData, FXSIZE* cbData) const
{
  ASSERT(ppData);
  FXSIZE cb;
  if (!CDataFrame::Serialize(ppData, &cb))
    return FALSE;
  FXSIZE dwNameLength = m_UserDataName.GetLength() + 1 ;
  *cbData = cb + dwNameLength + sizeof(DWORD) + m_dwDataLen ;
  *ppData = (LPBYTE)realloc(*ppData, *cbData);
  LPBYTE ptr = *ppData + cb;
  memcpy( ptr , (LPCTSTR) m_UserDataName , dwNameLength ) ;
  ptr += dwNameLength ;
  *((__int64*)ptr) = m_dwDataLen ;
  ptr += sizeof( __int64 ) ;
  memcpy( ptr , m_pData , m_dwDataLen ) ;
  return TRUE;
}

BOOL CIPImageFrame::Restore(LPBYTE lpData, FXSIZE cbData)
{
  FXSIZE cb;
  if (!CDataFrame::Serialize(NULL, &cb) || !CDataFrame::Restore(lpData, cbData))
    return FALSE;
  LPBYTE ptr = lpData + cb;
  m_UserDataName = (char*)ptr ;
  ptr += m_UserDataName.GetLength() + 1 ;
  m_dwDataLen = *((FXSIZE*)ptr) ;
  if ( m_dwDataLen )
  {
    ptr += sizeof(m_dwDataLen) ;
    m_pData = new BYTE[m_dwDataLen] ;
    memcpy( m_pData , ptr , m_dwDataLen ) ;
    return TRUE ;
  }
  else
    m_pData = NULL ;
  return TRUE;
}
