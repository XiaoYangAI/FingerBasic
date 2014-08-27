#include "fingerBasic.h"


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