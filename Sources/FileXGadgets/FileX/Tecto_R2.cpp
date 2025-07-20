#include "stdafx.h"

#include "Tecto_R2.h"
#include "Tecto_Polyline.h"

#include <algorithm>

using namespace nsTecto;

double R2::DistanceToRectangle(const nsTecto::Rectangle& rect) const
{
    return DistanceToRectangle(rect.s, rect.e);
}

double R2::DistanceToRectangle(const R2& minV, const R2& maxV) const
{
    auto max3 = [](double a, double b, double c) -> double
    {
        if (a > b)
            return (a > c) ? a : c;
        else
            return (b > c) ? b : c;
    };
    // get horizontal and vertical distances  
    double dx = max3(minV.x - x, 0, x - maxV.x);
    double dy = max3(minV.y - y, 0, y - maxV.y);
    return dx * dx + dy * dy;
}

double R2::DistanceToPolyline(const R2* vertices, int cnt, R2& minP) const
{
    // the distance to polyline is minimum of distances to its line segments.
    double distMin = DBL_MAX;
    for (int i = 1; i < cnt; i++)
    {
        R2 m;
        double dist = DistanceToLineSegment(vertices[i - 1], vertices[i], m);
        if (dist < distMin)
        {
            distMin = dist;
            minP = m;
        }
    }
    return distMin;
}

double R2::DistanceToPolyline(const nsTecto::Polyline& pl, R2& minP) const
{
    if (pl.size() == 0) // to avoid exception 
        return DBL_MAX;
    return DistanceToPolyline(&pl[0], int(pl.size()), minP);
}

double R2::DistanceToMashedPolylines(std::vector<DistanceToPolylineRectangle>& pl, double eps, R2& minP) const
{
    // compute distances to outer rectangles of parts of polylines. Move the pointers with distance 
    // < eps ahead.
    std::vector<DistanceToPolylineRectangle>::iterator itInside = pl.begin();
    for (std::vector<DistanceToPolylineRectangle>::iterator it = pl.begin(); it != pl.end(); ++it)
    {
        it->distance = DistanceToRectangle(it->pl->boundary);
        if (it->distance < eps)
        {
            if (itInside != it)
                std::swap(*itInside, *it);
            ++itInside;
        }
    }

    // distance to all rectangles with the distance to it < eps
    if (pl.begin() == itInside)
        return eps;

    // sort rectangles < eps
    std::sort(pl.begin(), itInside,
        [](const DistanceToPolylineRectangle& a, const DistanceToPolylineRectangle& b)
        {
            return a.distance < b.distance;
        });

    // search between rectangles with the distance to it < eps
    double distMin = eps;
    for (std::vector<DistanceToPolylineRectangle>::const_iterator it = pl.begin(); it != itInside; ++it)
    {
        // distance to outer rectange > distMin ==> distance to all line signments inside the rectangel > distMin.
        // rectangeles sorten in distance increasing order. 
        if (it->distance >= distMin)
            break;

        R2 m;
        const PolylineRectangle& plr = *(it->pl);
        const std::vector<R2>& vertices = *plr.vertices;
        const R2* start = &vertices[plr.startIndex];
        int size = int(plr.endIndex - plr.startIndex);
        double dist = DistanceToPolyline(start, size, m);
        // decrease the minimum distance.
        if (dist < distMin)
        {
            distMin = dist;
            minP = m;
        }
    }

    return distMin;
}

double R2::DistanceToLineSegment(const R2& s, const R2& e, R2& minP) const
{
    double A = x - s.x;
    double B = y - s.y;
    double C = e.x - s.x;
    double D = e.y - s.y;

    double len_sq = C * C + D * D;
    if (len_sq == 0) //in case of 0 length line
        minP = s;
    else
    {
        // search nearest point on the vector s + (e-s)*param
        double dot = A * C + B * D;
        double param = dot / len_sq;

        if (param <= 0)
            minP = s;
        else if (param >= 1)
            minP = e;
        else
        {
            minP.x = s.x + param * C;
            minP.y = s.y + param * D;
        }
    }

    double dx = x - minP.x;
    double dy = y - minP.y;
    return dx * dx + dy * dy;
}

int R2::VerticalLineAndLineSegmentIntersection(double x, const R2& s, const R2& e, double& y)
{
    double x0 = s.x, x1 = e.x;
    int ret = 0;

    //check equality only for the start point
    if (x0 == x1)
    {
        if (x0 == x)
        {
            y = s.y;
            ret = 1;
        }
        return ret;
    }

    // the x shold be in [x0,x1) segment
    if (x0 < x1)
    {
        if (x0 <= x && x < x1)
            ret = 1;
    }
    else //if (x0 > x1)
    {
        if (x1 < x && x <= x0)
            ret = 1;
    }
    // find y component of the intersection point
    if (ret == 1)
        y = s.y + (x - s.x) * (e.y - s.y) / (e.x - s.x);

    return ret;
}

double R2::dot(const nsTecto::R2& b) const
{
    return x * b.x + y * b.y;
}

double R2::length2() const
{
    return x * x + y * y;
}

bool R2::coeff(const R2& e0, const R2& e1, double& k0, double& k1) const
{
    double k00 = dot(e0, e0);
    double k01 = dot(e0, e1);
    double k11 = dot(e1, e1);

    double D = k00 * k11 - k01 * k01;
    if (D == 0)
        return false;


    double x0 = dot(e0);
    double x1 = dot(e1);

    k0 = ( k11 * x0 - k01 * x1) / D;
    k1 = (-k01 * x0 + k00 * x1) / D;

    return true;
}

bool Rectangle::IsOverlapped(const Rectangle& rect) const
{
    const R2& l1 = s, r1 = e, l2 = rect.s, r2 = rect.e;

    // If one rectangle is on left side of other
    if (l1.x > r2.x || l2.x > r1.x)
        return false;

    // If one rectangle is above other
    if (r1.y < l2.y || r2.y < l1.y)
        return false;

    return true;
}

double Rectangle::DistanceTo(const Rectangle& rect) const
{
    auto dist = [](double x, double y) { return x * x + y * y; };
    auto square = [](double x) { return x * x; };

    // position of rect related to this
    bool left = rect.e.x < s.x;
    bool right = e.x < rect.s.x;
    bool below = rect.e.y < s.y;
    bool above = e.y < rect.s.y;

    if (left && below)
        return R2::distance2(rect.e, s);
    else if (left && above)
        return dist(s.x - rect.e.x, e.y - rect.s.y);
    else if (right && below)
        return dist(e.x - rect.s.x, s.y - rect.e.y);
    else if (right && above)
        return R2::distance2(rect.s, e);
    else if (left)
        return square(s.x - rect.e.x);
    else if (right)
        return square(rect.s.x - e.x);
    else if (above)
        return square(rect.s.y - e.y);
    else if (below)
        return square(s.y - rect.e.y);

    return 0.0;
}


nsTecto::Rectangle Rectangle::BoundRectangle(const Rectangle& a, const Rectangle& b)
{
    Rectangle ret;
    ret.s.x = std::min<double>(a.s.x, b.s.x);
    ret.s.y = std::min<double>(a.s.y, b.s.y);
    ret.e.x = std::max<double>(a.e.x, b.e.x);
    ret.e.y = std::max<double>(a.e.y, b.e.y);
    return ret;
}


nsTecto::Rectangle Rectangle::BoundRectangle(const Rectangle& a, const R2& r)
{
    Rectangle ret;
    ret.s.x = std::min<double>(a.s.x, r.x);
    ret.s.y = std::min<double>(a.s.y, r.y);
    ret.e.x = std::max<double>(a.e.x, r.x);
    ret.e.y = std::max<double>(a.e.y, r.y);
    return ret;
}