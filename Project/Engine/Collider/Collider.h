#pragma once
#include "CollisionLayer.h"
#include "MathCore.h"

enum class ColliderType {
   None,
   Sphere,
   AABB,
};

class Object3d;

class Collider {
public:
   virtual ~Collider() = default;

   virtual bool CheckCollision(Collider* other) const = 0;

   Vector3 GetPosition() const;
   ColliderType GetType() const;

   void OnCollisionEnter(Collider* other);
   void OnCollisionStay(Collider* other);
   void OnCollisionExit(Collider* other);

   void SetLayer(CollisionLayer layer) { layer_ = layer; }
   CollisionLayer GetLayer() const { return layer_; }
   
   void SetOwner(Object3d* owner) { owner_ = owner; }
   Object3d* GetOwner() const { return owner_; }

   virtual void SetSize(const Vector3& size) { (void)size; }

   virtual void SetRadius(float radius) { (void)radius; }
protected:
   ColliderType type_ = ColliderType::None;
   Object3d* owner_ = nullptr;
   CollisionLayer layer_ = CollisionLayer::Default;
};
