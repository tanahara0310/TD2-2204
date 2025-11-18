#pragma once

#include "Object3D.h"
#include <memory>

/// @brief Terrainモデルオブジェクト
class TerrainObject : public Object3d {
public:
    /// @brief 初期化処理
    /// @param engine エンジンシステムへのポインタ
    void Initialize(EngineSystem* engine) override;

    /// @brief オブジェクト名を取得
    /// @return オブジェクト名
    const char* GetObjectName() const override { return "Terrain"; }
};
