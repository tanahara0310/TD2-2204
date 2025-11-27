#include "AABBCollider.h"
#include "SphereCollider.h"
#include "Utility/Collision/CollisionUtils.h"

AABBCollider::AABBCollider(Object3d* owner, const Vector3& size) {
   owner_ = owner;
   size_ = size;
   type_ = ColliderType::AABB;
}

bool AABBCollider::CheckCollision(Collider* other) const {
   if (other->GetType() == ColliderType::Sphere) {
	  const SphereCollider& s = static_cast<const SphereCollider&>(*other);
	  BoundingBox aabb = { GetMin(),GetMax() };
	  CollisionUtils::Sphere sphere = { s.GetPosition(), s.GetRadius() };
	  return CollisionUtils::IsColliding(sphere,aabb);
   } else if (other->GetType() == ColliderType::AABB) {
	  const AABBCollider& a = static_cast<const AABBCollider&>(*other);
	  BoundingBox aabb1 = { GetMin(),GetMax() };
	  BoundingBox aabb2 = { a.GetMin(), a.GetMax() };
	  return  CollisionUtils::IsColliding(aabb1, aabb2);
   }

   return false;
}

Vector3 AABBCollider::GetMax() const { return GetPosition() + size_ * 0.5f; }

Vector3 AABBCollider::GetMin() const { return GetPosition() - size_ * 0.5f; }
