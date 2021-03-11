//
// Create a series of lookup tables for 4 bit audio over a set of volume levels
// Volume changes more pronounced at higher levels.
//


#include <stdio.h>
#include <stdlib.h>

void main()
{
#define MAX_LEVELS	  11
#define DEFAULT_LEVEL 6

	float	fromA = 0.0f;
	float	toA = DEFAULT_LEVEL;

	float	fromB = 0.0f;
	float	toB = 1.0f;

	fprintf(stdout, "#define MAX_VOLUME %d\n", MAX_LEVELS);
	fprintf(stdout, "#define DEFAULT_VOLUME %d\n", DEFAULT_LEVEL);

	for (int i=0; i<MAX_LEVELS; i++)
	{
		float	scale = (i - fromA) / (toA - fromA) * (toB - fromB) + fromB;

		if (i > DEFAULT_LEVEL)
			scale = (scale * scale);

		fprintf(stdout, "const uint8_t scale%d[16] = {", i, scale);

		for (int j=0; j<16; j++)
		{
			float	diff = j - 7.5f;

			int vi = (7.5f + diff*scale + 0.5f);

			if (vi < 0)
				vi = 0;
			if (vi > 15)
				vi = 15;

			if (j)
				fprintf(stdout, ",");
			fprintf(stdout, "%3d", vi);
		}

		fprintf(stdout, "}; /* %0.4f */ \n", scale);
	}

	fprintf(stdout, "\n");
	fprintf(stdout, "const uint8_t	*scales[%d] = {", MAX_LEVELS);

	for (int i=0; i<MAX_LEVELS; i++)
	{
		if (i)
			fprintf(stdout, ", ");
		fprintf(stdout, "scale%d", i);
	}
	fprintf(stdout, "};\n");

}

