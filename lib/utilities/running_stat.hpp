#include <math.h>
#include <stdio.h>
#include <iostream>
using namespace std;

class running_stat {
  public:
    double ss;           // (running) sum of square deviations from mean
    double m;            // (running) mean
    // double last_m;
    double n;
    // unsigned int n;      // number of items seen
    double totalW;          // weight of items seen
	string word;
    bool is_started; 		//internal variable 
	double probablity_non_zero;

    running_stat() : ss(0), m(0), n(0), totalW(0), is_started(false),probablity_non_zero(0.0) {}

    void add(double x) {
      add(x,1);
    }

    void add(double x, double w) {
      n++;
      if (!is_started) {
        m = x;
        ss = 0;
        totalW = w;
        is_started = true;
      } else {
        float tmpW = totalW + w;
        ss += totalW*w * (x-m)*(x-m) / tmpW;
        m += (x-m)*w / tmpW;
        totalW = tmpW;
      }
    }

    double var() const { return ss / totalW; }
    double sd() const { return sqrt(var()); }
    double mean() const { return m; }
};

/*
ostream& operator<<(ostream& output, const running_stat& rs) {
  // output << "mean=" << rs.mean() << "  sd=" << rs.sd() << "  n="<<rs.n;
  output << "mean=" << rs.mean() << "  sd=" << rs.sd() << "  n="<<rs.n << "  ss=" << rs.ss;
  return output;  // for multiple << operators.
}
*/

