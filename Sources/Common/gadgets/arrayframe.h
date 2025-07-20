#pragma once
#include <gadgets\gadbase.h>

typedef CDataFrame* (*FN_CREATE_FROM)(void*, UINT);

typedef struct tagARRAYFRAME
{
  datatype    _dt;
  FN_CREATE_FROM _fnCreateFrom;
  UINT        _cbElement;
  int         _count;
  void*       _lpData;
}ARRAYFRAME, *LPARRAYFRAME;

class FX_EXT_GADGET CArrayFrame : public CDataFrame, public ARRAYFRAME
{
protected:
  CArrayFrame(datatype dt, FN_CREATE_FROM fnCreateFrom, int cbElement);
  CArrayFrame(ARRAYFRAME* ArrayFrame);
  ~CArrayFrame();
public:
  void*               GetData() const         { return _lpData; }
  int                 GetCount() const        { return _count; }
  UINT                GetElementSize() const  { return _cbElement; }
  void                SetAt(int nElement, void* pData, int count);
  const CDataFrame*   GetAt(int nElement) const;
  //CDataFrame*         GetAt(int nElement);
  CArrayFrame* GetArrayFrame(LPCTSTR label = DEFAULT_LABEL);
  const CArrayFrame* GetArrayFrame(LPCTSTR label = DEFAULT_LABEL) const;
  static CArrayFrame* Create(datatype dt, FN_CREATE_FROM fnCreateFrom, UINT cbElement);
  static CDataFrame*  CreateFrom(void* pData, UINT cData);
  virtual void ToLogString(FXString& Output);
};
