#include "FingerBasic.h"
#include "MinuDesc.h"
#include "MinuMatch.h"
#include <algorithm>

void MinuMatch::LocalMatch()
{
	int n1 = pMinus[0]->size();
	int n2 = pMinus[1]->size();
	simi.resize(n1 * n2);
	for(int i = 0; i < n1; i++)
	{
		for(int j = 0; j < n2; j++)
		{
			MinuDesc* mcc1 = pDescs[0]->GetDesc(i);
			MinuDesc* mcc2 = pDescs[1]->GetDesc(j);
			simi[i*n2+j] = 255 * mcc1->CompareDesc(mcc2);
		}
	}
}

void MinuMatch::SortLocalMatch()
{
	int n1 = pMinus[0]->size();
	int n2 = pMinus[1]->size();
	all_pairs.resize(n1*n2);
	for(int i = 0; i < n1; i++)
	{
		for(int j = 0; j < n2; j++)
		{
			all_pairs[i*n2+j] = MinuPair(i, j, simi[i*n2+j]);
		}
	}
	sort(all_pairs.begin(), all_pairs.end());
}

void MinuMatch::ShowSimilarityMatrix(bool wait)
{
	int n1 = pMinus[0]->size();
	int n2 = pMinus[1]->size();
	IplImage* img = cvCreateImage(cvSize(n2, n1), 8, 1);
	for(int i = 0; i < n1; i++)
	{
		for(int j = 0; j < n2; j++)
		{
			IMG(img, i, j) = simi[i*n2+j];
		}
	}
	ShowImage("Similarity matrix", img, wait, 5);
	cvReleaseImage(&img);
}

void MinuMatch::DrawTopLocalMatch(IplImage* img0, IplImage* img1, unsigned int n)
{
	for(int i = 0; i < n; i++)
	{
		DrawMinu(img0, (*(pMinus[0]))[all_pairs[i].index[0]], i, C_RED);
		DrawMinu(img1, (*(pMinus[1]))[all_pairs[i].index[1]], i, C_RED);
	}
}

void MinuMatch::DrawMatchPairs(IplImage* img0, IplImage* img1, vector<MinuPair>& pairs)
{
	for(int i = 0; i < pairs.size(); i++)
	{
		DrawMinu(img0, (*(pMinus[0]))[pairs[i].index[0]], i, C_RED);
		DrawMinu(img1, (*(pMinus[1]))[pairs[i].index[1]], i, C_RED);
	}
}
