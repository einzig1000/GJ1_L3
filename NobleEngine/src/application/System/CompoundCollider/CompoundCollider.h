#pragma once

#include<vector>
#include<memory>
#include "../../../engine/EngineDefinition/EngineDefinition.h"
#include"../Collider/Collider.h"
namespace Collision {

    struct CompoundCollider {
        //モデルデータからコライダーを作るよ
        std::vector <std::unique_ptr<Collider>> colliders;
        /// @brief モデルデータから複合コライダーを作成する。
        /// @param model
        /// @param mat ワールド行列
        /// @param attribute 属性
        /// @param mask マスク属性
        void CreateFromModelData(
            const int32_t modelID,
            Matrix4x4& mat,
            const uint32_t attribute,
            const uint32_t mask);
    };
};
