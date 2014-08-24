#include "MinuMatch.h"

class MinuMatchTico:
	public MinuMatch
{
public:
	MinuMatchTico(Minutiae* _pMinus[2], MinuDescs* _pDescs[2]):MinuMatch(_pMinus, _pDescs){};
	~MinuMatchTico(void) {};

	//virtual void LocalMatch();
	virtual void Pairing(unsigned int nAlignment);
};
