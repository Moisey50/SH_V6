// Amplifier.h.h : Implementation of the Amplifier class


#include "StdAfx.h"
#include "Holoor.h"
#include "helpers\FramesHelper.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "Holoor"

// Check with -r C:\Document\Holo-Or\holoor_by8_NumbersViewExper.tvg
// Do replace "TODO_Unknown" to necessary gadget folder name in gadgets tree
IMPLEMENT_RUNTIME_GADGET_EX(Holoor , CCollectorGadget, "FileX_Specific", TVDB400_PLUGIN_NAME);

Holoor::Holoor(void)
  : m_CoordsUp( 100. , 100. )
  , m_CoordsDown(200. , 200. )
  , m_StepUp( 10. , 0. )
  , m_StepDown( 10. , 0. )
  , m_iNSpotsDown( 0 )
  , m_iNSpotsUp( 0 )
  , m_dScale( 1.0 )
  , m_dDiffrCorrection(0.5)
  , m_iAverageFactor( 1 )
  , m_State( 0 )
  , m_dVibrationThres( 0.5 )
  , m_dToleranceXY( 0.4 )
  , m_dToleranceAngle( 80. )
  , m_iTimeout_ms( 100 )
  , m_AveEtch(0.,0.)
  , m_AveMask(0.,0.)
  , m_LastWas( LAST_UNKNOWN )
  , m_bSave( false )
{
  m_pInput = new CInputConnector( transparent ) ;
  m_pOutput = new COutputConnector( transparent ) ;
  m_pDuplexConnector = new CDuplexConnector( this , transparent, transparent);
  //m_pDuplexConnector = NULL ;
  Resume();
}


void Holoor::ShutDown()
{
  //TODO: Add all destruction code here
  CFilterGadget::ShutDown();
  delete m_pInput ;
  m_pInput = NULL ;
  delete m_pOutput ;
  m_pOutput = NULL ;

  delete m_pDuplexConnector ;
  m_pDuplexConnector = NULL ;
}

bool Holoor::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropKit2 pk(text);
  pk.FXPropertyKit::GetString( "Serial#" , m_Notes ) ;
  pk.FXPropertyKit::GetString( "DataFileName" , m_DataFileName ) ;
//   if ( !m_MaskDescriptionFileName.IsEmpty() )
//     LoadMaskDescription( m_MaskDescriptionFileName  ) ;
//   else
//   {
    pk.GetCmplx("CoordUp" , m_MaskData.m_MaskUp ) ;
    pk.GetCmplx("CoordDown" , m_MaskData.m_MaskDown ) ;
    pk.GetCmplx("StepUp" , m_MaskData.m_StepUp ) ;
    pk.GetCmplx("StepDown" , m_MaskData.m_StepDown ) ;
//   }
  m_MaskData.m_Center = 0.5 * (m_MaskData.m_MaskDown + m_MaskData.m_MaskUp) ;
  pk.GetCmplx("Offset" , m_MeasOffset ) ;
  pk.FXPropertyKit::GetInt( "AverageFactor" , m_iAverageFactor ) ;
  pk.FXPropertyKit::GetDouble( "Scale" , m_dScale ) ;
  pk.FXPropertyKit::GetDouble( "DiffrCorrection" , m_dDiffrCorrection ) ;
  pk.FXPropertyKit::GetDouble( "VibrationThres" , m_dVibrationThres ) ;
  pk.FXPropertyKit::GetDouble( "TeoleranceXY" , m_dToleranceXY ) ;
  pk.FXPropertyKit::GetDouble( "ToleranceAngle" , m_dToleranceAngle ) ;
  pk.FXPropertyKit::GetInt( "Timeout_ms" , m_iTimeout_ms ) ;
  return true;
}

bool Holoor::PrintProperties(FXString& text)
{
  FXPropKit2 pk;
  pk.WriteString("Serial#" , m_Notes) ;
  pk.WriteString("DataFileName" , m_DataFileName) ;
  pk.WriteCmplx("CoordUp" , m_MaskData.m_MaskUp ) ;
  pk.WriteCmplx("CoordDown" , m_MaskData.m_MaskDown ) ;
  pk.WriteCmplx("StepUp" , m_MaskData.m_StepUp ) ;
  pk.WriteCmplx("StepDown" , m_MaskData.m_StepDown ) ;
  pk.WriteCmplx("Offset" , m_MeasOffset) ;
  pk.WriteInt("AverageFactor" , m_iAverageFactor ) ;
  pk.WriteDouble("Scale" , m_dScale ) ;
  pk.WriteDouble("DiffrCorrection" , m_dDiffrCorrection ) ;
  pk.WriteDouble("VibrationThres" , m_dVibrationThres ) ;
  pk.WriteDouble("TeoleranceXY" , m_dToleranceXY ) ;
  pk.WriteDouble("ToleranceAngle" , m_dToleranceAngle ) ;
  CFilterGadget::PrintProperties( pk ) ;
  pk.WriteInt( "Timeout_ms" , m_iTimeout_ms ) ;

  text = pk ;

  return true;

}

bool Holoor::ScanSettings(FXString& text)
{
  text = _T("template(Editbox(Serial#),"
    "EditBox(DataFileName),"
    "EditBox(CoordUp),"
    "EditBox(CoordDown),"
    "EditBox(StepUp),"
    "EditBox(StepDown),"
    "EditBox(Offset),"
    "EditBox(TeoleranceXY),"
    "EditBox(ToleranceAngle),"
    "Spin(AverageFactor,1,100),"
    "EditBox(Scale),"
    "EditBox(DiffrCorrection),"
    "EditBox(VibrationThres),"
    "Spin(Timeout_ms,50,1000000),"
    ")") ;
  return true;
}

int Holoor::GetDuplexCount()
{
  return 1 ;
  //return 0 ;
}

CDuplexConnector* Holoor::GetDuplexConnector(int n)
{
  return (n == 0) ? m_pDuplexConnector : NULL ;
}

void Holoor::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  const CTextFrame * pCommand = pParamFrame->GetTextFrame() ;
  if ( pCommand )
  {
    FXString Command = pCommand->GetString() ;
    if ( Command.Find( _T("set") == 0 ) )
    {
      bool Invalidate = false ;
      ScanProperties( ((LPCTSTR)Command) + 4 , Invalidate ) ;
    }
  }
  pParamFrame->Release( pParamFrame );
}

CDataFrame * Holoor::DoProcessing(const CDataFrame * pDataFrame )
{
  if ( !pDataFrame )
    return NULL ;
  bool bRes = false;


  m_LastData.Reset() ;
  m_iNSpotsDown = m_iNSpotsUp = 0 ;
  m_PrevState = m_State ;
  m_State = 0 ;
  FXString Out ;
  CContainerFrame * pResult = CContainerFrame::Create() ;
  pResult->AddFrame( pDataFrame ) ; // first of all, image and all results

  cmplx Shift ;
  cmplx MaskToEtchCenter ;
  cmplx EtchToEtchCenter ;
  double dRotation = 0.0 ;

  CFramesIterator * Iterator = pDataFrame->CreateFramesIterator( text ) ;
  if ( Iterator )
  {
    CTextFrame * pText = (CTextFrame*) Iterator->Next() ;
    while ( pText )
    {
      m_LastData.m_iAddedMask |= DecodeElementData( pText ) ;
      pText = (CTextFrame*) Iterator->Next() ;
    }
    delete Iterator ;
  }
  m_State = 0 ;
  if ( (m_LastData.m_iAddedMask & MASK_ALL_NECESSARY) == MASK_ALL_NECESSARY )
  {  // OK, all data received, possible to do data processing

    if ( m_iNSpotsUp && m_iNSpotsDown )  // error, should be spot on one side only 
    {
    }
    else
    {
      m_State = m_iNSpotsUp ? 1 : 2 ;
      m_LastData.m_dMeasTime = GetHRTickCount() ;
      cmplx EtchingPos = m_LastData.GetEtchingPosition() ;
      cmplx MaskPos = m_LastData.GetMaskPosition() ;

      if ( ((m_LastData.m_dMeasTime - m_dLastDataTime) > m_iTimeout_ms) 
        || ( m_State != m_PrevState) ) 
        m_iAccumulated = 0 ;
 
      m_dLastDataTime = m_LastData.m_dMeasTime ; 

      double dAverager = (++m_iAccumulated >= m_iAverageFactor )?
        m_iAverageFactor : m_iAccumulated ;
      double dMulNew = 1./dAverager ;
      double dMul = (dAverager - 1.)/dAverager ;
      m_AveMask *= dMul ;
      cmplx NewPart = dMulNew * MaskPos ;
      m_AveMask += NewPart ;
     // m_AveMask = (m_AveMask * dMul) + (MaskPos * dMulNew) ;
      m_AveEtch = (m_AveEtch * dMul) + (EtchingPos * dMulNew) ;
 
      if ( m_iAccumulated >= m_iAverageFactor )
      {
        USHORT CTRL_Pressed = GetKeyState( VK_CONTROL ) ;
        USHORT SHIFT_Pressed = GetKeyState( VK_SHIFT ) ;
        USHORT S_Pressed = GetKeyState( _T('S') ) ;
        bool bRotation = ( CTRL_Pressed & 0x8000) != 0 ;
        bool bShift = (SHIFT_Pressed & 0x8000) != 0 ;
        bool bSave = (S_Pressed & 0x8000) != 0 ;
        m_iCurrentCross = (m_iNSpotsUp + m_iNSpotsDown - 1) ;
        m_CoordsUp = m_MaskData.m_CurrentMaskUp = m_MaskData.m_MaskUp 
          + m_MaskData.m_StepUp * (double)m_iCurrentCross ;
        m_CoordsDown = m_MaskData.m_CurrentMaskDown = m_MaskData.m_MaskDown 
          + m_MaskData.m_StepDown * (double)m_iCurrentCross ;
        m_MaskData.m_Center = 0.5 * ( m_CoordsDown + m_CoordsUp ) ;
        cmplx MaskDir = m_CoordsDown - m_CoordsUp ;

        double dVibrationIndex = abs(m_AveMask - MaskPos) ;
        if ( dVibrationIndex <= m_dVibrationThres ) // system is stable
        {
          cmplx ShiftNow = m_AveEtch - m_AveMask ; 
          if ( m_iNSpotsDown )  // when spots are on down side, we measure upper mask
          {
            m_EtchUp = m_AveEtch ;
            m_MaskUp = m_AveMask ;
            m_ShiftUpPrev = m_ShiftUp ;
            m_ShiftUp = ShiftNow ;
          }
          else
          {
            m_EtchDown = m_AveEtch ;
            m_MaskDown = m_AveMask ;
            m_ShiftDownPrev = m_ShiftDown ;
            m_ShiftDown = ShiftNow ;
          }
          FXString Info , Positions , AddInfo  ;
          Positions.Format( 
//               "EtchUp=(%6.2f,%6.2f) MaskUp=(%6.2f,%6.2f)\n"
//               "EtchDwn=(%6.2f,%6.2f) MaskDwn=(%6.2f,%6.2f)\n"
              "ShiftUp=(%6.2f,%6.2f) ShiftDwn=(%6.2f,%6.2f)\n",
//               m_EtchUp.real() , m_EtchUp.imag() , m_MaskUp.real() , m_MaskUp.imag() ,
//               m_EtchDown.real() , m_EtchDown.imag() , m_MaskDown.real() , m_MaskDown.imag() ,
              m_ShiftUp.real() , m_ShiftUp.imag() , m_ShiftDown.real() , m_ShiftDown.imag()) ;
          if ( bShift )
          {
            SaveOrigins() ;
            Info = FXString( "SaveOrigins by SHIFT\n" ) ;
          }
         // else
          {
            if ( bRotation != m_bRotation )
            {
//               if ( !bShift )
//                SaveOrigins() ;
              m_bRotation = bRotation ;
            }
            if ( m_iNSpotsDown ) // when spots are on down side, we measure upper mask
            {
              Info = FXString( "Up Mask Measurement\n" )  ;
//              cmplx ShiftFromOrig = m_ShiftUp - m_ShiftUpOrig ;
              if ( m_LastWas == LAST_UP )
              {
                cmplx ShiftFromOrig = m_ShiftUp - m_ShiftUpPrev ;
  //               // Calculate position on opposite side
  //               cmplx EtchDownShift = (bRotation) ? 
  //                 m_ShiftDownOrig - ShiftFromOrig : m_ShiftDownOrig + ShiftFromOrig ; // in pixels
                cmplx EtchDownShift = (bRotation) ? 
                  m_ShiftDown - ShiftFromOrig : m_ShiftDown + ShiftFromOrig ; // in pixels
                m_ShiftDownPrev = m_ShiftDown ;
                m_ShiftDown = EtchDownShift ;
              }
              else
                m_LastWas = LAST_UP ;
            }
            else
            {
              Info = FXString( "Down Mask Measurement\n" )  ;
              //               cmplx ShiftFromOrig = m_ShiftDown - m_ShiftDownOrig ;
              if ( m_LastWas = LAST_DOWN )
              {
                cmplx ShiftFromOrig = m_ShiftDown - m_ShiftDownPrev ;
                // Calculate position on opposite side
                //             cmplx EtchUpShift = (bRotation) ? 
                //                m_ShiftUpOrig - ShiftFromOrig : m_ShiftUpOrig + ShiftFromOrig ; // in pixels
                cmplx EtchUpShift = (bRotation) ? m_ShiftUp - ShiftFromOrig : m_ShiftUp + ShiftFromOrig ; // in pixels
                m_ShiftUpPrev = m_ShiftUp ;
                m_ShiftUp = EtchUpShift ;
              }
              else
                m_LastWas = LAST_DOWN ;
            }
            cmplx EtchToMaskUp = m_ShiftUp * m_dScale ; // in microns
            cmplx EtchToMaskDown = m_ShiftDown * m_dScale ; // in  microns
            if ( m_MaskData.m_MaskUp.imag() > m_MaskData.m_MaskDown.imag() ) // is Y  directed in opposite side?
            {
              EtchToMaskUp._Val[_IM] = -EtchToMaskUp._Val[_IM] ; 
              EtchToMaskDown._Val[_IM] = -EtchToMaskDown._Val[_IM] ; 
            }

            Shift = 0.5 * ( EtchToMaskUp + EtchToMaskDown ) ;

            m_EtchUpAbs = m_MaskData.m_CurrentMaskUp + EtchToMaskUp ;
            m_EtchDownAbs = m_MaskData.m_CurrentMaskDown + EtchToMaskDown ;
            cmplx EtchDir = m_EtchDownAbs - m_EtchUpAbs ;
            m_EtchCenter = m_MaskData.m_Center + Shift ;
            cmplx ShiftUpFromOrig = m_ShiftUp - m_ShiftUpOrig ;
            cmplx ShiftDownFromOrig = m_ShiftDown - m_ShiftDownOrig ;

            AddInfo.Format( "UpFromOrig=(%6.2f,%6.2f) DownFromOrig=(%6.2f,%6.2f)\n" 
              "Shift=(%6.2f,%6.2f) EtchCent=(%6.2f,%6.2f)\n" ,
              ShiftUpFromOrig.real() , ShiftUpFromOrig.imag() ,
              ShiftDownFromOrig.real() , ShiftDownFromOrig.imag() , 
              Shift.real() , Shift.imag() , m_EtchCenter.real() , m_EtchCenter.imag() ) ;
            Info += Positions /*+ AddInfo*/ ;
            CTextFrame * pViewInfo = CTextFrame::Create( (LPCTSTR)Info ) ;
            if ( pViewInfo )
            {
              *(pViewInfo->Attributes()) = "color=0xffff00;Sz=15;x=350;y=30;" ;
              CopyIdAndTime( pResult , pDataFrame );
              pResult->AddFrame( pViewInfo ) ;
            }

            double dEtchAngle = arg( EtchDir ) ;  // angle of vector from etch center to etch cross
            double dMaskAngle = arg( MaskDir ) ;  // angle of vector from etch center to mask cross 
            dRotation = dEtchAngle - dMaskAngle ; // we have to rotate for this angle
          }
          cmplx ShiftWithOffset = Shift + m_MeasOffset ;
          Out.Format( "dX=%5.2f um" , ShiftWithOffset.real() ) ;
          CTextFrame * pViewInfo = CTextFrame::Create( (LPCTSTR)Out ) ;
          if ( pViewInfo )
          {
            *(pViewInfo->Attributes()) = (abs(ShiftWithOffset.real()) <= m_dToleranceXY) ?
              "color=0x00c000;Sz=36;x=30;y=250;back=0;" : "color=0x0000c0;Sz=36;x=30;y=250;back=0;" ;
            CopyIdAndTime( pResult , pDataFrame );
            pResult->AddFrame( pViewInfo ) ;
          }
          Out.Format( "dY=%5.2f um" , ShiftWithOffset.imag() ) ;
          pViewInfo = CTextFrame::Create( (LPCTSTR)Out ) ;
          if ( pViewInfo )
          {
            *(pViewInfo->Attributes()) = (abs(ShiftWithOffset.imag()) <= m_dToleranceXY) ?
              "color=0x00c000;Sz=36;x=30;y=275;back=0;" : "color=0x0000c0;Sz=36;x=30;y=275;back=0;" ;
            CopyIdAndTime( pResult , pDataFrame );
            pResult->AddFrame( pViewInfo ) ;
          }
          Out.Format( "dA=%6.2f uRad" , dRotation *= 1e6 ) ;
          pViewInfo = CTextFrame::Create( (LPCTSTR)Out ) ;
          if ( pViewInfo )
          {
            *(pViewInfo->Attributes()) = ( fabs(dRotation) <= m_dToleranceAngle) ?
              "color=0x00c000;Sz=36;x=30;y=300;back=0;" : "color=0x0000c0;Sz=36;x=30;y=300;back=0;" ;
            CopyIdAndTime( pResult , pDataFrame );
            pResult->AddFrame( pViewInfo ) ;
          }
          Out = "CTRL(" ;
          Out += (m_bRotation) ? "ON) - Rotation" : "OFF) - Shift" ;
          pViewInfo = CTextFrame::Create( (LPCTSTR)Out ) ;
          if ( pViewInfo )
          {
            *(pViewInfo->Attributes()) = ( m_bRotation ) ?
              "color=0x0080FF;Sz=36;x=30;y=60;back=0;" : "color=0xFF8000;Sz=36;x=30;y=60;back=0;" ;
            CopyIdAndTime( pResult , pDataFrame );
            pResult->AddFrame( pViewInfo ) ;
          }
          Out = "SHIFT(" ;
          Out += (bShift) ? "ON) - Save Position" : "OFF)" ;
          pViewInfo = CTextFrame::Create( (LPCTSTR)Out ) ;
          if ( pViewInfo )
          {
            *(pViewInfo->Attributes()) = ( bShift ) ?
              "color=0x0080FF;Sz=36;x=30;y=20;back=0;" : "color=0xFF8000;Sz=36;x=30;y=20;back=0;" ;
            CopyIdAndTime( pResult , pDataFrame );
            pResult->AddFrame( pViewInfo ) ;
          }
          Out.Format( "Vibration %5.2f" , dVibrationIndex ) ;
          pViewInfo = CTextFrame::Create( Out ) ;
          if ( pViewInfo )
          {
            *(pViewInfo->Attributes()) = "color=0x00ffff;Sz=36;x=30;y=150;back=0;" ;
            CopyIdAndTime( pResult , pDataFrame );
            pResult->AddFrame( pViewInfo ) ;
          }

          if ( bSave )
          {
            if ( !m_bSave )
            {
              m_bSave = true ;
              if ( !m_DataFileName.IsEmpty() )
              {
                FILE * pW = NULL ;
                
                errno_t err = _tfopen_s( &pW , (LPCTSTR)m_DataFileName , _T("a")) ;
                if ( err == 0 ) // no error
                {
                  if ( pW )
                  {
                    fseek( pW , 0L , SEEK_END) ;
                    fpos_t Pos = ftell( pW ) ;
                    int iErr = 0 ;//fgetpos( pW , &Pos ) ;
                    if ( iErr == 0 )
                    {
                      if ( Pos < 20 ) // first opening
                      {
                        _fputts( 
                          _T("    Time                , Pos, Step , dX um , dY um , dA uRad , dXUp ,  dYUp ,  dXDn ,  dYDn ,  Serial# and Notes\n")  ,
                          pW ) ;
                      }
                    }
                    time_t t ;
                    time( &t ) ;
                    FXString Out = ctime( &t ) ;
                    Out.Remove(_T('\r')) ;
                    Out.Remove(_T('\n')) ;
                    Out += (m_iNSpotsDown) ? ",Up  " : ",Down" ;
                    Positions.Format( ",   %d  , %6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f,   %s\n", 
                      (m_iNSpotsDown) ? m_iNSpotsDown : m_iNSpotsUp , // step number
                      ShiftWithOffset.real() , ShiftWithOffset.imag() , dRotation , 
                      m_ShiftUp.real() , m_ShiftUp.imag() , 
                      m_ShiftDown.real() , m_ShiftDown.imag(), (LPCTSTR)m_Notes ) ;
                    Out += Positions ;
                    _fputts( (LPCTSTR)Out , pW ) ;
                    fclose( pW ) ;
                  }
                }
              }
            }
          }
          else
            m_bSave = false ;
        }
        else
        {
          Out.Format( "Big Vibration %5.2f" , dVibrationIndex ) ;
          CTextFrame * pViewInfo = CTextFrame::Create( Out ) ;
          if ( pViewInfo )
          {
            *(pViewInfo->Attributes()) = "color=0x0000ff;Sz=36;x=30;y=150;" ;
            CopyIdAndTime( pResult , pDataFrame );
            pResult->AddFrame( pViewInfo ) ;
          }
          m_iAccumulated = 0 ;
        }

      }
      else
        m_LastWas = LAST_UNKNOWN ;
    }

  }
  m_PrevState = m_State ;
  return pResult ;
}




int Holoor::DecodeElementData(CTextFrame * pTextData)
{
  FXString Label = pTextData->GetLabel() ;
  FXString Text = pTextData->GetString() ;
  FXSIZE iPos = 0 ;
  char DataSep = ':' ;
  if ( (iPos = Label.Find("Data_Spot")) >= 0 )
  {
    if ( (iPos = Label.Find( "mask_spot" , iPos + 10 )) >= 0 )
    {
      FXSIZE iPos3 = Text.Find( '=' ) ;
      int iNSpots = 0 ;
      if ( iPos3 >= 0 )
        iNSpots = atoi( (LPCTSTR)Text + iPos3 + 1 ) ;
      else 
        return 0 ;
      if ( Label.Find( "lower" , iPos + 9 ) >= 0 )
      {
        m_iNSpotsDown = iNSpots ;
        return MASK_NSPOTS | MASK_NSPOTS_DOWN ;
      }
      else if ( Label.Find( "upper" , iPos + 9 ) >= 0 )
      {
        m_iNSpotsUp = iNSpots ;
        return MASK_NSPOTS | MASK_NSPOTS_UP ;
      }
      else
        return 0 ;
    }
  }
  else if ( (iPos = Label.Find("Data_Line")) >= 0 )
  {
    double Data[11] ;
    if ( Label.Find("etch_hor_lower" , iPos + 9) >= 0 )
    {
      int iRes = GetArrayFromString( Text , 0 , 0 , Data , 10 , &DataSep ) ;
      if ( iRes >= 9 )
      {
        m_LastData.m_dEtch_hrd = Data[2] - (Data[8] - Data[7]) * m_dDiffrCorrection ;
        return MASK_ETCH_H_LOWER ;
      }
      
//       int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dEtch_hrd ,
//         2 , 0 , &DataSep ) ;
//       if ( iRes )
//         return MASK_ETCH_H_LOWER ;
      else
        return 0 ;
    }
    else if ( Label.Find("etch_hor_upper" , iPos + 9) >= 0 )
    {
      int iRes = GetArrayFromString( Text , 0 , 0 , Data , 10 , &DataSep ) ;
      if ( iRes >= 9 )
      {
        m_LastData.m_dEtch_hru = Data[2] + (Data[8] - Data[7]) * m_dDiffrCorrection ;
        return MASK_ETCH_H_UPPER ;
      }
//       int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dEtch_hru ,
//         2 , 0 , &DataSep ) ;
//       if ( iRes )
//         return MASK_ETCH_H_UPPER ;
      else
        return 0 ;
    }
    else if ( Label.Find("etch_v_left" , iPos + 9) >= 0 )
    {
      int iRes = GetArrayFromString( Text , 0 , 0 , Data , 10 , &DataSep ) ;
      if ( iRes >= 9 )
      {
        m_LastData.m_dEtch_vul = Data[1] + (Data[6] - Data[5]) * m_dDiffrCorrection ;
        return MASK_ETCH_V_LEFT ;
      }
//       int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dEtch_vul ,
//         1 , 0 , &DataSep ) ;
//       if ( iRes )
//         return MASK_ETCH_V_LEFT ;
      else
        return 0 ;
    }
    else if ( Label.Find("etch_v_right" , iPos + 9) >= 0 )
    {
      int iRes = GetArrayFromString( Text , 0 , 0 , Data , 10 , &DataSep ) ;
      if ( iRes >= 9 )
      {
        m_LastData.m_dEtch_vur = Data[1] - (Data[6] - Data[5]) * m_dDiffrCorrection ;
        return MASK_ETCH_V_RIGHT ;
      }
//       int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dEtch_vur ,
//         1 , 0 , &DataSep ) ;
//       if ( iRes )
//         return MASK_ETCH_V_RIGHT ;
      else
        return 0 ;
    }
    else if ( Label.Find("etch_hor_ld" , iPos + 9) >= 0 ) // horizontal left down
    {
      int iRes = GetArrayFromString( Text , 0 , 0 , Data , 10 , &DataSep ) ;
      if ( iRes >= 9 )
      {
        m_LastData.m_dEtch_hld = Data[2] - (Data[8] - Data[7]) * m_dDiffrCorrection ;
        return MASK_ETCH_HLEFT_D ;
      }
//       int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dEtch_hld ,
//         2 , 0 , &DataSep ) ;
//       if ( iRes )
//         return MASK_ETCH_HLEFT_D ;
      else
        return 0 ;
    }
    else if ( Label.Find("etch_hor_lu" , iPos + 9) >= 0 )
    {
      int iRes = GetArrayFromString( Text , 0 , 0 , Data , 10 , &DataSep ) ;
      if ( iRes >= 9 )
      {
        m_LastData.m_dEtch_hlu = Data[2] + (Data[8] - Data[7]) * m_dDiffrCorrection ;
        return MASK_ETCH_HLEFT_U ;
      }
//       int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dEtch_hlu ,
//         1 , 0 , &DataSep ) ;
//       if ( iRes )
//         return MASK_ETCH_HLEFT_U ;
      else
        return 0 ;
    }
    else if ( Label.Find("etch_v_ld" , iPos + 9) >= 0 )
    {
      int iRes = GetArrayFromString( Text , 0 , 0 , Data , 10 , &DataSep ) ;
      if ( iRes >= 9 )
      {
        m_LastData.m_dEtch_vdl = Data[1] + (Data[6] - Data[5]) * m_dDiffrCorrection ;
        return MASK_ETCH_VDOWN_L ;
      }
//       int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dEtch_vdl ,
//         1 , 0 , &DataSep ) ;
//       if ( iRes )
//         return MASK_ETCH_VDOWN_L ;
      else
        return 0 ;
    }
    if ( Label.Find("etch_v_rd" , iPos + 9) >= 0 )
    {
      int iRes = GetArrayFromString( Text , 0 , 0 , Data , 10 , &DataSep ) ;
      if ( iRes >= 9 )
      {
        m_LastData.m_dEtch_vdr = Data[1] - (Data[6] - Data[5]) * m_dDiffrCorrection ;
        return MASK_ETCH_VDOWN_R ;
      }
//       int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dEtch_vdr ,
//         2 , 0 , &DataSep ) ;
//       if ( iRes )
//         return MASK_ETCH_VDOWN_R ;
      else
        return 0 ;
    }
   else if ( Label.Find("mask_vert_fine" , iPos + 9) >= 0 )
    {
      int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dMaskHor ,
        1 , 0 , &DataSep ) ;
      if ( iRes )
        return MASK_MASK_VERT ;
      else
        return 0 ;
    }
    else if ( Label.Find("mask_hor_fine" , iPos + 9) >= 0 )
    {
      int iRes = GetNumberInString( Text , "%lf" , &m_LastData.m_dMaskVert ,
        2 , 0 , &DataSep ) ;
      if ( iRes )
        return MASK_MASK_HORIZ ;
      else
        return 0 ;
    }
  }


  return 0;
}

int Holoor::GetNumberInString(FXString& Str, LPCTSTR pFormat, void * pVal , 
                              int iIndex , FXSIZE iPos , const char * pAfter )
{
  if ( pAfter )
  {
    FXSIZE iAfterPos = Str.Find( *pAfter ) ;
    if ( iAfterPos >= iPos )
      iPos = iAfterPos + 1 ;
    else if ( iAfterPos < 0 )
      return 0 ;
  }
  FXString Token = Str.Tokenize( " ,\t" , iPos ) ;
  int iCnt = 0 ;
  while ( iPos >= 0 )
  {
    if ( iCnt++ == iIndex )
    {
      int iRes = sscanf( (LPCTSTR)Token , pFormat , pVal ) ;
      return iRes ;
    }
    Token = Str.Tokenize(  " ,\t" , iPos  ) ;
  }
  return 0;
}

int Holoor::GetArrayFromString( FXString& Str,  int iIndex , FXSIZE iPos , 
  double * pArray , int iArrLen , const char * pAfter )
{
  if ( pAfter )
  {
    int iAfterPos = (int) Str.Find( *pAfter ) ;
    if ( iAfterPos >= iPos )
      iPos = iAfterPos + 1 ;
    else if ( iAfterPos < 0 )
      return 0 ;
  }
  FXString Token = Str.Tokenize( " ,\t()[];" , iPos ) ;
  int iCnt = 0 ;
  while ( iPos >= 0 )
  {
    if ( (iCnt >= iIndex) && ((iCnt - iIndex) < iArrLen) )
    {
      int iRes = sscanf( (LPCTSTR)Token , _T("%lf") , pArray ) ;
      if ( !iRes )
        break ;
       ;
    }
    pArray++ ;
    iCnt++ ;
    Token = Str.Tokenize(  " ,\t" , iPos  ) ;
  }
  return iCnt - iIndex ;
}



bool Holoor::LoadMaskDescription(LPCTSTR FileName)
{
  char Buf[60000] ;

  FILE * pFr = NULL ;

  errno_t err = fopen_s( &pFr , FileName , "r" ) ;
  if ( err == 0 ) // OK
  {
    size_t tLen = fread_s( Buf , sizeof(Buf) , 1 , sizeof(Buf) , pFr ) ;
    fclose( pFr ) ;
    if ( tLen != EINVAL )
    {
      Buf[tLen] = 0 ;
      m_MaskConfigData = Buf ;
      FXSIZE iPos = m_MaskConfigData.Find( _T("MaskUp") ) ;
      if ( iPos >= 0 )
      {
        iPos =  m_MaskConfigData.Find( '=' , iPos + 5 ) ;
        if ( iPos >= 0 )
        {
          memset( &m_MaskData , 0 , sizeof( m_MaskData ) ) ;
          StrToCmplx( Buf + iPos + 1 , m_MaskData.m_MaskUp ) ;
        }
        else 
        {
          SENDERR_0("Can't find equal sign (=) with upper mask description (named MaskUp)" );
          return false ;
        }
      }
      else 
      {
        SENDERR_0("Can't find upper mask description (named MaskUp)" );
        return false ;
      }
      iPos = m_MaskConfigData.Find( _T("MaskDown") ) ;
      if ( iPos >= 0 )
      {
        iPos =  m_MaskConfigData.Find( '=' , iPos + 5 ) ;
        if ( iPos >= 0 )
          StrToCmplx( Buf + iPos + 1 , m_MaskData.m_MaskDown ) ;
        else 
        {
          SENDERR_0("Can't find equal sign (=) with lower mask description (named MaskDown)" );
          return false ;
        }
      }
      else 
      {
        SENDERR_0("Can't find lower mask description (named MaskDown)" );
        return false ;
      }

      iPos = m_MaskConfigData.Find( _T("StepUp") ) ;
      if ( iPos >= 0 )
      {
        iPos = m_MaskConfigData.Find( '=' , iPos + 5 ) ;
        if ( iPos >= 0 )
          StrToCmplx( Buf + iPos + 1 , m_MaskData.m_StepUp ) ;
        else 
        {
          SENDERR_0("Can't find equal sign (=) with upper step description (named StepUp)" );
          return false ;
        }
      }
      else 
      {
        SENDERR_0("Can't find upper step description (named StepUp)" );
        return false ;
      }
      iPos = m_MaskConfigData.Find( _T("StepDown") ) ;
      if ( iPos >= 0 )
      {
        iPos = m_MaskConfigData.Find( '=' , iPos + 5 ) ;
        if ( iPos >= 0 )
          StrToCmplx( Buf + iPos + 1 , m_MaskData.m_StepDown ) ;
        else 
        {
          SENDERR_0("Can't find equal sign (=) with upper step description (named StepDown)" );
          return false ;
        }
      }
      else 
      {
        SENDERR_0("Can't find upper step description (named StepDown)" );
        return false ;
      }
      m_MaskData.m_Center = (m_MaskData.m_MaskUp + m_MaskData.m_MaskDown)/2. ;
      SENDINFO("Mask File %s is loaded" , (LPCTSTR)FileName ) ;
      return true ;
    }
  }

  return false;
}

