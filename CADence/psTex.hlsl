Texture2D leftEyeTex: register(t0);
Texture2D rightEyeTex: register(t1);
SamplerState texSampler: register(s0);

struct PSIn
{
	float4 pos : SV_POSITION;
	float2 tex: TEXCOORDS0;
};

float4 main(PSIn i) : SV_TARGET
{

	float2 texScaled = i.tex.xy - float2(1.0f, 1.0f);
	texScaled.x /= 2;
	texScaled.y /= -2;
	float3 black = float3(0.0f, 0.0f, 0.0f);
	float3 cyan = float3(0.0f, 1.0f, 1.0f);
	float3 magenta = float3(1.0f, 0.0f, 1.0f);
	float3 cyanTex;
	float3 magentaTex;
	// sample tex1 at tex.xy - C
	float4 sample1 = leftEyeTex.Sample(texSampler, texScaled.xy);
	
	// sample tex2 at tex.xy - M
	magentaTex = rightEyeTex.Sample(texSampler, texScaled);

	float green = 0.0f;
	if (sample1.x < 2)
		green = 1.0f;

	// Filter C to cyan
	float3 clampedC = clamp(cyanTex, black, cyan);
	// Filter M to magenta
	float3 clampedM = clamp(magentaTex, black, magenta);
	// float4(1.0f, 0.0f, 0.0f, 1.0f); //
	float red = 0.0f;
	if (texScaled.x > 0 && texScaled.x < 1)
		red = texScaled.x;

	float blue = 0.0f;
	if (texScaled.y > 0 && texScaled.y < 1)
		blue = texScaled.y;
	//float4(red, 0.0f, blue, 1.0f);
	return sample1;
}
