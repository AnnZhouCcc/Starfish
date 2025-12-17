///////////////////////////////////////////////////////////////////////////////
///  BhandariTopKDisjointPathsAlg.h
///
///  The implementation of Bhandari's algorithm to get the top k disjoint paths 
///  connecting a pair of vertices in a graph. 
///
///  @remarks <TODO: insert remarks here>
///
///  @author Vipul Harsh
/// 
///
///////////////////////////////////////////////////////////////////////////////

#pragma once
using namespace std;

class BhandariTopKDisjointPathsAlg
{
	Graph* m_pGraph;

	vector<BasePath*> m_vResultList;
	map<BasePath*, BaseVertex*> m_mpDerivationVertexIndex;
	multiset<BasePath*, WeightLess<BasePath> > m_quPathCandidates;

	BaseVertex* m_pSourceVertex;
	BaseVertex* m_pTargetVertex;

    map<int, set<int> > m_stReversedEdge;

	int m_nGeneratedPathNum;

private:


public:

//	BhandariTopKDisjointPathsAlg(Graph& graph)
//	{
//		BhandariTopKDisjointPathsAlg(graph, NULL, NULL);
//	}

	BhandariTopKDisjointPathsAlg(Graph* graph, BaseVertex* pSource, BaseVertex* pTarget)
		:m_pSourceVertex(pSource), m_pTargetVertex(pTarget)
	{
		m_pGraph = graph;
        clear();
	}

	~BhandariTopKDisjointPathsAlg(void){
		clear();
	}

	void clear();

	void KDisjointPaths(int top_k, vector<BasePath*>&);
	void KDisjointPaths(BaseVertex* pSource, BaseVertex* pTarget, int top_k, 
		vector<BasePath*>&);

    BasePath* bellman_ford_path(BaseVertex* src, BaseVertex* dest);
    BasePath* shortest_path_revgraph(int srcid, int destid);

	BasePath* get_shortest_path(BaseVertex* pSource, BaseVertex* pTarget);

    void partition_paths(int numpaths, vector<BasePath*>& result_list);
};
