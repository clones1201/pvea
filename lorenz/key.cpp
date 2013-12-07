
#include "key.h"

double KEY::GetSigma()
{
	return KEY::Sigma;
}

//	double SetSigma(double param);

double KEY::GetB()
{
	return KEY::b;
}

//	double SetB(double param);

double KEY::GetR()
{
	return r;
}

//	double SetR(double param);

double KEY::GetX0()
{
	return KEY::x0;
}

//	double SetX0(double param);

double KEY::GetY0()
{
	return KEY::y0;
}


//	double SetY0(double param);

double KEY::GetZ0()
{
	return KEY::z0;
}

//	double SetZ0(double param);

int KEY::GetIterationTime()
{
	return KEY::t;
}

//	int SetIterationTime(int param);


void KEY::GenerateKey(string strKeyFileName)
{
	srand(time(0));

	/*
	Lorenz系统的参数不能随便设啊……
	*/

	Sigma = 10 + double(rand())/double(RAND_MAX) ;
	b =  8.0/3.0 + double(rand())/double(RAND_MAX) ;
	r = 24.7 * (1 + double(rand())/double(RAND_MAX)) ;

	//初始位置随便来应该没问题
	x0 = 30 * double(rand())/double(RAND_MAX) ;
	y0 = 30 * double(rand())/double(RAND_MAX) ;
	z0 = 30 * double(rand())/double(RAND_MAX) ;
	t = int(20000 * (1 + double(rand())/double(RAND_MAX)));

	ofstream out;
	out.open(strKeyFileName.c_str(),ios_base::out);

	if(!out.is_open())
	{
		throw new ErrorFileOpen();
	}

	out.precision(18); //这个有效数字的精度和编译器以及机器环境有关吗？

	out<<Sigma<<endl;
	out<<b<<endl;
	out<<r<<endl;
	out<<x0<<endl;
	out<<y0<<endl;
	out<<z0<<endl;
	out<<t<<endl;
}


bool KEY::LoadKeyFromFile(string strKeyFileName)
{
	ifstream in;
	in.open(strKeyFileName.c_str(),ios_base::in);
	if(!in.is_open())
	{
		throw new ErrorFileOpen();
	}

	in>>Sigma;
	in>>b;
	in>>r;
	in>>x0;
	in>>y0;
	in>>z0;
	in>>t;

	return true;
}

/*
#ifdef _DEBUG
istream operator>>(istream& in, Key &key)
{

}
ostream operator<<(ostream& out, Key &key)
{

}
#endif
*/
