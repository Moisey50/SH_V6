// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__NormByRect_H__
#define __INCLUDE__NormByRect_H__




#pragma once
#include "helpers\UserBaseGadget.h"
#include "math\Intf_sup.h"

#include "gadgets/ContainerFrame.h"
#include <map>

enum NormWorkingMode
{
  NormNone = 0 ,
  NormByMult = 1 ,
  NormAdd
};

class LabeledAverageData
{
public:
  LabeledAverageData( int iAveraging = 10 )
  {
    m_iAveraging = iAveraging ;
    m_iAveraged = 0 ;
    m_dCurrentValue = 0. ;
    m_dAveragedValue = 0. ;
  }
  LabeledAverageData& operator=( LabeledAverageData& Orig )
  {
    memcpy( this , &Orig , sizeof( LabeledAverageData ) ) ;
    return *this ;
  }

  int m_iAveraged ;
  int m_iAveraging ;
  double m_dCurrentValue ;
  double m_dAveragedValue ;
};

typedef std::map <int , LabeledAverageData> IntDataMap ;
typedef std::map <FXString , LabeledAverageData> NamedDataMap ;

class NormByRect : public UserBaseGadget
{
protected:
	NormByRect();

	int m_iX, m_iY, m_iHeight, m_iWidth;
	double m_dCoefficient;
	BOOL m_bShowROI;
  BOOL m_bSelectByLabel ;
  int  m_iAveraging ;
  NormWorkingMode m_WorkingMode ;
  FXString m_GadgetInfo ;
  IntDataMap m_WaveLengthMap ;
public:

	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  int ExtractWaveLength( FXString& Label ) ;
  LPCTSTR GetGadgetInfo() { return ( LPCTSTR ) m_GadgetInfo ; } ;

	DECLARE_RUNTIME_GADGET(NormByRect);
  CVideoFrame * CorrectByMultiplication( CVideoFrame * pFrame , double dMult );
  CVideoFrame * CorrectAdditiveNoise( CVideoFrame * pFrame , LabeledAverageData& Params );
};

#endif	

