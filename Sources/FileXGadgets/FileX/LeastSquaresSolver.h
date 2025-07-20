#pragma once

#include <vector>
#include "asa007.hpp"

class LeastSquaresSolver
{
public:
    LeastSquaresSolver(int numParam);

    void addMeasurement(double meas, const double* partials);

    bool solve();

    const std::vector<double>& GetSolution() const
    {
        return solution;
    }

protected:

    double GetHtH(int i, int j) const
    {
        return asa007::sym(&HtH[0], i, j);
    }

    double& GetHtH(int i, int j)
    {
        return asa007::sym(&HtH[0], i, j);;
    }

protected:
    int numMeas;
    std::vector<double> HtH, Htz, solution;
};

