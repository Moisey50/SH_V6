#include "LeastSquaresSolver.h"
#include "asa007.hpp"

LeastSquaresSolver::LeastSquaresSolver(int numParam)
    : numMeas(0), HtH(numParam* (numParam + 1) / 2, 0), Htz(numParam, 0), solution(numParam)
{
    // Htz.size is count of parameters.
}

void LeastSquaresSolver::addMeasurement(double meas, const double* partials)
{
    int size = int(Htz.size());
    for (int i = 0; i < size; ++i)
    {
        Htz[i] += meas * partials[i];
        // HtH is symmetrix, so update only half of it
        for (int j = 0; j <= i; ++j)
            GetHtH(i, j) += partials[i] * partials[j];
    }

    ++numMeas;
}


bool LeastSquaresSolver::solve()
{
    int size = int(Htz.size());
    if (numMeas < size)
    {
        solution.resize(0);
        return false;
    }

    // solution is used as temporary data storage.
    solution.resize(Htz.size());
    if (false == asa007::inverse(&HtH[0], size, &solution[0]))
        return false;

    for (int i = 0; i < size; ++i)
    {
        solution[i] = 0;
        for (int j = 0; j <= i; ++j)
            solution[i] += GetHtH(i, j) * Htz[j];
        for (int j = i + 1; j < size; ++j)
            solution[i] += GetHtH(j, i) * Htz[j];
    }

    return true;
}
