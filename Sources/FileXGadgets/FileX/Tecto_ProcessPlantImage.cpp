#include "stdafx.h"

#include "Tecto_ProcessPlantImage.h"

#include <fstream>
#include <sstream>
#include <string>
#include <deque>
#include <algorithm>
#include <numeric>

using namespace nsTecto;


void ProcessPlantImage::process( const char* szMFiguresName )
{
  std::ifstream strm( szMFiguresName );

  process( strm );
}

void ProcessPlantImage::process( std::istream& strm )
{
  std::vector< std::vector<R2> > figures = readFigures( strm );

  process( figures );
}

void ProcessPlantImage::MashPolygons()
{
  for ( std::vector< Polyline >::iterator it = figures.begin(); it != figures.end(); ++it )
    it->Mash( parameters.MashingSize );
}


void ProcessPlantImage::copyToPolylines( const std::vector< std::vector<R2> >& v , std::vector< nsTecto::Polyline >& f )
{
  f.clear();
  for ( std::vector< std::vector<R2> >::const_iterator it = v.begin(); it != v.end(); ++it )
    f.push_back( nsTecto::Polyline( *it ) );
}

std::vector< std::vector<R2> > ProcessPlantImage::readFigures( std::istream& strm )
{
  std::vector< std::vector<R2> > figures;

  // figure start marker.
  //Figure Frame 'Contur[flower_355]' Id = 16 Length=29 points
  std::string s;
  while ( std::getline( strm , s ) )
  {
    size_t pos = s.rfind( "Length=" );

    if ( pos == std::string::npos )
    {
    }
    else
    {
      std::istringstream ostrm( s.substr( pos + strlen( "Length=" ) ) );
      int cnt; ostrm >> cnt;
      std::vector<R2> vertices;
      for ( int i = 0; i < cnt; ++i )
      {
        std::getline( strm , s );
        std::istringstream ostrm( s );
        R2 r;
        int idx;
        //0,1017.600,892.000;
        ostrm >> idx;
        ostrm.ignore( 1 ) >> r.x;
        ostrm.ignore( 1 ) >> r.y;

        vertices.push_back( r );
      }
      figures.push_back( vertices );
    }
  }

  return figures;
}

ProcessPlantImage::eErrorCode ProcessPlantImage::process( const std::vector< std::vector<R2> >& input )
{
  copyToPolylines( input , figures );

  //some cleaning.
  RemoveDuplicateVertices();
  SetStartingPoints();
  RemoveDuplicatedConturs();

  ScaleFigures();
  ConvertPolygonsToPolylines();
  FindBasePolygon();
  FindOuterRectangles();
  MashPolygons();

  GetConnectedComponents();

  GetConnectedComponentsBoundary();

  GetStalkSlices();
  EstimateStalkRectangle();

  PrepareResults();

  return OK;
}

void ProcessPlantImage::SetStartingPoints()
{
  for ( std::vector< Polyline >::iterator it = figures.begin(); it != figures.end(); ++it )
    it->SetStartingPoint();
}

void ProcessPlantImage::RemoveDuplicatedConturs()
{
  std::vector<int> tmp( figures.size() );
  std::swap( InitialIndices , tmp );

  // keep old indices of conturs.
  for ( int i = 0; i < ( int ) figures.size(); ++i )
    InitialIndices[ i ] = i;

  for ( int i = 0; i < ( int ) figures.size(); ++i )
  {
    for ( int j = i + 1; j < ( int ) figures.size(); )
    {
      if ( figures[ i ].IsEqualTo( figures[ j ] ) )
      {
        figures.erase( figures.begin() + j );
        InitialIndices.erase( InitialIndices.begin() + j );
      }
      else
        j++;
    }
  }
}

void ProcessPlantImage::RemoveDuplicateVertices()
{
  for ( std::vector< Polyline >::iterator it = figures.begin(); it != figures.end(); ++it )
    it->RemoveDuplicateVertices();
}

void ProcessPlantImage::ScaleFigures()
{
  for ( std::vector< Polyline >::iterator it = figures.begin(); it != figures.end(); ++it )
    it->scaleX( parameters.XAxeCompressCoeff );
}

std::vector<R2> ProcessPlantImage::RescaleFigure( const nsTecto::Polyline& pl ) const
{
  Polyline ret( pl );

  ret.rescaleX( parameters.XAxeCompressCoeff );

  return ret.base();
};

void ProcessPlantImage::ConvertPolygonsToPolylines()
{
  for ( std::vector< Polyline >::iterator it = figures.begin(); it != figures.end(); ++it )
    it->ConvertPolygonToPolyline();
}

void ProcessPlantImage::FindOuterRectangles()
{
  for ( std::vector< Polyline >::iterator it = figures.begin(); it != figures.end(); ++it )
    it->FindOuterRectangle();
}

void ProcessPlantImage::FindBasePolygon()
{
    // find contour with maximal area.
  if ( figures.size() < 1 )
  {
    IdxMaxArea = -1;
    return;
  }

  double areaMax = figures[ 0 ].Area();
  int idx = 0;
  for ( int i = 1; i < ( int ) figures.size(); i++ )
  {
    double area = figures[ i ].Area();
    if ( area > areaMax )
    {
      areaMax = area;
      idx = i;
    }
  }

  IdxMaxArea = idx;
}

void ProcessPlantImage::GetConnectedComponents()
{
    // find connected components, starting from one with maximal area.
    // this is a searching the connected component of a graph.
  includedConturs.clear();
  if ( IdxMaxArea != -1 )
  {
    double RelationDistance = parameters.RelationDistance;

    // mark all conturs as not visited.
    std::vector<bool> visited( figures.size() , false );
    std::deque< int > ToVisit;
    ToVisit.push_back( IdxMaxArea ); // push base component to deque

    visited[ IdxMaxArea ] = true;
    int componentCnt = 0;

    componentCnt++;
    while ( ToVisit.empty() == false )
    {
      int idx = ToVisit.front();
      ToVisit.pop_front();

      //store the index of contur include into connected component
      includedConturs.push_back( idx );

      for ( int i = 0; i < ( int ) figures.size(); ++i )
      {
        if ( visited[ i ] == true )
          continue;

        Relation relation;

        // if a contur is related to current contur, keep infomation about their relations
        // ( connecting points, indices ) and put its index to deque.
        if ( figures[ idx ].IsRelated( figures[ i ] , RelationDistance , relation ) )
        {
          ToVisit.push_back( i );
          relation.idx0 = idx;
          relation.idx1 = i;
          relations.push_back( relation );
          visited[ i ] = true;
          componentCnt++;
        }
      }
    }

  }
}

void ProcessPlantImage::GetConnectedComponentsBoundary()
{
  connectedComponentsBoundary = figures[ IdxMaxArea ].BoundRectangle();
  for ( std::vector<int>::const_iterator it = includedConturs.begin(); it != includedConturs.end(); ++it )
  {
    connectedComponentsBoundary = Rectangle::BoundRectangle(
      connectedComponentsBoundary , figures[ *it ].BoundRectangle() );
  }
}

void ProcessPlantImage::PrepareResults()
{
    // index and point convertin functions.
  auto convertIndex = [ & ]( int thisIdx ) { return InitialIndices[ thisIdx ]; };
  auto convertPoint = [ & ]( R2 point ) { point.x /= parameters.XAxeCompressCoeff; return point; };

  // reference Contur index - just to know.
  results.ReferenceConturIndex = convertIndex( IdxMaxArea );

  // copy relation information - how two components are connectd: indices of components, 
  // nearest points and distance.
  results.relations.clear();
  for ( std::vector<Relation>::iterator it = relations.begin();
    it != relations.end(); ++it )
  {
    Relation relation;

    relation.idx0 = convertIndex( it->idx0 );
    relation.idx1 = convertIndex( it->idx1 );
    relation.r0 = convertPoint( it->r0 );
    relation.r1 = convertPoint( it->r1 );
    relation.distance = sqrt( R2::distance2( it->r0 , it->r1 ) );

    results.relations.push_back( relation );
  }

  // boundary of the plant.
  results.boundary.s = convertPoint( connectedComponentsBoundary.s );
  results.boundary.e = convertPoint( connectedComponentsBoundary.e );

  // indices of included Conturs.
  results.includedConturs.clear();
  for ( std::vector<int>::const_iterator it = includedConturs.begin(); it != includedConturs.end(); ++it )
    results.includedConturs.push_back( convertIndex( *it ) );

// make estimated rectangle of stalk
  if ( stalk.size() >= 2 )
  {
    R2 begin = convertPoint( stalk[ 0 ] );
    R2 end = convertPoint( stalk[ 1 ] );
    double dx = end.x - begin.x;
    double dy = end.y - begin.y;
    double ex = dx / sqrt( dx * dx + dy * dy );
    double ey = dy / sqrt( dx * dx + dy * dy );

    results.stalkWidth = stalkWidth * ex;
    // find the side of rectangel
    R2 rectVector { -ey * 0.5 * results.stalkWidth , ex * 0.5 * results.stalkWidth };

    R2 tmp_stalk[ 5 ];
    // form rectangle.
    tmp_stalk[ 0 ] = begin; tmp_stalk[ 0 ].x += rectVector.x;  tmp_stalk[ 0 ].y += rectVector.y;
    tmp_stalk[ 1 ] = end; tmp_stalk[ 1 ].x += rectVector.x;  tmp_stalk[ 1 ].y += rectVector.y;
    tmp_stalk[ 2 ] = end; tmp_stalk[ 2 ].x -= rectVector.x;  tmp_stalk[ 2 ].y -= rectVector.y;
    tmp_stalk[ 3 ] = begin; tmp_stalk[ 3 ].x -= rectVector.x;  tmp_stalk[ 3 ].y -= rectVector.y;
    tmp_stalk[ 4 ] = tmp_stalk[ 0 ];

    results.stalkContur.clear();
    for ( int i = 0; i < 5; ++i )
      results.stalkContur.push_back( tmp_stalk[ i ] );
  }
}


void ProcessPlantImage::GetStalkSlices()
{
  stalkSlices.clear();

  double endX = connectedComponentsBoundary.s.x +
    parameters.SlicedPartForStalk * ( connectedComponentsBoundary.e.x - connectedComponentsBoundary.s.x );
  double stepX = parameters.XAxeCompressCoeff;

  // make slice for each pixel
  for ( double x = connectedComponentsBoundary.s.x + magic_irrational_number; x < endX; x += stepX )
  {
    StalkSlice slice;

    //count the intersection of vertical axe contours.
    int cnt = 0;

    double yminSlice = DBL_MAX;
    double ymaxSlice = -DBL_MAX;

    for ( std::vector< int >::const_iterator it = includedConturs.begin(); it != includedConturs.end(); ++it )
    {
      double ymin , ymax;
      if ( int cnt0 = figures[ *it ].VerticalAxeIntersection( x , ymin , ymax ) )
      {
        yminSlice = std::min<double>( yminSlice , ymin );
        ymaxSlice = std::max<double>( ymaxSlice , ymax );
        cnt += cnt0;
      }
    }

    // stop when count of intersection >= 3 ( more than one contour has been intersected ) 
    // and if we have enough slices .
    if ( ( int ) stalkSlices.size() >= parameters.MinStalkSlicedCount && cnt >= 3 ) //
    {
      break;
    }
    else if ( cnt != 0 ) // 2.
    {
      slice.x = x;
      slice.count = cnt;
      slice.y = 0.5 * ( yminSlice + ymaxSlice );
      slice.ylength = ymaxSlice - yminSlice;
      stalkSlices.push_back( slice );
    }
  }
}

void ProcessPlantImage::EstimateStalkRectangle()
{
  stalk.clear();
  stalkWidth = 0;

  if ( stalkSlices.size() < 2 )
    return;

// estimate best fit line from middle of segments by simple LQ method.
  double sumX =
    std::accumulate( stalkSlices.begin() , stalkSlices.end() , 0.0 ,
      []( double d , const StalkSlice& o ) { return o.x + d; } );

  double sumY =
    std::accumulate( stalkSlices.begin() , stalkSlices.end() , 0.0 ,
      []( double d , const StalkSlice& o ) { return o.y + d; } );

  sumX /= stalkSlices.size();
  sumY /= stalkSlices.size();

  double sumDXX =
    std::accumulate( stalkSlices.begin() , stalkSlices.end() , 0.0 ,
      [ & ]( double d , const StalkSlice& o ) { return ( o.x - sumX ) * ( o.x - sumX ) + d; } );

  double sumDXY =
    std::accumulate( stalkSlices.begin() , stalkSlices.end() , 0.0 ,
      [ & ]( double d , const StalkSlice& o ) { return ( o.x - sumX ) * ( o.y - sumY ) + d; } );

  double k = sumDXY / sumDXX;
  double y = sumY - k * sumX;

  auto estY = [ & ]( double x ) -> double
  {
    return y + k * x;
  };

  int size = int( stalkSlices.size() );
  // start and end of line segment ( middle line of stalk )
  stalk.push_back( R2 { stalkSlices[ 0 ].x , estY( stalkSlices[ 0 ].x ) } );
  int idx = size - 1;
  stalk.push_back( R2 { stalkSlices[ idx ].x , estY( stalkSlices[ idx ].x ) } );

  // get length as stalk width
  // sort by length of slice 
  std::sort( stalkSlices.begin() , stalkSlices.end() ,
    []( const StalkSlice& a , const StalkSlice& b )
  {
    return a.ylength < b.ylength;
  } );

  int cutOff = int( stalkSlices.size() ) / parameters.StalkSlicesCutOffcoeff;
  stalkWidth = stalkSlices[ cutOff ].ylength;
}

