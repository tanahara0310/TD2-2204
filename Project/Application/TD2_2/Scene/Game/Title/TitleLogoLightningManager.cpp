#include "TitleLogoLightningManager.h"
#include "../../../Effect/Lightning/LightningEffectManager.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include "IDrawable.h"

void TitleLogoLightningManager::Initialize(LightningEffectManager* lightningManager, std::vector<std::unique_ptr<IDrawable>>& gameObjects) {
	lightningManager_ = lightningManager;

	if (!lightningManager_) {
		return;
	}

	LightningEffectManager::EffectConfig config;
	config.segmentCount = 12;
	config.noiseScale = 1.7f;
	config.noiseSpeed = 60.0f;
	config.randomOffsetRange = 0.0f;
	config.voxelScale = { 1.0f, 1.0f, 1.0f };
	config.initialVisible = false;
	config.voxelSpacing = 0.2f;
	config.color = kLightningColor_;

	// 初期状態: 左端の点
	// position を {0,0,0} にして、offset で絶対座標を指定
	config.startOffset = { -kHalfWidth, kLogoY, kLogoZ };
	config.endOffset = { -kHalfWidth, kLogoY, kLogoZ };
	
	lightningEffectId_ = lightningManager_->CreateEffect({ 0.0f, 0.0f, 0.0f }, config, gameObjects);

	// 初期は非表示
	if (lightningEffectId_ >= 0) {
		lightningManager_->SetEffectVisible(lightningEffectId_, false);
	}
}

void TitleLogoLightningManager::Update(float deltaTime) {
	if (!isAnimating_) {
		return;
	}

	// 待機中の場合
	if (isWaiting_) {
		waitTimer_.Update(deltaTime);
		if (waitTimer_.IsFinished()) {
			// 待機終了、雷を再表示してアニメーション再開
			isWaiting_ = false;
			isMovingForward_ = true;
			
			// 雷を再表示
			if (lightningManager_) {
				lightningManager_->SetEffectVisibleImmediate(lightningEffectId_, true);
			}
			
			animationTimer_.Start(kAnimationDuration, false);
		}
		return;
	}

	UpdateAnimation(deltaTime);
}

void TitleLogoLightningManager::UpdateAnimation(float deltaTime) {
	if (!lightningManager_ || lightningEffectId_ < 0) {
		return;
	}

	animationTimer_.Update(deltaTime);
	float progress = animationTimer_.GetProgress();
	
	if (isMovingForward_) {
		// 左→右: 始点は左端固定、終点が右端まで一気に伸びる（雷の一閃）
		// EaseOutCubicで素早く伸びて減速
		float easedProgress = EasingUtil::Apply(progress, EasingUtil::Type::EaseOutCubic);
		
		float endX = -kHalfWidth + (kHalfWidth * 2.0f) * easedProgress;
		
		Vector3 startOffset = { -kHalfWidth, kLogoY, kLogoZ };
		Vector3 endOffset = { endX, kLogoY, kLogoZ };
		
		lightningManager_->SetEffectOffsets(lightningEffectId_, startOffset, endOffset);
	} else {
		// 右→左: 始点が左端まで素早く移動して消える（残像のように）
		// EaseInCubicで加速しながら縮む
		float easedProgress = EasingUtil::Apply(progress, EasingUtil::Type::EaseInCubic);
		
		float startX = -kHalfWidth + (kHalfWidth * 2.0f) * easedProgress;
		
		Vector3 startOffset = { startX, kLogoY, kLogoZ };
		Vector3 endOffset = { kHalfWidth, kLogoY, kLogoZ };
		
		lightningManager_->SetEffectOffsets(lightningEffectId_, startOffset, endOffset);
	}

	// アニメーション完了時の処理
	if (animationTimer_.IsFinished()) {
		if (isMovingForward_) {
			// 伸びるアニメーション完了 → 縮むアニメーション開始
			isMovingForward_ = false;
			animationTimer_.Start(kAnimationDuration, false);
		} else {
			// 縮むアニメーション完了 → 待機開始
			isMovingForward_ = true;
			isWaiting_ = true;
			waitTimer_.Start(kWaitDuration, false);
			
			// 雷を非表示にする
			if (lightningManager_) {
				lightningManager_->SetEffectVisibleImmediate(lightningEffectId_, false);
			}
		}
	}
}

void TitleLogoLightningManager::ShowLightning() {
	if (!lightningManager_ || lightningEffectId_ < 0) {
		return;
	}

	lightningManager_->SetEffectVisibleImmediate(lightningEffectId_, true);
	
	// アニメーション開始
	isAnimating_ = true;
	isMovingForward_ = true;
	isWaiting_ = false;
	animationTimer_.Start(kAnimationDuration, false);
}

void TitleLogoLightningManager::HideLightning() {
	if (!lightningManager_ || lightningEffectId_ < 0) {
		return;
	}

	lightningManager_->SetEffectVisibleImmediate(lightningEffectId_, false);
	isAnimating_ = false;
	isWaiting_ = false;
}
