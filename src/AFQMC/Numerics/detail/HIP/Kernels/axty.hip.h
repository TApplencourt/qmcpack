///////////////////////////////////////////////////////////////////////////////
// This file is distributed under the University of Illinois/NCSA Open Source
// License.  See LICENSE file in top directory for details.
//
// Copyright (c) 2020 QMCPACK developers.
//
// File developed by: Fionn Malone, malone14@llnl.gov, LLNL
//
// File created by: Fionn Malone, malone14@llnl.gov, LLNL
////////////////////////////////////////////////////////////////////////////////

#ifndef AFQMC_AXTY_KERNELS_HPP
#define AFQMC_AXTY_KERNELS_HPP

#include<cassert>
#include <complex>

namespace kernels
{

void axty(int n, float alpha, float const* x, float *y);
void axty(int n, double alpha, double const* x, double *y);
void axty(int n, std::complex<float> alpha,
                 std::complex<float> const* x, std::complex<float> *y);
void axty(int n, std::complex<double> alpha,
                 std::complex<double> const* x, std::complex<double> *y);

}

#endif
