#include "GameObject.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include <numbers>
#include <algorithm>

void GameObject::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
   auto engine = GetEngineSystem();
   auto dxCommon = engine->GetComponent<DirectXCommon>();

   {
	  model_ = std::move(model);
	  transform_.Initialize(dxCommon->GetDevice());
	  transform_.SetRotationMode(WorldTransform::RotationMode::Quaternion);
	  texture_ = texture;
   }
}

void GameObject::AttachCollider(std::unique_ptr<Collider> collider) {
   collider_ = std::move(collider);
}

void GameObject::AttachStateMachine() {
   stateMachine_ = std::make_unique<StateMachine>();
}

Quaternion GameObject::CalculateBaseRotation(const Vector2& dir) {
   // dirを-1.0～1.0にクランプ
   float clampedX = std::clamp(dir.x, -1.0f, 1.0f);
   float clampedY = std::clamp(dir.y, -1.0f, 1.0f);

   // デフォルトの前傾角度（常に適用される）- モデルが逆向きなので負の値
   const float defaultPitchAngle = -std::numbers::pi_v<float> / 12.0f; // -15度（後ろ傾き→モデルにとって前傾）

   // Y方向の入力による追加ピッチ角度（-y: さらに前傾, +y: 上を向く）
   const float maxAdditionalPitchAngle = std::numbers::pi_v<float> / 6.0f; // 30度
   float additionalPitchAngle = clampedY * maxAdditionalPitchAngle; // -yで前傾、+yで上向き

   // 最終的なピッチ角度
   float totalPitchAngle = defaultPitchAngle + additionalPitchAngle;

   // 傾き角度を計算（ロール：最大30度）
   const float maxTiltAngle = std::numbers::pi_v<float> / 6.0f; // 30度
   float rollAngle = -clampedX * maxTiltAngle; // 負の符号で正しい方向

   // 回転角度を計算（ヨー：最大30度）
   const float maxYawAngle = std::numbers::pi_v<float> / 6.0f; // 30度
   float yawAngle = clampedX * -maxYawAngle; // 右プラス、左マイナス

   // 1. X軸を中心にピッチ回転（前傾姿勢 - デフォルト + Y入力）
   Vector3 rightAxis = { 1.0f, 0.0f, 0.0f };
   Quaternion pitchRotation = MathCore::QuaternionMath::MakeRotateAxisAngle(rightAxis, totalPitchAngle);

   // 2. Y軸を中心にヨー回転（左右への回転）
   Vector3 upAxis = { 0.0f, 1.0f, 0.0f };
   Quaternion yawRotation = MathCore::QuaternionMath::MakeRotateAxisAngle(upAxis, yawAngle);

   // 3. Z軸を中心にロール回転（傾き）
   Vector3 forwardAxis = { 0.0f, 0.0f, 1.0f };
   Quaternion rollRotation = MathCore::QuaternionMath::MakeRotateAxisAngle(forwardAxis, rollAngle);

   // ピッチ → ヨー → ロールの順で合成
   Quaternion combinedRotation = MathCore::QuaternionMath::Multiply(
       MathCore::QuaternionMath::Multiply(pitchRotation, yawRotation),
       rollRotation
   );

   return MathCore::QuaternionMath::Normalize(combinedRotation);
}

void GameObject::TiltByVelocity(const Vector2& dir) {
   // 目標方向を設定（各成分を個別にクランプ）
   targetDir_.x = std::clamp(dir.x, -1.0f, 1.0f);
   targetDir_.y = std::clamp(dir.y, -1.0f, 1.0f);

   // デルタタイムを取得
   float deltaTime = GameUtils::GetDeltaTime();

   // 現在の方向を目標方向に向けて滑らかに補間（各成分を個別に）
   float lerpFactor = std::clamp(dirLerpSpeed_ * deltaTime, 0.0f, 1.0f);
   currentDir_.x = currentDir_.x + (targetDir_.x - currentDir_.x) * lerpFactor;
   currentDir_.y = currentDir_.y + (targetDir_.y - currentDir_.y) * lerpFactor;

   // 補間された方向でベース回転を計算
   Quaternion baseRotation = CalculateBaseRotation(currentDir_);

   // 回転アニメーション中かどうかで処理を分岐
   if (isRotationActive_ && rotationTimer_ && rotationTimer_->IsActive()) {
       // 回転アニメーション中：ベース回転を更新
       rotationStartQuaternion_ = baseRotation;
   } else {
       // 通常時：直接適用
       transform_.quaternionRotate = baseRotation;
   }
}

void GameObject::StartRotateAroundAxis(float duration, float rotationCount) {
   // タイマーを新規作成して開始
   rotationTimer_ = std::make_unique<GameTimer>(duration, false);
   rotationTimer_->Start(duration, false);

   // 回転回数を保存
   rotationCount_ = rotationCount;

   // Y軸（上方向）を回転軸として使用
   rotationAxis_ = { 0.0f, 1.0f, 0.0f };

   // 回転開始時のクォータニオンを保存（現在の傾きを含む）
   rotationStartQuaternion_ = transform_.quaternionRotate;
   isRotationActive_ = true;
}

void GameObject::UpdateRotation() {
   // タイマーが存在しない、または動作していない場合は何もしない
   if (!rotationTimer_ || !rotationTimer_->IsActive()) {
       isRotationActive_ = false;
       return;
   }

   // デルタタイムを取得
   float deltaTime = GameUtils::GetDeltaTime();

   // タイマーを更新
   rotationTimer_->Update(deltaTime);

   // イージング進行率を取得（EaseInOut）
   float easedProgress = rotationTimer_->GetEasedProgress(EasingUtil::Type::EaseInOutQuad);

   // 回転回数を考慮した角度を計算
   const float baseRotation = 2.0f * std::numbers::pi_v<float>; // 1回転 = 2π
   float totalRotation = baseRotation * rotationCount_; // 指定回数分の回転
   float currentAngle = totalRotation * easedProgress;

   // Y軸を中心とした回転クォータニオンを作成
   Quaternion axisRotation = MathCore::QuaternionMath::MakeRotateAxisAngle(rotationAxis_, currentAngle);

   // ベース回転（傾きと左右回転を含む）に軸回転を適用
   Quaternion newRotation = MathCore::QuaternionMath::Multiply(rotationStartQuaternion_, axisRotation);
   transform_.quaternionRotate = MathCore::QuaternionMath::Normalize(newRotation);

   // タイマー終了時の処理
   if (rotationTimer_->IsFinished()) {
       isRotationActive_ = false;
   }
}