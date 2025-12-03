#pragma once
#include "../GameObject.h"

/// @brief 弾クラス
class Bullet : public GameObject {
public:
   enum class Type {
	  Lightning,
	  ElasticSphere,
   };

   Bullet() = default;
   ~Bullet() = default;

   /// @brief 初期化
   /// @param model モデル
   /// @param texture テクスチャ
   /// @param direction 進行方向（正規化されていなくても自動で正規化）
   void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture, const Vector3& direction);

   /// @brief 更新処理
   void Update() override;

   /// @brief 描画処理
   /// @param camera カメラ
   void Draw(const ICamera* camera) override;

   /// @brief オブジェクト名を取得
   const char* GetObjectName() const override { return "Bullet"; }
   
   /// @brief 削除可能かどうか（非アクティブな弾は自動削除）
   bool CanBeDeleted() const override { return !IsActive(); }

   /// @brief 速度を設定
   /// @param speed 速度
   void SetSpeed(float speed) { speed_ = speed; }

   /// @brief 速度を取得
   float GetSpeed() const { return speed_; }

   /// @brief 生存時間を設定
   /// @param lifetime 生存時間（秒）
   void SetLifetime(float lifetime);

private:
   Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 速度ベクトル
   float speed_ = 30.0f; // 速度
   GameTimer lifetimeTimer_; // 生存時間タイマー
   
   // 初期化時のタイマー
   GameTimer scaleUpTimer_;

   float initialScale_ = 0.0f; // 初期スケール

private:
   /// @brief コライダーの初期化
   void InitializeCollider();

   void InitializeStateMachine();

   void InitializeScaleUpState();

   void ScaleUp();

   void InitializeMoveState();

   void Move();
};
