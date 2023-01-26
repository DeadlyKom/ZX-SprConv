cbuffer pixelBuffer :register(b0)
{
    float4   GridColor;
    float2   GridWidth;
    int      Flags;
    float    TimeCounter;
    float3   BackgroundColor;
    int      Dummy;
    float2   TextureSize;
    float2   GridSize;
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
    float C = ((int)(Input.pos.x + Input.pos.y - TimeCounter * 16) % 8);
    return (C < 4 ? float4(0, 0, 0, 1) : float4(1, 1, 1, 1));
}
