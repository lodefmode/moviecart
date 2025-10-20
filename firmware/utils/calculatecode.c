#include <stdint.h>
#include <stdio.h>

uint32_t
calculateCode(const char* linput)
{
	const char*	code = linput;

	for (int i=0; i<3; i++)
	{
		while (!isspace(*code))
			code++;

		while (isspace(*code))
			code++;
	}

	uint32_t v = 0;

	for (int i=0; i<=5; i++)
	{
		int	c = code[i];
		if (!isxdigit(c))
		{
			fprintf(stderr, "ERROR %s", code);
			exit(1);
		}

		v *= 16;

		if (c >= '0' && c <= '9')
			v += c - '0';
		else if (c >= 'A' && c <= 'F')
			v += c - 'A' + 10;
	}

	return v;
}
