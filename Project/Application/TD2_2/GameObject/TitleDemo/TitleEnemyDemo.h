#pragma once
#include "../GameObject.h"

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
	Vector3 initialPosition_ = { 10.0f, 4.0f, -22.7f }; // 初期位置（プレイヤーの後ろ）
	float chaseSpeed_ = 11.6f; // 追跡速度（プレイヤーと同じ速度に変更）
	float moveDirection_ = 1.0f; // 移動方向（1.0 = 右, -1.0 = 左）
	bool isChasing_ = true; // 追跡モードかどうか
	TitlePlayerDemo* target_ = nullptr; // 追跡対象
};
