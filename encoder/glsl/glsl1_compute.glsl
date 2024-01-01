// Floyd-steinberg in which each cell (8 pixels) can be one of 2 colors,
// but second color (background) is common for entire horizontal line

// Original image is input 1/3
// Palette is is input 2/3

// Output:
// pixel(x,y) is the background score for each x=background index, y=horizontal line
//  rgba is treated as one single int

//layout (local_size_x = 8, local_size_y = 4, local_size_z = 4) in;
layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;


uniform float bleedScale;
uniform vec4 weight0;
//uniform vec4 weight1;
//uniform vec4 weight2;

shared float foreDist[128]; // shared implies coherent
shared uint foreIndex[128];
shared vec4 foreOffset1[128];
shared vec4 foreOffset2[128];


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
		vec4	o12 = foreOffset1[i2];
		vec4	o22 = foreOffset2[i2];

#if 0	// not faster?
		uint	f1 = foreIndex[i1];
		float	edge = step(d2, d1);
		float	minD = mix(d1, d2, edge);
		float	minF = mix(f1, f2, edge);

		foreDist[f] = minD;
		foreIndex[f] = uint(minF);
		foreOffset1[f] = ... 
#else

		if (d2 < d1)
		{
			foreDist[f] = d2;
			foreIndex[f] = f2;
			foreOffset1[f] = o12;
			foreOffset2[f] = o22;
		}
#endif

		fbit /= 2;
		barrier();
	}

	return foreIndex[0];
}

void
main()
{
	memoryBarrierShared(); // Ensure change to shared is visible in other invocations

	uint	f = gl_LocalInvocationIndex;
	uint	f8 = (f&7);

	ivec2	foreCoord = ivec2(f, 0);
	vec4	foreColor = texelFetch(sTD2DInputs[1], foreCoord, 0);

	uint	y = gl_WorkGroupID.y;
	uint	bidx = gl_WorkGroupID.x;

	ivec2	backCoord = ivec2(bidx, 0);
	vec4	backColor = texelFetch(sTD2DInputs[1], backCoord, 0);

	float	lineDist = 0;

    uint	width = uint(uTD2DInfos[0].res.z);
    uint	height = uint(uTD2DInfos[0].res.w);

	vec4	currOffset1 = texelFetch(sTD2DInputs[0], ivec2(0, y), 0);
	vec4	currOffset2 = texelFetch(sTD2DInputs[0], ivec2(1, y), 0);

	for (uint x=0; x<width; x+=8)
	{
		// calculate cell distance (floyd-steinberg) 
		{
			float	cellDist = 0.0f;

			for (uint c=0; c<8; c++)
			{
				uint xx = x+c;
				
				vec4	color = currOffset1;
				clamp(color, 0, 1);
						
				float distf = colorDist(color, foreColor);
				float distb = colorDist(color, backColor);

				vec4	oc;

				if (distf < distb)
				{
					oc = foreColor;
					cellDist += distf;
				}
				else
				{
					oc = backColor;
					cellDist += distb;
				}
				
				vec4 diffColor = (color - oc) * bleedScale;
				currOffset1 = diffColor * weight0[2] + currOffset2;
				clamp(currOffset1, 0, 1);

				vec4	tcolor = texelFetch(sTD2DInputs[0], ivec2(xx+2, y), 0);
				currOffset2 = diffColor * weight0[3] + tcolor;
				clamp(currOffset2, 0, 1);
			}

			// stuff in results
			foreDist[f] = cellDist;
			foreIndex[f] = f;
			foreOffset1[f] = currOffset1;
			foreOffset2[f] = currOffset2;
			barrier();
		}

		
		// find best on each thread
		uint	bestF = sortForeArray(f);

		lineDist += foreDist[bestF];
		currOffset1 = foreOffset1[bestF];
		currOffset2 = foreOffset2[bestF];

	} // x


	// now store float lineDist at (bidx,y)
	vec4 outColor = vec4(lineDist, 0, 0, 1);

	imageStore(mTDComputeOutputs[0], ivec2(bidx, y), outColor);

}
