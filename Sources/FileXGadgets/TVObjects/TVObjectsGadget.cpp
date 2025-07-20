// TVObjectsGadget.cpp: implementation of the TVObjectsGadget class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <fxfc\fxfc.h>
#include "helpers\FramesHelper.h"
#include <math\hbmath.h>
#include "TVObjects.h"
#include "TVObjectsGadget.h"
#include <Gadgets\TextFrame.h>
#include <imageproc\cut.h>
#include <helpers\fxparser2.h>
#include <helpers/propertykitEx.h>
#include "TVObjDataFrame.h"
#include <fxfc/FXRegistry.h>
#include <imageproc/ExtractVObjectResult.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "TVObjects"

IMPLEMENT_RUNTIME_GADGET_EX( TVObjects , CFilterGadget , "Video.recognition" , TVDB400_PLUGIN_NAME );
IMPLEMENT_RUNTIME_GADGET_EX( TVObjMeas , CFilterGadget , "Video.recognition" , TVDB400_PLUGIN_NAME );

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
  if (!(vfr) || ((vfr)->IsNullFrame()))	    \
{	                                        \
  fr->RELEASE(fr);						\
  return vfr;			                    \
}                                           \
}

const char * pSeparators = " \t\r\n()=;," ;

static BOOL bTraceObjects = false ;
static const UINT VM_TVDB400_SHOWVOSETUPDLG = ::RegisterWindowMessage(
  _T( "Tvdb400_ShowVOSetupDialog" ) );

using namespace tesseract;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CALLBACK OnScript( CDataFrame* lpData , void* lpParam , CConnector* lpInput )
{
  ( ( TVObjects* ) lpParam )->OnScript( lpData );
}

void CALLBACK OnControl( CDataFrame* lpData , void* lpParam , CConnector* lpInput )
{
  ( ( TVObjects* ) lpParam )->OnControl( lpData );
}

void CALLBACK OnCoordinates( CDataFrame* lpData , void* lpParam , CConnector* lpInput )
{
  ( ( TVObjects* ) lpParam )->OnCoordinates( lpData );
}

int GetObjArray( CGadget * pGadget , void ** ppVOArray )
{
  return ( ( TVObjects * ) pGadget )->GetVideoObjectsArray( ( VOArray** ) ppVOArray ) ;
}

int GetTasks( CGadget * pGadget , FXStringArray * TasksAsText )
{
  return ( ( TVObjects * ) pGadget )->GetTasksAsText( TasksAsText ) ;
}

int GetObjectsAndTaskList( CGadget * pGadget , void ** ppVOArray ,
  void** ppTasksList , int * piActiveTask , FXLockObject ** pLockVideoObjects )
{
  return ( ( TVObjects * ) pGadget )->GetObjectsAndTasks(
    ( VOArray** ) ppVOArray , ( CVOJobList** ) ppTasksList , piActiveTask , pLockVideoObjects )  ;
}


bool Link( VOArray * pObjects , CVOJobList& Jobs , int * pActiveTask )
{
  int i;
  for (i = 0; i <= pObjects->GetUpperBound(); i++)
  {
    CVideoObject& Obj = pObjects->GetAt( i ) ;
    if (!Obj.m_LeaderName.IsEmpty())
    {
      int pos = 0;
      while (pos <= pObjects->GetUpperBound())
      {
        if (Obj.m_LeaderName.MakeLower() == pObjects->GetAt( pos ).m_ObjectName.MakeLower() )
        { // X leader found
          Obj.m_LeaderIndex.x = pos ;  // fill X index
          if (Obj.m_LeaderNameY.IsEmpty())
          {  // No Y leader, fill Y with the same value
            Obj.m_LeaderIndex.y = pos ;
            break;
          }
          else
          {
            if (Obj.m_LeaderIndex.y > -1) // Y already filled, exit
              break ;
            else if (Obj.m_LeaderNameY.MakeLower() == pObjects->GetAt( pos ).m_ObjectName.MakeLower())
            {  // Y leader is the same
              Obj.m_LeaderIndex.y = pos ; // fill and exit
              break;
            }
          }
        }
        if (Obj.m_LeaderNameY.MakeLower() == pObjects->GetAt( pos ).m_ObjectName.MakeLower())
        {  // Y leader found
          Obj.m_LeaderIndex.y = pos ;
          if (Obj.m_LeaderIndex.x > -1)
            break ; // if X leader already filled, exit
        }

        pos++;
      }
      if (Obj.m_LeaderIndex.x == -1)
      {
        SENDERR_2( "Object '%s' can't be linked to object '%s', reason: object can't be found" , \
          Obj.m_ObjectName , Obj.m_LeaderName );
      }
      if (Obj.m_LeaderIndex.y == -1)
      {
        SENDERR_2( "Object '%s' can't be linked to object '%s', reason: object can't be found" , \
          Obj.m_ObjectName , Obj.m_LeaderNameY );
      }
    }
  }

  POSITION pos = Jobs.GetStartPosition();
  while (pos)
  {
    WORD rKey;
    void* rValue;

    Jobs.GetNextAssoc( pos , rKey , rValue );
    CVOJob* job = ( CVOJob* ) rValue;
    for (i = 0; i <= job->GetUpperBound(); i++)
    {
      ( *job )[ i ].m_ObjectID = -1 ;
      int j = 0;
      while (j <= pObjects->GetUpperBound())
      {
        if (( *job )[ i ].m_ObjectName.MakeLower() == pObjects->GetAt( j ).m_ObjectName.MakeLower())
        {
          ( *job )[ i ].m_ObjectID = j;
          break;
        }
        j++;
      }
      if (( *job )[ i ].m_ObjectID == -1)
        SENDERR_2( "Job '%d' can't be linked to object '%s', reason: object can't be found" , rKey , ( *job )[ i ].m_ObjectName );
    }
  }
  return true;
}


TVObjects::TVObjects() :
  m_CurrentJobID( -1 ) ,
  m_iCurrentAOSIndex( -1 ) ,
  m_CallOnStartFunc( 1 ) ,
  m_dScale_um_per_pix( 1.0 ) ,
  m_bExtModifications( false ) ,
  m_pTesseract( NULL ) ,
  m_iTesseractRetryDelay( 0 ) ,
  m_pLastDataFrame( NULL )
  //m_AddEOSToOutPutPackets(0)
{
  m_pInput = m_pInputs[ 0 ] = new CInputConnector( vframe );
  m_pInputs[ 1 ] = new CInputConnector( text , ::OnControl , this );
  m_pInputs[ 1 ]->SetName( "Control" );
  m_pInputs[ 2 ] = new CInputConnector( text , ::OnScript , this );
  m_pInputs[ 2 ]->SetName( "ScriptLoad" );
  m_pInputs[ 3 ] = new CInputConnector( text , ::OnCoordinates , this );
  //new CInputPinWithQueue( this , ::OnCoordinates );
  m_pInputs[ 3 ]->SetName( "Coordinates" );
  m_pOutput = new COutputConnector( transparent );
  m_pDiagOut = new COutputConnector( transparent );
  m_pDiagOut->SetName( "Diagnostics" ) ;
  //   m_pDescriptionOutput  =    new COutputConnector( text );
  m_SetupObject = new TVObjectsGadgetSetupDialog( this , NULL , GetObjArray , GetTasks , GetObjectsAndTaskList ) ;
  ( ( TVObjectsGadgetSetupDialog * ) m_SetupObject )->ConnectToGadgetViewMode( &m_ViewModeOption ) ;
  m_FormatErrorProcessed = false;
  m_LastFormat = 0;
  m_pObjects = m_pNewObjects = m_pOldObjects = NULL ;
  m_bNewObjectsUpdated = false ;
  m_iCaptionPeriod = 0 ;
  m_OutputMode = modeReplace ;
  m_iNObjectsMax = 500 ;
  m_dTimeout = 0. ;
  Resume();
}

int TVObjects::GetVideoObjectsArray( VOArray ** pDestination )
{
  if (pDestination)
  {
    if (m_Lock.Lock( 500 , "TVObjects::GetVideoObjectsArray" ))
    {
      if (*pDestination) // set object array
      {
        if (m_pNewObjects)
        {
          m_pNewObjects->RemoveAll() ;
          delete m_pNewObjects ;
        }
        m_pNewObjects = *pDestination ;
        int iNObjects = ( int ) m_pNewObjects->GetCount() ;
        m_bExtModifications = false ;
        m_Lock.Unlock() ;
        return iNObjects ;
      }
      else
      {
        m_bExtModifications = true ;
        if (m_pNewObjects)
        {
          if (m_pObjects)
          {
            m_pObjects->RemoveAll() ;
            m_pObjects = NULL ;
          }
          m_pObjects = m_pNewObjects ;
          m_pNewObjects = NULL ;
        }
      }
      *pDestination = m_pObjects ;
      if (*pDestination)
        return ( int ) ( ( *pDestination )->GetCount() ) ;
    }
  }
  else if (m_bExtModifications)
  {
    m_Lock.Unlock() ;
    m_bExtModifications = false ;
  }
  return 0 ;
}

int TVObjects::GetTasksAsText( FXString * pTasks )
{
  if (pTasks)
  {
    if (m_Lock.Lock( 500 , "TVObjects::GetTasksAsText 1" ))
    {
      m_bExtModifications = true ;
      m_Jobs.PrintJobs( *pTasks ) ;
      return ( int ) m_Jobs.GetCount() ;
    }
  }
  else if (m_bExtModifications)
  {
    m_Lock.Unlock() ;
    m_bExtModifications = false ;
  }
  return 0 ;
}

int TVObjects::GetTasksAsText( FXStringArray * pTasks )
{
  if (pTasks)
  {
    if (m_Lock.Lock( 500 , "TVObjects::GetTasksAsText 2" ))
    {
      m_bExtModifications = true ;
      m_Jobs.PrintJobs( *pTasks ) ;
      return ( int ) m_Jobs.GetCount() ;
    }
  }
  else if (m_bExtModifications)
  {
    m_Lock.Unlock() ;
    m_bExtModifications = false ;
  }
  return 0 ;
}

int TVObjects::GetObjectsAndTasks( VOArray ** ppObjects ,
  CVOJobList** ppTasksList , int *piActiveTask , FXLockObject ** pLockVideoObjects )
{
  if (pLockVideoObjects)
    *pLockVideoObjects = &m_Lock ;

  if (ppObjects || ppTasksList)
  {
    if (m_Lock.Lock( 500 , "TVObjects::GetObjectsAndTasks" ))
    {
      if (*ppObjects && *ppTasksList)
      {
        m_LockModify.Lock() ;
        if (m_pNewObjects)
        {
          m_pNewObjects->RemoveAll() ;
          delete m_pNewObjects ;
          m_pNewObjects = NULL ;
        }

        m_Jobs.RemoveAll() ;
        m_pNewObjects = *ppObjects ;
        int iNObjects = ( int ) m_pNewObjects->GetCount() ;
        m_bExtModifications = false ;
        CVOJobList * pTasks = *ppTasksList ;
        POSITION pos = pTasks->GetStartPosition() ;
        while (pos)
        {
          WORD rKey;
          void* rValue;

          pTasks->GetNextAssoc( pos , rKey , rValue );
          CVOJob * job = ( CVOJob* ) rValue;

          m_Jobs.SetAt( rKey , *job ) ;
        }
        WORD wTask = ( WORD ) ( ( piActiveTask && *piActiveTask >= 0 ) ?
          *piActiveTask : m_CurrentJobID ) ;
        m_CurrentJobID = -1 ;
        if (wTask < 0x8000)
        {
          CVOJob CurrJob ;
          if (m_Jobs.Lookup( wTask , CurrJob ))
          {
            m_CurrentJobID = wTask ;
            m_CurrentJob = CurrJob ;
            m_NewJob.RemoveAll() ;
          }
        }
        if (m_CurrentJobID < 0)
          m_CurrentJob.RemoveAll() ;
        m_LockModify.Unlock() ;
        m_Lock.Unlock() ;
        return iNObjects ;
      }
      else
      {
        m_bExtModifications = true ;
        if (ppObjects)
        {
          if (m_pNewObjects)
          {
            if (m_pObjects)
            {
              m_pObjects->RemoveAll() ;
              m_pObjects = NULL ;
            }
            m_pObjects = m_pNewObjects ;
            m_pNewObjects = NULL ;
          }
        }
        *ppObjects = m_pObjects ;
      }
      if (ppTasksList)
        *ppTasksList = &m_Jobs ;

      if (piActiveTask)
        *piActiveTask = m_CurrentJobID ;
      return ( *ppObjects ) ? ( int ) ( ( *ppObjects )->GetCount() + 1 ) : 0 ;
    }
  }
  else if (m_bExtModifications)
  {
    if (piActiveTask)
      m_CurrentJobID = *piActiveTask ;
    m_Lock.Unlock() ;
  }
  return 0 ;
}

TVObjMeas::TVObjMeas() :TVObjects()
{
  m_iCaptionPeriod = 20 ;
}


//TVObjectsGadget::~TVObjectsGadget()
void TVObjects::ShutDown()
{
  //ShutDown();
  CGadget::ShutDown();
  CDataFrame * pFr ;
  while (m_pInputs[ 0 ]->Get( pFr ))
    pFr->Release( pFr )  ;
  if (m_pLastDataFrame)
  {
    ( ( CDataFrame* ) m_pLastDataFrame )->Release() ;
    m_pLastDataFrame = NULL ;
  }

  delete m_pInputs[ 0 ];
  m_pInput = m_pInputs[ 0 ] = NULL;
  delete m_pInputs[ 1 ];
  m_pInputs[ 1 ] = NULL;
  delete m_pInputs[ 2 ];
  m_pInputs[ 2 ] = NULL;
  delete ( CInputPinWithQueue* ) ( m_pInputs[ 3 ] );
  m_pInputs[ 3 ] = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pDiagOut ;
  m_pDiagOut = NULL ;
  if (m_Lock.Lock( 1000 , "TVObjects::ShutDown" ))
  {
    if (m_pNewObjects)
      delete m_pNewObjects ;
    if (m_pObjects)
      delete m_pObjects ;
    if (m_pOldObjects)
      delete m_pOldObjects ;
    if (m_pTesseract)
    {
      m_pTesseract->End() ;
      delete m_pTesseract ;
      m_pTesseract = NULL ;
    }
    m_Lock.Unlock() ;
  }

}

void TVObjects::OnScript( CDataFrame* lpData )		//Third Input of TVObject
{
  FXAutolock al( m_Lock ) ;
  CTextFrame* TextFrame = lpData->GetTextFrame( DEFAULT_LABEL );
  if (TextFrame)
  {
    FXString Template ;
    FXString sTemplateTemp = TextFrame->GetString();
    FXString sTemplatePart = "";
    //int nCutter = 0;
    int nOpenBrktCntr = 0;
    int nCloseBrktCntr = 0;
    int nBrktPos = 0;
    int nBrktPosOpen = 0;
    int nBrktPosClose = 0;
    FXSIZE nStartRow = 0;
    while (( sTemplatePart = sTemplateTemp.Tokenize( _T( "\r\n" ) , nStartRow ) ).IsEmpty() == FALSE)
    {
      while (nBrktPosOpen != -1)
      {
        while (( nOpenBrktCntr > nCloseBrktCntr ) || ( nOpenBrktCntr == 0 ))
        {
          nBrktPosOpen = ( int ) sTemplatePart.Find( "(" , nBrktPos + 1 );
          nBrktPosClose = ( int ) sTemplatePart.Find( ")" , nBrktPos + 1 );
          if (( nBrktPosOpen < nBrktPosClose ) && ( nBrktPosOpen != -1 ))
          {
            nOpenBrktCntr++;
            nBrktPos = nBrktPosOpen;
          }
          else if (nBrktPosClose != -1)
          {
            nCloseBrktCntr++;
            nBrktPos = nBrktPosClose;
          }
        }
        nBrktPos = ( int ) sTemplatePart.Find( "," , nBrktPos );
        sTemplatePart.TrimRight( "\r\n" );
        sTemplatePart.Insert( nBrktPos + 1 , "\r\n" );
        nOpenBrktCntr = 0;
        nCloseBrktCntr = 0;
      }
      Template.Append( sTemplatePart );
    }
    Template.TrimLeft( "\r\n" );
    LoadTemplate( &Template );
  }
  lpData->Release( lpData );
}
void TVObjects::OnControl( CDataFrame* lpData )	//Second Input of TVObject
{
  CTextFrame* TextFrame = lpData->GetTextFrame( DEFAULT_LABEL );
  if (TextFrame)
  {
    if (_tcsstr( TextFrame->GetLabel() , "SetObjectProp" ))
    {
      FXPropKit2 pk( TextFrame->GetString() );
      FXString Name ;
      VOArray * pObjects = m_pObjects ? m_pObjects : m_pNewObjects;
      if (pk.GetString( "name" , Name ) && pObjects)
      {
        pk.DeleteKey( "name" ) ;
//         if (m_Lock.Lock( 3000 , "TVObjects::OnControl" ))
        {
          for (int i = 0 ; i < pObjects->GetCount() ; i++)
          {
            CVideoObject& Obj = pObjects->GetAt( i ) ;
            if (Name == Obj.m_ObjectName)
            {
              bool bInvalidate = false ;
              // Moisey: For current version (16.08.2023) -
              // if placement is not absolute, ROI changes is not allowed
              Obj.m_bExternalModification = true ; 
              Obj.ScanProperties( pk , bInvalidate ) ;
              Obj.m_bExternalModification = false ;
              CSetupObject * pSetupObject = Obj.GetSetupObject() ;
              if (bInvalidate && pSetupObject)
              {
                HWND hWnd = ( ( CObjectStdSetup* ) pSetupObject )->GetSafeHwnd() ;
                if (hWnd)
                {
                  ::RedrawWindow( hWnd , NULL , NULL , 0 );
                }
              }
              break ;
            }
          }
//           m_Lock.Unlock() ;
        }
      }
    }
    else
    {
      TVObjectsGadgetSetupDialog * pSetup = ( TVObjectsGadgetSetupDialog* ) m_SetupObject ;
      FXParser2 temp = ( LPCTSTR ) TextFrame->GetString() , res;
      CPoint offset( 0 , 0 );
      FXString val;

      if (temp.FindElement( "CPoint" , res ))
      {
        int pos = 0;
        if (res.GetParam( pos , val , ',' ))
          offset.x = atoi( val );
        if (res.GetParam( pos , val , ',' ))
          offset.y = atoi( val );
        m_Lock.Lock( 500 , "TVObjects::OnControl" ) ;
        m_pObjects->SetCommonOffset( offset );
        m_Lock.Unlock() ;
      }
      if (temp.FindElement( "Task" , res ) )
      {
        int pos = 0;
        if (res.GetParam( pos , val , ',' ))
        {
          int iJobID = atoi( val ) ;
          if (iJobID != m_CurrentJobID) // if the same - nothing to do
          {
            if (m_Lock.Lock( 500 , "TVObjects::OnControl" ))
            {
              m_NewJob.RemoveAll() ;
              if (m_Jobs.Lookup( iJobID , m_NewJob ))
                m_CurrentJobID = iJobID ;
              else
              {
                m_CurrentJobID = -1 ;
              }
              m_Lock.Unlock() ;
            }
          }
        }
      }
      if (temp.FindElement( "AOSInd" , res ))
      {
        int pos = 0;
        if (res.GetParam( pos , val , ',' ))
          m_iCurrentAOSIndex = atoi( val );
      }
      if (temp.FindElement( "Scale" , res ))
      {
        int pos = 0;
        if (res.GetParam( pos , val , ',' ))
        {
          m_dScale_um_per_pix = atof( val );
          if (res.GetParam( pos , val , ',' ))
            m_ScaleUnits = val;
          if (m_ScaleUnits.IsEmpty())
            m_ScaleUnits = _T( "pix" ) ;
        }
      }
      if (temp.FindElement( "gtimeout_ms" , res ))
      {
        int pos = 0;
        if (res.GetParam( pos , val , ',' ))
          m_dTimeout = atof( val );
      }
      if (temp.FindElement( "max_n_objects" , res ))
      {
        int pos = 0;
        if (res.GetParam( pos , val , ',' ))
          m_iNObjectsMax = atoi( val );
      }
      if (temp.FindElement( "ViewMode" , res ))
      {
        DWORD mask = ( int ) ConvToBinary( res ) ;
        m_ViewModeOption.SetViewMode( mask ) ;
      }
      if (temp.FindElement( "BinaryOut" , res ))
      {
        int pos = 0;
        if (res.GetParam( pos , val , ',' ))
        {
          int bBinaryOutEnable = atoi( val ) ;
          pSetup->m_bBinaryOutput = ( bBinaryOutEnable != 0 ) ;
        }
      }
    }
  }
  lpData->Release( lpData );
}

void TVObjects::OnCoordinates( CDataFrame* lpData )		//Fourth Input of TVObject
{
  CTextFrame* TextFrame = lpData->GetTextFrame( DEFAULT_LABEL );
  if (TextFrame)
  {
    FXPropertyKit Text( TextFrame->GetString() ) ;
    int iSetROIDiff = ( int ) Text.Find( _T( "setroidiff" ) ) ;
    int iSetROIPos = (iSetROIDiff >= 0) ? -1 : ( int ) Text.Find( _T( "setroi" ) ) ;

    FXString ObjectName ;
    FXString RectAsString ;
    CRect ExtractedRect ;
    if ( (iSetROIDiff >= 0) || (iSetROIPos >= 0) )
    {
      FXSIZE iSpacePos = Text.Find( ' ' ) ;
      FXString Content = Text.Mid( iSpacePos + 1 ) ;
      FXSIZE iEqPos = Text.Find( '=' , iSpacePos ) ;
      if ( (iSpacePos > 0) && (iEqPos > 0) )
      {
        ObjectName = Text.Mid( iSpacePos + 1 , iEqPos - iSpacePos - 1 ).Trim() ;
        RectAsString = Text.Mid( iEqPos + 1 ).Trim() ;
        
        int iNItems = GetArray( RectAsString , 'i' , 4 , ( void* ) &ExtractedRect ) ;
        if ( iNItems == 4 )
        {
          if ( m_Lock.Lock( 500 , "TVObjects::OnCoordinates" ) )
          {
            for ( int i = 0 ; i < ( int ) m_pObjects->GetCount() ; i++ )
            {
              CVideoObject& Obj = m_pObjects->GetAt( i ) ;
              if ( Obj.m_ObjectName.CompareNoCase( ObjectName ) == 0 )
              {
                if ( iSetROIDiff >= 0 )
                {
                  Obj.m_AOS.left += ExtractedRect.left ;
                  Obj.m_AOS.top += ExtractedRect.top ;
                  Obj.m_AOS.right += ExtractedRect.right ;
                  Obj.m_AOS.bottom += ExtractedRect.bottom ;
                  break ;
                }
                else if ( ( iSetROIPos >= 0 ) && ( Obj.m_Placement == PLACE_ABS ) )
                  Obj.m_AOS = ExtractedRect ;

                CObjectStdSetup * pSetupObject = ( CObjectStdSetup* ) Obj.GetSetupObject() ;
                if ( pSetupObject && pSetupObject->IsOn() )
                {
                  HWND hSetupWnd = pSetupObject->GetSafeHwnd() ;

                  if ( hSetupWnd )
                  {
                    pSetupObject->SetCellInt( "xoffset" , Obj.m_AOS.left ) ;
                    pSetupObject->SetCellInt( "yoffset" , Obj.m_AOS.top ) ;
                    pSetupObject->SetCellInt( "width" , Obj.m_AOS.right ) ;
                    pSetupObject->SetCellInt( "height" , Obj.m_AOS.bottom ) ;
                    ::PostMessage( hSetupWnd , WM_PAINT , NULL , NULL ) ;
                  }
                }
                break ;
              }
            }
            m_Lock.Unlock() ;
          }
        }
      }
    }
    else
    {
      TVObjectsGadgetSetupDialog * pSetup = ( TVObjectsGadgetSetupDialog* )m_SetupObject ;
      FXString SelState ;
      bool bSelected = Text.GetString( "selected" , SelState ) ;
      if (iSetROIPos >= 0)
      {
        FXPropertyKit ObjectNameAndRect = Text.Mid( iSetROIPos + 7 ) ; // setroi and space cutting
        int iEquSignPos = ( int ) ObjectNameAndRect.Find( _T( '=' ) ) ;
        if (iEquSignPos >= 1)
        {
          FXString ROIName = ObjectNameAndRect.Left( iEquSignPos ) ;
        }
      }
      else if (pSetup &&  pSetup->IsOn())
      {
        if (pSetup->m_bSelectRenderer == TRUE)
        {
          bool bReady = false ;
          int iLinePos = ( int ) Text.Find( _T( "Line=" ) ) ;
          int iRectPos = ( int ) Text.Find( _T( "Rect=" ) ) ;
          FXIntArray Coords ;
          if (( iLinePos >= 0 )   // Line information
            || ( iRectPos >= 0 )) // Rect information
          {
            FXString CoordsAsString =
              Text.Mid( ( ( iLinePos >= 0 ) ? iLinePos : iRectPos ) + 5 ) ;
            if (GetIntArrayFromString( CoordsAsString , Coords )
              && ( Coords.GetCount() == 4 ))
            {
              CRect Rect( Coords[ 0 ] , Coords[ 1 ] , Coords[ 2 ] , Coords[ 3 ] ) ;

              if (( abs( Rect.Width() ) >= 2 ) && ( abs( Rect.Height() ) >= 2 ))
              {
                Rect.NormalizeRect() ;
                pSetup->m_LastRectFromUI = Rect ;
                if (GetKeyState( VK_SHIFT ) & 0x8000)
                {
                  pSetup->m_PtSelectStart = Rect.TopLeft() ;
                  pSetup->m_PtSelectEnd = Rect.BottomRight() ;
                  pSetup->m_sSelectRendererStart = Text ;	// - update first.
                  pSetup->m_sSelectRendererEnd = Text ;	// - update second.
                }
              }
              else
              {
                pSetup->m_sSelectRendererEnd =
                  pSetup->m_sSelectRendererStart ;
              }
              bReady = true ;
            }
          }
          else  // working with separate points
          {
            if (bSelected)
            {
              pSetup->m_PtCurrent = GetXYfromString( Text ) ;
              if (SelState == "true")  // first or tracking point
              {
                if (pSetup->m_sSelectRendererStart.IsEmpty())
                {
                  pSetup->m_sSelectRendererStart = Text ;	// - update first.
                  pSetup->m_PtSelectEnd = pSetup->m_PtSelectStart ;
                  pSetup->m_PtSelectStart = pSetup->m_PtCurrent ;
                }
              }
              else if (SelState == "false") // last point
              {
                if (pSetup->m_sSelectRendererEnd.IsEmpty())
                {
                  pSetup->m_sSelectRendererEnd = Text ;	// - update first.
                  pSetup->m_PtSelectEnd = pSetup->m_PtCurrent ;
                  bReady = true ;
                }
              }
            m_Lock.Unlock() ;
            }
          }
          if (bReady)
          {
            ::PostMessage( pSetup->m_hWnd , NEW_OBJECT_FROM_UI_MSG ,
              ( iRectPos >= 0 ) ? 2 : 1 , NULL ) ;
            //         if ( m_ViewModeOption.getbCatchObject() )
            //           ::PostMessage( pSetup->m_hWnd , NEW_OBJECT_FROM_UI_MSG , 1 , NULL ) ;
            //         else if ( m_ViewModeOption.getbFindObject() )
            //           ::PostMessage( pSetup->m_hWnd , NEW_OBJECT_FROM_UI_MSG , 2 , NULL ) ;
            //         else if ( m_ViewModeOption.getbFindObjectInArea() )
            //           ::PostMessage( pSetup->m_hWnd , NEW_OBJECT_FROM_UI_MSG , 3 , NULL ) ;
          }
        }
      }
      else  if (SelState == "dbl") // last point
      {
        FXString ObjName ;
        if (Text.GetString( "rect_name" , ObjName ))
        {
          FXAutolock al( m_Lock ) ;
          for (int i = 0; i <= m_CurrentJob.GetUpperBound(); i++)
          {

            int iObjId = m_CurrentJob.GetAt( i ).m_ObjectID ;
            if (iObjId >= 0
              && m_pObjects->GetAt( iObjId ).IsObjectName( ObjName ))
            {
              CWnd * pMainWnd = AfxGetApp()->GetMainWnd() ;
              if (pMainWnd)
              {
                CPoint Pt = GetXYfromString( Text ) ;
                WPARAM wPar = ( Pt.x & 0xffff ) | ( ( Pt.y << 16 ) & 0xffff ) ;
                CVideoObjectBase * pObj = &( m_pObjects->GetAt( iObjId ) ) ;
                ::PostMessage( pMainWnd->m_hWnd , VM_TVDB400_SHOWVOSETUPDLG ,
                  wPar , ( LPARAM ) pObj ) ;
              }
              //             Tvdb400_ShowObjectSetupDlg( &(m_pObjects->GetAt( iObjId )) ,
              //               GetXYfromString( Text ) ) ;
            }
          }
        }
      }
    }
  }

  lpData->Release( lpData );
}

void TVObjects::OnStart()
{
  m_CallOnStartFunc = 0 ;
  m_bRun = true ;
} ;

void TVObjects::OnStop()
{
  m_CallOnStartFunc = 1;
  CDataFrame* pFrame = CDataFrame::Create();

  //pFrame->SetLabel("TVObject");
  Tvdb400_SetEOS( pFrame );

  //   CDataFrame* Container = CContainerFrame::Create() ;
  //   ((CContainerFrame*)Container)->AddFrame(pFrame);
  //   CQuantityFrame* pQuanFrame = CQuantityFrame::Create(0);
  //   pQuanFrame->ChangeId(pFrame->GetId());
  //   pQuanFrame->SetTime(pFrame->GetTime());
  //   pQuanFrame->SetLabel("TVObject");
  //   ((CContainerFrame*)Container)->AddFrame(pQuanFrame);  
  //  m_Lock.Lock();  // now we need to deliver the EOS message to the rest of the graph -> send it to all outputs

  COutputConnector* pOutput = ( COutputConnector* ) m_pOutput;
  //   if ( !pOutput->Put(Container) )
  //     Container->Release( Container ); 
  if (!pOutput || !pOutput->Put( pFrame ))
    pFrame->Release( pFrame );

  //  m_Lock.Unlock();
  m_bRun = false ;
} ;

bool    TVObjects::PrintProperties( FXString& text )
{
  FXPropKit2 pk;
  m_LockModify.Lock() ;
  VOArray * pObjects = ( m_pNewObjects && m_pNewObjects->GetCount() ) ? m_pNewObjects : m_pObjects ;
  if (pObjects)
  {
    FXString Template , ObjectAsText , TasksAsText ;
    for (int i = 0 ; i < pObjects->GetCount() ; i++)
    {
      if (pObjects->GetAt( i ).PrintProperties( ObjectAsText , pObjects ))
      {
        Template += ObjectAsText + _T( "\n" ) ;
      }
    }
    m_Jobs.PrintJobs( TasksAsText ) ;
    Template += TasksAsText ;
    m_Template = Template ;
  }
  pk.WriteString( "Template" , m_Template , false );
  m_LockModify.Unlock() ;
  pk.WriteInt( "ActiveTask" , m_CurrentJobID );
  pk.WriteInt( "CaptionPeriod" , m_iCaptionPeriod ) ;
  pk.WriteInt( _T( "ViewMode" ) , m_ViewModeOption.GetViewMode() ) ;
  FXString Scaling ;
  if (m_ScaleUnits.IsEmpty())
    m_ScaleUnits = _T( "pix" ) ;
  Scaling.Format( _T( "%g,%s" ) , m_dScale_um_per_pix , m_ScaleUnits ) ;
  pk.WriteString( _T( "Scaling" ) , Scaling ) ;
  pk.WriteDouble( _T( "gtimeout_ms" ) , m_dTimeout ) ;
  pk.WriteInt( _T( "NObjectsMax" ) , m_iNObjectsMax ) ;
  pk.WriteInt( _T( "LastImageWidth" ) , m_LastFrameSize.cx ) ;
  pk.WriteInt( _T( "LastImageHeight" ) , m_LastFrameSize.cy ) ;
  CFilterGadget::PrintProperties( pk ) ;
  text = pk ;
  return true;
}
static const char replaced_chars[][ 2 ] = { ";" , "\n" , "\r" , "\\" };
static const int  replacemen_nmb = 4;
static const char replacement[][ 3 ] = { "\\s" , "\\n" , "\\r" , "\\\\" };

bool TVObjects::ScanProperties( LPCTSTR Text , bool& Invalidate )
{
  if (!m_bExtModifications)
    m_Lock.Lock( 1000 , "TVObjects::ScanProperties" ) ;

  //FXAutolock al( m_Lock ) ;
  FXPropKit2 pk( Text );
  pk.Remove( _T( ' ' ) ) ;
  pk.Remove( _T( '\t' ) ) ;
  FXPropKit2 pkUnreg( FxUnregularize( pk ) ) ;

  FXString Template ;
  bool bTake2 = pkUnreg.GetTillSemicolonWithBrackets( "Template" , Template ) ;
  if (bTake2)
  {
    //     int i = 0;
    //     while ( i < replacemen_nmb )
    //     {
    //       Template.Replace( replacement[ i ] , replaced_chars[ i ] );
    //       i++;
    //     }
    LoadTemplate( &Template );
  }
  int iMayBeTask = -2 ;
  if (pkUnreg.GetInt( "ActiveTask" , iMayBeTask ))
  {
    //     if ( iMayBeTask != m_CurrentJobID ) // if the same - nothing to do
    //     {
    m_NewJob.RemoveAll() ;
    if (m_Jobs.Lookup( iMayBeTask , m_NewJob ))
      m_CurrentJobID = iMayBeTask ;
    else
    {
      m_CurrentJobID = -1 ;
    }
    //     }
  }
  int iViewMode ;
  if (pkUnreg.GetInt( _T( "ViewMode" ) , iViewMode ))
    m_ViewModeOption.SetViewMode( iViewMode ) ;
  FXParser2 Scaling ;
  if (pk.GetString( _T( "Scaling" ) , Scaling ))
  {
    int pos = 0 ;
    FXString val ;
    if (Scaling.GetParam( pos , val , ',' ))
      m_dScale_um_per_pix = atof( val );
    if (Scaling.GetParam( pos , val , ',' ))
      m_ScaleUnits = val;
    if (m_ScaleUnits.IsEmpty())
      m_ScaleUnits = _T( "pix" ) ;
    m_ViewModeOption.setbViewCoordScaled( m_dScale_um_per_pix != 1.0 ) ;
  }
  BOOL bReinject = FALSE ;
  if (pkUnreg.GetInt( _T( "Reinject" ) , bReinject )
    && bReinject && m_pLastDataFrame && m_pInputs[ 0 ])
  {
    ( ( CDataFrame* ) m_pLastDataFrame )->AddRef() ;
    if (!m_pInputs[ 0 ]->Send( ( CDataFrame* ) m_pLastDataFrame ))
      ( ( CDataFrame* ) m_pLastDataFrame )->Release() ;
  }
  if (!m_bExtModifications)
    m_Lock.Unlock() ;

  pk.GetInt( "CaptionPeriod" , m_iCaptionPeriod ) ;
  return true;
}


bool TVObjMeas::ScanProperties( LPCTSTR Text , bool& Invalidate )
{
  TVObjects::ScanProperties( Text , Invalidate ) ;
  FXPropertyKit pk( Text ) ;
  //pk.GetInt( "CaptionPeriod" , m_iCaptionPeriod ) ;
  return true;
}

bool    TVObjects::ScanSettings( FXString& text )
{
  text = "calldialog(true)";
  return true;
}

static int iTraceEnable = 0 ;

CDataFrame* TVObjects::DoProcessing( const CDataFrame* pDataFrame )
{
  if ( iTraceEnable )
  {
    TRACE( "***** DoProcessing() - start: pDataFrame# Type=%d; Users=%d; Labes=%s; Regitred=%d; ID=%d;\n"
      , pDataFrame->GetDataType()
      , pDataFrame->GetUserCnt()
      , pDataFrame->GetLabel()
      , ( int ) pDataFrame->IsRegistered()
      , pDataFrame->GetId()
    );
  }
  
  double dBeginProcessingTime = GetHRTickCount() ;
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  if (!VideoFrame || !VideoFrame->lpBMIH)
    return NULL ;
  else if (pDataFrame != m_pLastDataFrame)
  {
    if (m_pLastDataFrame)
      ( ( CDataFrame* ) m_pLastDataFrame )->Release( NULL ) ;
    ( ( CDataFrame* ) pDataFrame )->AddRef() ;
    m_pLastDataFrame = pDataFrame ;
  }
  if (m_LastFormat != VideoFrame->lpBMIH->biCompression)
  {
    m_LastFormat = VideoFrame->lpBMIH->biCompression;
    m_FormatErrorProcessed = false;
  }
  if (( m_LastFormat != BI_YUV9 ) && ( m_LastFormat != BI_YUV12 )
    && ( m_LastFormat != BI_Y8 ) && ( m_LastFormat != BI_Y800 ) && ( m_LastFormat != BI_Y16 ))
  {
    if (!m_FormatErrorProcessed)
      SENDERR_0( "TVObjects can only accept formats Y16,YUV9, YUV12 and Y8" );
    m_FormatErrorProcessed = true;
    return NULL;
  }
  m_LastFrameSize.cx = VideoFrame->lpBMIH->biWidth ;
  m_LastFrameSize.cy = VideoFrame->lpBMIH->biHeight ;
  int iJobID = m_CurrentJobID ;

  if (m_Lock.Lock( 500 , "TVObjects::DoProcessing" ))
  {
    if (m_pNewObjects)
    {
      FXArray<ExistingObjSetupDlg> ExistingDlgs ;
      FXArray<CPoint , CPoint> PositionsOfDlgs ;
      FXString ObjName ;
      CRect rc ;
      if (m_pObjects)
      {
        for (int i = 0 ; i < m_pObjects->GetCount() ; i++)
        {
          CVideoObject& Obj = m_pObjects->GetAt( i ) ;
          CObjectStdSetup* pSetup = ( CObjectStdSetup* ) Obj.GetSetupObject() ;
          if (pSetup) // object setup dialog is open
          {
            if (Obj.GetObjectName( ObjName ))
            {
              ExistingObjSetupDlg Dlg( ObjName , pSetup ) ;
              Obj.SetSetupObject( NULL ) ;
              ExistingDlgs.Add( Dlg ) ;
              pSetup->GetWindowRect( &rc ) ;
              PositionsOfDlgs.Add( rc.TopLeft() ) ;
            }
          }
        }
        delete m_pObjects ;
      }
      m_pObjects = m_pNewObjects ;
      m_pNewObjects = NULL ;
      for (int iDlgCnt = 0 ; iDlgCnt < ExistingDlgs.GetCount() ; iDlgCnt++)
      {
        FXString OldObjName = ExistingDlgs[ iDlgCnt ].m_ObjectName ;
        int i = 0 ;
        for (; i < m_pObjects->GetCount() ; i++)
        {
          CVideoObject& Obj = m_pObjects->GetAt( i ) ;
          if (Obj.GetObjectName( ObjName ) && ( ObjName == OldObjName ))
          { // there is new object with the same name
            // we have to open setup dialog for him, because it was opened
            // Tvdb400_ShowObjectSetupDlg( &Obj , PositionsOfDlgs[ i ] ) ;
            Obj.SetSetupObject( ExistingDlgs[ iDlgCnt ].m_pObjStdDlg ) ;
            break ;
          }
        }
        if (i == m_pObjects->GetCount())
          ExistingDlgs[ iDlgCnt ].m_pObjStdDlg->PostMessageA( WM_DESTROY ) ;
      }
    }
    iJobID = m_CurrentJobID ;
    if (m_NewJob.GetCount())
    {
      m_CurrentJob = m_NewJob ;
      m_NewJob.RemoveAll() ;
    }
    if (iJobID < 0)
      m_CurrentJob.RemoveAll() ;

    m_Lock.Unlock() ;
  }
  else
    return NULL ;

  if (!m_pOutput->IsConnected())
  {
    return CreateTextFrame( _T( "Dummy, No Connections" ) , pDataFrame->GetId() ) ;
  }

  TVObjectsGadgetSetupDialog * pSetup = ( TVObjectsGadgetSetupDialog* ) m_SetupObject ;
  CContainerFrame * pOutputContainer = CContainerFrame::Create() ;
  pOutputContainer->CopyAttributes( pDataFrame ) ;
  pOutputContainer->AddFrame( pDataFrame ) ;
  if (iJobID < 0)
  {
    CTextFrame * pDummyText = CreateTextFrame( _T( "Dummy, No Task" ) , pDataFrame->GetId() ) ;
    pOutputContainer->AddFrame( pDummyText ) ;
    return pOutputContainer ;
  }

  CContainerFrame* roiVal = NULL ;
  CContainerFrame* resVal = NULL ;

  if (pSetup && !pSetup->m_bBinaryOutput)
  {
    roiVal = CContainerFrame::Create();
    roiVal->CopyAttributes( pDataFrame ) ;
    roiVal->SetLabel( "ROI" );

    resVal = CContainerFrame::Create();
    resVal->CopyAttributes( pDataFrame ) ;
    resVal->SetLabel( "ResultsRes" );
  }
  else
  {
    switch (WaitForSingleObject( m_evExit , 0 ))
    {
      case WAIT_FAILED:
      case WAIT_OBJECT_0: return NULL ;
    }
  }

  int iViewMode = m_ViewModeOption.GetViewMode() | OBJ_OUT_MOMENTS | OBJ_VIEW_CSV ;
  CRectFrame* rf = NULL;
  CTextFrame * pTextResult = NULL ;
  CTVObjDataFrame * pBinaryResult = NULL ;
  for (int i = 0; i <= m_CurrentJob.GetUpperBound(); i++)
  {
    int iObjId = m_CurrentJob.GetAt( i ).m_ObjectID ;
    if (iObjId >= 0)
    {
      double dObjectBeginProcTime_ms = GetHRTickCount() ;
      CVideoObject& Obj = m_pObjects->GetAt( iObjId ) ;

      FXAutolock al( Obj.m_VObjectLock ) ; // Object data are in using

      FXString Color , InverseColor ;
      Color.Format( "0x%06X" , Obj.m_ViewColor ) ;
      InverseColor.Format( "0x%06X" , ~Obj.m_ViewColor & 0x00ffffff ) ;
      if (Obj.m_bForROIViewOnly && roiVal)
      {
        CFigureFrame * pViewRect = CreatePtFrame( cmplx( Obj.m_RectForROIViewOnly.left , Obj.m_RectForROIViewOnly.top ) ,
          GetHRTickCount() , Color , Obj.m_ObjectName , pDataFrame->GetId() ) ;
        if (pViewRect)
        {
          pViewRect->Add( CDPoint( Obj.m_RectForROIViewOnly.right , Obj.m_RectForROIViewOnly.top ) ) ;
          pViewRect->Add( CDPoint( Obj.m_RectForROIViewOnly.right , Obj.m_RectForROIViewOnly.bottom ) ) ;
          pViewRect->Add( CDPoint( Obj.m_RectForROIViewOnly.left , Obj.m_RectForROIViewOnly.bottom ) ) ;
          pViewRect->Add( CDPoint( Obj.m_RectForROIViewOnly.left , Obj.m_RectForROIViewOnly.top ) ) ;
          roiVal->AddFrame( pViewRect ) ;
        }

        continue ;
      }
      int iObjViewMode = Obj.m_dwViewMode & iViewMode ;
      int iNMeasCycles = 1 ;
      switch (Obj.m_Type)
      {
        case SPOT:
          iNMeasCycles = ( int ) max( Obj.m_Thresholds.GetCount() ,
            Obj.m_RotationThresholds.GetCount() ) ;
          break ;
        default:
          break ;
      }
      for (int iMeasCnt = 0 ; iMeasCnt < iNMeasCycles ; iMeasCnt++)
      {
        Obj.m_pDiagnostics = NULL ;
        if (Obj.m_Type == OCR)
        {
          if (iMeasCnt > 0)
            continue;

          if ( ( Obj.m_WhatToMeasure & MEASURE_TXT_FAST ) && ( m_pTesseract == NULL ))
          {
            if (m_iTesseractRetryDelay == 0)
            {
              m_pTesseract = new tesseract::TessBaseAPI() ;
              FXRegistry Reg( "TheFileX\\SHStudio" ) ;
              FXString OCRDataPath = Reg.GetRegiString(
                "OCR" , "OCRDataPath" ,
                "C:\\Program Files (x86)\\Tesseract-OCR\\tessdata" ) ;
              int iErr = m_pTesseract->Init( OCRDataPath , "eng" ) ;
              if (iErr != 0)
              {
                SENDERR( "Tesseract INIT ERROR %d ... OCR is Off" , iErr ) ;
                m_pTesseract->End() ;
                delete m_pTesseract ;
                m_pTesseract = NULL ;
                m_iTesseractRetryDelay = Reg.GetRegiInt(
                  "OCR" , "OCRRetryDelay" , 100 ) ;
              }
              else
              {
                m_pTesseract->SetPageSegMode( PSM_SINGLE_LINE ) ;
                FILE * fw = fopen( "TesseractPars.txt" , "w" ) ;
                if (fw)
                {
                  m_pTesseract->PrintVariables( fw ) ;
                  fclose( fw );
                }
              }
            }
            else
              m_iTesseractRetryDelay-- ;
          }
          Obj.m_pTesseract = ( Obj.m_WhatToMeasure & MEASURE_TXT_FAST ) ?
            m_pTesseract : NULL ;
        }

        FXString ObjName = Obj.m_ObjectName ;
        if (( Obj.m_Type == SPOT ) && ( iNMeasCycles > 1 ))
        {
          int iThresIndex = iMeasCnt ;
          if (iThresIndex >= Obj.m_Thresholds.GetCount())
            iThresIndex = ( int ) Obj.m_Thresholds.GetUpperBound() ;
          int iRotThresIndex = iMeasCnt ;
          if (iRotThresIndex >= Obj.m_RotationThresholds.GetCount())
            iRotThresIndex = ( int ) Obj.m_RotationThresholds.GetUpperBound() ;
          Obj.m_dProfileThreshold = Obj.m_Thresholds[ iThresIndex ] ;
          Obj.m_dRotationThreshold = Obj.m_RotationThresholds[ iRotThresIndex ] ;
          ObjName.Format( _T( "%s[%d,%d]_%5.3f_%5.3f" ) ,
            ( LPCTSTR ) Obj.m_ObjectName , iMeasCnt , iNMeasCycles ,
            Obj.m_dProfileThreshold , Obj.m_dRotationThreshold ) ;
        }
        Obj.m_WhatIsMeasured = 0 ;
        
        if( iTraceEnable )
        {
          TRACE( "***** DoProcessing() - before DoMeasure(): pDataFrame# Type=%d; Users=%d; Labes=%s; Regitred=%d; ID=%d;\n"
            , pDataFrame->GetDataType()
            , pDataFrame->GetUserCnt()
            , pDataFrame->GetLabel()
            , ( int ) pDataFrame->IsRegistered()
            , pDataFrame->GetId()
          );
        }
        //search for the object and measure.
        bool bMeasResult = m_pObjects->DoMeasure( iObjId , VideoFrame , m_iCurrentAOSIndex ) ;
        double dObjMeasFinished = GetHRTickCount() ;

        if ( iTraceEnable )
        {
          TRACE( "***** DoProcessing() - after-DoMeasure(): pDataFrame# Type=%d; Users=%d; Labes=%s; Regitred=%d; ID=%d;\n"
            , pDataFrame->GetDataType()
            , pDataFrame->GetUserCnt()
            , pDataFrame->GetLabel()
            , ( int ) pDataFrame->IsRegistered()
            , pDataFrame->GetId()
          );
        }

        CRect rc( 0 , 0 , 0 , 0 );
        if (m_iCurrentAOSIndex < 0)
          rc = Obj.m_absAOS ;
        else if (m_iCurrentAOSIndex <= Obj.m_SavedAOS.GetUpperBound())
          rc = Obj.m_SavedAOS[ m_iCurrentAOSIndex ] ;

        if (pSetup && pSetup->m_bBinaryOutput && bMeasResult)
        {
          if (!pBinaryResult)
            pBinaryResult = CTVObjDataFrame::Create() ;
          if (pBinaryResult)
            pBinaryResult->AddObject( &Obj ) ;
          else
            ASSERT( 0 ) ;
          continue ; // go to another object, another frames are omitted
        }
        if (roiVal && !rc.IsRectNull()
          && ( iObjViewMode & OBJ_VIEW_ROI ) && ( iMeasCnt == 0 ))
        {
          CRectFrame* rff = CRectFrame::Create( &rc );
          CopyIdAndTime( rff , pDataFrame ) ;
          rff->Attributes()->WriteString( "color" , bMeasResult ? Color : InverseColor ) ;
          rff->Attributes()->WriteInt( "Selectable" , 1 ) ;
          FXString Label( "ROI:" ) ;
          Label += ObjName;
          rff->SetLabel( Label );
          roiVal->AddFrame( rff );
          if ( Obj.m_Placement != PLACE_ABS )
            Obj.SaveROI( pDataFrame->GetId() ) ;
        }
        if (Obj.m_pDiagnostics)
        {
          resVal->AddFrame( Obj.m_pDiagnostics ) ;
          Obj.m_pDiagnostics = NULL ;
        }

        double dBeforeConturView = 0. ;
        if (bMeasResult)
        {
          if (Obj.m_WhatIsMeasured & MEASURE_POSITION)
          {
            if (Obj.m_Type == SPOT /*&& Obj.m_WhatToMeasure*/)
            {
              bool bMultiCenter = ( Obj.m_WhatToMeasure & ( DET_MEASURE_ANGLE_CW | DET_MEASURE_ANGLE_CNW ) ) != FALSE ;
              FXUintArray& ActiveIndexes = Obj.m_pSegmentation->m_ActiveSpots ;
              char Buf[ 100000 ] ;
              int iIndex = 0 ;
              FXString ForMomentsOut , TmpForMoments ;
              for (int i = 0 ; i < ActiveIndexes.GetCount() ; i++)
              {
                bool bCentViewed = false ;
                DWORD iSpotIndex = ActiveIndexes[ i ] ;
                CColorSpot& Spot = Obj.m_pSegmentation->m_ColSpots[ iSpotIndex ] ;
                if (Spot.m_Area <= 0)
                  continue ;
                Spot.m_iIndex = iSpotIndex ;
                if (Obj.m_LeaderIndex.x >= 0
                  || Obj.m_LeaderIndex.y >= 0)
                {
                  CDPoint Shift( Obj.m_absAOS.left , Obj.m_absAOS.top ) ;
                  //   Spot.m_SimpleCenter += Shift ;
                  if (Spot.m_CenterByOutFrame.x > 0.)
                    Spot.m_CenterByOutFrame += Shift ;
                  if (Spot.m_CenterByIntense.x > 0.)
                    Spot.m_CenterByIntense += Shift ;
                  if (Spot.m_CenterByContur.x > 0.)
                    Spot.m_CenterByContur += Shift ;
                  Spot.m_OuterFrame.OffsetRect( Obj.m_absAOS.left , Obj.m_absAOS.top ) ;
                }

                if (Spot.m_Centers[ iMeasCnt ].real() == 0.0)
                  Spot.m_Centers[ iMeasCnt ] = cmplx( Spot.m_SimpleCenter.x , Spot.m_SimpleCenter.y ) ;
                //                 if (m_ViewModeOption.getbDispPos() == TRUE)
                FXString ObjNameForLabel ;
                ObjNameForLabel.Format( "%s_%d" , ( LPCTSTR ) ObjName , i ) ;
                if (resVal && ( iObjViewMode & OBJ_VIEW_POS ) && ( iMeasCnt == 0 ))
                {
                  CFigureFrame* ff = CFigureFrame::Create();
                  CopyIdAndTime( ff , pDataFrame ) ;
                  ff->Attributes()->WriteString( "color" , Color ) ;
                  if (!( ( Obj.m_WhatIsMeasured & MEASURE_DIAMETERS )
                    && ( iObjViewMode & OBJ_VIEW_DIA ) )
                    || ( Spot.m_UpLong.real() == 0. ))
                  {
                    ff->AddPoint( Spot.m_SimpleCenter ) ;
                    ff->SetLabel( ObjNameForLabel + "_cent" );
                    resVal->AddFrame( ff );
                  }
                  else
                  {
                    CDPoint LongUp( Spot.m_UpLong.real() , Spot.m_UpLong.imag() ) ;
                    CDPoint LongDown( Spot.m_DownLong.real() , Spot.m_DownLong.imag() ) ;
                    ff->AddPoint( LongUp ) ;
                    ff->AddPoint( LongDown ) ;
                    ff->ChangeId( pDataFrame->GetId() );
                    ff->SetLabel( ObjNameForLabel + _T( "_LongDia" ) );
                    ff->SetTime( pDataFrame->GetTime() );
                    resVal->AddFrame( ff );
                    ff = CFigureFrame::Create();
                    CopyIdAndTime( ff , pDataFrame ) ;
                    ff->Attributes()->WriteString( "color" , Color ) ;
                    CDPoint ShortUp( Spot.m_UpShort.real() , Spot.m_UpShort.imag() ) ;
                    CDPoint ShortDown( Spot.m_DownShort.real() , Spot.m_DownShort.imag() ) ;
                    ff->AddPoint( ShortUp ) ;
                    ff->AddPoint( ShortDown ) ;
                    ff->SetLabel( ObjNameForLabel + _T( "_ShortDia" ) );
                    resVal->AddFrame( ff );
                  }
                }
                if (resVal && ( iObjViewMode & OBJ_VIEW_COORD ) && ( iMeasCnt == 0 ))
                {
                  FXString CoordView , AddInfo ;
                  if (Spot.m_dBlobWidth == 0)
                    Spot.m_dBlobWidth = Spot.m_OuterFrame.Width() + 1 ;
                  if (Spot.m_dBlobHeigth == 0)
                    Spot.m_dBlobHeigth = Spot.m_OuterFrame.Height() + 1 ;

                  cmplx CenterPos( Spot.m_SimpleCenter.x , Spot.m_SimpleCenter.y ) ;
                  FXString Units( _T( "pix" ) ) ;
                  if (iObjViewMode & OBJ_VIEW_SCALED)
                  {
                    CenterPos *= m_dScale_um_per_pix ;
                    if (!m_ScaleUnits.IsEmpty())
                      Units = m_ScaleUnits ;
                  }


                  if (!( Spot.m_WhatIsMeasured & MEASURE_ANGLE )
                    || !( iObjViewMode & OBJ_VIEW_ANGLE ))
                  {
                    CoordView.Format( "%6.2f\n%6.2f,%s" , CenterPos.real()
                      , CenterPos.imag() , ( LPCTSTR ) Units ) ;
                  }
                  else if (( Spot.m_WhatIsMeasured & MEASURE_ANGLE ) && ( iObjViewMode & OBJ_VIEW_ANGLE ))
                  {
                    CoordView.Format( "C(%6.2f,%6.2f)%s Th=%6.2f" , CenterPos.real()
                      , CenterPos.imag() , ( LPCTSTR ) Units , Spot.m_dAngle ,
                      ( Spot.m_dAccurateArea != 0.0 ) ?
                      Spot.m_dAccurateArea : ( double ) Spot.m_Area ,
                      Spot.m_iMinPixel , Spot.m_iMaxPixel ) ;
                  }
                  if (1)
                  {
                    if (Spot.m_dPerimeter != 0.)
                    {
                      AddInfo.Format( " Per=%6.2f" ,
                        Spot.m_dPerimeter ) ;
                      CoordView += AddInfo ;
                    }
                    double dWidth = ( Spot.m_WhatIsMeasured & MEASURE_DIAMETERS ) ?
                      Spot.m_dLongDiametr : Spot.m_dBlobWidth ;
                    double dHeight = ( Spot.m_WhatIsMeasured & MEASURE_DIAMETERS ) ?
                      Spot.m_dShortDiametr : Spot.m_dBlobHeigth ;
                    AddInfo.Format( "\nSz(%6.2f,%6.2f)%s S=%8.2f\nBr[%5d,%5d]" ,
                      dWidth * m_dScale_um_per_pix ,
                      dHeight * m_dScale_um_per_pix ,
                      ( !m_ScaleUnits.IsEmpty() ) ? ( LPCTSTR ) m_ScaleUnits : "pix" ,
                      ( Spot.m_dAccurateArea != 0.0 ) ?
                      Spot.m_dAccurateArea : ( double ) Spot.m_Area ,
                      Spot.m_iMinPixel , Spot.m_iMaxPixel ) ;
                    CoordView += AddInfo ;
                  }
                  if (iObjViewMode & OBJ_VIEW_DIFFR)
                  {
                    if (( Obj.m_WhatIsMeasured & MEASURE_DIFFRACT ) && Spot.m_dCentralIntegral > 0.)
                    {
                      double dSquareArea = 4. * Obj.m_DiffrRadius.cx * Obj.m_DiffrRadius.cy;
                      AddInfo.Format( " Pc=%6.2f Po=%6.2f\n(C%4.1f L%4.1f R%4.1f U%4.1f D%4.1f)" ,
                        Spot.m_dIntegrals[ 4 ] / dSquareArea ,
                        ( Spot.m_dSumPower - Spot.m_dIntegrals[ 4 ] ) / ( 8. * dSquareArea ) ,
                        Spot.m_dCentralIntegral * 100. ,
                        Spot.m_dLDiffraction * 100. , Spot.m_dRDiffraction * 100. ,
                        Spot.m_dUDiffraction * 100. , Spot.m_dDDiffraction * 100. ) ;
                      CoordView += AddInfo ;
                    }
                  }
                  if (!CoordView.IsEmpty())
                  {
                    CTextFrame * ViewText = CTextFrame::Create( CoordView ) ;
                    CopyIdAndTime( ViewText , pDataFrame ) ;
                    if (abs( Obj.m_ViewOffset.cx ) < 10000)
                    {
                      ViewText->Attributes()->WriteInt( "x" ,
                        ( int ) ( Spot.m_SimpleCenter.x + .5 ) + Obj.m_ViewOffset.cx );
                    }
                    else
                      ViewText->Attributes()->WriteInt( "x" , ( Obj.m_ViewOffset.cx % 10000 ) );
                    if (abs( Obj.m_ViewOffset.cy ) < 10000)
                    {
                      ViewText->Attributes()->WriteInt( "y" ,
                        ( int ) ( Spot.m_SimpleCenter.y - .5 ) + Obj.m_ViewOffset.cy );
                    }
                    else
                      ViewText->Attributes()->WriteInt( "y" , ( Obj.m_ViewOffset.cy % 10000 ) );

                    ViewText->Attributes()->WriteString( "color" , Color );
                    ViewText->Attributes()->WriteInt( "Sz" , Obj.m_iViewSize ) ;
                    ViewText->SetLabel( FXString( _T( "CoordView:" ) ) + ObjNameForLabel );
                    resVal->AddFrame( ViewText ) ;
                  }
                  if (( iObjViewMode & OBJ_VIEW_MRECTS ) && ( iMeasCnt == 0 )
                    && Obj.m_DiffrRadius.cx && Obj.m_DiffrRadius.cy)
                  {
                    CDPoint Pt = Spot.m_SimpleCenter
                      - ( CDPoint( Obj.m_DiffrRadius.cx , Obj.m_DiffrRadius.cy ) * 3 ) ;
                    CFigureFrame * pDiffrShow = CFigureFrame::Create() ;
                    CopyIdAndTime( pDiffrShow , pDataFrame ) ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.x += Obj.m_DiffrRadius.cx * 6 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.y += Obj.m_DiffrRadius.cy * 6 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.x -= Obj.m_DiffrRadius.cx * 6 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.y -= Obj.m_DiffrRadius.cy * 6 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.x += Obj.m_DiffrRadius.cx * 2 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.y += Obj.m_DiffrRadius.cy * 6 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.x += Obj.m_DiffrRadius.cx * 2;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.y -= Obj.m_DiffrRadius.cy * 6 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.x += Obj.m_DiffrRadius.cx * 2 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.y += Obj.m_DiffrRadius.cy * 2 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.x -= Obj.m_DiffrRadius.cx * 6 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.y += Obj.m_DiffrRadius.cy * 2 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    Pt.x += Obj.m_DiffrRadius.cx * 6 ;
                    pDiffrShow->AddPoint( Pt ) ;
                    pDiffrShow->Attributes()->WriteString( "color" , Color ) ;
                    pDiffrShow->SetLabel( ObjNameForLabel + _T( "_DiffrAreas" ) );
                    resVal->AddFrame( pDiffrShow ) ;
                  }
                }
                if (iObjViewMode & OBJ_VIEW_PROFX) // Show Profiles
                {

                }
                dBeforeConturView = GetHRTickCount() ;
                if (resVal && ( iObjViewMode & OBJ_VIEW_CONT )) // show contour borders
                {
                  if (Spot.m_Contur.GetCount())
                  {
                    FXString Label ;
                    Label.Format( _T( "Contur[%s_%d]" ) , ( LPCTSTR ) Obj.m_ObjectName , i ) ;
                    CFigureFrame * pContur = CreateFigureFrame(
                      Spot.m_Contur.GetData() ,
                      ( int ) Spot.m_Contur.GetCount() , GetHRTickCount() , Color ,
                      Label , pDataFrame->GetId() ) ;
                    pContur->SetTime( pDataFrame->GetTime() ) ;
                    resVal->AddFrame( pContur ) ;

                    //                       cmplx SCent( Spot.m_SimpleCenter.x , Spot.m_SimpleCenter.y ) ;
                    //                       CFigureFrame * pSimple = CreatePtFrame( SCent , GetHRTickCount() , Color  ) ;
                    //                       resVal->AddFrame( pSimple ) ;
                    //                       cmplx OutframeCenter( Spot.m_OuterFrame.CenterPoint().x ,
                    //                         Spot.m_OuterFrame.CenterPoint().y ) ;
                    //                       pSimple = CreatePtFrame( OutframeCenter , GetHRTickCount() , Color  ) ;
                    //                       pSimple->Attributes()->WriteInt( "thickness" , 2 ) ;
                    //                       resVal->AddFrame( pSimple ) ;
                  }
                }
                if (( pTextResult != NULL ) && ( !Obj.m_bMulti || bMultiCenter ))
                {
                  pTextResult->Release() ;
                  pTextResult = NULL ;
                }

                if (resVal)
                {
                  LPCTSTR pExtNames[] = { _T( "_RW" ) , _T( "_CW" ) , _T( "_CNW" ) } ;
                  for (int iOutCnt = 0 ; iOutCnt < 3 ; iOutCnt++)
                  {
                    FXString Label ;
                    if (!pTextResult)
                    {
                      pTextResult = CTextFrame::Create() ;
                      Label.Format( "Data_Spot:%s%s" , ( LPCTSTR ) ObjName ,
                        ( Obj.m_WhatToMeasure ) ? pExtNames[ iOutCnt ] : _T( "" ) ) ;
                      pTextResult->SetLabel( Label ) ;
                      pTextResult->ChangeId( pDataFrame->GetId() ) ;
                      pTextResult->SetTime( pDataFrame->GetTime() ) ;
                      pTextResult->GetString().Format( "\nSpots=%d Max=%.2f Min=%.2f Width=%d Height=%d T=%9.3f:\n" ,
                        ( Obj.m_pSegmentation ) ? ActiveIndexes.GetCount() : 0 ,
                        Obj.m_dMeasuredMax , Obj.m_dMeasuredMin ,
                        VideoFrame->lpBMIH->biWidth , VideoFrame->lpBMIH->biHeight ,
                        dObjMeasFinished - dBeginProcessingTime ) ;
                    }
                    //                    double dAngle = Spot.m_dAngles[iOutCnt] ; // Assume, that angle for rectangle on index 0
                    double dAngle = Spot.m_dAngle ; // !!!!!!!!!!!!!!!!!!!!!Not good for multi threshold.  Moisey 31.01.18
                    cmplx Cent = Spot.m_Centers[ iOutCnt ] ;  // The same assumption
                    FXString ResultAsText ;
                    if (Obj.m_WhatToMeasure /*|| Obj.m_bMulti*/)
                    {
                      //                         ResultAsText.Format(
                      iIndex += sprintf_s( Buf + iIndex , ( sizeof( Buf ) - iIndex ) ,
                        "%6d  %7.2f %7.2f %6.2f %6.2f %7d %6d %8.1f %8.1f %7.2f %7.2f %7.2f "
                        "%6.2f %6.2f %6.2f %6.2f %6.2f %5d %5d %5d %5d %5d %5d Name=%s; \n" ,
                        i ,
                        Spot.m_SimpleCenter.x , Spot.m_SimpleCenter.y ,
                        Spot.m_dBlobWidth ,
                        Spot.m_dBlobHeigth ,
                        Spot.m_Area ,
                        Spot.m_iMaxPixel ,
                        Spot.m_dCentral5x5Sum ,
                        Spot.m_dSumOverThreshold ,
                        dAngle ,
                        Spot.m_dLongDiametr ,
                        Spot.m_dShortDiametr ,
                        100. *Spot.m_dRDiffraction ,
                        100. *Spot.m_dLDiffraction ,
                        100. *Spot.m_dDDiffraction ,
                        100. *Spot.m_dUDiffraction ,
                        100. *Spot.m_dCentralIntegral ,
                        Spot.m_OuterFrame.left ,
                        Spot.m_OuterFrame.top ,
                        Spot.m_OuterFrame.right ,
                        Spot.m_OuterFrame.bottom ,
                        VideoFrame->lpBMIH->biWidth ,
                        VideoFrame->lpBMIH->biHeight ,
                        ( LPCTSTR ) ObjName
                      ) ;
                      if (iIndex > sizeof( Buf ) - 400)
                      {
                        pTextResult->GetString() += Buf /*(LPCTSTR) ResultAsText*/ ;
                        iIndex = 0 ;
                      }
                      if (( m_iCaptionPeriod > 0 ) && ( i == 0 ))
                      {
                        if (( Obj.m_iResultsCounter++ % m_iCaptionPeriod ) == 0)
                        {
                          //                       if ( m_pDescriptionOutput->IsConnected() )
                          //                       {
                          FXString ResultsOrder(
                            _T( /*",Spots,,Max,,Min,,FrameWidth,,FrameHeight,,Tmeas,"*/
                              "//Spot#,   Xc,     Yc,   Width,  Height,  Area,"
                              "MaxInten,CentSum,OverThr, Angle,"
                              " Dlong, Dshort, EnergyR,  EnL,"
                              "  EnT,   EnB,  EnC,"
                              " Left,  Top,Right,Bottom,FrWidth,FrHeight;" ) ) ;
                          pTextResult->GetString() += ResultsOrder += _T( '\n' ) ;
//                             CDataFrame * pDescription = FormDescription(
//                               ResultsOrder , pTextResult->GetLabel() , pDataFrame->GetId() ) ;
//                             if (pDescription)
//                             {
//                               pDescription->SetTime( pDataFrame->GetTime() ) ;
//                               resVal->AddFrame( pDescription ) ;
//                             }
                          //                       }
                        }
                      }
                    }
                    else
                    {
                      //                         ResultAsText.Format(
                      iIndex += sprintf_s( Buf + iIndex , ( sizeof( Buf ) - iIndex ) ,
                        "%d %7.2f %7.2f %d %d %d %d %d %d %d %d %d Name=%s;\n" ,
                        i ,
                        Spot.m_SimpleCenter.x ,
                        Spot.m_SimpleCenter.y ,
                        Spot.m_OuterFrame.Width() ,
                        Spot.m_OuterFrame.Height() ,
                        Spot.m_Area ,
                        Spot.m_OuterFrame.left ,
                        Spot.m_OuterFrame.top ,
                        Spot.m_OuterFrame.right ,
                        Spot.m_OuterFrame.bottom ,
                        VideoFrame->lpBMIH->biWidth ,
                        VideoFrame->lpBMIH->biHeight ,
                        ( LPCTSTR ) ObjName
                      ) ;
                      //                         pTextResult->GetString() += (LPCTSTR) ResultAsText ;
                      if (iIndex > sizeof( Buf ) - 400)
                      {
                        pTextResult->GetString() += Buf /*(LPCTSTR) ResultAsText*/ ;
                        iIndex = 0 ;
                      }
                      if (( m_iCaptionPeriod > 0 ) && ( i == 0 ))
                      {
                        if (( Obj.m_iResultsCounter++ % m_iCaptionPeriod ) == 0)
                        {
                          FXString ResultsOrder(
                            _T( ",Spots,,Max,,Min,,FrameWidth,,FrameHeight,,Tmeas"
                              "Spot#,Xc,Yc,Width,Height,Area,"
                              "Left,Top,Right,Bottom,FrameWidth,FrameHeight;" ) ) ;
                          CDataFrame * pDescription = FormDescription(
                            ResultsOrder , pTextResult->GetLabel() , pDataFrame->GetId() ) ;
                          if (pDescription)
                          {
                            pDescription->SetTime( pDataFrame->GetTime() ) ;
                            resVal->AddFrame( pDescription ) ;
                          }
                        }
                      }
                    }
                    if (!Obj.m_bMulti || bMultiCenter)
                    {
                      if (iIndex)
                      {
                        pTextResult->GetString() += Buf /*(LPCTSTR) ResultAsText*/ ;
                        iIndex = 0 ;
                      }
                      resVal->AddFrame( pTextResult ) ;
                      pTextResult = NULL ;
                    }
                    if (!bMultiCenter)
                      break ; // there is no additional methods for angle measurement
                    else if (iOutCnt > 0)
                    {
                      cmplx VectToSide = polar( 20. , -DegToRad( Spot.m_dAngles[ iOutCnt ] ) ) ;
                      cmplx Pt1 = Spot.m_Centers[ iOutCnt ] + VectToSide ;
                      cmplx Pt2 = Spot.m_Centers[ iOutCnt ] - VectToSide ;
                      FXString Label ;
                      Label.Format( _T( "Center1_%s" ) , ( iOutCnt == 1 ) ? _T( "CW" ) : _T( "CNW" ) ) ;
                      DWORD dwColor = Obj.m_ViewColor ^ ( 0x0000ff << ( ( iOutCnt - 1 ) * 8 ) ) ;
                      FXString CrossColor ;
                      CrossColor.Format( _T( "%06X" ) , dwColor ) ;
                      CFigureFrame * pCenterCross = CreateLineFrame( Pt1 , Pt2 ,
                        CrossColor , Label , pDataFrame->GetId() ) ;
                      resVal->AddFrame( pCenterCross ) ;

                      VectToSide *= cmplx( 0. , 1. ) ; // rotate for 90 degrees

                      cmplx Pt3 = Spot.m_Centers[ iOutCnt ] + VectToSide ;
                      cmplx Pt4 = Spot.m_Centers[ iOutCnt ] - VectToSide ;
                      Label.Format( _T( "Center2_%s" ) , ( iOutCnt == 1 ) ? _T( "CW" ) : _T( "CNW" ) ) ;
                      pCenterCross = CreateLineFrame( Pt3 , Pt4 ,
                        CrossColor , Label , pDataFrame->GetId() ) ;
                      resVal->AddFrame( pCenterCross ) ;
                    }
                  }
                }
                if ( Obj.m_WhatIsMeasured & MEASURE_IMG_MOMENTS_W 
                  && resVal && ( iObjViewMode & OBJ_OUT_MOMENTS ) && Spot.m_ImgMomentsWeighted.m_dM00 )
                {
                  TmpForMoments.Format( "#=%6d,M00=%12g,M01=%12g,M10=%12g,M11=%12g,M02=%12g,M20=%12g\n" ,
                    i ,
                    Spot.m_ImgMomentsWeighted.m_dM00 ,
                    Spot.m_ImgMomentsWeighted.m_dM01 ,
                    Spot.m_ImgMomentsWeighted.m_dM10 ,
                    Spot.m_ImgMomentsWeighted.m_dM11 ,
                    Spot.m_ImgMomentsWeighted.m_dM02 ,
                    Spot.m_ImgMomentsWeighted.m_dM20 ) ;
                  ForMomentsOut += TmpForMoments ;
                }
              }  // measured spot iterations
              if (iIndex)
              {
                pTextResult->GetString() += Buf /*(LPCTSTR) ResultAsText*/ ;
                iIndex = 0 ;
              }
              if (pTextResult)
              {
                if (resVal)
                  resVal->AddFrame( pTextResult ) ;
                pTextResult = NULL ;
              }
              if ( !ForMomentsOut.IsEmpty() )
              {
                CTextFrame * pMomentsOut = CreateTextFrame( ( LPCTSTR ) ForMomentsOut , "WeightedMoments" , pDataFrame->GetId() ) ;
                resVal->AddFrame( pMomentsOut ) ;
              }

              Obj.m_Timing.push_back( GetHRTickCount() - dObjMeasFinished ) ;
              double dNow = GetHRTickCount() ;
              FXString Diag ;
              Diag.Format( "Obj=%s: Simple=%.3f Edge=%.3f Final=%.3f Out=%.3f Full=%.3f #Pts=%d FromCapt=%.3f" ,
                ( LPCTSTR ) Obj.m_ObjectName ,
                ( Obj.m_Timing.size() ) ? Obj.m_Timing[ TI_Prepare ] : 0. ,
                ( Obj.m_Timing.size() >= TI_Segmentation + 1 ) ?
                Obj.m_Timing[ TI_Segmentation ] : 0. ,
                //                 ( Obj.m_EndOfFinalProcessing.GetCount() ) ?
                //                 Obj.m_EndOfFinalProcessing[ 0 ] - dBeginProcessingTime : 0. ,
                //                 dBeforeConturView - dBeginProcessingTime ,
                ( Obj.m_Timing.size() >= TI_ConturDiametersDetailed + 1 ) ?
                Obj.m_Timing[ TI_ConturDiametersDetailed ] : 0. ,
                ( Obj.m_Timing.size() >= TI_FormOutput + 1 ) ?
                Obj.m_Timing[ TI_FormOutput ] : 0. ,
                dNow - dObjectBeginProcTime_ms ,
                ( int ) ActiveIndexes.GetCount() ,
                dNow - pDataFrame->GetAbsTime()
              ) ;
              CTextFrame * pDiag = CreateTextFrame( Diag , "TVObjectsDiag" , pDataFrame->GetId() ) ;
              PutFrame( m_pDiagOut , pDiag ) ;
            }
            else if (resVal && ( ( Obj.m_Type == LINE_SEGMENT )
              || ( ( Obj.m_Type == EDGE ) && !( Obj.m_WhatToMeasure & MEASURE_EDGE_AS_CONTUR ) ) ))
            {
              bool bIsVertical = ( Obj.m_Direction == VERTICAL 
                || Obj.m_Direction == DIR_03 || Obj.m_Direction == DIR_09 ) ;
              CLineResult FirstLine ;
              for (int i = 0 ; i < Obj.m_LineResults.GetCount() ; i++)
              {
                FXString ObjNameForLabel ;
                ObjNameForLabel.Format( "%s_%d" , ( LPCTSTR ) ObjName , i ) ;
                CFigureFrame* ff = CFigureFrame::Create();
                ff->ChangeId( pDataFrame->GetId() );
                ff->SetTime( pDataFrame->GetTime() ) ;
                if (iObjViewMode & OBJ_VIEW_POS)
                  ff->Attributes()->WriteString( "color" , Color ) ;
                CLineResult Line = Obj.m_LineResults[ i ] ;
                if (i == 0)
                  FirstLine = Line ;
                ff->AddPoint( Line.m_Center );
                //               FXString Label ;
                //               Label.Format("%s[%d]" , (LPCTSTR)ObjName , i ) ;
                //               ff->SetLabel( Label );
                ff->SetLabel( ObjNameForLabel );
                ff->SetTime( pDataFrame->GetTime() );
                resVal->AddFrame( ff );
                if (iObjViewMode & OBJ_VIEW_COORD)
                {
                  cmplx CenterPos( Line.m_Center.x , Line.m_Center.y ) ;
                  double dThickness = bIsVertical ? Line.m_DRect.Width() : Line.m_DRect.Height() ;
                  if ( iObjViewMode & OBJ_VIEW_SCALED )
                    dThickness *= m_dScale_um_per_pix ;
                  FXString Units( _T( "pix" ) ) ;
                  if (iObjViewMode & OBJ_VIEW_SCALED)
                  {
                    CenterPos *= m_dScale_um_per_pix ;
                    Units = ( !m_ScaleUnits.IsEmpty() ) ? m_ScaleUnits : _T( "pix" ) ;
                  }

                  FXString CoordView ;
                  switch ( Obj.m_Type )
                  {
                  case LINE_SEGMENT: 
                    CoordView.Format( "X%6.2f\nY%6.2f\nW=%.2f%s" ,
                      CenterPos , dThickness , ( LPCTSTR ) Units ) ;
                    break ;
                  case EDGE:
                    CoordView.Format( "X%6.2f\nY%6.2f" , CenterPos ) ;
                    break ;
                  }
                  CTextFrame * ViewText = CTextFrame::Create( CoordView ) ;
                  ViewText->Attributes()->WriteInt( "x" ,
                    ( int ) ( Line.m_Center.x + .5 ) + Obj.m_ViewOffset.cx );
                  ViewText->Attributes()->WriteInt( "y" ,
                    ( int ) ( Line.m_Center.y - .5 ) + Obj.m_ViewOffset.cy );
                  ViewText->Attributes()->WriteString( "color" , Color );
                  ViewText->Attributes()->WriteInt( "Sz" , Obj.m_iViewSize ) ;
                  ViewText->SetLabel( FXString( _T( "CoordView:" ) ) + ObjNameForLabel );
                  CopyIdAndTime( ViewText , pDataFrame ) ;
                  resVal->AddFrame( ViewText ) ;
                  if (Obj.m_LineResults.GetCount() == 2 && i == 1) // second line from two lines
                  {
                    double dDist = ( Line.m_Center.x == FirstLine.m_Center.x ) ?
                      ( Line.m_Center.y - FirstLine.m_Center.y )
                      : ( Line.m_Center.x - FirstLine.m_Center.x ) ;
                    CoordView.Format( "%7.2f %s" , fabs( dDist ) * m_dScale_um_per_pix ,
                      ( !m_ScaleUnits.IsEmpty() ) ? ( LPCTSTR ) m_ScaleUnits : "pix" ) ;
                    ViewText = CTextFrame::Create( CoordView ) ;
                    ViewText->Attributes()->WriteInt( "x" ,
                      ( int ) ( Line.m_Center.x + .5 ) + Obj.m_ViewOffset.cx );
                    ViewText->Attributes()->WriteInt( "y" , 10 );
                    ViewText->Attributes()->WriteString( "color" , Color );
                    ViewText->Attributes()->WriteInt( "Sz" , Obj.m_iViewSize * 2 ) ;
                    ViewText->SetLabel( FXString( _T( "DistView:" ) ) + ObjName );
                    CopyIdAndTime( ViewText , pDataFrame ) ;
                    resVal->AddFrame( ViewText ) ;
                  }
                }
                //               if ( Obj.m_WhatToMeasure || Obj.m_bMulti )
                //               {
                if (Obj.m_Type == LINE_SEGMENT)
                {
                  if (pTextResult == NULL)
                  {
                    pTextResult = CTextFrame::Create() ;
                    FXString Label ;
                    Label.Format( "Data_Line:%s" , ( LPCTSTR ) ObjName ) ;
                    CopyIdAndTime( pTextResult , pDataFrame ) ;
                    pTextResult->SetLabel( Label ) ;
                    pTextResult->GetString().Format( "Lines=%d Max=%8.2f Min=%8.2f:\n" ,
                      Obj.m_LineResults.GetCount() ,
                      Obj.m_dMeasuredMax , Obj.m_dMeasuredMin ) ;
                  }
                  if (( m_iCaptionPeriod > 0 ) && ( i == 0 ))
                  {
                    if (( Obj.m_iResultsCounter++ % m_iCaptionPeriod ) == 0)
                    {
                      pTextResult->GetString() += Line.GetCaption() ;
                    }
                  }
                  FXString ResultAsText ;
                  Line.ToString( ResultAsText , ( LPCTSTR ) ObjName ) ;
                  pTextResult->GetString() += ( LPCTSTR ) ResultAsText ;
                }
                else if (Obj.m_Type == EDGE)
                {
                  if (pTextResult == NULL)
                  {
                    pTextResult = CTextFrame::Create() ;
                    CopyIdAndTime( pTextResult , pDataFrame ) ;
                    FXString Label ;
                    Label.Format( "Data_Edge:%s" , ( LPCTSTR ) ObjName ) ;
                    pTextResult->SetLabel( Label ) ;
                    pTextResult->GetString().Format( "Edges=%d Max=%8.2f Min=%8.2f:\n" ,
                      Obj.m_LineResults.GetCount() ,
                      Obj.m_dMeasuredMax , Obj.m_dMeasuredMin ) ;
                  }
                  FXString ResultAsText ;
                  ResultAsText.Format( "%d %7.2f %7.2f %d %d %d %d Name=%s; \n" ,
                    i ,
                    Line.m_Center.x , Line.m_Center.y ,
                    //                    Obj.m_InFOVPosition.x , Obj.m_InFOVPosition.y ,
                    Obj.m_absAOS.left , Obj.m_absAOS.right ,
                    Obj.m_absAOS.top , Obj.m_absAOS.bottom ,
                    ( LPCTSTR ) ObjName
                  ) ;
                  pTextResult->GetString() += ( LPCTSTR ) ResultAsText ;
                  if (( m_iCaptionPeriod > 0 ) && ( i == 0 ))
                  {
                    if (Obj.m_iResultsCounter++ % m_iCaptionPeriod == 0)
                    {
                      FXString ResultsOrder(
                        _T( "#,Edges,,Max,,Min,,Xcent,Ycent,MaxAmpl,AverCent,,,,,MinINtens,MaxIntens" ) ) ;
                      CDataFrame * pDescription = FormDescription(
                        ResultsOrder , pTextResult->GetLabel() , pDataFrame->GetId() ) ;
                      if (pDescription)
                      {
                        CopyIdAndTime( pDescription , pDataFrame ) ;
                        resVal->AddFrame( pDescription ) ;
                      }
                    }
                  }
                }
              }
              //             }
              if (pTextResult)
              {
                resVal->AddFrame( pTextResult ) ;
                pTextResult = NULL ;
              }
            }  // LINE_SEGMENT
          }  // if (Obj.m_WhatIsMeasured & MEASURE_POSITION)
          if (Obj.m_Type == EDGE && ( Obj.m_WhatToMeasure & MEASURE_EDGE_AS_CONTUR )
            && Obj.m_InternalSegment.Count())
          {
            FXString Label ;
            Label.Format( _T( "EdgeSegment[%s]" ) , ( LPCTSTR ) Obj.m_ObjectName ) ;
            CFigureFrame * edge = CreateFigureFrame( Obj.m_InternalSegment.GetData() ,
              ( int ) Obj.m_InternalSegment.Count() , ( DWORD ) 0x00ff00 , ( LPCTSTR ) Label ) ;
            resVal->AddFrame( edge ) ;
          }
          if (resVal && ( Obj.m_WhatIsMeasured & MEASURE_TEXT ))
          {
            CTextFrame* ff = CTextFrame::Create();
            CopyIdAndTime( ff , pDataFrame ) ;
            ff->SetString( Obj.m_TextResult );
            ff->ChangeId( pDataFrame->GetId() );
            ff->SetLabel( ObjName );
            ff->SetTime( pDataFrame->GetTime() );
            if (Obj.m_dwViewMode & OBJ_VIEW_TEXT)  // view recognition result on renderer
            {
              ff->Attributes()->WriteInt( "x" , ROUND( Obj.m_absAOS.TopLeft().x + Obj.m_ViewOffset.cx ) );
              ff->Attributes()->WriteInt( "y" , ROUND( Obj.m_absAOS.TopLeft().y + Obj.m_ViewOffset.cy ) );
              ff->Attributes()->WriteString( "color" , Color );
              ff->Attributes()->WriteInt( "Sz" , Obj.m_iViewSize ) ;
            }
            resVal->AddFrame( ff );
          }
          if (pTextResult)
          {
            pTextResult->Release( pTextResult ); // this causes the memory leaks. 
            pTextResult = NULL ;               // but if you comment in - it doesn't work properly.
          }
          if (pSetup && ( pSetup->m_hWnd )					//The dialog is open
            && ( pSetup->m_bSelectRenderer == TRUE ) 	//Selecting on renderer is on
            && ( pSetup->m_nAutoFind == 0 )				    //Auto find mode was selected
            && ( pSetup->m_SelectedOnRenderObjName == ( LPCTSTR ) ObjName )
            )
          {
            ////////////////////////////////////////////////////////////////////
            //TBD : Change the m_WhatIsMeasured condition to a boolean variable 
            //which symbolizes whether the object was successfully found
            //(right now - bMeasResult is sometimes true when search fails and false when successful
            ////////////////////////////////////////////////////////////////////

            //checking if the automatic search was successful.
            if (Obj.m_WhatIsMeasured & ( MEASURE_POSITION | MEASURE_TEXT ))				// object found
              pSetup->m_nAutoFind = 1;		//Points that the automatic search succeeded for this object.
            else
              pSetup->m_nAutoFind = 2;		//Points that the automatic search failed for this object.

          }
          if (resVal && ( Obj.m_WhatToMeasure & MEASURE_PROFILE & Obj.m_WhatIsMeasured ))
          { // send profile result
            pProfile pProf ;
            int iLen = 0 ;
            if (Obj.m_Type == SPOT
              || Obj.m_Direction == HORIZONTAL
              || Obj.m_Direction == ANY_DIRECTION
              || Obj.m_Direction == DIR_UD
              || Obj.m_Direction == DIR_DU)
            {
              {
                double dAmpl = Obj.m_absAOS.Width() ;
                pProf = &Obj.m_VProfile ;
                if (dAmpl > 0.01  && pProf && (Obj.m_dwViewMode & OBJ_VIEW_PROFY)
                  && ( m_ViewModeOption.getbDispProfY() == TRUE ))
                {
                  iLen = pProf->m_iProfLen ;
                  double dMin = pProf->m_dMinValue ;
                  double dMax = pProf->m_dMaxValue ;
                  double dScale = ( dMax - dMin ) / dAmpl ;
                  if (dScale > 0.01)
                    dScale = 1. / dScale ;
                  else
                    dScale = 0 ;
                  double dOriginX = Obj.m_absAOS.right ;
                  if (dOriginX + dAmpl > VideoFrame->lpBMIH->biWidth)
                  {
                    if (Obj.m_absAOS.left - dAmpl > 0)
                      dOriginX = ( int ) ( Obj.m_absAOS.left - dAmpl ) ; // put profile on the left side
                    else
                      dOriginX = Obj.m_absAOS.left ; // put profile on the object
                  }
                  double dOriginY = Obj.m_absAOS.top ;
                  if (iLen > 1)
                  {
                    CFigureFrame * pFrame = CFigureFrame::Create() ;
                    CopyIdAndTime( pFrame , pDataFrame ) ;
                    pFrame->Attributes()->WriteString( "color" , Color ) ;
                    pFrame->SetLabel( ObjName + "_ProfileY" );
                    for (int i = 1 ; i < iLen ; i++)
                    {
                      CDPoint NextPoint(
                        dOriginX + ( dScale * ( pProf->m_pProfData[ i ] - dMin ) ) , dOriginY + i ) ;
                      pFrame->Add( NextPoint ) ;
                    }
                    resVal->AddFrame( pFrame ) ;
                  }
                }
              }
            }
            if (Obj.m_Type == SPOT
              || Obj.m_Direction == VERTICAL
              || Obj.m_Direction == ANY_DIRECTION
              || Obj.m_Direction == DIR_LR
              || Obj.m_Direction == DIR_RL)
            {
              double dAmpl = Obj.m_absAOS.Height() ;
              pProf = &Obj.m_HProfile ;
              if (dAmpl > 0.01  && pProf && ( Obj.m_dwViewMode & OBJ_VIEW_PROFX )
                && ( m_ViewModeOption.getbDispProfX() == TRUE ))
              {
                iLen = pProf->m_iProfLen ;
                double dMin = pProf->m_dMinValue ;
                double dMax = pProf->m_dMaxValue ;
                double dScale = ( dMax - dMin ) / dAmpl ;
                if (dScale > 0.01)
                  dScale = 1. / dScale ;
                else
                  dScale = 0 ;
                double dOriginX = Obj.m_absAOS.left/* + 1.*/ ;
                double dOriginY = Obj.m_absAOS.top - 1 /*Obj.m_absAOS.Height()*/ ;
                if (dOriginY < Obj.m_absAOS.Height())
                {
                  if (Obj.m_absAOS.bottom + dAmpl < VideoFrame->lpBMIH->biHeight)
                    dOriginY = ( int ) ( Obj.m_absAOS.bottom + dAmpl + 1 ) ; // put profile on the bottom side
                  else
                    dOriginY = Obj.m_absAOS.bottom ; // put profile on the object
                }
                if (iLen > 1)
                {
                  CFigureFrame * pFrame = CFigureFrame::Create() ;
                  CopyIdAndTime( pFrame , pDataFrame ) ;
                  pFrame->Attributes()->WriteString( "color" , Color ) ;
                  pFrame->SetLabel( ObjName + "_ProfileX" );
                  for (int i = 1 ; i < iLen/* - 3 */; i++)
                  {
                    CDPoint NextPoint(
                      dOriginX + i , dOriginY - ( dScale * ( pProf->m_pProfData[ i ] - dMin ) ) ) ;
                    pFrame->Add( NextPoint ) ;
                  }
                  resVal->AddFrame( pFrame ) ;
                }
              }
            }
          }
        }
        else
        {
          if (resVal && !Obj.m_Status.IsEmpty())
          {
            CTextFrame * pStatusOut = CreateTextFrame(
              cmplx( ( double ) Obj.m_absAOS.left , ( double ) Obj.m_absAOS.top ) ,
              Obj.m_Status , ( LPCTSTR ) Color , 16 ,  // 16 - text size
              "ObjMeasError" , pDataFrame->GetId() ) ;
            resVal->AddFrame( pStatusOut ) ;
          }
        }
      }
    }
  }
  // All data is sent, clean allocated memory
  for (int i = 0; i <= m_CurrentJob.GetUpperBound(); i++)
  {
    int iObjId = m_CurrentJob.GetAt( i ).m_ObjectID ;
    if (iObjId >= 0)
    {
      CVideoObject& Obj = m_pObjects->GetAt( iObjId ) ;
      Obj.ClearData() ;
    }
  }

  if (pSetup)
  {
    if (pSetup->m_bBinaryOutput)
    {
      if (pBinaryResult)
      {
        CopyIdAndTime( pBinaryResult , pDataFrame ) ;
        pOutputContainer->AddFrame( pBinaryResult ) ;
      }
    }
    else
    {
      if (pBinaryResult)
        pBinaryResult->Release() ;
      if (resVal)
        pOutputContainer->AddFrame( resVal );
      if (roiVal)
        pOutputContainer->AddFrame( roiVal );
      // the next is removed from DoJob by Moisey
      CQuantityFrame* pQuanFrame = CQuantityFrame::Create( 0 ); // Moisey 23NOV2010 
      //adding label TVObject for DoJob in GenericScriptGadget
      pQuanFrame->SetLabel( "TVObject" );   // shifriss 220610
      CopyIdAndTime( pQuanFrame , pDataFrame ) ;
      pOutputContainer->AddFrame( pQuanFrame );       // shifriss 220610
    }
  }
  // Following is for ExtractData... debugging
//   SpotVectors ExtractedSpots ;
//   NamedCmplxVectors Profiles ;
// 
//   ExtractDataAboutSpots( ( const CDataFrame* ) pOutputContainer , ExtractedSpots , Profiles ) ;
  int iFirst = -1 , iSecond = -1 ;
  CheckContainer( pOutputContainer , iFirst , iSecond ) ;
  return pOutputContainer;
}

bool TVObjects::LoadTemplate( FXString * pTemplate ,
  VOArray **pObjects , CVOJobList& Jobs , int& iActiveTask )
{
  if (bTraceObjects)
    TRACE( "+++ It's about to load template\n" );

  bool bResult = true ;
  FXParser parser( *pTemplate ) , temp;
  FXString     key;
  int i = 0;

  Jobs.RemoveAll();
  //   m_ExistentTasks.RemoveAll() ;
  VOArray * pNewObjects = new VOArray ;

  while (parser.GetElementNo( i , key , temp ))
  {
    temp.Remove( _T( ' ' ) ) ;
    temp.Remove( _T( '\t' ) ) ;
    CVideoObject vo( m_iNObjectsMax , m_dTimeout );
    if (bTraceObjects)
      TRACE( "\tNew object description for %s\n" , key );
    if (key.CompareNoCase( "line" ) == 0)
      vo.m_Type = LINE_SEGMENT;
    else if (key.CompareNoCase( "spot" ) == 0)
      vo.m_Type = SPOT;
    else if (key.CompareNoCase( "edge" ) == 0)
      vo.m_Type = EDGE;
    else if (key.CompareNoCase( "ocr" ) == 0)
      vo.m_Type = OCR ;

    if (vo.m_Type != NONE)
    {
      bResult &= vo.InitVideoObject( key , temp.MakeLower() );
      vo.m_pVObjects = pNewObjects ;
      vo.m_pVOArrayLock = &m_Lock ;
      pNewObjects->Add( vo );
    }
    else if (key.CompareNoCase( "task" ) == 0)
    {
      FXSIZE pos = 0;
      FXString word;
      int     listid;
      CVOJob  job;
      CVOTask task;
      if (temp.GetWord( pos , word ))
        listid = atoi( word );
      else
      {
        SENDERR_0( "Error: task isn't defined" );
        bResult = false ;
        continue;
      }
      //       m_ExistentTasks.Add( listid ) ;
      while (temp.GetWord( pos , word ))
      {
        task.m_ObjectID = -1;
        task.m_ObjectName = word;
        job.Add( task );
      }
      if (job.GetUpperBound() < 0)
      {
        SENDERR_1( "Error: didn't defined objects for task #%d" , listid );
        bResult = false ;
        break;
        //continue;
      }
      Jobs.SetAt( listid , job );
    }
    else if (key.CompareNoCase( "ActiveTask" ) == 0)
    {
      FXSIZE pos = 0;
      FXString word;
      int     iMayBeTask = -2 ;
      if (temp.GetWord( pos , word ))
      {
        iMayBeTask = atoi( word );
        if (iMayBeTask >= -1)
          iActiveTask = iMayBeTask ;
        else
        {
          SENDERR_1( "Error: Bad Active task number %d" , iMayBeTask );
          bResult = false ;
          continue;
        }
      }
      else
      {
        SENDERR_0( "Error: Active task isn't properly defined" );
        bResult = false ;
        continue;
      }
    }
    else
    {
      SENDERR_1( "Unknown key '%s' in template description" , key );
      bResult = false ;
    }
    i++;
  }
  bResult &= Link( pNewObjects , Jobs );

  *pObjects = pNewObjects ;
  return bResult ;
}

bool TVObjects::LoadTemplate( FXString * pTemplate )
{
  //   m_Lock.Lock();
  if (bTraceObjects)
    TRACE( "+++ It's about to load template\n" );

  FXString * pTemp = pTemplate ? pTemplate : &m_Template ;
  m_LockModify.Lock() ;
  VOArray * pNewObjects = NULL ;
  int iActiveTask = -2 ;
  LoadTemplate( pTemp , &pNewObjects , m_Jobs , iActiveTask ) ;

  if (m_pNewObjects) // DoProcessing did not use previous configuration
    delete m_pNewObjects ;
  m_pNewObjects = pNewObjects ; // set new objects
  m_NewJob.RemoveAll() ;
  if (iActiveTask >= -1)
    m_CurrentJobID = iActiveTask ;

  if (pTemplate)
    m_Template = *pTemplate ;
  if (!m_Jobs.Lookup( m_CurrentJobID , m_NewJob ))
    m_CurrentJobID = -1 ;

  m_LockModify.Unlock();
  if (bTraceObjects)
    TRACE( "+++ End loading template\n" );
  return true;
}

bool TVObjects::Link( VOArray * pObjects , CVOJobList& Jobs )
{
  return ::Link( pObjects , Jobs ) ;
}

bool TVObjects::Link( VOArray * pObjects )
{
  return Link( pObjects , m_Jobs ) ;
}

CDataFrame * TVObjects::FormDescription( LPCTSTR szDescription , LPCTSTR szLabel , DWORD Id )
{
  //   if ( m_pDescriptionOutput->IsConnected() )
  //   {
  //     CTextFrame * pOrderAsText = CTextFrame::Create( szDescription ) ;
  //     if ( pOrderAsText )
  //     {
  //       pOrderAsText->SetLabel( 
  //         FXString( _T( "ResultsOrder:" )) + szLabel) ;
  //       pOrderAsText->ChangeId( Id ) ;
  //       if ( !m_pDescriptionOutput->Put( pOrderAsText ) )
  //         pOrderAsText->Release( pOrderAsText ) ;
  //     }
  //   }
  //   else
  //   {
  //     CTextFrame * pDummy = CreateTextFrame( 
  //       _T("Dummy, No Connections") , Id ) ;
  //     if ( !m_pDescriptionOutput->Put( pDummy ) )
  //       pDummy->Release( pDummy ) ;
  //   }
  CTextFrame * pOrderAsText = CTextFrame::Create( szDescription ) ;
  if (pOrderAsText)
  {
    pOrderAsText->SetLabel(
      FXString( _T( "ResultsOrder:" ) ) + szLabel ) ;
    pOrderAsText->ChangeId( Id ) ;
  }
  return pOrderAsText ;
}

bool TVObjects::GetObjectPosByName( LPCTSTR ObjName , CRect& Pos , FXString& AnchorObject )
{
  FXAutolock al( m_Lock ) ;

  for (int i = 0 ; i < m_pObjects->GetCount() ; i++)
  {
    CVideoObject& VO = m_pObjects->GetAt( i ) ;
    if (VO.m_ObjectName == ObjName)  // necessary object 
    {
      Pos = VO.m_absAOS ;
      AnchorObject = VO.m_LeaderName ;
      return true ;
    }
  }
  return false ;
}

