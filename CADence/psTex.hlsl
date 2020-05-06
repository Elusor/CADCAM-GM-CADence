SamplerState texSampler: register(s0);
Texture2D leftEyeTex: register(t0);
Texture2D rightEyeTex: register(t1);

struct PSIn
{
	float4 pos : SV_POSITION;
	float2 tex: TEXCOORDS0;
};

float4 main(PSIn i) : SV_TARGET
{

	float2 texScaled = i.tex.xy + float2(1.0f, 1.0f);
	texScaled /= 2;

	float3 black = float3(0.0f, 0.0f, 0.0f);
	float3 cyan = float3(0.0f, 1.0f, 1.0f);
	float3 magenta = float3(1.0f, 0.0f, 1.0f);
	float3 cyanTex;
	float3 magentaTex;
	// sample tex1 at tex.xy - C
	cyanTex = leftEyeTex.Sample(texSampler, texScaled);
	// sample tex2 at tex.xy - M
	magentaTex = rightEyeTex.Sample(texSampler, texScaled);

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
	return float4(cyanTex,1.0f);
}
