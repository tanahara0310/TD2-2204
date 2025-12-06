#pragma once
#include "Engine/Graphics/Sprite/Sprite.h"
#include "Engine/Graphics/TextureManager.h"
#include "MathCore.h"
#include "Engine/ObjectCommon/SpriteObject.h"
#include <memory>
#include <string>

class SpriteRenderer;
class CameraManager;
class GameObject;

class GaugeUI {
public:
	GaugeUI() = default;
	~GaugeUI() = default;

	/// <summary>
	/// 初期化関数
	/// </summary>
	/// <param name="cameraManager">カメラマネージャー</param>
	std::vector<std::unique_ptr<IDrawable>> Initialize(CameraManager* cameraManager, float maxGauge);

	// 追従対象を設定
	void SetTarget(GameObject* target) { target_ = target; }

	// 外部から分割数を設定
	void SetValue(float current);

	// 毎フレーム呼ぶ
	void Update();

	// ゲージが点滅する割合指定
	void SetBlinkThreshold(float blinkThreshold) { blinkThreshold_ = blinkThreshold; }

	// 前面のゲージの色を指定
	void SetFillColor(const Vector4& color);

	// 後面のゲージの色を指定
	void SetSegmentColor(const Vector4& color);

	// 点滅の基本速度を指定
	void SetBlinkBaseSpeed(float blinkBaseSpeed) { blinkBaseSpeed_ = blinkBaseSpeed; }

private:
	// Fillを作成
	std::unique_ptr<SpriteObject> CreateFill();

	// BGを作成
	std::unique_ptr<SpriteObject> CreateBG();

	// Segmentを作成
	std::unique_ptr<SpriteObject> CreateSegment();

private:
	// ゲームシーンからSpriteを借りてくる
	SpriteObject* spriteFill_ = nullptr; // 実際のHP
	SpriteObject* spriteBG_ = nullptr;   // 背景
	SpriteObject* spriteSegment_ = nullptr; // ブロック

	CameraManager* cameraManager_ = nullptr;
	GameObject* target_ = nullptr;

	// ゲージ値管理
	float maxGauge_ = 5;
	float currentGauge_ = 0;

	// 見た目設定（ピクセル単位）
	float fullWidth_ = 120.0f; // HPゲージの最大幅（px）
	float fullHeight_ = 16.0f; // 高さ（px）

	// 分割数分増やす
	float segmentWidth_ = 0.0f;

	// Segmentが増える速度
	float segmentDecreaseSpeed_ = 2.0f;

	// 位置オフセット
	Vector2 screenOffset_ = {0.0f, -100.0f};

	// ゲージの最大サイズ
	Vector2 maxSpriteSize_ = {};

	// ゲージ1個分のサイズX
	float spriteSizeX_ = 0.0f;

	// 累積時間
	float elapsedTime_ = 0.0f;

	// ゲージが点滅する割合
	float blinkThreshold_ = 0.7f;

	// 基本点滅速度
	float blinkBaseSpeed_ = 14.0f;
};
