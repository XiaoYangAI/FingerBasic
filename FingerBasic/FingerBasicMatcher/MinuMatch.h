#pragma once

#include "FingerBasic.h"
#include "MinuMatch.h"
#include "MinuDesc.h"

class MinuMatch
{
public:
	MinuMatch(Minutiae* _pMinus[2], MinuDescs* _pDescs[2])
	{
		for(int i=0; i < 2; i++)
		{
			pMinus[i] = _pMinus[i]; 
			pDescs[i] = _pDescs[i];
		}
	};

	virtual ~MinuMatch(void) {};

	virtual void LocalMatch();
	virtual void SortLocalMatch();
	virtual void Pairing(unsigned int nAlignment) = 0;
	virtual void ShowSimilarityMatrix(bool wait=true);
	virtual void DrawTopLocalMatch(IplImage* img0, IplImage* img1, unsigned int n);
	virtual void DrawMatchPairs(IplImage* img0, IplImage* img1, vector<MinuPair>& pairs);

	Minutiae* pMinus[2];
	MinuDescs* pDescs[2];
	vector<unsigned char> simi;// similarity matrix
	vector<MinuPair> all_pairs;
	vector<vector<MinuPair>> match_pair_sets;
};
