#include "HitPoint.h"
#include "Application/TD2_2/Utility/GameUtils.h"

std::vector<std::unique_ptr<IDrawable>> HitPoint::Initialize(Vector2 pivot, SettingObject setObj, int hpCount) {
	std::vector<std::unique_ptr<IDrawable>> sprites;

	// HPの数
	maxHPCount_ = hpCount;
	currentHPCount_ = hpCount;
	prevHPCount_ = hpCount;

	// 指定するオブジェクト（使いたいスプライト）
	setObj_ = setObj;

	// HPアイコンを作成
	for (int i = 0; i < maxHPCount_; i++) {
		auto hpIcon = CreateHPIcon();

		if (setObj_ == SettingObject::PLAYER) {
			hpIcon->GetTransform().translate.x = pivot.x + i * 64.0f;
		} else if (setObj_ == SettingObject::BOSS) {
			hpIcon->GetTransform().translate.x = pivot.x - i * 64.0f;
		}

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
	if (currentHPCount > maxHPCount_ || currentHPCount < 0)
		return;

	// HP更新
	currentHPCount_ = currentHPCount;

	// アイコンファイルパスを選択
	const std::string& normalIconFilePath = (setObj_ == SettingObject::PLAYER) ? playerIconFilePath_ : bossIconFilePath_;
	const std::string& damageIconFilePath = (setObj_ == SettingObject::PLAYER) ? playerDamageIconFilePath_ : bossDamageIconFilePath_;

	// ダメージアイコンに変更（減った分）
	for (int i = maxHPCount_ - 1; i >= currentHPCount_; i--) {
		hpIcon_[i]->SetTexture(damageIconFilePath);
	}

	// 通常アイコンに戻す（回復した分）
	for (int i = 0; i < currentHPCount_; i++) {
		hpIcon_[i]->SetTexture(normalIconFilePath);
	}

	// ダメージを受けた場合画像を少し拡大して0に縮小するアニメーションをつける
	if (prevHPCount_ > currentHPCount_) {

		// ダメージを受けたアイコンの番号を保存
		damageIconNum_ = prevHPCount_ - 1;

		for (int i = currentHPCount_; i < prevHPCount_; i++) {
			// アニメーションフラグを立てる
			isDamageAnimation_ = true;
			// 拡大
			hpIcon_[i]->GetTransform().scale = {1.5f, 1.5f, 1.0f};
		}
	}

	if (isDamageAnimation_) {
		// --- 振り子回転 ---
		pendulumTime_ += 0.5f;                         // 時間の進み具合（速度調整用）
		float angle = std::sin(pendulumTime_) * (30.0f * std::numbers::pi_v<float> / 180.0f); // -30°〜30°に変換
		hpIcon_[damageIconNum_]->GetTransform().rotate.z = angle;

		// 徐々に縮小
		hpIcon_[damageIconNum_]->GetTransform().scale.x -= 0.05f;
		hpIcon_[damageIconNum_]->GetTransform().scale.y -= 0.05f;

		// 最小スケールに達したらアニメーション終了
		if (hpIcon_[damageIconNum_]->GetTransform().scale.x <= 0.0f || hpIcon_[damageIconNum_]->GetTransform().scale.y <= 0.0f) {
			hpIcon_[damageIconNum_]->GetTransform().scale = {0.0f, 0.0f, 1.0f};
			isDamageAnimation_ = false;
		}
	}


	// 前回のHPを保存
	prevHPCount_ = currentHPCount_;
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