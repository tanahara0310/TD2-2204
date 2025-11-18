#include "Object3d.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Graphics/LineRenderer.h"

void Object3d::Update() {
	// デフォルト実装は空（派生クラスでオーバーライドすることを想定）
}

void Object3d::Draw(ICamera* camera) {
	// デフォルト実装は空（派生クラスでオーバーライドすることを想定）
	(void)camera;
}

void Object3d::DrawDebug(std::vector<LineRenderer::Line>& outLines) {
	// デフォルト実装は空（派生クラスでオーバーライドすることを想定）
	(void)outLines;
}

bool Object3d::DrawImGui() {
	// デフォルト実装は空（派生クラスでオーバーライドすることを想定）
	return false;
}

RenderPassType Object3d::GetRenderPassType() const {
	if (!model_) {
		return RenderPassType::Invalid;
	}
	
	// モデルのレンダータイプに基づいて描画パスを決定
	switch (model_->GetRenderType()) {
		case Model::RenderType::Normal:
			return RenderPassType::Model;
		case Model::RenderType::Skinning:
			return RenderPassType::SkinnedModel;
		default:
			return RenderPassType::Model;
	}
}
