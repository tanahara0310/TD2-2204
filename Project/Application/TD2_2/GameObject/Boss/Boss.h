#pragma once
#include "../GameObject.h"
#include "Application/TD2_2/AI/BehaviorTree/BehaviorTree.h"
#include <memory>

// 前方宣言
class Player;

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

   // ======================================================================
   // ビヘイビアツリー関連
   // ======================================================================
   
   /// @brief ビヘイビアツリーを設定
   void SetBehaviorTree(std::unique_ptr<BehaviorTree> tree);
   
   /// @brief ビヘイビアツリーを取得
   BehaviorTree* GetBehaviorTree() const { return behaviorTree_.get(); }
   
   /// @brief プレイヤーへの参照を設定
   void SetPlayer(Player* player) { player_ = player; }
   
   /// @brief プレイヤーへの参照を取得
   Player* GetPlayer() const { return player_; }

   // ======================================================================
   // アクションノードから呼び出すための公開メソッド
   // ======================================================================
   
   /// @brief 加速度を追加
   void AddAcceleration(const Vector2& accel);
   
   /// @brief 速度を設定
   void SetVelocity(const Vector2& vel);
   
   /// @brief 速度を取得
   Vector2 GetVelocity() const { return velocity_; }
   
   /// @brief 最大速度を設定
   void SetMaxSpeed(float maxSpeed);
   
   /// @brief 減衰率を設定
   void SetDamping(float damping);
   
   /// @brief 移動パラメータをリセット
   void ResetMovementParameters();
   
   /// @brief プレイヤーへの距離を取得
   float GetDistanceToPlayer() const;
   
   /// @brief プレイヤーへの方向ベクトルを取得（正規化済み）
   Vector3 GetDirectionToPlayer() const;
   
   /// @brief プレイヤーへの角度を取得（度数法）
   float GetAngleToPlayer() const;

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
   
   // ビヘイビアツリー
   std::unique_ptr<BehaviorTree> behaviorTree_;
   Player* player_ = nullptr;  // プレイヤーへの参照（ポインタのみ、所有権なし）

private:
   /// @brief コライダーの初期化
   void InitializeCollider();

   void UpdateMovement();

   void Move();

   void Charge();
};