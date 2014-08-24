#include "FingerBasic.h"
#include "MinuDesc.h"
#include "MinuDescMCC.h"
#include "assert.h"

//int PARAM_MCC_Ns = 0;
//int PARAM_MCC_Nd = 0;

MinuDescMCC::MinuDescMCC(void)
{
	pMCC = 0;
	pMask = 0;
}

MinuDescMCC::~MinuDescMCC(void)
{
	delete [] pMCC;
	delete [] pMask;
}

bool MinuDescsMCC::LoadDesc(char* fname)
{
	FILE* file = fopen(fname, "rb");
	LoadDesc(file);
	fclose(file);
	return true;
}

bool MinuDescsMCC::LoadDesc(FILE* file)
{
	if(pDescs!=0)
		delete [] pDescs;

	num = 0;
	fread(&num, sizeof(int), 1, file);

	BYTE* pValid = new BYTE [num];
	BYTE* pMCC = new BYTE [num * PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns];
	BYTE* pMask = new BYTE [num * PARAM_MCC_Ns * PARAM_MCC_Ns];

	fread(pValid, 1, num, file);
	fread(pMCC, sizeof(BYTE), num * PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns, file);
	fread(pMask, 1, num * PARAM_MCC_Ns * PARAM_MCC_Ns, file);

	pDescs = new MinuDescMCC[num];
	MinuDescMCC* p = (MinuDescMCC*)pDescs;
	for(int i = 0; i < num; i++)
	{
		p[i].valid = pValid[i];
		p[i].pMask = new BYTE[PARAM_MCC_Ns * PARAM_MCC_Ns];
		memcpy(p[i].pMask, pMask+i*PARAM_MCC_Ns * PARAM_MCC_Ns, PARAM_MCC_Ns * PARAM_MCC_Ns);
		p[i].pMCC = new BYTE[PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns];
		memcpy(p[i].pMCC, pMCC+i*PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns, sizeof(BYTE) * PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns);
	}

	delete [] pValid;
	delete [] pMCC;
	delete [] pMask;

	return true;
}

void MinuDescsMCC::CreateDesc(Minutiae& minus, BYTE* ROI, int nHeight, int nWidth)
{
	num = minus.size();
	if(num==0)
		return;

	assert(pDescs==0);

	pDescs = new MinuDescMCC[num];

	double radius_s = 3.0*PARAM_MCC_sigma_s;
	double radius_s2 = radius_s*radius_s;
	double delta_s = (2.0*PARAM_MCC_R)/PARAM_MCC_Ns;
	double delta_d = 360.0/PARAM_MCC_Nd;

	// 
	int* X0 = new int[PARAM_MCC_Ns*PARAM_MCC_Ns];
	int* Y0 = new int[PARAM_MCC_Ns*PARAM_MCC_Ns];
	int* X = new int[PARAM_MCC_Ns*PARAM_MCC_Ns];
	int* Y = new int[PARAM_MCC_Ns*PARAM_MCC_Ns];
	bool* mask_cell0 = new bool[PARAM_MCC_Ns*PARAM_MCC_Ns];
	bool* mask_cell = new bool[PARAM_MCC_Ns*PARAM_MCC_Ns];
	memset(mask_cell0, 0, sizeof(bool)*PARAM_MCC_Ns*PARAM_MCC_Ns);
	memset(mask_cell, 0, sizeof(bool)*PARAM_MCC_Ns*PARAM_MCC_Ns);
	int *px = X0, *py = Y0;
	int cell_num = 0;
	for(int y = 0; y < PARAM_MCC_Ns; y++)
	{
		for(int x = 0; x < PARAM_MCC_Ns; x++, px++, py++)
		{
			*px = delta_s*(x-(PARAM_MCC_Ns-1)/2.0);
			*py = delta_s*(y-(PARAM_MCC_Ns-1)/2.0);
			if(((*px)*(*px) + (*py)*(*py))<=PARAM_MCC_R*PARAM_MCC_R)
			{
				mask_cell0[y*PARAM_MCC_Ns+x] = true;
				cell_num++;
			}
		}
	}

	double* d_phi = new double[PARAM_MCC_Nd];
	for(int i = 0; i < PARAM_MCC_Nd; i++)
		d_phi[i] = (i+0.5)*delta_d - 180;

	// 
	vector<int> valid_cell_x, valid_cell_y;
	valid_cell_x.reserve(cell_num);
	valid_cell_y.reserve(cell_num);
	for(int i = 0; i < num; i++)
	{
		MinuDescMCC* desc = (MinuDescMCC*)pDescs + i;
		desc->valid = 0;
		desc->pMask = new BYTE[PARAM_MCC_Ns * PARAM_MCC_Ns];
		memset(desc->pMask, 0, PARAM_MCC_Ns * PARAM_MCC_Ns);
		desc->pMCC = new BYTE[PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns];
		memset(desc->pMCC, 0, sizeof(BYTE) * PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns);

		double cosv = cos(minus[i].dir*M_PI/180);
		double sinv = sin(minus[i].dir*M_PI/180);
		int *px = X, *py = Y;
		for(int y = 0; y < PARAM_MCC_Ns; y++)
		{
			for(int x = 0; x < PARAM_MCC_Ns; x++, px++, py++)
			{
				if(mask_cell0[y*PARAM_MCC_Ns+x])
				{
					*px = minus[i].x + cosv*X0[y*PARAM_MCC_Ns+x] + sinv*Y0[y*PARAM_MCC_Ns+x];
					*py = minus[i].y - sinv*X0[y*PARAM_MCC_Ns+x] + cosv*Y0[y*PARAM_MCC_Ns+x];
					if(*px>=0 && *px<nWidth && *py>=0 && *py<nHeight && ROI[(*py)*nWidth+(*px)]>0)
					{
						mask_cell[y*PARAM_MCC_Ns+x] = true;
						valid_cell_x.push_back(x);
						valid_cell_y.push_back(y);
					}
				}
			}
		}

		if(valid_cell_x.size()<(PARAM_MCC_Min_VC*cell_num))
			continue;

		// number of neighboring minutiae
		vector<int> mids;
		vector<int> nx, ny;
		for(int j = 0; j < num; j++)
		{
			if(j==i)
				continue;
			int dx = abs(minus[i].x - minus[j].x);
			int dy = abs(minus[i].y - minus[j].y);
			int d = sqrt((double)dx*dx + dy*dy);
			if(d<=PARAM_MCC_R+radius_s)
			{
				mids.push_back(j);
				//nx.push_back();
			}
		}
		if(mids.size()<PARAM_MCC_Min_M)
			continue;

		// This descriptor is valid
		desc->valid = 1;

		for(int n = 0; n < valid_cell_x.size(); n++)
		{
			desc->pMask[valid_cell_y[n]*PARAM_MCC_Ns+valid_cell_x[n]] = 1;
		}

		//
		double* pMCC = new double[PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns];
		memset(pMCC, 0, sizeof(double) * PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns);
		for(int m = 0; m < mids.size(); m++)
		{
			int j = mids[m];

			double nm_x = (minus[j].x-minus[i].x)*cosv+(-minus[j].y+minus[i].y)*sinv;
			double nm_y = (-minus[j].y+minus[i].y)*cosv-(minus[j].x-minus[i].x)*sinv;
			int nm_dir = NormalizeMinuDir(minus[j].dir-minus[i].dir);
			double nm_xb =  (nm_x+PARAM_MCC_R)/delta_s - 0.5;
			double nm_yb = (-nm_y+PARAM_MCC_R)/delta_s - 0.5;

			int cont_x1 = (nm_x-radius_s+PARAM_MCC_R)/delta_s - 0.5;
			cont_x1 = max(cont_x1, 0);
			int cont_x2 = (nm_x+radius_s+PARAM_MCC_R)/delta_s - 0.5;
			cont_x2 = min(cont_x2, PARAM_MCC_Ns-1);
			int cont_y1 = (-nm_y-radius_s+PARAM_MCC_R)/delta_s - 0.5;
			cont_y1 = max(cont_y1, 0);
			int cont_y2 = (-nm_y+radius_s+PARAM_MCC_R)/delta_s - 0.5;
			cont_y2 = min(cont_y2, PARAM_MCC_Ns-1);

			double* cd = new double[PARAM_MCC_Nd];
			const double* dir_contri = (PARAM_MCC_Nd==6)? DIR_CONTRIBUTION_6:DIR_CONTRIBUTION_2;
			for(int k = 0; k < PARAM_MCC_Nd; k++)
			{
				int dir_diff = NormalizeMinuDir(minus[j].dir - minus[i].dir - d_phi[k]);
				cd[k] = dir_contri[dir_diff + 179];
			}

			for(int y = cont_y1; y < cont_y2; y++)
			{
				for(int x = cont_x1; x < cont_x2; x++)
				{
					if(!mask_cell[y*PARAM_MCC_Ns+x])
						continue;
					double dist_s = delta_s*delta_s*((x-nm_xb)*(x-nm_xb)+(y-nm_yb)*(y-nm_yb));
					if(dist_s>radius_s2)
						continue;
					int valid_cell_idx = y*PARAM_MCC_Ns+x;
					double cs = exp(-dist_s/(2*PARAM_MCC_sigma_s*PARAM_MCC_sigma_s))/(2.5066*PARAM_MCC_sigma_s);//2.5066=sqrt(2*pi)
					for(int k = 0; k < PARAM_MCC_Nd; k++)
					{
						pMCC[valid_cell_idx+k*PARAM_MCC_Ns*PARAM_MCC_Ns] += cs * cd[k];
					}
				}
			}

			delete [] cd;
		}

		BYTE* pMCCbyte = desc->pMCC;
		for(int y = 0; y < PARAM_MCC_Ns; y++)
		{
			for(int x = 0; x < PARAM_MCC_Ns; x++, px++, py++)
			{
				if(mask_cell[y*PARAM_MCC_Ns+x])
				{
					for(int k = 0; k < PARAM_MCC_Nd; k++)
					{
						int idx = y*PARAM_MCC_Ns+x+k*PARAM_MCC_Ns*PARAM_MCC_Ns;
						pMCCbyte[idx] = 255/(1+exp(-PARAM_MCC_tau_psi*(pMCC[idx]-PARAM_MCC_mu_psi)));
					}
				}
			}
		}

		delete [] pMCC;
	}

	delete [] X0;
	delete [] Y0;
	delete [] X;
	delete [] Y;
	delete [] mask_cell0;
	delete [] mask_cell;
	delete [] d_phi;
}

void MinuDescsMCC::SaveDesc(char* fname)
{
	FILE* file = fopen(fname, "wb");
	SaveDesc(file);
	fclose(file);
}

void MinuDescsMCC::SaveDesc(FILE* file)
{
	fwrite(&num, sizeof(int), 1, file);

	BYTE* pValid = new BYTE [num];
	BYTE* pMCC = new BYTE [num * PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns];
	BYTE* pMask = new BYTE [num * PARAM_MCC_Ns * PARAM_MCC_Ns];

	for(int i = 0; i < num; i++)
	{
		MinuDescMCC* desc = (MinuDescMCC*)pDescs + i;
		pValid[i] = desc->valid;
		memcpy(pMask+i*PARAM_MCC_Ns * PARAM_MCC_Ns, desc->pMask, PARAM_MCC_Ns * PARAM_MCC_Ns);
		memcpy(pMCC+i*PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns,desc->pMCC, sizeof(BYTE) * PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns);
	}

	fwrite(pValid, 1, num, file);
	fwrite(pMCC, sizeof(BYTE), num * PARAM_MCC_Nd * PARAM_MCC_Ns * PARAM_MCC_Ns, file);
	fwrite(pMask, 1, num * PARAM_MCC_Ns * PARAM_MCC_Ns, file);

	delete [] pValid;
	delete [] pMCC;
	delete [] pMask;
}

double MinuDescMCC::CompareDesc(MinuDesc* desc)
{
	MinuDescMCC* d = (MinuDescMCC*)desc;
	if(valid==0 || d->valid==0)
		return 0;
	
	int nValidPointNum = 0;
	for(int i = 0; i < PARAM_MCC_Ns*PARAM_MCC_Ns; i++)
	{
		if(pMask[i]==1 && d->pMask[i]==1)
			nValidPointNum++;
	}

	if(nValidPointNum < PARAM_MCC_Min_ME*PARAM_MCC_Ns*PARAM_MCC_Ns)
		return 0;

	int nc1 = 0, nc2 = 0, nc3 = 0;
	for(int i = 0; i < PARAM_MCC_Ns*PARAM_MCC_Ns; i++)
	{
		if(pMask[i]==1 && d->pMask[i]==1)
		{
			for(int k = 0; k < PARAM_MCC_Nd; k++)
			{
				int c1 = pMCC[k*PARAM_MCC_Ns*PARAM_MCC_Ns+i];
				int c2 = d->pMCC[k*PARAM_MCC_Ns*PARAM_MCC_Ns+i];
				nc1 += c1*c1;
				nc2 += c2*c2;
				nc3 += (c1-c2)*(c1-c2);
			}
		}
	}

	double s = sqrt((double)nc1) + sqrt((double)nc2);
	if(s==0)
		return 0;

	s = 1-sqrt((double)nc3)/s;
	return s;
}

void MinuDescMCC::ShowDesc()
{
	IplImage* img = cvCreateImage(cvSize(PARAM_MCC_Ns*PARAM_MCC_Nd, PARAM_MCC_Ns), 8, 1);
	cvSet(img, cvScalar(255));
	for(int k = 0; k < PARAM_MCC_Nd; k++)
	{
		for(int y = 0; y < PARAM_MCC_Ns; y++)
		{
			for(int x = 0; x < PARAM_MCC_Ns; x++)
			{
				IMG(img, y, k*PARAM_MCC_Ns+x) = pMCC[k*PARAM_MCC_Ns*PARAM_MCC_Ns+y*PARAM_MCC_Ns+x];
			}
		}
	}
	ShowImage("MCC", img, false, 10);
	cvReleaseImage(&img);
}
