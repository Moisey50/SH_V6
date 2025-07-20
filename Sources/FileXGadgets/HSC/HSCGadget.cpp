/********************************************************************
  created:	2017/06/08
  created:	8:6:2017   12:12
  filename: 	C:\Dev\SH\Integration_20170607\Sources\FileXGadgets\HSC\HSCGadget.cpp
  file path:	C:\Dev\SH\Integration_20170607\Sources\FileXGadgets\HSC
  file base:	HSCGadget
  file ext:	cpp
  authors:		Daniel Zimmerman, Moisey Bernstein

  purpose: Recognition by Hyper Spectral Imaging
  Gadget receives separate images hyper spectral camera and does recognition
  Wave length of each image is marked by label
  *********************************************************************/


#include "StdAfx.h"
#include "HSCGadget.h"
#include "math\intf_sup.h" // for swapdouble availability
#include <helpers\propertykitEx.h>

USER_FILTER_RUNTIME_GADGET( HSCGadget , LINEAGE_FILEX );	//	Mandatory

//const char* UserExampleGadget::pList = "AA1; AA2; AA3; BB1; BB2; BB3; CC3";	//	Example
LPCTSTR GetWorkingModeName( DWORD mode )
{
  switch ( mode )
  {
    case WMode_CSVM: return _T( "CSVM" ) ;
    case WMode_Teaching: return _T( "Teaching" ) ;
    case WMode_Recognition_Amplitudes: return _T( "Amplitudes" ) ;
    case WMode_Recognition_Ratios: return _T( "Ratios" ) ;
    case WMode_Recognition_Ratios_Norm: return _T( "Normalized Ratios" ) ;
  }
  return _T( "Unknown Mode" ) ;
}


HSCGadget::HSCGadget()
{
  m_dEpsilon = 0.05;
  m_dAmplThres = 0.1 ;
  m_dwFrameCnt = 0 ;
  m_TeachingArea = CRect( 0 , 0 , 0 , 0 ) ;
  m_BlackRefArea = CRect( 0 , 0 , 0 , 0 ) ;
  m_WhiteRefArea = CRect( 0 , 0 , 0 , 0 ) ;
  m_iNChannels = 0 ;
  m_bInitialConfig = true ;
  m_OldWorkingMode = m_WorkingMode = WMode_CSVM ;
  m_iTeachingCounter = 0 ;
  m_bRefChanged = true;
  m_bUseInternal = false ;
  m_ScanMode = SCAN_Jumps ;
  m_iCurrentWL = -1 ;
  m_bSpectrumsChanged = false ;
  m_dWaveLengthStep_nm = 5.0;
  m_dMinWaveLength_nm = 900.;
  m_sCommandFormat = _T( "SWL %d\r" ) ;
  m_GadgetInfo = _T( "HSC" ) ;
  m_iFramesDivider = 3;
  m_iNotActiveFrameCntr = 0;
  m_bWasRecognition = false ;
  //m_iCommandsCounter = 0;
  init();
}
HSCGadget::~HSCGadget()
{
  m_Spectrums.clear() ;
  VFMap::iterator it = m_videoFramesMap.begin();
  if ( it != m_videoFramesMap.end() )
  {
    CVideoFrame * pOld = it->second ;
    pOld->Release() ;
  }
  m_videoFramesMap.clear() ;
}
bool  HSCGadget::SendNextWaveLength( int iWaveLength_nm , int iPinText , int iPinQuantity )
{
  COutputConnector * pCommandPin = GetOutputConnector( iPinText ) ;
  if ( pCommandPin )
  {
    if ( m_sCommandFormat.IsEmpty() )
    {
      CQuantityFrame * pWL = CQuantityFrame::Create( iWaveLength_nm );
      return PutFrame( pCommandPin , pWL );
    }
    else
    {
      CTextFrame * pAsTextFrame = CreateTextFrame( (LPCTSTR) NULL , _T( "NextWaveLength" ) ) ;
      if ( pAsTextFrame )
      {
        pAsTextFrame->GetString().Format( (LPCTSTR) m_sCommandFormat , iWaveLength_nm ) ;
        pAsTextFrame->ChangeId( m_iRequestCount++ ) ;
        PutFrame( pCommandPin , pAsTextFrame ) ;
      }
      if ( iPinQuantity > 0 )
      {
        COutputConnector * pQPin = GetOutputConnector( iPinQuantity ) ;
        CQuantityFrame * pWL = CreateQuantityFrame( iWaveLength_nm , "WL_Integer" , 0 ) ;
        PutFrame( pQPin , pWL );
      }
      m_iLastOrderedWL = iWaveLength_nm ;
      m_dLastWLOrderTime = GetHRTickCount() ;
      return true ;
    }
  }

  return false ;
}

int HSCGadget::InitRecognitionForRatio()
{
  int iSpecCnt = 0 ;
  m_LearnedData.clear() ;
  m_ActiveSpectrums.clear();

  // The first cycle is for data extraction from spectrums
  // We will take data about only necessary wave lengths
  Spectrums::iterator it = m_Spectrums.begin() ;
  while ( it != m_Spectrums.end() )
  {
    it->m_CurrentRecognition.clear() ;
    it->m_dMaxValue = 0. ;
    for ( int iWLCnt = 0; iWLCnt < m_WaveLengths.GetCount() ; iWLCnt++ )
    {
      int iWL = m_WaveLengths[ iWLCnt ] ;
      UINT iComponentCnt = 0 ;
      for ( ; iComponentCnt < it->m_Data.size() ; iComponentCnt++ )
      {
        if ( it->m_Data[ iComponentCnt ].first == iWL )
        {
          double dValue = it->m_Data[ iComponentCnt ].second ;
          it->m_CurrentRecognition.push_back( dValue ) ;
          if ( it->m_dMaxValue < dValue )
            it->m_dMaxValue = dValue ;
          break ;
        }
      }
      if ( iComponentCnt >= it->m_Data.size() )
        return -(iWLCnt * 1000000 + iSpecCnt) ;

    }
    // Normalization, if it's not base or wbase
    if ( it->m_Name != _T( "BASE" )
      && it->m_Name != _T( "WBASE" ) )
    {
      for ( UINT i = 0; i < it->m_CurrentRecognition.size() ; i++ )
        it->m_CurrentRecognition[ i ] /= it->m_dMaxValue ;
      m_ActiveSpectrums.push_back( iSpecCnt );
    }
    iSpecCnt++ ;
    it++ ;
  }
  return iSpecCnt ;
}

CDataFrame* HSCGadget::DoProcessing( const CDataFrame* pDataFrame )
{
  TCHAR WLAsString[ 100 ] ;
  if ( m_GadgetInfo == _T( "HSC" ) )
    GetGadgetName( m_GadgetInfo ) ;

  LPCTSTR pLabel = pDataFrame->GetLabel();
  if ( pDataFrame->GetDataType() == text )
  {
    if ( pLabel && (*pLabel) )
    {
      if ( _tcscmp( pLabel , "LoadConfig" ) == 0 )
      {
        FXString FileName = pDataFrame->GetTextFrame()->GetString() ;
        LoadConfigParameters( FileName ) ;
        return NULL ;
      }
      else if ( _tcscmp( pLabel , "SaveConfig" ) == 0 )
      {
        FXString FileName = pDataFrame->GetTextFrame()->GetString() ;
        SaveConfigParameters( FileName ) ;
        return NULL ;
      }
    }
  }
  if ( pLabel && (*pLabel) && (_tcsstr( pLabel , _T( "Start" ) ) == pLabel)
    && m_WaveLengths.GetCount() )
  {
    if ( _tcsstr( pLabel , _T( "StartByRatiosN" ) ) == pLabel )
    {
      m_WorkingMode = WMode_Recognition_Ratios_Norm;
      m_ScanMode = SCAN_Jumps ;
    }
    else if ( _tcsstr( pLabel , _T( "StartByRatios" ) ) == pLabel )
    {
      m_WorkingMode = WMode_Recognition_Ratios ;
      m_ScanMode = SCAN_Jumps ;
    }

    if ( m_ScanMode == SCAN_Jumps )
    {
      if ( m_Spectrums.size() == 0 )
      {
        SEND_GADGET_ERR( "No spectrums data" ) ;
        return NULL ;
      }
      int iInitResult = InitRecognitionForRatio() ;
      if ( iInitResult < 0 )
      {
        SEND_GADGET_ERR( "Init result %d" , iInitResult ) ;
        return NULL ;
      }
      m_LearnedData.clear() ;
      m_videoFramesMap.clear();
      //m_iCommandsCounter = 0;
      m_iNotActiveFrameCntr = 4;
      m_iCurrentWL = 0;
      m_bWasRecognition = true ;
      m_iRequestCount = 0 ;
      int iNextWaveLength = m_WaveLengths[ m_iCurrentWL ];
      SendNextWaveLength( iNextWaveLength , 2 , 1 );
      m_iLastOrderedWL = iNextWaveLength ;
      SEND_GADGET_INFO( "Start Recognition (%s) " , (LPCTSTR) m_sWaveLengths ) ;
    }
    return NULL;
  }
  else if ( (_tcsicmp( pLabel , _T( "Stop" ) ) == 0)
    && (m_ScanMode != SCAN_NoScan) )
  {
    m_iCurrentWL = -1;
    m_ScanMode = SCAN_NoScan ;
    m_WorkingMode = WMode_Teaching ;
    return NULL;
  }
  const CTextFrame * pTextFrame = pDataFrame->GetTextFrame();
  if ( pTextFrame )
  {
    if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
    {
      if ( _tcsicmp( pLabel , _T( "Teaching" ) ) == 0 )
      {
        FXAutolock al( m_Lock , _T( "HSCGadget::DoProcessing Teach" ) ) ;
        FXPropertyKit pk( pTextFrame->GetString() ) ;
        CRect Area ;
        if ( GetArray( pk , "Rect" , 'd' , 4 , &Area ) == 4 )
        {
          BOOL bRef = GetAsyncKeyState( VK_SHIFT ) & 0x8000 ;
          if ( bRef )
            //             m_BlackRefArea = Area ;
            m_NewRefArea = Area ;
          else
            m_TeachingArea = Area ;
          SEND_GADGET_INFO( "Teaching %s=(%d,%d,%d,%d)" ,
            (bRef) ? "Reference" : "Object" ,
            Area.left , Area.top , Area.right - Area.left , Area.bottom - Area.top ) ;
          m_iTeachingCounter = 0 ;
          m_bRefChanged = true;
        }
      }
      else if ( _tcsicmp( pLabel , _T( "Reference" ) ) == 0 )
      {
        FXPropertyKit pk( pTextFrame->GetString() );
        CRect Area;
        if ( GetArray( pk , "Rect" , 'd' , 4 , &Area ) == 4 )
        {
          FXAutolock al( m_Lock , _T( "HSCGadget::DoProcessing Ref" ) ) ;
          m_BlackRefArea = Area;
          m_bRefChanged = true;
          m_iTeachingCounter = 0;
        }
      }
      else if ( _tcsicmp( pLabel , _T( "BRef" ) ) == 0 )
      {
        FXPropertyKit pk( pTextFrame->GetString() );
        CRect Area;
        if ( GetArray( pk , "Rect" , 'd' , 4 , &Area ) == 4 )
        {
          FXAutolock al( m_Lock , _T( "HSCGadget::DoProcessing BRef" ) ) ;
          m_BlackRefArea = Area;
          m_bRefChanged = true;
          m_iTeachingCounter = 0;
        }
      }
      else if ( _tcsicmp( pLabel , _T( "WRef" ) ) == 0 )
      {
        FXPropertyKit pk( pTextFrame->GetString() );
        CRect Area;
        if ( GetArray( pk , "Rect" , 'd' , 4 , &Area ) == 4 )
        {
          FXAutolock al( m_Lock , _T( "HSCGadget::DoProcessing WRef" ) ) ;
          m_WhiteRefArea = Area;
          m_bRefChanged = true;
          m_iTeachingCounter = 0;
        }
      }
      //       else if ( Label == _T( "Teaching" ) )
      //       {
      //         FXPropertyKit pk( pTextFrame->GetString() );
      //         CRect Area;
      //         if ( GetArray( pk , "Rect" , 'd' , 4 , &Area ) == 4 )
      //         {
      //           m_TeachingArea = Area;
      //           m_bRefChanged = true;
      //           m_iTeachingCounter = 0;
      //         }
    }
  }
  FXString Label = pDataFrame->GetLabel();
  // Spectrum measurement result accumulation
  if ( Label.Find( _T( "Result" ) ) == 0 && m_WorkingMode == WMode_Teaching )
  {
    const CRectFrame * pRect = pDataFrame->GetRectFrame( _T( "BASE" ) );
    if ( pRect )
    {
      ASSERT( pRect->top != 0 ) ;
      m_BlackRefArea = *(RECT*) pRect;
    }
    pRect = pDataFrame->GetRectFrame( _T( "WBASE" ) );
    if ( pRect )
    {
      ASSERT( pRect->top != 0 ) ;
      m_WhiteRefArea = *(RECT*) pRect;
    }
    int iNumberPos = (int) Label.Find( _T( '_' ) ) + 1;
    if ( iNumberPos < (int) Label.GetLength() - 2 )
    {
      m_currentWaveLenth_nm = atoi( ((LPCTSTR) Label) + iNumberPos );
      CFramesIterator * it = pDataFrame->CreateFramesIterator( quantity );
      if ( it )
      {
        if ( m_bWasRecognition )
        {
          m_bWasRecognition = false ;
          m_Spectrums.clear() ;
        }
        CQuantityFrame * pNextFrame = NULL ;
        while ( pNextFrame = ((CQuantityFrame *) it->Next()) )
        {
          FXString ObjName = pNextFrame->GetLabel();
          int iColonPos = (int) ObjName.Find( ':' );
          if ( iColonPos >= 0 )
            ObjName = ObjName.Mid( iColonPos + 1 );
          Spectrum * pSpec = NULL;

          for ( int i = 0 ; i < (int) m_Spectrums.size() ; i++ )
          {
            if ( m_Spectrums[ i ].m_Name == (LPCTSTR) ObjName )
            {
              pSpec = &m_Spectrums.at( i ) ;
              break ;
            }
          }
          if ( !pSpec )
          {
            FXString ColorAsString ;
            DWORD dwColor = 0xff0000 ;
            if ( pNextFrame->Attributes()->GetString( "color" , ColorAsString ) )
              dwColor = (DWORD)ConvToBinary( ColorAsString );
            Spectrum NewSpec( ObjName , dwColor );
            NewSpec.m_iStepWL_nm = (int) m_dWaveLengthStep_nm ;
            m_Spectrums.push_back( NewSpec ) ;
            pSpec = &m_Spectrums[ m_Spectrums.size() - 1 ] ;
          }
          if ( pSpec )
          {
            SpectrumComponent NewComp( m_currentWaveLenth_nm , (double) (*pNextFrame) ) ;
            pSpec->Add( NewComp ) ;
          }
        }
        delete it;
      }
      m_bSpectrumsChanged = true ;
    }
  }

  if ( m_ScanMode == SCAN_NoScan )
    return NULL ;

  const CVideoFrame * pVF = pDataFrame->GetVideoFrame() ;
  CDataFrame * pOutData = NULL ;
  if ( pVF && (m_WorkingMode != WMode_Teaching) )
  {
    FXAutolock al( m_Lock , _T( "HSCGadget::DoProcessing NoTeach" ) ) ;
    CalcTargetRatios();
    bool bWaveLengthExtracted = false ;
    if ( m_ScanMode == SCAN_Jumps )
    {
      double dWaitForVFTime = pVF->GetAbsTime() - m_dLastWLOrderTime ;
      bool bBadStatus = dWaitForVFTime > 300. || dWaitForVFTime < 30.;
      if ( !bBadStatus )
      {
        if ( !ExtractWaveLength( pVF ) )
          bBadStatus = true ;
      else
        bBadStatus |= (m_currentWaveLenth_nm != m_iLastOrderedWL) ;
      }

      if ( bBadStatus )
      {
        // repeat WL ordering
        m_currentWaveLenth_nm = m_WaveLengths[ m_iCurrentWL = 0 ];
        SendNextWaveLength( m_currentWaveLenth_nm , 2 , 1 );
        m_iLastOrderedWL = m_currentWaveLenth_nm ;
        return NULL ;
      }
        AddImageToRepository( pVF ) ;


      bWaveLengthExtracted = true ;
    }
    CVideoFrame * pCopy = NULL ;
    COutputConnector * pVCopyPin = GetOutputConnector( 3 ) ;
    if ( bWaveLengthExtracted )
    {
      bool bNotNormalized = (m_WorkingMode == WMode_Recognition_Ratios) ;
      switch ( m_WorkingMode )
      {
        case WMode_CSVM:
        {
          int iChan = GetChannel( m_currentWaveLenth_nm ) ;
          m_Detector.CalcIntegralImage( pVF , iChan ) ;
          return RecognizeCSVM() ;
        }
        case WMode_Teaching:
        case WMode_Recognition_Amplitudes:
        case WMode_Recognition_Ratios:
        case WMode_Recognition_Ratios_Norm:
        {
          if ( m_ScanMode != SCAN_Jumps )
            AddImageToRepository( pVF );
          if ( (bNotNormalized || !m_WhiteRefArea.IsRectEmpty())
            && !m_BlackRefArea.IsRectEmpty()
            && (m_videoFramesMap.size() >= (UINT) m_WaveLengths.GetCount()) )
          {
            if ( m_WorkingMode != WMode_Teaching )
            {
              pOutData = RecognizeByRatioPatterns();
            }
          }
        }
        break;
      }
      if ( pVCopyPin->IsConnected() )
      {
        FXString DebugMsg ;
        DebugMsg.Format( "WL=%d nm, Src=%s(type=%s)" , 
          m_currentWaveLenth_nm , pDataFrame->GetLabel() ,
          Tvdb400_TypeToStr( pDataFrame->GetDataType() ) ) ;
        //_itot_s( m_currentWaveLenth_nm , WLAsString , 10 ) ;
        CTextFrame * pDebug = CreateTextFrame( 
          (LPCTSTR)DebugMsg , "LastWL" , pVF->GetId() ) ;
        PutFrame( pVCopyPin , pDebug ) ;
      }
      sprintf_s( WLAsString , "%d" , m_currentWaveLenth_nm ) ;

      // Processing is finished, switch to next wave length
    m_iCurrentWL = ++m_iCurrentWL % m_WaveLengths.GetCount();
    m_currentWaveLenth_nm = m_WaveLengths[ m_iCurrentWL ];

    }
    else // something worng, try to restore sequence
    {

    }
    // Send request for capturing
    SendNextWaveLength( m_currentWaveLenth_nm , 2 , 1 );
    m_iLastOrderedWL = m_currentWaveLenth_nm ;
    if ( pVCopyPin->IsConnected() )
    {
      pCopy = (CVideoFrame*) pVF->Copy() ;
      pCopy->SetLabel( (LPCTSTR) WLAsString ) ;
      PutFrame( pVCopyPin , pCopy ) ;
    }
  }

  return pOutData ;
  //return (CDataFrame*) pDataFrame;
}
void HSCGadget::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( pConnector == m_pDuplexConnector )
  {
    const CTextFrame * pText = pParamFrame->GetTextFrame() ;

    if ( pText )
    {
      FXString Label = pText->GetLabel() ;
      if ( !Label.IsEmpty() )
      {
        if ( Label == "Teaching" )
        {
          m_WorkingMode = WMode_Teaching ;
          return ;
        }
        else if ( Label == "Recognition" )
        {
          if ( !pText->GetString().IsEmpty() )
          {
            FXIntArray NewWLs ;
            if ( GetArray( pText->GetString() , _T( 'i' ) , &NewWLs ) )
            {
              FXAutolock al( m_Lock , _T( "HSCGadget::AsyncTransaction" ) ) ;
              m_bUseInternal = TRUE ;
              m_WaveLengths.Copy( NewWLs ) ;
              m_iNotActiveFrameCntr = m_iCurrentWL = 0;
              m_videoFramesMap.clear();
            }
          }

          m_WorkingMode = WMode_Recognition_Ratios_Norm ;
          return ;
        }
      }
      bool bDummy = false ;
      UserGadgetBase::ScanProperties( pText->GetString() , bDummy ) ;
    }
  }
  pParamFrame->Release( pParamFrame );
};
void HSCGadget::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "WorkingMode" ) , &m_WorkingMode ,
    SProperty::Int , _T( "CSVM;Teaching;Sniff.-Ampl.;Sniff.-Ratios;Sniff.-RatiosN;" ) ) ;
  addProperty( SProperty::SPIN , _T( "NChannels" ) , &m_iNChannels ,
    SProperty::Int , 1 , 100 ) ;
  addProperty( SProperty::EDITBOX , _T( "Epsilon" ) , &m_dEpsilon ,
    SProperty::Double );
  addProperty( SProperty::EDITBOX , _T( "dAmplThres" ) , &m_dAmplThres ,
    SProperty::Double );
  addProperty( SProperty::EDITBOX , _T( "WaveLengths" ) ,
    &m_sWaveLengths , SProperty::String );
  addProperty( SProperty::EDITBOX , _T( "CommandFormat" ) ,
    &m_sCommandFormat , SProperty::String );

  addProperty( SProperty::EDITBOX , _T( "ConfigFile" ) ,
    &m_ConfigFileName , SProperty::String );
  addProperty( SProperty::SPIN , _T( "WriteConfig" ) , &m_bWrite ,
    SProperty::Int , 0 , 1 ) ;

  SetChangeNotification(
    _T( "ConfigFile" ) , ConfigParamChange , this ) ;
  SetChangeNotification(
    _T( "WorkingMode" ) , ConfigParamChange , this ) ;
  SetChangeNotification(
    _T( "WriteConfig" ) , ConfigParamChange , this ) ;
  SetChangeNotification(
    _T( "WaveLengths" ) , ConfigParamChange , this ) ;
};

void HSCGadget::ConfigParamChange( LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bRescanProperties)
{
  HSCGadget * pGadget = (HSCGadget*) pObject ;
  if ( pGadget )
  {
    if ( !_tcsicmp( pName , _T( "ConfigFile" ) ) )
      pGadget->SetConfigParameters( pGadget->m_ConfigFileName ) ;

    if ( !_tcsicmp( pName , _T( "WorkingMode" ) ) )
    {
      if ( pGadget->m_OldWorkingMode != WMode_Teaching
        && pGadget->m_WorkingMode == WMode_Teaching )
      {
        pGadget->m_Spectrums.clear() ;
      }
      if ( pGadget->m_OldWorkingMode == WMode_Teaching
        && (pGadget->m_WorkingMode == WMode_Recognition_Amplitudes
        || pGadget->IsWorkingByRatios()) )
      {
        pGadget->SetConfigParameters( pGadget->m_ConfigFileName , true ) ;
        if ( pGadget->IsWorkingByRatios() )
        {
          pGadget->m_videoFramesMap.clear();
          //m_iCommandsCounter = 0;
          pGadget->m_iNotActiveFrameCntr = 0;
          pGadget->m_iCurrentWL = 0;
          pGadget->InitRecognitionForRatio() ;
          int iNextWaveLength = pGadget->m_WaveLengths[ 0 ];
          pGadget->SendNextWaveLength( iNextWaveLength , 2 , 1 );
        }
      }
      if ( pGadget->m_WorkingMode == WMode_Teaching )
      {
        pGadget->m_LearnedData.clear() ;
        pGadget->m_iTeachingCounter = 0 ;
      }
      pGadget->m_OldWorkingMode = pGadget->m_WorkingMode ;
    }
    if ( !_tcsicmp( pName , _T( "WriteConfig" ) ) )
    {
      pGadget->SetConfigParameters( pGadget->m_ConfigFileName , true ) ;
    }
    if ( !_tcsicmp( pName , _T( "WaveLengths" ) ) )
    {
      FXIntArray NewWLs ;
      if ( GetArray( pGadget->m_sWaveLengths , _T( 'i' ) , &NewWLs ) )
      {
        pGadget->m_bUseInternal = TRUE ;
        pGadget->m_WaveLengths.Copy( NewWLs ) ;
        pGadget->m_iNotActiveFrameCntr = pGadget->m_iCurrentWL = 0;
        pGadget->m_videoFramesMap.clear();
      }
    }
  }
}

void HSCGadget::ConnectorsRegistration()
{
  addInputConnector( transparent , "Image" );
  addOutputConnector( transparent , "Result" );
  addOutputConnector( transparent , "FrequencyAsNumber" );
  addOutputConnector( text , "Command" );
  addOutputConnector( transparent  , "Debug & LabeledVideo" ) ;


  //addInputConnector( createComplexDataType(2, rectangle, text), "InputName2");
  //addOutputConnector( text , "OutputName2");
  //addOutputConnector( createComplexDataType(3, rectangle, text, vframe) , "OutputName3");
  //addDuplexConnector( transparent, transparent, "DuplexName1");
  //addDuplexConnector( transparent, createComplexDataType(3, rectangle, text, vframe), "DuplexName2");
}
bool HSCGadget::ExtractWaveLength( const CDataFrame* pDataFrame )
{
  //update
  FXString Label = pDataFrame->GetLabel() ;
  if ( !Label.IsEmpty() )
  {
    int iWaveLength = atoi( Label ) ;
    if ( (iWaveLength != 0) && (iWaveLength == m_iLastOrderedWL) )
    {
      for ( int i = 0; i < m_WaveLengths.GetCount() ; i++ )
      {
        if ( iWaveLength == m_WaveLengths[ i ] )
        {
          m_currentWaveLenth_nm = iWaveLength ;
          return true ;
        }
      }
    }
    SEND_GADGET_ERR( "Bad wave length label: %s" , (LPCTSTR) Label ) ;
  }
  return false ;
}

int HSCGadget::GetChannel( int iWaveLength_nm )
{
  for ( int i = 0 ; i < m_Detector.m_dWaveLengths.GetCount() ; i++ )
  {
    if ( fabs( iWaveLength_nm - m_Detector.m_dWaveLengths[ i ] ) < 1. )
      return i ;
  }
  return -1 ;
}

int  HSCGadget::ExtractObjects( const CDataFrame* pDataFrame )
{
  m_objectsArr.RemoveAll();

  //m_masterWaveLenth_nm
  //fill m_objectsMap
  //m_masterWaveLenth_nm
  return (int) m_objectsArr.GetCount();
}

int HSCGadget::AddSample( int iWaveLength_nm , Sample& NewSample )
{
  SampleMap::iterator it = m_LearnedData.find( iWaveLength_nm ) ;
  if ( it != m_LearnedData.end() )
  {
    if ( m_iTeachingCounter > 0 )
    {
      double dK = (double) m_iTeachingCounter / (double) (m_iTeachingCounter + 1) ;
      NewSample.m_dAver = NewSample.m_dAver * dK
        + it->second.m_dAver * (1. - dK) ;
      NewSample.m_dRefBlack = NewSample.m_dRefBlack * dK
        + it->second.m_dRefBlack * (1. - dK) ;
      NewSample.m_dRefWhite = NewSample.m_dRefWhite * dK
        + it->second.m_dRefWhite * (1. - dK) ;
      NewSample.m_dStd = NewSample.m_dStd * dK
        + it->second.m_dStd * (1. - dK) ;
    }
  }
  m_LearnedData[ iWaveLength_nm ] = NewSample ;
  m_iTeachingCounter++ ;
  return (int) m_LearnedData.size() ;
}
int  HSCGadget::AddImageToRepository( const CDataFrame* pDataFrame )
{
  CVideoFrame* pNew = (CVideoFrame*) pDataFrame->GetVideoFrame()->Copy();
  // m_Lock.Lock();
  VFMap::iterator it = m_videoFramesMap.find( m_currentWaveLenth_nm );
  if ( it != m_videoFramesMap.end() )
  {
    CVideoFrame * pOld = it->second ;
    pOld->Release() ;
  }
  m_videoFramesMap[ m_currentWaveLenth_nm ] = pNew ;

  //m_Lock.Unlock();
  return (int) m_videoFramesMap.size();
}
void HSCGadget::Compare()
{
  if ( m_Ratios.size() == 0 )
    return;
  CreateMatrix();
}
void HSCGadget::CreateMatrix()
{
  int targetBrightness = (int) 256 / (int) m_objectsArr.GetSize();
  for ( int i = 0; i < m_objectsArr.GetSize(); i++ )
  {
    ObjectParams desiredObject = m_objectsArr.GetAt( i );
    for ( std::map<int , FXArray<double>>::iterator it = m_Ratios.begin(); it != m_Ratios.end(); ++it )
    {
      FXArray<double> ratios = it->second;
      UpdateMatrix( &desiredObject , &ratios , targetBrightness * (i + 1) );
    }
  }
}
bool HSCGadget::VerifyWaveLengths()
{
  if ( m_objectsArr.GetSize() == 0 ) return false;
  int counter = 0;
  for ( int i = 0; i < m_objectsArr[ 0 ].GetSize(); i++ )
  {
    if ( FindLambdaInImageRepo( m_objectsArr[ 0 ].GetAt( i ).currentLambda ) )
      counter++;
    else
      break;
  }

  return (counter == m_videoFramesMap.size()) ? true : false;
}
bool HSCGadget::FindLambdaInImageRepo( int lambda )
{
  for ( std::map<int , CVideoFrame*>::iterator it = m_videoFramesMap.begin();
    it != m_videoFramesMap.end();
    ++it )
  {
    if ( it->first == lambda )
      return true;
  }
  return false;
}
int  HSCGadget::CalculateRatio()
{
  std::map<int , CVideoFrame*> ::iterator it;
  it = m_videoFramesMap.find( m_masterWaveLenth_nm );
  CVideoFrame* pMasterVideoFrame;
  if ( it != m_videoFramesMap.end() )
  {
    pMasterVideoFrame = m_videoFramesMap[ m_masterWaveLenth_nm ];
  }

  m_Ratios.clear();
  int counter = 0;
  for ( std::map<int , CVideoFrame*>::iterator it = m_videoFramesMap.begin(); it != m_videoFramesMap.end(); ++it )
  {
    if ( it->first == m_masterWaveLenth_nm ) continue;

    CVideoFrame* pVideoFrame = it->second;
    CalculateRatio( pMasterVideoFrame , pVideoFrame , it->first );
    counter++;
  }

  return counter;
}
void HSCGadget::CalculateRatio( const CVideoFrame* pMasterFrame , const CVideoFrame* pVideoFrame , int waveLength )
{
  LPBITMAPINFOHEADER pMasterInfoHeader = pMasterFrame->lpBMIH;
  DWORD masterImgCompression = pMasterInfoHeader->biCompression;
  LONG masterImgWidth = pMasterInfoHeader->biWidth;
  LONG masterImgHeight = pMasterInfoHeader->biHeight;
  LPBYTE masterImg = GetData( pMasterFrame );
  LONG masterImgSize = (masterImgHeight - 2) * masterImgWidth;


  LPBITMAPINFOHEADER pInfoHeader = pVideoFrame->lpBMIH;
  DWORD imgCompression = pInfoHeader->biCompression;
  LONG imgWidth = pInfoHeader->biWidth;
  LONG imgHeight = pInfoHeader->biHeight;
  LPBYTE img = GetData( pVideoFrame );
  LONG imgSize = (imgHeight - 2) * imgWidth;


  ASSERT( masterImgCompression == imgCompression );
  ASSERT( masterImgSize == imgSize );


  FXArray<double> lambda1toLambdaNRatio;
  switch ( imgCompression )
  {
    case BI_Y8:
    case BI_Y800:
    case BI_YUV9:
    case BI_YUV12:
    {
      //masterImg += imgWidth; // Doesn't include first row
      //img += imgWidth; // Doesn't include first row

      for ( LONG uiCnt = 0; uiCnt < imgSize; uiCnt++ )
      {
        double d = masterImg[ uiCnt ] / img[ uiCnt ];
        lambda1toLambdaNRatio.Add( d );
      }
    }
    break;
    case BI_Y16:
    {
      //masterImg += imgWidth * 2; // omit first row, 2 bytes per pixel
      //img += imgWidth * 2;


      LPWORD pMaster16 = (LPWORD) masterImg;
      LPWORD p16 = (LPWORD) img;

      for ( LONG uiCnt = 0; uiCnt < imgSize; uiCnt++ )
      {
        double d = pMaster16[ uiCnt ] / p16[ uiCnt ];
        lambda1toLambdaNRatio.Add( d );
      }
    }
    break;
  }


  std::map<int , FXArray<double>> ::iterator it;
  it = m_Ratios.find( waveLength );
  ASSERT( it != m_Ratios.end() );


  m_Ratios[ waveLength ] = lambda1toLambdaNRatio;
}
void HSCGadget::UpdateMatrix( const ObjectParams* pMaster , const FXArray<double>* pRatios , int brightness )
{
  //LPBYTE  *szHash = (BYTE*)malloc(48);
  //m_outputData
  for ( int i = 0; i < pRatios->GetSize(); i++ )
  {
    double ratio = pRatios->GetAt( i );

    int counter = 0;
    for ( int j = 0; j < pMaster->GetSize(); j++ )
    {
      if ( (pMaster->GetAt( j ).k1 - m_dEpsilon <= ratio) && (ratio <= pMaster->GetAt( j ).k1 + m_dEpsilon) )
        counter++;
      else
      {
        break;
      }
    }

    m_outputData[ i ] = (counter == pMaster->GetSize()) ? brightness : 0;
  }
}
int  HSCGadget::Clear()
{
  SetChangeNotification(
    _T( "ConfigFile" ) , NULL , this ) ;
  SetChangeNotification(
    _T( "WorkingMode" ) , NULL , this ) ;
  SetChangeNotification(
    _T( "WriteConfig" ) , NULL , this ) ;

  m_Lock.Lock();
  int ret = (int) m_videoFramesMap.size();
  std::map<int , CVideoFrame*>::iterator it = m_videoFramesMap.begin();
  for ( ; it != m_videoFramesMap.end(); ++it )
  {
    it->second->Release();
  }
  m_videoFramesMap.clear();
  m_Ratios.clear();
  m_Lock.Unlock();
  return ret;
}
CVideoFrame* HSCGadget::RecognizeCSVM()
{
  if ( m_Detector.IsReady() )
  {
    m_Detector.Detect() ;
    CVideoFrame * pOut = CVideoFrame::Create() ;
    int iSize = m_Detector.m_ImageSize.cx * m_Detector.m_ImageSize.cy ;
    pOut->lpBMIH = (LPBITMAPINFOHEADER) new BYTE[ sizeof( BITMAPINFOHEADER ) + iSize ] ;
    pOut->lpData = NULL ;
    pOut->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
    pOut->lpBMIH->biBitCount = 8 ;
    pOut->lpBMIH->biCompression = BI_Y8 ;
    pOut->lpBMIH->biPlanes = 1;
    pOut->lpBMIH->biWidth = m_Detector.m_ImageSize.cx ;
    pOut->lpBMIH->biHeight = m_Detector.m_ImageSize.cy ;
    pOut->lpBMIH->biSizeImage = iSize ;
    pOut->lpBMIH->biClrImportant = pOut->lpBMIH->biClrUsed =
      pOut->lpBMIH->biXPelsPerMeter = pOut->lpBMIH->biYPelsPerMeter = 0 ;
    memcpy( GetData( pOut ) , m_Detector.m_pResult , pOut->lpBMIH->biSizeImage ) ;
    pOut->ChangeId( m_dwFrameCnt++ ) ;
    return pOut ;
  }

  return NULL ;
}

CVideoFrame* HSCGadget::RecognizeSimple()
{
  if ( m_videoFramesMap.size() == (UINT) m_WaveLengths.GetCount() )
  {
    FXAutolock al( m_Lock , _T( "HSCGadget::RecognizeSimple" ) ) ;
    VFMap::iterator VFIt = m_videoFramesMap.begin() ;
    CVideoFrame * pFr = VFIt->second ;
    int iWidth = GetWidth( pFr ) ;
    int iHeight = GetHeight( pFr ) ;
    CVideoFrame * pOut = CVideoFrame::Create() ;
    int iSize = iWidth * iHeight ;
    pOut->lpBMIH = (LPBITMAPINFOHEADER) new BYTE[ sizeof( BITMAPINFOHEADER ) + iSize ] ;
    pOut->lpData = NULL ;
    pOut->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
    pOut->lpBMIH->biBitCount = 8 ;
    pOut->lpBMIH->biCompression = BI_Y8 ;
    pOut->lpBMIH->biPlanes = 1;
    pOut->lpBMIH->biWidth = iWidth ;
    pOut->lpBMIH->biHeight = iHeight ;
    pOut->lpBMIH->biSizeImage = iSize ;
    pOut->lpBMIH->biClrImportant = pOut->lpBMIH->biClrUsed =
      pOut->lpBMIH->biXPelsPerMeter = pOut->lpBMIH->biYPelsPerMeter = 0 ;

    LPBYTE pOutPixel = GetData( pOut ) ;
    memset( pOutPixel , 0 , iSize ) ;

    bool bIs16Bits = Is16BitsImage( pFr ) ;

    FXArray<LPBYTE> Images ;
    FXArray<Sample> Samples ;
    FXDblArray BReferences , WReferences;
    FXDblArray Ratios ;
    double dMaxStd = 0. ;
    double dMaxAmpl = 0. ;
    int iMaxAmplIndex = -1 ;
    while ( VFIt != m_videoFramesMap.end() )
    {
      Images.Add( GetData( VFIt->second ) ) ;
      SampleMap::iterator SIt = m_LearnedData.find( VFIt->first ) ;
      if ( SIt != m_LearnedData.end() )
      {
        TRACE( "\nWL=%d Av=%6.1f Std=%5.1f RefB=%6.1f RefW=%6.1f MaxStd=%5.1f" ,
          SIt->first , SIt->second.m_dAver , SIt->second.m_dStd ,
          SIt->second.m_dRefBlack , SIt->second.m_dRefWhite , dMaxStd ) ;
        double dBRef = GetAverage( VFIt->second , m_BlackRefArea ) ;
        BReferences.Add( dBRef ) ;
        double dWRef = GetAverage( VFIt->second , m_WhiteRefArea ) ;
        WReferences.Add( dWRef ) ;

        double dRatio = (SIt->second.m_dAver - dBRef) / (dWRef - dBRef) ;

        SIt->second.m_dNormalizedByRef = SIt->second.m_dAver * dRatio ;
        //        SIt->second.m_dNormalizedByRef = SIt->second.m_dAver * dRef / SIt->second.m_dRef ;
        if ( dMaxAmpl < SIt->second.m_dNormalizedByRef )
        {
          dMaxAmpl = SIt->second.m_dNormalizedByRef ;
          iMaxAmplIndex = (int) Samples.GetCount() ;
        }
        if ( dMaxStd < SIt->second.m_dStd )
          dMaxStd = SIt->second.m_dStd ;
        Samples.Add( SIt->second ) ;
        VFIt++ ;
        continue ;
      }
      else
        return NULL ;
    }
  #define Clearance (10)

    switch ( m_WorkingMode )
    {
      case WMode_Recognition_Amplitudes:
      {
        for ( int iY = Clearance ; iY < iHeight - Clearance ; iY++ )
        {
          int iShift = iY * iWidth + Clearance ;
          int iEnd = iShift + iWidth - Clearance * 2 ;
          while ( iShift < iEnd )
          {
            double dDist = 0 ;
            for ( int iWaveLength = 0 ; iWaveLength < Samples.GetCount() ; iWaveLength++ )
            {
              LPBYTE pCurrentImage = Images[ iWaveLength ] ;
              double dIntens = (bIs16Bits) ?
                *(((LPWORD) pCurrentImage) + iShift) : *(pCurrentImage + iShift) ;
              dIntens -= Samples[ iWaveLength ].m_dRefBlack ;

              double dSideDist = dIntens - Samples[ iWaveLength ].m_dNormalizedByRef ;
              if ( Samples[ iWaveLength ].m_dStd > 1e-6 )
              {
                double dNormDist = dSideDist ;
                double d2Dist = dNormDist * dNormDist ;
                dDist += d2Dist ;
              }
            }
            if ( dDist < m_dEpsilon )
              *(pOutPixel + iShift) = 255 ;
            iShift++ ;
          }
        }
      }
      break ;
      case WMode_Recognition_Ratios:
      {
        for ( int i = 0 ; i < Samples.GetCount() ; i++ )
        {
          Samples[ i ].m_dRatio = Samples[ i ].m_dNormalizedByRef / dMaxAmpl ;
        }
        LPBYTE pMaxAmplImage = Images[ iMaxAmplIndex ] ;
        for ( int iY = Clearance ; iY < iHeight - Clearance ; iY++ )
        {
          int iShift = iY * iWidth + Clearance ;
          int iEnd = iShift + iWidth - Clearance * 2 ;
          while ( iShift < iEnd )
          {
            double dDist = 0 ;
            double dAmplOnMaxIndex = (bIs16Bits) ?
              *(((LPWORD) pMaxAmplImage) + iShift) : *(pMaxAmplImage + iShift) ;
            for ( int iWaveLength = 0 ; iWaveLength < Samples.GetCount() ; iWaveLength++ )
            {
              LPBYTE pCurrentImage = Images[ iWaveLength ] ;
              double dIntens = (bIs16Bits) ?
                *(((LPWORD) pCurrentImage) + iShift) : *(pCurrentImage + iShift) ;
              dIntens -= Samples[ iWaveLength ].m_dRefBlack ;
              double dRatio = dIntens / dAmplOnMaxIndex ;
              double dThisDist = dRatio - Samples[ iWaveLength ].m_dRatio ;
              double d2Dist = dThisDist * dThisDist ;
              dDist += d2Dist ;
            }
            if ( dDist < m_dEpsilon )
              *(pOutPixel + iShift) = 255 ;
            iShift++ ;
          }
        }
      }
      break ;

    }

    pOut->ChangeId( m_dwFrameCnt++ ) ;
    return pOut ;
  }

  return NULL ;
}

CVideoFrame* HSCGadget::RecognizeByRatioPatterns()
{
  int iNActivWLs = 0;

  //     vector<LPBYTE> Images;
  //     vector<double> BReferences, WReferences , AmplRef ;
#define MAX_N_WaveLengths 20
  LPBYTE Images[ MAX_N_WaveLengths ];
  double BRefs[ MAX_N_WaveLengths ];
  double WRefs[ MAX_N_WaveLengths ];
  double Ampls[ MAX_N_WaveLengths ];
  bool bNormalize = (m_WorkingMode != WMode_Recognition_Ratios) ;
  if ( ++m_iNotActiveFrameCntr < m_WaveLengths.GetCount() )
  {
    //SendNextWaveLength( iNextWaveLength , 2 , 1 );
    return NULL;
  }
  else
    m_iNotActiveFrameCntr = 0;

  if ( (UINT) m_WaveLengths.GetCount() <= m_videoFramesMap.size() )
  {
    VFMap::iterator VFIt = m_videoFramesMap.begin();
    CVideoFrame * pFr = VFIt->second;
    bool bIs16Bits = Is16BitsImage( pFr );
    int iWidth = GetWidth( pFr );
    int iHeight = GetHeight( pFr );
    CVideoFrame * pOut = CVideoFrame::Create();
    if ( !pOut )
      return NULL ;

    int iImSize = iWidth * iHeight;
    int iSize = iImSize * 3 ; // simple interleaved RGB format
//     int iUorVSize = (iSize - iImSize) / 2;
// 
    pOut->lpBMIH = (LPBITMAPINFOHEADER) new BYTE[ sizeof( BITMAPINFOHEADER ) + iSize ];
    pOut->lpData = NULL;
    pOut->lpBMIH->biSize = sizeof( BITMAPINFOHEADER );
    pOut->lpBMIH->biBitCount = 24;
    pOut->lpBMIH->biCompression = BI_RGB;
    pOut->lpBMIH->biPlanes = 1;
    pOut->lpBMIH->biWidth = iWidth;
    pOut->lpBMIH->biHeight = iHeight;
    pOut->lpBMIH->biSizeImage = iSize;
    pOut->lpBMIH->biClrImportant = pOut->lpBMIH->biClrUsed =
      pOut->lpBMIH->biXPelsPerMeter = pOut->lpBMIH->biYPelsPerMeter = 0;

    LPBYTE pOutPixel = GetData( pOut );
    //     LPBYTE pU = pOutPixel + iImSize;
    //     LPBYTE pV = pU + iUorVSize;

    memset( pOutPixel , 0 , iSize );
    //     memset(pU, 128, iSize - iImSize);


    for ( int i = 0; i < m_WaveLengths.GetCount(); i++ )
    {
      VFMap::iterator FoundVF = m_videoFramesMap.find( m_WaveLengths[ i ] );
      if ( FoundVF != m_videoFramesMap.end() )
      {
        //         Images.push_back(GetData(FoundVF->second));
        //         double dBRef = GetAverage(FoundVF->second, m_BlackRefArea);
        //         BReferences.push_back(dBRef);
        //         double dWRef = GetAverage(FoundVF->second, m_WhiteRefArea);
        //         WReferences.push_back(dWRef);
        //         double dDiff = dWRef - dBRef;
        //         AmplRef.push_back((dDiff > 1.) ? dDiff : 1.);
        Images[ iNActivWLs ] = GetData( FoundVF->second );
        BRefs[ iNActivWLs ] = GetAverage( FoundVF->second , m_BlackRefArea );
        if ( bNormalize )
        {
          WRefs[ iNActivWLs ] = GetAverage( FoundVF->second , m_WhiteRefArea );
          double dDiff = WRefs[ iNActivWLs ] - BRefs[ iNActivWLs ];
          Ampls[ iNActivWLs ] = (dDiff > 1.) ? dDiff : 1. ;
        }
        if ( ++iNActivWLs >= MAX_N_WaveLengths )
          break;
      }
      else
      {
        pOut->Release();
        m_videoFramesMap.clear();
        m_iNotActiveFrameCntr = 0;
        int iNextWaveLength = m_WaveLengths[ m_iCurrentWL = 0 ];
        //SendNextWaveLength( iNextWaveLength, 2 , 1 );
        return NULL;
      }
    }

  #define Clearance (10)
  #define MAX_NOBJECTS (50)
    AllSpectrums DebugData;
    memset( &DebugData , 0 , sizeof( DebugData ) );
    switch ( m_WorkingMode )
    {
      case WMode_Recognition_Ratios:
      case WMode_Recognition_Ratios_Norm:
      {
        double Dists[ MAX_NOBJECTS ];
        int iNObjects = 0;
        for ( int iY = Clearance; iY < iHeight - Clearance; iY++ ) // Y cycle
        {
          int iYShift = iY * iWidth;
          int iShift = iYShift + Clearance;
          int iEnd = iShift + iWidth - Clearance * 2;
          int iOutShift = (iHeight - iY) * iWidth * 3;
          int iX = Clearance;
          double dIntens;
          while ( iShift < iEnd )  // X Cycle
          {
            memset( Dists , 0 , sizeof( Dists ) );
            double * pBlack = BRefs;
            double * pAmpl = Ampls;
            double * pDistPtr ;
            double dLastRatios[ MAX_NComponents ];
            double dLastMax = -10.;
            double dDistMin = 1000.;
            bool bEnoughAmpl = false ;
            size_t iMinIndex ;
            for ( int iWaveLength = 0; iWaveLength < m_WaveLengths.GetCount(); iWaveLength++ )
            {
              LPBYTE pCurrentImage = Images[ iWaveLength ];
              dIntens = (bIs16Bits) ?
                *(((LPWORD) pCurrentImage) + iShift) : *(pCurrentImage + iShift);
              double dVal = dIntens - *(pBlack++);
              if ( dVal > *pAmpl * m_dAmplThres )
                bEnoughAmpl = true ;
              if ( bNormalize )
                dVal /= *(pAmpl++) ;
              dLastRatios[ iWaveLength ] = dVal ;
              if ( dLastMax < dVal )
                dLastMax = dVal;
            }
            if ( bEnoughAmpl && (dLastMax > 0.) )
            {
              for ( int iWaveLength = 0; iWaveLength < m_WaveLengths.GetCount(); iWaveLength++ )
                dLastRatios[ iWaveLength ] /= dLastMax;

              for ( int iWaveLength = 0; iWaveLength < m_WaveLengths.GetCount(); iWaveLength++ )
              {
                pDistPtr = Dists;
                for ( size_t iPattern = 0; iPattern < m_ActiveSpectrums.size(); iPattern++ )
                {
                  int iActiveIndex = m_ActiveSpectrums[ iPattern ];
                  Spectrum& PatternSpectrum = m_Spectrums[ iActiveIndex ];
                  double dPatternValue = PatternSpectrum.m_CurrentRecognition[ iWaveLength ];
                  double dDist = dPatternValue - dLastRatios[ iWaveLength ];
                  *(pDistPtr++) += dDist * dDist;
                  //                 DebugData.Values[iPattern].Spectrumindex = iPattern;
                  //                 DebugData.Values[iPattern].ActiveIndex = iActiveIndex;
                  //                 strcpy_s( DebugData.Values[iPattern].Name , PatternSpectrum.m_Name.c_str() ) ;
                  //                 DebugData.Values[iPattern].dPattern[iWaveLength] = dPatternValue;
                  //                 DebugData.Values[iPattern].dValues[iWaveLength] = dLastRatios[iWaveLength];
                  //                 DebugData.Values[iPattern].dNotNormValues[iWaveLength] = dLastRatios[iWaveLength] * dLastMax ;
                  //                 DebugData.Values[iPattern].dDiffs[iWaveLength] = dLastRatios[iWaveLength] - dPatternValue;
                }
              }
              double dDist = dDistMin = sqrt( Dists[0] / m_ActiveSpectrums.size() );
              DebugData.Values[ 0 ].dDiff = dDist;
              size_t iIndex = 1 ;
              iMinIndex = 0 ;
              pDistPtr = Dists + 1 ;

              for ( ; iIndex < m_ActiveSpectrums.size(); pDistPtr++ , iIndex++ )
              {
                dDist = DebugData.Values[ iIndex ].dDiff =
                  sqrt( *pDistPtr / m_ActiveSpectrums.size() );
                if ( dDist < dDistMin )
                {
                  dDistMin = dDist;
                  iMinIndex = iIndex;
                }
              }
            }
            LPBYTE pOutY = pOutPixel + iOutShift + iX * 3 ; // RGB takes 3 bytes per pixel
            if ( bEnoughAmpl && dDistMin <= m_dEpsilon )
            {
              Spectrum& FoundSpectrum = m_Spectrums[ m_ActiveSpectrums[ iMinIndex ] ];
              // We do use RGB color, BMP image has BGR color
              LPBYTE pColor = ((LPBYTE) &FoundSpectrum.m_Color) + 2;
              *(pOutY++) = *(pColor--);
              *(pOutY++) = *(pColor--);
              *(pOutY) = *(pColor);
            }
            else
            {
              BYTE bVal = (BYTE) (dIntens / 12.); //
              *(pOutY++) = bVal;
              *(pOutY++) = bVal; //
              *(pOutY++) = bVal ; //
            }
            iShift++;
            iX++;
          } // end of iX cycle
        }   // end of iY cycle
      }
      break;

    }
    //if ( (m_iCurrentWL >= 0) && m_WaveLengths.GetCount() )
    //{
    //  m_iCurrentWL = ++m_iCurrentWL % m_WaveLengths.GetCount();
    //  m_iNotActiveFrameCntr = 0;
    //  int iNextWaveLength = m_WaveLengths[m_iCurrentWL];
    //  //SendNextWaveLength( iNextWaveLength , 2 , 1 );
    //}
    //else
    //{
    //  CQuantityFrame * pGiveImage = CQuantityFrame::Create(0);
    //  PutFrame(GetOutputConnector(2), pGiveImage);
    //}


    pOut->ChangeId( m_dwFrameCnt++ );
    return pOut;
  }

  return NULL;
}


int HSCGadget::CalcTargetRatios()
{
  if ( !m_bSpectrumsChanged || !m_WaveLengths.GetCount() )
    return 0 ;

  m_bSpectrumsChanged = false ;
  m_ActiveSpectrums.clear();
  for ( int i = 0; i < (int) m_Spectrums.size(); i++ )
  {
    m_Spectrums[ i ].m_CurrentRecognition.clear();
    vector <double> Values ;
    for ( int iWLNum = 0 ; iWLNum < m_WaveLengths.GetCount() ; iWLNum++ )
    {
      int iCurrWL = m_WaveLengths[ iWLNum ] ;
      vector<SpectrumComponent>::iterator SCIter = m_Spectrums[ i ].m_Data.begin();
      for ( ; SCIter != m_Spectrums[ i ].m_Data.end(); SCIter++ )
      {
        if ( SCIter->first == iCurrWL )
        {
          Values.push_back( SCIter->second ) ;
          break ;
        }
      }
    }
    if ( Values.size() == m_WaveLengths.GetCount() )
    {
      double dMax = Values[ 0 ] ;
      for ( size_t iVIndex = 1; iVIndex < Values.size(); iVIndex++ )
      {
        if ( dMax < Values[ iVIndex ] )
          dMax = Values[ iVIndex ];
      }
      dMax = fabs( dMax ) ;
      if ( dMax > 1. )
      {
        for ( int iVIndex = 0; iVIndex < (int) Values.size(); iVIndex++ )
          m_Spectrums[ i ].m_CurrentRecognition.push_back( Values[ iVIndex ] / dMax );

        DWORD RGBColor = m_Spectrums[ i ].m_Color;
        int iY = RGB2Y( (RGBColor >> 16) & 0xff , (RGBColor >> 8) & 0xff , RGBColor & 0xff );
        int iU = RGB2U( (RGBColor >> 16) & 0xff , (RGBColor >> 8) & 0xff , RGBColor & 0xff );
        int iV = RGB2V( (RGBColor >> 16) & 0xff , (RGBColor >> 8) & 0xff , RGBColor & 0xff );
        DWORD YUVColor = iY + (iU << 8) + (iY << 16);
        m_Spectrums[ i ].m_YUVColor = YUVColor;

        m_Spectrums[ i ].m_bReadyForRecognition = true;
        if ( m_Spectrums[ i ].m_Name != "BASE" && m_Spectrums[ i ].m_Name != "WBASE" )
          m_ActiveSpectrums.push_back( i );
      }
      else
      {
        SEND_GADGET_ERR( _T( "Too low maximsum for %s recognition (%g)" ) ,
          m_Spectrums[ i ].m_Name.c_str() , dMax );
      }
    }
    else
    {
      SEND_GADGET_ERR( _T( "Not enough data for %s recognition (%d from %d)" ) ,
        m_Spectrums[ i ].m_Name.c_str() , Values.size() , m_WaveLengths.GetCount() );
    }
  }
  return (int) m_ActiveSpectrums.size() ;
}

bool HSCGadget::SetConfigParameters( LPCTSTR pFileName , bool bInsistWrite )
{
  CFileException ex;
  FXString ConfigFileName( pFileName );
  CFile ConfigFile;
  FXPropKit2 pk;
  GetGadgetName( m_GadgetInfo ) ;
  bool bRewrite = false;
  int bWrite = (bInsistWrite == true) ;
  SEND_GADGET_INFO( "Write Config File %s, Working mode %s, Old=%s, bWr=%d" ,
    pFileName , GetWorkingModeName( m_WorkingMode ) ,
    GetWorkingModeName( m_OldWorkingMode ) , bWrite ) ;
  if ( bInsistWrite )
    // data writing to config file
    // now, 13.05.17, it's working for simple recognition only
  {
    return SaveConfigParameters( pFileName ) ;
  }
  else if ( ((m_WorkingMode == WMode_Teaching) && m_bInitialConfig)
    || (m_WorkingMode != WMode_Teaching) )
  {
    m_bInitialConfig = false ;
    return LoadConfigParameters( pFileName ) ;
  }
  return true;
}

bool HSCGadget::LoadConfigParameters( LPCTSTR pFileName )
{
  if ( !pFileName )
    return false ;

  CFileException ex;
  FXString ConfigFileName( pFileName );
  CFile ConfigFile;
  FXPropKit2 pk;
  if ( ConfigFile.Open( pFileName , CFile::modeRead , &ex ) )
  {
    DWORD dwFLen = (DWORD) ConfigFile.GetLength() ;
    char * Buf = new TCHAR[ dwFLen + 5 ] ;
    UINT uiLen = ConfigFile.Read( Buf , dwFLen );
    ConfigFile.Close();
    if ( uiLen && uiLen <= dwFLen ) // is length and no too much
    {
      Buf[ uiLen ] = 0;
      pk += Buf;
    }
    else
    {
      SEND_GADGET_ERR( "ERROR Config loading: short file %s (L=%d)" ,
        pFileName , uiLen ) ;
      delete[] Buf ;
      return false ;
    }
    FXAutolock al( m_Lock , _T("HSCGadget::LoadConfigParameters") ) ;
    pk.GetInt( _T( "WorkingMode" ) , m_WorkingMode ) ;
    int iNSpectrums = 0 ;
    if ( pk.GetInt( _T( "NSpectrums" ) , iNSpectrums )
      && iNSpectrums && (m_WorkingMode == WMode_Teaching) )
      m_WorkingMode = WMode_Recognition_Ratios_Norm ;

    switch ( m_WorkingMode )
    {
      case WMode_CSVM:
      {
        pk.GetInt( "ImageWidth" , m_Detector.m_ImageSize.cx , 636 ) ;
        pk.GetInt( "ImageHeight" , m_Detector.m_ImageSize.cy , 508 ) ;
        pk.GetInt( "NWaveLengths" , m_Detector.m_iNChannels , 10 ) ;
        pk.GetInt( "Averaging" , m_Detector.m_iAveraging , 5 ) ;
        pk.GetDouble( "MinWaveLength_nm" , m_dMinWaveLength_nm , 900. ) ;
        pk.GetDouble( "Step_nm" , m_dWaveLengthStep_nm , 5.0 ) ;
        if ( !GetArray( pk , "CalibRect" , 'd' , 4 , &m_Detector.m_RefRect ) )
        {
          int Rect[] = {149 , 9 , 50 , 20} ;
          WriteArray( pk , "CalibRect" , 'd' , 4 , &m_Detector.m_RefRect ) ;
        }
        pk.GetDouble( "Bias" , m_Detector.m_dBias , -2.847312 ) ;
        GetArray( pk , "WaveLengths" , 'd' , &m_WaveLengths ) ;
        bool b_nanometers = true ;
        for ( int i = 0 ; i < m_WaveLengths.GetCount() ; i++ )
        {
          if ( m_WaveLengths[ i ] < 300. )
          {
            b_nanometers = false ;
            ASSERT( 0 ) ;
            break ;
          }
          else
            m_Detector.m_dWaveLengths.Add( (double) m_WaveLengths[ i ] ) ;
        }
        if ( !b_nanometers )
        {
          for ( int i = 0 ; i < m_WaveLengths.GetCount() ; i++ )
          {
            double dWaveLength = m_dMinWaveLength_nm
              + m_dWaveLengthStep_nm * m_WaveLengths[ i ] ;
            m_Detector.m_dWaveLengths.Add( dWaveLength ) ;
            m_WaveLengths[ i ] = ROUND( dWaveLength ) ;
          }
        }
        GetArray( pk , "RefVect" , 'f' , &m_Detector.m_dRefVect ) ;
        GetArray( pk , "LogMeanVect" , 'f' , &m_Detector.m_dLogMeanVect ) ;
        GetArray( pk , "LogStdVect" , 'f' , &m_Detector.m_dLogStdVect ) ;
        GetArray( pk , "BetaVect" , 'f' , &m_Detector.m_dBetaVect ) ;
        m_Detector.Init() ;
      }
      break ;
      case WMode_Teaching:
      {
        SEND_GADGET_INFO( "Read Config File, Working mode %s, Old=%s" ,
          GetWorkingModeName( m_WorkingMode ) ,
          GetWorkingModeName( m_OldWorkingMode ) ) ;

        pk.FXPropertyKit::GetInt( _T( "NChannels" ) , m_iNChannels ) ;
        if ( m_iNChannels )
        {
          m_LearnedData.clear() ;
          for ( int i = 0 ; i < m_iNChannels ; i++ )
          {
            FXString Name ;
            Name.Format( _T( "Sample%d" ) , i ) ;
            FXString SampleAsText ;
            pk.GetStringWithBrackets( Name , SampleAsText ) ;
            FXPropertyKit pkSample( SampleAsText ) ;
            Sample NewSample ;
            int iWaveLength ;
            if ( pkSample.GetInt( _T( "WaveLength_nm" ) , iWaveLength )
              && pkSample.GetDouble( _T( "Average" ) , NewSample.m_dAver )
              && pkSample.GetDouble( _T( "Std" ) , NewSample.m_dStd )
              && pkSample.GetDouble( _T( "BRef" ) , NewSample.m_dRefBlack )
              && pkSample.GetDouble( _T( "WRef" ) , NewSample.m_dRefWhite )
              )
            {
              m_LearnedData[ iWaveLength ] = NewSample ;
            }
            else
            {
              SEND_GADGET_ERR( "Incompatible config file %s " , pFileName ) ;
              return false ;
            }
          }
          BOOL bOK = GetRect( pk , _T( "RefRect" ) , m_BlackRefArea ) ;
          if ( !bOK )
            SEND_GADGET_ERR( "No data about reference in file %s" , pFileName ) ;
          bOK = GetRect( pk , _T( "TeachRect" ) , m_TeachingArea ) ;
          if ( !bOK )
            SEND_GADGET_ERR( "No data about Teaching Rect in file %s" , pFileName ) ;
          pk.GetDouble( _T( "Epsilon" ) , m_dEpsilon , 1.0 ) ;
          SEND_GADGET_INFO( "Recognition parameters loaded(teaching), %d wave lengths" ,
            m_LearnedData.size() ) ;
        }
      }
      break ;
      case WMode_Recognition_Amplitudes:
      case WMode_Recognition_Ratios:
      case WMode_Recognition_Ratios_Norm:
      {
        SEND_GADGET_INFO( "Read Config File %s, Working mode %s" ,
          pFileName ,
          GetWorkingModeName( m_WorkingMode ) ,
          GetWorkingModeName( m_OldWorkingMode ) ) ;
        if ( GetArray( pk , _T( "WaveLengths" ) , _T( 'i' ) , &m_WaveLengths ) )
          pk.GetString( _T( "WaveLengths" ) , m_sWaveLengths ) ;
        GetRect( pk , _T( "BRefRect" ) , m_BlackRefArea ) ;
        GetRect( pk , _T( "WRefRect" ) , m_WhiteRefArea ) ;
        GetRect( pk , _T( "TeachRect" ) , m_TeachingArea ) ;
        pk.GetDouble( _T( "Epsilon" ) , m_dEpsilon ) ;
        pk.GetDouble( _T( "dAmplThres" ) , m_dAmplThres ) ;
        FXString NamesOfObjects ;
        if ( iNSpectrums )
        {
          m_Spectrums.clear() ;
          FXString Name , AsString ;
          for ( int i = 0; i < iNSpectrums; i++ )
          {
            Name.Format( "Spectrum%d" , i ) ;
            if ( pk.GetStringWithBrackets( Name , AsString ) )
            {
              Spectrum NewSpectrum ;
              if ( NewSpectrum.FromString( AsString ) )
              {
                if ( !NamesOfObjects.IsEmpty() )
                  NamesOfObjects += _T( ',' ) ;
                m_Spectrums.push_back( NewSpectrum ) ;
                NamesOfObjects += NewSpectrum.m_Name.c_str();
              }
            }
          }
        }

        SEND_GADGET_INFO( "Load OK, %d wave lengths , %d spectrums. Objects: %s" ,
          m_WaveLengths.GetCount() , m_Spectrums.size() , (LPCTSTR)NamesOfObjects ) ;
      }
    }
      delete[] Buf ;

      return true ;
    }
  return false ;
}

bool HSCGadget::SaveConfigParameters( LPCTSTR pFileName )
{
  CFileException ex;
  FXString ConfigFileName( pFileName );
  CFile ConfigFile;
  FXPropKit2 pk;
  GetGadgetName( m_GadgetInfo ) ;
  SEND_GADGET_INFO( "Write Config Params: File %s, Working mode %s, Old=%s" ,
    pFileName , GetWorkingModeName( m_WorkingMode ) ,
    GetWorkingModeName( m_OldWorkingMode ) ) ;
  if ( ConfigFile.Open( pFileName , CFile::modeWrite | CFile::modeCreate , &ex ) )
  {
    FXAutolock al( m_Lock , _T( "HSCGadget::SaveConfigParameters" ) ) ;
    FXPropertyKit pk ;
    FXString Out , Add ;
    FXString Name ;
    pk.WriteInt( _T( "WorkingMode" ) , m_WorkingMode ) ;
    WriteArray( pk , _T( "WaveLengths" ) , _T( 'i' ) , &m_WaveLengths ) ;
    WriteRect( pk , _T( "BRefRect" ) , m_BlackRefArea ) ;
    WriteRect( pk , _T( "WRefRect" ) , m_WhiteRefArea ) ;
    WriteRect( pk , _T( "TeachRect" ) , m_TeachingArea ) ;
    pk.WriteDouble( _T( "Epsilon" ) , m_dEpsilon ) ;
    pk.WriteDouble( _T( "dAmplThres" ) , m_dAmplThres ) ;
    pk.WriteInt( "NSpectrums" , (int) m_Spectrums.size() ) ;
    Spectrums::iterator itsp = m_Spectrums.begin() ;
    int iSpecCnt = 0 ;
    FXString NamesOfSpectrums ;
    while ( itsp != m_Spectrums.end() )
    {
      FXString AsText ;
      itsp->ToString( AsText ) ;
      FXString Name ;
      Name.Format( _T( "Spectrum%d" ) , iSpecCnt++ ) ;
      pk.WriteString( Name , AsText ) ;
      NamesOfSpectrums += itsp->m_Name.c_str() ;
      itsp++ ;
      if ( itsp != m_Spectrums.end() )
        NamesOfSpectrums += _T( ',' ) ;
    }
    FXString Unregular = ::FxUnregularize( pk ) ;
    ConfigFile.Write( (LPCTSTR) Unregular , (int) Unregular.GetLength() ) ;
    ConfigFile.Close() ;
    SEND_GADGET_INFO( "Recognition parameters saved(after teaching), %d spectrums: %s" ,
      m_Spectrums.size() , (LPCTSTR) NamesOfSpectrums ) ;
  }
  else
  {
    SEND_GADGET_ERR( "Can't open file %s for writing" , pFileName ) ;
    return false ;
  }
  return true;
}
