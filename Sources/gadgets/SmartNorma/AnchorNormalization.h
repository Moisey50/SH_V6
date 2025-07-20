// AnchorNormalization.h: interface for the AnchorNormalization class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANCHORNORMALIZATION_H__A608BC7E_55A5_4747_A2AC_2815455BF28F__INCLUDED_)
#define AFX_ANCHORNORMALIZATION_H__A608BC7E_55A5_4747_A2AC_2815455BF28F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <Gadgets\VideoFrame.h>

class AnchorNormalization : public CFilterGadget
{
private:
  FXLockObject       m_Lock;
  CInputConnector*  m_pInputWhiteRef;
  CInputConnector*  m_pInputBlackRef;
  pTVFrame          m_Black;
  pTVFrame          m_White;
  int               m_iOffset ;
public:
  AnchorNormalization();
  virtual void ShutDown();
  virtual int GetInputsCount();
  virtual CInputConnector*    GetInputConnector( int n );
  virtual CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ScanSettings( FXString& text );
protected:
  void OnWhiteImage( CDataFrame* lpData );
  void OnBlackImage( CDataFrame* lpData );

  friend void CALLBACK AnchorNormalization_OnWhiteImage( CDataFrame* lpData , void* lpParam , CConnector* lpInput ) { ( ( AnchorNormalization* )lpParam )->OnWhiteImage( lpData ); }
  friend void CALLBACK AnchorNormalization_OnBlackImage( CDataFrame* lpData , void* lpParam , CConnector* lpInput ) { ( ( AnchorNormalization* )lpParam )->OnBlackImage( lpData ); }
  DECLARE_RUNTIME_GADGET( AnchorNormalization );
};

#endif // !defined(AFX_ANCHORNORMALIZATION_H__A608BC7E_55A5_4747_A2AC_2815455BF28F__INCLUDED_)
