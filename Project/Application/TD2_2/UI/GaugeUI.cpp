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

void GaugeUI::Initialize(Sprite* fill, Sprite* bg, Sprite* segment, CameraManager* cameraManager) {
	assert(cameraManager != nullptr);

	cameraManager_ = cameraManager;

	// 各スプライト作成
	spriteBG_ = bg;
	handleBG_ = TextureManager::GetInstance().Load("Resources/Textures/white.png");
	spriteFill_ = fill;
	handleFill_ = TextureManager::GetInstance().Load("Resources/Textures/white.png");
	spriteSegment_ = segment;
	handleSegment_ = TextureManager::GetInstance().Load("Resources/Textures/white.png");

	// アンカーポイント
	spriteBG_->SetAnchor({0.5f, 0.5f});      // 中央
	spriteFill_->SetAnchor({0.0f, 0.5f});    // 左寄せ
	spriteSegment_->SetAnchor({0.0f, 0.5f}); // 左寄せ

	// 色の初期化
	spriteBG_->SetColor({0.2f, 0.2f, 0.2f, 1.0f});
	spriteFill_->SetColor({0.5f, 0.5f, 0.0f, 1.0f});
	spriteSegment_->SetColor({0.8f, 1.0f, 0.0f, 1.0f});

	// 初期スケール設定
	Vector2 texBG = spriteBG_->GetTextureSize();
	maxSpriteSize_ = {fullWidth_ / texBG.x, fullHeight_ / texBG.y};

	// fullWidth_/fullHeight_を実際のピクセル幅にするためスケールを設定
	if (texBG.x > 0 && texBG.y > 0) {
		spriteBG_->SetScale({maxSpriteSize_.x, maxSpriteSize_.y, 1.0f});

		// Fill & Segment初期は0
		spriteFill_->SetScale({0.0f, maxSpriteSize_.y, 1.0f});

		// セグメントも初期は幅0にしておく
		spriteSegment_->SetScale({0.0f, maxSpriteSize_.y, 1.0f});
	}

	// 初期ゲージ値
	maxGauge_ = 5;
	currentGauge_ = 0;

	// ゲージ1個分のサイズ
	spriteSizeX_ = maxSpriteSize_.x / maxGauge_;
}

void GaugeUI::SetValue(int current) {
	if (current > maxGauge_)
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

	float screenX = (normalized.x + 1.0f) * 0.5f * static_cast<float>(WinApp::kClientWidth);
	float screenY = (-normalized.y + 1.0f) * 0.5f * static_cast<float>(WinApp::kClientHeight);

	screenX += screenOffset_.x;
	screenY += screenOffset_.y;

	// 左端xを計算（BGの中央を基点として左端を求める）
	float leftX = screenX - (fullWidth_ * 0.5f);

	// BGは中央に置く
	spriteBG_->SetPosition({screenX, screenY, drawDepth_});

	// 若干の補正としてYは中央合わせ
	spriteFill_->SetPosition({leftX, screenY, drawDepth_ + 0.0f});
    spriteSegment_->SetPosition({leftX, screenY, drawDepth_ + 0.0f});

	// スケール更新
	spriteFill_->SetScale({spriteSizeX_ * currentGauge_, maxSpriteSize_.y, 1.0f});

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
	spriteSegment_->SetScale({segmentWidth_, maxSpriteSize_.y, 1.0f});
}

void GaugeUI::Draw() {
	if (spriteBG_)
		spriteBG_->Draw(handleBG_.gpuHandle);
	if (spriteFill_)
		spriteFill_->Draw(handleFill_.gpuHandle);
	if (spriteSegment_)
		spriteSegment_->Draw(handleSegment_.gpuHandle);
}