#include "stdafx.h"
#include "GeometryAnalyzer.h"
#include "fxfc/FXRegistry.h"
#include "fxfc/fxext.h"



#define THIS_MODULENAME "Geom.Analysis"



FormDescriptor::FormDescriptor()
: m_Name()
, m_dHeight_um(0.)
, m_bIsActive(true)
, m_sPartID()
, m_sPONumber()
, m_dPerimeter_um(0.)
, m_dTolerance(0.05)
, m_Distances_um()
, m_dAverDist(0.)
, m_sCreatedAt()
, m_dLastOptimalZ(0)
, m_dArea(0.)
{
}


FormDescriptor::FormDescriptor( LPCTSTR pName , double dHeight_um ,
  double dPerimeter_um , const FXDblArray& Distances, 
  bool bIsActive , const FXString& sPartID , const FXString& sPONumber ,
  const FXString& sCreatedAt , double dArea)
  : FormDescriptor()
{
  SetData( pName , dHeight_um , dPerimeter_um , Distances, 
    bIsActive, sPartID, sPONumber, sCreatedAt , dArea ) ;
}

FormDescriptor::FormDescriptor( LPCTSTR pString )
  : FormDescriptor()
{
  FromString( pString ) ;
}

double FormDescriptor::CheckDistances()
{
  m_dAverDist = 0. ;
  if ( m_Distances_um.Count() > 0 )
  {
    for ( int i = 0 ; i < m_Distances_um.Count() ; i++ )
      m_dAverDist += m_Distances_um[ i ] ;

    if ( m_dAverDist != 0. )
      m_dAverDist /= m_Distances_um.Count() ;
  }
  return m_dAverDist ;
}
FormDescriptor& FormDescriptor::operator=( const FormDescriptor& Orig )
{
  m_Name = Orig.m_Name ;
  m_dHeight_um = Orig.m_dHeight_um ;
  ASSERT( m_dHeight_um < 40000. ) ;
  m_dPerimeter_um = Orig.m_dPerimeter_um ;
  m_dArea = Orig.m_dArea ;
  m_dTolerance = Orig.m_dTolerance ;
  m_dAverDist = Orig.m_dAverDist;
  m_Distances_um.RemoveAll() ;
  m_Distances_um.Copy( Orig.m_Distances_um ) ;
  m_bIsActive = Orig.m_bIsActive;
  m_sPartID = Orig.m_sPartID;
  m_sPONumber = Orig.m_sPONumber ;
  m_sCreatedAt = Orig.m_sCreatedAt;
  m_dLastOptimalZ = Orig.m_dLastOptimalZ;

  return *this ;
}

bool FormDescriptor::operator==( const FormDescriptor& Other )
{
  if ( m_Distances_um.Count() && Other.m_Distances_um.Count()
    && (m_Distances_um.Count() == Other.m_Distances_um.Count() )
    && IsInTolerance( m_dAverDist , Other.m_dAverDist , m_dTolerance) 
    && IsInTolerance( m_dPerimeter_um , Other.m_dPerimeter_um , m_dTolerance) )
  {

    int iNDist = (int) m_Distances_um.Count() ;
    for ( int i = 0 ; i < iNDist ; i++ )
    {
      if ( IsInTolerance( m_Distances_um[ i ] , Other.m_Distances_um[ 0 ] , m_dTolerance ) )
      {
        for ( int j = 1 ; j < m_Distances_um.Count() ; j++ )
        {
          if ( !IsInTolerance( m_Distances_um[ (i + j ) % iNDist ] ,
            Other.m_Distances_um[j] , m_dTolerance ) )
          {
            return false ;
          }
        }
        return true ;
      }
    }
  }
  return false ;
}

int FormDescriptor::SetData(
 LPCTSTR pName , double dHeight_um ,
  double dPerimeter_um , const FXDblArray& Distances , bool bIsActive , const FXString& sPartID , const FXString& sPONumber , const FXString& sCreatedAt , double dArea )
{
  m_Name = pName ;
  m_dHeight_um = dHeight_um ;
  m_dPerimeter_um = dPerimeter_um ;
  m_Distances_um.RemoveAll() ;
  m_Distances_um.Copy( Distances ) ;
  m_dTolerance = 0.05 ;
  m_sPartID = sPartID;
  m_sPONumber = sPONumber ;
  m_bIsActive = bIsActive ;
  m_sCreatedAt = sCreatedAt;
  m_dArea = dArea ;
  CheckDistances() ;
  return (int) m_Distances_um.Count() ;
}


#define btoa(x) ((x)?"true":"false")
#define atob(x) ((x)=="true")

FXString FormDescriptor::ToString() const
{
  FXString Result , Add ;
  Result.Format( "%34s,%10s,%10s,%10s,%20s,%16.1f,%16.1f,%18.3f" ,
    (LPCTSTR) m_Name ,
    btoa(m_bIsActive) ,
    m_sPartID ,
    m_sPONumber ,
    m_sCreatedAt ,
    m_dHeight_um ,
    m_dPerimeter_um ,
    m_dTolerance ) ;
  for ( int i = 0 ; i < m_Distances_um.Count() ; i++ )
  {
    Add.Format( ", %8.2f" , m_Distances_um[ i ] ) ;
    Result += Add ;
  }
  return Result ;
}

FXString FormDescriptor::GetTitle() const
{
  FXString title;
  title.Format( "%34s,%10s,%10s,%10s,%20s,%16s,%16s,%18s,%16s",
    _T( "Name" ) ,
    _T( "Is Active" ) ,
    _T( "Part ID" ) ,
    _T( "PO Number" ) ,
    _T( "Created At" ) ,
    _T( "Height (um)" ) , 
    _T( "Perimeter (um)" ) , 
    _T( "Tolerance (0/00)" ) ,
    _T( "Distances (um)" ) );
  return title ;
}


int FormDescriptor::FromString( LPCTSTR pAsText )
{
  FXString AsText( pAsText ) ;
  FXSIZE iPos = AsText.Find( _T( '\n' ) ) ;
  if ( iPos > 0 )
  {
    AsText = AsText.Right( AsText.GetLength() - iPos - 1) ;
    AsText = AsText.Trim() ;
  }

  m_Distances_um.RemoveAll() ;
  
  iPos = 0 ; 
  
  FXString Token = AsText.Tokenize( _T( "," ) , iPos ) ;
  m_Name = Token.Trim().TrimLeft('/') ;

  Token = AsText.Tokenize( _T( "," ) , iPos ) ;
  m_bIsActive = atob( Token.Trim() );

  Token = AsText.Tokenize( _T( "," ) , iPos ) ;
  m_sPartID = Token.Trim();
  
  Token = AsText.Tokenize( _T( "," ) , iPos ) ;
  m_sPONumber = Token.Trim();

  Token = AsText.Tokenize( _T( "," ) , iPos ) ;
  m_sCreatedAt = Token.Trim();

  Token = AsText.Tokenize( _T( "," ) , iPos ) ;
  m_dHeight_um = atof( Token.Trim() ) ;

  Token = AsText.Tokenize( _T( "," ) , iPos ) ;
  m_dPerimeter_um = atof( Token.Trim() ) ;

  Token = AsText.Tokenize( _T( "," ) , iPos ) ;
  m_dTolerance = atof( Token.Trim() ) ;

  while ( iPos >= 0 )
  {
    Token = AsText.Tokenize( _T( "," ) , iPos ) ;
    if (Token.IsEmpty())
      break;
    double dVal = atof( Token ) ;
    m_Distances_um.Add( dVal ) ;
  }
  CheckDistances() ;
  return (int) m_Distances_um.Count() ;
}







FormDescriptorEx::FormDescriptorEx( LPCTSTR pName , double dHeight_um ,
  double dPerimeter_um , const CmplxArray& Vertices , const cmplx& Center ,
  double dScale_pix_per_um , bool bIsActive , const FXString& sPartID , 
  const FXString& sPONumber , const FXString& sCreatedAt )
  : FormDescriptorEx()
{
  SetData( pName , dHeight_um , dPerimeter_um ,
    Vertices , Center , dScale_pix_per_um, bIsActive, sPartID, sPONumber, sCreatedAt ) ;
}
FormDescriptorEx::FormDescriptorEx( LPCTSTR pName , double dHeight_um ,
  const CFigure * pFigure , double dScale_pix_per_um , bool bIsActive , const FXString& sPartID , const FXString& sPONumber , const FXString& sCreatedAt )
  : FormDescriptorEx()
{
  SetData( pName , dHeight_um , pFigure , dScale_pix_per_um , bIsActive, sPartID, sPONumber, sCreatedAt) ;
}


int FormDescriptorEx::SetData( LPCTSTR pName , double dHeight_um ,
  double dPerimeter_um , const CmplxArray& Vertices , const cmplx& Center ,
  double dScale_pix_per_um , bool bIsActive , const FXString& sPartID , 
  const FXString& sPONumber , const FXString& sCreatedAt )
{
  FormDescriptor::SetData( pName , dHeight_um , dPerimeter_um , FXDblArray() , bIsActive , sPartID, sPONumber , sPONumber );

  m_Angles.RemoveAll() ;
  m_Vertices.RemoveAll() ;
  for ( int i = 0 ; i < Vertices.Count() ; i++ )
  {
    m_Distances_um.Add( abs( Vertices[ i ] - Center ) / dScale_pix_per_um ) ;
    m_Vertices.Add( (Vertices[ i ] - Center)/dScale_pix_per_um ) ;
    if ( i > 0 )
      m_Angles.Add( arg( Vertices[ i ] / Vertices[ i - 1 ] ) ) ;
  }
  if ( m_Vertices.Count() > 1 )
    m_Angles.Add( arg( Vertices[ 0 ] / Vertices.Last() ) ) ;

  CheckDistances() ;
  return (int) m_Distances_um.Count() ;
}


int FormDescriptorEx::SetData( LPCTSTR pName , double dHeight_um ,
  const CFigure * pFigure , double dScale_pix_per_um , bool bIsActive , const FXString& sPartID , const FXString& sPONumber , const FXString& sCreatedAt )
{
  cmplx CalculatedCenter;
  CmplxArray Vertices;
  double dPerimeter;

  m_Vertices.RemoveAll() ;
  m_Angles.RemoveAll() ;
  m_Sectors.RemoveAll() ;
  int NVertices = GetNFilteredVertices( *pFigure ,
    CalculatedCenter , &Vertices , 0.15 , &dPerimeter );

  FormDescriptor::SetData( pName , dHeight_um , dPerimeter / dScale_pix_per_um , FXDblArray() , bIsActive , sPartID, sPONumber , sCreatedAt );

  m_CalculatedCenter = CalculatedCenter ;
  if ( abs( CalculatedCenter ) != 0. )
  {
    if ( NVertices ) // not round form
    {
      for ( int i = 0 ; i < Vertices.GetCount() ; i++ )
      {
        cmplx VectToVertex_um = (Vertices[ i ] - CalculatedCenter) / dScale_pix_per_um ;
        m_Vertices.Add( VectToVertex_um ) ;
        if ( i > 0 )
          m_Angles.Add( arg( m_Vertices[ i ] / m_Vertices[ i - 1 ] ) ) ;

        m_Distances_um.Add( abs( VectToVertex_um ) );
      }
      if ( m_Vertices.Count() > 1 )
        m_Angles.Add( arg( m_Vertices[ 0 ] / m_Vertices.Last() ) ) ;
      CheckDistances();
    }
    else //absolutely round form
    {
      if ( Vertices.Count() == 1 )
      {
        m_Vertices.Add( Vertices[ 0 ] / dScale_pix_per_um ) ;
        m_Distances_um.Add( m_dAverDist = fabs( m_Vertices[ 0 ].real() ) ) ;
        m_Angles.Add( arg( m_Vertices[ 0 ] ) ) ;
      }
      else
        ASSERT( 0 ) ;
    }
    m_cMainDirection = CalcMainDirection() ;
  }
  return (int) m_Distances_um.Count() ;
}

FXString FormDescriptorEx::ToString()
{
  FXString Result( FormDescriptor::ToString() + _T(";\n") );
  FXString VertAsString , Addition ;
//  Addition.Format( _T( "DataBaseId=%s;\n" ) , (LPCTSTR)m_DataBaseId ) ;
  Result += Addition ;
  if ( m_Vertices.Count() )
  {
    Result += _T( "Vertices_um=" ) ;
    for ( int i = 0 ; i < m_Vertices.Count() ; i++ )
    {
      Addition.Format( _T( "(%.2f,%.2f)" ) ,
        m_Vertices[ i ].real() , m_Vertices[ i ].imag() ) ;
      VertAsString += Addition ;
      if ( i < m_Vertices.Count() - 1 )
        VertAsString += _T( ',' ) ;
    }
    Result += VertAsString + _T( ";\n" ) ;
  }
  if ( m_Angles.Count() )
  {
    Result += _T( "Angles_deg=" ) ;
    for ( int i = 0 ; i < m_Angles.Count() ; i++ )
    {
      Addition.Format( _T( "%.4f" ), RadToDeg( m_Angles[i]) ) ;
      Result += Addition ;
      if ( i < m_Angles.Count() - 1 )
        Result += _T( ',' ) ;
    }
    Result += _T( ";\n" ) ;
  }
  if ( m_Sectors.Count() )
  {
    Result += _T( "Sectors_deg=" ) ;
    for ( int i = 0 ; i < m_Sectors.Count() ; i++ )
    {
      Addition.Format( _T( "[%.4f,%.4f]" ) , 
        RadToDeg(m_Sectors[ i ].m_dBegin) , RadToDeg( m_Sectors[ i ].m_dEnd ) ) ;
      Result += Addition ;
      if ( i < m_Sectors.Count() - 1 )
        Result += _T( ',' ) ;
    }
    Result += _T( ";\n" ) ;
  }
  if ( abs(m_cVectorToMarker) > 0. )
  {
    Addition.Format( "MarkerVect=(%.1f,%.1f);\n" , 
      m_cVectorToMarker.real() , m_cVectorToMarker.imag() ) ;
    Result += Addition ;
  }
  if ( abs( m_cMainDirection ) > 0. )
  {
    Addition.Format( "MainVect=(%.1f,%.1f);\n" ,
      m_cMainDirection.real() , m_cMainDirection.imag() ) ;
    Result += Addition ;
  }
  return Result ;
}

FormDescriptorEx::FormDescriptorEx( LPCTSTR pAsString )
{
  FromString( pAsString ) ;
}

int FormDescriptorEx::FromString( LPCTSTR pAsText )
{
  FXPropKit2 AsText( pAsText ) ;

//   if ( !AsText.GetString( "DataBaseId" , m_DataBaseId ) )
//     m_DataBaseId.Empty() ;
  m_Vertices.RemoveAll() ;
  m_Angles.RemoveAll() ;
  m_Sectors.RemoveAll() ;
  int iPos = (int) AsText.Find( _T( ';' ) ) ;
  FXString Before( (iPos > 0) ? AsText.Left( iPos ) : AsText ) ;
  FormDescriptor::FromString( Before ) ;
  if ( iPos > 0 )
  {
    FXPropKit2 After( AsText.Mid( iPos ) ) ;
    FXPropKit2 AsString ;
    if ( After.GetString( "Vertices_um" , AsString ) )
    {
      int iBracketPos = 0 ;
      cmplx cVal ;
      int i = 0 ;
      while ( (iBracketPos = AsString.FindOpenBracket( iBracketPos )) >= 0 )
      {
        if ( StrToCmplx( (LPCTSTR) AsString + (++iBracketPos) , cVal ) )
        {
          m_Vertices.Add( cVal ) ;
          if ( i > 0 )
            m_Angles.Add( arg( m_Vertices[ i ] / m_Vertices[ i - 1 ] ) ) ;
          i++ ;
        }
        else
          ASSERT( 0 ) ; // bad format
      }
      if ( i > 1 )
        m_Angles.Add( arg( m_Vertices[ 0 ] / m_Vertices.Last() ) ) ;
    }
    if ( After.GetString( "Sectors_deg" , AsString ) )
    {
      int iBracketPos = 0 ;
      cmplx cVal ;
      while ( (iBracketPos = AsString.FindOpenBracket( iBracketPos )) >= 0 )
      {
        if ( StrToCmplx( (LPCTSTR) AsString + (++iBracketPos) , cVal ) )
          m_Sectors.Add( Sector( DegToRad( cVal.real() ) , DegToRad( cVal.imag() ) ) ) ;
        else
          ASSERT( 0 ) ; // bad format
      }
    }
    After.GetCmplx( "MarkerVect" , m_cVectorToMarker ) ;
    After.GetCmplx( "MainVect" , m_cMainDirection ) ;
  }
  return (int) m_Distances_um.Count() ;
}

bool FormDescriptorEx::GetFromFile( LPCTSTR pFileName )
{
  FXPropKit2 FromFile ;
  if ( !FromFile.GetFromFile( pFileName ) )
  {
    SENDERR( "FormDescriptorEx::GetFromFile -"
      "Can't get form description from file %s ", pFileName ) ;
    return false ;
  }

  return ( FromString( FromFile ) != 0 );
}

bool FormDescriptorEx::SaveToFile( LPCTSTR pFileName )
{
  FXPropKit2 FromFile = ToString() ;
  if ( !FromFile.WriteToFile( pFileName ) )
  {
    SENDERR( "FormDescriptorEx::SaveToFile -"
      "Can't write form description to file %s " , pFileName ) ;
    return false ;
  }

  return true ;
}

cmplx FormDescriptorEx::CalcMainDirection()
{
  int iVertCount = (int) m_Vertices.Count() ;
  if ( iVertCount == 0 )
    cmplx() ;
  if ( iVertCount == 1 )
    return m_Vertices[0] ;
  double dMaxDiffAngle = 0. , dMinDiffAngle = M_2PI ;
  double dMaxAngle = 0. , dMinAngle = M_2PI ;
  int iLeftIndex = -1 , iRightIndex = -1 ;
  int iMinDiffAngleIndex = -1 , iMaxDiffAngleIndex = -1 ;
  int iLongestLeftIndex = -1 ;
  double dLongestLeft = 0 ;
  double dMinDist = DBL_MAX ;
  double dMaxDist = -DBL_MAX ;
  for ( int i = 0 ; i < m_Vertices.Count() ; i++ )
  {
    double dAngle = abs( m_Angles[i] ) ;
    if ( dAngle > dMaxDiffAngle )
    {
      dMaxDiffAngle = dAngle ;
      iMaxDiffAngleIndex = i ;
    }
    if ( dAngle < dMinDiffAngle )
    {
      dMinDiffAngle = dAngle ;
      iMinDiffAngleIndex = i ;
    }
    double dDir = arg( m_Vertices[ i ] ) ;
    if ( dDir < -M_PI_2 || dDir > M_PI_2 )
    {
      if ( dLongestLeft < m_Distances_um[i] )
      {
        dLongestLeft = m_Distances_um[ i ] ;
        iLongestLeftIndex = i ;
      }
    }
    if ( dMinAngle > abs(dDir) )
    {
      dMinAngle = abs( dDir ) ;
      iRightIndex = i ;
    }
    if ( dMaxAngle < abs( dDir) )
    {
      dMaxAngle = abs( dDir ) ;
      iLeftIndex = i ;
    }
    if ( dMinDist > m_Distances_um[ i ] )
      dMinDist = m_Distances_um[ i ] ;
    if ( dMaxDist < m_Distances_um[ i ] )
      dMaxDist = m_Distances_um[ i ] ;
  }
  if ( iLeftIndex >= 0 )
  {
    double dAngleTol = DegToRad( 2. ) ; // 2. degrees
    if ( (dMaxDiffAngle - dMinDiffAngle) < dAngleTol )
    { // all angles are equal, it's full symmetry: 
      
      if ( ((dMaxDist - dMinDist) / dMaxDist) < m_dTolerance / 2 )
        // If distances are the same, return left vertex
        // i.e. nearest to PI (180 degrees)
        return m_Vertices[ iLeftIndex ] ;
      else
        // return most far left vertex
        return m_Vertices[ iLongestLeftIndex ] ;
    }

    double dLimitForAnalysis = M_PI_2 * 0.9 ;
    double dAngleCW = m_Angles[ iLeftIndex ] ;
    double dAngleCWNext = m_Angles[ (iLeftIndex + 1) % iVertCount ] ;
    double dAngleCCW = m_Angles[ (iLeftIndex - 1 + iVertCount) % iVertCount ] ;
    switch ( iVertCount )
    {
    case 3: // Most sharp corner selected
      return m_Vertices[ (iMinDiffAngleIndex + 2) % iVertCount ] ;
    case 4:
      {
        if ( fabs( dAngleCW ) > fabs( dAngleCCW ) )
          return m_Vertices[ ( iLeftIndex + 3 ) % iVertCount ] ;
        else
          return m_Vertices[ iLeftIndex ] ;
      }
    }
    return m_Vertices[ iLeftIndex ] ;
  }
  return cmplx() ;
}

cmplx FormDescriptorEx::GetVectorToMarker( cmplx& OtherMainDirection )
{
  cmplx cNormMainDir = GetNormalized( m_cMainDirection ) ;
  cmplx Result = m_cVectorToMarker * cNormMainDir ;
  return Result ;
}

int FormDescriptorEx::SetSector( cmplx cPt , double dRange )
{
  cmplx cNorm = GetNormalized( cPt ) ;
  cmplx cOrthoRight = GetOrthoRight( cNorm ) ;
  //double dRangeRad = dRange / abs( cPt ) ;
  cmplx cVectRight = cOrthoRight * dRange ;
  cmplx cPtRight = cPt + cVectRight ;
  cmplx cPtLeft = cPt - cVectRight ;
  cmplx cDivRight = cPtRight / m_cMainDirection ;
  double dAngleRight = arg( cDivRight ) ;
  cmplx cDivLeft = cPtLeft / m_cMainDirection ;
  double dAngleLeft = arg( cDivLeft ) ;

  dAngleLeft = NormTo2PI( dAngleLeft ) ;
  dAngleRight = NormTo2PI( dAngleRight ) ;


  Sector NewSector( dAngleLeft , dAngleRight ) ;
  for ( int i  = 0 ; i < m_Sectors.Count() ; i++ )
  {
    if ( m_Sectors[i].IsOverlapped( NewSector ) )
    {
      if ( m_Sectors[ i ].Union( NewSector ) )
        return i ;
      else
  return -1 ;
}
  }
  m_Sectors.Add( NewSector ) ;
  return (int) m_Sectors.Count() ;
}

int FormDescriptorEx::GetSector( cmplx cPt )
{
  cmplx cDiv = cPt / m_cMainDirection ;
  double dRelAngle = arg( cDiv ) ; // angle from cPt To Main Direction.

  for ( int i = 0 ; i < m_Sectors.Count() ; i++ )
  {
    if ( m_Sectors[ i ].IsInSector( dRelAngle ) )
      return i ;
  }
  
  return -1 ;
}

bool FormDescriptorEx::DeleteSector( cmplx cPt )
{
  cmplx cDiv = cPt / m_cMainDirection ;
  double dRelAngle = arg( cDiv ) ; // angle from cPt To Main Direction.

  for ( int i = 0 ; i < m_Sectors.Count() ; i++ )
  {
    if ( m_Sectors[ i ].IsInSector( dRelAngle ) )
    {
      m_Sectors.RemoveAt( i ) ;
      return true ;
    }
  }
  return false ;
}

bool FormDescriptorEx::DeleteSector( int iIndex )
{
  if ( 0 <= iIndex && iIndex < m_Sectors.Count() )
  {
    m_Sectors.RemoveAt( iIndex ) ;
    return true ;
  }
  return false ;
}

GeometryAnalyzer::GeometryAnalyzer()
{
  m_iVersion = -1;
}

bool FormDescriptorEx::IsInSector( cmplx cPt )
{
  cmplx cDiv = cPt / m_cMainDirection ;
  double dRelAngle = arg( cDiv ) ; // angle from cPt To Main Direction.
  for ( int i = 0 ; i < m_Sectors.Count() ; i++ )
  {
    if ( m_Sectors[ i ].IsInSector( dRelAngle ) )
      return true ;
  }
  return false ;
}

int GeometryAnalyzer::Init( LPCTSTR pFileName )
{
  if (pFileName && *pFileName)
    m_InitFileName = pFileName;
  else
  {
    if (GetFormFileNameAndVersion())
      m_InitFileName = m_FormattedFileName;
  }

  FXPropKit2 pk;
  pk.GetFromFile(m_InitFileName);

  FXSIZE iPos = 0;
  m_iVersion = -1;

  FXPropKit2 VersionString = pk.Tokenize("\n", iPos);
  if (iPos >= 0 && VersionString.GetInt("Version", m_iVersion))
  {
    m_KnownForms.RemoveAll() ;
    FXString DescriptionString = pk.Tokenize("\n", iPos);
    while (iPos > 0)
    {
      FXString Token = pk.Tokenize("\n", iPos).Trim();
      if (Token.IsEmpty())
        break;
      //if ( Token.Find( "//" ) == 0 )
      //  continue ;
      FormDescriptor NewForm(Token);
      if ( Token.Find( "//" ) == 0 )
        NewForm.m_bIsActive = false;
      if (NewForm.GetNDistances())
        m_KnownForms.Add(NewForm);
    };
  }
  FxSendLogMsg( MSG_INFO_LEVEL , 
    "GeometryAnalyzer::Init" "%d forms initialized from file %s" , 
    (int) m_KnownForms.Count() ,
    (LPCTSTR)m_InitFileName );

//     
// 
//   // remove extension (.dat)
//   int iPtPos = m_InitFileName.ReverseFind(_T('.'));
//   if (iPtPos)
//     m_InitFileName.Delete(iPtPos, m_InitFileName.GetLength() - iPtPos);
// 
//   // Remove version number from file name
//   int iLen = m_InitFileName.GetLength();
//   LPTSTR pBuf = m_InitFileName.GetBuffer();
//   LPTSTR pEnd = pBuf + iLen - 1;
//   while ( iLen && isdigit( *pEnd ) )
//   {
//     pEnd--;
//     iLen--;
//   }
//   m_InitFileName.ReleaseBuffer(iLen) ; // Now file name is without version number
  return (int) GetNKnownForms() ;
}

//TimeStamp =
#include <algorithm>
using namespace std;

bool GeometryAnalyzer::GetFormFileNameAndVersion()
{
  FXRegistry Reg( "TheFileX\\ConturASM" );
  FXString Directory = Reg.GetRegiString(
    "Data" , "MainDirectory" , "c:\\BurrInspector\\" );
  FXString DataSubDir = Reg.GetRegiString( "Data" , "DataSubDir" , "Data\\" );
  FXString DataDir( Directory + DataSubDir );
  if ( !FxVerifyCreateDirectory( DataDir ) )
  {
    SENDERR( "Can't create data directory '%s' " , (LPCTSTR) DataDir ) ;
    return false ;
  };
  FXString FileName = Reg.GetRegiString( "FormAnalysis" ,
    "FormsDescrFileName" , "FormDescription" );
  m_iVersion = Reg.GetRegiInt( "FormAnalysis" ,
    "FormsDescrVersion" , 0 );

  m_FormattedFileName.Format("%s%s.dat", (LPCTSTR)DataDir,
    (LPCTSTR)FileName /*, m_iVersion */);

  return true ;
}

int GeometryAnalyzer::Save()
{
  if ( !GetFormFileNameAndVersion() )
    return 0 ;

  if (m_iVersion < 0)
    m_iVersion = 0;

  FXPropKit2 Content; // First two strings with version and description
  Content.Format("Version=%d\n%s\n", m_iVersion,
    GetNKnownForms() ? (LPCTSTR)m_KnownForms[0].GetTitle() : "");

  //sort( m_KnownForms.begin() , m_KnownForms.end() );

  for ( int i = 0 ; i < GetNKnownForms() ; i++ )
  {
    Content += m_KnownForms[i].ToString() + _T("\n");
  }
  string Path((LPCTSTR)m_FormattedFileName);
  if (PathFileExists(Path.c_str()))
  {
    string PathForOld(Path);
    struct stat result;
    if (stat(Path.c_str(), &result) == 0)
    {
      time_t mod_time = result.st_mtime;
      tm *ltm = localtime(&mod_time);
      std::stringstream date(ios_base::out);
      date << 1900 + ltm->tm_year << 1 + ltm->tm_mon << ltm->tm_mday
        << "_" << ltm->tm_hour << ltm->tm_min << ltm->tm_sec;
      PathForOld += date.str() + ".dat";
    }
    else
      PathForOld += (LPCTSTR)(GetTimeAsString_ms() + ".dat");
    rename(Path.c_str(), PathForOld.c_str());
  }

  int iNWritten = Content.WriteToFile( m_FormattedFileName , true ) ; // true - rewrite file
  ASSERT( iNWritten );
  FxSendLogMsg( MSG_INFO_LEVEL ,
    "GeometryAnalyzer::Save" "%d forms saved to file %s" ,
    (int) m_KnownForms.Count() ,
    (LPCTSTR) m_FormattedFileName );

  return (int) GetNKnownForms();
}

int GeometryAnalyzer::Find(FormDescriptor& Form)
{
  for ( int i = 0 ; i < GetNKnownForms() ; i++ )
  {
    if ( m_KnownForms[ i ].m_bIsActive && m_KnownForms[i] == Form)
      return i;
  }
  return -1;
}

int GeometryAnalyzer::Find( FormDescriptor& Form , FXIntArray& Found )
{
  int iBefore = (int) Found.Count() ;

  for ( int i = 0 ; i < GetNKnownForms() ; i++ )
  {
    if ( m_KnownForms[ i ].m_bIsActive && m_KnownForms[ i ] == Form )
      Found.Add( i ) ;
  }
  return (int) Found.Count() - iBefore ;
}

int GeometryAnalyzer::Add(FormDescriptor& Form)
{
  if (ContainsFormName(Form.m_Name))
    return 0;

  m_KnownForms.Add(Form);

  return (int) GetNKnownForms();
}

bool GeometryAnalyzer::ContainsFormName(LPCTSTR pFormName)
{
  for (int i = 0; i < (int) GetNKnownForms(); i++)
  {
    if ( m_KnownForms[ i ].m_bIsActive && m_KnownForms[i].m_Name == pFormName)
      return true;
  }
  return false;
};


int GeometryAnalyzer::FindFormByID(LPCTSTR pIDAsText)
{
  for (int i = 0; i < (int)GetNKnownForms(); i++)
  {
    if (m_KnownForms[i].m_bIsActive 
      && (m_KnownForms[i].m_Name.Find( pIDAsText ) == 0) )
    {
      return i;
    }
  }
  return -1 ;
};

