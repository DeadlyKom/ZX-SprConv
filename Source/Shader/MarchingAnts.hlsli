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
    const int2 texAddrOffsets[8] = {
            int2(-1, -1),   // 0
            int2( 0, -1),   // 1
            int2( 1, -1),   // 2
            int2(-1,  0),   // 3
            int2( 1,  0),   // 4
            int2(-1,  1),   // 5
            int2( 0,  1),   // 6
            int2( 1,  1),   // 7
    };

    float lum[8];
    for (int i = 0; i < 8; ++i)
    {
        float3 color = Texture0.Load(int3((int2(Input.uv * TextureSize * 16.0) + texAddrOffsets[i]), 0));
        lum[i] = sqrt((color.x * color.x) + (color.y * color.y) + (color.z * color.z));
    }

    float x = 1 * lum[0] + 2 * lum[3] + 1 * lum[5] - 1 * lum[2] - 2 * lum[4] - 1 * lum[7];
    float y = 1 * lum[0] + 2 * lum[1] + 1 * lum[2] - 1 * lum[5] - 2 * lum[6] - 1 * lum[7];
    float sobel = sqrt(x * x + y * y);

    float c = ((int)(Input.pos.x + Input.pos.y + TimeCounter * 16) % 4);
    return (c < 2 ? float4(0, 0, 0, sobel) : float4(1, 1, 1, sobel));
}
