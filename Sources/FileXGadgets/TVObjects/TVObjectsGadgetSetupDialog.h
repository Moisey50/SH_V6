#if !defined(AFX_TVOBJECTSGADGETSETUPDIALOG_H__08272BBD_0FA2_4050_9FD3_03FAAD347DDD__INCLUDED_)
#define AFX_TVOBJECTSGADGETSETUPDIALOG_H__08272BBD_0FA2_4050_9FD3_03FAAD347DDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TVObjectsGadgetSetupDialog.h : header file
//
#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>
#include "afxwin.h"
#include "VideoObjects.h"


#define NEW_OBJECT_FROM_UI_MSG   (WM_APP + 100)
#define UPDATE_DLG_VIEW_MSG  (NEW_OBJECT_FROM_UI_MSG + 1)

class ObjectPos
{
public:
  ObjectPos() { memset( &m_Pos , 0 , sizeof(m_Pos)) ;}

  FXString m_ObjectName ;
  FXString m_AnchorName ;
  CRect    m_Pos ;
};

typedef FXArray<ObjectPos> Positions ;

typedef struct  
{
  VOBJ_TYPE Type ;
  LPCTSTR   Name ;
} ObjTypesAndNames ;

class ObjPlacement  
{
public:
  ObjPlacement() { m_Type = NONE ;} ;
  ObjPlacement& operator =(const ObjPlacement& Orig)
  {
    m_iBegin = Orig.m_iBegin ;
    m_iEnd = Orig.m_iEnd ;
    m_Type = Orig.m_Type ;
    m_iResStringNum = Orig.m_iResStringNum ;
    m_Name = Orig.m_Name ;

    return *this ;
  }
  int m_iBegin ;
  int m_iEnd ;
  int m_iResStringNum ;
  VOBJ_TYPE m_Type ;
  FXString m_Name ;
} ;
typedef FXArray<ObjPlacement> ObjOrTasksPositions ;

inline CPoint GetXYfromString( LPCTSTR Src )
{
  FXPropertyKit Parser( Src ) ;
  CPoint Result(0,0) ;
  if ( Parser.GetInt( "x" , (int&) Result.x) && Parser.GetInt( "y" , (int&) Result.y) )
  {
    return Result ;
  }
  return Result ;
}

inline bool GetIntArrayFromString( FXString& Src , CDWordArray& IntArr )  // returns true, if some numbers are written into array 
{
  FXSIZE iPos = 0 ;
  FXString Token = Src.Tokenize(  _T(" ,;\t") , iPos ) ;
  TCHAR SpecialChars[] = _T("-+") ;
  int iOriginalLength = (int) IntArr.GetCount() ;
  while ( !Token.IsEmpty() )
  {
    Token = Token.Trim() ;

    for ( int i = 0 ; i < Token.GetLength() ; i++ )
    {
      TCHAR Ch = Token[ i ] ;
      bool bPlusMinus = ( Ch == _T( '+' ) ) || ( Ch == _T( '-' ) ) ;
      if ( !_istdigit( Token[ i ] ) )
      {
        if ( (i == 0) && bPlusMinus )
          continue ;
        return (( IntArr.GetCount() - iOriginalLength ) > 0) ;
      }
    }
    int iVal = _tstoi( (LPCTSTR)Token ) ;
    IntArr.Add( iVal ) ;
    Token = Src.Tokenize( _T(" ,;\t") , iPos ) ;
  }
  return ( ( IntArr.GetCount() - iOriginalLength ) > 0 ) ;
}
inline bool GetIntArrayFromString( FXString& Src , FXIntArray& IntArr )  // returns true, if some numbers are written into array 
{
  FXSIZE iPos = 0 ;
  FXString Token = Src.Tokenize( _T( " ,;\t" ) , iPos ) ;
  TCHAR SpecialChars[] = _T( "-+" ) ;
  int iOriginalLength = (int) IntArr.GetCount() ;
  while ( !Token.IsEmpty() )
  {
    Token = Token.Trim() ;

    for ( int i = 0 ; i < Token.GetLength() ; i++ )
    {
      TCHAR Ch = Token[ i ] ;
      bool bPlusMinus = (Ch == _T( '+' )) || (Ch == _T( '-' )) ;
      if ( !_istdigit( Token[ i ] ) )
      {
        if ( (i == 0) && bPlusMinus )
          continue ;
        return ((IntArr.GetCount() - iOriginalLength) > 0) ;
      }
    }
    int iVal = _tstoi( (LPCTSTR) Token ) ;
    IntArr.Add( iVal ) ;
    Token = Src.Tokenize( _T( " ,;\t" ) , iPos ) ;
  }
  return ((IntArr.GetCount() - iOriginalLength) > 0) ;
}

inline bool GetDoubleArrayFromString( FXString& Src ,CDblArray& DblArr )  // returns true, if some numbers are written into array 
{
  FXSIZE iPos = 0 ;
  FXString Token = Src.Tokenize(  _T(" ,;\t") , iPos ) ;
  TCHAR SpecialChars[] = _T("-.eEdD+") ;
  while ( !Token.IsEmpty() )
  {
    for ( int i = 0 ; i < Token.GetLength() ; i++ )
    {
      if ( !_istdigit( Token[i] ) && !_tcschr( SpecialChars , Token[i] ) )
        return false ;
    }
    double dVal = _tstof( (LPCTSTR)Token ) ;
    DblArr.Add( dVal ) ;
    Token = Src.Tokenize( _T(" ,;\t") , iPos ) ;
  }
  return ( DblArr.GetCount() != 0 ) ;
}

inline FXString GetTextFromTo( LPCTSTR Origin , TCHAR From , TCHAR To , int& iFrom )
{
  FXString Result ;
  LPCTSTR pFrom = _tcschr( Origin , From ) ;
  if ( pFrom )
  {
    LPCTSTR pTo = _tcschr( pFrom , To ) ;
    if ( pTo > pFrom + 1 )
      Result.Append( pFrom + 1 , pTo - pFrom - 1 ) ;
    else if ( pTo == NULL )
      Result.Append( pFrom + 1 ) ;
  }
  return Result ;
}


inline BOOL GetFXStringFromDlgItem( HWND hWnd ,
  int iID , FXString& Result )
{
  if ( !hWnd )
    return FALSE ;

  HWND hControl = ::GetDlgItem( hWnd , iID ) ;
  int iNChars = ::GetWindowTextLength( hControl ) ;
  if ( iNChars)
  {
    LPTSTR pBuf = Result.GetBuffer( iNChars + 3 ) ;
    iNChars = ::GetDlgItemText( hWnd , iID , pBuf , iNChars + 3 ) ;
    Result.ReleaseBuffer( iNChars ) ;
  }
  else
    Result.Empty() ;
  return iNChars > 0 ;
}
inline BOOL GetFXStringFromDlgItem( HWND hWnd ,
  CDataExchange * pDX , int iID , FXString& Result )
{
  if ( !hWnd )
    return FALSE ;
  if ( pDX->m_bSaveAndValidate ) // take from dialog
  {
    return GetFXStringFromDlgItem( hWnd , iID , Result ) ;
  }
  else
  {
    return ::SetDlgItemText( hWnd , iID , Result ) ;
  }
}
inline void GetFXStringFromCombo( CComboBox& Combo , FXString& Result )
{
  LPTSTR pBuf = ( LPTSTR)Result.GetBuffer( 100 ) ;
  BOOL bRes = Combo.GetWindowText( pBuf , 99) ;
  Result.ReleaseBuffer( -1 ) ;
}


inline bool GetFXStringFromListBox( CListBox& List ,
  FXString& Result , int iIndex = -1 )
{
  if ( iIndex < 0 )
    iIndex = List.GetCurSel() ;
  if ( iIndex == LB_ERR )
    return false ;
  int iLen = List.GetTextLen( iIndex ) ;
  if ( iLen == LB_ERR )
    return false ;
  LPTSTR pBuf = (LPTSTR) Result.GetBuffer( iLen + 1 ) ;
  BOOL bRes = List.GetText( iIndex , pBuf ) ;
  Result.ReleaseBuffer( -1 ) ;

  return true ;
}

// FXString& GetParenthesesContent( FXString& Orig )
// {
//   int iPos = Orig.Find( _T( '(' ) ) ;
//   if ( iPos > 0 )
//   {
//     int iClosePos = Orig.Find( _T( ')' ) , iPos + 1 ) ;
//     if ( iClosePos > iPos )
//       return Orig.Mid( iPos + 1 , iClosePos - iPos - 1 ) ;
//     TRACE( "GetParenthesesContent: not correct     ")
//   }
//   return Orig ;
// }


// If pObjects is not zero, array of objects will be locked in
// called module and pointer to objects will be written to *pObjects
// if pObjects is zero, array of objects will be released 
// returned value is length of array of objects
typedef int( *GetAndLockObjects )( CGadget * pGadget , void ** pObjects) ;

typedef int( *GetTasksAsTextFunc )(CGadget * pGadget , FXStringArray * pTeasksAsText ) ;

typedef int( *GetAndLockObjectsAndTasks )(CGadget * pGadget , 
  void ** pObjects , void ** ppTasks , int * iActiveTask , 
  FXLockObject ** pLockVideoObjects ) ;


class SavedSetupDialog : public CVideoObject
{
public:
  SavedSetupDialog() { } ;
  ~SavedSetupDialog() { } ;
  FXString m_sSelectedParameters ;
  FXString m_sRelName;
  int m_iActiveTask ;
  
  int m_iCurrentTypeSelection ;
  int m_iCurrentOrientationSelection ;
  int m_iCurrentContrastSelection ;
  int m_iCurrentROIColorSelection ;
  FXString m_DetThresh;
};

/////////////////////////////////////////////////////////////////////////////
// TVObjectsGadgetSetupDialog dialog

class TVObjectsGadgetSetupDialog : 
  public CGadgetSetupDialog, public CVideoObject
{
private:
    CGadget* m_Gadget;
    GetAndLockObjects m_pGetDataFunction ;
    GetTasksAsTextFunc m_pGetTasksFunction ;
    GetAndLockObjectsAndTasks m_pGetObjectsAndTasksFunction ;
    CToolTipCtrl m_ToolTip ;

public:
	TVObjectsGadgetSetupDialog(CGadget* Gadget, CWnd* pParent = NULL ,
    GetAndLockObjects pGetDataFunction = NULL ,
    GetTasksAsTextFunc pGetTasksFunction = NULL ,
    GetAndLockObjectsAndTasks pGetObjectsAndTasksFunction = NULL 
  );   // standard constructor
  void UploadParams(CGadget* Gadget);
// Dialog Data
	//{{AFX_DATA(TVObjectsGadgetSetupDialog)
	enum { IDD = IDD_SETUPDLG };
	FXString	m_Template ;
  FXString m_TemplateView ;
  FXString m_SelectedOnRenderObjName ;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TVObjectsGadgetSetupDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual void UpdateData( BOOL bSaveAndValidate = TRUE ) ;
	//}}AFX_VIRTUAL

// Implementation
protected:
  
	// Generated message map functions
	//{{AFX_MSG(TVObjectsGadgetSetupDialog)
	afx_msg void OnApply();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
  ViewModeOptionData * m_pViewMode ;
  FXString m_sSelectedParameters ;
	FXString m_sRelName;
	FXString m_sRelNameY;
  int m_iActiveTask ;
  int m_iServiceTask ;
  int m_bAdjustCoordinates ;  // if 0 - no changes in AOI
                              // will be taken from dialog
                               // when 1, before object find
                               // AOI will be adjusted
                               // around mouse click area
                               // when 2 - two clicks will be used
                               // for AOI appointing 
	//BOOL m_bDispProf;
  int         m_iShownTask ;
  CDWordArray m_ExistentTasks ;
  FXString  m_sObjNames;
	CButton   m_buttAdd;
	FXString m_DetThres;
  FXString m_RotationThres ;
  CListBox m_ObjectNames ;
  CListBox m_Tasks ;
//   CDblArray m_Thresholds ;
//   CDblArray m_RotationThresholds ;
  ObjOrTasksPositions m_ObjPos ;
public:
	CButton m_buttSelectRenderer;
	BOOL m_bSelectRenderer;
	int m_nAutoFind;					//0- initial position,  1-Automatic find succeeded. 2- Failed
	int m_nTaskNumberEntering;			//The task number that is inputed into TVObject Gadget. (for automatic search)
	FXString m_sSelectRendererStart;
  CPoint  m_PtSelectStart ;
	FXString m_sSelectRendererEnd;
  CPoint  m_PtSelectEnd ;
  FXString m_CurrentCoord ;
  CPoint  m_PtCurrent ;
  CPoint  m_PtPrevious ;
  BOOL    m_bWasSelected ;
  // // 0 - find object in area, 1 - catch object automatically, 2 - no operation
  int m_iOnRendererOperation;
  // Processing details for object
  FXString m_Detailed ;
  VOBJ_TYPE m_SelectedObjectType ;
  CRect     m_LastRectFromUI ;
  FXLockObject * m_pLockObjects ;

  afx_msg void OnBnClickedButtonaddobj();
  afx_msg void OnBnClickedCheckselect();
  afx_msg void OnBnClickedCheckmultiple();
  afx_msg void OnBnClickedButtonclear();
  afx_msg void OnBnClickedButtonaddtask();
  afx_msg void OnBnClickedTrackobj();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void StartTimer();
  afx_msg void StopTimer();
  afx_msg void OnBnClickedCheckdisproi();
  afx_msg void OnBnClickedCheckdispcoor();
  afx_msg void OnBnClickedCheckdispprofx();
  afx_msg void OnBnClickedCheckdisppos();
  afx_msg void OnBnClickedCheckdispdetail();
  afx_msg void OnBnClickedCheckdispprofy();
  afx_msg void OnBnClickedCheckdispmgraphics();
  afx_msg void OnBnClickedCatchObject();
  afx_msg void OnBnClickedFindObject();
  afx_msg LRESULT OnNewObjectFromUI(WPARAM wParam, LPARAM lParam) ;
  afx_msg LRESULT OnUpdateDlgViewMessage(WPARAM wParam, LPARAM lParam) ;
  afx_msg void OnBnClickedNotActive();
  afx_msg void OnBnClickedDeleteObject();
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedCancel();
  afx_msg void OnClose();

  virtual bool Show( CPoint point , LPCTSTR uid) ;
  int DeleteObjWithTypeAndNameMatching(
    FXString& ObjectType , FXString& ObjectName );
  int RenameObj( FXString& OldName , FXString& ObjType , 
    FXString& NewObjectName );
  int m_iShownActiveTask;
  void GetParametersFromMainGadget( ViewModeOptionData * pViewMode = NULL ) ;
  void ConnectToGadgetViewMode( ViewModeOptionData * pViewMode ) { m_pViewMode = pViewMode ; }
private:
	BOOL UpdatePosFromRenderer(void);
	int  AddingObject( FXString& ObjName , int iPos = -1 ) ;
  int  AddNewObjectToTask( FXString& ObjName , int iTaskNumber ) ;
  int  DeleteObjectFromTask( FXString& ObjName , int iTaskNumber ) ;
  int  CreateNewTaskWithObject( FXString& ObjName ) ;
  int GetObjectsList( FXStringArray& Result ) ;
	void ClickingTaskButtonResponse(void);
  void ConvertColorToColorCode(FXString sText, FXString* sColor);
  void SetDefaultInitialization( ViewModeOptionData * pViewMode = NULL );
  void ReSelectComboButton(CComboBox* pCombo,FXString sText);
  bool UpdatePosFromRendererSignleClick(void);
  void UpdateTaskFromObjects(void);
  void ConvertColorCodeToColor(const FXString colorCode);
  int ObjectPosInListOfObjects( const FXString& ObjObjName , const FXString& List ) ;
  bool AddObjectToShownList( const FXString& ObjName ) ;
  bool RemoveObjectFromShownList( const FXString& ObjName ) ;
  CEdit m_editTemplate;
  CFont * m_pOldTemplateFont ;
	CEdit m_EditObjNames;
	CButton m_buttEdit;
	CEdit m_editAreaMin;
	CEdit m_editAreaMax;
	CEdit m_editDiffrX;
	CEdit m_editDiffrY;
	CButton m_buttDetailed;
	BOOL m_bDetailed;
  int m_iSelectBegin ;
  int m_iSelectEnd ;
  BOOL m_bInFocus ;
  TEXTMETRIC m_TextMetric ;
  SavedSetupDialog m_SavedSetupData ;
  FXStringArray    m_AnchorNames ;
public:
	BOOL m_bDispROI;
	BOOL m_bDispPos;
	BOOL m_bDispCoor;
	BOOL m_bDispDetails;
	BOOL m_bDispProfX;
	BOOL m_bDispProfY;
	BOOL m_bDispMGraphics;
//  BOOL m_bDispScaledCoords ;
//  BOOL m_bDispContur ;
  BOOL m_bMeasAngleCNW ;
  BOOL m_bMeasAngleCW ;
  BOOL m_bDontTouchEdge ;
  BOOL m_bDispWeighted;
  BOOL m_bMeasureAngle;
  BOOL m_bMeasureDiameters;
  BOOL m_bViewCoordsScaled ;
  BOOL m_bViewObjectContur ;
  BOOL m_bViewAngle ;
  BOOL m_bViewDia ;
  BOOL m_bWeighted ;
  BOOL m_bBinaryOutput ;
  afx_msg void OnBnClickedSetActiveTask();
  bool SaveDialog(SavedSetupDialog * pSave);
  bool RestoreDialog(SavedSetupDialog * pRestore);
  void UpdateViewCheckBoxes() ;
  void UpdateBoolFlags( 
    DWORD dwProcessMode , DWORD dwViewMode = 0xffffffff ) ;
  int  GetGadgetViewMode() ;
  afx_msg void OnBnClickedShowScaledCoords();
  afx_msg void OnBnClickedShowContur();
  afx_msg void OnBnClickedDontTouchEdge();
  inline BOOL GetCheckBoxState( DWORD Idc )
  {
    return (((CButton*) GetDlgItem( Idc ))->GetCheck() == BST_CHECKED) ;
  }
  inline void SetCheckBoxState( DWORD Idc , int iValue )
  {
    return (((CButton*) GetDlgItem( Idc ))->SetCheck( 
      (iValue > 0) ? BST_CHECKED : (iValue == 0) ? BST_UNCHECKED : BST_INDETERMINATE ) ) ;
  }
  int m_iCaptionPeriod;
  FXString m_LastRelativeTo ;
  int      m_iLastAbsRelSelection ;
  afx_msg void OnBnClickedFormMask();
  afx_msg void OnBnClickedCheckdispangle();
  afx_msg void OnBnClickedCheckdiameters();
  afx_msg void OnBnClickedCheckdispweighted();
  afx_msg void OnBnClickedBinaryOut();
  // Measurement timeout (mainly for spot objects)
  // Maximal number of found objects
  int m_iNObjectsMax;
  afx_msg void OnLbnDblclkObjList();
  afx_msg void OnLbnDblclkTaskList();
  VOBJ_TYPE GetSelectedType() ;
  void SetSelectedType( VOBJ_TYPE Type ) ;
  void UpdateObjectsAndTasksLists( VOArray * pObjects = NULL , CVOJobList * pTasks = NULL ) ;
  afx_msg void OnLbnSelchangeObjList();
  virtual BOOL PreTranslateMessage( MSG* pMsg );
  afx_msg void OnLbnSelchangeTaskList();
  BOOL m_bInjectLastImage;
  afx_msg void OnBnClickedReinjectImage();
  afx_msg LRESULT OnShowVOSetupDialog( WPARAM wParam , LPARAM lParam );

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TVOBJECTSGADGETSETUPDIALOG_H__08272BBD_0FA2_4050_9FD3_03FAAD347DDD__INCLUDED_)
