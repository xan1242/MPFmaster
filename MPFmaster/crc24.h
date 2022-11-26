#include <stdint.h>
#define CRC24_INIT 0xB704CEL
#define CRC24_POLY 0x1864CFBL
typedef long crc24;
crc24 crc_octets(unsigned char* octets, size_t len)
{
	crc24 crc = CRC24_INIT;
	int i;
	while (len--) {
		crc ^= (*octets++) << 16;
		for (i = 0; i < 8; i++) {
			crc <<= 1;
			if (crc & 0x1000000)
				crc ^= CRC24_POLY;
		}
	}
	return crc & 0xFFFFFFL;
}
