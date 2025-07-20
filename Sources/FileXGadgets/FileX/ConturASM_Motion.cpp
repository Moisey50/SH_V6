// ConturAsm.h : Motion methods implementation  of the ConturAsm class


#include "StdAfx.h"
#include "ConturAsm.h"
#include <imageproc\clusters\segmentation.h>
#include <imageproc\utilities.h>
#include <imageproc/simpleip.h>
#include <gadgets\vftempl.h>
#include <helpers\propertykitEx.h>
#include <helpers\FramesHelper.h>
#include "ConturData.h"
#include <files\imgfiles.h>

#define THIS_MODULENAME "ConturAsm"

FXString FormSpeedInUnitsFrom_um_persecond(double dSpeed_um_per_sec)
{
  double dUnits = dSpeed_um_per_sec * 64. * 200. / 609.6;
  FXString SpeedMsg;
  SpeedMsg.Format("Set_Target_Speed=%d", ROUND(dUnits));
  return SpeedMsg;
}


int ConturAsm::OrderAbsMotion( CCoordinate& Target )
{
  FXString NotCorrectPosWarning;
  if ( Target.m_x < 0. || Target.m_x > 203200.
    || Target.m_y < 0. || Target.m_y > 101600.
    || Target.m_z < 0. || Target.m_z > 101600. )
  {
    NotCorrectPosWarning.Format( "Target Coordinate [%.1f,%.1f,%.1f] is out of space "
      "[0.,203200.][0.,101600.][0.,101600.]. Operation STOPPED!  HOMING IS STARTED!!!" ,
      Target.m_x , Target.m_y , Target.m_z );
  }
  else
  {
    CDPoint XYTarget( Target.m_x , Target.m_y );
    m_cCurrentTarget = m_cRobotPos;
    if ( m_FreeZone.IsPtInside( XYTarget ) )
    {
      if ( m_FreeZone.IsPtInside( CmplxToCDPoint( m_cRobotPos ) ) )
      { // no restrictions for motion, move both axis
        m_cCurrentTarget = CDP2Cmplx( XYTarget );
        m_MotionSequence = MS_Both;
        m_cFinalTarget = 0.;
      }
      else if ( m_PlacementZone.IsPtInside( CmplxToCDPoint( m_cRobotPos ) ) )
      { // Move X from placement zone and after that move Y
        if ( m_PlacementZone.top <= Target.m_y
          && Target.m_y <= m_PlacementZone.bottom )
        {
          m_cCurrentTarget = CDP2Cmplx( XYTarget );
          m_MotionSequence = MS_Both;
          m_cFinalTarget = 0.;
        }
        else
        {
          m_MotionSequence = MS_YSecond;
          m_cFinalTarget = cmplx( XYTarget.x , XYTarget.y );
          m_cCurrentTarget._Val[ _RE ] = XYTarget.x;
        }
      }
      else
      {
        NotCorrectPosWarning.Format( "XY Target Coordinate [%.1f,%.1f] is out of allowed space. "
          "Operation STOPPED!  HOMING IS STARTED!!!" ,
          Target.m_x , Target.m_y );
      }
    }
    else if ( m_PlacementZone.IsPtInside( XYTarget ) )
    {  // Target in placement zone
      if ( m_FreeZone.IsPtInside( CmplxToCDPoint( m_cRobotPos ) ) )
      {  // move Y to proper position for placement zone entry
         // and only after that move X inside placement zone
         // BUT before check, may be Y is inside allowed range
        if ( m_PlacementZone.top <= m_cRobotPos.imag()
          && m_cRobotPos.imag() <= m_PlacementZone.bottom )
        {
          m_cCurrentTarget = CDP2Cmplx( XYTarget );
          m_MotionSequence = MS_Both;
          m_cFinalTarget = 0.;
        }
        else
        {
          m_MotionSequence = MS_XSecond;
          m_cFinalTarget = cmplx( XYTarget.x , XYTarget.y );
          m_cCurrentTarget._Val[ _IM ] = XYTarget.y;
        }
      }
      else if ( m_PlacementZone.IsPtInside( CmplxToCDPoint( m_cRobotPos ) ) )
      { // Initial and target positions are inside placement zone
        // possible to do free motion (there are several mm on Y)
        m_cCurrentTarget = CDP2Cmplx( XYTarget );
        m_MotionSequence = MS_Both;
        m_cFinalTarget = 0.;
      }
      else
      {
        NotCorrectPosWarning.Format( "Current motion XY position [%.1f,%.1f] "
          "is not in allowed space. "
          "Operation STOPPED!  HOMING IS STARTED!!!" ,
          Target.m_x , Target.m_y );
      }
    }
    else
    {
      NotCorrectPosWarning.Format( "Target XY position [%.1f,%.1f] "
        "is not in free or placement zone. "
        "Operation STOPPED!  HOMING IS STARTED!!!" ,
        Target.m_x , Target.m_y );
    }
  }
  if ( !NotCorrectPosWarning.IsEmpty() )
  {
    SEND_GADGET_ERR( NotCorrectPosWarning );
    if ( !IsInactive() )
    {
      CTextFrame * pEndOfScan = CreateTextFrame(
        "Motion ERROR. Operation Stopped. HOME initialized" ,
        "Result" );
      PutFrame( m_pOutput , pEndOfScan );
    }
    CTextFrame * pHomeZCommand = CreateTextFrame( "Home=1;" , "SendToMotion" );
    if ( PutFrame( m_pLightControl , pHomeZCommand ) )
    {
      while ( m_DelayedOperations.size() )
        m_DelayedOperations.pop();
      m_HomeSetMask |= MotionMaskZ;
      m_MotionMask |= MotionMaskZ;
      m_ProcessingStage = Stage_HomeZ;
    }
    m_MotionLogger.AddMsg((LPCTSTR)NotCorrectPosWarning, "  ERROR");
    return 0;
  }
  cmplx MotionVect = m_cCurrentTarget - m_cRobotPos ;
  bool bXChanged = false ;
  if ( fabs( MotionVect.real() ) > 0.5 )  // half of micron
  {
    FXString XMotionMsg;
    XMotionMsg.Format( "Move_Absolute=%d" , ROUND( m_cCurrentTarget.real() ) );
    CTextFrame * pXMove = CreateTextFrame( (LPCTSTR) XMotionMsg , (LPCTSTR) NULL );
    if ( !PutFrame( m_pXMotion , pXMove ) )
    {
      SEND_GADGET_ERR( "Can't do X motion for %7.1fum" ,
        m_cCurrentTarget.real() );
    }
    else
    {
      if ( (m_MotionMask & MotionMaskX) == 0 )
      {
        m_iNWaitsForMotionAnswers++;
        m_MotionMask |= MotionMaskX ;
      }
      bXChanged = true ;
      if ( m_ProcessingStage != Stage_WaitForMotionEnd )
        m_StageBeforeMotion = m_ProcessingStage ;
      //else
      //  ASSERT( 0 ) ;
      SetProcessingStage( Stage_WaitForMotionEnd , true ) ;
    }
  }
  else
    m_MotionMask &= ~(MotionMaskX) ;
  if ( fabs( MotionVect.imag() ) > 0.5 )
  {
    FXString YMotionMsg;
    YMotionMsg.Format( "Move_Absolute=%d" , ROUND( m_cCurrentTarget.imag() ) );
    CTextFrame * pYMove = CreateTextFrame( (LPCTSTR) YMotionMsg , (LPCTSTR) NULL );
    if ( !PutFrame( m_pYMotion , pYMove ) )
    {
      SEND_GADGET_ERR( "Can't do Y motion for %7.1fum" ,
        m_cCurrentTarget.imag() );
    }
    else
    {
      if ( (m_MotionMask & MotionMaskY) == 0 )
      {
        m_iNWaitsForMotionAnswers++;
        m_MotionMask |= MotionMaskY ;
      }
      if ( !bXChanged )
      {
        if ( m_ProcessingStage != Stage_WaitForMotionEnd )
          m_StageBeforeMotion = m_ProcessingStage ;
        //else
        //  ASSERT( 0 ) ;
        SetProcessingStage( Stage_WaitForMotionEnd , true ) ;
      }
    }
  }
  else
    m_MotionMask &= ~(MotionMaskY) ;
  OrderAbsZMotion( Target.m_z ) ;

  FXRegistry Reg("TheFileX\\ConturASM") ;
  m_bDoMotionLog = Reg.GetRegiInt("Parameters", "DoMotionLog", 1);
  if (m_bDoMotionLog)
  {
    FXString LogMsg;
    LogMsg.Format("Target(%.1f,%.1f,%.1f) Current((%.1f,%.1f,%.1f) Mask=%d",
      m_cCurrentTarget.real(), m_cCurrentTarget.imag(), m_dZTarget,
      m_CurrentPos.m_x, m_CurrentPos.m_y, m_CurrentPos.m_z,
      m_MotionMask);

    m_MotionLogger.AddMsg((LPCTSTR)LogMsg, "Abs Motion: ");
  }
  return 1;
}

int ConturAsm::OrderAbsMotion( cmplx& Target )
{
  CCoordinate Targ3d( Target.real() , Target.imag() , m_CurrentPos.m_z ) ;
  return OrderAbsMotion( Targ3d ) ;
}


int ConturAsm::OrderRelZMotion( double dDist , bool bReport , double dCheckDeviation )
{
  ASSERT( abs( dDist ) >= 1. ) ;
  double dTargetPos = m_CurrentPos.m_z + dDist ;
  ASSERT( 0. <= dTargetPos && dTargetPos <= 101500. ) ;
  if ( fabs( dDist ) >= 1. )
  {
    if ( dCheckDeviation != 0. )
    {
      if ( abs( dDist ) < dCheckDeviation )
      {
        ASSERT( (m_CurrentPos.m_z + dDist) - m_dMaxZWithGood <= dCheckDeviation ) ;
        ASSERT( m_dMinZWithGood - (m_CurrentPos.m_z + dDist) <= dCheckDeviation ) ;
      }
    }
    FXString ZMotionMsg;
    ZMotionMsg.Format( "Move_Relative=%d" , ROUNDPM( dDist ) );
    CTextFrame * pZMove = CreateTextFrame( (LPCTSTR) ZMotionMsg , _T( "SendToMotion" ) );
    if ( !PutFrame( m_pLightControl , pZMove ) )
    {
      SEND_GADGET_ERR( "Can't do Z motion for %dum" , ROUND( dDist ) );
    }
    else
    {
      if ( (m_MotionMask & MotionMaskZ) == 0 )
      {
        m_iNWaitsForMotionAnswers++;
        m_MotionMask |= MotionMaskZ ;
      }
      if ( m_ProcessingStage != Stage_WaitForMotionEnd )
        m_StageBeforeMotion = m_ProcessingStage ;
      //else
      //  ASSERT( 0 ) ;
      m_dLastdZ = dDist ;
      m_dZTarget = m_CurrentPos.m_z + dDist ;
      m_dLastMotionOrderTime = GetHRTickCount() ;
      SetProcessingStage( Stage_WaitForMotionEnd , bReport ) ;
    }
  }
  return 1;
}
int ConturAsm::OrderAbsZMotion( double dTarget , bool bReport )
{
  double dDist = dTarget - m_CurrentPos.m_z ;
  if ( fabs( dDist ) >= 1. ) // 1 micron
  {
    FXString ZMotionMsg;
    ZMotionMsg.Format( "Move_Absolute=%d" , ROUND( dTarget ) );
    CTextFrame * pZMove = CreateTextFrame(
      (LPCTSTR) ZMotionMsg , _T( "SendToMotion" ) );
    if ( !PutFrame( m_pLightControl , pZMove ) )
    {
      SEND_GADGET_ERR( "Can't do Z motion for %dum" , ROUND( dTarget ) );
    }
    else
    {
      if ( (m_MotionMask & MotionMaskZ) == 0 )
      {
        m_iNWaitsForMotionAnswers++;
        m_MotionMask |= MotionMaskZ ;
      }
      if ( m_ProcessingStage != Stage_WaitForMotionEnd )
        m_StageBeforeMotion = m_ProcessingStage ;
      m_dZTarget = dTarget ;
      m_dLastMotionOrderTime = GetHRTickCount() ;
      //else
      //  ASSERT( 0 ) ;
      SetProcessingStage( Stage_WaitForMotionEnd , bReport ) ;
    }
  }
  return 1;
}


int ConturAsm::OrderRelMotion( CCoordinate& Direction )
{
  int iXStep = ROUND( Direction.m_x );
  int iYStep = ROUND( Direction.m_y );
  bool bXChanged = false ;
  if ( iXStep != 0 )
  {
    FXString XMotionMsg;
    XMotionMsg.Format( "Move_Relative=%d" , iXStep );
    CTextFrame * pXMove = CreateTextFrame( (LPCTSTR) XMotionMsg , (LPCTSTR) NULL );
    if ( !PutFrame( m_pXMotion , pXMove ) )
    {
      SEND_GADGET_ERR( "Can't do X motion for %dum" , iXStep );
    }
    else
    {
      if ( (m_MotionMask & MotionMaskX) == 0 )
      {
        m_iNWaitsForMotionAnswers++;
        m_MotionMask |= MotionMaskX ;
      }
      if ( m_ProcessingStage != Stage_WaitForMotionEnd )
        m_StageBeforeMotion = m_ProcessingStage ;
      m_cCurrentTarget._Val[ _RE ] += iXStep ;
      m_dLastMotionOrderTime = GetHRTickCount() ;
      //else
      //  ASSERT( 0 ) ;
      SetProcessingStage( Stage_WaitForMotionEnd , true ) ;
      bXChanged = true ;
    }
  }
  if ( iYStep != 0 )
  {
    FXString YMotionMsg;
    YMotionMsg.Format( "Move_Relative=%d" , iYStep );
    CTextFrame * pYMove = CreateTextFrame( (LPCTSTR) YMotionMsg , (LPCTSTR) NULL );
    if ( !PutFrame( m_pYMotion , pYMove ) )
    {
      SEND_GADGET_ERR( "Can't do Y motion for %dum" , iXStep );
    }
    else
    {
      if ( (m_MotionMask & MotionMaskY) == 0 )
      {
        m_iNWaitsForMotionAnswers++;
        m_MotionMask |= MotionMaskY ;
      }
      if ( m_ProcessingStage != Stage_WaitForMotionEnd )
        m_StageBeforeMotion = m_ProcessingStage ;
      m_cCurrentTarget._Val[ _IM ] += iYStep ;
      m_dLastMotionOrderTime = GetHRTickCount() ;

      SetProcessingStage( Stage_WaitForMotionEnd , true ) ;
    }
  }
  return 1;
}

int ConturAsm::OrderRelMotion( double& dDX , double& dDY , double& dDZ )
{
  double dTargetX = m_CurrentPos.m_x + dDX ;
  double dTargetY = m_CurrentPos.m_y + dDY ;
  double dTargetZ = m_CurrentPos.m_z + dDZ ;

  if ( dTargetX < 0. || dTargetX > 203200. )
  {
    SEND_GADGET_ERR( "OrderRelMotion: TargetX=%.0f(dX=%.0f) "
      "is out of range [0.,203000.]" , dTargetX , dDX ) ;
    dDX = 0. ;
  }
  if ( dTargetY < 0. || dTargetY > 101600. )
  {
    SEND_GADGET_ERR( "OrderRelMotion: TargetY=%.0f(dY=%.0f) "
      "is out of range [0.,101600.]" , dTargetY , dDY ) ;
    dDY = 0. ;
  }
  if ( dTargetZ < 0. || dTargetZ > 101600. )
  {
    SEND_GADGET_ERR( "OrderRelMotion: TargetZ=%.0f(dZ=%.0f) "
      "is out of range [0.,101600.]" , dTargetZ , dDZ ) ;
    dDZ = 0. ;
  }

  int iXStep = ROUND( dDX );
  int iYStep = ROUND( dDY );
  int iZStep = ROUND( dDZ ) ;
  bool bMoved = false ;
  if ( iXStep != 0 )
  {
    FXString XMotionMsg;
    XMotionMsg.Format( "Move_Relative=%d" , iXStep );
    CTextFrame * pXMove = CreateTextFrame( (LPCTSTR) XMotionMsg , (LPCTSTR) NULL );
    if ( !PutFrame( m_pXMotion , pXMove ) )
    {
      SEND_GADGET_ERR( "Can't do X motion for %dum" , iXStep );
    }
    else
    {
      if ( (m_MotionMask & MotionMaskX) == 0 )
      {
        m_iNWaitsForMotionAnswers++;
        m_MotionMask |= MotionMaskX ;
      }
      if ( m_ProcessingStage != Stage_WaitForMotionEnd )
        m_StageBeforeMotion = m_ProcessingStage ;
      m_cCurrentTarget._Val[ _RE ] = dTargetX ;
      bMoved = true ;
    }
  }
  if ( iYStep != 0 )
  {
    FXString YMotionMsg;
    YMotionMsg.Format( "Move_Relative=%d" , iYStep );
    CTextFrame * pYMove = CreateTextFrame( (LPCTSTR) YMotionMsg , (LPCTSTR) NULL );
    if ( !PutFrame( m_pYMotion , pYMove ) )
    {
      SEND_GADGET_ERR( "Can't do Y motion for %dum" , iXStep );
    }
    else
    {
      if ( (m_MotionMask & MotionMaskY) == 0 )
      {
        m_iNWaitsForMotionAnswers++;
        m_MotionMask |= MotionMaskY ;
      }
      if ( m_ProcessingStage != Stage_WaitForMotionEnd )
        m_StageBeforeMotion = m_ProcessingStage ;
      m_cCurrentTarget._Val[ _IM ] = dTargetY  ;
      bMoved = true ;
    }
  }
  if ( iZStep != 0 )
  {
    OrderRelZMotion( dDZ ) ;
    bMoved = true ;
  }
  if ( bMoved )
  {
    m_dLastMotionOrderTime = GetHRTickCount() ;
    SetProcessingStage( Stage_WaitForMotionEnd ) ;
    m_bReportAfterMotion = true ;
    m_StringAfterMotion = _T( "Relative Motion Finished" ) ;
  }
  return 1;
}

bool ConturAsm::IsOnTargetPosition( double dTolerance )
{
  return (abs( m_CurrentPos.m_x - m_cCurrentTarget.real() ) <= dTolerance
    && abs( m_CurrentPos.m_y - m_cCurrentTarget.imag() ) <= dTolerance
    && abs( m_CurrentPos.m_z - m_dZTarget ) <= dTolerance) ;
}

void  ConturAsm::SetMotionSpeed(double dSpeed_um_per_sec)
{
  FXString SpeedMsg = FormSpeedInUnitsFrom_um_persecond(dSpeed_um_per_sec);
  CTextFrame * pSpeed = CreateTextFrame((LPCTSTR)SpeedMsg, (LPCTSTR)NULL);
  pSpeed->AddRef();
  PutFrame(m_pXMotion, pSpeed);
  Sleep(100);
  PutFrame(m_pYMotion, pSpeed);
  Sleep(100);
}
void  ConturAsm::SetMotionAcceleration(int iAccInUnits, int iAxesMask)
{
  //     Actual Acceleration
 //       = 10000 * iAccInUnits * M / 1.6384 mm / s ^ 2 or deg / s ^ 2
 //       = 10000 * iAccInUnits / 1.6384 microsteps / s ^ 2
 //       = 10000 * iAccInUnits / R / 1.6384 steps / s ^ 2
 //       Where:
 // 
 //     Data is the value specified in the Command Data
 //       M( mm or deg ) is the microstep size
 //       R is the microstep resolution set in command #37 (microsteps / step)
 //       The maximum value is 32767.
  if (iAxesMask &= 0x07)
  {
    int iNAxes = __popcnt(iAxesMask);
    FXString AccMsg;
    AccMsg.Format("Set_Acceleration=%d", iAccInUnits);
    CTextFrame * pAccel = CreateTextFrame((LPCTSTR)AccMsg, (LPCTSTR)NULL);
    if (iNAxes > 1)
      pAccel->AddRef(iNAxes - 1);
    if (iAxesMask & 1)
    {
      // m_MotionMask |= MotionMaskX ;
      PutFrame(m_pXMotion, pAccel);
      Sleep(100);
    }

    if (iAxesMask & 2)
    {
      //m_MotionMask |= MotionMaskY ;
      PutFrame(m_pYMotion, pAccel);
      Sleep(100);
    }
    if (iAxesMask & 4)
    {
      //m_MotionMask |= MotionMaskZ ;
      CTextFrame * pZAccel = CreateTextFrame(
        (LPCTSTR)AccMsg, _T("SendToMotion"));
      PutFrame(m_pLightControl, pZAccel);
      Sleep(100);
    }
  }

}
void  ConturAsm::StopMotion(int iAxesMask )
{
  if (iAxesMask &= 0x07)
  {
    if (iAxesMask & 3)
    {
      CTextFrame * pStop = CreateTextFrame("Stop=1", (LPCTSTR)NULL);
      if ((iAxesMask & 3) == 3)
        pStop->AddRef();

      if (iAxesMask & 1)
      {
        PutFrame(m_pXMotion, pStop);
        Sleep(50);
      }
      if (iAxesMask & 2)
      {
        PutFrame(m_pYMotion, pStop);
        Sleep(50);
      }
    }
    if (iAxesMask & 4)
    {
      CTextFrame * pStop = CreateTextFrame(
        "Stop=1", _T("SendToMotion"));
      PutFrame(m_pLightControl, pStop);
      Sleep(50);
    }
  }
}
 void  ConturAsm::SetZMotionSpeed(double dSpeed_um_per_sec)
{
  FXString SpeedMsg = FormSpeedInUnitsFrom_um_persecond(dSpeed_um_per_sec);
  CTextFrame * pSpeed = CreateTextFrame((LPCTSTR)SpeedMsg, _T("SetMotionSpeed"));
  PutFrame(m_pLightControl, pSpeed);
  Sleep(100);
}

bool  ConturAsm::MoveToTarget(cmplx& cTargetXY_um, double& dTargetZ_um,
  double dXYSpeed_um_sec)
{
  bool bMoved = false;
  double dZ_um = dTargetZ_um - m_CurrentPos.m_z;
  if (fabs(dZ_um >= 5.0))
  {
    OrderAbsZMotion(dTargetZ_um);
    bMoved = true;
  }
  if (dXYSpeed_um_sec != 0.)
    SetMotionSpeed(dXYSpeed_um_sec);
  double dXY_um = abs(cTargetXY_um - (cmplx)m_CurrentPos);
  if (dXY_um > 5.0)
  {
    OrderAbsMotion(cTargetXY_um);
    bMoved = true;
  }
  return bMoved;
}
bool  ConturAsm::MoveToTarget(CCoordinate& Target)
{
  bool bMoved = false;
  double dZ_um = Target.m_z - m_CurrentPos.m_z;
  if (fabs(dZ_um) >= 10.0)
  {
    OrderAbsZMotion(Target.m_z);
    bMoved = true;
  }
  cmplx cXYTarget = (cmplx)Target;
  double dXY_um = abs(cXYTarget - (cmplx)m_CurrentPos);
  if (dXY_um > 10.0)
  {
    OrderAbsMotion(cXYTarget);
    bMoved = true;
  }
  return bMoved;
}

void ConturAsm::RequestCurrentPosition()
{
  CTextFrame * pRequestCoord = CreateTextFrame(_T(
    "Return_Current_Position=1\r\n"), _T("GetCurrPos"));
  if (!PutFrame(m_pLightControl, pRequestCoord))
    SEND_GADGET_WARN(_T("Can't send Request for XYZ positions"));
}

