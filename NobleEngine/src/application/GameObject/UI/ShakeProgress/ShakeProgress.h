#pragma once
#include<Game.h>

class UIModel;

class ShakeProgress
{
public:
    ShakeProgress();
    ~ShakeProgress();
    //シェイク
    void SetShakeProgress(const float shake) { shake_.shakeProgress = shake; };
    const float GetShakeProgress() { return shake_.shakeProgress; }
    void Initialize();
    void Update(const int32_t uiCameraID);
    void Draw(const int32_t uiRenderTextureID);
    void DebugUI();
private:



    struct modelIDs {
        int32_t model_ = -1;
        int32_t texture_ = -1;
    };

    struct Texture {
        int32_t maskTexture_ = -1;
        int32_t texture_ = -1;   
        int32_t padding1 = -1;
        int32_t padding2 = -1;
    };

    // 閾値から ShakeParams に変更
    struct ShakeParams 
    {
        float shakeProgress = 0.0f; // シェイク進捗 (0.0 ~ 1.0)
        float time = 0.0f; // アニメーション用経過時間
        float glassTopY; // グラスの口（上端）の全画面UV.y (例: 0.05)
        float glassBottomY; // グラスの底（下端）の全画面UV.y (例: 0.35)
    };
private:

    Texture textureIDs_;
    ShakeParams shake_;

    //モデル
    std::unordered_map<std::string, modelIDs>modelIds_;
  
    //カクテル
    std::unique_ptr<UIModel>cockTailSignboard_ = nullptr;
    //カクテル　グラス
    std::unique_ptr<UIModel>glassModel_ = nullptr;

    std::unique_ptr<RenderObject>renderObject_ = nullptr;
};

