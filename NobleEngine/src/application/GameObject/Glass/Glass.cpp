#include "Glass.h"
#include"Utilities/Json/JsonManager.h"
namespace {
    //グラス共通の変数
    float deadLine_ = 0.0f;
}

Glass::Glass()
{
    //カクテルをロードする
    SetGlassTypeAndLoadModels(GLASS_COCKTAIL);

    JsonManager::Load("assets/application/json/Glass/Glass.json", "/deadLine", deadLine_);
}

Glass::~Glass()
{
}

void Glass::Initialize()
{
    //床との当たり判定
    isHitFloor_ = false;

    //レンダーオブジェクトのインスタンス作成
    glassObj_ = std::make_unique<RenderObject>();
    //シンプルモデルのシェーダー適用
    glassObj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModels.VS.hlsl";
    glassObj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModels.PS.hlsl";
    glassObj_->SetupFromShaders();

    glassObj_->modelID_ = modelID_;
    glassObj_->instanceNum_ = instanceCount_;

    worldMatrixHeapSlot_ = Game::Resource::CreateDynamic();
    colorHeapSlot_ = Game::Resource::CreateDynamic();
    textureIndexHeapSlot_ = Game::Resource::CreateDynamic();

    transforms_.resize(instanceCount_, EulerTransforms());
    worldMatrices_.resize(instanceCount_, Matrix4x4());
    //一旦半透明にしておく
    colors_.resize(instanceCount_, Vector4(1.0f, 1.0f, 1.0f, 0.5f));
    textureIndices_.resize(instanceCount_, textureID_);

    comCollider_.CreateFromModelData(
        modelID_,
        worldMatrices_[0],
        CollisionTag::GetTag("Glass"),
        CollisionTag::GetTag("Table") | CollisionTag::GetTag("Target"));
}

void Glass::Update(const int32_t cameraID)
{
    //毎フレーム当たり判定を初期化する
    isHitFloor_ = false;

    for (int i = 0; i < instanceCount_; i++)
    {
        if (transforms_[i].translate.y <= deadLine_) {
            //一旦インスタンス1つで実行　床に衝突、つまり壊れる。
            isHitFloor_ = true;
            break;
        }
    }
    

    for (int i = 0; i < instanceCount_; i++)
    {
        worldMatrices_[i] = transforms_[i].GetWorldMatrix();
    }

    Game::Resource::UpdateData(worldMatrixHeapSlot_, worldMatrices_);
    Game::Resource::UpdateData(colorHeapSlot_, colors_);
    Game::Resource::UpdateData(textureIndexHeapSlot_, textureIndices_);

    Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
    int32_t vsHeapSlot = Game::Resource::GetSRV(worldMatrixHeapSlot_);
    Vector2uint psHeapSlot{ Game::Resource::GetSRV(colorHeapSlot_), Game::Resource::GetSRV(textureIndexHeapSlot_) };

    glassObj_->SetCBufferData(0, ShaderType::VertexShader, &viewProjection);
    glassObj_->SetCBufferData(1, ShaderType::VertexShader, &vsHeapSlot);
    glassObj_->SetCBufferData(0, ShaderType::PixelShader, &psHeapSlot);
}

void Glass::Draw()
{
    glassObj_->Draw();
}

void Glass::DrawImGui()
{
    ImGui::Begin("GameObj");



    if (ImGui::TreeNode("Glass")) {

        ImGui::Checkbox("isHitFloor", &isHitFloor_);

        for (int i = 0; i < instanceCount_; i++)
        {
            if (ImGui::TreeNode(("Instance " + std::to_string(i)).c_str()))
            {
                ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), &transforms_[i].scale.x, 0.01f);
                ImGui::DragFloat3(("Rotate##" + std::to_string(i)).c_str(), &transforms_[i].rotate.x, 0.01f);
                ImGui::DragFloat3(("Translate##" + std::to_string(i)).c_str(), &transforms_[i].translate.x, 0.01f);
                ImGui::ColorEdit4(("Color##" + std::to_string(i)).c_str(), &colors_[i].x);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();

    }


    ImGui::End();
}

void Glass::SetGlassTypeAndLoadModels(const GLASS_TYPE type)
{

    std::string filePath;
    std::string textureFilePath = "assets/engine/texture/white1x1.png";

    switch (type)
    {
    case Glass::GLASS_COCKTAIL:
        filePath = "assets/application/model/Cocktail/Cocktail.obj";
        break;
    default:
        //デフォルトはカクテル
        filePath = "assets/application/model/Cocktail/Cocktail.obj";
        break;
    }

    //モデルとテクスチャIDをセットする
    modelID_ = Game::Asset::Model::Load(filePath);
    textureID_ = Game::Asset::Texture::Load(textureFilePath);
}
