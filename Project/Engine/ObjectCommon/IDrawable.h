#pragma once
#include "Engine/Graphics/Render/RenderPassType.h"
#include "Engine/Graphics/PipelineStateManager.h"

// Forward declaration
class EngineSystem;

/// @brief 描画可能オブジェクトの共通インターフェース
class IDrawable {
public:
   virtual ~IDrawable() = default;

   /// @brief 初期化処理（派生クラスでオーバーライド必須）
   /// @param engine エンジンシステムへのポインタ
   static void Initialize(EngineSystem* engine);

   /// @brief 更新処理（派生クラスでオーバーライド必須）
   virtual void Update() = 0;

   /// @brief アクティブ状態を設定
   virtual void SetActive(bool active) { isActive_ = active; }

   /// @brief アクティブ状態を取得
   virtual bool IsActive() const { return isActive_; }

   /// @brief 描画パスタイプを取得
   virtual RenderPassType GetRenderPassType() const = 0;

   /// @brief オブジェクト名を取得
   virtual const char* GetObjectName() const = 0;

   /// @brief ImGuiデバッグUI描画
   virtual bool DrawImGui() = 0;

   /// @brief 2Dオブジェクトかどうかを判定
   virtual bool Is2D() const = 0;

   /// @brief ブレンドモードを取得
   /// @return ブレンドモード（デフォルトはkBlendModeNone）
   virtual BlendMode GetBlendMode() const { return BlendMode::kBlendModeNone; }

   /// @brief エンジンシステムを取得
   /// @return 
   EngineSystem* GetEngineSystem() const;
protected:
   bool isActive_ = true;
};