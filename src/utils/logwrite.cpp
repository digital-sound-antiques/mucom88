#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "logwrite.h"

LogWrite::LogWrite() {
	BaseTick = 0;
	SyncBufferTicks = 0;
	LoopPoint = 0;
	DataLength = 0;

	Numerator = 0;
	Denominator = 0;
	DeviceCount = 0;

	HeaderOut = false;

	BaseTick = (double)10 / 1000;
	Loop = false;
}

LogWrite::~LogWrite() {
	Close();
}

bool LogWrite::Open(const char *filename) {
	fp = fopen(filename,"wb");
	return fp != NULL;
}

void LogWrite::Close(void) {
	WriteSync();
	WriteEnd();
	if (fp != NULL) fclose(fp);
	fp = NULL;
}

void LogWrite::WriteHeader() {
	HeaderOut = true;
	if (fp == NULL) return;
	fseek(fp, 0, SEEK_SET);

	unsigned char header[0x20];
	memset(header, 0, 0x20);
	memcpy(header, "S983",4);
	WriteDword(header + 0x04, Numerator);
	WriteDword(header + 0x08, Denominator);
	WriteDword(header + 0x0c, 0); // Reserved

	WriteDword(header + 0x10, 0); // Tag

	int DumpStart = 0x20 + (DeviceCount * 0x10);
	WriteDword(header + 0x14, DumpStart);
	WriteDword(header + 0x18, LoopPoint);
	WriteDword(header + 0x1c, DeviceCount);

	fwrite(header, 0x20, 1, fp);
}

void LogWrite::WriteDword(unsigned char *data, int value) {
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
	data[2] = (value >> 16) & 0xff;
	data[3] = (value >> 24) & 0xff;
}

void LogWrite::WriteWord(unsigned char *data, int value) {
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
}

void LogWrite::WriteData(int Device, int Address, int Value) {
	WriteSync();
	int d = (int)(Device * 2 + (Address >= 0x100 ? 1 : 0));
	int a = (int)(Address & 0xff);
	int v = (int)(Value & 0xff);
	WriteValue(d);
	WriteValue(a);
	WriteValue(v);
}


void LogWrite::WriteValue(int value) {
	if (!HeaderOut) WriteHeader();
	if (fp != NULL) fputc(value, fp);
	DataLength++;
}

void LogWrite::WriteSync() {
	if (SyncBufferTicks < BaseTick) return;

	int SyncCount = (int)(SyncBufferTicks / BaseTick);
	SyncBufferTicks -= BaseTick * SyncCount;
	if (SyncCount > 1) WriteSync2(SyncCount); else WriteSync1();
}

void LogWrite::WriteSync2(int Count) {
	Count -= 2;
	WriteValue(0xfe);
	while (true) {
		int v = (Count & 0x7f);
		Count = Count >> 7;
		bool Next = Count > 0;
		v = v | (Next ? 0x80 : 0x00);

		WriteValue(v & 0xff);
		if (!Next) break;
	}
}

void LogWrite::WriteSync1() {
	WriteValue(0xfe);
}

void LogWrite::WriteEnd() {
	WriteValue(0xfd);
}

void LogWrite::SetLoopPoint() {
	Loop = true;
	LoopPoint = DataLength;
}

void LogWrite::Wait(double seconds) {
	SyncBufferTicks += seconds;
}

