#pragma once
#define POLY16	0x1021

namespace CRC
{
	unsigned int crc32_table[256] = { 0 };

	unsigned short crc16(unsigned char *buf, int len, unsigned short crc)
	{
		int i, j;
		for (i = 0; i < len; i++)
		{
			crc ^= (buf[i] << 8);
			for (j = 0; j < 8; j++)
			{
				if ((crc & 0x8000) > 0)
					crc = ((crc << 1) ^ POLY16);
				else
					crc <<= 1;
			}
		}
		return crc;
	}
	
	void MakeCrc32Table()
	{
		unsigned int c;
		int i = 0;
		int bit = 0;

		for (i = 0; i < 256; i++)
		{
			c = i;
			for (bit = 0; bit < 8; bit++)
			{
				if (c & 1)
				{
					c = (c >> 1) ^ (0xEDB88320);
				}
				else
				{
					c = c >> 1;
				}
			}
			crc32_table[i] = c;
		}
	}

	unsigned int crc32(char *buf, int len, unsigned int crc)
	{
		unsigned char *p = (unsigned char *)buf;

		while (len-- > 0)
		{
			crc = (crc >> 8) ^ (crc32_table[(crc ^ *p++) & 0xFF]);
		}
		return ~crc;
	}

}
