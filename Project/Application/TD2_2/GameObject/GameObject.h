#pragma once
#include <memory>
#include <vector>
#include "MathCore.h"
#include "Object3D.h"
#include "../Collider/Collider.h"

/// @brief シンプルなゲームオブジェクト基底
class GameObject :Object3d {
public:

   static void InitializeSystem(EngineSystem* engine);
   virtual void Update() override {}
   virtual void Draw(ICamera* camera) override { (void)camera; }

   Vector3 GetWorldPosition() const {
	  return transform_.GetWorldPosition();
   }

   void SetWorldPosition(const Vector3& position) {
	  transform_.translate = position;
   }

   // 衝突イベント
   virtual void OnCollisionEnter(GameObject* other) { (void)other; }
   virtual void OnCollisionStay(GameObject* other) { (void)other; }
   virtual void OnCollisionExit(GameObject* other) { (void)other; }

   EngineSystem* GetEngineSystem() const;

   Collider* GetCollider() const { return collider_.get(); }

protected:
   std::unique_ptr<Collider> collider_ = nullptr;
};
