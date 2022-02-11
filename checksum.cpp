#pragma once
#include <cstring>

unsigned int UIntFromDwordLittle(unsigned char* data)
{
	return ((unsigned int)data[0] | 
			(unsigned int)data[1] << 8 | 
			(unsigned int)data[2] << 16 | 
			(unsigned int)data[3] << 24);
}

void SetChecksumFromIntLittle(unsigned char* data, unsigned int offset, unsigned int value)
{
	data[offset] = value & 0xff;
	data[offset + 1] = (value >> 8) & 0xff;
	data[offset + 2] = (value >> 16) & 0xff;
	data[offset + 3] = (value >> 24) & 0xff;
}

extern "C"
{
	__declspec(dllexport) void UpdChecksum(unsigned char* data, unsigned int size)
	{
		unsigned int e_lfanew = UIntFromDwordLittle(&data[0x3c]);
		unsigned int checksumOffset = e_lfanew + 4 + 20 + 64;
		unsigned long long checksum = 0;
		unsigned int remainder = size % 4;
		unsigned int dataLen = size + ((4 - remainder) * (remainder != 0));

		unsigned int dword = 0;
		for (unsigned int i = 0; i < dataLen / 4; i++)
		{
			if (i == checksumOffset / 4)
				continue;
			if ((i + 1 == dataLen / 4) && remainder)
			{
				unsigned char tmp[4] = { 0 };
				memcpy(tmp, &data[i * 4], remainder);
				dword = UIntFromDwordLittle(tmp);
			}
			else
			{
				dword = UIntFromDwordLittle(&data[i * 4]);
			}

			checksum += dword;
			if (checksum >= 4294967296)
				checksum = (checksum & 0xffffffff) + (checksum >> 32);
		}

		checksum = (checksum & 0xffff) + (checksum >> 16);
		checksum = checksum + (checksum >> 16);
		checksum = checksum & 0xffff;
		checksum = checksum + size;
		SetChecksumFromIntLittle(data, checksumOffset, checksum);
	}
}