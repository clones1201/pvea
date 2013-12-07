#ifndef __COMMON_EXCEPTION_H__
#define __COMMON_EXCEPTION_H__

#include "global.h"


enum objectType
{
	MEDIADATA,BITSTREAM
};

class Uninitialized : public exception
{
private:
	objectType _objectType;
public:
	Uninitialized(objectType type) throw();

	~Uninitialized() throw();

};

class Unsupported : public exception
{
public:
	Unsupported() throw();
	~Unsupported() throw();
};

class WrongStream : public exception
{
public:
	WrongStream() throw();
	~WrongStream() throw();
};

class OutOfRange : public exception
{
public:
	OutOfRange() throw();
	~OutOfRange() throw();
};

#endif