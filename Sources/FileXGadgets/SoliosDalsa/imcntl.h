// Machine generated IDispatch wrapper class(es) created with ClassWizard


#include "SMSoliosRequest.h"

typedef CArray<CSMSoliosRequest *> RequestArray ;

/////////////////////////////////////////////////////////////////////////////
// IImCNTL wrapper class


#define FIND_BAD_PIXELS 10
#define SET_GAIN 11
#define SET_ANALOG_OFFSET 12
#define SET_NORM_THRESH   13
#define SET_FIXED_THRESHOLD     14
#define GET_NEAREST_SPOT_NUMBER 15
#define RESET_ALL_BACKGROUNDS   16
#define IS_GRAPH_BASED_PROCESSING 17
#define SET_DELAY_IN_SCANS          18
#define SET_SCAN_TIME               19
#define SET_NORM_ROTATION_THRESH    20

class IImCNTL : public COleDispatchDriver
{
public:
	IImCNTL() {}		// Calls COleDispatchDriver default constructor

// Attributes
public:
  static RequestArray m_RQImaging ;
  double m_dScanTime_usec ;

// Operations
public:
	long GrabBack(int iCamNum = 0);
	long Grab(long bSubstrBackint , int iCamNum = 0);
	long MeasureBlobs(double dNormThreshold , int iCamNum = 0);
	CString GetBlobParam(long iBlobNumber , int iCamNum = 0);
	void SetFFTFlag(long bDoFFT , int iCamNum = 0);
	void SetCameraScale(double dScaleXmicronPerPixel, double dScaleYmicronPerPixel , int iCamNum = 0);
	double GetCameraScale(long iAxeNumber , int iCamNum = 0);
	long InitOperation(long InitValue , int iCamNum = 0);
  long GetExposure( int iCamNum = 0);
  long SetExposure(long iNewExposureInScans , int iCamNum = 0);
	long MeasureLines(double dNormThres , int iCamNum = 0);
	CString GetLineParam(long LineNumber , int iCamNum = 0);
	long GetLastMinIntensity( int iCamNum = 0);
	long GetLastMaxIntensity( int iCamNum = 0);
	void SetFFTZone(long iXc, long iYc , int iCamNum = 0);
	long DoFFTFiltration( int iCamNum = 0);
	long SaveRestoreImage(long bSave , int iCamNum = 0);
	double SetGetMeasAreaExpansion(long bSet, double dExp_in_um , int iCamNum = 0);
	double GetMaxIntensity( int iCamNum = 0);
	long SetWinPos(long lx, long ly , int iCamNum = 0);
	void SetExposureStartPosition(long iExpStartPos , int iCamNum = 0);
	long GetExposureStartPosition( int iCamNum = 0);
	long MeasurePower( int iCamNum = 0);
	CString GetPowerData(long iBlobNumber , int iCamNum = 0);
	void SetIntegrationRadius(long iRadius , int iCamNum = 0);
	long GetIntegrationRadius( int iCamNum = 0);
	double MeasLinePower(long iLineDirection, long iMeasureHalfWidth, long iCompensationWidth , int iCamNum = 0);
	void SetMinArea(long iMinArea , int iCamNum = 0);
	double AbstractCall(long iCallID, LPCTSTR pszParameters, long iPar, double dPar , int iCamNum = 0);
	long SetSyncMode(long bAsyncMode , int iCamNum = 0);
	long SetMarkerPosisiton(long iMarkerType, long iXPos, long iYPos, long dwMarkerColor , int iCamNum = 0);
	CString GetDiffractionMeasPar( int iCamNum = 0);
	CString SetDiffractionMeasPar(long iMeasMode, long iXDist_pix, long iYDist_pix, long iBackgoundDist_pix , int iCamNum = 0);
  long GetGain( int iCamNum = 0);
  long SetGain(long iNewExposureInScans , int iCamNum = 0);
  long GetTriggerDelay( int iCamNum = 0);
  long SetTriggerDelay(long iNewTriggerDelay_uS , int iCamNum = 0);
  long GetDebouncing( int iCamNum = 0);
  long SetDebouncing(long iNewDebouncing_units , int iCamNum = 0);
  long SavePicture (int iSave , int iCamNum = 0 );
  long EnableLPFilter (int iEnable, int iCamNum );
};
