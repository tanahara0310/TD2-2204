#define NOMINMAX
#include "GaugeUI.h"
#include "Application/TD2_2/GameObject/GameObject.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Application/TD2_2/Utility/KeyConfig.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Graphics/Render/Sprite/SpriteRenderer.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/WinApp/WinApp.h"
#include "MathCore.h"
#include <cassert>
#include <cmath>

using namespace MathCore;

std::vector<std::unique_ptr<IDrawable>> GaugeUI::Initialize(CameraManager* cameraManager) {
	assert(cameraManager != nullptr);

	std::vector<std::unique_ptr<IDrawable>> sprites;

	cameraManager_ = cameraManager;

	// BGを作成
	{
		auto bg = CreateBG();
		spriteBG_ = bg.get();
		sprites.push_back(std::move(bg));
	}

	// Fillを作成
	{
		auto fill = CreateFill();
		spriteFill_ = fill.get();
		sprites.push_back(std::move(fill));
	}

	// Segmentを作成
	{
		auto segment = CreateSegment();
		spriteSegment_ = segment.get();
		sprites.push_back(std::move(segment));
	}

	// 初期スケール設定
	Vector2 texBG = spriteBG_->GetTextureSize();
	maxSpriteSize_ = {fullWidth_ / texBG.x, fullHeight_ / texBG.y};

	// ゲージ1個分のサイズ
	spriteSizeX_ = maxSpriteSize_.x / maxGauge_;

	// fullWidth_/fullHeight_を実際のピクセル幅にするためスケールを設定
	if (texBG.x > 0 && texBG.y > 0) {
		spriteBG_->GetTransform().scale = {maxSpriteSize_.x, maxSpriteSize_.y, 1.0f};

		// Fill & Segment初期は0
		spriteFill_->GetTransform().scale = {0.0f, maxSpriteSize_.y, 1.0f};

		// セグメントも初期は幅0にしておく
		spriteSegment_->GetTransform().scale = {0.0f, maxSpriteSize_.y, 1.0f};
	}

	// 初期ゲージ値
	maxGauge_ = 5;
	currentGauge_ = 0;

	return sprites;
}

void GaugeUI::SetValue(int current) {
	if (current > maxGauge_ || current < 0)
		return;

	// ゲージの値更新
	currentGauge_ = current;
}

void GaugeUI::Update() {
	if (!spriteBG_ || !spriteFill_ || !cameraManager_ || !target_)
		return;

	ICamera* cam = cameraManager_->GetActiveCamera(CameraType::Camera3D);
	if (!cam)
		return;

	// ワールド->スクリーン位置
	Vector3 worldPos = target_->GetWorldPosition();
	const Matrix4x4& view = cam->GetViewMatrix();
	const Matrix4x4& proj = cam->GetProjectionMatrix();

	Vector2 normalized = Coordinate::WorldToNormalizedScreen(worldPos, view, proj, static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight));

	// NDC (normalized) のまま中心基準に変換
	float halfW = static_cast<float>(WinApp::kClientWidth) * 0.5f;
	float halfH = static_cast<float>(WinApp::kClientHeight) * 0.5f;

	// 中心原点座標（+Y 上向き）を計算
	float centerX = normalized.x * halfW;
	float centerY = normalized.y * halfH;

	// オフセット（ピクセル単位）を中心基準で加える
	centerX += screenOffset_.x;
	centerY += screenOffset_.y;

	// 左端を求めるときも中心基準で計算する
	float leftX_center = centerX - (fullWidth_ * 0.5f);

	// BG は中心基準位置に配置（アンカーに応じて見た目が変わる）
	spriteBG_->GetTransform().translate = {centerX, centerY, 0.0f};

	// Fill / Segment は左端基準にしたいので leftX を渡す（アンカーが左中央なら正しく表示される）
	spriteFill_->GetTransform().translate = {leftX_center, centerY, 0.0f};
	spriteSegment_->GetTransform().translate = {leftX_center, centerY, 0.0f};

	// スケール更新
	spriteFill_->GetTransform().scale = {spriteSizeX_ * currentGauge_, maxSpriteSize_.y, 0.0f};

	// セグメントのスケールを徐々に増やす
	float targetSegmentWidth = segmentWidth_;
	if (segmentWidth_ < spriteSizeX_ * currentGauge_) {
		targetSegmentWidth += segmentDecreaseSpeed_ * GameUtils::GetDeltaTime();
		if (targetSegmentWidth > spriteSizeX_ * currentGauge_) {
			targetSegmentWidth = spriteSizeX_ * currentGauge_;
		}
	} else {
		targetSegmentWidth = spriteSizeX_ * currentGauge_;
	}
	segmentWidth_ = targetSegmentWidth;
	spriteSegment_->GetTransform().scale = {segmentWidth_, maxSpriteSize_.y, 1.0f};
}

std::unique_ptr<SpriteObject> GaugeUI::CreateFill() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/Textures/white.png");
	sprite->GetTransform().translate = {0.0f, 0.0f, 0.0f};
	sprite->SetColor({0.5f, 0.5f, 0.0f, 1.0f});
	sprite->SetAnchor({0.0f, 0.5f});
	return sprite;
}

std::unique_ptr<SpriteObject> GaugeUI::CreateBG() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/Textures/white.png");
	sprite->GetTransform().translate = {0.0f, 0.0f, 0.0f};
	sprite->SetColor({0.2f, 0.2f, 0.2f, 1.0f});
	sprite->SetAnchor({0.5f, 0.5f});

	return sprite;
}

std::unique_ptr<SpriteObject> GaugeUI::CreateSegment() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/Textures/white.png");
	sprite->GetTransform().translate = {0.0f, 0.0f, 0.0f};
	sprite->SetColor({0.8f, 1.0f, 0.0f, 1.0f});
	sprite->SetAnchor({0.0f, 0.5f});

	return sprite;
}