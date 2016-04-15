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

cbuffer ConstantBuffer : register( cb0 )
{
	matrix World;
	matrix View;
	matrix Projection;

	SurfaceInfo surface;
	Light light;

	float3 EyePosW;
	float HasTexture;
	float drawingMode;
	float terrainScaledBy;
	matrix WorldInverseTranspose;
};

cbuffer BoneMatrixBuffer : register (cb1)
{
	float4x4 boneMatrices[96];
};

struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormL : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 weights : WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;

	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	int errorColour : TEXCOORD1;
};

bool matricesTheSame(float4x4 a, float4x4 b) // this was used for determeing if the matrices were working correctly
{
	if (a._m00 != b._m00)
	{
		return false;
	}
	else if (a._m01 != b._m01)
	{
		return false;
	}
	else if (a._m02 != b._m02)
	{
		return false;
	}
	else if (a._m03 != b._m03)
	{
		return false;
	}

	else if (a._m10 != b._m10)
	{
		return false;
	}
	else if (a._m11 != b._m11)
	{
		return false;
	}
	else if (a._m12 != b._m12)
	{
		return false;
	}
	else if (a._m13 != b._m13)
	{
		return false;
	}

	else if (a._m20 != b._m20)
	{
		return false;
	}
	else if (a._m21 != b._m21)
	{
		return false;
	}
	else if (a._m22 != b._m22)
	{
		return false;
	}
	else if (a._m23 != b._m23)
	{
		return false;
	}

	else if (a._m30 != b._m30)
	{
		return false;
	}
	else if (a._m31 != b._m31)
	{
		return false;
	}
	else if (a._m32 != b._m32)
	{
		return false;
	}
	else if (a._m33 != b._m33)
	{
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

	output.errorColour = 0;

	//if (false)
	if (drawingMode == 2.0f) // its an M3D mesh
	{
		// logic from MD3_SHADER.fx here
		// get array for the blend weights
		float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		weights[0] = input.weights.x;
		weights[1] = input.weights.y;
		weights[2] = input.weights.z;
		weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

		float3 posLocalSpace = float3(0.0f, 0.0f, 0.0f);
		float3 normalLocalSpace = float3(0.0f, 0.0f, 0.0f);

		int iter = 0;

		for (iter = 0; iter < 4; iter++)
		{
			posLocalSpace += weights[iter] * mul(float4(input.PosL.xyz, 1.0f), boneMatrices[input.BoneIndices[iter]]).xyz;
			normalLocalSpace += weights[iter] * mul(input.NormL, (float3x3)boneMatrices[input.BoneIndices[iter]]);
		}
		
		float sumWeights = 0.0f;
		for (iter = 0; iter < 4; iter++)
		{
			sumWeights += weights[iter];
		}

		if (sumWeights == 0.0f)
		{
			output.errorColour = 1;
		}

		// check the bone index
		int maxBoneIndex = 58; // the original test model
		for (iter = 0; iter < 4; iter++)
		{
			if ((input.BoneIndices[iter] >= maxBoneIndex
				|| input.BoneIndices[iter] < 0)
				&& output.errorColour == 0)
			{

				output.errorColour = 3;
			}
		}


		// temp test if the matrices are comming through coheriently
		bool testingForBoneMatrixBuffer = true;

		if (testingForBoneMatrixBuffer)
		{
			float4x4 identity = 
			{
				{ 1, 0, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, 0, 1 }
			};

			float4x4 testMat =
			{
				{ 1, 2, 3, 4 },
				{ 5, 6, 7, 8 },
				{ 9, 10, 11, 12 },
				{ 13, 14, 15, 16 }
			};

			for (int i = 0; i < 96; i++)
			{
				// if (!matricesTheSame(boneMatrices[i], testMat))
				if (!matricesTheSame(boneMatrices[i], identity))
				{
					if (output.errorColour == 0)
					{
						output.errorColour = 4;
					}
				}
			}
		}



		// check to see if the position isn't (0,0,0)
		if (posLocalSpace.x == 0.0
			&& posLocalSpace.y == 0.0
			&& posLocalSpace.z == 0.0)
		{
			posLocalSpace = input.PosL.xyz; // doing this caurses the model to render in bind pose, the weights or bone indercies maybe wrong
			normalLocalSpace = input.NormL.xyz;
			if (output.errorColour == 0)
			{
				output.errorColour = 2;
			}
			
		}
		

		//float4x4 wvpMat = World * View * Projection;
		float4 posW = mul(float4(posLocalSpace, 1.0f), World);
		output.PosW = posW.xyz;

		// output.PosH = mul(float4(posLocalSpace, 1.0f), wvpMat);
		output.PosH = mul(posW, View);
		output.PosH = mul(output.PosH, Projection);

		output.Tex = input.Tex;

		float3 normalWorldSpace = mul(normalLocalSpace, (float3x3)WorldInverseTranspose);
		normalWorldSpace = normalize(normalWorldSpace);
		output.NormW = normalWorldSpace;
		return output;
	}


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

float calcLerpValue(float min, float max, float x)
{
	// like in the Key frame animation in LLGC
	float rv = 1.0f;
	float diffMinMax = max - min;
	if (diffMinMax <= 0.0f)
	{
		rv = 1.0f;
	}
	else 
	{
		float diffMinX = x - min;
		rv = diffMinX / diffMinMax;
	}


	//float diffMinX;
	//diffMinX = x - min;

	// return diffMinX / diffMinMax;
	
	return rv;
}

bool between(float min, float max, float x)
{
	bool rv = true;
	if (x < min)
	{
		rv = false;
	}
	else if (x > max)
	{
		rv = false;
	}
	return rv;
}	

float4 drawTerrain(VS_OUTPUT input)
{
	// just gouraud shading for terrain, specular components, overkill? ask in next tutorial
	float4 textureColourValue = float4(1.0, 1.0f, 1.0f, 1.0f);

	float heightForVertex = input.PosW.y;

	/* value 0 - 255 * terrainScaledBy */ // see the ranges at top of this file
	// 0 - 255 wrong, 0.0 - 1.0 right


	
	
	
	
	
	// new order to down
	// snow
	// rock
	// light dirt
	// dark dirt
	// grass

	float snowMax = 1.0f;
	float stoneMax = 0.8f;
	float lightDirtMax = 0.6f;
	float darkDirtMax = 0.4f;
	float grassMax = 0.2f;

	// snow for any thing above stoneCutOff
	if (heightForVertex > snowMax * terrainScaledBy)
	{
		textureColourValue = terrainSnowTex.Sample(samLinear, input.Tex);
	}
	else if (between(stoneMax * terrainScaledBy, snowMax * terrainScaledBy, heightForVertex))
	{
		float4 snowTex = terrainSnowTex.Sample(samLinear, input.Tex);
		float4 stoneTex = terrainStoneTex.Sample(samLinear, input.Tex);
		float lerpVal = calcLerpValue(stoneMax * terrainScaledBy, snowMax * terrainScaledBy, heightForVertex);
		textureColourValue = lerp(stoneTex, snowTex, lerpVal);
	}
	else if (between(lightDirtMax * terrainScaledBy, stoneMax * terrainScaledBy, heightForVertex))
	{
		float4 stoneTex = terrainStoneTex.Sample(samLinear, input.Tex);
		float4 lightDirtTex = terrainLightDirtTex.Sample(samLinear, input.Tex);
		float lerpVal = calcLerpValue(lightDirtMax * terrainScaledBy, stoneMax * terrainScaledBy, heightForVertex);
		textureColourValue = lerp(lightDirtTex, stoneTex, lerpVal);
	}
	else if (between(darkDirtMax * terrainScaledBy, lightDirtMax * terrainScaledBy, heightForVertex))
	{
		float4 darkDirtTex = terrainDarkDirtTex.Sample(samLinear, input.Tex);
		float4 lightDirtTex = terrainLightDirtTex.Sample(samLinear, input.Tex);
		float lerpVal = calcLerpValue(darkDirtMax * terrainScaledBy, lightDirtMax * terrainScaledBy, heightForVertex);
		textureColourValue = lerp(darkDirtTex, lightDirtTex, lerpVal);
	}
	else if (between(grassMax * terrainScaledBy, darkDirtMax * terrainScaledBy, heightForVertex))
	{
		float4 darkDirtTex = terrainDarkDirtTex.Sample(samLinear, input.Tex);
		float4 grassTex = terrainGrassTex.Sample(samLinear, input.Tex);
		float lerpVal = calcLerpValue(grassMax * terrainScaledBy, darkDirtMax * terrainScaledBy, heightForVertex);
		textureColourValue = lerp(grassTex, darkDirtTex, lerpVal);
	}
	else
	{
		textureColourValue = terrainGrassTex.Sample(samLinear, input.Tex);
	}
	// following works, need to include some blending
	/*
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
	*/


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
	if (input.errorColour == 1) // the weights all add up to 0
	{
		return float4(1.0f, 0.0f, 0.0f, 1.0f); // red
	}
	else if (input.errorColour == 2) // the calculated position via weight didn't move
	{
		return float4(0.0f, 1.0f, 0.0f, 1.0f); // green
	}
	else if (input.errorColour == 3) // the bone matrix index was out of bounds
	{
		return float4(0.0f, 0.0f, 1.0f, 1.0f); // blue
	}
	else if (input.errorColour == 4) // the bone constant buffer isn't comming through correctly
	{
		return float4(1.0, 1.0, 0.0, 1.0); // yellow
	}

	if (drawingMode == 1.0f)
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