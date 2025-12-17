CC = g++
CFLAGS = -std=c++11 -O3 -w -Wall -Wno-deprecated -Wno-unused-variable

BASEDIR=/path/to/your/dir/Starfish/src/emp
CRT=$(BASEDIR)/datacentre
CRT_1UP=$(BASEDIR)
INCLUDE= -I/$(CRT_1UP) -I$(CRT)
LIB=-L..


all:	eval

eval: main_eval.o ../libhtsim.a leaf_spine_topology.o rand_regular_topology.o dragon_fly_topology.o connection_matrix_eval.o Graph.o YenTopKShortestPathsAlg.o BhandariTopKDisjointPathsAlg.o DijkstraShortestPathAlg.o
	$(CC) $(INCLUDE) $(CFLAGS)  main_eval.o leaf_spine_topology.o rand_regular_topology.o dragon_fly_topology.o connection_matrix_eval.o Graph.o YenTopKShortestPathsAlg.o BhandariTopKDisjointPathsAlg.o DijkstraShortestPathAlg.o $(LIB) -lhtsim -o eval


main_eval.o: main_eval.cpp main.h
	$(CC) $(INCLUDE) $(CFLAGS) -c main_eval.cpp 

connection_matrix_eval.o: connection_matrix_eval.cpp main.h 
	$(CC) $(INCLUDE) $(CFLAGS) -c connection_matrix_eval.cpp 

leaf_spine_topology.o: leaf_spine_topology.cpp leaf_spine_topology.h topology.h  main.h
	$(CC) $(INCLUDE) $(CFLAGS) -c leaf_spine_topology.cpp

dragon_fly_topology.o: dragon_fly_topology.cpp dragon_fly_topology.h topology.h  main.h
	$(CC) $(INCLUDE) $(CFLAGS) -c dragon_fly_topology.cpp

rand_regular_topology.o: rand_regular_topology.cpp rand_regular_topology.h topology.h Graph.h GraphElements.h main.h
	$(CC) $(INCLUDE) $(CFLAGS) -c rand_regular_topology.cpp

DijkstraShortestPathAlg.o: DijkstraShortestPathAlg.cpp DijkstraShortestPathAlg.h
	$(CC) $(INCLUDE) $(CFLAGS) -c DijkstraShortestPathAlg.cpp

Graph.o: Graph.cpp Graph.h GraphElements.h main.h
	$(CC) $(INCLUDE) $(CFLAGS) -c Graph.cpp

YenTopKShortestPathsAlg.o: YenTopKShortestPathsAlg.cpp YenTopKShortestPathsAlg.h
	$(CC) $(INCLUDE) $(CFLAGS) -c YenTopKShortestPathsAlg.cpp

BhandariTopKDisjointPathsAlg.o: BhandariTopKDisjointPathsAlg.cpp BhandariTopKDisjointPathsAlg.h
	$(CC) $(INCLUDE) $(CFLAGS) -c BhandariTopKDisjointPathsAlg.cpp

clean:	
	rm *.o eval
