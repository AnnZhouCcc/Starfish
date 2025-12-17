#ifndef TOPOLOGY
#define TOPOLOGY
#include <fstream>
#include "../network.h"
#include "../queue.h"
#include "../randomqueue.h"
#include "../ecnqueue.h"
#include "main.h"

#ifndef __QUEUE_TYPE__
#define __QUEUE_TYPE__ 
typedef enum {RANDOM, ECN, COMPOSITE, CTRL_PRIO, LOSSLESS, LOSSLESS_INPUT, LOSSLESS_INPUT_ECN} queue_type;
#endif

class Topology {
 public:
  queue_type qtype;
  Queue* alloc_queue(QueueLogger* queueLogger, uint64_t speed_pktps, mem_b queuesize){
    if (qtype==RANDOM) return new RandomQueue(speedFromPktps(speed_pktps), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
    else if (qtype==ECN)
        return new ECNQueue(speedFromPktps(speed_pktps), memFromPkt(2*SWITCH_BUFFER), *eventlist, queueLogger, memFromPkt(15));
    assert(0);
  }
  EventList* eventlist;
  virtual pair<vector<double>*, vector<route_t*>*> get_paths(int src,int dest)=0;
  virtual pair<vector<double>*, vector<route_t*>*> get_other_paths(int src,int dest)=0;
  virtual int get_distance(int src,int dest){
	std::cout<<"get_distance not implemented (Topology.h)"<<endl;
	exit(0);
	return -1;
  }
  virtual pair<vector<int>, vector<int> > getcsRacks(int clients, int servers){
	std::cout<<"getcsracks not implemented (Topology.h)"<<endl;
	exit(0);
	pair<vector<int>, vector<int> > ret;
	return ret;
  }
  virtual vector<int>* get_neighbours(int src) = 0;  
  virtual int ConvertHostToRack(int host){
	std::cout<<"ConvertHostToRack not implemented (Topology.h)"<<endl;
	exit(0);
	return -1;
  }
  void printServerDistance(string distfile, int N){
	ofstream file;
	file.open(distfile);
	for(int i=0; i<N; i++){
		for(int j=0; j<N; j++){
			if(i!=j) 
				file<<i<<" "<<j<<" "<<get_distance(i, j) + 2<<"\n";
		}
	}
	file.close();
  }
  virtual void fail_links(int fails, double loss_rate){
	std::cout<<"fail_links not implemented (Topology.h)"<<endl;
	exit(0);
  }
  virtual void localize_fails(vector<pair<TcpSrc*, pair<int, int> > >& tcpSrcs){
	std::cout<<"localize_fails not implemented (Topology.h)"<<endl;
  }
  virtual void findDisconnectedPairsOnSwitchFailure(){
	std::cout<<"findDisconnectedPairsOnSwitchFailure Not Implemented (Topology.h)"<<endl;
	exit(0);
  }
  virtual void findDisconnectedPairsOnLinkFailure(){
	std::cout<<"findDisconnectedPairsOnLinkFailure Not Implemented (Topology.h)"<<endl;
	exit(0);
  }
  virtual void findVarianceCoefficients(){
	std::cout<<"findVarianceCoefficients Not Implemented (Topology.h)"<<endl;
	exit(0);
  }
  virtual void print_link_bandwidth_usage(double simTimeInSec){};
 
  vector<route_t *>***net_paths_rack_based;
  vector<route_t *>***ecmp_net_paths;
  vector<route_t *>***su2_net_paths;
  vector<route_t *>***su3_net_paths;
  vector<route_t *>***maxdisj_net_paths;
  vector<int> net_paths_choice;
  vector< pair<int,double> >**** path_weights_rack_based;
  virtual void delete_net_paths_rack_based(int numintervals){
	std::cout<<"delete_net_paths_rack_based not implemented (topology.h)"<<endl;
	exit(0);
  }
  virtual route_t *attach_head_tail(int src, int dst, bool is_same_switch, int rand_choice){
	std::cout<<"attach_head_tail not implemented (topology.h)"<<endl;
	exit(0);
  }
};

#endif
