#pragma once

#include <stdio.h>

class LogWrite
{
public:
	LogWrite();
	~LogWrite();

	double BaseTick;
	double SyncBufferTicks;
	int LoopPoint;
	int DeviceCount;
	int DataLength;

	// 分子 / 分母
	int Numerator;
	int Denominator;


	bool Open(const char *filename);
	void Close();

	void SetLoopPoint();
	void Wait(double Seconds);
	void WriteData(int Device, int Address, int Value);
	void WriteEnd();

	void WriteDword(unsigned char *buf, int value);
	void WriteWord(unsigned char *buf, int value);
	void WriteHeader();

	void WriteValue(int value);
	void WriteSync();
	void WriteSync1();
	void WriteSync2(int Count);

	void WriteAdpcmMemory(void* pData, int size);



private:
	FILE *fp;
	bool Loop;
	bool HeaderOut;
};

