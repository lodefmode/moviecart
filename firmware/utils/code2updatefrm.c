#include <stdio.h>
#include <stdint.h>

#if 0
       Line      Address       Opcode      Label             DisAssy          
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
#endif

void main()
{
	FILE*	output = fopen("update.frm", "wb");

	char	linput[100];
	while(!feof(stdin))
	{
		if (!fgets(linput, 100, stdin))
			break;

		uint32_t	v = 0;

		char c = linput[27];
		if (isxdigit(c))
		{
			for (int i=27; i<=32; i++)
			{
				char c = linput[i];

				v *= 16;

				if (c >= '0' && c <= '9')
					v += c - '0';
				else if (c >= 'A' && c <= 'F')
					v += c - 'A' + 10;
			}

			fwrite(&v, sizeof(v), 1, output);
			fprintf(stdout, "%06x\n", v);
		}
	}

	fclose(output);

}

