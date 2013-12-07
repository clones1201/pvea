
#ifndef __ENCRYPTOR_H__
#define __ENCRYPTOR_H__

#include "global.h"
#include "Lorenz.h"

class Encryptor
{
private:
	LorenzSys core;
public:
	Encryptor(KEY &key);
	~Encryptor();

	BYTE* Encrypt( BYTE* data , int length);

	BYTE Encrypt( BYTE data);

};


#endif
