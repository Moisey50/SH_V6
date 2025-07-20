// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "SensorGadget.h"

USER_FILTER_RUNTIME_GADGET( SensorGadget , "HWControllers" );	//	Mandatory

// VOID CALLBACK TimerFunction( UINT uTimerID , UINT uMsg , DWORD dwUser , DWORD dw1 , DWORD dw2 )
// {
//   SensorGadget* params = ( SensorGadget* ) dwUser;
// }

temper_type_t tempers[4] = {
  temper_type_t( 0x0c45, 0x7401, "TEMPer1F", 0, 1, 0, NULL ), 
  temper_type_t(0x1a86, 0xe025, "TEMPer2", 0, 2, 1, NULL ),
  temper_type_t(0x413d, 0x2107, "TEMPerHUM", 0, 1, 1, NULL ),
  temper_type_t(0, 0 , "Dummy" , 0 , 0 , 0 , NULL )  // End of array
};

// temper_type_t tempers[*] = {
//   { 0x0c45, 0x7401, "TEMPer", 1, 2, 0, decode_answer_fm75 }, // TEMPer2* eg. TEMPer2V1.3
//   //{ 0x0c45, 0x7401, "TEMPer1", 0, 1, 0, decode_answer_fm75 }, // other 0c45:7401 eg. TEMPerV1.4
// { 0x0c45, 0x7401, "TEMPer1F", 0, 1, 1, decode_answer_sht1x },
// { 0x413d, 0x2107, "TEMPerHUM", 0, 1, 1, decode_answer_sht1x },
// { 0x1a86, 0xe025, "TEMPer2", 0, 2, 1, decode_answer_fm75 },
// };


SensorGadget::SensorGadget()
{
  EnumerateSensors(tempers, m_Sensors);
  init();
}

bool GetSensorValues(int VENDOR_ID, int PRODUCT_ID, LPTSTR pBuffer, int iBufferSize);


CDataFrame* SensorGadget::DoProcessing( const CDataFrame* pDataFrame )
{
  double dStart = GetHRTickCount() ;
  FXString AsText , NewValue , ForFormat ;
  
  for ( int i = 0 ; i < ARRSZ(tempers) ; i++ )
  {
    if (GetSensorValues( tempers[i].vendor_id , tempers[i].product_id , NewValue.GetBuffer(400), 400 , m_iViewDebug) )
    {
      NewValue.ReleaseBuffer();
      ForFormat.Format("S%d: %s;", i, (LPCTSTR)NewValue);
      AsText += ForFormat;
    }
    else
      AsText.ReleaseBuffer();
  }
  if ( !AsText.IsEmpty() )
    return CreateTextFrame(AsText, "From TSensor", pDataFrame->GetId());
  return NULL;
}

void SensorGadget::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  pParamFrame->Release( pParamFrame );
};
static const char * pDebugViewMode = "No View;View;";

void SensorGadget::PropertiesRegistration()
{
  addProperty(SProperty::COMBO, _T("ViewDebug"), (int *)&m_iViewDebug,
    SProperty::Int, pDebugViewMode);
  //   addProperty( SProperty::EDITBOX , _T( "Format" ) , &m_OutputFormat , SProperty::String );
//   addProperty( SProperty::SPIN , _T( "AdjustStep" ) , &m_dwAdjustStep ,
//     SProperty::Long , 1 , 50 );
//   addProperty( SProperty::EDITBOX , _T( "SpeedThres" ) ,
//     &m_dSpeedAccelThreshold , SProperty::Double );
//   addProperty( SProperty::EDITBOX , _T( "MaxSpeedDelta" ) ,
//     &m_dMaxSpeedAccel , SProperty::Double );
//   addProperty( SProperty::EDITBOX , _T( "Kp" ) ,
//     &m_PID.m_dKp , SProperty::Double );
//   addProperty( SProperty::EDITBOX , _T( "Ki" ) ,
//     &m_PID.m_dKi , SProperty::Double );
//   addProperty( SProperty::EDITBOX , _T( "Kd" ) ,
//     &m_PID.m_dKd , SProperty::Double );
// 
//   addProperty( SProperty::EDITBOX , _T( "IntegralDecr" ) ,
//     &m_PID.m_dDecrease , SProperty::Double );
// //   addProperty( SProperty::EDITBOX , _T( "MaxAngleAccel" ) ,
// //     &m_dMaxAngleAccel , SProperty::Double );
//   addProperty( SProperty::EDITBOX , _T( "TargetDegrees" ) ,
//     &m_dTargetAngle_Deg , SProperty::Double );
// 
};


void SensorGadget::ConnectorsRegistration()
{
  addInputConnector( transparent , "Coordinates" );
  addOutputConnector( transparent , "AngleSpeed" );
  //addDuplexConnector( transparent, transparent, "DuplexName1");

};




