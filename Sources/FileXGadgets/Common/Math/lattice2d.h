#pragma once

#include "R2.h"

struct SpiralwiseIterator
{
public:
    explicit SpiralwiseIterator(double offset) :
        dir((offset < 0) ? LEFT : RIGHT),
        i(0)
    {
    }

    SpiralwiseIterator& operator++()
    {
        i = -i;
        if (dir == LEFT)
        {
            if (i <= 0)
                --i;
        }
        else
        {
            if (i >= 0)
                ++i;
        }

        return *this;
    }

    int operator *() const
    {
        return i;
    }

protected:
    enum eDir { LEFT = -1, RIGHT = 1 } dir;
    int i;
};

class cLattice2D
{
public:
    typedef ::SpiralwiseIterator SpiralwiseIterator;

protected:
    ns::R2 origin, e0, e1;
    double d0, l01, d1; // quadratic form coefficients

public:
    cLattice2D(const ns::R2& origin_, const ns::R2& e0_, const ns::R2& e1_);

    cLattice2D() 
    {
        origin = e0 = e1 = ns::R2{0,0};
        d0 = l01 = d1 = 0;
    }

    const ns::R2& Origin() const { return origin; }
    const ns::R2& E0() const { return e0; }
    const ns::R2& E1() const { return e1; }

    ns::R2 node(int i0, int i1)
    {
        return ns::R2{
            origin.x + e0.x * i0 + e1.x * i1,
            origin.y + e0.y * i0 + e1.y * i1 };
    }

    double DistanceToNode(int i0, int i1, const ns::R2& r) const;

    bool NearestNode(const ns::R2& r, int &i0, int& i1) const;

    void ToLatticeCoordinates(const ns::R2& r, ns::R2& lt, int& i0, int& i1);
    void FromLatticeCoordinates(const ns::R2& lt, ns::R2& r);
};
