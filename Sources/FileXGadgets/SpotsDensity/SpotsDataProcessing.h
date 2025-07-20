#ifndef __INCLUDE__SpotsDataProcessing_H__
#define __INCLUDE__SpotsDataProcessing_H__

#pragma once

#include <math/Intf_sup.h>
#include <Math/FRegression.h>
#include "gadgets\ContainerFrame.h"
#include <helpers/FramesHelper.h>
#include "helpers/StatisticsForSmallDotsAndLines.h"
#include <Calibration\Grid.h>
#include "Math\R2.h"




class SpotsDataProcessing
{
public:
  FXString m_CalibDataLabel ;
  FXString m_CalibDataName ;
  FXString m_TextObjectName ;
  Grid m_CalibGrid;
  vector< CFigureFrame* > m_Conturs ;
  double   m_dScale_um_per_pixel = 1.00 ;
  SmallSpotsStatistics m_SpotsStatistics ;
  int     m_iMeasurentCount = 0 ;
  BOOL    m_bGenerateCaption ;
  bool    m_bIsCalibrated = false ;

  SpotsDataProcessing() ;
  bool DoCalibrationProcess(
    const CDataFrame* pDataFrame , bool bUnconditional ) ;
  size_t ExtractConturs( const CDataFrame * pDataFrame ) ;
  bool FormStatistics( const CDataFrame * pDataFrame ,
    FXString& ResultString ) ;
};

#endif // __INCLUDE__SpotsDataProcessing_H__