#include "GameFunction.h"
#include<Game.h>
#include<numbers>

namespace GameFunction {

        bool IsTouchingInnerEdge(
            const Vector2& bigCenter,
            float bigRadius,
            const Vector2& smallCenter,
            float smallRadius)
        {
            float dist = (smallCenter - bigCenter).Length();
            return (dist + smallRadius) >= bigRadius;
        }

        float GetContactAngleDeg(const Vector2& bigCenter, const Vector2& smallCenter)
        {
            Vector2 dir = smallCenter - bigCenter;
            if (dir.LengthSq() < 0.0001f)
            {
                return 0.0f;
            }

            float angleRad = std::atan2(dir.y, dir.x);
            float angleDeg = angleRad * (180.0f / std::numbers::pi_v<float>);
            if (angleDeg < 0.0f) angleDeg += 360.0f;
            return angleDeg; // 0〜360
        }

        // angleDegがfromDegからtoDegの範囲に入っているか
        bool IsInAngleRange(float angleDeg, float fromDeg, float toDeg)
        {
            auto Normalize = [](float a)
                {
                    a = fmod(a, 360.0f);
                    if (a < 0.0f) a += 360.0f;
                    return a;
                };

            float a = Normalize(angleDeg);
            float f = Normalize(fromDeg);
            float t = Normalize(toDeg);

            if (f <= t)
            {
                // 通常の範囲（例：30°～120°）
                return a >= f && a < t;
            } else
            {
                // 360°を跨ぐ範囲（例：350°～20°）
                return a >= f || a < t;
            }
        }


        Vector3 GetPositionOnCircle(const Vector3& center, float radius, float angleDeg)
        {
            float angleRad = angleDeg * (std::numbers::pi_v<float> / 180.0f);
            return Vector3(
                center.x + radius * std::cos(angleRad),
                center.y,
                center.z + radius * std::sin(angleRad)
            );
        }

        float NormalizeAngle(float a)
        {
            a = std::fmod(a, 360.0f);
            if (a < 0) a += 360.0f;
            return a;
        }

        float AngleDiff(float a, float b)
        {
            float diff = NormalizeAngle(a - b);
            if (diff > 180.0f) diff -= 360.0f;
            return diff;
        }

        float ClosestAngle(float target, float base)
        {
            float diff = AngleDiff(target, base);
            return target - diff;
        }

        // 現在のtheta(ラジアン)から見て、targetDeg(度)と等価な角度のうち最短距離になるものをラジアンで返す
        float ClosestThetaRadian(float currentThetaRad, float targetDeg)
        {
            float currentDeg = Game::Math::Converter::RadianToDegree(currentThetaRad);
            return Game::Math::Converter::DegreeToRadian(ClosestAngle(currentDeg, targetDeg));
        }


}