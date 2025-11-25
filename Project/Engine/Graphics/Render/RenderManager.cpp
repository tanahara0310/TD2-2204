#include "RenderManager.h"
#include <IDrawable.h>
#include "Engine/Particle/ParticleSystem.h"
#include "Engine/Graphics/Render/Particle/ParticleRenderer.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include <algorithm>

void RenderManager::Initialize(ID3D12Device* device) {
	// 現時点では特に初期化処理なし
	(void)device; // 未使用警告を回避
}

void RenderManager::RegisterRenderer(RenderPassType type, std::unique_ptr<IRenderer> renderer) {
	renderers_[type] = std::move(renderer);
}

IRenderer* RenderManager::GetRenderer(RenderPassType type) {
	auto it = renderers_.find(type);
	if (it != renderers_.end()) {
		return it->second.get();
	}
	return nullptr;
}

void RenderManager::SetCameraManager(CameraManager* cameraManager) {
	cameraManager_ = cameraManager;
}

void RenderManager::SetCamera(const ICamera* camera) {
	camera_ = camera;
	
	// 各レンダラーにもカメラを設定（互換性維持）
	for (auto& [type, renderer] : renderers_) {
		renderer->SetCamera(camera);
	}
}

void RenderManager::SetCommandList(ID3D12GraphicsCommandList* cmdList) {
	cmdList_ = cmdList;
}

void RenderManager::AddDrawable(IDrawable* obj) {
	if (!obj || !obj->IsActive()) return;

	DrawCommand cmd;
	cmd.object = obj;
	cmd.passType = obj->GetRenderPassType();

	drawQueue_.push_back(cmd);
}

const ICamera* RenderManager::GetCameraForPass(RenderPassType passType) {
	// カメラマネージャーがある場合はタイプ別のカメラを取得
	if (cameraManager_) {
		// 2D描画パス（Sprite）は2Dカメラを使用
		if (passType == RenderPassType::Sprite) {
			return cameraManager_->GetActiveCamera(CameraType::Camera2D);
		}
		// その他は3Dカメラを使用
		else {
			return cameraManager_->GetActiveCamera(CameraType::Camera3D);
		}
	}
	
	// カメラマネージャーがない場合は従来のカメラを使用
	return camera_;
}

void RenderManager::DrawAll() {
	if (drawQueue_.empty() || !cmdList_) return;

	SortDrawQueue();

	RenderPassType currentPass = RenderPassType::Invalid;
	IRenderer* currentRenderer = nullptr;
	const ICamera* currentCamera = nullptr;

	for (const auto& cmd : drawQueue_) {
		if (!cmd.object->IsActive()) continue;

		// パスが切り替わったら処理
		if (cmd.passType != currentPass) {
			// 前のパスを終了
			if (currentRenderer) {
				currentRenderer->EndPass();
			}

			// 新しいパスを開始
			currentPass = cmd.passType;
			auto it = renderers_.find(currentPass);
			if (it != renderers_.end()) {
				currentRenderer = it->second.get();
				
				// パスに応じたカメラを取得
				currentCamera = GetCameraForPass(currentPass);
				currentRenderer->SetCamera(currentCamera);
				
				currentRenderer->BeginPass(cmdList_, cmd.object->GetBlendMode());
			} else {
				currentRenderer = nullptr;
				currentCamera = nullptr;
			}
		}

		// オブジェクトを描画
		if (currentRenderer) {
			// オブジェクトのDraw()でGPUデータを更新
			cmd.object->Draw(currentCamera);
			
			// パーティクルの場合は、レンダラーに描画コマンド発行を依頼
			if (cmd.passType == RenderPassType::Particle) {
				if (auto* particleRenderer = static_cast<ParticleRenderer*>(currentRenderer)) {
					auto* particleSystem = static_cast<ParticleSystem*>(cmd.object);
					particleRenderer->Draw(particleSystem);
				}
			}
		}
	}

	// 最後のパスを終了
	if (currentRenderer) {
		currentRenderer->EndPass();
	}
}

void RenderManager::ClearQueue() {
	drawQueue_.clear();
}

void RenderManager::SortDrawQueue() {
	// 描画パスタイプでソート（パイプライン切り替え最小化）
	std::sort(drawQueue_.begin(), drawQueue_.end(),
		[](const DrawCommand& a, const DrawCommand& b) {
			return static_cast<int>(a.passType) < static_cast<int>(b.passType);
		});
}
