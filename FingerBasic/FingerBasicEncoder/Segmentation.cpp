#include "fingerBasic.h"

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