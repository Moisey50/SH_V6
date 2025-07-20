#include "stdafx.h"
#include "FindStraights.h"

#define THIS_MODULENAME "FindStraights"

USER_FILTER_RUNTIME_GADGET( FindStraights , "Video.recognition" );

FindStraights::FindStraights()
{
  m_iMinStraightSegment = 10 ;
  m_dTolerance = 40.0 ; //mrad
  m_dFinalTolerance = 5.0 ; //mrad
  m_dDiffFromLine_pix = 3 ; 
  m_iNMaxDeviated = 5 ;
  m_iViewMode = SVM_ViewAll;
  m_OutputMode = modeReplace ;
  init() ;
}


FindStraights::~FindStraights()
{
}

static const char * pViewMode = "View All;View Internal;Remove Origin;View Filtered;Filtered And Data;" ;
void FindStraights::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "RemoveOrigin" ) , ( int * )&m_iViewMode ,
    SProperty::Int , pViewMode ) ;
  addProperty( SProperty::SPIN , _T( "MinLength_pix" ) ,
    &m_iMinStraightSegment , SProperty::Long , 3 , 3000 ) ;
  addProperty( SProperty::EDITBOX , _T( "Tolerance_mrad" ) ,
    &m_dTolerance , SProperty::Double );
  addProperty( SProperty::EDITBOX , _T( "DiffFromLine_pix" ) ,
    &m_dDiffFromLine_pix , SProperty::Double );
  addProperty( SProperty::SPIN , _T( "NMaxDeviated" ) ,
    &m_iNMaxDeviated , SProperty::Long , 1 , 1000 ) ;
  addProperty( SProperty::SPIN , _T( "CutEdges_perc" ) ,
    &m_iCutOnEdge_perc , SProperty::Long , 0 , 20 ) ;
};

void FindStraights::ConnectorsRegistration()
{
  addInputConnector( transparent , "FigureInput" );
  addOutputConnector( transparent , "Lines+Origin" );
  addOutputConnector( text , "Diagnostics" ) ;
};

int FindStraights::FindLines( const CFigureFrame * pFF , int iGroupIndex ,
  const CRectFrame * pROI )
{
  int iNOld = (int) m_LastLines.GetCount() ;
  int iNStraights = FindStraightSegments( pFF , iGroupIndex , pROI ,
    m_LastLines , m_iMinStraightSegment , m_dDiffFromLine_pix , m_iNMaxDeviated ,
    ((int) m_iViewMode == (int) SVM_InternalSegments) ? &m_InternalSegments : NULL , &m_iNLastInternalSegments ) ;
  return ( int )m_LastLines.GetCount() - iNOld ;
}



CDataFrame* FindStraights::DoProcessing( const CDataFrame* pDataFrame )
{
  double dStart = GetHRTickCount() ;
  ASSERT( m_InternalSegments.GetCount() == 0 ) ;
  const CDataFrame * pDataContainer = pDataFrame->GetDataFrame( "ResultsRes" ) ;
  CContainerFrame * pOut = CContainerFrame::Create() ;
  FXIntArray LineCnt , NSegments ;
  FXDblArray Times ;
  if ( m_iViewMode == SVM_ViewAll )
    pOut->AddFrame( pDataFrame );
  else
  {
    const CVideoFrame * pVideoFrame = pDataFrame->GetVideoFrame() ;
    if ( pVideoFrame )
      pOut->AddFrame( pVideoFrame ) ;
  }

//#ifdef _DEBUG
  FXString DiagInfo ;
//#endif

  try
  {
    if ( pDataContainer )
    {
      FXString ROIName ;
      int iNLines = 0 ;
      const CDataFrame * pROIContainer = pDataFrame->GetDataFrame( "ROI" ) ;
      m_LastLines.RemoveAll() ;
      int iGroupIndex = 0 ;
      CFramesIterator* Iterator = pDataContainer->CreateFramesIterator( figure );
      if ( Iterator != NULL )
      {
        const CFigureFrame * pFigure = NULL ;
        while ( pFigure = (const CFigureFrame *) Iterator->Next() )
        {
          FXString Label = pFigure->GetLabel() ;

          if ( ExtractROIName( Label , ROIName ) )
          {
            const CRectFrame * pROI = pROIContainer->GetRectFrame( ROIName ) ;
            double dStart = GetHRTickCount() ;
            iNLines += FindLines( pFigure , iGroupIndex++ , pROI ) ;
            //           LineCnt.Add( iNLines ) ;
            //           Times.Add( GetHRTickCount() - dStart ) ;
            //#ifdef _DEBUG
            FXString DiagAdd ;
            DiagAdd.Format( "Fig %s(%d points): Nsegm=%d nLines=%d T=%.3f\n" ,
              pFigure->GetLabel() , pFigure->GetCount() , m_iNLastInternalSegments ,
              iNLines , GetHRTickCount() - dStart ) ;
            DiagInfo += DiagAdd ;
            //#endif
          }
        }
        delete Iterator ;
      }
      else
      {
        const CFigureFrame * pFigure = pDataContainer->GetFigureFrame() ;
        if ( pFigure )
        {
          FXString Label = pFigure->GetLabel() ;

          if ( ExtractROIName( Label , ROIName ) )
          {
            const CRectFrame * pROI = pROIContainer->GetRectFrame( ROIName ) ;

            double dStart = GetHRTickCount() ;
            iNLines += FindLines( pFigure , iGroupIndex++ , pROI ) ;
            LineCnt.Add( iNLines ) ;
            Times.Add( GetHRTickCount() - dStart ) ;
          }
        }
      }
      for ( int i = 0 ; i < m_InternalSegments.GetCount() ; i++ )
        pOut->AddFrame( m_InternalSegments[ i ] ) ;
      if ( m_LastLines.GetCount() )
      {
        for ( int i = 0 ; i < m_LastLines.GetCount() ; i++ )
        {
          StraightLine& SLine = m_LastLines.GetAt( i ) ;
          cmplx LinePt1 = SLine.GetPtOnLine( SLine.m_Begin ) ;
          cmplx LinePt2 = SLine.GetPtOnLine( SLine.m_End ) ;
          FXString Label ;
          Label.Format( "Straight_%d" , i ) ;
          CFigureFrame * pLine = CreateLineFrame(
            LinePt1 , LinePt2 ,
            SLine.m_Color , Label , i , pDataFrame->GetTime() ) ;
          pOut->AddFrame( pLine ) ;
          if ( (int) m_iViewMode < (int) SVM_ViewFiltered )
          {
            pLine = CreateLineFrame(
              SLine.m_Begin , SLine.m_End ,
              0xffff00 , NULL , i , pDataFrame->GetTime() ) ;
            pOut->AddFrame( pLine ) ;
          }
          if ( (int) m_iViewMode < (int) SVM_FilteredAndData )
          {
            CFigureFrame * pPt = CreatePtFrame( SLine.m_Begin ,
              pDataFrame->GetTime() , _T( "0x00ff00" ) ) ;
            pOut->AddFrame( pPt ) ;
            pPt = CreatePtFrame( SLine.m_End ,
              pDataFrame->GetTime() , _T( "0x00ffff" ) ) ;
            pOut->AddFrame( pPt ) ;
          }
          else
          {
            cmplx Center = (LinePt1 + LinePt2) * 0.5 ;
            CFigureFrame * pPt = CreatePtFrame( Center ,
              pDataFrame->GetTime() , SLine.m_Color , NULL , i ) ;
            cmplx Vect = conj( LinePt2 - LinePt1 );
            double dAngle = arg( Vect ) ;
            cmplx TextPt = Center + ((IsNearVert( dAngle )) ?
              cmplx( 10. , 0. ) : cmplx( 0. , -10. )) ;
            dAngle = RadToDeg( dAngle ) ;
            FXString AngleAsText ;
            AngleAsText.Format( "%.4f" , dAngle ) ;
            CTextFrame * pAngleText = CreateTextFrame( TextPt , AngleAsText , 0xc0ffc0 ) ;
            pOut->AddFrame( pPt ) ;
            pOut->AddFrame( pAngleText ) ;
          }
        }
      }
    }
    //#ifdef _DEBUG
    FXString DiagAdd ;
    DiagAdd.Format( "%d Full Time %.3f Nout=%d\n" ,
      pDataFrame->GetId() , GetHRTickCount() - dStart , pOut->GetFramesCount() ) ;
    DiagInfo += DiagAdd ;
    PutFrame( GetOutputConnector( 1 ) ,
      CreateTextFrame( DiagInfo , "DiagStraight" , pDataFrame->GetId() ) ) ;
    //#endif

  }
  catch ( CException* e )
  {
    char perrmsg[ 4096 ] = { 0 };
    e->GetErrorMessage( perrmsg , 4096 );
    SENDERR( "Exception: %s" , perrmsg ) ;
  }

  m_InternalSegments.RemoveAll() ;
  return pOut ;
}