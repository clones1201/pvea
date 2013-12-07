#ifndef __DATA_H__
#define __DATA_H__

#include "global.h"
#include "common_exception.h"


enum pos
{
	BEG,CUR,END
};

class MediaData 
{
private:
	fstream pFile;
	BYTE *pData;

	long curp;
	long curg;
	long length;

	enum
	{
		FILE,DATA,NONE
	}type;

	string fileName;
public:

	MediaData(BYTE * _pData,int _length);
	MediaData(string strFileName);
	MediaData();

	~MediaData();

    void Initialize(BYTE*  _pData,int _length);
    void Initialize(string strFileName);

	BYTE GetCurByte();
	BYTE GetNextByte();

	void seekg(int len , pos p);
	void seekp(int len , pos p);

	long tellp();
	long tellg();

	int WriteByte(const BYTE* data,long offset,int len);
	int WriteByte(const BYTE* data,int len);

	bool is_End();
	
	bool is_Empty();
	long GetLength();
	void clear();
};



#endif
