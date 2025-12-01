#pragma once
#include "../GameObject.h"
#include "Application/TD2_2/AI/BehaviorTree/BehaviorTree.h"
#include <memory>

class Boss : public GameObject {
public:
   Boss() = default;
   ~Boss() = default;
   void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);
   void Update() override;
   void Draw(const ICamera* camera) override;
   bool DrawImGui() override;
   
   void OnCollisionEnter(GameObject* other) override;
   void OnCollisionStay(GameObject* other) override;
   void OnCollisionExit(GameObject* other) override;

   const char* GetObjectName() const override { return "Boss"; }

   void SetAcceleration(const Vector2& accel) { acceleration_ = accel; }
   void SetVelocity(const Vector2& vel) { velocity_ = vel; }
   void SetMaxSpeed(float maxSpeed) { maxSpeed_ = maxSpeed; }
   void SetDampingPerSecond(float damping) { dampingPerSecond_ = damping; }
   void SetMoveableAreaRadius(float radius) { moveableAreaRadius_ = radius; }
   void SetDirection(const Vector2& dir) { direction_ = dir; }

   void AddAcceleration(const Vector2& accel) { acceleration_ += accel; }
   void AddVelocity(const Vector2& vel) { velocity_ += vel; }

   float GetMoveableAreaRadius() const { return moveableAreaRadius_; }

   Vector2 GetVelocity() const { return velocity_; }
   Vector2 GetDirection() const { return direction_; }

   void SetIsCharging(bool f) { isCharging_ = f; }
   bool IsCharging() const { return isCharging_; }

   // ノックバックによる中心バイアス増加を開始
   void StartKnockbackBias(float duration, float multiplier);
   float GetMoveBiasMultiplier() const { return knockbackBiasMultiplier_; }

   // 衝突反発の最大値設定
   void SetMaxCollisionResponse(float v) { maxCollisionResponse_ = v; }
   float GetMaxCollisionResponse() const { return maxCollisionResponse_; }

   //======================================================================
   // ビヘイビアツリー関連
   //======================================================================
   
   /// @brief ビヘイビアツリーを設定
   void SetBehaviorTree(std::unique_ptr<BehaviorTree> tree);
   
   /// @brief ビヘイビアツリーを取得
   BehaviorTree* GetBehaviorTree() const { return behaviorTree_.get(); }

private:

   Vector2 acceleration_ = { 0.0f, 0.0f }; // 加速度ベクトル
   Vector2 velocity_ = { 0.0f, 0.0f }; // 移動ベクトル
   float maxSpeed_ = 20.0f;    // 最大速度
   float dampingPerSecond_ = 0.8f;      // 減衰率
   float moveableAreaRadius_ = 50.0f; // 移動可能エリアの半径

   Vector2 direction_ = {};
   
   // 衝突反発の速度依存スケール
   float collisionResponseScale_ = 50.0f;
   float stunPower_ = 2000.0f; // 基礎反発力

   // 突進フラグ
   bool isCharging_ = false;

   // スタン
   float stunDuration_ = 0.3f;      // スタン持続時間（秒）
   float stunDamping_ = 0.02f;      // スタン減衰率
   float stunMaxSpeed_ = 35.0f;     // スタン最大速度
   GameTimer stunTimer_;            // スタンタイマー

   // ノックバックによる中心バイアス増加
   GameTimer knockbackBiasTimer_;
   float knockbackBiasMultiplier_ = 1.0f; // 1.0 = 通常, 小さいほど中心バイアスが強くなる

   // 衝突反発の最大値
   float maxCollisionResponse_ = 2000.0f;
   
   // ビヘイビアツリー
   std::unique_ptr<BehaviorTree> behaviorTree_;

private:
   /// @brief コライダーの初期化
   void InitializeCollider();

   /// @brief ステートマシンの初期化
   void InitializeStateMachine();

   void UpdateMovement();

   void UpdateRotation() override;

   /// @brief スタン処理
   void Stun();

   /// @brief スタン初期化
   void InitializeStun();

   /// @brief 通常状態の初期化
   void InitializeNormal();

   /// @brief 通常状態の処理
   void Normal();
};