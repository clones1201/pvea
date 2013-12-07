
#include "Decrypter.h"

Decrypter::Decrypter( KEY &key)
{
	core.Initialize(key);
}

Decrypter::~Decrypter()
{

}

BYTE* Decrypter::Decrypt( BYTE* data , int length)
{
	BYTE* result;

	result = new BYTE[length];

	for( int i = 0 ; i < length ; i ++ )
	{
		result[i] = core.GetNextByte() ^ data[i];
	}

	return result;
}

BYTE Decrypter::Decrypt( BYTE data)
{
	BYTE result;

	result =  core.GetNextByte() ^ data ;

	return result;
}