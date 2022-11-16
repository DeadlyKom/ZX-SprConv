cbuffer pixelBuffer :register(b0)
{
    float4   GridColor;
    float2   GridWidth;
    bool     AttributeGrid;
    float    TimeCounter;
    float3   BackgroundColor;
    bool     ForceNearestSampling;
    float2   TextureSize;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

sampler Sampler0;
Texture2D Texture0;

float4 main(PS_INPUT Input) : SV_TARGET
{
    float2 UV;

    float2 Texel = (Input.uv) * TextureSize;
    if (ForceNearestSampling)
	    UV = (floor(Texel) + float2(0.5, 0.5)) / TextureSize;
    else
	    UV = Input.uv;
    float2 TexelEdge = step(Texel - floor(Texel), GridWidth);
    float IsGrid = max(TexelEdge.x, TexelEdge.y);
    float4 C = Texture0.Sample(Sampler0, UV);
    C.rgb += BackgroundColor * (1.0 - C.a);

    float2 uv_g = UV;
    uv_g.y *= TextureSize.y / TextureSize.x;
    float repeats = floor(TextureSize.x / 8);
    float cx = floor(repeats * uv_g.x);
    float cy = floor(repeats * uv_g.y);
    float result = fmod(cx + cy, 2.0);
    float ch = sign(result);
    float gray = AttributeGrid ? lerp(0.8, 1.0, ch) : 1.0f;

    float2 TexelA = (Input.uv) * TextureSize / 8;
    float2 TexelEdgeA = step(TexelA - floor(TexelA), GridWidth * 0.35);
    float IsGridA = max(TexelEdgeA.x, TexelEdgeA.y);
    float4 GridColorA = float4(0,0,1,0.45);

    C = lerp(C * gray, float4(GridColor.rgb, 1), GridColor.a * IsGrid);
    C = lerp(C, float4(GridColorA.rgb, 1), GridColor.a * GridColorA.a * IsGridA);
    return C;
}
