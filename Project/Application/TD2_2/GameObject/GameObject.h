#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
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

   /// @brief 方向に応じて軸を傾ける（滑らかに補間）
   /// @param dir 方向ベクトル（x: 左右, y: 前後）
   void TiltByVelocity(const Vector2& dir);

   /// @brief 現在の軸を中心に回転を開始
   /// @param duration 回転にかける時間（秒）
   /// @param rotationCount 回転回数（デフォルト: 2.0）
   void StartRotateAroundAxis(float duration, float rotationCount = 2.0f);

   void StartShake(float intensity, float duration);

   void ChangeModelResource(const std::string& path);

   /// @brief モデルリソースを名前で登録
   /// @param name モデルの識別名
   /// @param modelPath モデルリソースパス
   void RegisterModelResource(const std::string& name, const std::string& modelPath);

   /// @brief 登録されたモデルリソースを取得
   /// @param name モデルの識別名
   /// @return モデルリソースパス（登録されていない場合は空文字列）
   std::string GetRegisteredModelPath(const std::string& name) const;

   /// @brief 登録されたモデルに切り替え
   /// @param name モデルの識別名
   void ChangeToRegisteredModel(const std::string& name);

   /// @brief 2つの登録されたモデル間でアニメーション（交互に切り替え）
   /// @param modelName1 モデルの識別名1
   /// @param modelName2 モデルの識別名2
   /// @param intervalSeconds 切り替え間隔（秒）
   /// @param loop ループするかどうか
   void StartModelSwapAnimation(const std::string& modelName1, const std::string& modelName2, 
                                 float intervalSeconds, bool loop = true);

   /// @brief モデル切り替えアニメーションを停止
   void StopModelSwapAnimation();

   /// @brief モデル切り替えアニメーションが動作中か確認
   /// @return 動作中の場合true
   bool IsModelSwapAnimationActive() const { return modelSwapTimer_.IsActive(); }

   
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

   /// @brief ベース回転を計算（傾き＋左右回転）
   /// @param dir 方向ベクトル（x: 左右, y: 前後）
   /// @return ベース回転のクォータニオン
   Quaternion CalculateBaseRotation(const Vector2& dir);

   /// @brief 回転の更新処理（Updateから呼び出す）
   virtual void UpdateRotation();

   bool UpdateShake();

   /// @brief モデル切り替えアニメーションの更新処理（Updateから呼び出す）
   void UpdateModelSwapAnimation();

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

   GameTimer shakeTimer_;

   Vector3 basePosition_ = { 0.0f, 0.0f, 0.0f };
   float shakeIntensity_ = 0.0f;

   // モデル切り替えアニメーション用
   GameTimer modelSwapTimer_;
   std::string modelName1_;
   std::string modelName2_;
   bool isShowingModel1_ = true;
   
   // モデルリソースの名前とパスのマップ
   std::unordered_map<std::string, std::string> registeredModels_;
};