#pragma once

#include "IDrawable.h"
#include "MathCore.h"
#include <memory>

// 前方宣言
class EngineSystem;
class ICamera;

/// @brief 2Dオブジェクトの基底クラス
class Object2d : public IDrawable {
public:
    virtual ~Object2d() = default;
    
    /// @brief 更新
    virtual void Update() {}
    
    /// @brief 描画（2Dオブジェクトはカメラを使用しない）
    /// @param camera カメラオブジェクト（2Dでは未使用）
    void Draw(const ICamera* camera) override { 
        (void)camera; // 未使用警告を回避
        Draw(); 
    }
    
    /// @brief 描画（2D専用 - カメラとライト不要）
    virtual void Draw() = 0;
    
    /// @brief ブレンドモードを取得（2Dオブジェクトはデフォルトでアルファブレンド）
    BlendMode GetBlendMode() const override { return BlendMode::kBlendModeNormal; }
    
    /// @brief トランスフォームを取得
    EulerTransform& GetTransform() { return transform_; }
    const EulerTransform& GetTransform() const { return transform_; }
    
protected:
    EulerTransform transform_;
};
