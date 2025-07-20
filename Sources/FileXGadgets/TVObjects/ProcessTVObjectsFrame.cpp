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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "ProcessTVObjectsFrame"

IMPLEMENT_RUNTIME_GADGET_EX( ProcessTVObjectsFrame , CFilterGadget , "Video.recognition" , TVDB400_PLUGIN_NAME );

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
  if (!(vfr) || ((vfr)->IsNullFrame()))	    \
{	                                        \
  fr->RELEASE(fr);						\
  return vfr;			                    \
}                                           \
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


ProcessTVObjectsFrame::ProcessTVObjectsFrame()
{
  m_pInput = new CInputConnector( transparent );
  m_pOutput  = new COutputConnector( text );
  m_iFramesCounter = 0 ;
  Resume();
}

ProcessTVObjectsFrame::~ProcessTVObjectsFrame()
{
} ;


//ProcessTVObjectsFrameGadget::~ProcessTVObjectsFrameGadget()
void ProcessTVObjectsFrame::ShutDown()
{
  //ShutDown();
  CGadget::ShutDown();
  CDataFrame * pFr ;
  while ( m_pInput->Get( pFr ) )
    pFr->Release( pFr )  ;

  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

void ProcessTVObjectsFrame::OnEOS()
{ 
  CFilterGadget::OnEOS() ;
  m_iFramesCounter = 0 ; 
  m_bRun = true ; 
} ;

// void ProcessTVObjectsFrame::OnStop()
// { 
//   CDataFrame* pFrame=CDataFrame::Create();
// 
//   Tvdb400_SetEOS(pFrame);
// 
//   COutputConnector* pOutput = (COutputConnector*)m_pOutput;
//   if ( !pOutput  ||  !pOutput->Put(pFrame) )
//     pFrame->Release( pFrame ); 
// 
//   m_bRun = false ; 
// } ;


CDataFrame* ProcessTVObjectsFrame::DoProcessing( const CDataFrame* pDataFrame )
{
  if ( Tvdb400_IsEOS( pDataFrame ) )
  {
    m_iFramesCounter = 0 ;
    (( CDataFrame* )pDataFrame)->AddRef();
    return (CDataFrame*)pDataFrame ;
  }
  const CUserDataFrame* pUserDataFrame = pDataFrame->GetUserDataFrame( TVOBJECTS_DATA_NAME );
  if ( pUserDataFrame )
  {
    m_iFramesCounter++ ;
    CTVObjDataFrame * pObjectsFrame = ( CTVObjDataFrame* )pUserDataFrame ;

    FXPtrArray Objects ;
    pObjectsFrame->ExtractObjects( Objects ) ;
    if ( Objects.GetCount() )
    {
      int iNSpots = 0 , iNLines = 0 , iNEdges = 0 , iNTexts = 0 , iNUnknowns = 0 ;
      for ( int i = 0 ; i < Objects.GetCount() ; i++ )
      {
        VOBJ_TYPE iTypeId = *( VOBJ_TYPE * )Objects[ i ] ;
        switch ( iTypeId )
        {
          case SPOT: 
            iNSpots++ ; 
            delete ( CColorSpot * )Objects[ i ] ;
            break ;
          case LINE_SEGMENT: 
            iNLines++ ; 
            delete ( CLineResult * )Objects[ i ] ;
            break ;
          case EDGE: 
            iNEdges++ ; 
            delete ( CLineResult * )Objects[ i ] ;
            break ;
          case OCR: 
            iNTexts++ ; 
            delete ( COCRResult * )Objects[ i ] ;
            break ;
          default: iNUnknowns++ ; break ;
        }

      }
      CTextFrame * pOut = CTextFrame::Create() ;
      pOut->GetString().Format( "Frame %d Extracted: %d Spots , %d Lines , %d Edges , %d Texts , %d Unknowns" , 
        m_iFramesCounter , iNSpots , iNLines , iNEdges , iNTexts , iNUnknowns ) ;
      pOut->CopyAttributes( pDataFrame ) ;
      return pOut ;
    }
  }
  return NULL ;
}

