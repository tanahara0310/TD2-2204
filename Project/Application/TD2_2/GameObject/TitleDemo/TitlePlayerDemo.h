#pragma once
#include "../GameObject.h"

/// @brief タイトルシーン用のデモプレイヤークラス（シンプル版）
class TitlePlayerDemo : public GameObject {
public:
	TitlePlayerDemo() = default;
	~TitlePlayerDemo() = default;

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
	const char* GetObjectName() const override { return "TitlePlayerDemo"; }

	/// @brief 初期位置にリセット
	void ResetToInitialPosition();

	/// @brief 移動速度を設定
	/// @param speed 移動速度
	void SetMoveSpeed(float speed) { moveSpeed_ = speed; }

	/// @brief 移動速度を取得
	float GetMoveSpeed() const { return moveSpeed_; }

	/// @brief 移動方向を設定（1.0 = 右, -1.0 = 左）
	void SetMoveDirection(float direction) { moveDirection_ = direction; }

	/// @brief 移動方向を取得
	float GetMoveDirection() const { return moveDirection_; }

	/// @brief 追跡モードを設定
	/// @param isChasing trueなら追跡する、falseなら逃げる
	void SetChasingMode(bool isChasing) { isChasing_ = isChasing; }

	/// @brief 追跡モードを取得
	bool IsChasingMode() const { return isChasing_; }

	/// @brief 追跡ターゲットを設定
	/// @param target 追跡対象
	void SetTarget(GameObject* target) { target_ = target; }

private:
	Vector3 initialPosition_ = { 20.0f, 4.0f, -22.7f }; // 初期位置（右側）
	float moveSpeed_ = 11.6f; // 移動速度
	float moveDirection_ = -1.0f; // 移動方向（1.0 = 右, -1.0 = 左）最初は左に移動
	bool isChasing_ = false; // 追跡モードかどうか
	GameObject* target_ = nullptr; // 追跡対象
	float rotationSpeed_ = 3.0f; // Y軸回転速度（ラジアン/秒）
	bool wasChasing_ = false; // 前フレームの追跡状態
	float accumulatedRotation_ = 0.0f; // 累積回転角
};
