#include "Collider.h"
#include <Utilities/Logger/Logger.h>

namespace {
    // コライダーの描画用モデルID
     int32_t colliderCubeModelID = -1;
     int32_t colliderSphereModelID = -1;
    // コライダーの描画用テクスチャID
     int32_t colliderTextureID =-1;
}

Vector3 ParentMatrix::GetWorldTransformByMatrix(const Matrix4x4& mat)
{
    return { mat.m[3][0], mat.m[3][1], mat.m[3][2] };
}

void Collider::InitCalcuatedTisFrameFlag()
{
    isCalculatedThisFrame_ = false;
}

void Collider::Load()
{
#ifdef _DEBUG
    colliderCubeModelID = Game::Asset::Model::Load("assets/engine/model/cube/cube.obj");
    colliderSphereModelID = Game::Asset::Model::Load("assets/engine/model/sphere/sphere.obj");
    colliderTextureID = Game::Asset::Texture::Load("assets/engine/texture/white1x1.png");
#endif
}

Collider::Collider()
{
    collisionInfo_.collided = false;
    collisionInfo_.normal = { 0.0f,0.0f,0.0f };
    collisionInfo_.penetration = { 0.0f };

    sphere_ = { .center = { 0.0f,0.0f,0.0f },.radius =  1.0f};

    float size = 0.5f;
    aabb_ = { {-size ,-size ,-size }, {size,size,size} };

    isCalculatedThisFrame_ = false;

    ResetColliderType(type_);

}

Collider::~Collider()
{
}

const Vector3& Collider::CalculateWorldPos()
{
    if (isCalculatedThisFrame_) {
        //計算のスキップをする
        return tempWorldTransform_;
    }

    if (worldMat_ == nullptr) {
        Log("ワールド行列がありませんでした。");
        //仮に原点とする
        tempWorldTransform_ = { 0.0f,0.0f,0.0f };
        return tempWorldTransform_;
    }

    //センターからワールド行列を作成する
    Vector3 center{};
    if (type_ == kColliderType_AABB) {
        center = aabb_.center();
    } else {
        center = sphere_.center;
    }

    Matrix4x4 child;
    child.MakeTranslateMatrix(center);
    child = child* *worldMat_;

    //ワールド座標の取得
    tempWorldTransform_ = ParentMatrix::GetWorldTransformByMatrix(child);

    //計算終了
    isCalculatedThisFrame_ = true;

    return tempWorldTransform_;
}

void Collider::OnCollisionCollider()
{
#ifdef _DEBUG
    colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
#endif // _DEBUG

}

void Collider::SetIsDrawCollider(const bool flag)
{
#ifdef _DEBUG
        isDrawCollider_ = flag;
#endif
}

void Collider::Update(const int32_t cameraID)
{
#ifdef _DEBUG


    const Matrix4x4& viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);

    Vector3 center;
    Vector3 halfExtent;

    //一旦AABBとそれ以外で分岐する
    if (type_ == kColliderType_AABB) {
        center = aabb_.center();
        halfExtent = (aabb_.max - aabb_.min);
    } else {
        center = sphere_.center;
        halfExtent = Vector3(sphere_.radius, sphere_.radius, sphere_.radius);
    }

    Matrix4x4 colliderLocal = Matrix4x4::MakeAffineMatrix(halfExtent, Vector3(0.0f, 0.0f, 0.0f), center);
  
    Matrix4x4 colliderWorld{};

    if (worldMat_) {
        //一応？安全設計
        colliderWorld = colliderLocal** worldMat_;
    } else {
        colliderWorld = Matrix4x4::MakeIdentity4x4();
    }

    Matrix4x4 colliderWvp = colliderWorld * viewProjection;

    colliderObj_->SetCBufferData(0, ShaderType::PixelShader, &colliderColor_);
    colliderObj_->SetCBufferData(1, ShaderType::PixelShader, &colliderTextureID);
    colliderObj_->SetCBufferData(0, ShaderType::VertexShader, &colliderWvp);
    colliderObj_->SetCBufferData(1, ShaderType::VertexShader, &colliderWorld);

    //毎フレーム元に戻す
    colliderColor_ = { 1.0f,1.0f,0.0f,1.0f };


#endif // DEBUG
}

void Collider::Draw()
{
#ifdef _DEBUG

    if (isDrawCollider_) {
        colliderObj_->Draw();
    }

#endif // DEBUG
}

void Collider::ResetColliderType(const ColliderType& type)
{
#ifdef _DEBUG
    isDrawCollider_ = true;
    colliderObj_ = std::make_unique<RenderObject>();
    colliderObj_->modelID_ = (type == kColliderType_AABB) ? colliderCubeModelID : colliderSphereModelID;
    colliderObj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
    colliderObj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
    colliderObj_->psoConfig_.rasterizerID = RasterizerID::Wireframe_NoCull;
    colliderObj_->SetupFromShaders();
#endif // DEBUG

}

