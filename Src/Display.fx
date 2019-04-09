//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------

PixelInputType VS( VertexInputType v )
{
	PixelInputType p;
	p.position = v.position;
	p.tex = v.tex;
    return p;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
Texture2D shaderTexture;
SamplerState SampleType;

float4 PS( PixelInputType input ) : SV_Target
{
	float4 textureColor;

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = 0.8f * shaderTexture.Sample(SampleType, input.tex);
    textureColor += 0.05f * shaderTexture.Sample(SampleType, input.tex, int2( 1,0));
    textureColor += 0.05f * shaderTexture.Sample(SampleType, input.tex, int2(0, 1));
    textureColor += 0.05f * shaderTexture.Sample(SampleType, input.tex, int2(0,-1));
    textureColor += 0.05f * shaderTexture.Sample(SampleType, input.tex, int2(-1,0));
    textureColor += 0.025f * shaderTexture.Sample(SampleType, input.tex, int2( 1,-1));
    textureColor += 0.025f * shaderTexture.Sample(SampleType, input.tex, int2(1, 1));
    textureColor += 0.025f * shaderTexture.Sample(SampleType, input.tex, int2(-1,-1));
    textureColor += 0.025f * shaderTexture.Sample(SampleType, input.tex, int2(-1,1));

    return textureColor;
}

//--------------------------------------------------------------------------------------
