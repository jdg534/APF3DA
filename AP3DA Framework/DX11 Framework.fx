//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);

SamplerState samLinear : register(s0);

/*

m_terrainLightDirtTex 0 - 51
m_terrainGrassTex 52 - 102
m_terrainDarkDirtTex 102 - 153
m_terrainStoneTex 154 - 205
m_terrainSnowTex 206+

// on terrain creation, values are scaled from 0 - 255 to 0.0 - 1.0
// then re-scale by value stored in terrainScaledBy


// initial test ranges (step up by (1.0f / 5.0f) = 0.2)
lightDirtCutOff 0.2
grassCutOff 0.4
darkDirtCutOff 0.6
stoneCutOff 0.8
snow for >= 0.8



*/

Texture2D terrainLightDirtTex : register(t1);
Texture2D terrainGrassTex : register(t2);
Texture2D terrainDarkDirtTex : register(t3);
Texture2D terrainStoneTex : register(t4);
Texture2D terrainSnowTex : register(t5);


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

struct SurfaceInfo
{
	float4 AmbientMtrl;
	float4 DiffuseMtrl;
	float4 SpecularMtrl;
};

struct Light
{
	float4 AmbientLight;
	float4 DiffuseLight;
	float4 SpecularLight;

	float SpecularPower;
	float3 LightVecW;
};

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;

	SurfaceInfo surface;
	Light light;

	float3 EyePosW;
	float HasTexture;
	float drawingTerrain;
	float terrainScaledBy;
}

struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormL : NORMAL;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;

	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

	float4 posW = mul(input.PosL, World);
	output.PosW = posW.xyz;

	output.PosH = mul(posW, View);
	output.PosH = mul(output.PosH, Projection);
	output.Tex = input.Tex;

	float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
	output.NormW = normalize(normalW);

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader(s) // sort of
//--------------------------------------------------------------------------------------

float4 drawTerrain(VS_OUTPUT input)
{
	// just gouraud shading for terrain, specular components, overkill? ask in next tutorial
	float4 textureColourValue = float4(1.0, 1.0f, 1.0f, 1.0f);

	float heightForVertex = input.PosW.y;

	/* value 0 - 255 * terrainScaledBy */ // see the ranges at top of this file
	// 0 - 255 wrong, 0.0 - 1.0 right


	float lightDirtCutOff = 0.2f;
	float grassCutOff = 0.4f;
	float darkDirtCutOff = 0.6f;
	float stoneCutOff = 0.8f;
	// snow for any thing above stoneCutOff


	// following works, need to include some blending

	if (heightForVertex < lightDirtCutOff * terrainScaledBy)
	{
		textureColourValue = terrainLightDirtTex.Sample(samLinear, input.Tex);
	}
	else if (heightForVertex < grassCutOff * terrainScaledBy)
	{
		textureColourValue = terrainGrassTex.Sample(samLinear, input.Tex);
	}
	else if (heightForVertex < darkDirtCutOff * terrainScaledBy)
	{
		textureColourValue = terrainDarkDirtTex.Sample(samLinear, input.Tex);
	}
	else if (heightForVertex < stoneCutOff * terrainScaledBy)
	{
		textureColourValue = terrainStoneTex.Sample(samLinear, input.Tex);
	}
	else
	{
		// just use the snow (for mountain tops)
		textureColourValue = terrainSnowTex.Sample(samLinear, input.Tex);
	}
	// now got the colour for a texture


	// following mostly based on the provided Pixel Shader
	float3 normalW = normalize(input.NormW);

	float3 toEye = normalize(EyePosW - input.PosW);

	float3 ambient = float3(0.0f, 0.0f, 0.0f);
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);

	float3 lightLecNorm = normalize(light.LightVecW);
	// Compute Colour

	// Compute the reflection vector.
	float3 r = reflect(-lightLecNorm, normalW);

	// Determine how much specular light makes it into the eye.
	float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);

	// Determine the diffuse light intensity that strikes the vertex.
	float diffuseAmount = max(dot(lightLecNorm, normalW), 0.0f);

	// Only display specular when there is diffuse
	if (diffuseAmount <= 0.0f)
	{
		specularAmount = 0.0f;
	}

	// Compute the ambient, diffuse, and specular terms separately.
	specular += specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;
	diffuse += diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb;
	ambient += (surface.AmbientMtrl * light.AmbientLight).rgb;

	// Sum all the terms together and copy over the diffuse alpha.
	float4 finalColour;

	finalColour.rgb = (textureColourValue.rgb * (ambient + diffuse)) + specular;

	finalColour.a = surface.DiffuseMtrl.a;

	return finalColour;

	// return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	if (drawingTerrain == 1.0f)
	{
		return drawTerrain(input);
	}

	float3 normalW = normalize(input.NormW);

	float3 toEye = normalize(EyePosW - input.PosW);

	// Get texture data from file
	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);

	float3 ambient = float3(0.0f, 0.0f, 0.0f);
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);

	float3 lightLecNorm = normalize(light.LightVecW);
	// Compute Colour

	// Compute the reflection vector.
	float3 r = reflect(-lightLecNorm, normalW);

	// Determine how much specular light makes it into the eye.
	float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);

	// Determine the diffuse light intensity that strikes the vertex.
	float diffuseAmount = max(dot(lightLecNorm, normalW), 0.0f);

	// Only display specular when there is diffuse
	if (diffuseAmount <= 0.0f)
	{
		specularAmount = 0.0f;
	}

	// Compute the ambient, diffuse, and specular terms separately.
	specular += specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;
	diffuse += diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb;
	ambient += (surface.AmbientMtrl * light.AmbientLight).rgb;

	// Sum all the terms together and copy over the diffuse alpha.
	float4 finalColour;

	if (HasTexture == 1.0f)
	{
		finalColour.rgb = (textureColour.rgb * (ambient + diffuse)) + specular;
	}
	else
	{
		finalColour.rgb = ambient + diffuse + specular;
	}

	finalColour.a = surface.DiffuseMtrl.a;

	return finalColour;
}