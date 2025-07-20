#include "Lattice2D.h"

cLattice2D::cLattice2D(const nsTecto::R2& origin_, const nsTecto::R2& e0_, const nsTecto::R2& e1_)
    :origin(origin_), e0(e0_), e1(e1_)
{
    double e0e0 = nsTecto::R2::dot(e0, e0);
    double e0e1 = nsTecto::R2::dot(e0, e1);
    double e1e1 = nsTecto::R2::dot(e1, e1);

    // quadratic form coefficients
    d1 = e1e1;
    l01 = e0e1 / d1;
    d0 = e0e0 - d1 * l01 * l01;
}

double cLattice2D::DistanceToNode(const int i0, int i1, const nsTecto::R2& r) const
{
    double ddx = origin.x + i0 * e0.x + i1 * e1.x - r.x;
    double ddy = origin.y + i0 * e0.y + i1 * e1.y - r.y;
    return ddx * ddx + ddy * ddy;

    nsTecto::R2 dr{ origin.x - r.x, origin.y - r.y }, r0;

    if (dr.coeff(e0, e1, r0.x, r0.y) == false)
        return DBL_MAX;

    auto sq = [](double x) { return x * x; };

    return d0 * sq(i0 + r0.x) + d1 * sq(l01 * (i0 + r0.x) + (i1 + r0.y));
}

bool cLattice2D::NearestNode(const nsTecto::R2& r, int& i0, int& i1) const
{
    nsTecto::R2 dr{ origin.x - r.x, origin.y - r.y }, r0;

    if (dr.coeff(e0, e1, r0.x, r0.y) == false)
        return false;

    auto sq = [](double x) { return x * x; };

    double x = r0.x;
    int i = i0 = lround(-x); //current x component of min arguments;
    double y = r0.y + l01 * (i + r0.x);
    int j = i1 = lround(-y); //current y component of min arguments;
    double dmin = d0 * sq(i + x) + d1 * sq(j + y);

    for (SpiralwiseIterator ik(x + i);; ++ik)
    {
        double x0 = r0.x + i + *ik;
        double f0 = d0 * sq(x0);

        if (f0 >= dmin)
            break;

        double y = r0.y + l01 * x0;
        int j = lround(-y);
        double y0 = y + j;
        double f1 = f0 + d1 * sq(y0);

        if (f1 < dmin)
        {
            i0 = i + *ik;
            i1 = j;
            dmin = f1;
        }
    }

    return true;
}

void cLattice2D::ToLatticeCoordinates(const nsTecto::R2& r, nsTecto::R2& lt, int& i0, int& i1)
{
    nsTecto::R2 dr{ r.x - origin.x, r.y - origin.y };

    dr.coeff(e0, e1, lt.x, lt.y);

    NearestNode(r, i0, i1);
}

void cLattice2D::FromLatticeCoordinates(const nsTecto::R2& lt, nsTecto::R2& r)
{
    r.x = origin.x + lt.x * e0.x + lt.y * e1.x;
    r.y = origin.y + lt.x * e0.y + lt.y * e1.y;
}