#include "leaf_spine_topology.h"
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include "string.h"
#include <sstream>
#include <strstream>
#include <iostream>
#include "main.h"

#define PATHWEIGHTS true

const bool USE_DISJOINT = false;

extern uint32_t RTT;

string ntoa(double n);
string itoa(uint64_t n);

// extern int N;

double my_ls_os_ratio = 1.0;

LeafSpineTopology::LeafSpineTopology(Logfile* lg, EventList* ev, queue_type qt, int numfaillinks, string linkfailurefile, string netpathfile, string pathweightfile, uint32_t numswitches, uint32_t ls_k, uint16_t _os, double os_ratio){
  logfile = lg;
  eventlist = ev;
  qtype = qt;

  my_ls_os_ratio = os_ratio;

  this->os = _os;
  ls_lsx = (3*ls_k/4);
  ls_lsy = (ls_k/4);
  ls_nl = (ls_lsx + ls_lsy);
  ls_nsp = (ls_lsy);
  ls_nsrv = (os*(ls_lsx + ls_lsy)*ls_lsx);

  pipes_nup_nlp.resize(ls_nsp, std::vector<Pipe*>(ls_nl, nullptr));
  pipes_nlp_ns.resize(ls_nl, std::vector<Pipe*>(ls_nsrv, nullptr));
  queues_nup_nlp.resize(ls_nsp, std::vector<Queue*>(ls_nl, nullptr));
  queues_nlp_ns.resize(ls_nl, std::vector<Queue*>(ls_nsrv, nullptr));

  pipes_nlp_nup.resize(ls_nl, std::vector<Pipe*>(ls_nsp, nullptr));
  pipes_ns_nlp.resize(ls_nsrv, std::vector<Pipe*>(ls_nl, nullptr));
  queues_nlp_nup.resize(ls_nl, std::vector<Queue*>(ls_nsp, nullptr));
  queues_ns_nlp.resize(ls_nsrv, std::vector<Queue*>(ls_nl, nullptr));

  linkFailure.resize(numswitches, std::vector<std::vector<int>>(numswitches, std::vector<int>(2)));
  
  for (int i=0; i < numswitches; i++) {
    for (int j=0; j<numswitches; j++) {
      for (int k=0; k<2; k++) {
        linkFailure[i][j][k] = 0;
      }
    }
  }

  if (numfaillinks == 0) {
    init_network_eval();
  } else {
    ifstream lffile(linkfailurefile.c_str());
    string lfline;
    if (lffile.is_open()){
      while(lffile.good()){
      getline(lffile, lfline);
      if (lfline.find_first_not_of(' ') == string::npos) break;
          stringstream ss(lfline);
          int lower, upper, direction;
          ss >> lower >> upper >> direction;
          if(lower >=ls_nl || upper >= ls_nsp){
            cout<<"linkfailurefile has out of bounds nodes, "<<lower<<","<<upper<<","<<direction<<endl;
            exit(0);
          }
          linkFailure[lower][upper][direction] = 1;
      }
      lffile.close();
    }
    cout<<"linkfailurefile: "<<linkfailurefile<<endl;

    init_network_withfaillinks_eval();
  }

  net_paths_rack_based = new vector<route_t*>**[ls_nl];
  for (int i=0;i<ls_nl;i++){
  	net_paths_rack_based[i] = new vector<route_t*>*[ls_nl];
  	for (int j = 0;j<ls_nl;j++){
  		net_paths_rack_based[i][j] = NULL;
  	}
  }

	// Read netpath from file
	ifstream npfile(netpathfile.c_str());
    string npline;
    if (npfile.is_open()){
      while(npfile.good()){
        getline(npfile, npline);
        if (npline.find_first_not_of(' ') == string::npos) break;
        stringstream npss(npline);
        int flowSrc,flowDst,num_paths;
        vector<route_t*> *paths_rack_based;
        if (npline.find_first_of("->") == string::npos) {
          npss >> flowSrc >> flowDst >> num_paths;
          paths_rack_based = new vector<route_t*>();
          net_paths_rack_based[flowSrc][flowDst] = paths_rack_based;
        } else {
          string link;
          int linkSrc,linkDst;
          route_t *routeout = new route_t();
          while (npss >> link) {
            size_t found = link.find("->");
            if (found != string::npos) {
              linkSrc = stoi(link.substr(0,found));
              linkDst = stoi(link.substr(found+2));
              if (linkSrc<ls_nl && linkDst<ls_nl) {
                cout << "***Error: linkSrc<NL and linkDst<NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(ls_nl) << endl;
                exit(1);
              } else if (linkSrc<ls_nl) { // nlp-nup
                linkDst -= ls_nl;
                routeout->push_back(queues_nlp_nup[linkSrc][linkDst]);
                routeout->push_back(pipes_nlp_nup[linkSrc][linkDst]);
              } else if (linkDst<ls_nl) { // nup-nlp
                linkSrc -= ls_nl;
                routeout->push_back(queues_nup_nlp[linkSrc][linkDst]);
                routeout->push_back(pipes_nup_nlp[linkSrc][linkDst]);
              } else {
                cout << "***Error: linkSrc>NL and linkDst>NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(ls_nl) << endl;
                exit(1);
              }
            }
          }
          paths_rack_based->push_back(routeout);
        }
      }
      npfile.close();
    } 
	  else {
      cout << "***Error opening netpathfile: " << netpathfile << endl;
      exit(1);
    }

	// Initialize path_weights_rack_based
	int numintervals = 1;
	path_weights_rack_based = new vector < pair<int,double> > ***[numintervals];
	for (int k=0; k<numintervals; k++) {
		path_weights_rack_based[k] = new vector < pair<int,double> > **[ls_nl];
		for (int i=0; i<ls_nl; i++) {
			path_weights_rack_based[k][i] = new vector < pair<int,double> > *[ls_nl];
			for (int j=0; j<ls_nl; j++) {
				path_weights_rack_based[k][i][j] = new vector < pair<int,double> > ();
			}
		}
	}

	// Read pathweight from file
	for (int i=0; i<numintervals; i++) {
		ifstream pwfile(pathweightfile.c_str());
		string pwline;
		if (pwfile.is_open()){
			while(pwfile.good()){
				getline(pwfile, pwline);
				if (pwline.find_first_not_of(' ') == string::npos) break;
				stringstream ss(pwline);
        string token;
				vector<string> tokens;
				while (getline(ss,token,'\t')) {
					tokens.push_back(token);
				}
				int flowSrc = stoi(tokens[0]);
				int flowDst = stoi(tokens[1]);
				int pid = stoi(tokens[2]);
				double weight = stod(tokens[5]);

				path_weights_rack_based[i][flowSrc][flowDst]->push_back(pair<int,double>(pid,weight));
			}
			pwfile.close();
		}
		else {
			cout << "***Error opening pathweightfile: " << pathweightfile << endl;
			exit(1);
		}
	}
}


uint32_t LeafSpineTopology::eval_host_tor_switch(uint32_t src) {
  return src/os/ls_lsx;
}


LeafSpineTopology::LeafSpineTopology(Logfile* lg, EventList* ev, queue_type qt, int numfaillinks, int failseed, string netpathfile, string pathweightfile){
  logfile = lg;
  eventlist = ev;
  qtype = qt;
  
  // N = NSRV;

  // int num_links = (N/OVERSUBSCRIPTION) * 2;
  
  //srand ( time(NULL));

  for (int i=0; i < NSW; i++) {
    for (int j=0; j<NSW; j++) {
      for (int k=0; k<2; k++) {
        linkFailure[i][j][k] = 0;
      }
    }
  }

  if (numfaillinks == 0) {
    init_network();
  } else {
    string linkfailurefile = "linkfailurefiles/leafspine_2048_"+std::to_string(numfaillinks)+"_"+std::to_string(failseed);
    ifstream lffile(linkfailurefile.c_str());
    string lfline;
    if (lffile.is_open()){
      while(lffile.good()){
      getline(lffile, lfline);
      if (lfline.find_first_not_of(' ') == string::npos) break;
          stringstream ss(lfline);
          int lower, upper, direction;
          ss >> lower >> upper >> direction;
          if(lower >= NL || upper >= NSP){
            cout<<"linkfailurefile has out of bounds nodes, "<<lower<<","<<upper<<","<<direction<<endl;
            exit(0);
          }
          linkFailure[lower][upper][direction] = 1;
      }
      lffile.close();
    }
    cout<<"linkfailurefile: "<<linkfailurefile<<endl;

    init_network_withfaillinks();
  }

  ecmp_net_paths = new vector<route_t*>**[NL];
  for (int i=0;i<NL;i++){
  	ecmp_net_paths[i] = new vector<route_t*>*[NL];
  	for (int j = 0;j<NL;j++){
  		ecmp_net_paths[i][j] = NULL;
  	}
  }

#if PATHWEIGHTS
	// Read netpath from file
	ifstream npfile(netpathfile.c_str());
    string npline;
    if (npfile.is_open()){
      while(npfile.good()){
        getline(npfile, npline);
        if (npline.find_first_not_of(' ') == string::npos) break;
        stringstream npss(npline);
        int flowSrc,flowDst,num_paths;
        vector<route_t*> *paths_rack_based;
        if (npline.find_first_of("->") == string::npos) {
          npss >> flowSrc >> flowDst >> num_paths;
          paths_rack_based = new vector<route_t*>();
          ecmp_net_paths[flowSrc][flowDst] = paths_rack_based;
        } else {
          string link;
          int linkSrc,linkDst;
          route_t *routeout = new route_t();
          while (npss >> link) {
            size_t found = link.find("->");
            if (found != string::npos) {
              linkSrc = stoi(link.substr(0,found));
              linkDst = stoi(link.substr(found+2));
              if (linkSrc<NL && linkDst<NL) {
                cout << "***Error: linkSrc<NL and linkDst<NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(NL) << endl;
                exit(1);
              } else if (linkSrc<NL) { // nlp-nup
                linkDst -= NL;
                routeout->push_back(queues_nlp_nup[linkSrc][linkDst]);
                routeout->push_back(pipes_nlp_nup[linkSrc][linkDst]);
              } else if (linkDst<NL) { // nup-nlp
                linkSrc -= NL;
                routeout->push_back(queues_nup_nlp[linkSrc][linkDst]);
                routeout->push_back(pipes_nup_nlp[linkSrc][linkDst]);
              } else {
                cout << "***Error: linkSrc>NL and linkDst>NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(NL) << endl;
                exit(1);
              }
            }
          }
          paths_rack_based->push_back(routeout);
        }
      }
      npfile.close();
    } 
	  else {
      cout << "***Error opening netpathfile: " << netpathfile << endl;
      exit(1);
    }

	// Initialize path_weights_rack_based
	int numintervals = 1;
	path_weights_rack_based = new vector < pair<int,double> > ***[numintervals];
	for (int k=0; k<numintervals; k++) {
		path_weights_rack_based[k] = new vector < pair<int,double> > **[NL];
		for (int i=0; i<NL; i++) {
			path_weights_rack_based[k][i] = new vector < pair<int,double> > *[NL];
			for (int j=0; j<NL; j++) {
				path_weights_rack_based[k][i][j] = new vector < pair<int,double> > ();
			}
		}
	}

	// Read pathweight from file
	for (int i=0; i<numintervals; i++) {
		ifstream pwfile(pathweightfile.c_str());
		string pwline;
		if (pwfile.is_open()){
			while(pwfile.good()){
				getline(pwfile, pwline);
				if (pwline.find_first_not_of(' ') == string::npos) break;
				stringstream ss(pwline);
        string token;
				vector<string> tokens;
				while (getline(ss,token,',')) {
					tokens.push_back(token);
				}
				int flowSrc = stoi(tokens[0]);
				int flowDst = stoi(tokens[1]);
				int pid = stoi(tokens[2]);
				double weight = stod(tokens[3]);

				// int flowSrc,flowDst,pid,linkSrc,linkDst;
				// double weight;
				// ss >> flowSrc >> flowDst >> pid >> linkSrc >> linkDst >> weight;

				// if (flowSrc>=NL || flowDst>=NL) {
				// 	cout << "***Error: flowSrc>=NL || flowDst>=NL, flowSrc=" << itoa(flowSrc) << ", flowDst=" << itoa(flowDst) << ", NL=" << itoa(NL) << endl;
				// 	exit(1);
				// }
        // if (linkSrc>=NL || linkDst<NL) {
				// 	cout << "***Error: linkSrc>=NL || linkDst<NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(NL) << endl;
				// 	exit(1);
				// }
        // linkDst-=NL;
				// // check whether netpathfile and pathweightfile are indeed matching
				// PacketSink *firstqueue = net_paths_rack_based[flowSrc][flowDst]->at(pid)->at(0);
				// if (firstqueue != queues_nlp_nup[linkSrc][linkDst]) {
				// 	cout << "***Error: netpathfile and pathweightfile mismatch, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", queue has name " << firstqueue->nodename() << endl;
				// 	exit(1);
				// }

				path_weights_rack_based[i][flowSrc][flowDst]->push_back(pair<int,double>(pid,weight));
			}
			pwfile.close();
		}
		else {
			cout << "***Error opening pathweightfile: " << pathweightfile << endl;
			exit(1);
		}
	}

  // AnnC: solely for testing purpose
  // std::cout << path_weights_rack_based[0][7][6]->at(0).first << ": " << path_weights_rack_based[0][7][6]->at(0).second << std::endl;
	// std::cout << path_weights_rack_based[0][42][48]->at(1).first << ": " << path_weights_rack_based[0][42][48]->at(1).second << std::endl;
	// std::cout << path_weights_rack_based[0][14][35]->at(2).first << ": " << path_weights_rack_based[0][14][35]->at(2).second << std::endl;
  
#endif

}

void LeafSpineTopology::init_network_eval(){
  QueueLoggerSampling* queueLogger;

  for (int j=0;j<ls_nsp;j++){
    for (int k=0;k<ls_nl;k++){
      queues_nup_nlp[j][k] = NULL;
      pipes_nup_nlp[j][k] = NULL;
      queues_nlp_nup[k][j] = NULL;
      pipes_nlp_nup[k][j] = NULL;
    }
  }
  
  for (int j=0;j<ls_nl;j++)
    for (int k=0;k<ls_nsrv;k++){
      queues_nlp_ns[j][k] = NULL;
      pipes_nlp_ns[j][k] = NULL;
      queues_ns_nlp[k][j] = NULL;
      pipes_ns_nlp[k][j] = NULL;
    }

    cout<<"Link Speed: "<<speedFromPktps(HOST_NIC)<<endl;
    mem_b queue_size = SWITCH_BUFFER * Packet::data_packet_size();

    // lower layer switch to server
    for (int j = 0; j < ls_nl; j++) {
        for (int l = 0; l < ls_lsx * os; l++) {
           int k = j * ls_lsx * os + l;
           int delay = 1000000;
           // Downlink
           queueLogger = new QueueLoggerSampling(timeFromMs(delay), *eventlist);
           //queueLogger = NULL;
           logfile->addLogger(*queueLogger);

           queues_nlp_ns[j][k] = alloc_queue(queueLogger, HOST_NIC*my_ls_os_ratio, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
           queues_nlp_ns[j][k]->setName("LS_" + ntoa(j) + "-" + "DST_" +ntoa(k));
           logfile->writeName(*(queues_nlp_ns[j][k]));

           pipes_nlp_ns[j][k] = new Pipe(timeFromUs(RTT), *eventlist);
           pipes_nlp_ns[j][k]->setName("Pipe-nt-ns-" + ntoa(j) + "-" + ntoa(k));
           logfile->writeName(*(pipes_nlp_ns[j][k]));
           
           // Uplink
           queueLogger = new QueueLoggerSampling(timeFromMs(delay), *eventlist);
           logfile->addLogger(*queueLogger);
           queues_ns_nlp[k][j] = alloc_queue(queueLogger, HOST_NIC*my_ls_os_ratio, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
           queues_ns_nlp[k][j]->setName("SRC_" + ntoa(k) + "-" + "LS_"+ntoa(j));
           logfile->writeName(*(queues_ns_nlp[k][j]));
           
           pipes_ns_nlp[k][j] = new Pipe(timeFromUs(RTT), *eventlist);
           pipes_ns_nlp[k][j]->setName("Pipe-ns-nt-" + ntoa(k) + "-" + ntoa(j));
           logfile->writeName(*(pipes_ns_nlp[k][j]));
        }
    }

    //Lower layer to upper layer 
    for (int j = 0; j < ls_nl; j++) {
      //Connect the lower layer switch to the upper layer switches 
      for (int k=0; k<ls_nsp;k++){
         // Downlink
         queueLogger = new QueueLoggerSampling(timeFromMs(1000), *eventlist);
         logfile->addLogger(*queueLogger);
         queues_nup_nlp[k][j] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
         queues_nup_nlp[k][j]->setName("US_" + ntoa(k) + "-" + "LS_"+ntoa(j));
         logfile->writeName(*(queues_nup_nlp[k][j]));
         
         pipes_nup_nlp[k][j] = new Pipe(timeFromUs(RTT), *eventlist);
         pipes_nup_nlp[k][j]->setName("Pipe-na-nt-" + ntoa(k) + "-" + ntoa(j));
         logfile->writeName(*(pipes_nup_nlp[k][j]));
         
         // Uplink
         queueLogger = new QueueLoggerSampling(timeFromMs(1000), *eventlist);
         logfile->addLogger(*queueLogger);
         queues_nlp_nup[j][k] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
         queues_nlp_nup[j][k]->setName("LS_" + ntoa(j) + "-" + "US_"+ntoa(k));
         logfile->writeName(*(queues_nlp_nup[j][k]));
         
         pipes_nlp_nup[j][k] = new Pipe(timeFromUs(RTT), *eventlist);
         pipes_nlp_nup[j][k]->setName("Pipe-nt-na-" + ntoa(j) + "-" + ntoa(k));
         logfile->writeName(*(pipes_nlp_nup[j][k]));
      }
    }
}

void LeafSpineTopology::init_network_withfaillinks_eval(){
  QueueLoggerSampling* queueLogger;

  for (int j=0;j<ls_nsp;j++){
    for (int k=0;k<ls_nl;k++){
      queues_nup_nlp[j][k] = NULL;
      pipes_nup_nlp[j][k] = NULL;
      queues_nlp_nup[k][j] = NULL;
      pipes_nlp_nup[k][j] = NULL;
    }
  }
  
  for (int j=0;j<ls_nl;j++)
    for (int k=0;k<ls_nsrv;k++){
      queues_nlp_ns[j][k] = NULL;
      pipes_nlp_ns[j][k] = NULL;
      queues_ns_nlp[k][j] = NULL;
      pipes_ns_nlp[k][j] = NULL;
    }

    cout<<"Link Speed: "<<speedFromPktps(HOST_NIC)<<endl;
    mem_b queue_size = SWITCH_BUFFER * Packet::data_packet_size();

    // lower layer switch to server
    for (int j = 0; j < ls_nl; j++) {
        for (int l = 0; l < ls_lsx * os; l++) {
           int k = j * ls_lsx * os + l;
           int delay = 1000000;
           // Downlink
           queueLogger = new QueueLoggerSampling(timeFromMs(delay), *eventlist);
           //queueLogger = NULL;
           logfile->addLogger(*queueLogger);

           queues_nlp_ns[j][k] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
           queues_nlp_ns[j][k]->setName("LS_" + ntoa(j) + "-" + "DST_" +ntoa(k));
           logfile->writeName(*(queues_nlp_ns[j][k]));

           pipes_nlp_ns[j][k] = new Pipe(timeFromUs(RTT), *eventlist);
           pipes_nlp_ns[j][k]->setName("Pipe-nt-ns-" + ntoa(j) + "-" + ntoa(k));
           logfile->writeName(*(pipes_nlp_ns[j][k]));
           
           // Uplink
           queueLogger = new QueueLoggerSampling(timeFromMs(delay), *eventlist);
           logfile->addLogger(*queueLogger);
           queues_ns_nlp[k][j] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
           queues_ns_nlp[k][j]->setName("SRC_" + ntoa(k) + "-" + "LS_"+ntoa(j));
           logfile->writeName(*(queues_ns_nlp[k][j]));
           
           pipes_ns_nlp[k][j] = new Pipe(timeFromUs(RTT), *eventlist);
           pipes_ns_nlp[k][j]->setName("Pipe-ns-nt-" + ntoa(k) + "-" + ntoa(j));
           logfile->writeName(*(pipes_ns_nlp[k][j]));
        }
    }

    //Lower layer to upper layer 
    for (int j = 0; j < ls_nl; j++) {
      //Connect the lower layer switch to the upper layer switches 
      for (int k=0; k<ls_nsp;k++){
         // Downlink
         uint64_t downbw = HOST_NIC;
         if (linkFailure[j][k][1] == 1) downbw /= 2;
         queueLogger = new QueueLoggerSampling(timeFromMs(1000), *eventlist);
         logfile->addLogger(*queueLogger);
         queues_nup_nlp[k][j] = alloc_queue(queueLogger, downbw, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
         queues_nup_nlp[k][j]->setName("US_" + ntoa(k) + "-" + "LS_"+ntoa(j));
         logfile->writeName(*(queues_nup_nlp[k][j]));
         
         pipes_nup_nlp[k][j] = new Pipe(timeFromUs(RTT), *eventlist);
         pipes_nup_nlp[k][j]->setName("Pipe-na-nt-" + ntoa(k) + "-" + ntoa(j));
         logfile->writeName(*(pipes_nup_nlp[k][j]));
         
         // Uplink
         uint64_t upbw = HOST_NIC;
         if (linkFailure[j][k][0] == 1) upbw /= 2;
         queueLogger = new QueueLoggerSampling(timeFromMs(1000), *eventlist);
         logfile->addLogger(*queueLogger);
         queues_nlp_nup[j][k] = alloc_queue(queueLogger, upbw, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
         queues_nlp_nup[j][k]->setName("LS_" + ntoa(j) + "-" + "US_"+ntoa(k));
         logfile->writeName(*(queues_nlp_nup[j][k]));
         
         pipes_nlp_nup[j][k] = new Pipe(timeFromUs(RTT), *eventlist);
         pipes_nlp_nup[j][k]->setName("Pipe-nt-na-" + ntoa(j) + "-" + ntoa(k));
         logfile->writeName(*(pipes_nlp_nup[j][k]));
      }
    }
}

void LeafSpineTopology::init_network(){
  QueueLoggerSampling* queueLogger;

  for (int j=0;j<NSP;j++){
    for (int k=0;k<NL;k++){
      queues_nup_nlp[j][k] = NULL;
      pipes_nup_nlp[j][k] = NULL;
      queues_nlp_nup[k][j] = NULL;
      pipes_nlp_nup[k][j] = NULL;
    }
  }
  
  for (int j=0;j<NL;j++)
    for (int k=0;k<NSRV;k++){
      queues_nlp_ns[j][k] = NULL;
      pipes_nlp_ns[j][k] = NULL;
      queues_ns_nlp[k][j] = NULL;
      pipes_ns_nlp[k][j] = NULL;
    }

    cout<<"Link Speed: "<<speedFromPktps(HOST_NIC)<<endl;
    mem_b queue_size = SWITCH_BUFFER * Packet::data_packet_size();

    // lower layer switch to server
    for (int j = 0; j < NL; j++) {
        for (int l = 0; l < LSX * OVERSUBSCRIPTION; l++) {
           int k = j * LSX * OVERSUBSCRIPTION + l;
           int delay = 1000000;
           // Downlink
           queueLogger = new QueueLoggerSampling(timeFromMs(delay), *eventlist);
           //queueLogger = NULL;
           logfile->addLogger(*queueLogger);

           queues_nlp_ns[j][k] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
           queues_nlp_ns[j][k]->setName("LS_" + ntoa(j) + "-" + "DST_" +ntoa(k));
           logfile->writeName(*(queues_nlp_ns[j][k]));

           pipes_nlp_ns[j][k] = new Pipe(timeFromUs(RTT), *eventlist);
           pipes_nlp_ns[j][k]->setName("Pipe-nt-ns-" + ntoa(j) + "-" + ntoa(k));
           logfile->writeName(*(pipes_nlp_ns[j][k]));
           
           // Uplink
           queueLogger = new QueueLoggerSampling(timeFromMs(delay), *eventlist);
           logfile->addLogger(*queueLogger);
           queues_ns_nlp[k][j] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
           queues_ns_nlp[k][j]->setName("SRC_" + ntoa(k) + "-" + "LS_"+ntoa(j));
           logfile->writeName(*(queues_ns_nlp[k][j]));
           
           pipes_ns_nlp[k][j] = new Pipe(timeFromUs(RTT), *eventlist);
           pipes_ns_nlp[k][j]->setName("Pipe-ns-nt-" + ntoa(k) + "-" + ntoa(j));
           logfile->writeName(*(pipes_ns_nlp[k][j]));
        }
    }

    /*    for (int i = 0;i<NSRV;i++){
      for (int j = 0;j<NK;j++){
	printf("%p/%p ",queues_ns_nlp[i][j], queues_nlp_ns[j][i]);
      }
      printf("\n");
      }*/
    
    //Lower layer to upper layer 
    for (int j = 0; j < NL; j++) {
      //Connect the lower layer switch to the upper layer switches 
      for (int k=0; k<NSP;k++){
         // Downlink
         queueLogger = new QueueLoggerSampling(timeFromMs(1000), *eventlist);
         logfile->addLogger(*queueLogger);
         queues_nup_nlp[k][j] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
         queues_nup_nlp[k][j]->setName("US_" + ntoa(k) + "-" + "LS_"+ntoa(j));
         logfile->writeName(*(queues_nup_nlp[k][j]));
         
         pipes_nup_nlp[k][j] = new Pipe(timeFromUs(RTT), *eventlist);
         pipes_nup_nlp[k][j]->setName("Pipe-na-nt-" + ntoa(k) + "-" + ntoa(j));
         logfile->writeName(*(pipes_nup_nlp[k][j]));
         
         // Uplink
         queueLogger = new QueueLoggerSampling(timeFromMs(1000), *eventlist);
         logfile->addLogger(*queueLogger);
         queues_nlp_nup[j][k] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
         queues_nlp_nup[j][k]->setName("LS_" + ntoa(j) + "-" + "US_"+ntoa(k));
         logfile->writeName(*(queues_nlp_nup[j][k]));
         
         pipes_nlp_nup[j][k] = new Pipe(timeFromUs(RTT), *eventlist);
         pipes_nlp_nup[j][k]->setName("Pipe-nt-na-" + ntoa(j) + "-" + ntoa(k));
         logfile->writeName(*(pipes_nlp_nup[j][k]));
      }
    }

    /*for (int i = 0;i<NK;i++){
      for (int j = 0;j<NK;j++){
	printf("%p/%p ",queues_nlp_nup[i][j], queues_nup_nlp[j][i]);
      }
      printf("\n");
      }*/
}

void LeafSpineTopology::init_network_withfaillinks(){
  QueueLoggerSampling* queueLogger;

  for (int j=0;j<NSP;j++){
    for (int k=0;k<NL;k++){
      queues_nup_nlp[j][k] = NULL;
      pipes_nup_nlp[j][k] = NULL;
      queues_nlp_nup[k][j] = NULL;
      pipes_nlp_nup[k][j] = NULL;
    }
  }
  
  for (int j=0;j<NL;j++)
    for (int k=0;k<NSRV;k++){
      queues_nlp_ns[j][k] = NULL;
      pipes_nlp_ns[j][k] = NULL;
      queues_ns_nlp[k][j] = NULL;
      pipes_ns_nlp[k][j] = NULL;
    }

    cout<<"Link Speed: "<<speedFromPktps(HOST_NIC)<<endl;
    mem_b queue_size = SWITCH_BUFFER * Packet::data_packet_size();

    // lower layer switch to server
    for (int j = 0; j < NL; j++) {
        for (int l = 0; l < LSX * OVERSUBSCRIPTION; l++) {
           int k = j * LSX * OVERSUBSCRIPTION + l;
           int delay = 1000000;
           // Downlink
           queueLogger = new QueueLoggerSampling(timeFromMs(delay), *eventlist);
           //queueLogger = NULL;
           logfile->addLogger(*queueLogger);

           queues_nlp_ns[j][k] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
           queues_nlp_ns[j][k]->setName("LS_" + ntoa(j) + "-" + "DST_" +ntoa(k));
           logfile->writeName(*(queues_nlp_ns[j][k]));

           pipes_nlp_ns[j][k] = new Pipe(timeFromUs(RTT), *eventlist);
           pipes_nlp_ns[j][k]->setName("Pipe-nt-ns-" + ntoa(j) + "-" + ntoa(k));
           logfile->writeName(*(pipes_nlp_ns[j][k]));
           
           // Uplink
           queueLogger = new QueueLoggerSampling(timeFromMs(delay), *eventlist);
           logfile->addLogger(*queueLogger);
           queues_ns_nlp[k][j] = alloc_queue(queueLogger, HOST_NIC, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
           queues_ns_nlp[k][j]->setName("SRC_" + ntoa(k) + "-" + "LS_"+ntoa(j));
           logfile->writeName(*(queues_ns_nlp[k][j]));
           
           pipes_ns_nlp[k][j] = new Pipe(timeFromUs(RTT), *eventlist);
           pipes_ns_nlp[k][j]->setName("Pipe-ns-nt-" + ntoa(k) + "-" + ntoa(j));
           logfile->writeName(*(pipes_ns_nlp[k][j]));
        }
    }

    /*    for (int i = 0;i<NSRV;i++){
      for (int j = 0;j<NK;j++){
	printf("%p/%p ",queues_ns_nlp[i][j], queues_nlp_ns[j][i]);
      }
      printf("\n");
      }*/
    
    //Lower layer to upper layer 
    for (int j = 0; j < NL; j++) {
      //Connect the lower layer switch to the upper layer switches 
      for (int k=0; k<NSP;k++){
         // Downlink
         uint64_t downbw = HOST_NIC;
         if (linkFailure[j][k][1] == 1) downbw /= 2;
         queueLogger = new QueueLoggerSampling(timeFromMs(1000), *eventlist);
         logfile->addLogger(*queueLogger);
         queues_nup_nlp[k][j] = alloc_queue(queueLogger, downbw, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
         queues_nup_nlp[k][j]->setName("US_" + ntoa(k) + "-" + "LS_"+ntoa(j));
         logfile->writeName(*(queues_nup_nlp[k][j]));
         
         pipes_nup_nlp[k][j] = new Pipe(timeFromUs(RTT), *eventlist);
         pipes_nup_nlp[k][j]->setName("Pipe-na-nt-" + ntoa(k) + "-" + ntoa(j));
         logfile->writeName(*(pipes_nup_nlp[k][j]));
         
         // Uplink
         uint64_t upbw = HOST_NIC;
         if (linkFailure[j][k][0] == 1) upbw /= 2;
         queueLogger = new QueueLoggerSampling(timeFromMs(1000), *eventlist);
         logfile->addLogger(*queueLogger);
         queues_nlp_nup[j][k] = alloc_queue(queueLogger, upbw, queue_size); //new RandomQueue(speedFromPktps(HOST_NIC), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
         queues_nlp_nup[j][k]->setName("LS_" + ntoa(j) + "-" + "US_"+ntoa(k));
         logfile->writeName(*(queues_nlp_nup[j][k]));
         
         pipes_nlp_nup[j][k] = new Pipe(timeFromUs(RTT), *eventlist);
         pipes_nlp_nup[j][k]->setName("Pipe-nt-na-" + ntoa(j) + "-" + ntoa(k));
         logfile->writeName(*(pipes_nlp_nup[j][k]));
      }
    }

    /*for (int i = 0;i<NK;i++){
      for (int j = 0;j<NK;j++){
	printf("%p/%p ",queues_nlp_nup[i][j], queues_nup_nlp[j][i]);
      }
      printf("\n");
      }*/
}

static void check_non_null(route_t* rt){
  int fail = 0;
  for (unsigned int i=1;i<rt->size()-1;i+=2)
    if (rt->at(i)==NULL){
      fail = 1;
      break;
    }
  
  if (fail){
    //    cout <<"Null queue in route"<<endl;
    for (unsigned int i=1;i<rt->size()-1;i+=2)
      printf("%p ",rt->at(i));

    cout<<endl;
    assert(0);
  }
}



int LeafSpineTopology::get_distance(int src, int dest){
  if (HOST_TOR_SWITCH(src)==HOST_TOR_SWITCH(dest)) return 0;
  else return 2;
}



pair<vector<double>*, vector<route_t*>*> LeafSpineTopology::get_other_paths(int src, int dest){
	return get_paths(src, dest);
}

pair<vector<double>*, vector<route_t*>*> LeafSpineTopology::get_paths(int src, int dest){
  vector<double>* weights = NULL;

  route_t* routeout;
  vector<route_t*>* paths = new vector<route_t*>();

  int src_sw = src;
  int dest_sw = dest;

  if (src_sw == dest_sw){
    routeout = new route_t();
    paths->push_back(routeout);
    net_paths_rack_based[src_sw][dest_sw] = paths;
  }
  else{
    //there are NSP paths between the source and the destination
    for (int upper = 0;upper < NSP; upper++){
      //upper is nup; lower is nlp
      routeout = new route_t();
      routeout->push_back(queues_nlp_nup[src_sw][upper]);
      routeout->push_back(pipes_nlp_nup[src_sw][upper]);
      routeout->push_back(queues_nup_nlp[upper][dest_sw]);
      routeout->push_back(pipes_nup_nlp[upper][dest_sw]);
      paths->push_back(routeout);
      check_non_null(routeout);
    }
    net_paths_rack_based[src_sw][dest_sw] = paths;
  }
  return pair<vector<double>*, vector<route_t*>*>(weights, paths);
}

route_t *LeafSpineTopology::attach_head_tail(int src, int dst, bool is_same_switch, int rand_choice) {
  int src_sw = ConvertHostToRack(src);
  int dst_sw = ConvertHostToRack(dst);
	route_t *this_route;

  #if IS_EVAL

  if (is_same_switch) {
    assert(rand_choice == 0);
    this_route = new route_t();

		Queue* pqueue = new Queue(speedFromPktps(HOST_NIC), memFromPkt(FEEDER_BUFFER), *eventlist, NULL);
		pqueue->setName("PQueue_" + ntoa(src) + "_" + ntoa(dst));
		logfile->writeName(*pqueue);

		this_route->push_back(pqueue);
		this_route->push_back(queues_ns_nlp[src][eval_host_tor_switch(src)]);
		this_route->push_back(pipes_ns_nlp[src][eval_host_tor_switch(src)]);

		this_route->push_back(queues_nlp_ns[eval_host_tor_switch(dst)][dst]);
		this_route->push_back(pipes_nlp_ns[eval_host_tor_switch(dst)][dst]);
	} 
  else {
    this_route = new route_t(*(net_paths_rack_based[src_sw][dst_sw]->at(rand_choice)));
		assert(this_route->size() > 0);

		Queue* pqueue = new Queue(speedFromPktps(HOST_NIC), memFromPkt(FEEDER_BUFFER), *eventlist, NULL);
		pqueue->setName("PQueue_" + ntoa(src) + "_" + ntoa(dst));
		logfile->writeName(*pqueue);
		this_route->push_front(pipes_ns_nlp[src][eval_host_tor_switch(src)]);
		this_route->push_front(queues_ns_nlp[src][eval_host_tor_switch(src)]);
		this_route->push_front(pqueue);

		this_route->push_back(queues_nlp_ns[eval_host_tor_switch(dst)][dst]);
		this_route->push_back(pipes_nlp_ns[eval_host_tor_switch(dst)][dst]);
	}

  #else

  if (is_same_switch) {
    assert(rand_choice == 0);
    this_route = new route_t();

		Queue* pqueue = new Queue(speedFromPktps(HOST_NIC), memFromPkt(FEEDER_BUFFER), *eventlist, NULL);
		pqueue->setName("PQueue_" + ntoa(src) + "_" + ntoa(dst));
		logfile->writeName(*pqueue);

		this_route->push_back(pqueue);
		this_route->push_back(queues_ns_nlp[src][HOST_TOR_SWITCH(src)]);
		this_route->push_back(pipes_ns_nlp[src][HOST_TOR_SWITCH(src)]);

		this_route->push_back(queues_nlp_ns[HOST_TOR_SWITCH(dst)][dst]);
		this_route->push_back(pipes_nlp_ns[HOST_TOR_SWITCH(dst)][dst]);
	} 
  else {
    this_route = new route_t(*(net_paths_rack_based[src_sw][dst_sw]->at(rand_choice)));
		assert(this_route->size() > 0);

		Queue* pqueue = new Queue(speedFromPktps(HOST_NIC), memFromPkt(FEEDER_BUFFER), *eventlist, NULL);
		pqueue->setName("PQueue_" + ntoa(src) + "_" + ntoa(dst));
		logfile->writeName(*pqueue);
		this_route->push_front(pipes_ns_nlp[src][HOST_TOR_SWITCH(src)]);
		this_route->push_front(queues_ns_nlp[src][HOST_TOR_SWITCH(src)]);
		this_route->push_front(pqueue);

		this_route->push_back(queues_nlp_ns[HOST_TOR_SWITCH(dst)][dst]);
		this_route->push_back(pipes_nlp_ns[HOST_TOR_SWITCH(dst)][dst]);
	}

  #endif
	return this_route;
}

void LeafSpineTopology::delete_net_paths_rack_based(int numintervals) {
  #if IS_EVAL

  for (int i=0; i<ls_nl; i++) {
    for (int j=0; j<ls_nl; j++) {
      if (net_paths_rack_based[i][j]) {
        for (auto p : (*net_paths_rack_based[i][j])) {
          delete p;
        }
        net_paths_rack_based[i][j]->clear();
        delete net_paths_rack_based[i][j];
      }
    }
    delete [] net_paths_rack_based[i];
  }
	delete [] net_paths_rack_based;

#if PATHWEIGHTS
  numintervals = 1;
	for (int k=0; k<numintervals; k++) {
		for (int i=0; i<ls_nl; i++) {
			for (int j=0; j<ls_nl; j++) {
				if (path_weights_rack_based[k][i][j]) {
					path_weights_rack_based[k][i][j]->clear();
					delete path_weights_rack_based[k][i][j];
				}
			}
			delete [] path_weights_rack_based[k][i];
		}	
		delete [] path_weights_rack_based[k];
	}
	delete [] path_weights_rack_based;
#endif
  
  #else

  for (int i=0; i<NL; i++) {
    for (int j=0; j<NL; j++) {
      if (net_paths_rack_based[i][j]) {
        for (auto p : (*net_paths_rack_based[i][j])) {
          delete p;
        }
        net_paths_rack_based[i][j]->clear();
        delete net_paths_rack_based[i][j];
      }
    }
    delete [] net_paths_rack_based[i];
  }
	delete [] net_paths_rack_based;

#if PATHWEIGHTS
  numintervals = 1;
	for (int k=0; k<numintervals; k++) {
		for (int i=0; i<NL; i++) {
			for (int j=0; j<NL; j++) {
				if (path_weights_rack_based[k][i][j]) {
					path_weights_rack_based[k][i][j]->clear();
					delete path_weights_rack_based[k][i][j];
				}
			}
			delete [] path_weights_rack_based[k][i];
		}	
		delete [] path_weights_rack_based[k];
	}
	delete [] path_weights_rack_based;
#endif

  #endif

}

void LeafSpineTopology::count_queue(Queue* queue){
  if (_link_usage.find(queue)==_link_usage.end()){
    _link_usage[queue] = 0;
  }
  _link_usage[queue] = _link_usage[queue] + 1;
}

int LeafSpineTopology::find_lp_switch(Queue* queue){
  //first check ns_nlp
  for (int i=0;i<NSRV;i++)
    for (int j = 0;j<NL;j++)
      if (queues_ns_nlp[i][j]==queue)
	      return j;

  //only count nup to nlp
  count_queue(queue);

  for (int i=0;i<NSP;i++)
    for (int j = 0;j<NL;j++)
      if (queues_nup_nlp[i][j]==queue)
	      return j;

  return -1;
}

int LeafSpineTopology::find_up_switch(Queue* queue){
  count_queue(queue);
  //check nlp_nup
  for (int i=0;i<NL;i++)
    for (int j = 0;j<NSP;j++)
      if (queues_nlp_nup[i][j]==queue)
	      return j;
  return -1;
}


int LeafSpineTopology::find_destination(Queue* queue){
  //first check nlp_ns
  for (int i=0;i<NL;i++)
    for (int j = 0;j<NSRV;j++)
      if (queues_nlp_ns[i][j]==queue)
	return j;

  return -1;
}

void LeafSpineTopology::print_path(std::ostream &paths,int src,route_t* route){
  paths << "SRC_" << src << " ";
  
  if (route->size()/2==2){
    paths << "LS_" << find_lp_switch((RandomQueue*)route->at(1)) << " ";
    paths << "DST_" << find_destination((RandomQueue*)route->at(3)) << " ";
  } else if (route->size()/2==4){
    paths << "LS_" << find_lp_switch((RandomQueue*)route->at(1)) << " ";
    paths << "US_" << find_up_switch((RandomQueue*)route->at(3)) << " ";
    paths << "LS_" << find_lp_switch((RandomQueue*)route->at(5)) << " ";
    paths << "DST_" << find_destination((RandomQueue*)route->at(7)) << " ";
  } else {
    paths << "Wrong hop count " << ntoa(route->size()/2);
  }
  
  paths << endl;
}

pair<vector<int>, vector<int> > LeafSpineTopology::getcsRacks(int clients, int servers){
   vector<int> clientracks, serverracks;
   vector<int> racks;
   for(int i=0; i<NL; i++){
    racks.push_back(i);
   }
   std::random_shuffle(racks.begin(), racks.end());
   int cCovered=0, sCovered=0;
   int curr_rack_id=0;
   while(curr_rack_id < NL and cCovered < clients){
    int rack = racks[curr_rack_id];
    cout<<"Client rack: "<<rack<<endl;
    clientracks.push_back(rack);
    cCovered += LSX;
    curr_rack_id++;
   }
   while(curr_rack_id < NL and sCovered < servers){
    int rack = racks[curr_rack_id];
    cout<<"Server rack: "<<rack<<endl;
    serverracks.push_back(rack);
    sCovered += LSX;
    curr_rack_id++;
   }

   if (cCovered < clients or sCovered < servers){
    cout << "Not enough racks for C-S model traffic with C=" << clients << " S=" << servers << endl;
    exit(0);
   }

   return pair<vector<int>, vector<int> > (clientracks, serverracks);
}


int LeafSpineTopology::ConvertHostToRack(int host) { 
  return eval_host_tor_switch(host);
};


LeafSpineTopology::LeafSpineTopology(Logfile* lg, EventList* ev, queue_type qt, string netpathfile, string pathweightfile){
//   logfile = lg;
//   eventlist = ev;
//   qtype = qt;
  
//   // N = NSRV;

//   // int num_links = (N/OVERSUBSCRIPTION) * 2;
  
//   //srand ( time(NULL));

//   init_network();

//   net_paths_rack_based = new vector<route_t*>**[NL];
//   for (int i=0;i<NL;i++){
//   	net_paths_rack_based[i] = new vector<route_t*>*[NL];
//   	for (int j = 0;j<NL;j++){
//   		net_paths_rack_based[i][j] = NULL;
//   	}
//   }

// #if PATHWEIGHTS
// 	// Read netpath from file
// 	ifstream npfile(netpathfile.c_str());
//     string npline;
//     if (npfile.is_open()){
//       while(npfile.good()){
//         getline(npfile, npline);
//         if (npline.find_first_not_of(' ') == string::npos) break;
//         stringstream npss(npline);
//         int flowSrc,flowDst,num_paths;
//         vector<route_t*> *paths_rack_based;
//         if (npline.find_first_of("->") == string::npos) {
//           npss >> flowSrc >> flowDst >> num_paths;
//           paths_rack_based = new vector<route_t*>();
//           net_paths_rack_based[flowSrc][flowDst] = paths_rack_based;
//         } else {
//           string link;
//           int linkSrc,linkDst;
//           route_t *routeout = new route_t();
//           while (npss >> link) {
//             size_t found = link.find("->");
//             if (found != string::npos) {
//               linkSrc = stoi(link.substr(0,found));
//               linkDst = stoi(link.substr(found+2));
//               if (linkSrc<NL && linkDst<NL) {
//                 cout << "***Error: linkSrc<NL and linkDst<NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(NL) << endl;
//                 exit(1);
//               } else if (linkSrc<NL) { // nlp-nup
//                 linkDst -= NL;
//                 routeout->push_back(queues_nlp_nup[linkSrc][linkDst]);
//                 routeout->push_back(pipes_nlp_nup[linkSrc][linkDst]);
//               } else if (linkDst<NL) { // nup-nlp
//                 linkSrc -= NL;
//                 routeout->push_back(queues_nup_nlp[linkSrc][linkDst]);
//                 routeout->push_back(pipes_nup_nlp[linkSrc][linkDst]);
//               } else {
//                 cout << "***Error: linkSrc>NL and linkDst>NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(NL) << endl;
//                 exit(1);
//               }
//             }
//           }
//           paths_rack_based->push_back(routeout);
//         }
//       }
//       npfile.close();
//     } 
// 	  else {
//       cout << "***Error opening netpathfile: " << netpathfile << endl;
//       exit(1);
//     }

// 	// Initialize path_weights_rack_based
// 	int numintervals = 1;
// 	path_weights_rack_based = new vector < pair<int,double> > ***[numintervals];
// 	for (int k=0; k<numintervals; k++) {
// 		path_weights_rack_based[k] = new vector < pair<int,double> > **[NL];
// 		for (int i=0; i<NL; i++) {
// 			path_weights_rack_based[k][i] = new vector < pair<int,double> > *[NL];
// 			for (int j=0; j<NL; j++) {
// 				path_weights_rack_based[k][i][j] = new vector < pair<int,double> > ();
// 			}
// 		}
// 	}

// 	// Read pathweight from file
// 	for (int i=0; i<numintervals; i++) {
// 		ifstream pwfile(pathweightfile.c_str());
// 		string pwline;
// 		if (pwfile.is_open()){
// 			while(pwfile.good()){
// 				getline(pwfile, pwline);
// 				if (pwline.find_first_not_of(' ') == string::npos) break;
// 				stringstream ss(pwline);
// 				int flowSrc,flowDst,pid,linkSrc,linkDst;
// 				double weight;
// 				ss >> flowSrc >> flowDst >> pid >> linkSrc >> linkDst >> weight;

// 				if (flowSrc>=NL || flowDst>=NL) {
// 					cout << "***Error: flowSrc>=NL || flowDst>=NL, flowSrc=" << itoa(flowSrc) << ", flowDst=" << itoa(flowDst) << ", NL=" << itoa(NL) << endl;
// 					exit(1);
// 				}
//         if (linkSrc>=NL || linkDst<NL) {
// 					cout << "***Error: linkSrc>=NL || linkDst<NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(NL) << endl;
// 					exit(1);
// 				}
//         linkDst-=NL;
// 				// check whether netpathfile and pathweightfile are indeed matching
// 				PacketSink *firstqueue = net_paths_rack_based[flowSrc][flowDst]->at(pid)->at(0);
// 				if (firstqueue != queues_nlp_nup[linkSrc][linkDst]) {
// 					cout << "***Error: netpathfile and pathweightfile mismatch, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", queue has name " << firstqueue->nodename() << endl;
// 					exit(1);
// 				}

// 				path_weights_rack_based[i][flowSrc][flowDst]->push_back(pair<int,double>(pid,weight));
// 			}
// 			pwfile.close();
// 		}
// 		else {
// 			cout << "***Error opening pathweightfile: " << pathweightfile << endl;
// 			exit(1);
// 		}
// 	}
// #endif

}


// LeafSpineTopology::LeafSpineTopology(Logfile* lg, EventList* ev, queue_type qt, int numfaillinks, int failseed, string netpathfile, string pathweightfile){
//   logfile = lg;
//   eventlist = ev;
//   qtype = qt;
  
//   // N = NSRV;

//   // int num_links = (N/OVERSUBSCRIPTION) * 2;
  
//   //srand ( time(NULL));

//   for (int i=0; i < NSW; i++) {
//     for (int j=0; j<NSW; j++) {
//       for (int k=0; k<2; k++) {
//         linkFailure[i][j][k] = 0;
//       }
//     }
//   }

//   if (numfaillinks == 0) {
//     init_network();
//   } else {
//     string linkfailurefile = "linkfailurefiles/leafspine_2048_"+std::to_string(numfaillinks)+"_"+std::to_string(failseed);
//     ifstream lffile(linkfailurefile.c_str());
//     string lfline;
//     if (lffile.is_open()){
//       while(lffile.good()){
//       getline(lffile, lfline);
//       if (lfline.find_first_not_of(' ') == string::npos) break;
//           stringstream ss(lfline);
//           int lower, upper, direction;
//           ss >> lower >> upper >> direction;
//           if(lower >= NL || upper >= NSP){
//             cout<<"linkfailurefile has out of bounds nodes, "<<lower<<","<<upper<<","<<direction<<endl;
//             exit(0);
//           }
//           linkFailure[lower][upper][direction] = 1;
//       }
//       lffile.close();
//     }
//     cout<<"linkfailurefile: "<<linkfailurefile<<endl;

//     init_network_withfaillinks();
//   }

//   net_paths_rack_based = new vector<route_t*>**[NL];
//   for (int i=0;i<NL;i++){
//   	net_paths_rack_based[i] = new vector<route_t*>*[NL];
//   	for (int j = 0;j<NL;j++){
//   		net_paths_rack_based[i][j] = NULL;
//   	}
//   }

// #if PATHWEIGHTS
// 	// Read netpath from file
// 	ifstream npfile(netpathfile.c_str());
//     string npline;
//     if (npfile.is_open()){
//       while(npfile.good()){
//         getline(npfile, npline);
//         if (npline.find_first_not_of(' ') == string::npos) break;
//         stringstream npss(npline);
//         int flowSrc,flowDst,num_paths;
//         vector<route_t*> *paths_rack_based;
//         if (npline.find_first_of("->") == string::npos) {
//           npss >> flowSrc >> flowDst >> num_paths;
//           paths_rack_based = new vector<route_t*>();
//           net_paths_rack_based[flowSrc][flowDst] = paths_rack_based;
//         } else {
//           string link;
//           int linkSrc,linkDst;
//           route_t *routeout = new route_t();
//           while (npss >> link) {
//             size_t found = link.find("->");
//             if (found != string::npos) {
//               linkSrc = stoi(link.substr(0,found));
//               linkDst = stoi(link.substr(found+2));
//               if (linkSrc<NL && linkDst<NL) {
//                 cout << "***Error: linkSrc<NL and linkDst<NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(NL) << endl;
//                 exit(1);
//               } else if (linkSrc<NL) { // nlp-nup
//                 linkDst -= NL;
//                 routeout->push_back(queues_nlp_nup[linkSrc][linkDst]);
//                 routeout->push_back(pipes_nlp_nup[linkSrc][linkDst]);
//               } else if (linkDst<NL) { // nup-nlp
//                 linkSrc -= NL;
//                 routeout->push_back(queues_nup_nlp[linkSrc][linkDst]);
//                 routeout->push_back(pipes_nup_nlp[linkSrc][linkDst]);
//               } else {
//                 cout << "***Error: linkSrc>NL and linkDst>NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(NL) << endl;
//                 exit(1);
//               }
//             }
//           }
//           paths_rack_based->push_back(routeout);
//         }
//       }
//       npfile.close();
//     } 
// 	  else {
//       cout << "***Error opening netpathfile: " << netpathfile << endl;
//       exit(1);
//     }

// 	// Initialize path_weights_rack_based
// 	int numintervals = 1;
// 	path_weights_rack_based = new vector < pair<int,double> > ***[numintervals];
// 	for (int k=0; k<numintervals; k++) {
// 		path_weights_rack_based[k] = new vector < pair<int,double> > **[NL];
// 		for (int i=0; i<NL; i++) {
// 			path_weights_rack_based[k][i] = new vector < pair<int,double> > *[NL];
// 			for (int j=0; j<NL; j++) {
// 				path_weights_rack_based[k][i][j] = new vector < pair<int,double> > ();
// 			}
// 		}
// 	}

// 	// Read pathweight from file
// 	for (int i=0; i<numintervals; i++) {
// 		ifstream pwfile(pathweightfile.c_str());
// 		string pwline;
// 		if (pwfile.is_open()){
// 			while(pwfile.good()){
// 				getline(pwfile, pwline);
// 				if (pwline.find_first_not_of(' ') == string::npos) break;
// 				stringstream ss(pwline);
//         string token;
// 				vector<string> tokens;
// 				while (getline(ss,token,',')) {
// 					tokens.push_back(token);
// 				}
// 				int flowSrc = stoi(tokens[0]);
// 				int flowDst = stoi(tokens[1]);
// 				int pid = stoi(tokens[2]);
// 				double weight = stod(tokens[3]);

// 				// int flowSrc,flowDst,pid,linkSrc,linkDst;
// 				// double weight;
// 				// ss >> flowSrc >> flowDst >> pid >> linkSrc >> linkDst >> weight;

// 				// if (flowSrc>=NL || flowDst>=NL) {
// 				// 	cout << "***Error: flowSrc>=NL || flowDst>=NL, flowSrc=" << itoa(flowSrc) << ", flowDst=" << itoa(flowDst) << ", NL=" << itoa(NL) << endl;
// 				// 	exit(1);
// 				// }
//         // if (linkSrc>=NL || linkDst<NL) {
// 				// 	cout << "***Error: linkSrc>=NL || linkDst<NL, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", NL=" << itoa(NL) << endl;
// 				// 	exit(1);
// 				// }
//         // linkDst-=NL;
// 				// // check whether netpathfile and pathweightfile are indeed matching
// 				// PacketSink *firstqueue = net_paths_rack_based[flowSrc][flowDst]->at(pid)->at(0);
// 				// if (firstqueue != queues_nlp_nup[linkSrc][linkDst]) {
// 				// 	cout << "***Error: netpathfile and pathweightfile mismatch, linkSrc=" << itoa(linkSrc) << ", linkDst=" << itoa(linkDst) << ", queue has name " << firstqueue->nodename() << endl;
// 				// 	exit(1);
// 				// }

// 				path_weights_rack_based[i][flowSrc][flowDst]->push_back(pair<int,double>(pid,weight));
// 			}
// 			pwfile.close();
// 		}
// 		else {
// 			cout << "***Error opening pathweightfile: " << pathweightfile << endl;
// 			exit(1);
// 		}
// 	}

//   // AnnC: solely for testing purpose
//   // std::cout << path_weights_rack_based[0][7][6]->at(0).first << ": " << path_weights_rack_based[0][7][6]->at(0).second << std::endl;
// 	// std::cout << path_weights_rack_based[0][42][48]->at(1).first << ": " << path_weights_rack_based[0][42][48]->at(1).second << std::endl;
// 	// std::cout << path_weights_rack_based[0][14][35]->at(2).first << ": " << path_weights_rack_based[0][14][35]->at(2).second << std::endl;
  
// #endif

// }