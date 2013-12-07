#include "data.h"

MediaData::MediaData()
{
	pData=NULL;
	curg=0;
	curp=0;
	length=0;
	type = NONE;
}

MediaData::MediaData(BYTE *_pData,int _length)
{
	Initialize(_pData,_length);
}

MediaData::MediaData(string strFileName)
{
	Initialize(strFileName);
}

MediaData::~MediaData()
{
	switch(type)
	{
	case FILE:
		pFile.close();
		break;
	case DATA:
		pData = NULL;
		break;
	default:
		break;
	}
}

void MediaData::Initialize(BYTE* _pData,int _length)
{
	pData=_pData;
	curg=0;
	curp=0;
	length=_length;

	type=DATA;
}

void MediaData::Initialize(string strFileName)
{

	fileName = strFileName;
	pFile.open(fileName,ios_base::app|ios_base::out|ios_base::in|ios_base::binary);
	curg=0;
	curp=0;
	length=0;
	if(!pFile.is_open())
	{
		type=NONE;
		return ;
	}
	type = FILE;
	pFile.seekp(0,ios_base::end);
	length = pFile.tellp();
	pFile.seekp(0,ios_base::beg);
	pFile.seekg(0,ios_base::beg);
}

BYTE MediaData::GetCurByte()
{
	BYTE result;

	if(type == FILE)
	{
		pFile.seekg(curg,ios_base::beg);
		pFile.read((char*)&result,sizeof(result));
		pFile.seekg(-1,ios_base::cur);
		curg = pFile.tellg();
	}
	else if(type == DATA)
	{
		result = pData[curg];
	}
	else
	{
		throw Uninitialized(MEDIADATA);
	}

	return result;
}

BYTE MediaData::GetNextByte()
{
	BYTE result;

	if( MediaData::is_End() )
		return NULL;

	if( type == FILE )
	{
		pFile.seekg(curg,ios_base::beg);
		pFile.read((char*)&result,sizeof(result));
		curg = pFile.tellg();
	}
	else if( type == DATA)
	{
		result = pData[curg];
		curg++;
	}
	else
	{
		throw Uninitialized(MEDIADATA);
	}
	return result;
}

bool MediaData::is_End()
{
	return (curg == length) || (curg == -1) ||
		(curp == length) || (curp == -1) ;
}

bool MediaData::is_Empty()
{
	return (type == NONE) || (length == 0);
}

void MediaData::clear()
{
	switch(type)
	{
	case FILE:
		pFile.close();
		pFile.open(fileName,ios_base::trunc|ios_base::out|ios_base::in|ios_base::binary);
		curg=0;
		curp=0;
		length=0;
		if(!pFile.is_open())
		{
			type=NONE;
			break ;
		}
		type = FILE;
		pFile.seekp(0,ios_base::end);
		length = pFile.tellp();
		pFile.seekp(0,ios_base::beg);
		pFile.seekg(0,ios_base::beg);
		break;
	case DATA:
		if( pData!= NULL)
			delete [] pData;
		pData=NULL;
		curg=0;
		curp=0;
		length=0;
		break;
	default:
		break;
	}
}

long MediaData::GetLength()
{
	return length;
}

int MediaData::WriteByte(const BYTE* data,long offset,int len)
{
	long tempg;
	switch(type)
	{
	case FILE:
		pFile.seekp(offset,ios_base::beg);
		pFile.write((char*)data,len);
		
		tempg = pFile.tellg();
		pFile.seekg(0,ios_base::end);
		length = pFile.tellg();
		pFile.seekg(tempg,ios_base::beg);
		curp += len;
		break;

	case DATA:
		if(( offset + len ) > MediaData::length )
			throw 1;
		for( int i = 0 ; i < len ; i ++ )
			pData[ offset + i] = data[i];
		break;

	default:
		throw Uninitialized(MEDIADATA);
		break;
	}
	return 0;
}

int MediaData::WriteByte(const BYTE* data,int len)
{
	long tempg;
	switch(type)
	{
	case FILE:
		pFile.seekp(curp,ios_base::beg);
		pFile.write((char*)data,len);

		tempg = pFile.tellg();
		pFile.seekg(0,ios_base::end);
		length = pFile.tellg();
		pFile.seekg(tempg,ios_base::beg);
		curp += len;
		break;

	case DATA:
		if(( curp + len ) > MediaData::length )
			throw 1;
		for( int i = 0 ; i < len ; i ++ )
			pData[ curp + i] = data[i];
		curp += len;
		break;

	default:
		throw Uninitialized(MEDIADATA);
		break;
	}
	return 0;
}

void MediaData::seekg(int len,pos p)
{

	switch(p)
	{
	case BEG:
		curg = len;
		break;

	case CUR:
		curg += len;
		if(curg < 0)
			curg = 0;
		else if(curg > length)
			curg = length;
		break;

	case END:
		curg = length + len;
		if(curg < 0)
			curg = 0;
		else if(curg > length)
			curg = length;
		break;

	default:
		break;
	}
}

void MediaData::seekp(int len,pos p)
{
	switch(p)
	{
	case BEG:
		curp = len;
		break;

	case CUR:
		curp += len;
		if(curp < 0)
			curp = 0;
		else if(curp > length)
			curp = length;
		break;

	case END:
		curp = length + len;
		if(curp < 0)
			curp = 0;
		else if(curp > length)
			curp = length;
		break;

	default:
		break;
	}
}

long MediaData::tellg()
{
	return curg;
}

long MediaData::tellp()
{
	return curp;
}


