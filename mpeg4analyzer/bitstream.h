#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include "global.h"
#include "data.h"


struct bits
{
	int offset;
	int bitLength;
	bits()
	{ offset = 0; bitLength = 0; }
	bits(int o,int bl)
	{
		offset = o;
		bitLength = bl;
	}
};

class bitstream
{
private:
	MediaData data;

	vector<BYTE> buffer;
	

	bits ReadBitsNoMove(BYTE* buff,int bitLength);
	bits ReadBits(BYTE * buff,int bitLength);
	int bitPosInByteR;
	int bitPosInByteW;
	bool is_end;
public:
	bitstream();
	bitstream(string strFileName);
	bitstream(BYTE* _pData,int _length);

	void Initialize(BYTE* _pData,int _length);
	void Initialize(string strFileName);

	~bitstream();
    int GetLength();
    void clear();

	void BitstreamPutBits(uint32_t data,int bitLength);
	uint32_t BitstreamShowBits(int bitLength);
	uint32_t BitstreamGetBits(int bitLength);
	int BitstreamSkip(int bitLength);
//	int BitstreamBack(int bitLength);
	int BitstreamNumBitsToByteAlign();
	uint32_t BitstreamShowBitsFromByteAlign(int bitsLength);
	
	bits WriteBits(const BYTE* buff,int bitLength,int offset);

	int BitstreamByteAlign();
	int StuffingBitsW();
	
	void flush();

	bool ByteAligned();
	bool valid_stuffing_bits();
	long tellg();
	long tellp();
	bool is_End();
};

int mod(int a , int b);

void shiftcode(int offset,int StartAt,BYTE* outdata,const BYTE* data,int BitLength); // it shift left when offset is positive 


static const BYTE stuffing_Code[8]=
{
	0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xFF
};

static const BYTE bitCode[8]=
{
	0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80
};

static const BYTE maskCodeF[8]=
{
	0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF
};

#define maskCodeB stuffing_Code

#endif