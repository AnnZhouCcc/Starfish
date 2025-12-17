#ifndef connection_matrix_eval
#define connection_matrix_eval

#include "main.h"
#include "../tcp.h"
#include "topology.h"
#include "../randomqueue.h"
#include "../eventlist.h"
#include <random>
#include <list>
#include <set>
#include <map>

struct connection{
  int src, dst;
};

struct Flow{
  int src, dst, bytes, interval;
  double start_time_ms;
  Flow (int _src, int _dst, int _bytes, double _start_time_ms):
    src(_src), dst(_dst), bytes(_bytes), start_time_ms(_start_time_ms), interval(0) {}
  Flow (int _src, int _dst, int _bytes, double _start_time_ms, int _interval):
    src(_src), dst(_dst), bytes(_bytes), start_time_ms(_start_time_ms), interval(_interval) {}
};

// bool compareFlow(Flow f1, Flow f2) {
//     return (f1.start_time_ms < f2.start_time_ms);
// }

class ConnectionMatrixEval{
 public:
  ConnectionMatrixEval();
  vector<Flow> flows;

  void setTopoFlowsEval(string cmfile);
};

#endif
