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

   // HP取得
   int GetHP() const { return hp_; }
   int GetMaxHP() const { return maxHp_; }
   float GetHPRatio() const { return static_cast<float>(hp_) / static_cast<float>(maxHp_); }

   void SetStartDamageFunction(const std::function<void()>& func) {
	  startDamageFunction_ = func;
   }

   void SetStartChargeFunction(const std::function<void()>& func) {
	  startChargeFunction_ = func;
   }

   void SetUpdateEffectFunction(const std::function<void(const Vector3&)>& func) {
	  updateEffectFunction_ = func;
   }

   void SetStartEffectFunction(const std::function<void()>& func) {
	  startEffectFunction_ = func;
   }

   void SetStopEffectFunction(const std::function<void()>& func) {
	  stopEffectFunction_ = func;
   }

   void SetEffectColorFunction(const std::function<void(const Vector4&)>& func) {
	  setEffectColorFunction_ = func;
   }

   // スパークエフェクト関連
   void SetStartSparkEffectFunction(const std::function<void()>& func) {
      startSparkEffectFunction_ = func;
   }

   void SetStopSparkEffectFunction(const std::function<void()>& func) {
      stopSparkEffectFunction_ = func;
   }

   void SetUpdateSparkEffectFunction(const std::function<void(const Vector3&)>& func) {
      updateSparkEffectFunction_ = func;
   }

   void StartSparkEffect();
   void StopSparkEffect();
   void UpdateSparkEffect();

   void StartChargeFunction();

   void EndChargeFunction();

   float GetStoredEnergy() const { return storedEnergy_; }

   void DecreaseHP(int amount) { hp_ = (std::max)(0, hp_ - amount); }

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
   float collisionResponseScale_ = 10.0f;
   float stunPower_ = 2000.0f; // 基礎反発力

   // 突進フラグ
   bool isCharging_ = false;

   // スタン
   float stunDuration_ = 0.3f;      // スタン持続時間（秒）
   float stunDamping_ = 0.04f;      // スタン減衰率
   float stunMaxSpeed_ = 35.0f;     // スタン最大速度
   GameTimer stunTimer_;            // スタンタイマー

   // パンク
   float punkDuration_ = 2.0f;      // パンク持続時間（秒）
   float punkDamping_ = 0.04f;      // パンク減衰率
   float punkMaxSpeed_ = 25.0f;     // パンク最大速度
   GameTimer punkTimer_;            // パンクタイマー

   // デスポーン・リスポーン
   float despawnDuration_ = 0.5f; // デスポーン持続時間（秒）
   float respawnDuration_ = 0.5f; // リスポーン持続時間（秒）
   GameTimer despawnTimer_;
   GameTimer respawnTimer_;

   GameTimer deathTimer_;
   GameTimer idleTimer_;
   float deathDuration_ = 1.0f; // 死亡持続時間（秒）

   // ノックバックによる中心バイアス増加
   GameTimer knockbackBiasTimer_;
   float knockbackBiasMultiplier_ = 1.0f; // 1.0 = 通常, 小さいほど中心バイアスが強くなる

   // 衝突反発の最大値
   float maxCollisionResponse_ = 2000.0f;

   // ビヘイビアツリー
   std::unique_ptr<BehaviorTree> behaviorTree_;

   int hp_ = 5;
   int maxHp_ = 5;

   std::function<void()> startDamageFunction_;

   std::function<void()> startChargeFunction_;

   std::function<void(const Vector3&)> updateEffectFunction_;

   std::function<void()> stopEffectFunction_;

   std::function<void()> startEffectFunction_;

   std::function<void(const Vector4&)> setEffectColorFunction_;

   // スパークエフェクト関連
   std::function<void()> startSparkEffectFunction_;
   std::function<void()> stopSparkEffectFunction_;
   std::function<void(const Vector3&)> updateSparkEffectFunction_;

   float storedEnergy_ = 0.0f;
   float energyScale_ = 0.3f;
   float playerStoredEnergy_ = 0.0f;
   float maxStoredEnergy_ = 5.0f;
   float energyDecayPerSecond_ = 0.5f;

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

   /// @brief ダメージ処理
   void Damage();

   /// @brief ダメージ初期化
   void InitializeDamage();

   /// @brief デスポーン処理
   void Despawn();

   /// @brief デスポーン初期化
   void InitializeDespawn();

   /// @brief リスポーン処理
   void Respawn();

   /// @brief リスポーン初期化
   void InitializeRespawn();

   void InitializePunk();

   void Punk();

   void InitializeDeath();

   void Death();

   /// @brief ダメージ壁との接触判定
   void CheckDamageWallCollision();

   void UpdateEnergy();

   void UpdateEffect();
};