#define NOMINMAX
#include "GaugeUI.h"
#include "Application/TD2_2/GameObject/GameObject.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Graphics/Render/Sprite/SpriteRenderer.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/WinApp/WinApp.h"
#include "MathCore.h"
#include "Application/TD2_2/Utility/KeyConfig.h"
#include <cassert>

using namespace MathCore;

void GaugeUI::Initialize(Sprite* fill, Sprite* bg, CameraManager* cameraManager) {
	assert(cameraManager != nullptr);

	cameraManager_ = cameraManager;

	// 各スプライト作成
	spriteBG_ = bg;
	handleBG_ = TextureManager::GetInstance().Load("Resources/Textures/white.png");

	spriteFill_ = fill;
	handleFill_ = TextureManager::GetInstance().Load("Resources/Textures/white.png");

	// BGは中央、Fill/Afterは左寄せ
	spriteBG_->SetAnchor({0.5f, 0.5f});
	spriteFill_->SetAnchor({0.0f, 0.5f});

	// 初期スケール設定
	Vector2 texBG = spriteBG_->GetTextureSize();
	Vector2 texFill = spriteFill_->GetTextureSize();

	// fullWidth_/fullHeight_を実際のピクセル幅にするためスケールを設定
	if (texBG.x > 0 && texBG.y > 0) {
		spriteBG_->SetScale({fullWidth_ / texBG.x, fullHeight_ / texBG.y, 1.0f});
	}
	if (texFill.x > 0 && texFill.y > 0) {
		// Fill & After 初期は満タン
		spriteFill_->SetScale({fullWidth_ / texFill.x, fullHeight_ / texFill.y, 1.0f});
	}

	// 色の初期化
	spriteBG_->SetColor({0.2f, 0.2f, 0.2f, 1.0f});
	spriteFill_->SetColor({0.0f, 1.0f, 0.0f, 1.0f});

	// 初期HP
	maxHP_ = 10.0f;
	currentHP_ = maxHP_;
}

void GaugeUI::SetHP(float current, float max) {
	if (max <= 0.0f)
		max = 1.0f;
	current = std::clamp(current, 0.0f, max);

	// 更新
	// currentHP_は即座に反映（Fill）
	// afterHP_はダメージ時にcurrentHP_より大きければ徐々に下がる
	currentHP_ = current;
	maxHP_ = max;
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

	// BGは中央に置く
	spriteBG_->SetPosition({screenX, screenY, drawDepth_});

	// 左端xを計算（BGの中央を基点として左端を求める）
	float leftX = screenX - (fullWidth_ * 0.5f);

	// Fillの幅（ピクセル）
	float fillRatio = (maxHP_ > 0.0f) ? (currentHP_ / maxHP_) : 0.0f;
	float fillWidthPx = fullWidth_ * fillRatio;

	// テクスチャ原寸を取得してスケールを決定
	Vector2 texFill = spriteFill_->GetTextureSize();

	if (texFill.x > 0.0f) {
		spriteFill_->SetScale({(fillWidthPx / texFill.x), (fullHeight_ / texFill.y), 1.0f});
	}

	// 若干の補正としてYは中央合わせ
	spriteFill_->SetPosition({leftX, screenY, drawDepth_ + 0.0f});

	// 画面外での非表示
	bool offscreen = (screenX < -fullWidth_ || screenX > WinApp::kClientWidth + fullWidth_ || screenY < -fullHeight_ || screenY > WinApp::kClientHeight + fullHeight_);

	Vector4 bgColor = spriteBG_->GetColor();
	bgColor.w = offscreen ? 0.0f : 1.0f;
	spriteBG_->SetColor(bgColor);
	Vector4 fillColor = spriteFill_->GetColor();
	fillColor.w = offscreen ? 0.0f : 1.0f;
	spriteFill_->SetColor(fillColor);
}

void GaugeUI::Draw() {
    if (spriteBG_) spriteBG_->Draw(handleBG_.gpuHandle);
    if (spriteFill_) spriteFill_->Draw(handleFill_.gpuHandle);
}