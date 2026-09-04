#include "CollisionManager.h"
#include "../Collider/Collider.h"
#include<algorithm>
#include "Utilities/functions.h"
#include"Game.h"

void CollisionManager::Load()
{
    Collider::Load();

    CollisionTag::LoadTagNames();
    //グラス
    CollisionTag::AddTag("Glass");
    //ターゲットのお客様
    CollisionTag::AddTag("Target");
    //椅子
    CollisionTag::AddTag("Chair");
    //障害物
    CollisionTag::AddTag("Obstacles");
    //障害物
    CollisionTag::AddTag("Table");

    CollisionTag::SaveTagNames();
}

void CollisionManager::Finalize()
{
    //コライダーを消去
    ClearColliders();
    //タグ名をセーブする
    CollisionTag::SaveTagNames();
}

#ifdef USE_IMGUI
namespace Collision {

    void CheckAABB(AABB& aabb, const char* label)
    {
        if (ImGui::TreeNode(label)) {
            ImGui::SliderFloat3("min", &aabb.min.x, -1000.0f, 1000.0f);

            aabb.min.x = std::min(aabb.min.x, aabb.max.x);
            aabb.min.y = std::min(aabb.min.y, aabb.max.y);
            aabb.min.z = std::min(aabb.min.z, aabb.max.z);

            ImGui::SliderFloat3("max", &aabb.max.x, -1000, 1000.0f);

            aabb.max.x = std::max(aabb.min.x, aabb.max.x);
            aabb.max.y = std::max(aabb.min.y, aabb.max.y);
            aabb.max.z = std::max(aabb.min.z, aabb.max.z);
            ImGui::TreePop();
        }
    }

    void CheckCollider(Collider& collider, const char* label) {

        if (ImGui::TreeNode(label)) {

            //MeshType
            const char* type[] = { "Sphere", "AABB","Circle"};
            int type_current = collider.GetType();
            ImGui::Text(type[type_current]);

            //InFo
            if (ImGui::TreeNode("CollisionInfo")) {
                auto& info = collider.GetCollisionInfo();
                ImGui::Checkbox("collided", &info.collided);
                ImGui::SliderFloat3("normal", &info.normal.x, -1000.0f, 1000.0f);
                ImGui::SliderFloat("penetration", &info.penetration, -1000.0f, 1000.0f);
                ImGui::TreePop();
            }

            ImGui::Text("attribute : %X", collider.GetCollisionAttribute());
            ImGui::Text("     mask : %X", collider.GetCollisionMask());


            if (ImGui::BeginCombo("Tag", CollisionTag::GetTagName(collider.GetCollisionAttribute()).c_str())) {

                // マップ内のすべてのタグをループして選択肢を作る
                for (const auto& [name, tagNum] : CollisionTag::GetAllTags()) {

                    // 選択肢を表示（クリックされたら true を返す）
                    if (ImGui::Selectable(name.c_str(), true)) {
                        // クリックされたら切り替え関数を呼ぶ
                        collider.SetCollisionAttribute(tagNum);
                        break;
                    }
                }

                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo("MaskTag", "Select Masks...")) {

                // 現在のマスク値を取得
                uint32_t currentMask = collider.GetCollisionMask();

                // すべてのタグをループして選択肢を作成
                for (const auto& [name, tagNum] : CollisionTag::GetAllTags()) {

                    // 1. 指定したフラグ(tagNum)が現在のマスクに含まれているかチェック
                    bool isSelected = (currentMask & tagNum) != 0;

                    // 2. Selectableの表示 (isSelected を渡すことでチェックマーク等の選択状態を表示)
                    if (ImGui::Selectable(name.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {

                        if (isSelected) {
                            // 既に選択されている場合 -> フラグを削除（ビット OFF）
                            currentMask &= ~tagNum;
                        } else {
                            // 選択されていない場合 -> フラグを追加（ビット ON）
                            currentMask |= tagNum;
                        }

                        // 更新したマスクをセット
                        collider.SetCollisionMask(currentMask);
                    }
                }

                ImGui::EndCombo();
            }


            ImGui::TreePop();
        }

    };

    void CheckAllTag() {

        if (ImGui::TreeNode("AddTag")) {
            static char tagBuffer[128] = "";
            static bool isInitialized = false;

            //ファイルタグ名を入力
            ImGui::InputText("TagName", tagBuffer, IM_ARRAYSIZE(tagBuffer));

            if (CollisionTag::GetAllTags().contains(tagBuffer)) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "This tagName already exists!!!");
            } else {
                // 3. 空文字でなければ追加ボタンを有効化
                if (tagBuffer[0] != '\0') {
                    if (ImGui::Button("AddTag")) {
                        CollisionTag::AddTag(tagBuffer);
                        // 追加に成功したら入力欄をリセット
                        tagBuffer[0] = '\0';
                        //全てのタグをセットする
                        CollisionTag::SaveTagNames();
                    }
                }

            }

            ImGui::TreePop();

        }

    }


    float Distance(const Circle& p1, const Circle& p2) {
        Vector2 distance = p1.center - p2.center;
        distance.Length();
        return  distance.Length();
    }

    bool IsCollision(const Circle& c1, const Circle& c2)
    {
        //2つの急の中心点間距離を求める 
        if (Distance(c1, c2) <= c1.radius + c2.radius) {
            return true;
        }

        return false;
    }

    AABB GetAABBWorldPos(Collider* aabb)
    {
        //中心点を考慮した座標を取得してくる
        EulerTransforms transform = aabb->CalculateWorldTransform();

        Vector3 halfScale = transform.scale * 0.5f;
        AABB aabbWorld;
        aabbWorld.min = -halfScale + transform.translate;
        aabbWorld.max = halfScale + transform.translate;
        return aabbWorld;
    }

    Sphere GetSphereWorldPos(Collider* sphere)
    {
        //中心点を考慮した座標を取得してくる
        EulerTransforms transform = sphere->CalculateWorldTransform();

        return Sphere{
          .center = transform.translate,
          //Xスケールのみを半径とする
          .radius = transform.scale.x
        };
    }

    Circle GetXZCircleWorldPos(Collider* circle)
    {
        //中心点を考慮した座標を取得してくる
        EulerTransforms transform = circle->CalculateWorldTransform();

        return Circle{
            //XZ平面の円を取得する
          .center = {transform.translate.x,transform.translate.z},
          //Xスケールのみを半径とする
          .radius = transform.scale.x
        };
    }

    CollisionInfo GetCollisionInfo(const Sphere& sphere, const AABB& AABB) {

        CollisionInfo result;
        // 最近接点をAABB内から計算（クランプ）
        Vector3 closestPoint;

        closestPoint.x = std::clamp(sphere.center.x, AABB.min.x, AABB.max.x);
        closestPoint.y = std::clamp(sphere.center.y, AABB.min.y, AABB.max.y);
        closestPoint.z = std::clamp(sphere.center.z, AABB.min.z, AABB.max.z);

        // 最近接点と球の中心の距離の2乗を計算
        Vector3 difference = sphere.center - closestPoint;
        float distanceSquared = difference.Dot(difference);

        result.collided = distanceSquared <= (sphere.radius * sphere.radius);

        // 球の半径の2乗と比較
        if (!result.collided) {
            return result;
        }

        result.collided = true;

        // 2. 球の中心がAABBの外側にある場合（表面での接触）
        if (distanceSquared > 1e-6f) {
            float distance = std::sqrt(distanceSquared);
            result.normal = difference / distance; // 正規化（AABBからSphereへ向かうベクトル）
            result.penetration = sphere.radius - distance;
            return result;
        }

        // 3. 球の中心がAABBの完全に内側にある場合（深くめり込んだ場合）
        // 各面への距離を計算
        float distX_min = sphere.center.x - AABB.min.x;
        float distX_max = AABB.max.x - sphere.center.x;
        float distY_min = sphere.center.y - AABB.min.y;
        float distY_max = AABB.max.y - sphere.center.y;
        float distZ_min = sphere.center.z - AABB.min.z;
        float distZ_max = AABB.max.z - sphere.center.z;

        // 最も近い面を探す
        float minDist = distX_min;
        Vector3 normal = Vector3(-1, 0, 0); // Xマイナス面

        if (distX_max < minDist) { minDist = distX_max; normal = Vector3(1, 0, 0); }
        if (distY_min < minDist) { minDist = distY_min; normal = Vector3(0, -1, 0); }
        if (distY_max < minDist) { minDist = distY_max; normal = Vector3(0, 1, 0); }
        if (distZ_min < minDist) { minDist = distZ_min; normal = Vector3(0, 0, -1); }
        if (distZ_max < minDist) { minDist = distZ_max; normal = Vector3(0, 0, 1); }

        result.normal = normal;
        result.penetration = sphere.radius + minDist;

        return result;
    }

    CollisionInfo GetCollisionInfo(const AABB& a, const AABB& b) {

        CollisionInfo result;

        if (!IsCollision(a, b)) {
            result.collided = false;
            return result;
        }

        result.collided = true;
        //オーバーラップを調べる
        float overlapX = std::min(a.max.x - b.min.x, b.max.x - a.min.x);
        float overlapY = std::min(a.max.y - b.min.y, b.max.y - a.min.y);
        float overlapZ = std::min(a.max.z - b.min.z, b.max.z - a.min.z);

        Vector3 centerA = a.center();
        Vector3 centerB = b.center();

        //最小のオーバーラップ軸を分離する
        if (overlapX <= overlapY && overlapX <= overlapZ) {

            result.penetration = overlapX;
            result.normal = (centerA.x < centerB.x) ? Vector3(-1.0f, 0.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);

        } else if (overlapY <= overlapZ) {
            result.penetration = overlapY;
            result.normal = (centerA.y < centerB.y) ? Vector3(0.0f, -1.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
        } else {
            result.penetration = overlapZ;
            result.normal = (centerA.z < centerB.z) ? Vector3(0.0f, 0.0f, -1.0f) : Vector3(0.0f, 0.0f, 1.0f);
        }

        return result;


    }

    void ResolveCollision(Vector3& pos, Vector3& velocity, const CollisionInfo& info) {

        if (!info.collided) return;

        pos += info.normal * info.penetration;

        float normalVelocity = velocity.Dot(info.normal);

        if (normalVelocity < 0.0f) {
            velocity -= info.normal * normalVelocity;
        }
    }
}
#endif //USE_IMGUI
void CollisionManager::DebugImGui()
{
#ifdef USE_IMGUI

    ImGui::Begin("GameSystem");

    if (ImGui::TreeNode("CollisionManager")) {

        Collision::CheckAllTag();

        int i = 0;

        for (auto& collider : colliders_)
        {
            std::string label = "Collider" + std::to_string(i);

            Collision::CheckCollider(*collider, label.c_str());


            ++i;
        }



        ImGui::TreePop();
    }
    ImGui::End();

#endif
}




void CollisionManager::CheckAllCollisions() {


    //全ての衝突判定を初期化する
    for (auto& collider : colliders_) {
        collider->InitCalcuatedTisFrameFlag();
        collider->GetCollisionInfo().collided = false;
        //このフレーム内で更新をかけてみる
        collider->CalculateWorldTransform();
    }

    // リスト内のペアを総当たり
    std::list<Collider*>::iterator itrA = colliders_.begin();
    for (; itrA != colliders_.end(); ++itrA) {

        std::list<Collider*>::iterator itrB = itrA;
        for (++itrB; itrB != colliders_.end(); ++itrB) {
            // 衝突フィルタリング
            if (((*itrA)->GetCollisionAttribute() & (*itrB)->GetCollisionMask()) == 0 ||
                ((*itrB)->GetCollisionAttribute() & (*itrA)->GetCollisionMask()) == 0) {
                continue; // 衝突しない
            }

            CheckCollisionPair(*itrA, *itrB);

        }
    }

}

void CollisionManager::DebugUpdate(const int32_t cameraID)
{
#ifdef _DEBUG

    for (auto& collider : colliders_) {

        collider->Update(cameraID);
    }

#endif
}

void CollisionManager::DebugDraw()
{
#ifdef _DEBUG

    for (auto& collider : colliders_) {
        collider->Draw();
    }

#endif
}
void CollisionManager::CheckCollisionCirclePair(Collider* colliderA, Collider* colliderB)
{
    // 衝突判定
    if (IsCollision(Collision::GetXZCircleWorldPos(colliderA), Collision::GetXZCircleWorldPos(colliderB))) {
        OnCollision(colliderA, colliderB);
    }
}


void CollisionManager::CheckCollisionSpherePair(Collider* colliderA, Collider* colliderB)
{
    // 衝突判定
    if (IsCollision(Collision::GetSphereWorldPos(colliderA), Collision::GetSphereWorldPos(colliderB))) {
        OnCollision(colliderA, colliderB);
    }
}

void CollisionManager::CheckCollisionAABBPair(Collider* colliderA, Collider* colliderB)
{
    AABB worldPosA = Collision::GetAABBWorldPos(colliderA);
    AABB worldPosB = Collision::GetAABBWorldPos(colliderB);

    colliderA->SetCollisionInfo(Collision::GetCollisionInfo(worldPosA, worldPosB));
    colliderB->SetCollisionInfo(Collision::GetCollisionInfo(worldPosB, worldPosA));

    // 衝突判定
    if (colliderA->GetCollisionInfo().collided && colliderB->GetCollisionInfo().collided) {
        OnCollision(colliderA, colliderB);
    }
}

void CollisionManager::CheckCollisionSphereAABBPair(Collider* sphereC, Collider* aabbC)
{

    Sphere worldSphereC = Collision::GetSphereWorldPos(sphereC);
    AABB worldAABBC = Collision::GetAABBWorldPos(aabbC);

    CollisionInfo info = Collision::GetCollisionInfo(worldSphereC, worldAABBC);

    // 衝突判定
    if (info.collided) {
        // Sphere側にはそのままセット

        sphereC->SetCollisionInfo(info);
        // AABB側には法線を逆向きにしてセット
        CollisionInfo aabbInfo = info;
        aabbInfo.normal = { -info.normal.x, -info.normal.y, -info.normal.z };
        aabbC->SetCollisionInfo(aabbInfo);

        OnCollision(aabbC, sphereC);
    }
}

void CollisionManager::CheckCollisionPair(Collider* a, Collider* b) {

    const Collider::ColliderType typeA = a->GetType();
    const Collider::ColliderType typeB = b->GetType();


    if (typeA == Collider::kColliderType_XZ_Circle && typeB == Collider::kColliderType_XZ_Circle) {
        CheckCollisionCirclePair(a, b);
    } else if (typeA == Collider::kColliderType_Sphere && typeB == Collider::kColliderType_Sphere) {
        CheckCollisionSpherePair(a, b);
    } else if (typeA == Collider::kColliderType_Sphere && typeB == Collider::kColliderType_AABB) {
        CheckCollisionSphereAABBPair(a, b);
    } else if (typeA == Collider::kColliderType_AABB && typeB == Collider::kColliderType_Sphere) {
        CheckCollisionSphereAABBPair(b, a); // 順番に注意！
    } else if (typeA == Collider::kColliderType_AABB && typeB == Collider::kColliderType_AABB) {
        CheckCollisionAABBPair(a, b);
    }
}

void CollisionManager::OnCollision(Collider* a, Collider* b)
{
    a->OnCollision(b);
    b->OnCollision(a);
}

