// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "GaussGen.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <random>
#include <cmath>
#include <math\intf_sup.h>

//std::default_random_engine generator; 
//std::normal_distribution<double> distribution(/*mean=*/0.0, /*stddev=*/1.0); 

int iSize;
const int iHeartArr[] = {71,70,70,69,70,69,70,70,70,71,74,
  79,86,95,106,112,108,104,96,87,78,74,70,70,69,69,58,49,100,
  165,215,270,330,272,214,167,103,53,20,55,67,73,73,72,71,72,
  70,71,72,72,73,71,75,80,87,95,105,112,120,126,129,132,133,135,
  133,131,129,124,118,110,105,97,90,80,70,71,70,69,70,71,70,71,
  71,76,85,87,88,86,84,80,71,70,71,71,70,71,71,70,71,71} ;

  std::random_device gRandomDevice ;
  std::mt19937       gRandomGen(gRandomDevice()) ;
  std::normal_distribution<double> gDistributionReal(0.,1.) ;
  std::normal_distribution<double> gDistributionImag(5.,2.) ;


USER_FILTER_RUNTIME_GADGET(GaussGen,"Math");	//	Mandatory



GaussGen::GaussGen()
{
  m_dMean = 5;
  m_dSTD = 0.8;
  iHRCounter = 0;
  m_bComplex = false ;
  m_bHeart = false ;
//   m_pRandomDevice = new std::random_device ;
//   m_pRandomGen = new std::mt19937( *m_pRandomDevice ) ;
//   m_pDistributionReal = NULL ;
//   m_pDistributionImag = NULL ;
//   m_NormGen.mu = dMean ;
//   m_NormGen.sig = m_dSTD ;
  m_CurrentReal._Init( m_dMean , m_dSTD ) ;
  m_CurrentImag._Init( m_dMean , m_dSTD ) ;

  gDistributionReal.param( m_CurrentReal ) ;
  gDistributionImag.param( m_CurrentImag ) ;

  init();

  iSize = sizeof(iHeartArr)/sizeof(int);
}
GaussGen::~GaussGen()
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
CDataFrame* GaussGen::DoProcessing(const CDataFrame* pDataFrame) 
{
  if (m_bHeart)
  {
    CContainerFrame * pResult = CContainerFrame::Create() ;
    pResult->ChangeId(pDataFrame->GetId());
    pResult->SetTime(pDataFrame->GetTime());
    pResult->SetLabel( "HeartRate") ;


    int iVal  = iHeartArr[iHRCounter]; 
    CQuantityFrame * ViewQuant = CQuantityFrame::Create(iVal) ;
    ViewQuant->SetLabel("Person's HeartRate");
    ViewQuant->SetTime(pDataFrame->GetTime());
    ViewQuant->ChangeId(pDataFrame->GetId());
    pResult->AddFrame(ViewQuant);

    if(iSize == ++iHRCounter)
      iHRCounter = 0;
    return pResult ;
  }
  else
  {
    CQuantityFrame * pResult = NULL ;
    //double dNextVal = m_NormGen.dev() ;
    m_Protect.Lock() ;
    double dNextVal = gDistributionReal(gRandomGen) ;

    if ( !m_bComplex )
    {
      pResult = CQuantityFrame::Create( dNextVal ) ;
      pResult->SetLabel( "Scalar" ) ;
    }
    else  // complex numbers
    {
      double dNextImag = gDistributionImag(gRandomGen) ;
      CDPoint NewPt( dNextVal , dNextImag ) ;
      pResult = CQuantityFrame::Create( NewPt ) ;
      pResult->SetLabel( "Complex" ) ;
    }
    m_Protect.Unlock() ;
    pResult->ChangeId(pDataFrame->GetId());
    pResult->SetTime(pDataFrame->GetTime());
    return pResult ;
  }

  return NULL ;
}

void GaussGen::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (!ParamText)
  {
    if ( ParamText->GetString().MakeLower().Find( "heart" ) > -1 )
    {
      m_bHeart = TRUE;
    }
    else
    {
      m_bHeart = FALSE;
      m_StockArr.RemoveAll();
      FXSIZE iTok = 0 ;
      while ( iTok != -1 )
      {
        FXString fx = ParamText->GetString().Tokenize( "," , iTok );
        if ( fx.GetLength() > 2 )
          m_StockArr.Add( fx );
      }
    }

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

void GaussGen::PropertiesRegistration() 
{

  addProperty(SProperty::EDITBOX		,	_T("Mean")	,	&m_sMean		,	SProperty::String		);
  addProperty(SProperty::EDITBOX		,	_T("STD")	,	&m_sStdDev		,	SProperty::String		);

};


void GaussGen::ConnectorsRegistration() 
{
  addInputConnector( transparent, "SourceName");
  addOutputConnector( text , "OutputNumber");
  addDuplexConnector( transparent, transparent, "Filter");
};

bool GaussGen::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  bool bRes = UserGadgetBase::ScanProperties( text , Invalidate ) ;
  double dData[10] ;
  memset( dData , 0 , sizeof( dData ) ) ;
  FXPropKit2 pk(text) ;
  int iNNumbers = GetArray( pk , "Mean" , 'f' , 10 , dData ) ;
  if ( iNNumbers )
  {
    if ( m_cmplxStd.imag() <= 0. )
      m_cmplxStd._Val[1] = DBL_MIN ;
    if ( m_cmplxStd.real() <= 0. )
      m_cmplxStd._Val[ 0 ] = DBL_MIN ;
    if ( m_dSTD <= 0. )
      m_dSTD = DBL_MIN ;
    m_CurrentReal._Init( m_cmplxMean._Val[0] = dData[0] , m_cmplxStd.real() ) ;
    m_dMean = dData[ 0 ] ;
    
    m_bComplex = ( iNNumbers >= 2 ) ;
    if ( m_bComplex )
    {
      m_cmplxMean = cmplx( dData[0] , dData[1] ) ;
      //m_ImagNormGen.SetParams( dData[1] , m_cmplxStd.imag() ) ;
      m_CurrentImag._Init( m_cmplxMean.imag() , m_cmplxStd.imag() ) ;
      m_sMean.Format( "%g,%g" , dData[ 0 ] , dData[ 1 ] ) ;
    }
    else
      m_sMean.Format( "%g" , dData[ 0 ] ) ;

//     else
//       m_cmplxMean._Val[1] = m_cmplxMean._Val[0] ;
  }
  iNNumbers = GetArray( pk , "STD" , 'f' , 10 , dData ) ;
  if ( iNNumbers )
  {
    //m_NormGen.SetParams( dMean , dSTD = dData[0] ) ;
    m_CurrentReal._Init( m_cmplxMean.real() , m_cmplxStd._Val[1] = dData[0] ) ;
    m_dSTD = dData[ 0 ] ;
    if ( m_bComplex && (iNNumbers == 2) )
    {
      m_cmplxStd = cmplx( dData[0] , dData[1] ) ;
      //m_ImagNormGen.SetParams( m_cmplxMean.real() , m_cmplxStd.imag() ) ;
      m_CurrentImag._Init( m_cmplxMean.imag() , m_cmplxStd.imag() ) ;
      m_sStdDev.Format( "%g,%g" , dData[ 0 ] , dData[ 1 ] ) ;
    }
    else
    {
      if ( dData[0] < 0. )
        dData[0] = 0.0 ;
      m_cmplxStd = cmplx( dData[0] , dData[0] ) ;
      //m_ImagNormGen.SetParams( dMean , m_cmplxStd.imag() ) ;
      m_CurrentImag._Init( m_cmplxMean.imag() , m_cmplxStd.imag() ) ;
      m_sStdDev.Format( "%g" , dData[ 0 ] ) ;
    }
  }
  m_Protect.Lock() ;
  gDistributionReal.param( m_CurrentReal ) ;
  gDistributionImag.param( m_CurrentImag ) ;
  m_Protect.Unlock() ;
  return bRes ;
}

bool GaussGen::PrintProperties(FXString& text)
{
  FXPropKit2 pk(text) ;
  if ( !m_bComplex )
  {
    pk.WriteDouble( "STD" , m_dSTD ) ;
    pk.WriteDouble( "Mean" , m_dMean ) ;
  }
  else
  {
    pk.WriteCmplx( "Mean" , m_cmplxMean ) ;
    pk.WriteCmplx( "STD" , m_cmplxStd ) ;
  }
  text = (LPCTSTR)pk ;
  return true ;
}




