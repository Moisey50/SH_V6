// DataChain.h: interface for the CDataChain class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATACHAIN_H__7F6A1D2E_E1EC_4D72_8275_E58A0A29BC0C__INCLUDED_)
#define AFX_DATACHAIN_H__7F6A1D2E_E1EC_4D72_8275_E58A0A29BC0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #ifdef NO_ATL_DEBUG
// #ifdef _DEBUG
// #ifdef ATLTRACE 
// #undef ATLTRACE
// #undef ATLTRACE2
// 
// #define ATLTRACE CustomTrace
// #define ATLTRACE2 ATLTRACE
// #endif // ATLTRACE
// #endif // _DEBUG
// 
// inline void CustomTrace( const TCHAR* format , ... )
// {
//   const int TraceBufferSize = 1024;
//   TCHAR buffer[ TraceBufferSize ];
// 
//   va_list argptr;
//   va_start( argptr , format );
//   _vstprintf_s( buffer , format , argptr );
//   va_end( argptr );
// 
//   ::OutputDebugString( buffer );
// }
// 
// inline void CustomTrace( int dwCategory , int line , const TCHAR* format , ... )
// {
//   va_list argptr; va_start( argptr , format );
//   CustomTrace( format , argptr );
//   va_end( argptr );
// }
// #endif // NO_ATL_DEBUG

#include <gadgets\gadbase.h>

#pragma pack(push, FLWstructdef)
#pragma pack(1)

#define HUNK 0x4B4E5548

typedef struct _tagFLWDataFrame
{
  unsigned    uLabel;     // signature of the 
  unsigned    uSize;      // full size of the data, strings and frame data included
  unsigned    uStreamNmb; // stream number
  unsigned    uFrames;    // number of the child frames in a frame, zero for not container frame
  double      dWriteTime; // time when frame was got for writing
  DWORD       dwID;
  double      dTime;
  unsigned    uDataType;
  bool        bRegistered;
  unsigned    uLabelOffset;
  unsigned    uAttributesOffset;
  unsigned    uFrameDataOffset;
} FLWDataFrame;

#pragma pack(pop, FLWstructdef)

#define FLWFrameSizeVisible ( sizeof(FLWDataFrame::uLabel) + sizeof(FLWDataFrame::uSize) )
#define NecessarySizeInClusters(sz_bytes) ( ( ( sz_bytes / CLUSTER_SIZE ) + 1) * CLUSTER_SIZE )
#define ClusterBegin( sz_bytes ) ( ( sz_bytes / CLUSTER_SIZE ) * CLUSTER_SIZE )
class FX_EXT_GADGET CDataChain
{
protected:
  CGadget*  m_HostGadget;
  BYTE      m_TheRest[ 4096 ] ;
  int       m_iTheRestLen ;
public:
  // Video Frame writing to file timing
  double      m_dWriteFLWHeaderTime_ms , m_dWriteLabelTime_ms , m_dWriteAttribTime_ms ;
  double      m_dWriteBMIHTime_ms , m_dWriteImageTime_ms ;

  double      m_dWriteFLWHeaderTimeMin_ms , m_dWriteFLWHeaderTimeMax_ms ;
  double      m_dWriteLabelTimeMin_ms , m_dWriteLabelTimeMax_ms ;
  double      m_dWriteAttribTimeMin_ms , m_dWriteAttribTimeMax_ms ;
  double      m_dWriteBMIHTimeMin_ms , m_dWriteBMIHTimeMax_ms ;
  double      m_dWriteImageTimeMin_ms , m_dWriteImageTimeMax_ms ;

  double      m_dWriteFLWHeaderTimeMaxTime_ms ;
  double      m_dWriteLabelTimeMaxTime_ms ;
  double      m_dWriteAttribTimeMaxTime_ms ;
  double      m_dWriteBMIHTimeMaxTime_ms ;
  double      m_dWriteImageTimeMaxTime_ms ;
  int         m_iClasterSize = 4096 ;

public:
  CDataChain( CGadget* host );
  ~CDataChain();
  CDataFrame* Restore( FLWDataFrame* dt );
  FLWDataFrame* Serialize( const CDataFrame* df ) const;
  FLWDataFrame* Serialize( const CDataFrame* df ,
    LPBYTE pBuffer , __int64& CurrentWriteIndex , __int64 iBufLen ) ;
  BOOL SerializeToBuf( const CDataFrame *df ,
    LPBYTE pBuffer , __int64& CurrentWriteIndex , __int64 iBufLen ) ;
  BOOL SerializeSeparateVideoFrameToFile( const CDataFrame * Vf , FILE * pFile ) ;
  int SerializeSeparateVideoFrameToBuf(
    LPBYTE pBuf , UINT iBufLen , const CDataFrame * df ) ;
  int SerializeCommonData( LPBYTE pBuf , FSIZE iBufLen , const CDataFrame * df ) ;
  UINT GetNecessaryBufferSize( const CDataFrame * df , FXSIZE& LabelLen , FXSIZE * AttribLen = NULL ) ;

  virtual int CorrectBuffer() { return 0 ; }
} ;

#endif // !defined(AFX_DATACHAIN_H__7F6A1D2E_E1EC_4D72_8275_E58A0A29BC0C__INCLUDED_)
