#pragma once

#include "CameraController.h"
#include "CinematicSequence.h"
#include <map>
#include <string>
#include <memory>

/// @brief カメラ演出プリセット管理クラス
class CinematicPresetManager {
public:
	/// @brief シングルトンインスタンスを取得
	static CinematicPresetManager& GetInstance();

	/// @brief 演出プリセットを登録
	/// @param name 演出名
	/// @param config 演出設定
	void RegisterPreset(const std::string& name, const CameraController::CinematicConfig& config);

	/// @brief 演出プリセットを取得
	/// @param name 演出名
	/// @return 演出設定（見つからない場合はnullptr）
	const CameraController::CinematicConfig* GetPreset(const std::string& name) const;

	/// @brief シーケンスプリセットを登録
	/// @param name シーケンス名
	/// @param sequence シーケンス
	void RegisterSequence(const std::string& name, std::shared_ptr<CinematicSequence> sequence);

	/// @brief シーケンスプリセットを取得
	/// @param name シーケンス名
	/// @return シーケンス（見つからない場合はnullptr）
	std::shared_ptr<CinematicSequence> GetSequence(const std::string& name) const;

	/// @brief JSONファイルから演出プリセットを読み込み
	/// @param jsonPath JSONファイルのパス
	/// @return 読み込みに成功した場合true
	bool LoadPresetsFromJson(const std::string& jsonPath);

	/// @brief JSONファイルからシーケンスプリセットを読み込み
	/// @param jsonPath JSONファイルのパス
	/// @return 読み込みに成功した場合true
	bool LoadSequenceFromJson(const std::string& name, const std::string& jsonPath);

	/// @brief デフォルトプリセットを登録
	void RegisterDefaultPresets();

	/// @brief 登録されているプリセット名のリストを取得
	/// @return プリセット名のリスト
	std::vector<std::string> GetPresetNames() const;

	/// @brief 登録されているシーケンス名のリストを取得
	/// @return シーケンス名のリスト
	std::vector<std::string> GetSequenceNames() const;

	/// @brief プリセットが存在するかチェック
	/// @param name 演出名
	/// @return 存在する場合true
	bool HasPreset(const std::string& name) const;

	/// @brief シーケンスが存在するかチェック
	/// @param name シーケンス名
	/// @return 存在する場合true
	bool HasSequence(const std::string& name) const;

private:
	CinematicPresetManager() = default;
	~CinematicPresetManager() = default;
	CinematicPresetManager(const CinematicPresetManager&) = delete;
	CinematicPresetManager& operator=(const CinematicPresetManager&) = delete;

	std::map<std::string, CameraController::CinematicConfig> presets_;
	std::map<std::string, std::shared_ptr<CinematicSequence>> sequences_;
};
