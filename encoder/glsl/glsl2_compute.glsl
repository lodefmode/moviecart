// Floyd-steinberg in which each cell (8 pixels) can be one of 2 colors,
// but second color (background) is common for entire horizontal line

// Original image is input 1/3
// Palette is is input 2/3
// Background score is input 3/3  (for each x=background index, y=horizontal line)

// Output:
//  r = graph bits (1=use foreground, 0=use background)
//  g = foreground palette index
//  b = background palette index (identical for entire horizontal line)


//layout (local_size_x = 8, local_size_y = 4, local_size_z = 4) in;
layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

uniform float bleedScale;

uniform vec4 weight0;
uniform vec4 weight1;
uniform vec4 weight2;

shared float foreDist[128]; // shared implies coherent
shared uint foreIndex[128];
shared uint foreGraph[128];

shared float backDist[128]; // shared implies coherent
shared uint backIndex[128];

shared vec4	currLine[128];
shared vec4	nextLine1[128+2];
shared vec4	nextLine2[128+2];

shared vec4 currOffset1;
shared vec4 currOffset2;

float
colorDist(const vec4 a, const vec4 b)
{
	vec4	diff = a - b;
	vec4	diffs = diff * vec4(0.299f, 0.587f, 0.114f, 0);
	return	dot(diff, diffs);
}


// fold in half, quicker search

uint
sortForeArray(uint f)
{
	uint	fbit =64;
	for (uint i=0; i<7; i++)
	{
		uint	i1 = (f & (fbit-1));
		uint	i2 = (f | fbit);

		float   d1 = foreDist[i1];
		float   d2 = foreDist[i2];

		uint	f2 = foreIndex[i2];
		uint	g2 = foreGraph[i2];

		if (d2 < d1)
		{
			foreDist[f] = d2;
			foreIndex[f] = f2;
			foreGraph[f] = g2;
		}

		fbit /= 2;
		barrier();
	}

	return foreIndex[0];
}

uint
sortBackArray(uint f)
{
	uint	fbit =64;
	for (uint i=0; i<7; i++)
	{
		uint	i1 = (f & (fbit-1));
		uint	i2 = (f | fbit);

		float	d1 = backDist[i1];
		float	d2 = backDist[i2];
		uint	f2 = backIndex[i2];

		if (d2 < d1)
		{
			backDist[f] = d2;
			backIndex[f] = f2;
		}

		fbit /= 2;
		barrier();
	}

	return backIndex[0];
}

void
main()
{
	memoryBarrierShared(); // Ensure change to shared is visible in other invocations

	// parallelized
//	uint		f = gl_LocalInvocationID.y * 8 + gl_LocalInvocationID.x;
	uint		f = gl_LocalInvocationIndex;
	uint		f8 = (f&7);

	ivec2	foreCoord = ivec2(f, 0);
	vec4	foreColor = texelFetch(sTD2DInputs[1], foreCoord, 0);

	currLine[f] = texelFetch(sTD2DInputs[0], ivec2(f, 0), 0);
	nextLine1[f] = texelFetch(sTD2DInputs[0], ivec2(f, 1), 0);
	nextLine2[f] = texelFetch(sTD2DInputs[0], ivec2(f, 2), 0);
	barrier();

	uint	width = uint(uTD2DInfos[0].res.z);
	uint	height = uint(uTD2DInfos[0].res.w);

	for (uint y=0; y<height; y++)
	{
		uint	bidx = 0;

		// sort best background texture
		{
			ivec2	coord = ivec2(f, y);
			vec4	color = texelFetch(sTD2DInputs[2], coord, 0);

			backDist[f] = color.r;
			backIndex[f] = f;
			barrier();

			bidx = sortBackArray(f);
		}

		ivec2	backCoord = ivec2(bidx, 0);
		vec4	backColor = texelFetch(sTD2DInputs[1], backCoord, 0);

		currOffset1 = vec4(0);
		currOffset2 = vec4(0);
		barrier();

		for (uint x=0; x<width; x+=8)
		{
			// calculate cell distance (floyd-steinberg) 
			{
				float		cellDist = 0.0f;
				uint		graph = 0;
				vec4		currOffsetA = currOffset1;
				vec4		currOffsetB = currOffset2;

				for (uint c=0; c<8; c++)
				{
					uint xx = x+c;
					
					vec4	color = currOffsetA + currLine[xx];
					clamp(color, 0, 1);
							
					float distf = colorDist(color, foreColor);
					float distb = colorDist(color, backColor);

					vec4	oc;

					graph <<= 1;

					if (distf < distb)
					{
						oc = foreColor;
						cellDist += distf;
						graph |= 1;
					}
					else
					{
						oc = backColor;
						cellDist += distb;
					}
					
					vec4 diffColor = (color - oc) * bleedScale;
					currOffsetA = diffColor * weight0[2] + currOffsetB;
					currOffsetB = diffColor * weight0[3];
				}

				// stuff in results
				foreDist[f] = cellDist;
				foreIndex[f] = f;
				foreGraph[f] = graph;
				barrier();
			}

			
			// find best on each thread
			uint	bestF = sortForeArray(f);

			// only one thread to update
			if (gl_LocalInvocationID == vec3(0))
			{
				// redo with bestF color now
				vec4	foreColor = texelFetch(sTD2DInputs[1], ivec2(bestF, 0), 0);
				uint	graph = foreGraph[bestF];

				for (uint c=0; c<8; c++)
				{
					uint xx = x+c;
					
					vec4	color = currOffset1 + currLine[xx];
					clamp(color, 0, 1);
							
					float distf = colorDist(color, foreColor);
					float distb = colorDist(color, backColor);

					vec4	oc;
					
					if ((graph & 128) == 0)
						oc = backColor;
					else
						oc = foreColor;

					graph <<= 1;
					
					vec4 diffColor = (color - oc) * bleedScale;
					currOffset1 = diffColor * weight0[2] + currOffset2;
					currOffset2 = diffColor * weight0[3];

					nextLine1[xx-1 + 1] += diffColor * weight1[0];
					clamp(nextLine1[xx-1 + 1], 0, 1);

					nextLine1[xx+0 + 1] += diffColor * weight1[1];
					clamp(nextLine1[xx+0 + 1], 0, 1);

					nextLine1[xx+1 + 1] += diffColor * weight1[2];
					clamp(nextLine1[xx+1 + 1], 0, 1);

					nextLine2[xx+0 + 1] += diffColor * weight2[1];
					clamp(nextLine2[xx+0 + 1], 0, 1);
				}
			}
			barrier();

			if (f < 8)
			{
				uint	gdata = (foreGraph[bestF] & (1<<(7-f8)));

				if (gdata != 0)
					gdata = 1;

				vec4	cdata = vec4(gdata, bestF/255.0f, bidx/255.0f, 1);
				imageStore(mTDComputeOutputs[0], ivec2(x + f, y), cdata);
			}

		} // for x


		currLine[f] = nextLine1[f + 1];
		barrier();

		nextLine1[f] = nextLine2[f];
		barrier();

		nextLine2[f] = texelFetch(sTD2DInputs[0], ivec2(f, y+2), 0);
		barrier();


	} // for y

}
