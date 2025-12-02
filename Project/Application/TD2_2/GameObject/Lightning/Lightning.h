#pragma once
#include "Object3d.h"
#include "MathCore.h"
#include <vector>
#include <memory>

// 前方宣言
class Voxel;

/// @brief 雷の表現クラス
/// 始点から終点までパーリンノイズを使って不規則に曲がる雷を、Voxelで隙間なく描画
class Lightning : public Object3d {
public:
	/// @brief 雷の設定データ
	struct LightningConfig {
		Vector3 startPoint = { 0.0f, 5.0f, 0.0f };   // 始点
		Vector3 endPoint = { 0.0f, 0.0f, 0.0f };     // 終点
		float noiseStrength = 0.3f;     // ノイズの強度
		float noiseFrequency = 1.5f;     // ノイズの周波数
		int segmentCount = 10;  // セグメント数（パフォーマンス重視で10に削減）
		float animationSpeed = 1.0f;    // アニメーション速度
		bool animate = true;        // アニメーションするか
	};

	/// @brief デフォルトコンストラクタ
	Lightning() = default;

	/// @brief デストラクタ
	~Lightning() override = default;

	/// @brief 初期化
	/// @param config 雷の設定
	void Initialize(const LightningConfig& config = LightningConfig());

	/// @brief 更新処理
	void Update() override;

	/// @brief 描画処理
	/// @param camera カメラ
	void Draw(const ICamera* camera) override;

	/// @brief ImGuiデバッグUI描画
	/// @return ImGuiで変更があった場合true
	bool DrawImGui() override;

	/// @brief オブジェクト名を取得
	/// @return オブジェクト名
	const char* GetObjectName() const override { return "Lightning"; }

	/// @brief 雷の設定を取得
	/// @return 雷の設定への参照
	LightningConfig& GetConfig() { return config_; }

	/// @brief 雷の設定を取得（const版）
	/// @return 雷の設定への参照
	const LightningConfig& GetConfig() const { return config_; }

	/// @brief 雷のパスを再生成
	void RegeneratePath();

private:
	/// @brief パーリンノイズ関数
	/// @param x X座標
	/// @param y Y座標
	/// @param z Z座標
	/// @return ノイズ値 (-1.0 ～ 1.0)
	float PerlinNoise3D(float x, float y, float z);

	/// @brief フェード関数（スムーズステップ）
	/// @param t 入力値 (0.0 - 1.0)
	/// @return フェード値
	float Fade(float t);

	/// @brief 線形補間
	/// @param a 開始値
	/// @param b 終了値
	/// @param t 補間係数
	/// @return 補間値
	float Lerp(float a, float b, float t);

	/// @brief グラディエント関数
	/// @param hash ハッシュ値
	/// @param x X座標
	/// @param y Y座標
	/// @param z Z座標
	/// @return グラディエント値
	float Gradient(int hash, float x, float y, float z);

	/// @brief 2点間のボクセルで補間
	/// @param start 開始点
	/// @param end 終了点
	/// @return 補間に必要なボクセル数
	int InterpolateVoxels(const Vector3& start, const Vector3& end);

	/// @brief 雷のセグメントを構築
	void BuildLightningSegments();

	/// @brief 既存ボクセルの位置を更新（アニメーション用・最適化版）
	void UpdateVoxelPositions();

	/// @brief 既存ボクセルの位置のみを更新
	void UpdateExistingVoxelPositions();

	// メンバ変数
	LightningConfig config_;    // 雷の設定
	std::vector<Vector3> pathPoints_; // 雷のパスポイント
	float animationTime_ = 0.0f;          // アニメーション時間
	int previousSegmentCount_ = 0;  // 前フレームのセグメント数（変更検知用）
	
	// 注: ボクセルは Object3d::children_ に格納されます
};
