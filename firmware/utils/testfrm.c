
#include <stdint.h>
#include <stdio.h>

void
main()
{
	// 2K instructions * 4 bytes each = 8K
	
	uint8_t	hb = 0xff;

	FILE*	output = fopen("update.frm", "wb");

	for (int i=0; i<2048; i++)
	{
		uint32_t	v = i | (hb << 16);

#if 1
		fprintf(output, "%04x %06x\n", 0x8000 + 2*i, v);
#else
		uint8_t	a = (v & 0x000000ff) >> 0;
		uint8_t b = (v & 0x0000ff00) >> 8;
		uint8_t c = (v & 0x00ff0000) >> 16;
		uint8_t d = (v & 0xff000000) >> 24;

		fprintf(output, "%c", a);
		fprintf(output, "%c", b);
		fprintf(output, "%c", c);
		fprintf(output, "%c", d);
#endif

		hb--;
	}

	fclose(output);
}

