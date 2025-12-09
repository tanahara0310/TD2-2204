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

	/// @brief 現在の速度を取得（加速度適用後）
	float GetCurrentSpeed() const { return currentSpeed_; }

	/// @brief 移動方向を設定（1.0 = 右, -1.0 = 左）
	void SetMoveDirection(float direction) { moveDirection_ = direction; }

	/// @brief 移動方向を取得
	float GetMoveDirection() const { return moveDirection_; }

	/// @brief 追跡モードを設定
	/// @param isChasing trueなら追跡する、falseなら逃げる
	void SetChasingMode(bool isChasing) { isChasing_ = isChasing; }

	/// @brief 追跡モードを取得
	bool IsChasingMode() const { return isChasing_; }

	/// @brief 初期位置を設定
	/// @param position 新しい初期位置
	void SetInitialPosition(const Vector3& position) { initialPosition_ = position; }

private:
	/// @brief 回転状態
	enum class RotationState {
		Rotating,  // 回転中
		Waiting    // 待機中
	};

	/// @brief 追跡モードの更新
	/// @param deltaTime デルタタイム
	void UpdateChaseMode(float deltaTime);

	/// @brief 通常移動モードの更新
	/// @param deltaTime デルタタイム
	void UpdateNormalMode(float deltaTime);

	/// @brief 回転の更新
	/// @param directionX X方向の移動方向
	/// @param deltaTime デルタタイム
	void UpdateRotation(float directionX, float deltaTime);

	/// @brief 回転中の処理
	/// @param deltaTime デルタタイム
	void ProcessRotating(float deltaTime);

	/// @brief 待機中の処理
	/// @param deltaTime デルタタイム
	void ProcessWaiting(float deltaTime);

	/// @brief 基本回転を設定
	/// @param direction 移動方向（1.0 = 右, -1.0 = 左）
	void SetBasicRotation(float direction);

	/// @brief 回転状態をリセット
	void ResetRotationState();

	Vector3 initialPosition_ = { 10.0f, 24.0f, 10.0f }; // 初期位置（背景より後ろ）
	float chaseSpeed_ = 23.4f; // 追跡速度（初期値23.4f、外部から設定可能）
	float moveDirection_ = 1.0f; // 移動方向（1.0 = 右, -1.0 = 左）
	bool isChasing_ = true; // 追跡モードかどうか
	TitlePlayerDemo* target_ = nullptr; // 追跡対象

	// 速度制御システム
	float currentSpeed_ = 23.4f; // 現在の速度（加速度適用後）
	float baseSpeed_ = 23.4f; // 基本速度
	float acceleration_ = 10.0f; // 加速度を上げる（6.0f → 10.0f）急激な加速のため
	float deceleration_ = 8.0f; // 減速度を下げる（10.0f → 8.0f）
	
	// 距離に応じた速度制御（明確な距離の段階を設定）
	float closeDistance_ = 6.0f; // 近い距離の閾値（8.0f → 6.0f）
	float farDistance_ = 30.0f; // 遠い距離の閾値をさらに広げる（25.0f → 30.0f）
	float closeSpeedMultiplier_ = 0.4f; // 近距離時の速度倍率をさらに遅く（0.5f → 0.4f）
	float farSpeedMultiplier_ = 2.0f; // 遠距離時の速度倍率を大幅に上げる（1.6f → 2.0f）
	
	// 追いつく瞬間の演出
	float catchUpDistance_ = 3.0f; // 追いつく瞬間と判定する距離（4.0f → 3.0f）
	float catchUpSpeedBoost_ = 2.5f; // 追いつく瞬間の速度ブーストをさらに上げる（2.2f → 2.5f）
	bool isCatchingUp_ = false; // 追いつく瞬間フラグ
	
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
