// Bi-cubic filtering using 3D hardware

// float4x4 worldViewProj : WorldViewProjection;

// you need to change these if you load a different texture:
const float2 imageSize = { 512.0, 256.0 };
const float2 intermediateSize = { 512.0*4, 256.0 };
const float4 scanIntensity = { 0.5, 0.5, 0.5, 0.0 };
const float  scanSize = 1.0;

#ifndef BC_MACROS
 // Use variables to allow real-time adjustments
 const float B = 1.0 / 3.0;
 const float C = 1.0 / 3.0;
#else
 // Use macros for speed
 #ifndef B
  #define B (1.0/3.0)
 #endif
 #ifndef C
  #define C ((1.0 - B) / 2.0)
 #endif
#endif

texture imageTexture;
texture intermediateTexture;
texture scanTexture;
texture weightTex;

sampler imageSampler = sampler_state 
{
    texture = <imageTexture>;
    MagFilter = Point;
    MinFilter = Point;
    MipFilter = None;
    AddressU = Border;
    AddressV = Border;
};

sampler intermediateSampler = sampler_state 
{
    texture = <intermediateTexture>;
    MagFilter = Point;
    MinFilter = Point;
    MipFilter = None;
    AddressU = Border;
    AddressV = Border;
};

sampler scanImageSampler = sampler_state 
{
    texture = <scanTexture>;
    MagFilter = Linear;
    MinFilter = Linear;
    MipFilter = None;
    AddressU = Wrap;
    AddressV = Wrap;
};

sampler weightSampler20 = sampler_state 
{
    texture = <weightTex>;
    MagFilter = Linear;
    MinFilter = Linear;
    MipFilter = None;
    AddressU = Clamp;
    AddressV = Clamp;
};

sampler weightSampler14 = sampler_state 
{
    texture = <weightTex>;
    MagFilter = Point;
    MinFilter = Point;
    MipFilter = None;
    AddressU = Wrap;
    AddressV = Clamp;
};

// vertex shader
struct a2v {
    float4 pos		: POSITION;
    float4 texcoord	: TEXCOORD0;
};
struct v2f {
    float4 pos		: POSITION;
    float4 texcoord	: TEXCOORD0;
};

v2f defaultVS(a2v IN)
{
	v2f OUT;
	OUT.pos = IN.pos;//mul(IN.pos, worldViewProj);
	OUT.texcoord = IN.texcoord;
    return OUT;
}

v2f bicubicVS(a2v IN)
{
	v2f OUT;
	OUT.pos = IN.pos;//mul(IN.pos, worldViewProj);
	OUT.texcoord = IN.texcoord - 0.5 / imageSize.xyxy;
    return OUT;
}

struct a2v2 {
    float4 pos			: POSITION;
    float4 texcoord0	: TEXCOORD0;
    float4 texcoord1	: TEXCOORD1;
};
struct v2f2 {
    float4 pos			: POSITION;
    float4 texcoord0	: TEXCOORD0;
	float4 texcoord1	: TEXCOORD1;
};
struct v2f5 {
    float4 pos			: POSITION;
    float4 texcoord0	: TEXCOORD0;
	float4 texcoord1	: TEXCOORD1;
	float4 texcoord2	: TEXCOORD2;
	float4 texcoord3	: TEXCOORD3;
	float4 texcoord4	: TEXCOORD4;
};

v2f5 bicubicP0VS(a2v2 IN)
{
	v2f5 OUT;
    float4 offsets = float4(-1.0f, 0.0f, 1.0f, 2.0f) / imageSize.xxxx;

	OUT.pos = IN.pos;
	OUT.texcoord0 = IN.texcoord0 - 0.5 / imageSize.xyxy;
	OUT.texcoord1 = IN.texcoord1;

	OUT.texcoord2 = OUT.texcoord0;
	OUT.texcoord3 = OUT.texcoord0;
	OUT.texcoord4 = OUT.texcoord0;
	
	OUT.texcoord2.x += offsets.x;
	OUT.texcoord3.x += offsets.z;
	OUT.texcoord4.x += offsets.w;

    return OUT;
}

v2f5 bicubicP1VS(a2v2 IN)
{
	v2f5 OUT;
    float4 offsets = float4(-1.0f, 0.0f, 1.0f, 2.0f) / imageSize.yyyy;

	OUT.pos = IN.pos;//mul(IN.pos, worldViewProj);
	OUT.texcoord0 = IN.texcoord0 + 0.5 / intermediateSize.xyxy;
	OUT.texcoord1 = IN.texcoord1;

	OUT.texcoord2 = OUT.texcoord0;
	OUT.texcoord3 = OUT.texcoord0;
	OUT.texcoord4 = OUT.texcoord0;
	
	OUT.texcoord2.y += offsets.x;
	OUT.texcoord3.y += offsets.z;
	OUT.texcoord4.y += offsets.w;

    return OUT;
}

v2f2 bicubicP1ScanVS(a2v2 IN)
{
	v2f2 OUT;

	OUT.pos = IN.pos;//mul(IN.pos, worldViewProj);
	OUT.texcoord0 = IN.texcoord0;
	OUT.texcoord1 = IN.texcoord1;

    return OUT;
}

// Compute 4 weights using a Mitchell-Netravali cubic polynomial:
// w = (2 - B*3/2 - C) * x^3 + (-3 + B*2 + C  ) * x^2                    + (1 - B/3        ) for 0 < x < 1
// w = (   -B/6   - C) * x^3 + (     B   + C*5) * x^2 + (-B*2 - C*8) * x + (    B*4/3 + C*4) for 1 < x < 2
// B = blurring, C = ringing, 0 <= B <= 1, B + 2*C = 1, using B = 0, C = 0.75 yields nVidia's sample
float4 computeWeights(float x)
{
	float4 w;

	// First calculate (x+1, x, 1-x, 2-x)
 	float4 x1 = x  * float4(1, 1, -1, -1) + float4(1, 0, 1, 2);
	float4 x2 = x1 * x1;
	float4 x3 = x2 * x1;
	
	// Calculate the polynomial
	w  = x3 * float2(-B/6.0     - C,      2.0 - B*3.0/2.0 - C).xyyx;
	w += x2 * float2( B         + C*5.0, -3.0 + B*2.0     + C).xyyx;
	w += x1 * float2(-B*2.0     - C*8.0,  0                  ).xyyx;
	w +=      float2( B*4.0/3.0 + C*4.0,  1.0 - B/3.0        ).xyyx;

	return w;
}

// Generate the LUT texture
float4 genWeightTex20(float2 p : POSITION) : COLOR
{
	return computeWeights(p.x);
}
float4 genWeightTex14(float2 p : POSITION) : COLOR
{
	return abs(computeWeights(p.x));
}

// filter 4 values
float4 cubicFilter(float4 w, float4 c0, float4 c1, float4 c2, float4 c3)
{
	return c0*w[0] + c1*w[1] + c2*w[2] + c3*w[3];
}
float4 cubicFilterh(half4 w, half4 c0, half4 c1, half4 c2, half4 c3)
{
	return c0*w[0] + c1*w[1] + c2*w[2] + c3*w[3];
}

// pixel shaders

// Single-pass, use full precision
float4 tex2D_bicubicSPHQ(sampler2D tex, float2 t)
{
	float2 f = frac(t*imageSize);

	// filter in x
	float4 w = computeWeights(f.x);
	float4 t0 = cubicFilter(w,	tex2D(tex, t+float2(-1, -1)/imageSize),
							  	tex2D(tex, t+float2( 0, -1)/imageSize),
							  	tex2D(tex, t+float2( 1, -1)/imageSize),
							  	tex2D(tex, t+float2( 2, -1)/imageSize) );
	float4 t1 = cubicFilter(w,	tex2D(tex, t+float2(-1,  0)/imageSize),
								tex2D(tex, t+float2( 0,  0)/imageSize),
								tex2D(tex, t+float2( 1,  0)/imageSize),
								tex2D(tex, t+float2( 2,  0)/imageSize) );
	float4 t2 = cubicFilter(w,	tex2D(tex, t+float2(-1,  1)/imageSize),
								tex2D(tex, t+float2( 0,  1)/imageSize),
								tex2D(tex, t+float2( 1,  1)/imageSize),
								tex2D(tex, t+float2( 2,  1)/imageSize) );
	float4 t3 = cubicFilter(w,	tex2D(tex, t+float2(-1,  2)/imageSize),
								tex2D(tex, t+float2( 0,  2)/imageSize),
								tex2D(tex, t+float2( 1,  2)/imageSize),
								tex2D(tex, t+float2( 2,  2)/imageSize) );

	// filter in y								
	w = computeWeights(f.y);
	return cubicFilter(w, t0, t1, t2, t3);
}

// Single-pass, use partial precision where possible
float4 tex2D_bicubicSP(sampler2D tex, sampler1D weightTex, float2 t)
{
	float2 f = frac(t*imageSize);

	// filter in x
	float4 w = tex1D(weightTex, f.x);
	half4 t0 = cubicFilterh(w,	tex2D(tex, t+float2(-1, -1)/imageSize),
							  	tex2D(tex, t+float2( 0, -1)/imageSize),
							  	tex2D(tex, t+float2( 1, -1)/imageSize),
							  	tex2D(tex, t+float2( 2, -1)/imageSize) );
	half4 t1 = cubicFilterh(w,	tex2D(tex, t+float2(-1,  0)/imageSize),
								tex2D(tex, t+float2( 0,  0)/imageSize),
								tex2D(tex, t+float2( 1,  0)/imageSize),
								tex2D(tex, t+float2( 2,  0)/imageSize) );
	half4 t2 = cubicFilterh(w,	tex2D(tex, t+float2(-1,  1)/imageSize),
								tex2D(tex, t+float2( 0,  1)/imageSize),
								tex2D(tex, t+float2( 1,  1)/imageSize),
								tex2D(tex, t+float2( 2,  1)/imageSize) );
	half4 t3 = cubicFilterh(w,	tex2D(tex, t+float2(-1,  2)/imageSize),
								tex2D(tex, t+float2( 0,  2)/imageSize),
								tex2D(tex, t+float2( 1,  2)/imageSize),
								tex2D(tex, t+float2( 2,  2)/imageSize) );

	// filter in y									
	w = tex1D(weightTex, f.y);
	return cubicFilterh(w, t0, t1, t2, t3);
}

// Multi-pass, use full precision
float4 tex2D_bicubicP0HQ(sampler2D tex, float2 t)
{
	float2 f = frac(t*imageSize);

	// filter in x
	float4 w = computeWeights(f.x);
	return cubicFilter(w,	tex2D(tex, t+float2(-1, 0)/imageSize),
							tex2D(tex, t+float2( 0, 0)/imageSize),
							tex2D(tex, t+float2( 1, 0)/imageSize),
							tex2D(tex, t+float2( 2, 0)/imageSize) );

}

float4 tex2D_bicubicP1HQ(sampler2D tex, float2 t)
{
	float2 f = frac(t*intermediateSize);
	
	// filter in y
	float4 w = computeWeights(f.y);
	return cubicFilter(w,	tex2D(tex, t+float2(0, -1)/intermediateSize),
							tex2D(tex, t+float2(0,  0)/intermediateSize),
							tex2D(tex, t+float2(0,  1)/intermediateSize),
							tex2D(tex, t+float2(0,  2)/intermediateSize) );

}

// Multi-pass, PS2.0, use partial precision where possible
float4 tex2D_bicubicP020(sampler2D tex, sampler1D weightTex, float2 t, float2 t0, float2 t1, float2 t2, float2 t3)
{
	float2 f = frac(t*imageSize);

	// filter in x
	float4 w = tex1D(weightTex, f.x);
	return cubicFilterh(w,	tex2D(tex, t0),
							tex2D(tex, t1),
							tex2D(tex, t2),
							tex2D(tex, t3) );
}

float4 tex2D_bicubicP120(sampler2D tex, sampler1D weightTex, float2 t, float2 t0, float2 t1, float2 t2, float2 t3)
{
	float2 f = frac(t*intermediateSize);

	// filter in y
	float4 w = tex1D(weightTex, f.y);
	return cubicFilterh(w,	tex2D(tex, t0),
							tex2D(tex, t1),
							tex2D(tex, t2),
							tex2D(tex, t3) );
}

// Multi-pass, compatible with PS1.4, use partial precision where possible
float4 tex2D_bicubicP014(sampler2D tex, sampler1D weightTex, float2 t, float2 t0, float2 t1, float2 t2, float2 t3)
{
	// filter in x
	float4 w = tex1D(weightTex, t.x);
	return cubicFilterh(w, -tex2D(tex, t0),
							tex2D(tex, t1),
							tex2D(tex, t2),
						   -tex2D(tex, t3) );
}

float4 tex2D_bicubicP114(sampler2D tex, sampler1D weightTex, float2 t, float2 t0, float2 t1, float2 t2, float2 t3)
{
	// filter in y
	float4 w = tex1D(weightTex, t.y);
	return cubicFilterh(w, -tex2D(tex, t0),
							tex2D(tex, t1),
							tex2D(tex, t2),
						   -tex2D(tex, t3) );
}

float4 bicubicSPHQPS(v2f IN) : COLOR
{
	return tex2D_bicubicSPHQ(imageSampler, IN.texcoord);
}

float4 bicubicSPPS(v2f IN) : COLOR
{
	return tex2D_bicubicSP(imageSampler, weightSampler20, IN.texcoord);
}

float4 bicubicP0HQPS(v2f IN) : COLOR
{
	return tex2D_bicubicP0HQ(imageSampler,        IN.texcoord);
}
float4 bicubicP1HQPS(v2f IN) : COLOR
{
	return tex2D_bicubicP1HQ(intermediateSampler, IN.texcoord);
}

float4 bicubicP0PS20(v2f5 IN) : COLOR
{
	return tex2D_bicubicP020(imageSampler,        weightSampler20, IN.texcoord0, IN.texcoord2, IN.texcoord0, IN.texcoord3, IN.texcoord4);
}
float4 bicubicP1PS20(v2f5 IN) : COLOR
{
	return tex2D_bicubicP120(intermediateSampler, weightSampler20, IN.texcoord0, IN.texcoord2, IN.texcoord0, IN.texcoord3, IN.texcoord4);
}

float4 bicubicP0PS14(v2f5 IN) : COLOR
{
	return tex2D_bicubicP014(imageSampler,        weightSampler14, IN.texcoord1, IN.texcoord2, IN.texcoord0, IN.texcoord3, IN.texcoord4);
}
float4 bicubicP1PS14(v2f5 IN) : COLOR
{
	return tex2D_bicubicP114(intermediateSampler, weightSampler14, IN.texcoord1, IN.texcoord2, IN.texcoord0, IN.texcoord3, IN.texcoord4);
}

// use this for arbitrarily sized scanlines
float4 bicubicP1ScanPS(v2f2 IN) : COLOR
{
	float4 col1 = tex2D(intermediateSampler, IN.texcoord0);
	float4 col2 = tex2D(scanImageSampler, frac(IN.texcoord1) * scanSize);

	return col1 * (scanIntensity + col2 - scanIntensity * col2);
}

technique SinglePassHQBicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=geometry;";
    > {
		VertexShader = compile vs_1_1 bicubicVS();
		PixelShader = compile ps_2_0 bicubicSPHQPS();
    }
}

technique SinglePassBicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=geometry;";
    > {
		VertexShader = compile vs_1_1 bicubicVS();
		PixelShader = compile ps_2_0 bicubicSPPS();
    }
}

technique MultiPassHQBicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=texture;";
    > {
		VertexShader = compile vs_1_1 bicubicP0VS();
		PixelShader = compile ps_2_0 bicubicP0HQPS();
    }
    pass p1 <
		string Script = "Draw=geometry;";
    > {
		VertexShader = compile vs_1_1 bicubicP1VS();
		PixelShader = compile ps_2_0 bicubicP1HQPS();
    }
}

technique MultiPassBicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=texture;";
    > {
		VertexShader = compile vs_1_1 bicubicP0VS();
		PixelShader = compile ps_2_0 bicubicP0PS20();
    }
    pass p1 <
		string Script = "Draw=geometry;";
    > {
		VertexShader = compile vs_1_1 bicubicP1VS();
		PixelShader = compile ps_2_0 bicubicP1PS20();
    }
}

technique MultiPassHP20Bicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=texture;";
    > {
		VertexShader = compile vs_1_1 bicubicP0VS();
		PixelShader = compile ps_2_0 bicubicP0PS14();
    }
    pass p1 <
		string Script = "Draw=geometry;";
    > {
		VertexShader = compile vs_1_1 bicubicP1VS();
		PixelShader = compile ps_2_0 bicubicP1PS14();
    }
}

technique MultiPassHP14Bicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=texture;";
    > {
		VertexShader = compile vs_1_1 bicubicP0VS();
		PixelShader = compile ps_1_4 bicubicP0PS14();
    }
    pass p1 <
		string Script = "Draw=geometry;";
    > {
		VertexShader = compile vs_1_1 bicubicP1VS();
		PixelShader = compile ps_1_4 bicubicP1PS14();
    }
}

technique Bilinear <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=geometry;";
    > {
		Texture[0]       = <imageTexture>;
		TexCoordIndex[0] = 0;
		AddressU[0]		 = WRAP;
		AddressV[0]		 = WRAP;
		MinFilter[0]     = LINEAR;
		MagFilter[0]     = LINEAR;
		ColorArg1[0]     = TEXTURE;
		ColorOP[0]       = SELECTARG1;
		AlphaOp[0]       = DISABLE;

		ColorOP[1]       = DISABLE;
		
		VertexShader = NULL;
		PixelShader = NULL;
	}
}

technique Point <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=geometry;";
    > {
		Texture[0]       = <imageTexture>;
		TexCoordIndex[0] = 0;
		AddressU[0]		 = CLAMP;
		AddressV[0]		 = CLAMP;
		MinFilter[0]     = POINT;
		MagFilter[0]     = POINT;
		ColorArg1[0]     = TEXTURE;
		ColorOP[0]       = SELECTARG1;

		AlphaOp[0]       = DISABLE;
		ColorOP[1]       = DISABLE;
		
		VertexShader = NULL;
		PixelShader = NULL;
	}
}

technique ScanHQBicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=texture;";
    > {
		VertexShader = compile vs_1_1 bicubicP0VS();
		PixelShader = compile ps_2_0 bicubicP0HQPS();
    }
    pass p1 <
		string Script = "Draw=geometry;";
    > {
#if 0
		VertexShader = compile vs_1_1 bicubicP1ScanVS();
		PixelShader = compile ps_2_0 bicubicP1ScanPS();
#else
		// Stage 0 has the scanlines
		Texture[0]       = <scanTexture>;
		TexCoordIndex[0] = 1;
		AddressU[0]		 = WRAP;
		AddressV[0]		 = WRAP;
		MinFilter[0]     = LINEAR;
		MagFilter[0]     = LINEAR;
    	ColorArg1[0]     = TEXTURE;
    	ColorArg2[0]     = TFACTOR;
		ColorOP[0]       = ADDSMOOTH;

		AlphaOp[0]       = DISABLE;

		// Stage 1 has the image
		Texture[1]       = <intermediateTexture>;
		TexCoordIndex[1] = 0;
		AddressU[1]		 = BORDER;
		AddressV[1]		 = BORDER;
		MinFilter[1]     = LINEAR;
		MagFilter[1]     = POINT;
    	ColorArg1[1]     = TEXTURE;
    	ColorArg2[1]     = CURRENT;
		ColorOP[1]       = MODULATE;

		ColorOP[2]       = DISABLE;

		VertexShader = NULL;
		PixelShader = NULL;
#endif
    }
}

technique ScanBicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=texture;";
    > {
		VertexShader = compile vs_1_1 bicubicP0VS();
		PixelShader = compile ps_2_0 bicubicP0PS20();
    }
    pass p1 <
		string Script = "Draw=geometry;";
    > {
		// Stage 0 has the scanlines
		Texture[0]       = <scanTexture>;
		TexCoordIndex[0] = 1;
		AddressU[0]		 = WRAP;
		AddressV[0]		 = WRAP;
		MinFilter[0]     = LINEAR;
		MagFilter[0]     = LINEAR;
    	ColorArg1[0]     = TEXTURE;
    	ColorArg2[0]     = TFACTOR;
		ColorOP[0]       = ADDSMOOTH;

		AlphaOp[0]       = DISABLE;

		// Stage 1 has the image
		Texture[1]       = <intermediateTexture>;
		TexCoordIndex[1] = 0;
		AddressU[1]		 = BORDER;
		AddressV[1]		 = BORDER;
		MinFilter[1]     = LINEAR;
		MagFilter[1]     = POINT;
    	ColorArg1[1]     = TEXTURE;
    	ColorArg2[1]     = CURRENT;
		ColorOP[1]       = MODULATE;

		ColorOP[2]       = DISABLE;

		VertexShader = NULL;
		PixelShader = NULL;
    }
}

technique ScanHP20Bicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=texture;";
    > {
		VertexShader = compile vs_1_1 bicubicP0VS();
		PixelShader = compile ps_2_0 bicubicP0PS14();
    }
    pass p1 <
		string Script = "Draw=geometry;";
    > {
		// Stage 0 has the scanlines
		Texture[0]       = <scanTexture>;
		TexCoordIndex[0] = 1;
		AddressU[0]		 = WRAP;
		AddressV[0]		 = WRAP;
		MinFilter[0]     = LINEAR;
		MagFilter[0]     = LINEAR;
    	ColorArg1[0]     = TEXTURE;
    	ColorArg2[0]     = TFACTOR;
		ColorOP[0]       = ADDSMOOTH;

		AlphaOp[0]       = DISABLE;

		// Stage 1 has the image
		Texture[1]       = <intermediateTexture>;
		TexCoordIndex[1] = 0;
		AddressU[1]		 = BORDER;
		AddressV[1]		 = BORDER;
		MinFilter[1]     = LINEAR;
		MagFilter[1]     = POINT;
    	ColorArg1[1]     = TEXTURE;
    	ColorArg2[1]     = CURRENT;
		ColorOP[1]       = MODULATE;

		ColorOP[2]       = DISABLE;

		VertexShader = NULL;
		PixelShader = NULL;
    }
}

technique ScanHP14Bicubic <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=texture;";
    > {
		VertexShader = compile vs_1_1 bicubicP0VS();
		PixelShader = compile ps_1_4 bicubicP0PS14();
    }
    pass p1 <
		string Script = "Draw=geometry;";
    > {
		// Stage 0 has the scanlines
		Texture[0]       = <scanTexture>;
		TexCoordIndex[0] = 1;
		AddressU[0]		 = WRAP;
		AddressV[0]		 = WRAP;
		MinFilter[0]     = LINEAR;
		MagFilter[0]     = LINEAR;
    	ColorArg1[0]     = TEXTURE;
    	ColorArg2[0]     = TFACTOR;
		ColorOP[0]       = ADDSMOOTH;

		AlphaOp[0]       = DISABLE;

		// Stage 1 has the image
		Texture[1]       = <intermediateTexture>;
		TexCoordIndex[1] = 0;
		AddressU[1]		 = BORDER;
		AddressV[1]		 = BORDER;
		MinFilter[1]     = LINEAR;
		MagFilter[1]     = POINT;
    	ColorArg1[1]     = TEXTURE;
    	ColorArg2[1]     = CURRENT;
		ColorOP[1]       = MODULATE;

		ColorOP[2]       = DISABLE;

		VertexShader = NULL;
		PixelShader = NULL;
    }
}

technique ScanBilinear <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=texture;";
    > {
		Texture[0]       = <imageTexture>;
		TexCoordIndex[0] = 0;
		AddressU[0]		 = BORDER;
		AddressV[0]		 = BORDER;
		MinFilter[0]     = LINEAR;
		MagFilter[0]     = LINEAR;
		ColorArg1[0]     = TEXTURE;
		ColorOP[0]       = SELECTARG1;
		AlphaOp[0]       = DISABLE;

		ColorOP[1]       = DISABLE;
		
		VertexShader = NULL;
		PixelShader = NULL;
	}
    pass p1 <
		string Script = "Draw=geometry;";
    > {
		// Stage 0 has the scanlines
		Texture[0]       = <scanTexture>;
		TexCoordIndex[0] = 1;
		AddressU[0]		 = WRAP;
		AddressV[0]		 = WRAP;
		MinFilter[0]     = LINEAR;
		MagFilter[0]     = POINT;
    	ColorArg1[0]     = TEXTURE;
    	ColorArg2[0]     = TFACTOR;
		ColorOP[0]       = ADDSMOOTH;

		AlphaOp[0]       = DISABLE;

		// Stage 1 has the image
		Texture[1]       = <intermediateTexture>;
		TexCoordIndex[1] = 0;
		AddressU[1]		 = BORDER;
		AddressV[1]		 = BORDER;
		MinFilter[1]     = LINEAR;
		MagFilter[1]     = POINT;
    	ColorArg1[1]     = TEXTURE;
    	ColorArg2[1]     = CURRENT;
		ColorOP[1]       = MODULATE;

		ColorOP[2]       = DISABLE;

		VertexShader = NULL;
		PixelShader = NULL;
	}
}

technique ScanPoint <
	string Script = "Pass=p0;";
> {
    pass p0 <
		string Script = "Draw=geometry;";
    > {
		// Stage 0 has the scanlines
		Texture[0]       = <scanTexture>;
		TexCoordIndex[0] = 1;
		AddressU[0]		 = WRAP;
		AddressV[0]		 = WRAP;
		MinFilter[0]     = LINEAR;
		MagFilter[0]     = LINEAR;
    	ColorArg1[0]     = TEXTURE;
    	ColorArg2[0]     = TFACTOR;
		ColorOP[0]       = ADDSMOOTH;

		AlphaOp[0]       = DISABLE;

		// Stage 1 has the image
		Texture[1]       = <imageTexture>;
		TexCoordIndex[1] = 0;
		AddressU[1]		 = BORDER;
		AddressV[1]		 = BORDER;
		MinFilter[1]     = LINEAR;
		MagFilter[1]     = POINT;
    	ColorArg1[1]     = TEXTURE;
    	ColorArg2[1]     = CURRENT;
		ColorOP[1]       = MODULATE;

		ColorOP[2]       = DISABLE;

		VertexShader = NULL;
		PixelShader = NULL;
	}
}

