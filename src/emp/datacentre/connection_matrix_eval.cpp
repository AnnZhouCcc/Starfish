#include "connection_matrix_eval.h"
#include "leaf_spine_topology.h"
#include <string.h>
#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <fstream>
#include "../tcppacket.h"
#include <bits/stdc++.h>
// #include <stdc++.h>
#include <math.h>
#include <random>
#include <iostream>
#include <set>
#include <queue>
#include <map>

int large_flow_threshold = 10 * 1024 * 1024;

ConnectionMatrixEval::ConnectionMatrixEval() {}

void ConnectionMatrixEval::setTopoFlowsEval(string cmfile) {
  ifstream TMFile(cmfile.c_str());
  string line;
  line.clear();
  if (TMFile.is_open()){
    while(TMFile.good()){
      getline(TMFile, line);
      //Whitespace line
      if (line.find_first_not_of(' ') == string::npos) break;
      stringstream ss(line);
      string token;
      vector<string> tokens;
      while (getline(ss,token,',')) {
        tokens.push_back(token);
      }
      int fromserver = stoi(tokens[0]);
      int toserver = stoi(tokens[1]);
      uint64_t bytes = stoi(tokens[2]);
      double start_time_ms = stod(tokens[3]);
      flows.push_back(Flow(fromserver, toserver, bytes, start_time_ms));
    }
    TMFile.close();
  } else {
    cout << "***Error opening connection matrix file: " << cmfile << endl;
    exit(1);
  }
}