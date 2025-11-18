#include "RenderManager.h"
#include "Engine/TestGameObject/IDrawable.h"
#include "Engine/TestGameObject/Object2d.h"
#include "Engine/TestGameObject/Object3d.h"
#include "Engine/Particle/ParticleSystem.h"
#include "Engine/Graphics/Render/Particle/ParticleRenderer.h"
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

void RenderManager::SetCamera(const ICamera* camera) {
	camera_ = camera;
	
	// 各レンダラーにもカメラを設定（互換性のため）
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

void RenderManager::DrawAll() {
	if (drawQueue_.empty() || !cmdList_) return;

	// 描画パスごとにソート
	SortDrawQueue();

	// 描画パスごとにまとめて描画
	RenderPassType currentPass = RenderPassType::Invalid;

	for (const auto& cmd : drawQueue_) {
		// パスが切り替わったら、前のパスを終了して新しいパスを開始
		if (cmd.passType != currentPass) {
			// 前のパスを終了
			if (currentPass != RenderPassType::Invalid) {
				auto it = renderers_.find(currentPass);
				if (it != renderers_.end()) {
					it->second->EndPass();
				}
			}

			// 新しいパスを開始
			currentPass = cmd.passType;
			auto it = renderers_.find(currentPass);
			if (it != renderers_.end()) {
				// パーティクルの場合はブレンドモードを取得して渡す
				BlendMode blendMode = BlendMode::kBlendModeNone;
				if (currentPass == RenderPassType::Particle) {
					auto* particle = static_cast<ParticleSystem*>(cmd.object);
					blendMode = particle->GetBlendMode();
				}
				it->second->BeginPass(cmdList_, blendMode);
			}
		}

		// オブジェクトを描画
		if (cmd.passType == RenderPassType::Particle) {
			// パーティクルシステム（ParticleRendererが描画）
			auto* particleRenderer = dynamic_cast<ParticleRenderer*>(renderers_[currentPass].get());
			if (particleRenderer && camera_) {
				particleRenderer->Draw(static_cast<ParticleSystem*>(cmd.object));
			}
		} else if (cmd.object->Is2D()) {
			// 2Dオブジェクト（カメラ不要）
			static_cast<Object2d*>(cmd.object)->Draw();
		} else {
			// 3Dオブジェクト（カメラ使用、ライトは自動セット）
			if (camera_) {
				static_cast<Object3d*>(cmd.object)->Draw(const_cast<ICamera*>(camera_));
			}
		}
	}

	// 最後のパスを終了
	if (currentPass != RenderPassType::Invalid) {
		auto it = renderers_.find(currentPass);
		if (it != renderers_.end()) {
			it->second->EndPass();
		}
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
