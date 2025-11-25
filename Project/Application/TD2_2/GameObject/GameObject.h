#pragma once
#include <memory>
#include <vector>
#include "MathCore.h"
#include "Object3D.h"
#include "../Collider/Collider.h"
#include "../Collider/AABBCollider.h"
#include "../Collider/SphereCollider.h"
#include "EngineSystem.h"
#include "../Utility/StateMachine.h"
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
   std::unique_ptr<StateMachine> stateMachine_;

protected:

   /// @brief 初期化処理
   /// @param model モデル
   /// @param texture テクスチャ
   void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);

   /// @brief コライダーを取り付ける
   /// @param collider コライダー
   void AttachCollider(std::unique_ptr<Collider> collider);

   /// @brief ステートマシンを取り付ける
   void AttachStateMachine();

   /// @brief 方向に応じて軸を傾ける（滑らかに補間）
   /// @param dir 方向ベクトル（x: 左右, y: 前後）
   void TiltByVelocity(const Vector2& dir);

   /// @brief ベース回転を計算（傾き＋左右回転）
   /// @param dir 方向ベクトル（x: 左右, y: 前後）
   /// @return ベース回転のクォータニオン
   Quaternion CalculateBaseRotation(const Vector2& dir);

   /// @brief 現在の軸を中心に回転を開始
   /// @param duration 回転にかける時間（秒）
   /// @param rotationCount 回転回数（デフォルト: 2.0）
   void StartRotateAroundAxis(float duration, float rotationCount = 2.0f);

   /// @brief 回転の更新処理（Updateから呼び出す）
   virtual void UpdateRotation();

private:
   std::unique_ptr<GameTimer> rotationTimer_;
   Vector3 rotationAxis_ = { 0.0f, 1.0f, 0.0f }; // 回転軸
   float rotationCount_ = 2.0f; // 回転回数
   Quaternion rotationStartQuaternion_ = { 0.0f, 0.0f, 0.0f, 1.0f }; // 回転開始時のベースクォータニオン
   bool isRotationActive_ = false; // 回転が進行中かどうか

   // 滑らかな補間用
   Vector2 currentDir_ = { 0.0f, 0.0f }; // 現在の方向ベクトル
   Vector2 targetDir_ = { 0.0f, 0.0f }; // 目標の方向ベクトル
   float dirLerpSpeed_ = 10.0f; // 補間速度（大きいほど速く追従）
};