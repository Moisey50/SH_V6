#include "stdafx.h"

#include "Tecto_DetectFlowerComponents.h"

#include <fstream>
#include <deque>

using namespace nsTecto;

void DetectFlowerComponents::process(const char* szMFiguresName)
{
    std::ifstream strm(szMFiguresName);

    process(strm);
}

void DetectFlowerComponents::process(std::istream& strm)
{
    std::vector< std::vector<R2> > t_figures = readFigures(strm);

    process(t_figures);
}

ProcessPlantImage::eErrorCode DetectFlowerComponents::process(const std::vector< std::vector<R2> >& input)
{
    copyToPolylines(input, figures);

    //some cleaning.
    RemoveDuplicateVertices();
    SetStartingPoints();
    RemoveDuplicatedConturs();


    ConvertPolygonsToPolylines();
    FindOuterRectangles();

    ComputeAreas();
    //RemoveSmallConturs();
    //RemoveExtendedConturs();

    MashPolygons();

    FindConturOfFlowers();

    PrepareResults();

    return OK;
}

void DetectFlowerComponents::ComputeAreas()
{
    AttributesOfConturs.clear();
    for (int i = 0; i < (int)figures.size(); ++i)
    {
        double area = figures[i].Area();
        PolylineAttributes tmp{ i, area, -1 };
        AttributesOfConturs.push_back(tmp);
    }
}

void DetectFlowerComponents::RemoveSmallConturs()
{
    for (int j = 0; j < (int)AttributesOfConturs.size(); )
    {
        if (AttributesOfConturs[j].area < parametersDFC.SmallAreaMaxSize)
        {
            AttributesOfConturs.erase(AttributesOfConturs.begin() + j);
        }
        else
        {
            j++;
        }
    }
}

void DetectFlowerComponents::RemoveExtendedConturs()
{
    for (int j = 0; j < (int)figures.size(); ++j )
    {
        if (figures[j].Area() / figures[j].Perimeter() / figures[j].Perimeter() > 0.06 ) //parametersDFC.ExtendedContursMaxRatio)
        {
            // index and point convertin functions.
            auto convertIndex = [&](int thisIdx) { return InitialIndices[thisIdx]; };

            results.LengthlyConturs.push_back(convertIndex(j));
        }
    }
}
void DetectFlowerComponents::PrepareResults()
{
    // index and point convertin functions.
    auto convertIndex = [&](int thisIdx) { return InitialIndices[thisIdx]; };

    // copy relation information - how two components are connectd: indices of components, 
    // nearest points and distance.
    results.relations.clear();
    for (std::vector<Relation>::iterator it = relations.begin();
        it != relations.end(); ++it)
    {
        Relation relation;

        relation.idx0 = convertIndex(it->idx0);
        relation.idx1 = convertIndex(it->idx1);
        relation.r0 = it->r0;
        relation.r1 = it->r1;
        relation.distance = sqrt(R2::distance2(it->r0, it->r1));

        results.relations.push_back(relation);
    }

    // indices of included Conturs.
    results.includedConturs.clear();
    for (std::vector<int>::const_iterator it = includedConturs.begin(); it != includedConturs.end(); ++it)
        results.includedConturs.push_back(convertIndex(*it));
}

void DetectFlowerComponents::FindConturOfFlowers()
{
    // Find contur with max area ( seed contur ) amond conturs non included yet to flower.
    // and add to seed contur nearest conturs.
    for( componentCount = 0;; ++componentCount)
    {
        double areaMax = 0.0;
        int maxAreaIdx = 0;
        for (int i = 0; i < (int)figures.size(); i++)
        {
            if (AttributesOfConturs[i].componentNum != -1 )
                continue;

            double area = AttributesOfConturs[i].area;
            if (area > areaMax)
            {
                areaMax = area;
                maxAreaIdx = i;
            }
        }

        if (areaMax < parametersDFC.BigAreaMinSize)
            break;

        // find connected component, related to contur with maximum area.
        IdxMaxArea = maxAreaIdx;
        GetConnectedComponents();

        // mark just added conturs as included
        for ( int i = 0; i < int( includedConturs.size() ); ++i)
            AttributesOfConturs[ includedConturs[i] ].componentNum = componentCount;
    }

    // store all found conturs.
    includedConturs.clear();
    for (int i = 0; i < (int)figures.size(); i++)
        if (AttributesOfConturs[i].componentNum != -1)
            includedConturs.push_back(i);
}
