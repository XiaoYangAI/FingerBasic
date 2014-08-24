#pragma once

class MinuDesc
{
public:
	MinuDesc(void) {};
	virtual ~MinuDesc(void) {};
	virtual double CompareDesc(MinuDesc* desc) = 0;
	virtual void ShowDesc() = 0;
};

class MinuDescs
{
public:
	MinuDescs(void) {pDescs=0; num=0;};
	virtual ~MinuDescs(void) {delete [] pDescs;};

	//virtual void CreateDesc() = 0;
	virtual void SaveDesc(FILE* file) = 0;
	virtual void SaveDesc(char* fname) = 0;
	virtual bool LoadDesc(FILE* file) = 0;
	virtual bool LoadDesc(char* fname) = 0;
	virtual MinuDesc* GetDesc(int i) {return pDescs+i;};

	MinuDesc* pDescs;
	int num;
};
