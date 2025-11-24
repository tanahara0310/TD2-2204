#include "GameObject.h"

void GameObject::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
   auto engine = GetEngineSystem();
   auto dxCommon = engine->GetComponent<DirectXCommon>();

   {
	  model_ = std::move(model);
	  transform_.Initialize(dxCommon->GetDevice());
	  texture_ = texture;
   }
}

void GameObject::AttachCollider(std::unique_ptr<Collider> collider) {
   collider_ = std::move(collider);
}

void GameObject::AttachBehaviorRequestManager() {
   behavior_ = std::make_unique<BehaviorRequestManager>();
}