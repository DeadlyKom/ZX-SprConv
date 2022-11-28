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

Texture2D Texture0;

float4 main(PS_INPUT Input) : SV_TARGET
{
    const int2 TexAddrOffsets[8] =
    {
            int2(-1, -1),
            int2( 0, -1),
            int2( 1, -1),
            int2(-1,  0),
            int2( 1,  0),
            int2(-1,  1),
            int2( 0,  1),
            int2( 1,  1),
    };

    uint Width, Height, Levels;
    Texture0.GetDimensions(0, Width, Height, Levels);
    //return Texture0.Load(int3((int2(Input.uv * float2(Width, Height))), 0));

    float Lum[8];
    for (int i = 0; i < 8; ++i)
    {
        float3 Color = Texture0.Load(int3((int2(Input.uv * float2(Width, Height) ) + TexAddrOffsets[i]), 0));
        Lum[i] = sqrt((Color.x* Color.x) + (Color.y * Color.y) + (Color.z * Color.z));
    }

    float x = 1 * Lum[0] + 2 * Lum[3] + 1 * Lum[5] - 1 * Lum[2] - 2 * Lum[4] - 1 * Lum[7];
    float y = 1 * Lum[0] + 2 * Lum[1] + 1 * Lum[2] - 1 * Lum[5] - 2 * Lum[6] - 1 * Lum[7];
    float Sobel = sqrt(x * x + y * y);

    float C = ((int)(Input.pos.x + Input.pos.y + TimeCounter * 16) % 4);
    return (C < 2 ? float4(0, 0, 0, Sobel) : float4(1, 1, 1, Sobel));
}
