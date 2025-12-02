#pragma once
#include "Engine/Graphics/Sprite/Sprite.h"
#include "Engine/Graphics/TextureManager.h"
#include "MathCore.h"
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
	/// <param name="fill">前面スプライト</param>
	/// <param name="bg">背景スプライト</param>
	/// <param name="segment">セグメントスプライト</param>
	/// <param name="cameraManager">カメラマネージャー</param>
	/// <param name="divisions">ゲージの分割数</param>
	void Initialize(Sprite* fill, Sprite* bg, Sprite* segment, CameraManager* cameraManager);

	// 追従対象を設定
	void SetTarget(GameObject* target) { target_ = target; }

	// 外部から分割数を設定
	void SetValue(int current);

	// 毎フレーム呼ぶ
	void Update();

	// 描画
	void Draw();

private:
	// ゲームシーンからSpriteを借りてくる
	Sprite* spriteFill_ = nullptr;    // 実際のHP
	Sprite* spriteBG_ = nullptr;      // 背景
	Sprite* spriteSegment_ = nullptr; // ブロック

	TextureManager::LoadedTexture handleFill_;
	TextureManager::LoadedTexture handleBG_;
	TextureManager::LoadedTexture handleSegment_;

	CameraManager* cameraManager_ = nullptr;
	GameObject* target_ = nullptr;

	// ゲージ値管理
	int maxGauge_ = 5;
	int currentGauge_ = 0;

	// 見た目設定（ピクセル単位）
	float fullWidth_ = 120.0f; // HPゲージの最大幅（px）
	float fullHeight_ = 16.0f; // 高さ（px）

	// 分割数分増やす
	float segmentWidth_ = 0.0f;

	// Segmentが増える速度
	float segmentDecreaseSpeed_ = 10.0f;

	// 位置オフセット
	Vector2 screenOffset_ = {0.0f, -100.0f};
	float drawDepth_ = 0.0f;

	// ゲージの最大サイズ
	Vector2 maxSpriteSize_ = {};

	// ゲージ1個分のサイズX
	float spriteSizeX_ = 0.0f;
};
