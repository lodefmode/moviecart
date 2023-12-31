// Floyd-steinberg in which each cell (8 pixels) can be one of 2 colors,
// but second color (background) is common for entire horizontal line

// (R= Graph bits,  G= foreground index, B= background index) is input 1/3
// Palette is is input 2/3
// Background score is input 3/3  (for each x=background index, y=horizontal line)

// Output:
//  r = graph bits (1=use foreground, 0=use background)
//  g = foreground palette index
//  b = background palette index (identical for entire horizontal line)
// uniform float exampleUniform;

out vec4 fragColor;
void main()
{
	// vec4 color = texture(sTD2DInputs[0], vUV.st);

	float width = uTD2DInfos[0].res.z;
	float height = uTD2DInfos[0].res.w;
	uint x = uint(width * vUV.x);
	uint y = uint(height * vUV.y);

	vec4 origColor = texture(sTD2DInputs[0], vUV.st);
	
	uint findex = uint(origColor.g * 255.0);

	// bindex should be identical throughout the entire line, so just use at pixel 0 to confirm
//	uint bindex = uint(origColor.b * 255.0);
	vec4 col0 = texelFetch(sTD2DInputs[0], ivec2(0, y), 0);
	uint bindex = uint(col0.b * 255.0);
	
	vec4 foreColor = texelFetch(sTD2DInputs[1], ivec2(findex, 0), 0);
	vec4 backColor = texelFetch(sTD2DInputs[1], ivec2(bindex, 0), 0);

	vec4 outColor = mix(backColor, foreColor, origColor.r);
	fragColor = TDOutputSwizzle(outColor);

#if 0	// debug background color
	if (x<2)
		fragColor = backColor;
	if (x == 2)
		fragColor = vec4(bindex, bindex, bindex, 1);
#endif

}
