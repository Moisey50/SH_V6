#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "BaseDataModel.cpp"
#include <gadgets\gadbase.h>
#include <gadgets\ContainerFrame.h>
#include <gadgets\MetafileFrame.h>
#include <gadgets\waveframe.h>
#include <fxfc/FXRegistry.h>
#include <time.h>

// global

bool Tvdb400_TypesCompatible(datatype output_type, datatype input_type)
{
    if ((input_type==transparent) || (output_type==transparent)) return true;
	return (output_type % input_type == 0);
}

bool Tvdb400_FrameLabelMatch(const CDataFrame* DataFrame, LPCTSTR label)
{
	if (!label || !_tcscmp(DataFrame->GetLabel(), label))
		return true;
	return false;
}

bool Tvdb400_IsEOS(const CDataFrame* pDataFrame)
{
	if (!pDataFrame) 
		return false;
	return (pDataFrame->GetId() == END_OF_STREAM_MARK);
}

void Tvdb400_SetEOS(CDataFrame* pDataFrame)
{
	pDataFrame->ChangeId(END_OF_STREAM_MARK);
    pDataFrame->SetRegistered();
}

bool Tvdb400_Serialize(CDataFrame* pDataFrame, LPBYTE* ppData, FXSIZE* cbData)
{
  UINT type = pDataFrame->GetDataType();
  FXSIZE cb;
	if (pDataFrame->IsContainer())
		type = (UINT)transparent; // see CContainer::Serialize()
	LPBYTE lpData;
	if (!pDataFrame->Serialize(&lpData, &cb))
		return false;
	*cbData = sizeof(type) + sizeof(cb) + cb;
	*ppData = (LPBYTE)malloc(*cbData);
	LPBYTE ptr = *ppData;
	memcpy(ptr, &type, sizeof(type));
	ptr += sizeof(type);
	memcpy(ptr, &cb, sizeof(cb));
	ptr += sizeof(cb);
	memcpy(ptr, lpData, cb);
	free(lpData);
	return true;
}

CDataFrame* Tvdb400_Restore(LPBYTE lpData, FXSIZE cbData)
{
  UINT type ;
  FXSIZE cb;
	if (cbData < sizeof(type) + sizeof(cb))
		return NULL;
	LPBYTE ptr = lpData;
	memcpy(&type, ptr, sizeof(type));
	ptr += sizeof(type);
	memcpy(&cb, ptr, sizeof(cb));
	ptr += sizeof(cb);
	CDataFrame* pDataFrame = NULL;
	if (type == transparent)
		pDataFrame = CContainerFrame::Create();
	else if (type == nulltype)
		pDataFrame = CDataFrame::Create();
	else if (type == vframe)
		pDataFrame = CVideoFrame::Create();
	else if (type == text)
		pDataFrame = CTextFrame::Create();
//	else if (type == wave)
//		pDataFrame = CWaveFrame::Create();
	else if (type == quantity)
		pDataFrame = CQuantityFrame::Create(0);
	else if (type == logical)
		pDataFrame = CBooleanFrame::Create();
	else if (type == rectangle)
		pDataFrame = CRectFrame::Create();
	else if (type == figure)
		pDataFrame = CFigureFrame::Create();
	else if (type == metafile)
		pDataFrame = CMetafileFrame::Create();
	else if (type == userdata) // Serialize and Restore for CUserDataFrame can not be properly used now and must be re-written
		pDataFrame = NULL;
	if (!pDataFrame)
		return NULL;
	if (!pDataFrame->Restore(ptr, cb))
	{
		pDataFrame->Release();
		return NULL;
	}
	return pDataFrame;
}

//
// CDataFrame
//

CDataFrame::CDataFrame(datatype dt):
m_DataType(dt),
m_cUsers(0),
m_ID(NOSYNC_FRAME),
m_Time( GetHRTickCount() ),
m_AbsTime( GetHRTickCount() ) ,
m_Label(_T("")),
m_Registered(false)
{
	AddRef();
}

CDataFrame::~CDataFrame()
{
}

datatype CDataFrame::GetDataType() const
{
	return m_DataType;
}

int CDataFrame::AddRef(int nRefs)
{
	m_Lock.Lock();
    while (nRefs)
    {
        InterlockedIncrement(&m_cUsers); nRefs--;
    }

	m_Lock.Unlock();
    return m_cUsers;
}

#ifdef _TRACE_DATAFRAMERELEASE
bool CDataFrame::Release(CDataFrame* Frame, LPCTSTR Name)
{
	m_Lock.Lock();
    if (!Frame) 
        Frame=this;
	if (this != Frame)
	{
		m_Lock.Unlock();
		return false;
	}
	ASSERT(m_cUsers > 0);
	InterlockedDecrement(&m_cUsers);
	if (!m_cUsers)
	{
		m_Lock.Unlock();
    if ( Name )
    {
      TRACE( "It's about to delete DataFrame 0x%x with ID = %d by gadget '%s'\n" , this , this->GetId() , Name );
    }
		delete this; // data frame is always allocated dynamically
	}
	else
    {
    if ( Name )
    {
      TRACE( "DataFrame 0x%x with ID = %d released by gadget '%s', m_cUsers=%d\n" , this , this->GetId() , Name , m_cUsers );
    }
		m_Lock.Unlock();
    }
	return true;
}
#else
bool CDataFrame::Release(CDataFrame* Frame)
{
	m_Lock.Lock();
  if (!Frame) 
      Frame=this;
	if (this != Frame)
	{
		m_Lock.Unlock();
		return false;
	}
	ASSERT(m_cUsers > 0);
	InterlockedDecrement(&m_cUsers);
	if (!m_cUsers)
	{
		m_Lock.Unlock();
		//TRACE(" !! Dataframe 0x%x is created!\n", this);
		delete this; // data frame is always allocated dynamically
	}
	else
		m_Lock.Unlock();
	return true;
}
#endif
 
CDataFrame* CDataFrame::Create(datatype dt)
{
	return new CDataFrame(dt);
}

CDataFrame* CDataFrame::CreateFrom(void* pData, UINT cData)
{
	if (!pData)
		return CDataFrame::Create();
	return CDataFrame::Create((datatype)((FXSIZE)pData));
}

CDataFrame* CDataFrame::Copy() const         
{ 
    CDataFrame* pNewFrame=Create();
    pNewFrame->m_DataType=m_DataType;
    pNewFrame->CopyAttributes(this);
    return pNewFrame; 
}

void CDataFrame::CopyAttributes(
  const CDataFrame* src , bool bCopyAbsTime )
{
    if (src)
    {
        m_Registered = src->m_Registered;
        m_ID        = src->m_ID;
        m_Label     = src->m_Label;
        m_Time      = src->m_Time;
        if ( bCopyAbsTime )
          m_AbsTime = src->m_AbsTime ;
        m_Attributes  = src->m_Attributes;
    }
}

CDataFrame* CDataFrame::GetDataFrame(LPCTSTR label)
{
    if (Tvdb400_FrameLabelMatch(this,label))
        return this;
	CDataFrame* Frame = NULL;
	CFramesIterator* Iterator = CreateFramesIterator(nulltype);
	if (Iterator)
	{
		Frame = Iterator->Next(label);
		delete Iterator;
	}
	return Frame;
}

const CDataFrame* CDataFrame::GetDataFrame(LPCTSTR label) const
{
    if (Tvdb400_FrameLabelMatch(this,label))
        return this;
	CDataFrame* Frame = NULL;
	CFramesIterator* Iterator = CreateFramesIterator(nulltype);
	if (Iterator)
	{
		Frame = Iterator->Next(label);
		delete Iterator;
	}
	return Frame;
}

BOOL CDataFrame::Serialize( LPBYTE* ppData , FXSIZE* cbData ) const
{
  if ( !ppData ) // for origin of derivative frame returning
    return TRUE;

  FXSIZE uiLabelLen = m_Label.GetLength() + 1 ; // trailing zero is included
  FXSIZE uiAttribLen = m_Attributes.GetLength() + 1 ; // trailing zero is included
  *cbData = sizeof( DestSerDataFrame ) - 1 + uiLabelLen + uiAttribLen ; // 1 is m_LabelAndAttrib len
  *ppData = ( LPBYTE ) malloc( *cbData );
  LPBYTE ptr = *ppData;
  DestSerDataFrame * pDest = ( DestSerDataFrame * ) ptr ;

  pDest->m_bRegistered = m_Registered ;
  pDest->m_ID = m_ID ;
  pDest->m_dTime = m_Time ;
  pDest->m_dAbsTime = m_AbsTime ;
  pDest->m_DataType = m_DataType ;
  strcpy_s( pDest->m_LabelAndAttrib , uiLabelLen , m_Label ) ;
  strcpy_s( pDest->m_LabelAndAttrib + uiLabelLen , uiAttribLen , m_Attributes ) ;
//     memcpy(ptr, &m_Time, sizeof(m_Time));
//     ptr += sizeof(m_Time);
//     memcpy(ptr, &m_AbsTime, sizeof(m_AbsTime));
//     ptr += sizeof(m_AbsTime);
//     memcpy(ptr, &m_DataType, sizeof(m_DataType));
//     ptr += sizeof(m_DataType);
//     memcpy(ptr, (LPCTSTR)m_Label, iLabelLen );
//     ptr += iLabelLen ;
//     //memcpy( ptr , ( LPCTSTR ) m_Attributes , iAttribLen );
  return TRUE;
}
BOOL CDataFrame::SerializeCommon(
  LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
{
  FXSIZE uiLabelLen , uiAttribLen ;
  FXSIZE uiAdditionLen = GetSerializeLength( uiLabelLen , &uiAttribLen ) ;
  if ( !pBufferOrigin || ( CurrentWriteIndex + uiAdditionLen >= BufLen ) )
    return FALSE ;

  FXSIZE iRegToAbsTimeLen = sizeof( m_Registered )
    + sizeof( m_ID ) + sizeof( m_Time ) + sizeof( m_AbsTime ) ;

  LPBYTE ptr = pBufferOrigin + CurrentWriteIndex ;
  DestSerDataFrame * pDest = ( DestSerDataFrame * ) ptr ;

  pDest->m_bRegistered = m_Registered ;
  pDest->m_ID = m_ID ;
  pDest->m_dTime = m_Time ;
  pDest->m_dAbsTime = m_AbsTime ;
  pDest->m_DataType = m_DataType ;
  strcpy_s( pDest->m_LabelAndAttrib , uiLabelLen , m_Label ) ;
  strcpy_s( pDest->m_LabelAndAttrib + uiLabelLen , uiAttribLen , m_Attributes ) ;
//     memcpy( ptr , &m_Registered , iRegToAbsTimeLen );
//     ptr += iRegToAbsTimeLen ;
//     memcpy( ptr , &m_DataType , sizeof( m_DataType ) );
//     ptr += sizeof( m_DataType );
//     if ( uiLabelLen > 1 )
//       memcpy( ptr , ( LPCTSTR ) m_Label , uiLabelLen );
//     else
//       *( ptr++ ) = 0 ;
//     if ( uiAttribLen > 1 )
//       memcpy( ptr , ( LPCTSTR ) m_Attributes , uiAttribLen );
//     else
//       *( ptr++ ) = 0 ;
  CurrentWriteIndex += uiAdditionLen ;
  return TRUE;
}

FXSIZE CDataFrame::GetSerializeLength( FXSIZE& uiLabelLen ,
  FXSIZE * pAttribLen ) const
{
  uiLabelLen = m_Label.GetLength() + 1 ; // trailing zero is included
  FXSIZE AdditionLen = sizeof( DestSerDataFrame ) - 1 + uiLabelLen ; // 1 is m_LabelAndAttrib len ;
  if ( pAttribLen )
  {
    *pAttribLen = m_Attributes.GetLength() + 1 ; // trailing zero is included
    AdditionLen += *pAttribLen ;
  }
  return AdditionLen ;
}

BOOL CDataFrame::Restore( LPBYTE lpData , FXSIZE cbData )
{
  if ( cbData < sizeof( m_Registered ) + sizeof( m_ID ) + sizeof( m_Time )
    + sizeof( m_AbsTime ) + sizeof( m_DataType ) + 1 )
    return FALSE;
  LPBYTE ptr = lpData;
  DestSerDataFrame * pSrc = ( DestSerDataFrame * ) ptr ;
  m_Registered = pSrc->m_bRegistered ;
  m_ID = pSrc->m_ID ;
  m_Time = pSrc->m_dTime ;
  m_AbsTime = pSrc->m_dAbsTime ;
  m_DataType = pSrc->m_DataType ;
  m_Label = pSrc->m_LabelAndAttrib ;
  size_t LabelLen = m_Label.GetLength() + 1 ; // with trailing zero
  m_Attributes = pSrc->m_LabelAndAttrib + LabelLen ;
//     memcpy(&m_Registered, ptr, sizeof(m_Registered));
//     ptr += sizeof(m_Registered);
//     memcpy(&m_ID, ptr, sizeof(m_ID));
//     ptr += sizeof(m_ID);
//     memcpy(&m_Time, ptr, sizeof(m_Time));
//     ptr += sizeof(m_Time);
//     memcpy(&m_AbsTime, ptr, sizeof(m_AbsTime));
//     ptr += sizeof(m_AbsTime);
//     memcpy(&m_DataType, ptr, sizeof(m_DataType));
//     ptr += sizeof(m_DataType);
//     m_Label = (LPCTSTR)ptr;
//     ptr += strlen( ( LPCTSTR )ptr ) + 1 ;
//     m_Attributes = ( LPCTSTR ) ptr ;
  return TRUE;
}

void CDataFrame::ToLogString(FXString& Output)
{
  FXString tmpS;
  tmpS.Format("%s:%s[%u] ",
    Tvdb400_TypeToStr(GetDataType()), GetLabel() , GetId());
  Output += tmpS;
}
