#include "Object3d.h"
#include "Camera/ICamera.h"
#include "Graphics/LineRenderer.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

void Object3d::Update() {
   // デフォルト実装は空（派生クラスでオーバーライドすることを想定）
}

void Object3d::Draw(const ICamera* camera) {
   // デフォルト実装は空（派生クラスでオーバーライドすることを想定）
   (void)camera;
}

void Object3d::DrawDebug(std::vector<LineRenderer::Line>& outLines) {
   // デフォルト実装は空（派生クラスでオーバーライドすることを想定）
   (void)outLines;
}

bool Object3d::DrawImGui() {
#ifdef _DEBUG
   bool changed = false;
   
   // オブジェクト名でツリーノードを作成
   if (ImGui::TreeNode(GetObjectName())) {
      // アクティブ状態の切り替え
      bool active = IsActive();
      if (ImGui::Checkbox("アクティブ", &active)) {
         SetActive(active);
         changed = true;
      }
      
      // トランスフォームの表示と編集
      if (transform_.DrawImGui(GetObjectName())) {
         transform_.TransferMatrix();
         changed = true;
      }
      
      ImGui::TreePop();
   }
   
   return changed;
#else
   return false;
#endif
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
