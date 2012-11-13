#ifndef NALL_INTERPOLATION_HPP
#define NALL_INTERPOLATION_HPP

namespace nall {

struct Interpolation {
  static inline double Nearest(double mu, double a, double b, double c, double d) {
    return (mu <= 0.5 ? b : c);
  }

  static inline double Sublinear(double mu, double a, double b, double c, double d) {
    mu = ((mu - 0.5) * 2.0) + 0.5;
    if(mu < 0) mu = 0;
    if(mu > 1) mu = 1;
    return b * (1.0 - mu) + c * mu;
  }

  static inline double Linear(double mu, double a, double b, double c, double d) {
    return b * (1.0 - mu) + c * mu;
  }

  static inline double Cosine(double mu, double a, double b, double c, double d) {
    mu = (1.0 - cos(mu * 3.14159265)) / 2.0;
    return b * (1.0 - mu) + c * mu;
  }

  static inline double Cubic(double mu, double a, double b, double c, double d) {
    double A = d - c - a + b;
    double B = a - b - A;
    double C = c - a;
    double D = b;
    return A * (mu * mu * mu) + B * (mu * mu) + C * mu + D;
  }

  static inline double Hermite(double mu1, double a, double b, double c, double d) {
    const double tension = 0.0;  //-1 = low, 0 = normal, +1 = high
    const double bias = 0.0;  //-1 = left, 0 = even, +1 = right
    double mu2, mu3, m0, m1, a0, a1, a2, a3;

    mu2 = mu1 * mu1;
    mu3 = mu2 * mu1;

    m0  = (b - a) * (1.0 + bias) * (1.0 - tension) / 2.0;
    m0 += (c - b) * (1.0 - bias) * (1.0 - tension) / 2.0;
    m1  = (c - b) * (1.0 + bias) * (1.0 - tension) / 2.0;
    m1 += (d - c) * (1.0 - bias) * (1.0 - tension) / 2.0;

    a0 = +2 * mu3 - 3 * mu2 + 1;
    a1 =      mu3 - 2 * mu2 + mu1;
    a2 =      mu3 -     mu2;
    a3 = -2 * mu3 + 3 * mu2;

    return (a0 * b) + (a1 * m0) + (a2 * m1) + (a3 * c);
  }
};

}

#endif
