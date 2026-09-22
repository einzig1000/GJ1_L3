
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};


cbuffer Texture : register(b0)
{
    int maskTextureIndex;
};

// 閾値から ShakeParams に変更
cbuffer ShakeParams : register(b1)
{
    float shakeProgress; // シェイク進捗 (0.0 ~ 1.0)
    float time; // アニメーション用経過時間
    float glassTopY; // グラスの口（上端）の全画面UV.y (例: 0.05)
    float glassBottomY; // グラスの底（下端）の全画面UV.y (例: 0.35)
};
cbuffer EdgeColor : register(b2)
{
    float3 edgeColor;
};

SamplerState gSampler : register(s0);

PSOutput main(PSInput input)
{
    PSOutput output;
    
    Texture2D<float4> maskTexture = ResourceDescriptorHeap[maskTextureIndex];
    
    float mask = maskTexture.Sample(gSampler, input.TexCoord).r;
    
    if (mask < 0.1f)
    {
        discard;
    }
    
    // -------------------------------------------------------------
    // グラスの領域内での高さ (0.0: グラスの底 ～ 1.0: グラスの口) に正規化
    // UVのYは「上が0.0、下が1.0」なので注意
    // -------------------------------------------------------------
    float heightFromGlassBottom = (glassBottomY - input.TexCoord.y) / (glassBottomY - glassTopY);
    
    // 範囲外（グラスの底より下や口より上）の安全処理
    heightFromGlassBottom = saturate(heightFromGlassBottom);
    
    // 水面より上のピクセルは描画しない（破棄）
    if (heightFromGlassBottom > shakeProgress)
    {
        output.Color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    }
    else
    {
       // 液体のテクスチャカラーを取得
        output.Color = float4(1.0f, 1.0f, 0.0f, 1.0f);
    
        
            // -------------------------------------------------------------
    // 水面（液面）のエッジ強調
    // -------------------------------------------------------------
        float edgeWidth = 0.05f; // エッジ幅（正規化空間での幅）
        float distToSurface = shakeProgress - heightFromGlassBottom;
    
        if (distToSurface < edgeWidth)
        {
            float edgeFactor = 1.0f - (distToSurface / edgeWidth);
            output.Color.rgb += edgeColor * edgeFactor;
        }
    }
    



    
    return output;
}