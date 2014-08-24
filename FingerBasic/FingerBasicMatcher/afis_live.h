//*================================================================================================
//SingleFingerMatch.h match dynamic link library header file
//constants defination, struct definations, and functions declaration
//all rights reserved (c) 2001.09.25
//===============================================================================================*/

#ifndef _AFIS_LIVE_H_
#define _AFIS_LIVE_H_

#define PAI				 3.141592653589793
#define HPAI			 1.570796326794897
#define DPAI			 6.283185307179586	//@. = 2*PAI			
#define NPAI           －3.141592653589793

#define ATR				 0.017453292519943	//@. = PAI/180 
#define RTA				57.295779513082321	//@. = 180/PAI

#define MAX_MINU_COUNT				   200	//@, 最大特征点数目

typedef struct  _MinutiaeInfo
{
	short iSita;
	short ix;	 
	short iy;
}MINUINFO;

typedef struct  _SPInfo
{
	unsigned char	cFid;			//@. 中心置信度
	short	cx;						//@. 中心坐标
	short	cy;						//@. 中心坐标
	short	cSita;					//@. 中心方向
}CENTERINFO;

typedef struct  _FeatureInfo
{
	short		minu_num;				//@. 特征点数目
	CENTERINFO		sp[4];
	MINUINFO	minu[MAX_MINU_COUNT];	//@. 特征点信息
}FEATUREINFO;

#endif	//#ifndef _AFIS_LIVE_H_
