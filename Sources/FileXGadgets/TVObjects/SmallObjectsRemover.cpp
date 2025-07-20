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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "SmallObjectsRemover"

IMPLEMENT_RUNTIME_GADGET_EX(RemoveSpots, CFilterGadget, "Video.recognition", TVDB400_PLUGIN_NAME);

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


RemoveSpots::RemoveSpots()
{
  m_pInput = new CInputConnector(vframe);
  m_pOutput  =    new COutputConnector( transparent );
  m_LastFormat=0;
  m_uiAreaMin = 1 ;
  m_uiAreaMax = 100 ;
  m_uiWidthMin = 1 ;
  m_uiWidthMax = 10 ;
  m_uiHeightMin = 1 ;
  m_uiHeightMax = 10 ;
  m_iReplaceValue = -1 ;
  m_iThreshold = 128 ;
  m_iPartSize = 400 ;
  m_iWallDepth = 40 ;
  m_iMaxEmpty = 15 ;
  m_WhatToRemove = BLACK_ON_WHITE ; 
  m_bRemoveOnEdge = FALSE ;
  m_bCutFromEdge = FALSE ;
  m_OutputMode = modeReplace ;
  m_pObject = NULL ;
  m_pNewObject = NULL ;
  InitNewObject() ;
  Resume();
}

RemoveSpots::~RemoveSpots()
{
  if ( m_pObject )
  {
    delete m_pObject ;
    m_pObject = NULL ;
  }
  if ( m_pNewObject )
  {
    delete m_pNewObject ;
    m_pNewObject = NULL ;
  }
} ;


//RemoveSpotsGadget::~RemoveSpotsGadget()
void RemoveSpots::ShutDown()
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

void RemoveSpots::OnStart()
{ 
  m_CallOnStartFunc = 0 ; 
  m_bRun = true ; 
} ;

void RemoveSpots::OnStop()
{ 
  m_CallOnStartFunc = 1;
  CDataFrame* pFrame=CDataFrame::Create();

  //pFrame->SetLabel("TVObject");
  Tvdb400_SetEOS(pFrame);

  //   CDataFrame* Container = CContainerFrame::Create() ;
  //   ((CContainerFrame*)Container)->AddFrame(pFrame);
  //   CQuantityFrame* pQuanFrame = CQuantityFrame::Create(0);
  //   pQuanFrame->ChangeId(pFrame->GetId());
  //   pQuanFrame->SetTime(pFrame->GetTime());
  //   pQuanFrame->SetLabel("TVObject");
  //   ((CContainerFrame*)Container)->AddFrame(pQuanFrame);  
  //  m_Lock.Lock();  // now we need to deliver the EOS message to the rest of the graph -> send it to all outputs

  COutputConnector* pOutput = (COutputConnector*)m_pOutput;
  //   if ( !pOutput->Put(Container) )
  //     Container->Release( Container ); 
  if ( !pOutput  ||  !pOutput->Put(pFrame) )
    pFrame->Release( pFrame ); 

  //  m_Lock.Unlock();
  m_bRun = false ; 
} ;

bool    RemoveSpots::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  pk.WriteInt("AreaMin_pix" , m_uiAreaMin );
  pk.WriteInt("AreaMax_pix" , m_uiAreaMax );
  pk.WriteInt("MinWidth_pix" , m_uiWidthMin );
  pk.WriteInt("MaxWidth_pix" , m_uiWidthMax );
  pk.WriteInt("MinHeight_pix" , m_uiHeightMin );
  pk.WriteInt("MaxHeight_pix" , m_uiHeightMax );
  pk.WriteInt("ReplaceValue" , m_iReplaceValue);
  pk.WriteInt("Threshold" , m_iThreshold );
  pk.WriteInt( "WhatToRemove" , m_WhatToRemove ) ;
  pk.WriteInt( "RemoveOnEdge" , m_bRemoveOnEdge ) ;
  pk.WriteInt( "CutFromEdge" , m_bCutFromEdge ) ;
  pk.WriteInt( "PartSize" , m_iPartSize ) ;
  pk.WriteInt( "WallDepth" , m_iWallDepth ) ;
  pk.WriteInt( "MaxEmpty" , m_iMaxEmpty ) ;
  text = pk ;
  return true;
}
static const char replaced_chars[][2]={";","\n","\r","\\"};
static const int  replacemen_nmb=4;
static const char replacement[][3]={"\\s","\\n","\\r","\\\\"};

bool RemoveSpots::ScanProperties(LPCTSTR Text, bool& Invalidate)
{
  FXPropertyKit pk(Text);
  pk.GetInt("AreaMin_pix" , m_uiAreaMin );
  pk.GetInt("AreaMax_pix" , m_uiAreaMax );
  pk.GetInt("MinWidth_pix" , m_uiWidthMin );
  pk.GetInt("MaxWidth_pix" , m_uiWidthMax );
  pk.GetInt("MinHeight_pix" , m_uiHeightMin );
  pk.GetInt("MaxHeight_pix" , m_uiHeightMax );
  pk.GetInt("ReplaceValue" , m_iReplaceValue);
  pk.GetInt("Threshold" , m_iThreshold );
  pk.GetInt( "WhatToRemove" , (int&)m_WhatToRemove ) ;
  pk.GetInt( "RemoveOnEdge" , m_bRemoveOnEdge ) ;
  pk.GetInt( "CutFromEdge" , m_bCutFromEdge ) ;
  pk.GetInt( "PartSize" , m_iPartSize ) ;
  pk.GetInt( "WallDepth" , m_iWallDepth ) ;
  pk.GetInt( "MaxEmpty" , m_iMaxEmpty ) ;

  InitNewObject() ;
  return true;
}
void RemoveSpots::InitNewObject()
{
  CVideoObject * pNewObject = new CVideoObject ;
  pNewObject->m_AOS = CRect( -1,-1,-1,-1 ) ;
  pNewObject->m_ExpectedSize = CRect( m_uiWidthMin , m_uiHeightMin , m_uiWidthMax , m_uiHeightMax ) ;
  //   pNewObject->m_iAreaMin = m_uiAreaMin ;
  //   pNewObject->m_iAreaMax = m_uiAreaMax ;
  pNewObject->m_bMulti = true ;
  pNewObject->m_WhatToMeasure = MEASURE_POSITION | MEASURE_AREA | MEASURE_RUNS ;
  pNewObject->m_Thresholds.Add( (double)m_iThreshold ) ;
  pNewObject->m_Contrast = m_WhatToRemove ; // WHITE_ON_BLACK(0), BLACK_ON_WHITE(1), ANY_CONTRAST(2) 
  pNewObject->m_bDontTouchBorder = false ;
  pNewObject->m_bNoFiltration = true ;
  FXAutolock al( m_Lock ) ;
  if ( m_pNewObject )
  {
    delete m_pNewObject ;
    m_pNewObject = NULL ;
  }
  m_pNewObject = pNewObject ;
}
bool    RemoveSpots::ScanSettings(FXString& text)
{
  text = "template("
    "Spin(AreaMin_pix,1,1000000),"
    "Spin(AreaMax_pix,2,1000000),"
    "Spin(MinWidth_pix,1,10000),"
    "Spin(MaxWidth_pix,2,10000),"
    "Spin(MinHeight_pix,1,10000),"
    "Spin(MaxHeight_pix,2,10000),"
    "Spin(ReplaceValue,-1,65536),"
    "Spin(Threshold,1,65535),"
    "ComboBox(WhatToRemove(White(0),Black(1),All(2))),"
    "ComboBox(RemoveOnEdge(No(0),Yes(1))),"
    "ComboBox(CutFromEdge(No(0),Yes(1))),"
    "Spin(PartSize,50,10000),"
    "Spin(WallDepth, 5,200),"
    "Spin(MaxEmpty, 5,200)"
    ")" ;
  return true;
}


CDataFrame* RemoveSpots::DoProcessing(const CDataFrame* pDataFrame)
{
  if ( m_pNewObject )
  {
    FXAutolock al( m_Lock ) ;
    if ( m_pObject )
    {
      delete m_pObject ;
      m_pObject = NULL ;
    }
    m_pObject = m_pNewObject ;
    m_pNewObject = NULL ;
  }
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
  if ( !VideoFrame || !VideoFrame->lpBMIH )
    return NULL ;
  m_LastFormat = VideoFrame->lpBMIH->biCompression ;
  if ((m_LastFormat!=BI_YUV9) && (m_LastFormat!=BI_YUV12)
    && (m_LastFormat!=BI_Y8) && (m_LastFormat!=BI_Y800) && (m_LastFormat != BI_Y16) ) 
  {
    SENDERR_0("RemoveSpots can only accept formats Y16,YUV9, YUV12 and Y8");
    return NULL;
  }

  if ( !m_pOutput->IsConnected() )
  {
    return CreateTextFrame( _T("Dummy, No Connections") , pDataFrame->GetId() ) ;
  }

  if ( !m_pObject )
    return NULL ;
  CVideoFrame * pOutFrame = (CVideoFrame*)VideoFrame->Copy() ;

  m_pObject->m_WhatIsMeasured = 0 ;

  bool bMeasResult = m_pObject->DoMeasure( pOutFrame ) ;	//search for the object.

  if ( bMeasResult )
  {
    SpotArray& Spots = *(m_pObject->m_pSegmentation->GetSpotsInfo()) ;
    FXUintArray& ActiveIndexes = m_pObject->m_pSegmentation->m_ActiveSpots ;
    ActiveIndexes.RemoveAll() ;

    if (m_pObject->m_WhatIsMeasured & MEASURE_POSITION)
    {
      LPBYTE pImage = GetData( pOutFrame ) ;
      int iWidth = GetWidth( pOutFrame ) ;
      int iHeight = GetHeight( pOutFrame ) ;
      int iPixSize = (m_LastFormat == BI_Y16) ? 2 : 1 ;
      int iGoodSpotCnt = 0 ;

      int iWhite = (m_pObject->m_Contrast == WHITE_ON_BLACK) ;
      CRect WorkingArea = m_pObject->m_absAOS ;
      cmplx Offset( WorkingArea.left , WorkingArea.top ) ;
      CRect ExpectedSize = m_pObject->m_ExpectedSize ;

      for ( int i = 0 ; i < Spots.GetCount() ; i++ )
      {
        CColorSpot& Spot = Spots.GetAt( i );
        if ( Spot.m_Area > 0 )  //not removed spot
        {
          CPoint Cent = Spot.m_SimpleCenter.GetCPoint() ;
          if ( (iWhite == Spot.m_iColor) || (m_pObject->m_Contrast == ANY_CONTRAST) )
          {
            bool bSmallSpot = (Spot.m_OuterFrame.Width() + 1) >= ExpectedSize.left 
              && Spot.m_OuterFrame.Width() <= ExpectedSize.right 
              && (Spot.m_OuterFrame.Height() + 1) >= ExpectedSize.top 
              && Spot.m_OuterFrame.Height() <= ExpectedSize.bottom 
              && Spot.m_Area >= m_uiAreaMin
              && Spot.m_Area <= m_uiAreaMax  ;
            bool bEdgeSpot = ( Spot.m_OuterFrame.left <= 1 
              || Spot.m_OuterFrame.top  <= 1 
              || Spot.m_OuterFrame.right >= iWidth - 2
              || Spot.m_OuterFrame.bottom >= iHeight - 2 )
              && (    (Spot.m_OuterFrame.Width() < (iWidth/3))  
              ||  (Spot.m_OuterFrame.Height() < (iHeight/3)) ) ;

            if ( bSmallSpot || bEdgeSpot )
            {
              RunsArray& TSRuns = Spot.m_Runs ;
              for ( int iRunCnt = 0 ; iRunCnt < TSRuns.GetCount() ; iRunCnt++ )
              {
                Runs& ThisRun = TSRuns.GetAt(iRunCnt) ;
                if ( iPixSize == 1 )
                {
                  memset( pImage + ThisRun.iY * iWidth + ThisRun.m_iB , 
                    (BYTE)m_iReplaceValue , ThisRun.GetLen() ) ;

                }
                else
                {
                  LPWORD p = ((LPWORD)pImage) + ThisRun.iY * iWidth + ThisRun.m_iB ; 
                  LPWORD pEnd = p + ThisRun.GetLen() ;
                  while ( p < pEnd )
                    *(p++) = (WORD)m_iReplaceValue ;
                }
              }
            }
            else // big spot
            {
              ActiveIndexes.Add(i) ;
            }
          }
        }
      }
      for ( int i = 0 ; i < ActiveIndexes.GetCount() ; i++ )
      {
        DWORD iActive = ActiveIndexes[i] ;
        CColorSpot& Spot = Spots.GetAt( iActive );
        int iFirstY = Spot.m_OuterFrame.top ;
        RunsArray& TSRuns = Spot.m_Runs ;
        int iNRows = Spot.m_OuterFrame.Height() + 1 ;
        int iLast = iFirstY + iNRows ; // really it's NLast + 1
        m_SortedTSRuns.SetSize( iHeight ) ;
        for ( int i = 0 ; i < m_SortedTSRuns.GetSize() ; i++ )
        {
          m_SortedTSRuns[ i ].RemoveAll() ;
        }
        for ( int i = 0 ; i < TSRuns.GetSize() ; i++ )
        {
          m_SortedTSRuns[ TSRuns[i].iY ].Add( TSRuns[i] ) ;
        }
        for ( int i = iFirstY ; i < iLast ; i++ )
        {
          bool bChanged = true ;
          RunsArray& ThisRow = m_SortedTSRuns[i] ;
          while ( bChanged )
          {
            bChanged = false ;
            for ( int j = 0 ; j < ThisRow.GetCount() - 1 ; j++ )
            {
              if ( ThisRow[j].m_iB > ThisRow[j+1].m_iB )
              {
                ThisRow[j].Swap( ThisRow[j+1] ) ;
                bChanged = true ;
              }
            }
          }
        }

        int iCent = iFirstY + iNRows/2 ;
        // remove left wall
        {
          int iForwCnt = iCent + 1 ;
          double dXBCent = m_SortedTSRuns[iCent].GetAt(0).m_iB ;
          for ( ; iForwCnt < iLast ; iForwCnt++ )
          {
            Runs& LeftEdgeRun = m_SortedTSRuns[iForwCnt].GetAt(0) ;
            if ( LeftEdgeRun.m_iB > Spot.m_SimpleCenter.x )
            {
              iForwCnt-- ;
              break ;
            }
            if ( fabs( dXBCent - LeftEdgeRun.m_iB ) > 
              20 + (iForwCnt - iCent)*0.1 )
              break ;
          }
          if ( iForwCnt >= iFirstY + iNRows )
            iForwCnt = iFirstY + iNRows - 1 ;
          int iBackCnt = iCent - 1 ;
          for ( ; iBackCnt >= iFirstY ; iBackCnt-- )
          {
            Runs& LeftEdgeRun = m_SortedTSRuns[iBackCnt].GetAt(0) ;
            if ( LeftEdgeRun.m_iB > Spot.m_SimpleCenter.x )
            {
              iBackCnt++ ;
              break ;
            }
            if ( fabs( dXBCent - LeftEdgeRun.m_iB ) > 
              20 + ( iCent - iBackCnt )*0.1)
              break ;
          }
          if ( iBackCnt < iFirstY )
            iBackCnt = iFirstY ;
          if ( iForwCnt - iBackCnt > Spot.m_OuterFrame.Height() / 3 )
          {
            // look for breaks near left border
            int iLeftWidth = 0 ;
            int iLeftBegin = -1 ;
            // Areas with spaces
            int iOnUpperPoint = 0 ;
            CSize UpperWidth(100000,-10000) ;
            int iAfterUpperPoint = 0 ;
            CSize AfterUpperWidth(100000,-10000) ;
            int iBeforeEnd = 0 ;
            CSize BeforeEndWidth(100000,-10000) ;
            int iOnEndPoint = 0 ;
            CSize EndWidth(100000,-10000) ;

            // Areas without spaces
            Runs UpperWall ; // vice versa of X and Y, iY is most left edge 
            Runs Contact ; // vice versa of X and Y, iY is most left edge
            Runs LowerWall ; // vice versa of X and Y, iY is most left edge
            FXArray <RunAndSpace> RunsAndSpaces ;
            for ( int i = iBackCnt ; i < iForwCnt ; i++ )
            {
              int iY = m_SortedTSRuns[i].GetAt(0).iY ;

              RunAndSpace OnEdge = CheckForSpaceAfter( m_SortedTSRuns[i] , m_iWallDepth , m_iMaxEmpty ) ;
              RunsAndSpaces.Add( OnEdge ) ;
              if ( OnEdge.GetLen() < m_iWallDepth ) // real space after edge
              {
                // analysis in reverse order (from the end to the beginning)
                if ( iOnEndPoint || LowerWall.m_iB )
                {
                  iOnEndPoint++ ;
                  CheckAndAdjustMinMax( EndWidth , OnEdge ) ;
                }
                else if ( iBeforeEnd || Contact.m_iB )
                {
                  iBeforeEnd++ ;
                  CheckAndAdjustMinMax( BeforeEndWidth , OnEdge ) ;
                }
                else if ( iAfterUpperPoint || UpperWall.m_iB )
                {
                  iAfterUpperPoint++ ;
                  CheckAndAdjustMinMax( AfterUpperWidth , OnEdge ) ;
                }
                else
                {
                  iOnUpperPoint++ ;
                  CheckAndAdjustMinMax( UpperWidth , OnEdge ) ;
                }
              }
              else // no space after edge
              {
                if ( iOnEndPoint )
                {
                  //  ASSERT(0) ; // second lower wall?
                }
                else if ( LowerWall.m_iB )
                {
                  LowerWall.m_iE = iY ;
                  if ( LowerWall.iY > OnEdge.m_iB )
                    LowerWall.iY = OnEdge.m_iB ; 
                }
                else if ( iBeforeEnd )
                {
                  LowerWall.m_iB = iY ;
                  LowerWall.m_iE = iY ;
                  LowerWall.iY = OnEdge.m_iB ;
                }
                else if ( Contact.m_iB )
                {
                  Contact.m_iE = iY ;
                  if ( Contact.iY > OnEdge.m_iB )
                    Contact.iY = OnEdge.m_iB ; 
                }
                else if ( iAfterUpperPoint )
                {
                  Contact.m_iB = iY ;
                  Contact.m_iE = iY ;
                  Contact.iY = OnEdge.m_iB ;
                }
                else if ( UpperWall.m_iB )
                {
                  if ( iY - UpperWall.m_iB > 30 )
                  {
                    Contact.m_iB = iY ;
                    Contact.m_iE = iY ;
                    Contact.iY = OnEdge.m_iB ;
                    UpperWall.m_iB = 0 ; // contact instead of upper wall
                  }
                  else
                  {
                    UpperWall.m_iE = iY ;
                    if ( UpperWall.iY > OnEdge.m_iB )
                      UpperWall.iY = OnEdge.m_iB ;
                  }
                }
                else
                {
                  UpperWall.m_iB = iY ;
                  UpperWall.m_iE = iY ;
                  UpperWall.iY = OnEdge.m_iB ;
                }
              }
            }
            if ( iOnUpperPoint )
            {
              iLeftBegin = UpperWidth.cx ;
              iLeftWidth = UpperWidth.cy - UpperWidth.cx + 1 ;
              if ( iOnEndPoint )
              {
                if ( EndWidth.cx < iLeftBegin )
                  iLeftBegin = EndWidth.cx ;
                if ( EndWidth.cy > UpperWidth.cy )
                  iLeftWidth += EndWidth.cy - UpperWidth.cy ;
              }
              else if ( iBeforeEnd )
              {
                if ( BeforeEndWidth.cx < iLeftBegin )
                  iLeftBegin = BeforeEndWidth.cx ;
                if ( BeforeEndWidth.cy > UpperWidth.cy )
                  iLeftWidth += BeforeEndWidth.cy - UpperWidth.cy ;
              }
              //               else if ( Contact.m_iB )
              //               {
              //                 if ( Contact.m_iB < iLeftBegin )
              //                 {
              //                   iLeftWidth += ( iLeftBegin - Contact.m_iB ) ;
              //                   iLeftBegin = Contact.m_iB ;
              //                 }
              //              }
              else if ( iAfterUpperPoint )
              {
                if ( AfterUpperWidth.cx < iLeftBegin )
                  iLeftBegin = AfterUpperWidth.cx ;
                if ( AfterUpperWidth.cy > UpperWidth.cy )
                  iLeftWidth += AfterUpperWidth.cy - UpperWidth.cy ;
              }
            }
            if ( iLeftWidth > 0 )
            {
              int iBegin = iLeftBegin /*- iLeftWidth*/ ;
              LPBYTE pSegm = pImage + iPixSize * iBegin ;
              for ( int iY = 0 ; iY < iHeight ; iY++ )
              {
                if ( pSegm + iLeftWidth >= pImage + GetImageSize( pOutFrame ) ) 
                  break ;
                if ( iPixSize == 1 )
                  memset( pSegm , (BYTE)m_iReplaceValue , iLeftWidth ) ;
                else
                {
                  LPWORD pwSegm = (LPWORD)pSegm ;
                  LPWORD pwEnd = pwSegm + iLeftWidth ;
                  do 
                  {
                    *(pwSegm++) = (WORD)m_iReplaceValue ;
                  } while ( pwSegm < pwEnd) ;
                }
                pSegm += iWidth * iPixSize ;
              }
            }
          }
        }
        // remove right wall
        {
          int iForwCnt = iCent + 1 ;
          double dXECent = GetLastRun( m_SortedTSRuns , iCent).m_iE ;
          for ( ; iForwCnt < iLast ; iForwCnt++ )
          {
            Runs& RightEdgeRun = GetLastRun( m_SortedTSRuns , iForwCnt) ;
            if ( RightEdgeRun.m_iE < Spot.m_SimpleCenter.x )
            {
              iForwCnt-- ;
              break ;
            }
            if ( fabs( dXECent - RightEdgeRun.m_iE ) > 
              20 + (iForwCnt - iCent)*0.1 )
              break ;
          }
          if ( iForwCnt >= iFirstY + iNRows )
            iForwCnt = iFirstY + iNRows - 1 ;
          int iBackCnt = iCent - 1 ;
          for ( ; iBackCnt >= iFirstY ; iBackCnt-- )
          {
            Runs& RightEdgeRun = GetLastRun( m_SortedTSRuns , iBackCnt) ;
            if ( RightEdgeRun.m_iE < Spot.m_SimpleCenter.x )
            {
              iBackCnt++ ;
              break ;
            }
            if ( fabs( dXECent - RightEdgeRun.m_iE ) > 
              20 + ( iCent - iBackCnt )*0.1)
              break ;
          }
          if ( iBackCnt < iFirstY )
            iBackCnt = iFirstY ;
          if ( iForwCnt - iBackCnt > Spot.m_OuterFrame.Height() / 3 )
          {
            // look for breaks near left border
            int iRightWidth = 0 ;
            int iRightBegin = -1 ;
            // Areas with spaces
            int iOnUpperPoint = 0 ;
            CSize UpperWidth(100000,-10000) ;
            int iAfterUpperPoint = 0 ;
            CSize AfterUpperWidth(100000,-10000) ;
            int iBeforeEnd = 0 ;
            CSize BeforeEndWidth(100000,-10000) ;
            int iOnEndPoint = 0 ;
            CSize EndWidth(100000,-10000) ;

            // Areas without spaces
            Runs UpperWall ; // vice versa of X and Y, iY is most left edge 
            Runs Contact ; // vice versa of X and Y, iY is most left edge
            Runs LowerWall ; // vice versa of X and Y, iY is most left edge
            FXArray <RunAndSpace> RunsAndSpaces ;
            for ( int i = iBackCnt ; i < iForwCnt ; i++ )
            {
              int iY = m_SortedTSRuns[i].GetAt(0).iY ;
              RunAndSpace OnEdge = CheckForSpaceBefore( m_SortedTSRuns[i] , m_iWallDepth , m_iMaxEmpty ) ;
              RunsAndSpaces.Add( OnEdge ) ;
              if ( OnEdge.GetLen() < m_iWallDepth ) // real space after edge
              {
                // analysis in reverse order (from the end to the beginning)
                if ( iOnEndPoint || LowerWall.m_iB )
                {
                  iOnEndPoint++ ;
                  CheckAndAdjustMinMax( EndWidth , OnEdge ) ;
                }
                else if ( iBeforeEnd || Contact.m_iB )
                {
                  iBeforeEnd++ ;
                  CheckAndAdjustMinMax( BeforeEndWidth , OnEdge ) ;
                }
                else if ( iAfterUpperPoint || UpperWall.m_iB )
                {
                  iAfterUpperPoint++ ;
                  CheckAndAdjustMinMax( AfterUpperWidth , OnEdge ) ;
                }
                else
                {
                  iOnUpperPoint++ ;
                  CheckAndAdjustMinMax( UpperWidth , OnEdge ) ;
                }
              }
              else // no space after edge
              {
                if ( iOnEndPoint )
                {
                  //  ASSERT(0) ; // second lower wall?
                }
                else if ( LowerWall.m_iB )
                {
                  LowerWall.m_iE = iY ;
                  if ( LowerWall.iY > OnEdge.m_iB )
                    LowerWall.iY = OnEdge.m_iB ; 
                }
                else if ( iBeforeEnd )
                {
                  LowerWall.m_iB = iY ;
                  LowerWall.m_iE = iY ;
                  LowerWall.iY = OnEdge.m_iB ;
                }
                else if ( Contact.m_iB )
                {
                  Contact.m_iE = iY ;
                  if ( Contact.iY > OnEdge.m_iB )
                    Contact.iY = OnEdge.m_iB ; 
                }
                else if ( iAfterUpperPoint )
                {
                  Contact.m_iB = iY ;
                  Contact.m_iE = iY ;
                  Contact.iY = OnEdge.m_iB ;
                }
                else if ( UpperWall.m_iB )
                {
                  if ( iY - UpperWall.m_iB > 30 )
                  {
                    Contact.m_iB = iY ;
                    Contact.m_iE = iY ;
                    Contact.iY = OnEdge.m_iB ;
                    UpperWall.m_iB = 0 ; // contact instead of upper wall
                  }
                  else
                  {
                    UpperWall.m_iE = iY ;
                    if ( UpperWall.iY > OnEdge.m_iE )
                      UpperWall.iY = OnEdge.m_iE ;
                  }
                }
                else
                {
                  UpperWall.m_iB = iY ;
                  UpperWall.m_iE = iY ;
                  UpperWall.iY = OnEdge.m_iB ;
                }
              }
            }
            if ( iOnUpperPoint )
            {
              iRightBegin = UpperWidth.cx ;
              iRightWidth = UpperWidth.cy - UpperWidth.cx + 1 ;
              if ( iOnEndPoint )
              {
                if ( EndWidth.cx < iRightBegin )
                  iRightBegin = EndWidth.cx ;
                if ( EndWidth.cy > UpperWidth.cy )
                  iRightWidth += EndWidth.cy - UpperWidth.cy ;
              }
              else if ( iBeforeEnd )
              {
                if ( BeforeEndWidth.cx < iRightBegin )
                  iRightBegin = BeforeEndWidth.cx ;
                if ( BeforeEndWidth.cy > UpperWidth.cy )
                  iRightWidth += BeforeEndWidth.cy - UpperWidth.cy ;
              }
              //               else if ( Contact.m_iB )
              //             {
              //                 if ( Contact.m_iB < iRightBegin )
              //               {
              //                   iRightWidth += ( iRightBegin - Contact.m_iB ) ;
              //                   iRightBegin = Contact.m_iB ;
              //               }
              //             }
              else if ( iAfterUpperPoint )
              {
                if ( AfterUpperWidth.cx < iRightBegin )
                  iRightBegin = AfterUpperWidth.cx ;
                if ( AfterUpperWidth.cy > UpperWidth.cy )
                  iRightWidth += AfterUpperWidth.cy - UpperWidth.cy ;
              }
            }
            if ( iRightWidth > 0 )
            {
              int iBegin = iRightBegin /*- iRightWidth*/ ;
              LPBYTE pSegm = pImage + iPixSize * iBegin ;
              for ( int iY = 0 ; iY < iHeight ; iY++ )
              {
                if ( pSegm + iRightWidth >= pImage + GetImageSize( pOutFrame ) ) 
                {
                  break ;
                }
                else
                {
                  if ( iPixSize == 1 )
                    memset( pSegm , (BYTE)m_iReplaceValue , iRightWidth ) ;
                  else
                  {
                    LPWORD pwSegm = (LPWORD)pSegm ;
                    LPWORD pwEnd = pwSegm + iRightWidth ;
                    do 
                    {
                      *(pwSegm++) = (WORD)m_iReplaceValue ;
                    } while ( pwSegm < pwEnd) ;
                  }
                }
                pSegm += iWidth * iPixSize ;
              }
            }
          }
        }
      }
    }

  }

  return pOutFrame ;
}


int RemoveSpots::GetFilledWidth(RunsArray& RunsArr , int& iWidth )
{
  int iSum = 0 ;
  iWidth = RunsArr[RunsArr.GetUpperBound()].m_iE - RunsArr[0].m_iB + 1 ;
  for ( int i = 0 ; i < RunsArr.GetCount() ; i++ )
    iSum += RunsArr[i].m_iE - RunsArr[i].m_iB + 1 ;

  return iSum ;
}
