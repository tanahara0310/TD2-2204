#include "Lightning.h"
#include "Application/TD2_2/GameObject/Voxel/Voxel.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#endif

void Lightning::Initialize(ModelResource* voxelModel, TextureManager::LoadedTexture voxelTexture,
	const Config& config, const std::string& name)
{
	if (!voxelModel) {
#ifdef _DEBUG
		OutputDebugStringW(L"[ERROR] Lightning::Initialize: voxelModelがnullptrです\n");
#endif
		return;
	}

	config_ = config;
	name_ = name;
	time_ = 0.0f;
	needsRegeneration_ = false;
	voxelModel_ = voxelModel;
	voxelTexture_ = voxelTexture;

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
	}

	if (config_.enableAnimation) {
		// 時間を進める（GetDeltaTime()が0の場合は固定値を使用）
		float deltaTime = GameUtils::GetDeltaTime();
		if (deltaTime <= 0.0f) {
			deltaTime = 1.0f / 60.0f; // 60FPS想定
		}
		time_ += config_.noiseSpeed * deltaTime;

		// パスを再生成して位置更新
		GeneratePath();
		UpdateVoxelPositions();
	}

	// 親クラスの更新
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
	pathPoints_.clear();

	// セグメント数が2未満の場合は始点と終点のみ
	if (config_.segmentCount < 2) {
		pathPoints_.push_back(config_.startPoint);
		pathPoints_.push_back(config_.endPoint);
		return;
	}

	// 始点→終点の方向
	Vector3 direction = config_.endPoint - config_.startPoint;
	Vector3 normalizedDir = MathCore::Vector::Normalize(direction);

	// 進行方向に垂直なベクトルを2つ作成
	Vector3 perpendicular1, perpendicular2;

	// Y軸方向の場合の特殊処理
	if (std::abs(normalizedDir.y) > 0.99f) {
		perpendicular1 = { 1.0f, 0.0f, 0.0f };
		perpendicular2 = { 0.0f, 0.0f, 1.0f };
	}
	else {
		// 通常の場合
		perpendicular1 = MathCore::Vector::Normalize(Vector3{ -normalizedDir.y, normalizedDir.x, 0.0f });
		perpendicular2 = MathCore::Vector::Normalize(MathCore::Vector::Cross(normalizedDir, perpendicular1));
	}

	// パスポイントを生成
	for (int i = 0; i <= config_.segmentCount; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(config_.segmentCount);

		// 基本位置（始点→終点の線形補間）
		Vector3 basePos = config_.startPoint + direction * t;

		// 始点と終点は完全固定（ノイズなし）
		if (i == 0 || i == config_.segmentCount) {
			pathPoints_.push_back(basePos);
			continue;
		}

		// 中間点：ParlineNoise2Dでずらす（X・Z両方向）
		// 周波数を上げて波形を荒くする（鋸歯状）
		float noiseFrequency = 15.0f;
		
		// 2Dノイズの入力座標（時間でアニメーション）
		float noiseInputX = t * noiseFrequency;
		float noiseInputY1 = time_; // X方向の時間オフセット
		float noiseInputY2 = time_ + 50.0f; // Z方向の時間オフセット

		// ParlineNoise2DでX・Z方向のノイズを取得
		float noiseX = GameUtils::ParlineNoise2D(noiseInputX, noiseInputY1);
		float noiseZ = GameUtils::ParlineNoise2D(noiseInputX, noiseInputY2);

		// ノイズを垂直方向に適用（X・Z両方向）
		Vector3 offset = perpendicular1 * noiseX * config_.noiseScale
			+ perpendicular2 * noiseZ * config_.noiseScale;

		// 端のフェードを正しく計算（始点と終点で確実に0になる）
		float edgeFade = 1.0f;
		if (t < 0.15f) {
			// 始点付近：0→1に線形補間
			edgeFade = t / 0.15f;
		}
		else if (t > 0.85f) {
			// 終点付近：1→0に線形補間
			edgeFade = (1.0f - t) / 0.15f;
		}
		
		offset = offset * edgeFade;

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
	finalVoxel->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
	AddChild(std::move(finalVoxel));
}

void Lightning::UpdateVoxelPositions()
{
	// パスポイントが不足または子オブジェクトが空の場合は何もしない
	if (pathPoints_.size() < 2 || children_.empty()) {
		return;
	}

	// 必要なボクセル総数を事前計算
	size_t requiredVoxels = 0;
	const float voxelSpacing = 0.3f; // PlaceVoxelsBetweenと同じ間隔（完全に隙間なし）
	
	for (size_t i = 0; i < pathPoints_.size() - 1; ++i) {
		Vector3 diff = pathPoints_[i + 1] - pathPoints_[i];
		float distance = MathCore::Vector::Length(diff);
		
		if (distance >= voxelSpacing * 0.1f) {
			int voxelCount = static_cast<int>(std::ceil(distance / voxelSpacing));
			requiredVoxels += voxelCount;
		}
	}
	
	// 最後のボクセル（終点）も数に含める
	requiredVoxels += 1;
	
	// ボクセル数が大きく異なる場合のみ再生成
	int voxelDiff = static_cast<int>(requiredVoxels) - static_cast<int>(children_.size());
	if (std::abs(voxelDiff) > 5) { // 許容範囲を広げる（密度が高いため）
		RequestRegeneration();
		return;
	}

	// ボクセルインデックス
	size_t voxelIndex = 0;

	// パスポイント間でボクセル位置を更新
	for (size_t i = 0; i < pathPoints_.size() - 1; ++i) {
		Vector3 start = pathPoints_[i];
		Vector3 end = pathPoints_[i + 1];
		Vector3 segmentDiff = end - start;
		float distance = MathCore::Vector::Length(segmentDiff);

		// 距離が短すぎる場合はスキップ
		if (distance < voxelSpacing * 0.1f) {
			continue;
		}

		// 必要なボクセル数
		int voxelCount = static_cast<int>(std::ceil(distance / voxelSpacing));

		// このセグメントのボクセル位置を更新
		for (int j = 0; j < voxelCount; ++j) {
			// ボクセルが足りない場合は抜ける
			if (voxelIndex >= children_.size()) {
				return;
			}

			// 位置計算
			float t = static_cast<float>(j) / static_cast<float>(voxelCount);
			Vector3 position = start + segmentDiff * t;

			// ボクセルの位置を更新
			if (auto* voxel = dynamic_cast<Voxel*>(children_[voxelIndex].get())) {
				voxel->GetTransform().translate = position;
			}

			voxelIndex++;
		}
	}
	
	// 最後のボクセル（終点）の位置を更新
	if (voxelIndex < children_.size()) {
		if (auto* voxel = dynamic_cast<Voxel*>(children_[voxelIndex].get())) {
			voxel->GetTransform().translate = pathPoints_.back();
		}
	}
}

void Lightning::PlaceVoxelsBetween(const Vector3& start, const Vector3& end)
{
	Vector3 diff = end - start;
	float distance = MathCore::Vector::Length(diff);

	// ボクセル配置間隔を極小にして完全に隙間をなくす
	const float voxelSpacing = 0.2f;

	// 距離が短すぎる場合は配置しない
	if (distance < voxelSpacing * 0.1f) {
		return;
	}

	// 必要なボクセル数（密に配置して完全に埋める）
	int voxelCount = static_cast<int>(std::ceil(distance / voxelSpacing));
	if (voxelCount < 1) {
		voxelCount = 1;
	}

	// ボクセルを配置（プールから再利用）
	for (int i = 0; i < voxelCount; ++i) {
		// 位置計算（完全に隙間なく配置）
		float t = static_cast<float>(i) / static_cast<float>(voxelCount);
		Vector3 position = start + diff * t;

		// プールからボクセルを取得（再利用）
		auto voxel = GetVoxelFromPool();
		voxel->GetTransform().translate = position;
		voxel->GetTransform().scale = { 1.0f, 1.0f, 1.0f };

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
		return voxel;
	}
	
	// プールが空の場合は新規作成
	auto voxel = std::make_unique<Voxel>();
	voxel->Initialize(voxelModel_, voxelTexture_); // モデルとテクスチャを渡す
	voxel->SetColor(config_.color); // 色を設定
	return voxel;
}

void Lightning::ReturnVoxelsToPool()
{
	// 現在のボクセルをプールに返却
	for (auto& child : children_) {
		if (auto* voxel = dynamic_cast<Voxel*>(child.get())) {
			voxelPool_.push_back(std::unique_ptr<Voxel>(voxel));
			child.release();
		}
	}
	children_.clear();
}
