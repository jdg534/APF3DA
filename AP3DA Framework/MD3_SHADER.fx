//--------------------------------------------------------------------------------------
// this is partly based on a modified version of: DX11 Framework.fx
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

Texture2D terrainLightDirtTex : register(t1); // not needed as this shader is only ment to address MD3 models, kept for simplisity (texture indexing system to have math accross all shaders)
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

cbuffer ConstantBuffer : register( cb0 )
{
	matrix World;
	matrix View;
	matrix Projection;

	SurfaceInfo surface;
	Light light;

	float3 EyePosW;
	float HasTexture;
	
	matrix WorldInverseTranspose;
};

cbuffer BoneMatricesBuffer : register(cb1)
{
	float4x4 boneMats[96];
	// above is how the book implerments it

	// could have been matrix boneMats[96];

};

struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormL : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 Weights : WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
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

	// the following is based off a shader presented on page 663 of
	// Introduction to 3D Game Programming with DirectX 11

	// get array for the blend weights
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	weights[0] = input.Weights.x;
	weights[1] = input.Weights.y;
	weights[2] = input.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	// now the vertex blending
	float3 posLocalSpace = float3(0.0f, 0.0f, 0.0f);
	float3 normalLocalSpace = float3(0.0f, 0.0f, 0.0f);
	// no tangent vector
	for (int i = 0; i < 4; i++)
	{
		posLocalSpace += weights[i] * mul(float4(input.PosL.xyz, 1.0f),	boneMats[input.BoneIndices[i]]).xyz;
		normalLocalSpace += weights[i] * mul(input.NormL, (float3x3)boneMats[input.BoneIndices[i]]);
			
	}

	// now the world space transforms



	// float4 posW = mul(input.PosL, World);
	float4 posW = mul(float4(posLocalSpace, 1.0f), World);
	output.PosW = posW.xyz;

	
	float4x4 worldViewProjection = World * View * Projection;

		//output.PosH = mul(posW, View);
		// output.PosH = mul(output.PosH, Projection);

	output.PosH = mul(float4(posLocalSpace, 1.0f), worldViewProjection);

	output.Tex = input.Tex;

	// float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
	float3 normalW = mul(float4(normalLocalSpace, 0.0f), World).xyz;
	output.NormW = normalize(normalW);

	// the book's approach to transforming to world space
	// vout.NormalW = mul(normalL, (float3x3)gWorldInvTranspose);

	float3 nw = mul(normalLocalSpace, (float3x3)WorldInverseTranspose);
	nw = normalize(nw);
	output.NormW = nw;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(VS_OUTPUT input) : SV_Target
{
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


	float4 white = float4(1.0f, 1.0f, 1.0f, 1.0f);

	//return finalColour;
	return white;
}