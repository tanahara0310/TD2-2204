#pragma once
#include "../GameObject.h"

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

private:

   Vector2 acceleration_ = { 0.0f, 0.0f }; // 加速度ベクトル
   Vector2 velocity_ = { 0.0f, 0.0f }; // 移動ベクトル
   float maxSpeed_ = 20.0f;    // 最大速度
   float dampingPerSecond_ = 0.8f;      // 減衰率

   // 移動
   float moveSpeed_ = 1.0f;   // 移動速度
   float moveDamping_ = 0.85f;    // 移動減衰率
   float moveMaxSpeed_ = 1.0f; // 移動最大速度

   // 突進
   float chargeSpeed_ = 50000.0f; // 突進速度
   float chargeDamping_ = 0.02f;  // 突進減衰率
   float chargeDuration_ = 0.3f; // 突進持続時間（秒）
   float chargeMaxSpeed_ = 45.0f; // 突進最大速度
   GameTimer chargeTimer_;

private:
   /// @brief コライダーの初期化
   void InitializeCollider();

   void UpdateMovement();

   void Move();

   void Charge();
};