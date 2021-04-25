
#include <stdio.h>
#include <stdlib.h>

void
main(int argc, char **argv)
{
	int VERSION_DATA_OFFSET = 0;
	int FRAME_DATA_OFFSET = 4;
	int AUDIO_DATA_OFFSET = 7;
	int GRAPH_DATA_OFFSET = 269;
	int TIMECODE_DATA_OFFSET = 1229;
	int COLOR_DATA_OFFSET = 1289;
	int END_DATA_OFFSET = 2249;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: inputfile outputfile\n");
		exit(1);
	}

	FILE*	input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		exit(1);
	}

	FILE*	output = fopen(argv[2], "wb");
	if (!output)
	{
		fprintf(stderr, "Cannot open %s\n", argv[2]);
		exit(1);
	}


	char	frame[4096];

	while (!feof(input))
	{
		int cnt = fread(frame, 1, 4096, input);

		// unswap colors, as they're not needed in latest kernel.

		// 8912034675
		// 8945671032

		for (int i = COLOR_DATA_OFFSET; i < END_DATA_OFFSET; i+= 10)
		{
			char	color[10];
			for (int j=0; j<10; j++)
				color[j] = frame[i + j];

			frame[i + 0] = color[8];
			frame[i + 1] = color[9];
			frame[i + 2] = color[1];
			frame[i + 3] = color[2];
			frame[i + 4] = color[0];
			frame[i + 5] = color[3];
			frame[i + 6] = color[4];
			frame[i + 7] = color[6];
			frame[i + 8] = color[7];
			frame[i + 9] = color[5];
		}

		fwrite(frame, 1, cnt, output);
	}

	fclose(input);
	fclose(output);


}
