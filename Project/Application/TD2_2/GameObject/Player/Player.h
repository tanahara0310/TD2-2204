#pragma once
#include "../GameObject.h"
#include "../../UI/GaugeUI.h"

class LightningEffectManager;

class Player : public GameObject {
public:
   Player() = default;
   ~Player() = default;
   void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);
   void Update() override;
   void Draw(const ICamera* camera) override;

   void OnCollisionEnter(GameObject* other) override;
   void OnCollisionStay(GameObject* other) override;
   void OnCollisionExit(GameObject* other) override;

   const char* GetObjectName() const override { return "Player"; }

   void SetStartDamageFunction(const std::function<void()>& func) {
	  startDamageFunction_ = func;
   }

   void SetDamageFunction(const std::function<void()>& func) {
	  damageFunction_ = func;
   }

   void SetHitEnemyFunction(const std::function<void()>& func) {
	  hitEnemyFunction_ = func;
   }

   void SetStartChargeFunction(const std::function<void()>& func) {
	  startChargeFunction_ = func;
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

   void SetCollisionEffectFunction(const std::function<void(const Vector3&)>& func) {
	  collisionEffectFunction_ = func;
   }

   void SetExplosionEffectFunction(const std::function<void(const Vector3&)>& func) {
	  explosionEffectFunction_ = func;
   }

   void SetSmokeEffectFunction(const std::function<void(const Vector3&)>& func) {
	  smokeEffectFunction_ = func;
   }

   // 速度取得
   Vector2 GetVelocity() const { return velocity_; }

   bool IsCharging() const { return isCharging_; }

   // 衝突反発の最大値設定
   void SetMaxCollisionResponse(float v) { maxCollisionResponse_ = v; }
   float GetMaxCollisionResponse() const { return maxCollisionResponse_; }

   // HP取得
   int GetHP() const { return hp_; }
   int GetMaxHP() const { return maxHp_; }

   // HPを減らす
   void DecreaseHP(int amount) { hp_ = (std::max)(0, hp_ - amount); }

   float GetStoredEnergy() const { return storedEnergy_; }

   void SetUpdateEffectFunction(const std::function<void(const Vector3&, float)>& func) {
	  updateEffectFunction_ = func;
   }

   bool IsStunned() const {
	  return stateMachine_->GetCurrentState() == "Stun";
   }

   bool IsDamage() const {
	  return stateMachine_->GetCurrentState() == "Damage";
   }

   bool IsDespawn() const {
	  return stateMachine_->GetCurrentState() == "Despawn";
   }
private:

   Vector2 acceleration_ = { 0.0f, 0.0f }; // 加速度ベクトル
   Vector2 velocity_ = { 0.0f, 0.0f }; // 移動ベクトル
   float maxSpeed_ = 20.0f;    // 最大速度
   float dampingPerSecond_ = 0.8f;      // 減衰率
   float moveableAreaRadius_ = 50.0f; // 移動可能エリアの半径

   // 移動
   float moveSpeed_ = 50.0f;   // 移動速度
   float moveDamping_ = 0.7f;    // 移動減衰率
   float moveMaxSpeed_ = 7.0f; // 移動最大速度

   // 突進
   float chargeSpeed_ = 5000.0f; // 突進速度
   float chargeDamping_ = 0.02f;  // 突進減衰率
   float chargeDuration_ = 0.3f; // 突進持続時間（秒）
   float chargeMaxSpeed_ = 45.0f; // 突進最大速度
   GameTimer chargeTimer_;

   // スタン
   float stunPower_ = 2000.0f; // スタン反発力 (基礎)
   float stunDuration_ = 0.3f; // スタン持続時間（秒）
   float stunDamping_ = 0.02f;  // スタン減衰率
   float stunMaxSpeed_ = 35.0f; // スタン最大速度
   GameTimer stunTimer_;

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

   // 衝突反発の速度依存スケール
   float collisionResponseScale_ = 10.0f; // 速度に応じて反発力がどれだけ増えるかの係数

   // 衝突反発の最大値
   float maxCollisionResponse_ = 5000.0f;

   Vector2 direction_ = {};

   std::unique_ptr<KeyConfig> keyConfig_;

   std::function<void()> startDamageFunction_;

   std::function<void()> damageFunction_;

   std::function<void()> startEffectFunction_;

   std::function<void()> stopEffectFunction_;

   std::function<void()> hitEnemyFunction_;

   std::function<void()> startChargeFunction_;

   std::function<void(const Vector3&, float)> updateEffectFunction_;

   std::function<void(const Vector4&)> setEffectColorFunction_;

   std::function<void(const Vector3&)> collisionEffectFunction_;

   std::function<void(const Vector3&)> explosionEffectFunction_;

   std::function<void(const Vector3&)> smokeEffectFunction_;

   // 突進中フラグ
   bool isCharging_ = false;

   int hp_ = 3;
   int maxHp_ = 3;

   float storedEnergy_ = 0.0f;
   float energyScale_ = 0.3f;
   float enemyStoredEnergy_ = 0.0f;
   float maxStoredEnergy_ = 5.0f;
   float energyDecayPerSecond_ = 0.5f;

private:
   /// @brief キーコンフィグの初期化
   void InitializeKeyConfig();

   /// @brief ステートマシンの初期化
   void InitializeStateMachine();

   /// @brief コライダーの初期化
   void InitializeCollider();

   void UpdateMovement();

   Vector2 GetMoveDirection() const;

   void UpdateRotation() override;

   /// @brief プレイヤーの移動処理
   void Move();

   /// @brief プレイヤーの突進処理
   void Charge();

   void Stun();

   void Damage();

   void Despawn();

   void Respawn();

   void InitializeCharge();

   void InitializeMove();

   void InitializeStun();

   void InitializeDamage();

   void InitializeDespawn();

   void InitializeRespawn();

   void InitializePunk();

   void Punk();

   void InitializeDeath();

   void Death();

   /// @brief ダメージ壁との接触判定
   void CheckDamageWallCollision();

   void UpdateEnergy();

   /// @brief ダメージエフェクトの更新
   void UpdateEffect();
};