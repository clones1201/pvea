
#ifndef __DECRYPTER_H__
#define __DECRYPTER_H__

#include "global.h"
#include "Lorenz.h"


class Decrypter
{
private:
	LorenzSys core;

public:
	Decrypter(KEY &key);
	~Decrypter();
	
	BYTE* Decrypt( BYTE* data , int length);

	BYTE Decrypt( BYTE data);
};


#endif
