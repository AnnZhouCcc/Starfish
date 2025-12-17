///////////////////////////////////////////////////////////////////////////////
///  BhandariTopKDisjointPathsAlg.cpp
///  The implementation of Bhandari's algorithm to get the top k disjoint paths 
///  connecting a pair of vertices in a graph. 
///
///  @remarks <TODO: insert remarks here>
///
///
///////////////////////////////////////////////////////////////////////////////

#include <set>
#include <map>
#include <queue>
#include <iostream>
#include <algorithm>
#include <vector>
#include <climits>
#include <stdlib.h> 
#include "GraphElements.h"
#include "Graph.h"
#include "DijkstraShortestPathAlg.h"
#include "BhandariTopKDisjointPathsAlg.h"

using namespace std;

void BhandariTopKDisjointPathsAlg::clear()
{
	m_nGeneratedPathNum = 0;
	m_mpDerivationVertexIndex.clear();
	m_vResultList.clear();
	m_quPathCandidates.clear();
    m_stReversedEdge.clear();
}


//Use Modified BFS
BasePath* BhandariTopKDisjointPathsAlg::get_shortest_path( BaseVertex* pSource, BaseVertex* pTarget )
{
	DijkstraShortestPathAlg dijkstra_alg(m_pGraph);
	return dijkstra_alg.get_shortest_path(pSource, pTarget);
}


void BhandariTopKDisjointPathsAlg::KDisjointPaths(BaseVertex* pSource, 
	BaseVertex* pTarget, int top_k, vector<BasePath*>& result_list)
{
	m_pSourceVertex = pSource;
	m_pTargetVertex = pTarget;

    KDisjointPaths(top_k, result_list);
}


BasePath* BhandariTopKDisjointPathsAlg::bellman_ford_path(BaseVertex* src, BaseVertex* dest){

    int nV = m_pGraph->get_numVertices();
    map<int, int> dist;
    map<int, int> parent;
    map<int, bool> changed;

    vector<BaseVertex*>& vertices = m_pGraph->get_allVertices();
    for(auto v: vertices){
        dist[v->getID()] = INT_MAX;
        parent[v->getID()] = -1;
        changed[v->getID()] = false;
    }

    dist[src->getID()] = 0;
    parent[src->getID()] = src->getID();
    changed[src->getID()] = true;

    int srcpod = m_pGraph->get_part(src->getID());
    int dstpod = m_pGraph->get_part(dest->getID());
    //printf("Bellman Ford, src(pod): %d(%d), dest(pod): %d(%d), nVertices: %d \n", src->getID(), srcpod, dest->getID(), dstpod, nV);

    bool edge_relax;
    do{
        edge_relax = false;
        for(auto v: vertices){
            //printf("vertex v: %d \n", v->getID()); 
            int vpod = m_pGraph->get_part(v->getID());
            if(vpod != srcpod && vpod != dstpod) continue;
            if(!changed[v->getID()]) continue;
            changed[v->getID()] = false;
            set<BaseVertex*> nbrs;
            m_pGraph->get_adjacent_vertices(v, nbrs);
            for(auto nbr: nbrs){
                int nbrpod = m_pGraph->get_part(nbr->getID());
                if(nbrpod != srcpod && nbrpod != dstpod) continue;
              
              //relax edge
              if(dist[v->getID()]+1 < dist[nbr->getID()] && 
                 (m_stReversedEdge[nbr->getID()].find(v->getID()) == m_stReversedEdge[nbr->getID()].end())){
                dist[nbr->getID()] = dist[v->getID()]+1; 
                parent[nbr->getID()] = v->getID(); 
                edge_relax = true;
                changed[nbr->getID()] = true;
              }
            }
            for(auto revnbr: m_stReversedEdge[v->getID()]){
              //printf("v(dist): %d (%d), revnbr(dist): %d (%d) \n", v->getID(), dist[v->getID()], revnbr, dist[revnbr]);
              //relax edge
              if(dist[v->getID()]-1 < dist[revnbr]){
                dist[revnbr] = dist[v->getID()]-1; 
                parent[revnbr] = v->getID(); 
                changed[revnbr] = true;
                edge_relax = true;
              }
            }
        }
        //reverse-edges
    }
    while(edge_relax);

    if(parent[dest->getID()] == -1){
        //cout<<"Bellman ford path not found"<<endl; 
        return NULL;
    }

    std::vector<BaseVertex*> vertex_list;
    int curr = dest->getID();
    while(curr != src->getID()){
        vertex_list.push_back(m_pGraph->get_vertex(curr));    
        curr = parent[curr];
    }
    vertex_list.push_back(m_pGraph->get_vertex(src->getID()));    
    reverse(vertex_list.begin(), vertex_list.end());

    return new BasePath(vertex_list, 1);
}


void BhandariTopKDisjointPathsAlg::KDisjointPaths(int top_k, vector<BasePath*>& result_list){

	clear();
    int pathcount = 0;
    
    //cout<<"KDisjoint Paths. Numpaths:"<<top_k<<endl;

    while(pathcount < top_k){
        BasePath *cur_path = bellman_ford_path(m_pSourceVertex, m_pTargetVertex);
        //BasePath *cur_path = bfs_path(m_pSourceVertex, m_pTargetVertex);

        if(cur_path == NULL)
            break;
        else
            pathcount++;

        int path_length = cur_path->length();
        for(int i=0; i<path_length-1; ++i)
        {
            int a = cur_path->GetVertex(i)->getID(); 
            int b = cur_path->GetVertex(i+1)->getID(); 
            pair<int, int> edge(a, b);
            pair<int, int> edge_rev(b, a);

            //printf("path[%d] (%d, %d) \n", i, a, b);
        
            if(m_stReversedEdge[a].find(b) != m_stReversedEdge[a].end()){
                //printf("condition 1\n");
                m_stReversedEdge[a].erase(b);
            }
            else if(m_stReversedEdge[b].find(a) != m_stReversedEdge[b].end()){
                //printf("condition 2\n");
                cout<<"Bellman ford paths are not disjoint, exiting"<<endl;
                exit(0);
            }
            else{
                //printf("condition 3\n");
                m_stReversedEdge[b].insert(a);
            }
        }
        //cur_path->PrintOut(std::cout);
    }
    //printf("Found %d Disjoint paths\n", pathcount);
    //Find paths in graph of reversed edges
    partition_paths(pathcount, result_list);

/*
    //Checks if the path crosses third pod if graph parititioned into pods
    for(int i=0; i<pathcount; i++){
      bool diffpod = false;
      BasePath *path = result_list[i]; 
      int srcpod = m_pGraph->get_part((path->GetVertex(0))->getID());
      int dstpod = m_pGraph->get_part((path->GetVertex(path->length()-1))->getID());
      cout<<srcpod<<" : ";
      for(int i=1; i<path->length()-1; i++){
            int vt = (path->GetVertex(i))->getID(); 
            int pod = m_pGraph->get_part(vt);
            if(pod != srcpod && pod != dstpod)
                diffpod = true;
            cout<<pod<<" : ";
      }
      cout<<dstpod<<". Path :"<<diffpod<<endl;
    }
*/    
}




void BhandariTopKDisjointPathsAlg::partition_paths(int numpaths, vector<BasePath*>& result_list){

    int srcid = m_pSourceVertex->getID();
    int destid = m_pTargetVertex->getID();
    int dist = INT_MAX;
    for(int i=0; i<numpaths; i++){
        //Find shortest path from source to dest
        BasePath *retpath = shortest_path_revgraph(srcid, destid);
        dist = min(dist, retpath->length());
        result_list.push_back(retpath);
        //retpath->PrintOut(std::cout);
    }
    /*
    for(int i=numpaths-1; i>=0; i--){
        BasePath *retpath = result_list[i];
        //eliminate long paths
        if(retpath->length() > dist+1)
            result_list.erase(result_list.begin()+i);
    }
    */
}



BasePath* BhandariTopKDisjointPathsAlg::shortest_path_revgraph(int srcid, int destid){

/*
    vector<BaseVertex*>& vertices = m_pGraph->get_allVertices();
    for(auto v: vertices){
        printf("vertex v: %d \n", v->getID()); 
        for(auto revnbr: m_stReversedEdge[v->getID()]){
            printf("v: %d, revnbr: %d \n", v->getID(), revnbr);
        }
    }
*/
    //printf("shortest path revgraph, src: %d, dest: %d \n", srcid, destid);

    queue<int> bfsq;
    map<int, int> bfspar;

    bfsq.push(destid);
    bfspar[destid] = destid;

    while(!bfsq.empty()){
        int node = bfsq.front();
        bfsq.pop();
        if(node == srcid) break; //bfs successful
        for(int nbr : m_stReversedEdge[node]){
            if(bfspar.find(nbr) == bfspar.end()){
                bfspar[nbr] = node;
                bfsq.push(nbr);
            }
        }
    }

    if(bfspar.find(srcid) == bfspar.end()){
        cout<<"BFS path search failed for reverse graph in Bhandari's algorithm"<<endl; 
        exit(0);
    }

    std::vector<BaseVertex*> vertex_list;
    int curr = srcid;
    while(curr != destid){
        vertex_list.push_back(m_pGraph->get_vertex(curr));    
        int next = bfspar[curr];
        m_stReversedEdge[next].erase(curr);
        curr = next;
    }
    vertex_list.push_back(m_pGraph->get_vertex(destid));    
    return new BasePath(vertex_list, 1);
}





