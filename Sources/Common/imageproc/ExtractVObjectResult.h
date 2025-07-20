
/*
*
* Copyright 2022 by File X Ltd., Ness Ziona, Israel
*
* All Rights Reserved
*
* Permission to use, copy, modify and distribute this
* software and its documentation for any purpose is
* restricted according to the software end user license
* attached to this software.
*
* Any use of this software is subject to the limitations
* of warranty and liability contained in the end user
* license.  Without derogating from the abovesaid,
* File X Ltd. disclaims all warranty with regard to
* this software, including all implied warranties of
* merchantability and fitness.  In no event shall
* File X Ltd. be held liable for any special, indirect
* or consequential damages or any damages whatsoever
* resulting from loss of use, data or profits, whether in
* an action of contract, negligence or other tortious
* action, arising out of or in connection with the use or
* performance of this software.
*
*/
#if !defined(__EXTRACTVOBJECTRESULT_H_)
#define __EXTRACTVOBJECTRESULT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <Gadgets\gadbase.h>
#include <gadgets\stdsetup.h>

#include <fxfc\fxfc.h>
#include "helpers\FramesHelper.h"
#include <math\hbmath.h>
#include <gadgets\shkernel.h>
#include <gadgets\gadbase.h>
#include <Gadgets\TextFrame.h>
#include <math/Intf_sup.h>
#include <imageproc/clusters/segmentation.h>

// following three functions for working with results from TVObjects
// 1. Extract data from text frame with spots measurement results
int ExtractDataAboutSpots(const CTextFrame * pSrcTextFrame ,
  SpotArray& Result , FXString& SpotName ,
  int& iNSpots , int& iMaxIntens , int& iMinIntens);
int ExtractDataAboutSpots(const CTextFrame * pSrcTextFrame ,
  SpotVector& Result , FXString& SpotName ,
  int& iNSpots , int& iMaxIntens , int& iMinIntens);
// 2. Extract data about all measured spots, may be in different text frames
int ExtractDataAboutSpots(const CDataFrame * pSrcFrame , SpotArray& Result);
int ExtractDataAboutSpots(const CDataFrame * pSrcFrame , SpotVector& Result);
// 3. Extract data about all spots, contours and profiles
int ExtractDataAboutSpots(const CDataFrame * pSrc ,
  SpotVectors& MeasurementResults , NamedCmplxVectors& Profiles , 
  NamedCDRects * pROIs = NULL , LPCTSTR pSpotName = NULL );
int ExtractDataAboutROIs( const CDataFrame * pSrcFrame ,
  NamedCDRects& pROIs );


#endif // __EXTRACTVOBJECTRESULT_H_