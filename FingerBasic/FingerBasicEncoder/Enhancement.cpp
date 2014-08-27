#include "fingerBasic.h"

IplImage* RidgeEnhanceGabor(IplImage* fpImage, IplImage* blockOF, int block_size, GaborFilter* filter)
{
	int height = fpImage->height;
	int width = fpImage->width;
	IplImage* enhancedImage = cvCreateImage(cvGetSize(fpImage), IPL_DEPTH_8U, 1);
	int* pData = new int[height*width];
	memset(pData, 0, sizeof(int)*height*width);

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			int by = y/block_size;
			int bx = x/block_size;
			int dir = CV_IMAGE_ELEM(blockOF, char, by, bx);
			if(dir==91)// background
				continue;
			if(dir<0)
				dir +=180;
			dir = dir/DIR_INTERVAL;

			int period = 9;
			if(period < MIN_PERIOD || period>MAX_PERIOD)
				continue;

			char* pDX = filter->GaborDX[dir][period-MIN_PERIOD];
			char* pDY = filter->GaborDY[dir][period-MIN_PERIOD];
			int* pGabor = filter->Gabor;

			int num = 0;
			int temp = 0;
			bool bOut = false;
			for(int m = -FILTER_RADIUS; m <= FILTER_RADIUS; m++)
			{
				for(int n = -FILTER_RADIUS; n <= FILTER_RADIUS; n++, pDX++, pDY++, pGabor++)
				{
					int x2 = x + *pDX;
					int y2 = y + *pDY;
					if(IN_IMAGE(x2, y2, width, height))
					{
						temp += (*pGabor) * (IMG(fpImage, y2, x2));
						num++;
					}
					else
					{
						bOut = true;
						break;
						//temp += (*pGabor) * PADDING_VALUE;// Padding with white background
					}
				}
				if(bOut)
					break;
			}
			if(!bOut)
				pData[y*width+x] = temp/num;
		}
	}

	// Normalize enhanced image to [0, 255]
	pair<int*, int*> p = std::minmax_element(pData, pData+height*width);
	int nMin = *(p.first);
	int nMax = *(p.second);
	double ratio = 255.0/(nMax - nMin);
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			IMG(enhancedImage, y, x) = (BYTE)((pData[y*width+x]-nMin)*ratio);
		}
	}

	delete [] pData;
	return enhancedImage;
}

// Based on 'CreateSampledGaborFilter.m'
GaborFilter* GaborFilterCreate()
{
	GaborFilter* filter = new GaborFilter;

	// Allocate
	for(int d = 0; d < FILTER_DIRS; d++)
	{
		for(int p = 0; p < FILTER_PERIODS; p++)
		{
			filter->GaborDX[d][p] = new char[FILTER_SIZE*FILTER_SIZE];
			filter->GaborDY[d][p] = new char[FILTER_SIZE*FILTER_SIZE];
		}
	}

	filter->Gabor = new int[FILTER_SIZE*FILTER_SIZE];

	// Positions of sampling points
	for(int d = 0; d < FILTER_DIRS; d++)
	{
		for(int p = 0; p < FILTER_PERIODS; p++)
		{
			char* pDX = filter->GaborDX[d][p];
			char* pDY = filter->GaborDY[d][p];
			double theta = (d + 0.5) * DIR_INTERVAL * M_PI / 180;
			double cosv = cos(theta);
			double sinv = sin(theta);
			double scale = ((double)p+MIN_PERIOD)/BASE_PERIOD;
			for(int dy = -FILTER_RADIUS; dy <= FILTER_RADIUS; dy++)
			{
				for(int dx = -FILTER_RADIUS; dx <= FILTER_RADIUS; dx++, pDX++, pDY++)
				{
					*pDX = (char)(( dx*cosv + dy*sinv)*scale);
					*pDY = (char)((-dx*sinv + dy*cosv)*scale);
				}
			}
		}
	}

	// Coefficents of filter
	int* pGabor = filter->Gabor;
	for(int dy = -FILTER_RADIUS; dy <= FILTER_RADIUS; dy++)
	{
		for(int dx = -FILTER_RADIUS; dx <= FILTER_RADIUS; dx++, pGabor++)
		{
			// (1000*255)*(FILTER_SIZE*FILTER_SIZE) < 2^31
			*pGabor = (int)(1000*(exp(-0.5*(dy*dy/(FILTER_SIGMA*FILTER_SIGMA)+dx*dx/(FILTER_SIGMA*FILTER_SIGMA)))*cos(2*M_PI*dy/BASE_PERIOD)));
		}
	}

	return filter;
}

void GaborFilterRelease(GaborFilter* filter)
{
	for(int d = 0; d < FILTER_DIRS; d++)
	{
		for(int p = 0; p < FILTER_PERIODS; p++)
		{
			delete [] (filter->GaborDX[d][p]);
			delete [] (filter->GaborDY[d][p]);
		}
	}
	delete [] (filter->Gabor);
	delete filter;
}


