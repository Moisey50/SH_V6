#pragma once
#include <fxfc\fxfc.h>
#include "helpers\UserBaseGadget.h"
#include "gadgets\videoframe.h"
#include "gadgets\QuantityFrame.h"
#include <gadgets\FigureFrame.h>
#include <Math\PlaneGeometry.h>
#include <helpers\FramesHelper.h>
#include <Math\FRegression.h>
#include <Math/FigureProcessing.h>




enum StraightViewMode
{
  SVM_ViewAll = 0 ,
  SVM_InternalSegments ,
  SVM_RemoveOrigin ,
  SVM_ViewFiltered ,
  SVM_FilteredAndData
};

class FindStraights : public UserBaseGadget
{
public:
  FindStraights();
  ~FindStraights();
  
//   void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  int FindLines( const CFigureFrame * pFF , int iGroupIndex ,
    const CRectFrame * pROI = NULL ) ;

  void PropertiesRegistration();
  void ConnectorsRegistration();

  int     m_iMinStraightSegment ;
  double  m_dTolerance ;
  double  m_dDiffFromLine_pix ;
  int     m_iNMaxDeviated ;
  double  m_dFinalTolerance ;
  int     m_iCutOnEdge_perc ;
  int     m_iNLastInternalSegments ;
  StraightViewMode     m_iViewMode ;

  StraightLines m_LastLines ;
  FigFrames m_InternalSegments ;
  
  DECLARE_RUNTIME_GADGET( FindStraights );
};

inline bool ExtractROIName( FXString& Label , FXString& ROIName )
{
  if ( Label.Find( _T( "Contur[" ) ) == 0 )
  {
    int iClosePos = (int)Label.Find( _T( ']' ) ) ;
    FXString ObjectName = Label.Mid( 7 , iClosePos - 8 ) ;
    int iUnderscorePos = ( int )ObjectName.ReverseFind( _T( '_' ) ) ;
    ROIName = ( iUnderscorePos > 0 ) ? ObjectName.Left( iUnderscorePos ) : ObjectName ;
    ROIName.Insert( 0 , _T( "ROI:" ) ) ;
    return true ;
  }
  return false ;
}

