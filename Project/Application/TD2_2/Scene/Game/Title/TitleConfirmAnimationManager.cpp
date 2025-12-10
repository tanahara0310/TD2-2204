#include "TitleConfirmAnimationManager.h"
#include "../../../GameObject/TitleUI/TitleUI.h"
#include "../../../Effect/Lightning/LightningEffectManager.h"
#include "Engine/Math/Easing/EasingUtil.h"

void TitleConfirmAnimationManager::Initialize(TitleUI* titleUI, LightningEffectManager* lightningManager) {
	titleUI_ = titleUI;
	lightningManager_ = lightningManager;
}

void TitleConfirmAnimationManager::StartAnimation(bool isStartSelected, int frameEffectIds[4]) {
	if (!titleUI_) {
		return;
	}

	isAnimating_ = true;
	wasStartSelected_ = isStartSelected;
	frameEffectIds_ = frameEffectIds;
	animationTimer_.Start(kAnimationDuration_, false);

	if (isStartSelected) {
		// スタート選択時
		if (titleUI_->GetStartModel()) {
			auto* startModel = titleUI_->GetStartModel();
			selectedModelInitialScale_ = startModel->GetTransform().scale.x;
			startModel->SetConfirmingMode(true);
		}

		// 非選択モデルの元の色を保存
		if (titleUI_->GetYameruModel() && titleUI_->GetYameruModel()->GetModel()) {
			auto* materialManager = titleUI_->GetYameruModel()->GetModel()->GetMaterialManager();
			if (materialManager) {
				unselectedModelOriginalColor_ = materialManager->GetColor();
			}
		}
	} else {
		// やめる選択時
		if (titleUI_->GetYameruModel()) {
			auto* yameruModel = titleUI_->GetYameruModel();
			selectedModelInitialScale_ = yameruModel->GetTransform().scale.x;
			yameruModel->SetConfirmingMode(true);
		}

		// 非選択モデルの元の色を保存
		if (titleUI_->GetStartModel() && titleUI_->GetStartModel()->GetModel()) {
			auto* materialManager = titleUI_->GetStartModel()->GetModel()->GetMaterialManager();
			if (materialManager) {
				unselectedModelOriginalColor_ = materialManager->GetColor();
			}
		}
	}

	// 雷エフェクトを全辺一斉に表示
	if (lightningManager_ && frameEffectIds_) {
		for (int i = 0; i < 4; ++i) {
			if (frameEffectIds_[i] >= 0) {
				lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], true);
			}
		}
	}
}

void TitleConfirmAnimationManager::Update(float deltaTime) {
	if (!isAnimating_ || !titleUI_) {
		return;
	}

	// タイマーの更新
	animationTimer_.Update(deltaTime);

	// 進行度を取得（0.0～1.0）
	float progress = animationTimer_.GetProgress();

	// 緩急のある山型イージング（0→1→0）
	float easedProgress;
	if (progress < 0.5f) {
		float t = progress * 3.0f;
		float eased = EasingUtil::Apply(t, EasingUtil::Type::EaseOutQuad);
		easedProgress = eased;
	} else {
		float t = (progress - 0.5f) * 3.0f;
		float eased = EasingUtil::Apply(t, EasingUtil::Type::EaseInQuad);
		easedProgress = 1.0f - eased;
	}

	// 選択状態に応じた処理
	if (wasStartSelected_) {
		// スタート選択時の処理
		auto* startModel = titleUI_->GetStartModel();
		if (startModel) {
			float scaleMultiplier = 1.0f + (kSelectedScaleMax_ - 1.0f) * easedProgress;
			float newScale = selectedModelInitialScale_ * scaleMultiplier;
			startModel->GetTransform().scale = { newScale, newScale, newScale };
		}

		// 非選択モデル（Yameru）のフェードアウト
		auto* yameruModel = titleUI_->GetYameruModel();
		if (yameruModel && yameruModel->GetModel()) {
			auto* materialManager = yameruModel->GetModel()->GetMaterialManager();
			if (materialManager) {
				float alpha = 1.0f - progress;
				Vector4 fadedColor = {
					unselectedModelOriginalColor_.x,
					unselectedModelOriginalColor_.y,
					unselectedModelOriginalColor_.z,
					alpha
				};
				materialManager->SetColor(fadedColor);
			}
		}
	} else {
		// やめる選択時の処理
		auto* yameruModel = titleUI_->GetYameruModel();
		if (yameruModel) {
			float scaleMultiplier = 1.0f + (kSelectedScaleMax_ - 1.0f) * easedProgress;
			float newScale = selectedModelInitialScale_ * scaleMultiplier;
			yameruModel->GetTransform().scale = { newScale, newScale, newScale };
		}

		// 非選択モデル（Start）のフェードアウト
		auto* startModel = titleUI_->GetStartModel();
		if (startModel && startModel->GetModel()) {
			auto* materialManager = startModel->GetModel()->GetMaterialManager();
			if (materialManager) {
				float alpha = 1.0f - progress;
				Vector4 fadedColor = {
					unselectedModelOriginalColor_.x,
					unselectedModelOriginalColor_.y,
					unselectedModelOriginalColor_.z,
					alpha
				};
				materialManager->SetColor(fadedColor);
			}
		}
	}

	// 雷エフェクトの点滅演出（0.05秒間隔で高速点滅）
	if (progress < 0.5f && lightningManager_ && frameEffectIds_) {
		static float flickerTimer = 0.0f;
		flickerTimer += deltaTime;
		if (flickerTimer >= 0.05f) {
			flickerTimer = 0.0f;
			static bool flickerOn = false;
			flickerOn = !flickerOn;
			for (int i = 0; i < 4; i++) {
				if (frameEffectIds_[i] >= 0) {
					lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], flickerOn);
				}
			}
		}
	}

	// 演出が終了したら
	if (animationTimer_.IsFinished()) {
		isAnimating_ = false;

		// 雷エフェクトを非表示に
		if (lightningManager_ && frameEffectIds_) {
			for (int i = 0; i < 4; ++i) {
				if (frameEffectIds_[i] >= 0) {
					lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], false);
				}
			}
		}
	}
}
