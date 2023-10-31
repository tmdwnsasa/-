#ifndef _FONT_FX_
#define _FONT_FX_

#include "params.fx"
#include "utils.fx"

struct VS_IN
{
	float3 pos	: POSITION;
	float2 uv : TEXCOORD;
};

struct VS_OUT
{
	float4 pos	: SV_POSITION;
	float3 PosW	: POSITION;
	float2 uv : TEXCOORD;
};

VS_OUT VS_Font(VS_IN input)
{
	VS_OUT output = (VS_OUT)0.0f;

	// Position
    output.pos = mul(float4(input.pos, 1.0f), g_matWVP);

	// uv
	output.uv = input.uv;

	return output;
}

float4 PS_Font(VS_OUT input) : SV_TARGET
{
	float4 color = g_tex_0.Sample(g_sam_0, input.uv);

	if (color.r <= 0.5f)
	{
		color.a = 0.f;
	}
	else
	{
		color.rgb = float3(0.9f, 0.9f, 0.9f);
		color.a = 0.8f;
	}

	return color;
}

#endif