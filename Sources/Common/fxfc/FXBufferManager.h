#pragma once

// FXBufferManager.h: interface for the FXBufferManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(FXBUFFER_MANAGER_H__)
#define FXBUFFER_MANAGER_H__

#include <fxfc/fxfc.h>
using namespace std;
#include <vector>

#pragma pack(push, BusyFlagAndBufdef)
#pragma pack(1)

struct BusyFlagAndBuf
{
  int m_BusyFlag ;
  BYTE m_Buf[ 1 ] ;
};

#pragma pack(pop, BusyFlagAndBufdef)

class FXFC_EXPORT FXBufferManager
{
protected:
  int m_iBufferSize ;
  vector<LPVOID> m_Buffers ;
  vector<bool> m_BusyFlags ;
  int m_iNextBufferIndex ;
  bool m_bGetEnabled ;
  FXString m_ManagerName ;
  std::mutex  m_VectorAccessMutex ;
  bool        m_bPageBoundary ; // if true, buffers will be allocated on page boundary
                                // and size will multiply of page size (4096)

public: 
  double m_dAverageLifeTime ;

public:
  FXBufferManager( int iBufferSize = 0 , int iNBuffers = 0 , 
    bool bPageBoundary = false , LPCTSTR = "Unknown" ) ;
  virtual ~FXBufferManager() ;
  BOOL    Allocate( int iBufferSize , int iNBuffers , bool bPageBoundary = false);
  int     GetBuffer() ; // Get buffer Index
  LPBYTE  GetBufferPtr( int iBufNumber ) ; // Get pointer to data for index
  int     GetBufferSize() { return m_iBufferSize ;  }
  UINT    GetNBuffers() { return (UINT)m_Buffers.size() ; }
  int     GetNBusyBuffers() ;
  BOOL    ReleaseBuffer( int iBufNumber ) ; // Marking buffer as free by index
  BOOL    ReleaseBuffer( LPVOID pBuffer ) ; // Marking buffer as free by pointer
  BOOL    ReleaseLastAllocatedBuffer() ; // Marking last allocated buffer as free by pointer
  BOOL    RemoveAllBuffers() ; // Full memory release
  void    SetEnable( bool bEnable ) { m_bGetEnabled = bEnable ; } ;// not providing buffers if not enabled
};

#endif



