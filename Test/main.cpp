
#include "../mpeg4analyzer/mpeg4analyzer.h"
#include "../lorenz/Encryptor.h"
#include "../lorenz/Decrypter.h"
#include "../lorenz/key.h"
#include <memory>
#include <time.h>

int main(int argc,char* argv[])
{
	string inputFileName;
	string outputFileName;
	string keyname;
	enum Mode
	{
		Enc,Dec
	}mode;

	if(argc < 2)
	{
		cerr<<"Miss Argument"<<endl;
		return -1;
	}
	else
	{
		string argument;
		int index = 1;

		enum Status
		{
			START,INFILENAME,OUTFILENAME,KEYFILENAME,HELP,ERROR
		}status;

		status = START;

		while( index < argc )
		{
			argument = argv[index++];
			
			if( status == START )
			{
				if( argument.compare("-e") == 0 )
				{
					mode = Enc;
					status = INFILENAME;
				}
				else if( argument.compare("-d") == 0 )
				{
					mode = Dec;
					status = INFILENAME;
				}
				else if( argument.compare("-o") == 0 )
				{
					status = OUTFILENAME;
				}
				else if( argument.compare("-k") == 0 )
				{
					status = KEYFILENAME;
				}
				else if( argument.compare("-h") == 0 || argument.compare("-help") == 0 )
				{
					status = HELP;
				}
				else
				{
					status = ERROR;
				}
			}
			else if( status == HELP )
			{
				cout<<"This a Xvid Codec MPEG4-Visual stream Encrypter"<<endl;
				cout<<endl;
				cout<<"pveaforxvid [-e/-d inputfile][-o outputfile][-k keyfile]"<<endl;
				cout<<"-e\t加密视频文件"<<endl;
				cout<<"-d\t解密视频文件"<<endl;
				cout<<"-o\t输出文件名，缺省为[输入文件名_[模式名].avi"<<endl;
				cout<<"-k\t密钥文件名，加密模式时将随机生成新的密钥，解密模式时须读取正确的密钥文件"<<endl;
				cout<<endl;
				return 0;
			}
			else if(status != ERROR )
			{
				if( argument[0] != '-' )
				{
					switch(status)
					{
					case INFILENAME:
						inputFileName = argument;
						status = START;
						break;
					case OUTFILENAME:
						outputFileName = argument;
						status = START;
						break;
					case KEYFILENAME:
						keyname = argument;
						status = START;
						break;
					default:
						break;
					}
				}
				else
				{
					status = ERROR;
					index = argc + 1;
				}
			}
			else
			{
				index = argc + 1;
			}
		}
		if( status == ERROR )
		{
			cerr<<"Wrong Argument"<<endl;
			return -1;
		}
		else if( status == HELP )
		{
			cout<<"This a Xvid Codec MPEG4-Visual stream Encrypter"<<endl;
			cout<<endl;
			cout<<"pveaforxvid [-e/-d inputfile][-o outputfile][-k keyfile]"<<endl;
			cout<<"-e\t加密视频文件"<<endl;
			cout<<"-d\t解密视频文件"<<endl;
			cout<<"-o\t输出文件名，缺省为[输入文件名_[模式名].avi"<<endl;
			cout<<"-k\t密钥文件名，加密模式时将随机生成新的密钥，解密模式时须读取正确的密钥文件"<<endl;
			cout<<endl;
			return 0;
		}
		else
		{
			if( inputFileName.size() == 0 )
			{
				cerr<<"No Filename"<<endl;
				return -1;
			}
			if( outputFileName.size() == 0 )
			{
				outputFileName = inputFileName;
				switch(mode)
				{
				case Enc:
					outputFileName = outputFileName.substr(0,outputFileName.find_last_of('.')).append("_Encrypeted.avi");
					break;
				case Dec:
					outputFileName = outputFileName.substr(0,outputFileName.find_last_of('.')).append("_Decrypeted.avi");
					break;
				default:
					break;
				}
			}
			if( keyname.size() == 0 )
			{
				keyname = inputFileName;
				keyname = keyname.substr(0,keyname.find_last_of('.')).append("_key");
			}
		}
	}

	KEY key;

	switch(mode)
	{
	case Enc:
		key.GenerateKey(keyname);
		break;
	case Dec:
		key.LoadKeyFromFile(keyname);
		break;
	default:
		return -1;
	}

	BYTE *data1/*,*data2*/;
	fstream file1(inputFileName.c_str(),ios_base::binary|ios_base::in);
	//fstream file2(output.c_str(),ios_base::binary|ios_base::out);
	if(!file1.is_open())
	{
		cout<<"no such file"<<endl;
		exit(0);
	}
	file1.seekg(0,ios_base::end);
	long length = file1.tellg();
	file1.seekg(0,ios_base::beg);
	data1 = new BYTE[length];
	file1.read((char*)data1,length);
	bitstream stream1(data1,length);
	bitstream stream2(outputFileName);
	stream2.clear();

	cout<<"视频流分析..."<<endl;
	mpeg4analyzer analyzer(data1,length);
	vector< pair<int ,bits > > blockdata = analyzer.Analyse();
	cout<<"\t\t\t\r100%"<<endl;

#ifdef _DEBUG
	ofstream temp(inputFileName.append("block.txt"),ios_base::out);
	for_each(blockdata.begin(),blockdata.end(),
		[&]( pair<int ,bits> p)
	    {
			temp<<"addr:0x"<<hex<<p.first<<" pos:"<<p.second.offset<<" length:"<<p.second.bitLength<<endl;
	  });
#endif

	LorenzSys Lorenz(key);
	/*	Encryptor Enc(key);
	Decrypter Dec(key);
*/
	/* Encrypt process */
	switch(mode)
	{
	case Enc:
		cout<<"加密..."<<endl;
		break;
	case Dec:
		cout<<"解密..."<<endl;
		break;
	default:
		return -1;
	}
	
	int oldpos = 0;
	int pos,distance;
	for_each(blockdata.begin(),blockdata.end(),
		
		[&] ( pair< int,bits > p )
	    {

			uint32_t data;
			pos = (p.first * 8) + p.second.offset ;
			distance = pos - oldpos;
			for( int i = 0 ; i < distance / 32 ; i++ )
			{
				data = stream1.BitstreamGetBits(32);
				stream2.BitstreamPutBits(data,32);
			}
			data = stream1.BitstreamGetBits(distance % 32);
			stream2.BitstreamPutBits(data,distance % 32);
			
			data = stream1.BitstreamGetBits( p.second.bitLength );
			data = data ^ Lorenz.GetNextByte() & maskCodeB[ p.second.bitLength - 1 ]; 
			stream2.BitstreamPutBits( data , p.second.bitLength );
			oldpos = pos + p.second.bitLength;
			
			cout<< int( (double( stream1.tellg() ) )/( double( stream1.GetLength() ) ) * 100 ) <<"%\r";
	     }
	);
	while( !stream1.is_End() )
	{
		uint32_t data;
		data = stream1.BitstreamGetBits(32);
		stream2.BitstreamPutBits(data,32);
	}
	stream2.flush();
	cout<<"100%"<<endl;

	cout<<"Press Any Key to Continue..."<<endl;
	cin.get();
	
	return 0;
}
