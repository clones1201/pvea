
#include "Encryptor.h"

Encryptor::Encryptor( KEY &key)
{
	core.Initialize(key);
}

Encryptor::~Encryptor()
{}

BYTE* Encryptor::Encrypt( BYTE* data , int length)
{
	BYTE* result;

	result = new BYTE[length];

	for( int i = 0 ; i < length ; i ++ )
	{
		result[i] = core.GetNextByte() ^ data[i];
	}

	return result;
}

BYTE Encryptor::Encrypt( BYTE data)
{
	BYTE result;

	result = core.GetNextByte() ^ data;

	return result;
}
