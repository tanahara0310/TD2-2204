#pragma once
#include "../GameObject.h"

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

private:

   Vector2 acceleration_ = { 0.0f, 0.0f }; // 加速度ベクトル
   Vector2 velocity_ = { 0.0f, 0.0f }; // 移動ベクトル
   float maxSpeed_ = 20.0f;    // 最大速度
   float dampingPerSecond_ = 0.8f;      // 減衰率
   float moveableAreaRadius_ = 50.0f; // 移動可能エリアの半径

   // 移動
   float moveSpeed_ = 50.0f;   // 移動速度
   float moveDamping_ = 0.7f;    // 移動減衰率
   float moveMaxSpeed_ = 10.0f; // 移動最大速度

   // 突進
   float chargeSpeed_ = 5000.0f; // 突進速度
   float chargeDamping_ = 0.02f;  // 突進減衰率
   float chargeDuration_ = 0.3f; // 突進持続時間（秒）
   float chargeMaxSpeed_ = 45.0f; // 突進最大速度
   GameTimer chargeTimer_;

   // スタン
   float stunPower_ = 2000.0f; // スタン反発力
   float stunDuration_ = 0.3f; // スタン持続時間（秒）
   float stunDamping_ = 0.02f;  // スタン減衰率
   float stunMaxSpeed_ = 35.0f; // スタン最大速度
   GameTimer stunTimer_;

   std::unique_ptr<KeyConfig> keyConfig_;

private:
   /// @brief キーコンフィグの初期化
   void InitializeKeyConfig();

   /// @brief ステートマシンの初期化
   void InitializeStateMachine();

   /// @brief コライダーの初期化
   void InitializeCollider();

   void UpdateMovement();

   Vector2 GetMoveDirection() const;

   /// @brief プレイヤーの移動処理
   void Move();

   /// @brief プレイヤーの突進処理
   void Charge();

   void Stun();

   void InitializeChargeBehavior();

   void InitializeMoveBehavior();

   void InitializeStunBehavior();
};