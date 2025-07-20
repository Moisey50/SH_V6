// FigureFrame.cpp: implementation of the CFigureFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "math/hbmath.h"
#include <gadgets\FigureFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CFigure::CFigure()    
{  
}

CFigure::~CFigure()   
{ 
    Clean(); 
}

void CFigure::Clean() 
{ 
    RemoveAll( ); 
}
double  CFigure::GetFigureLength() const
{
  double dLength = 0. ;
  int i = 1 ;
  for ( int i = 1 ; i < Count() ; i++ )
    dLength += GetAt( i ).GetDist( GetAt( i - 1 ) )  ;
  return dLength ;
}
double  CFigure::GetConturLength() const
{
  double dLength = 0. ;
  int i = 1 ;
  for ( int i = 1 ; i < Count() ; i++ )
    dLength += GetAt( i ).GetDist( GetAt( i - 1 ) )  ;

  dLength += GetAt( 0 ).GetDist( GetAt( i - 1 ) ) ;
  return dLength ;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFigureFrame::CFigureFrame()
{
    m_DataType = figure;
}

CFigureFrame::CFigureFrame(const CFigureFrame* Frame)
{
	m_DataType = figure;
    Clean();
    if (Frame)
    {
        ASSERT(Frame->m_DataType==figure);
        CopyAttributes(Frame);
        CDPointArray::Copy(*Frame);
    }
}

CFigureFrame::~CFigureFrame()
{
    Clean();
}

CFigureFrame* CFigureFrame::Create(CFigureFrame* Frame)
{
    return new CFigureFrame(Frame);
}

CFigureFrame* CFigureFrame::Create(CFigure* Frame)
{
    CFigureFrame* retV=new CFigureFrame();
    retV->CDPointArray::Copy(*Frame);
    return retV;
}

BOOL CFigureFrame::Serialize(LPBYTE* ppData, FXSIZE* cbData) const
{
	ASSERT(ppData);
	FXSIZE cb;
	if (!CDataFrame::Serialize(ppData, &cb))
		return FALSE;
	*cbData = cb + sizeof(int) + GetSize() * sizeof(DPOINT);
	*ppData = (LPBYTE)realloc(*ppData, *cbData);
	LPBYTE ptr = *ppData + cb;
	FXSIZE count = GetSize(), i;
	memcpy(ptr, &count, sizeof(int));
	ptr += sizeof(int);
	for (i = 0; i < count; i++)
	{
		CDPoint dp = GetAt(i);
		memcpy(ptr, (DPOINT*)&dp, sizeof(DPOINT));
		ptr += sizeof(DPOINT);
	}
	return TRUE;
}

BOOL CFigureFrame::Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
{
  FXSIZE OriginIndex = CurrentWriteIndex , uiLabelLen , uiAttribLen ;
  FXSIZE AdditionLen = GetSerializeLength( uiLabelLen , &uiAttribLen ) ;
  if ( ( CurrentWriteIndex + AdditionLen ) >= BufLen )
    return FALSE ;

  CDataFrame::Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) ;
  LPBYTE ptr = pBufferOrigin + CurrentWriteIndex ;
  FXSIZE count = GetSize() ;
  memcpy( ptr , &count , sizeof( int ) );
  ptr += sizeof( int );
  memcpy( ptr , GetData() , GetSize() * sizeof( DPOINT ) ) ;

  CurrentWriteIndex = OriginIndex + AdditionLen ;
  return TRUE;
}

FXSIZE CFigureFrame::GetSerializeLength( FXSIZE& uiLabelLen ,
  FXSIZE * pAttribLen ) const
{
  FXSIZE Len = CDataFrame::GetSerializeLength( uiLabelLen , pAttribLen ) ;
  Len += sizeof( int ) + GetSize() * sizeof( DPOINT ) ;
  return Len ;
};

BOOL CFigureFrame::Restore(LPBYTE lpData, FXSIZE cbData)
{
	FXSIZE cb;
	if (!CDataFrame::Serialize(NULL, &cb) || !CDataFrame::Restore(lpData, cbData))
		return FALSE;
	LPBYTE ptr = lpData + cb;
	if (cbData - cb < sizeof(int))
		return FALSE;
	int count;
	memcpy(&count, ptr, sizeof(int));
	ptr += sizeof(int);
	if (cbData - cb - sizeof(int) < count * sizeof(DPOINT))
		return FALSE;
	int i;
	RemoveAll();
	for (i = 0; i < count; i++)
	{
		DPOINT dp;
		memcpy(&dp, ptr, sizeof(DPOINT));
		ptr += sizeof(DPOINT);
		Add(CDPoint(dp));
	}
	return TRUE;
}
void CFigureFrame::ToLogString(FXString& Output)
{
  CDataFrame::ToLogString(Output);
  Output += ToString() ;
}

CFigureFrame * CreatePtFrameEx( const cmplx& Pt ,
  DWORD dwColor , int iSize , int iThickness )
{
  CFigureFrame * pPt = CFigureFrame::Create() ;
  if ( pPt )
  {
    pPt->Add( CDPoint( Pt.real() , Pt.imag() ) ) ;
    FXString Color ;
    Color.Format( "0x%X" , dwColor ) ;
    pPt->Attributes()->WriteString( "color" , Color ) ;
    if ( iSize != 2 )
      pPt->Attributes()->WriteInt( "Sz" , iSize ) ;
    if ( iThickness != 1 )
      pPt->Attributes()->WriteInt( "thickness" , iSize ) ;
  }
  return pPt ;
}

CFigureFrame * CreatePtFrameEx( const cmplx& Pt , LPCTSTR Attributes )
{
  CFigureFrame * pPt = CFigureFrame::Create() ;
  if ( pPt )
  {
    pPt->Add( CDPoint( Pt.real() , Pt.imag() ) ) ;
    *( pPt->Attributes() ) = Attributes ;
  }
  return pPt ;
}

CFigureFrame * CreateLineFrameEx( const cmplx& Pt1 , const cmplx& Pt2 ,
  DWORD dwColor , int iThickness )
{
  CFigureFrame * pLine = CFigureFrame::Create() ;
  if ( pLine )
  {
    pLine->Add( CDPoint( Pt1.real() , Pt1.imag() ) ) ;
    pLine->Add( CDPoint( Pt2.real() , Pt2.imag() ) ) ;
    FXString Color ;
    Color.Format( "0x%X" , dwColor ) ;
    pLine->Attributes()->WriteString( "color" , Color ) ;
    if ( iThickness != 1 )
      pLine->Attributes()->WriteInt( "thickness" , iThickness ) ;
  }
  return pLine ;
}

CFigureFrame * CreateLineFrameEx( const cmplx& Pt1 , const cmplx& Pt2 ,
  LPCTSTR Attributes )
{
  CFigureFrame * pLine = CFigureFrame::Create() ;
  if ( pLine )
  {
    pLine->Add( CDPoint( Pt1.real() , Pt1.imag() ) ) ;
    pLine->Add( CDPoint( Pt2.real() , Pt2.imag() ) ) ;
    *( pLine->Attributes() ) = Attributes ;
  }
  return pLine ;
}

CFigureFrame * CreateFigureFrameEx( const CRect& Rect ,
  DWORD dwColor , int iThickness )
{
  CFigureFrame * pFigFrame = CFigureFrame::Create();
  if ( pFigFrame )
  {
    pFigFrame->AddPoint( Rect.left , Rect.top );
    pFigFrame->AddPoint( Rect.right , Rect.top );
    pFigFrame->AddPoint( Rect.right , Rect.bottom );
    pFigFrame->AddPoint( Rect.left , Rect.bottom );
    pFigFrame->AddPoint( Rect.left , Rect.top );
    FXString Color ;
    Color.Format( "0x%X" , dwColor ) ;
    pFigFrame->Attributes()->WriteString( "color" , Color ) ;
    if ( iThickness != 1 )
      pFigFrame->Attributes()->WriteInt( "thickness" , iThickness ) ;
  }
  return pFigFrame;
}

CFigureFrame * CreateFigureFrameEx( 
  const cmplx * Pts , int iLen , DWORD dwColor , int iThickness )
{
  CFigureFrame * pFigure = CFigureFrame::Create() ;
  if ( pFigure )
  {
    if ( iLen )
    {
    pFigure->SetSize( iLen ) ;
    memcpy_s( pFigure->GetData() , pFigure->GetCount() * sizeof( CDPoint ) ,
      Pts , iLen * sizeof( cmplx ) ) ;
    }
    FXString Color ;
    Color.Format( "0x%X" , dwColor ) ;
    pFigure->Attributes()->WriteString( "color" , Color ) ;
    if ( iThickness != 1 )
      pFigure->Attributes()->WriteInt( "thickness" , iThickness ) ;
  }
  return pFigure ;
}

CFigureFrame * CreateFigureFrameEx( CmplxVector& Data ,
  DWORD dwColor , int iThickness )
{
  return CreateFigureFrameEx( Data.data() , 
    (int)Data.size() , dwColor , iThickness ) ;
}

CFigureFrame * CreateFigureFrameEx( const cmplx * Pts , int iLen ,
  LPCTSTR pAttributes )
{
  CFigureFrame * pFigure = CFigureFrame::Create() ;
  if ( pFigure )
  {
    if ( iLen )
    {
    pFigure->SetSize( iLen ) ;
    memcpy_s( pFigure->GetData() , pFigure->GetCount() * sizeof( CDPoint ) ,
      Pts , iLen * sizeof( cmplx ) ) ;
    }
    *( pFigure->Attributes() ) = pAttributes ;
  }
  return pFigure ;
}

CFigureFrame * CreateFigureFrameEx( CmplxVector& Data , LPCTSTR pAttributes )
{
  return CreateFigureFrameEx( Data.data() ,
    ( int ) Data.size() , pAttributes ) ;
}

CFigureFrame * CreateFigureFrameEx( const double * pData ,
  int iLen , const cmplx& cOrigin , const cmplx& cGraphStep , 
  double dYShift , double dYScale , DWORD dwColor , int iThickness )
{
  CFigureFrame * pFigure = CFigureFrame::Create() ;
  if ( pFigure )
  {
    pFigure->SetSize( iLen ) ;
    cmplx cOrthoStep = GetOrthoLeftOnVF( cGraphStep ) ;
    for ( int iX = 0 ; iX < iLen ; iX++ )
    {
      cmplx cPt( cOrigin + cGraphStep * (double)iX ) ;
      cPt += cOrthoStep * ((*(pData++) - dYShift) * dYScale) ;
      pFigure->SetAt( iX , CmplxToCDPoint( cPt ) ) ;
    }
    FXString Color ;
    Color.Format( "0x%X" , dwColor ) ;
    pFigure->Attributes()->WriteString( "color" , Color ) ;
    if ( iThickness != 1 )
      pFigure->Attributes()->WriteInt( "thickness" , iThickness ) ;
  }
  return pFigure ;
}
CFigureFrame * CreateFigureFrameEx( const DoubleVector& Data ,
  const cmplx& cOrigin , const cmplx& cGraphStep , double dYShift ,
  double dYScale , DWORD dwColor , int iThickness )
{
  return CreateFigureFrameEx( Data.data() , (int)Data.size() , 
    cOrigin , cGraphStep , dYShift ,
    dYScale , dwColor , iThickness ) ;
}

CFigureFrame * CreateFigureFrameEx( const double * pData ,
  int iLen , const cmplx& cOrigin , const cmplx& cGraphStep , 
  double dYShift , double dYScale , LPCTSTR pAttributes )
{
  CFigureFrame * pFigure = CFigureFrame::Create() ;
  if ( pFigure )
  {
    pFigure->SetSize( iLen ) ;
    cmplx cOrthoStep = GetOrthoLeftOnVF( cGraphStep ) ;
    for ( int iX = 0 ; iX < iLen ; iX++ )
    {
      cmplx cPt( cOrigin + cGraphStep * ( double ) iX ) ;
      cPt += cOrthoStep * ( ( *(pData++) - dYShift ) * dYScale ) ;
      pFigure->SetAt( iX , CmplxToCDPoint( cPt ) ) ;
    }
    *( pFigure->Attributes() ) = pAttributes ;
  }
  return pFigure ;
}

CFigureFrame * CreateFigureFrameEx( const DoubleVector& Data ,
  const cmplx& cOrigin , const cmplx& cGraphStep , double dYShift ,
  double dYScale , LPCTSTR pAttributes )
{
  return CreateFigureFrameEx( Data.data() , ( int ) Data.size() ,
    cOrigin , cGraphStep , dYShift ,
    dYScale , pAttributes ) ;
}

CFigureFrame * CreateCircleViewEx( const cmplx& cCenter ,
  double dRadius , DWORD dwCircColor , int iNPointsOnCircle )
{
  CFigureFrame * pCircle = CFigureFrame::Create() ;
  pCircle->SetSize( iNPointsOnCircle + 1 ) ;
  double dAngleBtwPoints = 2. * M_PI / iNPointsOnCircle ;
  for ( int i = 0 ; i < iNPointsOnCircle ; i++ )
  {
    cmplx cPt = cCenter + dRadius * polar( 1. , i * M_PI / 180. ) ;
    pCircle->SetAt( i , CmplxToCDPoint( cPt ) ) ;
  }
  pCircle->SetAt( 360 , pCircle->GetAt( 0 ) ) ;

  FXString AsHex ;
  AsHex.Format( "color=0x%08X;" , dwCircColor ) ;
  *( pCircle->Attributes() ) += AsHex ;
  return pCircle ;
}


