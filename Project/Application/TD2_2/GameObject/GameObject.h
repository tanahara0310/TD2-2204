#pragma once
#include <memory>
#include <vector>
#include "MathCore.h"
#include "Object3D.h"
#include "../Collider/Collider.h"
#include "../Collider/AABBCollider.h"
#include "../Collider/SphereCollider.h"
#include "EngineSystem.h"
#include "BehaviorRequestManager.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Utility/Timer/GameTimer.h"

class GameObject :public Object3d {
public:

   /// @brief 更新処理
   virtual void Update() override {}

   /// @brief ワールド座標での位置を取得
   /// @return ワールド座標位置
   Vector3 GetWorldPosition() const { return transform_.GetWorldPosition(); }

   /// @brief ワールド座標での位置を設定
   /// @param position ワールド座標位置
   void SetWorldPosition(const Vector3& position) { transform_.translate = position; }

   /// @brief 衝突開始時の処理
   /// @param other 衝突相手のゲームオブジェクト
   virtual void OnCollisionEnter(GameObject* other) { (void)other; }

   /// @brief 衝突継続時の処理
   /// @param other 衝突相手のゲームオブジェクト
   virtual void OnCollisionStay(GameObject* other) { (void)other; }

   /// @brief 衝突終了時の処理
   /// @param other 衝突相手のゲームオブジェクト
   virtual void OnCollisionExit(GameObject* other) { (void)other; }

   /// @brief コライダーを取得
   /// @return コライダーポインタ
   Collider* GetCollider() const { return collider_.get(); }

protected:

   std::unique_ptr<Collider> collider_;
   std::unique_ptr<BehaviorRequestManager>  behavior_;

protected:

   /// @brief 初期化処理
   /// @param model モデル
   /// @param texture テクスチャ
   void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);

   /// @brief コライダーを取り付ける
   /// @param collider コライダー
   void AttachCollider(std::unique_ptr<Collider> collider);

   /// @brief 行動リクエストマネージャーを取り付ける
   void AttachBehaviorRequestManager();
};