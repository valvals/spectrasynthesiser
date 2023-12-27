//---------------------------------------------------------------------------
#include "CubSpline.h"
#include <math.h>


//---------------------------------------------------------------------------
CubSpline::CubSpline(double* x, double* y, int N) {
  double* a1 = new double[N];
  double* a2 = new double[N];
  double* a3 = new double[N];
  double* b = new double[N];
  double* d = new double[N];

  int i;
  int tblsize;
  double delta;
  double delta2;
  double delta3;

  a1[0] = 0;
  a2[0] = 1;
  a3[0] = 0;
  b[0] = 0;

  for (i = 1; i < N - 1; i++) {
    a1[i] = (x[i] - x[i - 1]) / 6;
    a2[i] = (x[i + 1] - x[i - 1]) / 3;
    a3[i] = (x[i + 1] - x[i]) / 6;
    b[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
  }

  a1[N - 1] = 0;
  a2[N - 1] = 1;
  a3[N - 1] = 0;
  b[N - 1] = 0;

  solvetridiagonal(a1, a2, a3, b, N, d);

  tblsize = 3 + N - 1 + (N - 1) * 4;
  c = new double[tblsize];
  c[0] = tblsize;
  c[1] = 0;
  c[2] = N;
  for (i = 0; i < N - 1; i++) {
    c[3 + i] = x[i];
  }
  for (i = 0; i < N - 1; i++) {
    delta = x[i + 1] - x[i];
    delta2 = delta * delta;
    delta3 = delta * delta2;
    c[3 + N - 1 + 4 * i + 0] = y[i];
    c[3 + N - 1 + 4 * i + 1] = (y[i + 1] - y[i] - delta2 * (d[i] / 3 + d[i + 1] / 6)) / delta;
    c[3 + N - 1 + 4 * i + 2] = delta2 * d[i] / 2 / delta2;
    c[3 + N - 1 + 4 * i + 3] = delta2 / 6 * (d[i + 1] - d[i]) / delta3;
  }
  delete []a1;
  delete []a2;
  delete []a3;
  delete []b;
  delete []d;
}

double CubSpline::GetY(double x) {
  double result;
  int n;
  int l;
  int r;
  int m;

  n = static_cast<int>(c[2]);

  //
  // Binary search
  //
  l = 3;
  r = 3 + n - 2 + 1;
  while (l != r - 1) {
    m = (l + r) / 2;
    if (c[m] >= x) {
      r = m;
    } else {
      l = m;
    }
  }

  //
  // Interpolation
  //
  x = x - c[l];
  m = 3 + n - 1 + 4 * (l - 3);
  result = c[m] + x * (c[m + 1] + x * (c[m + 2] + x * c[m + 3]));
//  if (result < 0) result = 0;
  return result;

}



CubSpline::~CubSpline() {
  if (c != nullptr)
    delete [] c;
}

void CubSpline::solvetridiagonal(double* a, double* b, double* c, double* d, int n, double* x) {
  int k;
  double t;
  a[0] = 0;
  c[n - 1] = 0;
  for (k = 1; k < n; k++) {
    t = a[k] / b[k - 1];
    b[k] = b[k] - t * c[k - 1];
    d[k] = d[k] - t * d[k - 1];
  }

  x[n - 1] = d[n - 1] / b[n - 1];
  for (k = n - 2; k >= 0; k--) {
    x[k] = (d[k] - c[k] * x[k + 1]) / b[k];
  }
}

void LeastSquares(double* x, double* y, int n, double& a, double& b) {
  double Mx = 0;
  double Mx2 = 0;
  double My = 0;
  double Mxy = 0;
  for (int i = 0; i < n; i++) {
    Mx += x[i];
    Mx2 += x[i] * x[i];
    if (y[i] != 0) {
      My += (1 / y[i]);
      Mxy += (x[i] / y[i]);
    }
  }
  Mx /= n;
  My /= n;
  Mxy /= n;
  Mx2 /= n;
  if (Mx2 - Mx * Mx == 0) {
    a = 0;
    b = 0;
  } else {
    a = (-Mx * Mxy + Mx2 * My) / (Mx2 - Mx * Mx);
    b = (Mxy - Mx * My) / (Mx2 - Mx * Mx);
  }

}

double GetLSPoint(double* x, double* y, int n, double exp) {
  double a, b;
  LeastSquares(x, y, n, a, b);
  return  1 / (exp * b + a);
}

#pragma package(smart_init)
