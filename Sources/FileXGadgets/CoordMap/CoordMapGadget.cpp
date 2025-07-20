// CoordMapGadget.h.h : Implementation of the CoordMapGadget class


#include "StdAfx.h"
#include "CoordMapGadget.h"
#include "helpers\propertykitEx.h"
#include "Math/FRegression.h"

#define THIS_MODULENAME "CoordMap"

USER_FILTER_RUNTIME_GADGET( CoordMapGadget , "CoordMap" );
USER_FILTER_RUNTIME_GADGET( MapControl , "CoordMap" );

const char* CoordMapGadget::m_pModeList = "FOVtoWorld;Calibration;WorldToFOV;SimpleCopy;ImageOnly;ThrowData;SpotStatistics;";
const char*     MapControl::m_pModeList = "FOVtoWorld;Calibration;WorldToFOV;SimpleCopy;ImageOnly;ThrowData;";

CoordMapGadget::CoordMapGadget()
  : m_cOffset( 0. , 0. )
{
  m_ConvertMode = FOVtoWORLD;

  m_iNBigDotsX = 5;
  m_iNBigDotsY = 3;
  m_dWCalibStep = 2;
  m_dPosTolerance_perc = 20. ;
  m_bIsCalibrated = false;
  m_OutputMode = modeReplace ;
  m_CalibDataName = _T( "calib_spot" ) ;
  m_CenterDataName = _T( "laserspot" ) ;
  m_MeasureDataNames = _T( "hole" ) ;
  m_CalibDataLabel = _T( "CalibData" ) ;
  m_MeasNamesAsArray.Add( m_MeasureDataNames ) ;
  m_ConvertMode = FOVtoWORLD ;
  m_iMeasurentCount = 0 ;
  m_dScale_um_per_pixel = 1.0 ;
  m_bGenerateCaption = FALSE ;

  init();
  m_iInsertMissed = 1 ;
}

void CoordMapGadget::ShutDown()
{
  UserGadgetBase::ShutDown() ;
}

bool CoordMapGadget::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXString OldName = m_CalibDataFileName ;

  bool bRes = UserGadgetBase::ScanProperties( text , Invalidate ) ;
  m_cOffset = StringToCmplx( m_OffsetAsString ) ;
  if (OldName != m_CalibDataFileName)
  {
    LoadCalibData() ;
  }
  m_CalibGrid.SetInsertMissed( m_iInsertMissed ) ;
  m_CalibDataLabel = m_CalibDataLabel.Trim( _T( " \t\r\n" ) ) ;
  m_MeasNamesAsArray.RemoveAll() ;
  FXSIZE iPos = 0 ;
  FXString Name = m_MeasureDataNames.Tokenize( _T( " ,\t\n\r" ) , iPos ) ;
  while (!Name.IsEmpty())
  {
    m_MeasNamesAsArray.Add( Name.MakeLower() ) ;
    Name = m_MeasureDataNames.Tokenize( _T( " ,\t\n\r" ) , iPos ) ;
  }
  return bRes ;
}

CDataFrame * CoordMapGadget::DoCalibrationProcess(
  const CDataFrame* pDataFrame , bool bUnconditional )
{
  const CTextFrame * pCalibData = NULL ;
  const CTextFrame * pCenterData = NULL ;

  double dBeginTime = GetHRTickCount() ;
  if (!bUnconditional && pDataFrame->IsContainer())
  {
    CFramesIterator * Iterator = pDataFrame->CreateFramesIterator( text ) ;
    if (Iterator)
    {
      CTextFrame * pNext = ( CTextFrame* ) Iterator->Next() ;

      while (pNext)
      {
        const CTextFrame * pText = pDataFrame->GetTextFrame() ;
        if (!_tcsicmp( pText->GetLabel() , _T( "grid" ) ))
        {
          if (!pCalibData)
            pCalibData = pNext ;
          else
            ASSERT( 0 ) ; // several frames with calib data
        }
        else if (!_tcsicmp( pText->GetLabel() , _T( "center" ) ))
        {
          if (!pCenterData)
            pCenterData = pNext ;
          else
            ASSERT( 0 ) ; // several frames with Center data
        }
        pNext = ( CTextFrame* ) Iterator->Next() ;
      }
      delete Iterator ;
    }
  }
  else
  {
    const CTextFrame * pText = pDataFrame->GetTextFrame() ;
    if (bUnconditional || !_tcsicmp( pText->GetLabel() , _T( "grid" ) ))
      pCalibData = pText ;
    else if (!_tcsicmp( pText->GetLabel() , _T( "center" ) ))
      pCenterData = pText ;
  }
  const CVideoFrame * pvf = pDataFrame->GetVideoFrame() ;
  if (pvf)
  {
    m_CalibGrid.m_ImageSize = CSize( GetWidth( pvf ) , GetHeight( pvf ) ) ;
  }
  if (pCalibData)
  {
    double dBeginInitTime = GetHRTickCount() ;
    FXString CalibData = pCalibData->GetString() ;
    bool bIsOK = m_CalibGrid.init( CalibData ,
      m_iNBigDotsX , m_iNBigDotsY ,
      m_dWCalibStep , m_dPosTolerance_perc * 0.01 ,
      ( m_ConvertMode != ImageOnly ) && ( m_ConvertMode != SpotStatistics ) );

    if ( m_CalibGrid.m_bIsCalibrated )
      SENDINFO( "HCalibration is OK, Center is (%.2f,%.2f)" , m_CalibGrid.m_CenterSpot.m_imgCoord ) ;
    else
      SENDINFO( "HCalibration FAILED. #Spots=%d" , m_CalibGrid.m_Spots->GetNodeList()->getCnt() ) ;

    std::string DataForGrid( ( LPCTSTR ) CalibData ) ;

    GridDetectorParameters p;

    p.NeighbourDistanceMax = 100;
    p.LatticeBasisMaxAngle = 1.5;
    p.LatticeBasisMaxDistance = 2.0;
    p.LXLength = 4 ;
    p.LYLength = 2 ;
    m_GridDetector.SetParameters( p );

    bool bIsNewOK = m_GridDetector.process( DataForGrid );
    double dFullInitTime = GetHRTickCount() ;

    if (!bIsNewOK)
      return NULL ;

    m_bIsCalibrated = true ;

    string CalibDataFromGrid ;
    if (m_GridDetector.GetLastCalibData( CalibDataFromGrid ) 
      && CalibDataFromGrid.size() > 100 )
    {
      m_LastSavedCalibData = CalibDataFromGrid.c_str() ;
      if (!m_CalibDataFileName.IsEmpty())
        SaveCalibData() ;
      SENDINFO( "Calibration OK: Center (%.2f,%.2f)" , m_GridDetector.center() ) ;
    }
//     if (!m_CalibDataLabel.IsEmpty())
//     {
//       CTextFrame * pCalibData = CTextFrame::Create( ( LPCTSTR ) m_LastSavedCalibData ) ;
//       if (pCalibData)
//       {
//         pCalibData->CopyAttributes( pDataFrame ) ;
//         pCalibData->SetLabel( m_CalibDataLabel ) ;
//         pCalibData->SetTime( GetHRTickCount() ) ;
//         FXString Timing ;
//         CTextFrame * pTiming = CTextFrame::Create() ;
//         if (pTiming)
//         {
//           double dBegin = m_CalibGrid.m_dBeginTime ;
//           double dNeightbours = ( m_ConvertMode != ImageOnly ) ?
//             m_CalibGrid.m_dNeightboursTime - m_CalibGrid.m_dAllAxisTime : 0. ;
//           pTiming->GetString().Format( "Tld=%7.3f T1=%7.3f Ta=%7.3f Tn=%7.3f Tt=%7.3f Tf=%7.3f" ,
//             m_CalibGrid.m_dLoadTime , m_CalibGrid.m_dFirstAxisTime - dBegin ,
//             m_CalibGrid.m_dAllAxisTime - m_CalibGrid.m_dFirstAxisTime ,
//             dNeightbours ,
//             dFullInitTime - dBeginInitTime ) ;
//           pTiming->SetLabel( "Timing" ) ;
//           SENDINFO( "Calibration OK: %s" , ( LPCTSTR ) pTiming->GetString() ) ;
//           if (!m_pOutput || !m_pOutput->Put( pTiming ))
//             pTiming->Release();
//         }
//         return pCalibData ;
//       }
//     }
//     SENDERR( "Calibration FAILED" ) ;
    return NULL ;
  }
  else if (pCenterData && m_bIsCalibrated)
  {
    double x;
    double y;
    FXString CenterData = pCenterData->GetString() ;
    int i = sscanf( ( LPCTSTR ) CenterData , "%lf, %lf" , &x , &y );
    if (i == 2)
    {
      cmplx Img( x , y ) ;
      m_CalibGrid.setCenter( Img );
    }
  }

  return NULL ;
}

void CoordMapGadget::DoTextProcess( const CDataFrame* pDataFrame ,
  CContainerFrame* &container )
{
  const CTextFrame* textFrame ;

  double x;
  double y;
  int iNFields = 0 ;

  if (!pDataFrame || !( textFrame = pDataFrame->GetTextFrame() ))
    return;

  const FXString frameString = textFrame->GetString();

  if (_tcsstr( textFrame->GetLabel() , _T( "Data_Spot" ) ))
  {
    FXSIZE iPos = 0 ;
    int iIndex ;
    FXString Token = frameString.Tokenize( _T( "\n" ) , iPos ) ;

    // The first string is "Spots=%d Max=%.2f Min=%.2f ...
    // Take next one
    while (!( Token = frameString.Tokenize( _T( "\n" ) , iPos ) ).IsEmpty())
    {
      int iNItems = sscanf( ( LPCTSTR ) Token , _T( "%d %lf %lf" ) ,
        &iIndex , &x , &y ) ;
      if (iNItems == 3)
      {
        iNFields = 2 ; // x and y
        break ;
      }
    }
  }
  if (iNFields != 2)
  {
    iNFields = sscanf( frameString , "%lf, %lf" , &x , &y );

    if (iNFields != 2)
    {
      iNFields = sscanf( ( LPCTSTR ) frameString + 14 , "x=%lf;y=%lf;" , &x , &y );
    }

    if (iNFields != 2)
    {
      int iXPos = ( int ) frameString.Find( "x=" );
      iNFields = sscanf( ( LPCTSTR ) frameString + iXPos , "x=%lf;y=%lf;" , &x , &y );
    }

    if (iNFields != 2)
      return;
  }


  FXString AsString ;
  if (m_ConvertMode == FOVtoWORLD)
  {
//     cmplx Img( x , y ) ;
//     cmplx World = m_CalibGrid.ImgToWorld( Img ) ;// m_CalibGrid.getCoordinates(x, y);
//     World += m_cOffset ;
    ns::R2 ImgR2( x , y ) , WrldR2 ;
    int iInd1 , iInd2 ;
    m_GridDetector.ImageToWorld( ImgR2 , WrldR2 , iInd1 , iInd2 ) ;
    cmplx World( WrldR2.x , WrldR2.y ) ;
    ToStringFormatted( World , AsString , _T( "x=%.3f;y=%.3f" ) ) ;
  }
  else if (m_ConvertMode == WORLDtoFOV)
  {
//     cmplx World( x , y ) ;
//     cmplx Img = m_CalibGrid.WorldToImg( World ) ;// m_CalibGrid.getCoordinates(x, y);
    ns::R2 ImgR2 , WrldR2( x , y )  ;
    m_GridDetector.WorldToImage( ImgR2 , WrldR2  ) ;
    cmplx Img( ImgR2.x , ImgR2.y ) ;
    ToStringFormatted( Img , AsString , _T( "x=%.3f;y=%.3f" ) ) ;
  }
  if (!AsString.IsEmpty())
  {
    CTextFrame* resFrame = CTextFrame::Create( AsString );
    resFrame->CopyAttributes( pDataFrame );
    resFrame->SetLabel( FXString( resFrame->GetLabel() ) + "_Converted" );
    if (!container)
      container = CContainerFrame::Create();

    container->AddFrame( resFrame );
  }
}

void CoordMapGadget::DoFigureProcess( const CDataFrame* pDataFrame , CContainerFrame* &container )
{
  CFigureFrame* fig = ( CFigureFrame* ) pDataFrame->GetFigureFrame();
  int numOfPoints;

  if (!fig || ( ( numOfPoints = ( int ) fig->GetCount() ) < 1 ))
    return;

  CFigureFrame* resFrame = CFigureFrame::Create();

  CDPoint World ;
  for (int i = 0; i < numOfPoints; i++)
  {

    if (m_CalibGrid.ImgToWorld( fig->GetAt( i ) , World ))
      resFrame->AddPoint( World );
  }

  resFrame->CopyAttributes( pDataFrame );
  resFrame->SetLabel( FXString( resFrame->GetLabel() ) + "_Converted" );

  if (!container)
    container = CContainerFrame::Create();

  container->AddFrame( resFrame );
}

CDataFrame* CoordMapGadget::DoProcessing( const CDataFrame* pDataFrame )
{
  switch (m_ConvertMode)
  {
    case Throw: return NULL ;
    case SimpleCopy:
      ( ( CDataFrame* ) pDataFrame )->AddRef() ;
      return ( CDataFrame* ) pDataFrame ;
    case SpotStatistics:
      {
        if (!pDataFrame->IsContainer() && ( pDataFrame->GetDataType() == text ))
        {
          if (_tcsicmp( pDataFrame->GetLabel() , "DoCaption" ) == 0)
          {
            m_bGenerateCaption = TRUE ;
          }
        }
      }
    default: break ;
  }
  CDataFrame * pReturnFrame = NULL ;
  const CDataFrame * pCalibData = GetCalibData( pDataFrame ) ;
  if (pCalibData)
  {
    pReturnFrame = DoCalibrationProcess( pCalibData , true ) ;

    m_bIsCalibrated |= ( pReturnFrame != NULL ) ;
  }

  const CDataFrame * pCenterData = GetCenterData( pDataFrame ) ;
  if (pCenterData)
  {
    const CTextFrame * pCenterText = pCenterData->GetTextFrame() ;
    FXString Text = pCenterText->GetString() ;
    Spot CenterData ;
    bool bRes = Parser::parse_one_spot( Text , CenterData ) ;
    if ( m_CalibGrid.m_bIsCalibrated )
      m_CalibGrid.setCenter( CenterData.m_imgCoord ) ;
  }
  if ( m_CalibGrid.m_bIsCalibrated && ( m_ConvertMode == SpotStatistics ) )
    FormStatistics( pDataFrame ) ;

  CContainerFrame* container = NULL;
  if (GetMeasData( pDataFrame ))
  {
    for (int i = 0 ; i < m_FramesForConversion.GetCount() ; i++)
    {
      const CDataFrame * pFrame = m_FramesForConversion[ i ] ;
      if (pFrame->GetDataType() == basicdatatype::text)
      {
        DoTextProcess( pFrame , container );
      }
      else if (pFrame->GetDataType() == basicdatatype::figure)
      {
        DoFigureProcess( pFrame , container );
      }
    }
    return container ;
  }
  else if (m_ConvertMode == FOVtoWORLD
    || m_ConvertMode == WORLDtoFOV)
  {
    if (!m_bIsCalibrated)
    {
      ( ( CDataFrame* ) pDataFrame )->AddRef() ;
      return ( CDataFrame* ) pDataFrame ;
    }


    if (pDataFrame->IsContainer())
    {
      CFramesIterator* it;

      CDataFrame* pFrame;

      if (it = pDataFrame->CreateFramesIterator( text ))
      {
        while (pFrame = it->Next());
        {
          DoTextProcess( pFrame , container );
        }
        delete it ;
      }

      if (it = pDataFrame->CreateFramesIterator( figure ))
      {
        while (pFrame = it->Next())
        {
          DoFigureProcess( pFrame , container );
        }
        delete it ;
      }
    }
    else
    {
      if (pDataFrame->GetDataType() == basicdatatype::text)
        DoTextProcess( pDataFrame , container );

      else if (pDataFrame->GetDataType() == basicdatatype::figure)
        DoFigureProcess( pDataFrame , container );
    }

    if (container)
    {
      container->AddFrame( pDataFrame );
      return container;
    }
  }

  ( ( CDataFrame* ) pDataFrame )->AddRef() ;
  return ( CDataFrame* ) pDataFrame ;
}

void CoordMapGadget::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  CTextFrame * pText = pParamFrame->GetTextFrame() ;
  if (pText)
  {
    FXPropKit2 PK = pText->GetString() ;
    if (!PK.IsEmpty())
    {
      bool bInvalidate = false ;
      ScanProperties( PK , bInvalidate ) ;
    }
  }
  pParamFrame->Release() ;
};

void CoordMapGadget::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "Mode" ) ,
    &m_ConvertMode , SProperty::Int , m_pModeList );
  addProperty( SProperty::EDITBOX , _T( "BigDotsX" ) ,
    &m_iNBigDotsX , SProperty::Int );
  addProperty( SProperty::EDITBOX , _T( "BigDotsY" ) ,
    &m_iNBigDotsY , SProperty::Int );
  addProperty( SProperty::EDITBOX , _T( "Step_mm" ) ,
    &m_dWCalibStep , SProperty::Double );
  addProperty( SProperty::EDITBOX , _T( "Offset" ) ,
    &m_OffsetAsString , SProperty::String );
  addProperty( SProperty::EDITBOX , _T( "Pos_Tolerance_%" ) ,
    &m_dPosTolerance_perc , SProperty::Double );
  addProperty( SProperty::SPIN , _T( "Insert_missed" ) ,
    &m_iInsertMissed , SProperty::Int , 0 , 7 );
  addProperty( SProperty::EDITBOX , _T( "ScaleForSpotStatistics" ) ,
    &m_dScale_um_per_pixel , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , _T( "CalibDataFileName" ) ,
    &m_CalibDataFileName , SProperty::String );
  addProperty( SProperty::EDITBOX , _T( "CalibDataLabel" ) ,
    &m_CalibDataLabel , SProperty::String );
  addProperty( SProperty::EDITBOX , _T( "CalibDataName" ) ,
    &m_CalibDataName , SProperty::String );
  addProperty( SProperty::EDITBOX , _T( "CenterDataName" ) ,
    &m_CenterDataName , SProperty::String );
  addProperty( SProperty::EDITBOX , _T( "MeasureDataNames" ) ,
    &m_MeasureDataNames , SProperty::String );
};

void CoordMapGadget::ConnectorsRegistration()
{
  addInputConnector( transparent , "Input" );
  addOutputConnector( transparent , "Output" );
  addOutputConnector( transparent , "DataOutput" );
  addDuplexConnector( transparent , transparent , "Control" ) ;
};

bool CoordMapGadget::SaveCalibData()
{
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  if (!m_LastSavedCalibData.IsEmpty() && !m_CalibDataFileName.IsEmpty())
  {
    FILE * fw = NULL ;
    errno_t Err = fopen_s( &fw , ( LPCTSTR ) m_CalibDataFileName , _T( "wb" ) ) ;
    if (Err == 0)
    {
      if (fwrite( ( LPCTSTR ) m_LastSavedCalibData , m_LastSavedCalibData.GetLength() , 1 , fw ))
        SENDINFO( "Gadget %s did save Calib Data in file %s" , ( LPCTSTR ) GadgetName , ( LPCTSTR ) m_CalibDataFileName ) ;
      else
        SENDERR( "Gadget %s can't save Calib Data in file %s" , ( LPCTSTR ) GadgetName , ( LPCTSTR ) m_CalibDataFileName ) ;
      fclose( fw ) ;
      return true ;
    }
    SENDERR( "Gadget %s can't open file %s" , ( LPCTSTR ) GadgetName , ( LPCTSTR ) m_CalibDataFileName ) ;

  }
  return false ;
};

bool CoordMapGadget::SaveCalibData( FXString& Result )
{
  if (!m_bIsCalibrated || !m_CalibGrid.m_pSpotsAs2DArray)
    return false ;

  FXString GadgetName , GraphName ;
  GetGadgetName( GadgetName ) ;

  time_t rawtime;
  struct tm * timeinfo;
  time( &rawtime );
  timeinfo = localtime( &rawtime );
  TCHAR buf[ 100 ] ;
  errno_t err = _tasctime_s( buf , 99 , timeinfo ) ;
  if (err != 0)
    buf[ 0 ] = 0 ;

  Result.Format( _T( "//Calibdata for gadget %s Time %s \r\n" ) ,
    ( LPCTSTR ) GadgetName , buf ) ;
  bool bRes = m_CalibGrid.SaveCalibData( Result ) ;
  return bRes ;
};


bool CoordMapGadget::LoadCalibData()
{
  FILE * fr = NULL ;
  errno_t Err = fopen_s( &fr , ( LPCTSTR ) m_CalibDataFileName , _T( "rb" ) ) ;
  if (Err == 0)
  {
    m_CalibGrid.Clean() ;
    m_CalibGrid.InitReservuar() ;
    m_bIsCalibrated = m_CalibGrid.RestoreCalibData( fr ) ;
    fclose( fr ) ;
  }
  else
    m_bIsCalibrated = false ;
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;

  if (m_bIsCalibrated)
  {
    SENDINFO( "Gadget %s did Load Calib Data from file %s" , ( LPCTSTR ) GadgetName , ( LPCTSTR ) m_CalibDataFileName ) ;
    FXString ForSave ;
    if (SaveCalibData( ForSave ))
    {
      FXString Name = m_CalibDataFileName + _T( "_Test.dat" ) ;
      FILE * fw = NULL ;
      errno_t Err = fopen_s( &fw , ( LPCTSTR ) Name , _T( "wb" ) ) ;
      if (Err == 0)
      {
        fwrite( ( LPCTSTR ) ForSave , ForSave.GetLength() , 1 , fw ) ;
        fclose( fw ) ;
      }

    }

  }
  else
    SENDERR( "Gadget %s can't Load Calib Data from file %s" , ( LPCTSTR ) GadgetName , ( LPCTSTR ) m_CalibDataFileName ) ;

  return m_bIsCalibrated ;
};

const CDataFrame * CoordMapGadget::GetCalibData( const CDataFrame * pInputData )
{
  const CDataFrame * pResult = NULL ;
  if (pInputData->IsContainer())
  {
    CFramesIterator* it;

    const CDataFrame* pFrame;

    if (it = pInputData->CreateFramesIterator( text ))
    {
      while (pFrame = it->Next())
      {
        LPCTSTR p = ( LPCTSTR ) _tcsstr( pFrame->GetLabel() , _T( "Data_Spot:" ) ) ;
        if (p)
        {
          if (_tcsstr( p + 10 , ( LPCTSTR ) m_CalibDataName ))
          {
            pResult = pFrame ;
            break ;
          }
        }
      }
      delete it ;
    }
  }
  else
  {
    if (pInputData->GetDataType() == basicdatatype::text)
    {
      LPCTSTR p = ( LPCTSTR ) _tcsstr( pInputData->GetLabel() , _T( "Data_Spot:" ) ) ;
      if (p)
      {
        if (_tcsstr( p + 10 , ( LPCTSTR ) m_CalibDataName ))
          pResult = pInputData ;
      }
    }
  }

  return pResult ;
}
const CDataFrame * CoordMapGadget::GetCenterData( const CDataFrame * pInputData )
{
  const CDataFrame * pResult = NULL ;
  if (pInputData->IsContainer())
  {
    CFramesIterator* it;

    const CDataFrame* pFrame;

    if (it = pInputData->CreateFramesIterator( text ))
    {
      while (pFrame = it->Next())
      {
        LPCTSTR p = ( LPCTSTR ) _tcsstr( pFrame->GetLabel() , _T( "Data_Spot:" ) ) ;
        if (p)
        {
          if (_tcsstr( p + 10 , ( LPCTSTR ) m_CenterDataName ))
          {
            pResult = pFrame ;
            break ;
          }
        }
      }
      delete it ;
    }
  }
  else
  {
    if (pInputData->GetDataType() == basicdatatype::text)
    {
      LPCTSTR p = ( LPCTSTR ) _tcsstr( pInputData->GetLabel() , _T( "Data_Spot:" ) ) ;
      if (p)
      {
        if (_tcsstr( p + 10 , ( LPCTSTR ) m_CenterDataName ))
          pResult = pInputData ;
      }
    }
  }
  return pResult ;
}
int CoordMapGadget::GetMeasData( const CDataFrame * pInputData )
{
  m_FramesForConversion.RemoveAll() ;
  if (m_MeasNamesAsArray.GetCount())
  {
    if (pInputData->IsContainer())
    {
      CFramesIterator* it;

      const CDataFrame* pFrame;

      if (it = pInputData->CreateFramesIterator( figure ))
      {
        while (pFrame = it->Next())
        {
          LPCTSTR pLabel = pFrame->GetLabel() ;
          for (int i = 0 ; i < m_MeasNamesAsArray.GetCount() ; i++)
          {
            LPCTSTR p = ( LPCTSTR ) _tcsstr( pLabel , ( LPCTSTR ) m_MeasNamesAsArray[ i ] ) ;
            if (p)
              m_FramesForConversion.Add( pFrame ) ;
          }
        }
        delete it ;
      }
      if (it = pInputData->CreateFramesIterator( text ))
      {
        while (pFrame = it->Next())
        {
          LPCTSTR pLabel = pFrame->GetLabel() ;
          for (int i = 0 ; i < m_MeasNamesAsArray.GetCount() ; i++)
          {
            LPCTSTR p = ( LPCTSTR ) _tcsstr( pLabel , ( LPCTSTR ) m_MeasNamesAsArray[ i ] ) ;
            if (p)
              m_FramesForConversion.Add( pFrame ) ;
          }
        }
        delete it ;
      }
    }
    else
    {
      datatype Type = pInputData->GetDataType() ;
      if (Type == figure || Type == text)
      {
        for (int i = 0 ; i < m_MeasNamesAsArray.GetCount() ; i++)
        {
          LPCTSTR p = ( LPCTSTR ) _tcsstr( pInputData->GetLabel() , ( LPCTSTR ) m_MeasNamesAsArray[ i ] ) ;
          if (p)
          {
            m_FramesForConversion.Add( pInputData ) ;
            break ;
          }
        }
      }
    }
  }
  return ( int ) m_FramesForConversion.GetCount() ;
}

size_t CoordMapGadget::ExtractConturs( const CDataFrame * pDataFrame )
{
  m_Conturs.clear() ;
  CFigureFrame* pFrame = NULL ;
  CFramesIterator * it = pDataFrame->CreateFramesIterator( figure ) ;
  while (pFrame = ( CFigureFrame* ) it->Next())
  {
    LPCTSTR pLabel = pFrame->GetLabel() ;
    LPCTSTR pObjectName = _tcsstr( pLabel , m_CalibDataName ) ;
    if (pObjectName)
    {
      int iConturNum = atoi( pObjectName + m_CalibDataName.GetLength() + 1 ) ;
      if ((int)m_Conturs.size() < iConturNum + 1)
        m_Conturs.resize( iConturNum + 1 ) ;
      m_Conturs[ iConturNum ] = pFrame ;
    }
  }
  if (it)
    delete it ;
  return m_Conturs.size() ;
}

bool CoordMapGadget::FormStatistics( const CDataFrame * pDataFrame )
{
  bool bRes = false ;
  if ( m_CalibGrid.m_HorLines.size() < 2 )
    return false ;
  CFStatistics Area_um2 , Diameter_um , Perimeter_um , BoxRatio ;
  CFStatistics XCentroid , YCentroid , XMidPoint , YMidPoint ;
  CFStatistics BoundLeft , BoundTop , BoundRight , BoundBottom ;
  CFStatistics BoundWidth , BoundHeight , Spacing , Ruling_lpcm ;
  CFStatistics Ruling_lpi , Dot_Percent ;

  size_t nConturs = ExtractConturs( pDataFrame ) ;

  if ( !m_Conturs.size() )
  {
    SENDERR( "ERROR: No information about conturs" ) ;
    return false ;
  }
  Node1D * pNode = m_CalibGrid.m_Spots->GetNodeList()->GetHead() ;
  size_t iHorLineCnt = 0;
  Spot * pFirstInRow = NULL;
  // First cycle for statistics accumulation
  do
  {
    Axis& SpotHorAxis = *( m_CalibGrid.m_HorLines[ iHorLineCnt ] );

    Node1D * pCurrentNode = SpotHorAxis.GetHead() ;
    Spot& FirstInRow = *( pCurrentNode->m_Spot );
    do
    {
      Spot& CurrentSpot = *( pCurrentNode->m_Spot );
      int iIndex = CurrentSpot.GetGlobalIndex() ;
      if ( iIndex >= 0 )
      {
        int iXIndexDiffToRowBegin = CurrentSpot.m_Indexes.x - FirstInRow.m_Indexes.x ;
        cmplx cSpotShiftRelToFirstInRow = FirstInRow.m_imgCoord
          + SpotHorAxis.GetMainVector() * ( double ) ( iXIndexDiffToRowBegin ) ;
        Area_um2.Add( CurrentSpot.m_dArea ) ;
        Diameter_um.Add( ( CurrentSpot.m_Sizes.getWidth() + CurrentSpot.m_Sizes.getHeight() ) / 2. ) ;
        Perimeter_um.Add( m_Conturs[ iIndex ]->GetConturLength() ) ;
        CmplxArray Extrems ;
        cmplx cSize ;
        cmplx cMid = FindExtrems( m_Conturs[ iIndex ] , Extrems , NULL , &cSize ) ;
        BoxRatio.Add( cSize.imag() / cSize.real() ) ;

        cmplx cCentroid = CurrentSpot.m_imgCoord - cSpotShiftRelToFirstInRow ;
        XCentroid.Add( cCentroid.real() ) ;
        YCentroid.Add( cCentroid.imag() ) ;

        cmplx cMidPoint = ( Extrems[ EXTREME_INDEX_RIGHT ] + Extrems[ EXTREME_INDEX_LEFT ] ) / 2.
          - cSpotShiftRelToFirstInRow ;
        XMidPoint.Add( cMidPoint.real() ) ;
        YMidPoint.Add( cMidPoint.imag() ) ;

        BoundLeft.Add( Extrems[ EXTREME_INDEX_LEFT ].real() - cSpotShiftRelToFirstInRow.real() ) ;
        BoundTop.Add( Extrems[ EXTREME_INDEX_TOP ].imag() - cSpotShiftRelToFirstInRow.imag() ) ;
        BoundRight.Add( Extrems[ EXTREME_INDEX_RIGHT ].real() - cSpotShiftRelToFirstInRow.real() ) ;
        BoundBottom.Add( Extrems[ EXTREME_INDEX_BOTTOM ].imag() - cSpotShiftRelToFirstInRow.imag() ) ;

        BoundWidth.Add( Extrems[ EXTREME_INDEX_RIGHT ].real() - Extrems[ EXTREME_INDEX_LEFT ].real() ) ;
        BoundHeight.Add( Extrems[ EXTREME_INDEX_BOTTOM ].imag() - Extrems[ EXTREME_INDEX_TOP ].imag() ) ;

        if (CurrentSpot.m_DirE.real() || CurrentSpot.m_DirE.imag())
          Spacing.Add( CurrentSpot.m_DirE.real() ) ;
        Ruling_lpcm.Add( 10000. / abs( SpotHorAxis.GetMainVector() ) * m_dScale_um_per_pixel ) ;
        Ruling_lpi.Add( 25400. / abs( SpotHorAxis.GetMainVector() ) * m_dScale_um_per_pixel ) ;
        //    double dSquare_pix2 = abs( SpotHorAxis.GetMainVector() ) * abs( SpotVertAxis.GetMainVector() ) ;
        double dSquare_pix2 = abs( SpotHorAxis.GetMainVector() ) * abs( SpotHorAxis.GetMainVector() ) ;
        double dDotPercent = CurrentSpot.m_dArea * 100. / dSquare_pix2 ;
        Dot_Percent.Add( dDotPercent ) ;
      }
//       if (pCurrentNode == SpotHorAxis.GetTail())
//         break;
      pCurrentNode = pCurrentNode->next;
    } while ( pCurrentNode );
  } while (++iHorLineCnt < m_CalibGrid.m_HorLines.size());
  Area_um2.Calculate() ;
  Diameter_um.Calculate() ;
  Perimeter_um.Calculate() ;
  BoxRatio.Calculate() ;
  XCentroid.Calculate() ;
  YCentroid.Calculate() ;
  XMidPoint.Calculate() ;
  YMidPoint.Calculate() ;
  BoundLeft.Calculate() ;
  BoundTop.Calculate() ;
  BoundRight.Calculate() ;
  BoundBottom.Calculate() ;
  BoundWidth.Calculate() ;
  BoundHeight.Calculate() ;
  Spacing.Calculate() ;
  Ruling_lpcm.Calculate() ;
  Ruling_lpi.Calculate() ;
  Dot_Percent.Calculate() ;

  // Second cycle for histogram processing
  iHorLineCnt = 0;
  do
  {
    Axis& SpotHorAxis = *( m_CalibGrid.m_HorLines[ iHorLineCnt ] );
    Node1D * pCurrentNode = SpotHorAxis.GetHead();
    Spot& FirstInRow = *( pCurrentNode->m_Spot );
    do
    {
      Spot& CurrentSpot = *( pCurrentNode->m_Spot );
      int iIndex = CurrentSpot.GetGlobalIndex() ;
      if ( iIndex >= 0 )
      {
        Spot& FirstInRow = *( SpotHorAxis.GetHead()->m_Spot ) ;
        int iXIndexDiffToRowBegin = CurrentSpot.m_Indexes.x - FirstInRow.m_Indexes.x ;
        cmplx cSpotShiftRelToFirstInRow = FirstInRow.m_imgCoord
          + SpotHorAxis.GetMainVector() * ( double ) ( iXIndexDiffToRowBegin ) ;
        Area_um2.AddToHistogram( CurrentSpot.m_dArea ) ;
        Diameter_um.AddToHistogram( ( CurrentSpot.m_Sizes.getWidth() + CurrentSpot.m_Sizes.getHeight() ) / 2. ) ;
        Perimeter_um.AddToHistogram( m_Conturs[ iIndex ]->GetConturLength() ) ;
        CmplxArray Extrems ;
        cmplx cSize ;
        cmplx cMid = FindExtrems( m_Conturs[ iIndex ] , Extrems , NULL , &cSize ) ;
        BoxRatio.AddToHistogram( cSize.imag() / cSize.real() ) ;

        cmplx cCentroid = CurrentSpot.m_imgCoord - cSpotShiftRelToFirstInRow ;
        XCentroid.AddToHistogram( cCentroid.real() ) ;
        YCentroid.AddToHistogram( cCentroid.imag() ) ;

        cmplx cMidPoint = ( Extrems[ EXTREME_INDEX_RIGHT ] + Extrems[ EXTREME_INDEX_LEFT ] ) / 2.
          - cSpotShiftRelToFirstInRow ;
        XMidPoint.AddToHistogram( cMidPoint.real() ) ;
        YMidPoint.AddToHistogram( cMidPoint.imag() ) ;

        BoundLeft.AddToHistogram( Extrems[ EXTREME_INDEX_LEFT ].real() - cSpotShiftRelToFirstInRow.real() ) ;
        BoundTop.AddToHistogram( Extrems[ EXTREME_INDEX_TOP ].imag() - cSpotShiftRelToFirstInRow.imag() ) ;
        BoundRight.AddToHistogram( Extrems[ EXTREME_INDEX_RIGHT ].real() - cSpotShiftRelToFirstInRow.real() ) ;
        BoundBottom.AddToHistogram( Extrems[ EXTREME_INDEX_BOTTOM ].imag() - cSpotShiftRelToFirstInRow.imag() ) ;

        BoundWidth.AddToHistogram( Extrems[ EXTREME_INDEX_RIGHT ].real() - Extrems[ EXTREME_INDEX_LEFT ].real() ) ;
        BoundHeight.AddToHistogram( Extrems[ EXTREME_INDEX_BOTTOM ].imag() - Extrems[ EXTREME_INDEX_TOP ].imag() ) ;

        if (CurrentSpot.m_DirE.real() || CurrentSpot.m_DirE.imag())
          Spacing.AddToHistogram( CurrentSpot.m_DirE.real() ) ;
        Ruling_lpcm.AddToHistogram( 10000. / abs( SpotHorAxis.GetMainVector() ) * m_dScale_um_per_pixel ) ;
        Ruling_lpi.AddToHistogram( 25400. / abs( SpotHorAxis.GetMainVector() ) * m_dScale_um_per_pixel ) ;
        //    double dSquare_pix2 = abs( SpotHorAxis.GetMainVector() ) * abs( SpotVertAxis.GetMainVector() ) ;
        double dSquare_pix2 = abs( SpotHorAxis.GetMainVector() ) * abs( SpotHorAxis.GetMainVector() ) ;
        double dDotPercent = CurrentSpot.m_dArea * 100. / dSquare_pix2 ;
        Dot_Percent.AddToHistogram( dDotPercent ) ;
      }

//       if (pCurrentNode == SpotHorAxis.GetTail())
//         break;
      pCurrentNode = pCurrentNode->next;
    } while (pCurrentNode);
  } while (++iHorLineCnt < m_CalibGrid.m_HorLines.size());

  m_SpotsStatistics.Reset() ;
  m_SpotsStatistics.m_dArea = Area_um2.GetAverage() ;
  m_SpotsStatistics.m_dArea_Std = Area_um2.GetStd() ;
  Area_um2.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dArea_75percMin , m_SpotsStatistics.m_dArea_75percMax ) ;
  Area_um2.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dArea_95percMin , m_SpotsStatistics.m_dArea_95percMax ) ;
  m_SpotsStatistics.m_dDia = Diameter_um.GetAverage() ;
  m_SpotsStatistics.m_dDia_Std = Diameter_um.GetStd() ;
  Diameter_um.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dDia_75percMin , m_SpotsStatistics.m_dDia_75percMax ) ;
  Diameter_um.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dDia_95percMin , m_SpotsStatistics.m_dDia_95percMax ) ;
  m_SpotsStatistics.m_dPerimeter = Perimeter_um.GetAverage() ;
  m_SpotsStatistics.m_dPerimeter_Std = Perimeter_um.GetStd() ;
  Perimeter_um.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dPerimeter_75percMin , m_SpotsStatistics.m_dPerimeter_75percMax ) ;
  Perimeter_um.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dPerimeter_95percMin , m_SpotsStatistics.m_dPerimeter_95percMax ) ;
  m_SpotsStatistics.m_dBoxRatio = BoxRatio.GetAverage() ;
  m_SpotsStatistics.m_dBoxRatio_Std = BoxRatio.GetStd() ;
  BoxRatio.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dBoxRatio_75percMin , m_SpotsStatistics.m_dBoxRatio_75percMax ) ;
  BoxRatio.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dBoxRatio_95percMin , m_SpotsStatistics.m_dBoxRatio_95percMax ) ;
  m_SpotsStatistics.m_dXCentroid = XCentroid.GetAverage() ;
  m_SpotsStatistics.m_dXCentroid_Std = XCentroid.GetStd() ;
  XCentroid.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dXCentroid_75percMin , m_SpotsStatistics.m_dXCentroid_75percMax ) ;
  XCentroid.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dXCentroid_95percMin , m_SpotsStatistics.m_dXCentroid_95percMax ) ;
  m_SpotsStatistics.m_dYCentroid = YCentroid.GetAverage() ;
  m_SpotsStatistics.m_dYCentroid_Std = YCentroid.GetStd() ;
  YCentroid.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dYCentroid_75percMin , m_SpotsStatistics.m_dYCentroid_75percMax ) ;
  YCentroid.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dYCentroid_95percMin , m_SpotsStatistics.m_dYCentroid_95percMax ) ;
  m_SpotsStatistics.m_dXMid = XMidPoint.GetAverage() ;
  m_SpotsStatistics.m_dXMid_Std = XMidPoint.GetStd() ;
  XMidPoint.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dXMid_75percMin , m_SpotsStatistics.m_dXMid_75percMax ) ;
  XMidPoint.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dXMid_95percMin , m_SpotsStatistics.m_dXMid_95percMax ) ;
  m_SpotsStatistics.m_dYMid = YMidPoint.GetAverage() ;
  m_SpotsStatistics.m_dYMid_Std = YMidPoint.GetStd() ;
  YMidPoint.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dYMid_75percMin , m_SpotsStatistics.m_dYMid_75percMax ) ;
  YMidPoint.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dYMid_95percMin , m_SpotsStatistics.m_dYMid_95percMax ) ;
  m_SpotsStatistics.m_dBoundLeft = BoundLeft.GetAverage() ;
  m_SpotsStatistics.m_dBoundLeft_Std = BoundLeft.GetStd() ;
  m_SpotsStatistics.m_dBoundTop = BoundTop.GetAverage() ;
  m_SpotsStatistics.m_dBoundTop_Std = BoundTop.GetStd() ;
  m_SpotsStatistics.m_dBoundRight = BoundRight.GetAverage() ;
  m_SpotsStatistics.m_dBoundRight_Std = BoundRight.GetStd() ;
  m_SpotsStatistics.m_dBoundBottom = BoundBottom.GetAverage() ;
  m_SpotsStatistics.m_dBoundBottom_Std = BoundBottom.GetStd() ;
  m_SpotsStatistics.m_dBoundWidth = BoundWidth.GetAverage() ;
  m_SpotsStatistics.m_dBoundWidth_Std = BoundWidth.GetStd() ;
  BoundWidth.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dBoundWidth_75percMin , m_SpotsStatistics.m_dBoundWidth_75percMax ) ;
  BoundWidth.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dBoundWidth_95percMin , m_SpotsStatistics.m_dBoundWidth_95percMax ) ;
  m_SpotsStatistics.m_dBoundHeight = BoundHeight.GetAverage() ;
  m_SpotsStatistics.m_dBoundHeight_Std = BoundHeight.GetStd() ;
  BoundHeight.GetValueForPercentOnHistogram( 75 ,
    m_SpotsStatistics.m_dBoundHeight_75percMin , m_SpotsStatistics.m_dBoundHeight_75percMax ) ;
  BoundHeight.GetValueForPercentOnHistogram( 95 ,
    m_SpotsStatistics.m_dBoundHeight_95percMin , m_SpotsStatistics.m_dBoundHeight_95percMax ) ;
  m_SpotsStatistics.m_dSpacing = Spacing.GetAverage() ;
  m_SpotsStatistics.m_dSpacing_Std = Spacing.GetStd() ;
  Spacing.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dSpacing_75percMin , m_SpotsStatistics.m_dSpacing_75percMax ) ;
  Spacing.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dSpacing_95percMin , m_SpotsStatistics.m_dSpacing_95percMax ) ;
  m_SpotsStatistics.m_dRuling_lpcm = Ruling_lpcm.GetAverage() ;
  m_SpotsStatistics.m_dRuling_lpcm_Std = Ruling_lpcm.GetStd() ;
  Ruling_lpcm.GetValueForPercentOnHistogram( 75 ,
    m_SpotsStatistics.m_dRuling_lpcm_75percMin , m_SpotsStatistics.m_dRuling_lpcm_75percMax ) ;
  Ruling_lpcm.GetValueForPercentOnHistogram( 95 ,
    m_SpotsStatistics.m_dRuling_lpcm_95percMin , m_SpotsStatistics.m_dRuling_lpcm_95percMax ) ;
  m_SpotsStatistics.m_dRuling_lpi = Ruling_lpi.GetAverage() ;
  m_SpotsStatistics.m_dRuling_lpi_Std = Ruling_lpi.GetStd() ;
  Ruling_lpi.GetValueForPercentOnHistogram( 75 , m_SpotsStatistics.m_dRuling_lpi_75percMin , m_SpotsStatistics.m_dRuling_lpi_75percMax ) ;
  Ruling_lpi.GetValueForPercentOnHistogram( 95 , m_SpotsStatistics.m_dRuling_lpi_95percMin , m_SpotsStatistics.m_dRuling_lpi_95percMax ) ;
  m_SpotsStatistics.m_dDotPercent = Dot_Percent.GetAverage() ;
  m_SpotsStatistics.m_dDotPercent_Std = Dot_Percent.GetStd() ;
  Dot_Percent.GetValueForPercentOnHistogram( 75 ,
    m_SpotsStatistics.m_dDotPercent_75percMin , m_SpotsStatistics.m_dDotPercent_75percMax ) ;
  Dot_Percent.GetValueForPercentOnHistogram( 95 ,
    m_SpotsStatistics.m_dDotPercent_95percMin , m_SpotsStatistics.m_dDotPercent_95percMax ) ;

  if (m_iMeasurentCount == 0 || m_bGenerateCaption)
  {
    PutFrame( GetOutputConnector( 1 ) , CreateTextFrame(
      m_SpotsStatistics.GetCaption() , "StatisticsCaption" , pDataFrame->GetId() ) ) ;
    m_bGenerateCaption = FALSE ;
  }
  // Angle with minus because imaging coordinate system (Y is going down).
  FXString Out = m_SpotsStatistics.ToString( ++m_iMeasurentCount , -RadToDeg( m_CalibGrid.m_HorLines[ 0 ]->GetArg() ) ) ;
  PutFrame( GetOutputConnector( 1 ) , CreateTextFrame( Out , "Statistics" , pDataFrame->GetId() ) ) ;
  m_Conturs.clear() ;
  return bRes ;
}


MapControl::MapControl()
  : m_MatrixStep( 100 , 100 )
  , m_SpotSize( 20 , 20 )

{
  m_ConvertMode = FOVtoWORLD;

  m_iNBigDotsX = 5;
  m_iNBigDotsY = 3;
  m_dWCalibStep = 1;
  m_iDisplay = 0 ;
  m_hTargetWnd = m_hScreenOverlapWindow = NULL ;
  m_bIsCalibrated = false;
  m_CalibStage = Inactive ;
  m_OutputMode = modeReplace ;
  m_dLastCalibStartTime = 0. ;

  init();
  m_iInsertMissed = 1 ;
}

void MapControl::AsyncTransaction(
  CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  pParamFrame->Release( pParamFrame );
};

void MapControl::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "Mode" ) ,
    &m_ConvertMode , SProperty::Int , m_pModeList );
  addProperty( SProperty::SPIN , _T( "BigDotsX" ) ,
    &m_iNBigDotsX , SProperty::Int , 0 , 20 );
  addProperty( SProperty::SPIN , _T( "BigDotsY" ) ,
    &m_iNBigDotsY , SProperty::Int , 0 , 20 );
  addProperty( SProperty::SPIN , _T( "NetStep" ) ,
    &m_MatrixStep.cx , SProperty::Int , 20 , 300 );
  addProperty( SProperty::SPIN , _T( "Insert_missed" ) ,
    &m_iInsertMissed , SProperty::Int , 0 , 7 );
  addProperty( SProperty::SPIN , _T( "SpotSizeX_pix" ) ,
    &m_SpotSize.cx , SProperty::Int , 5 , 100 );
  addProperty( SProperty::SPIN , _T( "SpotSizeY_pix" ) ,
    &m_SpotSize.cy , SProperty::Int , 5 , 100 );
  addProperty( SProperty::SPIN , _T( "Display" ) ,
    &m_iDisplay , SProperty::Int , 0 , 10 );
  addProperty( SProperty::EDITBOX , _T( "CalibDataLabel" ) ,
    &m_CalibDataLabel , SProperty::String );
  addProperty( SProperty::EDITBOX , _T( "CalibDataName" ) ,
    &m_CalibDataName , SProperty::String );
};

void MapControl::ConnectorsRegistration()
{
  addInputConnector( transparent , "Input" );
  addOutputConnector( text , "CalibControl" );
  addOutputConnector( transparent , "MainPathControl" );
  addOutputConnector( transparent , "CalibPathControl" );
  addOutputConnector( transparent , "CalibEnable" );
  addDuplexConnector( transparent , transparent , "Control" ) ;
};


// BOOL CALLBACK MonitorEnumProc(
//   HMONITOR hMonitor ,
//   HDC hDC ,
//   LPRECT pRect ,
//   LPARAM Par
// )
// {
//   MapControl * pGadget = (MapControl*) Par ;
//   MONITORINFO info;
//   info.cbSize = sizeof( info );
//   if ( GetMonitorInfo( hMonitor , &info ) )
//   {
//     pGadget->m_MonitorRects.Add( info.rcMonitor ) ;
//   }
//   return TRUE ;
// }


// Global section for overscreen window creating

CPoint ClientTopLeft( 0 , 0 ) ;
CPoint ClientBottomRight( 0 , 0 ) ;
CRect ScreenRect( 0 , 0 , 0 , 0 ) ;
CSize MatrixStep( 0 , 0 ) ;
CSize SpotSize( 0 , 0 ) ;
CSize BigSpots( 5 , 3 ) ;
MapControl * g_pMapControl = NULL ;

CDataFrame* MapControl::DoProcessing( const CDataFrame* pDataFrame )
{
  CDataFrame * pReturnFrame = NULL ;
  const CTextFrame * pCommand = pDataFrame->GetTextFrame() ;
  if (pCommand)
  {
    FXString Label = pCommand->GetLabel() ;
    int iPosCalibrateInText = ( int ) pCommand->GetString().Find( "Calibrate" ) ;
    if (iPosCalibrateInText == 0 || Label == _T( "Calibrate" )
      && ( IsInactive() || ( GetHRTickCount() - m_dLastCalibStartTime ) > 2.0 ))
    {
      SENDINFO( "Calibration request received" ) ;
      FXString MapParams ;
      MapParams.Format( _T( "BigDotsX=%d;BigDotsY=%d;Step_mm=%.3f;" ) ,
        m_iNBigDotsX , m_iNBigDotsY , m_dWCalibStep ) ;
      CTextFrame * pSetMapParams = CreateTextFrame( MapParams , ( LPCTSTR ) NULL ) ;
      PutFrame( GetOutputConnector( 0 ) , pSetMapParams ) ;
      m_dLastCalibStartTime = GetHRTickCount() ;
      //       m_MonitorRects.RemoveAll() ;
      //       EnumDisplayMonitors( NULL , NULL , MonitorEnumProc , (LPARAM) this ) ;

      CMonitorEnum MonitorEnum ;
      if (MonitorEnum.GetNumberOfMonitors() <= 0)
        return NULL ;

      FXPropKit2 pk( pCommand->GetString() ) ;
      pk.GetInt( _T( "Display" ) , m_iDisplay ) ;
      //m_iDisplay-- ;  // our numbering is from zero
      HWND hWnd = NULL ;
      pk.GetInt( _T( "hWnd" ) , ( int& ) hWnd ) ;

      if (m_iDisplay >= MonitorEnum.GetNumberOfMonitors()
        || m_iDisplay < 0)
        m_iDisplay = 0 ;
      if (1)
      {
        if (m_hScreenOverlapWindow)
        {
          PostMessage( m_hScreenOverlapWindow , WM_CLOSE , NULL , NULL ) ;
          PostMessage( m_hScreenOverlapWindow , WM_QUIT , -2 , NULL ) ;
          while (m_hScreenOverlapWindow)
            Sleep( 10 ) ;
        }

        ScreenRect = MonitorEnum.GetMonitorRect( m_iDisplay ) ;
        HWND hTarget = hWnd ;
        if (!hTarget)
        {
          CPoint Center = ScreenRect.CenterPoint() ;
          HWND hTarget = WindowFromPoint( Center ) ;
        }
        if (hTarget)
        {
          m_hTargetWnd = hTarget ;
          m_CalibStage = DrawPattern ;
          CRect Client ;
          BOOL bHasClient = GetClientRect( hTarget , &Client ) ;
          ClientTopLeft = Client.TopLeft() ;
          ClientBottomRight = Client.BottomRight() ;

          ClientToScreen( hTarget , &ClientTopLeft ) ;
          ClientToScreen( hTarget , &ClientBottomRight ) ;
          MatrixStep = m_MatrixStep ;
          SpotSize = m_SpotSize ;
          BigSpots.cx = m_iNBigDotsX ;
          BigSpots.cy = m_iNBigDotsY ;

          if (DrawOnScreen())
          {
            FXString CalibParams ;
            // Mode = 4 - Throw, i.e. don't do nothing until some command
            CalibParams.Format( _T( "Mode=4;Step_mm=%d;Pos_Tolerance_%%=10;Insert_Missed=1;" ) , m_MatrixStep.cx ) ;
            CTextFrame * pCalibControl = CreateTextFrame( ( LPCTSTR ) CalibParams , _T( "CatchCalibControl" ) ) ;
            PutFrame( m_pOutput , pCalibControl ) ;
            CQuantityFrame * pOpenSwitchAndResetAver = CreateQuantityFrame( 1 , _T( "Reset" ) ) ;
            CQuantityFrame * pCloseSwitch = CreateQuantityFrame( 0 ) ;
            pCloseSwitch->AddRef() ;
            PutFrame( GetOutputConnector( 1 ) , pCloseSwitch ) ;
            PutFrame( GetOutputConnector( 2 ) , pOpenSwitchAndResetAver ) ;
            PutFrame( GetOutputConnector( 3 ) , pCloseSwitch ) ;
            m_CalibStage = CalibStarted ;
          }

        }
      }
    }
    else if (( m_CalibStage == CalibStarted )
      && Label == _T( "CalibData" ))
    {
      // Close calibration path and open measurement path
      CQuantityFrame * pOpenSwitch = CreateQuantityFrame( 1 ) ;
      CQuantityFrame * pCloseSwitch = CreateQuantityFrame( 0 ) ;
      pCloseSwitch->AddRef() ;
      PutFrame( GetOutputConnector( 1 ) , pOpenSwitch ) ;
      PutFrame( GetOutputConnector( 2 ) , pCloseSwitch ) ;
      PutFrame( GetOutputConnector( 3 ) , pCloseSwitch ) ;

      // Send message for coordinate conversion
      CTextFrame * pCalibControl = CreateTextFrame( _T( "Mode=0;" ) , _T( "CatchCalibControl" ) ) ;
      PutFrame( m_pOutput , pCalibControl ) ;
      if (m_hScreenOverlapWindow)
      {
        PostMessage( m_hScreenOverlapWindow , WM_CLOSE , NULL , NULL ) ;
        PostMessage( m_hScreenOverlapWindow , WM_DESTROY , NULL , NULL ) ;
      }

      CTextFrame * pPostEnable = CreateTextFrame( "PostControl:Enable" , ( LPCTSTR ) NULL ) ;
      PutFrame( GetOutputConnector( 3 ) , pPostEnable ) ;
    }
    return NULL ;
  }

  const CQuantityFrame * pNumber = pDataFrame->GetQuantityFrame() ;
  if (pNumber)
  {
    if (IsFrameLabelEqual( pNumber , _T( "AverCnt" ) ))
    {
      int iFrameNum = ( int ) ( *pNumber ) ;
      switch (iFrameNum)
      {
        case 15:
          {
            // Send message for start calibration
            CTextFrame * pCalibControl = CreateTextFrame( _T( "Mode=1;" ) , _T( "CatchCalibControl" ) ) ;
            PutFrame( m_pOutput , pCalibControl ) ;
            CQuantityFrame * pOpenSwitch = CreateQuantityFrame( 1 ) ;
            PutFrame( GetOutputConnector( 3 ) , pOpenSwitch ) ;
          }
          break ;
        case 30: // timeout, 
                 // Close calibration path and open measurement path

          {
            CQuantityFrame * pOpenSwitch = CreateQuantityFrame( 1 ) ;
            CQuantityFrame * pCloseSwitch = CreateQuantityFrame( 0 ) ;
            pCloseSwitch->AddRef() ;
            PutFrame( GetOutputConnector( 1 ) , pOpenSwitch ) ;
            PutFrame( GetOutputConnector( 2 ) , pCloseSwitch ) ;
            PutFrame( GetOutputConnector( 3 ) , pCloseSwitch ) ;
            // Send message for coordinate conversion
            CTextFrame * pCalibControl = CreateTextFrame( _T( "Mode=0;" ) , _T( "CatchCalibControl" ) ) ;
            PutFrame( m_pOutput , pCalibControl ) ;
            //RedrawWindow( m_hTargetWnd , NULL , NULL , RDW_INVALIDATE ) ;
            if (m_hScreenOverlapWindow)
            {
              PostMessage( m_hScreenOverlapWindow , WM_CLOSE , NULL , NULL ) ;
              PostMessage( m_hScreenOverlapWindow , WM_DESTROY , NULL , NULL ) ;
              if (m_hTargetWnd)
              {
                CTextFrame * pWndFrame = CreateTextFrame( "hWnd" , ( DWORD ) 0 ) ;
                pWndFrame->GetString().Format( _T( "hWnd=0x%X" ) , m_hTargetWnd ) ;
                PutFrame( GetOutputConnector( 3 ) , pWndFrame ) ;
              }
            }
          }
          break ;
      }
    }
  }

  return NULL ;
}

LRESULT CALLBACK WindowProc( HWND hwnd , UINT uMsg , WPARAM wParam , LPARAM lParam );

int WINAPI wWinMain( HINSTANCE hInstance , HINSTANCE , PWSTR pCmdLine , int nCmdShow )
{
  // Register the window class.
  const TCHAR CLASS_NAME[] = _T( "Sample Window Class" ) ;

  WNDCLASS wc = {};

  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  RegisterClass( &wc );

  // Create the window.

  HWND hwnd = CreateWindowEx(
    0 ,                               // Optional window styles.
    CLASS_NAME ,                      // Window class
    _T( "OverScreen Window" ) ,         // Window text
    WS_BORDER , //WS_OVERLAPPEDWINDOW ,            // Window style

    // Size and position
    ScreenRect.left , ScreenRect.top ,
    ScreenRect.Width() , ScreenRect.Height() ,

    NULL ,       // Parent window    
    NULL ,       // Menu
    hInstance ,  // Instance handle
    NULL        // Additional application data
  );

  if (hwnd == NULL)
  {
    return 0;
  }

  ShowWindow( hwnd , nCmdShow );

  SetWindowPos( hwnd , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOSIZE | SWP_NOMOVE ) ;

  if (g_pMapControl)
  {
    g_pMapControl->m_hScreenOverlapWindow = hwnd ;
  }
  // Run the message loop.

  MSG msg = {};
  while (GetMessage( &msg , NULL , 0 , 0 ))
  {
    TranslateMessage( &msg );
    DispatchMessage( &msg );
  }
  if (g_pMapControl)
  {
    g_pMapControl->m_hScreenOverlapWindow = NULL ;
    g_pMapControl = NULL ;
  }

  return 0;
}

LRESULT CALLBACK WindowProc( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam )
{
  switch (uMsg)
  {
    case WM_DESTROY:
      PostQuitMessage( 0 );
      g_pMapControl->m_bDrawn = false ;

      return 0;

    case WM_PAINT:
      {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint( hWnd , &ps );

        CRect FullScreenClient ;
        GetClientRect( hWnd , &FullScreenClient ) ;
        int iPrevMix = ::SetROP2( hDC , R2_COPYPEN ) ;
        HGDIOBJ hWhiteBrush = GetStockObject( WHITE_BRUSH ) ;
        HGDIOBJ hOldBrush = SelectObject( hDC , hWhiteBrush ) ;
        ::Rectangle( hDC , FullScreenClient.left , FullScreenClient.top ,
          FullScreenClient.right , FullScreenClient.bottom ) ;
        HGDIOBJ hBlackBrush = GetStockObject( BLACK_BRUSH ) ;


        CPoint CreatedClientTopLeft = FullScreenClient.TopLeft() ;
        CPoint CreatedClientBottomRight = FullScreenClient.BottomRight() ;

        ClientToScreen( hWnd , &CreatedClientTopLeft ) ;
        ClientToScreen( hWnd , &CreatedClientBottomRight ) ;

        CPoint TopLeftOrigin = FullScreenClient.TopLeft()
          + ClientTopLeft - CreatedClientTopLeft ;
        CPoint BottomRightLimit = ClientBottomRight - CreatedClientTopLeft ;
        SelectObject( hDC , hBlackBrush ) ;
        MatrixStep.cy = MatrixStep.cx ;
        int iXIndex , iYIndex = 0 ;
        for (int iY = TopLeftOrigin.y + ( MatrixStep.cy / 2 ) ;
          iY < ( BottomRightLimit.y - SpotSize.cy ) ; iY += MatrixStep.cy , iYIndex++)
        {
          iXIndex = 0 ;
          for (int iX = TopLeftOrigin.x + MatrixStep.cx / 2 ;
            iX < ( BottomRightLimit.x - SpotSize.cx ) ; iX += MatrixStep.cx , iXIndex++)
          {
            CSize ThisSpotSize = SpotSize ;
            if (BigSpots.cx && BigSpots.cy)
            {
              if (( ( iYIndex == 0 ) && ( iXIndex < BigSpots.cx ) )
                || ( ( iYIndex < BigSpots.cy ) && ( iXIndex == 0 ) ))
              {
                ThisSpotSize.cx *= 2 ;
                ThisSpotSize.cy *= 2 ;
              }
            }

            CPoint LeftUp( iX - ( ThisSpotSize.cx / 2 ) , iY - ( ThisSpotSize.cy / 2 ) ) ;
            CRect BlackSq( LeftUp , ThisSpotSize ) ;
            //FillRect( hDC , &BlackSq , (HBRUSH) hBlackBrush ) ;
            ::Rectangle( hDC , BlackSq.left , BlackSq.top ,
              BlackSq.right , BlackSq.bottom ) ;
          }
        }
        SelectObject( hDC , hOldBrush ) ;
        ::SetROP2( hDC , iPrevMix ) ;

        EndPaint( hWnd , &ps );
        if (g_pMapControl)
          g_pMapControl->m_bDrawn = true ;
      }
      return 0;

  }
  return DefWindowProc( hWnd , uMsg , wParam , lParam );

}
DWORD WINAPI MapControl::DrawThread( LPVOID Param )
{
  return wWinMain( NULL , NULL , NULL , SW_SHOW ) ;
}

size_t MapControl::DrawOnScreen()
{
  if (m_hScreenOverlapWindow)
  {
    PostMessage( m_hScreenOverlapWindow , WM_CLOSE , NULL , NULL ) ;
    PostMessage( m_hScreenOverlapWindow , WM_QUIT , -2 , NULL ) ;
    while (m_hScreenOverlapWindow)
      Sleep( 10 ) ;
  }
  m_bDrawn = false ;
  g_pMapControl = this ;
  m_hDrawThreadHandle = CreateThread( NULL , 0 , DrawThread , this , NULL , &( this->m_dwDrawThreadId ) ) ;

  return ( size_t ) m_hDrawThreadHandle ;
}
