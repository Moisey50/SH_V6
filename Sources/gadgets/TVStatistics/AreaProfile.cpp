// AreaProfile.h.h : Implementation of the CAreaProfile class


#include "StdAfx.h"
#include "AreaProfile.h"
#include <gadgets\vftempl.h>
#include <gadgets\videoframe.h>
#include <gadgets\figureframe.h>
#include <gadgets\containerframe.h>
#include <helpers/FramesHelper.h>


static LPCTSTR ProfNames[ 3 ] = { _T( "Horiz(0)" ) , _T( "Vert(1)" ) , _T( "Both(2)" ) } ;
static LPCTSTR NormalizationNames[ 2 ] = { _T( "NotNormalized(0)" ) , _T( "Normalized(1)" ) } ;
static LPCTSTR OperationNames[ 5 ] = { _T( "NoOperation(0)" ) , _T( "Add(1)" ) ,
_T( "Mult(2)" ) , _T( "Sub(3)" ) , _T( "Div(4)" ) } ;
static LPCTSTR ViewModeNames[ 3 ] = { _T( "All(-2)" ) , _T( "OperResult(-1)" ) , _T( "Profile(>=0)" ) } ;



// Do replace "TODO_Unknown" to necessary gadget folder name in gadgets tree
IMPLEMENT_RUNTIME_GADGET_EX( CAreaProfile , CFilterGadget , "Video.statistics" , TVDB400_PLUGIN_NAME );

CAreaProfile::CAreaProfile( void )
{
  m_pInput = new CInputConnector( vframe );
  m_pOutput = new COutputConnector( transparent );
  m_pDuplexConnector = new CDuplexConnector( this , transparent , transparent ) ;
  m_ProfType = PROFILE_HORIZ ;
  m_Normalization = NOT_NORMALIZED ;
  m_ROI = CRect( -1 , -1 , -1 , -1 ) ;// m_ROI consists of left,top,width,height; do recalculate
  m_RepeatNormalized = cmplx( 0. , 0. ) ;
  m_NSteps = CSize( 0 , 0 ) ;
  m_Operation = OPER_NO ;
  m_iViewMode = 0 ;
  m_iDifferential = 0 ;
  m_bReset = true ;
  Resume();
}

void CAreaProfile::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pDuplexConnector ;
  m_pDuplexConnector = NULL ;
}

CDataFrame* CAreaProfile::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );

  CRect ProcessingRect( 0 , 0 , GetWidth( VideoFrame ) , GetHeight( VideoFrame ) ) ;
  CRect ImageRect( ProcessingRect ) ;

  m_Lock.Lock() ;

  // Check ROI and frame size
  // m_ROI consists of left,top,width,height; do recalculate


  if ( (m_ROI.left >= 0) && (m_ROI.right > 0) )
  {
    if ( ProcessingRect.left < m_ROI.left )
      ProcessingRect.left = m_ROI.left ;
    if ( ProcessingRect.right > m_ROI.right + m_ROI.left )  // it's width
      ProcessingRect.right = m_ROI.right + m_ROI.left ;
  }
  if ( (m_ROI.top >= 0) && (m_ROI.bottom > 0) )
  {
    if ( ProcessingRect.top < m_ROI.top )
      ProcessingRect.top = m_ROI.top ;
    if ( ProcessingRect.bottom > m_ROI.bottom + m_ROI.top )  // it's width
      ProcessingRect.bottom = m_ROI.bottom + m_ROI.top ;
  }
  m_Lock.Unlock() ;
  Profile hProf( GetWidth( VideoFrame ) ) ;
  Profile vProf( GetHeight( VideoFrame ) ) ;
  Profile hProfResult , vProfResult ;
  bool bIsProfOper = (m_Operation != OPER_NO)
    && ((m_NSteps.cx > 1) || (m_NSteps.cy > 1)) ;
  if ( bIsProfOper )
  {
    hProfResult.Realloc( GetWidth( VideoFrame ) ) ;
    vProfResult.Realloc( GetHeight( VideoFrame ) ) ;
  }
  CContainerFrame * pContainer = NULL ;
  CSize Step(
    ROUND( ProcessingRect.right * m_RepeatNormalized.real() ) ,
    ROUND( ProcessingRect.bottom * m_RepeatNormalized.imag() ) ) ;
  CSize NSteps( m_NSteps ) ;
  int iProfCnt = 0 ;
  do
  {
    CRect RectWithLengthAndHeight( ProcessingRect ) ;
    RectWithLengthAndHeight.right -= RectWithLengthAndHeight.left ;
    RectWithLengthAndHeight.bottom -= RectWithLengthAndHeight.top ;
    bool bRes = false ;
    if ( m_iDifferential != 0 )
    {
      bRes = calc_diff_profiles( VideoFrame , m_iDifferential ,
        &hProf , &vProf , &RectWithLengthAndHeight ) ;
    }
    else
    {
      double dAvg = calc_profiles( VideoFrame ,
        &hProf , &vProf , &RectWithLengthAndHeight ) ;
      bRes = (dAvg != 0.) ;
    }
    if ( bRes )
    {
      if ( !pContainer )
      {
        pContainer = CContainerFrame::Create() ;
        pContainer->AddFrame( pDataFrame ) ;
      }
      pContainer->AddFrame( CreateRectFrame( ProcessingRect , 0x0000ff ,
        "ProfileROI" , pDataFrame->GetId() , ProcessingRect.Height() > 20 ? 3 : 1 ) ) ;
      if ( m_ProfType == PROFILE_HORIZ || m_ProfType == PROFILE_BOTH )
      {
        if ( (m_iViewMode == iProfCnt) || (m_iViewMode == -2) )
        {
          CFigureFrame * pFig = CFigureFrame::Create() ;
          double dProfAmpl = hProf.m_dMaxValue - hProf.m_dMinValue ;
          double dMin = hProf.m_dMinValue ;
          for ( int iX = ProcessingRect.left ; iX < ProcessingRect.right - 1 ; iX++ )
          {
            CDPoint NewPoint( (double) iX , hProf.m_pProfData[ iX ] ) ;
            if ( m_Normalization && (dProfAmpl > 1e-6) )
              NewPoint.y = (255. * (NewPoint.y - dMin) / dProfAmpl) ;
            NewPoint.y = 300. - NewPoint.y ;
            pFig->Add( NewPoint ) ;
          }
          if ( m_bReset )
          {
            COutputConnector * pOutput = GetOutputConnector( 0 ) ;
            if ( pOutput  &&  pOutput->IsConnected() )
            {
              CFigureFrame * pFigRemove = CFigureFrame::Create() ;
              FXString Label ;
              Label.Format( "Remove:HProf[%d]" , iProfCnt ) ;
              pFigRemove->SetLabel( Label ) ;
              pFig->ChangeId( pDataFrame->GetId() ) ;
              pFig->SetTime( GetHRTickCount() ) ;
              COutputConnector * pOutput = GetOutputConnector( 0 ) ;
              if ( !pOutput->Put( pFigRemove ) )
                pFigRemove->Release() ;
            }
          }
          FXString Label ;
          Label.Format( "Replace:HProf[%d]" , iProfCnt ) ;
          pFig->SetLabel( Label ) ;
          pFig->ChangeId( pDataFrame->GetId() ) ;
          pFig->SetTime( GetHRTickCount() ) ;
          pFig->Attributes()->Format( "MinMaxes=(%g,%g,%g,%g);" ,
            0. , (double) ProcessingRect.right ,
            hProf.m_dMinValue , hProf.m_dMaxValue ) ;
          pContainer->AddFrame( pFig ) ;
        }
        if ( bIsProfOper )
        {
          if ( iProfCnt == 0 )
            hProfResult.Add( hProf ) ;
          else
          {
            switch ( m_Operation )
            {
            case OPER_NO:
            default: break ;
            case OPER_ADD: hProfResult.Add( hProf ) ; break ;
            case OPER_SUB: hProfResult.Sub( hProf ) ; break ;
            case OPER_MULT: hProfResult.Mult( hProf ) ; break ;
            case OPER_DIV:  hProfResult.Div( hProf ) ; break ;
            }
          }
        }
      }
      if ( m_ProfType == PROFILE_VERT || m_ProfType == PROFILE_BOTH )
      {
        if ( (m_iViewMode == iProfCnt) || (m_iViewMode == -2) )
        {
          CFigureFrame * pFig = CFigureFrame::Create() ;
          double dProfAmpl = vProf.m_dMaxValue - vProf.m_dMinValue ;
          double dMin = vProf.m_dMinValue ;
          double dXOffset = ProcessingRect.left >= 260 ? 5
            : (DWORD)ProcessingRect.right < ( GetWidth( VideoFrame ) - 260 ) ?
            ProcessingRect.right + 5 : ProcessingRect.left + 5 ;
          for ( int iY = m_ROI.top ; iY < ProcessingRect.bottom - 1 ; iY++ )
          {
            CDPoint NewPoint( vProf.m_pProfData[ iY ] ,
              (double) iY ) ;
            if ( m_Normalization && (dProfAmpl > 1e-6) )
              NewPoint.x = (255. * (NewPoint.x - dMin) / dProfAmpl) ;
            NewPoint.x += dXOffset ;
            pFig->Add( NewPoint ) ;
          }
          if ( m_bReset )
          {
            COutputConnector * pOutput = GetOutputConnector( 0 ) ;
            if ( pOutput  &&  pOutput->IsConnected() )
            {
              CFigureFrame * pFigRemove = CFigureFrame::Create() ;
              FXString Label ;
              Label.Format( "Remove:VProf[%d]" , iProfCnt ) ;
              pFigRemove->SetLabel( Label ) ;
              pFigRemove->ChangeId( pDataFrame->GetId() ) ;
              pFigRemove->SetTime( GetHRTickCount() ) ;
              COutputConnector * pOutput = GetOutputConnector( 0 ) ;
              if ( !pOutput->Put( pFigRemove ) )
                pFigRemove->Release() ;
            }
          }
          FXString Label ;
          Label.Format( "Replace:VProf[%d]" , iProfCnt ) ;
          pFig->SetLabel( Label ) ;
          pFig->ChangeId( pDataFrame->GetId() ) ;
          pFig->SetTime( GetHRTickCount() ) ;
          pFig->Attributes()->Format( "MinMaxes=(%g,%g,%g,%g);" ,
            vProf.m_dMinValue , vProf.m_dMaxValue ,
            0. , (double) ProcessingRect.bottom ) ;
          pContainer->AddFrame( pFig ) ;
        }
        if ( bIsProfOper )
        {
          if ( iProfCnt == 0 )
            vProfResult.Add( vProf ) ;
          else
          {
            switch ( m_Operation )
            {
            case OPER_NO:
            default: break ;
            case OPER_ADD: vProfResult.Add( vProf ) ; break ;
            case OPER_SUB: vProfResult.Sub( vProf ) ; break ;
            case OPER_MULT: vProfResult.Mult( vProf ) ; break ;
            case OPER_DIV:  vProfResult.Div( vProf ) ; break ;
            }
          }
        }
      }
      ProcessingRect.left += Step.cx ;
      ProcessingRect.top += Step.cy ;
    }
    iProfCnt++ ;
  } while ( --NSteps.cx > 0 || --NSteps.cy > 0 ) ;
  if ( bIsProfOper && (m_iViewMode < 0) )
  {
    if ( m_ProfType == PROFILE_HORIZ || m_ProfType == PROFILE_BOTH )
    {
      if ( m_bReset )
      {
        COutputConnector * pOutput = GetOutputConnector( 0 ) ;
        if ( pOutput  &&  pOutput->IsConnected() )
        {
          CFigureFrame * pFigRemove = CFigureFrame::Create() ;
          pFigRemove->SetLabel( "Remove:HProfResult" ) ;
          pFigRemove->ChangeId( pDataFrame->GetId() ) ;
          pFigRemove->SetTime( GetHRTickCount() ) ;
          COutputConnector * pOutput = GetOutputConnector( 0 ) ;
          if ( !pOutput->Put( pFigRemove ) )
            pFigRemove->Release() ;
        }
      }
      if ( m_Normalization == NORMALIZED )
        hProfResult.Normalize() ;
      CFigureFrame * pFig = CFigureFrame::Create() ;
      for ( int iX = 0 ; iX < ProcessingRect.right ; iX++ )
      {
        CDPoint NewPoint( (double) iX ,
          hProfResult.m_pProfData[ iX + ProcessingRect.left ] ) ;
        pFig->Add( NewPoint ) ;
      }
      pFig->SetLabel( "Replace:HProfResult" ) ;
      pFig->ChangeId( pDataFrame->GetId() ) ;
      pFig->SetTime( GetHRTickCount() ) ;
      pFig->Attributes()->Format( "MinMaxes=(%g,%g,%g,%g);" ,
        0 , (double) ProcessingRect.right ,
        hProfResult.m_dMinValue , hProfResult.m_dMaxValue ) ;
      pContainer->AddFrame( pFig ) ;
    }
    if ( m_ProfType == PROFILE_VERT || m_ProfType == PROFILE_BOTH )
    {
      if ( m_bReset )
      {
        COutputConnector * pOutput = GetOutputConnector( 0 ) ;
        if ( pOutput  &&  pOutput->IsConnected() )
        {
          CFigureFrame * pFigRemove = CFigureFrame::Create() ;
          pFigRemove->SetLabel( "Remove:VProfResult" ) ;
          pFigRemove->ChangeId( pDataFrame->GetId() ) ;
          pFigRemove->SetTime( GetHRTickCount() ) ;
          COutputConnector * pOutput = GetOutputConnector( 0 ) ;
          if ( !pOutput->Put( pFigRemove ) )
            pFigRemove->Release() ;
        }
      }
      if ( m_Normalization == NORMALIZED )
        vProfResult.Normalize() ;
      CFigureFrame * pFig = CFigureFrame::Create() ;
      int iYLimit = ProcessingRect.top + ProcessingRect.bottom ;
      int iHighValue = iYLimit + ProcessingRect.top ;
      for ( int iY = ProcessingRect.top ; iY < iYLimit ; iY++ )
      {
        CDPoint NewPoint( vProfResult.m_pProfData[ iY ] , (double) (iHighValue - iY) ) ;
        pFig->Add( NewPoint ) ;
      }
      pFig->SetLabel( "Replace:VProfResult" ) ;
      pFig->ChangeId( pDataFrame->GetId() ) ;
      pFig->SetTime( GetHRTickCount() ) ;
      pFig->Attributes()->Format( "MinMaxes=(%g,%g,%g,%g);" ,
        vProfResult.m_dMinValue , vProfResult.m_dMaxValue ,
        0. , (double) ProcessingRect.bottom ) ;
      pContainer->AddFrame( pFig ) ;
    }

  }
  m_bReset = false ;
  if ( pContainer )
    return pContainer ;
  else
  {
    ((CDataFrame*) pDataFrame)->AddRef() ; // const restricts change of references
    return (CDataFrame*) pDataFrame ;      // function returns not const, but nobody will change this frame
  }
  return (pContainer) ? pContainer : (CDataFrame*) pDataFrame ;
}

bool CAreaProfile::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXAutolock al( m_Lock ) ;
  FXPropertyKit pk = text ;
  pk.GetInt( "Dir" , (int&) m_ProfType ) ;
  pk.GetInt( "Norm" , (int&) m_Normalization ) ;
  GetRect( pk , "ROI" , m_ROI ) ;
  pk.GetInt( "Diff" , m_iDifferential ) ;
  double dTemp[ 4 ] ;
  int iNItems = GetArray( pk , "Repeat" , 'g' , 4 , dTemp ) ;
  if ( iNItems )
  {
    if ( iNItems >= 2 )
    {
      m_RepeatNormalized = cmplx( dTemp[ 0 ] , dTemp[ 1 ] ) ;
    }
    if ( iNItems >= 3 )
      m_NSteps.cx = (int) dTemp[ 2 ] ;
    else
      m_NSteps.cx = 0 ;
    if ( iNItems == 4 )
      m_NSteps.cy = (int) dTemp[ 3 ] ;
    else
      m_NSteps.cy = 0 ;
  }
  pk.GetInt( "Operation" , (int&) m_Operation ) ;
  pk.GetInt( "View" , (int&) m_iViewMode ) ;
  m_bReset = true ;
  return true;
}

bool CAreaProfile::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties( text );
  pk.WriteInt( "Dir" , (int) m_ProfType ) ;
  pk.WriteInt( "Norm" , (int) m_Normalization ) ;
  WriteRect( pk , "ROI" , m_ROI ) ;
  pk.WriteInt( "Diff" , m_iDifferential ) ;
  FXString Out ;
  Out.Format( "%g,%g,%d,%d" , m_RepeatNormalized.real() ,
    m_RepeatNormalized.imag() , m_NSteps.cx , m_NSteps.cy ) ;
  pk.WriteString( "Repeat" , Out ) ;
  pk.WriteInt( "Operation" , m_Operation ) ;
  pk.WriteInt( "View" , m_iViewMode ) ;
  text += pk;
  return true;
}

bool CAreaProfile::ScanSettings( FXString& text )
{
  text = _T( "template(ComboBox(Dir(Horiz(0),Vert(1),Both(2)))"
    ",ComboBox(Norm(Abs(0),Normalized(1)))"
    ",EditBox(ROI),Spin(Diff,-20,20)"
    ",EditBox(Repeat)"
    ",ComboBox(Operation(No(0),Add(1),Mult(2),Sub(3),Div(4)))"
    ",Spin(View,-2,10)"
    ")" );
  return true;
}

int CAreaProfile::GetInputsCount()
{
  return CFilterGadget::GetInputsCount();
}

CInputConnector* CAreaProfile::GetInputConnector( int n )
{
  return CFilterGadget::GetInputConnector( n );
}

int CAreaProfile::GetOutputsCount()
{
  return CFilterGadget::GetOutputsCount();
}

COutputConnector* CAreaProfile::GetOutputConnector( int n )
{

  return CFilterGadget::GetOutputConnector( n );
}

int CAreaProfile::GetDuplexCount()
{
  return 1 ;
}

CDuplexConnector* CAreaProfile::GetDuplexConnector( int n )
{
  if ( n == 0 )
    return m_pDuplexConnector ;
  return CFilterGadget::GetDuplexConnector( n );
}

void CAreaProfile::AsyncTransaction(
  CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return;
  CTextFrame* tf = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( tf )
  {
    FXParser pk = (LPCTSTR) tf->GetString();
    FXString cmd;
    FXString param;
    FXSIZE pos = 0;
    pk.GetWord( pos , cmd );

    if ( cmd.CompareNoCase( "list" ) == 0 )
    {
      pk.Empty();
      FXString text ;
      CFilterGadget::PrintProperties( text );
      text += _T( "\r\n" ) ;
      param.Format( _T( "Dir=%s\r\n" ) , ProfNames[ m_ProfType ] ) ;
      text += param ;
      param.Format( _T( "Norm=%s\r\n" ) , NormalizationNames[ m_Normalization ] ) ;
      text += param ;
      param.Format( _T( "ROI=%d,%d,%d,%d\r\n" ) ,
        m_ROI.left , m_ROI.top , m_ROI.right , m_ROI.bottom ) ;
      text += param ;
      param.Format( _T( "Diff=%d (0 - simple, >0 - diff , <0 - abs of diff\r\n" ) ,
        m_iDifferential ) ;
      text += param ;
      param.Format( _T( "Repeat=%g,%g,%d,%d (Norm Steps X Y, N Steps X Y-only one not zero)\r\n" ) ,
        m_RepeatNormalized.real() , m_RepeatNormalized.imag() ,
        m_NSteps.cx , m_NSteps.cy ) ;
      text += param ;
      param.Format( _T( "Operation=%s\r\n" ) , OperationNames[ m_Operation ] ) ;
      text += param ;
      param.Format( _T( "ViewMode=%d [All(-2),OperResult(-1),Profile(>=0)]\r\n" ) , m_iViewMode ) ;
      text += param ;
      pk = text ;
    }
    else if ( (cmd.CompareNoCase( "get" ) == 0) && (pk.GetWord( pos , cmd )) )
    {
    }
    else if ( (cmd.CompareNoCase( "set" ) == 0) && (pk.GetWord( pos , cmd )) && (pk.GetParamString( pos , param )) )
    {

    }
    else if ( cmd.Find( "Rect" ) == 0 )
    {
      CRect rc ;
      int iPos = (int) pk.Find( _T( '=' ) ) ;
      if ( iPos > 0 )
      {
        int iNFields = sscanf_s( (LPCTSTR) pk + iPos + 1 , _T( "%d,%d,%d,%d" ) ,
          &rc.left , &rc.top , &rc.right , &rc.bottom ) ;
        if ( iNFields == 4 )
        {
          if ( rc.left >= 0 )
            rc.right = (rc.right < 0) ? -1 : rc.right - rc.left ;
          if ( rc.top >= 0 )
            rc.bottom = (rc.bottom < 0) ? -1 : rc.bottom - rc.top ;
          m_Lock.Lock() ;
          m_ROI = rc ;
          m_Lock.Unlock() ;
        }
      }
    }
    else
    {
      pk = "List of avalabale commands:\r\nlist - return list of properties\r\n"
        "get <item name> - return current value of item\r\n"
        "set <item name>(<value>) - change an item\r\n";
    }
    CTextFrame* retV = CTextFrame::Create( pk );
    retV->ChangeId( NOSYNC_FRAME );
    if ( !m_pDuplexConnector->Put( retV ) )
      retV->RELEASE( retV );
  }
  pParamFrame->Release( pParamFrame );
}
