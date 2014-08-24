#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4018)

#include "math.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "fingerBasic.h"


// Constants for thinning algorithm (internal use)
#define UP              0
#define DOWN            1
#define LEFT            2
#define RIGHT           3

int no_more[] = {0,0,0,0};

int next_thin_dir(int thin_dir)
{
	int next_dir; 

	switch(thin_dir)
	{
	case UP :
		next_dir=LEFT;
		break;
	case RIGHT :
		next_dir=DOWN;
		break;
	case DOWN :
		next_dir=UP;
		break;
	case LEFT :
		next_dir=RIGHT;
		break; 
	}
	return next_dir;
}

inline void check_neighbors_8simple_up(IplImage* img, IplImage* pMark, int y, int x)
{
	int neighbors = IMG(img,y-1,x-1) + IMG(img,y-1,x+1)
		+ IMG(img,y,x+1) + IMG(img,y+1,x+1) + IMG(img,y-1,x)
		+ IMG(img,y+1,x-1) + IMG(img,y,x-1);
	if(neighbors == 1)
		return;

	if( IMG(img,y,x+1) == 0 && IMG(img,y+1,x+1) == 1 )
		return;
	if( IMG(img,y,x-1) == 0 && IMG(img,y+1,x-1) == 1 )	
		return;
	if( IMG(img,y-1,x) == 0 )
		if( IMG(img,y,x+1) == 1 || IMG(img,y-1,x+1) == 1 )
			if(IMG(img,y,x-1) || IMG(img,y-1,x-1) == 1 )
				return;
	IMG(pMark,y,x) = 1;
}

inline void check_neighbors_8simple_down(IplImage* img, IplImage* pMark, int y, int x)
{
	int neighbors = IMG(img,y-1,x-1) + IMG(img,y-1,x+1)
		+ IMG(img,y,x+1) + IMG(img,y+1,x+1) + IMG(img,y+1,x)
		+ IMG(img,y+1,x-1) + IMG(img,y,x-1);
	if(neighbors == 1)
		return;

	if( IMG(img,y,x+1) == 0 && IMG(img,y-1,x+1) == 1 )
		return;
	if( IMG(img,y,x-1) == 0 && IMG(img,y-1,x-1) == 1 )	
		return;
	if( IMG(img,y+1,x) == 0 )
		if( IMG(img,y,x+1) == 1 || IMG(img,y+1,x+1) == 1 )
			if(IMG(img,y,x-1) ==1 || IMG(img,y+1,x-1) == 1 )
				return;

	IMG(pMark,y,x) = 1;
}

inline void check_neighbors_8simple_left(IplImage* img, IplImage* pMark, int y, int x)
{
	int neighbors = IMG(img,y-1,x-1) + IMG(img,y-1,x+1)
		+ IMG(img,y-1,x) + IMG(img,y+1,x+1) + IMG(img,y+1,x)
		+ IMG(img,y+1,x-1) + IMG(img,y,x-1);
	if(neighbors == 1)
		return;

	if( IMG(img,y-1,x) == 0 && IMG(img,y-1,x+1) == 1 )
		return;
	if( IMG(img,y+1,x) == 0 && IMG(img,y+1,x+1) == 1 )	
		return;
	if( IMG(img,y,x-1) == 0 )
		if( IMG(img,y-1,x-1) == 1 || IMG(img,y-1,x) == 1 )
			if(IMG(img,y+1,x-1) ==1 || IMG(img,y+1,x) == 1 )
				return;

	IMG(pMark,y,x) = 1;    			

}

inline void check_neighbors_8simple_right(IplImage* img, IplImage* pMark, int y, int x)
{
	int neighbors = IMG(img,y-1,x-1) + IMG(img,y-1,x+1)
		+ IMG(img,y-1,x) + IMG(img,y+1,x+1) + IMG(img,y+1,x)
		+ IMG(img,y+1,x-1) + IMG(img,y,x+1);
	if(neighbors == 1) 
		return;

	if( IMG(img,y-1,x) == 0 && IMG(img,y-1,x-1) == 1 )
		return;
	if( IMG(img,y+1,x) == 0 && IMG(img,y+1,x-1) == 1 )	
		return;
	if( IMG(img,y,x+1) == 0 )
		if( IMG(img,y-1,x) == 1 || IMG(img,y-1,x+1) == 1 )
			if(IMG(img,y+1,x) ==1 || IMG(img,y+1,x+1) == 1 )
				return;

	IMG(pMark,y,x) = 1;    			
}

void thin(IplImage* img, int thin_dir, IplImage* pMark)
{
	int height = img->height;
	int width = img->width;
	int	y, x;

	switch(thin_dir)
	{
	case DOWN :
		for(y = 1 ; y < height - 1; y ++)
			for(x = 1; x < width - 1; x ++)
				if(IMG(img,y,x) == 1)
					if(IMG(img,y-1,x) == 0)
						check_neighbors_8simple_down(img,pMark,y,x);
		break;
	case UP :
		for(y = 1 ; y < height - 1; y ++)
			for(x = 1; x < width - 1; x ++)
				if(IMG(img,y,x) == 1)
					if(IMG(img,y+1,x) == 0)
						check_neighbors_8simple_up(img,pMark,y,x);
		break;
	case LEFT :
		for(y = 1 ; y < height - 1; y ++)
			for(x = 1; x < width - 1; x ++)
				if(IMG(img,y,x) == 1)
					if(IMG(img,y,x+1) == 0)
						check_neighbors_8simple_left(img,pMark,y,x);
		break;
	case RIGHT :
		for(y = 1 ; y < height - 1; y ++)
			for(x = 1; x < width - 1; x ++)
				if(IMG(img,y,x) == 1)
					if(IMG(img,y,x-1) == 0)
						check_neighbors_8simple_right(img,pMark,y,x);
		break;
	}

	bool bChange = false;

	for(y = 1; y < height - 1; y ++)
	{
		for(x = 1; x < width - 1; x ++)
		{
			if(IMG(pMark,y,x) == 1)
			{
				IMG(img,y,x) = 0;
				IMG(pMark,y,x) = 0;
				bChange = true;
			}
		}
	}

	if(!bChange)
		no_more[thin_dir] = 1;
}

void RidgeThinning(IplImage* binaryImage)
{
	// Set ridge as 1, Valley as 0
	for(int y = 0; y < binaryImage->height; y ++)
	{
		for(int x = 0; x < binaryImage->width; x ++)
		{
			IMG(binaryImage, y, x) = (IMG(binaryImage, y, x)==0) ? 1:0;
		}
	}

	IplImage* pMark = cvCreateImage(cvGetSize(binaryImage), IPL_DEPTH_8U, 1);
	cvZero(pMark);

	for(int i = 0; i < 4; i++)       
		no_more[i] = 0;

	int thin_dir = UP;
	while(!(no_more[0] && no_more[1] && no_more[2] && no_more[3]))
	{
		if(!no_more[thin_dir]) 
			thin(binaryImage, thin_dir, pMark);
		thin_dir = next_thin_dir(thin_dir);
	}

	// Set ridge as 0, Valley as 255
	for(int y = 0; y < binaryImage->height; y ++)
	{
		for(int x = 0; x < binaryImage->width; x ++)
		{
			IMG(binaryImage, y, x) = (IMG(binaryImage, y, x)==1) ? 0:255;
		}
	}

	cvReleaseImage(&pMark);
}


void RidgeBinarization(IplImage* grayImage, IplImage* binaryImage)
{
	cvAdaptiveThreshold(grayImage, binaryImage, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 7);
}
