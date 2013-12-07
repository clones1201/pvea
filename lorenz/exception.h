#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include"global.h"

class ErrorFileOpen :public exception
{

public:

	ErrorFileOpen()throw(){};
	~ErrorFileOpen()throw(){};

};

#endif
