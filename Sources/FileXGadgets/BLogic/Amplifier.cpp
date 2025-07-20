// Amplifier.h.h : Implementation of the Amplifier class


#include "StdAfx.h"
#include "Amplifier.h"

// Do replace "TODO_Unknown" to necessary gadget folder name in gadgets tree
IMPLEMENT_RUNTIME_GADGET_EX(Amplifier, CCollectorGadget, "Math", TVDB400_PLUGIN_NAME);
IMPLEMENT_RUNTIME_GADGET_EX(ChkRange , CCollectorGadget, "Math", TVDB400_PLUGIN_NAME);


Amplifier::Amplifier(void)
{
    m_pOutput = new COutputConnector( transparent );
    CreateInputs( 2 , transparent ) ;
    CInputConnector * pInpPlus = GetInputConnector( 0 ) ;
    pInpPlus->SetName( _T("InputPlus") ) ;
    CInputConnector * pInpMinus = GetInputConnector( 1 ) ;
    pInpMinus->SetName( _T("InputMinus") ) ;

    m_pDuplexConnector = new CDuplexConnector( this , transparent, transparent);
    //m_pDuplexConnector = NULL ;
    m_dKPlus = m_dKMinus = 1.0 ;
    m_dOffset = 0.0 ;
    m_iAverage = 1 ;
    m_bCalcStd = FALSE ;
    m_dSumOfSquares = m_dStdDev = 0. ;
    m_dAveragedValue = 0. ;
    m_iNAccumulated = 0 ;
    Resume();
}

void Amplifier::ShutDown()
{
  //TODO: Add all destruction code here
  CCollectorGadget::ShutDown();
	delete m_pDuplexConnector ;
	m_pDuplexConnector = NULL ;
}

bool Amplifier::ScanProperties(LPCTSTR text, bool& Invalidate)
{
   FXPropertyKit pk(text);
  pk.GetDouble("KPlus" , m_dKPlus ) ;
  pk.GetDouble("KMinus" , m_dKMinus ) ;
  pk.GetDouble("Offset" , m_dOffset ) ;
  pk.GetString("Attributes" , m_Attributes ) ;
  pk.GetString( "TextFormat" , m_TextFormat ) ;
  pk.GetInt( "Average" , m_iAverage) ;
  pk.GetInt( "CalcStd" , m_bCalcStd) ;
  m_dAveragedValue = 0. ;
  m_dSumOfSquares = m_dStdDev = m_dAveragedValue = 0. ;
  m_iNAccumulated = 0 ;
  m_cmplxAveragedValue = cmplx(0.,0.) ;
  return true;
}

bool Amplifier::PrintProperties(FXString& text)
{
    FXPropertyKit pk;
  pk.WriteDouble("KPlus" , m_dKPlus ) ;
  pk.WriteDouble("KMinus" , m_dKMinus ) ;
  pk.WriteDouble("Offset" , m_dOffset ) ;
  pk.WriteString("Attributes" , m_Attributes ) ;
  pk.WriteString( "TextFormat" , m_TextFormat ) ;
  pk.WriteInt( "Average" , m_iAverage) ;
  pk.WriteInt( "CalcStd" , m_bCalcStd) ;
  text = pk ;

  return true;

}

bool Amplifier::ScanSettings(FXString& text)
{
    text = _T("template(EditBox(KPlus),"
      "EditBox(KMinus),"
      "EditBox(Offset),"
      "EditBox(Attributes),"
      "EditBox(TextFormat),"
      "Spin(Average,1,1000),"
      "ComboBox(CalcStd(No(0),Yes(1))),"
        ")") ;
  return true;
}

int Amplifier::GetDuplexCount()
{
  return 1 ;
  //return 0 ;
}

CDuplexConnector* Amplifier::GetDuplexConnector(int n)
{
  return (n == 0) ? m_pDuplexConnector : NULL ;
}

void Amplifier::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
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

CDataFrame * Amplifier::DoProcessing(CDataFrame const*const* frames, int nmb)
{
  bool res=false;
  const CDataFrame* reFrame=(*frames);
  ASSERT(reFrame!=NULL);
  double dValuePlus , dValueMinus ;
  int iValCnt = 0 ;
  cmplx cmplxValPlus , cmplxValMinus ;
  int iCmplxValCnt = 0 ;
  for (int i=0; i<nmb; i++)
  {
    ASSERT(*(frames+i)!=NULL);
    const CTextFrame* TextlFrame = (*(frames+i))->GetTextFrame(DEFAULT_LABEL);
    double dValue = 0.0 ;
    if (TextlFrame)
    {
      int j = 0 ;
      int iMaxLen = (int)TextlFrame->GetString().GetLength() ;
      LPCTSTR pString = TextlFrame->GetString() ;
      bool bFinish = false ;
      for (  ; j < iMaxLen ; j++ )
      {
        switch( pString[j] )
        {
        case _T(' '):
        case _T('\t'):
        case _T('('):
        case _T(')'):
        case _T(';'):
        case _T(','):
          break ;
        default: bFinish = true ; break ;
        }
        if ( bFinish )
          break ;
      }
      if ( j < iMaxLen )
      {
        int iRes = _stscanf( _T("%g") , &pString[j] , &dValue ) ;
        if ( iRes )
        {
          if ( i == 0 )
            dValuePlus = dValue * m_dKPlus ;
          else
            dValueMinus = dValue * m_dKMinus ;
        }
      }
    }
    else 
    {
      const CQuantityFrame* quantityframe = (*(frames+i))->GetQuantityFrame(DEFAULT_LABEL);
      if (quantityframe)
      {
        switch ( quantityframe->_type )
        {
        case CGenericQuantity::GQ_INTEGER:
        case CGenericQuantity::GQ_FLOATING:
          dValue = quantityframe->operator double() ;
          if ( i == 0 )
            dValuePlus = dValue * m_dKPlus ;
          else
            dValueMinus = dValue * m_dKMinus ;
          iValCnt++ ;
          break ;
        case CGenericQuantity::GQ_COMPLEX:
          {
            DPOINT Val = quantityframe->operator DPOINT() ;
            cmplx cmplxVal( Val.x , Val.y ) ;
            if ( i == 0 )
              cmplxValPlus = cmplxVal * m_dKPlus ;
            else
              cmplxValMinus = cmplxVal * m_dKMinus ;
            iCmplxValCnt++ ;
          }
        }
      }
    }
  }
  CDataFrame * pResult = NULL ;
  if ( iValCnt == 2 )
  {
    double dOutputValue = dValuePlus - dValueMinus + m_dOffset ;
    if ( m_iNAccumulated++ == 0 )
    {
      m_dAccumulated = m_dAveragedValue = dOutputValue ;
      m_dSumOfSquares += dOutputValue * dOutputValue ;
      m_dStdDev = 0. ;
    }
    else if ( m_iNAccumulated <= m_iAverage )
    {
      m_dAccumulated += dOutputValue ;
//       m_dAveragedValue *= (double) (m_iNAccumulated - 1) / (double) m_iNAccumulated ;
//      m_dAveragedValue += dOutputValue / (double) m_iNAccumulated ;
      m_dAveragedValue = m_dAccumulated / m_iNAccumulated ;
      m_dSumOfSquares += dOutputValue * dOutputValue ;
      if ( m_bCalcStd )
        m_dStdDev = sqrt( (m_dSumOfSquares - (m_dAccumulated * m_dAccumulated/m_iNAccumulated))/m_iNAccumulated ) ;

    }
    else
    {
      double dDeviation = dOutputValue - m_dAveragedValue ;
      double dMult = (m_iAverage > 1) ? (double) (m_iAverage - 1) / (double) m_iAverage : 0.95 ; // average=20
      if ( m_iAverage > 1 )
      {
        m_dAveragedValue = (dOutputValue/m_iAverage) + (m_dAveragedValue * dMult) ;
      }
      else
        m_dAveragedValue = dOutputValue ;

      if ( m_bCalcStd )
          m_dStdDev = (m_dStdDev * dMult) + fabs( dDeviation ) * ( 1.0 - dMult ) ;
    }

    if ( m_TextFormat.IsEmpty() )
      pResult = CQuantityFrame::Create( m_dAveragedValue ) ;
    else
    {
      CTextFrame * pTextOut = CTextFrame::Create() ;
      if ( pTextOut )
      {
        pTextOut->GetString().Format( (LPCTSTR)m_TextFormat , m_dAveragedValue ) ;
        if ( m_bCalcStd )
        {
          m_LastStdAsText.Format(" Std=%g " , m_dStdDev ) ;
          pTextOut->GetString() += m_LastStdAsText ;
        }
        pResult = pTextOut ;
      }
    }
  }
  if ( iCmplxValCnt == 2 )
  {
    cmplx cmplxOutputValue = cmplxValPlus - cmplxValMinus ;
    cmplx cmplxDeviation = cmplxOutputValue - m_cmplxAveragedValue ;
    double dMult = ( m_iAverage > 1 ) ? 
      (double)(m_iAverage - 1)/(double)m_iAverage : 0.95 ; // 19/20
    if ( abs(m_cmplxAveragedValue == 0.) )
      m_cmplxAveragedValue = cmplxOutputValue ;
    {
      if ( m_iAverage > 1 )
      {
        m_cmplxAveragedValue = (cmplxOutputValue/(double)m_iAverage) + (m_cmplxAveragedValue * dMult) ;
      }
      else
        m_cmplxAveragedValue = cmplxOutputValue ;
    }
    if ( m_bCalcStd )
    {
      m_cmplxStdDev = (m_cmplxStdDev * dMult) 
        + cmplx( cmplxDeviation.real() * cmplxDeviation.real() , 
                 cmplxDeviation.imag() * cmplxDeviation.imag() ) ;
      double dDeviation = abs( cmplxDeviation ) ;
      m_dStdDev = (m_dStdDev * dMult) + ( dDeviation * dDeviation ) ;
    }
    if ( m_TextFormat.IsEmpty() )
      pResult = CQuantityFrame::Create( *(DPOINT*)&m_cmplxAveragedValue ) ;
    else
    {
      CTextFrame * pTextOut = CTextFrame::Create() ;
      if ( pTextOut )
      {
        pTextOut->GetString().Format( (LPCTSTR)m_TextFormat ,
          m_cmplxAveragedValue.real() , m_cmplxAveragedValue.imag() ) ;
        if ( m_bCalcStd )
        {
          m_LastStdAsText.Format(" Std=%g (%g,%g)" , sqrt( m_dStdDev * (1.0 - dMult) ) ,
            sqrt( m_cmplxStdDev.real() *(1.0 - dMult) ) ,
            sqrt( m_cmplxStdDev.imag() *(1.0 - dMult) ) ) ;
          pTextOut->GetString() += m_LastStdAsText ;
        }
        pResult = pTextOut ;
      }
    }
  }
  if ( pResult )
  {
    if ( !m_Attributes.IsEmpty() )
    {
//       double * Pars[10] ;
//       int iPos = m_Attributes.Find('%') ;
//       int iLen = m_Attributes.GetLength() ;
//       int iNParam = 0 ;
//       if ( iPos >= 0 )
//       {
//         FXString Attrib ;
//         while ( (iPos >= 0) && (iPos < iLen - 2) && (iNParam < 10) )
//         {
//           TCHAR tInput = _toupper( m_Attributes[iPos+1] ) ;
//         }
//       }
//       else
        *(pResult->Attributes()) =  m_Attributes ;
        CopyIdAndTime( pResult , *frames ) ;
    }
    else
      CopyIdAndTime( pResult , reFrame );
  }

  return pResult ;
}


//   ChkRange Class


ChkRange::ChkRange(void)
{
  m_pInput = new CInputConnector( transparent ) ;
  m_pOutput = new COutputConnector( transparent ) ;
  m_pDuplexConnector = new CDuplexConnector( this , transparent, transparent);
  m_dRangeLow = 0. ;
  m_dRangeHigh = 1.0 ;
  m_dMaxTimeInterval_ms = 100000. ;
  m_iMinBefore = 0 ;
  m_iMinAfter = 0 ;
  m_iAccumulated = 0 ;
  m_dAccumulator = 0. ;
  m_bResetID = FALSE ;
  //m_pDuplexConnector = NULL ;
  Resume();
}

void ChkRange::ShutDown()
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

bool ChkRange::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  pk.GetDouble("RangeLow" , m_dRangeLow ) ;
  pk.GetDouble("RangeHigh" , m_dRangeHigh ) ;
  pk.GetDouble("MaxInterval_ms" , m_dMaxTimeInterval_ms ) ;
  pk.GetInt("MinSamplesBefore" , m_iMinBefore ) ;
  pk.GetInt("MinSamplesAfter" , m_iMinAfter ) ;
  pk.GetString("AttributesOK" , m_AttributesOK ) ;
  pk.GetString("AttributesNOK" , m_AttributesNOK ) ;
  pk.GetString( "TextFormat" , m_TextFormat ) ;
  pk.GetInt( "ResetID" , m_bResetID ) ;
  return true;
}

bool ChkRange::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  pk.WriteDouble("RangeLow" , m_dRangeLow ) ;
  pk.WriteDouble("RangeHigh" , m_dRangeHigh ) ;
  pk.WriteDouble("MaxInterval_ms" , m_dMaxTimeInterval_ms ) ;
  pk.WriteInt( "MinSamplesBefore" , m_iMinBefore ) ;
  pk.WriteInt( "MinSamplesAfter" , m_iMinAfter ) ;
  pk.WriteString("AttributesOK" , m_AttributesOK ) ;
  pk.WriteString("AttributesNOK" , m_AttributesNOK ) ;
  pk.WriteString( "TextFormat" , m_TextFormat ) ;
  pk.WriteInt( "ResetID" , m_bResetID ) ;
  text = pk ;

  return true;

}

bool ChkRange::ScanSettings(FXString& text)
{
  text = _T("template(EditBox(RangeLow),"
    "EditBox(RangeHigh),"
    "EditBox(MaxInterval_ms),"
    "Spin(MinSamplesBefore,0,100),"
    "EditBox(AttributesOK),"
    "EditBox(AttributesNOK),"
    "EditBox(TextFormat),"
    "ComboBox(ResetID(No(0),Yes(1)))"
    ")") ;
  return true;
}

int ChkRange::GetDuplexCount()
{
  return 1 ;
  //return 0 ;
}

CDuplexConnector* ChkRange::GetDuplexConnector(int n)
{
  return (n == 0) ? m_pDuplexConnector : NULL ;
}

void ChkRange::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
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

CDataFrame * ChkRange::DoProcessing(const CDataFrame * pDataFrame )
{
  if ( !pDataFrame )
    return NULL ;
  bool bRes = false;
  CDataFrame * pResult = NULL ;
  double dValue = 0.0 ;
  const CTextFrame* pTextFrame = pDataFrame->GetTextFrame(DEFAULT_LABEL) ;
  if ( pTextFrame )
  {
    if ( !m_InputFormat.IsEmpty() )
    {
      int iScanned = sscanf( (LPCTSTR)m_InputFormat , (LPCTSTR)pTextFrame->GetString() , &dValue ) ;
      if ( !iScanned )
        return NULL ;
    }
    else
    {
      int iScanned = sscanf( _T("%lf") , (LPCTSTR)pTextFrame->GetString() , &dValue ) ;
      if ( !iScanned )
        return NULL ;
    }
  }
  else 
  {
    const CQuantityFrame* quantityframe = pDataFrame->GetQuantityFrame(DEFAULT_LABEL);
    if (quantityframe)
      dValue = quantityframe->operator double() ;
    else
      return NULL ;
  }
  if ( m_iMinBefore )
  {
    double dNow = GetHRTickCount() ;
    double dTimeDiff = dNow - m_dLastDataTime ;
    m_dLastDataTime = dNow ;
    if ( dTimeDiff <= m_dMaxTimeInterval_ms )
    {
      if ( ++m_iAccumulated < m_iMinBefore )
      {
        m_dAccumulator += dValue ;
        m_dAveraged = m_dAccumulator / m_iAccumulated ;
      }
      else
      {
        m_dAveraged = (m_dAveraged * (m_iMinBefore - 1)/m_iMinBefore)
          + (dValue/m_iMinBefore) ;
        double dError = dValue - m_dAveraged ;
        bRes = (m_dRangeLow <= dError) && (dError <= m_dRangeHigh) ;
      }
    }
    else
    {
      m_iAccumulated = 0 ;
      m_dAccumulator = 0. ;
      m_Samples.RemoveAll() ;
    }
    pResult = CBooleanFrame::Create( bRes ) ;
    if ( pResult )
    {
      CopyIdAndTime( pResult , pDataFrame ) ;
      if ( m_bResetID )
        pResult->ChangeId( 0 ) ;
    }
    return pResult ;
  }
  bRes = ( m_dRangeLow <= dValue ) && ( dValue <= m_dRangeHigh ) ;
  if ( m_TextFormat.IsEmpty() )
  {
    pResult = CBooleanFrame::Create( bRes ) ;
  }
  else
  {
    CTextFrame * pTextOut = CTextFrame::Create() ;
    if ( pTextOut )
    {
      pTextOut->GetString().Format( (LPCTSTR)m_TextFormat , dValue ) ;
      pResult = pTextOut ;
      if ( bRes )
      {
        if ( !m_AttributesOK.IsEmpty() )
          *(pResult->Attributes()) = m_AttributesOK ;
      }
      else
      {
        if ( !m_AttributesNOK.IsEmpty() )
          *(pResult->Attributes()) = m_AttributesNOK ;
      }
    }
  }
  if ( pResult )
  {
    CopyIdAndTime( pResult , pDataFrame ) ;
    if ( m_bResetID )
      pResult->ChangeId( 0 ) ;
  }
  return pResult ;
}
