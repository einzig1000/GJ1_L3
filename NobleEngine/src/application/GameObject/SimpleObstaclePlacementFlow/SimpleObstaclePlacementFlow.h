#pragma once

#include <cstdint>
#include <vector>
#include <EngineDefinition/EngineDefinition.h>

class SimpleObstaclePlacementFlow
{

private:
    // 全障害物の発射開始位置・回転。
    EulerTransforms spawnPoint_;

    // 1個がSpawnPointから定位置へ到着するまでの時間。
    float throwDuration_ = 0.8f;
    // 飛行中の弧の高さ。
    float throwArcHeight_ = 3.0f;
    // 現在の障害物がこの進行率まで飛んだら次を発射可能にする。
    float nextThrowProgress_ = 0.55f;
    // 次の障害物を実際に発射するまでの追加待機時間。
    float nextThrowDelay_ = 0.05f;
    // 着地後の傾き・揺れ・回転・Bounceの演出時間。
    float landingDuration_ = 0.9f;
    // 着地時の基準傾き角度。
    float landingTiltAngle_ = 14.0f;
    // 着地時の基準Spin角度。
    float landingSpinAngle_ = 110.0f;
    // 着地演出中に揺れる回数。
    float landingWobbleCount_ = 2.5f;
    // 着地時に上方向へ跳ねる高さ。
    float landingBounceHeight_ = 0.08f;
    // 上昇退場にかける時間。
    float disappearDuration_ = 0.6f;
    // 退場時にWorld Up方向へ移動する高さ。
    float disappearHeight_ = 10.0f;
    //最大バウンド高さ
    float boundHeight_ = 0.125f;

    enum PieceState
    {
        Hidden,     // 発射前
        Throwing,   // 発射中
        Landing,    // 着地演出中
        Finished    // 配置完了
    };


    // 障害物1個分の配置・発射・着地に必要な情報。
    struct PieceMotion
    {
        EulerTransforms transform;
        PieceState state = PieceState::Hidden;

        // Editor上の元配置。演出終了時は必ずここへ戻す。
        Vector3 localPosition;
        Quaternion localRotation;
        Vector3 localScale;

        // 発射開始位置と最終着地点。
        Vector3 startPosition;
        Vector3 targetPosition;
        Quaternion startRotation;
        Quaternion targetRotation;

        float throwTimer;
        bool nextReady;

        // 着地演出用。
        float landingTimer;
        Vector3 landingPosition;
        Quaternion landingBaseRotation;

        Vector3 tiltAxis;
        Vector3 spinAxis;

        float tiltAmount;
        float spinAmount;
        float wobbleOffset;
    };

    // 退場中の障害物1個分の情報。
    struct DisappearMotion
    {
        EulerTransforms transform;
        Vector3 startPosition;
        Vector3 targetPosition;
        float timer;
    };

    std::vector<PieceMotion> pieces_;

    int nextPieceIndex_ = 0;
    float nextThrowDelayTimer_ = 0.0f;
    bool isPlacing_ = false;

    std::vector<DisappearMotion> disappearMotions_;

public:
    void DrawImGui();
    void SetSpawnPoint(const EulerTransforms& spawnPoint);

    const PieceMotion& GetPieces(int32_t index) { return pieces_[index]; };

    // Editorから障害物の位置を直接書き換える(localPositionと現在のtransformを両方更新)
    void SetPieceLocalPosition(int32_t index, const Vector3& position);

    void Initialize();

    void HideAllPieces();

    void Update();
    /// @brief 外部から呼び出しする
    /// @param transform 
    void ReadObstaclePlacement(const EulerTransforms& transform);

    // 外部から呼ぶと、現在表示中の障害物を上昇させて削除する。
    void StartDisappear();
    // 外部から呼ぶと、1個目から順番に配置演出を開始する。
    void StartPlacement();
    //配置完了を取得する
    bool IsPlacing() { return isPlacing_; };
private:
    void StartNextPiece();

    void UpdatePieces();
    void UpdateNextThrow();
    void UpdateDisappear();

    void UpdateThrow(PieceMotion& motion);

    void StartLanding(PieceMotion& motion);
    void UpdateLanding(PieceMotion& motion);
    void FinishLanding(PieceMotion& motion);


    // Ease In Out Cubic
    // 開始と終了をゆっくり、中央付近を速くする。
    float EaseInOutCubic(float t);
    float SmoothStep(float from, float to, float t);

};

