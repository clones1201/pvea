
#ifndef __LORENZ_H__
#define __LORENZ_H__

#include "key.h"

class LorenzSys
{
private:

	KEY key;

	double x;
	double y;
	double z;

	void roll();

public:

	LorenzSys(){};
	LorenzSys(KEY& key);

	~LorenzSys(){};

	void Initialize(KEY& key);

	BYTE GetNextByte();

};

#endif
