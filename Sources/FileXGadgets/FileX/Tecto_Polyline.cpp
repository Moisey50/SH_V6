#include "stdafx.h"
//#define NOMINMAX
#include "Tecto_Polyline.h"
#include <algorithm>

using namespace nsTecto;

PolylineRectangle::PolylineRectangle(int startIndex_, int endIndex_, const std::vector<R2>& vertices_) :
    startIndex(startIndex_),
    endIndex(endIndex_),
    vertices(&vertices_)
{
    if (startIndex >= 0
        && endIndex >= 0
        && startIndex < ( int )vertices->size()
        && endIndex <= ( int ) vertices->size()
        && startIndex < endIndex)
    {
        // compute outer boundary.
        boundary.s = boundary.e = (*vertices)[startIndex];
        for (int i = startIndex + 1; i < endIndex; i++)
            boundary = Rectangle::BoundRectangle(boundary, (*vertices)[i]);
    }
    else
    {
        startIndex = endIndex = 0;
        boundary.s = boundary.e = R2{ 0,0 };
    }
};

void Polyline::Mash(int SegmentCount)
{
    // mashing polyline sides. 
    // Split sides to group with count of sides < SegmentCount
    const std::vector<R2>& vertices = *this;

    mashedPolylines.clear();

    int nLines = int(vertices.size() - 1);
    int fullSegments = nLines / SegmentCount;

    for (int i = 0; i < fullSegments; ++i)
    {
        mashedPolylines.push_back(PolylineRectangle(i * SegmentCount, (i + 1) * SegmentCount + 1, vertices));
    }
    if (int rest = nLines % SegmentCount)
    {
        mashedPolylines.push_back(PolylineRectangle(fullSegments * SegmentCount, (int)vertices.size(), vertices));
    }

    // crate working variable workDistances
    for (int i = 0; i < ( int ) mashedPolylines.size(); ++i)
    {
        DistanceToPolylineRectangle tmp{ 0.0, &mashedPolylines[i] };
        workDistances.push_back(tmp);
    }

}

bool Polyline::IsRelated(const Polyline& base, double eps, PolylinesDistance& relation) const
{
    // two polylines are related when the distance between them < eps.
    // to avoid computing distance between them check first theier outer rectanges.
    R2 r0, r1;
    if (boundRectangle.IsOverlapped(base.boundRectangle)
        || boundRectangle.DistanceTo(base.boundRectangle) < eps)
    {
        double dist = DistanceTo(base, r0, r1, eps);
        if (dist < eps)
        {
            relation.distance = dist;
            relation.r0 = r0;
            relation.r1 = r1;
            return true;
        }
    }

    return false;
}

double Polyline::DistanceTo(std::vector<DistanceToPolylineRectangle>& wd, R2& p0, R2& p1, double distMin) const
{
    R2 minP;
    for (int i = 0; i < ( int ) size(); ++i)
    {
        double dist = (*this)[i].DistanceToMashedPolylines(wd, distMin, minP);
        if (dist < distMin)
        {
            p0 = (*this)[i];
            p1 = minP;
            distMin = dist;
        }
    }
    return distMin;
}

double Polyline::DistanceTo(const Polyline& p, R2& p0, R2& p1, double distMin)  const
{
    // distance is minimum of distances from verteces of first polyline to sides of second polyline and 
    // distances from verteces of second polyline to sides of first polyline
    distMin = DistanceTo(p.workDistances, p0, p1, distMin);
    distMin = p.DistanceTo(workDistances, p1, p0, distMin);
    return distMin;
}

double Polyline::TDistanceTo(const Polyline& p, R2& p0, R2& p1) const
{
    // brute force computing distance between two polylines.
    // it is minimum of distances from verteces of first polyline to sides of second polyline and 
    // distances from verteces of second polyline to sides of first polyline
    double distMin = DBL_MAX;
    R2 minP;
    for (int i = 0; i < ( int ) size(); ++i)
    {
        for (int j = 1; j < ( int ) p.size(); ++j)
        {
            double dist = (*this)[i].DistanceToLineSegment(p[j - 1], p[j], minP);
            if (dist < distMin)
            {
                p0 = (*this)[i];
                p1 = minP;
                distMin = dist;
            }
        }
    }

    for (int i = 0; i < ( int ) p.size(); ++i)
    {
        for (int j = 1; j < ( int ) size(); ++j)
        {
            double dist = p[i].DistanceToLineSegment((*this)[j - 1], (*this)[j], minP);
            if (dist < distMin)
            {
                p0 = minP;
                p1 = p[i];
                distMin = dist;
            }
        }
    }
    return distMin;
}

void Polyline::RemoveDuplicateVertices()
{
    for (;; )
    {
        if (size() <= 1)
            break;
        // if last vertex is equal first one, remove it.
        if ((*this)[int(size()) - 1] == (*this)[0])
            pop_back();
        else
            break;
    }
}

void Polyline::SetStartingPoint()
{
    // find the first point in lexicographic order
    std::vector<R2>::iterator itMin = begin();
    for (std::vector<R2>::iterator it = begin(); it != end(); ++it)
    {
        if (*it < *itMin)
            itMin = it;
    }

    // move it ahead 
    std::rotate(begin(), itMin, end());
}

bool Polyline::IsEqualTo(const Polyline& p) const
{
    // bit to bit comparing points of polylines
    if (size() != p.size())
        return false;

    for (int i = 0; i < ( int )size(); i++)
    {
        if ((*this)[i] != p[i])
            return false;
    }

    return true;
}

int Polyline::VerticalAxeIntersection(double x, double& ymin, double& ymax) const
{
    // x is outside of outer rectangle
    if (x < boundRectangle.s.x || x > boundRectangle.e.x)
        return 0;

    ymin = DBL_MAX;
    ymax = -DBL_MAX;

    // check intersectioin of each segment and compute the span of intersection segment
    int cnt = 0;
    for (int i = 1; i < ( int ) size(); ++i)
    {
        double y;
        int cnt0 = R2::VerticalLineAndLineSegmentIntersection(x, (*this)[i - 1], (*this)[i], y);

        if (cnt0)
        {
            ymin = std::min<double>(ymin, y);
            ymax = std::max<double>(ymax, y);
            cnt += cnt0;
        }
    }
    return cnt;
}

void Polyline::FindOuterRectangle()
{
    base_container::const_iterator it = begin();
    boundRectangle.s = boundRectangle.e = *it;
    for (; it != end(); ++it)
        boundRectangle = Rectangle::BoundRectangle(boundRectangle, *it );
}

double Polyline::Area() const
{
    const std::vector<R2>& v = *this;
    // common formula for the ared of polygon.
    int n = int(v.size()) - 1;
    double area = -v[n].y * v[0].x + v[n].x * v[0].y;
    for (int i = 0; i < n; i++)
    {
        area += -v[i].y * v[i + 1].x + v[i].x * v[i + 1].y;
    }

    return 0.5 * fabs(area);
}


double Polyline::Perimeter() const
{
    const std::vector<R2>& v = *this;

    // common formula for the ared of polygon.
    int n = int(v.size()) - 1;
    double perimeter = sqrt( R2::distance2(v[n], v[0]) ); 
    for (int i = 0; i < n; i++)
        perimeter += sqrt( R2::distance2(v[i], v[i + 1] ) );

    return perimeter;
}