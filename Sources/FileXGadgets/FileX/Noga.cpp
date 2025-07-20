// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "Noga.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <random>
#include <cmath>
#include <math\intf_sup.h>

//std::default_random_engine generator; 
//std::normal_distribution<double> distribution(/*mean=*/0.0, /*stddev=*/1.0); 

USER_FILTER_RUNTIME_GADGET(Noga,"Math");	//	Mandatory

void MeasPtsUpdate( LPCTSTR pPropertyName , void * pObject , bool& bInvalidate , bool& bRescanProperties )
{
  Noga * pGadget = (Noga*) pObject ;
  FXString PtsAsString = pGadget->m_sMeasurementPts ;
  if ( !PtsAsString.IsEmpty() )
  {
    FXAutolock al( pGadget->m_LockSettings , _T( "MeasPtsUpdate" ) ) ;
    pGadget->m_MeasPts.RemoveAll() ;

    FXSIZE iPos = 0 ;
    FXString Token = PtsAsString.Tokenize( _T( " \t,;" ) , iPos ) ;
    double dVal ;
    while ( iPos >= 0 )
    {

      if ( sscanf( (LPCTSTR) Token , "%lf" , &dVal ) )
      {
        pGadget->m_MeasPts.Add( dVal ) ;
        Token = PtsAsString.Tokenize( _T( " \t,;" ) , iPos ) ;
      }
      else
        break ;
    }
    if ( pGadget->m_MeasPts.GetCount() )
    {
      double * pArr = pGadget->m_MeasPts.GetData() ;
      bool bSorted = true ;
      do
      {
        bool bSorted = true ;
        for ( int i = 0; i < pGadget->m_MeasPts.GetCount() ; i++ )
        {
          if ( i < pGadget->m_MeasPts.GetCount() - 1 )
          {
            if ( pArr[i] > pArr[ i + 1] )
            {
              swapdouble( pArr[i] , pArr[i + 1] ) ;
              bSorted = false ;
            }
          }
        }
      } while ( !bSorted );
    }
  }
}



Noga::Noga()
{
  m_dScaleMicronPerPix = 19.8;
  m_MainConturName = _T( "spot_main" ) ;
  m_TripplePtsName = _T( "tripple_pts" ) ;
  m_sMeasurementPts = "0.1,0.5,0.9" ;
  init();
  bool bDummy ;
  MeasPtsUpdate( _T( "RelMeasPts" ) , this , bDummy , bDummy ) ;
}

Noga::~Noga()
{
//   if ( m_pDistributionReal )
//   {
//     delete m_pDistributionReal ;
//     m_pDistributionReal = NULL ;
//   }
//   if ( m_pDistributionImag )
//   {
//     delete m_pDistributionImag ;
//     m_pDistributionImag = NULL ;
//   }
//   if ( m_pRandomGen )
//   {
//     delete m_pRandomGen ;
//     m_pRandomGen = NULL ;
//   }
//   if ( m_pRandomDevice )
//   {
//     delete m_pRandomDevice ;
//     m_pRandomDevice = NULL ;
//   }
}
CDataFrame* Noga::DoProcessing(const CDataFrame* pDataFrame) 
{
  const CVideoFrame * pNewImage = pDataFrame->GetVideoFrame() ;
  if ( !pNewImage )
    return NULL ;

  CFramesIterator * pIt = pDataFrame->CreateFramesIterator( figure ) ;
  if ( !pIt )
    return NULL ;

  FXArray<CFigureFrame *> Mains ;
  FXArray<CFigureFrame *> TripplePts ;
  while ( CFigureFrame * pNextFigure = (CFigureFrame *) pIt->Next() )
  {
    FXString Label = pNextFigure->GetLabel() ;
    if ( pNextFigure->GetCount() > 10 )
    {
      if ( Label.Find( m_MainConturName ) >= 0 )
      {
        Mains.Add( pNextFigure ) ;
      }
      else if ( Label.Find( m_TripplePtsName ) >= 0 )
      {
        TripplePts.Add( pNextFigure ) ;
      }
    }
  };

  delete pIt ;

  if ( Mains.GetCount() < 1 
    || TripplePts.GetCount() < 2 )
  {
    return NULL ;
  }

  TRACE( "\n    NMains=%d NTripples=%d" , Mains.GetCount() , TripplePts.GetCount() ) ;

  CFigureFrame * pMain = Mains[ 0 ] ;

  bool bLeftFirst = (TripplePts[ 0 ]->GetAt( 0 ).x < TripplePts[ 1 ]->GetAt( 0 ).x) ;

  CFigureFrame * pLeftBlack = TripplePts[ bLeftFirst ? 0 : 1 ] ;
  CFigureFrame * pRightBlack = TripplePts[ bLeftFirst ? 1 : 0 ] ;
  CmplxArray LeftExtrems , RightExtrems , MainExtrems ;

  cmplx FirstAver = FindExtrems( pLeftBlack , LeftExtrems ) ;
  cmplx SecondAver = FindExtrems( pRightBlack , RightExtrems ) ;


  cmplx TrippleLeft = LeftExtrems[ 2 ] ;
  cmplx TrippleRight = RightExtrems[ 0 ] ;

  CFigureFrame* pLeftTripple = CreateFigureFrame( &TrippleLeft , 1 , GetHRTickCount() ,
    "0x0000ff" , "LeftTripple" , pDataFrame->GetId() ) ;
  CFigureFrame* pRightTripple = CreateFigureFrame( &TrippleRight , 1 , GetHRTickCount() ,
    "0x00ffff" , "RightTripple" , pDataFrame->GetId() ) ;

  CContainerFrame * pOut = CContainerFrame::Create() ;
  pOut->AddFrame( pMain ) ;
  pMain->AddRef() ;
  pOut->AddFrame( pLeftBlack ) ;
  pLeftBlack->AddRef() ;
  CFigureFrame * pRightWithDiffColor = (CFigureFrame*)pRightBlack->Copy() ;
  pRightWithDiffColor->Attributes()->WriteString( "color" , "0x00ffff" ) ;
  pOut->AddFrame( pRightWithDiffColor ) ;
  pOut->AddFrame( pLeftTripple ) ;
  pOut->AddFrame( pRightTripple ) ;
  pOut->AddFrame( pNewImage ) ;
  //((CVideoFrame*)pNewImage)->AddRef() ;

  FXIntArray MainExtremsIndexes ;
  FindExtrems( pMain , MainExtrems , &MainExtremsIndexes ) ;

  cmplx MainRightPt = MainExtrems[ 2 ] ;
  CFigureFrame* pMainRightPt = CreateFigureFrame( &MainRightPt , 1 , GetHRTickCount() ,
    "0xffff00" , "MainExtreme" , pDataFrame->GetId() ) ;
  pOut->AddFrame( pMainRightPt ) ;

  int iNearestIndexRightUp = -1 , iNearestIndexRightDown = -1 ;
  
  cmplx EdgeVect = TrippleLeft - TrippleRight ;
  FXAutolock al( m_LockSettings , _T( "DoProcessing" ) ) ;
  for ( int i = 0; i < m_MeasPts.GetCount() ; i++ )
  {
    cmplx PtNearEdge = TrippleRight + m_MeasPts[ i ] * EdgeVect ;
    cmplx NearestUp = FindNearest( pMain , PtNearEdge , iNearestIndexRightUp ,
      true , MainExtremsIndexes[ 2 ] , false ) ;
    cmplx NearestDown = FindNearest( pMain , PtNearEdge , iNearestIndexRightDown ,
      true , MainExtremsIndexes[ 2 ] , true ) ;

    CFigureFrame* pCross = CreateFigureFrame( &NearestUp , 1 , GetHRTickCount() ,
      "0x0000ff" , "LeftTripple" , pDataFrame->GetId() ) ;
    pCross->AddPoint( CmplxToCDPoint( NearestDown ) ) ;
    pOut->AddFrame( pCross ) ;
    double dCrossLen = abs( NearestUp - NearestDown ) * m_dScaleMicronPerPix ;
    FXString Msg ;
    Msg.Format( _T( "W=%.1f" ) , dCrossLen ) ;
    cmplx cPos = NearestUp + cmplx( -10. , -40. ) ;
    CTextFrame * pText = CreateTextFrame( cPos , Msg , "0x0000ff" , 12 , "CrossWidth" ) ;
    pOut->AddFrame( pText ) ;
  }

  return pOut ;
}

void Noga::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (ParamText)
  {
    //if (ParamText->GetString().MakeLower().Find("heart")>-1)
    //{
    //  m_bHeart = TRUE;
    //}
    //else
    //{
    //  m_bHeart = FALSE;
    //  m_StockArr.RemoveAll();
    //  int iTok = 0 ;
    //  while(iTok!=-1)
    //  {
    //    FXString fx = ParamText->GetString().Tokenize(",",iTok);
    //    if (fx.GetLength() > 2)				
    //      m_StockArr.Add(fx);
    //  }
    //}

    //	m_HistArr.RemoveAll();
    //	m_SingleArr.RemoveAll();
    //	m_IntArray.RemoveAll();
    //	for (int i = 0 ; i < m_StockArr.GetSize() ; i++)
    //	{
    // 		std::random_device rd;
    // 		std::mt19937 gen(rd());
    // 		std::normal_distribution<> d(dMean,dSTD);
    // 		std::map<int, int> hist;
    // 		for(int n=0; n<10000; ++n)
    // 		{
    // 			++hist[ROUND(d(gen))];
    // 		}
    // 		for(int i = 0 ;i < 10; i++)
    // 		{
    // 			std::default_random_engine generator; 
    // 			std::normal_distribution<double> distribution(/*mean=*/0.0, /*stddev=*/1.0); 
    // 			double randomNumber = distribution(generator) ;
    // 			m_SingleArr.Add(randomNumber );
    // 		}
    // 
    // 		for(int i = 0 ; i < m_SingleArr.GetSize(); i++)
    // 		{
    // 			std::default_random_engine generator; 
    // 			std::normal_distribution<double> distribution(dMean, dSTD); 
    // 			double randomNumber = distribution(generator) * m_SingleArr.GetAt(i);
    // 			m_SingleArr.GetAt( i ) = randomNumber;
    // 		}
    // 		m_IntArray.Add(m_SingleArr);
    // 	}
  }
  pParamFrame->Release( pParamFrame );
};

void Noga::PropertiesRegistration() 
{

  addProperty(SProperty::EDITBOX		,	_T("Scale_um_per_pix")	,	&m_dScaleMicronPerPix ,	SProperty::Double	);
  addProperty(SProperty::EDITBOX		,	_T("MainConturName")	,	&m_MainConturName ,	SProperty::String		);
  addProperty( SProperty::EDITBOX , _T( "TripplePtsName" ) , &m_TripplePtsName , SProperty::String );
  addProperty( SProperty::EDITBOX , _T( "RelMeasPts" ) , &m_sMeasurementPts , SProperty::String );

  SetChangeNotification( _T( "RelMeasPts" ) , MeasPtsUpdate , this ) ;

};


void Noga::ConnectorsRegistration() 
{
  addInputConnector( transparent, "FromTVObjects");
  addOutputConnector( transparent , "OutputImage");
  addOutputConnector( transparent , "OutputData" );
  
  addDuplexConnector( transparent, transparent, "Control");
};

//bool Noga::ScanProperties(LPCTSTR text, bool& Invalidate)
//{
//  bool bRes = UserGadgetBase::ScanProperties( text , Invalidate ) ;
//  double dData[10] ;
//  memset( dData , 0 , sizeof( dData ) ) ;
//  FXPropKit2 pk(text) ;
//  int iNNumbers = GetArray( pk , "Mean" , 'f' , 10 , dData ) ;
//  if ( iNNumbers )
//  {
//    if ( m_cmplxStd.imag() <= 0. )
//      m_cmplxStd._Val[1] = -DBL_MAX ;
//    if ( m_cmplxStd.real() <= 0. )
//      m_cmplxStd._Val[ 0 ] = -DBL_MAX ;
//    if ( m_dSTD <= 0. )
//      m_dSTD = -DBL_MAX ;
//    m_CurrentReal._Init( m_cmplxMean._Val[0] = dData[0] , m_cmplxStd.real() ) ;
//    m_dMean = dData[ 0 ] ;
//    
//    m_bComplex = ( iNNumbers >= 2 ) ;
//    if ( m_bComplex )
//    {
//      m_cmplxMean = cmplx( dData[0] , dData[1] ) ;
//      //m_ImagNormGen.SetParams( dData[1] , m_cmplxStd.imag() ) ;
//      m_CurrentImag._Init( m_cmplxMean.imag() , m_cmplxStd.imag() ) ;
//      m_sMean.Format( "%g,%g" , dData[ 0 ] , dData[ 1 ] ) ;
//    }
//    else
//      m_sMean.Format( "%g" , dData[ 0 ] ) ;
//
////     else
////       m_cmplxMean._Val[1] = m_cmplxMean._Val[0] ;
//  }
//  iNNumbers = GetArray( pk , "STD" , 'f' , 10 , dData ) ;
//  if ( iNNumbers )
//  {
//    //m_NormGen.SetParams( dMean , dSTD = dData[0] ) ;
//    m_CurrentReal._Init( m_cmplxMean.real() , m_cmplxStd._Val[1] = dData[0] ) ;
//    m_dSTD = dData[ 0 ] ;
//    if ( m_bComplex && (iNNumbers == 2) )
//    {
//      m_cmplxStd = cmplx( dData[0] , dData[1] ) ;
//      //m_ImagNormGen.SetParams( m_cmplxMean.real() , m_cmplxStd.imag() ) ;
//      m_CurrentImag._Init( m_cmplxMean.imag() , m_cmplxStd.imag() ) ;
//      m_sStdDev.Format( "%g,%g" , dData[ 0 ] , dData[ 1 ] ) ;
//    }
//    else
//    {
//      if ( dData[0] < 0. )
//        dData[0] = 0.0 ;
//      m_cmplxStd = cmplx( dData[0] , dData[0] ) ;
//      //m_ImagNormGen.SetParams( dMean , m_cmplxStd.imag() ) ;
//      m_CurrentImag._Init( m_cmplxMean.imag() , m_cmplxStd.imag() ) ;
//      m_sStdDev.Format( "%g" , dData[ 0 ] ) ;
//    }
//  }
//  m_Protect.Lock() ;
//  gDistributionReal.param( m_CurrentReal ) ;
//  gDistributionImag.param( m_CurrentImag ) ;
//  m_Protect.Unlock() ;
//  return bRes ;
//}

//bool Noga::PrintProperties(FXString& text)
//{
//  FXPropKit2 pk(text) ;
//  if ( !m_bComplex )
//  {
//    pk.WriteDouble( "STD" , m_dSTD ) ;
//    pk.WriteDouble( "Mean" , m_dMean ) ;
//  }
//  else
//  {
//    pk.WriteCmplx( "Mean" , m_cmplxMean ) ;
//    pk.WriteCmplx( "STD" , m_cmplxStd ) ;
//  }
//  text = (LPCTSTR)pk ;
//  return true ;
//}




