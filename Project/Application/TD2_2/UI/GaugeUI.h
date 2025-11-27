#pragma once
#include <memory>
#include <string>
#include "MathCore.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Graphics/Sprite/Sprite.h"

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
	/// <param name="fill">前面スプライト</param>
	/// <param name="bg">背景スプライト</param>
	/// <param name="bg">減少時スプライト</param>
	/// <param name="cameraManager">カメラマネージャー</param>
	void Initialize(Sprite* fill, Sprite* bg, CameraManager* cameraManager);

	// 追従対象を設定
	void SetTarget(GameObject* target) { target_ = target; }

	// 外部からHPを設定
	void SetHP(float current, float max);

	// 毎フレーム呼ぶ
	void Update();

	// 描画
	void Draw();

private:
	// ゲームシーンからSpriteを借りてくる
	Sprite* spriteFill_;  // 実際のHP
	Sprite* spriteBG_;    // 背景

	TextureManager::LoadedTexture handleFill_;
	TextureManager::LoadedTexture handleBG_;

	CameraManager* cameraManager_ = nullptr;
	GameObject* target_ = nullptr;

	// HP値管理
	float maxHP_ = 10.0f;
	float currentHP_ = 10.0f;

	// 見た目設定（ピクセル単位）
	float fullWidth_ = 120.0f;  // HPゲージの最大幅（px）
	float fullHeight_ = 16.0f;  // 高さ（px）

	// Afterが減る速度（HP/s）
	float afterDecreaseSpeed_ = 5.0f;

	// 位置オフセット
	Vector2 screenOffset_ = { 0.0f, -100.0f };
	float drawDepth_ = 0.0f;
};
