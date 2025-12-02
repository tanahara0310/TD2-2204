#include "Lightning.h"
#include "Application/TD2_2/GameObject/Voxel/Voxel.h"
#include "EngineSystem/EngineSystem.h"
#include <cmath>
#include <string>

#ifdef _DEBUG
#include <imgui.h>
#include <Windows.h>  // OutputDebugStringA用
#endif

// Perlin Noiseの順列テーブル（Ken Perlinのオリジナル実装を参考）
static const int permutation[512] = {
	151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
	8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
	35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
	134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
	55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
	18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
	250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
	189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
	172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
	228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
	107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	// 繰り返し
	151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
	8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
	35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
	134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
	55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
	18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
	250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
	189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
	172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
	228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
	107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

void Lightning::Initialize(const LightningConfig& config)
{
	config_ = config;
	previousSegmentCount_ = config_.segmentCount; // 初期値を設定

#ifdef _DEBUG
	OutputDebugStringW(L"[INFO] Lightning::Initialize - 開始\n");

	std::wstring msg = L"  始点: (" + std::to_wstring(config_.startPoint.x) + L", " +
		std::to_wstring(config_.startPoint.y) + L", " +
		std::to_wstring(config_.startPoint.z) + L")\n";
	OutputDebugStringW(msg.c_str());

	msg = L"  終点: (" + std::to_wstring(config_.endPoint.x) + L", " +
		std::to_wstring(config_.endPoint.y) + L", " +
		std::to_wstring(config_.endPoint.z) + L")\n";
	OutputDebugStringW(msg.c_str());

	msg = L"  セグメント数: " + std::to_wstring(config_.segmentCount) + L"\n";
	OutputDebugStringW(msg.c_str());

	// IsActive状態を確認
	msg = L"  IsActive: " + std::wstring(IsActive() ? L"true" : L"false") + L"\n";
	OutputDebugStringW(msg.c_str());
#endif

	// 雷のパスを生成
	RegeneratePath();

#ifdef _DEBUG
	std::wstring finalMsg = L"[INFO] Lightning::Initialize - 完了。ボクセル数: " + std::to_wstring(children_.size()) + L"\n";
	OutputDebugStringW(finalMsg.c_str());
#endif
}

void Lightning::Update()
{
	// セグメント数が変更されたかチェック
	bool segmentCountChanged = (previousSegmentCount_ != config_.segmentCount);

	if (segmentCountChanged) {
		// セグメント数が変更された場合は即座に完全再生成
		RegeneratePath();
		previousSegmentCount_ = config_.segmentCount;
	} else if (config_.animate) {
		// セグメント数が同じ場合のみアニメーション処理
		animationTime_ += config_.animationSpeed * (1.0f / 60.0f);
		UpdateVoxelPositions();
	}

	// Object3d::Update()を呼び出すことで、children_の更新も自動的に行われる
	Object3d::Update();
}

void Lightning::Draw(const ICamera* camera)
{
	if (!camera) {
#ifdef _DEBUG
		OutputDebugStringW(L"[WARNING] Lightning::Draw: カメラがnullptrです\n");
#endif
		return;
	}

#ifdef _DEBUG
	static int drawCallCount = 0;
	if (drawCallCount < 5) {  // 最初の5フレームだけ出力
		std::wstring msg = L"[INFO] Lightning::Draw - 子オブジェクト数: " + std::to_wstring(children_.size()) + L"\n";
		OutputDebugStringW(msg.c_str());
		drawCallCount++;
	}
#endif

	// Object3d::Draw()を呼び出すことで、children_の描画も自動的に行われる
	Object3d::Draw(camera);
}

bool Lightning::DrawImGui()
{
#ifdef _DEBUG
	bool changed = false;

	if (ImGui::CollapsingHeader("Lightning")) {
		ImGui::PushID("Lightning");

		// 基本設定
		ImGui::SeparatorText("基本設定");

		changed |= ImGui::DragFloat3("始点", &config_.startPoint.x, 0.1f);
		changed |= ImGui::DragFloat3("終点", &config_.endPoint.x, 0.1f);

		// セグメント数の変更（Update()で自動的に検知・再生成される）
		changed |= ImGui::SliderInt("セグメント数", &config_.segmentCount, 5, 200);

		ImGui::SeparatorText("ノイズ設定");

		changed |= ImGui::DragFloat("ノイズ強度", &config_.noiseStrength, 0.01f, 0.0f, 5.0f);
		ImGui::TextDisabled("横方向のずれの大きさ");

		changed |= ImGui::DragFloat("ノイズ周波数", &config_.noiseFrequency, 0.01f, 0.01f, 5.0f);
		ImGui::TextDisabled("ノイズの細かさ（高いほど細かい揺らぎ）");

		ImGui::SeparatorText("アニメーション");

		changed |= ImGui::Checkbox("アニメーション", &config_.animate);
		changed |= ImGui::DragFloat("アニメーション速度", &config_.animationSpeed, 0.1f, 0.0f, 10.0f);

		ImGui::Separator();

		// パス再生成ボタン
		if (ImGui::Button("パスを再生成")) {
			RegeneratePath();
			changed = true;
		}

		ImGui::Text("ボクセル数: %zu", children_.size());
		ImGui::Text("パスポイント数: %zu", pathPoints_.size());
		ImGui::Text("現在のセグメント数: %d", previousSegmentCount_);

		// デバッグ情報：最初のいくつかのボクセル位置を表示
		if (children_.size() > 0) {
			ImGui::Separator();
			ImGui::Text("ボクセル位置（最初の5個）:");
			int displayCount = (int)children_.size() < 5 ? (int)children_.size() : 5;
			for (int i = 0; i < displayCount; ++i) {
				if (auto* voxel = dynamic_cast<Voxel*>(children_[i].get())) {
					Vector3 pos = voxel->GetTransform().translate;
					ImGui::Text("[%d] (%.2f, %.2f, %.2f)", i, pos.x, pos.y, pos.z);
				}
			}
		}

		ImGui::PopID();
	}

	return changed;
#else
	return false;
#endif
}

void Lightning::RegeneratePath()
{
	// セグメント数の妥当性チェック
	if (config_.segmentCount < 1) {
		config_.segmentCount = 1;
	}
	if (config_.segmentCount > 500) {
		config_.segmentCount = 500;
	}

	// 前回のセグメント数を更新
	previousSegmentCount_ = config_.segmentCount;

	// パスポイントをクリア
	pathPoints_.clear();

	// 始点から終点への方向ベクトル
	Vector3 direction = config_.endPoint - config_.startPoint;
	Vector3 normalizedDir = MathCore::Vector::Normalize(direction);

	// セグメント単位で進む
	for (int i = 0; i <= config_.segmentCount; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(config_.segmentCount);

		// 基本的な線形補間位置
		Vector3 basePos = config_.startPoint + direction * t;

		// 始点（i=0）と終点（i=segmentCount）はオフセットなしで固定
		if (i == 0 || i == config_.segmentCount) {
			pathPoints_.push_back(basePos);
			continue;
		}

		// 中間点のみパーリンノイズを使って横方向にずらす
		float noiseX = PerlinNoise3D(
			t * config_.noiseFrequency + animationTime_,
			0.0f,
			animationTime_ * 0.5f
		);
		float noiseZ = PerlinNoise3D(
			t * config_.noiseFrequency + animationTime_ + 100.0f,
			100.0f,
			animationTime_ * 0.5f
		);

		// 進行方向に垂直なベクトルを作成（簡易的に）
		Vector3 perpendicular1 = { -normalizedDir.y, normalizedDir.x, 0.0f };
		Vector3 perpendicular2 = MathCore::Vector::Cross(normalizedDir, perpendicular1);

		// 垂直方向が0の場合の対処
		if (MathCore::Vector::Length(perpendicular1) < 0.001f) {
			perpendicular1 = { 1.0f, 0.0f, 0.0f };
			perpendicular2 = { 0.0f, 0.0f, 1.0f };
		} else {
			perpendicular1 = MathCore::Vector::Normalize(perpendicular1);
			perpendicular2 = MathCore::Vector::Normalize(perpendicular2);
		}

		// ノイズによるオフセットを適用
		Vector3 offset = perpendicular1 * noiseX * config_.noiseStrength +
			perpendicular2 * noiseZ * config_.noiseStrength;

		// 端に近いほど揺らぎを抑える（自然な接続のため）
		float edgeFade = std::sin(t * 3.14159265f);
		offset = offset * edgeFade;

		Vector3 finalPos = basePos + offset;
		pathPoints_.push_back(finalPos);
	}

	// パスポイント間をボクセルで補間
	BuildLightningSegments();
}

void Lightning::BuildLightningSegments()
{
	// 子オブジェクト（children_）をクリア
	children_.clear();

	if (pathPoints_.size() < 2) {
#ifdef _DEBUG
		std::wstring msg = L"[WARNING] Lightning::BuildLightningSegments: パスポイントが不足しています（" +
			std::to_wstring(pathPoints_.size()) + L"個）\n";
		OutputDebugStringW(msg.c_str());
#endif
		return;
	}

#ifdef _DEBUG
	std::wstring msg = L"[INFO] Lightning::BuildLightningSegments - パスポイント数: " +
		std::to_wstring(pathPoints_.size()) + L"\n";
	OutputDebugStringW(msg.c_str());
#endif

	// 各パスポイント間をボクセルで埋める
	int totalVoxels = 0;
	for (size_t i = 0; i < pathPoints_.size() - 1; ++i) {
		int count = InterpolateVoxels(pathPoints_[i], pathPoints_[i + 1]);
		totalVoxels += count;
	}

#ifdef _DEBUG
	msg = L"[INFO] Lightning::BuildLightningSegments - 生成されたボクセル総数: " +
		std::to_wstring(totalVoxels) + L" (children_.size()=" +
		std::to_wstring(children_.size()) + L")\n";
	OutputDebugStringW(msg.c_str());
#endif
}

int Lightning::InterpolateVoxels(const Vector3& start, const Vector3& end)
{
	// 2点間の距離を計算
	Vector3 diff = end - start;
	float distance = MathCore::Vector::Length(diff);

	// ボクセル1個分のサイズを大きくして生成数を大幅削減（0.3f → 0.8f）
	const float voxelSize = 1.0f;

	// 距離が短すぎる場合はスキップ
	if (distance < voxelSize * 0.5f) {
		return 0;
	}

	// 必要なボクセル数を計算
	int voxelCount = static_cast<int>(std::ceil(distance / voxelSize));
	if (voxelCount < 1) {
		voxelCount = 1;
	}

	// ボクセルを配置
	for (int i = 0; i < voxelCount; ++i) {
		// 均等に配置
		float t = static_cast<float>(i) / static_cast<float>(voxelCount);
		Vector3 position = start + diff * t;

		// ボクセルを作成
		auto voxel = std::make_unique<Voxel>();
		voxel->Initialize();

		// 位置を設定
		voxel->GetTransform().translate = position;

		// スケールは1.0
		voxel->GetTransform().scale = { 1.0f, 1.0f, 1.0f };

		// 回転処理は削除（不要）

		// children_に追加
		AddChild(std::move(voxel));
	}

	return voxelCount;
}

// ========================================
// パーリンノイズ実装
// ========================================

float Lightning::PerlinNoise3D(float x, float y, float z)
{
	// 単位立方体の座標を計算
	int X = static_cast<int>(std::floor(x)) & 255;
	int Y = static_cast<int>(std::floor(y)) & 255;
	int Z = static_cast<int>(std::floor(z)) & 255;

	// 立方体内の相対座標
	x -= std::floor(x);
	y -= std::floor(y);
	z -= std::floor(z);

	// フェードカーブを計算
	float u = Fade(x);
	float v = Fade(y);
	float w = Fade(z);

	// ハッシュ座標計算
	int A = permutation[X] + Y;
	int AA = permutation[A] + Z;
	int AB = permutation[A + 1] + Z;
	int B = permutation[X + 1] + Y;
	int BA = permutation[B] + Z;
	int BB = permutation[B + 1] + Z;

	// 8つの角のグラディエントを補間
	float gradAA = Gradient(permutation[AA], x, y, z);
	float gradBA = Gradient(permutation[BA], x - 1.0f, y, z);
	float gradAB = Gradient(permutation[AB], x, y - 1.0f, z);
	float gradBB = Gradient(permutation[BB], x - 1.0f, y - 1.0f, z);
	float gradAA1 = Gradient(permutation[AA + 1], x, y, z - 1.0f);
	float gradBA1 = Gradient(permutation[BA + 1], x - 1.0f, y, z - 1.0f);
	float gradAB1 = Gradient(permutation[AB + 1], x, y - 1.0f, z - 1.0f);
	float gradBB1 = Gradient(permutation[BB + 1], x - 1.0f, y - 1.0f, z - 1.0f);

	// 補間を実行
	float x1 = Lerp(gradAA, gradBA, u);
	float x2 = Lerp(gradAB, gradBB, u);
	float y1 = Lerp(x1, x2, v);

	float x3 = Lerp(gradAA1, gradBA1, u);
	float x4 = Lerp(gradAB1, gradBB1, u);
	float y2 = Lerp(x3, x4, v);

	return Lerp(y1, y2, w);
}

float Lightning::Fade(float t)
{
	// 6t^5 - 15t^4 + 10t^3
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Lightning::Lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

float Lightning::Gradient(int hash, float x, float y, float z)
{
	// 下位4ビットを使用してグラディエント方向を決定
	int h = hash & 15;
	float u = h < 8 ? x : y;
	float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

void Lightning::UpdateVoxelPositions()
{
	// パスポイントを再計算（始点・終点は固定）
	pathPoints_.clear();

	Vector3 direction = config_.endPoint - config_.startPoint;
	Vector3 normalizedDir = MathCore::Vector::Normalize(direction);

	// パスポイントを生成
	for (int i = 0; i <= config_.segmentCount; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(config_.segmentCount);
		Vector3 basePos = config_.startPoint + direction * t;

		// 始点（t=0）と終点（t=1）はオフセットなし
		if (i == 0 || i == config_.segmentCount) {
			pathPoints_.push_back(basePos);
			continue;
		}

		// 中間点のみノイズでオフセット
		float noiseX = PerlinNoise3D(t * config_.noiseFrequency + animationTime_, 0.0f, animationTime_ * 0.5f);
		float noiseZ = PerlinNoise3D(t * config_.noiseFrequency + animationTime_ + 100.0f, 100.0f, animationTime_ * 0.5f);

		Vector3 perpendicular1 = { -normalizedDir.y, normalizedDir.x, 0.0f };
		Vector3 perpendicular2 = MathCore::Vector::Cross(normalizedDir, perpendicular1);

		if (MathCore::Vector::Length(perpendicular1) < 0.001f) {
			perpendicular1 = { 1.0f, 0.0f, 0.0f };
			perpendicular2 = { 0.0f, 0.0f, 1.0f };
		} else {
			perpendicular1 = MathCore::Vector::Normalize(perpendicular1);
			perpendicular2 = MathCore::Vector::Normalize(perpendicular2);
		}

		Vector3 offset = perpendicular1 * noiseX * config_.noiseStrength + perpendicular2 * noiseZ * config_.noiseStrength;

		// edgeFadeは中間点では1に近い値、端に近づくほど0になるように調整
		float edgeFade = std::sin(t * 3.14159265f);
		offset = offset * edgeFade;

		pathPoints_.push_back(basePos + offset);
	}

	// 既存のボクセルの位置を更新（再生成しない！）
	UpdateExistingVoxelPositions();
}

void Lightning::UpdateExistingVoxelPositions()
{
	if (children_.empty() || pathPoints_.size() < 2) {
		return;
	}

	const float voxelSize = 1.0f;
	size_t voxelIndex = 0;

	// 各パスポイント間でボクセル位置を更新
	for (size_t i = 0; i < pathPoints_.size() - 1; ++i) {
		Vector3 diff = pathPoints_[i + 1] - pathPoints_[i];
		float distance = MathCore::Vector::Length(diff);

		if (distance < voxelSize * 0.5f) {
			continue;
		}

		int voxelCount = static_cast<int>(std::ceil(distance / voxelSize));
		if (voxelCount < 1) voxelCount = 1;

		// このセグメントのボクセル位置を更新
		for (int j = 0; j < voxelCount; ++j) {
			// ボクセルが足りない場合は抜ける（次フレームで再生成される）
			if (voxelIndex >= children_.size()) {
#ifdef _DEBUG
				OutputDebugStringW(L"[WARNING] UpdateExistingVoxelPositions: ボクセル不足 - 次フレームで再生成\n");
#endif
				return;
			}

			float t = static_cast<float>(j) / static_cast<float>(voxelCount);
			Vector3 position = pathPoints_[i] + diff * t;

			// ボクセルの位置のみ更新
			if (auto* voxel = dynamic_cast<Voxel*>(children_[voxelIndex].get())) {
				voxel->GetTransform().translate = position;
			}
			voxelIndex++;
		}
	}

	// ボクセルが余っている場合も次フレームで再生成
	if (voxelIndex < children_.size()) {
#ifdef _DEBUG
		OutputDebugStringW(L"[WARNING] UpdateExistingVoxelPositions: ボクセル余剰 - 次フレームで再生成\n");
#endif
	}
}
