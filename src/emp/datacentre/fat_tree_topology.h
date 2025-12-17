#ifndef FAT_TREE
#define FAT_TREE
#include "main.h"
#include "randomqueue.h"
#include "pipe.h"
#include "config.h"
#include "loggers.h"
#include "network.h"
#include "topology.h"
#include "logfile.h"
#include "eventlist.h"
#include <ostream>

#define NK (K*K/2)
#define NC (K*K/4)
#define NSRV (OVERSUBSCRIPTION*K*K*K/4)
//#define N K*K*K/4

#define FULL_FAT_TREE_SVR(src) (src/OVERSUBSCRIPTION)

#define HOST_POD_SWITCH(src) (2*FULL_FAT_TREE_SVR(src)/K)
//#define HOST_POD_ID(src) src%NSRV
#define HOST_POD(src) (FULL_FAT_TREE_SVR(src)/NC)

#define MIN_POD_ID(pod_id) (pod_id*K/2)
#define MAX_POD_ID(pod_id) ((pod_id+1)*K/2-1)

class FatTreeTopology: public Topology{
 public:
  Pipe * pipes_nc_nup[NC][NK];
  Pipe * pipes_nup_nlp[NK][NK];
  Pipe * pipes_nlp_ns[NK][NSRV];
  Queue * queues_nc_nup[NC][NK];
  Queue * queues_nup_nlp[NK][NK];
  Queue * queues_nlp_ns[NK][NSRV];

  Pipe * pipes_nup_nc[NK][NC];
  Pipe * pipes_nlp_nup[NK][NK];
  Pipe * pipes_ns_nlp[NSRV][NK];
  Queue * queues_nup_nc[NK][NC];
  Queue * queues_nlp_nup[NK][NK];
  Queue * queues_ns_nlp[NSRV][NK];

  Logfile* logfile;
  
  FatTreeTopology(Logfile* log,EventList* ev, queue_type qt);

  void init_network();
  virtual pair<vector<double>*, vector<route_t*>*> get_paths(int src, int dest);
  virtual pair<vector<double>*, vector<route_t*>*> get_other_paths(int src, int dest);
  virtual int get_distance(int src, int dest);
  
  void count_queue(Queue*);
  void print_path(std::ostream& paths,int src,route_t* route);
  vector<int>* get_neighbours(int src) { return NULL;};  
  int ConvertHostToRack(int host) { return HOST_POD_SWITCH(host);};
 private:
  map<Queue*,int> _link_usage;
  int find_lp_switch(Queue* queue);
  int find_up_switch(Queue* queue);
  int find_core_switch(Queue* queue);
  int find_destination(Queue* queue);
};

#endif
