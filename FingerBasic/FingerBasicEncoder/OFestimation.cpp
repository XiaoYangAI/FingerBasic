#include "fingerBasic.h"

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