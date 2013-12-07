
#ifndef __KEY_H__
#define __KEY_H__

#include "global.h"
#include "exception.h"

class KEY
{
private:
	double Sigma;   //���綯��ϵͳ����
	double b;
	double r;
	double x0;    //��ʼλ�� x0 , y0 , z0
	double y0;
	double z0;
	int t;      //��ʼ��������
public:
	KEY()
	{
		Sigma = 0;
		b = 0;
		r = 0;
		x0 = 0;
		y0 = 0;
		z0 = 0;
		t = 0;
	}
	~KEY(){}
	double GetSigma();
//	double SetSigma(double param);
	double GetB();
//	double SetB(double param);
	double GetR();
//	double SetR(double param);
	double GetX0();
//	double SetX0(double param);
	double GetY0();
//	double SetY0(double param);
	double GetZ0();
//	double SetZ0(double param);
	int GetIterationTime();
//	double SetIterationTime(int param);

	void GenerateKey(string strKeyFileName);
	bool LoadKeyFromFile(string strKeyFileName);

	/*#ifdef _DEBUG
	     friend istream operator>>(istream& in, Key &key);
	     friend ostream operator<<(ostream& out, Key &key);
	#endif  */
};

#endif
