
#ifndef VIDEOOBJECTS_INC
#define VIDEOOBJECTS_INC
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <afxtempl.h>
#include <gadgets\stdsetup.h>
#include <classes\dpoint.h>
#include <Gadgets\VideoFrame.h>
#include <imageproc\Clusters\Clusters.h>
#include <imageproc\Clusters\ClusterOp.h>
#include <imageproc\seekspots.h>
#include <imageproc\clusters\Segmentation.h>
#include <imageproc\LineData.h>
#include <imageproc\OCRData.h>
#include <helpers\FXParser2.h>

#include "ViewModeOptionData.h"


extern ObjNameToMask DefMeasurements[] ;
extern ObjNameToMask DefViews[] ;
DWORD GetMeasMaskForName( LPCTSTR pName , ObjNameToMask * pArray = DefMeasurements , DWORD dwBitMask = 0xffffffff ) ;
LPCTSTR GetMeasNameForMask( DWORD Mask , ObjNameToMask * pArray = DefMeasurements , DWORD dwBitMask = 0xffffffff ) ;

typedef FXArray<CRect,CRect&> CRectArray ;

#define TYPE_UNKNOWN 0
#define TYPE_SPOT 1
#define TYPE_LINE 2
#define TYPE_EDGE 3
#define TYPE_TEXT 4

enum ABS_POS_STATUS
{
  APS_ABS = 1 ,
  APS_REL_MEASURED ,
  APS_ABS_CALCULATED ,
  APS_ABS_UNKNOWN
};

enum TimingIndexes
{
  TI_Prepare = 0 ,
  TI_Profiles ,
  TI_Segmentation ,
  TI_SizeBorderAreaFiltration ,
  TI_DiffractionSortingAndUnion ,
  TI_ConturDiametersDetailed ,
  TI_FormOutput
};

class ProfAnalysisData
{
public:
  double m_dSlope ;
  double m_dAmpl ;
  double m_dScaleX ;
  double m_dCorrectedMin ;
  double m_dCorrectedMax ;
  CRect  m_ProfRect ;
};

class InfoROIandId
{
public:
  DWORD m_Id ;
  CRect m_ROI ;

  InfoROIandId( DWORD dwId , CRect& ROI )
  {
    m_Id = dwId ;
    m_ROI = ROI ;
  }

  InfoROIandId() { Reset() ; }
  void Reset() { memset( this , 0 , sizeof( *this ) ) ; } ;
};

typedef vector<InfoROIandId> ROIandIds ;

inline LPCTSTR VOTypeToVOName( VOBJ_TYPE Type )
{
  switch ( Type )
  {
  case SPOT: return _T( "spot" ) ;
  case LINE_SEGMENT: return _T( "line" ) ;
  case BORDER:
  case EDGE: return _T( "edge" ) ;
  case OCR: return _T( "text" ) ;
  }
  return NULL ;
}

inline VOBJ_TYPE VONameToVOType( LPCTSTR pName )
{
  if ( _tcscmp( pName , _T( "spot" ) ) == 0 )
    return SPOT ;
  if ( _tcscmp( pName , _T( "line" ) ) == 0 )
    return LINE_SEGMENT ;
  if ( _tcscmp( pName , _T( "edge" ) ) == 0 )
    return EDGE ;
  if ( _tcscmp( pName , _T( "text" ) ) == 0 )
    return OCR ;
  return NONE ;
}


inline BOOL PtInRectForStep( CPoint Pt , CRect Rect , CSize Step )
{
  if ( Step.cx )
    return ( (Rect.left <= Pt.x)  &&  (Pt.x < Rect.right) ) ;
  else
    return ( (Rect.top <= Pt.y)  &&  (Pt.y < Rect.bottom) ) ;
}

inline int iDratioIter = 0;
inline BOOL BorderByTwo( const pTVFrame fr , CPoint Cent ,
                        cmplx & OriRes, int iThres, CSize Dir )
{
  CPoint Next = Cent + Dir;
  int iCent = GetPixel( fr , Cent );
  int iNext = GetPixel( fr ,Next );

  if ( (iCent >= iThres) ^ (iNext >= iThres) )
  {
    double dRatio = ((double) (iCent - iThres)) / (double)(iCent - iNext) ;
    if (dRatio == 0)
      iDratioIter++;
    if (iDratioIter > 3)
    {
      dRatio = 0.1;
      iDratioIter = 0;
    }
    OriRes = cmplx( Cent.x + (Dir.cx * dRatio) , Cent.y + (Dir.cy * dRatio)) ;
    return TRUE ;
  }
  else
    return FALSE ; // no threshold between intensities
}
// Turn operations for video coordinate system (Y is down)
inline CSize TurnDirLeft( CSize DirBefore )
{
  return CSize( DirBefore.cy , -DirBefore.cx ) ;
}
inline CSize TurnDirRight( CSize DirBefore )
{
  return CSize( -DirBefore.cy , DirBefore.cx ) ;
}

inline void TurnRight( CSize& dirMove , CSize& dirToBord )
{
  CSize Tmp = dirToBord ;
  dirToBord = dirMove ;  
  dirMove = -Tmp ;
}
inline void TurnLeft( CSize& dirMove , CSize& dirToBord )
{
  CSize Tmp = dirToBord ;
  dirToBord = -dirMove ;  // it's the same with TurnDirRight for both dirs
  dirMove = Tmp ;
}

inline CPoint NextPoint( CPoint Current , CSize DirMove )
{
  return Current + DirMove ;
}

class COCR
{
public:
  COCR();
  ~COCR();
};

class CVideoObject ;

double _find_spot_direction_and_Profiles( 
  CColorSpot& Spot , const pTVFrame pFrame , int iThres , 
  CPoint& Offset , CVideoObject * pVO ) ;

class VOArray ;

class CVideoObject : public CVideoObjectBase
{
  friend class VOArray;
public:

  CRect     m_AOS ;         // AreaOfSearching
  CRect     m_absAOS;       // Absolute position after all corrections
  CRect     m_LastUsedAOS ;
  CRect     m_RelAOS ;      // for presentation in dialog only
  CRect     m_ExpectedSize; // minx(left) ,miny(top),maxx(right),maxy(bottom)
  int       m_iAreaMin ;
  int       m_iAreaMax ;
  bool      m_bNoFiltration ;

  DWORD     m_WhatToMeasure ;  
  DWORD     m_WhatIsMeasured ;

  DWORD     m_dwViewMode ;
  
  double    m_dMeasuredMin ;
  double    m_dMeasuredMax ;
  int       m_iDebouncing_pix ;
  BOOL      m_bMulti ;
  int       m_iRotationThres ;
  BOOL      m_ViewResCoord ;
  CSize     m_DiffrRadius ;
  BOOL      m_bDontTouchBorder ;
  int       m_iResultsCounter ;
  int       m_iCaptionOutPeriod ;
  int       m_iHorBackCalcShift ; // if not zero - distance from object center to
                                  // background calculation area in horizontal direction
  const CVideoFrame   * m_pOriginalFrame ;

      // The next will be initialized one by one or by default constructor
  CPoint    m_LeaderIndex ; // if (-1,-1) => Area of searching is absolute
  VOBJ_TYPE m_Type ;
  VOBJ_DIR  m_Direction ;
  int       m_iDirForEdgeContur ;
  int       m_iEdgeFindStep ;
  VOBJ_CONTRAST  m_Contrast ;
  VOBJ_PLACEMENT m_Placement ;
  bool      m_bExternalModification = false ; // changes by frame on control pin
  int       m_iMinContrast ; // for object presence checking
  double    m_dMinProfileContrast ;
  double    m_dProfileThreshold ;
  double    m_dRotationThreshold ;
  int       m_iProfileLocalization_pix = 0 ; // if not zero profile will be decreased for local
                                         // minimums (in interval m_iProfileLocalization_pix)
                                         // This is for pictures with high nonunifomity
  int       m_iViewSize ;
  CSize     m_ViewOffset ;
  COLORREF  m_ViewColor ;
  int       m_iViewMode = 3 ; // what to show for OCR
  CDPoint   m_InFOVPosition ;  // coordinate result
  bool      m_bForROIViewOnly ;
  bool      m_bRenamed ;
  CDRect    m_RectForROIViewOnly ;
  double    m_dROIViewScale ; // always more then 1.
  double    m_dHistThres ;
  double     m_dStartTime ;
  int        m_iNObjectsMax ;
  int        m_iExtendForward_perc ;
  bool       m_bLeaderChanged ;
  VOArray *  m_pVObjects ;
  FXLockObject * m_pVOArrayLock ;
//  FXLockObject m_ROIArrayLock ;

  CSegmentation * m_pSegmentation ;
  CDblArray m_Thresholds ;
  CDblArray m_RotationThresholds ;
  Profile   m_HProfile ;
  Profile   m_VProfile ;
  FXString  m_TextResult;    
  FXString  m_LeaderName;
  FXString  m_LeaderNameY ;
  FXString  m_Status ;
  CClusters    m_Clusters;
  CLineResults m_LineResults ;
  CmplxArray   m_InternalSegment ;
  CRectArray   m_SavedAOS ;
  CDblArray  m_EndOfInitialProcessing ;
  CDblArray  m_EndSubpixelEdgeFind ;
  CDblArray  m_EndOfFinalProcessing ;
  DoubleVector  m_Timing ;
  FXStringArray  m_RelativeToNames ;
  tesseract::TessBaseAPI * m_pTesseract ;
  CDataFrame * m_pDiagnostics ;
  ROIandIds    m_ROIandIds ;
  int          m_iCurrentROIIndex ;

  CVideoObject( int iNObjectsMax = 10 , double dTimeout = 0. ) ;
  CVideoObject( CVideoObject& Original ) ;

  ~CVideoObject() ;
  CVideoObject& operator= ( const CVideoObject& Original ) ;
  int AnalyzeAndSetViewmode( FXParser2& AsText ) ;
  int AnalyzeAndSetMeasurements( FXParser2& AsText ) ;
  void CopyProcessingParameters( CVideoObject& Prototype ) ;
  virtual bool DoMeasure( const CVideoFrame* vf );
  int FillRect( CRect& r , FXParser2& src , int& pos ) ; 
  bool FillDataAboutLeaders( FXPropKit2& pk , bool * pbInvalidate = NULL ) ;
  int GetActiveNames( FXString& ViewDescription , DWORD Mask ,
    ObjNameToMask * pArray = DefMeasurements ) ;
  double GetWorkingTime()
  {
    return GetHRTickCount() - m_dStartTime ;
  }
  virtual bool InitVideoObject(LPCTSTR key,
    LPCTSTR param , bool * pInvalidate = NULL);
  bool IsTimeout()
  {
    return ((m_dTimeout > 0.) && (GetWorkingTime() > m_dTimeout)) ;
  }

  virtual bool MeasureEdge( const CVideoFrame* vf ) ;
  virtual bool MeasureEdgeAsContur( const pTVFrame ptv , CDataFrame** pDiagnostics = NULL ) ;
  virtual bool MeasureEdgeAsConturLowResolution( const pTVFrame ptv , CDataFrame** pDiagnostics = NULL ) ;
  virtual bool MeasureLine( const CVideoFrame* vf ) ;
  virtual bool MeasureSpot( const CVideoFrame* vf ) ;
  virtual bool MeasureText( const CVideoFrame* vf ) ;
  virtual int RecognizeTextByTesseract( pTVFrame ptv_fragment , 
    int expectedH , int iBytesPerPixel , FXString& ResultString ) ;
  virtual int RecognizeTextByTesseract( const pTVFrame ptv_FullFrame , 
    FXString& ResultString ) ;
  virtual int RecognizeTextByOCRLib( pTVFrame ptv , int expectedH , int iBytesPerPixel ) ;
  LPCTSTR obj2name( VOBJ_TYPE type ) ;
  virtual bool PrintProperties( FXString& PropertiesOut , void * = NULL ) ;
  bool SaveNewAOS( int iPatternIndex , CRect& FrameRect ) ;
  virtual bool ScanProperties( const FXString& PropertiesIn , 
    bool &bInvalidate ) ;
  bool PrintVideoObject(CVideoObject &vo, FXString& AsText ) ;
  bool PrintVideoObject( FXString& AsText ) ;
  virtual bool ScanSettings( FXString& SettingsOut ) ;
  bool SetInitialPtAndStep(
    CPoint& Pt , CSize& Step , int width , int height ) ;
  virtual void SetSetupObject( CSetupObject* pSetupObject )
  {
    if ( m_SetupObject )
      ( ( CObjectStdSetup* ) m_SetupObject )->PostMessage( WM_DESTROY ) ;
    m_SetupObject = pSetupObject;
  };
protected:
public:
  ABS_POS_STATUS GetAbsPosition( CRect& rAbsPos );
  int GetPlacementAndLeaderOffset( CRect& rAOS , CPoint& LeaderOffset );
  bool IsRenamed() { return m_bRenamed ; }
  void SetRenamed( bool bSet ) { m_bRenamed = bSet ;  }
  void ClearData() ;
  int SaveROI( DWORD dwId ) ;
  bool GetSavedROI( CRect& ResultROI , DWORD dwId ) ;
};


class VOArray: public FXArray<CVideoObject,CVideoObject&>
{
  CPoint  m_CommonOffset;
public:
  VOArray(): m_CommonOffset(0,0) {} ;
  bool  DoMeasure( int nIndex , const CVideoFrame* vf , int iPatternIndex = -1 ) ;
  void SetCommonOffset(POINT pnt)
  {
    m_CommonOffset=pnt;
  }
};

class CVOTask
{
public:
  FXString m_ObjectName;
  int     m_ObjectID;
  CVOTask& operator =( const CVOTask& Orig )
  {
    m_ObjectName = Orig.m_ObjectName ;
    m_ObjectID = Orig.m_ObjectID ;
    return *this ;
  }
};

class CVOJob: public FXArray<CVOTask,CVOTask&>
{
public:
  CVOJob() {};
  CVOJob(CVOJob& Original)
  {
    Copy(Original);
  }
  CVOJob& operator= (CVOJob& Original) 
  {
    Copy(Original);
    return *this;
  }
  void PrintObjectList( FXString& AsText )
  {
    AsText.Empty() ;
    for ( int i = 0 ; i < GetCount() ; i++ )
    {
      AsText += GetAt(i).m_ObjectName ;
      if ( i < GetCount() - 1 )
        AsText += _T(',') ;
    }
  }
};

class CVOJobList : public CMapWordToPtr
{
public:
  void RemoveAll()
  {
    POSITION pos=GetStartPosition();
    while (pos)
    {
      WORD rKey;
      void* rValue;

      GetNextAssoc(pos, rKey, rValue);
      delete (CVOJob*)rValue;
    }
    CMapWordToPtr::RemoveAll( );
  }
  ~CVOJobList()
  {
    RemoveAll( );
  }
  void SetAt( WORD key, CVOJob& newValue )
  {
    RemoveKey( key ) ;
    CMapWordToPtr::SetAt(key,new CVOJob(newValue));
  }

  bool RemoveKey( WORD Key )
  {
    void* rValue;
    BOOL res = CMapWordToPtr::Lookup( Key , rValue );
    if ( res )
    {
      delete (CVOJob*) rValue;
      res = CMapWordToPtr::RemoveKey( Key ) ;
    }

    return res ;
  }
  BOOL Lookup(WORD key, CVOJob& val)
  {
    void* rValue;
    BOOL res=CMapWordToPtr::Lookup(key,rValue);
    if (res)
    {
      val=*(CVOJob*)rValue;
    }
    return res;
  }
  void PrintJobs( FXString& AsText )
  {
    AsText.Empty() ;
    POSITION pos=GetStartPosition();
    while (pos)
    {
      WORD rKey;
      void* rValue;

      GetNextAssoc(pos, rKey, rValue);
      FXString ObjectsAsText ;
      ((CVOJob*)rValue)->PrintObjectList( ObjectsAsText ) ;
      if ( !ObjectsAsText.IsEmpty() )
      {
        FXString TaskAsText ;
        TaskAsText.Format(_T("Task(%d,%s)\n") , rKey , (LPCTSTR)ObjectsAsText ) ;
        AsText += TaskAsText ;
      }
    }
  }
  void PrintJobs( FXStringArray& AsStringArray )
  {
    AsStringArray.RemoveAll() ;
    POSITION pos = GetStartPosition();
    while ( pos )
    {
      WORD rKey;
      void* rValue;

      GetNextAssoc( pos , rKey , rValue );
      FXString ObjectsAsText ;
      ((CVOJob*) rValue)->PrintObjectList( ObjectsAsText ) ;
      if ( !ObjectsAsText.IsEmpty() )
      {
        FXString TaskAsText ;
        TaskAsText.Format( _T( "Task(%d,%s)" ) , rKey , (LPCTSTR) ObjectsAsText ) ;
        AsStringArray.Add( TaskAsText );
      }
    }
  }
  
  int GetFreeJobNumber()
  {
    for ( int i = 0 ; i < 10000 ; i++ )
    {
      void * rValue ;
      if ( CMapWordToPtr::Lookup( i , rValue ) )
        return i ;
    }
    return -1 ;
  }
//   void Copy(CVOJobList& Original)
//   {
//     RemoveAll() ;
//     POSITION pos=Original.GetStartPosition();
//     while (pos)
//     {
//       WORD rKey;
//       void* rValue;
// 
//       Original.GetNextAssoc(pos, rKey, rValue);
//       SetAt( rKey , (CVOJob&)rValue ) ;
//     }
//   }
};

bool Link( VOArray * pObjects , CVOJobList& Jobs , int * pActiveTask = NULL ) ;


/*
class VOGroup
{
public:
VOArray m_Objects ;
FXString m_GroupName ;

VOGroup( VOArray& Proto , FXString * pName = NULL )
{
if ( pName ) m_GroupName = *pName ;
} ;
};

class VOPlacedGroup:public VOGroup
{
public:
VOPlacedGroup( VOArray& Proto , CPoint Placement , FXString * pName = NULL ):
VOGroup(Proto, pName)
{
for ( int i = 0 ; i < Proto.GetSize() ; i++ )
{
m_Objects.Add( Proto[i] ) ;
m_Objects[i].m_AOS.OffsetRect(Placement) ;
}
}

};
*/

#endif