#pragma once
#include "../GameObject.h"
#include "Engine/Math/Easing/EasingUtil.h"

class TitlePlayerDemo; // 前方宣言

/// @brief タイトルシーン用のデモエネミークラス（シンプル版）
class TitleEnemyDemo : public GameObject {
public:
	TitleEnemyDemo() = default;
	~TitleEnemyDemo() = default;

	/// @brief 初期化
	/// @param model モデル
	/// @param texture テクスチャ
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);

	/// @brief 更新処理
	void Update() override;

	/// @brief 描画処理
	/// @param camera カメラ
	void Draw(const ICamera* camera) override;

	/// @brief オブジェクト名を取得
	const char* GetObjectName() const override { return "TitleEnemyDemo"; }

	/// @brief 追跡ターゲットを設定
	/// @param target 追跡対象のプレイヤー
	void SetTarget(TitlePlayerDemo* target) { target_ = target; }

	/// @brief 初期位置にリセット
	void ResetToInitialPosition();

	/// @brief 追跡速度を設定
	/// @param speed 追跡速度
	void SetChaseSpeed(float speed) { chaseSpeed_ = speed; }

	/// @brief 追跡速度を取得
	float GetChaseSpeed() const { return chaseSpeed_; }

	/// @brief 移動方向を設定（1.0 = 右, -1.0 = 左）
	void SetMoveDirection(float direction) { moveDirection_ = direction; }

	/// @brief 移動方向を取得
	float GetMoveDirection() const { return moveDirection_; }

	/// @brief 追跡モードを設定
	/// @param isChasing trueなら追跡する、falseなら逃げる
	void SetChasingMode(bool isChasing) { isChasing_ = isChasing; }

	/// @brief 追跡モードを取得
	bool IsChasingMode() const { return isChasing_; }

private:
	/// @brief 回転状態
	enum class RotationState {
		Rotating,  // 回転中
		Waiting    // 待機中
	};

	Vector3 initialPosition_ = { 10.0f, 24.0f, 10.0f }; // 初期位置（背景より後ろ）
	const float kInitialSpeed_ = 15.6f; // 初期速度（定数として保持）
	float chaseSpeed_ = kInitialSpeed_; // 追跡速度
	float moveDirection_ = 1.0f; // 移動方向（1.0 = 右, -1.0 = 左）
	bool isChasing_ = true; // 追跡モードかどうか
	TitlePlayerDemo* target_ = nullptr; // 追跡対象
	
	// 回転関連
	RotationState rotationState_ = RotationState::Rotating; // 回転状態
	float rotationDuration_ = 0.4f; // 1回の回転にかける時間（秒）- 長くしてイージングを見やすく
	float rotationCount_ = 4.0f; // 回転回数
	float rotationWaitTime_ = 0.3f; // 回転後の待機時間（秒）
	float rotationTimer_ = 0.0f; // 回転タイマー
	float waitTimer_ = 0.0f; // 待機タイマー
	EasingUtil::Type rotationEasing_ = EasingUtil::Type::EaseOutCubic; // 終盤に減速が目立つイージング
	
	bool wasChasing_ = false; // 前フレームの追跡状態
	float baseRotationY_ = 0.0f; // 基本回転角度（移動方向による）
};
