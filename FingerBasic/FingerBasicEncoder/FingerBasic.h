#pragma once

#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4018)

#include "math.h"
#include <vector>
#include "opencv/cv.h"
#include "opencv/highgui.h"

using namespace std;

typedef unsigned char BYTE;


// Macro functions
#define IMG(img,y,x) *((BYTE*)(((img)->imageData)+(y)*((img)->widthStep)+(x)))
#define IN_IMAGE(x,y,w,h) (x>=0 && x<w && y>=0 && y<h)
#define SAFE_DELETE(p) {if(p) delete (p); (p) = 0;}
#define SAFE_DELETE_VECTOR(p) {if(p) delete [](p); (p) = 0;}

typedef struct _IndexPair
{
	short index[2];
}IndexPair;

class MinuPair
{
public:

	MinuPair()
	{
		index[0] = -1; index[1] = -1; score = 0;
	}

	MinuPair(short _i, short _j, float _s)
	{
		index[0] = _i; index[1] = _j; score = _s;
	}

	bool operator < (const MinuPair &mp)const {
		return score > mp.score;// for descending sort
	}

	short index[2];// index of minutiae
	float score;// similarity of the two minutiae
};


typedef struct _Minu
{
public:
	short	x;
	short	y;
	short	dir;
	BYTE	type;		// type=1 (termination), type=3 (bifurcation)
	BYTE	quality;
}Minu;

typedef vector<Minu> Minutiae;

typedef struct
{
	double dx,dy,rotS,rotC,differ;
	short xcenter,ycenter;
}Tran;

void TransMinu(Minu& pt, Tran tran);
void TransMinus(vector<Minu>& minuz, Tran tran);

// Constants for ridge period estimation
const int	N_LINE_LEN		= 41;
const int	INVALID_GRAY	= 100000000;
const int	N_LINE_NUM		= 21;
const int	MIN_PERIOD		= 3;
const int	MAX_PERIOD		= 25;

// Constants for Gabor filtering
const int		FILTER_PERIODS	= MAX_PERIOD-MIN_PERIOD+1;
const int		DIR_INTERVAL	= 2;
const int		FILTER_DIRS		= 180/DIR_INTERVAL;
const double	BASE_PERIOD		= 10;
const double	FILTER_SIGMA	= 0.5*BASE_PERIOD;
const int		FILTER_RADIUS	= (int)(2*FILTER_SIGMA);
const int		FILTER_SIZE		= FILTER_RADIUS*2+1;
const int		POINT_NUM = 7;

// Gabor filter
typedef struct _GaborFilter
{
public:
	char* GaborDX[FILTER_DIRS][FILTER_PERIODS];//[FILTER_SIZE][FILTER_SIZE];	// 127 > MAX_PERIOD
	char* GaborDY[FILTER_DIRS][FILTER_PERIODS];//[FILTER_SIZE][FILTER_SIZE];
	int* Gabor;//[FILTER_SIZE*FILTER_SIZE];	// int type for fast computation
}GaborFilter;

GaborFilter* GaborFilterCreate();
void GaborFilterRelease(GaborFilter*);

// color constant
#define C_RED		CV_RGB(255,0,0)
#define C_GREEN		CV_RGB(0,255,0)
#define C_BLUE		CV_RGB(0,0,255)
#define C_YELLOW	CV_RGB(255,255,0)
#define C_MAGENTA	CV_RGB(255,0,255)
#define C_CYAN		CV_RGB(0,255,255)
#define C_WHITE		CV_RGB(255,255,255)
#define C_BLACK		CV_RGB(0,0,0)

#define C_DRED		CV_RGB(200,0,0)
#define C_DGREEN	CV_RGB(0,200,0)
#define C_DBLUE		CV_RGB(0,0,200)

// IO
typedef enum{MINUFILE_TEXT3=0, MINUFILE_TEXT5, MINUFILE_NEU} MinuFileType;
bool LoadMinuTxt(char* fname, Minutiae& minus);
bool LoadMinu(char* fname, Minutiae& minus, MinuFileType type);

// Visualization
void ShowImage(char* name, IplImage* img, bool wait=true, double scale=1);
void DrawImage(IplImage* dst, IplImage* src);
void DrawOF(IplImage* img, IplImage* orientation_field, int block_size, CvScalar color=C_BLUE);
void DrawMinutiae(IplImage* img, Minutiae& minus, CvScalar color=C_RED, bool show_index=false, bool box_shape=true, unsigned int radius=3, unsigned int tail_length=10, unsigned int line_width=1);
void DrawMinu(IplImage* img, Minu minu, int mid, CvScalar color=C_RED, bool box_shape=true, unsigned int radius=3, unsigned int tail_length=10, unsigned int line_width=1);
int GetNearMinu(Minutiae& minus, int x, int y);

// Orientation field
IplImage* ComputeOFGradient(IplImage* fpImage, int block_size);

// Foreground segmentation
IplImage* ForegroundSegmentSimple(IplImage* fpImage, int block_size, int minStd);
void MaskOF(IplImage* blockMaskImg, IplImage* blockOF);

// Ridge enhancement
IplImage* RidgeEnhanceGabor(IplImage* fpImage, IplImage* blockOF, int block_size, GaborFilter* filter);

// Ridge binarization and thinning
void RidgeBinarization(IplImage* grayImage, IplImage* binaryImage);
void RidgeThinning(IplImage* binaryImage);

// Inline functions
inline int NormalizeMinuDir(int a)
{
	a = a%360;
	if(a<=-180)	a+=360;
	if(a>180)	a-=360;
	return a;
}

inline int NormalizeRidgeDir(int a)
{
	a = a%180;
	if(a<=-90)	a+=180;
	if(a>90)	a-=180;
	return a;
}
