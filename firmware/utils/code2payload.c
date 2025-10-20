#include <stdio.h>
#include <stdint.h>

#if 0
   Line      Address       Opcode      Label             DisAssy          

input:

    9,217    4800          B3C010       main2      MOV.B #0x1, W0             
    9,218    4802          07E9D7                  RCALL flash_led            
    9,219    4804          B3C020                  MOV.B #0x2, W0             
    9,220    4806          07E9D5                  RCALL flash_led            
    9,221    4808          B3C030                  MOV.B #0x3, W0             
    9,222    480A          07E9D3                  RCALL flash_led            
    9,223    480C          B3C010                  MOV.B #0x1, W0             
    9,224    480E          07E9D1                  RCALL flash_led            
    9,225    4810          B3C020                  MOV.B #0x2, W0             
    9,226    4812          07E9CF                  RCALL flash_led            
    9,227    4814          B3C030                  MOV.B #0x3, W0             
    9,228    4816          07E9CD                  RCALL flash_led            
    9,229    4818          37FFF3                  BRA main2                  


output:

/*
 format:
 crc:		24 bit value // includes version, dest, length + data
 version:	24 bit value 
 dest:		24 bit value 
 length:	24 bit value // length in (24 bit) values following this section
*/

.global _payloadData
.section .payload,code
_payloadData:

	.pword  0xDF1C07 	/* crc */
	.pword	0x000001	/* version */
	.pword	0x000000	/* dest */
	.pword	320			/* length */

	.pword 0xc00001, 0x1fd93e, 0x0ff8c1, 0x011ffb, 0xf10000, 0xf0866c, 0x1f07ff, 0x8d3ee7
	.pword 0x833ff8, 0xff7ff0, 0xfefce0, 0x0f0eff, 0x37f0d8, 0xbfde77, 0xef077b, 0xe00739
	.pword 0xfc0ffe, 0x7eff7f, 0xf1dfc7, 0xf0033f, 0x3ff8c7, 0x03f7f7, 0xffff37, 0xff8c63
	.pword 0x9cfaff, 0xffbfe3, 0x6771ff, 0xfcffff, 0xbf7377, 0x73ffff, 0xffff9d, 0x9c9cfa
	.pword 0xffff3f, 0xff99cc, 0xdcfaff, 0xff7f98, 0x66ccff, 0xfcffff, 0x7f7767, 0x31ffff
#endif

void
addToCrc(uint32_t* crc, uint32_t v)
{
//	fprintf(stdout, " ADDING 0x%06x to 0x%06x \n", v, *crc);

	*crc ^= v;
	for (uint8_t j=0; j<24; j++)
	{
		if (*crc & 1) 
			*crc = (*crc >> 1) ^ 0xEDB88320;
		else 
			*crc >>= 1;
	}
}

uint32_t calculateCode(const char* linput);

void
main(int argc, const char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
		exit();
	}

	FILE*		input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Unable to open input %s\n", argv[1]);
		exit();
	}

	FILE*		output = fopen(argv[2], "wb");
	if (!output)
	{
		fprintf(stderr, "Unable to open output %s\n", argv[1]);
		exit();
	}


	char		linput[500];


	uint32_t	version = 1;
	uint32_t	dest = 0x00;

	int			len = 0;
	int			len2 = 0;



    uint32_t crc = 0xFFFFFFFF;

	for (int loop=0; loop<3; loop++)
	{
		if (loop == 1)
		{
			// do in this order first
			addToCrc(&crc, version);
			addToCrc(&crc, dest);
			addToCrc(&crc, len);
		}

		// print crc, etc
		if (loop == 2)
		{
//			fprintf(stdout, "Final CRC 0x%06x\n", crc);

			crc = crc ^ 0xFFFFFFFF;

//			fprintf(stdout, "    ^ CRC 0x%06x\n", crc);

			crc &= 0xFFFFFF;	// 24 bits

//			fprintf(stdout, "    & CRC 0x%06x\n", crc);

			fprintf(output, 
				"\n"
				"/*\n"
				" format:\n"
				" crc:       24 bit value // includes version, dest, length + data\n"
				" version:   24 bit value\n"
				" dest:      24 bit value\n"
				" length:    24 bit value // length in (24 bit) values following this section\n"
				"*/\n"
				"\n"
				".global _payloadData\n"
				".section .payload,code\n"
				"_payloadData:\n"
			);

			fprintf(output, "\n");

			fprintf(output, "\t.pword 0x%06x\t/* crc */\n", crc);
			fprintf(output, "\t.pword 0x%06x\t/* version */\n", version);
			fprintf(output, "\t.pword 0x%06x\t/* dest */\n", dest);
			fprintf(output, "\t.pword 0x%06x\t/* length %d */\n", len, len);

			fprintf(output, "\n\n");
		}

		len = 0;
		len2 = 0;

		fseek(input, 0, SEEK_SET);
		while(!feof(input))
		{
			if (!fgets(linput, 500, input))
				break;

			uint32_t	v = calculateCode(linput);

			// calculate crc
			if (loop == 1)
				addToCrc(&crc, v);

			// print payload
			if (loop == 2)
			{
				if (len2 == 0)
					fprintf(output, "\t.pword ");
				else
					fprintf(output, ",");

				fprintf(output, " 0x%06x", v);

				if (len2 == 7)
					fprintf(output, "\n");
			}

			len2++;
			len2 %= 8;

			len++;
		}
	}

	fprintf(output, "\n");
	fprintf(output, "\n");

	fclose(input);
	fclose(output);

}

