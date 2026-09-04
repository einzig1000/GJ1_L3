#include "Collider.h"
#include <Utilities/Logger/Logger.h>

namespace {
    // コライダーの描画用モデルID
     int32_t colliderCubeModelID = -1;
     int32_t colliderSphereModelID = -1;
     int32_t colliderCylinderModelID = -1;
    // コライダーの描画用テクスチャID
     int32_t colliderTextureID =-1;
}

Vector3 ParentMatrix::GetWorldTransformByMatrix(const Matrix4x4& mat)
{
    return { mat.m[3][0], mat.m[3][1], mat.m[3][2] };
}

Vector3 ParentMatrix::GetParentScaleByMatrix(const Matrix4x4& mat)
{
    return { mat.m[0][0], mat.m[1][1], mat.m[2][2] };
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
    colliderCylinderModelID = Game::Asset::Model::Load("assets/engine/model/cylinder/cylinder.obj");
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

const EulerTransforms& Collider::CalculateWorldTransform()
{
    if (isCalculatedThisFrame_) {
        //計算のスキップをする
        return tempWorldTransform_;
    }

    if (parentWorldMat_ == nullptr) {
        Log("ワールド行列がありませんでした。");
        //仮に原点とする
        tempWorldTransform_ = {
            .scale = {1.0f,1.0f,1.0f},
            .rotate = { 0.0f, 0.0f, 0.0f } ,
             .translate = { 0.0f, 0.0f, 0.0f }
        };

        return tempWorldTransform_;
    }

    //センターからワールド行列を作成する
    Vector3 center{};
    Vector3 halfScale{};
    if (type_ == kColliderType_AABB) {
        center = aabb_.center();
        halfScale = aabb_.max - aabb_.min;
    } else {
        center = sphere_.center;
        halfScale = { sphere_.radius,sphere_.radius,sphere_.radius };
    }

    Matrix4x4 child = Matrix4x4::MakeTranslateMatrix(center);
    child = child * *parentWorldMat_;

    //ワールド座標の取得
    tempWorldTransform_.translate = ParentMatrix::GetWorldTransformByMatrix(child);

    Matrix4x4 scaleMatrix = Matrix4x4::MakeScaleMatrix(halfScale);
    scaleMatrix = scaleMatrix * *parentWorldMat_;
    tempWorldTransform_.scale = ParentMatrix::GetParentScaleByMatrix(scaleMatrix);

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

    Vector3 halfExtent;

    //一旦AABBとそれ以外で分岐する
    if (type_ == kColliderType_AABB) {
        halfExtent = (aabb_.max - aabb_.min);
    } else {
        //表示とずれるかも
        float size = sphere_.radius*2.0f;
        halfExtent = Vector3(size, size, size);
    }
    // コライダーの中心点オフセット
    Vector3 center = (type_ == kColliderType_AABB) ? aabb_.center() : sphere_.center;

    // コライダー自身のローカルアフィン行列（サイズ拡大 + オフセット移動）
    Matrix4x4 colliderLocal = Matrix4x4::MakeAffineMatrix(halfExtent, Vector3(0.0f, 0.0f, 0.0f), center);

    Matrix4x4 colliderWorld{};

    if (parentWorldMat_) {
        // ローカルアフィン行列に直接親のワールド行列を掛ける（これで二重適用を回避）
        colliderWorld = colliderLocal * *parentWorldMat_;
    } else {
        colliderWorld = colliderLocal;
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

    uint32_t modelID = -1;
    switch (type)
    {
    case kColliderType_AABB:
        modelID = colliderCubeModelID;
        break;
    case kColliderType_Sphere:
        modelID = colliderSphereModelID;
        break;
    case kColliderType_XZ_Circle:
        modelID = colliderCylinderModelID;
        break;
    default:
        modelID = colliderCubeModelID;
        break;
    }
    colliderObj_->modelID_ = modelID;
    colliderObj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
    colliderObj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
    colliderObj_->psoConfig_.rasterizerID = RasterizerID::Wireframe_NoCull;
    colliderObj_->SetupFromShaders();
#endif // DEBUG

}

