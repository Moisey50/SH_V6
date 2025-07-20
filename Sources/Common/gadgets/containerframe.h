// ContFrame.h: interface for the CContainerFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTAINERFRAME_H__6286ABBF_D172_4574_B380_10E521695C11__INCLUDED_)
#define AFX_CONTAINERFRAME_H__6286ABBF_D172_4574_B380_10E521695C11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include "VideoFrame.h"
#include "TextFrame.h"
#include "WaveFrame.h"
#include "QuantityFrame.h"
#include "RectFrame.h"
#include "FigureFrame.h"


class FX_EXT_GADGET CContainerFrame : public CDataFrame
{
  FXLockObject  m_Lock;
  FXPtrArray    m_Frames;
  FXInt64Array  m_SerializedLengthes; // for lengthes of sub frames
  UINT          m_uiSerializedLength ;  // for this frame
  UINT          m_uiNAllSubframes ;
public:
  enum CheckMode
  {
    NoCheck = 0 ,
    RepeatCheck ,
    RepeatAndNUsersCheck
  };
private:
  CheckMode     m_CheckMode ;
protected:
  CContainerFrame();
  ~CContainerFrame();
  bool IsInContainer( const CDataFrame * pDataFrame ) ;

public:
  // operations
  void AddFrame(CDataFrame* DataFrame);
  void AddFrame(const CDataFrame* DataFrame); // DataFrame->AddRef() will be called
  void PushFrame( CDataFrame* DataFrame ) ;
  void PushFrame( const CDataFrame* DataFrame ) ;
  bool InsertAfter( CDataFrame* RefFrame , CDataFrame* DataFrame );
  static CContainerFrame* Create();
#ifdef _TRACE_DATAFRAMERELEASE
  virtual bool Release(CDataFrame* Frame=NULL, LPCTSTR Name="");
#else
  virtual bool Release(CDataFrame* Frame = NULL);
#endif
  // data access
  CVideoFrame*    GetVideoFrame(LPCTSTR label = DEFAULT_LABEL);
  const CVideoFrame*    GetVideoFrame(LPCTSTR label = DEFAULT_LABEL) const;
  CTextFrame*     GetTextFrame(LPCTSTR label = DEFAULT_LABEL);
  const CTextFrame*     GetTextFrame(LPCTSTR label = DEFAULT_LABEL) const;
  CWaveFrame*     GetWaveFrame(LPCTSTR label = DEFAULT_LABEL);
  const CWaveFrame*     GetWaveFrame(LPCTSTR label = DEFAULT_LABEL) const;
  CQuantityFrame* GetQuantityFrame(LPCTSTR label = DEFAULT_LABEL);
  const CQuantityFrame* GetQuantityFrame(LPCTSTR label = DEFAULT_LABEL) const;
  CBooleanFrame*  GetBooleanFrame(LPCTSTR label = DEFAULT_LABEL);
  const CBooleanFrame*  GetBooleanFrame(LPCTSTR label = DEFAULT_LABEL) const;
  CRectFrame*     GetRectFrame(LPCTSTR label = DEFAULT_LABEL);
  const CRectFrame*     GetRectFrame(LPCTSTR label = DEFAULT_LABEL) const;
  CFigureFrame*   GetFigureFrame(LPCTSTR label = DEFAULT_LABEL);
  const CFigureFrame*   GetFigureFrame(LPCTSTR label = DEFAULT_LABEL) const;
  CUserDataFrame*	GetUserDataFrame(LPCTSTR uType, LPCTSTR label = DEFAULT_LABEL);
  const CUserDataFrame*	GetUserDataFrame(LPCTSTR uType, LPCTSTR label = DEFAULT_LABEL) const;
  const CArrayFrame* GetArrayFrame(LPCTSTR label = DEFAULT_LABEL) const;
  CArrayFrame* GetArrayFrame(LPCTSTR label = DEFAULT_LABEL);
  CFramesIterator* CreateFramesIterator(datatype type) const;
  CDataFrame* CopyContainer();
  CDataFrame* Copy() const;
  virtual FXSIZE GetSerializeLength( FXSIZE& uiLabelLen ,
    UINT& uiNFramesInContainer , FXSIZE * pAttribLen = NULL ) ;
  FXInt64Array * GetLengthsOfFrames() { return &m_SerializedLengthes ; }
  UINT GetNAllSubFrames() { return m_uiNAllSubframes ; }
  bool        IsContainer() const  { return true; }
  virtual FXSIZE GetFramesCount() { return m_Frames.GetCount(); }
  BOOL Serialize(LPBYTE* ppData, FXSIZE* cbData) const;
  BOOL Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const;
//   BOOL SerializeInLine( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const;
  BOOL Restore(LPBYTE lpData, FXSIZE cbData);
  virtual void ToLogString(FXString& Output);
  const bool IsFrameRepeated( FXString * pDescr = NULL ) ;
  void SetCheckMode( CheckMode Mode ) { m_CheckMode = Mode ; } ;
  CheckMode GetCheckMode() { return m_CheckMode ; }
  friend class CContainerIterator;
};

#endif // !defined(AFX_CONTAINERFRAME_H__6286ABBF_D172_4574_B380_10E521695C11__INCLUDED_)
