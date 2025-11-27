#pragma once
#include "Collider.h"

class AABBCollider : public Collider {
public:
   AABBCollider(Object3d* owner, const Vector3& size);

   bool CheckCollision(Collider* other) const override;

   Vector3 GetMax() const;
   Vector3 GetMin() const;

   void SetSize(const Vector3& size)override { size_ = size; }
private:
   Vector3 size_;
};
