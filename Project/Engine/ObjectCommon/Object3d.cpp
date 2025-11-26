#include "Object3d.h"
#include "Camera/ICamera.h"
#include "Graphics/LineRenderer.h"
#include "Graphics/Material/MaterialManager.h"

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
   
   // オブジェクト名をCollapsingHeaderとして表示（タブ）
   if (ImGui::CollapsingHeader(GetObjectName())) {
      ImGui::PushID(GetObjectName());
      
      // ===== 基本情報 =====
      ImGui::SeparatorText("基本情報");
      
      // アクティブ状態の切り替え
      bool active = IsActive();
      if (ImGui::Checkbox("アクティブ", &active)) {
         SetActive(active);
         changed = true;
      }
      
      // レンダーパスタイプ（読み取り専用）
      RenderPassType passType = GetRenderPassType();
      const char* passTypeName = "Unknown";
      switch (passType) {
         case RenderPassType::Model: passTypeName = "Model"; break;
         case RenderPassType::SkinnedModel: passTypeName = "SkinnedModel"; break;
         case RenderPassType::Sprite: passTypeName = "Sprite"; break;
         case RenderPassType::Particle: passTypeName = "Particle"; break;
         case RenderPassType::ModelParticle: passTypeName = "ModelParticle"; break;
         case RenderPassType::SkyBox: passTypeName = "SkyBox"; break;
         default: passTypeName = "Invalid"; break;
      }
      ImGui::Text("レンダーパス: %s", passTypeName);
      
      // レンダータイプ（読み取り専用）
      if (model_) {
         const char* renderTypeName = (GetRenderType() == Model::RenderType::Normal) ? "Normal" : "Skinning";
         ImGui::Text("レンダータイプ: %s", renderTypeName);
      }
      
      // ブレンドモード（変更可能）
      static const char* blendModeItems[] = { 
         "None", "Normal", "Add", "Subtract", "Multiply", "Screen" 
      };
      int currentBlendMode = static_cast<int>(blendMode_);
      if (ImGui::Combo("ブレンドモード", &currentBlendMode, blendModeItems, IM_ARRAYSIZE(blendModeItems))) {
         blendMode_ = static_cast<BlendMode>(currentBlendMode);
         changed = true;
      }
      
      ImGui::Spacing();
      
      // ===== トランスフォーム制御 =====
      if (ImGui::TreeNode("トランスフォーム")) {
         Vector3& scale = transform_.scale;
         Vector3& rotate = transform_.rotate;
         Vector3& translate = transform_.translate;

         if (ImGui::DragFloat3("スケール", &scale.x, 0.01f)) {
            changed = true;
         }

         if (ImGui::DragFloat3("回転", &rotate.x, 0.01f)) {
            changed = true;
         }

         if (ImGui::DragFloat3("位置", &translate.x, 0.1f)) {
            changed = true;
         }

         if (ImGui::Button("トランスフォームをリセット")) {
            transform_.scale = { 1.0f, 1.0f, 1.0f };
            transform_.rotate = { 0.0f, 0.0f, 0.0f };
            changed = true;
         }

         ImGui::TreePop();
      }
      
      // ===== マテリアル制御 =====
      if (model_) {
         MaterialManager* mat = model_->GetMaterialManager();
         if (mat && ImGui::TreeNode("マテリアル")) {
            Vector4& colorVec = mat->GetMaterialData()->color;
            float col[4] = { colorVec.x, colorVec.y, colorVec.z, colorVec.w };
            if (ImGui::ColorEdit4("色", col)) {
               mat->SetColor({ col[0], col[1], col[2], col[3] });
               changed = true;
            }

            static const char* shadingItems[] = { "なし", "ランバート", "ハーフランバート", "トゥーン" };
            int currentShadingMode = mat->GetMaterialData()->shadingMode;
            if (ImGui::Combo("シェーディングモード", &currentShadingMode, shadingItems, IM_ARRAYSIZE(shadingItems))) {
               mat->GetMaterialData()->shadingMode = currentShadingMode;
               changed = true;
            }

            ImGui::Spacing();
            ImGui::SeparatorText("ディザリング設定");
            
            bool enableDithering = mat->IsEnableDithering();
            if (ImGui::Checkbox("ディザリング有効", &enableDithering)) {
               mat->SetEnableDithering(enableDithering);
               changed = true;
            }
            
            if (enableDithering) {
               ImGui::Indent();
               float ditheringScale = mat->GetDitheringScale();
               if (ImGui::SliderFloat("パターンスケール", &ditheringScale, 0.1f, 3.0f, "%.2f")) {
                  mat->SetDitheringScale(ditheringScale);
                  changed = true;
               }
               ImGui::TextWrapped("値が大きいほど粗いパターンになります");
               ImGui::Unindent();
            }

            ImGui::TreePop();
         }
      }
      
      // ===== オブジェクト階層情報 =====
      const WorldTransform* parent = transform_.GetParent();
      if (parent || ImGui::TreeNode("階層情報")) {
         if (!parent) {
            // TreeNodeを開いた場合のみ表示
            ImGui::Text("親: なし");
            ImGui::TreePop();
         } else {
            if (ImGui::TreeNode("階層情報")) {
               ImGui::Text("親: あり");
               
               // ローカル座標
               ImGui::Text("ローカル座標:");
               ImGui::Text("  (%.2f, %.2f, %.2f)", 
                  transform_.translate.x, 
                  transform_.translate.y, 
                  transform_.translate.z);
               
               // ワールド座標
               Vector3 worldPos = transform_.GetWorldPosition();
               ImGui::Text("ワールド座標:");
               ImGui::Text("  (%.2f, %.2f, %.2f)", 
                  worldPos.x, worldPos.y, worldPos.z);
               
               ImGui::TreePop();
            }
         }
      }
      
      // ===== パフォーマンス情報 =====
      if (model_ && ImGui::TreeNode("パフォーマンス情報")) {
         // モデルリソースから頂点数を取得
         // ModelResourceへのアクセスが必要なため、内部実装に依存
         // ここでは概算として表示
         ImGui::Text("モデル情報:");
         ImGui::Text("  描画状態: %s", IsActive() ? "アクティブ" : "非アクティブ");
         
         // レンダータイプに応じた情報
         if (GetRenderType() == Model::RenderType::Skinning) {
            ImGui::Text("  スキニング: 有効");
            if (model_->HasSkinCluster()) {
               ImGui::Text("  スキンクラスター: あり");
            }
            if (model_->GetSkeleton().has_value()) {
               const auto& skeleton = model_->GetSkeleton().value();
               ImGui::Text("  ジョイント数: %zu", skeleton.joints.size());
            }
         } else {
            ImGui::Text("  スキニング: なし");
         }
         
         // アニメーション情報
         if (model_->HasAnimationController()) {
            ImGui::Text("  アニメーション: 有効");
            ImGui::Text("  時刻: %.2f秒", model_->GetAnimationTime());
         } else {
            ImGui::Text("  アニメーション: なし");
         }
         
         ImGui::TreePop();
      }
      
      // ===== 拡張UI（派生クラスで実装） =====
      changed |= DrawImGuiExtended();
      
      if (changed) {
         transform_.TransferMatrix();
      }
      
      ImGui::PopID();
   }
   
   return changed;
#else
   return false;
#endif
}

bool Object3d::DrawImGuiExtended() {
   // デフォルト実装は空（派生クラスでオーバーライドして特殊機能を追加）
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
