// DlgProxy.cpp : implementation file
//

#include "stdafx.h"
#include "ImCNTL.h"
#include "DlgProxy.h"
#include "ImCNTLDlg.h"
#include "helpers\Registry.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImCNTLDlgAutoProxy

IMPLEMENT_DYNCREATE(CImCNTLDlgAutoProxy, CCmdTarget)

CImCNTLDlgAutoProxy::CImCNTLDlgAutoProxy()
{
	EnableAutomation();
	
	// To keep the application running as long as an automation 
	//	object is active, the constructor calls AfxOleLockApp.
	AfxOleLockApp();

	// Get access to the dialog through the application's
	//  main window pointer.  Set the proxy's internal pointer
	//  to point to the dialog, and set the dialog's back pointer to
	//  this proxy.
	ASSERT (AfxGetApp()->m_pMainWnd != NULL);
	ASSERT_VALID (AfxGetApp()->m_pMainWnd);
	ASSERT_KINDOF(CImCNTLDlg, AfxGetApp()->m_pMainWnd);
	m_pDialog = (CImCNTLDlg*) AfxGetApp()->m_pMainWnd;
	m_pDialog->m_pAutoProxy = this;
	m_iLock = 0;
  m_View = m_pDialog->m_ViewFrame ;
}

CImCNTLDlgAutoProxy::~CImCNTLDlgAutoProxy()
{
	// To terminate the application when all objects created with
	// 	with automation, the destructor calls AfxOleUnlockApp.
	//  Among other things, this will destroy the main dialog
	if (m_pDialog != NULL)
  {
    m_pDialog->m_pAutoProxy = NULL;
    //m_pDialog->m_ViewFrame->Clean() ;
    IApp()->m_iViewExist = 0;
  }
	AfxOleUnlockApp();

}

void CImCNTLDlgAutoProxy::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CImCNTLDlgAutoProxy, CCmdTarget)
	//{{AFX_MSG_MAP(CImCNTLDlgAutoProxy)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CImCNTLDlgAutoProxy, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CImCNTLDlgAutoProxy)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GrabBack", GrabBack, VT_I4, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "Grab", Grab, VT_I4, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "MeasureBlobs", MeasureBlobs, VT_I4, VTS_R8)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetBlobParam", GetBlobParam, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetFFTFlag", SetFFTFlag, VT_EMPTY, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetCameraScale", SetCameraScale, VT_EMPTY, VTS_R8 VTS_R8)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetCameraScale", GetCameraScale, VT_R8, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "InitOperation", InitOperation, VT_I4, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetExposure", GetExposure, VT_I4, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetExposure", SetExposure, VT_I4, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "MeasureLines", MeasureLines, VT_I4, VTS_R8)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetLineParam", GetLineParam, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetLastMinIntensity", GetLastMinIntensity, VT_I4, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetLastMaxIntensity", GetLastMaxIntensity, VT_I4, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetFFTZone", SetFFTZone, VT_EMPTY, VTS_I4 VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "DoFFTFiltration", DoFFTFiltration, VT_I4, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SaveRestoreImage", SaveRestoreImage, VT_I4, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetGetMeasAreaExpansion", SetGetMeasAreaExpansion, VT_R8, VTS_I4 VTS_R8)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetMaxIntensity", GetMaxIntensity, VT_R8, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetWinPos", SetWinPos, VT_I4, VTS_I4 VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetExposureStartPosition", SetExposureStartPosition, VT_EMPTY, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetExposureStartPosition", GetExposureStartPosition, VT_I4, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "MeasurePower", MeasurePower, VT_I4, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetPowerData", GetPowerData, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetIntegrationRadius", SetIntegrationRadius, VT_EMPTY, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetIntegrationRadius", GetIntegrationRadius, VT_I4, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "MeasLinePower", MeasLinePower, VT_R8, VTS_I4 VTS_I4 VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetMinArea", SetMinArea, VT_EMPTY, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "AbstractCall", AbstractCall, VT_R8, VTS_I4 VTS_BSTR VTS_I4 VTS_R8)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetSyncMode", SetSyncMode, VT_I4, VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetMarkerPosisiton", SetMarkerPosisiton, VT_I4, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "GetDiffractionMeasPar", GetDiffractionMeasPar, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CImCNTLDlgAutoProxy, "SetDiffractionMeasPar", SetDiffractionMeasPar, VT_BSTR, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IImCNTL to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {A7F453B7-3255-11D3-8D4E-000000000000}
static const IID IID_IImCNTL =
{ 0xa7f453b7, 0x3255, 0x11d3, { 0x8d, 0x4e, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };

BEGIN_INTERFACE_MAP(CImCNTLDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CImCNTLDlgAutoProxy, IID_IImCNTL, Dispatch)
END_INTERFACE_MAP()

// The IMPLEMENT_OLECREATE2 macro is defined in StdAfx.h of this project
// {A7F453B5-3255-11D3-8D4E-000000000000}
IMPLEMENT_OLECREATE2(CImCNTLDlgAutoProxy, "ImCNTL.Application", 0xa7f453b5, 0x3255, 0x11d3, 0x8d, 0x4e, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0)

/////////////////////////////////////////////////////////////////////////////
// CImCNTLDlgAutoProxy message handlers

long 
CImCNTLDlgAutoProxy::GrabBack() 
{
	m_View->CheckAndStopGrab() ;

	return m_View->GrabBack() ;
}

long 
CImCNTLDlgAutoProxy::Grab(long bSubstrBack) 
{
  // edited on 30.10.09 by Alex
  // code taken from SOLIOS + adjustments

//   double dStart = get_current_time() ;
  if ( m_View->m_LiveVideo )
  {
    m_View->OnLiveVideo() ;
    m_View->m_LiveVideo = FALSE ;
  }
  m_View->SingleGrab( true ) ;
//   double dLen = get_current_time() - dStart ;

  return 1 ; 
}

long 
CImCNTLDlgAutoProxy::MeasureBlobs(double dNormThreshold) 
{
  if ( dNormThreshold == 0.0 )
  {
    if ( !m_View->WaitEndOfGrabAndProcess() )
      return 0 ;
    CMeasurementSet * pSet = &m_View->m_MeasSets[0] ;
    return pSet->m_SpotResults.GetSize() ;
  }
  else
  {
    m_View->SetThreshold( dNormThreshold ) ;
    CString Control( "Task(0)") ;
    for ( int i = 0 ; i < m_View->m_MeasSets.GetCount() ; i++ )
    {
      CTextFrame * p = CTextFrame::Create( Control ) ;
      if ( !m_View->m_pBuilder->SendDataFrame( p , m_View->m_MeasSets[i].m_ProcessControlName ) )
        p->Release( p );
    }
    return 1 ;
  }
}

BSTR 
CImCNTLDlgAutoProxy::GetBlobParam(long iBlobNumber) 
{

	CString strResult;
  if ( ! m_View->m_MeasSets.GetCount() )
  {
    strResult.Format( "ERROR: No measurement devices" ) ;
    return strResult.AllocSysString(); 
  }
  if ( !m_View->WaitEndOfGrabAndProcess() )
  {
    strResult.Format( "ERROR: Timeout on grab and measure" ) ;
    return strResult.AllocSysString(); 
  }

  CMeasurementSet * pSet = &m_View->m_MeasSets[0] ;

  pSet->m_SpotResults.Lock() ;
  if ( iBlobNumber == -1 )
    iBlobNumber = pSet->m_iMaxBlobNumber ;
  else if(iBlobNumber < 0 
    &&  iBlobNumber >= pSet->m_SpotResults.GetSize())
  {
    pSet->m_SpotResults.Unlock() ;
    strResult.Format( "ERROR: Blob %d doesn't exists" ,iBlobNumber ) ;
    return strResult.AllocSysString(); 
  }
  
  if ( iBlobNumber >= 0   
    && iBlobNumber <= pSet->m_SpotResults.GetSize() )
  {
	  CColorSpot mySpot = pSet->m_SpotResults.GetAt( iBlobNumber);
    pSet->m_SpotResults.Unlock() ;
    double dRat;
    if( mySpot.m_dLongDiametr && mySpot.m_dShortDiametr )
         dRat =  mySpot.m_dLongDiametr / mySpot.m_dShortDiametr ;
      else
       dRat = 0;//prevent dividing by zero

    if (mySpot.m_Area > 100. && mySpot.m_dBlobHeigth > 0 && mySpot.m_dBlobWidth > 0 )
    {
      if ( m_View->m_ImParam.m_iDiffractionMeasurementMethod == 0 )
      {
        strResult.Format( "%d %d %6.2f %6.2f %6.2f %5.2f %d %6.2f %6.2f %6.2f %6.2f %6.2f %8.1f %8.1f" ,
         ROUND(mySpot.m_SimpleCenter.x) ,
         ROUND(mySpot.m_SimpleCenter.y),
         mySpot.m_dAngle  ,
         mySpot.m_dBlobWidth ,
         mySpot.m_dBlobHeigth ,
         dRat,//mySpot.m_dLongDiametr / (mySpot.m_dShortDiametr > 0) ? mySpot.m_dShortDiametr : 1, 
          ( int )mySpot.m_iMaxPixel,
          (( int )mySpot.m_iMaxPixel > 0 ) ? 
             ( 100. *mySpot.m_dRDiffraction /mySpot.m_iMaxPixel) 
              :
              0.,
          (( int )mySpot.m_iMaxPixel > 0 ) ? 
             ( 100. *mySpot.m_dLDiffraction /mySpot.m_iMaxPixel) 
              :
              0.,
          (( int )mySpot.m_iMaxPixel > 0 ) ? 
             ( 100. *mySpot.m_dDDiffraction /mySpot.m_iMaxPixel) 
              :
              0.,
          (( int )mySpot.m_iMaxPixel > 0 ) ? 
             ( 100. *mySpot.m_dUDiffraction /mySpot.m_iMaxPixel) 
              :
              0.,
          mySpot.m_dAngle ,     //for two angle measure methods results presentation 
          mySpot.m_dCentral5x5Sum ,
          mySpot.m_dSumOverThreshold
         ) ;
      }
      else if ( m_View->m_ImParam.m_iDiffractionMeasurementMethod == 1 )
      {
        strResult.Format( "%d %d %6.2f %6.2f %6.2f %5.2f %d %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %8.1f %8.1f",
          ROUND(mySpot.m_SimpleCenter.x) ,
          ROUND(mySpot.m_SimpleCenter.y),
         mySpot.m_dAngle   ,
         mySpot.m_dBlobWidth ,
         mySpot.m_dBlobHeigth ,
         dRat,
         mySpot.m_iMaxPixel,
          100. *mySpot.m_dRDiffraction ,
          100. *mySpot.m_dLDiffraction ,
          100. *mySpot.m_dDDiffraction ,
          100. *mySpot.m_dUDiffraction ,
          100. *mySpot.m_dCentralIntegral ,
					mySpot.m_dAngle ,
          mySpot.m_dCentral5x5Sum ,
          mySpot.m_dSumOverThreshold
         ) ;
      }
    }
    else
      strResult.Format( "ERROR: Small blob area %4.0f or bad focus" ,mySpot.m_Area ) ;
        
  }
  else
  {
    pSet->m_SpotResults.Unlock() ;
    strResult.Format( "ERROR: Blob %d doesn't exists" ,iBlobNumber ) ;
  }
	return strResult.AllocSysString();
}

void 
CImCNTLDlgAutoProxy::SetFFTFlag(long bDoFFT) 
{
  m_View->m_ImParam.m_UseFFTFilter = bDoFFT ;
}

void 
CImCNTLDlgAutoProxy::SetCameraScale(
  double dScaleXmicronPerPixel, 
  double dScaleYmicronPerPixel ) 
{
  m_View->m_dScaleXmicronPerPixel = dScaleXmicronPerPixel ;
  m_View->m_dScaleYmicronPerPixel = dScaleYmicronPerPixel ;
}

double 
CImCNTLDlgAutoProxy::GetCameraScale(long iAxeNumber) 
{
	switch( iAxeNumber )
  {
  case 0: return m_View->m_dScaleXmicronPerPixel ;
  case 1: return m_View->m_dScaleXmicronPerPixel ;
  }

	return 0.0;
}

long 
CImCNTLDlgAutoProxy::InitOperation(long InitValue) 
{
// Next part taken from SOLIOS - 29.10.09 Alex
  if ( InitValue == -1 )
  {
    m_View->m_RButtonDlg.m_iViewMode = 0 ;
    ::ShowWindow( m_View->m_BigViewWnd.m_hWnd , SW_HIDE ) ;

  }

	return 0;
}

long 
CImCNTLDlgAutoProxy::GetExposure() 
{	
  if ( m_View->m_MeasSets[0].m_iExposure > m_View->m_dScanTime / 4 )
    return ROUND(m_View->m_MeasSets[0].m_iExposure / m_View->m_dScanTime) ;
  else
    return m_View->m_MeasSets[0].m_iExposure ;
}

long 
CImCNTLDlgAutoProxy::SetExposure(long iNewExposureInScans) 
{		
  return m_View->SetExposure( iNewExposureInScans ) ;
}

long
CImCNTLDlgAutoProxy::MeasureLines(double dNormThres) 
{
  if ( dNormThres == 0.0 )
  {
    if ( !m_View->WaitEndOfGrabAndProcess() )
      return 0 ;
    CMeasurementSet * pSet = &m_View->m_MeasSets[0] ;
    return pSet->m_ProcLResults.GetSize() ;
  }
  else
  {
    m_View->SetThreshold( dNormThres ) ;
    CString Control( "Task(1)") ;
    for ( int i = 0 ; i < m_View->m_MeasSets.GetCount() ; i++ )
    {
      CTextFrame * p = CTextFrame::Create( Control ) ;
      if ( !m_View->m_pBuilder->SendDataFrame( p , m_View->m_MeasSets[i].m_ProcessControlName ) )
        p->Release( p );
    }
    return 1 ;
  }
}

BSTR
CImCNTLDlgAutoProxy::GetLineParam(long iLineNumber) 
{
  CString strResult;
  if ( ! m_View->m_MeasSets.GetCount() )
  {
    strResult.Format( "ERROR: No measurement devices" ) ;
    return strResult.AllocSysString(); 
  }
  if ( !m_View->WaitEndOfGrabAndProcess() )
  {
    strResult.Format( "ERROR: Timeout on grab and measure" ) ;
    return strResult.AllocSysString(); 
  }

  CMeasurementSet * pSet = &m_View->m_MeasSets[0] ;

  if ( iLineNumber < 0 )
    iLineNumber = 0 ;

  pSet->m_LineResults.Lock() ;
  if ( iLineNumber >= 0   
    && iLineNumber < pSet->m_LineResults.GetCount() )
  {
    CLineResult Line = pSet->m_LineResults[iLineNumber] ;
    pSet->m_LineResults.Unlock() ;
    double dLineThikness = Line.m_DRect.bottom - Line.m_DRect.top ;
    if ( dLineThikness > 5. )
    {
      strResult.Format( "%d %6.2f %6.2f %6.2f %8.1f %8.1f %d %d" ,
        iLineNumber ,
        Line.m_Center.y , dLineThikness , Line.m_dAngle ,
        Line.m_dExtremalAmpl , Line.m_dAverCent5x5 ,
        Line.m_ImageMinBrightness , Line.m_ImageMaxBrightness ) ;
    }
    else
      strResult.Format( "ERROR: Small line %d thickness %4.0f" ,
        iLineNumber , dLineThikness ) ;
  }
  else
  {
    pSet->m_LineResults.Unlock() ;
    strResult.Format( "ERROR: Line %d doesn't exists" ,
      iLineNumber ) ;
  }

	return strResult.AllocSysString();
}

long CImCNTLDlgAutoProxy::GetLastMinIntensity() 
{
  if ( !m_View->WaitEndOfGrabAndProcess() )
    return 0 ;

  return m_View->m_iLastMinIntens ;
}

long CImCNTLDlgAutoProxy::GetLastMaxIntensity() 
{
  if ( !m_View->WaitEndOfGrabAndProcess() )
    return 0 ;
  
  return m_View->m_iLastMaxIntens ;
}
double CImCNTLDlgAutoProxy::GetMaxIntensity() 
{
  if ( !m_View->WaitEndOfGrabAndProcess() )
    return 0 ;

  return m_View->m_iLastMaxIntens ;
}

void CImCNTLDlgAutoProxy::SetFFTZone(long iXc, long iYc) 
{
  if ( iXc < IMAGE_WIDTH/4 )
    iXc = IMAGE_WIDTH/4 ;
  else
    if ( iXc > ( ( ( IMAGE_WIDTH * 3 )/4 ) - 1 ) ) 
      iXc = ( ( IMAGE_WIDTH * 3 )/4 ) - 1 ;

  if ( iYc < IMAGE_HEIGHT/4 )
    iYc = IMAGE_HEIGHT/4 ;
  else
    if ( iYc > ( ( ( IMAGE_HEIGHT * 3 )/4 ) - 1 ) ) 
      iYc = ( ( IMAGE_HEIGHT * 3 )/4 ) - 1 ;
  
  m_View->m_ImParam.m_iFFTXc = iXc ;
  m_View->m_ImParam.m_iFFTYc = iYc ;
}

long 
CImCNTLDlgAutoProxy::DoFFTFiltration() 
{
  m_View->OnFiltration() ;

	return 1;
}

long 
CImCNTLDlgAutoProxy::SaveRestoreImage(long bSave) 
{
  m_View->SaveRestoreImage( ( bSave )? true : false ) ;

	return 0;
}

double 
CImCNTLDlgAutoProxy::SetGetMeasAreaExpansion(
  long bSet, double dExp_in_um) 
{
  double Old = m_View->m_ImParam.m_iMeasExpansion * m_View->m_dScaleXmicronPerPixel ;
  if ( bSet )
    m_View->m_ImParam.m_iMeasExpansion = 
      ( int )( fabs( dExp_in_um / m_View->m_dScaleXmicronPerPixel ) + 0.5 ) ;

	return Old ;
}

// double CImCNTLDlgAutoProxy::GetMaxIntensity() 
// {
// 	CRect Img(5,5,m_View->m_CaptureSize.cx - 5,m_View->m_CaptureSize.cy - 5);
// 	m_View->FindMinMaxIntens(Img,1);
// 
// 	return m_View->m_iLastMaxIntens;
// }

long CImCNTLDlgAutoProxy::SetWinPos(long lx, long ly) 
{
	m_pDialog->m_ViewFrame->SetWindowPos( NULL, lx, ly, 0, 0, SWP_NOSIZE | SWP_NOZORDER ) ;
  m_pDialog->m_ViewFrame->BringWindowToTop() ;
  return 1;
}

void CImCNTLDlgAutoProxy::SetExposureStartPosition(long iExpStartPos) 
{
// 	m_View->m_ExposureControl.m_iExpStart = iExpStartPos ;
//   m_View->m_ImParam.m_iExposureBegin = iExpStartPos ;
// 
//   m_View->m_ExposureControl.SetVSyncFront( iExpStartPos < 300 ) ;
}

long CImCNTLDlgAutoProxy::GetExposureStartPosition() 
{
  //return m_View->m_ExposureControl.m_iExpStart ;
  return 0 ;
}

long CImCNTLDlgAutoProxy::MeasurePower() 
{
//   if ( dNormThresh < 1. )
//     m_View->m_ImParam.m_BinThreshold = dNormThresh ; //27.4.09 Alex Bernstein
//   else
//     m_View->m_ImParam.m_BinThreshold = 0.5 ;
 
  m_View->m_ImParam.m_BinThreshold = m_View->m_dNormThreshold ;
  if ( m_View->m_ImParam.m_BinThreshold > 1 )
  m_View->m_ImParam.m_BinThreshold = 0.5 ;
 
  m_View->OnMeasurePower() ;

	return m_View->m_NBlobs ;
}

BSTR CImCNTLDlgAutoProxy::GetPowerData(long iBlobNumber) 
{
  //////////////////////////////////////////////////////////////////////////
  /*
  Edited by Alexandra Bernstein 
  6.10.2009
  To make this function suitable for the Improved Camera Linearity test
  */
  //////////////////////////////////////////////////////////////////////////
	CString strResult;
  if ( iBlobNumber == -1 )
    iBlobNumber = m_View->m_iMaxBlobNumber ;

  int iNSpots = m_View->m_ColSpots.GetSize() ;
  if ( iBlobNumber >= 0   
    && iBlobNumber < iNSpots )
  {
    CColorSpot MySpot = m_View->m_ColSpots.GetAt( iBlobNumber ) ;
    // previously, before the 6.10.09 change, used m_View arrays such as 
    // m_View->m_dArea for spot areas,
    // m_View->m_dSumPower for spot powers etc.
    // Those arrays are not filled by the "OnMeasurePower" function
    // and therefore it is wrong to use them. See function OnMeasurePower
    // Alex
      
    if ( MySpot.m_Area > 100. )
    {
      strResult.Format( "%13.1f %6d %6d %6.2f %6.2f " ,
        MySpot.m_dSumPower ,
        MySpot.m_SimpleCenter.x ,
        MySpot.m_SimpleCenter.y ,
        MySpot.m_dBlobWidth ,
        MySpot.m_dBlobHeigth
      ) ;
    }
    else
      strResult.Format( "ERROR: Small blob area %4.0f" ,
        m_View->m_dArea[ iBlobNumber ] ) ;
  }
  else
    strResult.Format( "ERROR: Blob %d doesn't exists" ,
        iBlobNumber ) ;

	return strResult.AllocSysString();
}

void CImCNTLDlgAutoProxy::SetIntegrationRadius(long iRadius) 
{
	m_View->m_ImParam.m_iPowerRadius = iRadius ;
}

long CImCNTLDlgAutoProxy::GetIntegrationRadius() 
{
  return m_View->m_ImParam.m_iPowerRadius ;
}

double CImCNTLDlgAutoProxy::MeasLinePower(
  long iLineDirection, long iMeasureHalfWidth, long iCompensationWidth) 
{
  return 0.0 ;
//   return m_View->MeasLinePower( 
//     iLineDirection , iMeasureHalfWidth ,
//     200 , iCompensationWidth , &m_View->m_dAverSum ) ;
//   m_View->m_iLastMaxIntens = ROUND(m_View->m_dAverSum) ;
}

void CImCNTLDlgAutoProxy::SetMinArea(long iMinArea) 
{
	m_View->m_ImParam.m_iMinArea = iMinArea ;

}

double CImCNTLDlgAutoProxy::AbstractCall(
  long iCallID, LPCTSTR pszParameters, long iPar, double dPar) 
{
  switch(iCallID)
  {
  case SET_GAIN:
    {
      int igain = (int)dPar;
      int icam = iPar ;
      m_View->SetGain( igain , icam );
      break;
    }
    
  case FIND_BAD_PIXELS: 
    {
//       m_View->OnFindBadPixels();
      break;
    }
  case SET_ANALOG_OFFSET: 
    {
      m_View->m_iAnalogOffSet = iPar;
      //m_View->SetAnalogOffset(1);
      break;
    }
  case SET_NORM_THRESH:
    {
      m_View->m_dNormThreshold = dPar ;
      break ;
    }
  case SET_FIXED_THRESHOLD:
    {
      m_View->m_ImParam.m_bFixThreshold = (iPar != 0) ;
      if ( dPar > 0.  &&  dPar < 4095. )
        m_View->m_ImParam.m_iThresholdValue = ROUND(dPar) ;
      break ;
    }
    break ;
  case GET_NEAREST_SPOT_NUMBER:
    {
      cmplx Center( (double)iPar , dPar ) ;
      int iNearest = -1 ;
      double dMinDist = 10e6 ;
      for ( int i = 0 ; i < m_View->m_NBlobs ; i++ )
      {
        cmplx BlobC( (double)m_View->m_ColSpots[i].m_SimpleCenter.x , (double)m_View->m_ColSpots[i].m_SimpleCenter.y  ) ;
        double dDist = abs( Center - BlobC ) ;
        if ( dDist < dMinDist )
        {
          dMinDist = dDist ;
          iNearest = i ;
        }
      }
      return ROUND( iNearest ) ;
    }
    break ;
  case RESET_ALL_BACKGROUNDS:
    // m_View->m_Backgrounds.RemoveAll() ;
    break ;
  case IS_GRAPH_BASED_PROCESSING:
    return m_View->m_MeasSets.GetCount() ;
  }

	return 0.0;
}

long CImCNTLDlgAutoProxy::SetSyncMode(long bAsyncMode) 
{
  m_View->m_bAsyncGrab = (bAsyncMode == 0) ;  // inverse
  m_View->OnAsynchroneMode() ;
//   for ( int i = 0 ; i < m_View->m_MeasSets.GetCount() ; i++ )
//     m_View->SetGrabTrigger( (bAsyncMode == 0) , i ) ;
//    ;
//   m_View->m_wndToolBar.SetButtonInfo(
//     11, ID_ASYNCHRONE, TBBS_BUTTON, (bAsyncMode==0)? 8 : 20) ;

  return bAsyncMode;
}

long CImCNTLDlgAutoProxy::SetMarkerPosisiton(
  long iMarkerType, long iXPos, long iYPos, long dwMarkerColor) 
{
  m_View->m_MarkerType = iMarkerType ;
  m_View->m_MarkerPos = CPoint( iXPos , iYPos ) ;
  m_View->m_MarkerColor = dwMarkerColor ;

  m_View->Invalidate() ;

	return 0;
}

BSTR CImCNTLDlgAutoProxy::GetDiffractionMeasPar() 
{
	CString strResult;

  strResult.Format( "%d %d %d %d" , 
    m_View->m_ImParam.m_iDiffractionMeasurementMethod ,
    m_View->m_ImParam.m_iDiffractionRadius ,
    m_View->m_ImParam.m_iDiffractionRadius_Y ,
    m_View->m_ImParam.m_iBackgroundDist ) ;

	return strResult.AllocSysString();
}

BSTR CImCNTLDlgAutoProxy::SetDiffractionMeasPar(
  long iMeasMode, long iXDist_pix, long iYDist_pix, long iBackgoundDist_pix) 
{
	CString strResult;

  strResult.Format( "%d %d %d %d" ,    // for old parameters return
    m_View->m_ImParam.m_iDiffractionMeasurementMethod ,
    m_View->m_ImParam.m_iDiffractionRadius ,
    m_View->m_ImParam.m_iDiffractionRadius_Y ,
    m_View->m_ImParam.m_iBackgroundDist ) ;

  m_View->m_ImParam.m_iDiffractionMeasurementMethod = iMeasMode ;
  m_View->m_ImParam.m_iDiffractionRadius = iXDist_pix ;
  m_View->m_ImParam.m_iDiffractionRadius_Y = iYDist_pix ;
  m_View->m_ImParam.m_iBackgroundDist = iBackgoundDist_pix ;

	return strResult.AllocSysString();
}
