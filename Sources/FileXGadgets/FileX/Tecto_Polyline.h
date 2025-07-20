#pragma once

#include "Tecto_R2.h"

namespace nsTecto
{
    // class to keep information about part of the polyline ( mash ).
    class PolylineRectangle
    {
    public:

        PolylineRectangle()
        {
        }

        PolylineRectangle( int startIndex_ , int endIndex_ , const std::vector<R2>& vertices_);

    public:
        Rectangle boundary; // extern boundary
        int startIndex, endIndex; // start and end ( not included ) indices of vertices.
        const std::vector<R2>* vertices;
    };


    // class to keep distance between two polylines.
    class PolylinesDistance
    {
    public:
        R2 r0, r1; // points on first and second polyline providing the minimal distance between them.
        double distance; // squared distance between above points.
    };


    class Polyline : protected std::vector<R2>
    {
        friend class R2;

    protected:
        typedef std::vector<R2> base_container;

    public:

        Polyline() {}
        Polyline(const std::vector<R2>& v) : std::vector<R2>(v) {}

        using base_container::push_back;
        using base_container::size;

        const base_container& base() const
        {
            return *this;
        }

        const R2& operator[](int idx) const
        {
            return std::vector<R2> ::operator[](idx);
        }

        const Rectangle& BoundRectangle() const
        {
            return boundRectangle;
        }

        const std::vector<PolylineRectangle>& MashedPolylines() const
        {
            return mashedPolylines;
        }

        // scale along x axe
        void scaleX(double scale)
        {
            for (base_container::iterator it = begin(); it != end(); ++it)
                it->x *= scale;
        }

        // rescale along x axe
        void rescaleX(double scale)
        {
            for (base_container::iterator it = begin(); it != end(); ++it)
                it->x /= scale;
        }

        // duplicate the start of polyline to the end. This allows to use
        // p[size()-2], p[size()-1] instead of p[size()-1 ,p[0]. The first one 
        // don't need index rollover checking.
        void ConvertPolygonToPolyline()
        {
            base_container::push_back(base_container::operator[](0));
        }

        // find the outer boundary of the polyline 
        void FindOuterRectangle();

        // the area of polygon defined by polyline. 
        double Area() const;

        // the perimeter of the polyline. 
        double Perimeter() const;
        
        // mash the polyline. The polyline is splitted to smaller polylines with a count 
        // of segments <= SegmentCount
        void Mash(int SegmentCount);

        // Compute distance to polyline. distMin is initial value for the squared distance.
        // if the distance between polylines >= distMin distMin is returned otherwise squared 
        // minimal distance between polylines is returned. In this case p0 and p1 are points on
        // this and p providing such minimial distance.
        double DistanceTo(const Polyline& p, R2& p0, R2& p1, double distMin = DBL_MAX) const;

        // Compute relation state between two polylines.
        // The polylines are related if squared distance between them is less than given value eps.
        // If they are related the informaiton about relation is stored to relation.
        bool IsRelated(const Polyline& base, double eps, PolylinesDistance& relation) const;

        // some clearing functions.
        // remove vertices at the end duplicated first one.
        void RemoveDuplicateVertices();
        //shift polyline poinst so first vertex is first in lexicographic order of vertices 
        void SetStartingPoint();
        //compare two polylines. There is not any tolerance in comparing vertices. 
        bool IsEqualTo(const Polyline& p) const;

        // count the intersection between vertical axe with abscissa x and the polyline.
        // return count of intersections. Span of intersection is stored int ymin,ymax
        int VerticalAxeIntersection(double x, double& ymin, double& ymax) const;

    protected:
        // Compute distance to polyline (brute-forces) used for testing. p0 and p1 are points on
        // this and p providing such minimial distance.
        double TDistanceTo(const Polyline& p, R2& p0, R2& p1) const;

        // Compute distance to mashed polyline. distMin is initial value for the squared distance.
        // if the distance between polylines >= distMin distMin is returned otherwise squared 
        // minimal distance between polylines is returned. In this case p0 and p1 are points on
        // this and p providing such minimial distance.
        double DistanceTo(std::vector<DistanceToPolylineRectangle>& wd, R2& p0, R2& p1, double distMin) const;

    protected:

        Rectangle boundRectangle; // bound rectange
        std::vector<PolylineRectangle> mashedPolylines; //splitted polyline 
        mutable std::vector<DistanceToPolylineRectangle> workDistances; // temporary variable used to sort distances.

    };

}



