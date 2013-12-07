#include "common_exception.h"

Uninitialized::Uninitialized(objectType type) throw()
{
	_objectType = type;
}

Uninitialized::~Uninitialized() throw(){}

Unsupported::Unsupported() throw(){}
Unsupported::~Unsupported() throw(){}

WrongStream::WrongStream() throw(){}
WrongStream::~WrongStream() throw(){}

OutOfRange::OutOfRange() throw(){}
OutOfRange::~OutOfRange() throw(){}
