#include "TitleUI.h"
#include "EngineSystem/EngineSystem.h"
#include "ObjectCommon/IDrawable.h"
#include "Input/KeyboardInput.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include <cmath>

std::vector<std::unique_ptr<IDrawable>> TitleUI::Initialize(EngineSystem* engine) {
	(void)engine;

	std::vector<std::unique_ptr<IDrawable>> sprites;

	// タイトルロゴを作成
	auto titleLogo = CreateTitleLogo();
	titleLogo_ = titleLogo.get();
	sprites.push_back(std::move(titleLogo));

	// 開始ボタンUIを作成
	auto startButtonUI = CreateStartButtonUI();
	startButtonUI_ = startButtonUI.get();
	sprites.push_back(std::move(startButtonUI));

	// quitボタンUIを作成
	auto quitButtonUI = CreateQuitButtonUI();
	quitButtonUI_ = quitButtonUI.get();
	sprites.push_back(std::move(quitButtonUI));

	// 矢印UI（左）を作成
	auto arrowLeftUI = CreateArrowUI();
	arrowLeftUI_ = arrowLeftUI.get();
	sprites.push_back(std::move(arrowLeftUI));

	// 矢印UI（右）を作成
	auto arrowRightUI = CreateArrowUI();
	arrowRightUI_ = arrowRightUI.get();
	// 右側の矢印は左右反転
	if (arrowRightUI_) {
		arrowRightUI_->GetTransform().scale.x = -1.0f;
	}
	sprites.push_back(std::move(arrowRightUI));

	// 初期位置を保存
	if (titleLogo_) {
		titleLogoInitialPos_ = titleLogo_->GetTransform().translate;
	}
	if (startButtonUI_) {
		startButtonInitialPos_ = startButtonUI_->GetTransform().translate;
	}
	if (quitButtonUI_) {
		quitButtonInitialPos_ = quitButtonUI_->GetTransform().translate;
	}

	// ステートマシーンの初期化
	InitializeStateMachine();

	// 初期位置を設定
	UpdateArrowPosition();
	if (arrowLeftUI_) {
		arrowLeftInitialPos_ = arrowLeftUI_->GetTransform().translate;
	}
	if (arrowRightUI_) {
		arrowRightInitialPos_ = arrowRightUI_->GetTransform().translate;
	}

	// 登場アニメーション用に初期状態を設定
	if (titleLogo_) {
		titleLogo_->GetTransform().translate.y += kTitleBounceHeight;
		titleLogo_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
	}
	if (startButtonUI_) {
		startButtonUI_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
		startButtonUI_->GetTransform().scale = { 0.5f, 0.5f, 1.0f };
	}
	if (quitButtonUI_) {
		quitButtonUI_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
		quitButtonUI_->GetTransform().scale = { 0.5f, 0.5f, 1.0f };
	}
	if (arrowLeftUI_) {
		arrowLeftUI_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
	}
	if (arrowRightUI_) {
		arrowRightUI_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
	}

	// タイマーを初期化
	introAnimationTimer_.Start(kIntroAnimationDuration, false);
	idleAnimationTimer_.Start(100.0f, true);  // 大きな値でループ
	arrowAnimationTimer_.Start(100.0f, true);

	return sprites;
}

void TitleUI::Update() {
	float deltaTime = 1.0f / 60.0f;

	// タイマー更新
	introAnimationTimer_.Update(deltaTime);
	idleAnimationTimer_.Update(deltaTime);
	arrowAnimationTimer_.Update(deltaTime);
	confirmAnimationTimer_.Update(deltaTime);
	arrowTransitionTimer_.Update(deltaTime);  // 矢印遷移タイマー更新

	// 登場アニメーションの更新
	if (introAnimationTimer_.IsActive()) {
		UpdateIntroAnimation();
	} else {
		// 待機アニメーションの更新
		UpdateIdleAnimation();
	}

	// 矢印の揺れアニメーション
	UpdateArrowAnimation();

	// ステートマシーンの更新
	stateMachine_.Update();

	// 決定アニメーションの更新
	if (confirmAnimationTimer_.IsActive()) {
		UpdateConfirmAnimation();
	}
}

void TitleUI::SetSelectionState(SelectionState state) {
	// 前の状態と異なる場合のみ遷移アニメーションを開始
	if (selectionState_ != state && arrowLeftUI_ && arrowRightUI_) {
		// 現在の実際の位置を保存（揺れの影響を含む）
		arrowLeftStartPos_ = arrowLeftUI_->GetTransform().translate;
		arrowRightStartPos_ = arrowRightUI_->GetTransform().translate;
		
		// 目標位置を設定
		switch (state) {
		case SelectionState::Start:
			arrowLeftTargetPos_ = { kArrowOffsetX_Start_Left, kStartButtonY, 0.0f };
			arrowRightTargetPos_ = { kArrowOffsetX_Start_Right, kStartButtonY, 0.0f };
			break;
		case SelectionState::Quit:
			arrowLeftTargetPos_ = { kArrowOffsetX_Quit_Left, kQuitButtonY, 0.0f };
			arrowRightTargetPos_ = { kArrowOffsetX_Quit_Right, kQuitButtonY, 0.0f };
			break;
		}
		
		// 遷移アニメーション開始
		arrowTransitionTimer_.Start(kArrowTransitionDuration, false);
	}
	
	selectionState_ = state;

	// ステートマシーンにリクエスト
	switch (state) {
	case SelectionState::Start:
		stateMachine_.RequestState("Start", 100);
		break;
	case SelectionState::Quit:
		stateMachine_.RequestState("Quit", 100);
		break;
	}
}

void TitleUI::OnConfirm() {
	// アニメーション開始
	confirmAnimationTimer_.Start(kConfirmAnimationDuration, false);
}

void TitleUI::InitializeStateMachine() {
	// Start状態の登録
	stateMachine_.AddState("Start",
		[this]() {
			// Start状態への遷移時
			selectionState_ = SelectionState::Start;
			UpdateArrowPosition();
			if (arrowLeftUI_) {
				arrowLeftInitialPos_ = arrowLeftUI_->GetTransform().translate;
			}
			if (arrowRightUI_) {
				arrowRightInitialPos_ = arrowRightUI_->GetTransform().translate;
			}
		},
		[this]() {
			// Start状態の更新処理
		}
	);

	// Quit状態の登録
	stateMachine_.AddState("Quit",
		[this]() {
			// Quit状態への遷移時
			selectionState_ = SelectionState::Quit;
			UpdateArrowPosition();
			if (arrowLeftUI_) {
				arrowLeftInitialPos_ = arrowLeftUI_->GetTransform().translate;
			}
			if (arrowRightUI_) {
				arrowRightInitialPos_ = arrowRightUI_->GetTransform().translate;
			}
		},
		[this]() {
			// Quit状態の更新処理
		}
	);

	// 遷移ルールの設定
	stateMachine_.AddTransitionRule("Start", { "Quit" });
	stateMachine_.AddTransitionRule("Quit", { "Start" });

	// 初期状態をStartに設定
	stateMachine_.RequestState("Start", 100);
}

void TitleUI::UpdateArrowPosition() {
	if (!arrowLeftUI_ || !arrowRightUI_) {
		return;
	}

	// 選択状態に応じて矢印の位置を更新（UIフォントの両端に配置）
	switch (selectionState_) {
	case SelectionState::Start:
		arrowLeftUI_->GetTransform().translate = { kArrowOffsetX_Start_Left, kStartButtonY, 0.0f };
		arrowRightUI_->GetTransform().translate = { kArrowOffsetX_Start_Right, kStartButtonY, 0.0f };
		break;
	case SelectionState::Quit:
		arrowLeftUI_->GetTransform().translate = { kArrowOffsetX_Quit_Left, kQuitButtonY, 0.0f };
		arrowRightUI_->GetTransform().translate = { kArrowOffsetX_Quit_Right, kQuitButtonY, 0.0f };
		break;
	}
}

void TitleUI::UpdateIntroAnimation() {
	// 正規化された時間（0.0～1.0）
	float t = introAnimationTimer_.GetProgress();

	if (introAnimationTimer_.IsFinished()) {
		// 最終状態を設定
		if (titleLogo_) {
			titleLogo_->GetTransform().translate = titleLogoInitialPos_;
			titleLogo_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
		if (startButtonUI_) {
			startButtonUI_->GetTransform().translate = startButtonInitialPos_;
			startButtonUI_->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
			startButtonUI_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
		if (quitButtonUI_) {
			quitButtonUI_->GetTransform().translate = quitButtonInitialPos_;
			quitButtonUI_->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
			quitButtonUI_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
		if (arrowLeftUI_) {
			arrowLeftUI_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
		if (arrowRightUI_) {
			arrowRightUI_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
		return;
	}

	// タイトルロゴ：バウンスしながら降りてくる（0.0～0.4秒）
	if (titleLogo_ && t < 0.4f) {
		float titleT = t / 0.4f;
		// EaseOutBounceを使用
		float eased = EasingUtil::Apply(titleT, EasingUtil::Type::EaseOutBounce);

		float yOffset = kTitleBounceHeight * (1.0f - eased);
		titleLogo_->GetTransform().translate.y = titleLogoInitialPos_.y + yOffset;
		titleLogo_->SetColor({ 1.0f, 1.0f, 1.0f, titleT });
	}

	// スタートボタン：フェードイン＆スケールアップ（0.3～0.7秒）
	if (startButtonUI_ && t > 0.3f) {
		float buttonT = (t - 0.3f) / 0.4f;
		buttonT = std::clamp(buttonT, 0.0f, 1.0f);
		
		// EaseOutBackを使用
		float easedScale = EasingUtil::Apply(buttonT, EasingUtil::Type::EaseOutBack);
		float scale = 0.5f + 0.5f * easedScale;
		startButtonUI_->GetTransform().scale = { scale, scale, 1.0f };
		startButtonUI_->SetColor({ 1.0f, 1.0f, 1.0f, buttonT });
	}

	// Quitボタン：フェードイン＆スケールアップ（0.5～0.9秒）
	if (quitButtonUI_ && t > 0.5f) {
		float buttonT = (t - 0.5f) / 0.4f;
		buttonT = std::clamp(buttonT, 0.0f, 1.0f);
		
		// EaseOutBackを使用
		float easedScale = EasingUtil::Apply(buttonT, EasingUtil::Type::EaseOutBack);
		float scale = 0.5f + 0.5f * easedScale;
		quitButtonUI_->GetTransform().scale = { scale, scale, 1.0f };
		quitButtonUI_->SetColor({ 1.0f, 1.0f, 1.0f, buttonT });
	}

	// 矢印：フェードイン（0.7～1.0秒）
	if (t > 0.7f) {
		float arrowT = (t - 0.7f) / 0.3f;
		arrowT = std::clamp(arrowT, 0.0f, 1.0f);
		if (arrowLeftUI_) {
			arrowLeftUI_->SetColor({ 1.0f, 1.0f, 1.0f, arrowT });
		}
		if (arrowRightUI_) {
			arrowRightUI_->SetColor({ 1.0f, 1.0f, 1.0f, arrowT });
		}
	}
}

void TitleUI::UpdateIdleAnimation() {
	float time = idleAnimationTimer_.GetElapsedTime();

	// 浮遊オフセットを事前計算（共通処理）
	float floatOffset = std::sin(time * kIdleFloatSpeed) * kIdleFloatAmount;
	
	// 補間係数を事前計算
	float lerpFactor = 0.2f;
	float invLerpFactor = 1.0f - lerpFactor;
	bool isConfirming = confirmAnimationTimer_.IsActive();

	// 選択状態に応じてボタンを更新
	bool isStartSelected = (selectionState_ == SelectionState::Start);
	
	// スタートボタンの処理
	if (startButtonUI_) {
		if (isStartSelected) {
			// 選択中：浮遊アニメーション
			startButtonUI_->GetTransform().translate.y = startButtonInitialPos_.y + floatOffset;
			if (!isConfirming) {
				startButtonUI_->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
			}
		} else {
			// 非選択：初期位置に固定し、スケールを滑らかに1.0に戻す
			startButtonUI_->GetTransform().translate.y = startButtonInitialPos_.y;
			if (!isConfirming) {
				Vector3 currentScale = startButtonUI_->GetTransform().scale;
				startButtonUI_->GetTransform().scale = {
					currentScale.x * invLerpFactor + lerpFactor,
					currentScale.y * invLerpFactor + lerpFactor,
					1.0f
				};
			}
		}
	}

	// Quitボタンの処理
	if (quitButtonUI_) {
		if (!isStartSelected) {
			// 選択中：浮遊アニメーション
			quitButtonUI_->GetTransform().translate.y = quitButtonInitialPos_.y + floatOffset;
			if (!isConfirming) {
				quitButtonUI_->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
			}
		} else {
			// 非選択：初期位置に固定し、スケールを滑らかに1.0に戻す
			quitButtonUI_->GetTransform().translate.y = quitButtonInitialPos_.y;
			if (!isConfirming) {
				Vector3 currentScale = quitButtonUI_->GetTransform().scale;
				quitButtonUI_->GetTransform().scale = {
					currentScale.x * invLerpFactor + lerpFactor,
					currentScale.y * invLerpFactor + lerpFactor,
					1.0f
				};
			}
		}
	}

	// タイトルロゴは拡縮アニメーション
	if (titleLogo_) {
		float scaleOffset = std::sin(time * kIdleFloatSpeed * 0.5f) * kTitleScaleAmount;
		float scale = 1.0f + scaleOffset;
		titleLogo_->GetTransform().scale = { scale, scale, 1.0f };
		titleLogo_->GetTransform().translate = titleLogoInitialPos_;
	}
}

void TitleUI::UpdateArrowAnimation() {
	if (!arrowLeftUI_ || !arrowRightUI_ || introAnimationTimer_.IsActive()) {
		return;
	}

	float time = arrowAnimationTimer_.GetElapsedTime();
	
	// 遷移中かどうかを判定
	bool isTransitioning = arrowTransitionTimer_.IsActive();
	
	// 矢印の基準位置を決定
	Vector3 baseLeftPos;
	Vector3 baseRightPos;
	
	if (isTransitioning) {
		// 遷移アニメーション中：イージングで補間
		float t = arrowTransitionTimer_.GetProgress();
		baseLeftPos = EasingUtil::LerpVector3(arrowLeftStartPos_, arrowLeftTargetPos_, t, EasingUtil::Type::EaseOutCubic);
		baseRightPos = EasingUtil::LerpVector3(arrowRightStartPos_, arrowRightTargetPos_, t, EasingUtil::Type::EaseOutCubic);
		
		// 遷移完了直前に初期位置を更新（次フレームのためにここで更新）
		if (t >= 0.99f || arrowTransitionTimer_.IsFinished()) {
			arrowLeftInitialPos_ = arrowLeftTargetPos_;
			arrowRightInitialPos_ = arrowRightTargetPos_;
		}
	} else {
		// 通常状態：初期位置を使用
		baseLeftPos = arrowLeftInitialPos_;
		baseRightPos = arrowRightInitialPos_;
	}

	// 揺れアニメーション（遷移中は揺れを抑制）
	float swingMultiplier = isTransitioning ? 0.3f : 1.0f;  // 遷移中は揺れを30%に抑制
	float swingOffsetLeft = std::sin(time * kArrowSwingSpeed) * kArrowSwingAmount * swingMultiplier;
	float swingOffsetRight = -swingOffsetLeft;  // 右矢印は逆位相（計算を削減）

	// 位置を設定
	arrowLeftUI_->GetTransform().translate = {
		baseLeftPos.x + swingOffsetLeft,
		baseLeftPos.y,
		baseLeftPos.z
	};
	arrowRightUI_->GetTransform().translate = {
		baseRightPos.x + swingOffsetRight,
		baseRightPos.y,
		baseRightPos.z
	};

	// スケールアニメーション（決定アニメーション中は停止）
	if (!confirmAnimationTimer_.IsActive()) {
		float targetScale = 1.0f + 0.05f * std::sin(time * kArrowSwingSpeed * 2.0f);
		
		// 補間係数を事前に計算
		float lerpFactor = 0.3f;
		float invLerpFactor = 1.0f - lerpFactor;  // 1 - lerpFactor を事前計算
		
		// 左矢印のスケール補間（最適化）
		Vector3 currentScaleLeft = arrowLeftUI_->GetTransform().scale;
		arrowLeftUI_->GetTransform().scale = {
			currentScaleLeft.x * invLerpFactor + targetScale * lerpFactor,
			currentScaleLeft.y * invLerpFactor + targetScale * lerpFactor,
			1.0f
		};
		
		// 右矢印のスケール補間（最適化、左右反転）
		Vector3 currentScaleRight = arrowRightUI_->GetTransform().scale;
		arrowRightUI_->GetTransform().scale = {
			currentScaleRight.x * invLerpFactor + (-targetScale) * lerpFactor,
			currentScaleRight.y * invLerpFactor + targetScale * lerpFactor,
			1.0f
		};
	}
}

void TitleUI::UpdateConfirmAnimation() {
	float t = confirmAnimationTimer_.GetProgress();

	if (confirmAnimationTimer_.IsFinished()) {
		// スケールとカラーを確実にリセット
		if (startButtonUI_) {
			startButtonUI_->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
			startButtonUI_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
		if (quitButtonUI_) {
			quitButtonUI_->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
			quitButtonUI_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
		if (arrowLeftUI_) {
			arrowLeftUI_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
			Vector3 currentScale = arrowLeftUI_->GetTransform().scale;
			arrowLeftUI_->GetTransform().scale = { 
				std::abs(currentScale.x) > 0.01f ? currentScale.x : 1.0f, 
				currentScale.y, 
			1.0f 
			};
		}
		if (arrowRightUI_) {
			arrowRightUI_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
			Vector3 currentScale = arrowRightUI_->GetTransform().scale;
			arrowRightUI_->GetTransform().scale = { 
				std::abs(currentScale.x) > 0.01f ? currentScale.x : -1.0f, 
				currentScale.y, 
			1.0f 
			};
		}
		return;
	}

	// スケール計算を最適化
	float scale;
	if (t < 0.5f) {
		float t1 = t * 2.0f;  // 0.0～1.0に正規化
		scale = EasingUtil::Lerp(1.0f, kButtonScaleMax, t1, EasingUtil::Type::EaseOutQuad);
	} else {
		float t2 = (t - 0.5f) * 2.0f;  // 0.0～1.0に正規化
		scale = EasingUtil::Lerp(kButtonScaleMax, 1.0f, t2, EasingUtil::Type::EaseInQuad);
		// 終盤で確実に1.0にする（計算を削減）
		if (t2 > 0.95f) {
			scale = 1.0f;
		}
	}

	// 選択中のボタンを特定
	bool isStartSelected = (selectionState_ == SelectionState::Start);
	SpriteObject* selectedButton = isStartSelected ? startButtonUI_ : quitButtonUI_;
	SpriteObject* unselectedButton = isStartSelected ? quitButtonUI_ : startButtonUI_;

	// 選択中のボタンにスケールとカラーを適用
	if (selectedButton) {
		selectedButton->GetTransform().scale = { scale, scale, 1.0f };
		
		// 明るさを変化（パルス効果）- 計算を最適化
		float brightness = 1.0f + 0.3f * std::sin(t * 12.56637f);  // 3.14159 * 4 を事前計算
		selectedButton->SetColor({ brightness, brightness, brightness, 1.0f });
	}

	// 選択されていないボタンのスケールは1.0に保つ
	if (unselectedButton) {
		unselectedButton->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
		unselectedButton->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	// 矢印の点滅（両方に同じ処理）
	if (arrowLeftUI_ && arrowRightUI_) {
		float elapsedTime = confirmAnimationTimer_.GetElapsedTime();
		float alpha = 0.5f + 0.5f * std::sin(elapsedTime * kArrowBlinkSpeed);
		Vector4 arrowColor = { 1.0f, 1.0f, 1.0f, alpha };
		arrowLeftUI_->SetColor(arrowColor);
		arrowRightUI_->SetColor(arrowColor);
	}
}

std::unique_ptr<SpriteObject> TitleUI::CreateTitleLogo() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/Title.png");
	// 2Dカメラは画面中央が原点(0,0)なので、中央に配置するには(0,0)を指定
	sprite->GetTransform().translate = { 10.0f, 180.0f, 0.0f };
	sprite->SetAnchor({ 0.5f, 0.5f });
	return sprite;
}

std::unique_ptr<SpriteObject> TitleUI::CreateStartButtonUI()
{
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/titleStart.png");
	sprite->GetTransform().translate = { 6.0f, kStartButtonY, 0.0f };
	sprite->SetAnchor({ 0.5f, 0.5f });

	return sprite;
}

std::unique_ptr<SpriteObject> TitleUI::CreateQuitButtonUI()
{
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/quit.png");
	sprite->GetTransform().translate = { 0.0f, kQuitButtonY, 0.0f };
	sprite->SetAnchor({ 0.5f, 0.5f });

	return sprite;
}

std::unique_ptr<SpriteObject> TitleUI::CreateArrowUI()
{
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/arrow.png");
	// 初期位置はStart用の左矢印
	sprite->GetTransform().translate = { kArrowOffsetX_Start_Left, kStartButtonY, 0.0f };
	sprite->SetAnchor({ 0.5f, 0.5f });

	return sprite;
}
