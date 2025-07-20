#pragma once

void cholesky(double a[], int n, int nn, double u[], int* nullty,
    int* ifault);
void syminv(double a[], int n, double c[], double w[], int* nullty,
    int* ifault);

namespace asa007
{
    // look at comment of syminv function
    inline double& sym(double a[], int row, int col)
    {
        return a[row * (row + 1) / 2 + col];
    }

    inline double sym(const double a[], int row, int col)
    {
        return a[row * (row + 1) / 2 + col];
    }

    inline bool inverse(double a[], int n, double w[])
    {
        int nullty, ifault;
        syminv(a, n, a, w, &nullty, &ifault);
        return nullty == 0 && ifault == 0;
    }

}

