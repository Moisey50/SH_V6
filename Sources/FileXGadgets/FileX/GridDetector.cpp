// GridDetector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdafx.h"
#include "GridDetector.h"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <deque>

#include "Tecto_R2.h"
#include "LeastSquaresSolver.h"

using namespace std ;
using namespace nsTecto;

bool cGridDetector::Node::IsCenter(
    int LXLength,
    int LYLength,
    std::pair< cGridDetector::Node::enumDir, int>& dir,
    std::pair< cGridDetector::Node::enumDir, int>& dir1) const
{
    const std::pair< cGridDetector::Node::enumDir, int> res[4] =
    {
        { cGridDetector::Node::Right, StepsRight() },
        { cGridDetector::Node::Up, StepsUp() },
        { cGridDetector::Node::Left, StepsLeft() },
        { cGridDetector::Node::Down, StepsDown() }
    };

    std::pair< cGridDetector::Node::enumDir, int> d0 = res[0];

    for (int i = 1; i < 4; i++)
        if (res[i].second > d0.second)
            d0 = res[i];

    if (d0.second >= LXLength)
{
        std::pair< cGridDetector::Node::enumDir, int> d1 = res[(int(d0.first) + 1) % 4];
        const std::pair< cGridDetector::Node::enumDir, int>& res270 = res[(int(d0.first) + 3) % 4];

        if (res270.second > d1.second)
            d1 = res270;
        if (d1.second >= LYLength)
        {
            dir = d0;
            dir1 = d1;
            return true;
        }
    }
    return false;
}


bool cGridDetector::process(const char* szMFiguresName)
{
  std::ifstream strm( szMFiguresName );

  return process(strm);
}

bool cGridDetector::process(std::istream& strm)
{
    readCenters(strm);

    return process();
}

bool cGridDetector::process( std::string& AsString )
{
  readCenters( AsString );

  return process();
}

void cGridDetector::SetParameters( const GridDetectorParameters& p )
{
  if (p.NeighbourDistanceMax > 1.0) parameters.NeighbourDistanceMax = p.NeighbourDistanceMax;
  if (p.LatticeBasisMaxDistance > 0.05) parameters.LatticeBasisMaxDistance = p.LatticeBasisMaxDistance;
  if (p.LatticeBasisMaxAngle > 0.00001) parameters.LatticeBasisMaxAngle = p.LatticeBasisMaxAngle;

  dist2 = parameters.NeighbourDistanceMax * parameters.NeighbourDistanceMax;
  cosMaxAngle2 = cos( parameters.LatticeBasisMaxAngle * DPI / 180 );
  cosMaxAngle2 *= cosMaxAngle2;

    parameters.LXLength = 4;
    parameters.LYLength = 2;
}

void cGridDetector::init()
{
    mInput.clear();
    InputBoundary = nsTecto::Rectangle();

    unityVectorsCnt.clear();
    degFirst = degSecnd = 0;

    std::priority_queue<cGridStep> tmp;
    std::swap(pqShortestGridSteps, tmp);
    vShortestGridSteps.clear();

    gridStep0 = gridStep1 =
        gridStep2 = gridStep3 = nsTecto::R2{ 0, 0 };
    dBound0 = dBound1 = 0;

    nodes.clear();
    dxMin = dxMax = dyMin = dyMax = 0;
    dx0 = dy0 = 0; // origin of grid
    idxOrigin = 0;
    bLDetected = false;
    dirLongest = dirSecond = std::pair< Node::enumDir, int>{ Node::Up, 0 };
    estimatedCenter = estimatedXGridStep = estimatedYGridStep = nsTecto::R2{ 0, 0 };

    grid = cLattice2D();
}


void cGridDetector::readCenters(std::istream& strm)
{
  std::vector< InputRecord > centers;
  std::swap(centers, mInput);

  // figure start marker.
  std::string s;
  for (int i = 0; i < 3; i++)
    std::getline( strm , s );

  //0  1198.00  891.00  27.85  33.78     720    137      0.0      0.0    0.00    0.00    0.00   0.00   0.00   0.00   0.00   0.00  1185   875  1211   907  4504  4504 Name = spot_for_rect;
  while (std::getline( strm , s ))
  {
    InputRecord r2;
    double skip;

    std::istringstream ostrm( s );
    int cnt; 
    ostrm >> cnt >> r2.x >> r2.y >> skip >> skip >> r2.area;
    if (ostrm.fail())
      break;
    mInput.push_back(r2);
  }
}

void cGridDetector::readCenters( std::string& Src )
{
  std::vector< InputRecord > centers;
  std::swap( centers , mInput );

  // omit first two strings (frame number and Statistics data).
  size_t iPos = 0 ;
  for (int i = 0; i < 2; i++)
    iPos = Src.find( '\n' , iPos ) + 1 ;

  size_t iCurrPos = iPos + 1 ;
  while (( iPos = Src.find( '\n' , iCurrPos ) ) != string::npos)
  {
    InputRecord r2;
    double skip;

    string token = Src.substr( iCurrPos , iPos - iCurrPos ) ;
    iCurrPos = iPos + 1 ;
    if (token[ 0 ] != '/')
    {
      std::istringstream ostrm( token );
      int cnt; 
      ostrm >> cnt >> r2.x >> r2.y >> skip >> skip >> r2.area;
      if (ostrm.fail())
        break;
      mInput.push_back( r2 );
    }
  }

  if (mInput.size() > 5)
    m_LastCalibData = Src ;
  else
    m_LastCalibData.clear() ;
}

bool cGridDetector::process()
  {
    // find bounding rectangle.
    FindInputBoundary();

    // Detect points of calibrating "L" letter.
    // sort all input points by area in decreasing order
    // and mark first 2 * (parameters.LXLength + parameters.LYLength - 1) points.
    FindAreaDistribution();

    // find axes directions.
    // Find for each pair of point convert its direction's angle to degree
    // and increment counter of the degree in a array. At end the array 
    // contain counter of for all degrees in range[0,359]
    FindDirectionsDistribution();

    // Find degree with maximal counter in first part the array [0,179]. Find second in range [fist+45, first +135)
    // Rearrange ( swap and change signs, so first is in first quadrant and second in the second ).
    // Now we have directions of axes.
  FindLatticeUnityBasis();

    // for both directions collecnt shortest vectors from center to center.
    // count of collected shortest vectors is input.size().
    // filter out anormal vector from this collected vector and average the rest 
    // to get the axe vector. ( vector from center to center )
  CollectShortestLatticeVectors( degFirst );
  if (false == FindShortestLatticeVector( gridStep0 ))
        return false;

  CollectShortestLatticeVectors( degSecnd );
  if (false == FindShortestLatticeVector( gridStep1 ))
        return false;

    // find the neighbors of centers.
  MakeGraph();

    // starting a center visit all centers. Now each center have the counts of steps in
    // both directions to reach it from the starting center.
  WalkGraph();

    // find maximal steps to reach all nodes.
  FindMaxSteps();

    // Try to find the left bottom angle of "L" letter.
    // Find the center which have parameters.LXLength neighbor with big area 
    // in one direction and parameters.LYLength in the another.
    // If there is not center, chose the as the center of the centers.
    if (false == FindStepsOriginL())
  FindStepsOrigin();

    // Correct data so origin is (0,0).
  ShiftStepsOrigin();

    // Average steps to right and up. Using the steps to right and up and integer
    // coorinates of the centers, average of oorigin.
  if (false == FindGridParameters())
        return false;

    // Make one step Least Square method to correct origin and steps to right and up.
    if( false == LSMSolveGridParameters() )
        return  false;

    // If the "L" has been detected arrange step along long and short sides of "L".
    CorrectGridAxes();

  //ShowNodes();

  //std::cout << nodes[this->idxOrigin].center->x << " " << nodes[this->idxOrigin].center->y << std::endl;
  std::cout << this->estimatedCenter.x << " " << this->estimatedCenter.y << std::endl;
  std::cout << this->estimatedXGridStep.x << " " << this->estimatedXGridStep.y << std::endl;
  std::cout << this->estimatedYGridStep.x << " " << this->estimatedYGridStep.y << std::endl;

  //PrepareResults();

    return true;
}

void cGridDetector::AddCentersDelta( double dx , double dy )
{
  double d = atan2( dy , dx ) / DPI * 180;
  if (d < 0) d += 360;
  ++unityVectorsCnt[ ( ( int ) ( d + 0.5 ) ) % 360 ];
  ++unityVectorsCnt[ ( ( int ) ( d + 0.5 ) + 180 ) % 360 ];
}

void cGridDetector::FindAreaDistribution()
{
    std::vector< InputRecord >& input = mInput;

    std::sort(input.begin(), input.end(), 
      [](const InputRecord& a, const InputRecord& b) { return a.area > b.area; });

    // mark input points as with big area 
    const int markCount = 2 * (parameters.LXLength + parameters.LYLength - 1);

    for (int i = 0; i < (int)input.size(); i++)
        input[i].IsBigArea = (i < markCount);
}

void cGridDetector::FindDirectionsDistribution()
{
  const std::vector< InputRecord >& input = mInput;
  std::vector<int> tmp( 360 , 0 );
  std::swap( tmp , unityVectorsCnt );

  for (int i = 0; i < int( input.size() ); ++i)
    for (int j = 0; j < i; ++j)
    {
      double dx = input[ i ].x - input[ j ].x;
      double dy = input[ i ].y - input[ j ].y;

      AddCentersDelta( dx , dy );
    }
}

void cGridDetector::FindLatticeUnityBasis()
{
  std::vector<int>::const_iterator itMax = std::max_element( unityVectorsCnt.begin() , unityVectorsCnt.begin() + 180 );
  std::vector<int>::const_iterator itSec = std::max_element( itMax + ( 90 - 45 ) , itMax + ( 90 + 45 ) );

  degFirst = int( itMax - unityVectorsCnt.begin() );
  degSecnd = int( itSec - unityVectorsCnt.begin() );
  if (degFirst >= 90)
  {
    degSecnd -= 180;
    std::swap( degFirst , degSecnd );
  }
}

void cGridDetector::AddGridStep(int size, const cGridStep &g)
{
  if (int( pqShortestGridSteps.size() ) < size)
    pqShortestGridSteps.push( g );
  else
  {
    // replace .
    if (pqShortestGridSteps.top().length2 > g.length2)
    {
      pqShortestGridSteps.pop();
      pqShortestGridSteps.push( g );
    }
  }
}

bool cGridDetector::IsLatticeBasisVector( const R2& basisVector , cGridStep& gs )
{
  R2& r = gs.pnt;

  double r2 = r.length2();

  if (r2 < dist2)
  {
    double dot = basisVector.dot( r );
    if (dot < 0)
    {
      r.x = -r.x; // test opposite vector
      r.y = -r.y;
      dot = -dot;
    }

    double cos2 = ( dot * dot ) / r2;

    if (cos2 >= cosMaxAngle2)
    {
      gs.length2 = r2;
      return true;
    }

  }
  return false;
}


void cGridDetector::CollectShortestLatticeVectors( int degUnity )
{
  double radUnity = degUnity * DPI / 180;
  R2 basisVector { cos( radUnity ) , sin( radUnity ) };

    const std::vector< InputRecord >& input = mInput;

    int size = int(input.size());

  std::priority_queue<cGridStep> q;
  std::swap( pqShortestGridSteps , q );

  for (int i = 0; i < int( input.size() ); ++i)
    for (int j = 0; j < i; ++j)
    {
      cGridStep gs { { input[ i ].x - input[ j ].x , input[ i ].y - input[ j ].y } };
      if (true == IsLatticeBasisVector( basisVector , gs ))
      {
        AddGridStep( size , gs );
      }

    }
}

void cGridDetector::copy( std::priority_queue<cGridStep>& pq , std::vector<cGridStep>& v )
{
  std::vector<cGridStep> v_tmp;
  std::swap( v_tmp , v );

    while (!pq.empty()) {
    v.push_back( pq.top() );
    pq.pop();
  }
}

void cGridDetector::FilterOutShortestLatticeVector( const cGridStep& baseVector )
{
  // filter out the grid points which are too differ in length
  double bsLength2 = sqrt( baseVector.length2 );
  vShortestGridSteps.erase(
    std::remove_if( vShortestGridSteps.begin() , vShortestGridSteps.end() ,
      [ & ]( const cGridStep& gs )
  {
    return ( fabs( bsLength2 - sqrt( gs.length2 ) ) > parameters.LatticeBasisMaxDistance );
  } ) ,
    vShortestGridSteps.end() );
}

nsTecto::R2 cGridDetector::AverageOfShortestLatticeVector() const
{
  R2 average { 0 , 0 };
  average = std::accumulate( vShortestGridSteps.begin() , vShortestGridSteps.end() , average , []( const R2& a , const cGridStep& b )
  {
    return R2 { a.x + b.pnt.x , a.y + b.pnt.y };
  } );
  return R2 { average.x / vShortestGridSteps.size() , average.y / vShortestGridSteps.size() };
}

bool cGridDetector::FindShortestLatticeVector( nsTecto::R2& sh )
{
  if (pqShortestGridSteps.size() == 0)
    return false;

  // copy priority queue to vector
  copy( pqShortestGridSteps , vShortestGridSteps );
  // get median valie
  const cGridStep& medianValue = MedianOfShortestLatticeVectors();

  FilterOutShortestLatticeVector( medianValue );

  // get average of such vectors
  sh = AverageOfShortestLatticeVector();
  return true;
}

void cGridDetector::MakeConnection( int i , int j )
{
    const std::vector< InputRecord >& input = mInput;

  R2 r0 = input[ j ] - input[ i ];

  if (r0.length2() > dist2)
    return;

  if (R2::distance2( r0 , gridStep0 ) < dBound0)
  {
    // from i to j to right
        nodes[i].dirs[Node::Right] = &nodes[j];
        nodes[j].dirs[Node::Left] = &nodes[i];
  }
  else
    if (R2::distance2( r0 , gridStep2 ) < dBound0)
    {
            nodes[i].dirs[Node::Left] = &nodes[j];
            nodes[j].dirs[Node::Right] = &nodes[i];
    }

  if (R2::distance2( r0 , gridStep1 ) < dBound1)
  {
    // from i to j to right
        nodes[i].dirs[Node::Up] = &nodes[j];
        nodes[j].dirs[Node::Down] = &nodes[i];
  }
  else
    if (R2::distance2( r0 , gridStep3 ) < dBound1)
    {
            nodes[i].dirs[Node::Down] = &nodes[j];
            nodes[j].dirs[Node::Up] = &nodes[i];
    }
}

void cGridDetector::MakeGraph()
{
    const std::vector< InputRecord >& input = mInput;

  dBound0 = 0.15 * gridStep0.length2();
  dBound1 = 0.15 * gridStep1.length2();

  gridStep2 = -gridStep0;
  gridStep3 = -gridStep1;

  std::vector<Node> tmp( input.size() );
  std::swap( nodes , tmp );

  for (int i = 0; i < int( input.size() ); ++i)
  {
    nodes[ i ].center = &input[ i ];
    for (int j = 0; j < i; ++j)
      MakeConnection( i , j );
  }
}

void cGridDetector::WalkGraph( int startNodeIndex )
{
  if (int( nodes.size() ) < startNodeIndex) // bullet proofing.
    return;

  std::deque< Node* > ToVisit;

  Node* next = &nodes[ startNodeIndex ];
  ToVisit.push_back( next ); // push starting node to deque
  next->dx = next->dy = 0;
  next->visited = true;

  auto goDirection = [ & ]( Node* direction , int dx , int dy )
  {
    // check is there edge in this direction and it is not visited yet.
    if (direction != NULL && direction->visited != true)
    {
      // increment steps and put node "to be visited" list.
      direction->dx = next->dx + dx;
      direction->dy = next->dy + dy;
      direction->visited = true;
      ToVisit.push_back( direction );
    }
  };

  while (ToVisit.empty() == false)
  {
    next = ToVisit.front();
    ToVisit.pop_front();

        goDirection(next->dirs[Node::Up], 0, 1);
        goDirection(next->dirs[Node::Down], 0, -1);
        goDirection(next->dirs[Node::Left], -1, 0);
        goDirection(next->dirs[Node::Right], 1, 0);
  }
}


void cGridDetector::FindMaxSteps()
{
  dxMin = dyMin = INT_MAX;
  dxMax = dyMax = INT_MIN;

  for (std::vector<Node>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    dxMin = std::min<int>( dxMin , it->dx );
    dxMax = std::max<int>( dxMax , it->dx );
    dyMin = std::min<int>( dyMin , it->dy );
    dyMax = std::max<int>( dyMax , it->dy );
  }
  dx0 = ( dxMin + dxMax ) / 2;
  dy0 = ( dyMin + dyMax ) / 2;
}

void cGridDetector::FindStepsOrigin()
{
  int distMin = INT_MAX;
  std::vector<Node>::const_iterator itMin;

  for (std::vector<Node>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    int dx = it->dx - dx0;
    int dy = it->dy - dy0;
    int dist = dx * dx + dy * dy;
    if (distMin > dist)
    {
      distMin = dist;
      itMin = it;
    }
  }
  idxOrigin = int( itMin - nodes.begin() );
}

bool cGridDetector::FindStepsOriginL()
{
    for (std::vector<Node>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        if (it->center->IsBigArea == false)
            continue;

        if (true == it->IsCenter(parameters.LXLength, parameters.LYLength, dirLongest, dirSecond))
        {
            idxOrigin = int(it - nodes.begin());
            return bLDetected = true;
        }
    }

    dirLongest = dirSecond = std::pair< Node::enumDir, int>{ Node::Up, 0 };
    return bLDetected = false;
}


void cGridDetector::ShiftStepsOrigin()
{
  int dx = nodes[ idxOrigin ].dx;
  int dy = nodes[ idxOrigin ].dy;
  for (std::vector<Node>::iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    it->dx -= dx;
    it->dy -= dy;
  }

  dxMin -= dx;
  dyMin -= dy;

  dxMax -= dx;
  dyMax -= dy;

  dx0 -= dx;
  dy0 -= dy;
}

bool cGridDetector::FindGridParameters() const
{
  if (false == FindGridXStep()) return false;
  if (false == FindGridYStep()) return false;
  if (false == FindGridCenter()) return false;
  return true;
}

bool cGridDetector::FindGridCenter() const
{
  R2 center { 0 , 0 };
  int cnt = 0;

  for (std::vector<Node>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    if (false == it->IsConnected())
      continue;

    center.x += it->center->x - it->dx * estimatedXGridStep.x - it->dy * estimatedYGridStep.x;
    center.y += it->center->y - it->dx * estimatedXGridStep.y - it->dy * estimatedYGridStep.y;
    ++cnt;
  }

  return average( center , cnt , estimatedCenter );
}

bool cGridDetector::FindGridXStep() const
{
  R2 xStep { 0 , 0 };
  int cnt = 0;
    Node::enumDir dir = Node::Right;

  for (std::vector<Node>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
        if (it->dirs[dir] != NULL)
    {
            xStep.x += it->dirs[dir]->center->x - it->center->x;
            xStep.y += it->dirs[dir]->center->y - it->center->y;
      ++cnt;
    }
  }

  return average( xStep , cnt , estimatedXGridStep );
}

bool cGridDetector::FindGridYStep() const
{
  R2 yStep { 0 , 0 };
  int cnt = 0;

    Node::enumDir dir = Node::Up;

  for (std::vector<Node>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
        if (it->dirs[dir] != NULL)
    {
            yStep.x += it->dirs[dir]->center->x - it->center->x;
            yStep.y += it->dirs[dir]->center->y - it->center->y;
      ++cnt;
    }
  }

  return average( yStep , cnt , estimatedYGridStep );
}

void cGridDetector::CorrectGridAxes()
{
    if (false == bLDetected)
        return;
    //Right Up are default valus

    Node::enumDir newXDir = dirLongest.first;
    Node::enumDir newYDir = dirSecond.first;
    nsTecto::R2 newXStep, newYStep;

    if (newXDir == Node::Right) { newXStep = estimatedXGridStep; }
    else if (newXDir == Node::Up) { newXStep = estimatedYGridStep; }
    else if (newXDir == Node::Left) { newXStep = -estimatedXGridStep; }
    else if (newXDir == Node::Down) { newXStep = -estimatedYGridStep; }

    if (newYDir == Node::Right) { newYStep = estimatedXGridStep; }
    else if (newYDir == Node::Up) { newYStep = estimatedYGridStep; }
    else if (newYDir == Node::Left) { newYStep = -estimatedXGridStep; }
    else if (newYDir == Node::Down) { newYStep = -estimatedYGridStep; }

    estimatedXGridStep = newXStep;
    estimatedYGridStep = newYStep;
}

void cGridDetector::ShowNodes() const
{
    auto tmp_nodes(nodes);

    std::sort(tmp_nodes.begin(), tmp_nodes.end(), [](const Node& a, const Node& b)
  {
    if (a.dx == b.dx)
      return a.dy < b.dy;
    return a.dx < b.dx;
  } );

    for (std::vector<Node>::const_iterator it = tmp_nodes.begin(); it != tmp_nodes.end(); ++it)
  {
    double dX = it->center->x - ( estimatedCenter.x + it->dx * estimatedXGridStep.x + it->dy * estimatedYGridStep.x );
    double dY = it->center->y - ( estimatedCenter.y + it->dx * estimatedXGridStep.y + it->dy * estimatedYGridStep.y );

    double error = sqrt( dX * dX + dY * dY );

    int i0 , i1;

    NearestNode( *it->center , i0 , i1 );

    std::cout << ( error )
      << " " << it->dx
      << " " << it->dy
      << " (" << i0
      << " " << i1 << ")"
      << " " << ( it->center->x )
      << " " << ( it->center->y )
      << " " << ( dX )
      << " " << ( dY )
      << " " << ( error )
      << std::endl;
  }
}

bool cGridDetector::IterationLSMSolveGridParameters()
{
  LeastSquaresSolver lsm( 6 );
  double partialsX[ 6 ] = { 1 , 0 , 0 , 0 , 0 , 0 };
  double partialsY[ 6 ] = { 0 , 0 , 0 , 1 , 0 , 0 };
  double error = 0.0;
  int cnt = 0;

  for (std::vector<Node>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    if (false == it->IsConnected())
      continue;

    partialsX[ 1 ] = it->dx;
    partialsX[ 2 ] = it->dy;
    double measX = it->center->x - ( estimatedCenter.x + it->dx * estimatedXGridStep.x + it->dy * estimatedYGridStep.x );
    lsm.addMeasurement( measX , partialsX );

    partialsY[ 4 ] = it->dx;
    partialsY[ 5 ] = it->dy;
    double measY = it->center->y - ( estimatedCenter.y + it->dx * estimatedXGridStep.y + it->dy * estimatedYGridStep.y );
    lsm.addMeasurement( measY , partialsY );

    error += measX * measX;
    error += measY * measY;
    cnt++;
  }

  //std::cout << "error " << sqrt(error / 2 / cnt) << std::endl;
  if (lsm.solve() == false)
    return false;

  const std::vector<double>& solution = lsm.GetSolution();

  estimatedCenter.x += solution[ 0 ];
  estimatedXGridStep.x += solution[ 1 ];
  estimatedYGridStep.x += solution[ 2 ];
  estimatedCenter.y += solution[ 3 ];
  estimatedXGridStep.y += solution[ 4 ];
  estimatedYGridStep.y += solution[ 5 ];

  //error = 0.0;
  //
  //for (std::vector<Node>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  //{
  //    if (false == it->IsConnected())
  //        continue;
  //
  //    double measX = it->center->x - (estimatedCenter.x + it->dx * estimatedXGridStep.x + it->dy * estimatedYGridStep.x);
  //    double measY = it->center->y - (estimatedCenter.y + it->dx * estimatedXGridStep.y + it->dy * estimatedYGridStep.y);
  //
  //    error += measX * measX;
  //    error += measY * measY;
  //}
  //
  //std::cout << "error " << sqrt(error / 2 / cnt) << std::endl;

  return true;
}

bool cGridDetector::LSMSolveGridParameters()
{
  // model of parameters are linear, so one iteration is enough.
  if (false == IterationLSMSolveGridParameters())
    return false;

  grid = cLattice2D( estimatedCenter , estimatedXGridStep , estimatedYGridStep );

  return true;
}

void cGridDetector::FindInputBoundary()
{
    const std::vector< InputRecord >& input = mInput;
  nsTecto::Rectangle &boundary = InputBoundary;
  boundary.s = boundary.e = R2 { 0 , 0 };

  if (input.size() >= 1)
  {
    boundary.s = boundary.e = input[ 0 ];
        for (size_t i = 1; i < input.size(); i++)
      boundary = Rectangle::BoundRectangle( boundary , input[ i ] );
  }
  }
