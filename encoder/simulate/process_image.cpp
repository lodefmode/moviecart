
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class vec4
{
public:

	vec4(float r1, float g1, float b1, float a1) : r(r1), g(g1), b(b1), a(a1)
	{
	}

	vec4(float val=0) : r(val), g(val), b(val), a(val)
	{
	}

	float	r, g, b, a;

	int	operator==(const vec4& v) const
	{
		return(
			r == v.r &&
			g == v.g &&
			b == v.b &&
			a == v.a
		);
	}

	vec4&	operator=(const vec4& v)
	{
		r = v.r;
		g = v.g;
		b = v.b;
		a = v.a;
		return *this;
	}

	float& operator[](unsigned i)
	{
		switch(i)
		{
			case 0: return r;
			case 1: return g;
			case 2: return b;

			default:
			case 3:
				return a;
		}
	}

	const float& operator[](unsigned i) const
	{
		switch(i)
		{
			case 0: return r;
			case 1: return g;
			case 2: return b;

			default:
			case 3:
				return a;
		}
	}

	vec4&	operator*=(float v)
	{
		r *= v;
		g *= v;
		b *= v;
		a *= v;
		return *this;
	}

	vec4&	operator*=(const vec4 v)
	{
		r *= v.r;
		g *= v.g;
		b *= v.b;
		a *= v.a;
		return *this;
	}

	vec4	operator+(const vec4 v) const
	{
		return vec4(r+v.r, g+v.g, b+v.b, a+v.a);
	}

	vec4	operator-(const vec4 v) const
	{
		return vec4(r-v.r, g-v.g, b-v.b, a-v.a);
	}

	vec4	operator*(const vec4 v) const
	{
		return vec4(r*v.r, g*v.g, b*v.b, a*v.a);
	}

	vec4	operator*(float v) const
	{
		return vec4(r*v, g*v, b*v, a*v);
	}

};

class ivec2
{
public:

	ivec2(int x1, int y1) : x(x1), y(y1)
	{
	}

	int	x, y;

	ivec2&	operator=(const ivec2& v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

};

float
dot(const vec4 s, const vec4 t)
{
	return s.r*t.r + s.g*t.g + s.b*t.b + s.a*t.a;
}

void
clamp(vec4 v, float min, float max)
{
	for (int i=0; i<3; i++)
	{
		if (v[i]<min) v[i] = min;
		if (v[i]<max) v[i] = max;
	}
}

typedef unsigned int uint;

class
Buffer
{
public:

	Buffer(uint w, uint h)
	{
		width = w;
		height = h;
		data = new vec4 [width*height];

		memset(data, 0, sizeof(vec4) * width * height);
	}

	~Buffer()
	{
		delete [] data;
	}

	void
	copy(const Buffer& input)
	{
		for (int y=0; y<height && y<input.height; y++)
		{
			for (int x=0; x<width && x<input.width; x++)
			{
				get(x,y) = input.get(x,y);
			}
		}
	}

	vec4&
	get(uint x, uint y)
	{
		return data[y*width + x];
	}

	const vec4&
	get(uint x, uint y) const
	{
		return data[y*width + x];
	}

	void
	writeOutput()
	{
		for (int y=0; y<height; y++)
		{
			for (int i=0; i<4; i++)
			{
				for (int x=0; x<width; x++)
				{
					printf("%g", get(x,y)[i]);
					if (x < (width-1))
						printf(" ");
				}
				printf("\n");
			}
		}
	}

	void
	readInput()
	{
		for (int y=0; y<height; y++)
		{
			for (int i=0; i<4; i++)
			{
				for (int x=0; x<width; x++)
				{
					float	v;
					scanf("%f", &v);
					get(x,y)[i] = v;
				}
			}
		}
	}

	uint	width;
	uint	height;

private:

	vec4*	data;

};

Buffer	tempInput(128, 192 + 128*6);
Buffer	originalInput(80, 192);
Buffer	palInput(128, 1);

Buffer	finalOutput(128, 192 + 128*6);

Buffer* sTD2DInputs[3] = {&tempInput, &originalInput, &palInput};
Buffer* mTDComputeOutputs[1] = {&finalOutput};

uint	uTDPass;	// 0..191
vec4	gl_WorkGroupID;	// x/r 0..127
//uint	gl_LocalInvocationIndex;	// f 0..127


void
imageStore(Buffer* buffer, ivec2 coord, vec4 color)
{
	buffer->get(coord.x, coord.y) = color;
}

vec4
texelFetch(Buffer* buffer, ivec2 coord, int dummy)
{
	return buffer->get(coord.x, coord.y);
}

int ipal[128][3] =
{
	0,0,0, 74,74,74, 111,111,111, 142,142,142, 170,170,170, 192,192,192, 214,214,214, 236,236,236,
	72,72,0, 105,105,15, 134,134,29, 162,162,42, 187,187,53, 210,210,64, 232,232,74, 252,252,84,
	124,44,0, 144,72,17, 162,98,33, 180,122,48, 195,144,61, 210,164,74, 223,183,85, 236,200,96,
	144,28,0, 163,57,21, 181,83,40, 198,108,58, 213,130,74, 227,151,89, 240,170,103, 252,188,116,
	148,0,0, 167,26,26, 184,50,50, 200,72,72, 214,92,92, 228,111,111, 240,128,128, 252,144,144,
	132,0,100, 151,25,122, 168,48,143, 184,70,162, 198,89,179, 212,108,195, 224,124,210, 236,140,224,
	80,0,132, 104,25,154, 125,48,173, 146,70,192, 164,89,208, 181,108,224, 197,124,238, 212,140,252,
	20,0,144, 51,26,163, 78,50,181, 104,72,198, 127,92,213, 149,111,227, 169,128,240, 188,144,252,
	0,0,148, 24,26,167, 45,50,184, 66,72,200, 84,92,214, 101,111,228, 117,128,240, 132,144,252,
	0,28,136, 24,59,157, 45,87,176, 66,114,194, 84,138,210, 101,160,225, 117,181,239, 132,200,252,
	0,48,100, 24,80,128, 45,109,152, 66,136,176, 84,160,197, 101,183,217, 117,204,235, 132,224,252,
	0,64,48, 24,98,78, 45,129,105, 66,158,130, 84,184,153, 101,209,174, 117,231,194, 132,252,212,
	0,68,0, 26,102,26, 50,132,50, 72,160,72, 92,186,92, 111,210,111, 128,232,128, 144,252,144,
	20,60,0, 53,95,24, 82,126,45, 110,156,66, 135,183,84, 158,208,101, 180,231,117, 200,252,132,
	48,56,0, 80,89,22, 109,118,43, 136,146,62, 160,171,79, 183,194,95, 204,216,110, 224,236,124,
	72,44,0, 105,77,20, 134,106,38, 162,134,56, 187,159,71, 210,182,86, 232,204,99, 252,224,112
};


void
initPal()
{
	for (int p=0; p<128; p++)
	{
		palInput.get(p, 0) = vec4(ipal[p][0], ipal[p][1], ipal[p][2], 255.0f);
		palInput.get(p, 0) *= (1.0f / 255.0f);
	}
}

// Example Compute Shader
//layout (local_size_x = 128, local_size_y = 1) in;


const float bleedScale = 1.0f;
const vec4 weight0 = vec4(0, 0, 0.4375, 0);
const vec4 weight1 = vec4(0.1875, 0.3125, 0.0625, 0.0);
const vec4 weight2 = vec4(0, 0, 0, 0);

float arrayDist[128]; // shared implies coherent
uint arrayIndex[128];
uint arrayGraph[128];

vec4 currOffset1;
vec4 currOffset2;

void
barrier()
{
}
void
memoryBarrierShared()
{
}

// fold in half, quicker search

#if 0
uint
sortArray(uint f)
{
	uint	fbit =64;
	for (uint i=0; i<7; i++)
	{
		uint	i1 = (f & (fbit-1));
		uint	i2 = (f | fbit);

		float   d1 = arrayDist[i1];
		float   d2 = arrayDist[i2];

		uint	f2 = arrayIndex[i2];

		if (d2 < d1)
		{
			arrayDist[f] = d2;
			arrayIndex[f] = f2;
		}

		fbit /= 2;
		barrier();
	}

	return arrayIndex[0];
}
#endif

uint
sortArray()
{
	float	bestDist = arrayDist[0];
	uint	bestIndex = 0;
	
	for (uint i=0; i<128; i++)
	{
		float   d1 = arrayDist[i];
		uint	f1 = arrayIndex[i];

		if (d1 < bestDist)
		{
			bestDist = d1;
			bestIndex = f1;
		}
	}

	return bestIndex;
}

float
colorDist(const vec4 a, const vec4 b)
{
	vec4	diff = a - b;
	vec4	diffs = diff * vec4(0.299f, 0.587f, 0.114f, 0);
	return	dot(diff, diffs);
}


#if 1
void mainGLSL()
{
	memoryBarrierShared(); // Ensure change to shared is visible in other invocations

	uint	iwidth = originalInput.width; // uint(uTD2DInfos[1].res.z);
	uint	iheight = originalInput.height; // uint(uTD2DInfos[1].res.w);

	uint	owidth = finalOutput.width; // uint(uTDOutputInfo.res.z);	
	uint	oheight = finalOutput.height; // uint(uTDOutputInfo.res.w);


	uint	bidx = gl_WorkGroupID.r; // gl_WorkGroupID.x;
	uint	y = uTDPass;
	uint	y0 = (y&1);

	ivec2	backCoord = ivec2(bidx, 0);
	vec4	backColor = texelFetch(sTD2DInputs[2], backCoord, 0);



	// take best 3 lines from previous pass and copy them to output
	
	uint	sourceBase, nextBase;

	// never read and write from the same area
	// so alternate each pass	
	if (y0 == 0)
	{
		sourceBase = 0 + iheight;
		nextBase = 128*3 + iheight;
	}
	else
	{
		sourceBase = 128*3 + iheight;
		nextBase = 0 + iheight;
	}

	// find best background from previous pass
//	uint	f = gl_LocalInvocationIndex;

	for (uint f=0; f<128; f++)
	{
		arrayDist[f] = texelFetch(sTD2DInputs[0], ivec2(iwidth+0, sourceBase + f*3), 0).r;
		arrayIndex[f] = f;
		barrier();
	}
//	uint	bestB = sortArray(f);
	uint	bestB = sortArray();


	uint	sline0 = bestB*3 + sourceBase + 0;
	uint	sline1 = bestB*3 + sourceBase + 1;
	uint	sline2 = bestB*3 + sourceBase + 2;
	
	uint	nline0 = bidx*3 + nextBase + 0;
	uint	nline1 = bidx*3 + nextBase + 1;
	uint	nline2 = bidx*3 + nextBase + 2;
	
	for (uint f=0; f<128; f++)
		imageStore(mTDComputeOutputs[0], ivec2(f, y+0), texelFetch(sTD2DInputs[0], ivec2(f, sline0), 0));

	sline0 = sline1;
	sline1 = sline2;

	
	// now dither, using current bidx(0..127), and testing all fidx(0..127) at each cell

	float	lineDist = 0;

	currOffset1 = vec4(0.0f);
	currOffset2 = vec4(0.0f);
	barrier();
	
	for (uint x=0; x<iwidth; x+=8)
	{
		// calculate cell distance (floyd-steinberg) 
		for (uint f=0; f<128; f++)
		{
			float		cellDist = 0.0f;
			uint		graph = 0;
			vec4		currOffsetA = currOffset1;
			vec4		currOffsetB = currOffset2;

			// can be outside loop in glsl
			ivec2	foreCoord = ivec2(f, 0);
			vec4	foreColor = texelFetch(sTD2DInputs[2], foreCoord, 0);

			for (uint c=0; c<8; c++)
			{
				uint xx = x+c;
					
				vec4	curPixel = texelFetch(sTD2DInputs[0], ivec2(xx, sline0), 0);
				vec4	color = currOffsetA + curPixel;
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
			arrayDist[f] = cellDist;
			arrayIndex[f] = f;
			arrayGraph[f] = graph;
			
			barrier();
		}

		// find best foreground on each thread
		uint	bestF = sortArray();
		vec4	bestForeColor = texelFetch(sTD2DInputs[2], ivec2(bestF, 0), 0);

		// only one thread to update
//		if (gl_LocalInvocationID == vec3(0))
//		if (gl_LocalInvocationIndex == 0)
		{
			// redo with bestF color now
		
			uint	graph = arrayGraph[bestF];

			for (uint c=0; c<8; c++)
			{
				uint xx = x+c;
					
				vec4	curPixel = texelFetch(sTD2DInputs[0], ivec2(xx, sline0), 0);
				vec4	color = currOffset1 + curPixel;
				clamp(color, 0, 1);
							
				float distf = colorDist(color, bestForeColor);
				float distb = colorDist(color, backColor);

				vec4	oc;
					
				if ((graph & 128) == 0)
					oc = backColor;
				else
					oc = bestForeColor;

				graph <<= 1;
					
				vec4 diffColor = (color - oc) * bleedScale;
				currOffset1 = diffColor * weight0[2] + currOffset2;
				currOffset2 = diffColor * weight0[3];

				// diffuse error to next lines    
				// (xx+0, y+0) = oc
				imageStore(mTDComputeOutputs[0], ivec2(xx+0, nline0), oc);

				vec4	s, p;
				
				
				// (xx-1, y+1) weight1[0];
				// (xx+0, y+1) weight1[1];
				// (xx+1, y+1) weight1[2];

				s = texelFetch(sTD2DInputs[0],   ivec2(xx-1, sline1), 0);
				p = s + diffColor * weight1[0];
				clamp(p, 0, 1);
				imageStore(mTDComputeOutputs[0], ivec2(xx-1, nline1), p);

				s = texelFetch(sTD2DInputs[0],   ivec2(xx+0, sline1), 0);
				p = s + diffColor * weight1[1];
				clamp(p, 0, 1);
				imageStore(mTDComputeOutputs[0], ivec2(xx+0, nline1), p);

				s = texelFetch(sTD2DInputs[0],   ivec2(xx+1, sline1), 0);
				p = s + diffColor * weight1[2];
				clamp(p, 0, 1);
				imageStore(mTDComputeOutputs[0], ivec2(xx+1, nline1), p);

				// (xx+0, y+2) weight2[1]
				// third line needs to be fresh from input???				
				s = texelFetch(sTD2DInputs[1],   ivec2(xx+0, y+2), 0);				
				p = s + diffColor * weight2[1];
				clamp(p, 0, 1);
				imageStore(mTDComputeOutputs[0], ivec2(xx+0, nline2), p);
			}
		}
		barrier();
		
		lineDist += arrayDist[bestF];

	} // for x

	// store lineDist for this bidx

	for (int i=0; i<3; i++)
	{
		imageStore(mTDComputeOutputs[0], ivec2(iwidth+0, nline0+i), vec4(lineDist, bidx, 0, 1));
		imageStore(mTDComputeOutputs[0], ivec2(iwidth+1, nline0+i), backColor);
		imageStore(mTDComputeOutputs[0], ivec2(iwidth+2, nline0+i), backColor);
		imageStore(mTDComputeOutputs[0], ivec2(iwidth+3, nline0+i), backColor);
	}
}
#endif

void
main()
{
	initPal();

	originalInput.readInput();

	tempInput.copy(originalInput);
	// setup first lines
	for (uint y=0; y < 128*3; y++)
	{
		for (uint x=0; x < originalInput.width; x++)
		{
			uint y1 = y%3;
			tempInput.get(x, y + originalInput.height) = originalInput.get(x, y1);
		}
	}

	// loop all
	for (uint pass=0; pass<originalInput.height; pass++)
	{
		fprintf(stderr, "Pass %d\n", pass);

		uTDPass = pass;

		for (uint work=0; work<128; work++)
		{
			gl_WorkGroupID = vec4(work, 0, 0, 0);
			
			// local has to be inlined
			mainGLSL();
		}

		tempInput.copy(finalOutput);
	}

	finalOutput.writeOutput();
}


