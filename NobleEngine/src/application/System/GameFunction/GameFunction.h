#pragma once
#include "../../../engine/EngineDefinition/EngineDefinition.h"

namespace GameFunction

{       /// <summary>
        /// 大きい円の内壁と小さい円が接触しているかを判定する
        /// </summary>
        /// <param name="bigCenter">でかい円の中心</param>
        /// <param name="bigRadius">でかい円の半径</param>
        /// <param name="smallCenter">小さい円の中心</param>
        /// <param name="smallRadius">小さい円の半径</param>
        /// <returns></returns>
    bool IsTouchingInnerEdge(
        const Vector2& bigCenter,
        float bigRadius,
        const Vector2& smallCenter,
        float smallRadius);

    /// <summary>
    /// でかい円から見て小さい円がどの方向にあるかを返す
    /// </summary>
    /// <param name="bigCenter">でかい円の中心</param>
    /// <param name="smallCenter">小さい円の中心</param>
    /// <returns> 0〜360</returns>
    float GetContactAngleDeg(const Vector2& bigCenter, const Vector2& smallCenter);
    // angleDegがfromDegからtoDegの範囲に入っているか
    bool IsInAngleRange(float angleDeg, float fromDeg, float toDeg);


    Vector3 GetPositionOnCircle(const Vector3& center, float radius, float angleDeg);

    float NormalizeAngle(float a);

    float AngleDiff(float a, float b);

    float ClosestAngle(float target, float base);

    // 現在のtheta(ラジアン)から見て、targetDeg(度)と等価な角度のうち最短距離になるものをラジアンで返す
    float ClosestThetaRadian(float currentThetaRad, float targetDeg);
};

