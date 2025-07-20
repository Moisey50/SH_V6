#pragma once

#include "gadgets\gadbase.h"
#include "gadgets\videoframe.h"
#include "gadgets\textframe.h"
#include "gadgets\QuantityFrame.h"
#include <gadgets\FigureFrame.h>
#include <gadgets\containerframe.h>
#include <helpers\FramesHelper.h>


using namespace std ;

class FX_EXT_GADGET DebugBuffer
{
  vector< const CDataFrame * > m_Frames ;
  COutputConnector * m_pOutput ;
  size_t m_uiNFramesLimit ;
  FXLockObject m_VectorLock ;


public:
  DebugBuffer( size_t uiDepth , COutputConnector * pOutputConnector ) ;
  ~DebugBuffer() ;
  size_t AddFrame( const CDataFrame * pNewFrame ) ;
  bool Flush( DWORD DelayBetweenFrames_ms = 20 , bool bDelete = true ) ;
};

