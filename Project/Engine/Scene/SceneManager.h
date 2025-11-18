#pragma once

#include "IScene.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

class EngineSystem; // 前方宣言

class SceneManager {
public:

	void Initialize(EngineSystem* engine);

	/// @brief シーンを登録する
	/// @tparam T シーンクラス（ISceneを継承している必要がある）
	/// @param name シーン名
	template<typename T>
	void RegisterScene(const std::string& name) {
		static_assert(std::is_base_of<IScene, T>::value);
		sceneFactories_[name] = []() { return std::make_unique<T>(); };
	}

	/// @brief シーンを変える関数
	/// @param name 変更先のシーン名
	void ChangeScene(std::string name);

	/// @brief 更新処理
	void Update();

	/// @brief 描画処理
	void Draw();

	/// @brief シーンの終了処理
	void Finalize();

	/// @brief 指定した名前のシーンが存在するか確認する関数
	/// @param name シーン名
	/// @return true: 存在する, false: 存在しない
	bool HasScene(const std::string& name) const;

	/// @brief 現在のシーン名を取得
	/// @return 現在のシーン名（シーンが無い場合は"None"）
	std::string GetCurrentSceneName() const;

	/// @brief 登録されているすべてのシーン名を取得
	/// @return シーン名のリスト
	std::vector<std::string> GetAllSceneNames() const;

private:
	std::unordered_map<std::string, std::function<std::unique_ptr<IScene>()>> sceneFactories_;

	std::unique_ptr<IScene> currentScene_ = nullptr;
	std::string currentSceneName_ = "None"; // 現在のシーン名を保持

	//エンジンクラスのポインタ
	EngineSystem* engine_ = nullptr; // エンジンシステムへのポインタ

	// ──────────────────────────────────────────────────────────
	// 遅延シーン切り替え用
	// ──────────────────────────────────────────────────────────
	/// @brief 次のシーン名（遅延切り替え用）
	std::string nextSceneName_;

	/// @brief シーン切り替えリクエストフラグ
	bool isSceneChangeRequested_ = false;

	/// @brief 実際のシーン切り替えを実行（内部関数）
	/// @param name 変更先のシーン名
	void DoChangeScene(const std::string& name);
};
