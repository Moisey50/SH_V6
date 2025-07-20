// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__HSCGadget_H__
#define __INCLUDE__HSCGadget_H__

#pragma once
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <cassert>
#include <iostream>

#include "helpers\UserBaseGadget.h"
#include <helpers\FramesHelper.h>
#include <map>
#include <fxfc\fxfc.h>
#include "CSVM_Detector.h"

#define WMode_CSVM                    0
#define WMode_Teaching                1
#define WMode_Recognition_Amplitudes  2
#define WMode_Recognition_Ratios      3
#define WMode_Recognition_Ratios_Norm 4

LPCTSTR GetWorkingModeName( DWORD mode ) ;

enum ScanMode
{
  SCAN_NoScan = 0 ,
  SCAN_Jumps ,
  SCAN_Sweep
};

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// RGB -> YUV
#define RGB2Y(R, G, B) CLIP(( (  66 * (R) + 129 * (G) +  25 * (B) + 128) >> 8) +  16)
#define RGB2U(R, G, B) CLIP(( ( -38 * (R) -  74 * (G) + 112 * (B) + 128) >> 8) + 128)
#define RGB2V(R, G, B) CLIP(( ( 112 * (R) -  94 * (G) -  18 * (B) + 128) >> 8) + 128)

#define MAX_NComponents 50
struct Value
{
  double brightness;
  double error;
};

typedef pair<int, double> SpectrumComponent;

class Spectrum
{
public:
  int         m_iMinWL_nm;
  int         m_iMaxWL_nm;
  int         m_iStepWL_nm;
  double      m_dMaxValue ;
  std::string m_Name;
  DWORD       m_Color; // as RGB
  DWORD       m_YUVColor;
  bool        m_bReadyForRecognition;
  vector <double> m_CurrentRecognition ;
  double      m_dLastValues[MAX_NComponents];
  vector<SpectrumComponent> m_Data;

  Spectrum(LPCTSTR Name = NULL, DWORD dwColor = 0xffffff)
  {
    if ( Name )
      m_Name = Name;
    m_Color = dwColor ;
    m_iMinWL_nm = INT_MAX ;
    m_iMaxWL_nm = INT_MIN ;
    m_iStepWL_nm = 0;
    m_dMaxValue = 0. ;
    m_bReadyForRecognition = false;
  }

  int Add(SpectrumComponent& NewComp)
  {
    if (NewComp.first < m_iMinWL_nm)
      m_iMinWL_nm = NewComp.first;
    if (NewComp.first > m_iMaxWL_nm)
      m_iMaxWL_nm = NewComp.first;

    for (size_t i = 0 ; i < m_Data.size(); i++ )
    {
      if (NewComp.first > m_Data[i].first)
        continue;
      else if ((NewComp.first < m_Data[i].first))
      {
        m_Data.insert( m_Data.begin() + i , NewComp );
        m_bReadyForRecognition = false;
        return (int)i;
      }
      else
      {
        m_Data[i] = NewComp;
        m_bReadyForRecognition = false;
        return (int) i;
      }
    }
    m_Data.push_back(NewComp);
    m_bReadyForRecognition = false;
    return (int) m_Data.size() - 1 ;
  }
   
  Spectrum& operator=(Spectrum& Orig)
  {
    m_iMinWL_nm = Orig.m_iMinWL_nm;
    m_iMaxWL_nm = Orig.m_iMaxWL_nm;
    m_iStepWL_nm = Orig.m_iStepWL_nm;
    m_Name = Orig.m_Name;
    m_Color = Orig.m_Color;
    m_Data = Orig.m_Data;
    m_CurrentRecognition = Orig.m_CurrentRecognition ;
    m_bReadyForRecognition = Orig.m_bReadyForRecognition;
    return *this;
  }

  void ToString( FXString& Result , bool bInParenthesis = true )
  {
    FXPropKit2 SpectrumAsText ;
    if ( bInParenthesis )
      SpectrumAsText += ("(\n") ;
    SpectrumAsText.WriteString( "Name" , m_Name.c_str() ) ;
    SpectrumAsText.WriteInt( "MinWL_nm" , m_iMinWL_nm ) ;
    SpectrumAsText.WriteInt( "MaxWL_nm" , m_iMaxWL_nm ) ;
    SpectrumAsText.WriteInt( "StepWL_nm" , m_iStepWL_nm ) ;
    SpectrumAsText.WriteLong( "Color" , m_Color ) ;

    FXString  DataAsString( "\nData=(" ) ;
    FXString Add ;
    for ( size_t i = 0; i < m_Data.size() ; i++ )
    {
      Add.Format( "\n%d,%.1f" , m_Data[ i ].first , m_Data[ i ].second ) ;
      DataAsString += Add ;
    }
    DataAsString += _T("\n") ;
    if ( bInParenthesis )
      DataAsString += (")") ;
    DataAsString += (";") ;
    Result += SpectrumAsText + DataAsString + _T("\n)") ;
  }
  int FromString( FXString& AsString )
  {
    FXPropKit2 pk(AsString) ;
    if ( IsOpenBracket( pk[ 0 ] ) )
      pk.Delete( 0 ) ;
  
    FXString Tmp ;
    if ( !pk.GetString( "Name" , Tmp ) )
      return 0 ;
    m_Name = Tmp ;
    pk.GetLong( "Color" , (long&)m_Color ) ;
    pk.GetInt( "StepWL_nm" , m_iStepWL_nm ) ;

    FXSIZE iDataPos = pk.Find( "Data=(" ) ;
    if ( iDataPos < 0 )
      return 0 ;
   // int CloseParen = pk.Find( _T( ')' ) , iDataPos ) ;

    FXString Token = pk.Tokenize( "\n\r" , iDataPos ) ;  // omit first \n
    Token = pk.Tokenize( "\n\r" , iDataPos ) ;
    int iWL ;
    double dVal ;
    while ( (Token.GetLength() > 5) && (iDataPos >= 0) )
    {
      Token.Trim() ;
      if ( sscanf_s( Token , "%d,%lf" , &iWL , &dVal ) < 2 )
        break ;
      SpectrumComponent NewComp( iWL , dVal ) ;
      Add( NewComp ) ;
      Token = pk.Tokenize( "\n\r" , iDataPos ) ;
    }
    return (int) m_Data.size() ;
  }

};

typedef vector<Spectrum> Spectrums ;



struct PixelDebug
{
  int    Spectrumindex;
  int    ActiveIndex;
  char   Name[20];
  double dDiff;
  double dPattern[3];
  double dValues[3];
  double dNotNormValues[3];
  double dDiffs[3];

};

struct AllSpectrums
{
  PixelDebug Values[8];
};
struct ComparisonInfo
{
  int material_1;
  int material_2;
  double clarity;


};


typedef multimap< int, vector< ComparisonInfo > > Map;
typedef multimap< int, vector< ComparisonInfo > >::iterator Iterator;
typedef pair< Iterator, Iterator > Range;

vector< int > getTestingFequencies(vector< vector< Value > > const& Inputs);
vector< int > getBestOverlap(vector< Range > Ranges, vector< int > Counts, int nCouples);
int getNumOfCouplesPresent(vector< Range > const& Ranges);
vector< vector< Value > > getInputs(int nMaterials, int nFrequencies, double error);

template < typename T >
vector< T > getSetValues(set< T > const& s)
{
  vector< T > v;
  for (set< T >::const_iterator p = s.begin(); p != s.end(); p++)
    v.push_back(*p);
  return v;
}


//**********************************************************************************
// routines

inline bool Snap( const CVideoFrame * pFr , CRect& Rect )
{
  if ( Rect.left < 0 )
    Rect.left = 0 ;
  if ( Rect.right >= (LONG)GetWidth( pFr ) )
    Rect.right = GetWidth( pFr ) - 1 ;
  if ( Rect.top < 0 )
    Rect.top = 0 ;
  if ( Rect.bottom >= ( LONG ) GetHeight( pFr ) )
    Rect.bottom = GetHeight( pFr ) - 1 ;
  return !Rect.IsRectEmpty() ;
}

inline double GetAverage( const CVideoFrame * pFr , CRect Rect )
{
  if ( !Snap( pFr , Rect ) )
    return -1. ;
  __int64 i64Sum = 0 ;
  LPBYTE pData = GetData( pFr ) ;
  if ( Is8BitsImage( pFr ) )
  {
    for ( int iY = Rect.top ; iY <= Rect.bottom ; iY++ )
    {
      LPBYTE p = pData + iY * GetWidth( pFr ) + Rect.left ;
      LPBYTE pEnd = p + Rect.Width() ;
      while ( p <= pEnd )
        i64Sum += *( p++ ) ;
    }
    int iArea = ( Rect.Width() + 1 ) * ( Rect.Height() + 1 ) ;
    return ( double ) i64Sum / ( double ) iArea ;
  }
  else if ( Is16BitsImage( pFr ) )
  {
    for ( int iY = Rect.top ; iY <= Rect.bottom ; iY++ )
    {
      LPWORD p = ( ( LPWORD ) pData ) + iY * GetWidth( pFr ) + Rect.left ;
      LPWORD pEnd = p + Rect.Width() ;
      while ( p <= pEnd )
        i64Sum += *( p++ ) ;
    }
    int iArea = ( Rect.Width() + 1 ) * ( Rect.Height() + 1 ) ;
    return ( double ) i64Sum / ( double ) iArea ;
  }
  return -2. ;
}
inline double GetStatistics( const CVideoFrame * pFr , CRect Rect ,
  double& dStd , bool bDebug = false )
{
  double dAver ;
  if ( !Snap( pFr , Rect ) )
    return -1. ;
  __int64 i64Sum = 0 ;
  LPBYTE pData = GetData( pFr ) ;
  DWORD dwWidth = GetWidth( pFr ) ;
  if ( Is8BitsImage( pFr ) )
  {
    for ( int iY = Rect.top ; iY <= Rect.bottom ; iY++ )
    {
      LPBYTE p = pData + iY * GetWidth( pFr ) + Rect.left ;
      LPBYTE pEnd = p + Rect.Width() ;
      while ( p <= pEnd )
        i64Sum += *( p++ ) ;
    }
    int iArea = ( Rect.Width() + 1 ) * ( Rect.Height() + 1 ) ;
    dAver = ( double ) i64Sum / ( double ) iArea ;
    double dStdSum = 0. ;
    for ( int iY = Rect.top ; iY <= Rect.bottom ; iY++ )
    {
      LPBYTE p = pData + iY * GetWidth( pFr ) + Rect.left ;
      LPBYTE pEnd = p + Rect.Width() ;
      while ( p <= pEnd )
      {
        double dVal = *( p++ ) - dAver ;
        dStdSum += dVal * dVal ;
      }
    }
    if ( dStdSum > 1e-6 )
    {
      dStd = dStdSum / iArea ;
      dStd = sqrt( dStd ) ;
    }
    return dAver ;
  }
  else if ( Is16BitsImage( pFr ) )
  {
    for ( int iY = Rect.top ; iY <= Rect.bottom ; iY++ )
    {
      LPWORD p = ( ( LPWORD ) pData ) + iY * GetWidth( pFr ) + Rect.left ;
      LPWORD pEnd = p + Rect.Width() ;
      while ( p <= pEnd )
        i64Sum += *( p++ ) ;
    }
    int iArea = ( Rect.Width() + 1 ) * ( Rect.Height() + 1 ) ;
    dAver = ( double ) i64Sum / ( double ) iArea ;
    double dStdSum = 0. ;
    for ( int iY = Rect.top ; iY <= Rect.bottom ; iY++ )
    {
      int iShift = iY * dwWidth + Rect.left ;

      LPWORD p = GetData16( pFr ) ;
      p += iShift ;
      LPWORD pBeg = p ;
      LPWORD pEnd = p + Rect.Width() ;
      int iTraceCnt = 0 ;
      while ( p <= pEnd )
      {
        double dVal = *( p ) ;
        double dDiff = dVal - dAver ;
        double d2Val = dDiff * dDiff ;
        dStdSum += d2Val ;
        p++ ;
      }
    }
    if ( dStdSum > 1e-6 )
    {
      double d2Std = dStdSum / iArea ;
      dStd = sqrt( d2Std ) ;
    }
    return dAver ;
  }
  return -2. ;
}

class Sample
{
public:
  Sample(double dAver = 0. , double dStd = 0. , 
    double dRefBlack = 0. , double dRefWhite = 0. )
  {
    m_dAver = dAver ;
    m_dStd = dStd ;
    m_dRefBlack = dRefBlack ;
    m_dRefWhite = dRefWhite ;
    m_dWtoBAmpl = m_dRefWhite - m_dRefBlack ;
    m_dRatio = m_dNormalizedByRef = 0. ;
  }

  Sample& operator =( const Sample& Orig )
  {
    memcpy( this , &Orig , sizeof( Sample ) ) ;
    return *this ;
  }
  double m_dAver ;
  double m_dStd ;
  double m_dRefBlack ;
  double m_dRefWhite ;
  double m_dNormalizedByRef ;
  double m_dRatio ;
  double m_dWtoBAmpl ;
};

typedef std::map<int , Sample> SampleMap ;
typedef std::map<int , CVideoFrame*> VFMap ;

class HSCGadget : public UserBaseGadget
{
protected:
	FXLockObject m_Lock;
	typedef struct
	{
		int masterLambda;
		int currentLambda;
		double k1;
	}ObjectParam;

	HSCGadget();
  ~HSCGadget();

	typedef FXArray<ObjectParam> ObjectParams;

	//std::map<int, FXArray<double>>m_objectsMap;
  FXString     m_GadgetInfo ;
	double m_dEpsilon;
  double m_dAmplThres ; // < 1.; relatively to WBASE
  int    m_iNChannels ;
  int    m_WorkingMode ;
  int    m_OldWorkingMode ;
  CRect  m_TeachingArea ;
  CRect  m_BlackRefArea ;
  CRect  m_WhiteRefArea ;
  CRect  m_NewRefArea ;
  int    m_iTeachingCounter ;
  int    m_iLastReceivedWL_nm ;
	int    m_currentWaveLenth_nm;
  int    m_iRequestCount ;
	int    m_masterWaveLenth_nm;
  int    m_iLastOrderedWL ;
  double m_dLastWLOrderTime ;
  double m_dWaveLengthStep_nm ;
  double m_dMinWaveLength_nm ;
  bool   m_bInitialConfig ;
  bool   m_bRefChanged;
  bool   m_bWasRecognition ;
  BOOL   m_bWrite ;
  DWORD  m_dwFrameCnt ;
  FXString m_ConfigFileName ;
  FXString m_sWaveLengths ;
  ScanMode m_ScanMode ;
  FXString m_sCommandFormat ;
  BOOL     m_bUseInternal ;
  bool     m_bSpectrumsChanged ;
  int      m_iNotActiveFrameCntr;
  int      m_iFramesDivider;


  CSVM_Detector m_Detector ;

  FXIntArray m_WaveLengths ;
  int        m_iCurrentWL ;
//   int        m_iCommandsCounter ;
  //SampleArray m_LearnedData ;
  SampleMap m_LearnedData ;

  Spectrums  m_Spectrums ;
  vector<int> m_ActiveSpectrums;

	VFMap m_videoFramesMap;

	FXArray<ObjectParams> m_objectsArr;

	//FXArray<FXArray<double>>m_objectsArr;

	LPBYTE  m_outputData;
	std::map<int, FXArray<double>> m_Ratios;

	int CalculateRatio();
	bool ExtractWaveLength(const CDataFrame* pDataFrame);
  int GetChannel( int iWaveLength_nm ) ;
	int ExtractObjects(const CDataFrame* pDataFrame);
	int AddImageToRepository(const CDataFrame* pDataFrame);
  int AddSample( int iWaveLength_nm , Sample& NewSample ) ;
	void CalculateRatio(const CVideoFrame* pMasterFrame, const CVideoFrame* pVideoFrame, int waveLength);
	void UpdateMatrix(const ObjectParams* pMaster, const FXArray<double>* pRatios, int brightness);
  int  GetNWaveLengths() { return (int) m_videoFramesMap.size() ; }

	void Compare();
	void CreateMatrix();

	bool VerifyWaveLengths();
	bool FindLambdaInImageRepo(int lambda);
  bool SetConfigParameters( LPCTSTR pFileName , bool bINsistWrite = false ) ;
  bool SaveConfigParameters( LPCTSTR pFileName ) ;
  bool LoadConfigParameters( LPCTSTR pFileName ) ;
  bool SendNextWaveLength( int iWaveLength_nm , int iTextPin = 2 , int iQuantityPin = -1 ) ;

	int Clear();
  static void ConfigParamChange( LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bRescanProperties) ;

public:

	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  LPCTSTR GetGadgetInfo() { return ( LPCTSTR ) m_GadgetInfo ; } ;
  CVideoFrame* RecognizeCSVM() ;
  CVideoFrame* RecognizeSimple();
  CVideoFrame* RecognizeByRatioPatterns();
  int          CalcTargetRatios();
  int          InitRecognitionForRatio() ;

  bool IsWorkingByRatios( )
  {
    return (m_WorkingMode == WMode_Recognition_Ratios
      ||    m_WorkingMode == WMode_Recognition_Ratios_Norm) ;
  }
	DECLARE_RUNTIME_GADGET(HSCGadget);
};

#endif	

