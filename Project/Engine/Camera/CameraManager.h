#pragma once

#include "ICamera.h"
#include <memory>
#include <unordered_map>
#include <string>

/// @brief カメラマネージャー - 複数のカメラを管理して動的に切り替えるクラス
class CameraManager {
public:
	/// @brief コンストラクタ
	CameraManager() = default;

	/// @brief デストラクタ
	~CameraManager() = default;

	// コピー・ムーブを禁止
	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;
	CameraManager(CameraManager&&) = delete;
	CameraManager& operator=(CameraManager&&) = delete;

	/// @brief カメラを登録
	/// @param name カメラの名前
	/// @param camera 登録するカメラのユニークポインタ
	void RegisterCamera(const std::string& name, std::unique_ptr<ICamera> camera);

	/// @brief カメラを登録解除
	/// @param name カメラの名前
	void UnregisterCamera(const std::string& name);

	/// @brief アクティブカメラを設定
 /// @param name カメラの名前
	/// @return 設定に成功した場合true
	bool SetActiveCamera(const std::string& name);

	/// @brief アクティブカメラを取得
	  /// @return アクティブカメラのポインタ（存在しない場合nullptr）
	ICamera* GetActiveCamera() const;

	/// @brief 名前でカメラを取得
	/// @param name カメラの名前
	/// @return カメラのポインタ（存在しない場合nullptr）
	ICamera* GetCamera(const std::string& name) const;

	/// @brief アクティブカメラのビュー行列を取得
	/// @return ビュー行列
	const Matrix4x4& GetViewMatrix() const;

	/// @brief アクティブカメラのプロジェクション行列を取得
	/// @return プロジェクション行列
	const Matrix4x4& GetProjectionMatrix() const;

	/// @brief アクティブカメラの位置を取得
	/// @return カメラ位置
	Vector3 GetCameraPosition() const;

	/// @brief アクティブカメラを更新
	void Update();

	/// @brief 登録されているカメラの数を取得
	/// @return カメラの数
	size_t GetCameraCount() const { return cameras_.size(); }

	/// @brief アクティブカメラの名前を取得
	/// @return アクティブカメラの名前
	const std::string& GetActiveCameraName() const { return activeCameraName_; }

#ifdef _DEBUG
	/// @brief ImGuiデバッグウィンドウを描画
	void DrawImGui();
#endif

private:
	/// @brief カメラのコンテナ
	std::unordered_map<std::string, std::unique_ptr<ICamera>> cameras_;

	/// @brief アクティブなカメラの名前
	std::string activeCameraName_;

	/// @brief アクティブなカメラのポインタ（キャッシュ）
	ICamera* activeCamera_ = nullptr;
};
