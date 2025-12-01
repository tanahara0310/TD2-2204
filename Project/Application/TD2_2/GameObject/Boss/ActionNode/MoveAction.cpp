#include "MoveAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include <iostream>
#include <chrono>

MoveAction::MoveAction(Boss* boss)
   : BossActionNode(boss, "MoveAction")
{}

void MoveAction::Reset() {
   BossActionNode::Reset();
   moveTimer_.Reset();
}

void MoveAction::OnEnter() {
   moveTimer_.Start(5.0f, false);
   if (boss_) {
	  boss_->SetMaxSpeed(50.0f);
	  boss_->SetDampingPerSecond(0.3f);
   }
}

NodeState MoveAction::OnExecute() {
   // 移動フェーズ
   if (!moveTimer_.IsFinished()) {
	  ExecuteMove();
	  return BossActionHelper::Running();
   }
   // 移動完了
   return BossActionHelper::Success();
}

void MoveAction::OnExit() {
   // 移動終了時の処理（必要なら追加）
}

void MoveAction::SetupStateMachine() {
   // ステートマシンのセットアップ（必要なら追加）
}

void MoveAction::ExecuteMove() {
   moveTimer_.Update(GameUtils::GetDeltaTime());

   if (boss_) {
	  static float time = 0.0f;
	  time += GameUtils::GetDeltaTime();

	  // 2D Perlin Noiseを使用してより自然な移動方向を生成
	  float noiseX = GameUtils::ParlineNoise2D(time * 0.5f, 0.0f);
	  float noiseY = GameUtils::ParlineNoise2D(time * 0.5f, 100.0f);

	  // ノイズ値を正規化（-1~1の範囲）
	  Vector2 noiseDir = Vector2(noiseX, noiseY).Normalize();

	  // 中心への方向ベクトル
	  Vector2 toCenter = { -boss_->GetWorldPosition().x, -boss_->GetWorldPosition().y };
	  float distanceToCenter = toCenter.Length();

	  // 距離に応じて中心へのバイアスを動的に調整
	  // 中心から遠いほど中心へのバイアスが強くなる
	  float dynamicBias = biasAmount_;
	  if (distanceToCenter > boss_->GetMoveableAreaRadius() * 0.7f) {
		 // 移動可能範囲の70%を超えたら中心へのバイアスを強化
		 dynamicBias = 0.3f;
	  }

	  // knockback multiplier applies: smaller multiplier -> stronger center bias
	  float multiplier = boss_->GetMoveBiasMultiplier();
	  dynamicBias *= multiplier;

	  // ノイズと中心方向を組み合わせて最終的な移動方向を決定
	  Vector2 moveDir = noiseDir * dynamicBias + toCenter.Normalize() * (1.0f - dynamicBias);

	  boss_->SetDirection(moveDir.Normalize());

	  boss_->AddAcceleration({ moveDir.x * moveSpeed_, moveDir.y * moveSpeed_ });
   }
}