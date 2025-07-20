// Amplifier.h : Declaration of the Amplifier and ChkRange classes

#pragma once
#include <gadgets\gadbase.h>
#include <gadgets\textframe.h>
#include <gadgets\QuantityFrame.h>
#include <helpers\PropertyKitEx.h>
#include <math\Intf_sup.h>

#define MASK_NSPOTS        1
#define MASK_NSPOTS_UP     2
#define MASK_NSPOTS_DOWN   4
#define MASK_MASK_VERT     8
#define MASK_MASK_HORIZ    0x10
#define MASK_ETCH_V_LEFT   0x20
#define MASK_ETCH_V_RIGHT  0x40
#define MASK_ETCH_H_UPPER  0x80
#define MASK_ETCH_H_LOWER  0x100
#define MASK_ETCH_HLEFT_U  0x200
#define MASK_ETCH_HLEFT_D  0x400
#define MASK_ETCH_VDOWN_L  0x800
#define MASK_ETCH_VDOWN_R  0x1000


#define MASK_ALL_NECESSARY 0x1FF9


enum LastMeasured
{
  LAST_UNKNOWN = 0 ,
  LAST_UP ,
  LAST_DOWN
};



class MaskData
{
public:
  MaskData() { memset( this , 0 , sizeof(*this) ) ; }

  cmplx m_MaskUp ;
  cmplx m_MaskDown ;
  cmplx m_StepUp ;
  cmplx m_StepDown ;
  int m_iNMasks ;

  cmplx m_Center ; // default - (0.0,0.0)
  cmplx m_CurrentMaskUp ;
  cmplx m_CurrentMaskDown ;

};
class ItemInData
{
public:
  ItemInData() { }
  int iInLabelPos ;
  LPCTSTR m_szInLabel ;
  int     m_iNumberIndex ;
  LPCTSTR m_szFormat ;

};

class MeasData
{
public:
  MeasData() { Reset() ; }

  void Reset() { memset( this , 0 , sizeof(*this) ) ;}
  inline cmplx GetEtchingPosition()
  {
    double dX = 0.25 * ( m_dEtch_vdl + m_dEtch_vdr + m_dEtch_vul + m_dEtch_vur ) ;
    double dY = 0.25 * ( m_dEtch_hrd + m_dEtch_hru + m_dEtch_hld + m_dEtch_hlu ) ;
    return cmplx( dX , dY ) ;
  }
  inline cmplx GetMaskPosition()
  {
    return cmplx( m_dMaskHor , m_dMaskVert ) ;
  }

  double m_dEtch_hru ;    //horizontal right up   
  double m_dEtch_hrd ;  // horizontal right down
  double m_dEtch_vul ; // vertical up left
  double m_dEtch_vur ;// vertical up right
  double m_dEtch_vdl ; // vertical down left
  double m_dEtch_vdr ; // vertical down right
  double m_dEtch_hlu ; //horizontal left up
  double m_dEtch_hld ; // horizontal left down
  double m_dMaskHor ;
  double m_dMaskVert ;
  double m_dEtch_hru_Thickness ;
  double m_dEtch_hrd_Thickness ;
  double m_dEtch_vul_Thickness ;
  double m_dEtch_vur_Thickness ;
  double m_dEtch_vdl_Thickness ;
  double m_dEtch_vdr_Thickness ;
  double m_dEtch_hlu_Thickness ;
  double m_dEtch_hld_Thickness ;
  int    m_iNSpots ;

  int    m_iAddedMask ;
  double m_dMeasTime ;
};



class Holoor : public CFilterGadget
{
protected:
  CDuplexConnector * m_pDuplexConnector ;
    // Properties

  FXString m_Notes ;
  FXString m_DataFileName ;
  FXParser2 m_MaskConfigData ;
  MaskData m_MaskData ;

  MeasData m_LastData ;

  cmplx m_CoordsUp ; // Theoretical Upper Mask Center coords
  cmplx m_CoordsDown ; // Theoretical Down Mask Center coords
  cmplx m_StepUp ;
  cmplx m_StepDown ;
  cmplx m_MeasOffset ;
  int   m_iAverageFactor ;
  double  m_dScale ;  // units per pixel, ~0.9 for microns in Holoor's aligner
  double  m_dDiffrCorrection ;
  double  m_dVibrationThres ;
  double  m_dToleranceXY ;
  double  m_dToleranceAngle ;
  int     m_iTimeout_ms ;

  int   m_iNSpotsUp ;
  int   m_iNSpotsDown ;
  
  int  m_iCurrentCross ;
  cmplx m_MaskUp ;
  cmplx m_EtchUp ;
  cmplx m_MaskDown ;
  cmplx m_EtchDown ;
  cmplx m_ShiftUp ;
  cmplx m_ShiftDown ;
  cmplx m_ShiftUpPrev ;
  cmplx m_ShiftDownPrev ;

  cmplx m_MaskUpOrig ;
  cmplx m_EtchUpOrig ;
  cmplx m_MaskDownOrig ;
  cmplx m_EtchDownOrig ;
  cmplx m_ShiftUpOrig ;
  cmplx m_ShiftDownOrig ;

  cmplx m_MaskCenter ;
  cmplx m_EtchCenter ;
  cmplx m_EtchUpAbs ;
  cmplx m_EtchDownAbs ;

  cmplx m_AveMask ;
  cmplx m_AveEtch ;

  int      m_iAccumulated ;
  bool     m_bRotation ;
  int      m_LastWas ; // 0 - unknown, 1 - up , 2 - down
  bool     m_bSave ;
  
  double   m_dLastDataTime ;
  int      m_State ; // 0 - no image, 1 - up , 2 - down
  int      m_PrevState ; // the same values, but for previous measurement
  

private:
  Holoor(void);
  virtual void ShutDown();
public:
  bool ScanProperties(LPCTSTR text, bool& Invalidate);
  bool PrintProperties(FXString& text);
  bool ScanSettings(FXString& text);
  CDataFrame* DoProcessing(const CDataFrame * pDataFrame);
  int GetDuplexCount();
  CDuplexConnector* GetDuplexConnector(int n);
  virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  DECLARE_RUNTIME_GADGET(Holoor);
  bool LoadMaskDescription(LPCTSTR FileName);
  int DecodeElementData(CTextFrame * pTextData);
  int GetNumberInString(FXString& Str, LPCTSTR pFormat, void * pVal , 
    int iIndex , FXSIZE iPos , const char * pAfter = NULL );
  void SaveOrigins()
  {
    m_EtchUpOrig = m_EtchUp ;
    m_MaskUpOrig = m_MaskUp ;
    m_EtchDownOrig = m_EtchDown ;
    m_MaskDownOrig = m_MaskDown ;
    m_ShiftUpOrig = m_ShiftUp ;
    m_ShiftDownOrig = m_ShiftDown ;
  }
  inline bool FindAndDecode( FXString& Label , int iPos ,
    LPCTSTR Pattern , FXString& AsString , int iIndex , 
    LPCTSTR szFormat , double& Res )
  {
    if ( Label.Find(Pattern , iPos ) >= 0 )
    {
      int iRes = GetNumberInString( AsString , szFormat , &Res ,
        iIndex , 0 ,  _T(":") ) ;
      return (iRes != 0) ;
    }
    return false ;
  }

  int GetArrayFromString( FXString& Str, int iIndex , FXSIZE iPos , 
    double * pArray , int iArrLen , const char * pAfter = NULL ) ;
};

