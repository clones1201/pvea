#include "bitstream.h"

bitstream::bitstream()
{
	bitPosInByteR = 0;
	bitPosInByteW = 0;

	is_end = false;
}

void bitstream::Initialize(BYTE* _pData,int _length)
{
	data.Initialize(_pData,_length);
	bitPosInByteR = 0;
	bitPosInByteW = 0;
	is_end = false;
}

void bitstream::Initialize(string strFileName)
{
	data.Initialize(strFileName);
	bitPosInByteR = 0; 
	bitPosInByteW = 0;
	is_end = false;
}

bitstream::bitstream(BYTE* _pData,int _length)
{
	Initialize(_pData,_length);
}

bitstream::bitstream(string strFileName)
{
	Initialize(strFileName);
}

bitstream::~bitstream()
{

}

bool bitstream::is_End()
{
//	return data.is_End();
	return is_end;
}

void bitstream::clear()
{
	data.clear();
	buffer.clear();
	bitPosInByteR = 0;
	bitPosInByteW = 0;
}

bits bitstream::ReadBits( BYTE * buff, int bitLength)
{
	
	bits result;
	try
	{

		int offset = bitPosInByteR;

		result.offset  = offset;
		result.bitLength = bitLength;

		int ByteLength = (offset + bitLength != 0) ? ( (offset + bitLength - 1 ) / 8 ) + 1
			: 0 ; 

		if( ByteLength == 0 )
		{
			//		buff = NULL;        缓冲区应预先分配。谁分配，谁回收
			return result;
		}

		if( ByteLength + data.tellg() >= data.GetLength() )  //如果接下来要读取得位超过了总长度，那么截断需要读的位，并设置“到达结尾”为真
		{
			ByteLength = data.GetLength() - data.tellg();
			result.bitLength = ByteLength * 8 - offset;
			is_end = true;
		}
		//	buff = new BYTE[ ByteLength ];

		for( int i = 0 ; i < ByteLength ; i ++)
			buff[i] = 0x00;

		buff[ 0 ] =  maskCodeB[  7 - bitPosInByteR  ] & data.GetCurByte();
		if( ByteLength > 1)
		{
			data.seekg(1,CUR);
			for( int i = 1 ; i < ByteLength - 1; i++ )
			{
				buff[i] = data.GetNextByte();
			}
			buff[ ByteLength - 1 ] =  maskCodeF[ (offset + bitLength - 1 )%8] & data.GetCurByte();
		}
		else
		{
			buff[0] &= maskCodeF[ offset + bitLength - 1 ] ;
		}
		if(!is_end)
		{
			bitPosInByteR = (offset + bitLength)%8;
			if(bitPosInByteR == 0)
			{
				data.seekg(1,CUR);
			}
		}
	}catch(Uninitialized)
	{
		cout<<"bitstream Unitialized"<<endl;
	}
	return result;
}

bits bitstream::ReadBitsNoMove(BYTE* buff,int bitLength)
{
	int old_bitPosInByteR = bitPosInByteR;
	long old_curg = data.tellg();

	bits result = ReadBits(buff,bitLength);

	data.seekg(old_curg,BEG);
	bitPosInByteR = old_bitPosInByteR;
	return result;
}

void bitstream::BitstreamPutBits(uint32_t data,int bitLength)
{
	if( bitLength != 0 )
	{
		BYTE* buff = new BYTE[5];

		buff[0] = data >> ( 24 - 8 * ((32 - bitLength) / 8) );
		buff[1] = data >> ( 16 - 8 * ((32 - bitLength) / 8) );
		buff[2] = data >> ( 8 - 8 * ((32 - bitLength) / 8) );
		buff[3] = data >> (  - 8 * ((32 - bitLength) / 8) );
		buff[4] = 0 ;
		WriteBits(buff,bitLength,(32 - bitLength) % 8);
		delete [] buff;
		buff = NULL;
	}
}

bits bitstream::WriteBits(const BYTE*buff,int bitLength,int offset)
{
	bits result;
	try
	{
		int newByteLength = bitPosInByteW == 0 
			? 1 + ( bitPosInByteW + bitLength - 1) / 8
			: ( bitPosInByteW + bitLength - 1) / 8 ;

		for(int i = 0 ; i < newByteLength ; i++ )
			buffer.push_back( (BYTE)0 );               //新增的字节

		BYTE* temp = new BYTE[ 1 + (bitPosInByteW + bitLength - 1) / 8];
		shiftcode( offset - bitPosInByteW ,offset,temp,buff,bitLength); //把输入的码流对齐

		if( bitPosInByteW != 0 )  //如果此时输出的位置不是一个字节起始
		{
			for(int i = buffer.size() - newByteLength - 1 ; i < buffer.size() ; i++ )
				buffer[i] += temp[ i - ( buffer.size() - newByteLength - 1 )];
		}
		else 
		{
			for(int i = 0 ; i < buffer.size() ; i++ )
				buffer[i] += temp[ i ];
		}

		bitPosInByteW = (bitPosInByteW + bitLength) %8;

		if(bitPosInByteW == 0)
		{
			data.WriteByte(buffer.data(),buffer.size());
			buffer.clear();
		}
		
		delete [] temp;
		temp = NULL;
	}
	catch(Uninitialized)
	{
		cout<<"bitstream Unitialized"<<endl;
	}


	result.bitLength = bitLength;
	result.offset = offset;
	return result;
}

void bitstream::flush()
{
	data.WriteByte(buffer.data(),buffer.size());
	buffer.clear();
}

int bitstream::StuffingBitsW()
{
	try
	{
		if(bitPosInByteW != 0)
		{
			buffer[ buffer.size() ] += stuffing_Code[ 7 - bitPosInByteW ] ; 
			data.WriteByte(buffer.data(),buffer.size());
			buffer.clear();
			bitPosInByteW = 0;
		}
	}
	catch(Uninitialized)
	{
		cout<<"bitstream Unitialized"<<endl;
	}
	return 0;
}
int bitstream::BitstreamByteAlign()
{
	try
	{
		if(bitPosInByteR != 0)
		{
			bitPosInByteR = 0;
			data.seekg(1,CUR);
		}
	}
	catch(Uninitialized)
	{
		cout<<"bitstream Unitialized"<<endl;
	}
	return 0;
}

void shiftcode(int offset,int startAt,BYTE *outdata,const BYTE* data,int BitLength)
{
	if(startAt > 7)
		throw new OutOfRange;

	int move = startAt - mod( startAt - offset , 8) ;
	int ByteLength = 1 + (startAt + BitLength - 1) / 8 ;
	BYTE *temp = outdata;

	if( move == 0 )
	{

		for(int i = 0 ; i < ByteLength ; i ++ )
		{
			temp[i] = data[i];
		}

	}
	else 
	{
		
		int newByteLength = 1 + (startAt - move + BitLength - 1) / 8 ;
		if( move > 0 )
		{
			for( int i = 0 ; i < newByteLength ; i++ )
			{
				if( ( i + 1 ) <= ByteLength )
				{
					temp[i] = ( data[i] << move ) & maskCodeF[ 7 - move ] | (( data[i+1] & maskCodeF[ move - 1] ) >> ( 8 - move ));
				}
				else
					temp[i] = (data[i] << move);
			}
		}
		else if( move < 0 )
		{
			for( int i = newByteLength - 1 ; i >= 0 ; i-- )
			{
				if( ( i - 1 ) >= 0 )
				{
					temp[i] = ( data[i] >> (-move) ) & maskCodeB[ 7 + move ] | (( data[i-1] << ( 8 + move )) & maskCodeF[ (-move) - 1]) ;
				}
				else
					temp[i] = data[i] >> (-move);
			}
		}
	}
}

long bitstream::tellg()
{
	return data.tellg();
}

long bitstream::tellp()
{
	return data.tellp();
}

int mod(int a , int b)
{
	return ((a % b) >= 0 )? (a % b) : (a % b) + b ;
}

bool bitstream::ByteAligned()
{
	if( bitPosInByteR == 0 )
		return true;
	else 
		return false;
}

uint32_t bitstream::BitstreamShowBits(int bitLength)
{
	uint32_t result;
		int ByteLength = ( bitPosInByteR + bitLength != 0) ? ( ( bitPosInByteR + bitLength - 1 ) / 8 ) + 1
			: 0 ; 
	BYTE *buff = new BYTE[ ByteLength ];
	bits Bits = ReadBitsNoMove(buff,bitLength);

	BYTE *temp = new BYTE[ 4 ];
	
	shiftcode( Bits.offset + Bits.bitLength - 32,Bits.offset,temp,buff,Bits.bitLength);

	if( bitLength > 24 )
		result = ((int)temp[0]) << 24 | ((int)temp[1]) << 16 | ((int)temp[2]) << 8 | ((int)temp[3]) ;
	else if( bitLength > 16 )
		result = ((int)temp[0]) << 16 | ((int)temp[1]) << 8 | ((int)temp[2]);
	else if( bitLength > 8 )
		result = ((int)temp[0]) << 8 | ((int)temp[1]);
	else 
		result = ((int)temp[0]);


	delete [] temp;
	temp = NULL;
	delete [] buff;
	buff = NULL;

	return result; //
}

uint32_t bitstream::BitstreamGetBits(int bitLength)
{	
	uint32_t result;

	if( bitLength == 0 )
		return 0;

	int ByteLength = ( bitPosInByteR + bitLength != 0) ? ( ( bitPosInByteR + bitLength - 1 ) / 8 ) + 1
			: 0 ; 
	BYTE *buff = new BYTE[ ByteLength ];
	bits Bits = ReadBits(buff,bitLength);

	BYTE *temp = new BYTE[ 4 ];

	shiftcode( Bits.offset + Bits.bitLength - 32,Bits.offset,temp,buff,Bits.bitLength);

	if( bitLength > 24 )
		result = ((int)temp[0]) << 24 | ((int)temp[1]) << 16 | ((int)temp[2]) << 8 | ((int)temp[3]) ;
	else if( bitLength > 16 )
		result = ((int)temp[0]) << 16 | ((int)temp[1]) << 8 | ((int)temp[2]);
	else if( bitLength > 8 )
		result = ((int)temp[0]) << 8 | ((int)temp[1]);
	else 
		result = ((int)temp[0]);


	delete [] temp;
	temp = NULL;
	delete [] buff;
	buff = NULL;

	return result; //

}

int bitstream::BitstreamSkip(int bitLength)
{
	int ByteLength = ( bitPosInByteR + bitLength != 0) ? ( ( bitPosInByteR + bitLength - 1 ) / 8 ) + 1
			: 0 ; 
	BYTE *buff = new BYTE[ ByteLength ]; 
	bits Bits = ReadBits(buff,bitLength);
	delete [] buff;
	return Bits.bitLength;
}

/*
int bitstream::BitstreamBack(int bitLength)
{
	return 0;
}
*/
int bitstream::BitstreamNumBitsToByteAlign()
{
	return 8 - bitPosInByteR;
}

uint32_t bitstream::BitstreamShowBitsFromByteAlign(int bitsLength)
{
	uint32_t result;
	long oldCur = data.tellg();
	long oldPos = bitPosInByteR;

	BitstreamByteAlign();
	result = BitstreamShowBits(bitsLength);
	data.seekg(oldCur,BEG);
	bitPosInByteR = oldPos;

	return result;
}

bool bitstream::valid_stuffing_bits()
{
	if( bitPosInByteR == 7 )
		return ( BitstreamShowBits(1) == 0x00 );
	else
		return ( BitstreamShowBits( 8 - bitPosInByteR ) == stuffing_Code[ 6 - bitPosInByteR ]); 
}

int bitstream::GetLength()
{
	return data.GetLength();
}