//--------------------------------------------------------------------------------------
// File: Tutorial022.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
Texture2D tex : register(t0);
SamplerState samLinear : register(s0);

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	// more buffer stuff?
	float4 lightposition;
	float4 camposition;
	matrix world;
	matrix view;
	matrix projection;
};

//struct float4
//	{
//	float r, g, b, a;//same
//	float x, y, z, w;
//	}
struct SimpleVertex
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
	float3 Norm : NORMAL;
};

struct SimpleVertexRGB
{
	float4 Pos : POSITION;
	float4 Col : COLOR;
	float3 Norm : NORMAL;
};

// more simple vertices?

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : POSITION1;
	float2 Tex : TEXCOORD0;
	float3 Norm : NORMAL;
};

struct PS_INPUT_RGB
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : POSITION1;
	float4 Col : COLOR;
	float3 Norm : NORMAL;
};

// more structures?

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

PS_INPUT VShader(SimpleVertex input)
{
	PS_INPUT output;
	float4 pos = input.Pos;

	pos = mul(world, pos);	
	output.WorldPos = pos.xyz;
	pos = mul(view, pos);
	pos = mul(projection, pos);	
	
	matrix w = world;
	w._14 = 0;
	w._24 = 0;
	w._34 = 0;

	float4 norm;
	norm.xyz = input.Norm;
	norm.w = 1;
	norm = mul(w, norm);
	norm.x *= -1;
	output.Norm = normalize(norm.xyz);

	output.Pos = pos;
	output.Tex = input.Tex;
	return output;
}

PS_INPUT_RGB VShaderRGB(SimpleVertexRGB input) {
	PS_INPUT_RGB output;
	float4 pos = input.Pos;

	pos = mul(world, pos);
	output.WorldPos = pos.xyz;
	pos = mul(view, pos);
	pos = mul(projection, pos);

	matrix w = world;
	w._14 = 0;
	w._24 = 0;
	w._34 = 0;

	float4 norm;
	norm.xyz = input.Norm;
	norm.w = 1;
	norm = mul(w, norm);
	norm.x *= -1;
	output.Norm = normalize(norm.xyz);

	output.Pos = pos;
	output.Col = input.Col;
	return output;
}

// more vertex shaders?

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

// more pixel shaders?


float4 PShader(PS_INPUT input) : SV_Target
{
	float4 texture_color = tex.Sample(samLinear, input.Tex);
	float3 lp = lightposition.xyz;
	float3 normal = normalize(input.Norm);
	float3 world_pos = input.WorldPos.xyz;

	//calculate diffuse light
	float3 light_direction = normalize(lp - world_pos);
	float diffuse_light = saturate(dot(normal, light_direction));

	//calculate specular light
	float3 reflected_light = 2 * dot(light_direction, normal) * normal - light_direction;
	float3 cam_direction = normalize(camposition.xyz - world_pos);
	float specular_light = pow(saturate(dot(cam_direction, reflected_light)), 20);

	//make ambient light
	float ambient_light = 0.2;

	float4 color = texture_color * (diffuse_light + ambient_light) + specular_light;
	color.a = 1;
	return color;
}

float4 PShaderSky(PS_INPUT input) : SV_Target
{
	float4 color = tex.Sample(samLinear, input.Tex);
	color.a = 1;
	return color;
}

float4 PShaderRGB(PS_INPUT_RGB input) : SV_Target
{
	float4 rgb_color = input.Col;
	float3 lp = lightposition.xyz;
	float3 normal = normalize(input.Norm);
	float3 world_pos = input.WorldPos.xyz;

	//calculate diffuse light
	float3 light_direction = normalize(lp - world_pos);
	float diffuse_light = saturate(dot(normal, light_direction));

	//calculate specular light
	float3 reflected_light = 2 * dot(light_direction, normal) * normal - light_direction;
	float3 cam_direction = normalize(camposition.xyz - world_pos);
	float specular_light = pow(saturate(dot(cam_direction, reflected_light)), 20);

	//make ambient light
	float ambient_light = 0.2;

	float4 color = rgb_color * (diffuse_light + ambient_light) + specular_light;
	color.a = 1;
	return color;
}