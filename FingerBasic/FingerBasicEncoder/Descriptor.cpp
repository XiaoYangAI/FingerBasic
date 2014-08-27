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