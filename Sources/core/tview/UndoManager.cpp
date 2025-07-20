// UndoManager.cpp: implementation of the CUndoManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UndoManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifdef DEBUG_UNDO_MANAGER
#define UNDODUMP Dump()
#else
#define UNDODUMP
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUndoManager::CUndoManager() :
  m_nCurState( 0 ) ,
  m_Frozen( false ) ,
  m_ChangesKey( 0 )
{}

CUndoManager::~CUndoManager()
{
  Reset();
}

void CUndoManager::AddState( LPCTSTR state , LPCTSTR id )
{
  if ( m_Frozen )
    return;
  INT_PTR nObsolete = m_States.GetSize() - m_nCurState;
  if ( nObsolete )
  {
    m_States.RemoveAt( m_nCurState , nObsolete );
    m_StateIDs.RemoveAt( m_nCurState , nObsolete );
  }
  m_States.Add( state );
  m_StateIDs.Add( id );
  m_nCurState++;
  m_ChangesKey++;
  UNDODUMP;
}

int CUndoManager::CanUndo( CStringArray* IDs )
{
  if ( IDs )
  {
    for ( int i = 1; i < m_nCurState; i++ )
      IDs->Add( m_StateIDs.GetAt( i ) );
  }
  return m_nCurState - 1;
}

int CUndoManager::CanRedo( CStringArray* IDs )
{
  if ( IDs )
  {
    for ( int i = m_nCurState; i < m_StateIDs.GetSize(); i++ )
      IDs->Add( m_StateIDs.GetAt( i ) );
  }
  return (int)m_StateIDs.GetSize() - m_nCurState;
}

LPCTSTR CUndoManager::UndoStart( int steps )
{
  m_ChangesKey++;
  m_Frozen = true;
  int nPos = m_nCurState - steps;
  if ( (nPos > 0) && (nPos <= m_States.GetSize()) )
  {
    m_nCurState = nPos;
    UNDODUMP;
    return m_States.GetAt( nPos - 1 );
  }
  UNDODUMP;
  return NULL;
}

void CUndoManager::UndoEnd()
{
  m_Frozen = false;
}

#ifdef DEBUG_UNDO_MANAGER

void CUndoManager::Dump()
{
  TRACE( " UNDO MANAGER DUMP\n " );
  TRACE( "  m_States registered: %d\n" , m_States.GetSize() );
  TRACE( "  Current state: %d\n" , m_nCurState );
  CStringArray redoes;
  TRACE( "  REDO: Can redo %d step(s)\n" , CanRedo( &redoes ) );
  while ( redoes.GetSize() )
  {
    int i = redoes.GetSize() - 1;
    TRACE( "  (%d) %s\n" , i , redoes.GetAt( i ) );
    redoes.RemoveAt( i );
  }
  CStringArray undoes;
  TRACE( "  UNDO: Can undo %d step(s)\n" , CanUndo( &undoes ) );
  while ( undoes.GetSize() )
  {
    int i = undoes.GetSize() - 1;
    TRACE( "  (%d) %s\n" , i , undoes.GetAt( i ) );
    undoes.RemoveAt( i );
  }
  TRACE( "-------------------\n" );
}

#endif

void CUndoManager::Reset()
{
  m_ChangesKey++;
  m_nCurState = 0;
  m_States.RemoveAll();
  m_StateIDs.RemoveAll();
}

