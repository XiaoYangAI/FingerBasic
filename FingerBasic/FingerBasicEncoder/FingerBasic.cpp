#include "fingerBasic.h"

bool LoadMinuTxt(char* fname, Minutiae& minus)
{
	FILE* file = fopen(fname, "rb");
	Minu minu;
	int count = 0;
	int temp;
	while(fscanf(file, "%d", &temp)>0)
	{
		switch(count%3)
		{
		case 0:
			minu.x = temp;
			break;
		case 1:
			minu.y = temp;
			break;
		case 2:
			minu.dir = temp;
			//break;
		//case 3:
		//	minu.type = temp;
		//	break;
		//case 4:
		//	minu.quality = temp;
			minu.type = 1;
			minu.quality = 1;
			minus.push_back(minu);
			break;
		}
		count++;
	}
	fclose(file);
	return true;
}

bool LoadMinu(char* fname, Minutiae& minus, MinuFileType type)
{
	if(type==MINUFILE_TEXT3)
		LoadMinuTxt(fname, minus);
	return true;
}

void ShowImage(char* name, IplImage* img, bool wait, double scale)
{
	cvNamedWindow(name);
	if(scale==1)
		cvShowImage(name, img);
	else
	{
		IplImage* newImage = cvCreateImage(cvSize((int)(img->width * scale), (int)(img->height * scale)), img->depth, img->nChannels);
		cvResize(img, newImage, CV_INTER_NN);
		cvShowImage(name, newImage);
		cvReleaseImage(&newImage);
	}
	if(wait)
		cvWaitKey();
}

void DrawImage(IplImage* dst, IplImage* src)
{
	assert(dst!=0);
	assert(src!=0);

	for(int y = 0; y < src->height; y++)
	{
		for(int x = 0; x < src->width; x++)
		{
			CV_IMAGE_ELEM(dst, BYTE, y, x*3)   = CV_IMAGE_ELEM(src, BYTE, y, x);
			CV_IMAGE_ELEM(dst, BYTE, y, x*3+1) = CV_IMAGE_ELEM(src, BYTE, y, x);
			CV_IMAGE_ELEM(dst, BYTE, y, x*3+2) = CV_IMAGE_ELEM(src, BYTE, y, x);
		}
	}
}

void DrawOF(IplImage* img, IplImage* orientation_field, int block_size, CvScalar color)
{
	CvPoint pt1, pt2;
	for(int y = 0; y < orientation_field->height; y++)
	{
		for(int x = 0; x < orientation_field->width; x++)
		{
			char dir = CV_IMAGE_ELEM(orientation_field, char, y, x);
			if(dir!=91)
			{
				int cx = x*block_size + block_size/2;
				int cy = y*block_size + block_size/2;
				pt1.x = cx - (int)(cos(dir * M_PI/180) * 0.5 * (block_size-2));
				pt1.y = cy + (int)(sin(dir * M_PI/180) * 0.5 * (block_size-2));
				pt2.x = cx + (int)(cos(dir * M_PI/180) * 0.5 * (block_size-2));
				pt2.y = cy - (int)(sin(dir * M_PI/180) * 0.5 * (block_size-2));
				cvLine(img, pt1, pt2, C_DBLUE, 2);
			}
		}
	}
}

void DrawMinutiae(IplImage* img, Minutiae& minus, CvScalar color, bool show_index, bool box_shape, unsigned int radius, unsigned int tail_length, unsigned int line_width)
{
	for(int i = 0; i < minus.size(); i++)
	{
		if(show_index)
			DrawMinu(img, minus[i], i, color, box_shape, radius, tail_length, line_width);
		else
			DrawMinu(img, minus[i], -1, color, box_shape, radius, tail_length, line_width);
	}
}

void DrawMinu(IplImage* img, Minu minu, int mid, CvScalar color, bool box_shape, unsigned int radius, unsigned int tail_length, unsigned int line_width)
{
	CvPoint center, pt;
	center.x = minu.x;
	center.y = minu.y;
	pt.x = (int)(center.x+tail_length*cos(M_PI*minu.dir/180));
	pt.y = (int)(center.y-tail_length*sin(M_PI*minu.dir/180));
	cvRectangle(img, cvPoint(center.x-3,center.y-3), cvPoint(center.x+3,center.y+3), color);
	cvLine(img, center, pt, color);

	if(mid!=-1)
	{
		char text[10] = "";
		sprintf(text, "%d", mid);
		CvFont font;
		cvInitFont(&font, CV_FONT_VECTOR0, 0.5, 0.5);
		if(minu.dir>180)
		{
			pt.x = center.x;
			pt.y = center.y-3;
		}
		cvPutText(img, text, pt, &font, color);
	}
}

int GetNearMinu(Minutiae& minus, int x, int y)
{
	int mid = -1;
	for(int n = 0; n < minus.size(); n++)
	{
		if(abs(minus[n].x-x)<4 && abs(minus[n].y-y)<4)
		{
			mid = n;
			return mid;
		}
	}
	return mid;
}

IplImage* ComputeOFGradient(IplImage* fpImage, int block_size)
{
	int width = fpImage->width;
	int height = fpImage->height;
	int blockWidth = (int)ceil(width/block_size);
	int blockHeight = (int)ceil(height/block_size);
	IplImage* blockDirImage = cvCreateImage(cvSize(blockWidth, blockHeight), IPL_DEPTH_8U, 1);

	// gradient
	IplImage *Gx, *Gy, *Gxy;
	Gx = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
	Gy = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
	Gxy = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);

	// cvSobel only supports float image.
	IplImage* imgSrcF = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
	cvConvert(fpImage, imgSrcF);

	cvSobel(imgSrcF, Gx, 1, 0, 3);
	cvSobel(imgSrcF, Gy, 0, 1, 3);
	cvReleaseImage(&imgSrcF);

	// Set border to 0
	for(int i = 0; i < 5; i++)
	{
		for(int j = 0; j < width; j++)
		{
			CV_IMAGE_ELEM(Gx, float, i, j) = 0;
			CV_IMAGE_ELEM(Gy, float, i, j) = 0;
		}
	}
	for(int i = height-5; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			CV_IMAGE_ELEM(Gx, float, i, j) = 0;
			CV_IMAGE_ELEM(Gy, float, i, j) = 0;
		}
	}
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < 5; j++)
		{
			CV_IMAGE_ELEM(Gx, float, i, j) = 0;
			CV_IMAGE_ELEM(Gy, float, i, j) = 0;
		}
	}
	for(int i = 0; i < height; i++)
	{
		for(int j = width-5; j < width; j++)
		{
			CV_IMAGE_ELEM(Gx, float, i, j) = 0;
			CV_IMAGE_ELEM(Gy, float, i, j) = 0;
		}
	}

	cvMul(Gx, Gy, Gxy);
	cvMul(Gx, Gx, Gx);
	cvMul(Gy, Gy, Gy);

	double temp1, temp2;
	int angle;

	const int win_size = 31;
	cvSmooth(Gx, Gx, CV_BLUR_NO_SCALE, win_size, win_size);
	cvSmooth(Gy, Gy, CV_BLUR_NO_SCALE, win_size, win_size);
	cvSmooth(Gxy, Gxy, CV_BLUR_NO_SCALE, win_size, win_size);
	//cvSmooth(mod, mod, CV_BLUR_NO_SCALE, win_size, win_size);

	// Since calling atan2() for each pixel is low, we only call it for each block.

	for(int by = 0; by < blockHeight; by++)
	{
		for(int bx = 0; bx < blockWidth; bx++)
		{
			int x = bx*block_size+block_size/2;
			int y = by*block_size+block_size/2;
			temp1 = 2 * CV_IMAGE_ELEM(Gxy, float, y, x);
			temp2 = CV_IMAGE_ELEM(Gx, float, y, x) - CV_IMAGE_ELEM(Gy, float, y, x);
			angle = 90 + (int)(atan2(-temp1, temp2) * 0.5 * 180 / M_PI);// [0, 180]
			if(angle > 90)
				angle = angle - 180;
			CV_IMAGE_ELEM(blockDirImage, char, by, bx) = angle;
		}
	}

	cvReleaseImage(&Gx);
	cvReleaseImage(&Gy);
	cvReleaseImage(&Gxy);

	return blockDirImage;
}

IplImage* ForegroundSegmentSimple(IplImage* fpImage, int block_size, int minStd)
{
	int width = fpImage->width;
	int height = fpImage->height;
	int blockWidth = (int)ceil(width/block_size);
	int blockHeight = (int)ceil(height/block_size);
	IplImage* blockMaskImage = cvCreateImage(cvSize(blockWidth, blockHeight), IPL_DEPTH_8U, 1);
	cvSet(blockMaskImage, cvScalar(0));

	int minVar = minStd * minStd;

	for(int by = 0; by < blockHeight; by++)
	{
		for(int bx = 0; bx < blockWidth; bx++)
		{
			int mn = 0, num = 0, var = 0, value;
			for(int r = 0; r < block_size; r++)
			{
				for(int c = 0; c < block_size; c++)
				{
					int y = by*block_size+r;
					int x = bx*block_size+c;
					if(x<width && y<height)
					{
						value = CV_IMAGE_ELEM(fpImage, BYTE, y, x);
						mn += value;
						var += value*value;
						num++;
					}
				}
			}

			mn = mn/num;
			var = var/num - mn*mn;
			if(var>=minVar)
				CV_IMAGE_ELEM(blockMaskImage, BYTE, by, bx) = 1;
			else
				CV_IMAGE_ELEM(blockMaskImage, BYTE, by, bx) = 0;
		}
	}

	return blockMaskImage;
}

void MaskOF(IplImage* blockMaskImg, IplImage* blockDirImg)
{
	for(int by = 0; by < blockMaskImg->height; by++)
	{
		for(int bx = 0; bx < blockMaskImg->width; bx++)
		{
			if(CV_IMAGE_ELEM(blockMaskImg, BYTE, by, bx)==0)
				CV_IMAGE_ELEM(blockDirImg, char, by, bx) = 91;
		}
	}
}

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

void TransMinu(Minu& pt, Tran tran)
{
	int ox, oy;
	ox = pt.x;
	oy = pt.y;
	pt.x = ox*tran.rotC+oy*tran.rotS+tran.dx;
	pt.y =-ox*tran.rotS+oy*tran.rotC+tran.dy;
}

void TransMinus(vector<Minu>& minuz, Tran tran)
{
	for(int i = 0; i < minuz.size(); i++)
	{
		TransMinu(minuz[i], tran);
	}
}
