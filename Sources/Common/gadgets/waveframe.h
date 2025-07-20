// WaveFrameNew.h: interface for the CWaveFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEFRAME_H__9600ACD1_B777_4B0D_BBE3_172EA39E7DE2__INCLUDED_)
#define AFX_WAVEFRAME_H__9600ACD1_B777_4B0D_BBE3_172EA39E7DE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <mmsystem.h>

typedef struct tagWaveData
{
  PWAVEFORMATEX  waveformat;
  LPWAVEHDR      data;
  HWAVEIN        hWaveIn;
}WaveData , *pWaveData;

class FX_EXT_GADGET CWaveFrame : public CDataFrame ,
  public WaveData
{
protected:
  CWaveFrame( const pWaveData wd );
  CWaveFrame( const CWaveFrame* wf );
  virtual ~CWaveFrame();
  void    UnprepareHeader( HWAVEIN hwi , LPWAVEHDR pwh , UINT cbwh );
public:
  pWaveData   GetData();
  CWaveFrame* GetWaveFrame( LPCTSTR label = DEFAULT_LABEL );
  const CWaveFrame* GetWaveFrame( LPCTSTR label = DEFAULT_LABEL ) const;
  CDataFrame* Copy() const { return new CWaveFrame( this ); }
  static  CWaveFrame* Create( pWaveData data = NULL );
  BOOL        IsNullFrame() const;
  BOOL Serialize( LPBYTE* ppData , UINT* cbData ) const;
  BOOL Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const;
  BOOL Restore( LPBYTE lpData , UINT cbData );
};

#endif // !defined(AFX_WAVEFRAME_H__9600ACD1_B777_4B0D_BBE3_172EA39E7DE2__INCLUDED_)
