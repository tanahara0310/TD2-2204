#pragma once
#include "Application/TD2_2/GameObject/GameObject.h"
#include "MathCore.h"
#include <vector>
#include <memory>
#include <string>

class Voxel;

/// @brief 雷エフェクトクラス
/// 始点から終点までパーリンノイズで揺らぐ電撃をボクセルで表現
class Lightning : public GameObject {
public:
	/// @brief 雷の設定
	struct Config {
		Vector3 startPoint = { 0.0f, 5.0f, 0.0f };  // 始点（固定）
		Vector3 endPoint = { 0.0f, 0.0f, 0.0f };    // 終点（固定）
		int segmentCount = 10;          // セグメント数（パス分割数）
		float noiseScale = 0.3f;        // ノイズの振幅
		float noiseSpeed = 1.0f;        // ノイズアニメーション速度
		bool enableAnimation = true;    // アニメーション有効
		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 雷の色（RGBA）
	};

	Lightning() = default;
	~Lightning() override = default;

	/// @brief 初期化
	/// @param voxelModel ボクセルモデル（シーン側で生成）
	/// @param voxelTexture ボクセルテクスチャ（シーン側で読み込み）
	/// @param config 雷の設定
	/// @param name 識別名（ImGui ID用）
	void Initialize(ModelResource* voxelModel, TextureManager::LoadedTexture voxelTexture,
		const Config& config, const std::string& name = "Lightning");

	/// @brief 更新
	void Update() override;

	/// @brief 描画
	void Draw(const ICamera* camera) override;

	/// @brief ImGui表示
	bool DrawImGui() override;

	/// @brief オブジェクト名
	const char* GetObjectName() const override { return name_.c_str(); }

	/// @brief 設定取得
	Config& GetConfig() { return config_; }
	const Config& GetConfig() const { return config_; }

private:
	/// @brief パスポイントを生成
	void GeneratePath();

	/// @brief ボクセルを生成・配置（プールから再利用）
	void GenerateVoxels();

	/// @brief 既存ボクセルの位置を更新（アニメーション用）
	void UpdateVoxelPositions();

	/// @brief 2点間にボクセルを配置（プールから取得）
	void PlaceVoxelsBetween(const Vector3& start, const Vector3& end);

	/// @brief 再生成をリクエスト（遅延実行）
	void RequestRegeneration();

	/// @brief 遅延削除されたボクセルをクリア
	void ClearDeferredDeletions();

	/// @brief ボクセルをプールから取得（なければ新規作成）
	std::unique_ptr<Voxel> GetVoxelFromPool();

	/// @brief 不要なボクセルをプールに返却
	void ReturnVoxelsToPool();

	Config config_;                     // 設定
	std::vector<Vector3> pathPoints_;   // パスポイント（始点→終点の経路）
	float time_ = 0.0f;                 // アニメーション時間
	bool needsRegeneration_ = false;    // 再生成フラグ
	std::string name_ = "Lightning";    // 識別名（ImGui ID用）
	
	// シーン側から渡されるリソース
	ModelResource* voxelModel_ = nullptr;
	TextureManager::LoadedTexture voxelTexture_;
	
	// GPU安全な遅延削除用（次フレームまで保持）
	std::vector<std::unique_ptr<IDrawable>> deferredDeletions_;
	
	// ボクセルプール（再利用用）
	std::vector<std::unique_ptr<Voxel>> voxelPool_;
};
