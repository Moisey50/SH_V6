#pragma once

#include <fxfc\fxfc.h>
#include "gadgets\videoframe.h"
#include "gadgets\textframe.h"
#include "gadgets\QuantityFrame.h"
#include <gadgets\FigureFrame.h>
#include <imageproc/ImageProfile.h>
#include <helpers\FramesHelper.h>
#include <Math\FRegression.h>

double CheckForVLineOnHBlackStrip( const CVideoFrame * pVF ,
  CRect& rcLinePos , // !!! right holds width and bottom holds height
  double& dLineWidth , // Output line width
  double dNormThres = 0.5 , 
  CContainerFrame * pMarking = NULL ) ;
