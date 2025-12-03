#include "HitPoint.h"

std::vector<std::unique_ptr<IDrawable>> HitPoint::Initialize(Vector2 pivot, SettingObject setObj, int hpCount) {
	std::vector<std::unique_ptr<IDrawable>> sprites;

	// HPの数
	maxHPCount_ = hpCount;
	currentHPCount_ = hpCount;

	// 指定するオブジェクト（使いたいスプライト）
	setObj_ = setObj;

	// HPアイコンを作成
	for (int i = 0; i < maxHPCount_; i++) {
		auto hpIcon = CreateHPIcon();
		hpIcon->GetTransform().translate.x = pivot.x + i * 64.0f;
		hpIcon->GetTransform().translate.y = pivot.y;
		hpIcon->GetTransform().translate.z = 0.0f;
		hpIcon_.push_back(hpIcon.get());
		sprites.push_back(std::move(hpIcon));
	}

	return sprites;
}

void HitPoint::Update() {}

void HitPoint::SetHP(int currentHPCount) {
	// 範囲チェック 
	if (currentHPCount > maxHPCount_ || currentHPCount < 0) return;

	// HP更新
	currentHPCount_ = currentHPCount;

	if (setObj_ == SettingObject::PLAYER) { // プレイヤーの場合
		// ダメージアイコンに変更（減った分）
		for (int i = maxHPCount_ - 1; i >= currentHPCount_; i--) {
			hpIcon_[i]->SetTexture(playerDamageIconFilePath_);
		}
		// 通常アイコンに戻す（回復した分）
		for (int i = 0; i < currentHPCount_; i++) {
			hpIcon_[i]->SetTexture(playerIconFilePath_);
		}
	} else if (setObj_ == SettingObject::BOSS) { // 敵の場合
		// ダメージアイコンに変更（減った分）
		for (int i = 0; i < maxHPCount_ - currentHPCount_; i++) {
			hpIcon_[i]->SetTexture(bossDamageIconFilePath_);
		}
		// 通常アイコンに戻す（回復した分）
		for (int i = maxHPCount_ - currentHPCount_; i < maxHPCount_; i++) {
			hpIcon_[i]->SetTexture(bossIconFilePath_);
		}
	}
}

std::unique_ptr<SpriteObject> HitPoint::CreateHPIcon() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->SetAnchor({0.5f, 0.5f});

	if (setObj_ == SettingObject::PLAYER) {
		sprite->Initialize(playerIconFilePath_); // プレイヤーの画像を使用
	} else if (setObj_ == SettingObject::BOSS) {
		sprite->Initialize(bossIconFilePath_); // 敵の画像を使用
	}

	return sprite;
}