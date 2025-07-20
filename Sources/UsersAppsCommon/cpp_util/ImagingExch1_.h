/********************************************************************
	created:	27/6/2005   17:27
	filename: 	C:\Document\Indigo\OpticJigDEV\ImCNTL\ImagingExch.h
	file path:	C:\Document\Indigo\OpticJigDEV\ImCNTL
	file base:	ImagingExch
	file ext:	h
	author:		Moisey Bernstein
	
	purpose: Shared memory block for exchange with imaging application
*********************************************************************/




#ifndef _IMAGINGEXCH_H__
#define _IMAGINGEXCH_H__

#define MAX_BLOBS (100)
#define MAX_LINES  (30)
typedef struct tagBlobParameters 
{
  double dArea ;
  double dSumPixels ;
  double dMaxIntens ;
//  cmplx  CenterOfGravity ;
//  cmplx  BlobSize ;
  double dMeanPixel ;
  double dFeretElongation ;
  double dAxisPrincipalAngle ;
  CRect  BlobBox ;

  double dRightDiffraction ;
  double dLeftDiffraction ;
  double dDownDiffraction ;
  double dUpDiffraction ;
} BlobParameters;

typedef struct tagLineParameters 
{
  double dLinePos ;
  double dLineAngle ;
  double dLineWidth ;
} LineParameters;

typedef struct tagPowerImagingExch 
{
  int iOperation ;            // Several fields, processing in the order:
                              // Field 0 - grabbing (bits 0-3)
                              // 0 - nothing
                              // 1 - grab
                              // 2 - grab background
                              // 3 - grab and substruct background
                              // 13 - Set Markers 
                              // 14 - Set exposure and sync mode
                              // 15 - set window position

                              // Field 1 - measurement (bits 4-7)
                              // 1 - measure blobs
                              // 2 - measure lines

                              // Field 2 - saving (bits 8-11)
                              // 1 - save image
                              // 2 - save near main blob

  int iProcessingAlgorithm ;  // In:
                              // 0 - no algorithm
                              // 1 - blob measure
                              // 2 - lines measure
  int iViewResultsMode ;      // Like view mode in BIG window

  int iViewImageMode ;        // 0 - no view
                              // 1 - view all captured
                              // 2 - view last in series

  // Parameters
//  cmplx ImageScale_um_per_pixel ;
  
  CSize DiffractionMeasureArea ;
  int   iDiffrMeasureMode ;

  int iMeasExpansionArea ;
  int iIntegrationRadius ;

  int iExposure ;
  int iExpMax ;
  int iExpMin ;
  int iExpStartPosition ;
  int iExpResolution ;

  CRect WindowPosition ;

  int iMinBlobArea ;
  double dNormThresForSizes ;
  double dNormThresForAngles ;
																			
  // Results
  int   iLastMinIntensity ;
  int   iLastMaxIntensity ;

  int   iNLastFoundBlobs ;

  BlobParameters Blobs[ MAX_BLOBS ] ;

  int   iAveragedBlobs ;
  BlobParameters AvrBlobs[ MAX_BLOBS ] ;

  int   iNLastFoundLines ;
  LineParameters Lines[ MAX_LINES ] ;

  int   iAveragedLines ;
  LineParameters AvrLines[ MAX_LINES ] ;

	int iping;
	int iCounter;
	int iDoSingleGrab;
	int iDoContinueGrab ;
	int iIsGrab;
	int iDoQuit;

} PowerImagingExch;
#endif  // _POWERMETEREXCH_H__