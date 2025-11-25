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
    
    /// @brief 描画（2Dカメラを使用）
    /// @param camera カメラオブジェクト（2D用カメラ）
    void Draw(const ICamera* camera) override { 
        Draw2D(camera);
    }
    
    /// @brief 描画（2D専用 - カメラ対応）
    /// @param camera 2D用カメラ
    virtual void Draw2D(const ICamera* camera) = 0;
    
    /// @brief ブレンドモードを取得（2Dオブジェクトはデフォルトでアルファブレンド）
    BlendMode GetBlendMode() const override { return BlendMode::kBlendModeNormal; }
    
    /// @brief トランスフォームを取得
    EulerTransform& GetTransform() { return transform_; }
    const EulerTransform& GetTransform() const { return transform_; }
    
protected:
    EulerTransform transform_;
};
