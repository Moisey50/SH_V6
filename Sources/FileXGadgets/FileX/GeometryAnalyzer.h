#pragma once

#include<math/Intf_sup.h>
#include <helpers//propertykitEx.h>
#include <gadgets\figureframe.h>
#include <Math\FigureProcessing.h>



class FormDescriptor
{
public:
  FXString   m_Name ;
  bool       m_bIsActive ;

  double     m_dHeight_um ;
  double     m_dPerimeter_um ;
  double     m_dArea ;
  FXDblArray m_Distances_um ;
  double     m_dTolerance ;
  double     m_dAverDist ;

  FXString   m_sPartID;
  FXString   m_sPONumber ;

  FXString   m_sCreatedAt;
  double     m_dLastOptimalZ;

  FormDescriptor();
  FormDescriptor( LPCTSTR pName , double dHeight_um ,
    double dPerimeter_um , const FXDblArray& Distances, bool bIsActive , 
    const FXString& sPartID , const FXString& sPONumber , const FXString& sCreatedAt , double dArea = 0. ) ;
  FormDescriptor( LPCTSTR pString ) ;

  int SetData( LPCTSTR pName , double dHeight_um ,
    double dPerimeter_um , const FXDblArray& Distances , bool bIsActive , 
    const FXString& sPartID , const FXString& sPONumber , const FXString& sCreatedAt , double dArea = 0. ) ;
  double CheckDistances() ;

  FormDescriptor& operator=( const FormDescriptor& Orig ) ;
  bool operator==( const FormDescriptor& Other ) ;
  FXString ToString() const ;
  FXString GetTitle() const ;
  int FromString( LPCTSTR AsString ) ;
  FXSIZE GetNDistances() { return m_Distances_um.Count(); }
private:

};

class FXFormsArray : public FXArray<FormDescriptor>{};

// Class for sectors description
// Sector begin always on the right side from sector end
// i.e. sector is going CCW from begin to end
class Sector: public Segment1d
{
public:
  bool   m_bCrossZero ;
  Sector( double dBegin = 0. , double dEnd = 0. )
  {
    dBegin = NormTo2PI( dBegin ) ;
    dEnd = NormTo2PI( dEnd ) ;
    m_dBegin = dBegin , m_dEnd = dEnd ;
    m_bCrossZero = (m_dBegin > m_dEnd) ;
  }
  Sector( Sector& Other )
  {
    *this = Other ;
  }
  bool IsCrossZero() { return m_bCrossZero ; }

  bool IsInSector( double dAngle )
  {
    dAngle = NormTo2PI( dAngle ) ;
    if ( m_dBegin < m_dEnd )
      return IsInRange( dAngle , m_dBegin , m_dEnd ) ;
    return ((dAngle >= m_dBegin) || (dAngle <= m_dEnd)) ;
  }
  bool IsOverlapped( Sector& Other )
  {
    if ( IsInSector( Other.m_dBegin ) )
      return true ;
    if ( IsInSector( Other.m_dEnd ) )
      return true ;
    if ( Other.IsInSector( m_dBegin ) )
      return true ;
    return false ;
  }
  bool IsInside( Sector& Other ) // other sector is inside this sector
  {
    if ( m_bCrossZero != Other.IsCrossZero() )
      return false ;
    return (IsInSector( Other.m_dBegin ) && IsInSector( Other.m_dEnd ) ) ;
  }
  double GetLength()
  {
    if ( !m_bCrossZero )
      return Segment1d::GetLength() ;
    return (m_dEnd + M_PI * 2. - m_dBegin);
  }
  bool bUnion( Sector& Other )
  {
    bool bRes = false ;
    if ( IsOverlapped(Other) )
    {
      if ( !m_bCrossZero ) // this sector is not crossing zero
      {
        if ( !Other.IsCrossZero() )
          bRes = Union( Other ) ;
        else
        {
          Segment1d Tmp( *((Segment1d*)this) ) ;
          Other.m_dEnd += M_PI * 2. ;
          bRes = Tmp.Segment1d::Union( (Segment1d)Other ) ;
          Other.m_dEnd -= M_PI * 2. ;
          if ( Tmp.GetLength() >= M_PI * 2. )
            return false ;
          m_dBegin = Tmp.m_dBegin ;
          m_dEnd = Tmp.m_dEnd - M_PI * 2. ;
          m_bCrossZero = true ;
        }
      }
      else // this sector is crossing zero
      {
        Segment1d Tmp( *((Segment1d*) this) ) ;
        Tmp.m_dEnd += M_PI * 2. ;
        if ( Other.IsCrossZero() )
        {
          Other.m_dEnd += M_PI * 2. ;
          bRes = Tmp.Union( Other ) ;
          Other.m_dEnd -= M_PI * 2. ;
        }
        else
          bRes = Tmp.Union( Other ) ;


        if ( bRes && Tmp.GetLength() < M_PI * 2. )
        {
          m_dBegin = Tmp.m_dBegin ;
          m_dEnd = Tmp.m_dEnd - M_PI * 2. ;
        }
      }
    }
    return bRes ;
  }
};

typedef FXArray<Sector> ArrayOfSectors ;

class FormDescriptorEx: public FormDescriptor
{
public:
  cmplx           m_CalculatedCenter ;
  cmplx           m_cVectorToMarker ; // Relatively to main direction
  cmplx           m_cMainDirection ;
  CmplxArray      m_Vertices ; // the first vertex (index 0) gives main direction
  FXDblArray      m_Angles ;
  ArrayOfSectors  m_Sectors ;

  FormDescriptorEx()
    : FormDescriptor()
    , m_CalculatedCenter()
    , m_cMainDirection()
    , m_cVectorToMarker()
    , m_Vertices()
    , m_Angles()
    , m_Sectors()
  {
  }
  FormDescriptorEx( LPCTSTR pName , double dHeight_um ,
    double dPerimeter_um , const CmplxArray& Vertices , const cmplx& Center ,
    double dScale_pix_per_um , bool bIsActive , const FXString& sPartID , const FXString& sPONumber , const FXString& sCreatedAt ) ;

  FormDescriptorEx( LPCTSTR pName , double dHeight_um ,
    const CFigure * pFigure , double dScale_pix_per_um , bool bIsActive , const FXString& sPartID , const FXString& sPONumber , const FXString& sCreatedAt ) ;

  FormDescriptorEx( LPCTSTR pString ) ;

  int SetData( LPCTSTR pName , double dHeight_um ,
    double dPerimeter_um , const CmplxArray& Vertices , const cmplx& Center
    , double dScale_pix_per_um , bool bIsActive , const FXString& sPartID , const FXString& sPONumber , const FXString& sCreatedAt ) ;
  int SetData( LPCTSTR pName , double dHeight_um ,
    const CFigure * pFigure , double dScale_pix_per_um , bool bIsActive , const FXString& sPartID , const FXString& sPONumber , const FXString& sCreatedAt ) ;

  FXString ToString() ;
  int FromString( LPCTSTR AsString ) ;
  bool GetFromFile( LPCTSTR pFileName ) ;
  bool SaveToFile( LPCTSTR pFileName ) ;

  // next function returns direction to 
  // "Left" vertex:
  // For form with 3 vertices it will be nearest to left direction (angle PI) vertex
  // There is two variants for forms with 4 vertices:
  //   If angles between vertices are the same, nearest to left direction will be chosen
  //   If angles are not the same and two vertices are on left side, will be chosen vertex 
  //     with smallest angle another vertex in CW direction
  // For all other forms the nearest to left direction vertex will be chosen
  // Arrays of vertices, angles and distances will be rearranged: chosen vertex will be
  // first in arrays (with index 0)
  cmplx GetMainDirection() { return m_cMainDirection ; }
  cmplx CalcMainDirection() ;
  cmplx SetVectorToMarker( cmplx Vect ) { m_cVectorToMarker = Vect ; }
  cmplx GetVectorToMarker( cmplx& OtherMainDirection ) ;
  int   SetSector( cmplx cPt , double dRange ) ;
  int   AddSector( cmplx cPt , double dRange ) { return SetSector( cPt , dRange ); } ;
  int   GetSector( cmplx cPt ) ;
  bool  DeleteSector( int iIndex ) ;
  bool  DeleteSector( cmplx cPt ) ;
  bool  IsInSector( cmplx cPt ) ;
};

class GeometryAnalyzer
{
private:
  /*vector<FormDescriptor>*/ FXFormsArray m_KnownForms ;
public:

  FXString     m_InitFileName;
  FXString     m_FormattedFileName;
  int          m_iVersion;

  GeometryAnalyzer() ;
  int Init( LPCTSTR pFileName = NULL ) ;
  bool GetFormFileNameAndVersion() ;
  int Save();
  int Find( FormDescriptor& Form );
  int Find( FormDescriptor& Form , FXIntArray& Found );
  int Add(FormDescriptor& Form);
  FXSIZE GetNKnownForms() const { return m_KnownForms.Count(); }
  bool ContainsFormName(LPCTSTR pFormName);
  int FindFormByID(LPCTSTR pIDAsText); // if found, returns index, if not -1

  const FormDescriptor& GetAt( FXSIZE nIndex ) const
  {
    //ASSERT( nIndex >= 0 && nIndex < m_nSize );
    if ( nIndex >= 0 && nIndex < GetNKnownForms() )
      return m_KnownForms[ nIndex ];
    throw 1;
  }
  FormDescriptor&   GetAt( FXSIZE nIndex )
  {
    //ASSERT( nIndex >= 0 && nIndex < m_nSize );
    if ( nIndex >= 0 && nIndex < GetNKnownForms() )
      return m_KnownForms[ nIndex ];
    throw 1;
  }
  const FormDescriptor& ElementAt( FXSIZE nIndex ) const
  {
    //ASSERT( nIndex >= 0 && nIndex < m_nSize );
    if ( nIndex >= 0 && nIndex < GetNKnownForms() )
      return m_KnownForms[ nIndex ];
    throw 1;
  }
  FormDescriptor&   ElementAt( FXSIZE nIndex )
  {
    //ASSERT( nIndex >= 0 && nIndex < m_nSize );
    if ( nIndex >= 0 && nIndex < GetNKnownForms() )
      return m_KnownForms[ nIndex ];
    throw 1;
  }
  const FormDescriptor& operator[]( size_t nIndex ) const { return GetAt( nIndex ); }
  FormDescriptor& operator[]( FXSIZE nIndex ) { return ElementAt( nIndex ); }
};

