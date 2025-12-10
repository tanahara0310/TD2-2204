#include "TitleDemoManager.h"
#include "../../../GameObject/TitleDemo/TitlePlayerDemo.h"
#include "../../../GameObject/TitleDemo/TitleEnemyDemo.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include <numbers>

#ifdef _DEBUG
#include "Engine/Utility/Debug/ImGui/ImGuiManager.h"
#endif

void TitleDemoManager::Initialize(TitlePlayerDemo* player, TitleEnemyDemo* enemy) {
	demoPlayer_ = player;
	demoEnemy_ = enemy;

	// 初期パターン設定（敵が自機を追跡、+X方向）
	if (demoPlayer_) {
		demoPlayer_->SetChasingMode(false);
		demoPlayer_->SetMoveDirection(1.0f);
	}
	if (demoEnemy_) {
		demoEnemy_->SetChasingMode(true);
		demoEnemy_->SetMoveDirection(1.0f);
	}
}

void TitleDemoManager::Update(float deltaTime) {
	(void)deltaTime; // 未使用パラメータ警告を回避
	
	if (!demoPlayer_) {
		return;
	}

	// デモの自動リセット（画面外に出た場合）
	Vector3 playerPos = demoPlayer_->GetWorldPosition();
	bool shouldSwitch = false;

	if (isMovingRight_ && playerPos.x > 50.0f) {
		shouldSwitch = true;
	} else if (!isMovingRight_ && playerPos.x < -50.0f) {
		shouldSwitch = true;
	}

	if (shouldSwitch) {
		SwitchPattern();
	}
}

void TitleDemoManager::SwitchPattern() {
	if (!demoPlayer_ || !demoEnemy_) {
		return;
	}

	// カウンターを増加
	demoSwitchCounter_++;

	// カウンターが総サイクルに達したらリセット
	if (demoSwitchCounter_ >= kDemoTotalCycle_) {
		demoSwitchCounter_ = 0;
	}

	// 2回後ろ→1回前のパターン
	if (demoSwitchCounter_ < kDemoBehindCount_) {
		isDemoBehindBackground_ = true;
	} else {
		isDemoBehindBackground_ = false;
	}

	// 移動方向を反転
	isMovingRight_ = !isMovingRight_;
	float direction = isMovingRight_ ? 1.0f : -1.0f;

	// パターンを切り替え
	if (currentDemoPattern_ == DemoPattern::EnemyChasePlayer) {
		SwitchToPlayerChaseEnemy(direction);
	} else {
		SwitchToEnemyChasePlayer(direction);
	}

	// 位置と回転を設定
	SetDemoPositions();

	// トランスフォームを更新
	demoPlayer_->GetTransform().TransferMatrix();
	demoEnemy_->GetTransform().TransferMatrix();
}

void TitleDemoManager::SwitchToPlayerChaseEnemy(float direction) {
	currentDemoPattern_ = DemoPattern::PlayerChaseEnemy;

	demoPlayer_->SetChasingMode(true);
	demoPlayer_->SetTarget(demoEnemy_);
	demoPlayer_->SetMoveDirection(direction);
	demoPlayer_->SetMoveSpeed(15.0f); // 追いかける側：さらに速く（12.0f → 15.0f）

	demoEnemy_->SetChasingMode(false);
	demoEnemy_->SetMoveDirection(direction);
	demoEnemy_->SetChaseSpeed(6.0f); // 逃げる側：さらに遅く（8.0f → 6.0f）
}

void TitleDemoManager::SwitchToEnemyChasePlayer(float direction) {
	currentDemoPattern_ = DemoPattern::EnemyChasePlayer;

	demoPlayer_->SetChasingMode(false);
	demoPlayer_->SetMoveDirection(direction);
	demoPlayer_->SetMoveSpeed(6.0f); // 逃げる側：さらに遅く（8.0f → 6.0f）

	demoEnemy_->SetChasingMode(true);
	demoEnemy_->SetMoveDirection(direction);
	demoEnemy_->SetChaseSpeed(15.0f); // 追いかける側：さらに速く（12.0f → 15.0f）
}

void TitleDemoManager::SetDemoPositions() {
	constexpr float kLeftStartX = -35.0f;
	constexpr float kLeftBackX = -70.0f; // さらに間隔を広げる（-55.0f → -70.0f）
	constexpr float kRightStartX = 45.0f;
	constexpr float kRightBackX = 80.0f; // さらに間隔を広げる（65.0f → 80.0f）

	// Y座標の配列（上、中、下）
	constexpr float yPositions[3] = { kDemoYTop_, kDemoYMiddle_, kDemoYBottom_ };

	// 前回と異なるY座標をランダムに選択
	int newYIndex;
	do {
		newYIndex = RandomGenerator::GetInstance().GetInt(0, 2);
	} while (newYIndex == lastDemoYIndex_);
	lastDemoYIndex_ = newYIndex;

	float demoY = yPositions[newYIndex];

	// Z座標を背景の前後で切り替え
	float demoZ = isDemoBehindBackground_ ? kDemoZBehind_ : kDemoZFront_;

	float rotation = isMovingRight_ 
		? std::numbers::pi_v<float> / 2.0f
		: -std::numbers::pi_v<float> / 2.0f;

	// 追跡パターンに応じて前後を決定
	bool isPlayerFront = (currentDemoPattern_ == DemoPattern::EnemyChasePlayer);
	
	if (isMovingRight_) {
		// 左から右へ
		if (isPlayerFront) {
			demoPlayer_->GetTransform().translate = { kLeftStartX, demoY, demoZ };
			demoPlayer_->GetTransform().rotate.y = rotation;
			demoPlayer_->SetInitialPosition({ kLeftStartX, demoY, demoZ });
			demoEnemy_->GetTransform().translate = { kLeftBackX, demoY, demoZ };
			demoEnemy_->GetTransform().rotate.y = rotation;
			demoEnemy_->SetInitialPosition({ kLeftBackX, demoY, demoZ });
		} else {
			demoEnemy_->GetTransform().translate = { kLeftStartX, demoY, demoZ };
			demoEnemy_->GetTransform().rotate.y = rotation;
			demoEnemy_->SetInitialPosition({ kLeftStartX, demoY, demoZ });
			demoPlayer_->GetTransform().translate = { kLeftBackX, demoY, demoZ };
			demoPlayer_->GetTransform().rotate.y = rotation;
			demoPlayer_->SetInitialPosition({ kLeftBackX, demoY, demoZ });
		}
	} else {
		// 右から左へ
		if (isPlayerFront) {
			demoPlayer_->GetTransform().translate = { kRightStartX, demoY, demoZ };
			demoPlayer_->GetTransform().rotate.y = rotation;
			demoPlayer_->SetInitialPosition({ kRightStartX, demoY, demoZ });
			demoEnemy_->GetTransform().translate = { kRightBackX, demoY, demoZ };
			demoEnemy_->GetTransform().rotate.y = rotation;
			demoEnemy_->SetInitialPosition({ kRightBackX, demoY, demoZ });
		} else {
			demoEnemy_->GetTransform().translate = { kRightStartX, demoY, demoZ };
			demoEnemy_->GetTransform().rotate.y = rotation;
			demoEnemy_->SetInitialPosition({ kRightStartX, demoY, demoZ });
			demoPlayer_->GetTransform().translate = { kRightBackX, demoY, demoZ };
			demoPlayer_->GetTransform().rotate.y = rotation;
			demoPlayer_->SetInitialPosition({ kRightBackX, demoY, demoZ });
		}
	}
}

#ifdef _DEBUG
void TitleDemoManager::DrawImGui() {
	if (ImGui::Begin("Title Demo Control")) {
		ImGui::Text("Demo Settings");
		ImGui::Separator();

		// 現在のパターン表示
		const char* patternName = (currentDemoPattern_ == DemoPattern::EnemyChasePlayer)
			? "Enemy Chase Player" : "Player Chase Enemy";
		ImGui::Text("Current Pattern: %s", patternName);
		ImGui::Text("Moving Direction: %s", isMovingRight_ ? "+X (Right)" : "-X (Left)");
		ImGui::Text("Z Position: %s", isDemoBehindBackground_ ? "Behind Background" : "In Front of Background");
		ImGui::Text("Switch Counter: %d / %d (Behind: %d, Front: %d)", 
			demoSwitchCounter_, 
			kDemoTotalCycle_,
			kDemoBehindCount_,
			kDemoInFrontCount_);

		ImGui::Spacing();

		// パターン手動切り替えボタン
		if (ImGui::Button("Switch Demo Pattern")) {
			SwitchPattern();
		}

		ImGui::Spacing();
		ImGui::Separator();

		// リセットボタン
		if (ImGui::Button("Reset Demo Positions")) {
			if (demoPlayer_) {
				demoPlayer_->ResetToInitialPosition();
			}
			if (demoEnemy_) {
				demoEnemy_->ResetToInitialPosition();
			}
		}

		ImGui::Spacing();

		// プレイヤーの速度調整
		if (demoPlayer_) {
			float playerSpeed = demoPlayer_->GetMoveSpeed();
			if (ImGui::SliderFloat("Player Speed", &playerSpeed, 1.0f, 20.0f)) {
				demoPlayer_->SetMoveSpeed(playerSpeed);
			}
		}

		// エネミーの速度調整
		if (demoEnemy_) {
			float enemySpeed = demoEnemy_->GetChaseSpeed();
			if (ImGui::SliderFloat("Enemy Chase Speed", &enemySpeed, 1.0f, 25.0f)) {
				demoEnemy_->SetChaseSpeed(enemySpeed);
			}
		}

		ImGui::Spacing();
		ImGui::Separator();

		// 現在位置の表示
		if (demoPlayer_) {
			Vector3 playerPos = demoPlayer_->GetWorldPosition();
			ImGui::Text("Player Pos: (%.1f, %.1f, %.1f)", playerPos.x, playerPos.y, playerPos.z);
			ImGui::Text("Player Mode: %s", demoPlayer_->IsChasingMode() ? "Chasing" : "Moving");
		}
		if (demoEnemy_) {
			Vector3 enemyPos = demoEnemy_->GetWorldPosition();
			ImGui::Text("Enemy Pos: (%.1f, %.1f, %.1f)", enemyPos.x, enemyPos.y, enemyPos.z);
			ImGui::Text("Enemy Mode: %s", demoEnemy_->IsChasingMode() ? "Chasing" : "Moving");
		}

		ImGui::End();
	}
}
#endif
