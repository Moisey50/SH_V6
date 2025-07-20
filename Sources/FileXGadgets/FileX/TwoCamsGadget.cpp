#include "stdafx.h"
#include "TwoCamsGadget.h"
#include "fxfc/FXRegistry.h"

USER_FILTER_RUNTIME_GADGET( TwoChans , "Video.FileX_Specific" );

TwoChans::TwoChans()
{
  m_WorkingMode = WM_Green_Measure ;
  m_OutputMode = modeReplace ;
  m_BGMforBasePoint = BGM_Corners ;
  m_dScale_um_per_pix = 3.577818 ;
  m_dTolerance_um = 7. ;
  m_iZOffset_um = m_iYOffset_um = m_iXOffset_um = 0 ;
  m_dX_um = m_dY_um = m_dZ_um = 0. ;
  m_dLineAngle = 40. ;
  m_LastROI = CRect( 0 , 0 , 640 , 480 ) ;

  m_dWedgeAngleToElectrode_deg = 45.0 ;
//  m_dElectrodeToCornerHorizDist_um = 27. * 2.54 ; // tenth * 2.54 micron per tenth
  m_dElectrodeToCornerHorizDist_tenth = 150 / 2.54 ; // 2.54 micron per tenth
  m_iShiftFromCorner_Tx10 = ROUND( 10. * m_dElectrodeToCornerHorizDist_tenth ) ;

  m_iVertLinesDist_tenth = 15 ;
  FXRegistry Reg( "TheFileX\\Micropoint" );
  m_ApplicationMode = ( AppMode ) Reg.GetRegiInt(
    "MPP_Green" , "AppMode(0-Green,1-Holes)" , 0 ) ;
  m_GadgetMode = GM_FRONT ;

  init() ;
}


TwoChans::~TwoChans()
{}

static const char * pViewMode = "Unknown;Free;Lock;Manual;" ;
static const char * pGadgetMode = "WedgeSide;WedgeFront;" ;
static const char * pSideCalcAlgorithm = "ByLowerPoint;ByOrtho;" ;

void TwoChans::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "WorkingMode" ) , ( int * ) &m_WorkingMode ,
    SProperty::Int , pViewMode ) ;
  addProperty( SProperty::COMBO , _T( "GadgetMode" ) , ( int * ) &m_GadgetMode ,
    SProperty::Int , pGadgetMode ) ;
  addProperty( SProperty::EDITBOX , "Scale_um/pix" , &m_dScale_um_per_pix , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "Tolerance_um" , &m_dTolerance_um , SProperty::Double ) ;
  switch ( m_ApplicationMode )
  {
  case AppMode_Green_Machine:
    {
      addProperty( SProperty::SPIN , _T( "HLinePos_pix" ) ,
        &m_iHorLinePosition_pix , SProperty::Long , 0 , 1900 ) ;
      addProperty( SProperty::SPIN , "ZOffset_um" ,
        &m_iZOffset_um , SProperty::Long , -500 , 500 ) ;
      addProperty( SProperty::SPIN , "YOffset_um" ,
        &m_iYOffset_um , SProperty::Long , -300 , 300 ) ;
      addProperty( SProperty::SPIN , "XOffset_um" ,
        &m_iXOffset_um , SProperty::Long , -100 , 100 ) ;
      addProperty( SProperty::EDITBOX , "LastLocked" ,
        &m_cLastLocked , SProperty::Cmplx ) ;
      addProperty( SProperty::EDITBOX , "LineAngle_deg" , &m_dLineAngle , SProperty::Double ) ;
    }
    break;
  case AppMode_Holes_Erosion:
    {
      addProperty( SProperty::COMBO , _T( "Algorithm" ) , ( int * ) &m_SideAlgorithm ,
        SProperty::Int , pSideCalcAlgorithm ) ;
      addProperty( SProperty::EDITBOX , "WedgeToElectrAng_deg" , 
        &m_dWedgeAngleToElectrode_deg , SProperty::Double ) ;
//       addProperty( SProperty::EDITBOX , "ShiftFromCorner_um" ,
//         &m_dElectrodeToCornerHorizDist_um , SProperty::Double ) ;
      addProperty( SProperty::SPIN , "ShiftFromCorner_Tx10" ,
        &m_iShiftFromCorner_Tx10 , SProperty::Int , 0 , 1500 ) ;
//       addProperty( SProperty::EDITBOX , "PartK" ,
//         &m_dK , SProperty::Double ) ;
      addProperty( SProperty::SPIN , "VertLinesDist_tenth" ,
        &m_iVertLinesDist_tenth , SProperty::Int , 0 , 100 ) ;
      addProperty( SProperty::SPIN , "MoveFrom_pix" ,
        &m_iMoveFragmentFrom_pix , SProperty::Int , -1 , 1900 ) ;
      addProperty( SProperty::SPIN , "MoveTo_pix" ,
        &m_iMoveFragmentTo_pix , SProperty::Int , 0 , 1900 ) ;
      addProperty( SProperty::SPIN , "ZoneHeight_pix" ,
        &m_iFragmentHeight_pix , SProperty::Int , 10 , 300 ) ;
      addProperty( SProperty::SPIN , _T( "HLinePos_pix" ) ,
        &m_iHorLinePosition_pix , SProperty::Long , 0 , 1900 );
      addProperty( SProperty::SPIN , _T( "VLinePos_pix" ) ,
        &m_iVertLinePosition_pix , SProperty::Long , 0 , 1200 );
    }
    break;
  default:
    break;
  }
};

void TwoChans::ConnectorsRegistration()
{
  addInputConnector( transparent , "FigureInput" );
  addOutputConnector( transparent , "OutputView" );
  addOutputConnector( text , "Diagnostics" ) ;
};

void TwoChans::PrintAdditionalData( FXString& Text )
{
  FXString AdditionalData ;
  switch ( m_ApplicationMode )
  {
  case AppMode_Green_Machine:
    {
      switch ( m_GadgetMode )
      {
      case GM_SIDE:
        AdditionalData.Format( "dY_um=%.2f;dZ_um=%.2f;LockedCoords=(%.2f,%.2f);" ,
          m_dY_um , m_dZ_um , m_cLastLocked.real() , m_cLastLocked.imag() ) ;
        break ;
      case GM_FRONT:
        AdditionalData.Format( "dX_um=%.2f;dZ_um=%.2f;LockedCoords=(%.2f,%.2f);" ,
          m_dX_um , m_dZ_um , m_cLastLocked.real() , m_cLastLocked.imag() ) ;
        break ;
      }
    }
    break ;
  case AppMode_Holes_Erosion:
    break ;
  }
  Text += AdditionalData ;
} ;


CDataFrame* TwoChans::DoProcessing( const CDataFrame* pDataFrame )
{
  double dStart = GetHRTickCount() ;
  CContainerFrame * pOut = CContainerFrame::Create() ;
  pOut->CopyAttributes( pDataFrame ) ;

  pOut->AddFrame( pDataFrame ) ;

  FXIntArray LineCnt , NSegments ;
  FXDblArray Times ;
  //#ifdef _DEBUG
  FXString DiagInfo ;
  //#endif
  const CVideoFrame * pImage = pDataFrame->GetVideoFrame() ;
  if ( pImage )
  {
    m_LastROI.right = GetWidth( pImage ) ;
    m_LastROI.bottom = GetHeight( pImage ) ;
  }
  m_cLastROICent_pix = cmplx( ( double ) m_LastROI.CenterPoint().x , ( double ) m_LastROI.CenterPoint().y ) ;
 
  switch ( m_ApplicationMode )
  {
  case AppMode_Green_Machine: ProcessForGreenMachine( pDataFrame , pOut ) ; break ;
  case AppMode_Holes_Erosion: ProcessForHolesInWedges( pDataFrame , pOut ) ; break ;
  }
  return pOut ;
}
