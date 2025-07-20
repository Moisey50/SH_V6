// stdsetup.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <gadgets\stdsetup.h>
#include <gadgets\gadbase.h>
#include <helpers\propertykitEx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern HINSTANCE thisInst;

static LPCTSTR pOpenParenth = _T( "(" ) ; // _T( "({[" ) ;
static LPCTSTR pCloseParenth = _T( ")" ) ; // _T( ")}]" ) ;
// Generic Gadget setup dialog

CGadgetSetupDialog::CGadgetSetupDialog( CGadget* pGadget , UINT idd , CWnd* pParent ) :
  CDialog( idd , pParent ) 
{
  m_pGadget = pGadget ;
}

CGadgetSetupDialog::~CGadgetSetupDialog()
{}

BOOL CGadgetSetupDialog::IsOn()
{
  return (::IsWindow( GetSafeHwnd() ) && IsWindowVisible());
}

void CGadgetSetupDialog::Delete()
{
  if ( GetSafeHwnd() )
    DestroyWindow();
  delete this;
}

void CGadgetSetupDialog::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CGadgetSetupDialog)
    // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CGadgetSetupDialog , CDialog )
  //{{AFX_MSG_MAP(CGadgetSetupDialog)
  //}}AFX_MSG_MAP
  //ON_BN_CLICKED( ID_RESTORE_INITIAL , &CGadgetSetupDialog::OnBnClickedRestoreInitial )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGadgetSetupDialog message handlers

void CGadgetSetupDialog::OnOK()
{
  UpdateData( TRUE );
  UploadParams();
  DestroyWindow();
}

void CGadgetSetupDialog::OnCancel()
{
  DestroyWindow();
}

void CGadgetSetupDialog::UploadParams()
{
  m_pGadget->Status().WriteBool( STATUS_MODIFIED , TRUE );
}


//////

#define THIS_MODULENAME "Tvdb400_ShowGadgetSetupDlg"
BOOL Tvdb400_ShowGadgetSetupDlg( IGraphbuilder* pBuilder , LPCTSTR uid , CPoint point )
{
  FXString suid( uid );
  FXString cGadget;
  BOOL retV = TRUE;
  FXSIZE pos;
  if ( (pos = suid.Find( '.' )) >= 0 )
  {
    cGadget = suid.Left( pos );
    IGraphbuilder* iGB = pBuilder->GetSubBuilder( cGadget );
    suid = suid.Mid( pos + 1 );
    return Tvdb400_ShowGadgetSetupDlg( iGB , suid , point );
  }
  else
  {
    FXString tmpS;
    CGadget* Gadget = pBuilder->GetGadget( uid );
    if ( (Gadget) && (Gadget->ScanSettings( tmpS )) )
    {
#ifdef _DEBUG
      int iNOpen = (int)tmpS.GetOneOfCnt( pOpenParenth ) ;
      int iNClose = (int)tmpS.GetOneOfCnt( pCloseParenth ) ;
      if ( iNOpen != iNClose )
      {
        FXString Error ;
        Error.Format( "Gadget %s ScanSettings Error: "
          "not equal numbers of open (%d) and close (%d) parenthesis" ,
          uid , iNOpen , iNClose ) ;
        SENDERR_1( _T( "%s" ) , (LPCTSTR)Error) ;
        return FALSE ;
      }
#endif
      FXString Name( uid ) ;
      Name = ((Name + _T( " (" )) 
        + (Gadget->GetRuntimeGadget()->m_lpszClassName)) + _T(')') ;
      if ( (tmpS.CompareNoCase( _T( "calldialog(true)" ) ) != 0) 
        && (Gadget->GetSetupObject() == NULL) )
      {
        CGadgetStdSetup* pStdSetupDlg = new CGadgetStdSetup( pBuilder , uid , NULL );
        Gadget->SetSetupObject( pStdSetupDlg );
      }
      if ( Gadget->GetSetupObject() )
        retV &= (Gadget->GetSetupObject()->Show( point , Name ) != false);
      else
        retV = FALSE;
      if ( !retV )
        SENDERR_1( _T( "Failed to create Setup Dialog for Gadget \"%s\"" ) , uid );
      return retV;
    }
    return FALSE;
  }
}
#undef THIS_MODULENAME

//////

#define THIS_MODULENAME "Tvdb400_ShowObjectSetupDlg"
BOOL Tvdb400_ShowObjectSetupDlg( CVideoObjectBase * pObject , CPoint point )
{
  FXString Settings , ObjectName ;
  BOOL retV = FALSE ;

  if ( (pObject) && (pObject->ScanSettings( Settings )) )
  {
    pObject->GetObjectName( ObjectName ) ;
#ifdef _DEBUG
    int iNOpen = (int) Settings.GetOneOfCnt( pOpenParenth ) ;
    int iNClose = (int) Settings.GetOneOfCnt( pCloseParenth ) ;
    if ( iNOpen != iNClose )
    {
      FXString Error ;
      Error.Format( "Video Object %s ScanSettings Error: "
        "not equal numbers of open (%d) and close (%d) parenthesis" ,
        (LPCTSTR) ObjectName , iNOpen , iNClose ) ;
      SENDERR_1( _T( "%s" ) , (LPCTSTR) Error ) ;
      return FALSE ;
    }
#endif
    if ( pObject->GetSetupObject() == NULL )
    {
      CObjectStdSetup* pStdSetupDlg = new CObjectStdSetup( pObject , NULL );
      pObject->SetSetupObject( pStdSetupDlg );
    }
    if ( pObject->GetSetupObject() )
      retV = (pObject->GetSetupObject()->Show( point , ObjectName ) != false);
  }
  if ( !retV )
    SENDERR_1( _T( "Failed to create Setup Dialog for video object \"%s\"" ) , (LPCTSTR) ObjectName );
  return retV;
}
#undef THIS_MODULENAME

static int CompareAscending( const void *a , const void *b )
{
  CString *pA = (CString*) a;
  CString *pB = (CString*) b;
  return (pA->CompareNoCase( *pB ));
}

static double Timing[ 20 ] ;

BOOL Tvdb400_RunSetupDialog( IGraphbuilder* pBuilder )
{
  double dStart = GetHRTickCount() ;
  CString gadgetData;
  //CGraphSettingsDialog gsd( pBuilder );

  CGraphSettingsDialog * pGSD = new CGraphSettingsDialog( pBuilder );
  ASSERT( pGSD ) ;
  ASSERT( AfxGetApp() != NULL );
  pBuilder->SetGraphSetupObject( pGSD ) ;

  CStringArray srcGadgets;
  CStringArray dstGadgets;
  pBuilder->EnumGadgets( srcGadgets , dstGadgets );
  srcGadgets.Append( dstGadgets ) ;
  Timing[ 0 ] = GetHRTickCount() - dStart ;
  qsort( (void*) &srcGadgets[ 0 ] , srcGadgets.GetCount() , 
    sizeof( CString* ) , CompareAscending ) ;
  Timing[ 1 ] = GetHRTickCount() - dStart - Timing[ 0 ] ;
  for ( INT_PTR i = 0; i < srcGadgets.GetSize(); i++ )
  {
    FXString settings;
    if ( pBuilder->ScanSettings( srcGadgets[ i ] , settings ) )
    {
      CString sItem; 
      sItem.Format( "settings(name(%s),%s)\n" , srcGadgets[ i ] , settings );
      gadgetData += sItem;
    }
  }
  Timing[ 2 ] = GetHRTickCount() - dStart - Timing[ 1 ] ;
  //for ( INT_PTR i = 0; i < dstGadgets.GetSize(); i++ )
  //{
  //  FXString settings;
  //  if ( pBuilder->ScanSettings( dstGadgets[ i ] , settings ) )
  //  {
  //    CString sItem; 
  //    sItem.Format( "settings(name(%s),%s)\n" , dstGadgets[ i ] , settings );
  //    gadgetData += sItem;
  //  }
  //}
  pGSD->SetData( gadgetData );
  pGSD->Create() ;
  FXString GraphName = pBuilder->GetID() ;
  CSetupObject * pSetupObject = pBuilder->GetGraphSetupObject() ;
  BOOL bRes = FALSE ;
  if ( pSetupObject )
    BOOL bRes = pSetupObject->Show( CPoint( 100 , 100 ) , GraphName ) ;
  //AfxSetResourceHandle(thisInst);
  Timing[ 3 ] = GetHRTickCount() - dStart - Timing[ 2 ] ;
  return bRes ;
}

BOOL Tvdb400_GetGadgetList( IGraphbuilder* pBuilder , FXString& GadgetList )
{
  CString gadgetData;
  //CGraphSettingsDialog gsd( pBuilder );

  ASSERT( AfxGetApp() != NULL );

  CStringArray srcGadgets;
  CStringArray dstGadgets;
  pBuilder->EnumGadgets( srcGadgets , dstGadgets );
  srcGadgets.Append( dstGadgets ) ;

  qsort( (void*) &srcGadgets[ 0 ] , srcGadgets.GetCount() , 
    sizeof( CString* ) , CompareAscending ) ;

  for ( INT_PTR i = 0; i < srcGadgets.GetCount() ; i++ )
  {
    CGadget* pGadget = pBuilder->GetGadget( srcGadgets[ i ] );
    if ( pGadget )
    {
      BOOL IsSetupObject = (pGadget->GetSetupObject() != NULL) ;
      GadgetList += (LPCTSTR) (
        (srcGadgets[ i ] + ((IsSetupObject) ? "(1)" : "(0)")) + _T( ';' )) ;

    }
  }
  return GadgetList.GetLength() != 0 ;
}

