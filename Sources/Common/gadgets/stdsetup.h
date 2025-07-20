#ifndef _STDSETUP_TVDB400_INCLUDED
#define _STDSETUP_TVDB400_INCLUDED


#ifdef _DEBUG
#define STDSETUP_DLL_NAME "stdsetupd.dll"
#define STDSETUP_LIB_NAME "stdsetupd.lib"
#else
#define STDSETUP_DLL_NAME "stdsetup.dll"
#define STDSETUP_LIB_NAME "stdsetup.lib"
#endif

#ifndef STDSETUP_DLL
#define FX_EXT_STDSETUP __declspec(dllimport)
#pragma comment(lib, STDSETUP_LIB_NAME)
#else
#define FX_EXT_STDSETUP __declspec(dllexport)
#endif

#include <fxfc\fxfc.h>
#include <gadgets\gridlistctrl.h>
//#include <gadgets\stdsetup.h>
#include <gadgets\shkernel.h>

void FX_EXT_STDSETUP attachstdsetupDLL();

#define SETUP_COMBOBOX  _T("ComboBox")
#define SETUP_SPIN      _T("Spin")
#define SETUP_SPINABOOL _T("Spin&Bool")
#define SETUP_EDITBOX   _T("EditBox")
#define SETUP_CHECKBOX  _T("CheckBox")
#define SETUP_INDENT    _T("Indent")

#define MSG_UPDATE (WM_USER + 100)

#define TIMER_ID_FOR_GREYED 100

typedef struct  
{
  __int64  iItemId ;
  LPCTSTR psItemName ;
} ComboItem ;;

int     FX_EXT_STDSETUP FormComboSetup( ComboItem * pItems , FXString& Result , LPCTSTR pName ) ;
LPCTSTR FX_EXT_STDSETUP GetNameForId( ComboItem * pItems , int iId ) ;
bool    FX_EXT_STDSETUP GetIdForName( ComboItem * pItems , LPCTSTR pName , int& iId) ;

typedef struct _itemData
{
  CString itemId;
  CString cmdId;
  int     ctrlId;
}itemData;

class FX_EXT_STDSETUP CGadgetSetupDialog : public CDialog , public CSetupObject
{
protected:
  // Construction
  virtual ~CGadgetSetupDialog();
  CGadget* m_pGadget;
public:
  CGadgetSetupDialog( CGadget* pGadget , UINT idd , CWnd* pParent = NULL );
  virtual BOOL IsOn();
  virtual void Delete();
  // Dialog Data
    //{{AFX_DATA(CGadgetSetupDialog)
  enum
  {
    IDD = 0
  };
  // NOTE: the ClassWizard will add data members here
//}}AFX_DATA


// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CGadgetSetupDialog)
protected:
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
  virtual void OnOK();
  virtual void OnCancel();
  virtual void UploadParams(); // this function is called in OnOK(), use it to upload params to gadgets with non-standard setup dialogs
  //}}AFX_VIRTUAL

// Implementation
protected:
  //virtual void UploadParams(CGadget* pGadget);
  // Generated message map functions
  //{{AFX_MSG(CGadgetSetupDialog)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  //    afx_msg void OnDestroy();
//  afx_msg void OnBnClickedRestoreInitial();
};

#define ITEM_DATA_ARRAY FXArray<itemData,itemData>
#define ITEM_DATA(p) ((ITEM_DATA_ARRAY*)p)

enum StdDialogMode
{
  SDM_Unknown = 0 ,
  SDM_Gadget ,
  SDM_VideoObject
};

class FX_EXT_STDSETUP CStdSetupDlg : public CDialog , public CSetupObject
{
protected:
  ReadOnlyForStdSetupState m_ReadOnlyState ;
  StdDialogMode   m_DlgMode ;
  FXString        m_UID; // Object name for VideoObject or gadget name for Gadget
  FXParser        m_LastSettings ;
  FXParser        m_LastParams ;
  FXPropertyKit   m_pk ;
  FXPropertyKit   m_InitialPropValues ;
  FXPropertyKit   m_SavedPropValues ;
  FXStringArray   m_LastKeys , m_LastLineParams ;
  FXStringArray   m_GreyedItems ;
  UINT_PTR        m_iTimerID = 0 ;
  int             m_ItemsCnt;
  int             m_iNRequiredCnt ;
  int             m_iSpareHeight ;
  int             m_iNLastCheckBoxes ;
  void*           m_pItemsData;
  bool            m_bInvalidateProcess ;
  CGridListCtrl   m_SetupGrid;
  CGridRow        m_row;

public:
  CStdSetupDlg( UINT IDD , CWnd * pParent ) ;
  virtual ~CStdSetupDlg();
  virtual BOOL OnInitDialog();
  virtual ReadOnlyForStdSetupState GetROState( FXString& ItemName ) ;
  virtual int CntRequired( bool bROCheck = false ) ;
  virtual void ResetData() ;
  virtual BOOL ParseSetupData();
  bool InsertLine( LPCTSTR key , LPCTSTR params ) ;
  virtual int SetDlgSizeAndPos();
  virtual bool EndLineIfNotEmpty();
  bool    SetCellText( LPCTSTR pName , LPCTSTR pText )
  {
    return m_SetupGrid.SetCellText( pName , pText ) ;
  }
  bool    SetCellInt( LPCTSTR pName , int iValue )
  {
    return m_SetupGrid.SetCellInt( pName , iValue ) ;
  };
  bool    SetCellChk( LPCTSTR pName , bool enable )
  {
    return m_SetupGrid.SetCellChk( pName , enable ) ;
  };

 // DECLARE_MESSAGE_MAP()
 // afx_msg void OnTimer( UINT_PTR nIDEvent );
  int CheckAndAddToGreyedList( LPCTSTR pParameterName );
  int RemoveFromGreyedList( LPCTSTR pParameterName );
};

class FX_EXT_STDSETUP CGadgetStdSetup : public CStdSetupDlg
{
  friend void OnGadgetSetupEvent( int Event , void *wParam , int col , int row , int uData );
protected:
  IGraphbuilder*  m_pBuilder;
public:
  CGadgetStdSetup( IGraphbuilder* pBuilder , LPCTSTR uid , CWnd* pParent = NULL );   // standard constructor
  virtual ~CGadgetStdSetup();
  virtual bool Show( CPoint point , LPCTSTR uid );
  virtual void Delete();
  bool    InsertLine( LPCTSTR key , LPCTSTR params );
  // Dialog Data
    //{{AFX_DATA(CGadgetStdSetup)
  //}}AFX_DATA
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CGadgetStdSetup)
public:
  virtual BOOL Create( CWnd* pParentWnd = NULL );
  virtual FXPropertyKit& GetSavedProperties() ;
  virtual void SetSavedProperties( FXString& Properties ) ;
  virtual void Update() ;
protected:
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  void OnGridEvent( int Event , int col , int row , int uData );
  // Generated message map functions
  //{{AFX_MSG(CGadgetStdSetup)
//  virtual BOOL OnInitDialog();
  afx_msg void OnDestroy();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  BOOL LoadSetup();
  //    afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnBnClickedRestoreInitial();
  afx_msg void OnBnClickedSaveSet();
  afx_msg void OnBnClickedRestoreSet();
  afx_msg void OnBnClickedApply();
  afx_msg BOOL OnMouseWheel( UINT nFlags , short zDelta , CPoint pt );
  afx_msg void OnTimer( UINT_PTR nIDEvent );
};

class FX_EXT_STDSETUP CVideoObjectBase
{
protected:
  CSetupObject*	m_SetupObject;
public:
  FXLockObject  m_VObjectLock ;
  FXString  m_ObjectName ;
  CVideoObjectBase()
    : m_dTimeout( 100. ) 
    , m_dStartTime( GetHRTickCount() )
    , m_SetupObject(NULL)
  {};
  virtual ~CVideoObjectBase() ;


  virtual bool DoMeasure( const CVideoFrame* vf ) { return false; };
  virtual bool InitVideoObject( LPCTSTR key ,
    LPCTSTR param , bool * pInvalidate = NULL ) { return false; };
  virtual bool PrintProperties( FXString& PropertiesOut , void * pRef= NULL ) 
  {
    return false;
  };
  virtual bool ScanProperties( const FXString& PropertiesIn , bool &bInvalidate ) 
  {
    return false;
  };
  virtual bool ScanSettings( FXString& SettingsOut ) { return false; };
  virtual bool MeasureSpot( const CVideoFrame* vf )  { return false; };
  virtual bool MeasureLine( const CVideoFrame* vf )  { return false; };
  virtual bool MeasureText( const CVideoFrame* vf )  { return false; };
  virtual bool MeasureEdge( const CVideoFrame* vf )  { return false; };
  virtual bool GetObjectName( FXString& Name ) { Name = m_ObjectName ; return true ; }
  virtual bool IsObjectName( const FXString& Name ) { return (Name == m_ObjectName) ; }
  virtual void SetSetupObject( CSetupObject* pSetupObject )
  {
    m_SetupObject->Delete() ;
    m_SetupObject = pSetupObject;
  };
  virtual void ResetSetupObject()
  {
    m_SetupObject = NULL ;
  }
  virtual CSetupObject* GetSetupObject()
  {
    return m_SetupObject;
  }
  double GetWorkingTime()
  {
    return GetHRTickCount() - m_dStartTime ;
  }
  bool IsTimeout()
  {
    return ((m_dTimeout > 0.) && (GetWorkingTime() > m_dTimeout)) ;
  }

  double m_dTimeout ;
  double m_dStartTime ;
};

class FX_EXT_STDSETUP CObjectSetupDialog : public CDialog , public CSetupObject
{
protected:
  // Construction
  virtual ~CObjectSetupDialog();
  CVideoObjectBase* m_pObject;
public:
  CObjectSetupDialog( CVideoObjectBase* pObject , UINT idd , CWnd* pParent = NULL );
  virtual BOOL IsOn();
  virtual void Delete();
  // Dialog Data
    //{{AFX_DATA(CGadgetSetupDialog)
  enum
  {
    IDD = 0
  };
  // NOTE: the ClassWizard will add data members here
//}}AFX_DATA


// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CGadgetSetupDialog)
protected:
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
  virtual void OnOK();
  virtual void OnCancel();
  virtual void UploadParams(); // this function is called in OnOK(), use it to upload params to gadgets with non-standard setup dialogs
  //}}AFX_VIRTUAL

// Implementation
protected:
  //virtual void UploadParams(CGadget* pGadget);
  // Generated message map functions
  //{{AFX_MSG(CGadgetSetupDialog)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  //    afx_msg void OnDestroy();
//  afx_msg void OnBnClickedRestoreInitial();
};

class FX_EXT_STDSETUP CObjectStdSetup : public CStdSetupDlg
{
  friend void OnObjectSetupEvent( int Event , void *wParam , int col , int row , int uData );
protected:
  CVideoObjectBase*  m_pObject;

public:
  CObjectStdSetup( CVideoObjectBase* pObject , LPCTSTR uid , CWnd* pParent = NULL );   // standard constructor
  virtual ~CObjectStdSetup();
  virtual BOOL IsOn();
  virtual bool Show( CPoint point , LPCTSTR ObjectName );
  virtual void Delete();
  bool    InsertLine( LPCTSTR key , LPCTSTR params );
  // Dialog Data
    //{{AFX_DATA(CObjectStdSetup)
  //}}AFX_DATA
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CObjectStdSetup)
public:
  virtual BOOL Create( CWnd* pParentWnd = NULL );
  virtual FXPropertyKit& GetSavedProperties() ;
  virtual void SetSavedProperties( FXString& Properties ) ;
protected:
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  void OnGridEvent( int Event , int col , int row , int uData );
  // Generated message map functions
  //{{AFX_MSG(CGadgetStdSetup)
//   virtual BOOL OnInitDialog();
  afx_msg void OnDestroy();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  BOOL LoadSetup();
  afx_msg LRESULT OnUpdateSetup( WPARAM wPar , LPARAM lPar ) ;
  //    afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnBnClickedRestoreInitial();
  afx_msg void OnBnClickedSaveSet();
  afx_msg void OnBnClickedRestoreSet();
  afx_msg void OnBnClickedApply();
};

class ExistingObjSetupDlg
{
public:
  FXString m_ObjectName ;
  CObjectStdSetup * m_pObjStdDlg ;

  ExistingObjSetupDlg( LPCTSTR pName = NULL , CObjectStdSetup * pDlg = NULL ) 
  {
    m_ObjectName = pName ;
    m_pObjStdDlg = pDlg ;
  }

  ExistingObjSetupDlg& operator=( const ExistingObjSetupDlg& Src )
  {
    m_ObjectName = Src.m_ObjectName ;
    m_pObjStdDlg = Src.m_pObjStdDlg ;
    return *this ;
  }
}  ;


#define CALL_FITTEDSETUP -1

/////////////////////////////////////////////////////////////////////////////
// CGraphSettingsDialog dialog

class FX_EXT_STDSETUP CGraphSettingsDialog : public CDialog , public CSetupObject
{
  friend void OnGridEvent( int Event , void *wParam , int col , int row , int uData );
private:
  IGraphbuilder*  m_Builder;
  FXParser        m_Data;
  int             m_ItemsCnt;
  CArray<itemData , itemData> m_ItemsData;
public:
  CGraphSettingsDialog( IGraphbuilder* gb , CWnd* pParent = NULL );   // standard constructor
  void    SetData( LPCTSTR data );
  bool    InsertStdDlg( LPCTSTR name , LPCTSTR key , LPCTSTR params , int pos = -1 );
  bool    RemoveStdDlg( LPCTSTR gadgetname );
  virtual bool Show( CPoint point , LPCTSTR uid );
  virtual void Delete();
  virtual BOOL Create( CWnd* pParentWnd = NULL );
  BOOL LoadSetup();
  //{{AFX_DATA(CGraphSettingsDialog)
  //enum { IDD = IDD_SETTINGS_DIALOG };
  const int IDD;
  CGridListCtrl m_SetupGrid;
  //}}AFX_DATA
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CGraphSettingsDialog)
public:
  virtual BOOL PreTranslateMessage( MSG* pMsg );
protected:
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  void CheckScrollbar();
  int  GetCurrentGap();
  //bool ParseGadgetItem(CInfoParser& ip, CString cname);
  bool ParseGadgetItem( FXParser& ip , CString& cname );
  bool InsertSimple( LPCTSTR name , LPCTSTR key , LPCTSTR params );
  void CallBack( LPCTSTR dlgID , int ctrlID , CString* Data , bool &bVal );
  void OnGridEvent( int Event , int col , int row , int uData );
  // Generated message map functions
  //{{AFX_MSG(CGraphSettingsDialog)
  //virtual BOOL OnInitDialog();
  afx_msg void OnDestroy();
  //afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

class FX_UPDATERESOURCE
{
  HINSTANCE m_hOldResource;
public:
  FX_UPDATERESOURCE( HINSTANCE hInst )
  {
    m_hOldResource = AfxGetResourceHandle();
    AfxSetResourceHandle( hInst );
  }
  ~FX_UPDATERESOURCE()
  {
    AfxSetResourceHandle( m_hOldResource );
  }
};

/////////////

FX_EXT_STDSETUP BOOL Tvdb400_ShowGadgetSetupDlg( IGraphbuilder* pBuilder , LPCTSTR uid , CPoint point );
FX_EXT_STDSETUP BOOL Tvdb400_RunSetupDialog( IGraphbuilder* pBuilder );
FX_EXT_STDSETUP BOOL Tvdb400_GetGadgetList( IGraphbuilder* pBuilder , FXString& GadgetList );
FX_EXT_STDSETUP BOOL Tvdb400_ShowObjectSetupDlg( CVideoObjectBase * pObject , CPoint point );
#endif