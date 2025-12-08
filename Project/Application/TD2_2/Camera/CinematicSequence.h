#pragma once

#include "CameraController.h"
#include "Engine/Utility/Timer/GameTimer.h"
#include <vector>
#include <string>

/// @brief カット割り演出のカット情報
struct CinematicCut {
	CameraController::CinematicConfig config; ///< カットの演出設定
	float duration = 1.0f;                    ///< カットの継続時間
	std::string name = "Cut";                  ///< カット名（デバッグ用）
};

/// @brief カット割り演出シーケンス管理クラス
class CinematicSequence {
public:
	/// @brief コンストラクタ
	CinematicSequence() = default;

	/// @brief デストラクタ
	~CinematicSequence() = default;

	/// @brief カットを追加
	/// @param cut 追加するカット
	void AddCut(const CinematicCut& cut);

	/// @brief シーケンスを開始
	void Start();

	/// @brief シーケンスを停止
	void Stop();

	/// @brief 更新処理
	/// @param deltaTime デルタタイム
	void Update(float deltaTime);

	/// @brief シーケンスが実行中かどうか
	/// @return 実行中の場合true
	bool IsActive() const { return isActive_; }

	/// @brief 現在のカットインデックスを取得
	/// @return 現在のカットインデックス
	int GetCurrentCutIndex() const { return currentCutIndex_; }

	/// @brief カット数を取得
	/// @return カット数
	size_t GetCutCount() const { return cuts_.size(); }

	/// @brief 全体の進行度を取得
	/// @return 進行度（0.0～1.0）
	float GetProgress() const;

	/// @brief 現在のカットを取得
	/// @return 現在のカット（nullptrの場合あり）
	const CinematicCut* GetCurrentCut() const;

	/// @brief 指定インデックスのカットを取得
	/// @param index カットのインデックス
	/// @return カット（範囲外の場合nullptr）
	const CinematicCut* GetCutAt(int index) const;

	/// @brief すべてのカットを取得
	/// @return カットのリスト
	const std::vector<CinematicCut>& GetCuts() const { return cuts_; }

	/// @brief JSONから読み込み
	/// @param jsonPath JSONファイルのパス
	/// @return 読み込みに成功した場合true
	bool LoadFromJson(const std::string& jsonPath);

	/// @brief JSONに保存
	/// @param jsonPath JSONファイルのパス
	/// @return 保存に成功した場合true
	bool SaveToJson(const std::string& jsonPath) const;

	/// @brief カットをクリア
	void Clear();

private:
	std::vector<CinematicCut> cuts_;    ///< カットのリスト
	int currentCutIndex_ = -1;          ///< 現在のカットインデックス
	GameTimer cutTimer_;                ///< カット用タイマー
	bool isActive_ = false;             ///< シーケンスが実行中か
	float totalDuration_ = 0.0f;        ///< 全体の継続時間
	float elapsedTime_ = 0.0f;          ///< 経過時間
};
