#include "../config.h"
#include <sstream>
#include <strstream>
#include <iostream>
#include <string.h>
#include <list>
#include <math.h>
#include <fstream>
#include "../network.h"
#include "../randomqueue.h"
#include "../pipe.h"
#include "../eventlist.h"
#include "../logfile.h"
#include "../loggers.h"
#include "../clock.h"
#include "../tcp.h"
#include "../dctcp.h"
#include "topology.h"
#include "connection_matrix_eval.h"

#include "main.h"

#include "leaf_spine_topology.h"
#include "rand_regular_topology.h"
#include "dragon_fly_topology.h"

// Simulation params
#define PERIODIC 0
#define DEBUG_MODE false

uint32_t RTT = 2; // us // AnnC: not really RTT, more like link latency
int ssthresh = 43; //65 KB

double warmup_percentage = 0.2;
uint16_t endtime_extension = 50; // ms

unsigned int subflow_count = 1;

string ntoa(double n);
string itoa(uint64_t n);

//#define SWITCH_BUFFER (SERVICE * RTT / 1000)

const double SIMTIME = 10;

EventList eventlist;

Logfile* lg;

uint64_t total_path_lengths = 0;
uint64_t total_available_paths = 0;
uint64_t total_available_first_hops = 0;

void exit_error(char* progr) {
    cout << "Usage " << progr << " [UNCOUPLED(DEFAULT)|COUPLED_INC|FULLY_COUPLED|COUPLED_EPSILON] [epsilon][COUPLED_SCALABLE_TCP" << endl;
    exit(1);
}

int choose_a_path(vector< pair<int,double> >* path_weights, int net_paths_size) {
    // dp means precision
    int num_paths = path_weights->size();

    if (num_paths == 0) {
        //cout << "***Error : main_flowsize.cpp choose_a_path : num_paths=0, src_sw=" << src_sw << ", dst_sw=" << dst_sw << endl;
        //exit(1);
        return rand()%net_paths_size;
    } else {
        double random = rand()/(double)(RAND_MAX);
    
    #if DEBUG_MODE
    cout << "num_paths = " << num_paths << endl;
    cout << "random = " << random << endl;
    std::cout << "net_paths = " << net_paths_size << std::endl;
    #endif

        double sum = 0;
        for (int i=0; i<num_paths; i++) {
            double prev_sum = sum;
            sum += path_weights->at(i).second;

        #if DEBUG_MODE
            cout << path_weights->at(i).first << " " << path_weights->at(i).second << " | " << endl;
        #endif

            if (random<sum && random>=prev_sum) return path_weights->at(i).first;
        }
    }
}


int main(int argc, char **argv) {
    eventlist.setEndtime(timeFromMs(SIMTIME));
    Clock c(timeFromSec(50 / 100.), eventlist);
    double simtime_ms = 1;
    int stime = 0, numfaillinks = 0;
    string serverfile = "none", cmfile = "none", topologyfile = "none", npfile = "none", pwfileprefix = "none", linkfailurefile = "none";
    uint16_t topology_type = 1; // 0 for FAT, 1 for LEAFSPINE, 2 for RRG, 3 for DRAGONFLY
    uint32_t numswitches = 80;	
    uint32_t numhosts = 3072;
    uint16_t os = 1;
    uint32_t ls_k = 64;
    uint32_t numintervals = 1;
    uint32_t df_p = 2, df_a = 3, df_h = 1; // dragonfly params
    int seed=240;
    double os_ratio = 1;
    stringstream filename(ios_base::out);
    filename << "logout.dat";

    int i = 1;
    while (i<argc) {
      if (!strcmp(argv[i],"-o")){
          filename << argv[i+1];
          i++;
      }
      else if (!strcmp(argv[i],"-sub")){
          subflow_count = atoi(argv[i+1]);
          i++;
          cout << "Using subflow count " << subflow_count <<endl;
      }
      else if (!strcmp(argv[i],"-cmfile")){
          cmfile = argv[i+1];
          i++;
          cout << "connection matrix file: " << cmfile << endl; 
      }
      else if (!strcmp(argv[i],"-serverfile")){
          serverfile = argv[i+1];
          i++;
          cout << "serverfile: " << serverfile <<endl;
      }
      else if (!strcmp(argv[i],"-topologyfile")){
          topologyfile = argv[i+1];
          i++;
          cout << "Topology File: " << topologyfile << endl;
      }
      else if (!strcmp(argv[i],"-seed")){
          seed = atoi(argv[i+1]);
          srand(seed);
          i++;
          cout << "Using seed: " << seed << endl;
      }
      else if (!strcmp(argv[i],"-npfile")){
          npfile = argv[i+1];
          i++;
          cout << "Netpath File: " << npfile << endl;
      }
      else if (!strcmp(argv[i],"-pwfileprefix")){
          pwfileprefix = argv[i+1];
          i++;
          cout << "Pathweight File prefix: " << pwfileprefix << endl;
      }
      else if (!strcmp(argv[i],"-stime")){
          stime = atoi(argv[i+1]);
          i++;

          simtime_ms = stime;
        eventlist.measurement_start_ms = stime * warmup_percentage;
        // eventlist.setEndtime(timeFromMs(simtime_ms + endtime_extension));
        eventlist.setEndtime(timeFromMs(simtime_ms * 100));
          cout << "stime = " << stime << endl;
      }
      else if (!strcmp(argv[i],"-numintervals")){
          numintervals = atoi(argv[i+1]);
          i++;
          cout << "numintervals = " << numintervals << endl;
      }
      else if (!strcmp(argv[i],"-numfaillinks")){
          numfaillinks = atoi(argv[i+1]);
          i++;
          cout << "numfaillinks = " << numfaillinks << endl;
      }
      else if (!strcmp(argv[i],"-linkfailurefile")){
          linkfailurefile = argv[i+1];
          i++;
          cout << "link failure file: " << linkfailurefile << endl;
      }
      else if (!strcmp(argv[i],"-topologytype")){
          topology_type = atoi(argv[i+1]);
          i++;
          cout << "topology_type = " << topology_type << endl;
      }
      else if (!strcmp(argv[i], "-numswitches")) {
        numswitches = atoi(argv[i+1]);
        i++;
        cout << "numswitches = " << numswitches << endl;
    }
    else if (!strcmp(argv[i], "-numhosts")) {
        numhosts = atoi(argv[i+1]);
        i ++;
        cout << "numhosts = " << numhosts << endl;
    }
    else if (!strcmp(argv[i], "-os")) {
        os = atoi(argv[i+1]);
        i ++;
        cout << "os = " << os << endl;
    }
    else if (!strcmp(argv[i], "-ls_k")) {
        ls_k = atoi(argv[i+1]);
        i ++;
        cout << "ls_k = " << ls_k << endl;
    }
    else if (!strcmp(argv[i], "-df_p")) {
        df_p = atoi(argv[i+1]);
        i ++;
        cout << "df_p = " << df_p << endl;
    }
    else if (!strcmp(argv[i], "-df_a")) {
        df_a = atoi(argv[i+1]);
        i ++;
        cout << "df_a = " << df_a << endl;
    }
    else if (!strcmp(argv[i], "-df_h")) {
        df_h = atoi(argv[i+1]);
        i ++;
        cout << "df_h = " << df_h << endl;
    }
    else if (!strcmp(argv[i], "-os_ratio")) {
        os_ratio = atof(argv[i+1]);
        i ++;
        cout << "os_ratio = " << os_ratio << endl;
    }
    else {
         exit_error(argv[0]);
    }
    
      i++;
    }
    //srand(time(NULL));

    // prepare the loggers
    cout << "Logging to " << filename.str() << endl;
    //Logfile 
    Logfile logfile(filename.str(), eventlist);

    lg = &logfile;

    logfile.setStartTime(timeFromSec(0));

    int logger_period_ms = 1000000;
    TcpSinkLoggerSampling sinkLogger = TcpSinkLoggerSampling(timeFromMs(logger_period_ms), eventlist);
    logfile.addLogger(sinkLogger);

    TcpLoggerSimple logTcp;logfile.addLogger(logTcp);

    TcpSrc* tcpSrc;
    TcpSink* tcpSnk;
    route_t* routeout, *routein;
    TcpRtxTimerScanner tcpRtxScanner(timeFromMs(10), eventlist);


    Topology* top = nullptr;
    if (topology_type == FAT) {
        // top = new FatTreeTopology(&logfile, &eventlist, RANDOM);
    } else if (topology_type == LEAFSPINE) {
        top = new LeafSpineTopology(&logfile, &eventlist, RANDOM, numfaillinks, linkfailurefile, npfile, pwfileprefix, numswitches, ls_k, os, os_ratio);
    } else if (topology_type == RRG) {
        top = new RandRegularTopology(&logfile, &eventlist, topologyfile, RANDOM, numfaillinks, linkfailurefile, npfile, pwfileprefix, numintervals, serverfile, numswitches, numhosts, os, ls_k, os_ratio);
    } else if (topology_type == DRAGONFLY) {
        top = new DragonFlyTopology(df_p, df_a, df_h, &logfile, &eventlist, RANDOM, npfile, pwfileprefix, numintervals);
    } else {
        cout << "Unknown topology type: " << topology_type << endl;
        exit(1);
    }


    ConnectionMatrixEval* conns = new ConnectionMatrixEval();
    conns->setTopoFlowsEval(cmfile);
    
    cout << "Starting to produce flow paths" << endl;
    typedef pair<int, int> PII;
    // int flowID = 0, whichinterval=0, numintervals=1;
    int flowID = 0, whichinterval=0;
    int src_sw, dst_sw, num_paths_srcsw_dstsw, num_paths_dstsw_srcsw;

    vector<route_t*>*** net_paths = top->net_paths_rack_based;


    for (Flow& flow: conns->flows){
        flowID++;

        src_sw = top->ConvertHostToRack(flow.src);
		dst_sw = top->ConvertHostToRack(flow.dst);

        num_paths_srcsw_dstsw = net_paths[src_sw][dst_sw]->size();
        num_paths_dstsw_srcsw = net_paths[dst_sw][src_sw]->size();

        assert (subflow_count == 1);
        tcpSrc = new TcpSrc(NULL, NULL, eventlist);
        tcpSnk = new TcpSink();
        tcpSrc->set_ssthresh(ssthresh*Packet::data_packet_size());
        tcpSrc->set_flowsize(flow.bytes);

    #if DEBUG_MODE
        cout << "before randomly taking paths: flowID = " << flowID << ", src_sw=" << src_sw << ",dst_sw=" << dst_sw << endl;
    #endif

        if (src_sw == dst_sw) {
		    routeout = top->attach_head_tail(flow.src, flow.dst, true, 0);
            routein = top->attach_head_tail(flow.dst, flow.src, true, 0);
	    } else {
            int choice = choose_a_path(top->path_weights_rack_based[whichinterval][src_sw][dst_sw], net_paths[src_sw][dst_sw]->size());
            int rchoice = choose_a_path(top->path_weights_rack_based[whichinterval][dst_sw][src_sw], net_paths[dst_sw][src_sw]->size());

            if (choice>=num_paths_srcsw_dstsw){
                cout << "***Weird path choice " << choice << " out of " << num_paths_srcsw_dstsw << ", src_sw=" << src_sw << ", dst_sw=" << dst_sw << endl;
                exit(1);
            }
            if (rchoice>=num_paths_dstsw_srcsw){
                cout << "***Weird path rchoice " << rchoice << " out of " << num_paths_dstsw_srcsw << ", src_sw=" << src_sw << ", dst_sw=" << dst_sw << endl;
                exit(1);
            }

            routeout = top->attach_head_tail(flow.src, flow.dst, false, choice);
            routein = top->attach_head_tail(flow.dst, flow.src, false, rchoice);

        #if DEBUG_MODE
            cout << "**debug info** routeout: " << endl;
            cout << routeout->size() << endl;
            for (unsigned int i=0; i<routeout->size(); i++) {
                cout << routeout->at(i) << endl;
                cout << routeout->at(i)->nodename() << endl;
            }
            cout << "**debug info** routein: size=" << routein->size() << endl;
            for (unsigned int i=0; i<routein->size(); i++) {
                cout << routein->at(i)->nodename() << endl;
            }
        #endif

	    }

        total_path_lengths += (routeout->size()-5)/2;

        //last entry is path length
        tcpSrc->setName("tcp_src_" + ntoa(flow.src) + "_" + ntoa(flow.dst) + "_" + ntoa(routeout->size()/2 - 2));
        logfile.writeName(*tcpSrc);

        tcpSnk->setName("tcp_sink_" + ntoa(flow.src) + "_" + ntoa(flow.dst) + "_" + ntoa(routeout->size()/2 - 2));
        logfile.writeName(*tcpSnk);

        tcpRtxScanner.registerTcp(*tcpSrc);

        routeout->push_back(tcpSnk);
        routein->push_back(tcpSrc);

        tcpSrc->connect(*routeout, *routein, *tcpSnk, timeFromMs(flow.start_time_ms));
    }

    cout << "Set up all flows" << endl;
    top->delete_net_paths_rack_based(numintervals);

    // Record the setup
    int pktsize = Packet::data_packet_size();
    logfile.write("# pktsize=" + ntoa(pktsize) + " bytes");
    logfile.write("# hostnicrate = " + ntoa(HOST_NIC) + " pkt/sec");
    // logfile.write("# corelinkrate = " + ntoa(HOST_NIC*CORE_TO_HOST) + " pkt/sec");
    //logfile.write("# buffer = " + ntoa((double) (queues_na_ni[0][1]->_maxsize) / ((double) pktsize)) + " pkt");
    double rtt = timeAsSec(timeFromUs(RTT));
    logfile.write("# rtt =" + ntoa(rtt));

    // GO!
    cout << "topology  " << total_path_lengths << " " << total_available_paths << " " << total_available_first_hops << endl;
    cout << "starting simulation " << endl;
    while (eventlist.doNextEvent()) {}
}

string ntoa(double n) {
    stringstream s;
    s << n;
    return s.str();
}

string itoa(uint64_t n) {
    stringstream s;
    s << n;
    return s.str();
}
