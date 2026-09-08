#include "SimpleObstaclePlacementFlow.h"
#include <ImGuiManager/ImGuiManager.h>
#include"Game.h"
#include<numbers>
#include<algorithm>

const float kPi = std::numbers::pi_v<float>;
void SimpleObstaclePlacementFlow::DrawImGui()
{
    ImGui::Begin("Obstacle Placement Settings");

    // --- 1. 配置オブジェクト（PieceMotion）のデータ確認 ---
    if (ImGui::TreeNode("Pieces List"))
    {
        ImGui::Text("Total Pieces: %d", static_cast<int>(pieces_.size()));
        ImGui::Text("Next Index: %d | Is Placing: %s", nextPieceIndex_, isPlacing_ ? "true" : "false");
        ImGui::Separator();

        for (size_t i = 0; i < pieces_.size(); ++i)
        {
            const auto& p = pieces_[i];

            // 状態（Enum）を文字列に変換
            const char* stateStr = "Unknown";
            switch (p.state)
            {
            case PieceState::Hidden:   stateStr = "Hidden"; break;
            case PieceState::Throwing: stateStr = "Throwing"; break;
            case PieceState::Landing:  stateStr = "Landing"; break;
            case PieceState::Finished: stateStr = "Finished"; break;
            }

            // 要素ごとにツリーノードを作成 (識別IDにポインタアドレスを使用)
            if (ImGui::TreeNode((void*)(intptr_t)i, "Piece [%d] - State: %s", static_cast<int>(i), stateStr))
            {
                // 現在の Transform
                if (ImGui::TreeNode("Current Transform"))
                {
                    ImGui::Text("Pos: (%.2f, %.2f, %.2f)", p.transform.translate.x, p.transform.translate.y, p.transform.translate.z);
                    ImGui::Text("Rot: (%.2f, %.2f, %.2f)", p.transform.rotate.x, p.transform.rotate.y, p.transform.rotate.z);
                    ImGui::Text("Scale: (%.2f, %.2f, %.2f)", p.transform.scale.x, p.transform.scale.y, p.transform.scale.z);
                    ImGui::TreePop();
                }

                // 元の配置データ (Editor上の位置)
                if (ImGui::TreeNode("Local Base Data"))
                {
                    ImGui::Text("Local Pos: (%.2f, %.2f, %.2f)", p.localPosition.x, p.localPosition.y, p.localPosition.z);
                    ImGui::Text("Local Rot: (w:%.2f, x:%.2f, y:%.2f, z:%.2f)", p.localRotation.w, p.localRotation.x, p.localRotation.y, p.localRotation.z);
                    ImGui::Text("Local Scale: (%.2f, %.2f, %.2f)", p.localScale.x, p.localScale.y, p.localScale.z);
                    ImGui::TreePop();
                }

                // 進行状態・演出用パラメータ
                if (p.state == PieceState::Throwing)
                {
                    ImGui::Text("Throw Timer: %.2f s", p.throwTimer);
                    ImGui::Text("Next Ready: %s", p.nextReady ? "YES" : "NO");
                    ImGui::Text("Start Pos: (%.2f, %.2f, %.2f)", p.startPosition.x, p.startPosition.y, p.startPosition.z);
                    ImGui::Text("Target Pos: (%.2f, %.2f, %.2f)", p.targetPosition.x, p.targetPosition.y, p.targetPosition.z);
                } else if (p.state == PieceState::Landing)
                {
                    ImGui::Text("Landing Timer: %.2f s", p.landingTimer);
                    ImGui::Text("Tilt Amount: %.2f", p.tiltAmount);
                    ImGui::Text("Spin Amount: %.2f", p.spinAmount);
                    ImGui::Text("Wobble Offset: %.2f", p.wobbleOffset);
                    ImGui::Text("Tilt Axis: (%.2f, %.2f, %.2f)", p.tiltAxis.x, p.tiltAxis.y, p.tiltAxis.z);
                    ImGui::Text("Spin Axis: (%.2f, %.2f, %.2f)", p.spinAxis.x, p.spinAxis.y, p.spinAxis.z);
                }

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    // --- 2. 退場中オブジェクト（DisappearMotion）のデータ確認 ---
    if (ImGui::TreeNode("Disappear Motions List"))
    {
        ImGui::Text("Active Disappears: %d", static_cast<int>(disappearMotions_.size()));
        ImGui::Separator();

        for (size_t i = 0; i < disappearMotions_.size(); ++i)
        {
            const auto& d = disappearMotions_[i];

            if (ImGui::TreeNode((void*)(intptr_t)i, "Disappear [%d]", static_cast<int>(i)))
            {
                ImGui::Text("Timer: %.2f s", d.timer);
                ImGui::Text("Current Pos: (%.2f, %.2f, %.2f)", d.transform.translate.x, d.transform.translate.y, d.transform.translate.z);
                ImGui::Text("Start Pos: (%.2f, %.2f, %.2f)", d.startPosition.x, d.startPosition.y, d.startPosition.z);
                ImGui::Text("Target Pos: (%.2f, %.2f, %.2f)", d.targetPosition.x, d.targetPosition.y, d.targetPosition.z);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }


    // --- 発射地点 (SpawnPoint) ---
    if (ImGui::TreeNode("Spawn Point"))
    {
        ImGui::DragFloat3("Position", &spawnPoint_.translate.x, 0.1f);
        ImGui::DragFloat3("Rotation", &spawnPoint_.rotate.x, 0.1f);
        ImGui::TreePop();
    }

    // --- 飛行・発射パラメータ ---
    if (ImGui::TreeNode("Throw Settings"))
    {
        ImGui::DragFloat("Throw Duration (s)", &throwDuration_, 0.05f, 0.1f, 5.0f);
        ImGui::DragFloat("Throw Arc Height", &throwArcHeight_, 0.1f, 0.0f, 10.0f);
        ImGui::SliderFloat("Next Throw Progress", &nextThrowProgress_, 0.0f, 1.0f, "%.2f");
        ImGui::DragFloat("Next Throw Delay (s)", &nextThrowDelay_, 0.01f, 0.0f, 2.0f);
        ImGui::TreePop();
    }

    // --- 着地・演出パラメータ ---
    if (ImGui::TreeNode("Landing Settings"))
    {
        ImGui::DragFloat("Landing Duration (s)", &landingDuration_, 0.05f, 0.1f, 5.0f);
        ImGui::DragFloat("Landing Tilt Angle", &landingTiltAngle_, 0.5f, 0.0f, 90.0f);
        ImGui::DragFloat("Landing Spin Angle", &landingSpinAngle_, 1.0f, 0.0f, 360.0f);
        ImGui::DragFloat("Landing Wobble Count", &landingWobbleCount_, 0.1f, 0.0f, 10.0f);
        ImGui::DragFloat("Landing Bounce Height", &landingBounceHeight_, 0.01f, 0.0f, 2.0f);
        ImGui::TreePop();
    }

    // --- 退場（消去）パラメータ ---
    if (ImGui::TreeNode("Disappear Settings"))
    {
        ImGui::DragFloat("Disappear Duration (s)", &disappearDuration_, 0.05f, 0.1f, 5.0f);
        ImGui::DragFloat("Disappear Height", &disappearHeight_, 0.1f, 0.0f, 10.0f);
        ImGui::TreePop();
    }

    ImGui::Separator();

    // --- 動作確認用ボタン ---
    if (ImGui::Button("Test Placement"))
    {
        StartPlacement();
    }
    ImGui::SameLine();
    if (ImGui::Button("Test Disappear"))
    {
        StartDisappear();
    }

    ImGui::End();
}

void SimpleObstaclePlacementFlow::SetSpawnPoint(const EulerTransforms& spawnPoint)
{
    spawnPoint_ = spawnPoint;
}

void SimpleObstaclePlacementFlow::Initialize()
{
    pieces_.clear();
    disappearMotions_.clear();
}


void SimpleObstaclePlacementFlow::Update()
{
    // 発射・着地・次発射・退場を毎フレーム更新する。
    UpdatePieces();
    UpdateNextThrow();
    UpdateDisappear();
}

void SimpleObstaclePlacementFlow::ReadObstaclePlacement(const EulerTransforms& transform)
{
    PieceMotion newP;
    // このLocal Transformが最終的な完成形になる。
    newP.transform = transform;
    newP.localPosition = transform.translate;
  // MakeFromEuler は (yaw, pitch, roll) のため (Y, X, Z) で指定
        newP.localRotation = Quaternion::MakeFromEuler(transform.rotate.y, transform.rotate.x, transform.rotate.z);
    newP.localScale = transform.scale;
    newP.state = PieceState::Hidden;

    pieces_.push_back(newP);

}

void SimpleObstaclePlacementFlow::HideAllPieces()
{

    if (pieces_.empty()) {
        return;
    }

    for (auto& p : pieces_) {
        // 元配置へ戻してから非表示にする。
        p.transform.translate = spawnPoint_.translate;
        p.transform.rotate = spawnPoint_.rotate;
        p.transform.scale = p.localScale;
        p.state = PieceState::Hidden;
    
    }

}

void SimpleObstaclePlacementFlow::StartPlacement()
{
    if (pieces_.empty()) {
        return;
    }
    
    // 全要素の状態と初期座標を SpawnPoint に揃えてリセット
    for (auto& p : pieces_) {
        p.state = PieceState::Hidden;
        p.throwTimer = 0.0f;
        p.landingTimer = 0.0f;
        p.nextReady = false;
        p.transform.translate = spawnPoint_.translate;
        p.transform.rotate = spawnPoint_.rotate;
    }

    nextPieceIndex_ = 0;
    nextThrowDelayTimer_ = 0.0f;
    isPlacing_ = true;

    StartNextPiece();
}

void SimpleObstaclePlacementFlow::StartNextPiece()
{

    if (!isPlacing_ || pieces_.empty())
        return;
    if (nextPieceIndex_ >= static_cast<int>(pieces_.size())) return;

    PieceMotion& motion = pieces_[nextPieceIndex_];
    EulerTransforms& piece = motion.transform;

    // SpawnPointへ動かす前に元のLocal Transformを復元する。
    // この状態のWorld Position / Rotationが最終着地点になる。
    piece.translate = motion.localPosition;
    piece.rotate = motion.localRotation.ToEuler();
    piece.scale = motion.localScale;

    motion.targetPosition = piece.translate;
    motion.targetRotation = Quaternion::MakeFromEuler(piece.rotate.y, piece.rotate.x, piece.rotate.z);
  
    // 発射開始地点はSpawnPoint。
    motion.startPosition = spawnPoint_.translate;
    motion.startRotation = Quaternion::MakeFromEuler(spawnPoint_.rotate.y, spawnPoint_.rotate.x, spawnPoint_.rotate.z);

    piece.translate = motion.startPosition;
    piece.rotate = motion.startRotation.ToEuler();

    motion.throwTimer = 0.0f;
    motion.landingTimer = 0.0f;
    motion.nextReady = false;
    motion.state = PieceState::Throwing;

    nextPieceIndex_++;
    nextThrowDelayTimer_ = 0.0f;
}

void SimpleObstaclePlacementFlow::UpdatePieces()
{

    if (pieces_.empty()) {
        return;
    }

    bool moving = false;

    for (auto& p : pieces_) {

        if (p.state == PieceState::Throwing)
        {
            moving = true;
            UpdateThrow(p);

        } else if (p.state == PieceState::Landing)
        {
            moving = true;
            UpdateLanding(p);
        }
    }

    // 全て発射済みで、発射中・着地演出中のPieceも無ければ配置完了。
    if (isPlacing_ &&
        nextPieceIndex_ >= pieces_.size() &&
        !moving)
    {
        isPlacing_ = false;
    }

}

void SimpleObstaclePlacementFlow::UpdateThrow(PieceMotion& motion)
{

    motion.throwTimer += Game::Time::GetScaledDeltaTimeMs() * 0.001f;

    // 発射開始から着地までの進行率を0～1にする。
    // t = 経過時間 / 発射時間
    float t = std::clamp(motion.throwTimer / throwDuration_, 0.0f, 1.0f);

    // 指定した進行率に達したら次のPieceを発射可能にする。
    if (!motion.nextReady &&
        t >= nextThrowProgress_)
    {
        motion.nextReady = true;
        nextThrowDelayTimer_ = 0.0f;
    }

    // 開始と終了をゆっくりさせる。
    float moveT = EaseInOutCubic(t);

    // 基本移動:
    Vector3 position = Game::Math::Ease::Easing(motion.startPosition,
        motion.targetPosition, EaseType::IN_OUT_CUBIC, t);


    float pi = std::numbers::pi_v<float>;
    position += Vector3{ 0.0f, 1.0f, 0.0f } * std::sin(pi * t) * throwArcHeight_;

    motion.transform.translate = position;

    // SpawnPointの回転から本来の回転へ滑らかに補間する。
    motion.transform.rotate = Quaternion::Slerp(motion.startRotation,
        motion.targetRotation,
        moveT).ToEuler();

    if (t >= 1.0f)
    {
        // 誤差を残さず定位置へ合わせてから着地演出へ移る。
        motion.transform.translate = motion.targetPosition;
        motion.transform.rotate = motion.targetRotation.ToEuler();

        StartLanding(motion);
    }


}

void SimpleObstaclePlacementFlow::UpdateNextThrow()
{

    if (!isPlacing_ || pieces_.empty())
    {
        return;
    }

    // 発射中のPieceがnextThrowProgress_を越えていれば次を出せる。
    bool canThrow = false;

    for (auto& p : pieces_) {
        if (p.state == PieceState::Throwing && p.nextReady)
        {
            canThrow = true;
            break;
        }
    }

    if (!canThrow) {
        return;
    }

    nextThrowDelayTimer_ += Game::Time::GetScaledDeltaTimeMs() * 0.001f;

    if (nextThrowDelayTimer_ < nextThrowDelay_) {
        return;
    }

    nextThrowDelayTimer_ = 0.0f;
    StartNextPiece();


}

void SimpleObstaclePlacementFlow::StartLanding(PieceMotion& motion)
{

    // 着地点と本来の回転を着地演出の基準にする。
    motion.landingPosition = motion.targetPosition;
    motion.landingBaseRotation = motion.targetRotation;
    motion.landingTimer = 0.0f;

    // XZ平面上にランダムな傾き軸を作る。
    float tiltDirection = Game::Math::Rand::RandFloat(0.0f, kPi * 2.0f, 1);

    motion.tiltAxis = Vector3(
        std::cosf(tiltDirection),
        0.0f,
        std::sinf(tiltDirection)
    ).Normalize();

    // Spinは完全なY軸ではなく、少しX/Zへ傾いた軸で行う。
    motion.spinAxis = Vector3(
        Game::Math::Rand::RandFloat(-0.35f, 0.35f, 1),
        1.0f,
        Game::Math::Rand::RandFloat(-0.35f, 0.35f, 1)
    ).Normalize();


    // Pieceごとに傾き量とSpin量へ個体差を付ける。
    motion.tiltAmount = landingTiltAngle_ * Game::Math::Rand::RandFloat(0.7f, 1.15f, 1);

    motion.spinAmount =
        landingSpinAngle_ * Game::Math::Rand::RandFloat(0.75f, 1.25f, 1);

    ;
    // 半分の確率でSpin方向を逆転させる。
    if (Game::Math::Rand::RandInt(0, 1) == 0)
        motion.spinAmount = -motion.spinAmount;

    // 揺れ始める位相もPieceごとにずらす。
    motion.wobbleOffset = Game::Math::Rand::RandFloat(0.0f, kPi * 2.0f, 1);
    motion.state = PieceState::Landing;

}

void SimpleObstaclePlacementFlow::UpdateLanding(PieceMotion& motion)
{

    motion.landingTimer += Game::Time::GetScaledDeltaTimeMs() * 0.001f;

    // 着地演出の進行率。0が開始、1が終了。
    float t = std::clamp(motion.landingTimer / landingDuration_, 0.0f, 1.0f);

    // damping = (1 - t)^2
    // 着地直後を1、終了時を0として演出を徐々に弱める。
    float damping = 1.0f - t;
    damping *= damping;

    // メイン揺れ:
    // wobble = sin(t * WobbleCount * 2PI + Offset)
    float wobble = std::sinf(
        t *
        landingWobbleCount_ *
       kPi*
        2.0f +
        motion.wobbleOffset);

    // 傾き角 = 個体ごとの傾き量 * sin波 * 減衰
    float tiltAngle = motion.tiltAmount * wobble * damping;

    Quaternion tiltRotation = Quaternion::MakeRotateAxisAngleQuaternion(motion.tiltAxis, tiltAngle);

    // Spinは序盤で増やし、終盤で0へ減衰させる。
    float spinProgress = 1.0f - damping;

    float spinFade = SmoothStep(1.0f, 0.0f, t);

    float spinAngle =
        motion.spinAmount *
        spinProgress *
        spinFade;

    Quaternion spinRotation = Quaternion::MakeRotateAxisAngleQuaternion(motion.spinAxis, spinAngle);

    // メインTiltと直角方向にも小さく揺らし、
    // 単純な振り子のように見えるのを防ぐ。
    Vector3 secondaryAxis = { 0.0f,1.0f,0.0f };
    secondaryAxis = secondaryAxis.Cross(motion.tiltAxis);

    Vector3 right = { std::cos(motion.transform.rotate.y), 0.0f, std::sin(motion.transform.rotate.y) };

    if (secondaryAxis.Dot(secondaryAxis) < 0.0001f)
        secondaryAxis = right;

    secondaryAxis.Normalize();

    // Secondary Tiltは
    // メインTiltの35%、周波数1.35倍、位相+1.2でずらす。
    float secondaryTilt =
        motion.tiltAmount *
        0.35f *
        std::sinf(
            t *
            landingWobbleCount_ *
            kPi *
            2.0f *
            1.35f +
            motion.wobbleOffset +
            1.2f
        ) *
        damping;

    Quaternion secondaryRotation = Quaternion::MakeRotateAxisAngleQuaternion(secondaryAxis, secondaryTilt);

    // 本来の回転
    // × Spin
    // × メインTilt
    // × Secondary Tilt
    // の順で回転を合成する。
    motion.transform.rotate = Quaternion{
        motion.landingBaseRotation *
        spinRotation *
        tiltRotation *
        secondaryRotation }.ToEuler();

    // absで必ず0以上にし、定位置より下へ潜らず上方向だけに跳ねる。
    float bounce = std::abs(std::sinf(t * kPi * 2.0f))* boundHeight_;

    motion.transform.translate =
        motion.landingPosition +
        Vector3{ 0.0f,1.0f,0.0f } *
        bounce;

    if (t >= 1.0f)
        FinishLanding(motion);

}

void SimpleObstaclePlacementFlow::FinishLanding(PieceMotion& motion)
{
    // 演出用の傾き・Spin・Bounceを残さず、
    // Editor上で設定されていた元配置へ完全に戻す。
    motion.transform.translate = motion.localPosition;
    //オイラー角にする
    motion.transform.rotate = motion.localRotation.ToEuler();
    motion.transform.scale = motion.localScale;

    motion.state = PieceState::Finished;

}

void SimpleObstaclePlacementFlow::StartDisappear()
{

    if (pieces_.empty()) {
        return;
    }


    for (auto& p : pieces_) {
        // 現在位置からWorld Up方向へ
            // disappearHeight_だけ上昇させる。
        DisappearMotion motion;
        motion.transform = p.transform;
        motion.startPosition = p.transform.translate;
        motion.targetPosition = p.transform.translate + Vector3{ 0.0f,1.0f,0.0f } * disappearHeight_;
        motion.timer = 0.0f;

        disappearMotions_.push_back(motion);

        // 配置側の更新対象から外す。
        p.state = PieceState::Finished;
    }


    isPlacing_ = false;

}

void SimpleObstaclePlacementFlow::UpdateDisappear()
{

    // 削除しながらListから外すため後ろから走査する。
    for (int i = disappearMotions_.size() - 1; i >= 0; i--)
    {
        DisappearMotion& motion = disappearMotions_[i];

        motion.timer += Game::Time::GetScaledDeltaTimeMs() * 0.001f;

        float t = std::clamp(motion.timer / disappearDuration_, 0.0f, 1.0f);

        float moveT = EaseInOutCubic(t);

        pieces_[i].transform.translate =  Game::Math::Ease::Easing(
            motion.startPosition,
            motion.targetPosition,
            EaseType::IN_OUT_CUBIC,
            t);

        if (t >= 1.0f)
        {
            disappearMotions_.erase(disappearMotions_.begin() + i);
            //ここでオブジェクトを破棄したい。

        }
    }


}

float SimpleObstaclePlacementFlow::EaseInOutCubic(float t)
{

    t = std::clamp(t, 0.0f, 1.0f);
    if (t < 0.5f) return 4.0f * t * t * t;

    float value = -2.0f * t + 2.0f;
    return 1.0f - value * value * value / 2.0f;

}

float SimpleObstaclePlacementFlow::SmoothStep(float from, float to, float t)
{

    // t を 0.0 〜 1.0 の範囲に制限
    t = std::clamp(t, 0.0f, 1.0f);

    // S字カーブ補間 (3*t^2 - 2*t^3)
    t = t * t * (3.0f - 2.0f * t);

    // 補間計算
    return from + (to - from) * t;

}
