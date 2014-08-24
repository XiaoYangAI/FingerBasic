#include "MinuMatchTico.h"
#include "MinuDescMCC.h"

const int MAX_ROTATION = 90;	// maximum rotation between two impressions
const double R_LOW = 80, R_HIGH = 80+25;
const double LEN_LOW = 20, LEN_HIGH = 70;
const double DIFF_LEN_LOW = 15, DIFF_LEN_HIGH = 25;
const double DIFF_BETA_LOW = 45, DIFF_BETA_HIGH = 65;

const int BOUND_NUM = 3;
const double LEN_BOUND[BOUND_NUM] = {20, 100, 250};
const double DIFF_POS[BOUND_NUM] = {16, 30, 35};//{12, 20, 25};

void MinuMatchTico::Pairing(unsigned int nAlignment)
{
	int n1 = pMinus[0]->size();
	int n2 = pMinus[1]->size();
	// Assume all_pairs have been sorted
	vector<MinuPair> initPairs(all_pairs.begin(), all_pairs.begin()+nAlignment);
	for(unsigned int i = 0; i < nAlignment; i++)
	{
		// Alignment
		Minu initMinu[2];
		initMinu[0] = (*(pMinus[0]))[initPairs[i].index[0]];
		initMinu[1] = (*(pMinus[1]))[initPairs[i].index[1]];

		Tran tran;
		int rot = NormalizeMinuDir(initMinu[1].dir-initMinu[0].dir);
		tran.rotC = cos(rot*M_PI/180);
		tran.rotS = sin(rot*M_PI/180);
		tran.dx = initMinu[1].x - initMinu[0].x * tran.rotC - initMinu[0].y * tran.rotS;
		tran.dy = initMinu[1].y + initMinu[0].x * tran.rotS - initMinu[0].y * tran.rotC;

		vector<Minu> minuz(*(pMinus[0]));
		TransMinus(minuz, tran);
		for(unsigned int m = 0; m < minuz.size(); m++)
			minuz[m].dir = NormalizeMinuDir(minuz[m].dir + rot);

		// Match
		vector<MinuPair> match_pairs;
		match_pairs.push_back(MinuPair(initPairs[i].index[0], initPairs[i].index[1], 0));

		vector<bool> used_flag1(n1, false), used_flag2(n2, false);
		used_flag1[initPairs[i].index[0]] = true;
		used_flag2[initPairs[i].index[1]] = true;

		for(unsigned int m = 0; m < all_pairs.size(); m++)
		{
			if(all_pairs[m].score==0)
				break;

			//if(g_pairRank[allPairs[m].ii*g_nMinu[1]+allPairs[m].jj]>g_thRank)
			//	continue;

			//if(g_minuSimi[allPairs[m].ii*g_nMinu[1]+allPairs[m].jj]<=g_thSimi)
			//	continue;

			if(all_pairs[m].score<=10)
				break;

			int ii = all_pairs[m].index[0];
			int jj = all_pairs[m].index[1];
			if(used_flag1[ii] || used_flag2[jj])
				continue;

			// geometrically compatible with initial pair
			double dx = minuz[ii].x - (*(pMinus[1]))[jj].x;
			double dy = minuz[ii].y - (*(pMinus[1]))[jj].y;
			double diff_pos = sqrt(dx*dx+dy*dy);
			int ddir = minuz[ii].dir - (*(pMinus[1]))[jj].dir;

			dx = (*(pMinus[0]))[initPairs[i].index[0]].x - (*(pMinus[0]))[ii].x;
			dy = (*(pMinus[0]))[initPairs[i].index[0]].y - (*(pMinus[0]))[ii].y;
			double lLen = sqrt(dx*dx+dy*dy);

			double diff_pos_th;
			if(lLen <= LEN_BOUND[0])
				diff_pos_th = DIFF_POS[0];
			else if(lLen >= LEN_BOUND[BOUND_NUM-1])
				diff_pos_th = DIFF_POS[BOUND_NUM-1];
			else if(lLen<=LEN_BOUND[1])
				diff_pos_th = DIFF_POS[0] + (lLen-LEN_BOUND[0]) * (DIFF_POS[1]-DIFF_POS[0])/(LEN_BOUND[1]-LEN_BOUND[0]);
			else
				diff_pos_th = DIFF_POS[1] + (lLen-LEN_BOUND[1]) * (DIFF_POS[2]-DIFF_POS[1])/(LEN_BOUND[2]-LEN_BOUND[1]);

			if(diff_pos<=diff_pos_th && abs(NormalizeMinuDir(ddir))<=30)
			{
				used_flag1[ii] = true;
				used_flag2[jj] = true;
				match_pairs.push_back(MinuPair(ii, jj, 0));
				if((int)match_pairs.size() >= min(n1, n2))
					break;
			}
		}

		match_pair_sets.push_back(match_pairs);
	}
}
