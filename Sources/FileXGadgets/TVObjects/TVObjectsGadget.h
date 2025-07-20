// TVObjectsGadget.h: interface for the TVObjectsGadget class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TVOBJECTSGADGET_H__23D164BA_6E75_400A_A2B9_1F4A40003D06__INCLUDED_)
#define AFX_TVOBJECTSGADGET_H__23D164BA_6E75_400A_A2B9_1F4A40003D06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
  // OCR library interfaces
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <Gadgets\gadbase.h>
#include <Gadgets\VideoFrame.h>
#include <fxfc\fxfc.h>
#include "ViewModeOptionData.h"
#include <gadgets\stdsetup.h>
#include <helpers\internalwatcher.h>




FXString GetTextFromTo( LPCTSTR Origin , 
  TCHAR From , TCHAR To , int& iFrom ) ;
CPoint GetXYfromString( LPCTSTR Src ) ;


#include "TVObjectsGadgetSetupDialog.h"

inline void CheckAndAdjustMinMax( CSize& Range , Runs& Vals )
{
  if ( Range.cx > Vals.m_iB )
    Range.cx = Vals.m_iB ;
  if ( Range.cy < Vals.m_iE )
    Range.cy = Vals.m_iE ;
}
class TVObjects : public CFilterGadget  
{
  friend class TVObjMeas ;
private:
  CInputConnector* m_pInputs[4];
  COutputConnector * m_pDiagOut ;
//   COutputConnector * m_pDescriptionOutput ;
//   CGadgetSetupDialog * m_pSetupObject ;
  FXString  m_Template;
  ViewModeOptionData m_ViewModeOption ;
  VOArray *  m_pObjects;
  VOArray *  m_pNewObjects ;
  VOArray *  m_pOldObjects ;
  bool       m_bNewObjectsUpdated ;
  CVOJobList m_Jobs;
  CVOJob     m_CurrentJob ;
  CVOJob     m_NewJob ;
  COCR       m_OCR ;

  int      m_CurrentJobID;
  int      m_iCurrentAOSIndex ;
  int      m_iCaptionPeriod ;
  FXLockObject m_Lock;        // For job-objects data taking or updating
  FXLockObject m_LockModify;  // for data modifying by Dlg or packet
  bool     m_bExtModifications ;
  bool     m_FormatErrorProcessed;
  DWORD    m_LastFormat;
  bool     m_bRun ;
  //bool		 m_AddEOSToOutPutPackets;
  bool		 m_CallOnStartFunc;
  double   m_dScale_um_per_pix ;
  FXString  m_ScaleUnits ;
  int      m_iNObjectsMax ;
  double   m_dTimeout ;
  CSize    m_LastFrameSize ;
  FXStaticQueue<FXString> m_ExternalControls ;
  tesseract::TessBaseAPI * m_pTesseract ;
  int      m_iTesseractRetryDelay ;
  const CDataFrame * m_pLastDataFrame ;

public:
  bool GetThisGadgetName( FXString& Name )
  {
    GetGadgetName( Name ) ;
    return !Name.IsEmpty() ;
  }
  int GetVideoObjectsArray( VOArray ** pDestination ) ;
  int GetTasksAsText( FXString * pTasks = NULL ) ;
  int GetTasksAsText( FXStringArray * pTasks = NULL ) ;
  int GetObjectsAndTasks(
    VOArray** ppVOArray , CVOJobList** ppTasksList , 
    int *piActiveTask , FXLockObject ** pLockVideoObjects )  ;
  bool Link( VOArray * pObjects );
  bool Link( VOArray * pObjects , CVOJobList& Jobs );
  TVObjects();
  ~TVObjects() {} ;
  void ShutDown(); 
  virtual int GetInputsCount() { return 4; }
  CInputConnector* GetInputConnector(int n)
    { return ((n<4)&&(n>=0))?m_pInputs[n]:NULL; }
  
  virtual int GetOutputsCount() { return 2; }
  COutputConnector* GetOutputConnector(int n)
    { return (n==0)?m_pOutput: ((n==1) ? m_pDiagOut : 0) ; }
  bool    PrintProperties(FXString& text);
  bool    ScanProperties(LPCTSTR Text, bool& Invalidate);    
  bool    ScanSettings(FXString& text);
  bool    LoadTemplate( FXString * pTemplate = NULL );
  bool    LoadTemplate( FXString * pTemplate , 
    VOArray ** pObjects , CVOJobList& Jobs , int& iActiveTask );
  void    OnScript(CDataFrame* lpData);
  void    OnControl(CDataFrame* lpData);
  void    OnCoordinates(CDataFrame* lpData);
  virtual void OnStart();
  virtual void OnStop();
  CDataFrame *    FormDescription( LPCTSTR szDescription , LPCTSTR szLabel , DWORD Id ) ;
  bool    GetObjectPosByName( LPCTSTR ObjName , CRect& Pos , FXString& AnchorObject ) ;

protected:
  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
  DECLARE_RUNTIME_GADGET(TVObjects);
};

class TVObjMeas: public TVObjects
{
public:
  TVObjMeas() ;
  bool    ScanProperties(LPCTSTR Text, bool& Invalidate);    
  DECLARE_RUNTIME_GADGET(TVObjMeas);
};

class RemoveSpots : public CFilterGadget  
{
private:
  FXLockObject m_Lock;        // For job-objects data taking or updating
  bool     m_bRun ;
  bool		 m_CallOnStartFunc;

  // UI Properties
  int     m_uiAreaMin ;
  int     m_uiAreaMax ;
  int     m_uiWidthMin ;
  int     m_uiWidthMax ;
  int     m_uiHeightMin ;
  int     m_uiHeightMax ;
  int      m_iReplaceValue ;
  int      m_iThreshold ;
  int     m_iPartSize ;
  int     m_iWallDepth ;
  int     m_iMaxEmpty ;
  VOBJ_CONTRAST m_WhatToRemove ;// WHITE_ON_BLACK(0), BLACK_ON_WHITE(1), ANY_CONTRAST(2) 
  BOOL    m_bRemoveOnEdge ;
  BOOL    m_bCutFromEdge ;

  UINT     m_LastFormat ;
  RunsArrays m_SortedTSRuns ;

  CVideoObject * m_pObject ; // video object for processing, always is spot
  CVideoObject * m_pNewObject ; // video object for UI; it will be copied to processing object

public:
  RemoveSpots();
  ~RemoveSpots() ;
  void ShutDown(); 
  virtual int GetInputsCount() { return 1; }
  CInputConnector* GetInputConnector(int n) { return (n==0)?m_pInput:NULL; }
  virtual int GetOutputsCount() { return 1; }
  COutputConnector* GetOutputConnector(int n) { return (n==0)?m_pOutput: 0  ; }
  bool    PrintProperties(FXString& text);
  bool    ScanProperties(LPCTSTR Text, bool& Invalidate);    
  bool    ScanSettings(FXString& text);
  virtual void OnStart();
  virtual void OnStop();
  int     GetFilledWidth(RunsArray& RunsArr , int& iWidth ) ;
protected:
  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
  void InitNewObject() ;
  DECLARE_RUNTIME_GADGET(RemoveSpots);
public:

  int IsBigSpaceAfter(RunsArray& TSRuns, int iDepth, int iNRuns )
  {
    int iLast = (int) TSRuns.GetUpperBound() ;
    if ( iLast == 0 )
    {
      if ( TSRuns[0].GetLen() < iDepth )
        return 1000 + iDepth ;
      else
        return 0 ;
    }
    if ( iNRuns > iLast )
      iNRuns = iLast ;
    int iLeft = TSRuns[0].m_iB ;
    int iRight = iLeft + iDepth ;
    for ( int i = 1 ; i <= iNRuns ; i++ )
    {
      iRight = TSRuns[i - 1].m_iE ;
      int iThisLeft = TSRuns[i].m_iB ;
      int iSpace = iThisLeft - iRight ;
      if ( iSpace > iDepth )
        return iSpace * 1000 + iRight - iLeft ;
      if ( iThisLeft - iLeft > iDepth * 2 )
        return 0 ;
    }
    // no runs after
    return 1000 + iRight - iLeft ; // ~infinite distance to next run
  }
  int GetBigSpaceAfter(RunsArray& TSRuns, int iFilledDepth , int NotFilledDepth )
  {
    if ( TSRuns[0].GetLen() > iFilledDepth )
      return -TSRuns[0].m_iB ;

    int iRight = TSRuns[0].m_iE ;
    int iLast = (int) TSRuns.GetUpperBound() ;
    if ( iLast == 0 )
      return iRight ;
    int iLastEnd = iRight ;
    for ( int i = 1 ; i <= iLast ; i++ )
    {
      int iNextBegin = TSRuns[i].m_iB ;
      int iSpace = iNextBegin - iLastEnd ;
      if ( iSpace > NotFilledDepth )
        return iLastEnd ;
      if ( TSRuns[i].GetLen() > iFilledDepth )
        return iLastEnd ;
      iLastEnd = TSRuns[i].m_iE ;
    }
    // no runs after
    return 0 ; // ~infinite distance to next run
  }
  RunAndSpace CheckForSpaceAfter(RunsArray& TSRuns, int iFilledDepth , int NotFilledDepth )
  {
    RunAndSpace Result(TSRuns[0]) ; 
    int iLast = (int) TSRuns.GetUpperBound() ;
    if ( TSRuns[0].GetLen() > iFilledDepth )
      return Result ;

    for ( int i = 1 ; i <= iLast ; i++ )
    {
      int iNextBegin = TSRuns[i].m_iB ;
      int iSpace = iNextBegin - Result.m_iE ;
      if ( iSpace > NotFilledDepth )
      {
        Result.m_iSpaceAfter = iSpace ;
        return Result ;
      }
      Result.m_iE = TSRuns[i].m_iE ;
      if ( Result.GetLen() > iFilledDepth )
      {
        Result.m_iSpaceAfter = (i < iLast) ? TSRuns[i+1].m_iB - TSRuns[i].m_iE : 0 ;
        return Result ;
      }
    }
    // no runs after
    return Result ; // ~infinite distance to next run
  }

  RunAndSpace CheckForSpaceBefore(RunsArray& TSRuns, int iFilledDepth , int NotFilledDepth )
  {
    int iLast = (int) TSRuns.GetUpperBound() ;
    RunAndSpace Result(TSRuns[iLast]) ; 
    if ( TSRuns[iLast].GetLen() > iFilledDepth )
      return Result ;

    for ( int i = iLast - 1 ; i >= 0 ; i-- )
    {
      int iNextBegin = TSRuns[i].m_iE ;
      int iSpace = iNextBegin - Result.m_iB ;
      if ( iSpace > NotFilledDepth )
      {
        Result.m_iSpaceAfter = iSpace ;
        return Result ;
      }
      Result.m_iB = TSRuns[i].m_iB ;
      if ( Result.GetLen() > iFilledDepth )
      {
        Result.m_iSpaceAfter = (i > 0) ? TSRuns[i-1].m_iE - TSRuns[i].m_iB : 0 ;
        return Result ;
      }
    }
    // no runs after
    return Result ; // ~infinite distance to next run
  }

  int IsBigSpaceBefore(RunsArray& TSRuns, int iDepth, int iNRuns)
  {
    int iLast = (int) TSRuns.GetUpperBound() ;
    if ( iLast == 0 )
    {
      if ( TSRuns[0].GetLen() < iDepth )
        return 1000 + TSRuns[0].GetLen() ;
      else
    return 0;
  }
    int iRight = TSRuns[iLast].m_iE ;
    if ( iNRuns > iLast  )
      iNRuns = iLast ;
    for ( int i = 1 ; i <= iNRuns ; i++ )
  {
      int iLeft = TSRuns[iLast-i].m_iB ;
      int iNextRight =  TSRuns[iLast - i - 1].m_iE ;
      int iSpace = TSRuns[iLast-i].m_iB - iNextRight ;
      if ( iSpace > iDepth )
        return iSpace * 1000 + iRight - iLeft ;
      if ( iRight - iNextRight > iDepth * 2 )
        return 0 ;
    }
    return 0;
  }
};

class ProcessTVObjectsFrame : public CFilterGadget
{
private:
  FXLockObject m_Lock;        // For job-objects data taking or updating
  bool     m_bRun ;
  int 		 m_iFramesCounter ;


public:
  ProcessTVObjectsFrame();
  ~ProcessTVObjectsFrame() ;
  void ShutDown();
//   virtual int GetInputsCount() { return 1; }
//   CInputConnector* GetInputConnector( int n ) { return ( n == 0 ) ? m_pInput : NULL; }
//   virtual int GetOutputsCount() { return 1; }
//   COutputConnector* GetOutputConnector( int n ) { return ( n == 0 ) ? m_pOutput : 0  ; }
//   bool    PrintProperties( FXString& text );
//   bool    ScanProperties( LPCTSTR Text , bool& Invalidate );
//   bool    ScanSettings( FXString& text );
//   virtual void OnStart();
//   virtual void OnStop();
  virtual void OnEOS();

//  int     GetFilledWidth( RunsArray& RunsArr , int& iWidth ) ;
protected:
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
//   void InitNewObject() ;
  DECLARE_RUNTIME_GADGET( ProcessTVObjectsFrame );
public:
};



#endif // !defined(AFX_TVOBJECTSGADGET_H__23D164BA_6E75_400A_A2B9_1F4A40003D06__INCLUDED_)
