#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include <vector>
#include <optional>

#include "ModelResource.h"
#include "Engine/WorldTransfom/WorldTransform.h"
#include "Engine/Graphics/Material/MaterialManager.h"
#include "Engine/Graphics/Structs/TransformationMatrix.h"
#include "Engine/Graphics/Structs/SkinCluster.h"
#include "Animation/IAnimationController.h"
#include "Skeleton/Skeleton.h"

class ICamera;
class DirectXCommon;
class ResourceFactory;
class LightBase;

/// @brief 配置された3Dモデルのインスタンスクラス
/// ModelResourceへの参照と、個別のトランスフォーム・マテリアルを持つ
class Model {
public:
	/// @brief モデルの描画タイプ
	enum class RenderType {
		Normal,   // 通常モデル
		Skinning  // スキニングモデル
	};

	/// @brief デフォルトコンストラクタ
	Model() = default;

	/// @brief デストラクタ
	~Model() = default;

	/// @brief 静的初期化（全Modelインスタンス共通のリソースを初期化）
	/// @param dxCommon DirectXCommonのポインタ
	/// @param factory リソースファクトリのポインタ
	static void Initialize(DirectXCommon* dxCommon, ResourceFactory* factory);

	/// @brief 初期化（アニメーションコントローラーなし）
	/// @param resource 共有するModelResourceのポインタ
	void Initialize(ModelResource* resource);

	/// @brief 初期化（アニメーションコントローラーあり）
	/// @param resource 共有するModelResourceのポインタ
	/// @param controller アニメーションコントローラー
	void Initialize(ModelResource* resource, std::unique_ptr<IAnimationController> controller);

	/// @brief モデルリソースを切り替える
	/// @param resource 新しいModelResourceのポインタ
	void ChangeModelResource(ModelResource* resource);

	/// @brief モデルを描画（スキニングモデルか通常モデルかは内部で自動判別）
	/// @param transform ワールドトランスフォーム
	/// @param camera カメラ（ICamera インターフェース）
	/// @param textureHandle テクスチャハンドル
	void Draw(const WorldTransform& transform, const ICamera* camera,
		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	/// @brief 初期化されているか確認
	/// @return 初期化済みならtrue
	bool IsInitialized() const { return resource_ != nullptr && materialManager_ != nullptr; }

	/// @brief マテリアルマネージャーを取得
	/// @return マテリアルマネージャー
	MaterialManager* GetMaterialManager() { return materialManager_.get(); }
	const MaterialManager* GetMaterialManager() const { return materialManager_.get(); }

	/// @brief UV変換行列を設定
	/// @param uvTransform UV変換行列
	void SetUVTransform(const Matrix4x4& uvTransform);

	/// @brief UV変換行列を取得
	/// @return UV変換行列
	Matrix4x4 GetUVTransform() const;

	/// @brief Skeletonを取得（スケルトンアニメーションから同期）
	/// @return Skeleton（存在しない場合はnullopt）
	const std::optional<Skeleton>& GetSkeleton() const { return skeleton_; }

	/// @brief SkinClusterを持っているか確認
	/// @return SkinClusterがあればtrue
	bool HasSkinCluster() const { return skinCluster_.has_value(); }

	/// @brief アニメーションコントローラーを持っているか確認
	/// @return コントローラーがあればtrue
	bool HasAnimationController() const { return animationController_ != nullptr; }

	/// @brief アニメーションを更新
	/// @param deltaTime デルタタイム（秒）
	void UpdateAnimation(float deltaTime);

	/// @brief アニメーションをリセット
	void ResetAnimation();

	/// @brief アニメーション時刻を取得
	/// @return 現在のアニメーション時刻（秒）
	float GetAnimationTime() const;

	/// @brief アニメーションが終了したか確認
	/// @return アニメーションが終了していればtrue
	bool IsAnimationFinished() const;

	/// @brief 描画タイプを取得（スキニングか通常かを判別）
	/// @return 描画タイプ
	RenderType GetRenderType() const { 
		return HasSkinCluster() ? RenderType::Skinning : RenderType::Normal; 
	}

private:
	// 参照するModelResource
	ModelResource* resource_ = nullptr;
	
	// インスタンス固有のマテリアル
	std::unique_ptr<MaterialManager> materialManager_;
	
	// WVP行列用のリソース（1インスタンスにつき1つのみ）
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	
	// Skeleton（スケルトンアニメーターから同期される）
	std::optional<Skeleton> skeleton_;
	
	// SkinCluster（存在する場合）
	std::optional<SkinCluster> skinCluster_;
	
	// アニメーションコントローラー
	std::unique_ptr<IAnimationController> animationController_;

	// 内部ヘルパーメソッド
	/// @brief WVP行列データを更新
	void UpdateTransformationMatrix(const WorldTransform& transform, const ICamera* camera);

	/// @brief SkinClusterを更新（スケルトンアニメーションの場合のみ）
	void UpdateSkinCluster();

	/// @brief 通常モデルの描画コマンドを設定
	void SetupNormalDrawCommands(ID3D12GraphicsCommandList* cmdList,
		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	/// @brief スキニングモデルの描画コマンドを設定
	void SetupSkinningDrawCommands(ID3D12GraphicsCommandList* cmdList,
		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);
};