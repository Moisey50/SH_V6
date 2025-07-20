// TVObjectsGadgetSetupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "math\intf_sup.h"
#include <math\hbmath.h>
#include "TVObjects.h"
//#include "TVObjectsGadgetSetupDialog.h"
// #include "TVObjectsGadget.h"
#include "ViewModeOptionData.h"
#include "TVObjectsGadgetSetupDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "TVObjectsGadgetSetupDialog"

#define NUM_OF_CHARS_IN_ROW 92
#define ALLOWED_EXTENT      8
#define LINE_CONTINUATION_SEPARATOR _T("\r\n-->>")
int g_iContinSepLen = (int) _tcslen( LINE_CONTINUATION_SEPARATOR ) ;
// Timer ID constants.
const UINT ID_TIMER = 0x1000;
const UINT ID_TIMER_TEMPLATE_SELECT = 0x1001 ;

static const UINT VM_TVDB400_SHOWVOSETUPDLG = ::RegisterWindowMessage(
  _T( "Tvdb400_ShowVOSetupDialog" ) );




/////////////////////////////////////////////////////////////////////////////
// TVObjectsGadgetSetupDialog dialog

//creating the dialog
TVObjectsGadgetSetupDialog::TVObjectsGadgetSetupDialog(
  CGadget* Gadget, CWnd* pParent /*=NULL*/ , 
  GetAndLockObjects pGetDataFunction /* =NULL*/ ,
  GetTasksAsTextFunc pGetTasksFunction /* =NULL*/ ,
  GetAndLockObjectsAndTasks pGetObjectsAndTasksFunction /* NULL */)
: CGadgetSetupDialog(Gadget, TVObjectsGadgetSetupDialog::IDD, pParent)
//, m_bDispProf(FALSE)
, m_DetThres( _T("0.5") )
, m_bSelectRenderer(FALSE)
, m_bDetailed(FALSE)
, m_bDispROI(FALSE)
, m_bDispPos(FALSE)
, m_bDispCoor(FALSE)
, m_bDispDetails(FALSE)
, m_bDispProfX(FALSE)
, m_bDispProfY(FALSE)
, m_bDispMGraphics(FALSE)
// , m_bDispScaledCoords(FALSE)
//, m_bDispContur(FALSE)
, m_iOnRendererOperation(0)
, m_bAdjustCoordinates(FALSE)
, m_bWasSelected( FALSE )
, m_bInFocus(FALSE)
, m_pOldTemplateFont(NULL)
, m_nTaskNumberEntering(0)
, m_iActiveTask(-1)
, m_iServiceTask(-1)
, m_iShownActiveTask(-1)
, m_iCaptionPeriod(0)
, m_pViewMode(NULL)
, m_bDispWeighted(FALSE)
, m_bMeasureAngle(FALSE)
, m_bMeasureDiameters(FALSE)
, m_bViewCoordsScaled(FALSE)
, m_bViewObjectContur(FALSE)
, m_bViewAngle(FALSE)
, m_bViewDia(FALSE)
, m_bBinaryOutput( FALSE )
, m_iNObjectsMax( 5000 )
, m_pGetDataFunction( pGetDataFunction )
, m_pGetTasksFunction( pGetTasksFunction )
, m_pGetObjectsAndTasksFunction( pGetObjectsAndTasksFunction )
, m_bInjectLastImage( FALSE )
, m_pLockObjects( NULL )
{
  //{{AFX_DATA_INIT(TVObjectsGadgetSetupDialog)
  //}}AFX_DATA_INIT
  m_Gadget=Gadget;
}

void TVObjectsGadgetSetupDialog::DoDataExchange(CDataExchange* pDX)	
//connecting the objects in the dialog with variables
{
  CGadgetSetupDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(TVObjectsGadgetSetupDialog)
  GetFXStringFromDlgItem( m_hWnd , pDX , IDC_EDITNAME , m_ObjectName );

  if ( !pDX->m_bSaveAndValidate ) // take from dialog
  {
    m_Detailed.Format( _T( "0x%x" ) , m_WhatToMeasure )  ;
  }


  //   DDX_Check( pDX , IDC_CHECKMULTIPLE , m_bMulti );
  //DDX_Check(pDX, IDC_CHECKDISPPROFILE, m_bDispProf);

  DDX_Control( pDX , IDC_OBJ_LIST , m_ObjectNames ) ;
  DDX_Control( pDX , IDC_TASK_LIST , m_Tasks ) ;

  DDX_Text( pDX , IDC_EDITTASKNUM , m_iShownTask );
  DDX_Control( pDX , IDC_BUTTONADDOBJ , m_buttAdd );
  DDX_Control( pDX , IDC_CHECKSELECT , m_buttSelectRenderer );
  DDX_Check( pDX , IDC_CHECKSELECT , m_bSelectRenderer );
  DDX_Control( pDX , IDC_BUTTONEDITOBJECT , m_buttEdit );
  DDX_Check( pDX , IDC_CHECKDISPANGLE , m_bMeasureAngle );
  DDX_Check( pDX , IDC_CHECKDIAMETERS , m_bMeasureDiameters );
  DDX_Check( pDX , IDC_SHOW_CONTUR , m_bViewObjectContur );
  DDX_Check( pDX , IDC_CHECKDISPROI , m_bDispROI );
  DDX_Check( pDX , IDC_CHECKDISPCOOR , m_bDispCoor );
  DDX_Check( pDX , IDC_SHOW_SCALED_COORDS , m_bViewCoordsScaled );
  DDX_Check( pDX , IDC_CHECKDISPPOS , m_bDispPos );
  DDX_Check( pDX , IDC_CHECKDISPDETAIL , m_bDispDetails );
  DDX_Check( pDX , IDC_CHECKDISPMGRAPHICS , m_bDispMGraphics );
  DDX_Check( pDX , IDC_CHECKDISPPROFX , m_bDispProfX );
  DDX_Check( pDX , IDC_CHECKDISPPROFY , m_bDispProfY );
  DDX_Check( pDX , IDC_CHECKDISPWEIGHTED , m_bDispWeighted );
  CString Tmp( (LPCTSTR) m_Detailed ) ;
  DDX_Text( pDX , IDC_ACTIVE_TASK , m_iShownActiveTask );
  //}}AFX_DATA_MAP
  DDX_Text( pDX , IDC_CAPTION_PERIOD , m_iCaptionPeriod );
  DDV_MinMaxInt( pDX , m_iCaptionPeriod , 0 , 1000 );
  if ( pDX->m_bSaveAndValidate ) // take from dialog
  {
    m_WhatToMeasure = (int) ConvToBinary( m_Detailed ) ;
  }
  DDX_Check( pDX , IDC_INJECT_LAST_IMAGE , m_bInjectLastImage );
}


BEGIN_MESSAGE_MAP(TVObjectsGadgetSetupDialog, CGadgetSetupDialog)
  //{{AFX_MSG_MAP(TVObjectsGadgetSetupDialog)
  ON_BN_CLICKED(IDC_APPLY, OnApply)
  ON_BN_CLICKED(IDC_BUTTONADDOBJ, OnBnClickedButtonaddobj)
  ON_BN_CLICKED(IDC_CHECKMULTIPLE, OnBnClickedCheckmultiple)
  ON_BN_CLICKED(IDC_BUTTONCLEAR, OnBnClickedButtonclear)
  ON_BN_CLICKED(IDC_BUTTONADDTASK, OnBnClickedButtonaddtask)
  ON_BN_CLICKED(IDC_CHECKSELECT, OnBnClickedCheckselect)
  ON_WM_TIMER()
  ON_BN_CLICKED(IDC_CHECKDISPROI, OnBnClickedCheckdisproi)
  ON_BN_CLICKED(IDC_CHECKDISPCOOR, OnBnClickedCheckdispcoor)
  ON_BN_CLICKED(IDC_CHECKDISPPROFX, OnBnClickedCheckdispprofx)
  ON_BN_CLICKED(IDC_CHECKDISPPOS, OnBnClickedCheckdisppos)
  ON_BN_CLICKED(IDC_CHECKDISPDETAIL, OnBnClickedCheckdispdetail)
  ON_BN_CLICKED(IDC_CHECKDISPPROFY, OnBnClickedCheckdispprofy)
  ON_BN_CLICKED(IDC_CHECKDISPMGRAPHICS, OnBnClickedCheckdispmgraphics)
  ON_BN_CLICKED(IDC_MARK_CENTER , OnBnClickedCatchObject)
  ON_BN_CLICKED(IDC_FIND_OBJECT, OnBnClickedCatchObject)
  ON_MESSAGE( NEW_OBJECT_FROM_UI_MSG , OnNewObjectFromUI)
  ON_MESSAGE( UPDATE_DLG_VIEW_MSG , OnUpdateDlgViewMessage)
  ON_BN_CLICKED(IDC_MARK_AREA_ON_RENDER , OnBnClickedCatchObject)
  ON_BN_CLICKED(IDC_DELETE_OBJECT, OnBnClickedDeleteObject)
  ON_BN_CLICKED(IDOK, &TVObjectsGadgetSetupDialog::OnBnClickedOk)
  ON_BN_CLICKED(IDCANCEL, &TVObjectsGadgetSetupDialog::OnBnClickedCancel)
  ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_SET_ACTIVE_TASK, OnBnClickedSetActiveTask)
  //}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_SHOW_SCALED_COORDS, &TVObjectsGadgetSetupDialog::OnBnClickedShowScaledCoords)
  ON_BN_CLICKED(IDC_SHOW_CONTUR, &TVObjectsGadgetSetupDialog::OnBnClickedShowContur)
  ON_BN_CLICKED(IDC_DONT_TOUCH_EDGE, &TVObjectsGadgetSetupDialog::OnBnClickedDontTouchEdge)
  ON_BN_CLICKED(IDC_FORM_MASK, &TVObjectsGadgetSetupDialog::OnBnClickedFormMask)
  ON_BN_CLICKED(IDC_CHECKDISPANGLE, &TVObjectsGadgetSetupDialog::OnBnClickedCheckdispangle)
  ON_BN_CLICKED(IDC_CHECKDIAMETERS, &TVObjectsGadgetSetupDialog::OnBnClickedCheckdiameters)
  ON_BN_CLICKED(IDC_CHECKDISPWEIGHTED, &TVObjectsGadgetSetupDialog::OnBnClickedCheckdispweighted)
  ON_BN_CLICKED( IDC_BINARY_OUT , &TVObjectsGadgetSetupDialog::OnBnClickedBinaryOut )
  ON_LBN_DBLCLK( IDC_OBJ_LIST , &TVObjectsGadgetSetupDialog::OnLbnDblclkObjList )
  ON_LBN_DBLCLK( IDC_TASK_LIST , &TVObjectsGadgetSetupDialog::OnLbnDblclkTaskList )
  ON_LBN_SELCHANGE( IDC_OBJ_LIST , &TVObjectsGadgetSetupDialog::OnLbnSelchangeObjList )
  ON_LBN_SELCHANGE( IDC_TASK_LIST , &TVObjectsGadgetSetupDialog::OnLbnSelchangeTaskList )
  ON_BN_CLICKED( IDC_REINJECT_IMAGE , &TVObjectsGadgetSetupDialog::OnBnClickedReinjectImage )
  ON_REGISTERED_MESSAGE( VM_TVDB400_SHOWVOSETUPDLG , OnShowVOSetupDialog )
END_MESSAGE_MAP()


bool TVObjectsGadgetSetupDialog::Show(CPoint point, LPCTSTR uid)
{
  HINSTANCE hOldResource=AfxGetResourceHandle();
  AfxSetResourceHandle(pThisDll->m_hResource);

  FXString DlgHead;
  DlgHead.Format("%s Setup Dialog", uid);
  if (!m_hWnd)
  {
    if (!Create(IDD_SETUPDLG, NULL))
    {
      SENDERR_0("Failed to create Setup Dialog");
      AfxSetResourceHandle(hOldResource);
      return false;
    }
  }
  SetWindowText(DlgHead);
  SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  ShowWindow(SW_SHOWNORMAL);
  AfxSetResourceHandle(hOldResource);
  return true;
}


VOBJ_TYPE TVObjectsGadgetSetupDialog::GetSelectedType()
{
  if ( GetCheckBoxState( IDC_TYPE_SPOT ) )
    m_SelectedObjectType = SPOT ;
  else  if ( GetCheckBoxState( IDC_TYPE_LINE ) )
    m_SelectedObjectType = LINE_SEGMENT ;
  else  if ( GetCheckBoxState( IDC_TYPE_EDGE ) )
    m_SelectedObjectType = EDGE ;
  else  if ( GetCheckBoxState( IDC_TYPE_OCR ) )
    m_SelectedObjectType = OCR ;
  else
  {
    ASSERT( 0 ) ;
    m_SelectedObjectType = SPOT ;
    SetSelectedType( SPOT ) ;
  }
  return m_SelectedObjectType ;
}
void TVObjectsGadgetSetupDialog::SetSelectedType( VOBJ_TYPE Type )
{
  SetCheckBoxState( IDC_TYPE_SPOT , 0 ) ;
  SetCheckBoxState( IDC_TYPE_LINE , 0 ) ;
  SetCheckBoxState( IDC_TYPE_EDGE , 0 ) ;
  SetCheckBoxState( IDC_TYPE_OCR , 0 ) ; 
  m_SelectedObjectType = Type ;
  switch ( Type )
  {
  case SPOT: SetCheckBoxState( IDC_TYPE_SPOT , 1 ) ; break ;
  case LINE_SEGMENT: SetCheckBoxState( IDC_TYPE_LINE , 1 ) ; break ;
  case EDGE: SetCheckBoxState( IDC_TYPE_EDGE , 1 ) ; break ;
  default: ASSERT( 0 ) ; 
  case OCR: SetCheckBoxState( IDC_TYPE_OCR , 1 ) ; break ;
  }
}

/////////////////////////////////////////////////////////////////////////////
// TVObjectsGadgetSetupDialog message handlers

LRESULT TVObjectsGadgetSetupDialog::OnNewObjectFromUI(
  WPARAM wParam, LPARAM lParam)
{
  //   switch ( wParam )
  //   {
  //   case 1: m_bAdjustCoordinates = TRUE ; OnBnClickedTrackobj() ; break ;
  //   case 2: m_bAdjustCoordinates = FALSE ; OnBnClickedTrackobj() ; break ;
  //   case 3: /*OnBnClickedButtonfindobject() ;*/ break ;
  //   }

  CVideoObject NewObj ;
  if ( !m_LastRectFromUI.IsRectNull() )
  {
    NewObj.m_AOS = m_LastRectFromUI ;
    NewObj.m_AOS.bottom -= NewObj.m_AOS.top ;
    NewObj.m_AOS.right -= NewObj.m_AOS.left ;

    m_LastRectFromUI.SetRectEmpty() ;
  }
  else
  {
    NewObj.m_AOS = CRect( m_PtSelectStart.x , m_PtSelectStart.y ,
      m_PtSelectEnd.x - m_PtSelectStart.x , 
      m_PtSelectEnd.y - m_PtSelectStart.y ) ;
  }

  NewObj.m_Placement = PLACE_ABS ;

  GetSelectedType() ;
  GetFXStringFromDlgItem( m_hWnd , IDC_EDITNAME , m_ObjectName ) ;
  if ( m_ObjectName.IsEmpty() )
  {
    NewObj.m_ObjectName = VOTypeToVOName( m_SelectedObjectType ) ;
    NewObj.m_ObjectName += _T( '_' ) ;
    m_ObjectName = NewObj.m_ObjectName ;
  }
  else
    NewObj.m_ObjectName = m_ObjectName ;

  NewObj.m_dwViewMode = OBJ_VIEW_ROI ;
  NewObj.m_iMinContrast = MIN_AMPL ;
  NewObj.m_dMinProfileContrast = MIN_AMPL ;

  NewObj.m_Type = m_SelectedObjectType ;
  switch ( m_SelectedObjectType )
  {
  case LINE_SEGMENT:
    NewObj.m_Direction = (NewObj.m_AOS.right > NewObj.m_AOS.bottom ) ? VERTICAL : HORIZONTAL ;
    NewObj.m_Contrast = BLACK_ON_WHITE ;
    NewObj.m_ExpectedSize = CRect( 1 , 1 , 10 , 10 ) ;
    NewObj.m_WhatToMeasure |= MEASURE_THICKNESS | MEASURE_POSITION;
    NewObj.m_dwViewMode |= OBJ_VIEW_POS | OBJ_VIEW_COORD ;

    break ;
  case EDGE:
    NewObj.m_Direction = (NewObj.m_AOS.right > NewObj.m_AOS.bottom) ? DIR_RL : DIR_DU ;
    NewObj.m_Contrast = WHITE_TO_BLACK_BRD ;
    NewObj.m_dwViewMode |= OBJ_VIEW_POS | OBJ_VIEW_COORD ;
    NewObj.m_WhatToMeasure |= MEASURE_POSITION;
    break ;
  case OCR:
    NewObj.m_Direction = (m_Direction == VERTICAL) ? DIR_03 : DIR_06 ;
    NewObj.m_Contrast = BLACK_ON_WHITE ;
    NewObj.m_WhatToMeasure |= MEASURE_TEXT | MEASURE_TXT_FAST ;
    NewObj.m_dwViewMode |= OBJ_VIEW_TEXT ;
    break ;
  case SPOT:
  default:
    NewObj.m_Type = SPOT ; // for default
    NewObj.m_Contrast = BLACK_ON_WHITE ;
    NewObj.m_ExpectedSize = CRect( 1 , 1 , (NewObj.m_AOS.right * 2)/3 , (NewObj.m_AOS.bottom * 2) / 3 ) ;
    NewObj.m_iAreaMin = 1 ;
    NewObj.m_iAreaMax = (NewObj.m_AOS.right * NewObj.m_AOS.bottom * 4) / 9 ;
    NewObj.m_dwViewMode |= OBJ_VIEW_POS | OBJ_VIEW_COORD ;
    NewObj.m_WhatToMeasure |= MEASURE_POSITION | MEASURE_AREA ;
    break ;
  }

  FXString ObjectsForTask ;
  GetFXStringFromDlgItem( m_hWnd , IDC_EDITNAME , ObjectsForTask ) ;
  m_iShownTask = GetDlgItemInt( IDC_EDITTASKNUM ) ;

  if ( m_pGetObjectsAndTasksFunction )
  {
    VOArray * pObjects = NULL , * pNewObjects = new VOArray ;
    CVOJobList * pTasks = NULL ;
    CVOJobList NewTasks ;
    int iActiveTask = -1 ;
    m_pGetObjectsAndTasksFunction( m_pGadget ,
      (void**) &pObjects , (void**) &pTasks , &iActiveTask , &m_pLockObjects ) ;
    if ( pObjects && pTasks )
    {
      if ( pNewObjects )
        pNewObjects->Copy( *pObjects ) ;
      
      POSITION pos = pTasks->GetStartPosition();
      while ( pos )
      {
        WORD rKey;
        void* rValue;

        pTasks->GetNextAssoc( pos , rKey , rValue );
        CVOJob* job = (CVOJob*) rValue;
        NewTasks.SetAt( rKey , *job ) ;
      }
    }
    NewObj.m_pVOArrayLock = m_pLockObjects ;

    m_pGetObjectsAndTasksFunction( m_pGadget , NULL , NULL , NULL , NULL ) ; // release for normal work
      
    // Check for the same name absence
    TCHAR Addition = _T( 'a' ) ;
    for ( int i = 0 ; i < pNewObjects->GetCount() ; i++ )
    {
      CVideoObject& Obj = pNewObjects->GetAt( i ) ;
      Obj.m_pVObjects = pNewObjects ;
      if ( Obj.m_ObjectName == NewObj.m_ObjectName )
      {
        NewObj.m_ObjectName += Addition ;
        Addition++ ;
        i = -1 ;
        continue ; // repeat loop with name
      }
    }
    if ( Addition != _T('a') )  // set name on dialog
      SetDlgItemText( IDC_EDITNAME , (LPCTSTR) NewObj.m_ObjectName ) ;

    m_ObjectNames.AddString( (LPCTSTR) NewObj.m_ObjectName ) ;
    m_ObjectNames.Invalidate() ;
    pNewObjects->Add( NewObj ) ;
    CVideoObject& InArray = pNewObjects->GetAt( pNewObjects->GetUpperBound() ) ;
    InArray.m_pVObjects = pNewObjects ;
    // OK, object added to existent objects array

    // Now we should look at jobs:
    // If ActiveJob and ShownJob are the same, object will be added 
    // to shown/active job and object will be visible on next arrived image
    // If Active job and shown job are not the same, object will be 
    // simply added to shown task PRESENTATION (Objects in Task string).
    // New object could be added to shown task by pressing on Update task
    // button

    // 1. In any case add to the shown task string
    AddObjectToShownList( NewObj.m_ObjectName ) ;
    // 2. If shown task is active task, add to the this task
    if ( iActiveTask == m_iShownTask )
    {
      CVOJob Job ;
      CVOTask NewTask = { NewObj.m_ObjectName , -1 } ;
      if ( NewTasks.Lookup( iActiveTask , Job ) )
      {
        Job.Add( NewTask ) ;
        //NewTasks.SetAt( iActiveTask , Job ) ;
      }

      Link( pNewObjects , NewTasks ) ;

//       m_Tasks.ResetContent() ;
//       FXStringArray Jobs ;
//       NewTasks.PrintJobs( Jobs ) ;
//       for ( int i = 0 ; i < Jobs.GetCount() ; i++ )
//       {
//         m_Tasks.AddString( Jobs[ i ] ) ;
//         if ( i == m_iShownTask  &&  i == iActiveTask )
//         {
//           FXString Inside ;
//           if ( Jobs[i].GetSubStringInBrackets( Inside ) )
//           {
//             SetDlgItemText( IDC_EDITOBJNAMES , (LPCTSTR) Inside ) ;
//           }
//         }
//       }
    }

    CVOJobList *pJobs = &NewTasks ;
    UpdateObjectsAndTasksLists( pNewObjects , pJobs ) ;
    m_pGetObjectsAndTasksFunction( m_pGadget , (void**)&pNewObjects , (void**)&pJobs , &iActiveTask , NULL ) ;

    FXPropKit2 pk ;
    pk.WriteInt( "ViewMode" , GetGadgetViewMode() ) ;
    pk.WriteInt( "ActiveTask" , iActiveTask ) ;
    if ( GetCheckBoxState( IDC_REINJECT_IMAGE ) )
      pk.WriteInt( "Reinject" , 1 ) ;

    bool bInvalidate = false ;
    m_pGadget->ScanProperties( pk , bInvalidate ) ;

    CWnd * pMainWnd = AfxGetApp()->GetMainWnd() ;
    if ( pMainWnd )
    {
      CRect rc ;
      pMainWnd->GetWindowRect( &rc ) ;
      CPoint Pt = rc.TopLeft() ;
      Pt += CSize( 150 , 100 + 5 * (int) pNewObjects->GetCount() ) ;
      WPARAM wPar = (Pt.x & 0xffff) | ((Pt.y << 16) & 0xffff) ;
      ::PostMessage( pMainWnd->m_hWnd , VM_TVDB400_SHOWVOSETUPDLG ,
        wPar , (LPARAM) &InArray ) ;
    }

    NewTasks.RemoveAll() ;
    m_iActiveTask = iActiveTask ;
    
    UpdateData( FALSE ) ;  // take data from variables
  }
// 
// 
// 
// 
// 
// 
//   m_bAdjustCoordinates = (int) wParam ;
//   OnBnClickedTrackobj() ; 
  GetParametersFromMainGadget( NULL ) ;
  return 1 ;
}

LRESULT TVObjectsGadgetSetupDialog::OnUpdateDlgViewMessage(
  WPARAM wParam, LPARAM lParam)
{
  GetParametersFromMainGadget() ;
  return 1 ;
}


void TVObjectsGadgetSetupDialog::UploadParams(CGadget* Gadget)
{
  bool Invalidate=false;
  FXPropertyKit pk;
//   pk.WriteString("Template", m_Template , false ) ;
  pk.WriteInt( "ActiveTask" , m_iActiveTask ) ;
  pk.WriteInt( "CaptionPeriod" , m_iCaptionPeriod ) ;
  pk.WriteInt( "ViewMode" , GetGadgetViewMode() ) ;
  Gadget->ScanProperties(pk,Invalidate);
}

void TVObjectsGadgetSetupDialog::OnApply()		//When pressing Apply
{
  UpdateData(TRUE);
  if ( m_pViewMode )
  {
    m_dwViewMode = (((DWORD)m_bDispROI) * OBJ_VIEW_ROI)
      | ( ((DWORD)m_bDispPos) * OBJ_VIEW_POS ) 
      | ( ((DWORD)m_bDispCoor) * OBJ_VIEW_COORD )
      | ( ((DWORD)m_bDispDetails) * OBJ_VIEW_DIFFR ) 
      | ( ((DWORD)m_bDispProfX) * OBJ_VIEW_PROFX ) 
      | ( ((DWORD)m_bDispProfY) * OBJ_VIEW_PROFY )  
      | ( ((DWORD)m_bDispMGraphics) * OBJ_VIEW_MRECTS ) 
      | ( ((DWORD)m_bViewCoordsScaled)  * OBJ_VIEW_SCALED )
      | ( ((DWORD)m_bViewObjectContur) * OBJ_VIEW_CONT ) 
      | ( ((DWORD)m_bViewAngle) * OBJ_VIEW_ANGLE) 
      | ( ((DWORD)m_bDispWeighted) * OBJ_WEIGHTED) 
      | ( ((DWORD)m_bViewDia) * OBJ_VIEW_DIA) ;

    m_pViewMode->SetViewMode( m_dwViewMode ) ;
    //m_WhatToMeasure = m_dwViewMode ;
  }

  UploadParams(m_pGadget);
  CGadgetSetupDialog::UploadParams() ; // modified marking
  UpdateData( FALSE );
}

static const char replaced_chars[][2]={";","\n","\r","\\"};
static const int  replacemen_nmb=4;
static const char replacement[][3]={"\\s","\\n","\\r","\\\\"};


BOOL TVObjectsGadgetSetupDialog::OnInitDialog() //When the dialog starts
{
  CGadgetSetupDialog::OnInitDialog();
  EnableToolTips() ;

  if ( !m_ToolTip.Create( this ) )
  {
    TRACE( "Unable to create ToolTipCtrl.\r\n" );
  }
  else if ( !m_ToolTip.AddTool( GetDlgItem( IDC_OBJ_LIST ) , 
    _T("List of available objects for all tasks:   "
       "Click - select object name and type in dlg "
       "Dbl Click:  View object properties Dlg  "
       "CTRL+Click:  add object to Active Task   "
       "SHIFT+Click: add object to Edited Task   "
       "CTRL-SHIFT-Click: delete from Edited Task"
       ) ) )
  {
    TRACE( "Unable to add tool tip for Object list" );
  }
  else if ( !m_ToolTip.AddTool( GetDlgItem( IDC_TASK_LIST ) , 
    _T("List of all tasks (format:<task number>, "
       "<List of objects with comma as separator)"
       "Dbl Click - Select Task for editing  "
       "CTRL-SHIFT-Click - delete Task"
    ) ) )
  {
    TRACE( _T("Unable to add tool tip for tasks") );
  }
  m_ToolTip.SetMaxTipWidth( 220 ) ;
  m_ToolTip.Activate( TRUE );

  m_ObjectName.Empty();
  SetSelectedType( SPOT ) ;

  m_iShownTask = -1 ;
  GetParametersFromMainGadget( m_pViewMode ) ;
  return TRUE;  
}

void TVObjectsGadgetSetupDialog::GetParametersFromMainGadget( 
  ViewModeOptionData * pViewMode ) //When the dialog starts
{
  FXString text;
  if (m_Gadget->PrintProperties(text))
  {
    FXPropKit2 pk(text);
    if ( m_pGetDataFunction )
    {
      VOArray * pAllObjects = NULL ;
      // Get all objects array and lock it
      m_pGetDataFunction( m_pGadget , (void**) &pAllObjects ) ;
      // Unlock all objects array
      m_ObjectNames.ResetContent() ;
      if ( pAllObjects && pAllObjects->GetCount() )
      {
        for ( int i = 0 ; i < pAllObjects->GetCount() ; i++ )
        {
          FXString ObjName ;
          CVideoObject& Obj = pAllObjects->GetAt( i ) ;
          if ( Obj.GetObjectName( ObjName ) )
          {
            ObjName += " ( " ;
            ObjName += VOTypeToVOName( Obj.m_Type ) ;
            ObjName += " )" ;
            m_ObjectNames.AddString( ObjName ) ;
          }
        }
      }
      m_pGetDataFunction( m_pGadget , NULL ) ;
    }
    if ( m_pGetTasksFunction )
    {
      FXStringArray Tasks ;
      int iCnt = m_pGetTasksFunction( m_pGadget , &Tasks ) ;
      m_Tasks.ResetContent() ;
      for ( int i = 0 ; i < iCnt ; i++ )
        m_Tasks.AddString( Tasks[ i ] ) ;
      m_pGetTasksFunction( m_pGadget , NULL ) ;
    }

    if ( !pk.GetInt( "ActiveTask" , m_iActiveTask ) )
      m_iActiveTask = -1 ;
    m_iShownActiveTask = m_iActiveTask ;
    pk.GetInt( "CaptionPeriod" , m_iCaptionPeriod ) ;
    if ( pViewMode == NULL )
    {
      int iViewMode ;
      if ( pk.GetInt( "ViewMode" , iViewMode ) )
      {
        m_pViewMode->SetViewMode( iViewMode ) ;
      }
    }
  }
//  m_Template = m_Template.TrimLeft(" \t\r\n") ;
  SetDefaultInitialization( pViewMode );
  UpdateData(FALSE);
}

ObjTypesAndNames TypesAndNames[]=
{
  { TASK , _T("task") } ,
  { LINE_SEGMENT , _T("line") } ,
  { SPOT , _T("spot") } ,
  { EDGE , _T("edge") } ,
  { OCR , _T("ocr") } 
} ;



void TVObjectsGadgetSetupDialog::OnBnClickedButtonaddobj()		//When pressing Add Object
  {
  FXPropKit2 pk ;
  m_pGadget->PrintProperties( pk ) ;
  CSize FrameSize( 640 , 480 ) ;
  pk.GetInt( "LastImageWidth" , FrameSize.cx ) ;
  pk.GetInt( "LastImageHeight" , FrameSize.cy ) ;
  if ( FrameSize.cx == 0 ) // no frames passed
    FrameSize.cx = 640 ;
  if ( FrameSize.cy == 0 )
    FrameSize.cy = 480 ;
  FrameSize.cx /= 2 ;
  FrameSize.cy /= 2 ;
  m_PtSelectStart = CPoint( FrameSize.cx - 50 , FrameSize.cy - 50 ) ;
  m_PtSelectEnd = CPoint( FrameSize.cx + 50 , FrameSize.cy + 50 ) ;
  OnNewObjectFromUI( 0 , NULL ) ;
}


void TVObjectsGadgetSetupDialog::OnBnClickedCheckmultiple()	//When Checking\Unchecking Multiple
{
  UpdateData(TRUE);
}

void TVObjectsGadgetSetupDialog::SetDefaultInitialization( ViewModeOptionData * pViewMode )	//Default Initialization
{
  if ( pViewMode )
    m_pViewMode = pViewMode ;
  m_buttSelectRenderer.EnableWindow(TRUE);
  m_AOS.left = 0;
  m_AOS.top = 0;
  m_AOS.right = 0;
  m_AOS.bottom = 0;
  m_RelAOS.left = 0;
  m_RelAOS.top = 0;
  m_RelAOS.right = 0;
  m_RelAOS.bottom = 0;
  m_sRelName.Empty();
  m_ExpectedSize.left = 0;
  m_ExpectedSize.top = 0;
  m_ExpectedSize.right = 0;
  m_ExpectedSize.bottom = 0;
  m_iMinContrast = 20;
  m_ViewOffset.cx = 5;
  m_ViewOffset.cy = -5;
  m_iViewSize = 20;
  m_iAreaMin = 0;
  m_iAreaMax = 0;
  m_DiffrRadius.cx = 0;
  m_DiffrRadius.cy = 0;
  m_bMulti = FALSE;
  m_WhatToMeasure = 0 ;
  //m_bDispProf = FALSE;
  m_bDetailed = FALSE;
  m_bDontTouchEdge = TRUE ;
//   m_bSelectRenderer = FALSE;

  if ( m_iShownTask >= 0 )
  {
    if ( m_pGetTasksFunction )
    {
      FXStringArray Tasks ;
      m_pGetTasksFunction( m_pGadget , &Tasks ) ;
      m_pGetTasksFunction( m_pGadget , NULL ) ; // release lock

      for ( int i = 0 ; i < Tasks.GetCount() ; i++ )
      {
        int iPos = (int) Tasks[ i ].Find( _T( '(' ) ) ;
        if ( iPos >= 0 )
        {
          int iTaskNum = atoi( ((LPCTSTR) Tasks[ i ]) + iPos + 1 ) ;
          if ( iTaskNum == m_iShownTask )
          {
            int iCommaPos = (int) Tasks[ i ].Find( _T( ',' ) , iPos + 1 ) ;
            int iClosePos = (int) Tasks[ i ].Find( _T( ')' ) , iCommaPos + 1 ) ;
            SetDlgItemText( IDC_EDITOBJNAMES , Tasks[ i ].Mid(
              iCommaPos + 1 , (iClosePos) ? iClosePos - iCommaPos - 1 : (int) Tasks[ i ].GetLength() - iCommaPos - 1 ) ) ;
          }
        }
      }
    }
        
  }
  m_DetThres = _T("0.5") ;
  m_buttEdit.EnableWindow(FALSE);
  if ( m_pViewMode )
  {
    m_bDispROI = m_pViewMode->getbDispROI();
    m_bDispPos = m_pViewMode->getbDispPos();
    m_bDispCoor = m_pViewMode->getbDispCoor();
    m_bDispDetails = m_pViewMode->getbDispDetails();
    m_bDispProfX = m_pViewMode->getbDispProfX();
    m_bDispProfY = m_pViewMode->getbDispProfY();
    m_bDispMGraphics = m_pViewMode->getbDispMGraphics();
    m_bViewCoordsScaled = m_pViewMode->getbViewCoordScaled() ;
    m_bViewObjectContur = m_pViewMode->getbViewObjectContur() ;
  }
}

void TVObjectsGadgetSetupDialog::ReSelectComboButton(CComboBox* pCombo, FXString sText)
{
  FXString sRes;
  for (int i=0 ; i < pCombo->GetCount(); i++)
  {
    TCHAR Buf[90] ;
    pCombo->GetLBText(i,Buf);
    sRes = Buf ;
    if(sRes.Compare(sText) == 0)
    {
      pCombo->SetCurSel(i);
      return;
    }
  }
  pCombo->SetCurSel(0);
}

void TVObjectsGadgetSetupDialog::OnBnClickedButtonclear()
{
  SetDefaultInitialization();
}

void TVObjectsGadgetSetupDialog::ConvertColorToColorCode(FXString sText, FXString* sColor)
{
  if (sText.CompareNoCase("white") == 0)
    *sColor = "0xFFFFFF";
  if (sText.CompareNoCase("silver") == 0)
    *sColor = "0xC0C0C0";
  if (sText.CompareNoCase("gray") == 0)
    *sColor = "0x808080";
  if (sText.CompareNoCase("black") == 0)
    *sColor = "0x000000";
  if (sText.CompareNoCase("blue") == 0)
    *sColor = "0xFF0000";
  if (sText.CompareNoCase("navy") == 0)
    *sColor = "0x800000";
  if (sText.CompareNoCase("teal") == 0)
    *sColor = "0xFFFF00";
  if (sText.CompareNoCase("green") == 0)
    *sColor = "0x00FF00";
  if (sText.CompareNoCase("lime") == 0)
    *sColor = "0x80FF00";
  if (sText.CompareNoCase("olive") == 0)
    *sColor = "0x008000";
  if (sText.CompareNoCase("yellow") == 0)
    *sColor = "0x00FFFF";
  if (sText.CompareNoCase("red") == 0)
    *sColor = "0x0000FF";
  if (sText.CompareNoCase("maroon") == 0)
    *sColor = "0x000080";
  if (sText.CompareNoCase("fuchsia") == 0)
    *sColor = "0xFF00FF";
  if (sText.CompareNoCase("purple") == 0)
    *sColor = "0x800080";
  *sColor = sColor->MakeLower() ;
}
// void TVObjectsGadgetSetupDialog::ConvertColorCodeToColor(const FXString colorCode)
// {
//   if (colorCode.CompareNoCase("0xFFFFFF") == 0)
//     m_comboColorROI.SetCurSel(0);
//   if (colorCode.CompareNoCase("0xC0C0C0") == 0)
//     m_comboColorROI.SetCurSel(1);
//   if (colorCode.CompareNoCase("0x808080") == 0)
//     m_comboColorROI.SetCurSel(2);
//   if (colorCode.CompareNoCase("0x000000") == 0)
//     m_comboColorROI.SetCurSel(3);
//   if (colorCode.CompareNoCase("0xFF0000") == 0)
//     m_comboColorROI.SetCurSel(4);
//   if (colorCode.CompareNoCase("0x800000") == 0)
//     m_comboColorROI.SetCurSel(5);
//   if (colorCode.CompareNoCase("0xFFFF00") == 0)
//     m_comboColorROI.SetCurSel(6);
//   if (colorCode.CompareNoCase("0x00FF00") == 0)
//     m_comboColorROI.SetCurSel(7);
//   if (colorCode.CompareNoCase("0x80FF00") == 0)
//     m_comboColorROI.SetCurSel(8);
//   if (colorCode.CompareNoCase("0x008000") == 0)
//     m_comboColorROI.SetCurSel(9);
//   if (colorCode.CompareNoCase("0x00FFFF") == 0)
//     m_comboColorROI.SetCurSel(10);
//   if (colorCode.CompareNoCase("0x0000FF") == 0)
//     m_comboColorROI.SetCurSel(11);
//   if (colorCode.CompareNoCase("0x000080") == 0)
//     m_comboColorROI.SetCurSel(12);
//   if (colorCode.CompareNoCase("0xFF00FF") == 0)
//     m_comboColorROI.SetCurSel(13);
//   if (colorCode.CompareNoCase("0x800080") == 0)
//     m_comboColorROI.SetCurSel(14);
// }

void TVObjectsGadgetSetupDialog::OnBnClickedButtonaddtask()
{
  FXString ObjectsForTask ;
  GetFXStringFromDlgItem( m_hWnd , IDC_EDITOBJNAMES , ObjectsForTask ) ;
  m_iShownTask = GetDlgItemInt( IDC_EDITTASKNUM ) ;

  if ( m_pGetObjectsAndTasksFunction )
  {
    VOArray * pObjects = NULL ;
    CVOJobList * pTasks = NULL ;
    int iActiveTask = -1 ;
    m_pGetObjectsAndTasksFunction( m_pGadget ,
      (void**) &pObjects , (void**) &pTasks , &iActiveTask , &m_pLockObjects ) ;
    if ( pObjects && pTasks )
    {
      CVOJob Job ;
      if ( pTasks->Lookup( m_iShownTask , Job ) )
        Job.RemoveAll() ;

      FXSIZE iPos = 0 ;
      while ( iPos >= 0 )
      {
        FXString ObjName = ObjectsForTask.Tokenize( _T( "," ) , iPos ) ;
        if ( !ObjName.IsEmpty() )
        {
          CVOTask NewTask = { ObjName , -1 } ;
          Job.Add( NewTask ) ;
        }
        else
        {
          if ( Job.GetCount() > 0 )
            pTasks->SetAt( m_iShownTask , Job ) ;
          break ;
        }
      }
      Link( pObjects , *pTasks ) ;

      m_Tasks.ResetContent() ;
      FXStringArray Jobs ;
      pTasks->PrintJobs( Jobs ) ;
      for ( int i = 0 ; i < Jobs.GetCount() ; i++ )
        m_Tasks.AddString( Jobs[i] ) ;

      FXPropKit2 pk ;
      pk.WriteInt( "ViewMode" , GetGadgetViewMode() ) ;

      if ( m_iShownTask == iActiveTask )
      {
        pk.WriteInt( "ActiveTask" , iActiveTask ) ;
      }
      bool bInvalidate = false ;
      m_pGadget->ScanProperties( pk , bInvalidate ) ;
    }
    m_pGetObjectsAndTasksFunction( m_pGadget , NULL , NULL , NULL , NULL ) ;
  }

}

void TVObjectsGadgetSetupDialog::OnBnClickedCheckselect()
{
  m_bSelectRenderer = !m_bSelectRenderer;
  FXString sText ;
  if (m_bSelectRenderer == FALSE)
  {
    m_sSelectRendererStart.Empty();
    m_sSelectRendererEnd.Empty();
  }
  UpdateData(TRUE);
}

BOOL TVObjectsGadgetSetupDialog::UpdatePosFromRenderer(void)
{
  if ( m_PtSelectEnd.x < 0 )
    return FALSE;

//   switch ( m_comboAbsRel.GetCurSel() )
//   {
//   case 0:
//     m_AOS.left = m_PtSelectStart.x ;
//     m_AOS.top = m_PtSelectStart.y ;
//     m_AOS.right = m_PtSelectEnd.x - m_AOS.left;
//     m_AOS.bottom = m_PtSelectEnd.y - m_AOS.top;
//     if ((m_AOS.right < 0) || (m_AOS.bottom < 0))
//       SENDERR_0("Error : Selected Width/Height is negative");
//     break ;
//   }
  m_sSelectRendererStart.Empty();
  m_sSelectRendererEnd.Empty();
  m_PtSelectStart.x = -1 ;
  return TRUE;
}

int TVObjectsGadgetSetupDialog::AddingObject( FXString& ObjName , int iPos )
{
  return TRUE;
}

int TVObjectsGadgetSetupDialog::AddNewObjectToTask(
  FXString& ObjName , int iTaskNumber )
{
//   int nBracket = 0;
//   for ( int i = 0 ; i < m_ExistentTasks.GetCount() ; i++ )
//   {
//     if ( (int)m_ExistentTasks[i] == iTaskNumber )   // necessary task exists
//     {
//       int nTaskPos = (int) m_Template.Find("task");    // find row with task
//       while ( nTaskPos != -1)		//there is task in template
//       {
//         int nBracket = (int) m_Template.Find("(",nTaskPos);   // find '(' - task number is after that
//         if ( atoi( ((LPCTSTR)m_Template) + nBracket + 1 ) == iTaskNumber ) // check task number
//         {
//           int iCloseBracket = (int) m_Template.Find( ")" , nBracket ) ; // find closing
//           if ( iCloseBracket < 0 )
//             return -1 ;
//           m_Template.Insert( iCloseBracket , ',' ) ;  // insert comma nad object name after that
//           m_Template.Insert( iCloseBracket + 1 , ObjName ) ;
//           return iCloseBracket + 1 ;
//         }
//         nTaskPos = (int) m_Template.Find("task" , nBracket + 1 ) ;
//       }
//     }
//   }
  return 0 ; // nothing inserted
}
int TVObjectsGadgetSetupDialog::DeleteObjectFromTask(
  FXString& ObjName , int iTaskNumber )
{
//   int nBracket = 0;
//   for ( int i = 0 ; i < m_ExistentTasks.GetCount() ; i++ )
//   {
//     if ( (int)m_ExistentTasks[i] == iTaskNumber )   // necessary task exists
//     {
//       int nTaskPos = (int) m_Template.Find("task");    // find row with task
//       while ( nTaskPos != -1)		//there is task in template
//       {
//         int nBracket = (int) m_Template.Find("(",nTaskPos);   // find '(' - task number is after that
//         if ( atoi( ((LPCTSTR)m_Template) + nBracket + 1 ) == iTaskNumber ) // check task number
//         {
//           int iCloseBracket = (int) m_Template.Find( ")" , nBracket ) ; // find closing
//           if ( iCloseBracket < 0 )
//             return -1 ;
//           int iObjPos = (int) m_Template.Find( ObjName , nBracket ) ;
//           if ( iObjPos >= 0  &&  iObjPos < iCloseBracket )
//           {
//             m_Template.Delete( iObjPos , ObjName.GetLength() ) ;
//             while ( m_Template[iObjPos] == ' '
//               || m_Template[iObjPos] == ',' )
//             {
//               if ( m_Template.GetLength() > iObjPos )
//                 m_Template.Delete( iObjPos ) ;
//             }
//           }
//           iCloseBracket = (int) m_Template.Find( ")" , nBracket ) ; // find closing        
//           return iCloseBracket + 1 ;
//         }
//         nTaskPos = (int) m_Template.Find("task" , nBracket + 1 ) ;
//       }
//     }
//   }
  return 0 ; // nothing removed
}

int TVObjectsGadgetSetupDialog::CreateNewTaskWithObject( FXString& ObjName )
{
//   int iLastTask = -1 ;
//   int nTaskPos = (int) m_Template.Find("task");
//   if ( nTaskPos < 0 )
// 
//   {
//     m_ExistentTasks.RemoveAll() ;
//   }
//   else
//   {
//     for ( int iTaskCnt = 0 ; iTaskCnt < (int) m_ExistentTasks.GetCount() ; iTaskCnt++ )
//     {
//       if ( (m_ExistentTasks[iTaskCnt] - iLastTask) > 1 )
//         break ;
//       iLastTask = m_ExistentTasks[ iTaskCnt ] ;
//     }
//   }
  int iNewTask = 100 ; // iLastTask + 1 ;
//   m_Template.TrimRight( " \t\n\r") ;
//   FXString NewTask ;
//   NewTask.Format( "task(%d,%s),\r\n" , iNewTask , (LPCTSTR)ObjName ) ;
//   m_Template.Append( NewTask ) ;
//   m_ExistentTasks.Add(iNewTask) ;
  return iNewTask ;
}

void TVObjectsGadgetSetupDialog::ClickingTaskButtonResponse(void)
{
//   int nTaskPos = -1;
//   int nBracket = 0;
//   int nComma = 0;
//   int nTaskEndPos = 0;
//   UpdateData(TRUE);	//Updating the parameters using the filled boxes
//   FXParser2 temp = m_Template  ;
//   FXString val;
//   int pos=0;
// 
//   while ((nTaskPos = (int) temp.Find("task",nTaskPos+1)) != -1)	//if a task was found
//   {
//     nComma = (int) temp.Find(",",nTaskPos);
//     val = (temp.Mid(nTaskPos+5,nComma-nTaskPos-5)).Trim(" ");
//     {
//       if (m_iShownTask == atoi(val))
//       {
//         nTaskEndPos = (int) temp.Find("\n",nTaskPos+1);
//         if (nTaskEndPos == -1)		//No more tasks after this one
//         {
//           if ( nTaskPos >= 2 )
//             temp.Truncate((nTaskPos-2));		//-2 for "\r\n"
//         }
//         else
//           temp.Delete(nTaskPos,nTaskEndPos-nTaskPos);
//         //         break;
//       }
//     }
//   }
// 
//   if (m_sObjNames.IsEmpty() )
//   {
//     //     SENDERR_0("Error: Fill the Objects you would like to target");
//     SENDTRACE_1( "Task %d deleted" , m_iShownTask ) ;
//     m_Template = temp ;
//     //     return;
//   }
//   else
//   {
//     nTaskPos = 0;	//the position of the first "task" string in m_Template
//     nBracket = 0;	//the position of the first ")" after the task.
//     nTaskPos = (int) m_Template.Find("task");
//     if (nTaskPos != -1)		//a task was written
//     {
//       //Delete it
//       nBracket = (int) m_Template.Find(")",nTaskPos);
//       m_Template.Truncate(nTaskPos);
//     }
//     //Write new task
//     m_Template = temp ;
//     FXString sText;
//     FXString sRow;
//     if (m_Template.IsEmpty() == FALSE)
//       sRow.Append("\r\n");
//     sRow.Append("task(");
//     sText.Format("%d",m_iShownTask);
//     sRow.Append(sText);
//     sRow.Append(",");
//     sRow.Append(m_sObjNames);
//     sRow.Append("),");
//     m_Template.Append(sRow);
//     m_iActiveTask = m_iShownTask ;
//   }
//   UpdateData(FALSE);
}

void TVObjectsGadgetSetupDialog::OnBnClickedTrackobj()
{
//   //Remember absolute position before taking it from the selection on the renderer.
//   UpdateData(TRUE);
//   int currAbsX = m_AOS.left;
//   int currAbsY = m_AOS.top;
//   int currAbsWidth = m_AOS.right;
//   int currAbsLen = m_AOS.bottom;	
// 
//   SaveDialog( &m_SavedSetupData ) ;
//   if(UpdatePosFromRendererSignleClick())
//   {
//     //Remember all the data before the search
//     FXString currObjectsLine( m_Template );    // Save current template
//     TCHAR Buf[100] ;
//     m_comboType.GetWindowText(Buf , 99);			//save the current type
//     FXString currType(Buf);
//     FXString currName( m_ObjectName );           //save the current object name
//     m_comboContrast.GetWindowText(Buf , 99);	//save the current contrast
//     FXString currContrast(Buf);
//     m_comboOrient.GetWindowText(Buf , 99);	//save the current orientation
//     FXString currOrientation(Buf);
//     FXString currObjectsNames( m_sObjNames ); // same current object names for task
//     BOOL currMultiple = m_bMulti;						//save the current multiple check
//     int  currMinRadMinWid = m_ExpectedSize.left;
//     int  currMaxRadMinLen = m_ExpectedSize.top;
//     int  currMaxWid = m_ExpectedSize.right;
//     int  currMaxHeight = m_ExpectedSize.bottom;
//     int  currTaskNum = m_iActiveTask;
// 
//     BOOL objectFound = FALSE;
//     //Filling possible object:
// 
//     m_ObjectName = "OBJ";			//Select temporary name for the object
//     m_bMulti = FALSE;				//UnSelect Multiple
//     m_bDetailed = TRUE;
//     m_WhatToMeasure = 2 ;
//     m_iShownTask = m_nTaskNumberEntering;
// 
//     CRect SelectedArea = CRect( m_PtSelectStart , m_PtSelectEnd ) ;
//     SelectedArea.NormalizeRect() ;
//     if ( SelectedArea.Width() < 5 )
//     {
//       m_ExpectedSize.left = 5;
//       m_ExpectedSize.top = 5;
//       m_ExpectedSize.bottom = 50;
//       m_ExpectedSize.right = 50;
//     }
//     else
//     {
//       m_ExpectedSize.left = 5;
//       m_ExpectedSize.top = 5;
//       m_ExpectedSize.bottom = (SelectedArea.Height() * 2) / 3 ;
//       m_ExpectedSize.right = (SelectedArea.Width() * 2 ) / 3 ;
//     }
// 
//     int iCurrentType =  m_comboType.GetCurSel() ;
//     m_ObjectName = "obj";	//Select the name of the object that will be searched for.
//     m_comboType.GetWindowText(Buf , 99);
//     FXString sType(Buf) ;
//     if ( sType == "ocr" )
//     {
//       FXString sOrient ;
//       GetFXStringFromCombo( m_comboOrient , sOrient );
//       if ( m_AOS.right > m_AOS.bottom )
//       {
//         if ( sOrient != "3"  &&  sOrient != "9" )
//           m_comboOrient.SetCurSel( 1 ) ;  // set to 3
//       }
//       else
//       {
//         if ( sOrient != "0"  &&  sOrient != "6" &&  sOrient != "12")
//           m_comboOrient.SetCurSel( 0 ) ;  // set to 12
//       }
//     }
// 
//     if ( !AddingObject( m_ObjectName ) )
//       return ;
//     bool bNewTaskCreated = false ;
//     if ( (m_iActiveTask < 0)  ||  !AddNewObjectToTask( m_ObjectName , m_iActiveTask ))
//     {
//       m_iActiveTask = CreateNewTaskWithObject( m_ObjectName ) ;
//       bNewTaskCreated = true ;
//     }
//     m_SelectedOnRenderObjName = m_ObjectName ;
// 
//     //Applying:
//     UploadParams(m_pGadget);
//     int nCounter = 0;
//     int nNoResponse = 0;
//     m_nAutoFind = 0;				//Initial position.
//     while(m_nAutoFind == 0)			//wait until an object is found\not found.
//     {
//       Sleep(50);
//       nCounter++;
//       if (nCounter == 50)
//         break;
//     }
// 
//     if (m_nAutoFind == 1)		
//       objectFound = TRUE;			//object is found
// 
//     //Returning the object line to how it looked before the automatic search.
//     //     m_Template.Empty();
//     //     m_Template.Append(currObjectsLine);
//     if (objectFound == TRUE)			//Automatic search succeeded
//     {
//       m_comboType.GetWindowText(Buf , 99);
//       FXString sTemp(Buf) ;
//       FXString NewObjName ;
//       FXString TempName( m_ObjectName ) ;
//       NewObjName.Format("%s_%d_%d",sTemp,m_AOS.left,m_AOS.top) ; 
//       //       DeleteObjWithTypeAndNameMatching( sTemp , m_ObjectName ) ;
//       RenameObj( m_ObjectName , sTemp , NewObjName ) ;
//       m_ObjectName = NewObjName ;
// 
//       //       if ( currTaskNum < 0 )
//       //       {
//       m_iShownTask = m_iActiveTask ;
// 
//       SetDlgItemInt( IDC_EDITTASKNUM , m_iShownTask ) ;
//       //       }
//       //       else
//       //       {
//       m_ObjectName = NewObjName ;
//       SetDlgItemText( IDC_EDITNAME , m_ObjectName ) ;
//       SetDlgItemText( IDC_EDITOBJNAMES , _T("") ) ;
//       AddNewObjectToTask( m_ObjectName , m_iActiveTask ) ;
//       DeleteObjectFromTask( TempName , m_iActiveTask ) ;
//       //       }
//       //       ClickingTaskButtonResponse();
//       UploadParams(m_pGadget);
//       //       SetDefaultInitialization();
//     }
//     else					//Automatic search failed
//     {
//       SENDERR_0("Automatic search failed");
//       m_Template = currObjectsLine;
//     }
//     //Returning the Dialog to how it looked before the automatic search.
//     m_ObjectName.Empty();
//     m_ObjectName.Append(currName);			//Restore the name
//     //Restore the comboBoxes positions:
//     if (currType.CompareNoCase("spot") == 0)
//     {
//       m_comboType.SetCurSel(0);			//Select Spot
//     }
//     else //if (currType.CompareNoCase("line") == 0)
//     {	
//       m_comboType.SetCurSel(1);			//Select Line
//     }	
// 
//     m_bMulti = currMultiple;						
//     m_AOS.left = currAbsX;
//     m_AOS.top = currAbsY;
//     m_AOS.right = currAbsWidth;
//     m_AOS.bottom = currAbsLen;
//     m_ExpectedSize.left = currMinRadMinWid;
//     m_ExpectedSize.top = currMaxRadMinLen;
//     m_ExpectedSize.right = currMaxWid;
//     m_ExpectedSize.bottom = currMaxHeight;
//     //m_sObjNames.Empty();
//     m_sObjNames = currObjectsNames;
//     m_iShownTask = currTaskNum;
//     RestoreDialog( &m_SavedSetupData ) ;
//     UploadParams(m_pGadget);
//   }
}


bool TVObjectsGadgetSetupDialog::UpdatePosFromRendererSignleClick(void)
{
  if ( m_PtSelectStart.x < 0 )
  {
    //     FxMessageBox("Error : Position wasn't Selected in the Renderer");
    m_sSelectRendererStart.Empty();
    m_sSelectRendererEnd.Empty();
    m_PtSelectStart = m_PtSelectEnd = CPoint(-1,-1) ;
    return FALSE;
  }
  int nTempHeight = 60;
  int nTempWidth = 60;

  if ((m_AOS.right != 0) && (m_AOS.bottom != 0))
  {
    nTempHeight = m_AOS.bottom;
    nTempWidth = m_AOS.right;
  }


  if ( m_bAdjustCoordinates == 2 
    && m_PtSelectStart.x != m_PtSelectEnd.x 
    && m_PtSelectStart.y != m_PtSelectEnd.y )
  {
    m_AOS.left = m_PtSelectStart.x ;
    m_AOS.top = m_PtSelectStart.y ;
    m_AOS.right = m_PtSelectEnd.x - m_PtSelectStart.x ;
    m_AOS.bottom = m_PtSelectEnd.y - m_PtSelectStart.y ;
  }
  else
  {
    m_AOS.left = m_PtSelectEnd.x - (nTempWidth/2) ;
    if ( m_AOS.left < 1 )
      m_AOS.left = 1 ;
    m_AOS.top = m_PtSelectEnd.y - (nTempHeight/2) ; 
    if ( m_AOS.top < 1 )
      m_AOS.top = 1 ;
    m_AOS.bottom = nTempHeight ;
    m_AOS.right = nTempWidth ;
  }
  UpdateData(FALSE);
  m_sSelectRendererStart.Empty();
  m_sSelectRendererEnd.Empty();
  m_PtSelectStart.x = -1 ; 
  return TRUE;
}

void TVObjectsGadgetSetupDialog::UpdateTaskFromObjects(void)
{
//   FXString sNewTemplate( m_Template ) ;
//   m_sObjNames.Empty();
//   int nTaskPos;
//   int nBracket;
//   int nCommaPos;
//   nTaskPos = (int) sNewTemplate.Find("task");
//   if (nTaskPos != -1)
//   {
//     nCommaPos = (int) sNewTemplate.Find(",",nTaskPos);
//     nBracket = (int) sNewTemplate.Find(")",nTaskPos);
//     m_sObjNames = sNewTemplate.Mid(nCommaPos+1,nBracket-nCommaPos-1); //save the current objects and tasks.
//   }
}


void TVObjectsGadgetSetupDialog::OnTimer(UINT_PTR nIDEvent)
{
  if ( nIDEvent == ID_TIMER )
  {
    m_buttEdit.EnableWindow(FALSE);
    KillTimer( ID_TIMER );
  }
  CGadgetSetupDialog::OnTimer(nIDEvent);
}

// Start the timers.
void TVObjectsGadgetSetupDialog::StartTimer()
{
  // Set timer.
  SetTimer( ID_TIMER, 500, 0 );
}

void TVObjectsGadgetSetupDialog::StopTimer()
{
  // Stop timer.
  KillTimer( ID_TIMER );
}

void TVObjectsGadgetSetupDialog::OnBnClickedCheckdisproi()
{
  m_bDispROI = GetCheckBoxState( IDC_CHECKDISPROI ) ;
  if ( m_pViewMode ) 
    m_pViewMode->setbDispROI( m_bDispROI );
}

void TVObjectsGadgetSetupDialog::OnBnClickedCheckdispcoor()
{
  m_bDispCoor = GetCheckBoxState( IDC_CHECKDISPCOOR ) ;
  if ( m_pViewMode ) 
    m_pViewMode->setbDispCoor( m_bDispCoor );
}

void TVObjectsGadgetSetupDialog::OnBnClickedCheckdispprofx()
{
  m_bDispProfX = GetCheckBoxState( IDC_CHECKDISPPROFX ) ;
  if ( m_pViewMode )
    m_pViewMode->setbDispCoor( m_bDispProfX );
}

void TVObjectsGadgetSetupDialog::OnBnClickedCheckdispprofy()
{
  m_bDispProfY = GetCheckBoxState( IDC_CHECKDISPPROFY ) ;
  if ( m_pViewMode ) 
    m_pViewMode->setbDispProfY( m_bDispProfY  );
}

void TVObjectsGadgetSetupDialog::OnBnClickedCheckdisppos()
{
  m_bDispPos = GetCheckBoxState( IDC_CHECKDISPPOS ) ;
  if ( m_pViewMode )
    m_pViewMode->setbDispPos( m_bDispPos );
}

void TVObjectsGadgetSetupDialog::OnBnClickedCheckdispdetail()
{
  m_bDispDetails = GetCheckBoxState( IDC_CHECKDISPDETAIL ) ;
  if ( m_pViewMode ) 
    m_pViewMode->setbDispDetails( m_bDispDetails );
}

void TVObjectsGadgetSetupDialog::OnBnClickedCheckdispmgraphics()
{
  m_bDispMGraphics = GetCheckBoxState( IDC_CHECKDISPMGRAPHICS ) ;
  if ( m_pViewMode ) 
    m_pViewMode->setbDispMGraphics( m_bDispMGraphics );
}



int TVObjectsGadgetSetupDialog::GetGadgetViewMode()
{
  OnBnClickedCheckdispmgraphics() ;
  OnBnClickedCheckdispdetail() ;
  OnBnClickedCheckdisppos() ;
  OnBnClickedCheckdispprofx() ;
  OnBnClickedCheckdispprofy() ;
  OnBnClickedCheckdisproi() ;
  OnBnClickedCheckdispcoor() ;
  m_bViewAngle = GetCheckBoxState( IDC_CHECKDISPANGLE ) ;
  m_bViewDia = GetCheckBoxState( IDC_CHECKDIAMETERS ) ;
  m_bDispWeighted = GetCheckBoxState( IDC_CHECKDISPWEIGHTED ) ;
  m_bViewObjectContur = GetCheckBoxState( IDC_SHOW_CONTUR ) ;
  m_bViewCoordsScaled = GetCheckBoxState( IDC_SHOW_SCALED_COORDS ) ;
  int iViewMode = (m_bDispROI * OBJ_VIEW_ROI)
    | (m_bDispPos * OBJ_VIEW_POS)
    | (m_bDispCoor * OBJ_VIEW_COORD)
    | (m_bDispDetails * (OBJ_VIEW_DET + OBJ_VIEW_DIFFR) )
    | (m_bDispProfX * OBJ_VIEW_PROFX)
    | (m_bDispProfY * OBJ_VIEW_PROFY)
    | (m_bDispMGraphics * OBJ_VIEW_MRECTS)
    | (m_bViewCoordsScaled * OBJ_VIEW_SCALED)
    | (m_bViewObjectContur * OBJ_VIEW_CONT)
    | (m_bViewAngle * OBJ_VIEW_ANGLE)
    | (m_bViewDia * OBJ_VIEW_DIA)
    | (m_bDispWeighted * OBJ_WEIGHTED) ;
  if ( m_pViewMode )
    m_pViewMode->SetViewMode( iViewMode ) ;

  return iViewMode ;

//   if ( m_pViewMode )
//     return m_pViewMode->GetViewMode() ;
//   int iViewMode = (m_bDispROI * OBJ_VIEW_ROI)  
//     | (m_bDispPos * OBJ_VIEW_POS) 
//     | ( m_bDispCoor * OBJ_VIEW_COORD) 
//     | ( m_bDispDetails * OBJ_VIEW_DET) 
//     | ( m_bDispProfX * OBJ_VIEW_PROFX ) 
//     | ( m_bDispProfY * OBJ_VIEW_PROFY )  
//     | ( m_bDispMGraphics * OBJ_VIEW_DIFFR) 
//     | ( m_bViewCoordsScaled * OBJ_VIEW_SCALED ) 
//     | ( m_bViewObjectContur * OBJ_VIEW_CONT ) 
//     | ( m_bViewAngle * OBJ_VIEW_ANGLE ) 
//     | ( m_bViewDia * OBJ_VIEW_DIA ) 
//     | ( m_bDispWeighted * OBJ_WEIGHTED ) ;
//   return iViewMode ;
}


int TVObjectsGadgetSetupDialog::DeleteObjWithTypeAndNameMatching(
  FXString& ObjType , FXString& ObjName )
{
  FXParser parser(m_Template), temp;
  FXString     key;
  int i=0;
  FXPropertyKit tempRow;


  while (parser.GetElementNo(i,key,temp))
  {
    if ( key.CompareNoCase( ObjType ) == 0 )
    {
      tempRow = temp;
      FXString sTempObjNames ;
      tempRow.GetString("name", sTempObjNames);
      if (sTempObjNames.CompareNoCase(ObjName) == 0)
      {
        int iContentPos = (int) m_Template.Find( temp ) ;
        int iElementPos = iContentPos - (int) key.GetLength() - 4 ;
        if ( iElementPos < 0 )
          iElementPos = 0 ;
        int iTypePos = (int) m_Template.Find( key , iElementPos );
        int iStartPos = iTypePos ;
        int iEndPos = (int) m_Template.Find(_T('\n') , iStartPos + (int) temp.GetLength());
        if ( iEndPos < 0 )
          iEndPos = iStartPos + (int) temp.GetLength() ;
        if ( iStartPos == 0)	//if it is the first object
          m_Template.Delete( iStartPos , iEndPos -iStartPos+1);			//Deleting the old object to replace him with the new.
        else
          m_Template.Delete( iStartPos, iEndPos - iStartPos);			//Deleting the old object to replace him with the new.
        return iStartPos ;
      }
    }
    i++;
  }

  return -1;
}

int TVObjectsGadgetSetupDialog::RenameObj( FXString& OldName , 
   FXString& ObjType , FXString& NewObjectName )
{
  FXString sTempObjNames ;
  FXParser parser( (LPCTSTR) m_Template) ;
  FXParser2  temp;
  FXString NewTemplate ;
  FXString     key;
  int i=0;
  FXPropertyKit tempRow;
  int iBegin = 0 , iEnd = 0 ;
  int iReplaced = 0 ;

  while ( iBegin < parser.GetLength()
    && parser.GetElementNo(i,key,temp) )
  {
    if ( key.CompareNoCase( ObjType ) == 0 )
    {
      tempRow = temp;
      tempRow.Remove( _T(' ') ) ;
      tempRow.Remove( _T('\t') ) ;
      tempRow.GetString("name", sTempObjNames);
      if (sTempObjNames.CompareNoCase(OldName) == 0)
      {
        tempRow.WriteString( _T("name") , NewObjectName ) ;
//         tempRow.DeleteKey( "name" ) ;
//         FXString Name( "name=" ) ;
//         tempRow = Name + (NewObjectName + ';') + (LPCTSTR)tempRow ;
        iReplaced++ ;
        temp = (LPCTSTR)tempRow ;
      }
    }
    else if ( key.CompareNoCase( "task" ) == 0 )
    {
      int iPos = 0 ;
      temp.FindAndReplaceWord( (LPCTSTR)OldName , (LPCTSTR)NewObjectName , iPos ) ;
      iReplaced++ ;
      if ( atoi(temp) == m_iActiveTask )
      {
        int iCommaPos = (int) temp.Find(',') ;
        m_sObjNames = temp.Mid( iCommaPos + 1) ;
        SetDlgItemText( IDC_EDITOBJNAMES , temp.Mid( iCommaPos + 1) ) ;
      }
    }
    key += '(' ;
    NewTemplate += (key + temp) + "),\r\n" ;
    iBegin = iEnd + 1 ;
    while ( strchr( pSeparators , parser[iBegin] )  &&  iBegin < parser.GetLength() )
      iBegin++ ;
    i++;
  }
  m_Template = NewTemplate ;

  return iReplaced ;
}

void TVObjectsGadgetSetupDialog::OnBnClickedCatchObject()
{
  if ( m_pViewMode ) m_pViewMode->setbCatchObject( 
    ((CButton*)GetDlgItem( IDC_MARK_CENTER))->GetCheck() == BST_CHECKED ) ;
  if ( m_pViewMode ) m_pViewMode->setbFindObject( 
    ((CButton*)GetDlgItem( IDC_FIND_OBJECT))->GetCheck() == BST_CHECKED ) ;
  if ( m_pViewMode ) m_pViewMode->setbFindObjectInArea( 
    ((CButton*)GetDlgItem( IDC_MARK_AREA_ON_RENDER))->GetCheck() == BST_CHECKED ) ;
}

void TVObjectsGadgetSetupDialog::OnBnClickedDeleteObject()
{
  FXString ObjName ;
  GetFXStringFromDlgItem( m_hWnd , IDC_EDITNAME , ObjName ) ;
  
  GetSelectedType() ; // for m_SelectedObjectType set
  if ( !ObjName.IsEmpty() && m_pGetObjectsAndTasksFunction )
  {
    VOArray * pObjects = NULL ;
    CVOJobList * pTasks = NULL ;
    int iActiveTask = -1 ;
    m_pGetObjectsAndTasksFunction( m_pGadget , 
      (void**) &pObjects , (void**) &pTasks , &iActiveTask , NULL ) ;
    if ( pObjects && pTasks )
    {
      for ( int i = 0 ; i < pObjects->GetCount() ; i++ )
      {
        if ( pObjects->GetAt(i).m_ObjectName == ObjName )
        {
          if ( pObjects->GetAt( i ).m_Type == m_SelectedObjectType )
          {
            pObjects->RemoveAt( i ) ;
            POSITION pos = pTasks->GetStartPosition();
            while ( pos )
            {
              WORD rKey;
              void* rValue;

              pTasks->GetNextAssoc( pos , rKey , rValue );
              CVOJob* job = (CVOJob*) rValue;
              for ( i = 0; i <= job->GetUpperBound(); i++ )
              {
                if ( (*job)[ i ].m_ObjectName == ObjName )
                {
                  job->RemoveAt( i ) ;
                }
              }
            }
            Link( pObjects , *pTasks ) ;
            UpdateObjectsAndTasksLists( pObjects , pTasks ) ;

            FXPropKit2 pk ;
            pk.WriteInt( "ViewMode" , GetGadgetViewMode() ) ;
            pk.WriteInt( "ActiveTask" , iActiveTask ) ;

            bool bInvalidate = false ;
            m_pGadget->ScanProperties( pk , bInvalidate ) ;

          }
        }
      }
    }
    m_pGetObjectsAndTasksFunction( m_pGadget , NULL , NULL , NULL , NULL ) ;
  }
}

void TVObjectsGadgetSetupDialog::OnBnClickedOk()
{
  OnApply() ;
  if ( m_pOldTemplateFont )
  {
    CFont * pUsedFont = m_editTemplate.GetFont() ;
    m_editTemplate.SetFont( m_pOldTemplateFont ) ;
    m_pOldTemplateFont = NULL ;
    delete pUsedFont ;
  }
  CDialog::OnOK() ;
}

void TVObjectsGadgetSetupDialog::OnBnClickedCancel()
{
  if ( m_pOldTemplateFont )
  {
    CFont * pUsedFont = m_editTemplate.GetFont() ;
    m_editTemplate.SetFont( m_pOldTemplateFont ) ;
    m_pOldTemplateFont = NULL ;
    delete pUsedFont ;
  }
  CDialog::OnCancel() ;
}

void TVObjectsGadgetSetupDialog::OnClose()
{
  if ( m_pOldTemplateFont )
  {
    CFont * pUsedFont = m_editTemplate.GetFont() ;
    m_editTemplate.SetFont( m_pOldTemplateFont ) ;
    m_pOldTemplateFont = NULL ;
    delete pUsedFont ;
  }
  CGadgetSetupDialog::OnClose();
}

void TVObjectsGadgetSetupDialog:: UpdateData( BOOL bSaveAndValidate )
{
  if ( !bSaveAndValidate )
  {
    ( ( ( CButton* )GetDlgItem( IDC_BINARY_OUT ) )->SetState( m_bBinaryOutput ) ) ;

  }
  CDialog::UpdateData( bSaveAndValidate ) ;
  if ( bSaveAndValidate )
  {
    m_bBinaryOutput = (((CButton*) GetDlgItem( IDC_BINARY_OUT ))->GetState() == BST_CHECKED) ;
    m_WhatToMeasure = GetGadgetViewMode() ;
  }
}

void TVObjectsGadgetSetupDialog::OnBnClickedSetActiveTask()
{
  m_iActiveTask = GetDlgItemInt( IDC_EDITTASKNUM ) ;
  SetDlgItemInt( IDC_ACTIVE_TASK , m_iActiveTask ) ;
  FXPropertyKit pk;
  if ( !pk.WriteInt( "ActiveTask" , m_iActiveTask ) )
    m_iActiveTask = -1 ;
  m_iShownActiveTask = m_iActiveTask ;

  pk.WriteInt( "ViewMode" , GetGadgetViewMode() ) ;

  bool bInvalidate = true ;
  m_Gadget->ScanProperties( pk , bInvalidate ) ;
}


bool TVObjectsGadgetSetupDialog::SaveDialog(SavedSetupDialog * pSave)
{
  UpdateData( TRUE ) ;
  return true ;
}
bool TVObjectsGadgetSetupDialog::RestoreDialog(SavedSetupDialog * pRestore)
{

  return true ;
}

void TVObjectsGadgetSetupDialog::UpdateViewCheckBoxes()
{
  ((CButton*)GetDlgItem( IDC_CHECKDISPROI))->SetCheck( m_bDispROI ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_CHECKDISPPOS))->SetCheck( m_bDispPos ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_CHECKDISPCOOR))->SetCheck( m_bDispCoor ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_CHECKDISPDETAIL))->SetCheck( m_bDispDetails ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_CHECKDISPPROFX))->SetCheck( m_bDispProfX ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_CHECKDISPPROFY))->SetCheck( m_bDispProfY ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_CHECKDISPMGRAPHICS))->SetCheck( m_bDispMGraphics ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_SHOW_SCALED_COORDS))->SetCheck( m_bViewCoordsScaled ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_SHOW_CONTUR))->SetCheck( m_bViewObjectContur ? BST_CHECKED :BST_UNCHECKED ) ;
  //((CButton*)GetDlgItem( IDC_DONT_TOUCH_EDGE))->SetCheck( m_bDontTouchEdge ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_CHECKDISPWEIGHTED))->SetCheck( m_bDispWeighted ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_CHECKDISPANGLE))->SetCheck(
    (m_bMeasAngleCW | m_bMeasAngleCNW | m_bDispWeighted ) ? BST_CHECKED :BST_UNCHECKED ) ;
  ((CButton*)GetDlgItem( IDC_CHECKDIAMETERS))->SetCheck(
    (m_bMeasureDiameters) ? BST_CHECKED :BST_UNCHECKED ) ;
}

void TVObjectsGadgetSetupDialog::UpdateBoolFlags( 
  DWORD dwProcessMode , DWORD dwViewMode )
{
  m_bDispROI = (dwViewMode & OBJ_VIEW_ROI) != 0 ;
  m_bDispPos = (dwViewMode & OBJ_VIEW_POS) != 0 ;
  m_bDispCoor = (dwViewMode & OBJ_VIEW_COORD) != 0 ;
  m_bDispDetails = (dwProcessMode & ( MEASURE_DIFFRACT | MEASURE_RATIO )) != 0 ;
  m_bDispProfX = (dwProcessMode & MEASURE_PROFILE) && ( dwViewMode & OBJ_VIEW_PROFX ) ;
  m_bDispProfY = (dwProcessMode & MEASURE_PROFILE) && ( dwViewMode & OBJ_VIEW_PROFY ) ;
  m_bDispMGraphics = (dwViewMode & OBJ_VIEW_MRECTS) != 0 ;
  m_bMeasAngleCNW = (dwProcessMode & MEASURE_IMG_MOMENTS_W) != 0 ;
  m_bMeasAngleCW = (dwProcessMode & MEASURE_IMG_MOMENTS_NW) != 0 ;
  //m_bDontTouchEdge ;
  m_bDispWeighted = (dwProcessMode & MEASURE_IMG_MOMENTS_W) != 0 ;
  m_bMeasureAngle = (dwProcessMode & ( MEASURE_IMG_MOMENTS_W | MEASURE_IMG_MOMENTS_NW )) != 0 ;
  m_bMeasureDiameters = (dwProcessMode & MEASURE_DIAMETERS) != 0 ;
  m_bViewCoordsScaled = (dwViewMode & OBJ_VIEW_SCALED) != 0 ;
  m_bViewObjectContur = (dwProcessMode & MEASURE_CONTUR) && (dwViewMode & OBJ_VIEW_CONT) ;
  m_bViewAngle = (dwProcessMode & (MEASURE_IMG_MOMENTS_W | MEASURE_IMG_MOMENTS_NW))
    && ( dwViewMode & OBJ_VIEW_ANGLE ) ;
  m_bViewDia = (dwProcessMode & MEASURE_DIAMETERS) && (dwViewMode & OBJ_VIEW_DIA) ;

  UpdateViewCheckBoxes() ;
}


void TVObjectsGadgetSetupDialog::OnBnClickedDontTouchEdge()
{
  m_bDontTouchEdge = ((CButton*)GetDlgItem( IDC_DONT_TOUCH_EDGE))
    ->GetCheck() == BST_CHECKED ; 
}

int TVObjectsGadgetSetupDialog::GetObjectsList( FXStringArray& Result )
{
  if ( m_pGetDataFunction )
  {
    VOArray * pAllObjects = NULL ;
    // Get all objects array and lock it
    m_pGetDataFunction( m_pGadget , (void**) &pAllObjects ) ;
    // Unlock all objects array
    if ( pAllObjects && pAllObjects->GetCount() )
    {
      Result.RemoveAll() ;
      for ( int i = 0 ; i < pAllObjects->GetCount() ; i++ )
      {
        CVideoObject& Obj = pAllObjects->GetAt( i ) ;
        Result.Add( Obj.m_ObjectName ) ;
      }
    }
    m_pGetDataFunction( m_pGadget , NULL ) ;
    return (int) Result.GetCount() ;
  }
  return 0 ;
}

void TVObjectsGadgetSetupDialog::OnBnClickedFormMask()
{
  UpdateData(TRUE);
  BOOL bAreDiffractRadiuses = ( m_DiffrRadius.cx != 0 ) && ( m_DiffrRadius.cy != 0 ) ;

  m_bMeasAngleCW = m_bMeasureAngle && m_bDispWeighted ;
  m_bMeasAngleCNW = m_bMeasureAngle && !m_bDispWeighted ;
  GetSelectedType() ;
  switch( m_SelectedObjectType )
  {
  default:
  case SPOT: m_WhatToMeasure |= MEASURE_AREA 
    | (m_bDispDetails * (((m_bDispWeighted) ? MEASURE_IMG_MOMENTS_W : MEASURE_IMG_MOMENTS_NW)
    | (bAreDiffractRadiuses * (MEASURE_DIFFRACT | MEASURE_SUB_SIDE_BACKS))
    | MEASURE_RATIO))
    | ((m_bDispProfX || m_bDispProfY) * MEASURE_PROFILE)
    | ((m_bMeasureAngle = m_bViewAngle) * (MEASURE_ANGLE
    |   ((m_bDispWeighted) ? MEASURE_IMG_MOMENTS_W : MEASURE_IMG_MOMENTS_NW)
    )
    ); break ;
  case LINE_SEGMENT: m_WhatToMeasure |= MEASURE_THICKNESS ; break ;
  case OCR: m_WhatToMeasure |= MEASURE_TEXT ; break ;
  }
  
  m_Detailed.Format( _T("0x%x") , m_WhatToMeasure )  ;
  SetDlgItemText( IDC_EDIT_DETAILED, m_Detailed);
  UpdateViewCheckBoxes() ;
  OnApply() ;
}

void TVObjectsGadgetSetupDialog::OnBnClickedShowScaledCoords()
{
  if ( m_pViewMode ) m_pViewMode->setbViewCoordScaled(
    m_bViewCoordsScaled  = GetCheckBoxState( IDC_SHOW_SCALED_COORDS ) );
}

void TVObjectsGadgetSetupDialog::OnBnClickedShowContur()
{
  if ( m_pViewMode ) m_pViewMode->setbViewObjectContur(
    m_bViewObjectContur  = GetCheckBoxState( IDC_SHOW_CONTUR) );
}


void TVObjectsGadgetSetupDialog::OnBnClickedCheckdispangle()
{
  if ( m_pViewMode ) m_pViewMode->setbViewAngle(
    m_bViewAngle  = GetCheckBoxState( IDC_CHECKDISPANGLE ) );
}


void TVObjectsGadgetSetupDialog::OnBnClickedCheckdiameters()
{
  if ( m_pViewMode ) m_pViewMode->setbViewDia(
    m_bViewDia  = GetCheckBoxState( IDC_CHECKDIAMETERS ) );
}


void TVObjectsGadgetSetupDialog::OnBnClickedCheckdispweighted()
{
  if ( m_pViewMode ) m_pViewMode->setbWeighted(
    m_bDispWeighted  = GetCheckBoxState( IDC_CHECKDISPWEIGHTED ) );
}


void TVObjectsGadgetSetupDialog::OnBnClickedBinaryOut()
{
  UINT State = ( ( CButton* )GetDlgItem( IDC_BINARY_OUT ) )->GetState() ;
  m_bBinaryOutput = (( State & BST_CHECKED ) != 0) ;
}


void TVObjectsGadgetSetupDialog::OnLbnDblclkObjList()
{
  OnLbnSelchangeObjList();
  if ( !m_ObjectName.IsEmpty() )
  {
    if ( m_pGetDataFunction )
    {
      VOArray * pAllObjects = NULL ;
      // Get all objects array and lock it
      m_pGetDataFunction( m_pGadget , (void**) &pAllObjects ) ;
      // Unlock all objects array
      if ( pAllObjects && pAllObjects->GetCount() )
      {
        for ( int i = 0 ; i < pAllObjects->GetCount() ; i++ )
        {
          CVideoObject& Obj = pAllObjects->GetAt( i ) ;
          if ( Obj.IsObjectName( m_ObjectName ) )
          {
            CWnd * pMainWnd = AfxGetApp()->GetMainWnd() ;
            if ( pMainWnd )
            {
              CRect rc ;
              pMainWnd->GetWindowRect( &rc ) ;
              CPoint Pt = rc.TopLeft() ;
              Pt += CSize( 50 + i * 10 , 50 + i * 10 ) ;
              WPARAM wPar = (Pt.x & 0xffff) | ((Pt.y << 16) & 0xffff) ;
//               ::PostMessage( pMainWnd->m_hWnd , VM_TVDB400_SHOWVOSETUPDLG ,
//                 wPar , (LPARAM) &Obj ) ;
              ::PostMessage( this->m_hWnd , VM_TVDB400_SHOWVOSETUPDLG ,
                wPar , (LPARAM) &Obj ) ;
            }
            break ;
          }
        }
      }
      m_pGetDataFunction( m_pGadget , NULL ) ;
    }
  }
}

void TVObjectsGadgetSetupDialog::OnLbnDblclkTaskList()
{
  FXString Task ;
  if ( GetFXStringFromListBox( m_Tasks , Task ) )
  {
    int iPos = (int) Task.Find( _T( '(' ) ) ;
    if ( iPos > 0 && isdigit( Task[ iPos + 1 ] ) )
    {
      int iTaskNum = atoi( (LPCTSTR) Task + iPos + 1 ) ;
      iPos = (int) Task.Find( _T( ',' ) , iPos + 1 ) ;
      if ( iPos > 0 )
      {
        int iClosePos = (int) Task.Find( _T( ')' ) , iPos + 1 ) ;
        SetDlgItemInt( IDC_EDITTASKNUM , m_iShownTask = iTaskNum ) ;
        SetDlgItemText( IDC_EDITOBJNAMES ,
          Task.Mid( iPos + 1 , iClosePos - iPos - 1 ) ) ;
      }
    }
  }
}

static int iTraceObjAndTasks = 3 ;

void TVObjectsGadgetSetupDialog::UpdateObjectsAndTasksLists( 
  VOArray * pObjects , CVOJobList * pTasks )
{
  m_ObjectNames.ResetContent() ;
  if ( pObjects )
  {
    for ( int i = 0 ; i < pObjects->GetCount() ; i++ )
    {
      FXString NameAndType = ((pObjects->GetAt( i ).m_ObjectName
        + _T( "( " ))
        + VOTypeToVOName( pObjects->GetAt( i ).m_Type ))
        + _T( " )" ) ;

      m_ObjectNames.AddString( NameAndType ) ;
      if ( iTraceObjAndTasks & 1 )
        TRACE( "\n            Object %s" , (LPCTSTR) NameAndType ) ;
    }
  }

  m_Tasks.ResetContent() ;
  if ( pTasks )
  {
    FXStringArray Jobs ;
    pTasks->PrintJobs( Jobs ) ;
    for ( int i = 0 ; i < Jobs.GetCount() ; i++ )
    {
      m_Tasks.AddString( Jobs[ i ] ) ;
      if ( iTraceObjAndTasks & 2 )
        TRACE( "\n    Task %s" , (LPCTSTR) Jobs[ i ] ) ;
    }
  }
}

void TVObjectsGadgetSetupDialog::OnLbnSelchangeObjList()
{
  FXString ObjName ;
  if ( GetFXStringFromListBox( m_ObjectNames , ObjName ) )
  {
    FXSIZE Pos = 0 ;
    m_ObjectName = ObjName.Tokenize( " (\t" , Pos ) ;
    m_ObjectName.Trim( " ()\t" ) ;
    GetDlgItem( IDC_EDITNAME )->SetWindowTextA( m_ObjectName ) ;
    FXString TypeName = ObjName.Mid( Pos + 1 ) ;
    TypeName = TypeName.Trim( _T( " ()" ) ) ;
    VOBJ_TYPE Type = VONameToVOType( TypeName ) ;
    SetSelectedType( Type ) ;
    ObjName = m_ObjectName ;
    int iActiveTask = GetDlgItemInt( IDC_ACTIVE_TASK ) ;
    int iShownTask = GetDlgItemInt( IDC_EDITTASKNUM ) ;

    bool bControl = (GetAsyncKeyState( VK_CONTROL ) & 0x8000) != 0 ;
    bool bShift = (GetAsyncKeyState( VK_SHIFT ) & 0x8000) != 0 ;
    if ( bControl && !bShift ) // control pressed: add to active task, if exists
    {
//       for ( int i = 0 ; i < m_Tasks.GetCount() ; i++ )
//       {
//         FXString TaskAsString , InBrackets ;
//         if ( GetFXStringFromListBox( m_Tasks , TaskAsString , i ) 
//           && TaskAsString.GetSubStringInBrackets( InBrackets ) )
//         {
//           if ( isdigit( InBrackets[ 0 ] ) )
//           {
//             int iTaskNum = atoi( (LPCTSTR)InBrackets ) ;
//             if ( iActiveTask == iTaskNum )
//             {
//               if ( ObjectPosInListOfObjects( ObjName , InBrackets ) < 0 )
//               {
//                 InBrackets += _T( ',' ) ;
//                 InBrackets += ObjName ;
//                 FXString ObjectsForTask = InBrackets.Mid(
//                   InBrackets.Find( _T( ',' ) ) ) ;
//                 if ( m_pGetObjectsAndTasksFunction )
//                 {
//                   VOArray * pObjects = NULL ;
//                   CVOJobList * pTasks = NULL ;
//                   int iActiveTask = -1 ;
//                   m_pGetObjectsAndTasksFunction( m_pGadget ,
//                     (void**) &pObjects , (void**) &pTasks , &iActiveTask ) ;
//                   if ( pObjects && pTasks )
//                   {
//                     CVOJob Job ;
//                     if ( pTasks->Lookup( iActiveTask , Job ) )
//                       Job.RemoveAll() ;
// 
//                     int iPos = 0 ;
//                     while ( iPos >= 0 )
//                     {
//                       FXString ObjName = ObjectsForTask.Tokenize( _T( "," ) , iPos ) ;
//                       if ( !ObjName.IsEmpty() )
//                       {
//                         CVOTask NewTask = { ObjName , -1 } ;
//                         Job.Add( NewTask ) ;
//                       }
//                       else
//                       {
//                         if ( Job.GetCount() > 0 )
//                           pTasks->SetAt( iActiveTask , Job ) ;
//                         break ;
//                       }
//                     }
//                     Link( pObjects , *pTasks ) ;
// 
//                     if ( iShownTask == iActiveTask )
//                       AddObjectToShownList( ObjName ) ;
// 
//                     m_Tasks.ResetContent() ;
//                     FXStringArray Jobs ;
//                     pTasks->PrintJobs( Jobs ) ;
//                     for ( int i = 0 ; i < Jobs.GetCount() ; i++ )
//                       m_Tasks.AddString( Jobs[ i ] ) ;
// 
//                     FXPropKit2 pk ;
//                     pk.WriteInt( "ViewMode" , GetGadgetViewMode() ) ;
//                     if ( GetCheckBoxState( IDC_REINJECT_IMAGE ) )
//                       pk.WriteInt( "Reinject" , 1 ) ;
// 
//                     bool bInvalidate = false ;
//                     m_pGadget->ScanProperties( pk , bInvalidate ) ;
//                   }
//                   m_pGetObjectsAndTasksFunction( m_pGadget , NULL , NULL , NULL ) ;
//                 }
//               }
//             }
//           }
// 
//         }
//       }
//     }
//     else if ( bShift && !bControl ) // shift is pressed: add to objects of shown task
//     {
      if ( AddObjectToShownList( ObjName ) )
      {
        if ( m_iShownTask == m_iActiveTask )
        {
          OnBnClickedButtonaddtask() ;
          if ( GetCheckBoxState( IDC_REINJECT_IMAGE ) )
            OnBnClickedReinjectImage() ;
        }
      }
    }
    else if ( bShift && bControl )
    {
      RemoveObjectFromShownList( ObjName ) ;
      if ( m_iShownTask == m_iActiveTask )
      {
        OnBnClickedButtonaddtask() ;
        if ( GetCheckBoxState( IDC_REINJECT_IMAGE ) )
          OnBnClickedReinjectImage() ;
      }
    }
  }
}

bool TVObjectsGadgetSetupDialog::AddObjectToShownList( const FXString& ObjName )
{
  FXString Shown ;
  GetFXStringFromDlgItem( m_hWnd , IDC_EDITOBJNAMES , Shown ) ;
  Shown.Remove( _T( ' ' ) );
  Shown.Remove( _T( '\t' ) );
  if ( ObjectPosInListOfObjects( ObjName , Shown ) < 0 )
  {
    if ( !Shown.IsEmpty() )
      Shown += _T( ',' ) ;
    Shown += ObjName ;

    SetDlgItemText( IDC_EDITOBJNAMES , (LPCTSTR) Shown ) ;
    return true ;
  }
  return false ;
}

bool TVObjectsGadgetSetupDialog::RemoveObjectFromShownList( const FXString& ObjName )
{
  FXString Shown ;
  GetFXStringFromDlgItem( m_hWnd , IDC_EDITOBJNAMES , Shown ) ;
  Shown.Remove( _T( ' ' ) );
  Shown.Remove( _T( '\t' ) );
  int iObjectPosInList = ObjectPosInListOfObjects( ObjName , Shown ) ;
  if ( iObjectPosInList >= 0 )
  {
    Shown.Delete( iObjectPosInList , ObjName.GetLength() ) ;
    int iNewListLength = (int) Shown.GetLength() ;
    if ( iNewListLength > 0 )
    {
      if ( iNewListLength <= iObjectPosInList )
        Shown.Delete( iNewListLength - 1 ) ; // delete comma before deleted object
      else
        Shown.Delete( 0 ) ; // delete comma after deleted object 
    }
    SetDlgItemText( IDC_EDITOBJNAMES , (LPCTSTR) Shown ) ;
    return true ;
  }
  return false ;
}

int TVObjectsGadgetSetupDialog::ObjectPosInListOfObjects(
  const FXString& Obj , const FXString& List )
{
  int iPos = (int) List.Find( Obj ) ;
  if ( iPos >= 0 )
  {
    bool bPrevComma = (iPos == 0) || (List[ iPos - 1 ] == _T( ',' )) ;
    int iShownLen = (int) List.GetLength() ;
    bool bNextComma = (iPos + (int) Obj.GetLength() < iShownLen)
      || (List[ iPos + (int) Obj.GetLength() ] == _T( ',' )) ;
  }
  return iPos ;
}
BOOL TVObjectsGadgetSetupDialog::PreTranslateMessage( MSG* pMsg )
{
  m_ToolTip.RelayEvent( pMsg );

  return __super::PreTranslateMessage( pMsg );
}


void TVObjectsGadgetSetupDialog::OnLbnSelchangeTaskList()
{
  bool bControl = (GetAsyncKeyState( VK_CONTROL ) & 0x8000) != 0 ;
  bool bShift = (GetAsyncKeyState( VK_SHIFT ) & 0x8000) != 0 ;
  if ( bControl && bShift )
  {
    FXString Task ;
    if ( GetFXStringFromListBox( m_Tasks , Task ) )
    {
      int iPos = (int) Task.Find( _T( '(' ) ) ;
      if ( iPos > 0 && isdigit( Task[ iPos + 1 ] ) )
      {
        int iTaskNum = atoi( (LPCTSTR) Task + iPos + 1 ) ;
        if ( m_pGetObjectsAndTasksFunction )
        {
          VOArray * pObjects = NULL , *pNewObjects = new VOArray ;
          CVOJobList * pTasks = NULL ;
          CVOJobList NewTasks ;
          int iActiveTask = -1 ;
          m_pGetObjectsAndTasksFunction( m_pGadget ,
            (void**) &pObjects , (void**) &pTasks , &iActiveTask , NULL ) ;
          if ( pTasks )
          {
            pTasks->RemoveKey( (WORD) iTaskNum ) ;
          }
          UpdateObjectsAndTasksLists( pObjects , pTasks ) ;
          m_pGetObjectsAndTasksFunction( m_pGadget , NULL , NULL , NULL , NULL ) ; // release for normal work
          int iIndex = m_Tasks.GetCurSel() ;
          m_Tasks.DeleteString( iIndex ) ;
        }
      }
    }
  }
}

void TVObjectsGadgetSetupDialog::OnBnClickedReinjectImage()
{
  if ( m_pGadget )
  {
    bool bInvalidate = false ;
    m_pGadget->ScanProperties( _T( "Reinject=1;" ) , bInvalidate ) ;
  }
}

LRESULT TVObjectsGadgetSetupDialog::OnShowVOSetupDialog( WPARAM wParam , LPARAM lParam )
{
  CVideoObjectBase * pVO = (CVideoObjectBase*) lParam ;
  int x = (int) ((short int) (wParam & 0xffff)) ;
  int y = (int) ((short int) ((wParam >> 16) & 0xffff)) ;
  CPoint Pt( x , y ) ;
  Tvdb400_ShowObjectSetupDlg( pVO , Pt ) ;
  return 0;
}
