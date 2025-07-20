#pragma once


#include <fstream>
#include <sstream>
#include <queue>

#include "Math/R2.h"
#include "Math/Lattice2D.h"

#define NOMINMAX
// parameters of processing
struct GridDetectorParameters
{
    double NeighbourDistanceMax;  // to filter too far neighbours.
    double LatticeBasisMaxAngle; // degree      *
    double LatticeBasisMaxDistance; // squared. *
    int LXLength, LYLength;  // L letter mark   ** -- LXLength = 1, LYLength = 2, count of segments, not points.
};

static const double DPI = 2.0 * asin(1.0);
using namespace ns;

class cGridDetector
{
public:

    struct InputRecord : public ns::R2
    {
        double area;
        bool IsBigArea;

        InputRecord()
        {
        }

        InputRecord(const ns::R2& t)
            : ns::R2(t), area(-1) {}

        InputRecord(const ns::R2& t, double a)
            : ns::R2(t), area(a) {}

        InputRecord& operator=(const ns::R2& t)
        {
            ns::R2::operator=(t);
            return *this;
        }

        double RadiusOfArea() const
{
            return sqrt(area / DPI);
        }
    };

protected:
    struct Node
    {
        enum enumDir { Right, Up, Left, Down };

        bool visited;
        int dx, dy; // steps to reach the node from the starting points
        Node* dirs[4]; // right, up, left, down

        const InputRecord* center;

        Node() : center(NULL)
        {
            visited = false;
            dirs[Up] = dirs[Down] = dirs[Left] = dirs[Right] = NULL;
            dx = dy = 0;
        }
        Node(const InputRecord* c) : center(c)
        {
            visited = false;
            dirs[Up] = dirs[Down] = dirs[Left] = dirs[Right] = NULL;
            dx = dy = 0;
        }

        bool IsConnected() const
        {
            if ((dirs[Up] == NULL) && (dirs[Down] == NULL) && (dirs[Left] == NULL) && (dirs[Right] == NULL))
                return false;
            return true;
        }

        int StepsDir(enumDir dir) const
        {
            const Node* t = this;
            int i = 0;

            while (t->dirs[dir] != NULL && t->dirs[dir]->center->IsBigArea == true)
                ++i, t = t->dirs[dir];

            return i;
        }

        int StepsUp() const
        {
            return StepsDir(Up);
        }

        int StepsDown() const
        {
            return StepsDir(Down);
        }

        int StepsLeft() const
        {
            return StepsDir(Left);
        }

        int StepsRight() const
        {
            return StepsDir(Right);
        }

        bool IsCenter(
            int LXLength,
            int LYLength,
            std::pair< cGridDetector::Node::enumDir, int>& dir,
            std::pair< cGridDetector::Node::enumDir, int>& dir1) const;

    };

    // datatype to keep the information about distance between grid step
    struct cGridStep
    {
        ns::R2 pnt;
        double length2;

        bool operator<(const cGridStep& a) const
        {
            return length2 < a.length2;
        }
    };

    struct IndexedR2 : public ns::R2
    {
        int index;

        using ns::R2::R2; // same ctors.

        IndexedR2()
        {
        }

        IndexedR2(const ns::R2 &t, int idx ) 
            : ns::R2(t), index( idx ) {}

        IndexedR2& operator=(const ns::R2 &t )
        {
            ns::R2::operator=(t);
            return *this;
        }
    };

    struct IndexedRectangle : public ns::Rectangle
    {
        int index;

        using ns::Rectangle::Rectangle; // same ctors.

        IndexedRectangle()
        {}

        IndexedRectangle(const ns::Rectangle& t, int idx)
            : ns::Rectangle(t), index(idx) {}

        IndexedRectangle& operator=(const ns::Rectangle& t)
        {
            ns::Rectangle::operator=(t);
            return *this;
        }
    };

    struct HashedCenters
    {
        IndexedRectangle boundary;
        int startIndex, endIndex; // range inteval values in the std::vector< IndexedR2 >
        std::vector< IndexedR2 >* indexedR2;

        HashedCenters() {}
    };

    // read data from stream and process figures.
    bool process( std::istream& strm );

    std::string m_LastCalibData ;
    // estimate grid steps in horizontal and vertical directions.
    void FindAreaDistribution();
    void FindDirectionsDistribution();
    void AddCentersDelta(double dx, double dy);
    void FindLatticeUnityBasis();
    void CollectShortestLatticeVectors(int degUnity);
    void AddGridStep(int size, const cGridStep &g);
    bool IsLatticeBasisVector(const ns::R2& basisVector, cGridStep& gs);
    static void copy(std::priority_queue<cGridStep>&, std::vector<cGridStep>&);
    const cGridStep& MedianOfShortestLatticeVectors() const
    {
        return vShortestGridSteps[vShortestGridSteps.size() / 2];
    }

    void FilterOutShortestLatticeVector(const cGridStep& baseVector);
    ns::R2 AverageOfShortestLatticeVector() const;
    bool FindShortestLatticeVector(ns::R2& sh);
    void MakeConnection(int i, int j);

    // estimate grid steps in horizontal and vertical directions.
    void MakeGraph();

    // visit nodes of the graph estimate grid steps in horizontal and vertical directions to
    // reach the node from a starting point.
    void WalkGraph(int startNodeIndex = 0);

    // find maximal steps to reach all nodes.
    void FindMaxSteps();

    // find the  nearest to (dx0,dy0).node
    void FindStepsOrigin();
    bool FindStepsOriginL();

    // move origin to the node founed in FindStepsOrigin()
    void ShiftStepsOrigin();

    bool FindGridCenter() const;
    bool FindGridXStep() const;
    bool FindGridYStep() const;

    // find initial estiated center and steps of the grid.
    bool FindGridParameters() const;

    void CorrectGridAxes();

    /////////////////////////

    // debug function
    void ShowNodes() const;

    bool LSMSolveGridParameters();
    bool IterationLSMSolveGridParameters();

    static bool average( const ns::R2 &collected, int cnt, ns::R2 &averaged )
    {
        if (cnt == 0)
            return false;
        averaged.x = collected.x / cnt;
        averaged.y = collected.y / cnt;

        return true;
    }

    void FindInputBoundary(void);

public:
    cGridDetector()
    {
        GridDetectorParameters p;
        p.NeighbourDistanceMax = 200;
        p.LatticeBasisMaxAngle = 1.0;
        p.LatticeBasisMaxDistance = 2;

        SetParameters(p);

        init();
    }

    void SetParameters(const GridDetectorParameters& p);

    // process data from file.
    bool process(const char* szMFiguresName);

    // read data from stream
    void readCenters(std::istream&);

    // read data from string
    void cGridDetector::readCenters( std::string& Src ) ;

    // process figures.
    bool process();
    bool process( std::string& AsString );
    bool GetLastCalibData( std::string& LastDataAsString )
    {
      if ( m_LastCalibData.size() > 100 )
      {
        LastDataAsString = m_LastCalibData ;
        return true ;
      }
      return false ;
    }

    // interface to lattice and function to evaluate nearest node 
    // for a given point.
    const cLattice2D& Grid() const { return grid;  };
    bool NearestNode(const ns::R2& r, int& i0, int& i1) const
    {
        return grid.NearestNode(r, i0, i1);
    }

    void ImageToWorld(const ns::R2& r, ns::R2& word, int& i0, int& i1)
    {
        grid.ToLatticeCoordinates(r, word, i0, i1);
    }

    void WorldToImage(const ns::R2& w, ns::R2& r)
    {
        grid.FromLatticeCoordinates(w, r);
    }

    const ns::Rectangle& GetInputBoundary() const
    {
        return InputBoundary;
    }

    const std::vector< InputRecord >& GetInput() const
    {
        return mInput;
    }

    const GridDetectorParameters& GetParameters()
    {
        return parameters;
    }

    void init();


    const ns::R2& center() const 
    {
        return estimatedCenter;
    }

    const ns::R2& vx() const
    {
        return estimatedXGridStep;
    }

    const ns::R2& vy() const
    {
        return estimatedYGridStep;
    }

protected:

    GridDetectorParameters parameters;
    double dist2;// = parameters.NeighbourDistanceMax * parameters.NeighbourDistanceMax;
    double cosMaxAngle2; // = squar( cos(parameters.LatticeBasisMaxAngle * DPI / 180) );

    std::vector< InputRecord > mInput;
    ns::Rectangle InputBoundary;

    std::vector<int> unityVectorsCnt;
    int degFirst, degSecnd;

    std::priority_queue<cGridStep> pqShortestGridSteps;
    std::vector<cGridStep> vShortestGridSteps;
    mutable ns::R2 gridStep0, gridStep1;
    mutable ns::R2 gridStep2, gridStep3; // opposite to aboves
    double dBound0, dBound1;

    std::vector<Node> nodes;
    int dxMin, dxMax, dyMin, dyMax; // max step to reach nodes.
    int dx0, dy0; // origin of grid
    int idxOrigin;
    bool bLDetected;
    std::pair< Node::enumDir, int> dirLongest, dirSecond;
    mutable ns::R2 estimatedCenter, estimatedXGridStep, estimatedYGridStep;

    cLattice2D grid;
}; 
