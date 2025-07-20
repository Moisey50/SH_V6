// ContFrame.cpp: implementation of the CContainerFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\ContainerFrame.h>
#include <gadgets\MetafileFrame.h>
#include <gadgets\waveframe.h>
#ifdef DEBUG_CONTAINER_RELEASE
int FormFrameTextView(
  const CDataFrame * pDataFrame , FXString& outp ,
  FXString& Prefix , int& iNFrames )
{
  if ( !pDataFrame )
    return 0 ;
  
  int iNScanned = 1 , iNChilds = 0 ;
  FXString tmpS , Addition ;
  tmpS.Format( "%s %3d %s(%d) [%u-%u" , (LPCTSTR) Prefix , iNFrames ,
    pDataFrame->IsContainer() ?
    _T( "Cont." ) : Tvdb400_TypeToStr( pDataFrame->GetDataType() ) ,
    pDataFrame->IsContainer() ? 
    ((CContainerFrame*)pDataFrame)->GetFramesCount() : 1 ,
    pDataFrame->GetId() , pDataFrame->GetUserCnt() );
  Addition += tmpS;

  tmpS.Format( ":'%s'-p=0x%p]" , pDataFrame->GetLabel() , pDataFrame);
  Addition += tmpS;
  iNFrames++ ;
  if ( pDataFrame->IsContainer() )
  {
    Addition += _T( "\n" ) ;
    FXString OldPrefix = Prefix ;
    Prefix += _T( "  " ) ;
    CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( transparent );
    if ( Iterator )
    {
      const CDataFrame* pNextFrame = Iterator->NextChild( DEFAULT_LABEL );
      while ( pNextFrame )
      {
        iNScanned++ ;
        iNChilds = FormFrameTextView( pNextFrame , Addition , Prefix , iNFrames );
        pNextFrame = Iterator->NextChild( DEFAULT_LABEL );
      }
      delete Iterator;
    }
    Prefix = OldPrefix ;
  }
  else
  {
    switch ( pDataFrame->GetDataType() )
    {
    case text:
      {
        const CTextFrame* TextFrame = pDataFrame->GetTextFrame();
        FXString Text = TextFrame->GetString();
        int iTextLen = (int) Text.GetLength();
        bool bIsCRLF = iTextLen
          && (Text[ iTextLen - 1 ] == '\r' || Text[ iTextLen - 1 ] == '\n');
        tmpS.Format( "%s%s" ,
          (LPCTSTR) Text , (bIsCRLF) ? "" : "\r\n" );
      }
      break;
    case quantity:
      {
        const CQuantityFrame* QuantityFrame =
          pDataFrame->GetQuantityFrame();
        tmpS.Format( "%s\r\n" ,
          QuantityFrame->ToString() );
      }
      break;
    case logical:
      {
        const CBooleanFrame* BooleanFrame =
          pDataFrame->GetBooleanFrame();
        tmpS.Format( "%s\r\n" ,
          (BooleanFrame->operator bool() ? "true" : "false") );
      }
      break;
    case rectangle:
      {
        const CRectFrame* RectFrame = pDataFrame->GetRectFrame();
        CRect rc = (LPRECT) RectFrame;
        tmpS.Format( "(%d,%d,%d,%d)\r\n" ,
          rc.left , rc.top , rc.right , rc.bottom );
      }
      break;
    case figure:
      {
        const CFigureFrame * pFig = pDataFrame->GetFigureFrame() ;
        tmpS.Format( "\t%d pts\r\n" , pFig->GetCount() ) ;
      }
      break ;
    case vframe:
      {
        const CVideoFrame * pFr = pDataFrame->GetVideoFrame() ;
        if ( pFr && pFr->lpBMIH )
        {
          tmpS.Format( "\t[%dx%d]\r\n" ,
            pFr->lpBMIH->biWidth , pFr->lpBMIH->biHeight ) ;
        }
      }
      break ;
    default:
      tmpS = "\r\n";
      break;
    }
    Addition += Prefix + tmpS ;
  }
  outp += Addition ;
  return iNScanned + iNChilds ;
}
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

class CContainerIterator : public CFramesIterator
{
  const CContainerFrame* m_Container;
  CFramesIterator* m_SubIterator;
  datatype m_Type;
  int m_nIndex;
public:
  CContainerIterator( datatype type , const CContainerFrame* frame );
  virtual ~CContainerIterator();
  virtual CDataFrame* Next( LPCTSTR label = "" );
  virtual CDataFrame* NextChild( LPCTSTR label = "" );
};

CContainerIterator::CContainerIterator( datatype type , const CContainerFrame* frame ) :
m_Type( type ) ,
m_Container( frame ) ,
m_nIndex( 0 ) ,
m_SubIterator( NULL )
{
}

CContainerIterator::~CContainerIterator()
{
  if ( m_SubIterator )
    delete m_SubIterator;
  m_SubIterator = NULL;
}

CDataFrame* CContainerIterator::Next( LPCTSTR label ) //recursive function!!!
{
  ( ( CContainerFrame* ) m_Container )->m_Lock.Lock();
  CDataFrame* DataFrame = NULL;
  if ( m_SubIterator )
  {
    DataFrame = m_SubIterator->Next( label );
    if ( DataFrame )
    {
      ( ( CContainerFrame* ) m_Container )->m_Lock.Unlock();
      return DataFrame;
    }
    delete m_SubIterator;
    m_SubIterator = NULL;
  }
  ASSERT( !m_SubIterator );
  while ( true )
  {
    if ( m_nIndex >= (int) m_Container->m_Frames.GetSize() )
    {
      ( ( CContainerFrame* ) m_Container )->m_Lock.Unlock();
      return NULL;
    }
    DataFrame = ( CDataFrame* ) m_Container->m_Frames.GetAt( m_nIndex++ );
    ASSERT( DataFrame );
    if ( ( ( label != DEFAULT_LABEL ) || ( !DataFrame->IsContainer() ) ) &&
      Tvdb400_TypesCompatible( DataFrame->GetDataType() , m_Type ) &&
      ( ( DataFrame->GetDataType() != transparent ) || ( ( m_Type == transparent ) &&
      ( DataFrame->GetDataType() == transparent ) ) ) &&
      Tvdb400_FrameLabelMatch( DataFrame , label )
      )
    {
      ( ( CContainerFrame* ) m_Container )->m_Lock.Unlock();
      return DataFrame;
    }
    else
    {
      m_SubIterator = DataFrame->CreateFramesIterator( m_Type );
      if ( m_SubIterator )
      {
        ( ( CContainerFrame* ) m_Container )->m_Lock.Unlock();
        return Next( label );
      }
    }
  };
}

CDataFrame* CContainerIterator::NextChild( LPCTSTR label )
{
  FXAutolock al( ( ( CContainerFrame* ) m_Container )->m_Lock );
  CDataFrame* DataFrame = NULL;
  while ( m_nIndex < (int) m_Container->m_Frames.GetSize() )
  {
    DataFrame = ( CDataFrame* ) m_Container->m_Frames.GetAt( m_nIndex++ );
    if ( Tvdb400_TypesCompatible( DataFrame->GetDataType() , m_Type ) &&
      Tvdb400_FrameLabelMatch( DataFrame , label ) )
    {
      return DataFrame;
    }
  }
  return NULL;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CContainerFrame::CContainerFrame()
  : m_CheckMode(NoCheck)
  , m_uiSerializedLength( 0 )
  , m_uiNAllSubframes( 0 )
{
  m_DataType = transparent ;
  m_Frames.SetSize( 0 , 1 );
}

CContainerFrame::~CContainerFrame()
{
#ifdef DEBUG_CONTAINER_RELEASE
  FXString ContList , Prefix ;
  int iLevel = 0 ;
  int iNFrames = FormFrameTextView(
    (CDataFrame*) this , ContList , Prefix , iLevel ) ;
#endif
  while ( m_Frames.GetSize() )
  {
    CDataFrame* Frame = ( CDataFrame* ) m_Frames.GetAt( 0 );
    ASSERT( Frame );
    Release( Frame );
  }
}

#ifdef _TRACE_DATAFRAMERELEASE
bool CContainerFrame::Release(CDataFrame* Frame, LPCTSTR Name)
{
  m_Lock.Lock();
  if (!Frame) 
    Frame=this;
  if (Frame == this)
  {
    m_Lock.Unlock();
    return CDataFrame::Release(Frame, Name);
  }
  for (int i = 0; i < m_Frames.GetSize(); i++)
  {
    CDataFrame* pFrame = (CDataFrame*)m_Frames.GetAt(i);
    if (pFrame->Release(Frame, Name))
    {
      if (pFrame == Frame)
        m_Frames.RemoveAt(i);
      m_Lock.Unlock();
      return true;
    }
  }
  m_Lock.Unlock();
  return false;
}
#else
bool CContainerFrame::Release( CDataFrame* Frame )
{
  m_Lock.Lock();
  if ( !Frame )
    Frame = this;
  if ( Frame == this )
  {
    m_Lock.Unlock();
    return CDataFrame::Release( Frame );
  }
  for ( int i = 0; i < (int) m_Frames.GetSize(); i++ )
  {
    CDataFrame* pFrame = ( CDataFrame* ) m_Frames.GetAt( i );
    if ( pFrame->Release( Frame ) )
    {
      if ( pFrame == Frame )
        m_Frames.RemoveAt( i );
      m_Lock.Unlock();
      return true;
    }
  }
  m_Lock.Unlock();
  return false;
}
#endif

FXSIZE CContainerFrame::GetSerializeLength( FXSIZE& uiLabelLen ,
  UINT& uiNFramesInContainer , FXSIZE * pAttribLen )
{
  if ( m_uiSerializedLength && ( m_Frames.size() == m_SerializedLengthes.size() ) )
  {
    uiLabelLen = m_Label.GetLength() + 1 ; // trailing zero is included
    if ( pAttribLen )
      *pAttribLen = Attributes()->GetLength() + 1 ; // trailing zero is included
    uiNFramesInContainer = m_uiNAllSubframes ;
#ifdef SERIALIZE_DEBUG
    TRACE( "Cont.old data Lab=%s Lser=%u Lflw=%u Nasf=%u Nfr=%u\n" , GetLabel() , 
      m_uiSerializedLength , m_uiSerializedLength + FLW_ADDITION ,
      m_uiNAllSubframes , uiNFramesInContainer ) ;
#endif

    return m_uiSerializedLength ; // already calculated
  }

  int Len = (int)CDataFrame::GetSerializeLength( uiLabelLen , pAttribLen ) ;
 
  FXSIZE uiLocalLabelLen , uiLocalAttribLen ;
  size_t iNSubFrames = m_Frames.GetSize();
  int iNAllFramesBefore = uiNFramesInContainer ;
  uiNFramesInContainer += (UINT)iNSubFrames ;
  m_SerializedLengthes.SetSize( iNSubFrames ) ;
  while ( iNSubFrames > 0 )
  {
    CDataFrame* pDataFrame = ( CDataFrame* ) m_Frames.GetAt( --iNSubFrames );
    ASSERT( pDataFrame );
    int iFrameSerLength = 0 ;
    if ( pDataFrame->IsContainer() )
    {
      CContainerFrame * pContainer = ( CContainerFrame* ) pDataFrame ;
      iFrameSerLength = ( int ) ( pContainer->GetSerializeLength( uiLocalLabelLen , uiNFramesInContainer , &uiLocalAttribLen ) );
#ifdef SERIALIZE_DEBUG
      TRACE( "    SubContainer Lab=%s Lser=%li Lflw=%li Nasf=%u \n" , 
        pDataFrame->GetLabel() , iFrameSerLength , iFrameSerLength + FLW_ADDITION ,
        pContainer->GetNAllSubFrames() ) ;
#endif
    }
    else
    {
      iFrameSerLength = (int)( (( const CDataFrame* ) pDataFrame)
        ->GetSerializeLength( uiLocalLabelLen , &uiLocalAttribLen ) );
#ifdef SERIALIZE_DEBUG
      TRACE( "   %s Lab=%s Lser=%li Lflw=%li \n" ,
        Tvdb400_TypeToStr( pDataFrame->GetDataType() ) ,
        pDataFrame->GetLabel() , iFrameSerLength , iFrameSerLength + FLW_ADDITION ) ;
#endif
    }

    Len += iFrameSerLength ;
    m_SerializedLengthes.SetAt( iNSubFrames , iFrameSerLength );
  }
  m_uiSerializedLength = (UINT)Len ;
  m_uiNAllSubframes = uiNFramesInContainer - iNAllFramesBefore ;
#ifdef SERIALIZE_DEBUG
  TRACE( "Container Lab=%s Lser=%u Lflw=%u Nasf=%u Nfr=%u\n" , GetLabel() , 
    Len , Len + FLW_ADDITION , m_uiNAllSubframes , uiNFramesInContainer ) ;
#endif
  return Len ;
};


CVideoFrame* CContainerFrame::GetVideoFrame( LPCTSTR label )
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( vframe );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetVideoFrame( label );
}

const CVideoFrame* CContainerFrame::GetVideoFrame( LPCTSTR label ) const
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( vframe );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetVideoFrame( label );
}

CTextFrame* CContainerFrame::GetTextFrame( LPCTSTR label )
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( text );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetTextFrame( label );
}

const CTextFrame* CContainerFrame::GetTextFrame( LPCTSTR label ) const
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( text );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetTextFrame( label );
}

CWaveFrame* CContainerFrame::GetWaveFrame( LPCTSTR label )
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( wave );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetWaveFrame( label );
}

const CWaveFrame* CContainerFrame::GetWaveFrame( LPCTSTR label ) const
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( wave );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetWaveFrame( label );
}

CQuantityFrame* CContainerFrame::GetQuantityFrame( LPCTSTR label )
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( quantity );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetQuantityFrame( label );
}

const CQuantityFrame* CContainerFrame::GetQuantityFrame( LPCTSTR label ) const
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( quantity );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetQuantityFrame( label );
}

CBooleanFrame* CContainerFrame::GetBooleanFrame( LPCTSTR label )
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( logical );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetBooleanFrame( label );
}

const CBooleanFrame* CContainerFrame::GetBooleanFrame( LPCTSTR label ) const
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( logical );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetBooleanFrame( label );
}

CRectFrame* CContainerFrame::GetRectFrame( LPCTSTR label )
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( rectangle );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetRectFrame( label );
}

const CRectFrame* CContainerFrame::GetRectFrame( LPCTSTR label ) const
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( rectangle );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetRectFrame( label );
}

CFigureFrame* CContainerFrame::GetFigureFrame( LPCTSTR label )
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( figure );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetFigureFrame( label );
}

const CFigureFrame* CContainerFrame::GetFigureFrame( LPCTSTR label ) const
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( figure );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetFigureFrame( label );
}

const CArrayFrame* CContainerFrame::GetArrayFrame( LPCTSTR label ) const
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( arraytype );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetArrayFrame( label );
}

CArrayFrame* CContainerFrame::GetArrayFrame( LPCTSTR label )
{
  CDataFrame* Frame = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( arraytype );
  if ( Iterator )
  {
    Frame = Iterator->Next( label );
    delete Iterator;
  }
  if ( !Frame )
    return NULL;
  return Frame->GetArrayFrame( label );
}

CUserDataFrame*	CContainerFrame::GetUserDataFrame( LPCTSTR uType , LPCTSTR label )
{
  CDataFrame* Frame = NULL;
  CUserDataFrame* udf = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( userdata );
  if ( Iterator )
  {
    do
    {
      Frame = Iterator->Next( label );
      if ( !Frame ) break;
      udf = Frame->GetUserDataFrame( uType , label );
    } while ( udf == NULL );
    delete Iterator;
  }
  return udf;
}

const CUserDataFrame*	CContainerFrame::GetUserDataFrame( LPCTSTR uType , LPCTSTR label ) const
{
  CDataFrame* Frame = NULL;
  CUserDataFrame* udf = NULL;
  CFramesIterator* Iterator = CreateFramesIterator( userdata );
  if ( Iterator )
  {
    do
    {
      Frame = Iterator->Next( label );
      if ( !Frame ) break;
      udf = Frame->GetUserDataFrame( uType , label );
    } while ( udf == NULL );
    delete Iterator;
  }
  return udf;
}

void CContainerFrame::AddFrame( CDataFrame* DataFrame )
{
  m_Lock.Lock();
  ASSERT( DataFrame );
#ifdef _DEBUG
  if ( m_CheckMode != NoCheck )
  {
    ASSERT( !IsInContainer( DataFrame ) ) ;
  }
#endif
  datatype type = DataFrame->GetDataType();
  if ( m_DataType == transparent )
    m_DataType = type;
  else
    if ( ( !Tvdb400_TypesCompatible( m_DataType , type ) ) && ( type != transparent ) )
      m_DataType *= type;
  m_Frames.InsertAt( 0 , DataFrame );
  m_Lock.Unlock();
}

void CContainerFrame::AddFrame( const CDataFrame* DataFrame )
{
  ASSERT( DataFrame );
  m_Lock.Lock();
  datatype type = DataFrame->GetDataType();
  if ( m_DataType == transparent )
    m_DataType = type;
  else
    if ( ( !Tvdb400_TypesCompatible( m_DataType , type ) ) && ( type != transparent ) )
      m_DataType *= type;
  ( ( CDataFrame* ) DataFrame )->AddRef();
  m_Frames.InsertAt( 0 , ( CDataFrame* ) DataFrame );
  m_Lock.Unlock();
}

void CContainerFrame::PushFrame( CDataFrame* DataFrame )
{
  m_Lock.Lock();
  ASSERT( DataFrame );
#ifdef _DEBUG
  if ( m_CheckMode != NoCheck )
  {
    ASSERT( !IsInContainer( DataFrame ) ) ;
  }
#endif
  datatype type = DataFrame->GetDataType();
  if ( m_DataType == transparent )
    m_DataType = type;
  else
    if ( ( !Tvdb400_TypesCompatible( m_DataType , type ) ) && ( type != transparent ) )
      m_DataType *= type;
  m_Frames.Add( DataFrame );
  m_Lock.Unlock();
}


void CContainerFrame::PushFrame( const CDataFrame* DataFrame )
{
  m_Lock.Lock();
  ASSERT( DataFrame );
#ifdef _DEBUG
  if ( m_CheckMode != NoCheck )
  {
    ASSERT( !IsInContainer( DataFrame ) ) ;
  }
#endif
  datatype type = DataFrame->GetDataType();
  if ( m_DataType == transparent )
    m_DataType = type;
  else
    if ( ( !Tvdb400_TypesCompatible( m_DataType , type ) ) && ( type != transparent ) )
      m_DataType *= type;
  m_Frames.Add( ( CDataFrame* ) DataFrame );
  m_Lock.Unlock();
}


bool CContainerFrame::InsertAfter( CDataFrame* RefFrame , CDataFrame* DataFrame )
{
  m_Lock.Lock();
  for ( int i = 0; i < (int) m_Frames.GetSize(); i++ )
  {
    CDataFrame* Frame = ( CDataFrame* ) m_Frames.GetAt( i );
    ASSERT( Frame );
    if ( Frame == RefFrame )
    {
      m_Frames.InsertAt( i + 1 , DataFrame );
      m_Lock.Unlock();
      return true;
    }
    CFramesIterator* Iterator = Frame->CreateFramesIterator( nulltype );
    if ( Iterator )
    {
      delete Iterator;
      if ( ( ( CContainerFrame* ) Frame )->InsertAfter( RefFrame , DataFrame ) )
      {
        m_Lock.Unlock();
        return true;
      }
    }
  }
  m_Lock.Unlock();
  return false;
}

CContainerFrame* CContainerFrame::Create()
{
  return new CContainerFrame();
}

CFramesIterator* CContainerFrame::CreateFramesIterator( datatype type ) const
{
  return new CContainerIterator( type , this );
}

CDataFrame* CContainerFrame::CopyContainer()
{
  CContainerFrame* Container = CContainerFrame::Create();
  m_Lock.Lock();
  Container->CopyAttributes( this );
  for ( size_t i = m_Frames.GetSize(); i > 0; i-- )
  {
    CDataFrame* Frame = ( CDataFrame* ) m_Frames.GetAt( i - 1 );
    ASSERT( Frame );
    Frame->AddRef();
    Container->AddFrame( Frame );
  }
  m_Lock.Unlock();
  return Container;
}

CDataFrame*     CContainerFrame::Copy() const
{
  FXAutolock al( ( ( CContainerFrame* )this )->m_Lock );
  CContainerFrame* Container = CContainerFrame::Create();
  Container->CopyAttributes( this );
  for ( size_t i = m_Frames.GetSize(); i > 0; i-- )
  {
    CDataFrame* Frame = ( ( CDataFrame* ) m_Frames.GetAt( i - 1 ) )->Copy();
    ASSERT( Frame != 0 );
    Container->AddFrame( Frame );
  }
  return Container;
}

BOOL CContainerFrame::Serialize( LPBYTE* ppData , FXSIZE* cbData ) const
{
  FXAutolock al( ( ( CContainerFrame* )this )->m_Lock );
  if ( !CDataFrame::Serialize( ppData , cbData ) )
    return FALSE;
  size_t i = m_Frames.GetSize();
  while ( i > 0 )
  {
    CDataFrame* pDataFrame = ( CDataFrame* ) m_Frames.GetAt( --i );
    ASSERT( pDataFrame );
    UINT type = (UINT) pDataFrame->GetDataType() ;
    FXSIZE cb;
    if ( pDataFrame->IsContainer() )
      type = ( UINT ) transparent; // denotes "container" here
    LPBYTE lpData;
    if ( !pDataFrame->Serialize( &lpData , &cb ) )
    {
      free( *ppData );
      return FALSE;
    }
    *ppData = ( LPBYTE ) realloc( *ppData , *cbData + sizeof( type ) + sizeof( cb ) + cb );
    LPBYTE ptr = *ppData + *cbData;
    memcpy( ptr , &type , sizeof( type ) );
    ptr += sizeof( type );
    memcpy( ptr , &cb , sizeof( cb ) );
    ptr += sizeof( cb );
    memcpy( ptr , lpData , cb );
    free( lpData );
    *cbData += sizeof( type ) + sizeof( cb ) + cb;
  }
  return TRUE;
}

BOOL CContainerFrame::Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
{
  FXAutolock al( ( ( CContainerFrame* )this )->m_Lock );
  if ( !CDataFrame::Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) )
    return FALSE;

  size_t i = m_Frames.GetSize();
  while ( i > 0 )
  {
    CDataFrame* pDataFrame = ( CDataFrame* ) m_Frames.GetAt( --i );
    ASSERT( pDataFrame );
    UINT type = (UINT) pDataFrame->GetDataType() ;
    FXSIZE SerializationLen ;
    if ( pDataFrame->IsContainer() )
      type = ( UINT ) transparent; // denotes "container" here
    FXSIZE FrameOriginIndex = CurrentWriteIndex ;
    CurrentWriteIndex += sizeof( type ) + sizeof( SerializationLen ) ; // reserve space for container id and len
    if ( !pDataFrame->Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) )
      return FALSE;

    LPBYTE ptr = pBufferOrigin + FrameOriginIndex ;
    memcpy( ptr , &type , sizeof( type ) );
    ptr += sizeof( type ) ;
    FXSIZE SerializedSize = CurrentWriteIndex - FrameOriginIndex ;
    memcpy( ptr , &SerializedSize , sizeof( SerializedSize ) );
  }
  return TRUE;
}

// BOOL CContainerFrame::SerializeInLine( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
// {
//   FXAutolock al( ( ( CContainerFrame* ) this )->m_Lock );
//   DestSerDataFrame * pFrameHeader = ( DestSerDataFrame * ) pBufferOrigin + CurrentWriteIndex ;
//   if ( !CDataFrame::Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) )
//     return FALSE;
// 
// 
//   size_t i = m_Frames.GetSize();
//   while ( i > 0 )
//   {
//     CDataFrame* pDataFrame = ( CDataFrame* ) m_Frames.GetAt( --i );
//     ASSERT( pDataFrame );
//     UINT type = ( UINT ) pDataFrame->GetDataType() ;
//     if ( pDataFrame->IsContainer() )
//       type = ( UINT ) transparent; // denotes "container" here
//     FXSIZE FrameOriginIndex = CurrentWriteIndex ;
//     FXSIZE SerializedOriginIndex = CurrentWriteIndex ;
//     if ( !pDataFrame->Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) )
//       return FALSE;
// 
//     LPBYTE ptr = pBufferOrigin + FrameOriginIndex ;
//     memcpy( ptr , &type , sizeof( type ) );
//     ptr += sizeof( type ) ;
//     FXSIZE SerializedSize = CurrentWriteIndex - SerializedOriginIndex ;
//     memcpy( ptr , &SerializedSize , sizeof( SerializedSize ) );
//   }
//   return TRUE;
// }

BOOL CContainerFrame::Restore( LPBYTE lpData , FXSIZE cbData )
{
  FXAutolock lock( m_Lock );
  FXSIZE cb;
  if ( !CDataFrame::Serialize( NULL , &cb ) || !CDataFrame::Restore( lpData , cbData ) )
    return FALSE;
  LPBYTE ptr = lpData + cb , end = lpData + cbData;
  while ( ptr < end )
  {
    UINT type;
    if ( ( end - ptr ) < sizeof( type ) + sizeof( cb ) )
      return FALSE;
    memcpy( &type , ptr , sizeof( type ) );
    ptr += sizeof( type );
    memcpy( &cb , ptr , sizeof( cb ) );
    ptr += sizeof( cb );
    if ( ( FXSIZE ) ( end - ptr ) < cb )
      return FALSE;
    CDataFrame* pDataFrame = NULL;
    if ( type == transparent )
      pDataFrame = CContainerFrame::Create();
    else if ( type == nulltype )
      pDataFrame = CDataFrame::Create();
    else if ( type == vframe )
      pDataFrame = CVideoFrame::Create();
    else if ( type == text )
      pDataFrame = CTextFrame::Create();
    //		else if (type == wave)
    //			pDataFrame = CWaveFrame::Create();
    else if ( type == quantity )
      pDataFrame = CQuantityFrame::Create( 0 );
    else if ( type == logical )
      pDataFrame = CBooleanFrame::Create();
    else if ( type == rectangle )
      pDataFrame = CRectFrame::Create();
    else if ( type == figure )
      pDataFrame = CFigureFrame::Create();
    else if ( type == metafile )
      pDataFrame = CMetafileFrame::Create();
    else if ( type == userdata ) // ???
      pDataFrame = NULL;
    if ( !pDataFrame )
      return FALSE;
    if ( !pDataFrame->Restore( ptr , cb ) )
    {
      pDataFrame->Release();
      return FALSE;
    }
    AddFrame( pDataFrame );
    ptr += cb;
  }
  return TRUE;
}

void CContainerFrame::ToLogString(FXString& Output)
{
  int iEnterPos = (int)Output.ReverseFind(_T('\n'));
  int iNSpaces = iEnterPos >= 0 ? (int)Output.GetLength() - iEnterPos - 1 : 0 ;
  FXString Spaces;
  if ( iNSpaces )
  {
    LPTSTR pSp = Spaces.GetBuffer(iNSpaces + 1);
    LPTSTR pEnd = pSp + iNSpaces;
    while (pSp < pEnd)
      *(pSp++) = _T(' ');
    *pSp = 0;
    Spaces.ReleaseBuffer();
  }
  CDataFrame::ToLogString(Output);
  Output += _T('\n');
  Spaces += _T("    ");
  size_t i = m_Frames.GetSize();
  while (i > 0)
  {
    CDataFrame* pDataFrame = (CDataFrame*)m_Frames.GetAt(--i);
    ASSERT(pDataFrame);
    Output += Spaces;
    pDataFrame->ToLogString(Output);
    Output += _T('\n');
  }
}

const bool CContainerFrame::IsFrameRepeated( FXString * pDescr )
{
  CFramesIterator * Iter = CreateFramesIterator( transparent ) ;
  if ( Iter )
  {
    FXPtrArray FramePtrs ;
    const CDataFrame* pNextFrame = Iter->Next( DEFAULT_LABEL );
    while ( pNextFrame )
    {
      FramePtrs.Add( (void*) pNextFrame ) ;
      pNextFrame = Iter->Next( DEFAULT_LABEL );
    }
    delete Iter;
    for ( int i = 0 ; i < FramePtrs.GetCount() ; i++ )
    {
      void * ptr = FramePtrs[ i ] ;
      for ( int j = i + 1 ; j < FramePtrs.GetCount() ; j++ )
      {
        if ( FramePtrs[ j ] == ptr )
        {
          if ( pDescr )
          {
            FXString Addition ;
            const CDataFrame *pFrame = (const CDataFrame*) ptr ;
            Addition.Format( "The Same Frame 0x%p '%s' Lab='%s' "
              "on positions %d and %d UserCnt=%d" ,
              ptr , (LPCTSTR) Tvdb400_TypeToStr( pFrame->GetDataType() ) ,
              pFrame->GetLabel() , i , j , pFrame->GetUserCnt() ) ;
          }
          return false ;
        }
      }
    }
  }
  return true ;
}

bool CContainerFrame::IsInContainer( const CDataFrame * pFrame )
{
  CFramesIterator * Iter = CreateFramesIterator( transparent ) ;
  if ( Iter )
  {
    FXPtrArray FramePtrs ;
    const CDataFrame* pNextFrame = Iter->Next( DEFAULT_LABEL );
    while ( pNextFrame )
    {
      if ( (void*) pNextFrame == (void*) pFrame )
      {
        delete Iter ;
        return true ;
      }
      pNextFrame = Iter->Next( DEFAULT_LABEL );
    }
    delete Iter;
  }
  return false ;
}

