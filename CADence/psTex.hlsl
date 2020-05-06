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
	float4 black = 0;
	float4 cyan = float4(0.0f, 1.0f, 1.0f, 1.0f);
	float4 magenta = float4(1.0f, 0.0f, 0.2f, 1.0f);

	// scale and flip the Y axis
	float2 texScaled = i.tex.xy - float2(1.0f, 1.0f);
	texScaled.x /= 2;
	texScaled.y /= -2;
	
	float4 leftTex;
	float4 rightTex;
	
	// sample tex1 at tex.xy - C
	leftTex = leftEyeTex.Sample(texSampler, texScaled.xy);

	texScaled += float2(0.03f, 0.03f);
	// sample tex2 at tex.xy - M
	rightTex = rightEyeTex.Sample(texSampler, texScaled);
	
	// Filter C to cyan
	float4 clampedC = clamp(leftTex, black, cyan);
	// Filter M to magenta
	float4 clampedM = clamp(rightTex, black, magenta);
	
	return saturate(clampedC + clampedM);
}
