// ExtremumMeas.h : Declaration of the CExtremumMeas class

#pragma once

#include <gadgets\gadbase.h>
#include <Gadgets\VideoFrame.h>
#include <Gadgets\TextFrame.h>
#include <math\intf_sup.h>
#include <gadgets\vftempl.h>
#include "imageproc\clusters\Segmentation.h"

#define VIEW_DIR      1
#define VIEW_CROSSES  2
#define VIEW_PROFILES 4

typedef struct  
{
  FXString m_Name ;
  double m_dDirection_rad ;
} Extremum ;

typedef FXArray<Extremum> ObjArray ;

class CExtremumMeas : public CFilterGadget
{
private:
    CExtremumMeas(void);
    void ShutDown();
public:
//
  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);

  int	 GetInputsCount();
  CInputConnector* GetInputConnector(int n);
  int	 GetOutputsCount();
  COutputConnector* GetOutputConnector(int n);


  // If you want to have control pin (duplex connector 
  //  on bottom side of gadget)
  // do uncomment next row and all mentions about m_pDuplexConnector
  // in cpp file
  CDuplexConnector * m_pDuplexConnector ;
  
  // The next functions will be used by duplex connector only

  int GetDuplexCount();
  CDuplexConnector* GetDuplexConnector(int n);
  virtual void AsyncTransaction(
    CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  // String with object names and directions to extream for setup
  FXString m_ObjectsAndDirections ;
  // Array of object extreams 
  int      m_iAverage_pix ;
  int      m_iFromEdge_perc ;
  int      m_iThres_perc ;
  int      m_dwViewMode ; // bit 0 - direction
                          // bit 1 - crosses
                          // bit 2 - profiles
  bool     m_FormatErrorProcessed;
  DWORD    m_LastFormat;
  FXLockObject m_Lock;        // For job-objects data taking or updating
  FXLockObject m_LockModify;  // for data modifying by Dlg or packet
  ObjArray *  m_pObjects;
  ObjArray *  m_pNewObjects ;
  ObjArray *  m_pOldObjects ;
  bool       m_bNewObjectsUpdated ;

  DECLARE_RUNTIME_GADGET(CExtremumMeas);
};
