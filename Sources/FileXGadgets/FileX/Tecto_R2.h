#pragma once

// File contains R2 and Rectangle classes.

#include <vector>

namespace nsTecto
{
    class Rectangle;
    class Polyline;
    class PolylineRectangle;
}

namespace nsTecto
{
    // to avoid problem with a lot of continious line segments with same x.
    static const double magic_irrational_number = 3.14159265358979 * 0.001;

    struct DistanceToPolylineRectangle
    {
        double distance;
        const PolylineRectangle* pl;
    };


    // class for 2D vector/point. The access to x and y are public.
    class R2
    {
    public:
        R2()
        {
        }

        R2(double x_, double y_) :
            x(x_), y(y_)
        {
        }

        bool operator==(const R2& p) const
        {
            return (x == p.x) && (y == p.y);
        }

        bool operator!=(const R2& p) const
        {
            return (x != p.x) || (y != p.y);
        }

        // lexicographic order comparing.
        bool operator<(const R2& p) const
        {
            if (x == p.x)
                return (y < p.y);
            else
                return (x < p.x);
        }

        R2 operator-(const R2& p) const
        {
            return R2{ x - p.x, y - p.y };
        }

        R2 operator-() const
        {
            return R2(-x, -y);
        }

        // squared distance between 2 points.
        static double distance2(const R2& r1, const R2& r2)
        {
            double dx = r1.x - r2.x;
            double dy = r1.y - r2.y;
            return dx * dx + dy * dy;
        }

        // squared distance to line segment. minP is the point on line segment [s,e] 
        // providing the minimum distance.
        double DistanceToLineSegment(const R2& s, const R2& e, R2& minP) const;

        // squared distance to rectangle. Point must be outside rectangle.
        double DistanceToRectangle(const R2& minV, const R2& maxV) const;
        double DistanceToRectangle(const Rectangle& rect) const;

        // detect a intersection of a vertical line with abscissa = x  and a line segnemt.
        // return 1 if the axe and the segment intersects, 0 otherwise. In case of intersection y is the ordinate of the intersection.
        static int VerticalLineAndLineSegmentIntersection(double x, const R2& s, const R2& e, double& y);

        // compute squared distance to a polyline. minP is the point on the polyline 
        // providing the minimum distance.
        double DistanceToPolyline(const R2* vertices, int cnt, R2& minP) const;
        double DistanceToPolyline(const Polyline& vertices, R2& minP) const;


        // Search the point on the polyline in mashed form so the squared distance to the point < esp. 
        // If there is no such point, eps is returned, otherwise squared distance value is returned and
        // minP is point on polyline providing the this minmum distance.
        double DistanceToMashedPolylines(std::vector<DistanceToPolylineRectangle>& pl, double eps, R2& minP) const;

        // dot product of two vectors.
        static double dot(const nsTecto::R2& a, const nsTecto::R2& b)
        {
            return a.dot(b);
        }
        double dot(const nsTecto::R2& b) const;
        double length2() const;


        // find coefficient of decompositon of this by basis {e0,e1}
        // so *this = e0*k0 + e1*k1. Return false, if e0 and e1 are colinear.
        bool coeff(const R2& e0, const R2& e1, double& k0, double& k1) const;


    public:
        double x, y;
    };

    // class for rectangle. The access to s and e are public.
    class Rectangle
    {
    public:

        Rectangle() {}
        Rectangle(const R2& s_, const R2& e_) : s(s_), e(e_)
        {
        }

        void Reset() { s.x = s.y = e.x = e.y = 0. ; }
        // compute the area of a rectangle.
        double Area() const
        {
            return (e.x - s.x) * (e.y - s.y);
        }

        // check if two rectangles are overlapped.
        bool IsOverlapped(const Rectangle& rect) const;

        // Compute the distance between two non-overlappend rectangles.
        // Return squared distance between rectangles.
        double DistanceTo(const Rectangle& rect) const;

        // Compupte the rectangle containing both given rectangle. 
        static Rectangle BoundRectangle(const Rectangle& a, const Rectangle& b);
        // Compupte the rectangle containing both given rectangle and given point. 
        static Rectangle BoundRectangle(const Rectangle& a, const R2& r);
    public:
        R2 s, e; // left-upper and right-bottom corner points.
    };
}
