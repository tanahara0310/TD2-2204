#include "Lightning.h"
#include "Application/TD2_2/GameObject/Voxel/Voxel.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#include <Windows.h>
#endif

namespace {
	// 定数をキャッシュ
	constexpr float kVoxelSpacing = 0.5f;      // 0.3f → 0.5f に変更（配置間隔をさらに広げる）
	constexpr float kVoxelScale = 3.0f;        // 1.5f → 3.0f に変更（ボクセルを3倍に）
	constexpr float kMinDistance = kVoxelSpacing * 0.1f;
	constexpr float kNoiseFrequency = 15.0f;
	constexpr float kEdgeFadeStart = 0.15f;
	constexpr float kEdgeFadeEnd = 0.85f;
	constexpr float kEdgeFadeInvStart = 1.0f / kEdgeFadeStart;
	constexpr float kEdgeFadeInvEnd = 1.0f / (1.0f - kEdgeFadeEnd);
	
	// アニメーション更新の間引き（毎フレームではなく数フレームに1回）
	constexpr int kAnimationUpdateInterval = 4; // 3 → 4 に変更（4フレームに1回）
}

void Lightning::Initialize(ModelResource* voxelModel, TextureManager::LoadedTexture voxelTexture,
	const Config& config, const std::string& name)
{
	config_ = config;
	name_ = name;
	time_ = 0.0f;
	needsRegeneration_ = false;
	voxelModel_ = voxelModel;
	voxelTexture_ = voxelTexture;
	animationFrameCounter_ = 0;

	// パスを生成してボクセルを配置
	GeneratePath();
	GenerateVoxels();
}

void Lightning::Update()
{
	// 前フレームで削除予約されたオブジェクトをクリア（GPU安全）
	ClearDeferredDeletions();

	// 再生成が必要な場合
	if (needsRegeneration_) {
		GeneratePath();
		GenerateVoxels();
		needsRegeneration_ = false;
		animationFrameCounter_ = 0;
	}

	if (config_.enableAnimation) {
		// フレームカウンタを更新
		animationFrameCounter_++;
		
		// 間引き: N フレームに1回だけ更新
		if (animationFrameCounter_ >= kAnimationUpdateInterval) {
			animationFrameCounter_ = 0;
			
			// 時間を進める
			float deltaTime = GameUtils::GetDeltaTime();
			if (deltaTime <= 0.0f) {
				deltaTime = 1.0f / 60.0f;
			}
			time_ += config_.noiseSpeed * deltaTime;

			// パスを再生成して位置更新
			GeneratePath();
			UpdateVoxelPositions();
		}
	}

	// 親の変換行列を更新
	transform_.TransferMatrix();

	// 親クラスの更新（子オブジェクトの更新を含む）
	Object3d::Update();
}

void Lightning::Draw(const ICamera* camera)
{
	// 親クラスの描画（children_を自動描画）
	Object3d::Draw(camera);
}

bool Lightning::DrawImGui()
{
#ifdef _DEBUG
	bool changed = false;

	// 一意な名前をヘッダーに使用
	std::string headerName = name_ + "##" + std::to_string(reinterpret_cast<uintptr_t>(this));
	if (ImGui::CollapsingHeader(headerName.c_str())) {
		// 一意なIDをPush
		ImGui::PushID(this);

		// 基本設定
		ImGui::Text("基本設定");
		ImGui::Separator();

		changed |= ImGui::DragFloat3("始点", &config_.startPoint.x, 0.1f);
		changed |= ImGui::DragFloat3("終点", &config_.endPoint.x, 0.1f);

		// セグメント数変更時は再生成をリクエスト（遅延実行）
		int oldSegment = config_.segmentCount;
		changed |= ImGui::DragInt("セグメント数", &config_.segmentCount, 1.0f, 2, 100);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("パスの分割数。多いほど滑らかな曲線になる");
		}
		if (oldSegment != config_.segmentCount) {
			RequestRegeneration();
		}

		ImGui::Spacing();
		ImGui::Text("ノイズ設定");
		ImGui::Separator();

		changed |= ImGui::DragFloat("ノイズ振幅", &config_.noiseScale, 0.01f, 0.0f, 2.0f);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("雷の揺れの大きさ");
		}
		
		changed |= ImGui::DragFloat("ノイズ速度", &config_.noiseSpeed, 0.01f, 0.0f, 20.0f);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("アニメーションの速さ");
		}

		ImGui::Spacing();
		ImGui::Text("外観");
		ImGui::Separator();

		if (ImGui::ColorEdit4("色", &config_.color.x)) {
			// 色が変更されたら全てのボクセルに適用
			for (auto& child : children_) {
				if (auto* voxel = dynamic_cast<Voxel*>(child.get())) {
					voxel->SetColor(config_.color);
				}
			}
			changed = true;
		}

		// パスタイプの選択
		const char* pathTypeNames[] = { "直線", "円弧" };
		int currentPathType = static_cast<int>(config_.pathType);
		if (ImGui::Combo("パスタイプ", &currentPathType, pathTypeNames, IM_ARRAYSIZE(pathTypeNames))) {
			config_.pathType = static_cast<PathType>(currentPathType);
			RequestRegeneration();
			changed = true;
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("始点と終点の補間方法\n直線: 一直線に補間\n円弧: 円弧状に補間");
		}

		ImGui::Spacing();
		ImGui::Text("アニメーション");
		ImGui::Separator();

		changed |= ImGui::Checkbox("有効", &config_.enableAnimation);

		ImGui::Spacing();
		if (ImGui::Button("再生成")) {
			RequestRegeneration();
			changed = true;
		}

		ImGui::Spacing();
		ImGui::Text("情報");
		ImGui::Separator();
		ImGui::Text("パスポイント数: %zu", pathPoints_.size());
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("セグメント数+1個のポイントで構成");
		}
		
		ImGui::Text("ボクセル数: %zu", children_.size());
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("パス全体の距離÷0.2で自動計算される");
		}
		
		ImGui::Text("プール内ボクセル: %zu", voxelPool_.size());
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("再利用待ちのボクセル数（パフォーマンス最適化用）");
		}
		
		ImGui::Text("遅延削除待ち: %zu", deferredDeletions_.size());
		ImGui::Text("アニメーション時間: %.2f", time_);

		ImGui::PopID();
	}

	return changed;
#else
	return false;
#endif
}

void Lightning::GeneratePath()
{
	// PathTypeに応じて補間方法を切り替え
	switch (config_.pathType) {
	case PathType::Linear:
		GenerateLinearPath();
		break;
	case PathType::CircularArc:
		GenerateCircularArcPath();
		break;
	default:
		GenerateLinearPath();
		break;
	}
}

void Lightning::GenerateLinearPath()
{
	pathPoints_.clear();

	if (config_.segmentCount < 2) {
		pathPoints_.push_back(config_.startPoint);
		pathPoints_.push_back(config_.endPoint);
		return;
	}

	// 始点→終点の方向（最適化: 一度だけ計算）
	Vector3 direction = config_.endPoint - config_.startPoint;
	float directionLengthSq = direction.x * direction.x + direction.y * direction.y + direction.z * direction.z;
	
	if (directionLengthSq < 0.0001f) {
		pathPoints_.push_back(config_.startPoint);
		pathPoints_.push_back(config_.endPoint);
		return;
	}
	
	// 逆数を使用して除算を減らす
	float invLength = 1.0f / std::sqrt(directionLengthSq);
	Vector3 normalizedDir = { direction.x * invLength, direction.y * invLength, direction.z * invLength };

	// 垂直ベクトル（キャッシュ）
	Vector3 perpendicular1, perpendicular2;
	if (std::abs(normalizedDir.y) > 0.99f) {
		perpendicular1 = { 1.0f, 0.0f, 0.0f };
		perpendicular2 = { 0.0f, 0.0f, 1.0f };
	} else {
		float invLen = 1.0f / std::sqrt(normalizedDir.y * normalizedDir.y + normalizedDir.x * normalizedDir.x);
		perpendicular1 = { -normalizedDir.y * invLen, normalizedDir.x * invLen, 0.0f };
		perpendicular2 = MathCore::Vector::Normalize(MathCore::Vector::Cross(normalizedDir, perpendicular1));
	}

	// パスポイントを生成
	float invSegmentCount = 1.0f / static_cast<float>(config_.segmentCount);
	
	for (int i = 0; i <= config_.segmentCount; ++i) {
		float t = static_cast<float>(i) * invSegmentCount;
		Vector3 basePos = config_.startPoint + direction * t;

		// 始点と終点は固定
		if (i == 0 || i == config_.segmentCount) {
			pathPoints_.push_back(basePos);
			continue;
		}

		// ノイズ計算
		float noiseInputX = t * kNoiseFrequency;
		float noiseX = GameUtils::ParlineNoise2D(noiseInputX, time_);
		float noiseZ = GameUtils::ParlineNoise2D(noiseInputX, time_ + 50.0f);

		// 端のフェード（条件分岐を減らす）
		float edgeFade = 1.0f;
		if (t < kEdgeFadeStart) {
			edgeFade = t * kEdgeFadeInvStart;
		} else if (t > kEdgeFadeEnd) {
			edgeFade = (1.0f - t) * kEdgeFadeInvEnd;
		}
		
		float scaledNoise = config_.noiseScale * edgeFade;
		Vector3 offset = perpendicular1 * (noiseX * scaledNoise) + perpendicular2 * (noiseZ * scaledNoise);

		pathPoints_.push_back(basePos + offset);
	}
}

void Lightning::GenerateCircularArcPath()
{
	pathPoints_.clear();

	if (config_.segmentCount < 2) {
		pathPoints_.push_back(config_.startPoint);
		pathPoints_.push_back(config_.endPoint);
		return;
	}

	// 半径計算（最適化）
	float startRadiusSq = config_.startPoint.x * config_.startPoint.x + config_.startPoint.y * config_.startPoint.y;
	float endRadiusSq = config_.endPoint.x * config_.endPoint.x + config_.endPoint.y * config_.endPoint.y;
	
	if (startRadiusSq < 0.0001f || endRadiusSq < 0.0001f) {
		pathPoints_.push_back(config_.startPoint);
		pathPoints_.push_back(config_.endPoint);
		return;
	}

	float startRadius = std::sqrt(startRadiusSq);
	float endRadius = std::sqrt(endRadiusSq);

	// 角度計算
	float startAngle = std::atan2(config_.startPoint.y, config_.startPoint.x);
	float endAngle = std::atan2(config_.endPoint.y, config_.endPoint.x);
	
	float angleDiff = endAngle - startAngle;
	if (angleDiff > std::numbers::pi_v<float>) {
		angleDiff -= 2.0f * std::numbers::pi_v<float>;
	} else if (angleDiff < -std::numbers::pi_v<float>) {
		angleDiff += 2.0f * std::numbers::pi_v<float>;
	}

	float radiusDiff = endRadius - startRadius;
	float zDiff = config_.endPoint.z - config_.startPoint.z;
	float invSegmentCount = 1.0f / static_cast<float>(config_.segmentCount);

	for (int i = 0; i <= config_.segmentCount; ++i) {
		float t = static_cast<float>(i) * invSegmentCount;
		float currentAngle = startAngle + angleDiff * t;
		float currentRadius = startRadius + radiusDiff * t;
		
		// 三角関数を一度だけ計算
		float cosAngle = std::cos(currentAngle);
		float sinAngle = std::sin(currentAngle);
		
		Vector3 basePos = {
			cosAngle * currentRadius,
			sinAngle * currentRadius,
			config_.startPoint.z + zDiff * t
		};

		if (i == 0 || i == config_.segmentCount) {
			pathPoints_.push_back(basePos);
			continue;
		}

		// ノイズ
		float noiseInputX = t * kNoiseFrequency;
		float noiseTangent = GameUtils::ParlineNoise2D(noiseInputX, time_);
		float noiseNormal = GameUtils::ParlineNoise2D(noiseInputX, time_ + 50.0f);

		Vector3 tangent = { -sinAngle, cosAngle, 0.0f };
		Vector3 normal = { cosAngle, sinAngle, 0.0f };

		// 端のフェード
		float edgeFade = 1.0f;
		if (t < kEdgeFadeStart) {
			edgeFade = t * kEdgeFadeInvStart;
		} else if (t > kEdgeFadeEnd) {
			edgeFade = (1.0f - t) * kEdgeFadeInvEnd;
		}
		
		float scaledNoise = config_.noiseScale * edgeFade;
		Vector3 offset = tangent * (noiseTangent * scaledNoise) + normal * (noiseNormal * scaledNoise);

		pathPoints_.push_back(basePos + offset);
	}
}

void Lightning::GenerateVoxels()
{
	// 古いボクセルをプールに返却
	ReturnVoxelsToPool();

	// パスポイントが不足している場合は何もしない
	if (pathPoints_.size() < 2) {
		return;
	}

	// パスポイント間にボクセルを配置
	for (size_t i = 0; i < pathPoints_.size() - 1; ++i) {
		PlaceVoxelsBetween(pathPoints_[i], pathPoints_[i + 1]);
	}
	
	// 最後のパスポイント（終点）にもボクセルを配置して完全な線にする
	auto finalVoxel = GetVoxelFromPool();
	finalVoxel->GetTransform().translate = pathPoints_.back();
	finalVoxel->GetTransform().scale = { kVoxelScale, kVoxelScale, kVoxelScale }; // スケールを大きく
	AddChild(std::move(finalVoxel));
}

void Lightning::UpdateVoxelPositions()
{
	if (pathPoints_.size() < 2 || children_.empty()) {
		return;
	}

	// 必要数の事前計算（最適化: sqrt呼び出しを減らす）
	size_t requiredVoxels = 0;
	
	for (size_t i = 0; i < pathPoints_.size() - 1; ++i) {
		Vector3 diff = pathPoints_[i + 1] - pathPoints_[i];
		float distanceSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		
		if (distanceSq >= kMinDistance * kMinDistance) {
			// sqrt を ceilf の中でのみ使用
			int voxelCount = static_cast<int>(std::ceilf(std::sqrt(distanceSq) / kVoxelSpacing));
			requiredVoxels += voxelCount;
		}
	}
	requiredVoxels += 1;
	
	// 許容範囲を広げる（スケールが大きくなったため）
	int voxelDiff = static_cast<int>(requiredVoxels) - static_cast<int>(children_.size());
	if (std::abs(voxelDiff) > 3) { // 5 → 3 に変更（スケールが大きいので厳しめに）
		RequestRegeneration();
		return;
	}

	size_t voxelIndex = 0;

	for (size_t i = 0; i < pathPoints_.size() - 1; ++i) {
		Vector3 start = pathPoints_[i];
		Vector3 end = pathPoints_[i + 1];
		Vector3 segmentDiff = end - start;
		float distanceSq = segmentDiff.x * segmentDiff.x + segmentDiff.y * segmentDiff.y + segmentDiff.z * segmentDiff.z;

		if (distanceSq < kMinDistance * kMinDistance) {
			continue;
		}

		float distance = std::sqrt(distanceSq);
		int voxelCount = static_cast<int>(std::ceilf(distance / kVoxelSpacing));
		float invVoxelCount = 1.0f / static_cast<float>(voxelCount);

		for (int j = 0; j < voxelCount; ++j) {
			if (voxelIndex >= children_.size()) {
				return;
			}

			float t = static_cast<float>(j) * invVoxelCount;
			Vector3 position = start + segmentDiff * t;

			// Voxelにキャスト（子オブジェクトは全てVoxelと仮定）
			if (auto* voxel = static_cast<Voxel*>(children_[voxelIndex].get())) {
				voxel->GetTransform().translate = position;
				// スケールは変更しない（GenerateVoxelsで既に設定済み）
			}
			voxelIndex++;
		}
	}
	
	if (voxelIndex < children_.size()) {
		if (auto* voxel = static_cast<Voxel*>(children_[voxelIndex].get())) {
			voxel->GetTransform().translate = pathPoints_.back();
			// スケールは変更しない
		}
	}
}

void Lightning::PlaceVoxelsBetween(const Vector3& start, const Vector3& end)
{
	Vector3 diff = end - start;
	float distanceSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	// 距離が短すぎる場合は配置しない（最適化)
	if (distanceSq < kMinDistance * kMinDistance) {
		return;
	}

	float distance = std::sqrt(distanceSq);

	// 必要なボクセル数（配置間隔を広げて数を削減）
	int voxelCount = static_cast<int>(std::ceilf(distance / kVoxelSpacing));
	if (voxelCount < 1) {
		voxelCount = 1;
	}

	float invVoxelCount = 1.0f / static_cast<float>(voxelCount);

	// ボクセルを配置（プールから再利用）
	for (int i = 0; i < voxelCount; ++i) {
		// 位置計算
		float t = static_cast<float>(i) * invVoxelCount;
		Vector3 position = start + diff * t;

		// プールからボクセルを取得（再利用）
		auto voxel = GetVoxelFromPool();
		voxel->GetTransform().translate = position;
		voxel->GetTransform().scale = { kVoxelScale, kVoxelScale, kVoxelScale }; // スケールを大きく

		// 子オブジェクトとして追加
		AddChild(std::move(voxel));
	}
}

void Lightning::RequestRegeneration()
{
	// 次のUpdateで再生成
	needsRegeneration_ = true;
}

void Lightning::ClearDeferredDeletions()
{
	// 前フレームで遅延削除されたオブジェクトをプールに返却
	for (auto& obj : deferredDeletions_) {
		if (auto* voxel = dynamic_cast<Voxel*>(obj.get())) {
			// Voxelの場合はプールに返却（所有権を移動）
			// 親の参照をクリア
			voxel->GetTransform().SetParent(nullptr);
			voxelPool_.push_back(std::unique_ptr<Voxel>(voxel));
			obj.release(); // 所有権を手放す
		}
	}
	
	// それ以外のオブジェクトは削除
	deferredDeletions_.clear();
}

std::unique_ptr<Voxel> Lightning::GetVoxelFromPool()
{
	// プールに使用可能なボクセルがあれば再利用
	if (!voxelPool_.empty()) {
		auto voxel = std::move(voxelPool_.back());
		voxelPool_.pop_back();
		// 色を設定
		voxel->SetColor(config_.color);
		// 親を設定（Lightningのワールド変換を継承）
		voxel->GetTransform().SetParent(&transform_);
		return voxel;
	}
	
	// プールが空の場合は新規作成
	auto voxel = std::make_unique<Voxel>();
	voxel->Initialize(voxelModel_, voxelTexture_); // モデルとテクスチャを渡す
	voxel->SetColor(config_.color); // 色を設定
	// 親を設定（Lightningのワールド変換を継承）
	voxel->GetTransform().SetParent(&transform_);
	return voxel;
}

void Lightning::ReturnVoxelsToPool()
{
	// 現在のボクセルをプールに返却
	for (auto& child : children_) {
		if (auto* voxel = dynamic_cast<Voxel*>(child.get())) {
			// 親の参照をクリア
			voxel->GetTransform().SetParent(nullptr);
			voxelPool_.push_back(std::unique_ptr<Voxel>(voxel));
			child.release();
		}
	}
	children_.clear();
}
