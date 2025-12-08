#pragma once
#include "../GameObject.h"

/// @brief スパーク当たり判定用のシンプルなGameObject
/// モデルを持たず、コライダーのみを提供する
class SparkColliderObject : public GameObject {
public:
   SparkColliderObject() = default;
   ~SparkColliderObject() = default;

   /// @brief 初期化
   /// @param radius コライダーの半径
   void Initialize(float radius);

   /// @brief 更新処理
   void Update() override;

   /// @brief 描画処理（何もしない）
   void Draw(const ICamera* camera) override { (void)camera; }

   /// @brief オブジェクト名を取得
   const char* GetObjectName() const override { return "Spark"; }

   /// @brief 位置を設定
   void SetPosition(const Vector3& position) { transform_.translate = position; }

   /// @brief スパーク当たり判定を有効化/無効化
   void SetSparkActive(bool active) { isSparkActive_ = active; }

   /// @brief スパーク当たり判定が有効かどうか
   bool IsSparkActive() const { return isSparkActive_; }

private:
   bool isSparkActive_ = false;
};
