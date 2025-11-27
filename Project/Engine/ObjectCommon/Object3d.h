#pragma once

#include <memory>
#include <d3d12.h>
#include <vector>

#include "IDrawable.h"
#include "Graphics/Render/RenderPassType.h"
#include "Graphics/TextureManager.h"
#include "Graphics/LineRenderer.h"
#include "Graphics/Model/Model.h"
#include "WorldTransfom/WorldTransform.h"
#include "Engine/Collider/Collider.h"

// 前方宣言
class ICamera;
class EngineSystem;

/// @brief 3Dゲームオブジェクトの基底クラス
class Object3d : public IDrawable {
public:
   /// @brief デフォルトコンストラクタ
   Object3d() = default;

   /// @brief 仮想デストラクタ
   virtual ~Object3d() = default;

   /// @brief 更新処理（派生クラスでオーバーライド可能）
   /// @note IDrawable::Update を正しくオーバーライド
   virtual void Update() override;

   /// @brief 描画処理（3D専用 - カメラ必須）
   /// @param camera カメラ
   virtual void Draw(const ICamera* camera) override;

   /// @brief デバッグ描画処理（派生クラスでオーバーライド可能）
   /// @param outLines ライン配列の出力先
   virtual void DrawDebug(std::vector<LineRenderer::Line>& outLines);

   /// @brief ImGuiデバッグUI描画（派生クラスでオーバーライド可能）
   /// @return ImGuiで変更があった場合true
   bool DrawImGui() override;

   /// @brief ImGui拡張UI描画（派生クラスでオーバーライドして特殊機能を追加）
   /// @return ImGuiで変更があった場合true
   virtual bool DrawImGuiExtended();

   /// @brief オブジェクト名を取得（派生クラスでオーバーライド推奨）
   /// @return オブジェクト名
   const char* GetObjectName() const override { return "Object3D"; }

   /// @brief トランスフォームを取得
   /// @return トランスフォーム参照
   WorldTransform& GetTransform() { return transform_; }

   /// @brief トランスフォームを取得（const版）
   /// @return トランスフォーム参照
   const WorldTransform& GetTransform() const { return transform_; }

   /// @brief モデルを取得
   /// @return モデルポインタ
   Model* GetModel() { return model_.get(); }

   /// @brief モデルを取得（const版）
   /// @return モデルポインタ
   const Model* GetModel() const { return model_.get(); }

   /// @brief モデルリソースを変更（既存のModelインスタンスを再初期化）
   /// @param resource 新しいModelResourceのポインタ
   void ChangeModelResource(ModelResource* resource);

   /// @brief このオブジェクトの描画タイプを取得
   /// @return 描画タイプ（モデルがない場合はNormal）
   Model::RenderType GetRenderType() const {
      return model_ ? model_->GetRenderType() : Model::RenderType::Normal;
   }

   /// @brief このオブジェクトの描画パスタイプを取得
   /// @return 描画パスタイプ
   RenderPassType GetRenderPassType() const override;

   /// @brief ブレンドモードを取得
   /// @return ブレンドモード
   BlendMode GetBlendMode() const override { return blendMode_; }

   /// @brief ブレンドモードを設定
   /// @param blendMode 設定するブレンドモード
   void SetBlendMode(BlendMode blendMode) override { blendMode_ = blendMode; }

   /// @brief コライダーを取得
   /// @return コライダーポインタ（未設定の場合nullptr）
   Collider* GetCollider() const { return collider_.get(); }

   /// @brief コライダーを登録
   /// @param collider 登録するコライダー
   void AttachCollider(std::unique_ptr<Collider> collider) {
      collider_ = std::move(collider);
   }

   /// @brief 衝突時のコールバック（派生クラスでオーバーライド可能）
   /// @param other 衝突相手のオブジェクト
   virtual void OnCollisionEnter(Object3d* other) { (void)other; }

   /// @brief 衝突中のコールバック（派生クラスでオーバーライド可能）
   /// @param other 衝突相手のオブジェクト
   virtual void OnCollisionStay(Object3d* other) { (void)other; }

   /// @brief 衝突終了時のコールバック（派生クラスでオーバーライド可能）
   /// @param other 衝突相手のオブジェクト
   virtual void OnCollisionExit(Object3d* other) { (void)other; }

protected:
   /// @brief モデルインスタンス
   std::unique_ptr<Model> model_;

   /// @brief ワールドトランスフォーム
   WorldTransform transform_;

   /// @brief テクスチャハンドル（派生クラスで使用可能）
   TextureManager::LoadedTexture texture_;

   /// @brief ブレンドモード
   BlendMode blendMode_ = BlendMode::kBlendModeNone;

   /// @brief コライダー
   std::unique_ptr<Collider> collider_;
};
