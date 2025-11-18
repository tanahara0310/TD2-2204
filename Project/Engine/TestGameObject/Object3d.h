#pragma once

#include <memory>
#include <d3d12.h>
#include <vector>

#include "IDrawable.h"
#include <EngineSystem.h>
#include "Engine/Graphics/Render/RenderPassType.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Graphics/LineRenderer.h"

// 前方宣言
class ICamera;

/// @brief 3Dゲームオブジェクトの基底クラス
class Object3d : public IDrawable {
public:
	/// @brief デフォルトコンストラクタ
	Object3d() = default;

	/// @brief 仮想デストラクタ
	virtual ~Object3d() = default;

	/// @brief 初期化処理（派生クラスでオーバーライド必須）
	/// @param engine エンジンシステムへのポインタ
	virtual void Initialize(EngineSystem* engine) = 0;

	/// @brief 更新処理（派生クラスでオーバーライド可能）
	virtual void Update();

	/// @brief 描画処理（3D専用 - カメラ必須）
	/// @param camera カメラ
	virtual void Draw(ICamera* camera);

	/// @brief デバッグ描画処理（派生クラスでオーバーライド可能）
	/// @param outLines ライン配列の出力先
	virtual void DrawDebug(std::vector<LineRenderer::Line>& outLines);

	/// @brief ImGuiデバッグUI描画（派生クラスでオーバーライド可能）
	/// @return ImGuiで変更があった場合true
	bool DrawImGui() override;

	/// @brief オブジェクト名を取得（派生クラスでオーバーライド推奨）
	/// @return オブジェクト名
	const char* GetObjectName() const override { return "Object3D"; }
	
	/// @brief 3Dオブジェクトであることを示す
	bool Is2D() const override { return false; }

	/// @brief トランスフォームを取得
	/// @return トランスフォーム参照
	WorldTransform& GetTransform() { return transform_; }

	/// @brief トランスフォームを取得（const版）
	/// @return トランスフォーム参照
	const WorldTransform& GetTransform() const { return transform_; }

	/// @brief モデルを取得
	/// @return モデルポインタ
	Model* GetModel() { return model_.get(); }

	/// @brief モデルを取得（const版）
	/// @return モデルポインタ
	const Model* GetModel() const { return model_.get(); }

	/// @brief このオブジェクトの描画タイプを取得
	/// @return 描画タイプ（モデルがない場合はNormal）
	Model::RenderType GetRenderType() const {
		return model_ ? model_->GetRenderType() : Model::RenderType::Normal;
	}

	/// @brief このオブジェクトの描画パスタイプを取得
	/// @return 描画パスタイプ
	RenderPassType GetRenderPassType() const override;

protected:
	/// @brief モデルインスタンス
	std::unique_ptr<Model> model_;

	/// @brief ワールドトランスフォーム
	WorldTransform transform_;
	
	/// @brief テクスチャハンドル（派生クラスで使用可能）
	TextureManager::LoadedTexture texture_;
};
